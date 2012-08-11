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
//	12: 1.08
//		CDynamicDesignTable in CUniverse
//
//	13: 1.08
//		Flags in CUniverse
//
//	14: 1.08c
//		dwRelease for extensions
//		Save extension UNID instead of adventureDesc UNID
//
//	15: 1.08d
//		dwRelease in CDynamicDesignTable
//
//	16: Fixed a corruption bug.
//		Versions 15 and 14 are marked are corrrupt
//
//	See: TSEUtil.h for definition of UNIVERSE_SAVE_VERSION

#include "PreComp.h"

struct SExtensionSaveDesc
	{
	DWORD dwUNID;
	DWORD dwRelease;
	};

const DWORD UNIVERSE_VERSION_MARKER =					0xffffffff;

const DWORD UNID_FIRST_DEFAULT_EFFECT =					0x00000010;

CUniverse *g_pUniverse = NULL;
Metric g_KlicksPerPixel = KLICKS_PER_PIXEL;
Metric g_TimeScale = TIME_SCALE;
Metric g_SecondsPerUpdate =	g_TimeScale / g_TicksPerSecond;

static CObjectClass<CUniverse>g_Class(OBJID_CUNIVERSE, NULL);

CEffectCreator *g_DefaultHitEffect[damageCount];
bool g_bDefaultHitEffectsInit = false;

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
		m_bBasicInit(false),

		m_bRegistered(false),
		m_iTick(1),
		m_pAdventure(NULL),
		m_pPOV(NULL),
		m_pPlayer(NULL),
		m_pCurrentSystem(NULL),
		m_StarSystems(TRUE, FALSE),
		m_dwNextID(1),

		m_pSoundMgr(NULL),

		m_pHost(&g_DefaultHost),
		m_bDebugMode(false),
		m_bNoSound(false),
		m_iLogImageLoad(0)

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

	m_Design.CleanUp();

	g_pUniverse = NULL;
	}

void CUniverse::AddSound (DWORD dwUNID, int iChannel)

