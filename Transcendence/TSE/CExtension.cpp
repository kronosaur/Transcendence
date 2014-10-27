//	CExtension.cpp
//
//	CExtension class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.
//
//	API VERSION HISTORY
//
//	 0: Unknown version
//
//	 1: 95-0.96b
//		Original Extensions
//
//	 2: 0.97
//		Changed gStation to gSource
//
//	 3: 1.1
//		<SmokeTrail>: emitSpeed fixed (used in klicks per tick instead of per second)
//
//	See: LoadExtensionVersion in Utilities.cpp

#include "PreComp.h"

#define ADVENTURE_DESC_TAG						CONSTLIT("AdventureDesc")
#define GLOBALS_TAG								CONSTLIT("Globals")
#define IMAGE_TAG								CONSTLIT("Image")
#define IMAGES_TAG								CONSTLIT("Images")
#define LIBRARY_TAG								CONSTLIT("Library")
#define MODULE_TAG								CONSTLIT("Module")
#define MODULES_TAG								CONSTLIT("Modules")
#define SOUND_TAG								CONSTLIT("Sound")
#define SOUNDS_TAG								CONSTLIT("Sounds")
#define STAR_SYSTEM_TOPOLOGY_TAG				CONSTLIT("StarSystemTopology")
#define SYSTEM_TOPOLOGY_TAG						CONSTLIT("SystemTopology")
#define SYSTEM_TYPES_TAG						CONSTLIT("SystemTypes")
#define TABLES_TAG								CONSTLIT("Tables")
#define TRANSCENDENCE_ADVENTURE_TAG				CONSTLIT("TranscendenceAdventure")
#define TRANSCENDENCE_EXTENSION_TAG				CONSTLIT("TranscendenceExtension")
#define TRANSCENDENCE_LIBRARY_TAG				CONSTLIT("TranscendenceLibrary")
#define TRANSCENDENCE_MODULE_TAG				CONSTLIT("TranscendenceModule")

#define API_VERSION_ATTRIB						CONSTLIT("apiVersion")
#define AUTO_INCLUDE_ATTRIB						CONSTLIT("autoInclude")
#define AUTO_INCLUDE_FOR_COMPATIBILITY_ATTRIB	CONSTLIT("autoIncludeForCompatibility")
#define COVER_IMAGE_UNID_ATTRIB					CONSTLIT("coverImageID")
#define CREDITS_ATTRIB							CONSTLIT("credits")
#define DEBUG_ONLY_ATTRIB						CONSTLIT("debugOnly")
#define EXTENDS_ATTRIB							CONSTLIT("extends")
#define FILENAME_ATTRIB							CONSTLIT("filename")
#define FOLDER_ATTRIB							CONSTLIT("folder")
#define NAME_ATTRIB								CONSTLIT("name")
#define PRIVATE_ATTRIB							CONSTLIT("private")
#define RELEASE_ATTRIB							CONSTLIT("release")
#define UNID_ATTRIB								CONSTLIT("UNID")
#define USES_XML_ATTRIB							CONSTLIT("usesXML")
#define VERSION_ATTRIB							CONSTLIT("version")

#define FILESPEC_TDB_EXTENSION					CONSTLIT("tdb")

//	The center of an adventure cover image is at this position relative to the
//	right edge of the image.

const int RIGHT_COVER_OFFSET =					256 + 160;

CExtension::CExtension (void) :
		m_dwUNID(0),
		m_iType(extUnknown),
		m_iLoadState(loadNone),
		m_iFolderType(folderUnknown),
		m_dwAPIVersion(0),
		m_pEntities(NULL),
		m_dwRelease(0),
		m_pCoverImage(NULL),
		m_pAdventureDesc(NULL),
		m_pRootXML(NULL),
		m_bMarked(false),
		m_bDebugOnly(false),
		m_bRegistered(false),
		m_bVerified(false),
		m_bDisabled(false),
		m_bDeleted(false),
		m_bUsesXML(false)

//	CExtension constructor

	{
	}

CExtension::~CExtension (void)

//	CExtension destructor

	{
	CleanUp();
	}

bool CExtension::CanExtend (CExtension *pAdventure) const

//	CanExtend
//
//	Returns TRUE if this extension can extend the given adventure.

	{
	ASSERT(pAdventure);

	//	If our extend list is empty then we extend everything.

	if (m_Extends.GetCount() == 0)
		return true;

	//	Otherwise, see if this adventure is in the list.

	return m_Extends.Find(pAdventure->GetUNID());
	}

void CExtension::CleanUp (void)

//	CleanUp
//
//	Cleans up class and frees up resources.

	{
	int i;

	//	Delete entities

	if (m_pEntities)
		{
		delete m_pEntities;
		m_pEntities = NULL;
		}

	//	Delete design types

	for (i = 0; i < m_DesignTypes.GetCount(); i++)
		delete m_DesignTypes.GetEntry(i);

	m_DesignTypes.DeleteAll();
	m_Externals.DeleteAll();

	//	Delete global functions

	CCodeChain *pCC = &g_pUniverse->GetCC();
	for (i = 0; i < m_Globals.GetCount(); i++)
		m_Globals[i].pCode->Discard(pCC);

	m_Globals.DeleteAll();

	//	Delete XML representation

	CleanUpXML();

	//	Delete other stuff

	m_Topology.CleanUp();
	SweepImages();
	}

