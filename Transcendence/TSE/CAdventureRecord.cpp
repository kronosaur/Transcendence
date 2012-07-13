//	CAdventureRecord.cpp
//
//	CAdventureRecord class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

CAdventureRecord::CAdventureRecord (DWORD dwAdventure, int iHighScoreCount) :
		m_dwAdventure(dwAdventure)

//	CAdventureRecord constructor

	{
	if (iHighScoreCount > 0)
		m_HighScores.InsertEmpty(iHighScoreCount);
	}

bool CAdventureRecord::FindRecordByGameID (const CString &sGameID, DWORD *retdwID) const

//	FindRecordByGameID
//
//	Looks for a game record with the given gameID. We look in the special
//	section first (callers rely on this).

	{
	int i;

	for (i = 0; i < specialIDCount; i++)
		if (strEquals(m_Special[i].GetGameID(), sGameID))
			{
			if (retdwID)
				*retdwID = specialIDFirst + i;
			return true;
			}

	for (i = 0; i < m_HighScores.GetCount(); i++)
		if (strEquals(m_HighScores[i].GetGameID(), sGameID))
			{
			if (retdwID)
				*retdwID = i;
			return true;
			}

	return false;
	}

CString CAdventureRecord::GetAdventureName (void) const

//	GetAdventureName
//
//	Get the name of the adventure

	{
	CExtension *pAdventure;

	if (!g_pUniverse->FindExtension(m_dwAdventure, 0, &pAdventure))
		return strPatternSubst(CONSTLIT("Adventure %x"), m_dwAdventure);

	return pAdventure->GetName();
	}

CGameRecord &CAdventureRecord::GetRecordAt (DWORD dwID)

//	GetRecordAt
//
//	Returns the given record

	{
	//	If this is a special ID, then return the appropriate record.

	if (dwID >= specialIDFirst && (dwID - specialIDFirst) < specialIDCount)
		return m_Special[dwID - specialIDFirst];

	//	Otherwise see if we can return a record from the high score list

	else if (dwID < (DWORD)m_HighScores.GetCount())
		return m_HighScores[dwID];

	//	Otherwise, this is an error

	else
		{
		ASSERT(false);
		return m_Special[specialIDFirst];
		}
	}

void CAdventureRecord::Init (DWORD dwAdventure)

//	Init
//
//	Initialize

	{
	m_HighScores.DeleteAll();
	m_dwAdventure = dwAdventure;
	}
