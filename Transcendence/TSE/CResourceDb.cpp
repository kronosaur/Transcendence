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

#define TDB_SIGNATURE							'TRDB'
#define TDB_VERSION								11

#define FILE_TYPE_XML							CONSTLIT("xml")
#define FILE_TYPE_TDB							CONSTLIT("tdb")
#define RESOURCES_FOLDER						CONSTLIT("Resources")

CResourceDb::CResourceDb (const CString &sFilespec, CResourceDb *pMainDb, bool bExtension) : 
		m_sFilespec(sFilespec),
		m_pResourceMap(NULL),
		m_iVersion(TDB_VERSION)

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
	char *pszResID;
	if (pathIsResourcePath(sFilespec, &pszResID))
		{
		m_pDb = new CDataFile(m_sFilespec);
		m_bGameFileInDb = true;
		m_bResourcesInDb = true;
		m_pMainDb = pMainDb;
		}
	else if (pMainDb || bExtension)
		{
		//	We assume this is an extension

		CString sTDB = sFilespec;
		sTDB.Append(CONSTLIT("."));
		sTDB.Append(FILE_TYPE_TDB);

		if (pathExists(sTDB))
			{
			m_pDb = new CDataFile(sTDB);

			m_sRoot = pathGetPath(sTDB);
			m_sGameFile = pathGetFilename(sTDB);
			m_bGameFileInDb = true;
			m_bResourcesInDb = true;
			}
		else
			{
			CString sXML = sFilespec;
			sXML.Append(CONSTLIT("."));
			sXML.Append(FILE_TYPE_XML);

			m_pDb = NULL;

			m_sRoot = pathGetPath(sFilespec);
			m_sGameFile = pathGetFilename(sXML);
			m_bGameFileInDb = false;
			m_bResourcesInDb = false;
			}

		m_pMainDb = pMainDb;
		}
	else
		{
		CString sType = pathGetExtension(sFilespec);
		if (sType.IsBlank())
			{
			CString sXML = sFilespec;
			sXML.Append(CONSTLIT("."));
			sXML.Append(FILE_TYPE_XML);

			//	If Transcendence.xml exists, then use the file and load
			//	the resources from the same location.

			if (pathExists(sXML))
				{
				m_sRoot = pathGetPath(sXML);
				m_sGameFile = pathGetFilename(sXML);
				m_bGameFileInDb = false;

				//	If a resources path exists, then use the resources in the
				//	folder. Otherwise, use resources in the tdb

				CString sResourcesPath = pathAddComponent(m_sRoot, RESOURCES_FOLDER);
				if (pathExists(sResourcesPath))
					{
					m_pDb = NULL;
					m_bResourcesInDb = false;
					}

				//	Otherwise, use the tdb file

				else
					{
					CString sTDB = sFilespec;
					sTDB.Append(CONSTLIT("."));
					sTDB.Append(FILE_TYPE_TDB);
					m_pDb = new CDataFile(sTDB);
					m_bResourcesInDb = true;
					}
				}

			//	Otherwise, use the .tdb file for both

			else
				{
				CString sTDB = sFilespec;
				sTDB.Append(CONSTLIT("."));
				sTDB.Append(FILE_TYPE_TDB);
				m_pDb = new CDataFile(sTDB);

				m_bGameFileInDb = true;
				m_bResourcesInDb = true;
				}
			}
		else if (strEquals(sType, FILE_TYPE_XML))
			{
			m_sRoot = pathGetPath(sFilespec);
			m_sGameFile = pathGetFilename(sFilespec);
			m_pDb = NULL;

			m_bGameFileInDb = false;
			m_bResourcesInDb = false;
			}
		else
			{
			m_pDb = new CDataFile(sFilespec);
			m_bGameFileInDb = true;
			m_bResourcesInDb = true;
			}

		//	This is the main file

		m_pMainDb = NULL;
		}

	//	If we have a main db then this is the parent of the entities
	//	that we're loading.

	if (m_pMainDb)
		m_Entities.SetParent(&m_pMainDb->m_Entities);
	}

CResourceDb::~CResourceDb (void)

//	CResourceDb destructor

	{
	if (m_pDb)
		delete m_pDb;
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
		//	Look-up the resource in the map

		int iEntry;
		if (error = m_pResourceMap->Lookup(sFilespec, (CObject **)&iEntry))
			return error;

		if (error = m_pDb->ReadEntry(iEntry, retsData))
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
	if (m_pDb && m_pResourceMap)
		return m_pResourceMap->GetCount();
	else
		return 0;
	}

CString CResourceDb::GetResourceFilespec (int iIndex)

//	GetResourceFilespec
//
//	Returns the filespec of the given resource

	{
	if (m_pDb && m_pResourceMap)
		return m_pResourceMap->GetKey(iIndex);
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
		ASSERT(m_pResourceMap);

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
		ASSERT(m_pResourceMap);

		CString sFilespec;
		if (m_iVersion >= 11 && !sFolder.IsBlank())
			sFilespec = pathAddComponent(sFolder, sFilename);
		else
			sFilespec = sFilename;

		//	Look-up the resource in the map

		int iEntry;
		return (m_pResourceMap->Lookup(sFilespec, (CObject **)&iEntry) == NOERROR);
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

	if (m_pMainDb == NULL)
		{
		if (m_bGameFileInDb && m_pDb)
			{
			ASSERT(m_pResourceMap);

			CString sGameFile;
			if (error = m_pDb->ReadEntry(m_iGameFile, &sGameFile))
				{
				*retsError = strPatternSubst(CONSTLIT("%s is corrupt"), m_sGameFile);
				return error;
				}

			//	Parse the XML file from the buffer

			CBufferReadBlock GameFile(sGameFile);

			CString sError;
			if (error = CXMLElement::ParseEntityTable(&GameFile, &m_Entities, &sError))
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

			if (error = CXMLElement::ParseEntityTable(&DataFile, &m_Entities, &sError))
				{
				*retsError = strPatternSubst(CONSTLIT("Unable to parse %s: %s"), m_sGameFile, sError);
				return error;
				}
			}
		}

	if (retEntities)
		*retEntities = &m_Entities;

	return NOERROR;
	}

