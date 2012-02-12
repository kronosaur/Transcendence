//	CDataPackStruct
//
//	Implementation of CDataPackStruct object
//
//	The serialized format is as follows:
//
//	Integer 0
//		[BYTE] ItemType = Integer; ItemSize = 0-Value
//
//	8-bit Integer
//		[BYTE] ItemType = Integer; ItemSize = 8-bits
//		[BYTE] Value
//
//	16-bit Integer
//		[BYTE] ItemType = Integer; ItemSize = 16-bits
//		[WORD] Value
//
//	32-bit Integer
//		[BYTE] ItemType = Integer; ItemSize = 32-bits
//		[DWORD] Value
//
//	Null String
//		[BYTE] ItemType = String; ItemSize = 0-Value
//
//	String of less than 256 bytes
//		[BYTE] ItemType = String; ItemSize = 8-bits
//		[BYTE] Length of string
//		... String data (no NULL termination)
//
//	String of 32-bit length
//		[BYTE] ItemType = String; ItemSize = 32-bits
//		[DWORD] Length of string
//		... String data (no NULL termination)
//
//	Empty struct
//		[BYTE] ItemType = Struct; ItemSize = 0-Value
//
//	Struct of items
//		[BYTE] ItemType = Struct; ItemSize = 32-bits
//		[DWORD] Count of items
//		... Recursive struct for each item
//
//	Struct of 8-bit integers
//		[BYTE] ItemType = IntStruct; ItemSize = 8-bits
//		[DWORD] Count of integers
//		[BYTEs] one byte per integer
//
//	Struct of 16-bit integers
//		[BYTE] ItemType = IntStruct; ItemSize = 16-bits
//		[DWORD] Count of items
//		[WORDs] one for each integer
//
//	Struct of 32-bit integers
//		[BYTE] ItemType = IntStruct; ItemSize = 32-bits
//		[DWORD] Count of items
//		[DWORDs] one for each integer

#include "Alchemy.h"
#include "NetUtil.h"

#define ITEMTYPE_INTEGER				(0 << 5)
#define ITEMTYPE_STRING					(1 << 5)
#define ITEMTYPE_LIST					(2 << 5)
#define ITEMTYPE_INTLIST				(3 << 5)
#define ITEMTYPE_MASK					0xE0

#define ITEMSIZE_0						(0 << 3)
#define ITEMSIZE_8BITS					(1 << 3)
#define ITEMSIZE_16BITS					(2 << 3)
#define ITEMSIZE_32BITS					(3 << 3)
#define ITEMSIZE_MASK					0x18

CDataPackStruct::~CDataPackStruct (void)

//	Destructor

	{
	//	Iterate over every element and free the appropriate ones

	for (int i = 0; i < GetCount(); i++)
		{
		switch (GetItemType(i))
			{
			case dpiInteger:
				break;

			case dpiString:
				{
				void *pStore = (void *)GetItemData(i);
				CString::INTFreeStorage(pStore);
				break;
				}

			case dpiStruct:
				{
				CDataPackStruct *pStruct = (CDataPackStruct *)GetItemData(i);
				delete pStruct;
				break;
				}

			default:
				ASSERT(FALSE);
			}
		}
	}

void CDataPackStruct::AppendIntArray (const CIntArray &Array)

//	AppendIntArray
//
//	Appends an integer array as a sub structure

	{
	CDataPackStruct *pDest;

	AppendNewStruct(&pDest);
	for (int i = 0; i < Array.GetCount(); i++)
		pDest->AppendInteger(Array.GetElement(i));
	}

void CDataPackStruct::AppendInteger (int iData)

//	AppendInteger
//
//	Appends an integer to the structure

	{
	m_Data.AppendElement(dpiInteger, NULL);
	m_Data.AppendElement(iData, NULL);
	}

void CDataPackStruct::AppendString (const CString &sData)

//	AppendString
//
//	Appends a string to the structure

	{
	m_Data.AppendElement(dpiString, NULL);
	m_Data.AppendElement((int)CString::INTGetStorage(sData), NULL);
	}

void CDataPackStruct::AppendNewStruct (CDataPackStruct **retpStruct)

//	AppendNewStruct
//
//	Appends a new structure and returns a pointer to it

	{
	CDataPackStruct *pStruct = new CDataPackStruct;

	m_Data.AppendElement(dpiStruct, NULL);
	m_Data.AppendElement((int)pStruct, NULL);

	if (retpStruct)
		*retpStruct = pStruct;
	}

ALERROR CDataPackStruct::CreateFromBuffer (IReadBlock *pBuffer, CDataPackStruct **retpPack)

