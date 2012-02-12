//	Topology.cpp
//
//	Star system topology

#include "PreComp.h"


#define SYSTEM_TAG								CONSTLIT("System")
#define STARGATES_TAG							CONSTLIT("StarGates")

#define ID_ATTRIB								CONSTLIT("ID")
#define UNID_ATTRIB								CONSTLIT("UNID")
#define NAME_ATTRIB								CONSTLIT("name")
#define DESTID_ATTRIB							CONSTLIT("destID")
#define DESTGATE_ATTRIB							CONSTLIT("destGate")
#define VARIANT_ATTRIB							CONSTLIT("variant")
#define LEVEL_ATTRIB							CONSTLIT("level")
#define ROOT_NODE_ATTRIB						CONSTLIT("rootNode")
#define EPITAPH_ATTRIB							CONSTLIT("epitaph")
#define END_GAME_REASON_ATTRIB					CONSTLIT("endGameReason")
#define END_GAME_ATTRIB							CONSTLIT("endGame")
#define ATTRIBUTES_ATTRIB						CONSTLIT("attributes")
#define X_ATTRIB								CONSTLIT("x")
#define Y_ATTRIB								CONSTLIT("y")
#define DEST_FRAGMENT_ROTATION_ATTRIB			CONSTLIT("destFragmentRotation")

#define PREV_DEST								CONSTLIT("[Prev]")
#define END_GAME_VALUE							CONSTLIT("[EndGame]")

ALERROR CUniverse::InitTopology (CString *retsError)

//	InitTopology
//
//	Initializes the star system topology

	{
	ALERROR error;
	int i;

	//	No need to initialize if we've already loaded it

	if (m_Topology.GetTopologyNodeCount() > 0)
		return NOERROR;

	//	Add all root nodes in order. AddTopologyNode will recurse and
	//	add all nodes connected to each root.

	CTopologyDescTable *pTopology = m_Design.GetTopologyDesc();
	if (pTopology)
		{
		STopologyCreateCtx Ctx;
		Ctx.pTopologyTable = pTopology;

		if (error = m_Topology.AddTopology(Ctx))
			{
			*retsError = Ctx.sError;
			return error;
			}
		}

	//	Let the maps add their fixed topologies too

	for (i = 0; i < m_Design.GetCount(designSystemMap); i++)
		{
		CSystemMap *pMap = CSystemMap::AsType(m_Design.GetEntry(designSystemMap, i));
		if (error = pMap->AddFixedTopology(m_Topology, retsError))
			return error;
		}

	//	Now add generated topologies as a second pass

	for (i = 0; i < m_Design.GetCount(designSystemMap); i++)
		{
		CSystemMap *pMap = CSystemMap::AsType(m_Design.GetEntry(designSystemMap, i));
		if (error = pMap->AddGeneratedTopology(m_Topology, retsError))
			return error;
		}

	//	If we've got no topology nodes then that's an error

	if (m_Topology.GetTopologyNodeCount() == 0)
		{
		*retsError = CONSTLIT("No topology nodes found");
		return ERR_FAIL;
		}

	//	Call OnGlobalTopologyCreated

	for (i = 0; i < m_Design.GetCount(); i++)
		if (error = m_Design.GetEntry(i)->FireOnGlobalTopologyCreated(retsError))
			return error;

	return NOERROR;
	}

//	CTopologyNode class --------------------------------------------------------

CTopologyNode::CTopologyNode (const CString &sID, DWORD SystemUNID, CSystemMap *pMap) : m_sID(sID),
		m_SystemUNID(SystemUNID),
		m_pMap(pMap),
		m_NamedGates(FALSE, TRUE),
		m_pSystem(NULL),
		m_dwID(0xffffffff),
		m_bKnown(false)

//	CTopology constructor

	{
	}

CTopologyNode::~CTopologyNode (void)

//	CTopology destructor

	{
	for (int i = 0; i < m_NamedGates.GetCount(); i++)
		{
		StarGateDesc *pDesc = (StarGateDesc *)m_NamedGates.GetValue(i);
		delete pDesc;
		}
	}

