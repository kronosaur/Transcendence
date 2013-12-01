//	CResourceDb.cpp
//
//	CResourceDb class
//	Copyright (c) 2003 by George Moromisato. All Rights Reserved.
//
//	For TDB files the format of the default entry is:
//
//	DWORD		'TRDB'
//	DWORD		version (11)
//	DWORD		game file entry ID
//	CString		game title
//	CString		resource table (flattened CSymbolTable)

#include "PreComp.h"
#include "Zip.h"

#define TDB_SIGNATURE							'TRDB'
#define TDB_VERSION								12

#define FILE_TYPE_XML							CONSTLIT("xml")
#define FILE_TYPE_TDB							CONSTLIT("tdb")
#define RESOURCES_FOLDER						CONSTLIT("Resources")

CResourceDb::CResourceDb (const CString &sFilespec, bool bExtension) : 
		m_sFilespec(sFilespec),
		m_iVersion(TDB_VERSION),
		m_pEntities(NULL),
		m_bFreeEntities(false)

//	CResourceDb constructor
//
//	sFilespec = "Extensions/MyExtension"
//
//	If Extensions/MyExtension.tdb exists, then look for the definitions
//	and the resources in the .tdb file.
//
//	Otherwise, use Extensions/MyExtension.xml for the definitions and
//	Look for resource files in the Extensions folder.

	{
	//	If this is a resource path, then we look in the resources.

	char *pszResID;
	if (pathIsResourcePath(sFilespec, &pszResID))
		{
		m_pDb = new CDataFile(m_sFilespec);
		m_bGameFileInDb = true;
		m_bResourcesInDb = true;
		}

	//	Otherwise we look in a file.

	else
		{
		//	If we don't have an extension then we look for an XML file and a
		//	TDB file (in that order).

		CString sType = pathGetExtension(m_sFilespec);
		if (sType.IsBlank())
			{
			//	Look for the XML file

			CString sTry = strPatternSubst(CONSTLIT("%s.xml"), m_sFilespec);
			if (pathExists(sTry))
				{
				m_sFilespec = sTry;
				sType = FILE_TYPE_XML;
				}

			//	Otherwise, assume TDB

			else
				{
				m_sFilespec = strPatternSubst(CONSTLIT("%s.tdb"), m_sFilespec);
				sType = FILE_TYPE_TDB;
				}
			}

		//	Keep track of the main file

		m_sRoot = pathGetPath(m_sFilespec);
		m_sGameFile = pathGetFilename(m_sFilespec);

		//	Are we an XML file?

		if (strEquals(sType, FILE_TYPE_XML))
			{
			m_bGameFileInDb = false;

			//	If we're the main XML file and the resource path does not exist,
			//	then use the TDB file for resources.

			if (!bExtension && !pathExists(pathAddComponent(m_sRoot, RESOURCES_FOLDER)))
				{
				m_pDb = new CDataFile(strPatternSubst(CONSTLIT("%s.tdb"), pathStripExtension(m_sFilespec)));
				m_bResourcesInDb = true;
				}

			//	Otherwise, just get the resource the normal way

			else
				{
				m_pDb = NULL;
				m_bResourcesInDb = false;
				}
			}

		//	Otherwise, load the TDB file

		else
			{
			m_pDb = new CDataFile(m_sFilespec);

			m_bGameFileInDb = true;
			m_bResourcesInDb = true;
			}
		}
	}

CResourceDb::~CResourceDb (void)

//	CResourceDb destructor

	{
	if (m_pDb)
		delete m_pDb;

	SetEntities(NULL);
	}

void CResourceDb::ComputeFileDigest (CIntegerIP *retDigest)

//	ComputeFileDigest
//
//	Compute a digest for our file.

	{
	if (fileCreateDigest(m_sFilespec, retDigest) != NOERROR)
		*retDigest = CIntegerIP();
	}

ALERROR CResourceDb::ExtractMain (CString *retsData)

//	ExtractMain
//
//	Extract the main game file and return it

	{
	ALERROR error;

	if (m_bGameFileInDb && m_pDb)
		{
		if (error = m_pDb->ReadEntry(m_iGameFile, retsData))
			return error;
		}
	else
		return ERR_FAIL;

	return NOERROR;
	}

ALERROR CResourceDb::ExtractResource (const CString sFilespec, CString *retsData)

//	ExtractResource
//
//	Extracts the given resource

	{
	ALERROR error;

	if (m_bResourcesInDb && m_pDb)
		{
		if (error = ReadEntry(sFilespec, retsData))
			return error;
		}
	else
		return ERR_FAIL;

	return NOERROR;
	}

