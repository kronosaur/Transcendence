//	CAtomTable.cpp
//
//	CAtomTable class.
//	Copyright (c) 2001 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"

CAtomTable::CAtomTable (int iBaseAtom) : 
		m_iCount(0), 
		m_iBaseAtom(iBaseAtom)

//	CAtomTable constructor

	{
	for (int i = 0; i < ctHashTableSize; i++)
		m_HashTable[i] = -1;
	}

CAtomTable::~CAtomTable (void)

//	CAtomTable destructor

	{
	int i;

	for (i = 0; i < m_iCount; i++)
		{
		char *pEntry = (char *)GetEntry(i);
		delete [] pEntry;
		}

	for (i = 0; i < m_Backbone.GetCount(); i++)
		delete m_Backbone[i];
	}

int CAtomTable::AddAtomWithData (const CString &sString, void *pData, int iDataLen)

//	AddAtomWithData
//
//	Adds an atom. If the atom does not exist, adds the given data also

	{
	//	Hash the string

	int iHash = (utlHashFunctionCase((BYTE *)sString.GetASCIIZ(), sString.GetLength()) % ctHashTableSize);

	//	See if we've found it

	int iNext = m_HashTable[iHash];
	while (iNext != -1)
		{
		SEntry *pEntry = GetEntry(iNext);
		if (strEqualsCase(sString, pEntry->GetString()))
			return (m_iBaseAtom + iNext);

		iNext = pEntry->iNext;
		}

	//	We did not find the atom, so add it

	return (m_iBaseAtom + AllocAtom(sString, iHash, pData, iDataLen));
	}

int CAtomTable::AllocAtom (const CString &sString, int iHash, void *pExtraData, int iExtraDataLen)

//	AllocAtom
//
//	Allocate a new atom

	{
	int iAtom = m_iCount;

	//	Compute length of entry

	int iStringLen = sString.GetLength();
	int iStringPad = ((~((DWORD)(iStringLen + 1))) + 1) & 0x3;
	int iEntryLen = sizeof(SEntry)				//	Header
			+ iStringLen						//	String
			+ 1									//	'/0' terminator
			+ iStringPad						//	Pad to 32-bits
			+ iExtraDataLen;					//	Extra data

	//	Allocate a new entry

	SEntry *pEntry = (SEntry *)(new char[iEntryLen]);
	pEntry->iNext = m_HashTable[iHash];
	pEntry->iLen = -iStringLen;

	*((DWORD *)&pEntry->sStr) = 0;
	pEntry->sStr = CString(((char *)pEntry) + sizeof(SEntry), true);
	
	char *pSource = sString.GetASCIIZ();
	char *pDest = (char *)(&pEntry[1]);

	while (*pSource)
		*pDest++ = *pSource++;

	*pDest = '\0';

	//	Copy any extra data

	if (pExtraData)
		{
		pDest += iStringPad;

		pSource = (char *)pExtraData;
		char *pSourceEnd = pSource + iExtraDataLen;
		while (pSource < pSourceEnd)
			*pDest++ = *pSource++;
		}

	//	Figure out which segment to place the entry in

	int iSegment = (iAtom / ctSegmentSize);

	//	Make sure this segment is allocated

	if (iSegment == m_Backbone.GetCount())
		{
		SEntry **pSegment = new SEntry *[ctSegmentSize];
		m_Backbone.Insert(pSegment);
		}

	ASSERT(iSegment < m_Backbone.GetCount());

	//	Place the entry in the segment

	int iSlot = (iAtom % ctSegmentSize);
	m_Backbone[iSegment][iSlot] = pEntry;

	//	Update the hash table

	m_HashTable[iHash] = iAtom;

	//	Update the count

	m_iCount++;

	//	Done

	return iAtom;
	}

bool CAtomTable::FindAtom (const CString &sString, int *retiAtom)

//	FindAtom
//
//	Returns TRUE if the atom exists, FALSE otherwise

	{
	//	Hash the string

	int iHash = (utlHashFunctionCase((BYTE *)sString.GetASCIIZ(), sString.GetLength()) % ctHashTableSize);

	//	See if we've found it

	int iNext = m_HashTable[iHash];
	while (iNext != -1)
		{
		SEntry *pEntry = GetEntry(iNext);
		if (strEqualsCase(sString, pEntry->GetString()))
			{
			if (retiAtom)
				*retiAtom = (m_iBaseAtom + iNext);
			return true;
			}

		iNext = pEntry->iNext;
		}

	//	We did not find the atom

	return false;
	}

const CString &CAtomTable::GetAtom (int iAtom)

//	GetAtom
//
//	Returns the string for this atom

	{
	SEntry *pEntry = GetEntry(iAtom - m_iBaseAtom);
	if (pEntry == NULL)
		{
		ASSERT(false);
		return NULL_STRING;
		}
	else
		return pEntry->GetString();
	}

void *CAtomTable::GetAtomData (int iAtom)

//	GetAtomData
//
//	Returns atom data

	{
	SEntry *pEntry = GetEntry(iAtom - m_iBaseAtom);
	if (pEntry == NULL)
		return NULL;
	else
		return pEntry->GetData();
	}

CAtomTable::SEntry *CAtomTable::GetEntry (int iAtom)

//	GetEntry
//
//	Returns the entry for the atom

	{
	ASSERT(iAtom >= 0 && iAtom < m_iCount);

	int iSegment = (iAtom / ctSegmentSize);
	int iSlot = (iAtom % ctSegmentSize);

	return m_Backbone[iSegment][iSlot];
	}
