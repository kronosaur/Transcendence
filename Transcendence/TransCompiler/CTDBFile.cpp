//	CTDBFile.cpp
//
//	CTDBFile Class
//
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TDB.h"

#define TDB_SIGNATURE							'TRDB'
#define TDB_VERSION								11

CTDBFile::~CTDBFile (void)

//	CTDBFile destructor

	{
	}

void CTDBFile::Close (void)

//	Close
//
//	Close the database

	{
	m_Db.Close();
	m_sTitle = NULL_STR;
	m_ResourceMap.DeleteAll();
	}

ALERROR CTDBFile::Open (const CString &sFilespec, DWORD dwFlags)

//	Open
//
//	Open the database

	{
	ALERROR error;
	int i;

	//	Close everything

	Close();

	//	Open the file

	if (error = m_Db.Open(sFilespec, (dwFlags & FLAG_READ_ONLY ? DFOPEN_FLAG_READ_ONLY : 0)))
		return error;

	//	Read the default entry

	CString sData;
	if (error = m_Db.ReadEntry(m_Db.GetDefaultEntry(), &sData))
		{
		m_Db.Close();
		return error;
		}

	CMemoryReadStream Stream(sData.GetASCIIZPointer(), sData.GetLength());
	if (error = Stream.Open())
		{
		m_Db.Close();
		return error;
		}

	//	Check the signature

	DWORD dwLoad;
	Stream.Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad != TDB_SIGNATURE)
		{
		m_Db.Close();
		return ERR_FAIL;
		}

	//	Check the version

	Stream.Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad > TDB_VERSION)
		{
		m_Db.Close();
		return ERR_FAIL;
		}

	//	Read the game file

	Stream.Read((char *)&m_iGameFile, sizeof(DWORD));

	//	Read the game title

	m_sTitle.ReadFromStream(&Stream);

	//	Read the flattened symbol table

	CString sLoad;
	sLoad.ReadFromStream(&Stream);

	//	Unflatten the symbol table

	CSymbolTable *pTable;
	if (error = CObject::Unflatten(sLoad, (CObject **)&pTable))
		return error;

	//	Convert to TSortMap

	for (i = 0; i < pTable->GetCount(); i++)
		m_ResourceMap.Insert(pTable->GetKey(i), (int)pTable->GetValue(i));

	delete pTable;

	//	Done

	return NOERROR;
	}
