//	CConquerNodesProc.cpp
//
//	CConquerNodesProce class

#include "PreComp.h"

#define CRITERIA_TAG						CONSTLIT("Criteria")
#define EXPAND_CHANCE_TAG					CONSTLIT("ExpandChance")
#define PROCESSOR_TAG						CONSTLIT("Processor")
#define SEED_CHANCE_TAG						CONSTLIT("SeedChance")

#define CHANCE_ATTRIB						CONSTLIT("chance")
#define CRITERIA_ATTRIB						CONSTLIT("criteria")
#define EXPAND_CHANCE_ATTRIB				CONSTLIT("expandChance")
#define MAX_NODES_ATTRIB					CONSTLIT("maxNodes")
#define MAX_SEEDS_ATTRIB					CONSTLIT("maxSeeds")
#define SEED_CHANCE_ATTRIB					CONSTLIT("seedChance")
#define SUCCESS_CHANCE_ATTRIB				CONSTLIT("successChance")
#define WEIGHT_ATTRIB						CONSTLIT("weight")

CConquerNodesProc::~CConquerNodesProc (void)

//	CConquerNodesProc destructor

	{
	int i;

	for (i = 0; i < m_Conquerors.GetCount(); i++)
		delete m_Conquerors[i].pProc;
	}

void CConquerNodesProc::CalcAdjacentNodes (CTopologyNodeList &NodeList, CTopologyNodeList *retOutput)

//	CalcAdjacentNodes
//
//	Returns a list of all marked nodes that are adjacent to the given list

	{
	int i, j;

	for (i = 0; i < NodeList.GetCount(); i++)
		{
		for (j = 0; j < NodeList[i]->GetStargateCount(); j++)
			{
			CTopologyNode *pDest = NodeList[i]->GetStargateDest(j);
			if (pDest != NodeList[i] && pDest->IsMarked())
				{
				if (!retOutput->FindNode(pDest))
					retOutput->Insert(pDest);
				}
			}
		}
	}

int CConquerNodesProc::CalcNodeWeight (CTopologyNode *pNode, TArray<SNodeWeight> &Weights, int *retiSuccessChance)

//	CalcNodeWeight
//
//	Calculates the weight of the given node

	{
	int i;

	CTopologyNode::SCriteriaCtx Ctx;
	Ctx.pTopology = NULL;

	for (i = 0; i < Weights.GetCount(); i++)
		{
		if (pNode->MatchesCriteria(Ctx, Weights[i].Criteria))
			{
			if (retiSuccessChance)
				*retiSuccessChance = Weights[i].iSuccessChance;

			return Weights[i].iWeight;
			}
		}

	if (retiSuccessChance)
		*retiSuccessChance = 0;

	return 0;
	}

CTopologyNode *CConquerNodesProc::ChooseRandomNode (CTopologyNodeList &NodeList, TArray<SNodeWeight> &Weights)

//	ChooseRandomNode
//
//	Chooses a random node from the list, using Weights as a descriminator

	{
	int i;

	//	Generate a weight for each node

	TArray<int> Chance;
	TArray<int> Success;
	Chance.InsertEmpty(NodeList.GetCount());
	Success.InsertEmpty(NodeList.GetCount());

	int iTotalChance = 0;
	for (i = 0; i < NodeList.GetCount(); i++)
		if (NodeList[i]->IsMarked())
			{
			Chance[i] = CalcNodeWeight(NodeList[i], Weights, &Success[i]);
			iTotalChance += Chance[i];
			}
		else
			Chance[i] = 0;

	//	If nothing left, return

	if (iTotalChance == 0)
		return NULL;

	//	Pick a random node

	int iRoll = mathRandom(1, iTotalChance);
	for (i = 0; i < Chance.GetCount(); i++)
		{
		if (iRoll <= Chance[i])
			{
			if (mathRandom(1, 100) <= Success[i])
				return NodeList[i];
			else
				return NULL;
			}
		else
			iRoll -= Chance[i];
		}

	ASSERT(false);
	return NULL;
	}

