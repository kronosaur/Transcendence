//	CDesignCollection.cpp
//
//	CDesignCollection class

#include "PreComp.h"

#define MODULES_TAG								CONSTLIT("Modules")
#define STAR_SYSTEM_TOPOLOGY_TAG				CONSTLIT("StarSystemTopology")
#define SYSTEM_TOPOLOGY_TAG						CONSTLIT("SystemTopology")

#define CREDITS_ATTRIB							CONSTLIT("credits")
#define DEBUG_ONLY_ATTRIB						CONSTLIT("debugOnly")
#define EXTENDS_ATTRIB							CONSTLIT("extends")
#define FILENAME_ATTRIB							CONSTLIT("filename")
#define NAME_ATTRIB								CONSTLIT("name")
#define UNID_ATTRIB								CONSTLIT("UNID")
#define VERSION_ATTRIB							CONSTLIT("version")

static char *CACHED_EVENTS[CDesignCollection::evtCount] =
	{
		"GetGlobalAchievements",
		"GetGlobalDockScreen",
		"OnGlobalPaneInit",
		"OnGlobalObjDestroyed",
		"OnGlobalUniverseCreated",

		"OnGlobalUniverseLoad",
		"OnGlobalUniverseSave",
	};

CDesignCollection::CDesignCollection (void) :
		m_pAdventureDesc(NULL),
		m_dwNextAnonymousUNID(0xf0000001)

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

	RemoveAll();
	}

ALERROR CDesignCollection::AddEntry (SDesignLoadCtx &Ctx, CDesignType *pEntry)

//	AddEntry
//
//	Adds an entry to the collection

	{
	ALERROR error;

	DWORD dwUNID = pEntry->GetUNID();

	//	If this is an extension, then add to the appropriate extension

	CDesignTable *pTable = NULL;
	if (Ctx.pExtension)
		{
		pTable = &Ctx.pExtension->Table;

		//	If the UNID of the entry does not belong to the extension, then make sure it
		//	overrides a valid base entry

		if (!Ctx.pExtension->bRegistered
				&& (dwUNID & UNID_DOMAIN_AND_MODULE_MASK) != (Ctx.pExtension->dwUNID & UNID_DOMAIN_AND_MODULE_MASK))
			{
			//	Cannot override AdventureDesc

			if (pEntry->GetType() == designAdventureDesc)
				{
				Ctx.sError = CONSTLIT("<AdventureDesc> UNID must be part of extension.");
				return ERR_FAIL;
				}

			//	Make sure we override a base type

			else if (m_Base.FindByUNID(dwUNID) == NULL)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Invalid UNID: %x [does not match extension UNID or override base type]"), dwUNID);
				return ERR_FAIL;
				}
			}
		}

	//	Otherwise, add to the base design types

	else
		pTable = &m_Base;

	//	Add

	if (error = pTable->AddEntry(pEntry))
		{
		if (pTable->FindByUNID(dwUNID))
			Ctx.sError = strPatternSubst(CONSTLIT("Duplicate UNID: %x"), dwUNID);
		else
			Ctx.sError = strPatternSubst(CONSTLIT("Error adding design entry UNID: %x"), dwUNID);
		return error;
		}

	return NOERROR;
	}

ALERROR CDesignCollection::AddExtension (SDesignLoadCtx &Ctx, EExtensionTypes iType, DWORD dwUNID, bool bDefaultResource, SExtensionDesc **retpExtension)

//	AddExtension
//
//	Adds an extension entry

	{
	ASSERT(!Ctx.sResDb.IsBlank());
	ASSERT(dwUNID != 0);

	bool bAdded;
	SExtensionDesc **pSlot = m_Extensions.Insert(dwUNID, &bAdded);
	if (!bAdded)
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Duplicate extension: %x"), dwUNID);
		return ERR_FAIL;
		}

	SExtensionDesc *pEntry = new SExtensionDesc;
	*pSlot = pEntry;

	pEntry->sResDb = Ctx.sResDb;
	pEntry->dwUNID = dwUNID;
	pEntry->iType = iType;
	pEntry->bDefaultResource = bDefaultResource;

	//	Defaults
	pEntry->dwVersion = EXTENSION_VERSION;	//	Latest version
	pEntry->bRegistered = false;
	pEntry->bEnabled = false;
	pEntry->bLoaded = false;
	pEntry->bDebugOnly = false;

	//	Done

	*retpExtension = pEntry;
	return NOERROR;
	}

