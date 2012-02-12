//	XMLLoader.cpp
//
//	Initializes the universe from XML
//

#include "PreComp.h"

#ifdef DEBUG
//#define DEBUG_TIME_IMAGE_LOAD
#endif

#define ADVENTURE_DESC_TAG					CONSTLIT("AdventureDesc")
#define ENCOUNTER_TABLES_TAG				CONSTLIT("EncounterTables")
#define TABLES_TAG							CONSTLIT("Tables")
#define POWERS_TAG							CONSTLIT("Powers")
#define SOUNDS_TAG							CONSTLIT("Sounds")
#define GLOBALS_TAG							CONSTLIT("Globals")
#define SPACE_ENVIRONMENT_TYPE_TAG			CONSTLIT("SpaceEnvironmentType")
#define MODULES_TAG							CONSTLIT("Modules")
#define ITEM_TABLE_TAG						CONSTLIT("ItemTable")
#define DOCK_SCREEN_TAG						(CONSTLIT("DockScreen"))
#define ITEM_TYPE_TAG						(CONSTLIT("ItemType"))
#define SHIP_CLASS_TAG						(CONSTLIT("ShipClass"))
#define IMAGE_TAG							(CONSTLIT("Image"))
#define SOUND_TAG							(CONSTLIT("Sound"))
#define STATION_TYPE_TAG					(CONSTLIT("StationType"))
#define SYSTEM_TYPE_TAG						(CONSTLIT("SystemType"))
#define SOVEREIGN_TAG						(CONSTLIT("Sovereign"))
#define SOVEREIGNS_TAG						(CONSTLIT("Sovereigns"))
#define IMAGES_TAG							CONSTLIT("Images")
#define STAR_SYSTEM_TYPES_TAG				CONSTLIT("SystemTypes")
#define SYSTEM_NODE_TAG						CONSTLIT("SystemNode")
#define POWER_TAG							CONSTLIT("Power")
#define TRANSCENDENCE_ADVENTURE_TAG			CONSTLIT("TranscendenceAdventure")
#define TRANSCENDENCE_MODULE_TAG			CONSTLIT("TranscendenceModule")

#define ID_ATTRIB							CONSTLIT("ID")
#define UNID_ATTRIB							CONSTLIT("UNID")
#define FOLDER_ATTRIB						CONSTLIT("folder")
#define FILENAME_ATTRIB						CONSTLIT("filename")
#define MIN_VERSION_ATTRIB					CONSTLIT("minVersion")
#define VERSION_ATTRIB						CONSTLIT("version")

#define FILE_TYPE_XML						CONSTLIT("xml")
#define FILE_TYPE_TDB						CONSTLIT("tdb")

static char g_ShipClassTag[] = "ShipClasses";
static char g_StationTypesTag[] = "StationTypes";
static char g_ArmorClassesTag[] = "ArmorClasses";
static char g_DockScreensTag[] = "DockScreens";
static char g_ItemTypesTag[] = "ItemTypes";

static char g_BitmapAttrib[] = "bitmap";
static char g_BitmaskAttrib[] = "bitmask";
static char g_TransColorAttrib[] = "backColor";

ALERROR CUniverse::BindDesign (SDesignLoadCtx &Ctx)

//	BindDesign
//
//	Binds the design

	{
	ALERROR error;

	//	Done loading design elements

	if (error = m_Design.BindDesign(Ctx))
		return error;

	//	Initialize some secondary structures

	InitLevelEncounterTables();

	return NOERROR;
	}

ALERROR CUniverse::Init (const CString &sFilespec, CString *retsError, DWORD dwFlags)
	{
	ALERROR error;

	CResourceDb Resources(sFilespec);
	if (error = Resources.Open(DFOPEN_FLAG_READ_ONLY))
		{
		*retsError = CONSTLIT("Unable to initialize Transcendence");
		return error;
		}

	return Init(Resources, retsError, dwFlags);
	}

