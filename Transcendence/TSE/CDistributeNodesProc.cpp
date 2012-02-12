//	CDistributeNodesProc.cpp
//
//	CDistributeNodesProce class

#include "PreComp.h"

#define CRITERIA_TAG						CONSTLIT("Criteria")

CDistributeNodesProc::~CDistributeNodesProc (void)

//	CDistributeNodesProc destructor

	{
	int i;

	for (i = 0; i < m_Systems.GetCount(); i++)
		delete m_Systems[i].pDesc;
	}

ALERROR CDistributeNodesProc::OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnInitFromXML
//
//	Initialize from XML element

	{
	ALERROR error;
	int i;

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

		//	Otherwise, treat it as a system definition and insert it in the list

		else
			{
			SSystemEntry *pEntry = m_Systems.Insert();
			pEntry->pDesc = pItem->OrphanCopy();
			}
		}

	return NOERROR;
	}

ALERROR CDistributeNodesProc::OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError)

//	OnProcess
//
//	Process on topology

	{
	ALERROR error;
	int i, j;

	CTopologyNodeList NodesToDelete;

	//	If no systems, then we're done

	if (m_Systems.GetCount() == 0)
		return NOERROR;

	//	If we have a criteria, the filter the nodes

	CTopologyNodeList FilteredNodeList;
	CTopologyNodeList *pNodeList = FilterNodes(Topology, m_Criteria, NodeList, FilteredNodeList);
	if (pNodeList == NULL)
		{
		*retsError = CONSTLIT("Error filtering nodes");
		return ERR_FAIL;
		}

	//	We better have enough nodes

	if (pNodeList->GetCount() < m_Systems.GetCount())
		{
		*retsError = CONSTLIT("<DistributeNodes>: Not enough available nodes.");
		return ERR_FAIL;
		}

	//	If we don't have any constraints between nodes then we just
	//	pick random nodes from the list

	if (m_Criteria.iMinInterNodeDist == 0 && m_Criteria.iMaxInterNodeDist == -1)
		{
		pNodeList->Shuffle();

		for (i = 0; i < m_Systems.GetCount(); i++)
			{
			if (error = pNodeList->GetAt(i)->InitFromAdditionalXML(m_Systems[i].pDesc, retsError))
				return error;

			//	Remove this node from NodeList so that it is not re-used by our callers

			NodesToDelete.Insert(pNodeList->GetAt(i));
			}
		}

	//	Otherwise, we randomly pick nodes until they match the constraints

	else
		{
		pNodeList->Shuffle();

		int iNodeCount = pNodeList->GetCount();
		int iSystemCount = m_Systems.GetCount();

		int iOffset = 0;
		TArray<int> Chosen;
		Chosen.InsertEmpty(iSystemCount);

		int iBestMatch = -1;
		TArray<int> Best;

		int iTries = 100;
		while (iTries-- > 0)
			{
			//	Assign nodes in order

			for (i = 0; i < iSystemCount; i++)
				Chosen[i] = (i + iOffset) % iNodeCount;

			//	Compute the minimum and maximum internode distance

			int iMin = INFINITE_NODE_DIST;
			int iMax = 0;
			for (i = 0; i < iSystemCount - 1; i++)
				for (j = i + 1; j < iSystemCount; j++)
					{
					int iDist = Topology.GetDistance(pNodeList->GetAt(Chosen[i])->GetID(), pNodeList->GetAt(Chosen[j])->GetID());
					if (iDist > iMax)
						iMax = iDist;
					if (iDist < iMin)
						iMin = iDist;
					}

			//	Compute how close we are to meeting the criteria (lower numbers
			//	are better).

			int iMinDiff = (iMin < m_Criteria.iMinInterNodeDist ? m_Criteria.iMinInterNodeDist - iMin : 0);
			int iMaxDiff = (m_Criteria.iMaxInterNodeDist != -1 && iMax > m_Criteria.iMaxInterNodeDist ? iMax - m_Criteria.iMaxInterNodeDist : 0);
			int iMatch = (iMinDiff * iMinDiff) + (iMaxDiff * iMaxDiff);

			//	Even if we don't match the criteria, see if this is a better match
			//	than anything else.

			if (iBestMatch == -1 || iMatch < iBestMatch)
				{
				iBestMatch = iMatch;
				Best = Chosen;
				}

			//	If we match the criteria perfectly, then we're done

			if (iMatch == 0)
				break;

			//	Otherwise, move down the line of random nodes and loop

			iOffset += 1;
			if (iOffset >= iNodeCount)
				{
				pNodeList->Shuffle();
				iOffset = 0;
				}
			}

		//	Apply the systems

		for (i = 0; i < iSystemCount; i++)
			{
			if (error = pNodeList->GetAt(Best[i])->InitFromSystemXML(m_Systems[i].pDesc, retsError))
				return error;

			//	Remove this node from NodeList so that it is not re-used by our callers

			NodesToDelete.Insert(pNodeList->GetAt(Best[i]));
			}
		}

	//	Delete

	for (i = 0; i < NodesToDelete.GetCount(); i++)
		NodeList.Delete(NodesToDelete[i]);

	return NOERROR;
	}
