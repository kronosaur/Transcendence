//	CDesignCollection.cpp
//
//	CDesignCollection class

#include "PreComp.h"

#define STAR_SYSTEM_TOPOLOGY_TAG				CONSTLIT("StarSystemTopology")
#define SYSTEM_TOPOLOGY_TAG						CONSTLIT("SystemTopology")

#define CREDITS_ATTRIB							CONSTLIT("credits")
#define DEBUG_ONLY_ATTRIB						CONSTLIT("debugOnly")
#define EXTENDS_ATTRIB							CONSTLIT("extends")
#define FILENAME_ATTRIB							CONSTLIT("filename")
#define NAME_ATTRIB								CONSTLIT("name")
#define UNID_ATTRIB								CONSTLIT("UNID")
#define VERSION_ATTRIB							CONSTLIT("version")

#define GET_TYPE_SOURCE_EVENT					CONSTLIT("GetTypeSource")

static char *CACHED_EVENTS[CDesignCollection::evtCount] =
	{
		"GetGlobalAchievements",
		"GetGlobalDockScreen",
		"GetGlobalPlayerPriceAdj",
		"OnGlobalPaneInit",
		"OnGlobalMarkImages",

		"OnGlobalObjDestroyed",
		"OnGlobalSystemStarted",
		"OnGlobalSystemStopped",
		"OnGlobalUniverseCreated",
		"OnGlobalUniverseLoad",

		"OnGlobalUniverseSave",
		"OnGlobalUpdate",
	};

CDesignCollection::CDesignCollection (void) :
		m_Base(true),	//	m_Base owns its types and will free them at the end
		m_pAdventureDesc(NULL)

//	CDesignCollection construtor

	{
	int i;

	for (i = 0; i < evtCount; i++)
		m_EventsCache[i] = new CGlobalEventCache(CString(CACHED_EVENTS[i], -1, true));
	}

CDesignCollection::~CDesignCollection (void)

//	CDesignCollection destructor

	{
	int i;

	for (i = 0; i < evtCount; i++)
		delete m_EventsCache[i];

	CleanUp();
	}

ALERROR CDesignCollection::AddDynamicType (CExtension *pExtension, DWORD dwUNID, ICCItem *pSource, bool bNewGame, CString *retsError)

//	AddDynamicType
//
//	Adds a dynamic type at runtime

	{
	ALERROR error;

	//	If we're past game-create, the UNID must not already exist

	if (!bNewGame && FindEntry(dwUNID))
		{
		if (retsError)
			*retsError = strPatternSubst(CONSTLIT("Type already exists: %x"), dwUNID);
		return ERR_FAIL;
		}

	//	Add it to the dynamics table

	CDesignType *pType;
	if (error = m_DynamicTypes.DefineType(pExtension, dwUNID, pSource, &pType, retsError))
		return error;

	//	Make sure that the type can be created at this point.
	//	For example, we can't create SystemMap types after the game has started.

	switch (pType->GetType())
		{
		case designAdventureDesc:
		case designGlobals:
		case designImage:
		case designSound:
		case designEconomyType:
		case designTemplateType:
			{
			m_DynamicTypes.Delete(dwUNID);
			if (retsError)
				*retsError = CONSTLIT("Dynamic design type not supported.");
			return ERR_FAIL;
			}

		case designSystemType:
		case designSystemTable:
		case designSystemMap:
			{
			if (!bNewGame)
				{
				m_DynamicTypes.Delete(dwUNID);
				if (retsError)
					*retsError = CONSTLIT("Dynamic design type not supported after new game created.");
				return ERR_FAIL;
				}
			}
		}

	//	Since we've already bound, we need to simulate that here (although we
	//	[obviously] don't allow existing types to bind to dynamic types).
	//
	//	We start by adding the type to the AllTypes list

	m_AllTypes.AddOrReplaceEntry(pType);

	//	If this is new game time, then it means that we are inside of BindDesign. In
	//	that case, we don't do anything more (since BindDesign will take care of
	//	it).

	if (!bNewGame)
		{
		//	Next we add it to the specific type tables

		m_ByType[pType->GetType()].AddEntry(pType);

		//	Bind

		SDesignLoadCtx Ctx;
		Ctx.pExtension = pExtension;
		Ctx.bBindAsNewGame = false;

		if (error = pType->PrepareBindDesign(Ctx))
			{
			m_AllTypes.Delete(dwUNID);
			m_ByType[pType->GetType()].Delete(dwUNID);
			m_DynamicTypes.Delete(dwUNID);
			if (retsError)
				*retsError = Ctx.sError;
			return error;
			}

		if (error = pType->BindDesign(Ctx))
			{
			m_AllTypes.Delete(dwUNID);
			m_ByType[pType->GetType()].Delete(dwUNID);
			m_DynamicTypes.Delete(dwUNID);
			if (retsError)
				*retsError = Ctx.sError;
			return error;
			}

		//	Cache some global events

		CacheGlobalEvents(pType);

		//	Done binding

		if (error = pType->FinishBindDesign(Ctx))
			{
			m_AllTypes.Delete(dwUNID);
			m_ByType[pType->GetType()].Delete(dwUNID);
			m_DynamicTypes.Delete(dwUNID);
			if (retsError)
				*retsError = Ctx.sError;
			return error;
			}
		}

	return NOERROR;
	}

