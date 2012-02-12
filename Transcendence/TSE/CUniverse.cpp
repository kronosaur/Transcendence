//	CUniverse.cpp
//
//	CUniverse class
//
//	VERSION HISTORY
//
//	 0: Pre-0.98
//
//	 1: 0.98
//		version marker in CUniverse
//		version in CUniverse
//		m_Data in CTopologyNode
//
//	 2: 0.99
//		Adventure UNID in CUniverse
//
//	 3: 0.99
//		All design type info is together
//
//	 4: 0.99d
//		m_ShipNamesIndices, and m_iShipName in CShipClass
//
//	 5: 0.99d
//		m_sEpitaph and m_sEndGameReason in CTopologyNode
//
//	 6: 1.03
//		m_pMap in CTopologyNode
//
//	 7: 1.03
//		m_Time in CUniverse
//
//	 8: 1.03
//		Extension UNID list in CUniverse
//
//	 9: 1.04
//		system save version
//		m_Annotations in CSystemMap
//
//	10: 1.04
//		m_sUnknownName in CItemType
//
//	11: 1.07
//		m_Registered in CDesignType
//
//	See: TSEUtil.h for definition of UNIVERSE_SAVE_VERSION

#include "PreComp.h"

const DWORD UNIVERSE_VERSION_MARKER =					0xffffffff;

CUniverse *g_pUniverse = NULL;
Metric g_KlicksPerPixel = KLICKS_PER_PIXEL;
Metric g_TimeScale = TIME_SCALE;
Metric g_SecondsPerUpdate =	g_TimeScale / g_TicksPerSecond;

static CObjectClass<CUniverse>g_Class(OBJID_CUNIVERSE, NULL);

#ifdef DEBUG_PROGRAMSTATE
ProgramStates g_iProgramState = psUnknown;
CSpaceObject *g_pProgramObj = NULL;
CTimedEvent *g_pProgramEvent = NULL;
CString *g_sProgramError = NULL;
#endif

#ifdef DEBUG_PERFORMANCE
DWORD g_dwPerformanceTimer;
#endif

static CUniverse::IHost g_DefaultHost;

CUniverse::CUniverse (void) : CObject(&g_Class),
		m_Sounds(FALSE, TRUE),
		m_LevelEncounterTables(TRUE),

		m_iTick(0),
		m_pAdventure(NULL),
		m_pPOV(NULL),
		m_pPlayer(NULL),
		m_pCurrentSystem(NULL),
		m_StarSystems(TRUE, FALSE),
		m_dwNextID(1),

		m_pSoundMgr(NULL),

		m_pHost(&g_DefaultHost),
		m_bDebugMode(false),
		m_bNoSound(false)

//	CUniverse constructor

	{
	ASSERT(g_pUniverse == NULL);
	g_pUniverse = this;
	EuclidInit();

	//	We instantiate a CBeam object so that the code gets linked
	//	(Otherwise, the CBeam code would be optimized out, and need it
	//	because an older version might load one).

	CBeam *pDummy = new CBeam;
	delete pDummy;
	}

CUniverse::~CUniverse (void)

//	CUniverse destructor

	{
	SetPOV(NULL);
	m_pPlayer = NULL;

	//	Destroy all star systems. We do this here because we want to
	//	guarantee that we destroy all objects before we destruct
	//	codechain, et al

	m_StarSystems.RemoveAll();

	//	Free up various arrays whose cleanup requires m_CC

	m_Design.RemoveAll();

	g_pUniverse = NULL;
	}

ALERROR CUniverse::AddStarSystem (CTopologyNode *pTopology, CSystem *pSystem)

//	AddStarSystem
//
//	Adds a system to the universe

	{
	ALERROR error;

	//	Add it to our list

	if (error = m_StarSystems.ReplaceEntry((int)pSystem->GetID(), 
			pSystem,
			TRUE,
			NULL))
		{
		ASSERT(false);
		return error;
		}

	//	Set the system for the topoplogy

	if (pTopology)
		{
		ASSERT(pTopology->GetSystemID() == pSystem->GetID());
		pTopology->SetSystem(pSystem);
		}

	return NOERROR;
	}

ALERROR CUniverse::CreateEmptyStarSystem (CSystem **retpSystem)

//	CreateEmptyStarSystem
//
//	Creates an empty star system

	{
	ALERROR error;
	CSystem *pSystem;

	if (error = CSystem::CreateEmpty(this, NULL, &pSystem))
		return error;

	//	Add to our list

	DWORD dwID = CreateGlobalID();
	if (error = m_StarSystems.AddEntry((int)dwID, pSystem))
		{
		delete pSystem;
		return error;
		}

	pSystem->SetID(dwID);

	//	Done

	if (retpSystem)
		*retpSystem = pSystem;

	return NOERROR;
	}