int CResourceDb::GetResourceCount (void)

//	GetResourceCount
//
//	Returns the number of resources

	{
	if (m_pDb)
		return m_ResourceMap.GetCount();
	else
		return 0;
	}

CString CResourceDb::GetResourceFilespec (int iIndex)

//	GetResourceFilespec
//
//	Returns the filespec of the given resource

	{
	if (m_pDb)
		return m_ResourceMap.GetKey(iIndex);
	else
		return NULL_STR;
	}

CString CResourceDb::GetRootTag (void)

//	GetRootTag
//
//	Returns the tag of the root element (or NULL_STR if there is an error)

	{
	if (m_bGameFileInDb && m_pDb)
		{
		int iReadSize = Min(m_pDb->GetEntryLength(m_iGameFile), 1024);

		CString sGameFile;
		if (m_pDb->ReadEntryPartial(m_iGameFile, 0, iReadSize, &sGameFile) != NOERROR)
			return NULL_STR;

		//	Parse the XML file from the buffer

		CBufferReadBlock GameFile(sGameFile);
		CString sTag;

		if (CXMLElement::ParseRootTag(&GameFile, &sTag) != NOERROR)
			return NULL_STR;

		return sTag;
		}
	else
		{
		//	Parse the XML file on disk

		CFileReadBlock DataFile(pathAddComponent(m_sRoot, m_sGameFile));
		CString sTag;

		if (CXMLElement::ParseRootTag(&DataFile, &sTag) != NOERROR)
			return NULL_STR;

		return sTag;
		}
	}

bool CResourceDb::ImageExists (const CString &sFolder, const CString &sFilename)

//	ImageExists
//
//	Returns TRUE if the given image exists

	{
	if (m_bResourcesInDb && m_pDb)
		{
		CString sFilespec;
		if (m_iVersion >= 11 && !sFolder.IsBlank())
			sFilespec = pathAddComponent(sFolder, sFilename);
		else
			sFilespec = sFilename;

		//	Look-up the resource in the map

		SResourceEntry *pEntry = m_ResourceMap.GetAt(sFilespec);
		return (pEntry != NULL);
		}
	else
		{
		CString sFilespec = pathAddComponent(m_sRoot, sFolder);
		sFilespec = pathAddComponent(sFilespec, sFilename);

		return pathExists(sFilespec);
		}
	}

ALERROR CResourceDb::LoadEntities (CString *retsError, CExternalEntityTable **retEntities)

//	LoadEntities
//
//	Loads the entities of a game file

	{
	ALERROR error;

	CExternalEntityTable *pEntities = new CExternalEntityTable;
	SetEntities(pEntities, true);

	if (m_bGameFileInDb && m_pDb)
		{
		CString sGameFile;
		if (error = m_pDb->ReadEntry(m_iGameFile, &sGameFile))
			{
			*retsError = strPatternSubst(CONSTLIT("%s is corrupt"), m_sGameFile);
			return error;
			}

		//	Parse the XML file from the buffer

		CBufferReadBlock GameFile(sGameFile);

		CString sError;
		if (error = CXMLElement::ParseEntityTable(&GameFile, pEntities, &sError))
			{
			*retsError = strPatternSubst(CONSTLIT("Unable to parse %s: %s"), m_sGameFile, sError);
			return error;
			}
		}
	else
		{
		//	Parse the XML file on disk

		CFileReadBlock DataFile(pathAddComponent(m_sRoot, m_sGameFile));
		CString sError;

		if (error = CXMLElement::ParseEntityTable(&DataFile, pEntities, &sError))
			{
			*retsError = strPatternSubst(CONSTLIT("Unable to parse %s: %s"), m_sGameFile, sError);
			return error;
			}
		}

	if (retEntities)
		*retEntities = pEntities;

	return NOERROR;
	}

ALERROR CResourceDb::LoadGameFileStub (CXMLElement **retpData, CExternalEntityTable *ioEntityTable, CString *retsError)

