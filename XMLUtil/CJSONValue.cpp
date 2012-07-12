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

CJSONValue::CJSONValue (const CString &sValue, bool bToUTF8)

//	CJSONValue constructor

	{
	m_iType = typeString;

	if (bToUTF8)
		{
		//	See if there are any characters that we need to encode.

		char *pPos = sValue.GetASCIIZPointer();
		char *pPosEnd = pPos + sValue.GetLength();
		while (pPos < pPosEnd && (BYTE)*pPos < 0x80)
			pPos++;

		//	If we don't have to encode anything, just copy

		if (pPos == pPosEnd)
			m_pValue = CString::INTGetStorage(sValue);
		else
			{
			//	Otherwise, we encode

			CMemoryWriteStream Output;
			if (Output.Create() != NOERROR)
				m_pValue = CString::INTGetStorage(CONSTLIT("Out of memory"));
			else
				{
				pPos = sValue.GetASCIIZPointer();
				pPosEnd = pPos + sValue.GetLength();
				while (pPos < pPosEnd)
					{
					if ((BYTE)*pPos < 0x80)
						Output.Write(pPos, 1);
					else
						{
						CString sUTF8 = strEncodeW1252ToUTF8Char(*pPos);
						Output.Write(sUTF8.GetASCIIZPointer(), sUTF8.GetLength());
						}

					pPos++;
					}

				m_pValue = CString::INTGetStorage(CString(Output.GetPointer(), Output.GetLength()));
				}
			}
		}
	else
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

CString CJSONValue::AsCP1252 (void) const

//	AsCP1252
//
//	Converts to (Windows Western) CP1252 string

	{
	if (m_iType != typeString)
		return NULL_STR;

	//	Do a first pass and see if we have any high-bit characters

	CString sValue = AsString();
	char *pPos = sValue.GetASCIIZPointer();
	char *pPosEnd = pPos + sValue.GetLength();
	while (pPos < pPosEnd && (BYTE)*pPos < 0x80)
		pPos++;

	if (pPos == pPosEnd)
		return sValue;

	//	Convert

	CMemoryWriteStream Output;
	if (Output.Create() != NOERROR)
		return CONSTLIT("Out of memory");
	else
		{
		pPos = sValue.GetASCIIZPointer();
		pPosEnd = pPos + sValue.GetLength();
		while (pPos < pPosEnd)
			{
			if ((BYTE)*pPos == 0xc2)
				{
				pPos++;
				Output.Write(pPos, 1);
				}
			else if ((BYTE)*pPos == 0xc3)
				{
				pPos++;
				BYTE byCode = ((BYTE)*pPos) | 0x40;
				Output.Write((char *)&byCode, 1);
				}
			else if ((BYTE)*pPos == 0xc5)
				{
				pPos++;
				BYTE byCode;
				switch ((BYTE)*pPos)
					{
					case 0xa0:
						byCode = 0x8a;
						break;

					case 0x92:
						byCode = 0x8c;
						break;

					case 0x93:
						byCode = 0x9c;
						break;

					case 0xa1:
						byCode = 0x9a;
						break;

					case 0xb8:
						byCode = 0x9f;
						break;

					case 0xbd:
						byCode = 0x8e;
						break;

					case 0xbe:
						byCode = 0x9e;
						break;

					default:
						byCode = 0x20;
					}

				Output.Write((char *)&byCode, 1);
				}
			else if ((BYTE)*pPos == 0xc6)
				{
				pPos++;
				BYTE byCode;
				switch ((BYTE)*pPos)
					{
					case 0x92:
						byCode = 0x83;
						break;

					default:
						byCode = 0x20;
					}

				Output.Write((char *)&byCode, 1);
				}
			else if ((BYTE)*pPos == 0xcb)
				{
				pPos++;
				BYTE byCode;
				switch ((BYTE)*pPos)
					{
					case 0x86:
						byCode = 0x88;
						break;

					case 0x9c:
						byCode = 0x98;
						break;

					default:
						byCode = 0x20;
					}

				Output.Write((char *)&byCode, 1);
				}
			else if ((BYTE)*pPos == 0xe2)
				{
				pPos++;
				BYTE byCode;
				if ((BYTE)*pPos == 0x80)
					{
					pPos++;
					switch ((BYTE)*pPos)
						{
						case 0x93:
							byCode = 0x96;
							break;

						case 0x94:
							byCode = 0x97;
							break;

						case 0x98:
							byCode = 0x91;
							break;

						case 0x99:
							byCode = 0x92;
							break;

						case 0x9a:
							byCode = 0x82;
							break;

						case 0x9c:
							byCode = 0x93;
							break;

						case 0x9d:
							byCode = 0x94;
							break;

						case 0x9e:
							byCode = 0x84;
							break;

						case 0xa0:
							byCode = 0x86;
							break;

						case 0xa1:
							byCode = 0x87;
							break;

						case 0xa2:
							byCode = 0x95;
							break;

						case 0xa6:
							byCode = 0x85;
							break;

						case 0xb0:
							byCode = 0x89;
							break;

						case 0xb9:
							byCode = 0x8b;
							break;

						case 0xba:
							byCode = 0x9b;
							break;

						default:
							byCode = 0x20;
						}
					}
				else if ((BYTE)*pPos == 0x82)
					{
					pPos++;
					switch ((BYTE)*pPos)
						{
						case 0xac:
							byCode = 0x80;
							break;

						default:
							byCode = 0x20;
						}
					}
				else if ((BYTE)*pPos == 0x84)
					{
					pPos++;
					switch ((BYTE)*pPos)
						{
						case 0xa2:
							byCode = 0x99;
							break;

						default:
							byCode = 0x20;
						}
					}
				else
					{
					pPos++;
					byCode = 0x20;
					}

				Output.Write((char *)&byCode, 1);
				}
			else
				Output.Write(pPos, 1);

			pPos++;
			}

		return CString(Output.GetPointer(), Output.GetLength());
		}
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
		case typeNull:
			return 0;

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
