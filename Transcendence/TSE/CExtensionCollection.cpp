//	CExtensionCollection.cpp
//
//	CExtensionCollection class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define LIBRARY_TAG									CONSTLIT("Library")
#define TRANSCENDENCE_ADVENTURE_TAG					CONSTLIT("TranscendenceAdventure")
#define TRANSCENDENCE_EXTENSION_TAG					CONSTLIT("TranscendenceExtension")
#define TRANSCENDENCE_LIBRARY_TAG					CONSTLIT("TranscendenceLibrary")
#define TRANSCENDENCE_MODULE_TAG					CONSTLIT("TranscendenceModule")

#define RELEASE_ATTRIB								CONSTLIT("release")
#define UNID_ATTRIB									CONSTLIT("unid")

#define EXTENSIONS_FILTER							CONSTLIT("*.*")
#define EXTENSION_TDB								CONSTLIT("tdb")
#define EXTENSION_XML								CONSTLIT("xml")

#define FILESPEC_COLLECTION_FOLDER					CONSTLIT("Collection")
#define FILESPEC_EXTENSIONS_FOLDER					CONSTLIT("Extensions")

#define ERR_CANT_MOVE								CONSTLIT("%s: Unable to move to %s.")

const int DIGEST_SIZE = 20;
static BYTE g_BaseFileDigest[] =
	{
	209, 165,  91, 183,  31,  61, 117, 161, 100,  38,
	147,  20, 226, 197,  34,  31,  59,  20,  75,  44,
	};

class CLibraryResolver : public IXMLParserController
	{
	public:
		CLibraryResolver (CExtensionCollection &Extensions) : 
				m_Extensions(Extensions),
				m_bReportError(false)
			{ }

		inline void AddLibrary (CExtension *pLibrary) { m_Libraries.Insert(pLibrary); }
		inline void ReportLibraryErrors (void) { m_bReportError = true; }

		//	IXMLParserController virtuals
		virtual ALERROR OnOpenTag (CXMLElement *pElement, CString *retsError);
		virtual CString ResolveExternalEntity (const CString &sName, bool *retbFound = NULL);

	private:
		CExtensionCollection &m_Extensions;

		TArray<CExtension *> m_Libraries;
		bool m_bReportError;				//	If TRUE, we report errors if we fail to load a library
	};

CExtensionCollection::CExtensionCollection (void) :
		m_pBase(NULL),
		m_bReloadNeeded(true),
		m_bLoadedInDebugMode(false)

//	CExtensionCollection constructor

	{
	}

CExtensionCollection::~CExtensionCollection (void)

//	CExtensionCollection destructor

	{
	int i;

	for (i = 0; i < m_Extensions.GetCount(); i++)
		delete m_Extensions[i];

	FreeDeleted();
	}

void CExtensionCollection::AddOrReplace (CExtension *pExtension)

//	AddOrReplace
//
//	Adds the given extension to the main array and all indices. If this replaces
//	an existing extension (by filespec), the previous one is deleted.

	{
	//	First see if we're replacing this extension (we only check for non-base
	//	extensions).

	CExtension *pPreviousExtension;
	if (pExtension->GetFolderType() != CExtension::folderBase
			&& m_ByFilespec.Find(pExtension->GetFilespec(), &pPreviousExtension))
		{
		//	Remove from UNID index

		int iPos;
		if (m_ByUNID.FindPos(pPreviousExtension->GetUNID(), &iPos))
			{
			TArray<CExtension *> &List = m_ByUNID.GetValue(iPos);

			int iIndex;
			if (List.Find(pPreviousExtension, &iIndex))
				List.Delete(iIndex);
			}

		//	Remove from main list

		int iIndex;
		if (m_Extensions.Find(pPreviousExtension, &iIndex))
			m_Extensions.Delete(iIndex);

		//	Add to deleted list. We can't delete outright because its 
		//	structures may have been bound by CDesignCollection.

		m_Deleted.Insert(pPreviousExtension);
		}

	//	Insert

	m_Extensions.Insert(pExtension);

	//	Base index

	if (pExtension->GetUNID() == 0)
		m_pBase = pExtension;

	//	UNID index

	TArray<CExtension *> *pList = m_ByUNID.SetAt(pExtension->GetUNID());
	pList->Insert(pExtension);

	//	Filespec index
	//
	//	NOTE: We don't add base extensions to the list because otherwise they 
	//	would conflict. (E.g., Transcendence.xml could have multiple adventure 
	//	extensions).

	if (pExtension->GetFolderType() != CExtension::folderBase)
		m_ByFilespec.SetAt(pExtension->GetFilespec(), pExtension);
	}