//	CreateFromBuffer
//
//	Unpacks a buffer into a new pack

	{
	ALERROR error;
	BYTE *pPos = (BYTE *)pBuffer->GetPointer(0, -1);
	BYTE *pEnd = pPos + pBuffer->GetLength();

	CDataPackStruct *pStruct = new CDataPackStruct;
	if (pStruct == NULL)
		return ERR_MEMORY;

	if (pPos + sizeof(BYTE) > pEnd)
		return ERR_FAIL;

	switch (*pPos & ITEMTYPE_MASK)
		{
		case ITEMTYPE_INTEGER:
			{
			int iValue;

			if (error = UnpackInteger(&pPos, pEnd, &iValue))
				return error;

			pStruct->AppendInteger(iValue);
			break;
			}

		case ITEMTYPE_STRING:
			{
			CString sValue;

			if (error = UnpackString(&pPos, pEnd, &sValue))
				return error;

			pStruct->AppendString(sValue);
			break;
			}

		case ITEMTYPE_INTLIST:
			{
			if (error = UnpackIntegerStruct(&pPos, pEnd, pStruct))
				return error;
			break;
			}

		case ITEMTYPE_LIST:
			{
			if (error = UnpackStruct(&pPos, pEnd, pStruct))
				return error;
			break;
			}

		default:
			return ERR_FAIL;
		}

	//	Done

	*retpPack = pStruct;

	return NOERROR;
	}

BYTE CDataPackStruct::GetSerializedStructType (void) const

//	GetSerializedStructType
//
//	Returns the type and size of struct

	{
	if (GetCount() == 0)
		return ITEMTYPE_LIST | ITEMSIZE_0;

	//	Check to see if all items are integers and calculate
	//	their maximum absolute size

	int iMaxAbs = 0;
	for (int i = 0; i < GetCount(); i++)
		{
		if (GetItemType(i) != dpiInteger)
			return ITEMTYPE_LIST | ITEMSIZE_32BITS;

		int iTest = Absolute(GetIntegerItem(i));
		if (iTest > iMaxAbs)
			iMaxAbs = iTest;
		}

	//	If we get here then we know that the list consists
	//	only of integers <= iMaxAbs

	if (iMaxAbs == 0)
		return ITEMTYPE_INTLIST | ITEMSIZE_0;
	else if (iMaxAbs < 128)
		return ITEMTYPE_INTLIST | ITEMSIZE_8BITS;
	else if (iMaxAbs < 32768)
		return ITEMTYPE_INTLIST | ITEMSIZE_16BITS;
	else
		return ITEMTYPE_INTLIST | ITEMSIZE_32BITS;
	}

CString CDataPackStruct::GetStringItem (int iIndex) const

//	GetStringItem
//
//	Returns a string item

	{
	CString sData;

	CString::INTSetStorage(sData, (void *)GetItemData(iIndex));
	return sData;
	}

ALERROR CDataPackStruct::SerializeToBuffer (IWriteStream *pStream) const

//	SerializeToBuffer
//
//	Serializes the whole pack recursively

	{
	//	If we've got an empty list then write that

	if (GetCount() == 0)
		{
		BYTE byData = ITEMTYPE_LIST | ITEMSIZE_0;
		pStream->Write((char *)&byData, sizeof(byData), NULL);
		}

	//	If we've got a single integer then write it out alone

	else if (GetCount() == 1 && GetItemType(0) == dpiInteger)
		SerializeInteger(pStream, GetIntegerItem(0));

	//	If we've got a single string then write it out alone

	else if (GetCount() == 1 && GetItemType(0) == dpiString)
		SerializeString(pStream, GetStringItem(0));

	//	Otherwise we treat it as a struct (list)

	else
		SerializeStruct(pStream, this);

	return NOERROR;
	}

CString CDataPackStruct::SerializeToString (void) const
	{
	CMemoryWriteStream Output(1000000);

	if (Output.Create() != NOERROR)
		return CString("");

	if (SerializeToBuffer(&Output) != NOERROR)
		return CString("");

	Output.Close();

	//	Done

	return CString(Output.GetPointer(), Output.GetLength());
	}

ALERROR CDataPackStruct::SerializeInteger (IWriteStream *pStream, int iValue)
	{
	int iAbsValue = Absolute(iValue);
	BYTE iType = ITEMTYPE_INTEGER;

	if (iAbsValue == 0)
		{
		iType |= ITEMSIZE_0;
		pStream->Write((char *)&iType, sizeof(iType), NULL);
		}
	else if (iAbsValue < 128)
		{
		iType |= ITEMSIZE_8BITS;
		pStream->Write((char *)&iType, sizeof(iType), NULL);

		char iOut = iValue;
		pStream->Write((char *)&iOut, sizeof(iOut), NULL);
		}
	else if (iAbsValue < 32768)
		{
		iType |= ITEMSIZE_16BITS;
		pStream->Write((char *)&iType, sizeof(iType), NULL);

		short iOut = iValue;
		pStream->Write((char *)&iOut, sizeof(iOut), NULL);
		}
	else
		{
		iType |= ITEMSIZE_32BITS;
		pStream->Write((char *)&iType, sizeof(iType), NULL);

		pStream->Write((char *)&iValue, sizeof(iValue), NULL);
		}

	return NOERROR;
	}

