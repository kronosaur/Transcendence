//	CTDBCompiler.cpp
//
//	CTDBCompiler class
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "TDBCompiler.h"

#define TDB_SIGNATURE							'TRDB'
#define TDB_VERSION								12

#define ATTRIB_FILENAME							CONSTLIT("filename")

#define FILENAME_AMERICA						CONSTLIT("America.xml")
#define FILENAME_TRANSCENDENCE					CONSTLIT("Transcendence.xml")

#define TAG_AMERICA_CAMPAIGN					CONSTLIT("AmericaCampaign")
#define TAG_AMERICA_LIBRARY						CONSTLIT("AmericaLibrary")
#define TAG_AMERICA_EXTENSION					CONSTLIT("AmericaExtension")
#define TAG_CORE_LIBRARY						CONSTLIT("CoreLibrary")
#define TAG_TRANSCENDENCE_ADVENTURE				CONSTLIT("TranscendenceAdventure")
#define TAG_TRANSCENDENCE_EXTENSION				CONSTLIT("TranscendenceExtension")
#define TAG_TRANSCENDENCE_LIBRARY				CONSTLIT("TranscendenceLibrary")

bool CTDBCompiler::AddCoreEntityTables (const CString &sCoreTDB, CString *retsError)

//	AddCoreEntityTables
//
//	Loads the input file XML and takes all its entities and any entities it 
//	references.

	{
	CString sError;
	int i;

	//	Open the TDB

	CResourceDb TDBFile(sCoreTDB);
	if (TDBFile.Open(DFOPEN_FLAG_READ_ONLY, &sError) != NOERROR)
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to open '%s': %s"), sCoreTDB, sError);
		return false;
		}

	//	Parse the root file

	CXMLElement *pRootXML;
	CExternalEntityTable *pRootEntities = new CExternalEntityTable;
	if (TDBFile.LoadGameFile(&pRootXML, NULL, &sError, pRootEntities) != NOERROR)
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to open '%s': %s"), sCoreTDB, sError);
		return false;
		}

	//	Add the root entities to our table (we take ownership).

	AddEntityTable(pRootEntities);

	//	Loop over all sub elements and see if we find any TranscendenceLibrary
	//	references. If so, we need to add them to our entity tables.

	for (i = 0; i < pRootXML->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pRootXML->GetContentElement(i);
		CString sFilename;
		if (strEquals(pItem->GetTag(), TAG_CORE_LIBRARY)
				&& pItem->FindAttribute(ATTRIB_FILENAME, &sFilename))
			{
			//	Load this

			CExternalEntityTable *pEntities;
			if (TDBFile.LoadModuleEntities(NULL_STR, sFilename, &pEntities, retsError))
				return false;

			//	Add it

			AddEntityTable(pEntities);
			}
		}

	//	Done

	return true;
	}

void CTDBCompiler::AddEntityTable (CExternalEntityTable *pTable)

//	AddCoreEntitiesTable
//
//	Adds the given table to our list of entities. Note that we take ownership
//	of pTable.

	{
	//	Keep the table in a list (we take ownership).

	m_CoreEntities.Insert(pTable);

	//	Link to previous tables

	if (m_pCoreEntities == NULL)
		m_pCoreEntities = pTable;
	else
		{
		pTable->SetParent(m_pCoreEntities);
		m_pCoreEntities = pTable;
		}
	}

void CTDBCompiler::AddResource (const CString &sName, int iEntryID, bool bCompress)

//	AddResource
//
//	Adds a resource to our resource map.

	{
	SResourceEntry *pEntry = m_ResourceMap.Insert(sName);
	pEntry->sFilename = sName;
	pEntry->iEntryID = iEntryID;
	pEntry->dwFlags = (bCompress ? FLAG_COMPRESS_ZLIB : 0);
	}

bool CTDBCompiler::Init (const CString &sInputFilespec, const CString &sOutputFilespec, const TArray<CString> &EntityFilespecs, CXMLElement *pCmdLine, CString *retsError)

//	Init
//
//	Initialize based on main XML file.

	{
	int i;

	m_sInputFilespec = sInputFilespec;
	m_sOutputFilespec = sOutputFilespec;
	m_sRootPath = pathGetPath(sInputFilespec);

	CString sInputFilename = pathGetFilename(sInputFilespec);

	//	If this is one of the root XML files, then we process it as such.

	if (strEquals(sInputFilename, FILENAME_AMERICA))
		m_iType = typeAmericaUniverse;
	else if (strEquals(sInputFilename, FILENAME_TRANSCENDENCE))
		m_iType = typeTranscendenceUniverse;

	//	Otherwise try to figure it out based on the root element.

	else
		{
		CFileReadBlock InputFile(sInputFilespec);
		CString sRootTag;
		if (CXMLElement::ParseRootTag(&InputFile, &sRootTag) != NOERROR)
			{
			*retsError = strPatternSubst(CONSTLIT("Unable to read '%s'."), sInputFilespec);
			return false;
			}

		if (strEquals(sRootTag, TAG_TRANSCENDENCE_ADVENTURE)
				|| strEquals(sRootTag, TAG_TRANSCENDENCE_EXTENSION)
				|| strEquals(sRootTag, TAG_TRANSCENDENCE_LIBRARY))
			m_iType = typeTransExtension;
		else if (strEquals(sRootTag, TAG_AMERICA_CAMPAIGN)
				|| strEquals(sRootTag, TAG_AMERICA_EXTENSION)
				|| strEquals(sRootTag, TAG_AMERICA_LIBRARY))
			m_iType = typeAmericaExtension;
		}

	//	For Transcendence extensions we need to include entities from 
	//	Transcendence.tdb

	if (m_iType == typeTransExtension)
		{
		if (!AddCoreEntityTables(CONSTLIT("Transcendence.tdb"), retsError))
			return false;
		}

	//	For CSC America extension we include entities from America.tdb

	else if (m_iType == typeAmericaExtension)
		{
		if (!AddCoreEntityTables(CONSTLIT("America.tdb"), retsError))
			return false;
		}

	//	Add all entities from external libraries

	for (i = 0; i < EntityFilespecs.GetCount(); i++)
		{
		CExternalEntityTable *pNewEntities;
		if (!ReadEntities(EntityFilespecs[i], &pNewEntities))
			{
			*retsError = strPatternSubst(CONSTLIT("Unable to read entities file '%s'."), EntityFilespecs[i]);
			return false;
			}

		AddEntityTable(pNewEntities);
		}

	return true;
	}

