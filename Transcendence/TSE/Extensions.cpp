//	Extensions.cpp
//
//	Loads extensions
//
//	VERSION HISTORY
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

#define TRANSCENDENCE_ADVENTURE_TAG					CONSTLIT("TranscendenceAdventure")
#define TRANSCENDENCE_EXTENSION_TAG					CONSTLIT("TranscendenceExtension")
#define TRANSCENDENCE_MODULE_TAG					CONSTLIT("TranscendenceModule")

#define EXTENSIONS_FOLDER							CONSTLIT("Extensions")
#define EXTENSIONS_FILTER							CONSTLIT("*.*")
#define EXTENSION_XML								CONSTLIT("xml")
#define EXTENSION_TDB								CONSTLIT("tdb")

#define UNID_ATTRIB									CONSTLIT("UNID")

class CSavedDesignLoadCtx
	{
	public:
		CSavedDesignLoadCtx (SDesignLoadCtx &Ctx) :
				m_pCtx(&Ctx),
				m_sSavedResDb(Ctx.sResDb),
				m_pSavedResDb(Ctx.pResDb)
			{ }

		~CSavedDesignLoadCtx (void)
			{
			m_pCtx->sResDb = m_sSavedResDb;
			m_pCtx->pResDb = m_pSavedResDb;
			}

	private:
		SDesignLoadCtx *m_pCtx;
		CString m_sSavedResDb;
		CResourceDb *m_pSavedResDb;
	};

ALERROR CUniverse::InitExtensions (SDesignLoadCtx &Ctx, const CString &sFilespec)

//	InitExtensions
//
//	Load extensions into the game

	{
	CString sPath = pathGetPath(sFilespec);
	CString sExtensionsPath = pathAddComponent(sPath, EXTENSIONS_FOLDER);
	if (!pathExists(sExtensionsPath))
		{
		pathCreate(sExtensionsPath);
		return NOERROR;
		}

	return InitExtensionsFolder(Ctx, sExtensionsPath);
	}

ALERROR CUniverse::InitExtensionsFolder (SDesignLoadCtx &Ctx, const CString &sPath)

