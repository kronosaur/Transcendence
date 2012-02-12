//	CWORM.cpp
//
//	CWORM class. Write-Once Read-Many memory block
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"

const int g_iPageSize = 4096;

CWORM::CWORM (int iMaxSize) : m_iMaxSize(iMaxSize),
		m_iCommittedSize(0),
		m_iCurrentSize(0)

//	CWORM constructor

	{
	//	Reserve a block of memory equal to the maximum size requested

	m_pBlock = (char *)::VirtualAlloc(NULL, m_iMaxSize, MEM_RESERVE, PAGE_NOACCESS);
	if (m_pBlock == NULL)
		throw CException(errOutOfMemory);

	m_pPointer = m_pBlock;
	}

CWORM::~CWORM (void)

//	CWORM destructor

	{
	if (m_pBlock)
		{
        ::VirtualFree(m_pBlock, m_iCommittedSize, MEM_DECOMMIT);
        ::VirtualFree(m_pBlock, 0, MEM_RELEASE);
		}
	}

void CWORM::Commit (int iSize)

//	Commit
//
//	Commits the memory block so that it is at least iSize

	{
	if (iSize > m_iCommittedSize)
		{
		int iAdditionalSize = AlignUp(iSize - m_iCommittedSize, g_iPageSize);

		//	Figure out if we're over the limit. We cannot rely on VirtualAlloc
		//	to keep track of our maximum reservation

		if (m_iCommittedSize + iAdditionalSize > m_iMaxSize)
			throw CException(errOutOfMemory);

		//	Commit

		if (::VirtualAlloc(m_pBlock + m_iCommittedSize,
				iAdditionalSize,
				MEM_COMMIT,
				PAGE_READWRITE) == NULL)
			throw CException(errOutOfMemory);

		m_iCommittedSize += iAdditionalSize;
		}
	}

int CWORM::Read (void *pData, int iLength)

//	Read
//
//	Read from the WORM, returns bytes read

	{
	int iLeft = m_iCurrentSize - (m_pPointer - m_pBlock);
	int iRead = min(iLeft, iLength);

	utlMemCopy(m_pPointer, pData, iRead);
	m_pPointer += iRead;
	return iRead;
	}

void CWORM::Seek (int iPos, bool bFromEnd)

//	ReadSeek
//
//	Move the read position

	{
	if (bFromEnd)
		m_pPointer = (m_pBlock + m_iCurrentSize) + max(iPos, -m_iCurrentSize);
	else
		{
		Commit(iPos);
		m_pPointer = m_pBlock + iPos;
		m_iCurrentSize = max(m_iCurrentSize, iPos);
		}
	}

int CWORM::Write (void *pData, int iLength)

//	Write
//
//	Write to the WORM, returns bytes written

	{
	Commit((m_pPointer - m_pBlock) + iLength);

	utlMemCopy(pData, m_pPointer, iLength);
	m_pPointer += iLength;
	m_iCurrentSize = max(m_iCurrentSize, m_pPointer - m_pBlock);
	return iLength;
	}