ALERROR CUniverse::Init (CResourceDb &Resources, CString *retsError, DWORD dwFlags)

//	Init
//
//	Initializes either from an XML file or from a TDB

	{
	ALERROR error;

	if (Resources.IsUsingExternalGameFile())
		kernelDebugLogMessage("Using external Transcendence.xml");

	if (Resources.IsUsingExternalResources())
		kernelDebugLogMessage("Using external resource files");

	CXMLElement *pGameFile;
	if (error = Resources.LoadGameFile(&pGameFile, retsError))
		return error;

	SDesignLoadCtx Ctx;
	Ctx.sResDb = Resources.GetFilespec();
	Ctx.pResDb = &Resources;
	Ctx.bNoResources = ((dwFlags & flagNoResources) ? true : false);
	Ctx.bNoVersionCheck = ((dwFlags & flagNoVersionCheck) ? true : false);

	//	Remember the resource Db for deferred loads

	m_sResourceDb = Resources.GetFilespec();

	//	Initialize the universe

	if (error = InitFromXML(Ctx, pGameFile, Resources))
		goto Fail;

	//	Load extensions

	if (!m_sResourceDb.IsBlank())
		if (error = InitExtensions(Ctx, m_sResourceDb))
			goto Fail;

	//	Done loading design elements

	if (error = BindDesign(Ctx))
		goto Fail;

	//	Done

	delete pGameFile;
	return NOERROR;

Fail:

	*retsError = Ctx.sError;
	return error;
	}

ALERROR CUniverse::InitAdventure (DWORD dwAdventureUNID, TArray<DWORD> *pExtensionList, CString *retsError, DWORD dwFlags)

//	InitAdventure
//
//	Loads and selects the given adventure
//
//	pExtensionList is the list of extensions that should be loaded.
//	(If NULL, all valid extensions should be loaded).

	{
	ALERROR error;
	bool bBindNeeded;

	//	Get the adventure

	CAdventureDesc *pAdventure = FindAdventureDesc(dwAdventureUNID);
	if (pAdventure == NULL)
		{
		//	See if this is an adventure extension

		pAdventure = m_Design.FindAdventureForExtension(dwAdventureUNID);
		if (pAdventure == NULL)
			{
			*retsError = strPatternSubst(CONSTLIT("Unable to find adventure desc: %x"), dwAdventureUNID);
			return ERR_FAIL;
			}
		}

	//	Figure out which extensions need to be enabled.

	if (error = m_Design.SelectExtensions(pAdventure, pExtensionList, &bBindNeeded, retsError))
		return error;

	//	If we're not yet bound to this extension, then bind

	if (!m_Design.IsAdventureExtensionBound(pAdventure->GetExtensionUNID())
			|| bBindNeeded)
		{
		//	Initialize resources

		CResourceDb Resources(m_sResourceDb);
		if (error = Resources.Open(DFOPEN_FLAG_READ_ONLY))
			return error;

		if (error = Resources.LoadEntities(retsError))
			return error;

		SDesignLoadCtx Ctx;
		Ctx.sResDb = m_sResourceDb;
		Ctx.pResDb = &Resources;
		Ctx.bNoResources = ((dwFlags & flagNoResources) ? true : false);
		Ctx.bNoVersionCheck = ((dwFlags & flagNoVersionCheck) ? true : false);

		//	If we have an adventure then load it and select it

		if (pAdventure)
			{
			//	Load the adventure

			if (error = LoadAdventure(Ctx, pAdventure))
				{
				*retsError = Ctx.sError;
				return error;
				}

			//	Select it so that it is the only one enabled

			m_Design.SelectAdventure(pAdventure);
			}

		//	Bind the design

		if (error = BindDesign(Ctx))
			{
			*retsError = Ctx.sError;
			return error;
			}
		}

	//	Set the current adventure

	SetCurrentAdventureDesc(pAdventure);

	return NOERROR;
	}

