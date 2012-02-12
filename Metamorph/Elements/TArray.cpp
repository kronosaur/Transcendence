//	TArray.cpp
//
//	Dynamic array class
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"

CArrayBase::CArrayBase (HANDLE hHeap) : m_hHeap(hHeap),
		m_iSize(0),
		m_iAllocSize(0),
		m_pArray(NULL)

//	CArrayBase constructor

	{
	if (m_hHeap == NULL)
		m_hHeap = ::GetProcessHeap();
	}

CArrayBase::~CArrayBase (void)

//	CArrayBase destructor

	{
	if (m_pArray)
		::HeapFree(m_hHeap, 0, m_pArray);
	}

void CArrayBase::DeleteBytes (int iOffset, int iLength)

//	Delete
//
//	Delete iLength bytes in the array at the given offset

	{
	char *pSource;
	char *pDest;
	int i;

	//	Move stuff down

	pSource = m_pArray + iOffset + iLength;
	pDest = m_pArray + iOffset;
	for (i = 0; i < m_iSize - (iOffset + iLength); i++)
		*pDest++ = *pSource++;

	//	Done

	m_iSize -= iLength;
	}

void CArrayBase::InsertBytes (int iOffset, void *pData, int iLength, int iAllocQuantum)

//	Insert
//
//	Insert the given data at the offset

	{
	int i;
	char *pSource;
	char *pDest;

	if (iOffset == -1)
		iOffset = m_iSize;

	ASSERT(iOffset >= 0 && iOffset <= m_iSize);

    //	Reallocate if necessary

	Resize(m_iSize + iLength, true, iAllocQuantum);
    
	//	Move the array up
    
	pSource = m_pArray + m_iSize-1;
	pDest = pSource + iLength;
    for (i = m_iSize-1; i >= iOffset; i--)
		*pDest-- = *pSource--;

	//	Copy the new values

	if (pData)
		{
		pSource = (char *)pData;
		pDest = m_pArray + iOffset;
		for (i = 0; i < iLength; i++)
			*pDest++ = *pSource++;
		}

	//	Done
    
	m_iSize += iLength;
	}

void CArrayBase::Resize (int iNewSize, bool bPreserve, int iAllocQuantum)

//	Resize
//
//	Resize the array so that it is at least the given new size

	{
	ASSERT(iAllocQuantum > 0);

	//	If we don't have storage yet, allocate a new one

	if (m_pArray == NULL && iNewSize > 0)
		{
		m_iAllocSize = AlignUp(iNewSize, iAllocQuantum);
		m_pArray = (char *)::HeapAlloc(m_hHeap, 0, m_iAllocSize);
		if (m_pArray == NULL)
			throw CException(errOutOfMemory);
		}

	//	If we're not big enough, reallocate the array

	if (m_iAllocSize < iNewSize)
		{
		char *pNewArray;
		int iNewAllocSize;

		iNewAllocSize = AlignUp(iNewSize, iAllocQuantum);
		pNewArray = (char *)::HeapAlloc(m_hHeap, 0, iNewAllocSize);
		if (pNewArray == NULL)
			throw CException(errOutOfMemory);

		//	Transfer the contents, if necessary

		if (bPreserve)
			{
			int i;
			char *pSource = m_pArray;
			char *pDest = pNewArray;

			for (i = 0; i < m_iSize; i++)
				*pDest++ = *pSource++;
			}

		//	Free the original buffer

		::HeapFree(m_hHeap, 0, m_pArray);
		m_pArray = pNewArray;
		m_iAllocSize = iNewAllocSize;
		}
	}