ALERROR CExtensionCollection::AddToBindList (CExtension *pExtension, DWORD dwFlags, TArray<CExtension *> *retList, CString *retsError)

//	AddToBindList
//
//	Recursively adds the extension and any dependencies to the bind list.
//	(We make sure that we don't add the same extension twice.)

	{
	ALERROR error;
	int i;

	//	If we're already marked, then we're already in the list, so we don't
	//	need to continue.

	if (pExtension->IsMarked())
		return NOERROR;

	bool bNoResources = ((dwFlags & FLAG_NO_RESOURCES) == FLAG_NO_RESOURCES);

	//	Mark now in case there is a circular dependency (in that case, we will
	//	ignore the circular dependency.)

	pExtension->SetMarked();

	//	Create a resolver

	CLibraryResolver Resolver(*this);
	Resolver.AddLibrary(m_pBase);
	Resolver.AddLibrary(pExtension);
	Resolver.ReportLibraryErrors();

	//	Make sure the extension is loaded completely.

	if (error = pExtension->Load(CExtension::loadComplete, &Resolver, bNoResources, retsError))
		return error;

	//	Now add any libraries used by this extension to the list.

	for (i = 0; i < pExtension->GetLibraryCount(); i++)
		{
		const CExtension::SLibraryDesc &LibraryDesc = pExtension->GetLibrary(i);

		//	Find the extension

		CExtension *pLibrary;
		if (!FindBestExtension(LibraryDesc.dwUNID, LibraryDesc.dwRelease, dwFlags, &pLibrary))
			{
			//	If we can't find the library, disable the extension with the appropriate message,
			//	but otherwise continue loading.

			pExtension->SetDisabled(strPatternSubst(CONSTLIT("Cannot find library: %08x"), LibraryDesc.dwUNID));
			continue;
			}

		//	If the library is disabled, then the extension is also disabled.

		if (pLibrary->IsDisabled())
			{
			pExtension->SetDisabled(strPatternSubst(CONSTLIT("Required library disabled: %s (%08x)"), pLibrary->GetName(), pLibrary->GetUNID()));
			continue;
			}

		//	Make sure it is a library

		if (pLibrary->GetType() != extLibrary)
			{
			*retsError = strPatternSubst(CONSTLIT("UNID referenced by %s (%08x) is not a library: %08x"), pExtension->GetName(), pExtension->GetUNID(), LibraryDesc.dwUNID);
			return ERR_FAIL;
			}

		//	Add it to the bind list (recursively)

		if (error = AddToBindList(pLibrary, dwFlags, retList, retsError))
			return error;
		}

	//	Finally add the extension itself.

	if (!pExtension->IsDisabled())
		retList->Insert(pExtension);

	//	Success.

	return NOERROR;
	}

void CExtensionCollection::ClearAllMarks (void)

//	ClearAllMarks
//
//	Clear marks on all extensions.

	{
	int i;

	for (i = 0; i < m_Extensions.GetCount(); i++)
		m_Extensions[i]->SetMarked(false);
	}

int CExtensionCollection::Compare (CExtension *pExt1, CExtension *pExt2, bool bDebugMode)

//	Compare
//
//	If pExt1 overrides pExt2 then we return 1. Otherwise we return -1.
//	(If neither overrides the other we return 0).

	{
	//	If either is NULL then non-NULL wins

	if (pExt1 == NULL && pExt2 == NULL)
		return 0;
	else if (pExt1 == NULL)
		return -1;
	else if (pExt2 == NULL)
		return 1;

	//	A later release always wins

	if (pExt1->GetRelease() > pExt2->GetRelease())
		return 1;

	//	A lesser release always loses

	else if (pExt1->GetRelease() < pExt2->GetRelease())
		return -1;

	//	If we're the same release then we must be in different folders. In that
	//	case see which folder wins

	else
		{
		//	If the same folder, then we're even

		if (pExt1->GetFolderType() == pExt2->GetFolderType())
			return 0;

		//	In debug mode, the Extensions folder wins

		else if (bDebugMode)
			{
			if (pExt1->GetFolderType() == CExtension::folderExtensions)
				return 1;
			else if (pExt2->GetFolderType() == CExtension::folderExtensions)
				return -1;
			else
				return 0;
			}

		//	In non-debug, the Collection folder wins

		else
			{
			if (pExt1->GetFolderType() == CExtension::folderCollection)
				return 1;
			else if (pExt2->GetFolderType() == CExtension::folderCollection)
				return -1;
			else
				return 0;
			}
		}
	}