ALERROR CDesignCollection::BindDesign (const TArray<CExtension *> &BindOrder, bool bNewGame, bool bNoResources, CString *retsError)

//	BindDesign
//
//	Binds the design collection to the set of design types in the given list of
//	extensions.

	{
	DEBUG_TRY

	ALERROR error;
	int i;

	//	Unbind everything

	DEBUG_TRY

	CShipClass::UnbindGlobal();

	for (i = 0; i < m_AllTypes.GetCount(); i++)
		m_AllTypes.GetEntry(i)->UnbindDesign();
	m_AllTypes.DeleteAll();

	DEBUG_CATCH_MSG("Crash unbinding types.");

	//	Reset the bind tables

	for (i = 0; i < designCount; i++)
		m_ByType[i].DeleteAll();

	m_CreatedTypes.DeleteAll(true);
	m_OverrideTypes.DeleteAll();

	//	Reset

	m_pTopology = NULL;
	m_pAdventureExtension = NULL;

	//	Create a design load context

	SDesignLoadCtx Ctx;
	Ctx.bBindAsNewGame = bNewGame;
	Ctx.bNoResources = bNoResources;

	//	Loop over the bind list in order and add appropriate types to m_AllTypes
	//	(The order guarantees that the proper types override)

	for (i = 0; i < BindOrder.GetCount(); i++)
		{
		CExtension *pExtension = BindOrder[i];

		try {

		const CDesignTable &Types = pExtension->GetDesignTypes();

#ifdef DEBUG_BIND
		::OutputDebugString(strPatternSubst(CONSTLIT("EXTENSION %s\n"), pExtension->GetName()));
		for (int j = 0; j < Types.GetCount(); j++)
			{
			::OutputDebugString(strPatternSubst(CONSTLIT("%08x: %s\n"), Types.GetEntry(j)->GetUNID(), Types.GetEntry(j)->GetTypeName()));
			}
#endif

		//	Run globals for the extension

		if (error = pExtension->ExecuteGlobals(Ctx))
			{
			*retsError = Ctx.sError;
			return error;
			}

		//	Add the types

		m_AllTypes.Merge(Types, &m_OverrideTypes);

		//	If this is the adventure, then remember it

		if (pExtension->GetType() == extAdventure)
			{
			m_pAdventureExtension = pExtension;
			m_pAdventureDesc = pExtension->GetAdventureDesc();
			}

		//	If this is an adventure or the base extension then take the 
		//	topology.

		if (pExtension->GetType() == extAdventure || pExtension->GetType() == extBase)
			m_pTopology = &pExtension->GetTopology();

		} catch (...)
			{
			::kernelDebugLogMessage("Crash processing extension:");
			CExtension::DebugDump(pExtension, true);
			throw;
			}
		}

	//	If this is a new game, then create all the Template types

	if (bNewGame)
		{
		DEBUG_TRY

		m_DynamicUNIDs.DeleteAll();
		m_DynamicTypes.DeleteAll();

		if (error = FireOnGlobalTypesInit(Ctx))
			{
			*retsError = Ctx.sError;
			return error;
			}

		if (error = CreateTemplateTypes(Ctx))
			{
			*retsError = Ctx.sError;
			return error;
			}

		DEBUG_CATCH_MSG("Crash defining dynamic types.");
		}

	//	Add all the dynamic types. These came either from the saved game file or
	//	from the Template types above.

	m_AllTypes.Merge(m_DynamicTypes, &m_OverrideTypes);

	//	Now resolve all overrides and inheritance

	if (error = ResolveOverrides(Ctx))
		{
		*retsError = Ctx.sError;
		return error;
		}

	//	Initialize the byType lists

	DEBUG_TRY
	for (i = 0; i < m_AllTypes.GetCount(); i++)
		{
		CDesignType *pEntry = m_AllTypes.GetEntry(i);
		m_ByType[pEntry->GetType()].AddEntry(pEntry);
		}
	DEBUG_CATCH_MSG("Crash initializing byType lists.");

	//	Set our adventure desc as current; since adventure descs are always 
	//	loaded this is the only thing that we can use to tell if we should
	//	call global events.
	//
	//	This must happen after Unbind (because that clears it) and before
	//	PrepareBindDesign.
	//
	//	NOTE: m_pAdventureDesc can be NULL (e.g., in the intro screen).

	DEBUG_TRY
	if (m_pAdventureDesc)
		m_pAdventureDesc->SetCurrentAdventure();
	DEBUG_CATCH_MSG("Crash setting current adventure.");

	//	Cache a map between currency name and economy type
	//	We need to do this before Bind because some types will lookup
	//	a currency name during Bind.

	DEBUG_TRY
	m_EconomyIndex.DeleteAll();
	for (i = 0; i < GetCount(designEconomyType); i++)
		{
		CEconomyType *pEcon = CEconomyType::AsType(GetEntry(designEconomyType, i));
		const CString &sName = pEcon->GetSID();

		bool bUnique;
		CEconomyType **ppDest = m_EconomyIndex.SetAt(sName, &bUnique);
		if (!bUnique)
			{
			pEcon->ComposeLoadError(Ctx, CONSTLIT("Currency ID must be unique"));
			*retsError = Ctx.sError;
			return ERR_FAIL;
			}

		*ppDest = pEcon;
		}
	DEBUG_CATCH_MSG("Crash initializing economies.");

	//	Prepare to bind. This is used by design elements that need two passes
	//	to bind. We also use it to set up the inheritence hierarchy, which means
	//	that we rely on the map from UNID to valid design type (m_AllTypes)

	m_DisplayAttribs.DeleteAll();

	DEBUG_TRY
	for (i = 0; i < m_AllTypes.GetCount(); i++)
		{
		CDesignType *pEntry = m_AllTypes.GetEntry(i);
		if (error = pEntry->PrepareBindDesign(Ctx))
			{
			*retsError = Ctx.sError;
			return error;
			}

		//	We take this opportunity to build a list of display attributes
		//	defined by each type.

		const CDisplayAttributeDefinitions &Attribs = pEntry->GetDisplayAttributes();
		if (!Attribs.IsEmpty())
			m_DisplayAttribs.Append(Attribs);
		}
	DEBUG_CATCH_MSG("Crash in PrepareBind.");

	//	Now call Bind on all active design entries

	for (i = 0; i < evtCount; i++)
		m_EventsCache[i]->DeleteAll();

	DEBUG_TRY
	for (i = 0; i < m_AllTypes.GetCount(); i++)
		{
		CDesignType *pEntry = m_AllTypes.GetEntry(i);
		if (error = pEntry->BindDesign(Ctx))
			{
			*retsError = Ctx.sError;
			return error;
			}

		//	Cache some global events. We keep track of the global events for
		//	all types so that we can access them faster.

		CacheGlobalEvents(pEntry);
		}
	DEBUG_CATCH_MSG("Crash in BindDesign.");

	//	Finish binding. This pass is used by design elements
	//	that need to do stuff after all designs are bound.

	DEBUG_TRY
	for (i = 0; i < m_AllTypes.GetCount(); i++)
		{
		CDesignType *pEntry = m_AllTypes.GetEntry(i);
		if (error = pEntry->FinishBindDesign(Ctx))
			{
			*retsError = Ctx.sError;
			return error;
			}
		}
	DEBUG_CATCH_MSG("Crash in FinishBind.");

	//	Remember what we bound

	m_BoundExtensions = BindOrder;

	return NOERROR;

	DEBUG_CATCH
	}