//	LoadGameFileStub
//
//	Loads the entities and the root element (without and sub elements).

	{
	ALERROR error;

	if (m_bGameFileInDb && m_pDb)
		{
		CString sGameFile;
		if (error = m_pDb->ReadEntry(m_iGameFile, &sGameFile))
			{
			*retsError = strPatternSubst(CONSTLIT("%s is corrupt"), m_sGameFile);
			return error;
			}

		//	Parse the XML file from the buffer

		CBufferReadBlock GameFile(sGameFile);
		CString sError;

		if (error = CXMLElement::ParseRootElement(&GameFile, retpData, ioEntityTable, &sError))
			{
			if (error == ERR_NOTFOUND)
				*retsError = strPatternSubst(CONSTLIT("Unable to open file: %s"), m_sGameFile);
			else
				*retsError = strPatternSubst(CONSTLIT("%s: %s"), m_sGameFile, sError);
			return error;
			}
		}
	else
		{
		//	Parse the XML file on disk

		CFileReadBlock DataFile(pathAddComponent(m_sRoot, m_sGameFile));
		CString sError;

		if (error = CXMLElement::ParseRootElement(&DataFile, retpData, ioEntityTable, &sError))
			{
			if (error == ERR_NOTFOUND)
				*retsError = strPatternSubst(CONSTLIT("Unable to open file: %s"), m_sGameFile);
			else
				*retsError = strPatternSubst(CONSTLIT("%s: %s"), m_sGameFile, sError);
			return error;
			}
		}

	return NOERROR;
	}

ALERROR CResourceDb::LoadGameFile (CXMLElement **retpData, IXMLParserController *pEntities, CString *retsError, CExternalEntityTable *ioEntityTable)

//	LoadGameFile
//
//	Loads and parses the XML game file

	{
	ALERROR error;

	if (m_bGameFileInDb && m_pDb)
		{
		CString sGameFile;
		if (error = m_pDb->ReadEntry(m_iGameFile, &sGameFile))
			{
			if (retsError) *retsError = strPatternSubst(CONSTLIT("%s is corrupt"), m_sGameFile);
			return error;
			}

		//	Parse the XML file from the buffer

		CBufferReadBlock GameFile(sGameFile);
		CString sError;

		if (error = CXMLElement::ParseXML(&GameFile, pEntities, retpData, &sError, ioEntityTable))
			{
			if (retsError)
				{
				if (error == ERR_NOTFOUND)
					*retsError = strPatternSubst(CONSTLIT("Unable to open file: %s"), m_sGameFile);
				else
					*retsError = strPatternSubst(CONSTLIT("%s: %s"), m_sGameFile, sError);
				}

			return error;
			}
		}
	else
		{
		//	Parse the XML file on disk

		CFileReadBlock DataFile(pathAddComponent(m_sRoot, m_sGameFile));
		CString sError;

		if (error = CXMLElement::ParseXML(&DataFile, pEntities, retpData, &sError, ioEntityTable))
			{
			if (retsError)
				{
				if (error == ERR_NOTFOUND)
					*retsError = strPatternSubst(CONSTLIT("Unable to open file: %s"), m_sGameFile);
				else
					*retsError = strPatternSubst(CONSTLIT("%s: %s"), m_sGameFile, sError);
				}

			return error;
			}
		}

	//	Remember our entity table so that future calls (e.g., LoadModule) can 
	//	get access to them.
	//
	//	If we're supposed to return one, then use that one, since it contains
	//	entities that we declared in this file. Otherwise, we just use the one
	//	that was passed in.

	if (ioEntityTable)
		SetEntities(ioEntityTable);
	else if (pEntities)
		SetEntities(pEntities);

	return NOERROR;
	}

ALERROR CResourceDb::LoadImage (const CString &sFolder, const CString &sFilename, HBITMAP *rethImage, EBitmapTypes *retiImageType)

//	LoadImage
//
//	Loads an image and returns it

	{
	ALERROR error;

	try
		{
		if (m_bResourcesInDb && m_pDb)
			{
			CString sFilespec;
			if (m_iVersion >= 11 && !sFolder.IsBlank())
				sFilespec = pathAddComponent(sFolder, sFilename);
			else
				sFilespec = sFilename;

			CString sData;
			if (error = ReadEntry(sFilespec, &sData))
				return error;

			CString sType = pathGetExtension(sFilespec);
			if (strEquals(sType, CONSTLIT("jpg")))
				{
				if (error = JPEGLoadFromMemory(sData.GetASCIIZPointer(),
						sData.GetLength(),
						JPEG_LFR_DIB,
						NULL,
						rethImage))
					{
					kernelDebugLogMessage("Unable to load JPEG resource '%s'", sFilename);
					return error;
					}

				if (retiImageType)
					*retiImageType = bitmapRGB;
				}
			else
				{
				CBufferReadBlock Data(sData);

				if (error = dibLoadFromBlock(Data, rethImage, retiImageType))
					{
					kernelDebugLogMessage("Unable to load DIB resource '%s'", sFilename);
					return error;
					}
				}
			}
		else
			{
			CString sFilespec = pathAddComponent(m_sRoot, sFolder);
			sFilespec = pathAddComponent(sFilespec, sFilename);

			CString sType = pathGetExtension(sFilespec);
			if (strEquals(sType, CONSTLIT("jpg")))
				{
				//	Load the JPEG file

				if (error = JPEGLoadFromFile(sFilespec, JPEG_LFR_DIB, NULL, rethImage))
					{
					kernelDebugLogMessage("Unable to load JPEG file '%s'", sFilename);
					return error;
					}

				if (retiImageType)
					*retiImageType = bitmapRGB;
				}
			else
				{
				//	Load bitmap

				if (error = dibLoadFromFile(sFilespec, rethImage, retiImageType))
					{
					kernelDebugLogMessage("Unable to load DIB file '%s'", sFilename);
					return error;
					}
				}
			}
		}
	catch (...)
		{
		kernelDebugLogMessage("Crash loading image from resource db: %s.", sFilename);
		return ERR_FAIL;
		}

	return NOERROR;
	}