ALERROR CUniverse::CreateRandomItem (const CItemCriteria &Crit, 
									 const CString &sLevelFrequency,
									 CItem *retItem)

//	CreateRandomItem
//
//	Generates a random item

	{
	ALERROR error;

	//	Create the generator

	IItemGenerator *pTable;
	if (error = IItemGenerator::CreateRandomItemTable(Crit, sLevelFrequency, &pTable))
		return error;

	//	Pick an item

	CItemList ItemList;
	CItemListManipulator Items(ItemList);
	SItemAddCtx Ctx(Items);
	Ctx.iLevel = (GetCurrentSystem() ? GetCurrentSystem()->GetLevel() : 1);

	pTable->AddItems(Ctx);

	//	Done

	Items.ResetCursor();
	if (!Items.MoveCursorForward())
		return ERR_FAIL;

	*retItem = Items.GetItemAtCursor();
	return NOERROR;
	}

ALERROR CUniverse::CreateStarSystem (const CString &sNodeID, CSystem **retpSystem, CString *retsError, CSystemCreateStats *pStats)

//	CreateStarSystem
//
//	Creates a new system

	{
	ALERROR error;

	//	Figure out where to start

	CString sFirstNode = sNodeID;
	if (sFirstNode.IsBlank())
		{
		sFirstNode = m_Design.GetTopologyDesc()->GetFirstNodeID();

		if (sFirstNode.IsBlank())
			{
			if (retsError)
				*retsError = CONSTLIT("No first star system defined in topology.");

			return ERR_FAIL;
			}
		}

	//	Find the topology node

	CTopologyNode *pNode = FindTopologyNode(sFirstNode);
	if (pNode == NULL)
		{
		if (retsError)
			*retsError = strPatternSubst(CONSTLIT("Unable to find topology node: %s"), sFirstNode);

		return ERR_FAIL;
		}

	//	Create the first star system

	if (error = CreateStarSystem(pNode, retpSystem, retsError, pStats))
		return error;

	//	Done

	return NOERROR;
	}

ALERROR CUniverse::CreateStarSystem (CTopologyNode *pTopology, CSystem **retpSystem, CString *retsError, CSystemCreateStats *pStats)

//	CreateStarSystem
//
//	Creates a new system based on the description

	{
	ALERROR error;
	CSystem *pSystem;

	//	Get the description

	CSystemType *pSystemType = FindSystemType(pTopology->GetSystemDescUNID());
	if (pSystemType == NULL)
		{
		if (retsError)
			*retsError = strPatternSubst(CONSTLIT("Cannot create system %s: Undefined system type %x"), pTopology->GetID(), pTopology->GetSystemDescUNID());
		return ERR_FAIL;
		}

	//	Create the system

	CString sError;
	if (error = CSystem::CreateFromXML(this, pSystemType, pTopology, &pSystem, &sError, pStats))
		{
		if (retsError)
			*retsError = strPatternSubst(CONSTLIT("Cannot create system %s: %s"), pTopology->GetID(), sError);
		return error;
		}

	//	Add to our list

	DWORD dwID = CreateGlobalID();
	if (error = m_StarSystems.AddEntry(dwID, pSystem))
		{
		if (retsError)
			*retsError = strPatternSubst(CONSTLIT("Cannot create system %s: Unable to add system to list"), pTopology->GetID());
		delete pSystem;
		return error;
		}

	pSystem->SetID(dwID);

	//	Set the system for the topoplogy

	pTopology->SetSystem(pSystem);
	pTopology->SetSystemID(pSystem->GetID());

	//	Done

	if (retpSystem)
		*retpSystem = pSystem;

	return NOERROR;
	}

void CUniverse::DebugOutput (char *pszLine, ...)

//	DebugOutput
//
//	Outputs debug line

	{
#ifdef DEBUG
	char *pArgs;
	char szBuffer[1024];
	int iLen;

	pArgs = (char *)&pszLine + sizeof(pszLine);
	iLen = wvsprintf(szBuffer, pszLine, pArgs);

	m_pHost->DebugOutput(CString(szBuffer));
#endif
	}

void CUniverse::DestroySystem (CSystem *pSystem)

//	DestroySystem
//
//	Destroys the given system

	{
	//	Change POV, if necessary

	if (m_pPOV && m_pPOV->GetSystem() == pSystem)
		SetPOV(NULL);

	//	Remove and destroy the system

	if (m_StarSystems.RemoveEntry(pSystem->GetID(), NULL) != NOERROR)
		{
		ASSERT(false);
		return;
		}
	}

CArmorClass *CUniverse::FindArmor (DWORD dwUNID)

//	FindArmor
//
//	Returns the armor class by item UNID

	{
	CItemType *pType = FindItemType(dwUNID);
	if (pType == NULL)
		return NULL;

	return pType->GetArmorClass();
	}