ALERROR CDataPackStruct::SerializeString (IWriteStream *pStream, const CString &sValue)
	{
	BYTE iType = ITEMTYPE_STRING;

	if (sValue.GetLength() == 0)
		{
		iType |= ITEMSIZE_0;
		pStream->Write((char *)&iType, sizeof(iType), NULL);
		}
	else if (sValue.GetLength() < 256)
		{
		iType |= ITEMSIZE_8BITS;
		pStream->Write((char *)&iType, sizeof(iType), NULL);

		BYTE iSize = sValue.GetLength();
		pStream->Write((char *)&iSize, sizeof(iSize), NULL);
		}
	else
		{
		iType |= ITEMSIZE_32BITS;
		pStream->Write((char *)&iType, sizeof(iType), NULL);

		DWORD iSize = sValue.GetLength();
		pStream->Write((char *)&iSize, sizeof(iSize), NULL);
		}

	pStream->Write(sValue.GetPointer(), sValue.GetLength(), NULL);

	return NOERROR;
	}

ALERROR CDataPackStruct::SerializeStruct (IWriteStream *pStream, const CDataPackStruct * const pData)
	{
	//	Check to see if all our items are integers

	BYTE byType = pData->GetSerializedStructType();
	pStream->Write((char *)&byType, sizeof(byType), NULL);

	//	If the list is empty, then we're done

	if (pData->GetCount() == 0)
		return NOERROR;

	//	Write the size of the list

	int iCount = pData->GetCount();
	pStream->Write((char *)&iCount, sizeof(iCount), NULL);

	//	If we have a normal list then just recurse

	if ((byType & ITEMTYPE_MASK) == ITEMTYPE_LIST)
		{
		for (int i = 0; i < iCount; i++)
			{
			switch (pData->GetItemType(i))
				{
				case dpiInteger:
					SerializeInteger(pStream, pData->GetIntegerItem(i));
					break;

				case dpiString:
					SerializeString(pStream, pData->GetStringItem(i));
					break;

				case dpiStruct:
					SerializeStruct(pStream, pData->GetStructItem(i));
					break;

				default:
					ASSERT(FALSE);
				}
			}
		}

	//	Otherwise, if we have an integer list, output
	//	a list of integers

	else
		{
		switch (byType & ITEMSIZE_MASK)
			{
			//	Do nothing if all values are zero

			case ITEMSIZE_0:
				break;

			case ITEMSIZE_8BITS:
				{
				for (int i = 0; i < pData->GetCount(); i++)
					{
					char iValue = pData->GetIntegerItem(i);
					pStream->Write(&iValue, sizeof(iValue), NULL);
					}
				break;
				}

			case ITEMSIZE_16BITS:
				{
				for (int i = 0; i < pData->GetCount(); i++)
					{
					short iValue = pData->GetIntegerItem(i);
					pStream->Write((char *)&iValue, sizeof(iValue), NULL);
					}
				break;
				}

			case ITEMSIZE_32BITS:
				{
				for (int i = 0; i < pData->GetCount(); i++)
					{
					int iValue = pData->GetIntegerItem(i);
					pStream->Write((char *)&iValue, sizeof(iValue), NULL);
					}
				break;
				}

			default:
				ASSERT(FALSE);
			}
		}

	return NOERROR;
	}

ALERROR CDataPackStruct::UnpackInteger (BYTE **pBuffer, BYTE *pEnd, int *retiValue)
	{
	BYTE *pPos = *pBuffer;

	if (pPos == pEnd)
		return ERR_FAIL;

	switch (*pPos & ITEMSIZE_MASK)
		{
		case ITEMSIZE_0:
			{
			pPos++;
			*retiValue = 0;
			break;
			}

		case ITEMSIZE_8BITS:
			{
			pPos++;
			if (pPos + sizeof(char) > pEnd)
				return ERR_FAIL;

			*retiValue = (int)*((char *)pPos);
			pPos++;
			break;
			}

		case ITEMSIZE_16BITS:
			{
			pPos++;
			if (pPos + sizeof(short) > pEnd)
				return ERR_FAIL;

			*retiValue = (int)*((short *)pPos);
			pPos += sizeof(short);
			break;
			}

		case ITEMSIZE_32BITS:
			{
			pPos++;
			if (pPos + sizeof(int) > pEnd)
				return ERR_FAIL;

			*retiValue = *((int *)pPos);
			pPos += sizeof(int);
			break;
			}

		default:
			return ERR_FAIL;
		}

	*pBuffer = pPos;

	return NOERROR;
	}