ALERROR CExtensionCollection::ComputeAvailableAdventures (DWORD dwFlags, TArray<CExtension *> *retList, CString *retsError)

//	ComputeAvailableAdventures
//
//	Computes a list of all available adventures.

	{
	CSmartLock Lock(m_cs);

	int i, j;

	//	Initialize

	bool bDebugMode = ((dwFlags & FLAG_DEBUG_MODE) == FLAG_DEBUG_MODE);
	retList->DeleteAll();

	//	Loop by UNID because we allow at most one of each UNID.

	for (i = 0; i < m_ByUNID.GetCount(); i++)
		{
		TArray<CExtension *> &ExtensionList = m_ByUNID.GetValue(i);
		if (ExtensionList.GetCount() == 0)
			continue;

		//	We only consider adventures (this property does not change across
		//	releases, so we only need to check once).

		if (ExtensionList[0]->GetType() != extAdventure)
			continue;

		//	Out of all the releases, select the latest version.

		CExtension *pBest = NULL;
		for (j = 0; j < ExtensionList.GetCount(); j++)
			{
			//	If this is debug only and we're not in debug mode then skip.

			if (ExtensionList[j]->IsDebugOnly() && !bDebugMode)
				continue;

			//	Compute the best extension out of the list.

			if (Compare(ExtensionList[j], pBest, bDebugMode) == 1)
				pBest = ExtensionList[j];
			}

		//	If we found one then add it to the list.

		if (pBest)
			retList->Insert(pBest);
		}

	//	Done

	return NOERROR;
	}

ALERROR CExtensionCollection::ComputeAvailableExtensions (CExtension *pAdventure, DWORD dwFlags, TArray<CExtension *> *retList, CString *retsError)

//	ComputeAvailableExtensions
//
//	Computes a list of all extensions that can be used with the given adventure.
//	(pAdventure may be NULL)

	{
	CSmartLock Lock(m_cs);

	int i, j;

	//	Initialize

	bool bDebugMode = ((dwFlags & FLAG_DEBUG_MODE) == FLAG_DEBUG_MODE);
	retList->DeleteAll();

	//	Loop by UNID because we allow at most one of each UNID.

	for (i = 0; i < m_ByUNID.GetCount(); i++)
		{
		TArray<CExtension *> &ExtensionList = m_ByUNID.GetValue(i);
		if (ExtensionList.GetCount() == 0)
			continue;

		//	We only consider extensions (this property does not change across
		//	releases, so we only need to check once).

		if (ExtensionList[0]->GetType() != extExtension)
			continue;

		//	Out of all the releases, select the latest version.

		CExtension *pBest = NULL;
		for (j = 0; j < ExtensionList.GetCount(); j++)
			{
			//	If this is debug only and we're not in debug mode then skip.

			if (ExtensionList[j]->IsDebugOnly() && !bDebugMode)
				continue;

			//	If this extension does not extend the adventure, then skip.

			if (pAdventure && !ExtensionList[j]->CanExtend(pAdventure))
				continue;

			//	Compute the best extension out of the list.

			if (Compare(ExtensionList[j], pBest, bDebugMode) == 1)
				pBest = ExtensionList[j];
			}

		//	If we found one then add it to the list.

		if (pBest)
			retList->Insert(pBest);
		}

	//	Done

	return NOERROR;
	}

ALERROR CExtensionCollection::ComputeBindOrder (CExtension *pAdventure, 
												const TArray<CExtension *> &DesiredExtensions, 
												DWORD dwFlags, 
												TArray<CExtension *> *retList, 
												CString *retsError)