ALERROR CDesignCollection::AddSystemTable (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	AddSystemTable
//
//	Add a system table as a design element
//	(We need this for backwards compatibility)

	{
	CSystemTable *pTable = new CSystemTable;
	pTable->InitFromXML(Ctx, pDesc);
	if (pTable->GetUNID() == 0)
		pTable->SetUNID(DEFAULT_SYSTEM_TABLE_UNID);

	return AddEntry(Ctx, pTable);
	}

ALERROR CDesignCollection::BeginLoadAdventure (SDesignLoadCtx &Ctx, SExtensionDesc *pExtension)

//	BeginLoadAdventure
//
//	Begin loading an adventure (not just the desc)

	{
	ASSERT(pExtension);

	pExtension->bLoaded = true;

	//	Set context

	Ctx.pExtension = pExtension;

	return NOERROR;
	}

ALERROR CDesignCollection::BeginLoadAdventureDesc (SDesignLoadCtx &Ctx, CXMLElement *pDesc, bool bDefaultResource)

//	BeginLoadAdventureDesc
//
//	Adds a new adventure extension to our internal list and sets Ctx.pExtension

	{
	ALERROR error;
	SExtensionDesc *pEntry;

	//	Load the structure

	if (error = LoadExtensionDesc(Ctx, pDesc, bDefaultResource, &pEntry))
		return error;

	pEntry->iType = extAdventure;
	pEntry->bEnabled = false;
	pEntry->bLoaded = false;

	//	Set context

	Ctx.pExtension = pEntry;
	Ctx.bLoadAdventureDesc = true;

	return NOERROR;
	}

ALERROR CDesignCollection::BeginLoadExtension (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	BeginLoadExtension
//
//	Adds a new extension to our internal list and sets Ctx.pExtension

	{
	ALERROR error;
	SExtensionDesc *pEntry;

	//	Load the structure

	if (error = LoadExtensionDesc(Ctx, pDesc, false, &pEntry))
		return error;

	pEntry->iType = extExtension;
	pEntry->bEnabled = true;
	pEntry->bLoaded = true;

	//	Set context

	Ctx.pExtension = pEntry;

	return NOERROR;
	}

ALERROR CDesignCollection::BindDesign (SDesignLoadCtx &Ctx)

//	BindDesign
//
//	Bind the design collection so that design types point the appropriate
//	pointers by UNID

	{
	ALERROR error;
	int i, j, k;

	//	Unbind everything

	for (i = 0; i < m_AllTypes.GetCount(); i++)
		m_AllTypes.GetEntry(i)->UnbindDesign();

	//	Reset the bind tables

	m_AllTypes.RemoveAll();
	for (i = 0; i < designCount; i++)
		m_ByType[i].DeleteAll();

	//	We start with all the base types

	for (i = 0; i < m_Base.GetCount(); i++)
		m_AllTypes.AddEntry(m_Base.GetEntry(i));

	//	Start with base topology

	m_pTopology = &m_BaseTopology;
	m_pAdventureExtension = NULL;

	//	Now add all enabled extensions

	for (i = 0; i < GetExtensionCount(); i++)
		{
		SExtensionDesc *pExtension = GetExtension(i);

		if (pExtension->bEnabled)
			{
			//	Add design elements in extension

			for (j = 0; j < pExtension->Table.GetCount(); j++)
				{
				CDesignType *pEntry = pExtension->Table.GetEntry(j);
				m_AllTypes.AddOrReplaceEntry(pEntry);
				}

			//	Handle adventure extensions

			if (pExtension->iType == extAdventure)
				{
				//	Keep track of extension

				m_pAdventureExtension = pExtension;

				//	Add topology

				m_pTopology = &pExtension->Topology;
				}
			}
		else
			{
			if (pExtension->iType == extAdventure)
				{
				DWORD dwCoverImage = 0;

				//	Adventure desc elements are added even if not enabled

				for (j = 0; j < pExtension->Table.GetCount(); j++)
					{
					CDesignType *pEntry = pExtension->Table.GetEntry(j);
					if (pEntry->GetType() == designAdventureDesc)
						{
						m_AllTypes.AddOrReplaceEntry(pEntry);

						//	Get the cover image used by the adventure, because
						//	we need to load that too.

						CAdventureDesc *pDesc = CAdventureDesc::AsType(pEntry);
						dwCoverImage = pDesc->GetBackgroundUNID();
						}
					}

				//	Make sure we load the cover image

				if (dwCoverImage)
					{
					for (j = 0; j < pExtension->Table.GetCount(); j++)
						{
						CDesignType *pEntry = pExtension->Table.GetEntry(j);
						if (pEntry->GetUNID() == dwCoverImage)
							m_AllTypes.AddOrReplaceEntry(pEntry);
						}
					}
				}
			}
		}

	//	Initialize the byType lists

	for (i = 0; i < m_AllTypes.GetCount(); i++)
		{
		CDesignType *pEntry = m_AllTypes.GetEntry(i);
		m_ByType[pEntry->GetType()].AddEntry(pEntry);
		}

	//	Set our adventure desc as current; since adventure descs are always 
	//	loaded this is the only thing that we can use to tell if we should
	//	call global events.
	//
	//	This must happen after Unbind (because that clears it) and before
	//	PrepareBindDesign.
	//
	//	NOTE: m_pAdventureDesc can be NULL (e.g., in the intro screen).

	if (m_pAdventureDesc)
		m_pAdventureDesc->SetCurrentAdventure();

	//	Cache a map between currency name and economy type
	//	We need to do this before Bind because some types will lookup
	//	a currency name during Bind.

	m_EconomyIndex.DeleteAll();
	for (i = 0; i < GetCount(designEconomyType); i++)
		{
		CEconomyType *pEcon = CEconomyType::AsType(GetEntry(designEconomyType, i));
		const CString &sName = pEcon->GetSID();

		bool bUnique;
		CEconomyType **ppDest = m_EconomyIndex.Insert(sName, &bUnique);
		if (!bUnique)
			return pEcon->ComposeLoadError(Ctx, CONSTLIT("Currency ID must be unique"));

		*ppDest = pEcon;
		}

	//	Prepare to bind. This is used by design elements
	//	that need two passes to bind.

	for (i = 0; i < m_AllTypes.GetCount(); i++)
		{
		CDesignType *pEntry = m_AllTypes.GetEntry(i);
		if (error = pEntry->PrepareBindDesign(Ctx))
			return error;
		}

	//	Now call Bind on all active design entries

	for (i = 0; i < evtCount; i++)
		m_EventsCache[i]->DeleteAll();

	for (i = 0; i < m_AllTypes.GetCount(); i++)
		{
		CDesignType *pEntry = m_AllTypes.GetEntry(i);
		if (error = pEntry->BindDesign(Ctx))
			return error;

		//	Cache some global events

		const CEventHandler &Events = pEntry->GetEventHandlers();
		for (j = 0; j < Events.GetCount(); j++)
			{
			ICCItem *pCode;
			CString sEvent = Events.GetEvent(j, &pCode);

			for (k = 0; k < evtCount; k++)
				if (m_EventsCache[k]->Insert(pEntry, sEvent, pCode))
					break;
			}
		}

	//	Finish binding. This pass is used by design elements
	//	that need to do stuff after all designs are bound.

	for (i = 0; i < m_AllTypes.GetCount(); i++)
		{
		CDesignType *pEntry = m_AllTypes.GetEntry(i);
		if (error = pEntry->FinishBindDesign(Ctx))
			return error;
		}

	return NOERROR;
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

CAdventureDesc *CDesignCollection::FindAdventureForExtension (DWORD dwUNID)

//	FindAdventureForExtension
//
//	Returns the adventure desc that belongs to the given extension.
//	(Or NULL if not found).

	{
	int i;

	SExtensionDesc *pExtension = FindExtension(dwUNID);
	if (pExtension == NULL || pExtension->iType != extAdventure)
		return NULL;

	for (i = 0; i < pExtension->Table.GetCount(); i++)
		{
		CDesignType *pType = pExtension->Table.GetEntry(i);
		if (pType->GetType() ==	designAdventureDesc)
			return CAdventureDesc::AsType(pType);
		}

	return NULL;
	}

SExtensionDesc *CDesignCollection::FindExtension (DWORD dwUNID)

//	FindExtension
//
//	Find the entry for the given extension

	{
	SExtensionDesc **pFind = m_Extensions.GetAt(dwUNID);
	return (pFind ? *pFind : NULL);
	}

CXMLElement *CDesignCollection::FindSystemFragment (const CString &sName) const

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
				return pFragment;
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
		ICCItem *pCode;
		CDesignType *pType = m_EventsCache[evtGetGlobalAchievements]->GetEntry(i, &pCode);

		pType->FireGetGlobalAchievements(Stats);
		}
	}

bool CDesignCollection::FireGetGlobalDockScreen (CSpaceObject *pObj, CString *retsScreen, int *retiPriority)

//	FireGetGlobalDockScreen
//
//	Allows types to override the dock screen for an object

	{
	int i;
	int iBestPriority = -1;
	CString sBestScreen;

	//	Loop over all types and get the highest priority screen

	for (i = 0; i < m_EventsCache[evtGetGlobalDockScreen]->GetCount(); i++)
		{
		ICCItem *pCode;
		CDesignType *pType = m_EventsCache[evtGetGlobalDockScreen]->GetEntry(i, &pCode);

		int iPriority;
		CString sScreen;
		if (pType->FireGetGlobalDockScreen(pCode, pObj, &sScreen, &iPriority)
				&& iPriority > iBestPriority)
			{
			iBestPriority = iPriority;
			sBestScreen = sScreen;
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

	return true;
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
		ICCItem *pCode;
		CDesignType *pType = m_EventsCache[evtOnGlobalObjDestroyed]->GetEntry(i, &pCode);

		pType->FireOnGlobalObjDestroyed(pCode, Ctx);
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
		ICCItem *pCode;
		CDesignType *pType = m_EventsCache[evtOnGlobalDockPaneInit]->GetEntry(i, &pCode);

		if (pType->FireOnGlobalDockPaneInit(pCode,
				pScreen,
				dwRootUNID,
				sScreenUNID,
				sPane,
				&sError) != NOERROR)
			kernelDebugLogMessage("%s", sError.GetASCIIZPointer());
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
			kernelDebugLogMessage("%s", sError.GetASCIIZPointer());
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
			kernelDebugLogMessage("%s", sError.GetASCIIZPointer());
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
			kernelDebugLogMessage("%s", sError.GetASCIIZPointer());
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
			kernelDebugLogMessage("%s", sError.GetASCIIZPointer());
		}
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
		ICCItem *pCode;
		CDesignType *pType = m_EventsCache[evtOnGlobalUniverseCreated]->GetEntry(i, &pCode);

		pType->FireOnGlobalUniverseCreated(pCode);
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
		ICCItem *pCode;
		CDesignType *pType = m_EventsCache[evtOnGlobalUniverseLoad]->GetEntry(i, &pCode);

		pType->FireOnGlobalUniverseLoad(pCode);
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
		ICCItem *pCode;
		CDesignType *pType = m_EventsCache[evtOnGlobalUniverseSave]->GetEntry(i, &pCode);

		pType->FireOnGlobalUniverseSave(pCode);
		}
	}

void CDesignCollection::GetEnabledExtensions (TArray<DWORD> *retExtensionList)

//	GetEnabledExtensions
//
//	Returns the list of enabled extensions

	{
	int i;

	retExtensionList->DeleteAll();

	for (i = 0; i < GetExtensionCount(); i++)
		{
		SExtensionDesc *pEntry = GetExtension(i);
		if (pEntry->iType == extExtension && pEntry->bEnabled)
			retExtensionList->Insert(pEntry->dwUNID);
		}
	}

CG16bitImage *CDesignCollection::GetImage (DWORD dwUNID, bool bCopy)

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

	if (bCopy)
		return pImage->CreateCopy();
	else
		return pImage->GetImage();
	}

