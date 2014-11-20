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
    182, 212, 173,   2, 112,  81, 123,  77,  88, 250,
    133, 183, 214, 222,  19, 184, 240, 182,  34, 144,
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
		m_sCollectionFolder(FILESPEC_COLLECTION_FOLDER),
		m_pBase(NULL),
		m_bReloadNeeded(true),
		m_bLoadedInDebugMode(false),
		m_bKeepXML(false)

//	CExtensionCollection constructor

	{
	}

CExtensionCollection::~CExtensionCollection (void)

//	CExtensionCollection destructor

	{
	CleanUp();
	}

void CExtensionCollection::AddOrReplace (CExtension *pExtension)

//	AddOrReplace
//
//	Adds the given extension to the main array and all indices. If this replaces
//	an existing extension (by filespec), the previous one is deleted.

	{
	CSmartLock Lock(m_cs);

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

		pPreviousExtension->SetDeleted();
		m_Deleted.Insert(pPreviousExtension);
		::kernelDebugLogMessage("Replaced extension: %s", pPreviousExtension->GetFilespec());
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

ALERROR CExtensionCollection::AddToBindList (CExtension *pExtension, DWORD dwFlags, const TArray<CExtension *> &Compatibility, TArray<CExtension *> *retList, CString *retsError)

//	AddToBindList
//
//	Recursively adds the extension and any dependencies to the bind list.
//	(We make sure that we don't add the same extension twice.)

	{
	CSmartLock Lock(m_cs);
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

	if (error = pExtension->Load(CExtension::loadComplete, &Resolver, bNoResources, m_bKeepXML, retsError))
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

		if (error = AddToBindList(pLibrary, dwFlags, Compatibility, retList, retsError))
			return error;
		}

	//	Check to see if there are any compatibility libraries that we need to include

	for (i = 0; i < Compatibility.GetCount(); i++)
		if (pExtension->GetAPIVersion() <= Compatibility[i]->GetAutoIncludeAPIVersion()
				&& pExtension->GetUNID() != Compatibility[i]->GetUNID())
			{
			if (error = AddToBindList(Compatibility[i], dwFlags, Compatibility, retList, retsError))
				return error;
			}

	//	Finally add the extension itself.

	if (!pExtension->IsDisabled())
		retList->Insert(pExtension);

	//	Success.

	return NOERROR;
	}

void CExtensionCollection::CleanUp (void)

//	CleanUp
//
//	Clean up and free data

	{
	int i;

	for (i = 0; i < m_Extensions.GetCount(); i++)
		delete m_Extensions[i];

	m_Extensions.DeleteAll();

	FreeDeleted();
	}

void CExtensionCollection::ClearAllMarks (void)