//	ComputeBindOrder
//
//	Generates an ordered list of extensions to bind for the given adventure.
//	(In the order in which they should be bound.)
//
//	DesiredExtensions is either a list of extensions selected by the user
//	(at game create time) or the set of extensions stored in the game file.
//
//	We return an error if we cannot compute the list for any reason (e.g., if
//	a required extension is missing).

	{
	CSmartLock Lock(m_cs);

	ALERROR error;
	int i;

	ASSERT(pAdventure);
	ASSERT(pAdventure->GetType() == extAdventure);

	//	Initialize

	retList->DeleteAll();

	//	Clear marks; we will used marks to determine when we've already added
	//	an extension to the list.

	ClearAllMarks();

	//	We always bind the base extension first

	m_pBase->SetMarked();
	retList->Insert(m_pBase);

	//	Now add the adventure and any dependencies

	if (pAdventure)
		if (error = AddToBindList(pAdventure, dwFlags, retList, retsError))
			return error;

	//	Now add all of the desired extensions.

	for (i = 0; i < DesiredExtensions.GetCount(); i++)
		{
		//	NOTE: We cannot add adventures, so check that here. (This should
		//	never happen since the list of desired extensions should come
		//	from us).

		if (DesiredExtensions[i]->GetType() == extAdventure)
			{
			*retsError = CONSTLIT("Cannot have more than one adventure.");
			return ERR_FAIL;
			}

		//	Add it and all dependencies

		if (error = AddToBindList(DesiredExtensions[i], dwFlags, retList, retsError))
			return error;
		}

	return NOERROR;
	}

ALERROR CExtensionCollection::ComputeFilesToLoad (const CString &sFilespec, CExtension::EFolderTypes iFolder, TSortMap<CString, int> &List, CString *retsError)

//	ComputeFilesToLoad
//
//	Initializes a list with the set of files to load in the given folder.

	{
	ALERROR error;

	//	Load all extensions in the folder

	CFileDirectory Dir(pathAddComponent(sFilespec, EXTENSIONS_FILTER));
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

		CString sFilepath = pathAddComponent(sFilespec, FileDesc.sFilename);
		CString sFileExtension = pathGetExtension(sFilepath);
		CString sExtensionRoot = pathStripExtension(sFilepath);

		//	If this is a folder, then recurse

		if (FileDesc.bFolder)
			{
			if (error = ComputeFilesToLoad(sFilepath, iFolder, List, retsError))
				return error;

			continue;
			}

		//	If this is an XML file then we overwrite a TDB file of the same name.

		if (strEquals(sFileExtension, EXTENSION_XML))
			{
			CString sTDBFilespec = strPatternSubst(CONSTLIT("%s.tdb"), sExtensionRoot);

			int iPos;
			if (List.FindPos(sTDBFilespec, &iPos))
				List.Delete(iPos);
			}

		//	If this is a TDB file, then add only if there isn't an XML file of the
		//	same name.

		else if (strEquals(sFileExtension, EXTENSION_TDB))
			{
			CString sXMLFilespec = strPatternSubst(CONSTLIT("%s.xml"), sExtensionRoot);

			if (List.FindPos(sXMLFilespec))
				continue;
			}

		//	We ignore all other extensions

		else
			continue;

		//	Open the file

		CResourceDb ExtDb(sFilepath, true);
		if (error = ExtDb.Open(DFOPEN_FLAG_READ_ONLY, retsError))
			return error;

		//	If this is a module, then skip it

		CString sDOCTYPERootTag = ExtDb.GetRootTag();
		if (strEquals(sDOCTYPERootTag, TRANSCENDENCE_MODULE_TAG))
			continue;
		else if (!strEquals(sDOCTYPERootTag, TRANSCENDENCE_EXTENSION_TAG)
				&& !strEquals(sDOCTYPERootTag, TRANSCENDENCE_ADVENTURE_TAG)
				&& !strEquals(sDOCTYPERootTag, TRANSCENDENCE_LIBRARY_TAG))
			{
			*retsError = strPatternSubst(CONSTLIT("%s: Expected <TranscendenceAdventure>, <TranscendenceExtension>, <TranscendenceLibrary> or <TranscendenceModule> instead of <%s>"), 
					sFilepath,
					sDOCTYPERootTag);
			return ERR_FAIL;
			}

		//	Add the full filepath to our list

		List.Insert(sFilepath);
		}

	return NOERROR;
	}

bool CExtensionCollection::FindAdventureFromDesc (DWORD dwUNID, DWORD dwFlags, CExtension **retpExtension)

