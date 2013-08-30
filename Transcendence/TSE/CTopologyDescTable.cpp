//	CTopologDescTable.cpp
//
//	CTopologyDescTable class

#include "PreComp.h"

#define NODE_TAG							CONSTLIT("Node")

#define ID_ATTRIB							CONSTLIT("ID")
#define ROOT_NODE_ATTRIB					CONSTLIT("rootNode")

#define DEST_ID_ATTRIB						CONSTLIT("destID")
#define DEST_GATE_ATTRIB					CONSTLIT("destGate")
#define FROM_ID_ATTRIB						CONSTLIT("fromID")
#define FROM_GATE_ATTRIB					CONSTLIT("fromGate")

CTopologyDescTable::CTopologyDescTable (void) : m_pIDToDesc(NULL)

//	CTopologyDescTable constructor

	{
	}

CTopologyDescTable::~CTopologyDescTable (void)

//	CTopologyDescTable destructor

	{
	CleanUp();
	}

ALERROR CTopologyDescTable::AddRootNode (SDesignLoadCtx &Ctx, const CString &sNodeID)

//	AddRootNode
//
//	Adds the given root node

	{
	//	Don't bother if it is already a root node

	if (FindRootNodeDesc(sNodeID))
		return NOERROR;

	//	Look for the node

	CTopologyDesc *pNode = FindTopologyDesc(sNodeID);
	if (pNode == NULL)
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unable to find root node desc: %s"), sNodeID);
		return ERR_FAIL;
		}

	//	Add it to our table of root nodes

	m_RootNodes.Insert(pNode);

	//	Mark the node

	pNode->SetRootNode();

	//	If we don't have a first node yet, set it

	if (m_sFirstNode.IsBlank())
		m_sFirstNode = sNodeID;

	//	Done

	return NOERROR;
	}

ALERROR CTopologyDescTable::BindDesign (SDesignLoadCtx &Ctx)

//	BindDesign
//
//	Bind the design

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		if (error = m_Table[i]->BindDesign(Ctx))
			return error;

	return NOERROR;
	}

void CTopologyDescTable::CleanUp (void)

//	CleanUp
//
//	Clean up table

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		delete m_Table[i];

	m_Table.DeleteAll();

	if (m_pIDToDesc)
		{
		delete m_pIDToDesc;
		m_pIDToDesc = NULL;
		}
	}

CEffectCreator *CTopologyDescTable::FindEffectCreator (const CString &sUNID)

//	FindEffectCreator
//
//	Finds the effect creator with the given UNID (in this case, then node ID)
//
//	{unid}/{nodeID}:l
//	{unid}/{nodeID}:m
//	{unid}/{nodeID}/{subNodeID}
//		   ^

	{
	//	Parse out the next part

	char *pPos = sUNID.GetASCIIZPointer();
	char *pStart = pPos;
	while (*pPos != '\0' && *pPos != '/' && *pPos != ':')
		pPos++;

	CString sPart(pStart, pPos - pStart);

	//	Look for the node

	CTopologyDesc *pNode = FindTopologyDesc(sPart);
	if (pNode == NULL)
		return NULL;

	//	Recurse

	return pNode->FindEffectCreator(CString(pPos));
	}

CTopologyDesc *CTopologyDescTable::FindTopologyDesc (const CString &sID)

//	FindTopologyDesc
//
//	Returns a topology node descriptor of the given ID or returns NULL
//	if the node is not found

	{
	InitIDMap();
	CTopologyDesc **pFound = m_pIDToDesc->Find(sID);

	return (pFound ? *pFound : NULL);
	}

CXMLElement *CTopologyDescTable::FindTopologyDescXML (const CString &sNodeID)

//	FindTopologyDesc
//
//	Returns a topology node descriptor of the given ID or returns NULL
//	if the node is not found

	{
	CTopologyDesc *pNodeDesc = FindTopologyDesc(sNodeID);
	if (pNodeDesc == NULL)
		return NULL;

	return pNodeDesc->GetDesc();
	}

CTopologyDesc *CTopologyDescTable::FindRootNodeDesc (const CString &sID)