void CExtension::CleanUpXML (void)

//	CleanUpXML
//
//	Deletes XML representation.

	{
	int i;

	if (m_pRootXML)
		{
		delete m_pRootXML;
		m_pRootXML = NULL;
		}

	for (i = 0; i < m_ModuleXML.GetCount(); i++)
		delete m_ModuleXML[i];

	m_ModuleXML.DeleteAll();

	//	Remove references to XML

	for (i = 0; i < m_DesignTypes.GetCount(); i++)
		{
		CDesignType *pType = m_DesignTypes.GetEntry(i);
		pType->SetXMLElement(NULL);
		}
	}

ALERROR CExtension::ComposeLoadError (SDesignLoadCtx &Ctx, CString *retsError)

//	ComposeLoadError
//
//	Adds the filename to the load error.

	{
	if (retsError)
		{
		if (Ctx.sErrorFilespec)
			*retsError = strPatternSubst(CONSTLIT("%s: %s"), Ctx.sErrorFilespec, Ctx.sError);
		else
			*retsError = Ctx.sError;
		}

	return ERR_FAIL;
	}

ALERROR CExtension::CreateBaseFile (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CExternalEntityTable *pEntities, TArray<CExtension *> *retExtensions)

//	CreateBaseFile
//
//	Loads a new extension from the base file.

	{
	ALERROR error;
	int i;

	//	Create an extension object

	CExtension *pExtension = new CExtension;
	pExtension->m_sFilespec = Ctx.sResDb;
	pExtension->m_dwUNID = 0;	//	Base is the only extension with 0 UNID.
	pExtension->m_iType = extBase;
	pExtension->m_iLoadState = loadEntities;
	pExtension->m_iFolderType = folderBase;
	pExtension->m_pEntities = pEntities;
	pExtension->m_ModifiedTime = fileGetModifiedTime(Ctx.sResDb);
	pExtension->m_bRegistered = true;
	pExtension->m_bPrivate = true;
	pExtension->m_bAutoInclude = true;
	pExtension->m_bUsesXML = false;

	//	Load the apiVersion

	CString sAPIVersion;
	if (pDesc->FindAttribute(API_VERSION_ATTRIB, &sAPIVersion))
		{
		pExtension->m_dwAPIVersion = (DWORD)strToInt(sAPIVersion, 0);
		if (pExtension->m_dwAPIVersion < 12)
			pExtension->m_dwAPIVersion = 0;
		}

	//	If this version is later than what we expect, then we fail.

	if (pExtension->m_dwAPIVersion > API_VERSION)
		{
		pExtension->m_pEntities = NULL;	//	Let our parent clean up
		delete pExtension;
		Ctx.sError = CONSTLIT("Newer version of Transcendence.exe required.");
		return ERR_FAIL;
		}

	//	We return the base extension

	retExtensions->Insert(pExtension);

	//	Set up context

	Ctx.pExtension = pExtension;

	//	Load the Main XML file

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pDesc->GetContentElement(i);

		//	<Images>

		if (strEquals(pItem->GetTag(), IMAGES_TAG))
			error = pExtension->LoadImagesElement(Ctx, pItem);

		//	<Sounds>

		else if (strEquals(pItem->GetTag(), SOUNDS_TAG))
			error = pExtension->LoadSoundsElement(Ctx, pItem);

		//	<SystemTypes>

		else if (strEquals(pItem->GetTag(), SYSTEM_TYPES_TAG))
			error = pExtension->LoadSystemTypesElement(Ctx, pItem);

		//	<TranscendenceAdventure>

		else if (strEquals(pItem->GetTag(), TRANSCENDENCE_ADVENTURE_TAG)
				|| strEquals(pItem->GetTag(), TRANSCENDENCE_LIBRARY_TAG))
			{
			//	Load an embedded adventure

			//	Get the entities from the base file

			CExternalEntityTable *pAdvEntities = new CExternalEntityTable;
			pAdvEntities->SetParent(pEntities);

			//	Create a load context

			SDesignLoadCtx AdvCtx;
			AdvCtx.sResDb = Ctx.sResDb;
			AdvCtx.pResDb = Ctx.pResDb;
			AdvCtx.bNoResources = Ctx.bNoResources;
			AdvCtx.bKeepXML = Ctx.bKeepXML;
			AdvCtx.bNoVersionCheck = true;	//	Obsolete now
			AdvCtx.dwInheritAPIVersion = pExtension->GetAPIVersion();
			//	No need to set bBindAsNewGame because it is only useful during Bind.
			//	AdvCtx.bBindAsNewGame = Ctx.bBindAsNewGame;

			//	We always load in full because we don't know how to load later.
			AdvCtx.bLoadAdventureDesc = false;

			//	Load the extension

			CExtension *pAdvExtension;
			error = CExtension::CreateExtension(AdvCtx, pItem, CExtension::folderBase, pAdvEntities, &pAdvExtension);

			//	If this worked, add to list of extensions

			if (error == NOERROR)
				retExtensions->Insert(pAdvExtension);

			//	Otherwise, clean up

			else
				{
				Ctx.sError = AdvCtx.sError;
				delete pAdvEntities;
				}
			}

		//	Other types

		else
			error = pExtension->LoadDesignElement(Ctx, pItem);

		//	Check for error

		if (error)
			{
			pExtension->m_pEntities = NULL;	//	Let our parent clean up
			delete pExtension;
			return error;
			}
		}

	//	Restore

	Ctx.pExtension = NULL;

	//	Done

	pExtension->m_iLoadState = loadComplete;

	return NOERROR;
	}