void CTopologyNode::AddAttributes (const CString &sAttribs)

//	AddAttributes
//
//	Append the given attributes

	{
	m_sAttributes = ::AppendModifiers(m_sAttributes, sAttribs);
	}

void CTopologyNode::AddGateInt (const CString &sName, const CString &sDestNode, const CString &sEntryPoint)

//	AddGateInt
//
//	Adds a gate to the topology

	{
	StarGateDesc *pDesc = new StarGateDesc;
	pDesc->sDestNode = sDestNode;
	pDesc->sDestEntryPoint = sEntryPoint;
	pDesc->pDestNode = NULL;

	m_NamedGates.AddEntry(sName, (CObject *)pDesc);
	}

ALERROR CTopologyNode::AddStargate (const CString &sGateID, const CString &sDestNodeID, const CString &sDestGateID)

//	AddStargate
//
//	Adds a new stargate to the topology

	{
	//	Get the destination node

	CTopologyNode *pDestNode = g_pUniverse->FindTopologyNode(sDestNodeID);
	if (pDestNode == NULL)
		{
		kernelDebugLogMessage("Unable to find destination node: %s", sDestNodeID.GetASCIIZPointer());
		return ERR_FAIL;
		}

	//	Look for the destination stargate

	CString sReturnNodeID;
	CString sReturnEntryPoint;
	if (!pDestNode->FindStargate(sDestGateID, &sReturnNodeID, &sReturnEntryPoint))
		{
		kernelDebugLogMessage("Unable to find destination stargate: %s", sDestGateID.GetASCIIZPointer());
		return ERR_FAIL;
		}

	//	Add the gate

	AddGateInt(sGateID, sDestNodeID, sDestGateID);

	//	See if we need to fix up the return gate

	if (strEquals(sReturnNodeID, PREV_DEST))
		pDestNode->SetStargateDest(sDestGateID, GetID(), sGateID);

	return NOERROR;
	}

ALERROR CTopologyNode::AddStargateConnection (const CString &sGateID, CTopologyNode *pDestNode, const CString &sDestGateID)

//	AddStargateConnection
//
//	Adds two stargates connecting the two nodes

	{
	//	Create the two stargates

	AddGateInt(sGateID, pDestNode->GetID(), sDestGateID);
	pDestNode->AddGateInt(sDestGateID, GetID(), sGateID);

	return NOERROR;
	}

void CTopologyNode::CreateFromStream (SUniverseLoadCtx &Ctx, CTopologyNode **retpNode)