//	FindAdventureFromDesc
//
//	Look for the adventure that has the given adventure descriptor.

	{
	CSmartLock Lock(m_cs);
	int i;

	bool bDebugMode = ((dwFlags & FLAG_DEBUG_MODE) == FLAG_DEBUG_MODE);

	//	Look for the adventure

	CExtension *pBest = NULL;
	for (i = 0; i < m_Extensions.GetCount(); i++)
		{
		CExtension *pExtension = m_Extensions[i];

		//	If not the right adventure then skip

		if (pExtension->GetAdventureDesc() == NULL || pExtension->GetAdventureDesc()->GetUNID() != dwUNID)
			continue;

		//	If this is debug only and we're not in debug mode then skip.

		if (pExtension->IsDebugOnly() && !bDebugMode)
			continue;

		//	Compute the best extension out of the list.

		if (pBest == NULL || (pBest->GetUNID() == pExtension->GetUNID() && Compare(pExtension, pBest, bDebugMode) == 1))
			pBest = pExtension;
		}

	//	Not found

	if (pBest == NULL)
		return false;

	//	Found

	if (retpExtension)
		*retpExtension = pBest;

	return true;
	}

bool CExtensionCollection::FindBestExtension (DWORD dwUNID, DWORD dwRelease, DWORD dwFlags, CExtension **retpExtension)

//	FindBestExtension
//
//	Look for the extension that meets the above criteria.

	{
	CSmartLock Lock(m_cs);
	int i;

	bool bDebugMode = ((dwFlags & FLAG_DEBUG_MODE) == FLAG_DEBUG_MODE);

	int iPos;
	if (!m_ByUNID.FindPos(dwUNID, &iPos))
		return false;

	CExtension *pBest = NULL;
	TArray<CExtension *> &List = m_ByUNID.GetValue(iPos);
	for (i = 0; i < List.GetCount(); i++)
		{
		//	If this is debug only and we're not in debug mode then skip.

		if (List[i]->IsDebugOnly() && !bDebugMode)
			continue;

		//	If we're looking for a specific release, then check

		if (dwRelease && dwRelease != List[i]->GetRelease())
			continue;

		//	Compute the best extension out of the list.

		if (Compare(List[i], pBest, bDebugMode) == 1)
			pBest = List[i];
		}

	//	Not found

	if (pBest == NULL)
		return false;

	//	If we're looking for an adventure and this is not it, then fail
	//
	//	NOTE: We assume that two extensions with the same UNID are always of the
	//	same type.

	if ((dwFlags & FLAG_ADVENTURE_ONLY) && pBest->GetType() != extAdventure)
		return false;

	//	Found

	if (retpExtension)
		*retpExtension = pBest;

	return true;
	}

bool CExtensionCollection::FindExtension (DWORD dwUNID, DWORD dwRelease, CExtension::EFolderTypes iFolder, CExtension **retpExtension)

//	FindExtension
//
//	Finds the given extension. If dwRelease is 0 then we return the latest
//	release. If iFolder is folderUnknown then we return either folder.

	{
	CSmartLock Lock(m_cs);
	int i;

	int iPos;
	if (!m_ByUNID.FindPos(dwUNID, &iPos))
		return false;

	CExtension *pBest = NULL;
	TArray<CExtension *> &List = m_ByUNID.GetValue(iPos);
	for (i = 0; i < List.GetCount(); i++)
		{
		//	Look in the proper folder

		if (iFolder != CExtension::folderUnknown && List[i]->GetFolderType() != iFolder)
			continue;

		//	Look for the appropriate release

		if (dwRelease != 0 && List[i]->GetRelease() != dwRelease)
			continue;

		//	Pick the best one so far

		if (pBest == NULL)
			pBest = List[i];
		else
			{
			if (List[i]->GetRelease() > pBest->GetRelease())
				pBest = List[i];
			}
		}

	//	Not found

	if (pBest == NULL)
		return false;

	//	Found

	if (retpExtension)
		*retpExtension = pBest;

	return true;
	}

void CExtensionCollection::FreeDeleted (void)

//	FreeDeleted
//
//	Frees deleted list

	{
	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < m_Deleted.GetCount(); i++)
		delete m_Deleted[i];

	m_Deleted.DeleteAll();
	}

void CExtensionCollection::InitEntityResolver (CExtension *pExtension, DWORD dwFlags, CEntityResolverList *retResolver)

