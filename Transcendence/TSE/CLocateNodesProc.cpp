//	CLocateNodesProc.cpp
//
//	CLocateNodesProce class

#include "PreComp.h"

#define CRITERIA_TAG						CONSTLIT("Criteria")
#define MAP_FUNCTION_TAG					CONSTLIT("MapFunction")

#define PERCENTILE_ATTRIB					CONSTLIT("percentile")

struct SComputedNode
	{
	CTopologyNode *pNode;
	float rValue;
	};

int KeyCompare (const SComputedNode &Key1, const SComputedNode &Key2)
	{
	if (Key1.rValue > Key2.rValue)
		return 1;
	else if (Key1.rValue < Key2.rValue)
		return -1;
	else
		return 0;
	}

CLocateNodesProc::~CLocateNodesProc (void)

//	CLocateNodesProc destructor

	{
	int i;

	for (i = 0; i < m_Locations.GetCount(); i++)
		delete m_Locations[i].pProc;

	if (m_pMapFunction)
		delete m_pMapFunction;
	}

ALERROR CLocateNodesProc::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_Locations.GetCount(); i++)
		if (error = m_Locations[i].pProc->BindDesign(Ctx))
			return error;

	return NOERROR;
	}

CEffectCreator *CLocateNodesProc::OnFindEffectCreator (const CString &sUNID)

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
		if (iIndex < 0 || iIndex >= m_Locations.GetCount())
			return NULL;

		//	Let the processor handle it

		return m_Locations[iIndex].pProc->FindEffectCreator(CString(pPos));
		}

	//	Otherwise we have no clue

	else
		return NULL;
	}

ALERROR CLocateNodesProc::OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

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

		//	If this is a map function, read it

		else if (strEquals(pItem->GetTag(), MAP_FUNCTION_TAG))
			{
			if (pItem->GetContentElementCount() > 0)
				{
				if (m_pMapFunction)
					delete m_pMapFunction;

				if (error = I2DFunction::CreateFromXML(Ctx, pItem->GetContentElement(0), &m_pMapFunction))
					return error;
				}
			}

		//	Otherwise, treat it as a location definition and insert it in the list

		else
			{
			CString sNewUNID = strPatternSubst(CONSTLIT("%s/%d"), sUNID, m_Locations.GetCount());
			SLocation *pEntry = m_Locations.Insert();

			if (error = ITopologyProcessor::CreateFromXMLAsGroup(Ctx, pItem, sNewUNID, &pEntry->pProc))
				return error;

			//	Read the range

			if (error = ParseRange(Ctx, pItem->GetAttribute(PERCENTILE_ATTRIB), &pEntry->rMin, &pEntry->rMax))
				return error;
			}
		}

	return NOERROR;
	}

ALERROR CLocateNodesProc::OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError)

//	OnProcess
//
//	Process on topology

	{
	ALERROR error;
	int i, j;

	//	If no locations, then we're done

	if (m_Locations.GetCount() == 0)
		return NOERROR;

	//	If we have a criteria, the filter the nodes

	CTopologyNodeList FilteredNodeList;
	CTopologyNodeList *pNodeList = FilterNodes(Topology, m_Criteria, NodeList, FilteredNodeList);
	if (pNodeList == NULL)
		{
		*retsError = CONSTLIT("Error filtering nodes");
		return ERR_FAIL;
		}

	//	If no nodes, then we're done

	if (pNodeList->GetCount() == 0)
		return NOERROR;

	//	Compute a value for all nodes

	TArray<SComputedNode> Results;
	Results.InsertEmpty(pNodeList->GetCount());
	if (m_pMapFunction)
		{
		for (i = 0; i < pNodeList->GetCount(); i++)
			{
			Results[i].pNode = pNodeList->GetAt(i);

			int x, y;
			Results[i].pNode->GetDisplayPos(&x, &y);

			Results[i].rValue = m_pMapFunction->Eval((float)x, (float)y);
			}

		//	Sort the list by ascending value

		Results.Sort();
		}
	else
		{
		for (i = 0; i < pNodeList->GetCount(); i++)
			{
			Results[i].pNode = pNodeList->GetAt(i);
			Results[i].rValue = 1.0f;
			}
		}

	//	Mark all nodes in the list; we clear the mark when we
	//	process a node so each node is only processed in a single
	//	location.

	TArray<bool> SavedMarks;
	SavedMarks.InsertEmpty(pNodeList->GetCount());
	for (i = 0; i < pNodeList->GetCount(); i++)
		{
		SavedMarks[i] = pNodeList->GetAt(i)->IsMarked();
		pNodeList->GetAt(i)->SetMarked(true);
		}

	//	Loop over all locations

	for (i = 0; i < m_Locations.GetCount(); i++)
		{
		float rMin = m_Locations[i].rMin;
		float rMax = m_Locations[i].rMax;

		//	Generate a list of nodes that match this location

		CTopologyNodeList LocationNodes;
		for (j = 0; j < Results.GetCount(); j++)
			{
			CTopologyNode *pNode = Results[j].pNode;
			float rPercentile = ((float)j + 0.5f) / (float)Results.GetCount();
			if (pNode->IsMarked() && rPercentile >= rMin && rPercentile <= rMax)
				{
				pNode->SetMarked(false);
				LocationNodes.Insert(pNode);
				}
			}
		
		//	Process

		if (LocationNodes.GetCount() > 0)
			if (error = m_Locations[i].pProc->Process(pMap, Topology, LocationNodes, retsError))
				return error;
		}

	//	Remove from the original node list

	for (i = 0; i < pNodeList->GetCount(); i++)
		if (!pNodeList->GetAt(i)->IsMarked())
			NodeList.Delete(pNodeList->GetAt(i));

	//	Done

	for (i = 0; i < pNodeList->GetCount(); i++)
		pNodeList->GetAt(i)->SetMarked(SavedMarks[i]);

	return NOERROR;
	}

ALERROR CLocateNodesProc::ParseRange (SDesignLoadCtx &Ctx, const CString &sRange, float *retrMin, float *retrMax)

//	ParseRange
//
//	Parse a string of the form
//
//	{min}-{max}
//
//	Where {min} and {max} are integers from 0 to 100 and returns a floating point
//	range from 0.0 to 1.0

	{
	if (sRange.IsBlank())
		{
		*retrMin = 0.0f;
		*retrMax = 1.0f;
		}
	else
		{
		char *pPos = sRange.GetASCIIZPointer();
		while (*pPos == ' ')
			pPos++;

		int iFirst = strParseInt(pPos, 1, &pPos);
		int iSecond = -1;
		while (*pPos == ' ')
			pPos++;

		if (*pPos == '-')
			{
			pPos++;
			while (*pPos == ' ')
				pPos++;

			iSecond = strParseInt(pPos, 100, &pPos);
			}

		*retrMin = ((float)iFirst / 100.0f) - 0.005f;
		if (iSecond == -1)
			*retrMax = *retrMin + 0.01f;
		else
			*retrMax = ((float)iSecond / 100.0f) + 0.005f;
		}

	return NOERROR;
	}
