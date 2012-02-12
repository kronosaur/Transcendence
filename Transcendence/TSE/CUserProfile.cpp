//	CUserProfile.cpp
//
//	CUserProfile class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

bool CUserProfile::FindAdventureRecord (DWORD dwAdventure, int *retiIndex)

//	FindAdventureRecord
//
//	Looks for the given adventure record

	{
	int i;

	for (i = 0; i < m_Records.GetCount(); i++)
		if (m_Records[i].GetAdventureUNID() == dwAdventure)
			{
			if (retiIndex)
				*retiIndex = i;

			return true;
			}

	return false;
	}

void CUserProfile::Init (const CString &sUsername)

//	Init
//
//	Initialize

	{
	m_sUsername = sUsername;
	m_Records.DeleteAll();
	}

CAdventureRecord &CUserProfile::InsertAdventureRecord (DWORD dwAdventure)

//	InsertAdventureRecord
//
//	Inserts a new adventure record or returns an existing one.
//
//	NOTE: The object returns from this call is only valid until the next call
//	to InsertAdventureRecord.

	{
	//	Look for the adventure. If we find it, return it.

	int iIndex;
	if (FindAdventureRecord(dwAdventure, &iIndex))
		return m_Records[iIndex];

	//	Otherwise, we add it

	CAdventureRecord *pNewRecord = m_Records.Insert();
	pNewRecord->Init(dwAdventure);

	return *pNewRecord;
	}