//	CreateFromStream
//
//	Creates a node from a stream
//
//	CString		m_sID
//	DWORD		m_SystemUNID
//	DWORD		m_pMap (UNID)
//	DWORD		m_xPos
//	DWORD		m_yPos
//	CString		m_sName
//	DWORD		m_iLevel
//	DWORD		m_dwID
//
//	DWORD		No of named gates
//	CString		gate: sName
//	CString		gate: sDestNode
//	CString		gate: sDestEntryPoint
//
//	DWORD		No of variant labels
//	CString		variant label
//
//	CAttributeDataBlock	m_Data
//	DWORD		flags
//
//	CString		m_sEpitaph
//	CString		m_sEndGameReason

	{
	int i;
	DWORD dwLoad;
	CTopologyNode *pNode;

	CString sID;
	sID.ReadFromStream(Ctx.pStream);

	DWORD dwSystemUNID;
	Ctx.pStream->Read((char *)&dwSystemUNID, sizeof(DWORD));

	CSystemMap *pMap;
	if (Ctx.dwVersion >= 6)
		{
		DWORD dwMapUNID;
		Ctx.pStream->Read((char *)&dwMapUNID, sizeof(DWORD));
		pMap = CSystemMap::AsType(g_pUniverse->FindDesignType(dwMapUNID));
		}
	else
		pMap = NULL;

	pNode = new CTopologyNode(sID, dwSystemUNID, pMap);

	if (Ctx.dwVersion >= 6)
		{
		Ctx.pStream->Read((char *)&pNode->m_xPos, sizeof(DWORD));
		Ctx.pStream->Read((char *)&pNode->m_yPos, sizeof(DWORD));
		}
	
	pNode->m_sName.ReadFromStream(Ctx.pStream);
	Ctx.pStream->Read((char *)&pNode->m_iLevel, sizeof(DWORD));
	Ctx.pStream->Read((char *)&pNode->m_dwID, sizeof(DWORD));

	DWORD dwCount;
	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		StarGateDesc *pDesc = new StarGateDesc;
		CString sName;
		sName.ReadFromStream(Ctx.pStream);
		pDesc->sDestNode.ReadFromStream(Ctx.pStream);
		pDesc->sDestEntryPoint.ReadFromStream(Ctx.pStream);
		pDesc->pDestNode = NULL;

		pNode->m_NamedGates.AddEntry(sName, (CObject *)pDesc);
		}

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		CString sLabel;
		sLabel.ReadFromStream(Ctx.pStream);
		pNode->m_VariantLabels.AppendString(sLabel, NULL);
		}

	if (Ctx.dwVersion >= 1)
		pNode->m_Data.ReadFromStream(Ctx.pStream);

	//	Flags

	if (Ctx.dwVersion >= 6)
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	else
		dwLoad = 0;

	pNode->m_bKnown = (dwLoad & 0x00000001 ? true : false);
	pNode->m_bMarked = false;

	//	More

	if (Ctx.dwVersion >= 5)
		{
		pNode->m_sEpitaph.ReadFromStream(Ctx.pStream);
		pNode->m_sEndGameReason.ReadFromStream(Ctx.pStream);
		}
	else
		{
		//	For previous version, we forgot to save this, so do it now

		if (pNode->IsEndGame())
			{
			pNode->m_sEpitaph = CONSTLIT("left Human Space on a journey to the Galactic Core");
			pNode->m_sEndGameReason = CONSTLIT("leftHumanSpace");
			}
		}

	//	Done

	*retpNode = pNode;
	}

bool CTopologyNode::FindStargate (const CString &sName, CString *retsDestNode, CString *retsEntryPoint)

//	FindStargate
//
//	Looks for the stargate by name and returns the destination node id and entry point

	{
	StarGateDesc *pDesc;
	if (m_NamedGates.Lookup(sName, (CObject **)&pDesc) != NOERROR)
		return false;

	if (retsDestNode)
		*retsDestNode = pDesc->sDestNode;

	if (retsEntryPoint)
		*retsEntryPoint = pDesc->sDestEntryPoint;

	return true;
	}

CString CTopologyNode::FindStargateName (const CString &sDestNode, const CString &sEntryPoint)

//	FindStargateName
//
//	Returns the name of the stargate that matches the node and entry point

	{
	int i;

	for (i = 0; i < m_NamedGates.GetCount(); i++)
		{
		StarGateDesc *pDesc = (StarGateDesc *)m_NamedGates.GetValue(i);
		if (strEquals(pDesc->sDestNode, sDestNode)
				&& strEquals(pDesc->sDestEntryPoint, sEntryPoint))
			return m_NamedGates.GetKey(i);
		}

	return NULL_STR;
	}

bool CTopologyNode::FindStargateTo (const CString &sDestNode, CString *retsName, CString *retsDestGateID)

//	FindStargateTo
//
//	Looks for a stargate to the given node; returns info on the first one.
//	Returns FALSE if none found.

	{
	int i;

	for (i = 0; i < m_NamedGates.GetCount(); i++)
		{
		StarGateDesc *pDesc = (StarGateDesc *)m_NamedGates.GetValue(i);
		if (strEquals(pDesc->sDestNode, sDestNode))
			{
			if (retsName)
				*retsName = m_NamedGates.GetKey(i);

			if (retsDestGateID)
				*retsDestGateID = pDesc->sDestEntryPoint;

			return true;
			}
		}

	return false;
	}

CTopologyNode *CTopologyNode::GetGateDest (const CString &sName, CString *retsEntryPoint)

