//	CBabelData.cpp
//
//	CBabelData class
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"
#include "Babel.h"
#include "BabelParser.h"

struct StructData
	{
	int iRefCount;
	TStringMap<CBabelData> Fields;
	};

struct ListData
	{
	int iRefCount;
	TArray<CBabelData> List;
	};

CBabelData g_Nil;

CBabelData::CBabelData (void) : m_Type(dtNil),
		m_dwData(0)
	{
	}

CBabelData::CBabelData (int iInteger) : m_Type(dtInteger),
		m_dwData((DWORD)iInteger)
	{
	}

CBabelData::CBabelData (const CString &sString) : m_Type(dtString),
		m_dwData(0)
	{
	CString *pString = (CString *)&m_dwData;
	*pString = sString;
	}

CBabelData::CBabelData (DataElementTypes Type, const CString &sString) : m_Type(Type),
		m_dwData(0)
	{
	ASSERT(Type == dtCode || Type == dtString);
	CString *pString = (CString *)&m_dwData;
	*pString = sString;
	}

CBabelData::CBabelData (DataElementTypes Type) : m_Type(Type)
	{
	switch (m_Type)
		{
		case dtNil:
		case dtInteger:
		case dtString:
		case dtCode:
			m_dwData = 0;
			break;

		case dtList:
			{
			ListData *pData = new ListData;
			pData->iRefCount = 1;
			m_dwData = (DWORD)pData;
			break;
			}

		case dtStructure:
			{
			StructData *pData = new StructData;
			pData->iRefCount = 1;
			m_dwData = (DWORD)pData;
			break;
			}

		default:
			ASSERT(false);
		}
	}

CBabelData::CBabelData (const CBabelData &sData)
	{
	CopyData(sData);
	}

CBabelData::~CBabelData (void)
	{
	CleanUp();
	}

CBabelData &CBabelData::operator= (const CBabelData &sData)
	{
	CleanUp();
	CopyData(sData);
	return *this;
	}

void CBabelData::AddElement (const CBabelData &Data)

//	AddElement
//
//	Adds an element to a list

	{
	ASSERT(m_Type == dtList);
	ListData *pData = (ListData *)m_dwData;

	pData->List.Insert(Data);
	}

void CBabelData::AddField (const CString &sFieldName, const CBabelData &Data)

//	AddField
//
//	Adds a field to a structure

	{
	ASSERT(m_Type == dtStructure);
	StructData *pData = (StructData *)m_dwData;

	pData->Fields.Insert(sFieldName, Data);
	}

void CBabelData::CleanUp (void)

//	CleanUp
//
//	Deallocates appropriately but does not reset the member
//	variables.

	{
	switch (m_Type)
		{
		case dtNil:
		case dtInteger:
			break;

		case dtString:
		case dtCode:
			{
			CString *pString = (CString *)&m_dwData;
			pString->Truncate(0);
			break;
			}

		case dtList:
			{
			ListData *pData = (ListData *)m_dwData;
			if (--pData->iRefCount == 0)
				delete pData;
			break;
			}

		case dtStructure:
			{
			StructData *pData = (StructData *)m_dwData;
			if (--pData->iRefCount == 0)
				delete pData;
			break;
			}

		default:
			ASSERT(false);
		}
	}

void CBabelData::CopyData (const CBabelData &sData)

//	CopyData
//
//	Copies data from another CBabelData. We assume that our
//	data structures are uninitialized.

	{
	m_Type = sData.m_Type;
	switch (m_Type)
		{
		case dtNil:
		case dtInteger:
			m_dwData = sData.m_dwData;
			break;

		case dtString:
		case dtCode:
			{
			m_dwData = 0;
			CString *pSource = (CString *)&sData.m_dwData;
			CString *pDest = (CString *)&m_dwData;

			*pDest = *pSource;
			break;
			}

		case dtList:
			{
			ListData *pData = (ListData *)sData.m_dwData;
			m_dwData = (DWORD)pData;
			pData->iRefCount++;
			break;
			}

		case dtStructure:
			{
			StructData *pData = (StructData *)sData.m_dwData;
			m_dwData = (DWORD)pData;
			pData->iRefCount++;
			break;
			}

		default:
			ASSERT(false);
		}
	}

const CBabelData &CBabelData::GetElement (int iIndex) const

//	GetElement
//
//	Returns the element at the given index

	{
	switch (m_Type)
		{
		case dtInteger:
		case dtString:
		case dtCode:
		case dtStructure:
			return *this;

		case dtList:
			{
			ListData *pData = (ListData *)m_dwData;

			if (iIndex < 0 || iIndex >= pData->List.GetCount())
				return g_Nil;
			else
				return pData->List[iIndex];
			}

		case dtNil:
		default:
			ASSERT(false);
			return *this;
		}
	}

const CBabelData &CBabelData::GetField (const CString &sFieldName) const

//	GetField
//
//	Returns the element for the given field

	{
	switch (m_Type)
		{
		case dtStructure:
			{
			StructData *pData = (StructData *)m_dwData;
			CBabelData *pValue = pData->Fields.Find(sFieldName);
			if (pValue)
				return *pValue;
			else
				return g_Nil;
			}

		default:
			return g_Nil;
		}
	}

int CBabelData::GetElementCount (void) const

//	GetElementCount
//
//	Returns the number of elements

	{
	switch (m_Type)
		{
		case dtNil:
			return 0;

		case dtInteger:
		case dtString:
		case dtCode:
			return 1;

		case dtList:
			{
			ListData *pData = (ListData *)m_dwData;
			return pData->List.GetCount();
			}

		case dtStructure:
			return 1;

		default:
			ASSERT(false);
			return 0;
		}
	}

CBabelData ReadBabel (IMemoryBlock &Input)

//	ReadBabel
//
//	Reads and loads babel text. Throws CException if there are errors

	{
	char *pPos = Input.GetPointer();
	char *pEndPos = pPos + Input.GetLength();

	CParser theParser(pPos, pEndPos);

	return theParser.Parse();
	}