CObject *CUniverse::FindByUNID (CIDTable &Table, DWORD dwUNID)

//	FindByUNID
//
//	Returns an object by UNID from the appropriate table

	{
	CObject *pObj;

	if (Table.Lookup((int)dwUNID, &pObj) == NOERROR)
		return pObj;
	else
		return NULL;
	}

CDeviceClass *CUniverse::FindDeviceClass (DWORD dwUNID)

//	FindDeviceClass
//
//	Returns the device class by item UNID

	{
	CItemType *pType = FindItemType(dwUNID);
	if (pType == NULL)
		return NULL;

	return pType->GetDeviceClass();
	}

CSpaceObject *CUniverse::FindObject (DWORD dwID)

//	FindObject
//
//	Finds the object by ID

	{
	if (m_pCurrentSystem)
		return m_pCurrentSystem->FindObject(dwID);
	else
		return NULL;
	}

CShipClass *CUniverse::FindShipClassByName (const CString &sName)

//	FindShipClassByName
//
//	Finds a ship class using partial name matches

	{
	int i;

	//	If this is a hex value, then assume an UNID

	CShipClass *pFound = NULL;
	if (strFind(sName, CONSTLIT("0x")) == 0
			&& (pFound = FindShipClass(strToInt(sName, 0))))
		return pFound;

	//	Otherwise look for a name match

	for (i = 0; i < GetShipClassCount(); i++)
		{
		CShipClass *pClass = GetShipClass(i);
		if (!pClass->IsVirtual()
				&& pClass->HasAttribute(CONSTLIT("genericClass"))
				&& strEquals(sName, pClass->GetName()))
			return pClass;
		}

	//	If not found, look for a partial match

	for (i = 0; i < GetShipClassCount(); i++)
		{
		CShipClass *pClass = GetShipClass(i);
		if (!pClass->IsVirtual()
				&& pClass->HasAttribute(CONSTLIT("genericClass"))
				&& strFind(pClass->GetName(), sName) != -1)
			return pClass;
		}

	//	If it's still not found, look for a partial match of
	//	any non-generic class

	for (i = 0; i < GetShipClassCount(); i++)
		{
		CShipClass *pClass = GetShipClass(i);
		if (!pClass->IsVirtual()
				&& strFind(pClass->GetName(), sName) != -1)
			return pClass;
		}

	//	Not found
	
	return NULL;
	}

CWeaponFireDesc *CUniverse::FindWeaponFireDesc (const CString &sName)

//	FindWeaponFireDesc
//
//	Returns a pointer to the descriptor by name

	{
	return CWeaponFireDesc::FindWeaponFireDescFromFullUNID(sName); 
	}

void CUniverse::FlushStarSystem (CTopologyNode *pTopology)

//	FlushStarSystem
//
//	Flushes the star system after it has been saved.

	{
	ASSERT(pTopology->GetSystemID() != 0xffffffff);
	m_StarSystems.RemoveEntry(pTopology->GetSystemID(), NULL);
	}

void CUniverse::GarbageCollectLibraryBitmaps (void)

//	GarbageCollectLibraryBitmaps
//
//	Garbage-collects any unused bitmaps

	{
	//	Mark all bitmaps currently in use

	m_Design.ClearImageMarks();
	if (m_pCurrentSystem)
		m_pCurrentSystem->MarkImages();

	//	Sweep

	m_Design.SweepImages();
	}

void CUniverse::GenerateGameStats (CGameStats &Stats)

//	GenerateGameStats
//
//	Generates the current game stats

	{
	int i;

	//	Ask all design types to generate game stats

	m_Design.FireGetGlobalAchievements(Stats);

	//	Add all extensions

	for (i = 0; i < m_Design.GetExtensionCount(); i++)
		{
		SExtensionDesc *pExtension = m_Design.GetExtension(i);
		if (pExtension->bEnabled)
			{
			CString sName = pExtension->sName;
			if (sName.IsBlank())
				sName = strPatternSubst(CONSTLIT("Extension %x"), pExtension->dwUNID);

			if (pExtension->iType == extAdventure)
				Stats.Insert(CONSTLIT("Adventure"), sName);
			else
				Stats.Insert(sName, NULL_STR, CONSTLIT("extensions"));
			}
		}
	}

CTimeSpan CUniverse::GetElapsedGameTime (void)

//	GetElapsedGameTime
//
//	Returns the amount of time that has elapsed in the game.

	{
	return m_Time.GetElapsedTimeAt(m_iTick);
	}

CTopologyNode *CUniverse::GetFirstTopologyNode (void)