void CDesignCollection::CacheGlobalEvents (CDesignType *pType)

//	CacheGlobalEvents
//
//	Caches global events for the given type

	{
	DEBUG_TRY

	int i, j;

	const CEventHandler *pEvents;
	TSortMap<CString, SEventHandlerDesc> FullEvents;
	pType->GetEventHandlers(&pEvents, &FullEvents);
	if (pEvents)
		{
		SEventHandlerDesc Event;
		Event.pExtension = pType->GetExtension();

		for (i = 0; i < pEvents->GetCount(); i++)
			{
			CString sEvent = pEvents->GetEvent(i, &Event.pCode);

			for (j = 0; j < evtCount; j++)
				if (m_EventsCache[j]->Insert(pType, sEvent, Event))
					break;
			}
		}
	else
		{
		for (i = 0; i < FullEvents.GetCount(); i++)
			{
			CString sEvent = FullEvents.GetKey(i);
			const SEventHandlerDesc &Event = FullEvents[i];

			for (j = 0; j < evtCount; j++)
				if (m_EventsCache[j]->Insert(pType, sEvent, Event))
					break;
			}
		}

	DEBUG_CATCH
	}

void CDesignCollection::CleanUp (void)

//	CleanUp
//
//	Free all entries so that we don't hold on to any resources.

	{
	m_CreatedTypes.DeleteAll(true);

	//	Some classes need to clean up global data
	//	(But we need to do this before we destroy the types)

	CStationType::Reinit();
	}