bool CDesignCollection::IsAdventureExtensionBound (DWORD dwUNID)

//	IsAdventureExtensionBound
//
//	Returns TRUE if we have bound on the given adventure extension

	{
	if (m_pAdventureExtension)
		return (m_pAdventureExtension->dwUNID == dwUNID);
	else
		return (dwUNID == 0);
	}

bool CDesignCollection::IsAdventureExtensionLoaded (DWORD dwUNID)

//	IsAdventureExtensionLoaded
//
//	Returns TRUE if adventure extension is loaded

	{
	//	0 = no extension
	if (dwUNID == 0)
		return true;

	SExtensionDesc *pExt = FindExtension(dwUNID);
	if (pExt == NULL)
		{
		ASSERT(false);
		return false;
		}

	ASSERT(pExt->iType == extAdventure);
	return pExt->bLoaded;
	}

bool CDesignCollection::IsExtensionCompatibleWithAdventure (SExtensionDesc *pExtension, CAdventureDesc *pAdventure)

//	IsExtensionCompatibleWithAdventure
//
//	Returns TRUE if the given extension works with the given adventure
//	(Some extensions are marked to only work with particular adventures).

	{
	int i;

	//	Some extensions don't care

	if (pExtension->Extends.GetCount() == 0)
		return true;

	//	Otherwise, see if the adventure is explicitly listed

	DWORD dwAdventureUNID = pAdventure->GetExtensionUNID();
	for (i = 0; i < pExtension->Extends.GetCount(); i++)
		if (pExtension->Extends[i] == dwAdventureUNID)
			return true;

	//	Otherwise, not compatible

	return false;
	}