ALERROR CResourceDb::LoadModule (const CString &sFolder, const CString &sFilename, CXMLElement **retpData, CString *retsError)

//	LoadModule
//
//	Loads a module

	{
	ALERROR error;

	if (m_bGameFileInDb && m_pDb)
		{
		CString sFilespec;
		if (m_iVersion >= 11)
			sFilespec = pathAddComponent(sFolder, sFilename);
		else
			sFilespec = sFilename;

		//	Look up the file in the map

		CString sGameFile;
		if (error = ReadEntry(sFilespec, &sGameFile))
			{
			*retsError = strPatternSubst(CONSTLIT("%s: Unable to read entry %s."), m_sGameFile, sFilespec);
			return error;
			}

		//	Parse the XML file from the buffer

		CBufferReadBlock GameFile(sGameFile);
		CString sError;
		TRY(CXMLElement::ParseXML(&GameFile, m_pEntities, retpData, &sError));
		if (error)
			{
			*retsError = strPatternSubst(CONSTLIT("%s: %s"), m_sGameFile, sError);
			return error;
			}
		}
	else
		{
		//	Parse the XML file on disk

		CFileReadBlock DataFile(pathAddComponent(m_sRoot, pathAddComponent(sFolder, sFilename)));
		CString sError;
		if (error = CXMLElement::ParseXML(&DataFile, m_pEntities, retpData, &sError))
			{
			if (error == ERR_NOTFOUND)
				*retsError = strPatternSubst(CONSTLIT("Unable to open file: %s"), DataFile.GetFilename());
			else
				*retsError = strPatternSubst(CONSTLIT("%s: %s"), sFilename, sError);
			return error;
			}
		}

	return NOERROR;
	}

ALERROR CResourceDb::LoadSound (CSoundMgr &SoundMgr, const CString &sFolder, const CString &sFilename, int *retiChannel)

//	LoadSound
//
//	Loads a sound file

	{
	ALERROR error;

	if (m_bResourcesInDb && m_pDb)
		{
		CString sFilespec;
		if (m_iVersion >= 11)
			sFilespec = pathAddComponent(sFolder, sFilename);
		else
			sFilespec = sFilename;

		CString sData;
		if (error = ReadEntry(sFilespec, &sData))
			return error;

		CBufferReadBlock Data(sData);
		if (error = Data.Open())
			return error;

		if (error = SoundMgr.LoadWaveFromBuffer(Data, retiChannel))
			return error;
		}
	else
		{
		CString sFilespec = pathAddComponent(m_sRoot, sFolder);
		sFilespec = pathAddComponent(sFilespec, sFilename);

		if (error = SoundMgr.LoadWaveFile(sFilespec, retiChannel))
			return error;
		}

	return NOERROR;
	}

ALERROR CResourceDb::ReadEntry (const CString &sFilespec, CString *retsData)

//	ReadEntry
//
//	Reads an entry

	{
	DEBUG_TRY

	ALERROR error;

	//	Look-up the resource in the map

	SResourceEntry *pEntry = m_ResourceMap.GetAt(sFilespec);
	if (pEntry == NULL)
		return ERR_FAIL;

	if (error = m_pDb->ReadEntry(pEntry->iEntryID, retsData))
		return error;

	//	If this is a compressed entry, we need to uncompress it.

	if (pEntry->dwFlags & FLAG_COMPRESS_ZLIB)
		{
		CBufferReadBlock Input(*retsData);

		CMemoryWriteStream Output;
		if (error = Output.Create())
			return ERR_FAIL;

		if (!::zipDecompress(Input, compressionZlib, Output))
			return ERR_FAIL;

		*retsData = CString(Output.GetPointer(), Output.GetLength());
		}

	return NOERROR;

	DEBUG_CATCH
	}

