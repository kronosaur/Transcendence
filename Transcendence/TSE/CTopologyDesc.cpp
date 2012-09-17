//	CTopologDesc.cpp
//
//	CTopologyDesc class

#include "PreComp.h"

#define FRAGMENT_TAG							CONSTLIT("Fragment")
#define LABEL_EFFECT_TAG						CONSTLIT("LabelEffect")
#define MAP_EFFECT_TAG							CONSTLIT("MapEffect")
#define NETWORK_TAG								CONSTLIT("Network")
#define NODE_TAG								CONSTLIT("Node")
#define NODE_GROUP_TAG							CONSTLIT("NodeGroup")
#define NODES_TAG								CONSTLIT("Nodes")
#define NODE_TABLE_TAG							CONSTLIT("NodeTable")
#define RANDOM_TAG								CONSTLIT("Random")
#define SET_NODES_TAG							CONSTLIT("SetNodes")
#define SYSTEM_TAG								CONSTLIT("System")

#define ATTRIBUTES_ATTRIB						CONSTLIT("attributes")
#define END_GAME_ATTRIB							CONSTLIT("endGame")
#define END_GAME_REASON_ATTRIB					CONSTLIT("endGameReason")
#define EPITAPH_ATTRIB							CONSTLIT("epitaph")
#define ID_ATTRIB								CONSTLIT("ID")
#define LABEL_EFFECT_ATTRIB						CONSTLIT("labelEffect")
#define MAP_EFFECT_ATTRIB						CONSTLIT("mapEffect")
#define POS_ATTRIB								CONSTLIT("pos")
#define ROOT_NODE_ATTRIB						CONSTLIT("rootNode")
#define X_ATTRIB								CONSTLIT("x")
#define Y_ATTRIB								CONSTLIT("y")

CTopologyDesc::CTopologyDesc (void) :
		m_pDesc(NULL),
		m_pDescList(NULL)

//	CTopologyDesc constructor

	{
	}

CTopologyDesc::~CTopologyDesc (void)

//	CTopologyDesc destructor

	{
	if (m_pDesc)
		delete m_pDesc;

	if (m_pDescList)
		delete m_pDescList;
	}

ALERROR CTopologyDesc::BindDesign (SDesignLoadCtx &Ctx)

//	BindDesign
//
//	Bind

	{
	ALERROR error;

	//	Effect

	if (error = m_pLabelEffect.Bind(Ctx))
		return error;

	if (error = m_pMapEffect.Bind(Ctx))
		return error;

	//	Recurse

	if (m_pDescList)
		if (error = m_pDescList->BindDesign(Ctx))
			return error;

	return NOERROR;
	}

CEffectCreator *CTopologyDesc::FindEffectCreator (const CString &sUNID)

//	FindEffectCreator
//
//	Finds the effect creator with the given UNID (in this case, then node ID)
//
//	{unid}/{nodeID}:m
//	{unid}/{nodeID}:l
//	{unid}/{nodeID}/{subNodeID}
//		           ^

	{
	//	If we've got another part, then we recurse

	char *pPos = sUNID.GetASCIIZPointer();
	if (*pPos == '/')
		{
		pPos++;

		if (m_pDescList == NULL)
			return NULL;

		return m_pDescList->FindEffectCreator(CString(pPos));
		}

	//	Otherwise, we load one of our effects

	else if (*pPos == ':')
		{
		pPos++;

		//	Map effect

		if (*pPos == 'm')
			return GetMapEffect();

		//	Label effect

		else if (*pPos == 'l')
			return GetLabelEffect();

		//	Unknown

		else
			return NULL;
		}

	//	Otherwise, we have no clue

	else
		return NULL;
	}

CString CTopologyDesc::GetAttributes (void)

//	GetAttributes
//
//	Returns the attributes for the node

	{
	return m_pDesc->GetAttribute(ATTRIBUTES_ATTRIB);
	}

bool CTopologyDesc::GetPos (int *retx, int *rety)

//	GetPos
//
//	Get the position of the node

	{
	CString sCoords;
	if (m_pDesc->FindAttribute(POS_ATTRIB, &sCoords))
		{
		CTopologyNode::ParsePosition(sCoords, retx, rety);
		return true;
		}
	else
		{
		bool bOK = false;

		if (m_pDesc->FindAttributeInteger(X_ATTRIB, retx))
			bOK = true;
		else
			*retx = 0;

		if (m_pDesc->FindAttributeInteger(Y_ATTRIB, rety))
			bOK = true;
		else
			*rety = 0;

		return bOK;
		}
	}

CXMLElement *CTopologyDesc::GetSystem (void)