ALERROR CExtension::CreateExtension (SDesignLoadCtx &Ctx, CXMLElement *pDesc, EFolderTypes iFolder, CExternalEntityTable *pEntities, CExtension **retpExtension)

//	CreateExtension
//
//	Loads the given extension or adventure. We take ownership of pEntities.

	{
	ALERROR error;
	int i;

	//	Create an extension object

	CExtension *pExtension;
	if (error = CreateExtensionFromRoot(Ctx.sResDb, pDesc, iFolder, pEntities, Ctx.dwInheritAPIVersion, &pExtension, &Ctx.sError))
		return error;

	//	Set up context

	Ctx.pExtension = pExtension;

	//	Load all the design elements

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pDesc->GetContentElement(i);

		if (error = pExtension->LoadDesignElement(Ctx, pItem))
			{
			pExtension->m_pEntities = NULL;	//	Let our parent clean up.
			delete pExtension;
			return error;
			}
		}

	//	If this is an adventure and we have no adventure descriptor then we
	//	fail.

	if (pExtension->m_iType == extAdventure && pExtension->m_pAdventureDesc == NULL)
		{
		pExtension->m_pEntities = NULL;	//	Let our parent clean up.
		delete pExtension;
		Ctx.sError = CONSTLIT("Adventure must have an AdventureDesc type.");
		return ERR_FAIL;
		}

	//	If we get this far and we have no libraries, then include the 
	//	compatibility library.

	if (pExtension->GetLibraryCount() == 0 && pExtension->GetFolderType() != folderBase)
		{
		SLibraryDesc *pLibrary = pExtension->m_Libraries.Insert();
		pLibrary->dwUNID = DEFAULT_COMPATIBILITY_LIBRARY_UNID;
		pLibrary->dwRelease = 1;
		}

	//	Restore

	Ctx.pExtension = NULL;

	//	Done

	pExtension->m_iLoadState = (Ctx.bLoadAdventureDesc ? loadAdventureDesc : loadComplete);
	*retpExtension = pExtension;

	return NOERROR;
	}

ALERROR CExtension::CreateExtensionFromRoot (const CString &sFilespec, CXMLElement *pDesc, EFolderTypes iFolder, CExternalEntityTable *pEntities, DWORD dwInheritAPIVersion, CExtension **retpExtension, CString *retsError)

