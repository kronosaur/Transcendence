//	CExtensionDirectory.cpp
//
//	CExtensionDirectory class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define ATTRIB_FILENAME                 CONSTLIT("filename")
#define ATTRIB_NAME                     CONSTLIT("name")
#define ATTRIB_UNID                     CONSTLIT("unid")
#define ATTRIB_VERSION                  CONSTLIT("version")

#define EXTENSIONS_FILTER               CONSTLIT("*.tdb")

#define TRANSCENDENCE_TDB               CONSTLIT("Transcendence.tdb")

#define CORE_LIBRARY_TAG				CONSTLIT("CoreLibrary")
#define LIBRARY_TAG                     CONSTLIT("Library")
#define MODULE_TAG                      CONSTLIT("Module")
#define MODULES_TAG                     CONSTLIT("Modules")
#define TRANSCENDENCE_ADVENTURE_TAG		CONSTLIT("TranscendenceAdventure")
#define TRANSCENDENCE_EXTENSION_TAG		CONSTLIT("TranscendenceExtension")
#define TRANSCENDENCE_LIBRARY_TAG		CONSTLIT("TranscendenceLibrary")
#define TRANSCENDENCE_MODULE_TAG		CONSTLIT("TranscendenceModule")

const DWORD UNID_CORE =                 0;
const DWORD UNID_CORE_RPG =			    0x00010000;
const DWORD UNID_CORE_UNIVERSE =		0x00020000;
const DWORD UNID_CORE_TYPES =	        0x00030000;
const DWORD UNID_HUMAN_SPACE =	        0x00100000;

CExtensionDirectory::~CExtensionDirectory (void)

//  CExtensionDirectory destructor

    {
    int i;

    for (i = 0; i < m_Extensions.GetCount(); i++)
        delete m_Extensions[i];
    }

void CExtensionDirectory::AddCoreLibraries (CSimpleLibraryResolver &Resolver, SExtensionDesc *pExtension) const

//  AddCoreLibraries
//
//  Add core libraries to the resolver.

    {
	//	We always need the core libraries

    AddLibrary(Resolver, UNID_CORE);
    AddLibrary(Resolver, UNID_CORE_TYPES);
    AddLibrary(Resolver, UNID_CORE_RPG);
    AddLibrary(Resolver, UNID_CORE_UNIVERSE);
    }

void CExtensionDirectory::AddLibrary (CSimpleLibraryResolver &Resolver, DWORD dwUNID) const

//  AddLibrary
//
//  Adds a library to the resolver.

    {
    SExtensionDesc *pExtension;
    if (!m_Extensions.Find(dwUNID, &pExtension))
        {
        printf("ERROR: Unable to find library: %08x.\n", dwUNID);
        return;
        }

    Resolver.AddTable(&pExtension->Entities);
    }

bool CExtensionDirectory::CalcRequiredFiles (DWORD dwUNID, TArray<DWORD> &Files) const

//  CalcRequiredFiles
//
//  Returns a list of all files (TDB libraries) used by the given extension

    {
    int i;

    Files.DeleteAll();

    //  Find the root extension.

    SExtensionDesc *pRoot;
    if (!m_Extensions.Find(dwUNID, &pRoot))
        return false;

    //  Clear all marks

    for (i = 0; i < m_Extensions.GetCount(); i++)
        m_Extensions[i]->bMarked = false;

    //  Recursively mark all files used by the given extension.

    if (!MarkRequiredExtensions(pRoot))
        return false;

    //  Return marked files.

    for (i = 0; i < m_Extensions.GetCount(); i++)
        if (m_Extensions[i]->bMarked && m_Extensions[i]->dwUNID != dwUNID)
            Files.Insert(m_Extensions[i]->dwUNID);

    //  Done

    return true;
    }

bool CExtensionDirectory::FindByFilespec (const CString &sFilespec, DWORD *retdwUNID) const

//  FindByFilespec
//
//  Returns the UNID of the extension of the given filespec.

    {
    int i;

    for (i = 0; i < m_Extensions.GetCount(); i++)
        if (strEquals(sFilespec, m_Extensions[i]->sFilespec))
            {
            if (retdwUNID)
                *retdwUNID = m_Extensions[i]->dwUNID;
            return true;
            }

    //  Not found

    return false;
    }