ALERROR CDataPackStruct::UnpackString (BYTE **pBuffer, BYTE *pEnd, CString *retsValue)
	{
	ALERROR error;
	BYTE *pPos = *pBuffer;
	int iLength;

	//	Get the length

	if (error = UnpackInteger(&pPos, pEnd, &iLength))
		return error;

	//	Now get the string

	if (pPos + iLength > pEnd)
		return ERR_FAIL;

	*retsValue = CString((char *)pPos, iLength);
	*pBuffer = pPos + iLength;

	return NOERROR;
	}

ALERROR CDataPackStruct::UnpackIntegerStruct (BYTE **pBuffer, BYTE *pEnd, CDataPackStruct *pStruct)

//	UnpackStruct
//
//	Unpacks the struct at the given pointer; updates the pointer
//	on return

	{
	BYTE *pPos = *pBuffer;

	if (pPos == pEnd)
		return ERR_FAIL;

	BYTE bySize = (*pPos & ITEMSIZE_MASK);
	int iCount;

	//	Get the number of items

	pPos++;
	if (pPos + sizeof(int) >= pEnd)
		return ERR_FAIL;

	iCount = (*(int *)pPos);
	pPos += sizeof(int);

	//	Get the integers

	for (int i = 0; i < iCount; i++)
		{
		int iValue;

		switch (bySize)
			{
			case ITEMSIZE_0:
				iValue = 0;
				break;

			case ITEMSIZE_8BITS:
				{
				if (pPos + sizeof(BYTE) > pEnd)
					return ERR_FAIL;

				iValue = (int)*((char *)pPos);
				pPos++;
				break;
				}

			case ITEMSIZE_16BITS:
				{
				if (pPos + sizeof(WORD) > pEnd)
					return ERR_FAIL;

				iValue = (int)*((short *)pPos);
				pPos += sizeof(short);
				break;
				}

			case ITEMSIZE_32BITS:
				{
				if (pPos + sizeof(DWORD) > pEnd)
					return ERR_FAIL;

				iValue = *((int *)pPos);
				pPos += sizeof(int);
				break;
				}

			default:
				return ERR_FAIL;
			}

		pStruct->AppendInteger(iValue);
		}

	//	Done

	*pBuffer = pPos;

	return NOERROR;
	}

ALERROR CDataPackStruct::UnpackStruct (BYTE **pBuffer, BYTE *pEnd, CDataPackStruct *pStruct)

//	UnpackStruct
//
//	Unpacks the struct at the given pointer; updates the pointer
//	on return

	{
	ALERROR error;
	BYTE *pPos = *pBuffer;

	if (pPos == pEnd)
		return ERR_FAIL;

	int iCount;

	//	Get the number of items in the list

	switch (*pPos & ITEMSIZE_MASK)
		{
		//	If 0-length, we're done

		case ITEMSIZE_0:
			pPos++;
			return NOERROR;

		case ITEMSIZE_32BITS:
			{
			pPos++;

			if (pPos + sizeof(int) >= pEnd)
				return ERR_FAIL;

			iCount = (*(int *)pPos);
			pPos += sizeof(int);
			break;
			}

		default:
			return ERR_FAIL;
		}

	//	Loop for each item

	for (int i = 0; i < iCount; i++)
		{
		if (pPos >= pEnd)
			return ERR_FAIL;

		switch (*pPos & ITEMTYPE_MASK)
			{
			case ITEMTYPE_INTEGER:
				{
				int iValue;
				
				if (error = UnpackInteger(&pPos, pEnd, &iValue))
					return error;

				pStruct->AppendInteger(iValue);
				break;
				}

			case ITEMTYPE_STRING:
				{
				CString sValue;

				if (error = UnpackString(&pPos, pEnd, &sValue))
					return error;

				pStruct->AppendString(sValue);
				break;
				}

			case ITEMTYPE_LIST:
				{
				CDataPackStruct *pNewStruct;

				pStruct->AppendNewStruct(&pNewStruct);

				if (error = UnpackStruct(&pPos, pEnd, pNewStruct))
					return error;

				break;
				}

			case ITEMTYPE_INTLIST:
				{
				CDataPackStruct *pNewStruct;

				pStruct->AppendNewStruct(&pNewStruct);

				if (error = UnpackIntegerStruct(&pPos, pEnd, pNewStruct))
					return error;

				break;
				}

			default:
				return ERR_FAIL;
			}
		}

	//	Done

	*pBuffer = pPos;

	return NOERROR;
	}