//	CreateExtension
//
//	Loads the given extension or adventure. We take ownership of pEntities.

	{
	int i;

	//	Create an extension object

	CExtension *pExtension = new CExtension;
	pExtension->m_sFilespec = sFilespec;
	pExtension->m_dwUNID = pDesc->GetAttributeInteger(UNID_ATTRIB);
	if (pExtension->m_dwUNID == 0)
		{
		delete pExtension;
		*retsError = CONSTLIT("Invalid UNID.");
		return ERR_FAIL;
		}

	if (strEquals(pDesc->GetTag(), TRANSCENDENCE_ADVENTURE_TAG))
		pExtension->m_iType = extAdventure;
	else if (strEquals(pDesc->GetTag(), TRANSCENDENCE_LIBRARY_TAG))
		pExtension->m_iType = extLibrary;
	else if (strEquals(pDesc->GetTag(), TRANSCENDENCE_EXTENSION_TAG))
		pExtension->m_iType = extExtension;
	else
		{
		delete pExtension;
		*retsError = strPatternSubst(CONSTLIT("Unknown root element: %s"), pDesc->GetTag());
		return ERR_FAIL;
		}
	
	pExtension->m_iLoadState = loadEntities;
	pExtension->m_iFolderType = iFolder;
	pExtension->m_pEntities = pEntities;
	pExtension->m_ModifiedTime = fileGetModifiedTime(sFilespec);
	pExtension->m_bDebugOnly = pDesc->GetAttributeBool(DEBUG_ONLY_ATTRIB);
	pExtension->m_bRegistered = IsRegisteredUNID(pExtension->m_dwUNID);
	pExtension->m_bPrivate = pDesc->GetAttributeBool(PRIVATE_ATTRIB);
	pExtension->m_bAutoInclude = pDesc->GetAttributeBool(AUTO_INCLUDE_ATTRIB);
	pExtension->m_bUsesXML = pDesc->GetAttributeBool(USES_XML_ATTRIB);

	//	API version

	CString sAPIVersion;
	if (pDesc->FindAttribute(API_VERSION_ATTRIB, &sAPIVersion))
		{
		pExtension->m_dwAPIVersion = (DWORD)strToInt(sAPIVersion, 0);
		if (pExtension->m_dwAPIVersion < 12)
			pExtension->m_dwAPIVersion = 0;
		pExtension->m_sVersion = pDesc->GetAttribute(VERSION_ATTRIB);
		}
	else if (dwInheritAPIVersion)
		{
		pExtension->m_dwAPIVersion = dwInheritAPIVersion;
		pExtension->m_sVersion = pDesc->GetAttribute(VERSION_ATTRIB);
		}
	else
		{
		sAPIVersion = pDesc->GetAttribute(VERSION_ATTRIB);
		pExtension->m_dwAPIVersion = ::LoadExtensionVersion(sAPIVersion);
		}

	if (pExtension->m_dwAPIVersion == 0)
		{
		pExtension->m_pEntities = NULL;	//	Let our parent clean up.
		delete pExtension;
		*retsError = strPatternSubst(CONSTLIT("Unable to load extension: incompatible version: %s"), sAPIVersion);
		return ERR_FAIL;
		}

	//	If this is a later version, then disabled it

	if (pExtension->m_dwAPIVersion > API_VERSION)
		pExtension->SetDisabled(CONSTLIT("Requires a newer version of Transcendence.exe"));

	//	Release

	pExtension->m_dwRelease = pDesc->GetAttributeInteger(RELEASE_ATTRIB);

	//	Registered extensions default to release 1.

	if (pExtension->m_dwRelease == 0 && iFolder == folderCollection)
		pExtension->m_dwRelease = 1;

	//	Name

	pExtension->m_sName = pDesc->GetAttribute(NAME_ATTRIB);
	if (pExtension->m_sName.IsBlank())
		pExtension->m_sName = strPatternSubst(CONSTLIT("Extension %x"), pExtension->m_dwUNID);

	//	Image

	pExtension->m_dwCoverUNID = (DWORD)pDesc->GetAttributeInteger(COVER_IMAGE_UNID_ATTRIB);

	//	Load credits (we parse them into a string array)

	CString sCredits = pDesc->GetAttribute(CREDITS_ATTRIB);
	if (!sCredits.IsBlank())
		strDelimitEx(sCredits, ';', DELIMIT_TRIM_WHITESPACE, 0, &pExtension->m_Credits);

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
				pExtension->m_Extends.Insert(dwUNID);
			}
		}

	//	Other options

	pExtension->m_dwAutoIncludeAPIVersion = (DWORD)pDesc->GetAttributeIntegerBounded(AUTO_INCLUDE_FOR_COMPATIBILITY_ATTRIB, 0, -1, 0);

	//	Done

	*retpExtension = pExtension;

	return NOERROR;
	}

ALERROR CExtension::CreateExtensionStub (const CString &sFilespec, EFolderTypes iFolder, CExtension **retpExtension, CString *retsError)

//	CreateExtensionStub
//
//	Loads enough of the given file to get the entities and the root element.

	{
	ALERROR error;

	//	Open up the file

	CResourceDb Resources(sFilespec, true);
	if (error = Resources.Open(DFOPEN_FLAG_READ_ONLY, retsError))
		return error;

	//	Create a object to receive all the entities

	CExternalEntityTable *pEntities = new CExternalEntityTable;

	//	Load the main XML file and get the entities

	CXMLElement *pGameFile;
	if (error = Resources.LoadGameFileStub(&pGameFile, pEntities, retsError))
		{
		delete pEntities;
		return error;
		}

	//	Create the extension
	//
	//	If sucessful then pExtension takes ownership of	pEntities.

	CExtension *pExtension;
	error = CreateExtensionFromRoot(sFilespec, pGameFile, iFolder, pEntities, 0, &pExtension, retsError);

	//	Clean up

	delete pGameFile;

	//	Error

	if (error)
		{
		delete pEntities;
		return error;
		}

	//	Done

	*retpExtension = pExtension;

	return NOERROR;
	}

void CExtension::CreateIcon (int cxWidth, int cyHeight, CG16bitImage **retpIcon) const

