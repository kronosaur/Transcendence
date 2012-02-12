//	CPartitionNodesProc.cpp
//
//	CPartitionNodesProce class

#include "PreComp.h"

#define CRITERIA_TAG						CONSTLIT("Criteria")

#define MAX_PARTITION_COUNT_ATTRIB			CONSTLIT("maxPartitionCount")
#define NODE_COUNT_ATTRIB					CONSTLIT("nodeCount")
#define PARTITION_ORDER_ATTRIB				CONSTLIT("partitionOrder")

#define PARTITION_ORDER_RANDOM				CONSTLIT("random")

CPartitionNodesProc::~CPartitionNodesProc (void)

//	CPartitionNodesProc destructor

	{
	int i;

	for (i = 0; i < m_Partitions.GetCount(); i++)
		delete m_Partitions[i].pProc;
	}

int CPartitionNodesProc::CreatePartitionRandomWalk (int iCount, CTopologyNode *pStart, CTopologyNodeList *retList)

//	CreatePartitionRandomWalk
//
//	Adds the given number of contiguous nodes, start at pStart, to the list
//
//	(Nodes are unmarked to indicate that they are no longer available for being
//	added to a partition.)

	{
	int i, j;

	CTopologyNode *pNode = pStart;
	for (i = 0; pNode && i < iCount; i++)
		{
		//	Add the node to the partition

		retList->Insert(pNode);
		pNode->SetMarked(false);

		//	Make a list of all adjacent nodes

		TArray<CTopologyNode *> AdjacentNodes;
		for (j = 0; j < pNode->GetStargateCount(); j++)
			{
			CTopologyNode *pDest = pNode->GetStargateDest(j);
			if (pDest != pNode && pDest->IsMarked())
				AdjacentNodes.Insert(pDest);
			}

		//	Pick a random node

		if (AdjacentNodes.GetCount())
			pNode = AdjacentNodes[mathRandom(0, AdjacentNodes.GetCount() - 1)];
		else
			pNode = NULL;
		}

	//	Done

	return retList->GetCount();
	}

bool CPartitionNodesProc::IsAvailable (SPartition &Partition)

//	IsAvailable
//
//	Returns TRUE if the given partition can be used

	{
	return ((Partition.iMaxPartitions == -1 
				|| Partition.iPartitionsCreated < Partition.iMaxPartitions)
			&& Partition.NodeCount.GetMaxValue() > 0);
	}

ALERROR CPartitionNodesProc::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_Partitions.GetCount(); i++)
		if (error = m_Partitions[i].pProc->BindDesign(Ctx))
			return error;

	return NOERROR;
	}

CEffectCreator *CPartitionNodesProc::OnFindEffectCreator (const CString &sUNID)

//	OnFindEffectCreator
//
//	Finds the effect creator
//
//	{unid}:p{index}/{index}
//		           ^

	{
	char *pPos = sUNID.GetASCIIZPointer();

	//	If we've got a slash, then recurse down

	if (*pPos == '/')
		{
		pPos++;

		//	Get the processor index

		int iIndex = ::strParseInt(pPos, -1, &pPos);
		if (iIndex < 0 || iIndex >= m_Partitions.GetCount())
			return NULL;

		//	Let the processor handle it

		return m_Partitions[iIndex].pProc->FindEffectCreator(CString(pPos));
		}

	//	Otherwise we have no clue

	else
		return NULL;
	}

ALERROR CPartitionNodesProc::OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnInitFromXML
//
//	Initialize from XML element

	{
	ALERROR error;
	int i;

	//	Load the partition order

	CString sAttrib = pDesc->GetAttribute(PARTITION_ORDER_ATTRIB);
	if (strEquals(sAttrib, PARTITION_ORDER_RANDOM))
		m_iOrder = orderRandom;
	else
		m_iOrder = orderFixed;

	//	Initialize criteria

	CTopologyNode::ParseCriteria(NULL, &m_Criteria);

	//	Loop over all elements

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pDesc->GetContentElement(i);

		//	If we have a criteria, parse it and remember it
		//	(Note: If multiple criteria are found, we take the latest one).

		if (strEquals(pItem->GetTag(), CRITERIA_TAG))
			{
			//	Parse the filter

			if (error = CTopologyNode::ParseCriteria(pItem, &m_Criteria, &Ctx.sError))
				return error;
			}

		//	Otherwise, treat it as a topology processor definition and insert it in the list

		else
			{
			CString sNewUNID = strPatternSubst(CONSTLIT("%s/%d"), sUNID, m_Partitions.GetCount());
			SPartition *pPart = m_Partitions.Insert();
			
			if (error = ITopologyProcessor::CreateFromXMLAsGroup(Ctx, pItem, sNewUNID, &pPart->pProc))
				return error;

			CString sPercent;
			if (error = pPart->NodeCount.LoadFromXML(pItem->GetAttribute(NODE_COUNT_ATTRIB), &sPercent))
				{
				Ctx.sError = CONSTLIT("Invalid nodeCount in <PartitionNodes>");
				return error;
				}

			pPart->bNodeCountPercent = strEquals(sPercent, CONSTLIT("%"));
			pPart->iMaxPartitions = pItem->GetAttributeIntegerBounded(MAX_PARTITION_COUNT_ATTRIB, 0, -1, -1);
			}
		}

	return NOERROR;
	}