//	InitExtensionsFolder
//
//	This is a recursive function that loads all the extensions in a folder
//	(and subfolders)

	{
	ALERROR error;

	CSymbolTable LoadedExtensions(FALSE, TRUE);

	//	Load all extensions in the folder

	CFileDirectory Dir(pathAddComponent(sPath, EXTENSIONS_FILTER));
	while (Dir.HasMore())
		{
		SFileDesc FileDesc;
		Dir.GetNextDesc(&FileDesc);

		//	Skip hidden or system files

		if (FileDesc.bHiddenFile || FileDesc.bSystemFile)
			continue;

		//	Skip any file or directory that starts with a dot

		if (*FileDesc.sFilename.GetASCIIZPointer() == '.')
			continue;

		//	Skip any file or directory that stars with '_'

		if (*FileDesc.sFilename.GetASCIIZPointer() == '_')
			continue;

		//	Get path and extension

		CString sFilepath = pathAddComponent(sPath, FileDesc.sFilename);
		CString sFileExtension = pathGetExtension(sFilepath);
		CString sExtensionRoot = pathStripExtension(sFilepath);

		//	If this is a folder, then recurse

		if (FileDesc.bFolder)
			{
			if (error = InitExtensionsFolder(Ctx, sFilepath))
				return error;

			continue;
			}

		//	If this is not .xml or .tdb, continue

		if (!strEquals(sFileExtension, EXTENSION_XML)
				&& !strEquals(sFileExtension, EXTENSION_TDB))
			continue;

		//	If we've already loaded this extension, continue. This avoids loading
		//	both a .xml and .tdb file for the same extension.

		if (LoadedExtensions.Lookup(sExtensionRoot, NULL) == NOERROR)
			{
			kernelDebugLogMessage("Skipped duplicate extension: %s", sExtensionRoot.GetASCIIZPointer());
			continue;
			}

		LoadedExtensions.AddEntry(sExtensionRoot, NULL);

		//	Open the file

		CResourceDb ExtDb(sExtensionRoot, true);
		if (error = ExtDb.Open(DFOPEN_FLAG_READ_ONLY))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unable to open file: %s"), sExtensionRoot);
			return ERR_FAIL;
			}

		//	If this is a module, then skip it

		CString sDOCTYPERootTag = ExtDb.GetRootTag();
		if (strEquals(sDOCTYPERootTag, TRANSCENDENCE_MODULE_TAG))
			continue;
		else if (!strEquals(sDOCTYPERootTag, TRANSCENDENCE_EXTENSION_TAG)
				&& !strEquals(sDOCTYPERootTag, TRANSCENDENCE_ADVENTURE_TAG))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("%s: Expected <TranscendenceAdventure>, <TranscendenceExtension>, or <TranscendenceModule> instead of <%s>"), 
					sExtensionRoot,
					sDOCTYPERootTag);
			return ERR_FAIL;
			}

		//	Parse the XML file into a structure

		CXMLElement *pRoot;
		CExternalEntityTable *pEntities = new CExternalEntityTable;
		pEntities->SetParent(&m_Design.GetBaseEntities());

		if (error = ExtDb.LoadGameFile(&pRoot, pEntities, &Ctx.sError, pEntities))
			{
			delete pEntities;
			Ctx.sError = strPatternSubst(CONSTLIT("Error parsing %s: %s"), sExtensionRoot, Ctx.sError);
			return ERR_FAIL;
			}

		//	Setup

		CSavedDesignLoadCtx SavedCtx(Ctx);
		Ctx.sResDb = sExtensionRoot;
		Ctx.pResDb = &ExtDb;

		//	Handle Extensions

		if (strEquals(pRoot->GetTag(), TRANSCENDENCE_EXTENSION_TAG))
			{
			kernelDebugLogMessage("Loading extension: %s", Ctx.pResDb->GetFilespec().GetASCIIZPointer());

			//	Add to design collection (takes ownership of pEntities, if successful)

			if (error = m_Design.BeginLoadExtension(Ctx, pRoot, pEntities))
				{
				delete pEntities;
				return error;
				}

			//	Load the design elements

			if (error = LoadExtension(Ctx, pRoot))
				return error;

			//	Done

			m_Design.EndLoadExtension(Ctx);
			}

		//	Handle Adventures

		else if (strEquals(pRoot->GetTag(), TRANSCENDENCE_ADVENTURE_TAG))
			{
			kernelDebugLogMessage("Loading adventure desc: %s", Ctx.pResDb->GetFilespec().GetASCIIZPointer());

			//	Add to design collection (takes owenership of pEntities, if successful)

			if (error = m_Design.BeginLoadAdventureDesc(Ctx, pRoot, false, pEntities))
				{
				delete pEntities;
				return error;
				}

			//	Load the design elements

			if (error = LoadExtension(Ctx, pRoot))
				return error;

			//	Done

			m_Design.EndLoadAdventureDesc(Ctx);
			}
		else
			{
			delete pEntities;

			if (!strEquals(pRoot->GetTag(), sDOCTYPERootTag))
				Ctx.sError = strPatternSubst(CONSTLIT("%s: DOCTYPE (%s) does not match root element (%s)"),
						sExtensionRoot,
						sDOCTYPERootTag,
						pRoot->GetTag());
			else
				Ctx.sError = strPatternSubst(CONSTLIT("%s: Expected <TranscendenceAdventure>, <TranscendenceExtension>, or <TranscendenceModule> instead of <%s>"), 
						sExtensionRoot,
						pRoot->GetTag());

			return ERR_FAIL;
			}

		//	Done

		delete pRoot;
		}

	return NOERROR;
	}

ALERROR CUniverse::LoadAdventure (SDesignLoadCtx &Ctx, CAdventureDesc *pAdventure)

