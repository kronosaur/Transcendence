//	CFillNodesProc.cpp
//
//	CFillNodesProc class

#include "PreComp.h"

#define ATTRIBUTES_TAG						CONSTLIT("Attributes")
#define CRITERIA_TAG						CONSTLIT("Criteria")
#define SYSTEM_TAG							CONSTLIT("System")

CFillNodesProc::~CFillNodesProc (void)

//	CFillNodesProc destructor

	{
	int i;

	for (i = 0; i < m_Procs.GetCount(); i++)
		delete m_Procs[i];
	}

ALERROR CFillNodesProc::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_Procs.GetCount(); i++)
		if (error = m_Procs[i]->BindDesign(Ctx))
			return error;

	return NOERROR;
	}

CEffectCreator *CFillNodesProc::OnFindEffectCreator (const CString &sUNID)

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

		return m_Procs[iIndex]->FindEffectCreator(CString(pPos));
		}

	//	Otherwise we have no clue

	else
		return NULL;
	}

ALERROR CFillNodesProc::OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

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

		//	Otherwise, load as a processor

		else
			{
			ITopologyProcessor *pNewProc;
			CString sNewUNID = strPatternSubst(CONSTLIT("%s/%d"), sUNID, m_Procs.GetCount());

			if (error = ITopologyProcessor::CreateFromXML(Ctx, pItem, sNewUNID, &pNewProc))
				return error;

			m_Procs.Insert(pNewProc);
			}
		}

	return NOERROR;
	}

ALERROR CFillNodesProc::OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError)

//	OnProcess
//
//	Process on topology

	{
	ALERROR error;
	int i;

	//	If no systems, then we're done

	int iProcCount = m_Procs.GetCount();
	if (iProcCount == 0)
		return NOERROR;

	//	If we have a criteria, the filter the nodes

	CTopologyNodeList FilteredNodeList;
	CTopologyNodeList *pNodeList = FilterNodes(Topology, m_Criteria, NodeList, FilteredNodeList);
	if (pNodeList == NULL)
		{
		*retsError = CONSTLIT("Error filtering nodes");
		return ERR_FAIL;
		}

	//	Loop over all nodes and place the systems in random order

	pNodeList->Shuffle();
	for (i = 0; i < pNodeList->GetCount(); i++)
		{
		CTopologyNodeList SingleNode;
		SingleNode.Insert(pNodeList->GetAt(i));

		if (error = m_Procs[i % iProcCount]->Process(pMap, Topology, SingleNode, retsError))
			return error;
		}

	//	Remove from the original node list

	if (pNodeList == &NodeList)
		NodeList.DeleteAll();
	else
		{
		for (i = 0; i < pNodeList->GetCount(); i++)
			NodeList.Delete(pNodeList->GetAt(i));
		}

	return NOERROR;
	}
