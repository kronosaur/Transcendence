//	ITopologyProcessor.cpp
//
//	ITopologyProcessor class

#include "PreComp.h"

#define ATTRIBUTES_TAG						CONSTLIT("Attributes")
#define CONQUER_NODES_TAG					CONSTLIT("ConquerNodes")
#define DISTRIBUTE_NODES_TAG				CONSTLIT("DistributeNodes")
#define FILL_NODES_TAG						CONSTLIT("FillNodes")
#define GROUP_TAG							CONSTLIT("Group")
#define LOCATE_NODES_TAG					CONSTLIT("LocateNodes")
#define PARTITION_NODES_TAG					CONSTLIT("PartitionNodes")
#define RANDOM_POINTS_TAG					CONSTLIT("RandomPoints")
#define SYSTEM_TAG							CONSTLIT("System")
#define TABLE_TAG							CONSTLIT("Table")

ALERROR ITopologyProcessor::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID, ITopologyProcessor **retpProc)

//	CreateFromXML
//
//	Creates a new processor based on the XML tag

	{
	ALERROR error;
	ITopologyProcessor *pProc;

	//	Create the approprate class

	if (strEquals(pDesc->GetTag(), ATTRIBUTES_TAG))
		pProc = new CApplySystemProc;
	else if (strEquals(pDesc->GetTag(), CONQUER_NODES_TAG))
		pProc = new CConquerNodesProc;
	else if (strEquals(pDesc->GetTag(), DISTRIBUTE_NODES_TAG))
		pProc = new CDistributeNodesProc;
	else if (strEquals(pDesc->GetTag(), FILL_NODES_TAG))
		pProc = new CFillNodesProc;
	else if (strEquals(pDesc->GetTag(), GROUP_TAG))
		pProc = new CGroupTopologyProc;
	else if (strEquals(pDesc->GetTag(), LOCATE_NODES_TAG))
		pProc = new CLocateNodesProc;
	else if (strEquals(pDesc->GetTag(), PARTITION_NODES_TAG))
		pProc = new CPartitionNodesProc;
	else if (strEquals(pDesc->GetTag(), RANDOM_POINTS_TAG))
		pProc = new CRandomPointsProc;
	else if (strEquals(pDesc->GetTag(), SYSTEM_TAG))
		pProc = new CApplySystemProc;
	else if (strEquals(pDesc->GetTag(), TABLE_TAG))
		pProc = new CTableTopologyProc;
	else
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unknown topology processor element: <%s>"), pDesc->GetTag());
		return ERR_FAIL;
		}

	//	Load it

	if (error = pProc->InitFromXML(Ctx, pDesc, sUNID))
		return error;

	//	Done

	*retpProc = pProc;

	return NOERROR;
	}

ALERROR ITopologyProcessor::CreateFromXMLAsGroup (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID, ITopologyProcessor **retpProc)

//	CreateFromXMLAsGroup
//
//	Creates a new CGroupTopologyProc from an XML element

	{
	*retpProc = new CGroupTopologyProc;
	return (*retpProc)->InitFromXML(Ctx, pDesc, sUNID);
	}

CTopologyNodeList *ITopologyProcessor::FilterNodes (CTopology &Topology, CTopologyNode::SCriteria &Criteria, CTopologyNodeList &Unfiltered, CTopologyNodeList &Filtered)

//	FilterNodes
//
//	Returns a list of filtered nodes

	{
	CTopologyNodeList *pNodeList = &Unfiltered;
	if (!CTopologyNode::IsCriteriaAll(Criteria))
		{
		//	Context

		CTopologyNode::SCriteriaCtx Ctx;
		Ctx.pTopology = &Topology;

		//	Filter

		if (Unfiltered.Filter(Ctx, Criteria, &Filtered) != NOERROR)
			return NULL;

		pNodeList = &Filtered;
		}

	return pNodeList;
	}

ALERROR ITopologyProcessor::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	InitFromXML
//
//	Initialize form XML element

	{
	//	Let subclass handle the rest.

	return OnInitFromXML(Ctx, pDesc, sUNID);
	}

void ITopologyProcessor::RestoreMarks (CTopology &Topology, TArray<bool> &Saved)

//	RestoreMarks
//
//	Restore the mark for nodes

	{
	Topology.GetTopologyNodeList().RestoreMarks(Saved);
	}

void ITopologyProcessor::SaveAndMarkNodes (CTopology &Topology, CTopologyNodeList &NodeList, TArray<bool> *retSaved)

//	SaveAndMarkNodes
//
//	Saves the mark for all topology nodes. Then it marks all nodes in the NodeList
//	and clears the mark on all others.

	{
	int i;

	Topology.GetTopologyNodeList().SaveAndSetMarks(false, retSaved);

	for (i = 0; i < NodeList.GetCount(); i++)
		NodeList[i]->SetMarked(true);
	}