//	ClearAllMarks
//
//	Clear marks on all extensions.

	{
	CSmartLock Lock(m_cs);
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

ALERROR CExtensionCollection::ComputeAvailableExtensions (CExtension *pAdventure, DWORD dwFlags, const TArray<DWORD> &Extensions, TArray<CExtension *> *retList, CString *retsError)

//	ComputeAvailableExtensions
//
//	Fills retList with extension objects for the given extensions by UNID.
//	We only include extensions that are compatible with the given adventure.
//
//	NOTE: An empty Extensions input means we want all extensions.

	{
	CSmartLock Lock(m_cs);

	int i, j;

	//	Initialize

	bool bDebugMode = ((dwFlags & FLAG_DEBUG_MODE) == FLAG_DEBUG_MODE);
	bool bAllExtensions = (Extensions.GetCount() == 0);
	bool bAutoOnly = ((dwFlags & FLAG_AUTO_ONLY) == FLAG_AUTO_ONLY);
	bool bIncludeAuto = bAutoOnly || ((dwFlags & FLAG_INCLUDE_AUTO) == FLAG_INCLUDE_AUTO);

	if (!(dwFlags & FLAG_ACCUMULATE))
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

		//	If this extension is not on our list, then skip it

		if (!bAllExtensions 
				&& !Extensions.Find(ExtensionList[0]->GetUNID())
				&& (!bIncludeAuto || !ExtensionList[0]->IsAutoInclude()))
			continue;

		//	Out of all the releases, select the latest version.

		CExtension *pBest = NULL;
		for (j = 0; j < ExtensionList.GetCount(); j++)
			{
			//	If this is debug only and we're not in debug mode then skip.

			if (ExtensionList[j]->IsDebugOnly() && !bDebugMode)
				continue;

			//	If this is an auto extension, include it only if we ask for it.

			if (ExtensionList[j]->IsAutoInclude() && !bIncludeAuto)
				continue;

			//	If this is not an auto extension, then exclude it if all we want
			//	is auto extensions

			if (!ExtensionList[j]->IsAutoInclude() && bAutoOnly)
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
	DEBUG_TRY

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

	//	Make a list of all compatibility libraries

	TArray<CExtension *> CompatibilityLibraries;
	ComputeCompatibilityLibraries(pAdventure, dwFlags, &CompatibilityLibraries);

	//	We always bind the base extension first

	m_pBase->SetMarked();
	retList->Insert(m_pBase);

	//	Now add the adventure and any dependencies

	if (pAdventure)
		if (error = AddToBindList(pAdventure, dwFlags, CompatibilityLibraries, retList, retsError))
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

		if (error = AddToBindList(DesiredExtensions[i], dwFlags, CompatibilityLibraries, retList, retsError))
			return error;
		}

	return NOERROR;

	DEBUG_CATCH
	}

void CExtensionCollection::ComputeCompatibilityLibraries (CExtension *pAdventure, DWORD dwFlags, TArray<CExtension *> *retList)

//	ComputeCompatibilityLibraries
//
//	Make a list of all libraries that may be auto included for compatibility.

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

		//	We only consider libraries with a compatibility value

		if (ExtensionList[0]->GetType() != extLibrary
				|| ExtensionList[0]->GetAutoIncludeAPIVersion() == 0)
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
		//	Except in the Collection folder, which only allows TDBs

		if (strEquals(sFileExtension, EXTENSION_XML)
				&& iFolder != CExtension::folderCollection)
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
			{
			//	If this is a TDB then ignore the error--we assume that we will try to 
			//	repair it when we open the collection

			if (ExtDb.IsTDB())
				{
				::kernelDebugLogMessage("Unable to load extension: %s", sFilepath);
				continue;
				}

			return error;
			}

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

void CExtensionCollection::DebugDump (void)

//	DebugDump
//
//	Dump information on crash

	{
	CSmartLock Lock(m_cs);
	int i, j;

	::kernelDebugLogMessage("m_pBase:");
	CExtension::DebugDump(m_pBase);

	::kernelDebugLogMessage("m_Extensions:");
	for (i = 0; i < m_Extensions.GetCount(); i++)
		CExtension::DebugDump(m_Extensions[i], true);

	::kernelDebugLogMessage("m_ByUNID:");
	for (i = 0; i < m_ByUNID.GetCount(); i++)
		{
		const TArray<CExtension *> &List = m_ByUNID[i];
		for (j = 0; j < List.GetCount(); j++)
			CExtension::DebugDump(List[j]);
		}

	::kernelDebugLogMessage("m_ByFilespec:");
	for (i = 0; i < m_ByFilespec.GetCount(); i++)
		CExtension::DebugDump(m_ByFilespec[i]);
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
	DEBUG_TRY

	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < m_Deleted.GetCount(); i++)
		delete m_Deleted[i];

	m_Deleted.DeleteAll();

	DEBUG_CATCH
	}

CString CExtensionCollection::GetExternalResourceFilespec (CExtension *pExtension, const CString &sFilename) const

//	GetExternalResourceFilespec
//
//	Returns a filespec to an external resource. NOTE: This is only valid for
//	extensions in the Collection folder.

	{
	ASSERT(pExtension && pExtension->GetFolderType() == CExtension::folderCollection);

	//	We look in a subdirectory of the Collection folder

	CString sExternalFolder = pathAddComponent(m_sCollectionFolder, strPatternSubst(CONSTLIT("%08X"), pExtension->GetUNID()));

	//	Compose the path

	return pathAddComponent(sExternalFolder, sFilename);
	}