ALERROR CDesignCollection::LoadAdventureDescMainRes (SDesignLoadCtx &Ctx, CAdventureDesc *pAdventure)

//	LoadAdventureDescMainRes
//
//	Loads an adventure desc from the main resource file.
//	Adds a new adventure extension to our internal list and sets Ctx.pExtension

	{
	ALERROR error;

	//	Do nothing if the adventure doesn't have an extension UNID. This happens in backwards-compatibility
	//	cases when people have replaced Transcendence.xml

	if (pAdventure->GetExtensionUNID() == 0)
		return NOERROR;

	//	Create an extension descriptor for this adventure

	SExtensionDesc *pEntry;
	if (error = AddExtension(Ctx, extAdventure, pAdventure->GetExtensionUNID(), true, &pEntry))
		return error;

	pEntry->bRegistered = true;				//	Since it is included in main resources, it counts as registered

	//	Load name

	pEntry->sName = pAdventure->GetName();
	if (pEntry->sName.IsBlank())
		pEntry->sName = strPatternSubst(CONSTLIT("Extension %x"), pAdventure->GetExtensionUNID());

	return NOERROR;
	}

ALERROR CDesignCollection::LoadDesignType (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadDesignType
//
//	Loads a standard design type

	{
	ALERROR error;
	CDesignType *pEntry;

	try
		{
		if (error = CDesignType::CreateFromXML(Ctx, pDesc, &pEntry))
			return error;
		}
	catch (...)
		{
		char szBuffer[1024];
		wsprintf(szBuffer, "Crash loading: %x", pDesc->GetAttributeInteger(CONSTLIT("UNID")));
		Ctx.sError = CString(szBuffer);
		return ERR_FAIL;
		}

	if (error = AddEntry(Ctx, pEntry))
		{
		delete pEntry;
		return error;
		}

	return NOERROR;
	}

ALERROR CDesignCollection::LoadEntryFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CDesignType **retpType)