//	CreateIcon
//
//	Creates a cover icon for the adventure. The caller is responsible for
//	freeing the result.

	{
	//	Load the image

	CG16bitImage *pBackground = GetCoverImage();
	if (pBackground == NULL || pBackground->GetWidth() == 0 || pBackground->GetHeight() == 0)
		{
		int cxSize = Min(cxWidth, cyHeight);
		*retpIcon = new CG16bitImage;
		(*retpIcon)->CreateBlank(cxSize, cxSize, false);
		return;
		}

	//	Figure out the dimensions of the icon based on the image size and the
	//	desired output.
	//
	//	If the background is larger than the icon size then we need to scale it.

	CG16bitImage *pIcon;
	if (pBackground->GetWidth() > cxWidth || pBackground->GetHeight() > cyHeight)
		{
		int xSrc, ySrc, cxSrc, cySrc;
		Metric rScale;

		//	If we have a widescreen cover image and we want a portrait or
		//	square icon, then we zoom in on the key part of the cover.

		if (pBackground->GetWidth() > 2 * pBackground->GetHeight())
			{
			rScale = (Metric)cyHeight / pBackground->GetHeight();

			cxSrc = (int)(cxWidth / rScale);
			xSrc = Min(pBackground->GetWidth() - cxSrc, pBackground->GetWidth() - (RIGHT_COVER_OFFSET + (cxSrc / 2)));

			ySrc = 0;
			cySrc = pBackground->GetHeight();
			}
		else
			{
			rScale = (Metric)cxWidth / pBackground->GetWidth();
			if (rScale * pBackground->GetHeight() > (Metric)cyHeight)
				rScale = (Metric)cyHeight / pBackground->GetHeight();

			xSrc = 0;
			ySrc = 0;
			cxSrc = pBackground->GetWidth();
			cySrc = pBackground->GetHeight();
			}

		//	Create the icon

		pIcon = new CG16bitImage;
		pIcon->CreateFromImageTransformed(*pBackground,
				xSrc,
				ySrc,
				cxSrc,
				cySrc,
				rScale,
				rScale,
				0.0);
		}

	//	Otherwise we center the image on the icon

	else
		{
		//	Create the icon

		pIcon = new CG16bitImage;
		pIcon->CreateBlank(cxWidth, cyHeight, false);

		//	Blt

		pIcon->Blt(0,
				0,
				pBackground->GetWidth(),
				pBackground->GetHeight(),
				*pBackground,
				(cxWidth - pBackground->GetWidth()) / 2,
				(cyHeight - pBackground->GetHeight()) / 2);
		}

	//	Done

	*retpIcon = pIcon;
	}

void CExtension::DebugDump (CExtension *pExtension, bool bFull)

//	DebugDump
//
//	Dumps debug output for the extension.

	{
	if (pExtension == NULL)
		{
		::kernelDebugLogMessage("Null extension pointer.");
		return;
		}

	try
		{
		::kernelDebugLogMessage("%08x %s [%08x]", pExtension->m_dwUNID, pExtension->m_sFilespec, (DWORD)pExtension);
		if (bFull)
			{
			if (pExtension->m_bDeleted)
				::kernelDebugLogMessage("DELETED");
			if (pExtension->m_bDisabled)
				::kernelDebugLogMessage("DISABLED: %s", pExtension->m_sDisabledReason);
			if (pExtension->m_bVerified)
				::kernelDebugLogMessage("VERIFIED");
			}
		}
	catch (...)
		{
		::kernelDebugLogMessage("Invalid extension pointer.");
		}
	}

ALERROR CExtension::ExecuteGlobals (SDesignLoadCtx &Ctx)

//	ExecuteGlobals
//
//	Execute the globals

	{
	DEBUG_TRY

	int i;
	CCodeChainCtx CCCtx;

	//	Add a hook so that all lambda expressions defined in this global block
	//	are wrapped with something that sets the extension UNID to the context.

	if (m_iType != extBase)
		CCCtx.SetGlobalDefineWrapper(this);

	//	Run the code (which will likely define a bunch of functions)

	for (i = 0; i < m_Globals.GetCount(); i++)
		{
		ICCItem *pResult = CCCtx.Run(m_Globals[i].pCode);
		if (pResult->IsError())
			{
			Ctx.sError = strPatternSubst(CONSTLIT("%s globals: %s"), m_Globals[i].sFilespec, pResult->GetStringValue());
			return ERR_FAIL;
			}

		CCCtx.Discard(pResult);
		}

	//	Done

	return NOERROR;

	DEBUG_CATCH
	}

CG16bitImage *CExtension::GetCoverImage (void) const

//	GetCoverImage
//
//	Returns the cover image (or NULL if none). The caller does NOT need to free
//	the image. However, the caller should not user the image past a SweepImages
//	call.

	{
	if (m_pCoverImage)
		return m_pCoverImage;

	//	Adventure desc overrides our UNID

	DWORD dwCoverUNID = m_dwCoverUNID;
	if (m_pAdventureDesc
			&& m_pAdventureDesc->GetBackgroundUNID() != 0)
		dwCoverUNID = m_pAdventureDesc->GetBackgroundUNID();

	if (dwCoverUNID == 0)
		return NULL;

	//	Find the image object

	CObjectImage *pObjImage = CObjectImage::AsType(m_DesignTypes.FindByUNID(dwCoverUNID));
	if (pObjImage == NULL)
		return NULL;

	//	Load the image

	g_pUniverse->SetLogImageLoad(false);
	m_pCoverImage = pObjImage->CreateCopy();
	g_pUniverse->SetLogImageLoad(true);

	//	Done

	return m_pCoverImage;
	}

ALERROR CExtension::Load (ELoadStates iDesiredState, IXMLParserController *pResolver, bool bNoResources, bool bKeepXML, CString *retsError)