bool CExtensionCollection::GetRequiredResources (TArray<CString> *retFilespecs)

//	GetRequiredResources
//
//	Returns a list of resources that we need to download.
//	NOTE: The filespec includes the path where the file is expected to be.

	{
	CSmartLock Lock(m_cs);
	int i, j;

	retFilespecs->DeleteAll();

	//	Loop over all extensions and return a list of missing TDB resources
	//	(generally music files that we download later).

	for (i = 0; i < m_Extensions.GetCount(); i++)
		{
		CExtension *pExtension = m_Extensions[i];
		if (pExtension == NULL
				|| pExtension->GetFolderType() != CExtension::folderCollection
				|| pExtension->GetLoadState() != CExtension::loadComplete
				|| pExtension->IsDisabled()
				|| !pExtension->IsRegistrationVerified())
			continue;

		//	Look for any deferred resources

		const TArray<CString> &Resources = pExtension->GetExternalResources();

		//	If any files don't exist, add them

		for (j = 0; j < Resources.GetCount(); j++)
			{
			if (!::pathExists(Resources[j]))
				{
				retFilespecs->Insert(Resources[j]);
				::kernelDebugLogMessage("Request download: %s", Resources[j]);
				}
			}
		}

	return (retFilespecs->GetCount() > 0);
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

bool CExtensionCollection::IsRegisteredGame (CExtension *pAdventure, const TArray<CExtension *> &DesiredExtensions, DWORD dwFlags)

//	IsRegisteredGame
//
//	Returns TRUE if the given adventure and set of extensions can be used
//	to create a registered game. We check to make sure all extensions and 
//	libraries are registered and verified.

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Compute the full set of extensions that we need to load. This will include 
	//	any libraries.

	TArray<CExtension *> BindOrder;
	CString sError;
	if (ComputeBindOrder(pAdventure, DesiredExtensions, dwFlags, &BindOrder, &sError) != NOERROR)
		return false;

	//	Loop over all extensions and make sure all are verified.

	for (i = 0; i < BindOrder.GetCount(); i++)
		if (!BindOrder[i]->IsRegistrationVerified())
			return false;

	return true;
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

	//	If we don't need to load, then we're done.

	if (!m_bReloadNeeded)
		return NOERROR;

	m_bLoadedInDebugMode = ((dwFlags & FLAG_DEBUG_MODE) == FLAG_DEBUG_MODE);

	//	Load base file

	if (error = LoadBaseFile(sFilespec, dwFlags, retsError))
		return error;

	//	We begin by loading stubs for all extension (i.e., only basic extension
	//	information and entities).

	if (error = LoadFolderStubsOnly(m_sCollectionFolder, CExtension::folderCollection, dwFlags, retsError))
		return error;

	for (i = 0; i < m_ExtensionFolders.GetCount(); i++)
		{
		if (error = LoadFolderStubsOnly(m_ExtensionFolders[i], CExtension::folderExtensions, dwFlags, retsError))
			return error;
		}

	//	Now that we know about all the extensions that we have, continue loading.

	for (i = 0; i < m_Extensions.GetCount(); i++)
		{
		CExtension *pExtension = m_Extensions[i];

		//	Generate a resolver so that we can look up entities. We always add
		//	the base file and the extension itself.

		CLibraryResolver Resolver(*this);
		Resolver.AddLibrary(m_pBase);
		Resolver.AddLibrary(pExtension);

		if (error = pExtension->Load(CExtension::loadAdventureDesc, 
				&Resolver, 
				((dwFlags & FLAG_NO_RESOURCES) == FLAG_NO_RESOURCES), 
				m_bKeepXML, 
				retsError))
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

	//	Always keep the base file XML because we can't tell yet if we need it.

	Ctx.bKeepXML = true;

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

ALERROR CExtensionCollection::LoadFile (const CString &sFilespec, CExtension::EFolderTypes iFolder, DWORD dwFlags, const CIntegerIP &CheckDigest, bool *retbReload, CString *retsError)

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
			m_bKeepXML,
			retsError))
		{
		delete pExtension;
		return error;
		}

	//	Did the load succeed?

	if ((dwFlags & FLAG_ERROR_ON_DISABLE)
			&& pExtension->IsDisabled())
		{
		if (retsError)
			*retsError = strPatternSubst(CONSTLIT("Extension %s: %s"), pExtension->GetFilespec(), pExtension->GetDisabledReason());
		delete pExtension;
		return ERR_FAIL;
		}

	//	Check the digest before we accept it.

	if (pExtension->IsRegistered() 
			&& !CheckDigest.IsEmpty())
		{
		//	NOTE: We set the verification flag even if the file failed to load.
		//	If the digest matches then the file is good--it probably failed because
		//	of a missing library.

		if (pExtension->GetDigest() == CheckDigest)
			pExtension->SetVerified();

		//	Otherwise, we have an error

		else
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Extension file corrupt: %s"), pExtension->GetFilespec());
			delete pExtension;
			return ERR_FAIL;
			}
		}

	//	If we just added a new library, then tell our caller that it might want
	//	to try reloading disabled extensions that rely on this library.

	if (retbReload)
		*retbReload = (pExtension->GetType() == extLibrary 
				&& !pExtension->IsDisabled());

	//	Add the extensions to our list. We lock because we expect this function
	//	to be called without a lock; we don't want to lock while doing the
	//	expensive load operation.

	AddOrReplace(pExtension);

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
		if (iFolder == CExtension::folderCollection)
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

		//	If this extension needs XML, then we remember that so that we keep
		//	XML around after load.

		if (pExtension->UsesXML())
			m_bKeepXML = true;

		//	Add the extensions to our list. We lock because we expect this function
		//	to be called without a lock; we don't want to lock while doing the
		//	expensive load operation.

		AddOrReplace(pExtension);
		}

	return NOERROR;
	}