//	LoadEntryFromXML
//
//	Load an entry into the collection

	{
	ALERROR error;

	//	Load topology

	if (strEquals(pDesc->GetTag(), STAR_SYSTEM_TOPOLOGY_TAG)
			|| strEquals(pDesc->GetTag(), SYSTEM_TOPOLOGY_TAG))
		{
		if (Ctx.pExtension)
			//	If we're in an extension (Adventure) then load into the extension
			error = Ctx.pExtension->Topology.LoadFromXML(Ctx, pDesc, NULL_STR);
		else
			//	Otherwise, load into the base game
			error = m_BaseTopology.LoadFromXML(Ctx, pDesc, NULL_STR);

		if (error)
			return error;

		//	There is no type for topologies

		if (retpType)
			*retpType = NULL;
		}

	//	Load standard design elements

	else
		{
		CDesignType *pEntry;

		try
			{
			if (error = CDesignType::CreateFromXML(Ctx, pDesc, &pEntry))
				return error;
			}
		catch (...)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Crash loading: %x"), pDesc->GetAttributeInteger(CONSTLIT("UNID")));
			return ERR_FAIL;
			}

		if (error = AddEntry(Ctx, pEntry))
			{
			delete pEntry;
			return error;
			}

		if (retpType)
			*retpType = pEntry;
		}

	return NOERROR;
	}

