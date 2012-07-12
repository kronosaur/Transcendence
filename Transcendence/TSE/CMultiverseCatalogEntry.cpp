//	CMultiverseCatalogEntry.cpp
//
//	CMultiverseCatalogEntry Class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define FIELD_FILE_TDB							CONSTLIT("fileTDB")
#define FIELD_RELEASE							CONSTLIT("release")
#define FIELD_UNID								CONSTLIT("unid")
#define FIELD_VERSION							CONSTLIT("$version")

#define STR_UNID_PREFIX							CONSTLIT("Transcendence:")

#define ERR_INVALID_UNID						CONSTLIT("Invalid UNID for catalog entry.")

CMultiverseCatalogEntry::CMultiverseCatalogEntry (const CMultiverseCatalogEntry &Src)

//	CMultiverseCatalogEntry constructor

	{
	m_sUNID = Src.m_sUNID;
	m_dwRelease = Src.m_dwRelease;
	m_dwVersion = Src.m_dwVersion;
	m_dwUNID = Src.m_dwUNID;
	m_TDBFile = Src.m_TDBFile;
	}

ALERROR CMultiverseCatalogEntry::CreateFromJSON (const CJSONValue &Entry, CMultiverseCatalogEntry **retpEntry, CString *retsResult)

//	CreateFromJSON
//
//	Creates a new entry from a JSON value.

	{
	CMultiverseCatalogEntry *pNewEntry = new CMultiverseCatalogEntry;

	pNewEntry->m_sUNID = Entry.GetElement(FIELD_UNID).AsString();
	if (pNewEntry->m_sUNID.IsBlank())
		{
		delete pNewEntry;
		*retsResult = ERR_INVALID_UNID;
		return ERR_FAIL;
		}

	pNewEntry->m_dwRelease = (DWORD)Entry.GetElement(FIELD_RELEASE).AsInt32();
	pNewEntry->m_dwVersion = (DWORD)Entry.GetElement(FIELD_VERSION).AsInt32();

	//	Parse the fully qualified UNID and get just the hex UNID.

	if (strStartsWith(pNewEntry->m_sUNID, STR_UNID_PREFIX))
		{
		pNewEntry->m_dwUNID = strParseIntOfBase(pNewEntry->m_sUNID.GetASCIIZPointer() + STR_UNID_PREFIX.GetLength(), 16, 0);
		if (pNewEntry->m_dwUNID == 0)
			{
			delete pNewEntry;
			*retsResult = ERR_INVALID_UNID;
			return ERR_FAIL;
			}
		}
	else
		{
		delete pNewEntry;
		*retsResult = ERR_INVALID_UNID;
		return ERR_FAIL;
		}

	//	Get the TDB file

	if (pNewEntry->m_TDBFile.InitFromJSON(Entry.GetElement(FIELD_FILE_TDB), retsResult) != NOERROR)
		{
		delete pNewEntry;
		return ERR_FAIL;
		}

	//	Done

	*retpEntry = pNewEntry;
	return NOERROR;
	}

bool CMultiverseCatalogEntry::IsValid (void)

//	IsValid
//
//	Returns TRUE if the entry is a valid entry (has all necessary fields).

	{
	//	We must have a TDB file.

	if (m_TDBFile.IsEmpty())
		return false;

	return true;
	}