ALERROR CExtensionCollection::LoadNewExtension (const CString &sFilespec, const CIntegerIP &FileDigest, CString *retsError)

//	LoadNewExtension
//
//	Loads a newly downloaded extension (in the Downloads folder) and moves it to
//	the Collection folder.

	{
	try
		{
		//	NOTE: We don't need to lock because LoadFile will lock appropriately.

		//	Delete the destination filespec

		CString sNewFilespec = pathAddComponent(m_sCollectionFolder, pathGetFilename(sFilespec));
		fileDelete(sNewFilespec);

		//	Generate a destination filespec and move the file.

		if (!fileMove(sFilespec, sNewFilespec))
			{
			*retsError = strPatternSubst(ERR_CANT_MOVE, sFilespec, sNewFilespec);
			return ERR_FAIL;
			}

		//	Load the file

		bool bReload;
		if (LoadFile(sNewFilespec, CExtension::folderCollection, FLAG_DESC_ONLY, FileDigest, &bReload, retsError) != NOERROR)
			return ERR_FAIL;

		//	If necessary, try reloading other extensions that might become enabled after
		//	this new file is loaded. We keep reloading until we've enabled no more
		//	extensions.

		if (bReload)
			{
			while (ReloadDisabledExtensions(FLAG_DESC_ONLY))
				;
			}

		return NOERROR;
		}
	catch (...)
		{
		*retsError = strPatternSubst(CONSTLIT("Crash loading new extension: %s."), sFilespec);
		return ERR_FAIL;
		}
	}

bool CExtensionCollection::ReloadDisabledExtensions (DWORD dwFlags)