//	Load
//
//	Makes sure that the extension is fully loaded.

	{
	ALERROR error;
	int i;

	switch (m_iLoadState)
		{
		case loadNone:
			{
			*retsError = CONSTLIT("Unable to load.");
			return ERR_FAIL;
			}

		case loadEntities:
		case loadAdventureDesc:
			{
			if (iDesiredState == loadNone || iDesiredState == loadEntities)
				return NOERROR;
			else if (iDesiredState == loadAdventureDesc && m_iLoadState == loadAdventureDesc)
				return NOERROR;

			//	Open the file

			CResourceDb ExtDb(m_sFilespec, true);
			if (error = ExtDb.Open(DFOPEN_FLAG_READ_ONLY, retsError))
				return ERR_FAIL;

			//	Setup

			SDesignLoadCtx Ctx;
			Ctx.sResDb = m_sFilespec;
			Ctx.pResDb = &ExtDb;
			Ctx.bNoVersionCheck = true;	//	Obsolete now
			Ctx.bNoResources = bNoResources;
			Ctx.bKeepXML = bKeepXML;
			Ctx.bLoadAdventureDesc = (iDesiredState == loadAdventureDesc && m_iType == extAdventure);
			Ctx.sErrorFilespec = m_sFilespec;

			//	If this is a registered extension then compute a digest for the
			//	file (so that we can compare against the cloud's digest).
			//	
			//	We need to do this even if we fail later because we don't want to
			//	have to recalc it later.

			if (m_Digest.IsEmpty() && GetFolderType() == folderCollection && IsRegistered())
				{
				if (error = fileCreateDigest(m_sFilespec, &m_Digest))
					{
					*retsError = strPatternSubst(CONSTLIT("Unable to compute digest for: %s."), m_sFilespec);
					return error;
					}
				}

			//	If we've already loaded a root element, then we need to clean up

			if (m_pRootXML)
				CleanUpXML();

			//	Parse the XML file into a structure

			if (error = ExtDb.LoadGameFile(&m_pRootXML, pResolver, retsError))
				{
				//	If we're in debug mode then this is a real error.

				if (g_pUniverse->InDebugMode()
						&& !ExtDb.IsTDB())
					{
					if (retsError) *retsError = strPatternSubst(CONSTLIT("Error parsing %s: %s"), m_sFilespec, *retsError);
					return ERR_FAIL;
					}

				//	Otherwise, we try to continue as if nothing bad had happened, but we
				//	disable the extension.

				else
					{
					SetDisabled((retsError ? *retsError : CONSTLIT("Unable to load")));
					return NOERROR;
					}
				}

			//	Set up context

			Ctx.pExtension = this;

			//	Load all the design elements

			for (i = 0; i < m_pRootXML->GetContentElementCount(); i++)
				{
				CXMLElement *pItem = m_pRootXML->GetContentElement(i);

				if (error = LoadDesignElement(Ctx, pItem))
					{
					if (!bKeepXML)
						{
						delete m_pRootXML;
						m_pRootXML = NULL;
						}

					if (g_pUniverse->InDebugMode()
							&& !ExtDb.IsTDB())
						return ComposeLoadError(Ctx, retsError);

					SetDisabled(Ctx.sError);
					return NOERROR;
					}
				}

			//	Restore

			Ctx.pExtension = NULL;

			//	Done

			m_iLoadState = (m_iType == extAdventure ? iDesiredState : loadComplete);
			if (!bKeepXML)
				{
				delete m_pRootXML;
				m_pRootXML = NULL;
				}

			//	If we get this far and we have no libraries, then include the 
			//	compatibility library.

			if (m_iLoadState == loadComplete && GetLibraryCount() == 0 && GetFolderType() != folderBase)
				{
				SLibraryDesc *pLibrary = m_Libraries.Insert();
				pLibrary->dwUNID = DEFAULT_COMPATIBILITY_LIBRARY_UNID;
				pLibrary->dwRelease = 1;
				}

			//	Debug output

			switch (m_iType)
				{
				case extAdventure:
					if (m_iLoadState == loadAdventureDesc)
						kernelDebugLogMessage("Loaded adventure desc: %s", m_sFilespec);
					else
						kernelDebugLogMessage("Loaded adventure: %s", m_sFilespec);
					break;

				case extExtension:
					kernelDebugLogMessage("Loaded extension: %s", m_sFilespec);
					break;

				case extLibrary:
					kernelDebugLogMessage("Loaded library: %s", m_sFilespec);
					break;
				}

			return NOERROR;
			}

		case loadComplete:
			return NOERROR;

		default:
			ASSERT(false);
			return ERR_FAIL;
		}
	}

