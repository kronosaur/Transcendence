//	UNIDs.cpp
//
//	Implements various classes for managing UNIDs and entries

#include "Alchemy.h"
#include "NPEngine.h"

CEntryNameTable::CEntryNameTable (void) : m_IDTable(TRUE, FALSE)

//	CEntryNameTable constructor

	{
	}

CString CEntryNameTable::GetEntryName (DWORD dwUNID)

//	GetEntryName
//
//	Returns the name associated with the given UNID

	{
	return LITERAL("Unknown");
	}

void CEntryNameTable::NameEntry (DWORD dwUNID, const CString &sName)

//	NameEntry
//
//	Assigns (or reassigns) a name to the given UNID

	{
	}

DWORD CEntryNameTable::TranslateName (const CString &sName)

//	TranslateName
//
//	Returns the UNID associated with the given name

	{
	return 0;
	}