//	ReloadDisabledExtensions
//
//	Loops over all disabled extensions and tries to reload any that required a
//	missing library.
//
//	We return TRUE if any extension was enabled in this way. 

	{
	int i;

	struct SEntry
		{
		CString sFilespec;
		CExtension::EFolderTypes iFolder;
		CIntegerIP FileDigest;
		};

	//	First make a list of all extensions that we want to reload

	m_cs.Lock();
	TArray<SEntry> ReloadList;
	for (i = 0; i < m_Extensions.GetCount(); i++)
		{
		CExtension *pExtension = m_Extensions[i];
		if (pExtension->IsDisabled())
			{
			SEntry *pEntry = ReloadList.Insert();
			pEntry->sFilespec = pExtension->GetFilespec();
			pEntry->iFolder = pExtension->GetFolderType();

			//	If the original load was verified, then we can
			//	verify a subsequent load (since it is the same file).

			if (pExtension->IsRegistrationVerified())
				pEntry->FileDigest = pExtension->GetDigest();
			}
		}
	m_cs.Unlock();

	//	Now try reloading each extension. We do this outside the lock because
	//	it is time-consuming. The calls will lock appropriately.

	bool bSuccess = false;
	for (i = 0; i < ReloadList.GetCount(); i++)
		{
		const SEntry &Entry = ReloadList[i];

		//	Load the extension. If we fail, then ignore it--it means we could
		//	not fix the problem.

		if (LoadFile(Entry.sFilespec, 
				Entry.iFolder, 
				FLAG_DESC_ONLY | FLAG_ERROR_ON_DISABLE, 
				Entry.FileDigest, 
				NULL, 
				NULL) != NOERROR)
			continue;

		//	Otherwise, we succeeded at least on one extension

		bSuccess = true;
		}

	//	Done

	return bSuccess;
	}

void CExtensionCollection::SetRegisteredExtensions (const CMultiverseCollection &Collection, TArray<CMultiverseCatalogEntry *> *retNotFound)

//	SetRegisteredExtensions
//
//	Given the user's collection, set the registered bit on all appropriate
//	extensions and return a list of extensions that need to be downloaded.

	{
	CSmartLock Lock(m_cs);
	int i;

	retNotFound->DeleteAll();

	for (i = 0; i < Collection.GetCount(); i++)
		{
		CMultiverseCatalogEntry *pEntry = Collection.GetEntry(i);

		//	Skip core entries

		if (pEntry->GetLicenseType() == CMultiverseCatalogEntry::licenseCore)
			continue;

		//	Look for this extension in our list. If we found it then compare
		//	the signature to make sure that we have the right version.

		CExtension *pExtension;
		if (FindExtension(pEntry->GetUNID(), pEntry->GetRelease(), CExtension::folderCollection, &pExtension))
			{
			//	Compare the digests. If they match, then this is a registered
			//	extension.

			if (pEntry->GetTDBFileRef().GetDigest() == pExtension->GetDigest())
				pExtension->SetVerified();
			
			//	Otherwise we assume that we have an old version and ask to download
			//	the file again.

			else
				retNotFound->Insert(pEntry);
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

void CExtensionCollection::UpdateCollectionStatus (CMultiverseCollection &Collection, int cxIconSize, int cyIconSize)

//	UpdateCollectionStatus
//
//	Updates the local status of all entries in the collection

	{
	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < Collection.GetCount(); i++)
		{
		CMultiverseCatalogEntry *pEntry = Collection.GetEntry(i);

		//	Figure out which folder to look in

		CExtension::EFolderTypes iFolder;
		if (pEntry->GetLicenseType() == CMultiverseCatalogEntry::licenseCore)
			iFolder = CExtension::folderBase;
		else
			iFolder = CExtension::folderCollection;

		//	Look for this extension in our list.

		CExtension *pExtension;
		if (FindExtension(pEntry->GetUNID(), 0, iFolder, &pExtension))
			{
			if (pExtension->IsDisabled())
				pEntry->SetStatus(CMultiverseCatalogEntry::statusError, pExtension->GetDisabledReason());
			else if (pExtension->IsRegistrationVerified())
				pEntry->SetStatus(CMultiverseCatalogEntry::statusLoaded);
			else
				pEntry->SetStatus(CMultiverseCatalogEntry::statusCorrupt);

			//	Set the icon

			CG16bitImage *pIcon;
			pExtension->CreateIcon(cxIconSize, cyIconSize, &pIcon);
			pEntry->SetIcon(pIcon);
			pEntry->SetVersion(pExtension->GetVersion());
			}

		//	If we can't find it, then we know that it's not loaded

		else
			pEntry->SetStatus(CMultiverseCatalogEntry::statusNotAvailable);
		}
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