//	LoadAdventure
//
//	Loads the adventure extension

	{
	ALERROR error;

	ASSERT(pAdventure);

	//	Get the extension for this adventure

	SExtensionDesc *pExtension = m_Design.FindExtension(pAdventure->GetExtensionUNID());
	if (pExtension == NULL)
		{
		ASSERT(false);
		Ctx.sError = CONSTLIT("Invalid extension UNID for adventure");
		return ERR_FAIL;
		}

	//	If we've already loaded this adventure, then we're done

	if (pExtension->bLoaded)
		return NOERROR;

	//	If this adventure is in the extensions directory, then we load it

	if (!pExtension->bDefaultResource)
		{
		//	Open the file

		CString sFilespec = pExtension->sResDb;
		CResourceDb ExtDb(sFilespec, (pExtension->bDefaultResource ? false : true));
		if (error = ExtDb.Open(DFOPEN_FLAG_READ_ONLY))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unable to open file: %s"), sFilespec);
			return ERR_FAIL;
			}

		//	Setup

		CSavedDesignLoadCtx SavedCtx(Ctx);
		Ctx.sResDb = sFilespec;
		Ctx.pResDb = &ExtDb;

		//	Parse the XML file into a structure

		CXMLElement *pRoot;
		if (error = ExtDb.LoadGameFile(&pRoot, pExtension->GetEntities(), &Ctx.sError))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Error parsing %s: %s"), sFilespec, Ctx.sError);
			return ERR_FAIL;
			}

		//	Make sure we have the right file

		if (!strEquals(pRoot->GetTag(), TRANSCENDENCE_ADVENTURE_TAG))
			{
			Ctx.sError = CONSTLIT("Adventure expected");
			return ERR_FAIL;
			}

		//	Load the adventure

		kernelDebugLogMessage("Loading adventure: %s", sFilespec.GetASCIIZPointer());

		//	Add to design collection

		if (error = m_Design.BeginLoadAdventure(Ctx, pExtension))
			return error;

		//	Load the design elements

		if (error = LoadExtension(Ctx, pRoot))
			return error;

		//	Done

		m_Design.EndLoadAdventure(Ctx);

		//	Done

		delete pRoot;
		}

	//	Otherwise, we load the modules from the default resource

	else
		{
		int i;

		//	Set the base entities (since this is a default module)

		Ctx.pResDb->SetEntities(&m_Design.GetBaseEntities());

		//	Add to design collection

		if (error = m_Design.BeginLoadAdventure(Ctx, pExtension))
			return error;

		//	Load the modules

		for (i = 0; i < pExtension->Modules.GetCount(); i++)
			{
			//	Load the module XML

			CXMLElement *pModuleXML;
			if (error = Ctx.pResDb->LoadModule(NULL_STR, pExtension->Modules[i], &pModuleXML, &Ctx.sError))
				{
				if (error == ERR_NOTFOUND)
					Ctx.sError = strPatternSubst(CONSTLIT("%s: %s"), Ctx.pResDb->GetFilespec(), Ctx.sError);
				return error;
				}

			if (!strEquals(pModuleXML->GetTag(), TRANSCENDENCE_MODULE_TAG))
				{
				delete pModuleXML;
				Ctx.sError = strPatternSubst(CONSTLIT("Module must have <TranscendenceModule> root element: %s"), pExtension->Modules[i]);
				return ERR_FAIL;
				}

			//	We are loading a module

			bool bOldLoadModule = Ctx.bLoadModule;
			Ctx.bLoadModule = true;

			//	Process each design element in the module

			if (error = LoadModule(Ctx, pModuleXML))
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Unable to load module (%s): %s"), pExtension->Modules[i], Ctx.sError);
				return error;
				}

			//	Clean up

			Ctx.bLoadModule = bOldLoadModule;
			delete pModuleXML;
			}

		//	Done

		m_Design.EndLoadAdventure(Ctx);
		}

	return NOERROR;
	}

ALERROR CUniverse::LoadExtension (SDesignLoadCtx &Ctx, CXMLElement *pExtension)

//	LoadExtension
//
//	Loads the extension

	{
	ALERROR error;
	int i;

	//	Load all the design elements

	for (i = 0; i < pExtension->GetContentElementCount(); i++)
		{
		CXMLElement *pElement = pExtension->GetContentElement(i);

		if (error = LoadDesignElement(Ctx, pElement))
			return error;
		}

	return NOERROR;
	}