ALERROR CExtension::LoadDesignElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadDesignElement
//
//	Loads the element appropriately.

	{
	ALERROR error;

	//	<Image>

	if (strEquals(pDesc->GetTag(), IMAGE_TAG))
		{
		//	If we've already loaded adventure descriptors, and we're loading the
		//	main file, then we ignore images (since we already loaded them).

		if (m_iLoadState == loadAdventureDesc && !Ctx.bLoadModule)
			return NOERROR;

		//	Otherwise, load it

		return LoadDesignType(Ctx, pDesc);
		}

	//	<AdventureDesc>

	else if (strEquals(pDesc->GetTag(), ADVENTURE_DESC_TAG))
		{
		//	If we've already loaded adventure descs, then we're done

		if (m_iLoadState == loadAdventureDesc)
			return NOERROR;

		//	Must be an adventure extension

		if (m_iType != extAdventure)
			{
			Ctx.sError = CONSTLIT("Only adventures may have an AdventureDesc type.");
			return ERR_FAIL;
			}

		//	Can't have more than one

		if (m_pAdventureDesc != NULL)
			{
			Ctx.sError = CONSTLIT("Cannot not have more than one AdventureDesc.");
			return ERR_FAIL;
			}

		//	Load it

		CDesignType *pType;
		if (error = LoadDesignType(Ctx, pDesc, &pType))
			return error;

		//	Remember the adventure

		m_pAdventureDesc = CAdventureDesc::AsType(pType);
		if (m_pAdventureDesc == NULL)
			{
			Ctx.sError = CONSTLIT("Invalid AdventureDesc.");
			return ERR_FAIL;
			}

		return NOERROR;
		}

	//	If we're only loading adventure descs, then we don't care about other
	//	types.

	else if (Ctx.bLoadAdventureDesc)
		return NOERROR;
	
	//	Standard design element

	else
		return LoadDesignType(Ctx, pDesc);
	}

ALERROR CExtension::LoadDesignType (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CDesignType **retpType)

//	LoadDesignType
//
//	Loads a standard design type

	{
	ALERROR error;
	CDesignType *pType = NULL;

	//	Load topology

	if (strEquals(pDesc->GetTag(), STAR_SYSTEM_TOPOLOGY_TAG)
			|| strEquals(pDesc->GetTag(), SYSTEM_TOPOLOGY_TAG))
		{
		if (error = m_Topology.LoadFromXML(Ctx, pDesc, NULL, NULL_STR))
			return error;
		}

	//	<Sound>

	else if (strEquals(pDesc->GetTag(), SOUND_TAG))
		return LoadSoundElement(Ctx, pDesc);

	//	<Globals>

	else if (strEquals(pDesc->GetTag(), GLOBALS_TAG))
		return LoadGlobalsElement(Ctx, pDesc);

	//	<Library>

	else if (strEquals(pDesc->GetTag(), LIBRARY_TAG))
		return LoadLibraryElement(Ctx, pDesc);

	//	<Module>

	else if (strEquals(pDesc->GetTag(), MODULE_TAG))
		return LoadModuleElement(Ctx, pDesc);

	//	<Modules>

	else if (strEquals(pDesc->GetTag(), MODULES_TAG))
		return LoadModulesElement(Ctx, pDesc);

	//	Load standard design elements

	else
		{
		if (error = CDesignType::CreateFromXML(Ctx, pDesc, &pType))
			return error;

		DWORD dwUNID = pType->GetUNID();

		//	Can't use the reserved range

		if (IsReservedUNID(dwUNID))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Cannot use reserved UNID: %x"), dwUNID);
			return ERR_FAIL;
			}

		//	Add to our list

		if (error = m_DesignTypes.AddEntry(pType))
			{
			if (error == ERR_OUTOFROOM)
				{
				//	For backwards compatibility with earlier versions, we 
				//	disable this message.

				if (Ctx.GetAPIVersion() >= 12)
					{
					Ctx.sError = strPatternSubst(CONSTLIT("Duplicate UNID: %x"), dwUNID);
					return error;
					}
				}
			else
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Error adding design entry UNID: %x"), dwUNID);
				return error;
				}
			}

		//	Let this type add external definitions

		pType->AddExternals(&m_Externals);
		}

	//	Done

	if (retpType)
		*retpType = pType;

	return NOERROR;
	}

ALERROR CExtension::LoadGlobalsElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadGlobalsElement
//
//	Loads <Globals>

	{
	CCodeChainCtx CCCtx;

	//	Parse the code and keep it

	ICCItem *pCode = CCCtx.Link(pDesc->GetContentText(0), 0, NULL);
	if (pCode->IsError())
		{
		Ctx.sError = strPatternSubst(CONSTLIT("%s globals: %s"), Ctx.sErrorFilespec, pCode->GetStringValue());
		return ERR_FAIL;
		}

	SGlobalsEntry *pEntry = m_Globals.Insert();
	pEntry->pCode = pCode;
	pEntry->sFilespec = Ctx.sErrorFilespec;

#ifdef DEBUG_GLOBALS
	::kernelDebugLogMessage("Loading globals in %s", Ctx.sErrorFilespec);
#endif

	return NOERROR;
	}

ALERROR CExtension::LoadImagesElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadImagesElement
//
//	Loads <Images> element
//	(For backwards compatibility)

	{
	ALERROR error;
	int i;

	//	Figure out if we've got a special folder for the images

	Ctx.sFolder = pDesc->GetAttribute(FOLDER_ATTRIB);

	//	Load all images

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pDesc->GetContentElement(i);

		if (error = LoadDesignType(Ctx, pItem))
			return error;
		}

	//	Restore folder

	Ctx.sFolder = NULL_STR;

	return NOERROR;
	}

