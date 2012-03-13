//	CDeviceStorage.cpp
//
//	CDeviceStorage class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

const CString &CDeviceStorage::GetData (DWORD dwExtension, const CString &sAttrib) const

//	GetData
//
//	Returns data for the given key

	{
	int iIndex;
	if (!m_Storage.FindPos(MakeKey(dwExtension, sAttrib), &iIndex))
		return NULL_STR;

	return m_Storage[iIndex];
	}

bool CDeviceStorage::FindData (DWORD dwExtension, const CString &sAttrib, CString *retsData) const

//	FindData
//
//	Looks for the data in the storgage. Returns TRUE if found.

	{
	return m_Storage.Find(MakeKey(dwExtension, sAttrib), retsData);
	}

ALERROR CDeviceStorage::Load (const CString &sFilespec, CString *retsError)

//	Load
//
//	Loads the storage from a file

	{
	ALERROR error;
	int i;

	m_Storage.DeleteAll();
	m_bModified = false;

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

		CString sKey = pRow->GetAttribute(CONSTLIT("key"));
		const CString &sData = pRow->GetContentText(0);

		if (!sKey.IsBlank() && !sData.IsBlank())
			m_Storage.SetAt(sKey, sData);
		}

	//	Done

	return NOERROR;
	}

CString CDeviceStorage::MakeKey (DWORD dwExtension, const CString &sAttrib) const

//	MakeKey
//
//	Makes a key from an extension UNID and attribute

	{
	return strPatternSubst(CONSTLIT("%08x_%s"), dwExtension, sAttrib);
	}

ALERROR CDeviceStorage::Save (const CString &sFilespec)

//	Save
//
//	Saves the storage to a file

	{
	ALERROR error;
	int i;

	if (!m_bModified)
		return NOERROR;

	//	Create the file

	CFileWriteStream DataFile(sFilespec, FALSE);
	if (error = DataFile.Create())
		return error;

	//	Write the XML header

	CString sData = CONSTLIT("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n\r\n<TranscendenceStorage>\r\n\r\n");
	if (error = DataFile.Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	//	Loop over data

	for (i = 0; i < m_Storage.GetCount(); i++)
		{
		//	Don't bother saving if our current value is nil or blank

		if (m_Storage[i].IsBlank() || strEquals(m_Storage[i], CONSTLIT("nil")))
			continue;

		//	Compose option element and write

		sData = strPatternSubst(CONSTLIT("\t<Row key=\"%s\">%s</Row>\r\n"),
				strToXMLText(m_Storage.GetKey(i)),
				strToXMLText(m_Storage[i]));

		if (error = DataFile.Write(sData.GetPointer(), sData.GetLength(), NULL))
			return error;
		}

	//	Done

	sData = CONSTLIT("\r\n</TranscendenceStorage>\r\n");
	if (error = DataFile.Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	if (error = DataFile.Close())
		return error;

	return NOERROR;
	}

bool CDeviceStorage::SetData (DWORD dwExtension, const CString &sAttrib, const CString &sData)

//	SetData
//
//	Sets the data. Returns TRUE if successful (this may fail if the extension 
//	or attribute are not valid (e.g., is extension UNID is 0).

	{
	CString sKey;
	if (!ValidateAndMakeKey(dwExtension, sAttrib, &sKey))
		return false;

	m_Storage.SetAt(sKey, sData);
	m_bModified = true;

	return true;
	}

bool CDeviceStorage::ValidateAndMakeKey (DWORD dwExtension, const CString &sAttrib, CString *retsKey) const

//	ValidateAndMakeKey
//
//	Make sure the extension and attribute are valid

	{
	//	Must have a valid extension and attribute

	if (dwExtension == 0 || sAttrib.IsBlank())
		return false;

	if (retsKey)
		*retsKey = MakeKey(dwExtension, sAttrib);

	return true;
	}