//	GetFirstTopologyNode
//
//	Returns the starting node

	{
	ASSERT(m_StarSystems.GetCount() == 0);

	//	Initialize the topology

	CString sError;
	InitTopology(&sError);

	//	Figure out the starting node

	CString sNodeID;
	CAdventureDesc *pAdventure = GetCurrentAdventureDesc();
	if (pAdventure)
		sNodeID = pAdventure->GetStartingNodeID();

	if (sNodeID.IsBlank())
		sNodeID = m_Design.GetTopologyDesc()->GetFirstNodeID();

	return FindTopologyNode(sNodeID);
	}

IShipController *CUniverse::GetPlayerController (void) const

//	GetPlayerController
//
//	Returns the player's controller

	{
	CSpaceObject *pPlayer = GetPlayer();
	if (pPlayer == NULL)
		return NULL;

	CShip *pPlayerShip = pPlayer->AsShip();
	if (pPlayerShip == NULL)
		return NULL;

	return pPlayerShip->GetController();
	}

GenomeTypes CUniverse::GetPlayerGenome (void) const

//	GetPlayerGenome
//
//	Returns the player's genome

	{
	IShipController *pController = GetPlayerController();
	if (pController == NULL)
		return genomeUnknown;

	return pController->GetPlayerGenome();
	}

CString CUniverse::GetPlayerName (void) const

//	GetPlayerName
//
//	Returns the player's name

	{
	IShipController *pController = GetPlayerController();
	if (pController == NULL)
		return NULL_STR;

	return pController->GetPlayerName();
	}

CSovereign *CUniverse::GetPlayerSovereign (void) const

//	GetPlayerSovereign
//
//	Returns the player's sovereign

	{
	CSovereign *pSovereign = FindSovereign(g_PlayerSovereignUNID);
	if (pSovereign == NULL)
		{
		kernelDebugLogMessage("ERROR: Unable to find player sovereign");
		return NULL;
		}

	return pSovereign;
	}

void CUniverse::GetRandomLevelEncounter (int iLevel, 
										 CDesignType **retpType,
										 IShipGenerator **retpTable, 
										 CSovereign **retpBaseSovereign)

//	GetRandomLevelEncounter
//
//	Returns a random encounter appropriate for the given level

	{
	int i;

	iLevel--;	//	m_LevelEncounterTable is 0-based
	if (iLevel < 0 || iLevel >= m_LevelEncounterTables.GetCount())
		{
		*retpType = NULL;
		*retpTable = NULL;
		*retpBaseSovereign = NULL;
		return;
		}

	//	Get the level table

	CStructArray *pTable = (CStructArray *)m_LevelEncounterTables.GetObject(iLevel);

	//	Compute the totals for the table

	int iTotal = 0;
	for (i = 0; i < pTable->GetCount(); i++)
		iTotal += ((SLevelEncounter *)pTable->GetStruct(i))->iWeight;

	if (iTotal == 0)
		{
		*retpType = NULL;
		*retpTable = NULL;
		*retpBaseSovereign = NULL;
		return;
		}

	//	Pick a random entry

	int iRoll = mathRandom(0, iTotal - 1);
	int iPos = 0;

	//	Get the position

	while (((SLevelEncounter *)pTable->GetStruct(iPos))->iWeight <= iRoll)
		iRoll -= ((SLevelEncounter *)pTable->GetStruct(iPos++))->iWeight;

	//	Done

	*retpType = ((SLevelEncounter *)pTable->GetStruct(iPos))->pType;
	*retpTable = ((SLevelEncounter *)pTable->GetStruct(iPos))->pTable;
	*retpBaseSovereign = ((SLevelEncounter *)pTable->GetStruct(iPos))->pBaseSovereign;
	}

DWORD CUniverse::GetSoundUNID (int iChannel)

//	GetSoundUNID
//
//	Returns the UNID for the given sound

	{
	int i;

	for (i = 0; i < m_Sounds.GetCount(); i++)
		if ((int)m_Sounds.GetValue(i) == iChannel)
			return (DWORD)m_Sounds.GetKey(i);

	return INVALID_UNID;
	}

ALERROR CUniverse::InitGame (CString *retsError)

//	InitGame
//
//	Initializes a game instance, including the topology

	{
	ALERROR error;

	if (error = Reinit())
		{
		if (retsError)
			*retsError = CONSTLIT("Unable to initialize universe.");
		return ERR_FAIL;
		}

	//	Initialize the topology. This is the point at which the topology is created

	if (error = InitTopology(retsError))
		return error;

	return NOERROR;
	}

bool CUniverse::IsGlobalResurrectPending (CDesignType **retpType)

