//	CHighScoreList.cpp
//
//	CHighScoreList class

#include "PreComp.h"
#include "Transcendence.h"

#define LAST_PLAYER_NAME_ATTRIB				CONSTLIT("lastPlayerName")
#define LAST_PLAYER_GENOME_ATTRIB			CONSTLIT("lastPlayerGenome")

CHighScoreList::CHighScoreList (void) : m_bModified(false),
		m_iCount(0)

//	CHighScoreList constructor

	{
	}

ALERROR CHighScoreList::Load (const CString &sFilename)

//	Load
//
//	Load the high score list

	{
	ALERROR error;

	m_iCount = 0;

	//	Load XML

	CFileReadBlock DataFile(sFilename);
	CXMLElement *pData;
	CString sError;
	if (error = CXMLElement::ParseXML(&DataFile, &pData, &sError))
		//	Means we can't find it or is corrupt...
		return NOERROR;

	//	Fill the structures

	for (int i = 0; i < pData->GetContentElementCount(); i++)
		{
		if (error = m_List[m_iCount].InitFromXML(pData->GetContentElement(i)))
			return error;

		m_iCount++;
		}

	m_bModified = false;

	//	Done

	delete pData;

	return NOERROR;
	}

ALERROR CHighScoreList::Save (const CString &sFilename)

//	Save
//
//	Save the high score list

	{
	ALERROR error;

	if (m_bModified)
		{
		CFileWriteStream DataFile(sFilename, FALSE);

		if (error = DataFile.Create())
			return error;

		//	Write the XML header

		CString sData = strPatternSubst(CONSTLIT("<?xml version=\"1.0\"?>\r\n\r\n<TranscendenceHighScores>\r\n\r\n"));
		if (error = DataFile.Write(sData.GetPointer(), sData.GetLength(), NULL))
			return error;

		//	Loop over scores

		for (int i = 0; i < m_iCount; i++)
			{
			if (error = m_List[i].WriteToXML(DataFile))
				return error;
			}

		//	Done

		sData = CONSTLIT("\r\n</TranscendenceHighScores>\r\n");
		if (error = DataFile.Write(sData.GetPointer(), sData.GetLength(), NULL))
			return error;

		if (error = DataFile.Close())
			return error;
		}

	return NOERROR;
	}

int CHighScoreList::AddEntry (const CGameRecord &NewEntry)

//	AddEntry
//
//	Add another entry to the high score

	{
	int i, j;

	//	Score of 0 doesn't count

	if (NewEntry.GetScore() == 0)
		return -1;

	//	Modify the epitaph in the entry

	CGameRecord ModifiedEntry = NewEntry;
	if (strEquals(strWord(ModifiedEntry.GetEndGameEpitaph(), 0), CONSTLIT("was")))
		ModifiedEntry.SetEndGameEpitaph(strSubString(ModifiedEntry.GetEndGameEpitaph(), 4, -1));

	//	Find a spot on the list

	for (i = 0; i < m_iCount; i++)
		{
		if (ModifiedEntry.GetScore() > m_List[i].GetScore())
			break;
		}

	//	If we are the end of the list, then we didn't make the
	//	high score list.

	if (i == MAX_SCORES)
		return -1;

	//	Otherwise, move all scores below us by one

	m_bModified = true;

	if (m_iCount < MAX_SCORES)
		m_iCount++;

	for (j = m_iCount-2; j >= i; j--)
		m_List[j+1] = m_List[j];

	m_List[i] = ModifiedEntry;

	return i;
	}

