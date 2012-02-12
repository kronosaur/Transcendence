//	CStackBase.cpp
//
//	Implements CStackBase object
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "Kernel.h"
#include "KernelObjID.h"

const int PAGE_SIZE = 4096;

CStackBase::CStackBase (int iMaxSize) : m_iMaxSize(iMaxSize),
		m_iCommittedSize(0),
		m_iSP(0)

//	CStackBase constructor

	{
	//	Reserve a block of memory equal to the maximum size requested

	m_pBlock = (char *)::VirtualAlloc(NULL, m_iMaxSize, MEM_RESERVE, PAGE_NOACCESS);
	}

CStackBase::~CStackBase (void)

//	CStackBase destructor

	{
	if (m_pBlock)
		{
        ::VirtualFree(m_pBlock, m_iCommittedSize, MEM_DECOMMIT);
        ::VirtualFree(m_pBlock, 0, MEM_RELEASE);
		}
	}

ALERROR CStackBase::Commit (int iSize)

//	Commit
//
//	Commits the memory block so that it is at least iSize

	{
	if (iSize > m_iCommittedSize)
		{
		int iAdditionalSize = AlignUp(iSize - m_iCommittedSize, PAGE_SIZE);

		//	Figure out if we're over the limit. We cannot rely on VirtualAlloc
		//	to keep track of our maximum reservation

		if (m_iCommittedSize + iAdditionalSize > m_iMaxSize)
			return ERR_MEMORY;

		//	Commit

		if (::VirtualAlloc(m_pBlock + m_iCommittedSize,
				iAdditionalSize,
				MEM_COMMIT,
				PAGE_READWRITE) == NULL)
			return ERR_MEMORY;

		m_iCommittedSize += iAdditionalSize;
		}

	return NOERROR;
	}

void *CStackBase::GetDataAt (int iSP)

//	GetDataAt
//
//	Return a pointer to the element at the given stack pointer

	{
	return m_pBlock + iSP;
	}

void *CStackBase::PushData (int iDataSize)

//	PushData
//
//	Pushes data on the stack
	
	{
	if (m_iSP + iDataSize > m_iCommittedSize)
		Commit(m_iSP + iDataSize);

	char *pData = m_pBlock + m_iSP;
	m_iSP += iDataSize;
	return pData;
	}

void *CStackBase::PopData (int iDataSize)

//	PopData
//
//	Pops data on the stack

	{
	ASSERT(m_iSP >= iDataSize);

	m_iSP -= iDataSize;
	return m_pBlock + m_iSP;
	}