//	IsGlobalResurrectPending
//
//	Asks each design type if they want to resurrect the player

	{
	int i;
	CIntArray Types;
	CIntArray Chances;
	int iTotalChance = 0;

	//	Ask all design types if they are prepared to resurrect
	//	the player (they return a number from 0 to 100).

	for (i = 0; i < m_Design.GetCount(); i++)
		{
		int iChance = m_Design.GetEntry(i)->FireGetGlobalResurrectPotential();
		if (iChance > 0)
			{
			Types.AppendElement(i);
			Chances.AppendElement(iTotalChance + iChance);

			iTotalChance += iChance;
			}
		}

	//	If none want to resurrect, then we're done

	if (iTotalChance == 0)
		return false;

	//	Otherwise, pick a random design type in proportion to the
	//	value that they returned.

	int iRoll = mathRandom(0, iTotalChance-1);
	for (i = 0; i < Types.GetCount(); i++)
		if (iRoll < Chances.GetElement(i))
			{
			*retpType = m_Design.GetEntry(Types.GetElement(i));
			return true;
			}

	ASSERT(false);
	return false;
	}

bool CUniverse::IsStatsPostingEnabled (void)

//	IsStatsPostingEnabled
//
//	Returns TRUE if we should post stats for the current adventure.
//	FALSE otherwise.

	{
	//	Never post if we are in debug mode

	if (InDebugMode())
		return false;

	//	LATER: Only post for registered adventures

	return true;
	}

ALERROR CUniverse::LoadFromStream (IReadStream *pStream, DWORD *retdwSystemID, DWORD *retdwPlayerID, CString *retsError)

//	LoadFromStream
//
//	Loads the universe from a stream
//
//	DWORD		UNIVERSE_VERSION_MARKER
//	DWORD		universe version
//	DWORD		system version
//	DWORD		m_iTick
//
//	DWORD		m_dwNextID
//
//	DWORD		No of enabled extensions
//	DWORD			extension UNID
//	DWORD		Adventure UNID
//
//	DWORD		ID of POV system (0xffffffff if none)
//	DWORD		index of POV (0xffffffff if none)
//	CTimeSpan	time that we've spent playing the game
//
//	DWORD		No of topology nodes
//	CString		node: Node ID
//	DWORD		node: ID of system instance (0xffffffff if not yet created)
//
//	DWORD		No of types
//	DWORD		type: UNID
//	Type-specific data
	
	{
	int i;
	DWORD dwLoad;
	DWORD dwCount;

	//	Create load structure

	SUniverseLoadCtx Ctx;
	Ctx.pStream = pStream;

	//	Load the version. Version 0 we did not even have a place to store the
	//	version, so we see if the first entry is -1. If it is not, then we
	//	are at version 0.

	pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad == UNIVERSE_VERSION_MARKER)
		{
		pStream->Read((char *)&Ctx.dwVersion, sizeof(DWORD));

		//	Some types (e.g., CSystemMap) require saving IPainterEffects. In those
		//	cases we need to have the system version, so we started saving the system
		//	version here too. Since we never needed this until system version 57, we
		//	can be sure that we are at least version 57.

		if (Ctx.dwVersion >= 9)
			pStream->Read((char *)&Ctx.dwSystemVersion, sizeof(DWORD));
		else
			Ctx.dwSystemVersion = 57;

		//	Read the current tick

		pStream->Read((char *)&m_iTick, sizeof(DWORD));
		}
	else
		{
		Ctx.dwVersion = 0;
		Ctx.dwSystemVersion = 57;
		m_iTick = (int)dwLoad;
		}

	//	Load basic data

	pStream->Read((char *)&m_dwNextID, sizeof(DWORD));

	//	CGameTimeKeeper m_Time

	if (Ctx.dwVersion >= 7)
		m_Time.ReadFromStream(pStream);

	//	Load list of extensions used in this game

	TArray<DWORD> ExtensionList;
	if (Ctx.dwVersion >= 8)
		{
		pStream->Read((char *)&dwLoad, sizeof(DWORD));
		ExtensionList.InsertEmpty(dwLoad);
		pStream->Read((char *)&ExtensionList[0], dwLoad * sizeof(DWORD));
		}

	//	Load adventure UNID

	if (Ctx.dwVersion >= 2)
		pStream->Read((char *)&dwLoad, sizeof(DWORD));
	else
		dwLoad = DEFAULT_ADVENTURE_UNID;

	//	Select the proper adventure and extensions and bind design

	CString sError;
	if (InitAdventure(dwLoad, (Ctx.dwVersion >= 8 ? &ExtensionList : NULL), &sError) != NOERROR)
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to load universe: %s"), sError);
		return ERR_FAIL;
		}

	//	More data

	pStream->Read((char *)retdwSystemID, sizeof(DWORD));
	pStream->Read((char *)retdwPlayerID, sizeof(DWORD));

	CTimeSpan Time;
	Time.ReadFromStream(pStream);
	CTimeDate Now(CTimeDate::Now);
	m_StartTime = timeSubtractTime(Now, Time);

	//	Load item type data (we only do this for previous versions)

	if (Ctx.dwVersion < 3)
		{
		pStream->Read((char *)&dwCount, sizeof(DWORD));
		for (i = 0; i < (int)dwCount; i++)
			{
			pStream->Read((char *)&dwLoad, sizeof(DWORD));
			CItemType *pType = FindItemType(dwLoad);
			if (pType == NULL)
				{
				*retsError = strPatternSubst(CONSTLIT("Unable to find type: %x (missing mod?)"), dwLoad);
				return ERR_FAIL;
				}

			pType->ReadFromStream(Ctx);
			}
		}

	//	Load topology data

	m_Topology.ReadFromStream(Ctx);

	//	Load type data

	if (Ctx.dwVersion >= 3)
		{
		//	Load all design types

		pStream->Read((char *)&dwCount, sizeof(DWORD));
		for (i = 0; i < (int)dwCount; i++)
			{
			pStream->Read((char *)&dwLoad, sizeof(DWORD));
			CDesignType *pType = FindDesignType(dwLoad);
			if (pType == NULL)
				{
				*retsError = strPatternSubst(CONSTLIT("Unable to find type: %x (missing mod?)"), dwLoad);
				return ERR_FAIL;
				}

			pType->ReadFromStream(Ctx);
			}
		}

	//	Load previous versions' type data

	if (Ctx.dwVersion < 3)
		{
		pStream->Read((char *)&dwCount, sizeof(DWORD));
		for (i = 0; i < (int)dwCount; i++)
			{
			pStream->Read((char *)&dwLoad, sizeof(DWORD));
			CStationType *pType = FindStationType(dwLoad);
			if (pType == NULL)
				{
				*retsError = strPatternSubst(CONSTLIT("Unable to find type: %x (missing mod?)"), dwLoad);
				return ERR_FAIL;
				}

			pType->ReadFromStream(Ctx);
			}

		//	If we've still got data left, then this is a newer version and we
		//	can read the ship class data

		if (Ctx.dwVersion < 3)
			{
			if (pStream->Read((char *)&dwCount, sizeof(DWORD)) == NOERROR)
				{
				for (i = 0; i < (int)dwCount; i++)
					{
					pStream->Read((char *)&dwLoad, sizeof(DWORD));
					CShipClass *pClass = FindShipClass(dwLoad);
					if (pClass == NULL)
						{
						*retsError = strPatternSubst(CONSTLIT("Unable to find type: %x (missing mod?)"), dwLoad);
						return ERR_FAIL;
						}

					pClass->ReadFromStream(Ctx);
					}
				}
			}

		//	If we've still got data left, then read the sovereign data

		if (Ctx.dwVersion < 3)
			{
			if (pStream->Read((char *)&dwCount, sizeof(DWORD)) == NOERROR)
				{
				for (i = 0; i < (int)dwCount; i++)
					{
					pStream->Read((char *)&dwLoad, sizeof(DWORD));
					CSovereign *pSovereign = FindSovereign(dwLoad);
					if (pSovereign == NULL)
						{
						*retsError = strPatternSubst(CONSTLIT("Unable to find type: %x (missing mod?)"), dwLoad);
						return ERR_FAIL;
						}

					pSovereign->ReadFromStream(Ctx);
					}
				}
			}
		}

	return NOERROR;
	}