//	InitEntityResolver
//
//	Initializes an entity resolver for the given extension.

	{
	int i;

	//	Base extension is always added first

	retResolver->AddResolver(m_pBase->GetEntities());

	//	Next we add any libraries used by the extension

	for (i = 0; i < pExtension->GetLibraryCount(); i++)
		{
		const CExtension::SLibraryDesc &LibraryDesc = pExtension->GetLibrary(i);

		CExtension *pLibrary;
		if (!FindBestExtension(LibraryDesc.dwUNID, LibraryDesc.dwRelease, dwFlags, &pLibrary))
			continue;

		retResolver->AddResolver(pLibrary->GetEntities());
		}

	//	Finally, add entities defined in the extension

	retResolver->AddResolver(pExtension->GetEntities());
	}

ALERROR CExtensionCollection::Load (const CString &sFilespec, DWORD dwFlags, CString *retsError)

//	Load
//
//	Loads all extension files. This may be called at any time from any thread
//	to load or reload files on disk. It will only load new or modified files.

	{
	CSmartLock Lock(m_cs);
	ALERROR error;
	int i;

	CString sPath = pathGetPath(sFilespec);

	//	If we don't need to load, then we're done.

	if (!m_bReloadNeeded)
		return NOERROR;

	m_bLoadedInDebugMode = ((dwFlags & FLAG_DEBUG_MODE) == FLAG_DEBUG_MODE);

	//	Load base file

	if (error = LoadBaseFile(sFilespec, dwFlags, retsError))
		return error;

	//	We begin by loading stubs for all extension (i.e., only basic extension
	//	information and entities).

	if (error = LoadFolderStubsOnly(pathAddComponent(sPath, FILESPEC_COLLECTION_FOLDER), CExtension::folderCollection, dwFlags, retsError))
		return error;

	if (error = LoadFolderStubsOnly(pathAddComponent(sPath, FILESPEC_EXTENSIONS_FOLDER), CExtension::folderExtensions, dwFlags, retsError))
		return error;

	//	Now that we know about all the extensions that we have, continue loading.

	for (i = 0; i < m_Extensions.GetCount(); i++)
		{
		CExtension *pExtension = m_Extensions[i];

		//	Generate a resolver so that we can look up entities. We always add
		//	the base file and the extension itself.

		CLibraryResolver Resolver(*this);
		Resolver.AddLibrary(m_pBase);
		Resolver.AddLibrary(pExtension);

		if (error = pExtension->Load(CExtension::loadAdventureDesc, &Resolver, ((dwFlags & FLAG_NO_RESOURCES) == FLAG_NO_RESOURCES), retsError))
			return error;
		}

	//	Done

	m_bReloadNeeded = false;

	return NOERROR;
	}

ALERROR CExtensionCollection::LoadBaseFile (const CString &sFilespec, DWORD dwFlags, CString *retsError)

//	LoadBaseFile
//
//	Loads the base file. This is stored as an extension of UNID = 0.

	{
	ALERROR error;
	int i;

	//	If already loaded, nothing to do

	if (m_pBase)
		return NOERROR;

	//	Open up the file

	CResourceDb Resources(sFilespec);
	if (error = Resources.Open(DFOPEN_FLAG_READ_ONLY, retsError))
		return error;

	//	Log whether or not we're using the XML or TDB files.

	if (Resources.IsUsingExternalGameFile())
		kernelDebugLogMessage("Using external Transcendence.xml");

	if (Resources.IsUsingExternalResources())
		kernelDebugLogMessage("Using external resource files");

	//	Load the main XML file. Since this is the base file we don't need any
	//	additional entities. (But we do get a copy of the entities).

	CExternalEntityTable *pEntities = new CExternalEntityTable;
	CXMLElement *pGameFile;
	if (error = Resources.LoadGameFile(&pGameFile, NULL, retsError, pEntities))
		{
		delete pEntities;
		return error;
		}

	//	Create a load context

	SDesignLoadCtx Ctx;
	Ctx.sResDb = sFilespec;
	Ctx.pResDb = &Resources;
	Ctx.bNoResources = ((dwFlags & FLAG_NO_RESOURCES) ? true : false);
	Ctx.bNoVersionCheck = true;	//	Obsolete now
	Ctx.sErrorFilespec = sFilespec;

	//	Load it

	TArray<CExtension *> ExtensionsCreated;
	error = CExtension::CreateBaseFile(Ctx, pGameFile, pEntities, &ExtensionsCreated);

	//	Compare signature against what we expect. If valid then set 
	//	verification.
	//
	//	NOTE: Only TDB is verified; the XML is always considered unregistered.

	CIntegerIP Digest;
	Resources.ComputeFileDigest(&Digest);

	CIntegerIP CorrectDigest(DIGEST_SIZE, g_BaseFileDigest);

	if (Digest == CorrectDigest)
		{
		//	Everything we loaded is verified

		for (i = 0; i < ExtensionsCreated.GetCount(); i++)
			ExtensionsCreated[i]->SetVerified(true);
		}

	//	Clean up

	delete pGameFile;

	//	Error

	if (error)
		{
		delete pEntities;
		return CExtension::ComposeLoadError(Ctx, retsError);
		}

	//	Add the extensions to our list

	ASSERT(ExtensionsCreated.GetCount() > 0);

	for (i = 0; i < ExtensionsCreated.GetCount(); i++)
		AddOrReplace(ExtensionsCreated[i]);

	return NOERROR;
	}