ALERROR CResourceDb::Open (DWORD dwFlags, CString *retsError)

//	Open
//
//	Initializes database

	{
	ALERROR error;

	//	Load the resource map, if necessary

	if (m_pDb)
		{
		char *pszResID;
		if (pathIsResourcePath(m_sFilespec, &pszResID))
			{
			if (error = m_pDb->OpenFromResource(NULL, pszResID))
				{
				if (retsError)
					*retsError = strPatternSubst(CONSTLIT("Unable to load resource path: %s."), m_sFilespec);
				return error;
				}
			}
		else
			{
			if (error = m_pDb->Open(dwFlags))
				{
				if (retsError)
					*retsError = strPatternSubst(CONSTLIT("Unable to open file: %s."), m_sFilespec);
				return error;
				}
			}

		if (error = OpenDb())
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Invalid or corrupt resource database: %s."), m_sFilespec);
			return error;
			}
		}

	return NOERROR;
	}

ALERROR CResourceDb::OpenDb (void)

//	OpenDb
//
//	Initializes the database

	{
	ALERROR error;
	int i;

	//	Load the resource map, if necessary

	if (m_pDb)
		{
		CString sData;
		if (error = m_pDb->ReadEntry(m_pDb->GetDefaultEntry(), &sData))
			return error;

		CMemoryReadStream Stream(sData.GetASCIIZPointer(), sData.GetLength());
		if (error = Stream.Open())
			return error;

		//	Check the signature

		DWORD dwLoad;
		Stream.Read((char *)&dwLoad, sizeof(DWORD));
		if (dwLoad != TDB_SIGNATURE)
			return ERR_FAIL;

		//	Check the version

		Stream.Read((char *)&dwLoad, sizeof(DWORD));
		if (dwLoad > TDB_VERSION)
			return ERR_FAIL;

		m_iVersion = (int)dwLoad;

		//	Read the game file

		Stream.Read((char *)&m_iGameFile, sizeof(DWORD));

		//	Read the game title (and ignore it).

		CString sLoad;
		sLoad.ReadFromStream(&Stream);

		//	For versions 12 and higher we use a different format

		m_ResourceMap.DeleteAll();
		if (m_iVersion >= 12)
			{
			Stream.Read((char *)&dwLoad, sizeof(DWORD));
			int iCount = (int)dwLoad;
			for (i = 0; i < iCount; i++)
				{
				CString sFilespec;
				sFilespec.ReadFromStream(&Stream);
				SResourceEntry *pEntry = m_ResourceMap.Insert(sFilespec);
				pEntry->sFilename = sFilespec;
				Stream.Read((char *)&pEntry->iEntryID, sizeof(DWORD));
				Stream.Read((char *)&pEntry->dwFlags, sizeof(DWORD));
				}
			}

		//	Otherwise, old style

		else
			{
			//	Read the flattened symbol table

			sLoad.ReadFromStream(&Stream);

			//	Unflatten the symbol table

			CSymbolTable *pTable;
			if (error = CObject::Unflatten(sLoad, (CObject **)&pTable))
				return error;

			//	Convert to new format

			for (i = 0; i < pTable->GetCount(); i++)
				{
				SResourceEntry *pEntry = m_ResourceMap.Insert(pTable->GetKey(i));
				pEntry->sFilename = pTable->GetKey(i);
				pEntry->iEntryID = (int)pTable->GetValue(i);
				pEntry->dwFlags = 0;
				}

			delete pTable;
			}
		}

	return NOERROR;
	}

CString CResourceDb::ResolveFilespec (const CString &sFolder, const CString &sFilename) const

//	ResolveFilespec
//
//	Resolves the given filename to a filespec.
//	
//	NOTE: This is only appropriate for refering to stand-alone files. In all 
//	othercases we return NULL_STR.

	{
	CString sFilespec = pathAddComponent(m_sRoot, sFolder);
	sFilespec = pathAddComponent(sFilespec, sFilename);
	return sFilespec;
	}

void CResourceDb::SetEntities (IXMLParserController *pEntities, bool bFree)

//	SetEntities
//
//	Sets the entities parameter

	{
	if (m_pEntities && m_bFreeEntities)
		delete m_pEntities;

	m_pEntities = pEntities;
	m_bFreeEntities = bFree;
	}