void CDesignCollection::ClearImageMarks (void)

//	ClearImageMarks
//
//	Clears marks on all images

	{
	int i;

	for (i = 0; i < GetCount(designImage); i++)
		{
		CObjectImage *pImage = CObjectImage::AsType(GetEntry(designImage, i));
		pImage->ClearMark();
		}
	}

ALERROR CDesignCollection::CreateTemplateTypes (SDesignLoadCtx &Ctx)

//	CreateTemplateTypes
//
//	This is called inside of BindDesign to create all template types

	{
	DEBUG_TRY

	ALERROR error;
	int i;

	//	Create an appropriate context for running code

	CCodeChainCtx CCCtx;

	//	Loop over all active types looking for templates.
	//	NOTE: We cannot use the type-specific arrays because they have not been
	//	set up yet (remember that we are inside of BindDesign).

	for (i = 0; i < m_AllTypes.GetCount(); i++)
		{
		CDesignType *pTemplate = m_AllTypes.GetEntry(i);
		if (pTemplate->GetType() != designTemplateType)
			continue;

		//	Get the function to generate the type source

		SEventHandlerDesc Event;
		if (pTemplate->FindEventHandler(GET_TYPE_SOURCE_EVENT, &Event))
			{
			ICCItem *pResult = CCCtx.Run(Event);
			if (pResult->IsError())
				{
				Ctx.sError = strPatternSubst(CONSTLIT("GetTypeSource (%x): %s"), pTemplate->GetUNID(), pResult->GetStringValue());
				return ERR_FAIL;
				}
			else if (!pResult->IsNil())
				{
				if (error = AddDynamicType(pTemplate->GetExtension(), pTemplate->GetUNID(), pResult, true, &Ctx.sError))
					return error;
				}

			CCCtx.Discard(pResult);
			}
		}

	return NOERROR;

	DEBUG_CATCH
	}

CExtension *CDesignCollection::FindExtension (DWORD dwUNID) const

//	FindExtension
//
//	Find the entry for the given extension

	{
	int i;

	for (i = 0; i < m_BoundExtensions.GetCount(); i++)
		if (m_BoundExtensions[i]->GetUNID() == dwUNID)
			return m_BoundExtensions[i];

	return NULL;
	}

CXMLElement *CDesignCollection::FindSystemFragment (const CString &sName, CSystemTable **retpTable) const

//	FindSystemFragment
//
//	Looks up the given system fragment in all system tables

	{
	int i;

	for (i = 0; i < GetCount(designSystemTable); i++)
		{
		CSystemTable *pTable = CSystemTable::AsType(GetEntry(designSystemTable, i));
		if (pTable)
			{
			CXMLElement *pFragment = pTable->FindElement(sName);
			if (pFragment)
				{
				if (retpTable)
					*retpTable = pTable;

				return pFragment;
				}
			}
		}

	return NULL;
	}

void CDesignCollection::FireGetGlobalAchievements (CGameStats &Stats)

//	FireGetGlobalAchievements
//
//	Fire event to fill achievements

	{
	int i;

	for (i = 0; i < m_EventsCache[evtGetGlobalAchievements]->GetCount(); i++)
		{
		CDesignType *pType = m_EventsCache[evtGetGlobalAchievements]->GetEntry(i);

		pType->FireGetGlobalAchievements(Stats);
		}
	}

bool CDesignCollection::FireGetGlobalDockScreen (CSpaceObject *pObj, CString *retsScreen, ICCItem **retpData, int *retiPriority)