//	GetGateDest
//
//	Get stargate destination

	{
	StarGateDesc *pDesc;
	if (m_NamedGates.Lookup(sName, (CObject **)&pDesc) != NOERROR)
		return NULL;

	if (retsEntryPoint)
		*retsEntryPoint = pDesc->sDestEntryPoint;

	if (pDesc->pDestNode == NULL)
		pDesc->pDestNode = g_pUniverse->FindTopologyNode(pDesc->sDestNode);

	return pDesc->pDestNode;
	}

CString CTopologyNode::GetStargate (int iIndex)

//	GetStargate
//
//	Returns the stargate ID

	{
	return m_NamedGates.GetKey(iIndex);
	}

CTopologyNode *CTopologyNode::GetStargateDest (int iIndex, CString *retsEntryPoint)

//	GetStargateDest
//
//	Returns the destination node for the given stargate

	{
	StarGateDesc *pDesc = (StarGateDesc *)m_NamedGates.GetValue(iIndex);
	if (retsEntryPoint)
		*retsEntryPoint = pDesc->sDestEntryPoint;

	if (pDesc->pDestNode == NULL)
		pDesc->pDestNode = g_pUniverse->FindTopologyNode(pDesc->sDestNode);

	return pDesc->pDestNode;
	}

bool CTopologyNode::HasVariantLabel (const CString &sVariant)

//	HasVariantLabel
//
//	Returns TRUE if it has the given variant label

	{
	for (int i = 0; i < m_VariantLabels.GetCount(); i++)
		{
		if (strEquals(sVariant, m_VariantLabels.GetStringValue(i)))
			return true;
		}

	return false;
	}

void CTopologyNode::SetStargateDest (const CString &sName, const CString &sDestNode, const CString &sEntryPoint)

//	SetStargateDest
//
//	Sets the destination information for the given stargate

	{
	StarGateDesc *pDesc;
	if (m_NamedGates.Lookup(sName, (CObject **)&pDesc) != NOERROR)
		{
		ASSERT(false);
		return;
		}

	pDesc->sDestNode = sDestNode;
	pDesc->sDestEntryPoint = sEntryPoint;
	pDesc->pDestNode = NULL;
	}

void CTopologyNode::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Writes out the variable portions of the node
//
//	CString		m_sID
//	DWORD		m_SystemUNID
//	DWORD		m_pMap (UNID)
//	DWORD		m_xPos
//	DWORD		m_yPos
//	CString		m_sName
//	DWORD		m_iLevel
//	DWORD		m_dwID
//
//	DWORD		No of named gates
//	CString		gate: sName
//	CString		gate: sDestNode
//	CString		gate: sDestEntryPoint
//
//	DWORD		No of variant labels
//	CString		variant label
//
//	CAttributeDataBlock	m_Data
//	DWORD		flags
//
//	CString		m_sEpitaph
//	CString		m_sEndGameReason

	{
	int i;
	DWORD dwSave;

	m_sID.WriteToStream(pStream);
	pStream->Write((char *)&m_SystemUNID, sizeof(DWORD));

	dwSave = (m_pMap ? m_pMap->GetUNID() : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_xPos, sizeof(DWORD));
	pStream->Write((char *)&m_yPos, sizeof(DWORD));
	m_sName.WriteToStream(pStream);
	pStream->Write((char *)&m_iLevel, sizeof(DWORD));
	pStream->Write((char *)&m_dwID, sizeof(DWORD));

	DWORD dwCount = m_NamedGates.GetCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		StarGateDesc *pDesc = (StarGateDesc *)m_NamedGates.GetValue(i);
		CString sName = m_NamedGates.GetKey(i);
		sName.WriteToStream(pStream);
		pDesc->sDestNode.WriteToStream(pStream);
		pDesc->sDestEntryPoint.WriteToStream(pStream);
		}

	dwCount = m_VariantLabels.GetCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		m_VariantLabels.GetStringValue(i).WriteToStream(pStream);

	//	Write opaque data

	m_Data.WriteToStream(pStream);

	//	Flags

	dwSave = 0;
	dwSave |= (m_bKnown ? 0x00000001 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Write end game data

	m_sEpitaph.WriteToStream(pStream);
	m_sEndGameReason.WriteToStream(pStream);
	}