void CUniverse::LoadLibraryBitmaps (void)

//	LoadLibraryBitmaps
//
//	Make sure the bitmaps that we need are loaded

	{
	if (m_pCurrentSystem)
		m_pCurrentSystem->LoadImages();
	}

void CUniverse::PaintObject (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pObj)

//	PaintObject
//
//	Paints this object only

	{
	if (m_pPOV)
		m_pPOV->GetSystem()->PaintViewportObject(Dest, rcView, m_pPOV, pObj);
	}

void CUniverse::PaintObjectMap (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pObj)

//	PaintObjectMap
//
//	Paints this object only

	{
	if (m_pPOV)
		m_pPOV->GetSystem()->PaintViewportMapObject(Dest, rcView, m_pPOV, pObj);
	}

void CUniverse::PaintPOV (CG16bitImage &Dest, const RECT &rcView, bool bEnhanced)

//	PaintPOV
//
//	Paint the current point of view

	{
	if (m_pPOV)
		m_pPOV->GetSystem()->PaintViewport(Dest, rcView, m_pPOV, bEnhanced);
	}

void CUniverse::PaintPOVLRS (CG16bitImage &Dest, const RECT &rcView, bool *retbNewEnemies)

//	PaintPOVLRS
//
//	Paint the LRS from the current POV

	{
	if (m_pPOV)
		m_pPOV->GetSystem()->PaintViewportLRS(Dest, rcView, m_pPOV, retbNewEnemies);
	}

void CUniverse::PaintPOVMap (CG16bitImage &Dest, const RECT &rcView, Metric rMapScale)

//	PaintPOVMap
//
//	Paint the system map

	{
	if (m_pPOV)
		m_pPOV->GetSystem()->PaintViewportMap(Dest, rcView, m_pPOV, rMapScale);
	}