//	FireGetGlobalDockScreen
//
//	Allows types to override the dock screen for an object.
//	NOTE: If we return TRUE, callers must discard *retpData.

	{
	int i;
	CCodeChain &CC = g_pUniverse->GetCC();

	int iBestPriority = -1;
	CString sBestScreen;
	ICCItem *pBestData = NULL;

	//	Loop over all types and get the highest priority screen

	for (i = 0; i < m_EventsCache[evtGetGlobalDockScreen]->GetCount(); i++)
		{
		SEventHandlerDesc Event;
		CDesignType *pType = m_EventsCache[evtGetGlobalDockScreen]->GetEntry(i, &Event);

		int iPriority;
		CString sScreen;
		ICCItem *pData;
		if (pType->FireGetGlobalDockScreen(Event, pObj, &sScreen, &pData, &iPriority))
			{
			if (iPriority > iBestPriority)
				{
				iBestPriority = iPriority;
				sBestScreen = sScreen;

				if (pBestData)
					pBestData->Discard(&CC);

				pBestData = pData;
				}
			else
				{
				if (pData)
					pData->Discard(&CC);
				}
			}
		}

	//	If none found, then we're done

	if (iBestPriority == -1)
		return false;

	//	Otherwise, return screen

	if (retsScreen)
		*retsScreen = sBestScreen;

	if (retiPriority)
		*retiPriority = iBestPriority;

	if (retpData)
		*retpData = pBestData;
	else
		{
		if (pBestData)
			pBestData->Discard(&CC);
		}

	return true;
	}

bool CDesignCollection::FireGetGlobalPlayerPriceAdj (ETradeServiceTypes iService, CSpaceObject *pProvider, const CItem &Item, ICCItem *pData, int *retiPriceAdj)

//	FireGetGlobalPlayerPriceAdj
//
//	Returns a price adjustment for the player, given a service, a provider,
//	and an item.

	{
	int i;

	//	Fire all events

	int iPriceAdj = 100;
	for (i = 0; i < m_EventsCache[evtGetGlobalPlayerPriceAdj]->GetCount(); i++)
		{
		SEventHandlerDesc Event;
		CDesignType *pType = m_EventsCache[evtGetGlobalPlayerPriceAdj]->GetEntry(i, &Event);

		int iSinglePriceAdj;
		if (pType->FireGetGlobalPlayerPriceAdj(Event, iService, pProvider, Item, pData, &iSinglePriceAdj))
			{
			if (iSinglePriceAdj < 0)
				{
				iPriceAdj = -1;
				break;
				}
			else
				iPriceAdj = iPriceAdj * iSinglePriceAdj / 100;
			}
		}

	//	Done

	if (retiPriceAdj)
		*retiPriceAdj = (iPriceAdj < 0 ? -1 : iPriceAdj);

	return (iPriceAdj != 100);
	}

void CDesignCollection::FireOnGlobalMarkImages (void)

//	FireOnGlobalMarkImages
//
//	Allows types to mark images

	{
	int i;

	//	Fire all events

	for (i = 0; i < m_EventsCache[evtOnGlobalMarkImages]->GetCount(); i++)
		{
		SEventHandlerDesc Event;
		CDesignType *pType = m_EventsCache[evtOnGlobalMarkImages]->GetEntry(i, &Event);

		pType->FireOnGlobalMarkImages(Event);
		}
	}

void CDesignCollection::FireOnGlobalObjDestroyed (SDestroyCtx &Ctx)

//	FireOnGlobalObjDestroyed
//
//	Gives other types a notification when an object is destroyed

	{
	int i;

	//	Fire all events

	for (i = 0; i < m_EventsCache[evtOnGlobalObjDestroyed]->GetCount(); i++)
		{
		SEventHandlerDesc Event;
		CDesignType *pType = m_EventsCache[evtOnGlobalObjDestroyed]->GetEntry(i, &Event);

		pType->FireOnGlobalObjDestroyed(Event, Ctx);
		}
	}

void CDesignCollection::FireOnGlobalPaneInit (void *pScreen, CDesignType *pRoot, const CString &sScreen, const CString &sPane)

//	FireOnGlobalPaneInit
//
//	Give other design types a way to override screens

	{
	int i;
	CString sError;

	//	Generate a screen UNID that contains both the screen UNID and a local screen

	CString sScreenUNID = CDockScreenType::GetStringUNID(pRoot, sScreen);
	DWORD dwRootUNID = (pRoot ? pRoot->GetUNID() : 0);

	//	Fire all events

	for (i = 0; i < m_EventsCache[evtOnGlobalDockPaneInit]->GetCount(); i++)
		{
		SEventHandlerDesc Event;
		CDesignType *pType = m_EventsCache[evtOnGlobalDockPaneInit]->GetEntry(i, &Event);

		if (pType->FireOnGlobalDockPaneInit(Event,
				pScreen,
				dwRootUNID,
				sScreenUNID,
				sPane,
				&sError) != NOERROR)
			kernelDebugLogMessage(sError);
		}
	}

void CDesignCollection::FireOnGlobalPlayerChangedShips (CSpaceObject *pOldShip)

