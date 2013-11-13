//	CMultiverseNews.cpp
//
//	CMultiverseNews Class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define FILENAME_NEWS							CONSTLIT("News.xml")

#define FORMAT_INTERNET							CONSTLIT("internet")

const int CYCLE_LENGTH =						90;		//	Days before we start showing same news again
const int MAX_READ_COUNT_PER_CYCLE =			6;		//	Number of times we show news in a cycle.

void CMultiverseNews::DeleteAll (void)

//	DeleteAll
//
//	Delete all entries in the collection

	{
	int i;

	for (i = 0; i < m_List.GetCount(); i++)
		delete m_List[i];

	m_List.DeleteAll();
	}

ALERROR CMultiverseNews::LoadReadState (const CString &sFilespec, CString *retsError)

//	LoadReadState
//
//	Loads the read state from a file.

	{
	ALERROR error;
	int i;

	m_ReadState.DeleteAll();
	m_bReadStateModified = false;

	//	Load XML

	CFileReadBlock DataFile(sFilespec);
	CXMLElement *pData;
	if (error = CXMLElement::ParseXML(&DataFile, &pData, retsError))
		{
		//	ERR_NOTFOUND means that we couldn't find the file. In that case, 
		//	leave it empty.

		if (error == ERR_NOTFOUND)
			return NOERROR;

		//	Otherwise, it means that we got an error parsing the file.
		//	Return the error.

		else
			return error;
		}

	//	Loop over all elements

	for (i = 0; i < pData->GetContentElementCount(); i++)
		{
		CXMLElement *pRow = pData->GetContentElement(i);

		CString sID = pRow->GetAttribute(CONSTLIT("id"));
		if (sID.IsBlank())
			continue;

		SUserReadState *pState = m_ReadState.SetAt(sID);

		pState->iReadCount = pRow->GetAttributeInteger(CONSTLIT("readCount"));
		if (!pState->LastRead.Parse(FORMAT_INTERNET, pRow->GetAttribute(CONSTLIT("lastRead"))))
			{
			m_ReadState.DeleteAt(sID);
			continue;
			}
		}

	//	Done

	delete pData;

	return NOERROR;
	}

ALERROR CMultiverseNews::Save (const CString &sCacheFilespec, CString *retsResult)

//	Save
//
//	Saves to a file.

	{
	//	Save the user state

	CString sReadStateFilespec = pathAddComponent(sCacheFilespec, FILENAME_NEWS);
	if (SaveReadState(sReadStateFilespec, retsResult) != NOERROR)
		return ERR_FAIL;

	return NOERROR;
	}

ALERROR CMultiverseNews::SaveReadState (const CString &sFilespec, CString *retsError)

//	SaveReadState
//
//	Saves to a file.

	{
	ALERROR error;
	int i;

	if (!m_bReadStateModified)
		return NOERROR;

	//	Create the file

	CFileWriteStream DataFile(sFilespec, FALSE);
	if (error = DataFile.Create())
		{
		if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to create file: %s."), sFilespec);
		return error;
		}

	//	Write the XML header

	CString sData = CONSTLIT("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n\r\n<TranscendenceNews>\r\n\r\n");
	if (error = DataFile.Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	//	Loop over data

	for (i = 0; i < m_ReadState.GetCount(); i++)
		{
		const CString &sID = m_ReadState.GetKey(i);
		const SUserReadState &State = m_ReadState.GetValue(i);

		//	Compose option element and write

		sData = strPatternSubst(CONSTLIT("\t<News id=\"%s\" readCount=\"%d\" lastRead=\"%s\"/>\r\n"),
				strToXMLText(sID),
				State.iReadCount,
				strToXMLText(State.LastRead.Format(FORMAT_INTERNET)));

		if (error = DataFile.Write(sData.GetPointer(), sData.GetLength(), NULL))
			{
			if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to write to file: %s."), sFilespec);
			return error;
			}
		}

	//	Done

	sData = CONSTLIT("\r\n</TranscendenceNews>\r\n");
	if (error = DataFile.Write(sData.GetPointer(), sData.GetLength(), NULL))
		{
		if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to write to file: %s."), sFilespec);
		return error;
		}

	if (error = DataFile.Close())
		{
		if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to close file: %s."), sFilespec);
		return error;
		}

	m_bReadStateModified = false;

	return NOERROR;
	}

ALERROR CMultiverseNews::SetNews (const CJSONValue &Data, const CString &sCacheFilespec, TSortMap<CString, CString> *retDownloads, CString *retsResult)

//	SetNews
//
//	Sets news from Multiverse.

	{
	DEBUG_TRY

	int i;
	CString sError;

	//	Discard current news

	DeleteAll();

	//	Read the user read state

	CString sReadStateFilespec = pathAddComponent(sCacheFilespec, FILENAME_NEWS);
	if (LoadReadState(sReadStateFilespec, &sError) != NOERROR)
		::kernelDebugLogMessage(sError.GetASCIIZPointer());

	//	Read all news entries

	for (i = 0; i < Data.GetCount(); i++)
		{
		const CJSONValue &Entry = Data.GetElement(i);

		//	Create a news entry and add to our collection. If we get any errors
		//	then we just ignore them.

		CMultiverseNewsEntry *pNewEntry;
		if (CMultiverseNewsEntry::CreateFromJSON(Entry, &pNewEntry, &sError) != NOERROR)
			{
			::kernelDebugLogMessage("ERROR: Unable to load Multiverse news entry: %s.", sError);
			continue;
			}

		//	Look up this entry in our read state list

		SUserReadState *pState = m_ReadState.SetAt(pNewEntry->GetID());
		if (pState)
			{
			CTimeSpan SinceLastRead = timeSpan(pState->LastRead, CTimeDate(CTimeDate::Now));

			//	If it's been more than 90 days since we showed this to the user,
			//	then show it again.

			if (SinceLastRead.Days() >= CYCLE_LENGTH)
				pState->iReadCount = 0;

			//	If we've shown this news item less than 6 times, then it is OK
			//	to show it.

			else if (pState->iReadCount < MAX_READ_COUNT_PER_CYCLE)
				NULL;

			//	Otherwise, we don't bother showing this news

			else
				continue;
			}

		//	Add to the news

		Insert(pNewEntry);

		//	See if we have the required cached images. If not, we add to the
		//	list of files to download.

		pNewEntry->FindImages(sCacheFilespec, retDownloads);
		}

	return NOERROR;

	DEBUG_CATCH
	}

void CMultiverseNews::ShowNews (CMultiverseNewsEntry *pEntry)

//	ShowNews
//
//	The given entry has been shown to the player.

	{
	pEntry->SetShown();

	SUserReadState *pState = m_ReadState.GetAt(pEntry->GetID());
	if (pState)
		{
		pState->iReadCount++;
		pState->LastRead = CTimeDate(CTimeDate::Now);
		m_bReadStateModified = true;
		}
	}
