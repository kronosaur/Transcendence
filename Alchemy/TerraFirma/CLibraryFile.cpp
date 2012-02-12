//	CLibraryFile.cpp
//
//	Implements CLibraryFile object

#include "Alchemy.h"
#include "TerraFirma.h"

#define BLOCK_SIZE								256
#define INITIAL_ENTRIES							1024

CLibraryFile::CLibraryFile (CCodeChain *pCC, CString sFilename) :
		m_pCC(pCC),
		m_Db(sFilename),
		m_pMap(NULL)

//	CLibraryFile constructor

	{
	}

CLibraryFile::~CLibraryFile (void)

//	CLibraryFile destructor

	{
	//	Make sure we're closed

	Close();
	}

ALERROR CLibraryFile::Close (void)

//	Close
//
//	Close the file

	{
	ALERROR savederror = NOERROR;
	ALERROR error;

	//	If we don't have a map, then the database must not
	//	be open

	if (m_pMap == NULL)
		return NOERROR;

	//	Flush the database

	if (error = Flush())
		savederror = error;

	//	Close

	if (error = m_Db.Close())
		savederror = error;

	//	Done with the map

	m_pMap->Discard(m_pCC);
	m_pMap = NULL;

	return savederror;
	}

ALERROR CLibraryFile::Create (void)

//	Create
//
//	Creates the file and leaves it open

	{
	ALERROR error;
	int iEntry;

	//	Create an empty symbol table

	m_pMap = m_pCC->CreateSymbolTable();
	if (m_pMap->IsError())
		return ERR_MEMORY;

	//	Create the file

	if (error = CDataFile::Create(m_Db.GetFilename(), BLOCK_SIZE, INITIAL_ENTRIES))
		return error;

	//	Open the file

	if (error = m_Db.Open())
		{
		DeleteFile(m_Db.GetFilename().GetASCIIZPointer());
		return error;
		}

	//	Save the map entry

	if (error = CreateEntry(m_pMap, &iEntry))
		{
		m_Db.Close();
		DeleteFile(m_Db.GetFilename().GetASCIIZPointer());
		return error;
		}

	//	Make the map the default entry

	m_Db.SetDefaultEntry(iEntry);

	//	Flush the database

	if (error = m_Db.Flush())
		{
		m_Db.Close();
		DeleteFile(m_Db.GetFilename().GetASCIIZPointer());
		return error;
		}

	return NOERROR;
	}

ALERROR CLibraryFile::CreateEntry (ICCItem *pItem, int *retiUNID)

//	CreateEntry
//
//	Adds a new entry to the library

	{
	ALERROR error;
	CMemoryWriteStream sStream(0);
	ICCItem *pError;

	if (error = sStream.Create())
		return error;

	//	Stream the item out

	pError = m_pCC->StreamItem(pItem, &sStream);
	error = (pError->IsError() ? ERR_FAIL : NOERROR);
	pError->Discard(m_pCC);
	if (error)
		return error;

	CString sString(sStream.GetPointer(), sStream.GetLength(), TRUE);

	//	Save out to the database

	if (error = m_Db.AddEntry(sString, retiUNID))
		return error;

	//	Done

	return NOERROR;
	}

ALERROR CLibraryFile::DeleteEntry (int iUNID)

//	DeleteEntry
//
//	Delete the given entry

	{
	return m_Db.DeleteEntry(iUNID);
	}

ALERROR CLibraryFile::Flush (void)

//	Flush
//
//	Flush the stuff in memory

	{
	ALERROR error;

	//	If we need to save the map, do it now

	if (m_pMap->IsModified())
		{
		if (error = WriteEntry(m_Db.GetDefaultEntry(), m_pMap))
			return error;

		m_pMap->ClearModified();
		}

	//	Flush the database

	if (error = m_Db.Flush())
		return error;

	return NOERROR;
	}

ICCItem *CLibraryFile::LoadMapEntry (CString sKey)

//	LoadMapEntry
//
//	Loads an entry by name.

	{
	ICCItem *pTable = m_pMap;
	ICCItem *pKey;
	ICCItem *pResult;

	//	Convert the string to an item

	pKey = m_pCC->CreateString(sKey);
	if (pKey->IsError())
		return pKey;

	//	Lookup the key in the symbol table

	pResult = pTable->Lookup(m_pCC, pKey);
	pKey->Discard(m_pCC);

	//	Done

	return pResult;
	}

ALERROR CLibraryFile::Open (void)

//	Open
//
//	Open the file

	{
	ALERROR error;

	//	Open the database

	if (error = m_Db.Open())
		return error;

	//	Read the map

	if (error = ReadEntry(m_Db.GetDefaultEntry(), (ICCItem **)&m_pMap))
		return error;

	//	Clear the modified flag so we know when it gets changed

	m_pMap->ClearModified();

	return NOERROR;
	}

ALERROR CLibraryFile::ReadEntry (int iUNID, ICCItem **retpItem)

//	ReadEntry
//
//	Returns an entry from the database

	{
	ALERROR error;
	CString sString;

	if (error = m_Db.ReadEntry(iUNID, &sString))
		return error;

	//	Unstream

	CMemoryReadStream sStream(sString.GetPointer(), sString.GetLength());
	if (error = sStream.Open())
		return error;

	*retpItem = m_pCC->UnstreamItem(&sStream);

	return NOERROR;
	}

ICCItem *CLibraryFile::SaveMapEntry (CString sKey, ICCItem *pValue)

//	SaveMapEntry
//
//	Saves an entry by name.

	{
	ICCItem *pTable = m_pMap;
	ICCItem *pKey;
	ICCItem *pResult;

	//	Convert the string to an item

	pKey = m_pCC->CreateString(sKey);
	if (pKey->IsError())
		return pKey;

	//	Add the new entry

	pResult = pTable->AddEntry(m_pCC, pKey, pValue);
	pKey->Discard(m_pCC);

	//	Done

	return pResult;
	}

ALERROR CLibraryFile::WriteEntry (int iUNID, ICCItem *pItem)

//	WriteEntry
//
//	Writes an entry in the database

	{
	ALERROR error;
	CMemoryWriteStream sStream(0);
	ICCItem *pError;

	if (error = sStream.Create())
		return error;

	//	Stream the item out

	pError = m_pCC->StreamItem(pItem, &sStream);
	error = (pError->IsError() ? ERR_FAIL : NOERROR);
	pError->Discard(m_pCC);
	if (error)
		return error;

	CString sString(sStream.GetPointer(), sStream.GetLength(), TRUE);

	//	Save out to the database

	if (error = m_Db.WriteEntry(iUNID, sString))
		return error;

	//	Done

	return NOERROR;
	}