//	FireOnGlobalPlayerChangedShips
//
//	Fire event

	{
	int i;

	CString sError;
	for (i = 0; i < GetCount(); i++)
		{
		if (GetEntry(i)->FireOnGlobalPlayerChangedShips(pOldShip, &sError) != NOERROR)
			kernelDebugLogMessage(sError);
		}
	}

void CDesignCollection::FireOnGlobalPlayerEnteredSystem (void)

//	FireOnGlobalPlayerEnteredSystem
//
//	Fire event

	{
	int i;

	CString sError;
	for (i = 0; i < GetCount(); i++)
		{
		if (GetEntry(i)->FireOnGlobalPlayerEnteredSystem(&sError) != NOERROR)
			kernelDebugLogMessage(sError);
		}
	}

void CDesignCollection::FireOnGlobalPlayerLeftSystem (void)

//	FireOnGlobalPlayerLeftSystem
//
//	Fire event

	{
	int i;

	CString sError;
	for (i = 0; i < GetCount(); i++)
		{
		if (GetEntry(i)->FireOnGlobalPlayerLeftSystem(&sError) != NOERROR)
			kernelDebugLogMessage(sError);
		}
	}

void CDesignCollection::FireOnGlobalSystemCreated (SSystemCreateCtx &SysCreateCtx)

//	FireOnGlobalSystemCreate
//
//	Notify all type that a system has been created

	{
	int i;

	CString sError;
	for (i = 0; i < GetCount(); i++)
		{
		if (GetEntry(i)->FireOnGlobalSystemCreated(SysCreateCtx, &sError) != NOERROR)
			kernelDebugLogMessage(sError);
		}
	}

void CDesignCollection::FireOnGlobalSystemStarted (void)

//	FireOnGlobalSystemStarted
//
//	Notify all types that a system is starting

	{
	int i;

	//	Fire all events

	for (i = 0; i < m_EventsCache[evtOnGlobalSystemStarted]->GetCount(); i++)
		{
		SEventHandlerDesc Event;
		CDesignType *pType = m_EventsCache[evtOnGlobalSystemStarted]->GetEntry(i, &Event);

		pType->FireOnGlobalSystemStarted(Event);
		}
	}

void CDesignCollection::FireOnGlobalSystemStopped (void)

//	FireOnGlobalSystemStopped
//
//	Notify all types that a system has stopped

	{
	int i;

	//	Fire all events

	for (i = 0; i < m_EventsCache[evtOnGlobalSystemStopped]->GetCount(); i++)
		{
		SEventHandlerDesc Event;
		CDesignType *pType = m_EventsCache[evtOnGlobalSystemStopped]->GetEntry(i, &Event);

		pType->FireOnGlobalSystemStopped(Event);
		}
	}

ALERROR CDesignCollection::FireOnGlobalTypesInit (SDesignLoadCtx &Ctx)

//	FireOnGlobalTypesInit
//
//	Give each type a chance to create dynamic types before we bind.

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_AllTypes.GetCount(); i++)
		{
		CDesignType *pType = m_AllTypes.GetEntry(i);
		if (error = pType->FireOnGlobalTypesInit(Ctx))
			return error;
		}

	return NOERROR;
	}

void CDesignCollection::FireOnGlobalUniverseCreated (void)

//	FireOnGlobalUniverseCreated
//
//	Notify all types that the universe has been created

	{
	int i;

	CString sError;
	for (i = 0; i < m_EventsCache[evtOnGlobalUniverseCreated]->GetCount(); i++)
		{
		SEventHandlerDesc Event;
		CDesignType *pType = m_EventsCache[evtOnGlobalUniverseCreated]->GetEntry(i, &Event);

		pType->FireOnGlobalUniverseCreated(Event);
		}
	}

void CDesignCollection::FireOnGlobalUniverseLoad (void)

//	FireOnGlobalUniverseLoad
//
//	Notify all types that the universe has loaded

	{
	int i;

	CString sError;
	for (i = 0; i < m_EventsCache[evtOnGlobalUniverseLoad]->GetCount(); i++)
		{
		SEventHandlerDesc Event;
		CDesignType *pType = m_EventsCache[evtOnGlobalUniverseLoad]->GetEntry(i, &Event);

		pType->FireOnGlobalUniverseLoad(Event);
		}
	}

void CDesignCollection::FireOnGlobalUniverseSave (void)

//	FireOnGlobalUniverseSave
//
//	Notify all types that the universe is about to be saved to disk

	{
	int i;

	CString sError;
	for (i = 0; i < m_EventsCache[evtOnGlobalUniverseSave]->GetCount(); i++)
		{
		SEventHandlerDesc Event;
		CDesignType *pType = m_EventsCache[evtOnGlobalUniverseSave]->GetEntry(i, &Event);

		pType->FireOnGlobalUniverseSave(Event);
		}
	}

