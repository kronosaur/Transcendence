//	CExtensionListMap.cpp
//
//	CExtensionListMap class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define DEBUG_MODE_ATTRIB					CONSTLIT("debugMode")
#define UNID_ATTRIB							CONSTLIT("unid")

void CExtensionListMap::GetList (DWORD dwAdventure, bool bDebugMode, TArray<DWORD> *retList)

//	GetList
//
//	Returns a list

	{
	SEntry *pEntry = m_Map.GetAt(dwAdventure);
	if (pEntry)
		{
		if (bDebugMode)
			*retList = pEntry->DebugList;
		else
			*retList = pEntry->List;
		}
	else
		retList->DeleteAll();
	}

void CExtensionListMap::SetList (DWORD dwAdventure, bool bDebugMode, const TArray<DWORD> &List)

//	SetList
//
//	Sets a list

	{
	SEntry *pNewEntry = m_Map.SetAt(dwAdventure);
	if (bDebugMode)
		pNewEntry->DebugList = List;
	else
		pNewEntry->List = List;
	}

ALERROR CExtensionListMap::ReadFromXML (CXMLElement *pDesc)

//	ReadFromXML
//
//	Loads it from XML.

	{
	int i, j;

	//	Loop over all lists

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pEntry = pDesc->GetContentElement(i);
		DWORD dwAdventure = (DWORD)pEntry->GetAttributeInteger(UNID_ATTRIB);
		bool bDebugMode = pEntry->GetAttributeBool(DEBUG_MODE_ATTRIB);
		CString sList = pEntry->GetContentText(0);

		TArray<CString> Values;
		ParseStringList(sList, 0, &Values);

		TArray<DWORD> List;
		List.InsertEmpty(Values.GetCount());
		for (j = 0; j < Values.GetCount(); j++)
			List[j] = strToInt(Values[j], 0);

		//	Add the entry

		SetList(dwAdventure, bDebugMode, List);
		}

	return NOERROR;
	}

ALERROR CExtensionListMap::WriteAsXML (IWriteStream *pOutput)

//	WriteAsXML
//
//	Writes to XML

	{
	ALERROR error;
	int i;

	//	Open tag

	CString sData;
	sData = CONSTLIT("\t<Extensions>\r\n");
	if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	//	Entries

	for (i = 0; i < m_Map.GetCount(); i++)
		{
		DWORD dwAdventure = m_Map.GetKey(i);
		const SEntry &Entry = m_Map[i];

		//	Write the non-debug

		if (Entry.List.GetCount() > 0)
			{
			if (error = WriteList(pOutput, dwAdventure, false, Entry.List))
				return error;
			}

		//	Write the debug list

		if (Entry.DebugList.GetCount() > 0)
			{
			if (error = WriteList(pOutput, dwAdventure, true, Entry.DebugList))
				return error;
			}
		}

	//	Close tag

	sData = CONSTLIT("\t</Extensions>\r\n");
	if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	return NOERROR;
	}

ALERROR CExtensionListMap::WriteList (IWriteStream *pOutput, DWORD dwAdventure, bool bDebugMode, const TArray<DWORD> &List)

//	WriteList
//
//	Writes a single list

	{
	ALERROR error;
	int i;

	CString sData;
	if (bDebugMode)
		sData = strPatternSubst(CONSTLIT("\t\t<Default unid=\"0x%x\" debugMode=\"true\">"), dwAdventure);
	else
		sData = strPatternSubst(CONSTLIT("\t\t<Default unid=\"0x%x\">"), dwAdventure);

	if (error = pOutput->Write(sData.GetPointer(), sData.GetLength()))
		return error;

	for (i = 0; i < List.GetCount(); i++)
		{
		CString sEntry;
		if (i == 0)
			sEntry = strPatternSubst(CONSTLIT("0x%x"), List[i]);
		else
			sEntry = strPatternSubst(CONSTLIT(", 0x%x"), List[i]);

		if (error = pOutput->Write(sEntry.GetPointer(), sEntry.GetLength()))
			return error;
		}

	sData = CONSTLIT("</Default>\r\n");
	if (error = pOutput->Write(sData.GetPointer(), sData.GetLength()))
		return error;

	return NOERROR;
	}