//	AddSound
//
//	Adds a sound to the design

	{
	if (m_pSoundMgr == NULL)
		return;

	//	If this UNID is already in the list, then delete it

	int iOldChannel = FindSound(dwUNID);
	if (iOldChannel != -1)
		{
		m_pSoundMgr->Delete(iOldChannel);
		m_Sounds.RemoveEntry(iOldChannel, NULL);
		}

	//	Add the new one

	m_Sounds.AddEntry((int)dwUNID, (CObject *)iChannel);
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
		sFirstNode = m_Design.GetStartingNodeID();

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

	SetLogImageLoad(false);
	error = CSystem::CreateFromXML(this, pSystemType, pTopology, &pSystem, &sError, pStats);
	SetLogImageLoad(true);

	if (error)
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

bool CUniverse::FindByUNID (CIDTable &Table, DWORD dwUNID, CObject **retpObj)

//	FindByUNID
//
//	Returns an object by UNID from the appropriate table

	{
	CObject *pObj;

	if (Table.Lookup((int)dwUNID, &pObj) == NOERROR)
		{
		if (retpObj)
			*retpObj = pObj;

		return true;
		}
	else
		return false;
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

CEffectCreator *CUniverse::FindDefaultHitEffect (DamageTypes iDamage)

//	FindDefaultHitEffect
//
//	Returns the default hit effect for the given damage type

	{
	if (iDamage < damageLaser || iDamage >= damageCount)
		return FindEffectType(g_HitEffectUNID);

	InitDefaultHitEffects();
	return g_DefaultHitEffect[iDamage];
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
	ClearLibraryBitmapMarks();
	MarkLibraryBitmaps();
	SweepLibraryBitmaps();
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
		CExtension *pExtension = m_Design.GetExtension(i);

		if (!pExtension->IsHidden())
			{
			CString sName = pExtension->GetName();

			if (pExtension->GetType() == extAdventure)
				Stats.Insert(CONSTLIT("Adventure"), sName);
			else
				Stats.Insert(sName, NULL_STR, CONSTLIT("extensions"));
			}
		}

	//	Add registration status

	if (IsRegistered())
		Stats.Insert(CONSTLIT("Game"), CONSTLIT("Registered"));
	else if (InDebugMode())
		Stats.Insert(CONSTLIT("Game"), CONSTLIT("Debug"));
	else
		Stats.Insert(CONSTLIT("Game"), CONSTLIT("Unregistered"));
	}

const CDamageAdjDesc *CUniverse::GetArmorDamageAdj (int iLevel) const

//	GetArmorDamageAdj
//
//	Returns the armor damage adj table

	{
	if (m_pAdventure)
		return m_pAdventure->GetArmorDamageAdj(iLevel);
	else
		return CAdventureDesc::GetDefaultArmorDamageAdj(iLevel);
	}

const CDamageAdjDesc *CUniverse::GetShieldDamageAdj (int iLevel) const

//	GetShieldDamageAdj
//
//	Returns the shield damage table

	{
	if (m_pAdventure)
		return m_pAdventure->GetShieldDamageAdj(iLevel);
	else
		return CAdventureDesc::GetDefaultShieldDamageAdj(iLevel);
	}

void CUniverse::GetCurrentAdventureExtensions (TArray<DWORD> *retList)

//	GetCurrentAdventureExtensions
//
//	Returns the list of extensions enabled for the current adventure.
//	[Does not include any extensions that the adventure itself included.]

	{
	int i;

	retList->DeleteAll();
	for (i = 0; i < m_Design.GetExtensionCount(); i++)
		{
		CExtension *pExtension = m_Design.GetExtension(i);

		//	Do not include the actual adventure

		if (pExtension->GetType() == extAdventure || pExtension->GetType() == extBase)
			NULL;

		//	Do not include extensions that are explicitly included by the
		//	adventure;

		else if (pExtension->GetType() == extLibrary)
			NULL;

		//	Include other extensions

		else
			retList->Insert(pExtension->GetUNID());
		}
	}

CTimeSpan CUniverse::GetElapsedGameTime (void)

//	GetElapsedGameTime
//
//	Returns the amount of time that has elapsed in the game.

	{
	return m_Time.GetElapsedTimeAt(m_iTick);
	}

CString CUniverse::GetExtensionData (EStorageScopes iScope, DWORD dwExtension, const CString &sAttrib)

//	GetExtensionData
//
//	Returns data storage for the given extension

	{
	switch (iScope)
		{
		case storeDevice:
			return m_DeviceStorage.GetData(dwExtension, sAttrib);

		default:
			return NULL_STR;
		}
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
		sNodeID = m_Design.GetStartingNodeID();

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

ALERROR CUniverse::Init (SInitDesc &Ctx, CString *retsError)

//	Init
//
//	Initialize the universe according to the given settings. After this the
//	universe is fully loaded and bound.
//
//	This may be called (almost) at any time to change initialization parameters
//	(e.g., to change adventures or simply to reload the extensions directory.)

	{
	ALERROR error;

	//	Boot up

	if (!m_bBasicInit)
		{
		//	Initialize CodeChain

		if (error = InitCodeChain())
			{
			*retsError = CONSTLIT("Unable to initialize CodeChain.");
			return error;
			}

		//	Load local device storage

		if (error = InitDeviceStorage(retsError))
			return error;

		m_bBasicInit = true;
		}

	//	Initialize some stuff

	m_bDebugMode = Ctx.bDebugMode;

	//	We only load adventure desc (no need to load the whole thing)

	DWORD dwFlags = CExtensionCollection::FLAG_DESC_ONLY;

	//	Debug mode

	if (m_bDebugMode)
		dwFlags |= CExtensionCollection::FLAG_DEBUG_MODE;

	//	If requested we don't load resources

	if (Ctx.bNoResources)
		dwFlags |= CExtensionCollection::FLAG_NO_RESOURCES;

	//	Load everything

	if (error = m_Extensions.Load(Ctx.sFilespec, dwFlags, retsError))
		return error;

	//	Figure out the adventure to bind to.

	if (Ctx.pAdventure == NULL && Ctx.dwAdventure)
		{
		//	Look for the adventure by UNID.

		DWORD dwFindFlags = dwFlags | CExtensionCollection::FLAG_ADVENTURE_ONLY;

		if (!m_Extensions.FindBestExtension(Ctx.dwAdventure, 0, dwFindFlags, &Ctx.pAdventure))
			{
			*retsError = strPatternSubst(CONSTLIT("Unable to find adventure: %08x."), Ctx.dwAdventure);
			return ERR_FAIL;
			}
		}

	//	If necessary figure out which extensions to add

	if (Ctx.bDefaultExtensions)
		{
		if (error = m_Extensions.ComputeAvailableExtensions(Ctx.pAdventure, 
				dwFlags, 
				&Ctx.Extensions, 
				retsError))
			return error;
		}

	//	Get the bind order

	TArray<CExtension *> BindOrder;
	if (error = m_Extensions.ComputeBindOrder(Ctx.pAdventure,
			Ctx.Extensions,
			dwFlags, 
			&BindOrder,
			retsError))
		return error;

	//	Reinitialize. This clears out previous game state, but only if we
	//	are creating a new game.

	if (!Ctx.bInLoadGame)
		{
		if (error = Reinit())
			{
			*retsError = CONSTLIT("Unable to reinit.");
			return error;
			}
		}

	//	Set the current adventure (we need to do this before BindDesign, since
	//	we need the current adventure to get the shield and armor damage adj
	//	tables.

	SetCurrentAdventureDesc(Ctx.pAdventure->GetAdventureDesc());

	//	Bind
	//
	//	We don't need to log image load

	SetLogImageLoad(false);
	error = m_Design.BindDesign(BindOrder, !Ctx.bInLoadGame, retsError);
	SetLogImageLoad(true);

	if (error)
		return error;

	//	Now that we've bound we can delete any previous extensions

	m_Extensions.FreeDeleted();

	//	Init encounter tables

	InitLevelEncounterTables();

	return NOERROR;
	}

void CUniverse::InitDefaultHitEffects (void)

//	InitDefaultHitEffects
//
//	Initializes the default hit effects array

	{
	int i;

	if (!g_bDefaultHitEffectsInit)
		{
		for (i = 0; i < damageCount; i++)
			{
			g_DefaultHitEffect[i] = FindEffectType(UNID_FIRST_DEFAULT_EFFECT + i);
			if (g_DefaultHitEffect[i] == NULL)
				g_DefaultHitEffect[i] = FindEffectType(g_HitEffectUNID);
			}

		g_bDefaultHitEffectsInit = true;
		}
	}

ALERROR CUniverse::InitGame (CString *retsError)

//	InitGame
//
//	Initializes a game instance, including the topology. We assume that the
//	universe is loaded and bound.

	{
	ALERROR error;

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
	return m_bRegistered;
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
//	DWORD		flags
//	DWORD		m_dwNextID
//
//	CGameTimeKeeper
//
//	DWORD		No of enabled extensions
//	SExtensionSaveDesc for each
//
//	SExtensionSaveDesc for adventure
//
//	CDynamicDesignTable
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
//
//	NOTE: The debug mode flag is stored in the game file header. Callers must 
//	set the universe to debug mode from the game file header flag before calling
//	this function.
	
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

	//	Version 14 and 15 are corrupt.

	if (Ctx.dwVersion == 14 || Ctx.dwVersion == 15)
		{
		*retsError = CONSTLIT("The save file cannot be loaded due to a subtle corruption bug in 1.08c and 1.08d. I apologize for the problem.");
		return ERR_FAIL;
		}

	//	Flags

	if (Ctx.dwVersion >= 13)
		pStream->Read((char *)&dwLoad, sizeof(DWORD));
	else
		dwLoad = 0;

	m_bRegistered = ((dwLoad & 0x00000001) ? true : false);

	//	Load basic data

	pStream->Read((char *)&m_dwNextID, sizeof(DWORD));

	//	CGameTimeKeeper m_Time

	if (Ctx.dwVersion >= 7)
		m_Time.ReadFromStream(pStream);

	//	Prepare a universe initialization context
	//	NOTE: Caller has set debug mode based on game file header flag.

	CString sError;
	CUniverse::SInitDesc InitCtx;
	InitCtx.bDebugMode = g_pUniverse->InDebugMode();
	InitCtx.bInLoadGame = true;

	//	Load list of extensions used in this game

	TArray<SExtensionSaveDesc> ExtensionList;
	if (Ctx.dwVersion >= 14)
		{
		pStream->Read((char *)&dwLoad, sizeof(DWORD));
		ExtensionList.InsertEmpty(dwLoad);
		pStream->Read((char *)&ExtensionList[0], dwLoad * sizeof(SExtensionSaveDesc));
		}
	else if (Ctx.dwVersion >= 8)
		{
		TArray<DWORD> CompatibleExtensionList;

		pStream->Read((char *)&dwLoad, sizeof(DWORD));
		CompatibleExtensionList.InsertEmpty(dwLoad);
		pStream->Read((char *)&CompatibleExtensionList[0], dwLoad * sizeof(DWORD));

		ExtensionList.InsertEmpty(CompatibleExtensionList.GetCount());
		for (i = 0; i < CompatibleExtensionList.GetCount(); i++)
			{
			ExtensionList[i].dwUNID = CompatibleExtensionList[i];
			ExtensionList[i].dwRelease = 0;
			}
		}

	//	Get the actual extensions

	for (i = 0; i < ExtensionList.GetCount(); i++)
		{
		CExtension *pExtension;

		if (!m_Extensions.FindBestExtension(ExtensionList[i].dwUNID,
				ExtensionList[i].dwRelease,
				(g_pUniverse->InDebugMode() ? CExtensionCollection::FLAG_DEBUG_MODE : 0),
				&pExtension))
			{
			*retsError = strPatternSubst(CONSTLIT("Unable to find extension: %08x"), ExtensionList[i]);
			return ERR_FAIL;
			}

		InitCtx.Extensions.Insert(pExtension);
		}

	//	Load adventure extension

	if (Ctx.dwVersion >= 14)
		{
		SExtensionSaveDesc Desc;
		pStream->Read((char *)&Desc, sizeof(SExtensionSaveDesc));

		if (!m_Extensions.FindBestExtension(Desc.dwUNID,
				Desc.dwRelease,
				CExtensionCollection::FLAG_ADVENTURE_ONLY | (g_pUniverse->InDebugMode() ? CExtensionCollection::FLAG_DEBUG_MODE : 0),
				&InitCtx.pAdventure))
			{
			*retsError = strPatternSubst(CONSTLIT("Unable to find adventure: %08x"), Desc.dwUNID);
			return ERR_FAIL;
			}
		}
	else if (Ctx.dwVersion >= 2)
		{
		pStream->Read((char *)&dwLoad, sizeof(DWORD));

		if (!m_Extensions.FindAdventureFromDesc(dwLoad, 
				(g_pUniverse->InDebugMode() ? CExtensionCollection::FLAG_DEBUG_MODE : 0),
				&InitCtx.pAdventure))
			{
			*retsError = strPatternSubst(CONSTLIT("Unable to find adventure: %08x"), dwLoad);
			return ERR_FAIL;
			}
		}
	else
		{
		if (!m_Extensions.FindBestExtension(DEFAULT_ADVENTURE_EXTENSION_UNID, 
				0,
				(g_pUniverse->InDebugMode() ? CExtensionCollection::FLAG_DEBUG_MODE : 0),
				&InitCtx.pAdventure))
			{
			*retsError = strPatternSubst(CONSTLIT("Unable to find default adventure: %08x"), DEFAULT_ADVENTURE_EXTENSION_UNID);
			return ERR_FAIL;
			}
		}

	//	CDynamicDesignTable. Need to load this before we initialize.

	if (Ctx.dwVersion >= 12)
		m_Design.ReadDynamicTypes(Ctx);

	//	Select the proper adventure and extensions and bind design.

	if (Init(InitCtx, &sError) != NOERROR)
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

			try
				{
				pType->ReadFromStream(Ctx);
				}
			catch (...)
				{
				*retsError = strPatternSubst(CONSTLIT("Unable to read design type: %x"), pType->GetUNID());
				return ERR_FAIL;
				}
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
	//	We start at tick 1 because sometimes we need to start with some things
	//	in the past.

	m_iTick = 1;

	//	Clear some basic variables

	m_Time.DeleteAll();
	m_pPOV = NULL;
	SetCurrentSystem(NULL);
	m_StarSystems.RemoveAll();
	m_dwNextID = 1;

	//	NOTE: We don't reinitialize m_bDebugMode or m_bRegistered because those
	//	are set before Reinit (and thus we would overwrite them).

	//	Reinitialize some global classes

	CCompositeImageDesc::Reinit();

	//	Reinitialize types

	m_Design.Reinit();
	g_bDefaultHitEffectsInit = false;

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
//	DWORD		flags
//	DWORD		m_dwNextID
//	CGameTimeKeeper m_Time
//
//	DWORD		No of enabled extensions
//	SExtensionSaveDesc for each
//
//	SExtensionSaveDesc for adventure
//
//	CDynamicDesignTable
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

	dwSave = 0;
	dwSave |= (m_bRegistered ? 0x00000001 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_dwNextID, sizeof(DWORD));
	m_Time.WriteToStream(pStream);

	//	Extensions
	//
	//	NOTE: We don't save libraries because those will be automatically loaded
	//	based on adventure and extensions.

	TArray<CExtension *> ExtensionList;
	m_Design.GetEnabledExtensions(&ExtensionList);
	dwSave = ExtensionList.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < ExtensionList.GetCount(); i++)
		{
		SExtensionSaveDesc Desc;

		Desc.dwUNID = ExtensionList[i]->GetUNID();
		Desc.dwRelease = ExtensionList[i]->GetRelease();

		pStream->Write((char *)&Desc, sizeof(SExtensionSaveDesc));
		}

	//	Adventure UNID

	SExtensionSaveDesc Desc;
	Desc.dwUNID = m_pAdventure->GetExtension()->GetUNID();
	Desc.dwRelease = m_pAdventure->GetExtension()->GetRelease();
	pStream->Write((char *)&Desc, sizeof(SExtensionSaveDesc));

	//	CDynamicDesignTable

	m_Design.WriteDynamicTypes(pStream);

	//	Write the ID of POV system

	dwSave = 0xffffffff;
	if (m_pPOV && m_pPOV->GetSystem())
		dwSave = m_pPOV->GetSystem()->GetID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Write the ID of the POV

	dwSave = OBJID_NULL;
	if (m_pPOV && m_pPOV->GetSystem())
		{
		m_pPOV->GetSystem()->WriteObjRefToStream(m_pPOV, pStream);

		if (!m_pPOV->IsPlayer())
			kernelDebugLogMessage("ERROR: Saving without player ship.");
		}

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

bool CUniverse::SetExtensionData (EStorageScopes iScope, DWORD dwExtension, const CString &sAttrib, const CString &sData)

//	SetExtensionData
//
//	Sets data for the extension

	{
	switch (iScope)
		{
		case storeDevice:
			return m_DeviceStorage.SetData(dwExtension, sAttrib, sData);

		default:
			return false;
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

	MarkLibraryBitmaps();

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

CString CUniverse::ValidatePlayerName (const CString &sName)

//	ValidatePlayerName
//
//	Generates a valid name from the given user input

	{
	//	Trim leading and trailing whitespace

	CString sNewName = strTrimWhitespace(sName);

	//	Remove escape characters and symbols

	char *pPos = sNewName.GetASCIIZPointer();
	while (*pPos != '\0')
		{
		switch (*pPos)
			{
			case '/':
			case '%':
				*pPos = '_';
				break;

			default:
				if (strIsASCIIControl(pPos))
					*pPos = '_';
				break;
			}

		pPos++;
		}

	//	Make sure it is not blank

	if (sNewName.IsBlank())
		return CONSTLIT("Shofixti");

	//	Done

	return sNewName;
	}