ALERROR CExtensionCollection::LoadFile (const CString &sFilespec, CExtension::EFolderTypes iFolder, DWORD dwFlags, CString *retsError)

//	LoadFile
//
//	Loads the given extension file.

	{
	ALERROR error;

	//	Create the extension

	CExtension *pExtension;
	if (error = CExtension::CreateExtensionStub(sFilespec, iFolder, &pExtension, retsError))
		return error;

	//	Generate a resolver so that we can look up entities. We always add
	//	the base file and the extension itself.

	CLibraryResolver Resolver(*this);
	Resolver.AddLibrary(m_pBase);
	Resolver.AddLibrary(pExtension);

	//	Load it

	if (error = pExtension->Load(((dwFlags & FLAG_DESC_ONLY) ? CExtension::loadAdventureDesc : CExtension::loadComplete),
			&Resolver,
			((dwFlags & FLAG_NO_RESOURCES) == FLAG_NO_RESOURCES),
			retsError))
		{
		delete pExtension;
		return error;
		}

	//	Add the extensions to our list. We lock because we expect this function
	//	to be called without a lock; we don't want to lock while doing the
	//	expensive load operation.

	m_cs.Lock();
	AddOrReplace(pExtension);
	m_cs.Unlock();

	//	Done

	return NOERROR;
	}

ALERROR CExtensionCollection::LoadFolderStubsOnly (const CString &sFilespec, CExtension::EFolderTypes iFolder, DWORD dwFlags, CString *retsError)

//	LoadFolder
//
//	Loads all the extensions in the given folder.

	{
	ALERROR error;
	int i;

	//	Make sure the folder exists; if not, create it.

	if (!pathExists(sFilespec))
		{
		pathCreate(sFilespec);
		return NOERROR;
		}

	//	Make a list of all the files that we want to load.

	TSortMap<CString, int> FilesToLoad;
	if (error = ComputeFilesToLoad(sFilespec, iFolder, FilesToLoad, retsError))
		return error;

	//	Now loop over the files and load them (if necessary)

	for (i = 0; i < FilesToLoad.GetCount(); i++)
		{
		const CString &sExtensionFilespec = FilesToLoad.GetKey(i);

		//	If this file is already loaded then we don't need to do anything

		CExtension *pExtension;
		if (m_ByFilespec.Find(sExtensionFilespec, &pExtension))
			{
			CTimeDate FileTime = fileGetModifiedTime(sExtensionFilespec);

			//	If the file has not changed then skip this file.

			if (FileTime.Compare(pExtension->GetModifiedTime()) == 0)
				continue;
			}

		//	Create the extension

		if (error = CExtension::CreateExtensionStub(sExtensionFilespec, iFolder, &pExtension, retsError))
			return error;

		//	Add the extensions to our list. We lock because we expect this function
		//	to be called without a lock; we don't want to lock while doing the
		//	expensive load operation.

		m_cs.Lock();
		AddOrReplace(pExtension);
		m_cs.Unlock();
		}

	return NOERROR;
	}

ALERROR CExtensionCollection::LoadNewExtension (const CString &sFilespec, CString *retsError)