ALERROR CDesignCollection::LoadExtensionDesc (SDesignLoadCtx &Ctx, CXMLElement *pDesc, bool bDefaultResource, SExtensionDesc **retpExtension)

//	LoadExtensionDesc
//
//	Loads a new extension descriptor

	{
	ALERROR error;
	int i;

	//	Load version

	DWORD dwVersion = ::LoadExtensionVersion(pDesc->GetAttribute(VERSION_ATTRIB));
	if (dwVersion == 0)
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unable to load extension: incompatible version: %s"), pDesc->GetAttribute(VERSION_ATTRIB));
		return ERR_FAIL;
		}

	//	Load UNID

	DWORD dwUNID = pDesc->GetAttributeInteger(UNID_ATTRIB);

	//	See if this is registered
	//	LATER: For now, this is hard-coded

	bool bIsRegistered = (dwUNID == 0x00300000) 
			|| (dwUNID == 0x00200000)
			|| ((dwUNID & 0xF0000000) == 0xA0000000);

	//	Make sure that unregistered extensions are in the correct ranage

	if (!bIsRegistered)
		{
		DWORD dwDomain = (dwUNID & 0xF0000000);
		if (dwDomain < 0xA0000000 || dwDomain > 0xEFFFFFFF)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Invalid extension UNID: %x"), dwUNID);
			return ERR_FAIL;
			}
		}

	//	Create structure

	SExtensionDesc *pEntry;
	if (error = AddExtension(Ctx, extExtension, dwUNID, bDefaultResource, &pEntry))
		return error;

	pEntry->dwVersion = dwVersion;
	pEntry->bRegistered = bIsRegistered;
	pEntry->bEnabled = true;
	pEntry->bDebugOnly = pDesc->GetAttributeBool(DEBUG_ONLY_ATTRIB);

	//	Load name

	pEntry->sName = pDesc->GetAttribute(NAME_ATTRIB);
	if (pEntry->sName.IsBlank())
		pEntry->sName = strPatternSubst(CONSTLIT("Extension %x"), dwUNID);

	//	Load credits (we parse them into a string array)

	CString sCredits = pDesc->GetAttribute(CREDITS_ATTRIB);
	if (!sCredits.IsBlank())
		strDelimitEx(sCredits, ';', DELIMIT_TRIM_WHITESPACE, 0, &pEntry->Credits);

	//	Load extends attrib

	CString sExtends = pDesc->GetAttribute(EXTENDS_ATTRIB);
	if (!sExtends.IsBlank())
		{
		TArray<CString> Extends;
		strDelimitEx(sExtends, ';', DELIMIT_TRIM_WHITESPACE, 0, &Extends);
		for (i = 0; i < Extends.GetCount(); i++)
			{
			DWORD dwUNID = strToInt(Extends[i], INVALID_UNID);
			if (dwUNID != INVALID_UNID)
				pEntry->Extends.Insert(dwUNID);
			}
		}

	//	If we're an extension in the default resource, load modules

	if (bDefaultResource)
		{
		CXMLElement *pModules = pDesc->GetContentElementByTag(MODULES_TAG);
		if (pModules)
			{
			for (i = 0; i < pModules->GetContentElementCount(); i++)
				{
				CXMLElement *pModule = pModules->GetContentElement(i);
				CString sModule = pModule->GetAttribute(FILENAME_ATTRIB);
				if (!sModule.IsBlank())
					pEntry->Modules.Insert(sModule);
				}
			}
		}

	//	Done

	*retpExtension = pEntry;

	return NOERROR;
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

void CDesignCollection::RemoveAll (void)

//	RemoveAll
//
//	Remove all entries

	{
	int i;

	//	Some classes need to clean up global data
	//	(But we need to do this before we destroy the types)

	CStationType::Reinit();

	//	Delete the base types

	m_Base.DeleteAll();

	//	Delete all extensions

	for (i = 0; i < GetExtensionCount(); i++)
		{
		SExtensionDesc *pEntry = GetExtension(i);
		pEntry->Table.DeleteAll();
		delete pEntry;
		}

	m_Extensions.DeleteAll();
	}