void CDesignCollection::FireOnGlobalUpdate (int iTick)

//	FireOnGlobalUpdate
//
//	Types get a chance to do whatever they want once every 15 ticks.

	{
	int i;

	CString sError;
	for (i = 0; i < m_EventsCache[evtOnGlobalUpdate]->GetCount(); i++)
		{
		SEventHandlerDesc Event;
		CDesignType *pType = m_EventsCache[evtOnGlobalUpdate]->GetEntry(i, &Event);

		if ((((DWORD)iTick + pType->GetUNID()) % GLOBAL_ON_UPDATE_CYCLE) == 0)
			pType->FireOnGlobalUpdate(Event);
		}
	}

DWORD CDesignCollection::GetDynamicUNID (const CString &sName)

//	GetDynamicUNID
//
//	Returns an UNID for the given unique name.

	{
	//	First look in the table to see if the UNID already exists.

	DWORD dwAtom = m_DynamicUNIDs.atom_Find(sName);
	if (dwAtom == NULL_ATOM)
		m_DynamicUNIDs.atom_Insert(sName, &dwAtom);

	if (dwAtom >= 0x10000000)
		return 0;

	return 0xf0000000 + dwAtom;
	}

void CDesignCollection::GetEnabledExtensions (TArray<CExtension *> *retExtensionList)

//	GetEnabledExtensions
//
//	Returns the list of enabled extensions

	{
	int i;

	retExtensionList->DeleteAll();

	for (i = 0; i < GetExtensionCount(); i++)
		{
		CExtension *pEntry = GetExtension(i);
		if (pEntry->GetType() == extExtension)
			retExtensionList->Insert(pEntry);
		}
	}

CG16bitImage *CDesignCollection::GetImage (DWORD dwUNID, DWORD dwFlags)

//	GetImage
//
//	Returns an image

	{
	CDesignType *pType = m_AllTypes.FindByUNID(dwUNID);
	if (pType == NULL)
		return NULL;

	CObjectImage *pImage = CObjectImage::AsType(pType);
	if (pImage == NULL)
		return NULL;

	if (dwFlags & FLAG_IMAGE_COPY)
		return pImage->CreateCopy();
	else
		{
		CString sError;
		CG16bitImage *pRawImage = pImage->GetImage(strFromInt(dwUNID), &sError);

		if (pRawImage == NULL)
			kernelDebugLogMessage(sError);

		//	Lock, if requested. NOTE: Since we obtained the image above,
		//	this call is guaranteed to succeed.

		if (dwFlags & FLAG_IMAGE_LOCK)
			pImage->Lock(SDesignLoadCtx());

		//	Done

		return pRawImage;
		}
	}

CString CDesignCollection::GetStartingNodeID (void)

//	GetStartingNodeID
//
//	Gets the default starting node ID (if the player ship does not define it).

	{
	int i;

	if (!m_pTopology->GetFirstNodeID().IsBlank())
		return m_pTopology->GetFirstNodeID();

	//	See if any map defines a starting node ID

	for (i = 0; i < GetCount(designSystemMap); i++)
		{
		CSystemMap *pMap = CSystemMap::AsType(GetEntry(designSystemMap, i));

		if (!pMap->GetStartingNodeID().IsBlank())
			return pMap->GetStartingNodeID(); 
		}

	//	Not found

	return NULL_STR;
	}

bool CDesignCollection::IsAdventureExtensionBound (DWORD dwUNID)

//	IsAdventureExtensionBound
//
//	Returns TRUE if we have bound on the given adventure extension

	{
	if (m_pAdventureExtension)
		return (m_pAdventureExtension->GetUNID() == dwUNID);
	else
		return (dwUNID == 0);
	}

bool CDesignCollection::IsRegisteredGame (void)

//	IsRegisteredGame
//
//	Returns TRUE if all enabled extensions are registered.

	{
	int i;

	for (i = 0; i < m_BoundExtensions.GetCount(); i++)
		if (!m_BoundExtensions[i]->IsRegistrationVerified())
			return false;

	return true;
	}

void CDesignCollection::MarkGlobalImages (void)

//	MarkGlobalImages
//
//	Mark images needed by types. In general we only mark images that are global
//	(such as damage bitmap images used by ship classes).

	{
	}

void CDesignCollection::NotifyTopologyInit (void)