ALERROR CExtension::LoadLibraryElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadLibraryElement
//
//	Loads <Library>

	{
	ALERROR error;

	SLibraryDesc *pLibrary = m_Libraries.Insert();

	if (error = ::LoadUNID(Ctx, pDesc->GetAttribute(UNID_ATTRIB), &pLibrary->dwUNID))
		return error;

	pLibrary->dwRelease = pDesc->GetAttributeInteger(RELEASE_ATTRIB);

	return NOERROR;
	}

ALERROR CExtension::LoadModuleContent (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadModulesContent
//
//	Loads <Module> content

	{
	ALERROR error;
	int i;

	//	Load all the design elements

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pDesc->GetContentElement(i);

		if (error = LoadDesignElement(Ctx, pItem))
			return error;
		}

	return NOERROR;
	}

ALERROR CExtension::LoadModuleElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadModuleElement
//
//	Loads <Module>

	{
	ALERROR error;

	CString sFilename = pDesc->GetAttribute(FILENAME_ATTRIB);

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
	CString sOldErrorFilespec = Ctx.sErrorFilespec;
	Ctx.bLoadModule = true;
	if (strEquals(pathGetExtension(sOldErrorFilespec), FILESPEC_TDB_EXTENSION))
		Ctx.sErrorFilespec = strPatternSubst(CONSTLIT("%s#%s"), sOldErrorFilespec, sFilename);
	else
		Ctx.sErrorFilespec = sFilename;

	//	Process each design element in the module

	if (error = LoadModuleContent(Ctx, pModuleXML))
		return error;

	//	Clean up

	Ctx.sErrorFilespec = sOldErrorFilespec;
	Ctx.bLoadModule = bOldLoadModule;

	//	If we're keeping the XML, then add it to our table

	if (Ctx.bKeepXML && !m_ModuleXML.Find(sFilename))
		m_ModuleXML.Insert(sFilename, pModuleXML);
	else
		delete pModuleXML;

	return NOERROR;
	}

ALERROR CExtension::LoadModulesElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadModulesElement
//
//	Loads <Modules>

	{
	ALERROR error;
	int i;

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pModule = pDesc->GetContentElement(i);

		if (error = LoadModuleElement(Ctx, pModule))
			return error;
		}

	return NOERROR;
	}

ALERROR CExtension::LoadSoundElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadSoundElement
//
//	Loads <Sound> element

	{
	ALERROR error;

	if (Ctx.bNoResources || g_pUniverse->GetSoundMgr() == NULL)
		return NOERROR;

	DWORD dwUNID;
	if (error = LoadUNID(Ctx, pDesc->GetAttribute(UNID_ATTRIB), &dwUNID))
		return error;

	CString sFilename = pDesc->GetAttribute(FILENAME_ATTRIB);

	//	Load the sound

	int iChannel;
	if (error = Ctx.pResDb->LoadSound(*g_pUniverse->GetSoundMgr(), Ctx.sFolder, sFilename, &iChannel))
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unable to load sound: %s"), sFilename);
		return error;
		}

	g_pUniverse->AddSound(dwUNID, iChannel);

	return NOERROR;
	}

ALERROR CExtension::LoadSoundsElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadSoundsElement
//
//	Loads <Sounds> element
//	(For backwards compatibility)

	{
	ALERROR error;
	int i;

	//	Nothing to do if we don't want sound resources

	if (Ctx.bNoResources || g_pUniverse->GetSoundMgr() == NULL)
		return NOERROR;

	//	Figure out if we've got a special folder for the resources

	Ctx.sFolder = pDesc->GetAttribute(FOLDER_ATTRIB);

	//	Loop over all sound resources

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pDesc->GetContentElement(i);

		if (error = LoadSoundElement(Ctx, pItem))
			return error;
		}

	//	Restore folder

	Ctx.sFolder = NULL_STR;

	return NOERROR;
	}

ALERROR CExtension::LoadSystemTypesElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadSystemTypesElement
//
//	Loads <SystemTypes> element
//	(For backwards compatibility)

	{
	ALERROR error;
	int i;

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pDesc->GetContentElement(i);

		if (strEquals(pItem->GetTag(), TABLES_TAG))
			{
			CSystemTable *pTable = new CSystemTable;
			pTable->InitFromXML(Ctx, pItem);
			if (pTable->GetUNID() == 0)
				pTable->SetUNID(DEFAULT_SYSTEM_TABLE_UNID);

			if (error = m_DesignTypes.AddEntry(pTable))
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Error adding system table: %x"), pTable->GetUNID());
				return error;
				}
			}
		else
			{
			if (error = LoadDesignType(Ctx, pItem))
				return error;
			}
		}

	return NOERROR;
	}

void CExtension::SweepImages (void)

//	SweepImages
//
//	Deletes images to save space.

	{
	if (m_pCoverImage)
		{
		delete m_pCoverImage;
		m_pCoverImage = NULL;
		}
	}