ALERROR CUniverse::InitFromXML (SDesignLoadCtx &Ctx,
								CXMLElement *pElement, 
								CResourceDb &Resources)

//	InitFromXML
//
//	Initializes the universe from an XML database

	{
	ALERROR error;
	int i;
	CIDTable UNIDMap(FALSE, FALSE);

	m_bNoImages = Ctx.bNoResources;

	//	Initialize code chain

	if (error = m_CC.Boot())
		return error;

	if (error = InitCodeChainPrimitives())
		return error;

	//	Create some fonts

	HFONT hFont = ::CreateFont(-12,
			0,
			0,
			0,
			FW_NORMAL,
			false,
			false,
			false,
			ANSI_CHARSET,
			OUT_TT_ONLY_PRECIS,
			CLIP_DEFAULT_PRECIS,
			ANTIALIASED_QUALITY,
			FF_SWISS,
			"Tahoma");
	m_MapLabelFont.CreateFromFont(hFont);
	::DeleteObject(hFont);

	hFont = ::CreateFont(-11,
			0,
			0,
			0,
			FW_BOLD,
			false,
			false,
			false,
			ANSI_CHARSET,
			OUT_TT_ONLY_PRECIS,
			CLIP_DEFAULT_PRECIS,
			ANTIALIASED_QUALITY,
			FF_SWISS,
			"Tahoma");
	m_SignFont.CreateFromFont(hFont);
	::DeleteObject(hFont);

	//	Make sure we have the right version

	if (!Ctx.bNoVersionCheck)
		{
		DWORD dwVersion;

		//	Make sure we have the latest version of the .XML

		dwVersion = (DWORD)pElement->GetAttributeInteger(VERSION_ATTRIB);
		if (fileGetProductVersion() > dwVersion)
			{
			if (Resources.IsUsingExternalGameFile())
				Ctx.sError = CONSTLIT("External definitions file (Transcendence.xml) is obsolete.\nPlease remove 'Transcendence.xml' file from game folder.");
			else
				Ctx.sError = CONSTLIT("Source definitions file (Transcendence.xml) is obsolete.\nPlease download the latest version at http://www.neurohack.com/transcendence/Downloads.html.");
			return ERR_FAIL;
			}

		//	Make sure we have the latest version of the .EXE

		dwVersion = (DWORD)pElement->GetAttributeInteger(MIN_VERSION_ATTRIB);
		if (fileGetProductVersion() < dwVersion)
			{
			Ctx.sError = CONSTLIT("Source definitions file (Transcendence.xml) requires a newer version of Transcendence.exe.\nPlease download the latest version at http://www.neurohack.com/transcendence/Downloads.html.");
			return ERR_FAIL;
			}
		}

	//	Load the Main XML file

	for (i = 0; i < pElement->GetContentElementCount(); i++)
		{
		CXMLElement *pDesc = pElement->GetContentElement(i);

		if (strEquals(pDesc->GetTag(), IMAGES_TAG))
			error = InitImages(Ctx, pDesc, Resources);
		else if (strEquals(pDesc->GetTag(), SOUNDS_TAG))
			error = InitSounds(Ctx, pDesc, Resources);
		else if (strEquals(pDesc->GetTag(), STAR_SYSTEM_TYPES_TAG))
			error = InitStarSystemTypes(Ctx, pDesc);
		else
			error = LoadDesignElement(Ctx, pDesc);

		//	Check for error

		if (error)
			return error;
		}

	return NOERROR;
	}

ALERROR CUniverse::InitImages (SDesignLoadCtx &Ctx, CXMLElement *pImages, CResourceDb &Resources)