//	NotifyTopologyInit
//
//	Notify all types that the topology has been initialized.

	{
	int i;

	for (i = 0; i < m_AllTypes.GetCount(); i++)
		{
		CDesignType *pType = m_AllTypes.GetEntry(i);
		pType->TopologyInitialized();
		}
	}

void CDesignCollection::ReadDynamicTypes (SUniverseLoadCtx &Ctx)

//	ReadDynamicTypes
	
	{
	int i;

	//	We need to unbind because there may be some dynamic types from a previous
	//	game that we're about to delete.
	//
	//	This HACK is caused by the fact that Universe::Init is also calling 
	//	BindDesign but ReadDynamicTypes must be called before Init.
	//
	//	A better way to fix this is to load the dynamic types into a separate
	//	structure and pass them into Init to be added then.

	for (i = 0; i < m_AllTypes.GetCount(); i++)
		m_AllTypes.GetEntry(i)->UnbindDesign();
	m_AllTypes.DeleteAll();

	//	Read them

	m_DynamicTypes.ReadFromStream(Ctx);

	//	Read dynamic UNIDs

	m_DynamicUNIDs.DeleteAll();

	DWORD dwCount;
	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));

	for (i = 0; i < (int)dwCount; i++)
		{
		CString sName;
		sName.ReadFromStream(Ctx.pStream);

		DWORD dwAtom;
		m_DynamicUNIDs.atom_Insert(sName, &dwAtom);
		ASSERT(dwAtom == (DWORD)i);
		}
	}

void CDesignCollection::Reinit (void)

//	Reinit
//
//	Reinitialize all types

	{
	int i;

	//	Reinitialize some global classes

	CStationType::Reinit();

	//	For reinit that requires two passes

	for (i = 0; i < GetCount(); i++)
		{
		CDesignType *pType = GetEntry(i);
		pType->PrepareReinit();
		}

	//	Reinit design

	for (i = 0; i < GetCount(); i++)
		{
		CDesignType *pType = GetEntry(i);
		pType->Reinit();
		}
	}

ALERROR CDesignCollection::ResolveOverrides (SDesignLoadCtx &Ctx)

//	ResolveOverrides
//
//	Resolve all overrides

	{
	int i;

	//	Apply all overrides

	for (i = 0; i < m_OverrideTypes.GetCount(); i++)
		{
		CDesignType *pOverride = m_OverrideTypes.GetEntry(i);

		//	Find the type that we are trying to override. If we can't find it
		//	then just continue without error (it means we're trying to override
		//	a type that doesn't currently exist).

		CDesignType *pType = m_AllTypes.FindByUNID(pOverride->GetUNID());
		if (pType == NULL)
			continue;

		//	If this type is not already a clone then we need to clone it first
		//	(Because we never modify the original loaded type).

		if (!pType->IsClone())
			{
			CDesignType *pClone;
			pType->CreateClone(&pClone);

			m_CreatedTypes.AddEntry(pClone);

			pType = pClone;
			}

		//	Now modify the type with the override

		pType->MergeType(pOverride);

		//	Replace the original

		m_AllTypes.AddOrReplaceEntry(pType);
		}

	//	Done

	return NOERROR;
	}

void CDesignCollection::SweepImages (void)

//	SweepImages
//
//	Frees any images that are not marked

	{
	int i;

	for (i = 0; i < GetCount(designImage); i++)
		{
		CObjectImage *pImage = CObjectImage::AsType(GetEntry(designImage, i));
		pImage->Sweep();
		}
	}

void CDesignCollection::WriteDynamicTypes (IWriteStream *pStream)

//	WriteDynamicTypes
//
//	Write dynamic types to the game file
	
	{
	int i;

	m_DynamicTypes.WriteToStream(pStream);

	//	Now write any dynamic UNIDs

	DWORD dwCount = m_DynamicUNIDs.GetCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));

	for (i = 0; i < (int)dwCount; i++)
		m_DynamicUNIDs.atom_GetKey((DWORD)i).WriteToStream(pStream);
	}

//	CDesignList ----------------------------------------------------------------

void CDesignList::Delete (DWORD dwUNID)

//	Delete
//
//	Deletes the entry by UNID

	{
	int i;

	for (i = 0; i < m_List.GetCount(); i++)
		if (m_List[i]->GetUNID() == dwUNID)
			{
			m_List.Delete(i);
			break;
			}
	}

void CDesignList::DeleteAll (bool bFree)

//	DeleteAll
//
//	Delete all entries
	
	{
	int i;

	if (bFree)
		{
		for (i = 0; i < m_List.GetCount(); i++)
			delete m_List[i];
		}

	m_List.DeleteAll();
	}