//	FindRootNodeDesc
//
//	Finds the given root node.

	{
	int i;
	for (i = 0; i < GetRootNodeCount(); i++)
		if (strEquals(sID, m_RootNodes[i]->GetID()))
			return m_RootNodes[i];

	return NULL;
	}

CXMLElement *CTopologyDescTable::FindRootNodeDescXML (const CString &sNodeID)

//	FindRootNodeDescXML
//
//	Finds the given root node.

	{
	int i;
	for (i = 0; i < GetRootNodeCount(); i++)
		if (strEquals(sNodeID, m_RootNodes[i]->GetID()))
			return GetRootNodeDescXML(i);

	return NULL;
	}

void CTopologyDescTable::InitIDMap (void)

//	InitIDMap
//
//	Makes sure that the ID map is initialized

	{
	int i;

	if (m_pIDToDesc == NULL)
		{
		m_pIDToDesc = new TMap<CString, CTopologyDesc *>;

		for (i = 0; i < m_Table.GetCount(); i++)
			{
			const CString &sID = m_Table[i]->GetID();
			if (!sID.IsBlank())
				m_pIDToDesc->Insert(sID, m_Table[i]);
			}
		}
	}

ALERROR CTopologyDescTable::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CSystemMap *pMap, const CString &sUNID, bool bAddFirstAsRoot)

//	LoadFromXML
//
//	Load the table from an XML element (OK if this gets called multiple times)

	{
	ALERROR error;
	int i;

	bool bRootAdded = false;
	CTopologyDesc *pFirstNode = NULL;
	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pNode = pDesc->GetContentElement(i);

		//	Create a new node desc

		CTopologyDesc *pNodeDesc;
		if (error = LoadNodeFromXML(Ctx, pNode, pMap, sUNID, &pNodeDesc))
			return error;
		
		//	If this is a root node remember that we added it

		if (pNodeDesc->IsRootNode())
			bRootAdded = true;

		//	Keep track of the first node in case we find no other root node
		//	(We only consider Nodes, skipping NodeTables and Fragments)

		if (pFirstNode == NULL && pNodeDesc->GetType() == ndNode)
			pFirstNode = pNodeDesc;
		}

	//	If no system is marked as a root node then we take the first node in the list

	if (pFirstNode 
			&& (m_RootNodes.GetCount() == 0 || (!bRootAdded && bAddFirstAsRoot)))
		{
		pFirstNode->SetRootNode();
		m_RootNodes.Insert(pFirstNode);

		if (m_sFirstNode.IsBlank())
			m_sFirstNode = pFirstNode->GetID();
		}

	return NOERROR;
	}

ALERROR CTopologyDescTable::LoadNodeFromXML (SDesignLoadCtx &Ctx, CXMLElement *pNode, CSystemMap *pMap, const CString &sParentUNID, CTopologyDesc **retpNode)

//	LoadNodeFromXML
//
//	Loads a single node
//
//	NOTE: It is OK if pNode does not have an ID. In that case we expect that our caller knows
//	what its doing. (For example, the node table inside <Random> does not need IDs, since we autogenerate them).

	{
	ALERROR error;

	//	Create a new node desc

	CTopologyDesc *pNodeDesc = new CTopologyDesc;
	if (error = pNodeDesc->LoadFromXML(Ctx, pNode, pMap, sParentUNID))
		return error;

	//	If this is a root node, add it to our list

	if (pNodeDesc->IsRootNode())
		{
		m_RootNodes.Insert(pNodeDesc);

		//	Remember the first root node

		if (m_sFirstNode.IsBlank())
			m_sFirstNode = pNodeDesc->GetID();
		}

	//	Get the ID

	const CString &sID = pNodeDesc->GetID();

	//	Add it to the ID map, if necessary

	if (!sID.IsBlank())
		{
		InitIDMap();

		//	Make sure we don't have duplicates

		if (m_pIDToDesc->Find(sID))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Duplicate topology node desc: %s"), sID);
			return ERR_FAIL;
			}

		//	Add to table

		m_pIDToDesc->Insert(sID, pNodeDesc);
		}

	//	Add it to the table

	m_Table.Insert(pNodeDesc);

	//	Done

	if (retpNode)
		*retpNode = pNodeDesc;

	return NOERROR;
	}
