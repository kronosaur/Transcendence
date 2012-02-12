//	CJSONValue.cpp
//
//	CJSONValue class
//	Copyright (c) 2011 by Kronosaur Productions, LLC. All Rights Reserved.

#include <windows.h>
#include "Alchemy.h"
#include "JSONUtil.h"

typedef TSortMap<CString, CJSONValue> ObjectType;
typedef TArray<CJSONValue> ArrayType;

CJSONValue g_NullValue;

CJSONValue::CJSONValue (Types iType)

//	CJSONValue constructor

	{
	m_iType = iType;

	switch (iType)
		{
		case typeString:
			m_pValue = CString::INTGetStorage(NULL_STR);
			break;

		case typeNumber:
			m_pValue = new double (0.0);
			break;

		case typeObject:
			m_pValue = new ObjectType;
			break;

		case typeArray:
			m_pValue = new ArrayType;
			break;

		case typeTrue:
		case typeFalse:
		case typeNull:
			m_pValue = NULL;
			break;

		default:
			ASSERT(false);
		}
	}

CJSONValue::CJSONValue (const CJSONValue &Source)

//	CJSONValue constructor

	{
	Copy(Source);
	}

CJSONValue::CJSONValue (const CString &sValue)

//	CJSONValue constructor

	{
	m_iType = typeString;
	m_pValue = CString::INTGetStorage(sValue);
	}

CJSONValue::CJSONValue (double rValue)

//	CJSONValue constructor

	{
	m_iType = typeNumber;
	m_pValue = new double (rValue);
	}

CJSONValue::CJSONValue (int iValue)

//	CJSONValue constructor

	{
	m_iType = typeNumber;
	m_pValue = new double ((double)iValue);
	}

CJSONValue::~CJSONValue (void)

//	CJSONValue destructor

	{
	CleanUp();
	}

CJSONValue &CJSONValue::operator= (const CJSONValue &Source)

//	operator =

	{
	CleanUp();
	Copy(Source);
	return *this;
	}

void CJSONValue::CleanUp (void)

//	CleanUp
//
//	Deletes and sets to null

	{
	switch (m_iType)
		{
		case typeString:
			CString::INTFreeStorage(m_pValue);
			break;

		case typeNumber:
			delete (double *)m_pValue;
			break;

		case typeObject:
			delete (ObjectType *)m_pValue;
			break;

		case typeArray:
			delete (ArrayType *)m_pValue;
			break;

		case typeTrue:
		case typeFalse:
		case typeNull:
			break;

		default:
			ASSERT(false);
		}

	m_iType = typeNull;
	m_pValue = NULL;
	}

void CJSONValue::Copy (const CJSONValue &Source)

//	Copy
//
//	Makes a copy (we assume that we are initialized)

	{
	m_iType = Source.m_iType;

	switch (Source.m_iType)
		{
		case typeString:
			m_pValue = CString::INTCopyStorage(Source.m_pValue);
			break;

		case typeNumber:
			m_pValue = new double (*(double *)Source.m_pValue);
			break;

		case typeObject:
			m_pValue = new ObjectType(*(ObjectType *)Source.m_pValue);
			break;

		case typeArray:
			m_pValue = new ArrayType(*(ArrayType *)Source.m_pValue);
			break;

		case typeTrue:
		case typeFalse:
		case typeNull:
			m_pValue = NULL;
			break;

		default:
			ASSERT(false);
		}
	}

int CJSONValue::GetCount (void) const

//	GetCount
//
//	Returns the number of elements

	{
	switch (m_iType)
		{
		case typeArray:
			{
			ArrayType *pArray = (ArrayType *)m_pValue;
			return pArray->GetCount();
			}

		case typeObject:
			{
			ObjectType *pObj = (ObjectType *)m_pValue;
			return pObj->GetCount();
			}

		default:
			return 1;
		}
	}

const CJSONValue &CJSONValue::GetElement (int iIndex) const

//	GetElement
//
//	Returns the element by index

	{
	switch (m_iType)
		{
		case typeArray:
			{
			ArrayType *pArray = (ArrayType *)m_pValue;
			if (iIndex < 0 || iIndex >= pArray->GetCount())
				return g_NullValue;

			return pArray->GetAt(iIndex);
			}

		case typeObject:
			{
			ObjectType *pObj = (ObjectType *)m_pValue;
			if (iIndex < 0 || iIndex >= pObj->GetCount())
				return g_NullValue;

			return pObj->GetValue(iIndex);
			}

		default:
			return g_NullValue;
		}
	}

const CJSONValue &CJSONValue::GetElement (const CString &sKey) const

//	GetElement
//
//	Returns the element by key

	{
	switch (m_iType)
		{
		case typeObject:
			{
			ObjectType *pObj = (ObjectType *)m_pValue;

			CJSONValue *pValue = pObj->GetAt(sKey);
			if (pValue == NULL)
				return g_NullValue;

			return *pValue;
			}

		default:
			return g_NullValue;
		}
	}

const CString &CJSONValue::GetKey (int iIndex) const

//	GetKey
//
//	Gets the key of the given element

	{
	switch (m_iType)
		{
		case typeObject:
			{
			ObjectType *pObj = (ObjectType *)m_pValue;
			if (iIndex < 0 || iIndex >= pObj->GetCount())
				return NULL_STR;

			return pObj->GetKey(iIndex);
			}

		default:
			return NULL_STR;
		}
	}

bool CJSONValue::FindElement (const CString &sKey, CJSONValue *retValue) const

//	FindElement
//
//	Finds a field in a structure

	{
	switch (m_iType)
		{
		case typeObject:
			{
			ObjectType *pObj = (ObjectType *)m_pValue;

			CJSONValue *pValue = pObj->GetAt(sKey);
			if (pValue == NULL)
				return false;

			if (retValue)
				*retValue = *pValue;

			return true;
			}

		default:
			return false;
		}
	}

void CJSONValue::Insert (const CJSONValue &Source)

//	Insert
//
//	Appends to the end of an array

	{
	ASSERT(m_iType == typeArray);
	ArrayType *pArray = (ArrayType *)m_pValue;

	pArray->Insert(Source);
	}

void CJSONValue::Insert (const CString &sKey, const CJSONValue &Source)

//	Insert
//
//	Appends to a key to a structure

	{
	ASSERT(m_iType == typeObject);
	ObjectType *pObj = (ObjectType *)m_pValue;

	pObj->Insert(sKey, Source);
	}

void CJSONValue::InsertHandoff (CJSONValue &Source)

//	InsertHandoff
//
//	Appends to the end of an array

	{
	ASSERT(m_iType == typeArray);
	ArrayType *pArray = (ArrayType *)m_pValue;

	CJSONValue *pNewValue = pArray->Insert();
	pNewValue->TakeHandoff(Source);
	}

void CJSONValue::InsertHandoff (const CString &sKey, CJSONValue &Source)

//	InsertHandoff
//
//	Appends to a key to a structure

	{
	ASSERT(m_iType == typeObject);
	ObjectType *pObj = (ObjectType *)m_pValue;

	CJSONValue *pNewValue = pObj->Insert(sKey);
	pNewValue->TakeHandoff(Source);
	}

void CJSONValue::TakeHandoff (CJSONValue &Source)

//	TakeHandoff
//
//	Makes a copy and clobbers the source

	{
	CleanUp();

	m_iType = Source.m_iType;
	m_pValue = Source.m_pValue;

	Source.m_iType = typeNull;
	Source.m_pValue = NULL;
	}