bool CExtensionDirectory::FindLibraryEntities (DWORD dwUNID, CExternalEntityTable **retpTable) const

//  FindLibraryEntities
//
//  Returns entities for the given library. Returns FALSE if we cannot find the
//  library.

    {
    SExtensionDesc *pExtension;
    if (!m_Extensions.Find(dwUNID, &pExtension))
        return false;

    *retpTable = &pExtension->Entities;

    return true;
    }

bool CExtensionDirectory::GetExtensionInfo (DWORD dwUNID, SExtensionInfo &retInfo) const

//  GetExtensionInfo
//
//  Returns information about the given extension.

    {
    int i;

    SExtensionDesc *pExtension;
    if (!m_Extensions.Find(dwUNID, &pExtension))
        return false;

    retInfo.dwUNID = dwUNID;
    retInfo.iType = pExtension->iType;
    retInfo.sName = pExtension->sName;
    retInfo.sFilespec = pExtension->sFilespec;
    retInfo.sVersion = pExtension->sVersion;
    retInfo.dwCoverImage = 0;

    //  Get a list of dependencies

    if (!CalcRequiredFiles(dwUNID, retInfo.Dependencies))
        return false;

    //  Generate a list of all required files

    retInfo.Files.Insert(pExtension->sFilespec);
    for (i = 0; i < retInfo.Dependencies.GetCount(); i++)
        {
        SExtensionDesc *pDependency;
        if (!m_Extensions.Find(retInfo.Dependencies[i], &pDependency))
            return false;

        retInfo.Files.Insert(pDependency->sFilespec);
        }

    return true;
    }

bool CExtensionDirectory::Init (const CString &sRootPath, const CString &sFilespec, CString *retsError)

//  Init
//
//  Initializes the set of TDB files in the given directory.

    {
    CString sError;

    //  Load Transcendence.tdb first.

    if (!InitCore(sRootPath, retsError))
        return false;

	//	Load all extensions in the folder

    CString sPath = pathGetPath(sFilespec);
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

		//	Skip Transcendence.tdb file

		if (strEquals(FileDesc.sFilename, TRANSCENDENCE_TDB))
			continue;

        //	Get path and extension

        CString sFilepath = pathAddComponent(sPath, FileDesc.sFilename);

        //	If this is a folder, then skip

        if (FileDesc.bFolder)
            continue;

		//	Open the file

		CResourceDb ExtDb(sFilepath, true);
		if (ExtDb.Open(DFOPEN_FLAG_READ_ONLY, &sError) != NOERROR)
			{
            //  Report error, but continue

            printf("WARNING: Unable to open %s: %s\n", (LPSTR)sFilepath, (LPSTR)sError);
            continue;
			}

        //  Create an entry

        SExtensionDesc *pExtension = new SExtensionDesc;

        //  Load the root element as a stub.

        CXMLElement *pRoot;
        if (ExtDb.LoadGameFileStub(&pRoot, &pExtension->Entities, &sError) != NOERROR)
            {
            delete pExtension;
            printf("WARNING: Unable to open %s: %s\n", (LPSTR)sFilepath, (LPSTR)sError);
            continue;
            }

        pExtension->dwUNID = pRoot->GetAttributeInteger(ATTRIB_UNID);
        pExtension->sFilespec = sFilepath;
        pExtension->sName = pRoot->GetAttribute(ATTRIB_NAME);
        pExtension->sVersion = pRoot->GetAttribute(ATTRIB_VERSION);

        if (strEquals(pRoot->GetTag(), TRANSCENDENCE_ADVENTURE_TAG))
            pExtension->iType = extAdventure;
        else if (strEquals(pRoot->GetTag(), TRANSCENDENCE_LIBRARY_TAG))
            pExtension->iType = extLibrary;
        else if (strEquals(pRoot->GetTag(), TRANSCENDENCE_EXTENSION_TAG))
            pExtension->iType = extExtension;
        else
            {
            delete pExtension;
            printf("WARNING: Unknown extension type: %s\n", (LPSTR)pRoot->GetTag());
            continue;
            }

        //  Make sure there are no duplicates

        if (m_Extensions.GetAt(pExtension->dwUNID))
            {
            delete pExtension;
            printf("WARNING: Ignoring %s: duplicate UNID.\n", (LPSTR)sFilepath);
            continue;
            }

        //  Add to our list. Once added, we own the object.

        m_Extensions.Insert(pExtension->dwUNID, pExtension);

#ifdef DEBUG
        printf("DEBUG: Loaded %s (%08x)\n", (LPSTR)pExtension->sName, pExtension->dwUNID);
#endif
        }

    return true;
    }