//	InitImages
//
//	Loads all images

	{
	ALERROR error;
	HBITMAP hDIB = NULL;
	HBITMAP hBitmask = NULL;

	//	Nothing to do if we don't want to load resources

#ifdef NO_RESOURCES
	if (Ctx.bNoResources)
		return NOERROR;
#endif

	//	Figure out if we've got a special folder for the images

	Ctx.sFolder = pImages->GetAttribute(FOLDER_ATTRIB);

#ifdef DEBUG_TIME_IMAGE_LOAD
	CTimeDate StartTime(CTimeDate::Now);
#endif

	//	Load all images

	for (int i = 0; i < pImages->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pImages->GetContentElement(i);

		if (error = m_Design.LoadEntryFromXML(Ctx, pItem))
			return error;
		}

	//	Restore folder

	Ctx.sFolder = NULL_STR;

#ifdef DEBUG_TIME_IMAGE_LOAD
	{
	CTimeDate StopTime(CTimeDate::Now);
	CTimeSpan Timing = timeSpan(StartTime, StopTime);
	CString sTime = Timing.Format(CString());
	::OutputDebugString("Time to load images: ");
	::OutputDebugString(sTime.GetASCIIZPointer());
	::OutputDebugString("\n");
	}
#endif

	return NOERROR;
	}

ALERROR CUniverse::InitLevelEncounterTables (void)

//	InitLevelEncounterTables
//
//	Initializes the m_LevelEncounterTables array based on the encounter
//	tables of all the stations for each level.

	{
	m_LevelEncounterTables.RemoveAll();

	for (int i = 1; i <= MAX_ITEM_LEVEL; i++)
		{
		CStructArray *pTable = new CStructArray(sizeof(SLevelEncounter), 8);

		for (int j = 0; j < GetStationTypeCount(); j++)
			{
			CStationType *pType = GetStationType(j);

			//	Figure out the frequency of an encounter from this station based
			//	on the frequency of the station at this level and the frequency
			//	of encounters for this station.

			int iEncounterFreq = pType->GetEncounterFrequency();
			int iStationFreq = pType->GetFrequencyByLevel(i);
			int iFreq = iEncounterFreq * iStationFreq / ftCommon;

			//	Add to the table

			if (iFreq > 0)
				{
				pTable->ExpandArray(pTable->GetCount(), 1);
				SLevelEncounter *pEntry = (SLevelEncounter *)pTable->GetStruct(pTable->GetCount()-1);

				pEntry->pType = pType;
				pEntry->iWeight = iFreq;
				pEntry->pBaseSovereign = pType->GetSovereign();
				pEntry->pTable = pType->GetEncountersTable();
				}
			}

		m_LevelEncounterTables.AppendObject(pTable, NULL);
		}

	return NOERROR;
	}

ALERROR CUniverse::InitSounds (SDesignLoadCtx &Ctx, CXMLElement *pSounds, CResourceDb &Resources)

//	InitSounds
//
//	Loads sound resources

	{
	ALERROR error;

	//	Nothing to do if we don't want sound resources

	if (m_pSoundMgr == NULL || Ctx.bNoResources)
		return NOERROR;

	//	Figure out if we've got a special folder for the resources

	CString sRoot = pSounds->GetAttribute(FOLDER_ATTRIB);

	//	Loop over all sound resources

	for (int i = 0; i < pSounds->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pSounds->GetContentElement(i);
		DWORD dwUNID = (DWORD)pItem->GetAttributeInteger(UNID_ATTRIB);
		CString sFilename = pItem->GetAttribute(FILENAME_ATTRIB);
		int iChannel;

		//	Load the resource

		if (error = Resources.LoadSound(*m_pSoundMgr, sRoot, sFilename, &iChannel))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unable to load sound file: %s"), sFilename);
			return error;
			}

		//	Add to our map

		if (error = m_Sounds.AddEntry((int)dwUNID, (CObject *)iChannel))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unable to add sound file: %x"), dwUNID);
			return error;
			}
		}

	return NOERROR;
	}

ALERROR CUniverse::InitStarSystemTypes (SDesignLoadCtx &Ctx, CXMLElement *pElement)