void CConquerNodesProc::ConquerNode (SConqueror *pConqueror, SSeed *pSeed, CTopologyNode *pNode)

//	ConquerNode
//
//	Adds the node to a conquest

	{
	ASSERT(pNode->IsMarked());

	pConqueror->iNodeCount++;
	pSeed->Nodes.Insert(pNode);
	pNode->SetMarked(false);
	}

ALERROR CConquerNodesProc::LoadNodeWeightTable (SDesignLoadCtx &Ctx, CXMLElement *pDesc, TArray<SNodeWeight> *retTable)

//	LoadNodeWeightTable
//
//	Loads a node criteria/weight table

	{
	ALERROR error;
	int i;

	//	OK if NULL; it means the element is missing

	if (pDesc == NULL)
		return NOERROR;

	//	Load

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pChanceXML = pDesc->GetContentElement(i);
		SNodeWeight *pChance = retTable->Insert();

		if (error = CTopologyNode::ParseCriteria(pChanceXML->GetAttribute(CRITERIA_ATTRIB),
				&pChance->Criteria,
				&Ctx.sError))
			return error;

		pChance->iWeight = pChanceXML->GetAttributeIntegerBounded(WEIGHT_ATTRIB, 0, -1, 1);
		pChance->iSuccessChance = pChanceXML->GetAttributeIntegerBounded(SUCCESS_CHANCE_ATTRIB, 0, 100, 100);
		}

	return NOERROR;
	}

ALERROR CConquerNodesProc::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_Conquerors.GetCount(); i++)
		if (error = m_Conquerors[i].pProc->BindDesign(Ctx))
			return error;

	return NOERROR;
	}

CEffectCreator *CConquerNodesProc::OnFindEffectCreator (const CString &sUNID)

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
		if (iIndex < 0 || iIndex >= m_Conquerors.GetCount())
			return NULL;

		//	Let the processor handle it

		return m_Conquerors[iIndex].pProc->FindEffectCreator(CString(pPos));
		}

	//	Otherwise we have no clue

	else
		return NULL;
	}

ALERROR CConquerNodesProc::OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

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

		//	Otherwise, treat it as a conqueror definition and insert it in the list

		else
			{
			CString sNewUNID = strPatternSubst(CONSTLIT("%s/%d"), sUNID, m_Conquerors.GetCount());
			SConqueror *pConqueror = m_Conquerors.Insert();

			//	Get some properties

			pConqueror->iSeedChance = pItem->GetAttributeIntegerBounded(SEED_CHANCE_ATTRIB, 1, 100, 1);
			pConqueror->iMaxSeeds = pItem->GetAttributeIntegerBounded(MAX_SEEDS_ATTRIB, 1, -1, 1);
			pConqueror->iExpandChance = pItem->GetAttributeIntegerBounded(EXPAND_CHANCE_ATTRIB, 1, 100, 1);
			pConqueror->iMaxNodes = pItem->GetAttributeIntegerBounded(MAX_NODES_ATTRIB, 1, -1, 1);

			//	Get the seed and expand probabilities

			if (error = LoadNodeWeightTable(Ctx, pItem->GetContentElementByTag(SEED_CHANCE_TAG), &pConqueror->Seed))
				return error;

			if (error = LoadNodeWeightTable(Ctx, pItem->GetContentElementByTag(EXPAND_CHANCE_TAG), &pConqueror->Expand))
				return error;

			//	Get the processor itself

			CXMLElement *pProcXML = pItem->GetContentElementByTag(PROCESSOR_TAG);
			if (pProcXML)
				{

				if (error = ITopologyProcessor::CreateFromXMLAsGroup(Ctx, pProcXML, sNewUNID, &pConqueror->pProc))
					return error;
				}
			else
				{
				Ctx.sError = CONSTLIT("<Processor> element not found in <ConquerNodes>");
				return ERR_FAIL;
				}
			}
		}

	return NOERROR;
	}