bool CExtensionDirectory::InitCore (const CString &sRootPath, CString *retsError)

//  InitCore
//
//  Initialize core extensions

    {
    int i;
    CString sFilespec = pathAddComponent(sRootPath, TRANSCENDENCE_TDB);

    //  Load Transcendence.xml

	CResourceDb Resources(sFilespec);
    if (Resources.Open(DFOPEN_FLAG_READ_ONLY, retsError) != NOERROR)
        return false;

    //  Create an entry for the core file

    SExtensionDesc *pBase = new SExtensionDesc;

	CXMLElement *pGameFile;
	if (Resources.LoadGameFile(&pGameFile, NULL, retsError, &pBase->Entities) != NOERROR)
		{
		delete pBase;
        return false;
		}

    pBase->dwUNID = 0;
    pBase->sFilespec = sFilespec;
    pBase->sName = CONSTLIT("Transcendence Core");
    pBase->iType = extBase;
    pBase->bCore = true;

    m_Extensions.Insert(0, pBase);

#ifdef DEBUG
    printf("DEBUG: Loaded %s (%08x)\n", (LPSTR)pBase->sName, pBase->dwUNID);
#endif

    //  Look for embedded extensions and create them.

    for (i = 0; i < pGameFile->GetContentElementCount(); i++)
        {
        CXMLElement *pDesc = pGameFile->GetContentElement(i);
		if (strEquals(pDesc->GetTag(), TRANSCENDENCE_ADVENTURE_TAG)
				|| strEquals(pDesc->GetTag(), TRANSCENDENCE_LIBRARY_TAG)
				|| strEquals(pDesc->GetTag(), CORE_LIBRARY_TAG))
            {
            //  We create a new entry for it.

            SExtensionDesc *pCoreLibrary = new SExtensionDesc;

	        //	In some cases we need to load the extension XML from a separate file.
	        //	We need to free this when done.

	        CSimpleLibraryResolver Resolver(*this);

	        //	We also prepare an entity table which will get loaded with any
	        //	entities defined in the embedded extension.

            pCoreLibrary->Entities.SetParent(&pBase->Entities);

	        //	If we have a filespec, then the extension is in a separate file inside
	        //	the base TDB (or XML directory).

	        CString sFilename;
	        if (!pDesc->FindAttribute(ATTRIB_FILENAME, &sFilename))
                {
                delete pCoreLibrary;
                if (retsError) *retsError = CONSTLIT("Expected filename= for core library.");
                return false;
                }

		    //	This extension might refer to other embedded libraries, so we need
		    //	to give it a resolver

            Resolver.AddTable(&pBase->Entities);

		    //	Load the file

		    CString sError;
    	    CXMLElement *pRoot = NULL;
		    if (Resources.LoadEmbeddedGameFile(sFilename, &pRoot, &Resolver, &pCoreLibrary->Entities, &sError))
			    {
			    delete pCoreLibrary;
			    if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to load embedded file: %s"), sError);
			    return false;
			    }

            //  Add more details about the core library

            pCoreLibrary->dwUNID = pRoot->GetAttributeInteger(ATTRIB_UNID);
            pCoreLibrary->sName = pRoot->GetAttribute(ATTRIB_NAME);
            pCoreLibrary->bCore = true;

            if (strEquals(pDesc->GetTag(), TRANSCENDENCE_ADVENTURE_TAG))
                pCoreLibrary->iType = extAdventure;
            else
                pCoreLibrary->iType = extLibrary;

            //  Add it

            m_Extensions.Insert(pCoreLibrary->dwUNID, pCoreLibrary);

#ifdef DEBUG
            printf("DEBUG: Loaded %s (%08x)\n", (LPSTR)pCoreLibrary->sName, pCoreLibrary->dwUNID);
#endif
            }
        }

    //  Success!

    return true;
    }