void CTDBCompiler::PopEntityTable (CExternalEntityTable *pOldTable)

//	PopEntityTable
//
//	Restores the entity table previously returned by PushEntityTable.
//	And frees the discarded table.

	{
	delete m_pCoreEntities;
	m_pCoreEntities = pOldTable;
	}

CExternalEntityTable *CTDBCompiler::PushEntityTable (CExternalEntityTable *pNewTable)

//	PushEntityTable
//
//	Temporarily adds the given table to the chain of entity tables. We return
//	the previous table in the chain and expect to be called back with 
//	PopEntityTable.
//
//	We take ownership of pNewTable and will delete when we pop.

	{
	CExternalEntityTable *pOldTable = m_pCoreEntities;
	if (m_pCoreEntities)
		pNewTable->SetParent(m_pCoreEntities);

	m_pCoreEntities = pNewTable;

	return pOldTable;
	}

bool CTDBCompiler::ReadEntities (const CString &sFilespec, CExternalEntityTable **retpEntityTable) const
	{
	ALERROR error;

	//	Open the data file.

	CDataFile EntitiesTDB(sFilespec);
	if (error = EntitiesTDB.Open(DFOPEN_FLAG_READ_ONLY))
		return false;

	//	Open

	CString sData;
	if (error = EntitiesTDB.ReadEntry(EntitiesTDB.GetDefaultEntry(), &sData))
		return false;

	CMemoryReadStream Stream(sData.GetASCIIZPointer(), sData.GetLength());
	if (error = Stream.Open())
		return false;

	//	Check the signature

	DWORD dwLoad;
	Stream.Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad != TDB_SIGNATURE)
		return false;

	//	Check the version

	Stream.Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad > TDB_VERSION)
		return false;

	//	Read the game file

	int iGameFile;
	Stream.Read((char *)&iGameFile, sizeof(DWORD));

	//	Load the main entry

	CString sGameFile;
	if (error = EntitiesTDB.ReadEntry(iGameFile, &sGameFile))
		return false;

	//	Allocate entities

	CExternalEntityTable *pEntities = new CExternalEntityTable;

	//	Parse the XML file from the buffer

	CBufferReadBlock GameFile(sGameFile);

	CString sError;
	if (error = CXMLElement::ParseEntityTable(&GameFile, pEntities, &sError))
		return false;

	//	Done

	*retpEntityTable = pEntities;

	return true;
	}

void CTDBCompiler::ReportError (const CString &sError)

//	ReportError
//
//	Output an error.

	{
	m_iErrorCount++;
	printf("error : %s\n", sError.GetASCIIZPointer());
	}

void CTDBCompiler::ReportErrorCount (void)

//	ReportErrorCount
//
//	Output the number of errors encountered.

	{
	printf("\n%s - %d error%s\n", m_sOutputFilespec.GetASCIIZPointer(), m_iErrorCount, (m_iErrorCount == 1 ? "" : "s"));
	}

bool CTDBCompiler::WriteHeader (IWriteStream &Stream, int iGameFile, CString *retsError)

//	WriteHeader
//
//	Writes the TDB header to the stream.

	{
	DWORD dwSave;
	int i;

	//	Signature

	dwSave = TDB_SIGNATURE;
	Stream.Write((char *)&dwSave, sizeof(dwSave));

	//	Version

	dwSave = TDB_VERSION;
	Stream.Write((char *)&dwSave, sizeof(dwSave));

	//	Game file entry

	dwSave = iGameFile;
	Stream.Write((char *)&dwSave, sizeof(dwSave));

	//	Game title

	CString sGameTitle = CONSTLIT("Transcendence");
	sGameTitle.WriteToStream(&Stream);

	//	Resource map

	dwSave = m_ResourceMap.GetCount();
	Stream.Write((char *)&dwSave, sizeof(dwSave));
	for (i = 0; i < m_ResourceMap.GetCount(); i++)
		{
		const SResourceEntry &Entry = m_ResourceMap[i];
		Entry.sFilename.WriteToStream(&Stream);

		Stream.Write((char *)&Entry.iEntryID, sizeof(dwSave));
		Stream.Write((char *)&Entry.dwFlags, sizeof(dwSave));
		}

	return true;
	}