ALERROR CConquerNodesProc::OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError)

//	OnProcess
//
//	Process on topology

	{
	ALERROR error;
	int i;

	//	If no conquerors, then we're done

	int iConquerorCount = m_Conquerors.GetCount();
	if (iConquerorCount == 0)
		return NOERROR;

	//	If we have a criteria, the filter the nodes

	CTopologyNodeList FilteredNodeList;
	CTopologyNodeList *pNodeList = FilterNodes(Topology, m_Criteria, NodeList, FilteredNodeList);
	if (pNodeList == NULL)
		{
		*retsError = CONSTLIT("Error filtering nodes");
		return ERR_FAIL;
		}

	//	We mark nodes that are available for us
	//	(So we first save the marks)

	TArray<bool> SavedMarks;
	SaveAndMarkNodes(Topology, *pNodeList, &SavedMarks);

	//	Initialize some conqueror temporaries

	for (i = 0; i < m_Conquerors.GetCount(); i++)
		{
		m_Conquerors[i].iSeedCount = 0;
		m_Conquerors[i].iNodeCount = 0;
		}

	//	We keep a structure for each seed

	TArray<SSeed> Seeds;

	//	Loop until we have placed all nodes in a partition

	int iMaxRounds = 10000;
	int iRounds = 0;
	int iNodesLeft = pNodeList->GetCount();
	while (iNodesLeft > 0 && iRounds < iMaxRounds)
		{
		//	See if any conqueror adds a new seed

		for (i = 0; i < m_Conquerors.GetCount(); i++)
			{
			SConqueror *pConqueror = &m_Conquerors[i];

			if ((pConqueror->iMaxSeeds == -1 || pConqueror->iSeedCount < pConqueror->iMaxSeeds)
					&& (pConqueror->iMaxNodes == -1 || pConqueror->iNodeCount < pConqueror->iMaxNodes)
					&& (mathRandom(1, 100) <= pConqueror->iSeedChance))
				{
				//	Pick a random available node

				CTopologyNode *pNewNode = ChooseRandomNode(*pNodeList, GetSeedWeightTable(pConqueror));
				if (pNewNode == NULL)
					continue;

				//	Add a seed

				SSeed *pNewSeed = Seeds.Insert();
				pNewSeed->pConqueror = pConqueror;

				//	Node is conquered

				ConquerNode(pConqueror, pNewSeed, pNewNode);
				iNodesLeft--;
				}
			}

		//	Now expand each seed

		for (i = 0; i < Seeds.GetCount(); i++)
			{
			SSeed *pSeed = &Seeds[i];
			SConqueror *pConqueror = pSeed->pConqueror;

			//	Compute the chance that this seed will expand

			int iExpandChance = pConqueror->iExpandChance;

			//	See if we expand

			if ((pConqueror->iMaxNodes == -1 || pConqueror->iNodeCount < pConqueror->iMaxNodes)
					&& (mathRandom(1, 100) <= iExpandChance))
				{
				//	Generate a list of all available adjacent nodes

				CTopologyNodeList NodesAvail;
				CalcAdjacentNodes(pSeed->Nodes, &NodesAvail);

				//	Pick a random node

				CTopologyNode *pNewNode = ChooseRandomNode(NodesAvail, GetExpandWeightTable(pConqueror));
				if (pNewNode == NULL)
					continue;

				//	Expand

				ConquerNode(pConqueror, pSeed, pNewNode);
				iNodesLeft--;
				}
			}

		iRounds++;
		}

	//	Run the processor on all seeds

	for (i = 0; i < Seeds.GetCount(); i++)
		{
		SSeed *pSeed = &Seeds[i];

		if (error = pSeed->pConqueror->pProc->Process(pMap, Topology, pSeed->Nodes, retsError))
			return error;
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