void CUniverse::PlaySound (CSpaceObject *pSource, int iChannel)

//	PlaySound
//
//	Plays a sound from the given source

	{
	if (!m_bNoSound && m_pSoundMgr && m_pPOV)
		{
		//	Default to full volume

		int iVolume = 0;
		int iPan = 0;

		//	Figure out how close the source is to the POV. The sound fades as we get
		//	further away.

		if (pSource)
			{
			CVector vDist = pSource->GetPos() - m_pPOV->GetPos();
			Metric rDist2 = vDist.Length2();
			iVolume = -(int)(10000.0 * rDist2 / (MAX_SOUND_DISTANCE * MAX_SOUND_DISTANCE));

			//	If below a certain level, then it is silent anyway

			if (iVolume <= -10000)
				return;

			//	Adjust left/right volume based on direction

			iPan = (int)(10000.0 * (vDist.GetX() / MAX_SOUND_DISTANCE));
			}

		m_pSoundMgr->Play(iChannel, iVolume, iPan);
		}
	}

ALERROR CUniverse::Reinit (void)

//	Reinit
//	
//	Reinitializes the universe

	{
	//	Clear some basic variables

	m_iTick = 0;
	m_Time.DeleteAll();
	m_pPOV = NULL;
	SetCurrentSystem(NULL);
	m_StarSystems.RemoveAll();
	m_dwNextID = 1;

	//	Reinitialize some global classes

	CCompositeImageDesc::Reinit();

	//	Reinitialize types

	m_Design.Reinit();

	//	Clear the topology nodes

	m_Topology.DeleteAll();

	//	Clear out player variables

	SetPlayer(NULL);

	//	Reinitialize noise randomness

	NoiseReinit();

	return NOERROR;
	}

ALERROR CUniverse::SaveToStream (IWriteStream *pStream)

//	SaveToStream
//
//	Saves the universe to a stream.
//
//	DWORD		UNIVERSE_VERSION_MARKER
//	DWORD		universe version
//	DWORD		system version
//	DWORD		m_iTick
//	DWORD		m_dwNextID
//	CGameTimeKeeper m_Time
//
//	DWORD		No of enabled extensions
//	DWORD			extension UNID
//	DWORD		Adventure UNID
//
//	DWORD		ID of POV system (0xffffffff if none)
//	DWORD		index of POV (0xffffffff if none)
//	DWORD		milliseconds that we've spent playing the game
//
//	DWORD		No of topology nodes
//	CString		node: Node ID
//	DWORD		node: ID of system instance (0xffffffff if not yet created)
//
//	DWORD		No of types
//	DWORD		type: UNID
//	CDesignType

	{
	int i;
	DWORD dwSave;

	//	Write out version

	dwSave = UNIVERSE_VERSION_MARKER;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	dwSave = UNIVERSE_SAVE_VERSION;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	dwSave = SYSTEM_SAVE_VERSION;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Write basic data

	pStream->Write((char *)&m_iTick, sizeof(DWORD));
	pStream->Write((char *)&m_dwNextID, sizeof(DWORD));
	m_Time.WriteToStream(pStream);

	//	Extensions

	TArray<DWORD> ExtensionList;
	m_Design.GetEnabledExtensions(&ExtensionList);
	dwSave = ExtensionList.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&ExtensionList[0], dwSave * sizeof(DWORD));

	//	Adventure UNID

	dwSave = m_pAdventure->GetUNID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Write the ID of POV system

	dwSave = 0xffffffff;
	if (m_pPOV && m_pPOV->GetSystem())
		dwSave = m_pPOV->GetSystem()->GetID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Write the ID of the POV

	dwSave = OBJID_NULL;
	if (m_pPOV && m_pPOV->GetSystem())
		m_pPOV->GetSystem()->WriteObjRefToStream(m_pPOV, pStream);

	//	Calculate the amount of time that we've been playing the game

	CTimeSpan GameLength = StopGameTime();
	GameLength.WriteToStream(pStream);

	//	Save out topology node data

	DWORD dwCount = GetTopologyNodeCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		CTopologyNode *pNode = GetTopologyNode(i);
		pNode->WriteToStream(pStream);
		}

	//	Save out type data

	dwCount = m_Design.GetCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		CDesignType *pType = m_Design.GetEntry(i);

		dwSave = pType->GetUNID();
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		pType->WriteToStream(pStream);
		}

	return NOERROR;
	}

void CUniverse::SetCurrentSystem (CSystem *pSystem)

