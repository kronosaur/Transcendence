//	TMap.cpp
//
//	Hash table class
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"

CMapBase::CMapBase (int iTableSize) : m_iTableSize(iTableSize), m_pTable(NULL), m_iCount(0)

//	CMapBase constructor

	{
	typedef EntryBase *EntryBasePtr;
	m_pTable = new EntryBasePtr [iTableSize];
	for (int i = 0; i < iTableSize; i++)
		m_pTable[i] = NULL;
	}

CMapBase::~CMapBase (void)

//	CMapBase destructor

	{
	delete [] m_pTable;
	}

CMapBase::EntryBase *CMapBase::DeleteEntry (void *pVoidKey, void *pKey, int iKeyLen)

//	DeleteEntry
//
//	Removes the entry from the map and returns a pointer to
//	the entry

	{
	int iSlot;
	EntryBase *pPrevEntry;
	EntryBase *pEntryToDelete = FindEntry(pVoidKey, pKey, iKeyLen, &iSlot, &pPrevEntry);
	if (pEntryToDelete)
		{
		if (pPrevEntry)
			pPrevEntry->pNext = pEntryToDelete->pNext;
		else
			m_pTable[iSlot] = pEntryToDelete->pNext;

		m_iCount--;
		return pEntryToDelete;
		}
	else
		return NULL;
	}

CMapBase::EntryBase *CMapBase::FindEntry (void *pVoidKey, void *pKey, int iKeyLen, int *retiSlot, EntryBase **retpPrevEntry) const

//	FindEntry
//
//	Find the entry with the given key

	{
	int iSlot = Hash(pKey, iKeyLen);

	EntryBase *pEntryFound = (EntryBase *)m_pTable[iSlot];
	EntryBase *pPrevEntry = NULL;
	while (pEntryFound && !KeyEquals(pVoidKey, pEntryFound))
		{
		pPrevEntry = pEntryFound;
		pEntryFound = pEntryFound->pNext;
		}

	if (retiSlot)
		*retiSlot = iSlot;

	if (retpPrevEntry)
		*retpPrevEntry = pPrevEntry;

	return pEntryFound;
	}

CMapBase::EntryBase *CMapBase::GetNext (CMapIterator &Iterator)

//	GetNext
//
//	Return the next entry in the iterator

	{
	//	If we have pPos then we check if we've got a next entry

	if (Iterator.m_pPos)
		{
		EntryBase *pEntry = (EntryBase *)Iterator.m_pPos;
		if (pEntry->pNext)
			{
			Iterator.m_pPos = pEntry->pNext;
			return pEntry->pNext;
			}

		//	Start checking at the next slot

		Iterator.m_iTableEntry++;
		}
	else
		{
		//	We start at the beginning of the list

		Iterator.m_iTableEntry = 0;
		}

	//	We get here if

	while (Iterator.m_iTableEntry < m_iTableSize)
		{
		if (m_pTable[Iterator.m_iTableEntry])
			{
			Iterator.m_pPos = m_pTable[Iterator.m_iTableEntry];
			return m_pTable[Iterator.m_iTableEntry];
			}

		Iterator.m_iTableEntry++;
		}

	ASSERT(false);
	return NULL;
	}

int CMapBase::Hash (void *pKey, int iKeyLen) const

//	Hash
//
//	Hash the key

	{
	return (utlHashFunctionCase((BYTE *)pKey, iKeyLen) % m_iTableSize);
	}

bool CMapBase::HasMore (CMapIterator &Iterator)

//	HasMore
//
//	Returns TRUE if there are more entries in the iterator

	{
	int iSlot;

	//	If we have pPos then we check if we've got a next entry

	if (Iterator.m_pPos)
		{
		EntryBase *pEntry = (EntryBase *)Iterator.m_pPos;
		if (pEntry->pNext)
			return true;

		//	Start checking at the next slot

		iSlot = Iterator.m_iTableEntry + 1;
		}
	else
		{
		//	We start at the beginning of the list

		iSlot = 0;
		}

	//	We get here if

	while (iSlot < m_iTableSize)
		{
		if (m_pTable[iSlot])
			return true;
		iSlot++;
		}

	return false;
	}

void CMapBase::InsertEntry (void *pKey, int iKeyLen, EntryBase *pEntry)

//	InsertEntry
//
//	Inserts an entry into the hash table

	{
	int iSlot = Hash(pKey, iKeyLen);

	if (m_pTable[iSlot])
		pEntry->pNext = m_pTable[iSlot];

	m_pTable[iSlot] = pEntry;
	m_iCount++;
	}

void CMapBase::Reset (CMapIterator &Iterator)

//	Reset
//
//	Reset the iterator

	{
	Iterator.m_pPos = NULL;
	Iterator.m_iTableEntry = 0;
	}

bool MapKeyEquals (const CString &sKey1, const CString &sKey2)
	{
	return strEquals(sKey1, sKey2);
	}

void *MapKeyHashData (const CString &Key)
	{
	return Key.GetASCIIZ(); 
	}

int MapKeyHashDataSize (const CString &Key)
	{
	return Key.GetLength() + 1; 
	}