//	InitStarSystemTypes
//
//	Load <StarSystemTypes> tag

	{
	ALERROR error;
	int i;

	for (i = 0; i < pElement->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pElement->GetContentElement(i);

		if (strEquals(pItem->GetTag(), TABLES_TAG))
			{
			if (error = m_Design.AddSystemTable(Ctx, pItem))
				return error;
			}
		else
			{
			if (error = m_Design.LoadEntryFromXML(Ctx, pItem))
				return error;

#ifdef DEBUG_SOURCE_LOAD_TRACE
			kernelDebugLogMessage("Loaded system type: %x", dwUNID);
#endif
			}
		}

	return NOERROR;
	}

ALERROR CUniverse::LoadDesignElement (SDesignLoadCtx &Ctx, CXMLElement *pElement)

//	LoadDesignElement
//
//	Loads a design element

	{
	ALERROR error;

	//	If we're loading just an adventure description, then only
	//	load certain elements

	if (Ctx.bLoadAdventureDesc)
		{
		//	<Image>
		if (strEquals(pElement->GetTag(), IMAGE_TAG))
			return m_Design.LoadEntryFromXML(Ctx, pElement);

		//	<AdventureDesc>
		else if (strEquals(pElement->GetTag(), ADVENTURE_DESC_TAG))
			return m_Design.LoadEntryFromXML(Ctx, pElement);

		//	Ignore all other elements
		else
			return NOERROR;
		}

	//	Otherwise, load everything else

	else
		{
		//	Are we loading an adventure?

		bool bIsAdventure = (Ctx.pExtension && Ctx.pExtension->iType == extAdventure);

		//	<Image>
		if (strEquals(pElement->GetTag(), IMAGE_TAG))
			{
			//	We don't always load images because we load all images for an
			//	adventure desc (but not its modules)

			return ((!bIsAdventure || Ctx.bLoadModule) ? m_Design.LoadEntryFromXML(Ctx, pElement) : NOERROR);
			}

		//	<AdventureDesc> already loaded 
		else if (strEquals(pElement->GetTag(), ADVENTURE_DESC_TAG))
			{
			//	If we're loading an adventure, then we already loaded the AdventureDesc, so
			//	we don't need to do anything

			if (bIsAdventure)
				return NOERROR;

			//	Otherwise, we are loading the main resource file. In that case, we need to
			//	process the adventure desc.

			else
				{
				CDesignType *pType;

				if (error = m_Design.LoadEntryFromXML(Ctx, pElement, &pType))
					return error;

				//	Get the adventure desc

				CAdventureDesc *pAdventure = CAdventureDesc::AsType(pType);
				if (pAdventure == NULL)
					{
					Ctx.sError = CONSTLIT("Adventure descriptor expected.");
					return ERR_FAIL;
					}

				return NOERROR;
				}
			}
		
		//	<TranscendenceAdventure> in default resource file
		else if (strEquals(pElement->GetTag(), TRANSCENDENCE_ADVENTURE_TAG))
			{
			//	Add to design collection

			if (error = m_Design.BeginLoadAdventureDesc(Ctx, pElement, true))
				return error;

			//	Load the design elements

			if (error = LoadExtension(Ctx, pElement))
				return error;

			//	Done

			m_Design.EndLoadAdventureDesc(Ctx);

			return NOERROR;
			}

		//	<Sound>
		else if (strEquals(pElement->GetTag(), SOUND_TAG))
			return LoadSound(Ctx, pElement);

		//	<Globals>
		else if (strEquals(pElement->GetTag(), GLOBALS_TAG))
			return LoadGlobals(Ctx, pElement);

		//	<Modules>
		else if (strEquals(pElement->GetTag(), MODULES_TAG))
			return LoadModules(Ctx, pElement);

		//	Standard design element
		else
			return m_Design.LoadEntryFromXML(Ctx, pElement);
		}
	}

ALERROR CUniverse::LoadGlobals (SDesignLoadCtx &Ctx, CXMLElement *pElement)