void CDesignCollection::SelectAdventure (CAdventureDesc *pAdventure)

//	SelectAdventure
//
//	Enable the given adventure and disable all other adventures

	{
	int i;

	m_pAdventureDesc = pAdventure;
	DWORD dwUNID = pAdventure->GetExtensionUNID();

	for (i = 0; i < GetExtensionCount(); i++)
		{
		SExtensionDesc *pEntry = GetExtension(i);
		if (pEntry->iType == extAdventure)
			pEntry->bEnabled = (pEntry->dwUNID == dwUNID);
		}
	}

ALERROR CDesignCollection::SelectExtensions (CAdventureDesc *pAdventure, TArray<DWORD> *pExtensionList, bool *retbBindNeeded, CString *retsError)

//	SelectExtensions
//
//	Enables all extensions in pExtensionList and disables all others
//	(if pExtensionList == NULL then we enable all extensions).
//
//	Returns an error if an extension on the list could not be found.

	{
	int i;
	bool bBindNeeded = false;

	TArray<bool> OldState;
	OldState.InsertEmpty(GetExtensionCount());

	//	Disable all extensions

	for (i = 0; i < GetExtensionCount(); i++)
		{
		SExtensionDesc *pEntry = GetExtension(i);
		if (pEntry->iType == extExtension)
			{
			OldState[i] = pEntry->bEnabled;
			pEntry->bEnabled = false;
			}
		}

	//	Enable all extensions in the list

	if (pExtensionList)
		{
		for (i = 0; i < pExtensionList->GetCount(); i++)
			{
			SExtensionDesc *pEntry = FindExtension(pExtensionList->GetAt(i));
			if (pEntry == NULL || pEntry->iType == extAdventure)
				{
				if (retsError)
					*retsError = strPatternSubst(CONSTLIT("Unable to find extension: %x"), pExtensionList->GetAt(i));
				return ERR_NOTFOUND;
				}

			if (pEntry->iType == extExtension
					&& IsExtensionCompatibleWithAdventure(pEntry, pAdventure))
				pEntry->bEnabled = true;
			}
		}
	else
		{
		//	Enable all extensions

		for (i = 0; i < GetExtensionCount(); i++)
			{
			SExtensionDesc *pEntry = GetExtension(i);
			if (pEntry->iType == extExtension 
					&& IsExtensionCompatibleWithAdventure(pEntry, pAdventure)
					&& (!pEntry->bDebugOnly || g_pUniverse->InDebugMode()))
				pEntry->bEnabled = true;
			}
		}

	//	See if we made any changes

	for (i = 0; i < GetExtensionCount(); i++)
		{
		SExtensionDesc *pEntry = GetExtension(i);
		if (pEntry->iType == extExtension && pEntry->bEnabled != OldState[i])
			{
			bBindNeeded = true;
			break;
			}
		}

	//	Done

	if (retbBindNeeded)
		*retbBindNeeded = bBindNeeded;

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

//	CDesignTable --------------------------------------------------------------

ALERROR CDesignTable::AddEntry (CDesignType *pEntry)

//	AddEntry
//
//	Adds an entry to the table

	{
	m_Table.Insert(pEntry->GetUNID(), pEntry);
	return NOERROR;
	}

ALERROR CDesignTable::AddOrReplaceEntry (CDesignType *pEntry, CDesignType **retpOldEntry)

//	AddOrReplaceEntry
//
//	Adds or replaces an entry

	{
	bool bAdded;
	CDesignType **pSlot = m_Table.Insert(pEntry->GetUNID(), &bAdded);

	if (retpOldEntry)
		*retpOldEntry = (!bAdded ? *pSlot : NULL);

	*pSlot = pEntry;

	return NOERROR;
	}

void CDesignTable::DeleteAll (void)

//	DeleteAll
//
//	Removes all entries and deletes the object that they point to

	{
	int i;

	for (i = 0; i < GetCount(); i++)
		delete GetEntry(i);

	m_Table.DeleteAll();
	}

CDesignType *CDesignTable::FindByUNID (DWORD dwUNID) const

//	FindByUNID
//
//	Returns a pointer to the given entry or NULL

	{
	CDesignType **pObj = m_Table.GetAt(dwUNID);
	return (pObj ? *pObj : NULL);
	}