bool CExtensionDirectory::MarkLibraries (SExtensionDesc *pExtension, CResourceDb &Resources, CXMLElement *pRoot, CSimpleLibraryResolver &Resolver, CString *retsError) const

//  MarkLibraries
//
//  Loops over all design types in pRoot and marks references to libraries.

    {
    int i, j;

    for (i = 0; i < pRoot->GetContentElementCount(); i++)
        {
        CXMLElement *pType = pRoot->GetContentElement(i);
        if (strEquals(pType->GetTag(), LIBRARY_TAG))
            {
            DWORD dwUNID = pType->GetAttributeInteger(ATTRIB_UNID);
            SExtensionDesc *pLibrary;
            if (!m_Extensions.Find(dwUNID, &pLibrary))
                {
                if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to find library %08x."), dwUNID);
                return false;
                }

            if (!MarkRequiredExtensions(pLibrary, retsError))
                return false;
            }
        else if (strEquals(pType->GetTag(), MODULE_TAG))
            {
            if (!MarkModule(pExtension, Resources, pType, Resolver, retsError))
                return false;
            }
        else if (strEquals(pType->GetTag(), MODULES_TAG))
            {
            for (j = 0; j < pType->GetContentElementCount(); j++)
                {
                CXMLElement *pModule = pType->GetContentElement(j);
                if (!MarkModule(pExtension, Resources, pModule, Resolver, retsError))
                    return false;
                }
            }
        }

    return true;
    }

bool CExtensionDirectory::MarkModule (SExtensionDesc *pExtension, CResourceDb &Resources, CXMLElement *pModule, CSimpleLibraryResolver &Resolver, CString *retsError) const

//  MarkModule
//
//  Recurses into a module (in case there are any libraries).

    {
	CString sFilename = pModule->GetAttribute(ATTRIB_FILENAME);

	//	Load the module XML

	CXMLElement *pModuleXML;
    if (Resources.LoadModule(NULL_STR, sFilename, &pModuleXML, retsError) != NOERROR)
        return false;

	if (!strEquals(pModuleXML->GetTag(), TRANSCENDENCE_MODULE_TAG))
		{
		delete pModuleXML;
		if (retsError) *retsError = strPatternSubst(CONSTLIT("Module must have <TranscendenceModule> root element: %s"), sFilename);
        return false;
		}

    //  Recurse

    if (!MarkLibraries(pExtension, Resources, pModuleXML, Resolver, retsError))
        {
        delete pModuleXML;
        return false;
        }

    delete pModuleXML;
    return true;
    }

bool CExtensionDirectory::MarkRequiredExtensions (SExtensionDesc *pExtension, CString *retsError) const

//  MarkRequiredExtensions
//
//  Mark the given extension, and any extensions required.

    {
    //  If we're already marked, then nothing to do.

    if (pExtension->bMarked)
        return true;

    //  If we're a core library, then we don't need to mark (since we don't need
    //  to upload them).

    if (pExtension->bCore)
        return true;

    //  Mark ourselves first

    pExtension->bMarked = true;

    //  Now load the extension.

    CResourceDb Resources(pExtension->sFilespec);
    if (Resources.Open(DFOPEN_FLAG_READ_ONLY, retsError) != NOERROR)
        return false;

    //  Resolver

	CSimpleLibraryResolver Resolver(*this);
    AddCoreLibraries(Resolver, pExtension);
    Resolver.AddTable(&pExtension->Entities);

    //  Load the root file

	CXMLElement *pGameFile;
	if (Resources.LoadGameFile(&pGameFile, &Resolver, retsError, NULL) != NOERROR)
        return false;

    //  Mark all libraries we find

    if (!MarkLibraries(pExtension, Resources, pGameFile, Resolver, retsError))
        {
        delete pGameFile;
        return false;
        }

    //  Done

    delete pGameFile;
    return true;
    }