//	GetSystem
//
//	Get the system definition (may be NULL)

	{
	return m_pDesc->GetContentElementByTag(SYSTEM_TAG);
	}

bool CTopologyDesc::IsEndGameNode (CString *retsEpitaph, CString *retsReason) const

//	IsEndGameNode
//
//	Returns TRUE if this is the end game

	{
	if (m_pDesc->GetAttributeBool(END_GAME_ATTRIB))
		{
		if (retsEpitaph)
			*retsEpitaph = m_pDesc->GetAttribute(EPITAPH_ATTRIB);

		if (retsReason)
			*retsReason = m_pDesc->GetAttribute(END_GAME_REASON_ATTRIB);

		return true;
		}
	else
		return false;
	}

ALERROR CTopologyDesc::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pXMLDesc, CSystemMap *pMap, const CString &sParentUNID)

//	LoadFromXML
//
//	Load

	{
	ALERROR error;

	m_pMap = pMap;

	//	Keep a copy of the full node descriptor (because we are too lazy to turn
	//	it into a structure).

	m_pDesc = pXMLDesc->OrphanCopy();

	//	Initialize ID

	m_sID = pXMLDesc->GetAttribute(ID_ATTRIB);

	//	Figure out the type

	if (strEquals(pXMLDesc->GetTag(), NODE_TAG))
		m_iType = ndNode;
	else if (strEquals(pXMLDesc->GetTag(), NODE_GROUP_TAG))
		m_iType = ndNodeGroup;
	else if (strEquals(pXMLDesc->GetTag(), NODE_TABLE_TAG))
		m_iType = ndNodeTable;
	else if (strEquals(pXMLDesc->GetTag(), FRAGMENT_TAG))
		m_iType = ndFragment;
	else if (strEquals(pXMLDesc->GetTag(), NETWORK_TAG))
		m_iType = ndNetwork;
	else if (strEquals(pXMLDesc->GetTag(), RANDOM_TAG))
		m_iType = ndRandom;
	else
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Topology %s: Unknown node descriptor: <%s>"), m_sID, pXMLDesc->GetTag());
		return ERR_FAIL;
		}

	//	Load map effect (if we have no parent UNID then we are not part of
	//	a system map and thus cannot have any effects).

	CString sUNID;
	if (!sParentUNID.IsBlank())
		{
		sUNID = strPatternSubst(CONSTLIT("%s/%s"), sParentUNID, m_sID);

		if (error = m_pLabelEffect.LoadEffect(Ctx,
				strPatternSubst(CONSTLIT("%s:l"), sUNID),
				pXMLDesc->GetContentElementByTag(LABEL_EFFECT_TAG),
				pXMLDesc->GetAttribute(LABEL_EFFECT_ATTRIB)))
			return error;

		if (error = m_pMapEffect.LoadEffect(Ctx,
				strPatternSubst(CONSTLIT("%s:m"), sUNID),
				pXMLDesc->GetContentElementByTag(MAP_EFFECT_TAG),
				pXMLDesc->GetAttribute(MAP_EFFECT_ATTRIB)))
			return error;

		if ((m_pLabelEffect != NULL || m_pMapEffect != NULL) && m_sID.IsBlank())
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Topology %s: Nodes with effects must have an ID."), sParentUNID);
			return ERR_FAIL;
			}
		}

	//	Initialize flags

	m_dwFlags = 0;
	if (pXMLDesc->GetAttributeBool(ROOT_NODE_ATTRIB))
		m_dwFlags |= FLAG_IS_ROOT_NODE;

	//	Load type-specific info

	switch (m_iType)
		{
		case ndFragment:
			{
			m_pDescList = new CTopologyDescTable;
			if (error = m_pDescList->LoadFromXML(Ctx, pXMLDesc, pMap, sUNID, true))
				return error;

			break;
			}

		case ndNetwork:
			{
			CXMLElement *pNodeList = pXMLDesc->GetContentElementByTag(NODES_TAG);
			if (pNodeList)
				{
				m_pDescList = new CTopologyDescTable;
				if (error = m_pDescList->LoadFromXML(Ctx, pNodeList, pMap, sUNID))
					return error;

				//	After we've loaded the list, we can clear the ID map because
				//	we won't need it (this will save some memory).

				m_pDescList->DeleteIDMap();
				}

			break;
			}

		case ndRandom:
			{
			CXMLElement *pNodeList = pXMLDesc->GetContentElementByTag(SET_NODES_TAG);
			if (pNodeList)
				{
				m_pDescList = new CTopologyDescTable;
				if (error = m_pDescList->LoadFromXML(Ctx, pNodeList, pMap, sUNID))
					return error;
				}

			break;
			}
		}

	return NOERROR;
	}