//	LoadGlobals
//
//	Load global code

	{
	CCodeChainCtx CCCtx;

	ICCItem *pCode = CCCtx.Link(pElement->GetContentText(0), 0, NULL);
	ICCItem *pResult = CCCtx.Run(pCode);
	if (pResult->IsError())
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Globals: %s"), pResult->GetStringValue());
		return ERR_FAIL;
		}

	CCCtx.Discard(pResult);

	return NOERROR;
	}

ALERROR CUniverse::LoadModule (SDesignLoadCtx &Ctx, CXMLElement *pModule)

//	LoadModule
//
//	Loads the module

	{
	ALERROR error;
	int i;

	//	Load all the design elements

	for (i = 0; i < pModule->GetContentElementCount(); i++)
		{
		CXMLElement *pElement = pModule->GetContentElement(i);

		if (error = LoadDesignElement(Ctx, pElement))
			return error;
		}

	return NOERROR;
	}

ALERROR CUniverse::LoadModules (SDesignLoadCtx &Ctx, CXMLElement *pModules)

//	LoadModules
//
//	Load all sub-modules

	{
	ALERROR error;
	int i;

	for (i = 0; i < pModules->GetContentElementCount(); i++)
		{
		CXMLElement *pModule = pModules->GetContentElement(i);

		CString sFilename = pModule->GetAttribute(FILENAME_ATTRIB);

#ifdef DEBUG_SOURCE_LOAD_TRACE
		kernelDebugLogMessage("Loading module %s...", sFilename.GetASCIIZPointer());
#endif
		//	Load the module XML

		CXMLElement *pModuleXML;
		if (error = Ctx.pResDb->LoadModule(NULL_STR, sFilename, &pModuleXML, &Ctx.sError))
			{
			if (error == ERR_NOTFOUND)
				Ctx.sError = strPatternSubst(CONSTLIT("%s: %s"), Ctx.pResDb->GetFilespec(), Ctx.sError);
			return error;
			}

		if (!strEquals(pModuleXML->GetTag(), TRANSCENDENCE_MODULE_TAG))
			{
			delete pModuleXML;
			Ctx.sError = strPatternSubst(CONSTLIT("Module must have <TranscendenceModule> root element: %s"), sFilename);
			return ERR_FAIL;
			}

		//	We are loading a module

		bool bOldLoadModule = Ctx.bLoadModule;
		Ctx.bLoadModule = true;

		//	Process each design element in the module

#ifdef DEBUG_SOURCE_LOAD_TRACE
		kernelDebugLogMessage("...loaded XML from database");
#endif
		if (error = LoadModule(Ctx, pModuleXML))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unable to load module (%s): %s"), sFilename, Ctx.sError);
			return error;
			}

		//	Clean up

		Ctx.bLoadModule = bOldLoadModule;
		delete pModuleXML;

#ifdef DEBUG_SOURCE_LOAD_TRACE
		kernelDebugLogMessage("Done loading module", sFilename.GetASCIIZPointer());
#endif
		}

	return NOERROR;
	}

ALERROR CUniverse::LoadSound (SDesignLoadCtx &Ctx, CXMLElement *pElement)

//	LoadSound
//
//	Load a sound element

	{
	ALERROR error;

	if (Ctx.bNoResources)
		return NOERROR;

	DWORD dwUNID = LoadUNID(Ctx, pElement->GetAttribute(UNID_ATTRIB));
	CString sFilename = pElement->GetAttribute(FILENAME_ATTRIB);

	//	Load the image

	int iChannel;
	if (error = Ctx.pResDb->LoadSound(*m_pSoundMgr, NULL_STR, sFilename, &iChannel))
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unable to load sound: %s"), sFilename);
		return error;
		}

	if (error = m_Sounds.AddEntry((int)dwUNID, (CObject *)iChannel))
		{
		Ctx.sError = CONSTLIT("Unable to add sound");
		return error;
		}

#ifdef DEBUG_SOURCE_LOAD_TRACE
	kernelDebugLogMessage("Loaded sound: %x", dwUNID);
#endif

	return NOERROR;
	}
