//	CINTDynamicArray.cpp
//
//	Implementation of dynamic array of bytes

#include "Kernel.h"
#include "KernelObjID.h"

CINTDynamicArray::CINTDynamicArray (void) :
		m_iLength(0),
		m_iAllocSize(0),
		m_pArray(NULL)

//	CINTDynamicArray constructor

	{
	m_hHeap = GetProcessHeap();
	}

CINTDynamicArray::CINTDynamicArray (HANDLE hHeap) :
		m_iLength(0),
		m_iAllocSize(0),
		m_hHeap(hHeap),
		m_pArray(NULL)

//	CINTDynamicArray constructor

	{
	}

CINTDynamicArray::~CINTDynamicArray (void)

//	CINTDynamicArray destructor

	{
	if (m_pArray)
		HeapFree(m_hHeap, 0, m_pArray);
	}

ALERROR CINTDynamicArray::Delete (int iOffset, int iLength)

//	Delete
//
//	Deletes a part of the array

	{
	BYTE *pSource;
	BYTE *pDest;
	int i;

	//	Move stuff down

	pSource = m_pArray + iOffset + iLength;
	pDest = m_pArray + iOffset;
	for (i = 0; i < m_iLength - (iOffset + iLength); i++)
		*pDest++ = *pSource++;

	//	Done

	m_iLength -= iLength;

	return NOERROR;
	}

ALERROR CINTDynamicArray::Insert (int iOffset, BYTE *pData, int iLength, int iAllocQuantum)

//	Insert
//
//	Inserts the bytes at the given offset. If iOffset is -1, the bytes are inserted
//	at the end of the array

	{
	ALERROR error;
	int i;
	BYTE *pSource;
	BYTE *pDest;

	if (iOffset == -1)
		iOffset = m_iLength;

	ASSERT(iOffset >= 0 && iOffset <= m_iLength);

    //	Reallocate if necessary

	if (error = Resize(m_iLength + iLength, TRUE, iAllocQuantum))
		return error;
    
	//	Move the array up
    
	pSource = m_pArray + m_iLength-1;
	pDest = pSource + iLength;
    for (i = m_iLength-1; i >= iOffset; i--)
		*pDest-- = *pSource--;

	//	Copy the new values

	if (pData)
		{
		pSource = pData;
		pDest = m_pArray + iOffset;
		for (i = 0; i < iLength; i++)
			*pDest++ = *pSource++;
		}

	//	Done
    
	m_iLength += iLength;

	return NOERROR;
	}

ALERROR CINTDynamicArray::Resize (int iNewSize, BOOL bPreserve, int iAllocQuantum)

//	Resize
//
//	Resizes the array to the new size

	{
	ASSERT(iNewSize > 0);
	ASSERT(iAllocQuantum > 0);

	//	If we don't have storage yet, allocate a new one

	if (m_pArray == NULL)
		{
		m_iAllocSize = AlignUp(iNewSize, iAllocQuantum);
		m_pArray = (BYTE *)HeapAlloc(m_hHeap, 0, m_iAllocSize);
		if (m_pArray == NULL)
			return ERR_MEMORY;
		}

	//	If we're not big enough, reallocate the array

	if (m_iAllocSize < iNewSize)
		{
		BYTE *pNewArray;
		int iNewAllocSize;

		iNewAllocSize = AlignUp(iNewSize, iAllocQuantum);
		pNewArray = (BYTE *)HeapAlloc(m_hHeap, 0, iNewAllocSize);
		if (pNewArray == NULL)
			return ERR_MEMORY;

		//	Transfer the contents, if necessary

		if (bPreserve)
			{
			int i;
			BYTE *pSource = m_pArray;
			BYTE *pDest = pNewArray;

			for (i = 0; i < m_iLength; i++)
				*pDest++ = *pSource++;
			}

		//	Free the original buffer

		HeapFree(m_hHeap, 0, m_pArray);
		m_pArray = pNewArray;
		m_iAllocSize = iNewAllocSize;
		}

	return NOERROR;
	}