//	LoadNewExtension
//
//	Loads a newly downloaded extension (in the Downloads folder) and moves it to
//	the Collection folder.

	{
	//	NOTE: We don't need to lock because LoadFile will lock appropriately.

	//	Delete the destination filespec

	CString sNewFilespec = pathAddComponent(FILESPEC_COLLECTION_FOLDER, pathGetFilename(sFilespec));
	fileDelete(sNewFilespec);

	//	Generate a destination filespec and move the file.

	if (!fileMove(sFilespec, sNewFilespec))
		{
		*retsError = strPatternSubst(ERR_CANT_MOVE, sFilespec, sNewFilespec);
		return ERR_FAIL;
		}

	//	Load the file

	if (LoadFile(sNewFilespec, CExtension::folderCollection, FLAG_DESC_ONLY, retsError) != NOERROR)
		return ERR_FAIL;

	return NOERROR;
	}

void CExtensionCollection::SetRegisteredExtensions (const CMultiverseCollection &Collection, TArray<CMultiverseCatalogEntry *> *retNotFound)

//	SetRegisteredExtensions
//
//	Given the user's collection, set the registered bit on all appropriate
//	extensions.

	{
	CSmartLock Lock(m_cs);
	int i;

	retNotFound->DeleteAll();

	for (i = 0; i < Collection.GetCount(); i++)
		{
		CMultiverseCatalogEntry *pEntry = Collection.GetEntry(i);

		//	Look for this extension in our list. If we found it then compare
		//	the signature to make sure that we have the right version.

		CExtension *pExtension;
		if (FindExtension(pEntry->GetUNID(), pEntry->GetRelease(), CExtension::folderCollection, &pExtension))
			{
			//	LATER: Compare the digest...
			}

		//	If we did not find the extension, then add it to the list of entries that
		//	we need to download.

		else
			{
			retNotFound->Insert(pEntry);
			}
		}
	}

void CExtensionCollection::SweepImages (void)

//	SweepImages
//
//	Frees images that we're no longer using.

	{
	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < m_Extensions.GetCount(); i++)
		m_Extensions[i]->SweepImages();
	}

//	CLibraryResolver -----------------------------------------------------------

ALERROR CLibraryResolver::OnOpenTag (CXMLElement *pElement, CString *retsError)

//	OnOpenTag
//
//	Parses a <Library> element

	{
	if (strEquals(pElement->GetTag(), LIBRARY_TAG))
		{
		DWORD dwUNID = pElement->GetAttributeInteger(UNID_ATTRIB);
		DWORD dwRelease = pElement->GetAttributeInteger(RELEASE_ATTRIB);

		//	Get the best extension with this UNID. If we don't find it, then
		//	continue (we will report an error later when we can't find
		//	the entity).

		CExtension *pLibrary;
		if (!m_Extensions.FindBestExtension(dwUNID, dwRelease, (m_Extensions.LoadedInDebugMode() ? CExtensionCollection::FLAG_DEBUG_MODE : 0), &pLibrary))
			{
			*retsError = strPatternSubst(CONSTLIT("Unable to find library: %08x"), dwUNID);
			return ERR_FAIL;
			}

		//	Is this a library?

		if (pLibrary->GetType() != extLibrary)
			{
			*retsError = strPatternSubst(CONSTLIT("Expected %s (%08x) to be a library"), pLibrary->GetName(), pLibrary->GetUNID());
			return ERR_FAIL;
			}

		//	Must at least have stubs

		if (pLibrary->GetLoadState() == CExtension::loadNone)
			{
			*retsError = strPatternSubst(CONSTLIT("Unable to find library: %08x"), dwUNID);
			return ERR_FAIL;
			}

		//	Add it to our list so that we can use it to resolve entities.

		AddLibrary(pLibrary);
		}

	return NOERROR;
	}

CString CLibraryResolver::ResolveExternalEntity (const CString &sName, bool *retbFound)

//	ResolveExternalEntity
//
//	Resolves an entity.

	{
	int i;

	for (i = 0; i < m_Libraries.GetCount(); i++)
		{
		bool bFound;
		CString sResult = m_Libraries[i]->GetEntities()->ResolveExternalEntity(sName, &bFound);
		if (bFound)
			{
			if (retbFound)
				*retbFound = true;
			return sResult;
			}
		}

	//	Not found

	if (retbFound)
		*retbFound = false;

	return NULL_STR;
	}