ALERROR CResourceDb::LoadGameFile (CXMLElement **retpData, CString *retsError)

//	LoadGameFile
//
//	Loads and parses the XML game file

	{
	ALERROR error;

	if (m_bGameFileInDb && m_pDb)
		{
		ASSERT(m_pResourceMap);

		CString sGameFile;
		if (error = m_pDb->ReadEntry(m_iGameFile, &sGameFile))
			{
			*retsError = strPatternSubst(CONSTLIT("%s is corrupt"), m_sGameFile);
			return error;
			}

		//	Parse the XML file from the buffer

		CBufferReadBlock GameFile(sGameFile);
		CString sError;

		if (error = CXMLElement::ParseXML(&GameFile, &m_Entities, retpData, &sError, &m_Entities))
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

		if (error = CXMLElement::ParseXML(&DataFile, &m_Entities, retpData, &sError, &m_Entities))
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

ALERROR CResourceDb::LoadImage (const CString &sFolder, const CString &sFilename, HBITMAP *rethImage, EBitmapTypes *retiImageType)

//	LoadImage
//
//	Loads an image and returns it

	{
	ALERROR error;

	if (m_bResourcesInDb && m_pDb)
		{
		ASSERT(m_pResourceMap);

		CString sFilespec;
		if (m_iVersion >= 11 && !sFolder.IsBlank())
			sFilespec = pathAddComponent(sFolder, sFilename);
		else
			sFilespec = sFilename;

		//	Look-up the resource in the map

		int iEntry;
		if (error = m_pResourceMap->Lookup(sFilespec, (CObject **)&iEntry))
			return error;

		CString sData;
		if (error = m_pDb->ReadEntry(iEntry, &sData))
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
				kernelDebugLogMessage("Unable to load JPEG resource '%s'", sFilename.GetASCIIZPointer());
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
				kernelDebugLogMessage("Unable to load DIB resource '%s'", sFilename.GetASCIIZPointer());
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
				kernelDebugLogMessage("Unable to load JPEG file '%s'", sFilename.GetASCIIZPointer());
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
				kernelDebugLogMessage("Unable to load DIB file '%s'", sFilename.GetASCIIZPointer());
				return error;
				}
			}
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
		ASSERT(m_pResourceMap);

		CString sFilespec;
		if (m_iVersion >= 11)
			sFilespec = pathAddComponent(sFolder, sFilename);
		else
			sFilespec = sFilename;

		//	Look up the file in the map

		int iEntry;
		TRY(m_pResourceMap->Lookup(sFilespec, (CObject **)&iEntry));
		if (error)
			{
			*retsError = strPatternSubst(CONSTLIT("%s: Resource map corrupt."), m_sGameFile);
			return error;
			}

		CString sGameFile;
		TRY(m_pDb->ReadEntry(iEntry, &sGameFile));
		if (error)
			{
			*retsError = strPatternSubst(CONSTLIT("%s: Unable to read entry: %d"), m_sGameFile, iEntry);
			return error;
			}

		//	Parse the XML file from the buffer

		CBufferReadBlock GameFile(sGameFile);
		CString sError;
		TRY(CXMLElement::ParseXML(&GameFile, &m_Entities, retpData, &sError));
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
		if (error = CXMLElement::ParseXML(&DataFile, &m_Entities, retpData, &sError))
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
		ASSERT(m_pResourceMap);

		CString sFilespec;
		if (m_iVersion >= 11)
			sFilespec = pathAddComponent(sFolder, sFilename);
		else
			sFilespec = sFilename;

		//	Look-up the resource in the map

		int iEntry;
		if (error = m_pResourceMap->Lookup(sFilespec, (CObject **)&iEntry))
			return error;

		CString sData;
		if (error = m_pDb->ReadEntry(iEntry, &sData))
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

ALERROR CResourceDb::Open (DWORD dwFlags)

//	Open
//
//	Initializes database

	{
	ALERROR error;

	//	Load the resource map, if necessary

	if (m_pDb)
		{
		ASSERT(m_pResourceMap == NULL);

		char *pszResID;
		if (pathIsResourcePath(m_sFilespec, &pszResID))
			{
			if (error = m_pDb->OpenFromResource(NULL, pszResID))
				return error;
			}
		else
			{
			if (error = m_pDb->Open(dwFlags))
				return error;
			}

		if (error = OpenDb())
			return error;
		}

	return NOERROR;
	}

ALERROR CResourceDb::OpenDb (void)

//	OpenDb
//
//	Initializes the database

	{
	ALERROR error;

	//	Load the resource map, if necessary

	if (m_pDb)
		{
		ASSERT(m_pResourceMap == NULL);

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

		//	Read the game title

		CString sLoad;
		sLoad.ReadFromStream(&Stream);

		//	Read the flattened symbol table

		sLoad.ReadFromStream(&Stream);

		//	Unflatten the symbol table

		if (error = CObject::Unflatten(sLoad, (CObject **)&m_pResourceMap))
			return error;
		}

	return NOERROR;
	}