//	SetCurrentSystem
//
//	Sets the current system

	{
	m_pCurrentSystem = pSystem;

	if (pSystem)
		{
		g_KlicksPerPixel = pSystem->GetSpaceScale();
		g_TimeScale = pSystem->GetTimeScale();
		g_SecondsPerUpdate = g_TimeScale / g_TicksPerSecond;
		}
	else
		{
		g_KlicksPerPixel = KLICKS_PER_PIXEL;
		g_TimeScale = TIME_SCALE;
		g_SecondsPerUpdate = g_TimeScale / g_TicksPerSecond;
		}
	}

void CUniverse::SetHost (IHost *pHost)

//	SetHost
//
//	Sets the host for the universe
	
	{
	if (pHost == NULL)
		m_pHost = &g_DefaultHost;
	else
		m_pHost = pHost;
	}

void CUniverse::SetPlayer (CSpaceObject *pPlayer)

//	SetPlayer
//
//	Sets the current player

	{
	if (pPlayer == NULL)
		{
		CCodeChain &CC = GetCC();
		CC.DefineGlobal(CONSTLIT("gPlayer"), CC.CreateNil());
		CC.DefineGlobal(CONSTLIT("gPlayerShip"), CC.CreateNil());
		m_pPlayer = NULL;
		}
	else
		{
		CShip *pPlayerShip = pPlayer->AsShip();
		if (pPlayerShip == NULL)
			{
			ASSERT(false);
			return;
			}

		IShipController *pController = pPlayerShip->GetController();
		if (pController == NULL)
			{
			ASSERT(false);
			return;
			}

		if (!pController->IsPlayer())
			{
			ASSERT(false);
			return;
			}

		CObject *pControllerObj = dynamic_cast<CObject *>(pController);
		if (pControllerObj == NULL)
			{
			ASSERT(false);
			return;
			}

		CCodeChain &CC = GetCC();
		CC.DefineGlobal(CONSTLIT("gPlayer"), CC.CreateInteger((int)pControllerObj));
		CC.DefineGlobal(CONSTLIT("gPlayerShip"), CC.CreateInteger((int)pPlayer));
		m_pPlayer = pPlayer;
		}
	}

void CUniverse::SetPOV (CSpaceObject *pPOV)

//	SetPOV
//
//	Sets the current point of view. The POV is used for both painting
//	and to determine what should be updated.

	{
	if (m_pPOV)
		{
		CSpaceObject *pOldPOV = m_pPOV;
		m_pPOV = NULL;
		pOldPOV->OnLosePOV();
		}

	m_pPOV = pPOV;

	if (m_pPOV)
		SetCurrentSystem(m_pPOV->GetSystem());
	else
		SetCurrentSystem(NULL);
	}

void CUniverse::StartGame (bool bNewGame)

//	StartGame
//
//	Starts a game (either new or continued)

	{
	//	Load images necessary for the system

	LoadLibraryBitmaps();

	//	Fire some events

	if (bNewGame)
		{
		//	Tell all types that the universe have been created.
		//	This must be after m_Universe.SetPlayer, which sets up
		//	gPlayerShip.

		FireOnGlobalUniverseCreated();

		//	If we have a player then tell objects that the player has entered
		//	the system.

		if (m_pPlayer)
			{
			FireOnGlobalPlayerEnteredSystem();

			//	Tell all objects that the player has entered the system

			GetCurrentSystem()->PlayerEntered(m_pPlayer);
			}
		}

	//	Otherwise, if this is an old game being restored, fire OnGlobalUniverseLoad

	else
		{
		FireOnGlobalUniverseLoad();
		}
	}

void CUniverse::StartGameTime (void)

//	StartGameTime
//
//	Starts timing the game

	{
	m_StartTime = CTimeDate(CTimeDate::Now);
	}

CTimeSpan CUniverse::StopGameTime (void)

//	StopGameTime
//
//	Returns the time elapsed between m_StartTime and now.

	{
	CTimeDate StopTime(CTimeDate::Now);
	return timeSpan(m_StartTime, StopTime);
	}

void CUniverse::Update (Metric rSecondsPerTick, bool bForceEventFiring)

//	Update
//
//	Update the system of the current point of view

	{
	if (m_pPOV)
		m_pPOV->GetSystem()->Update(rSecondsPerTick, bForceEventFiring);

	m_iTick++;
	}

void CUniverse::UpdateExtended (void)

//	UpdateExtended
//
//	This is called when the player re-enters a system after having been
//	away. We update the system to reflect the amount of time that has passed.

	{
	CSystem *pSystem = GetCurrentSystem();
	ASSERT(pSystem);

	//	Calculate the amount of time that has elapsed from the last time the
	//	system was updated.

	CTimeSpan TotalTime(0);
	int iLastUpdated = pSystem->GetLastUpdated();
	if (iLastUpdated != -1)
		TotalTime = m_Time.GetElapsedTimeAt(m_iTick) - m_Time.GetElapsedTimeAt(iLastUpdated);

	//	Update the system 

	pSystem->UpdateExtended(TotalTime);
	}
