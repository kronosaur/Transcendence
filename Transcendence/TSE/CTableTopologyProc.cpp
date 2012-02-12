//	CTableTopologyProc.cpp
//
//	CTableTopologyProc class

#include "PreComp.h"

#define CRITERIA_TAG						CONSTLIT("Criteria")

#define CHANCE_ATTRIB						CONSTLIT("chance")

CTableTopologyProc::~CTableTopologyProc (void)

//	CTableTopologyProc destructor

	{
	int i;

	for (i = 0; i < m_Procs.GetCount(); i++)
		delete m_Procs[i].pProc;
	}

ALERROR CTableTopologyProc::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_Procs.GetCount(); i++)
		if (error = m_Procs[i].pProc->BindDesign(Ctx))
			return error;

	return NOERROR;
	}

CEffectCreator *CTableTopologyProc::OnFindEffectCreator (const CString &sUNID)

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
		if (iIndex < 0 || iIndex >= m_Procs.GetCount())
			return NULL;

		//	Let the processor handle it

		return m_Procs[iIndex].pProc->FindEffectCreator(CString(pPos));
		}

	//	Otherwise we have no clue

	else
		return NULL;
	}

ALERROR CTableTopologyProc::OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

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

		//	Otherwise, load it as a processor

		else
			{
			CString sNewUNID = strPatternSubst(CONSTLIT("%s/%d"), sUNID, m_Procs.GetCount());
			SEntry *pEntry = m_Procs.Insert();

			if (error = ITopologyProcessor::CreateFromXML(Ctx, pItem, sNewUNID, &pEntry->pProc))
				return error;

			pEntry->iChance = pItem->GetAttributeIntegerBounded(CHANCE_ATTRIB, 0, -1, 0);
			}
		}

	//	Add up the total chance

	m_iTotalChance = 0;
	for (i = 0; i < m_Procs.GetCount(); i++)
		m_iTotalChance += m_Procs[i].iChance;

	return NOERROR;
	}

ALERROR CTableTopologyProc::OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError)

//	OnProcess
//
//	Process on topology

	{
	ALERROR error;
	int i;

	//	If no processors, then we're done

	if (m_iTotalChance == 0)
		return NOERROR;

	//	If we have a criteria, the filter the nodes

	CTopologyNodeList FilteredNodeList;
	CTopologyNodeList *pNodeList = FilterNodes(Topology, m_Criteria, NodeList, FilteredNodeList);
	if (pNodeList == NULL)
		{
		*retsError = CONSTLIT("Error filtering nodes");
		return ERR_FAIL;
		}

	//	Pick a random value

	int iRoll = mathRandom(1, m_iTotalChance);

	//	Loop over all processors in order
	//	(Each call will potentially reduce the node list)

	for (i = 0; i < m_Procs.GetCount(); i++)
		{
		if (iRoll <= m_Procs[i].iChance)
			{
			if (error = m_Procs[i].pProc->Process(pMap, Topology, *pNodeList, retsError))
				return error;

			break;
			}
		else
			iRoll -= m_Procs[i].iChance;
		}

	return NOERROR;
	}