ALERROR CPartitionNodesProc::OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError)

//	OnProcess
//
//	Process on topology

	{
	ALERROR error;
	int i, j;

	//	If no partitions, then we're done

	int iPartitionCount = m_Partitions.GetCount();
	if (iPartitionCount == 0)
		return NOERROR;

	//	If we have a criteria, the filter the nodes

	CTopologyNodeList FilteredNodeList;
	CTopologyNodeList *pNodeList = FilterNodes(Topology, m_Criteria, NodeList, FilteredNodeList);
	if (pNodeList == NULL)
		{
		*retsError = CONSTLIT("Error filtering nodes");
		return ERR_FAIL;
		}

	//	Figure out the order in which we assign partitions

	switch (m_iOrder)
		{
		case orderFixed:
			break;

		case orderRandom:
			m_Partitions.Shuffle();
			break;

		default:
			ASSERT(false);
		}

	//	Initialize some partition temporaries

	for (i = 0; i < m_Partitions.GetCount(); i++)
		{
		m_Partitions[i].iPartitionsCreated = 0;
		}

	//	We mark nodes that are available for being placed in a partition
	//	(So we first save the marks)

	TArray<bool> SavedMarks;
	SaveAndMarkNodes(Topology, *pNodeList, &SavedMarks);

	//	Loop until we have placed all nodes in a partition

	int iLoopsWithoutProgress = 0;
	int iPartition = 0;
	int iNodesLeft = pNodeList->GetCount();
	while (iNodesLeft > 0)
		{
		//	Find the node with the fewest available stargates

		CTopologyNode *pStartNode = NULL;
		int iMinGates = -1;
		for (i = 0; i < pNodeList->GetCount(); i++)
			{
			CTopologyNode *pNode = pNodeList->GetAt(i);
			if (pNode->IsMarked())
				{
				int iGateCount = 0;
				for (j = 0; j < pNode->GetStargateCount(); j++)
					{
					CTopologyNode *pDest = pNode->GetStargateDest(j);
					if (pDest != pNode && pDest->IsMarked())
						iGateCount++;
					}

				if (iMinGates == -1 || iGateCount < iMinGates)
					{
					iMinGates = iGateCount;
					pStartNode = pNode;

					//	Can't get smaller than 0, so stop looking

					if (iMinGates == 0)
						break;
					}
				}
			}

		//	Figure out how many nodes we want in the partition

		int iNodeCount = m_Partitions[iPartition].NodeCount.Roll();
		if (m_Partitions[iPartition].bNodeCountPercent)
			iNodeCount = pNodeList->GetCount() * iNodeCount / 100;

		//	Generate a list of contiguous nodes for the partition
		//	This also unmarks any nodes chosen.

		CTopologyNodeList PartitionNodes;
		CreatePartitionRandomWalk(iNodeCount, pStartNode, &PartitionNodes);

		//	Apply the processing rules to all nodes in the partition

		if (PartitionNodes.GetCount() > 0)
			{
			//	Subtract this first because the call to process will remove
			//	all the nodes from the list.

			iNodesLeft -= PartitionNodes.GetCount();

			//	Process recursively

			if (error = m_Partitions[iPartition].pProc->Process(pMap, Topology, PartitionNodes, retsError))
				return error;

			//	If we're making progress, reset our loop counter

			iLoopsWithoutProgress = 0;
			}

		//	Increment the number of times that we've used this partition

		m_Partitions[iPartition].iPartitionsCreated++;

		//	Next partition

		int iLast = iPartition;
		do
			{
			iPartition = (iPartition + 1) % iPartitionCount;
			}
		while (!IsAvailable(m_Partitions[iPartition]) && iLast != iPartition);

		//	If we couldn't find a partition, then we can't continue

		if (!IsAvailable(m_Partitions[iPartition]))
			break;

		//	If we keep looping, exit so we avoid infinity

		if (++iLoopsWithoutProgress >= 1000)
			break;
		}

	//	Remove from the original node list

	if (pNodeList == &NodeList && iNodesLeft == 0)
		NodeList.DeleteAll();
	else
		{
		for (i = 0; i < pNodeList->GetCount(); i++)
			NodeList.Delete(pNodeList->GetAt(i));
		}

	//	Done

	RestoreMarks(Topology, SavedMarks);

	return NOERROR;
	}
