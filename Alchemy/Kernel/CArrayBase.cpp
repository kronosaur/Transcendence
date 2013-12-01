//	CArrayBase.cpp
//
//	Implements CArrayBase object

#include "Kernel.h"
#include "KernelObjID.h"

CArrayBase::CArrayBase (HANDLE hHeap, int iGranularity) : m_pBlock(NULL)

//	CArrayBase constructor

	{
	if (hHeap == NULL)
		hHeap = ::GetProcessHeap();

	//	If we have anything except the default options then we need
	//	to allocate the block

	if (hHeap != ::GetProcessHeap()	|| (iGranularity != DEFAULT_ARRAY_GRANULARITY))
		{
		m_pBlock = (SHeader *)::HeapAlloc(hHeap, 0, sizeof(SHeader));
		m_pBlock->m_hHeap = hHeap;
		m_pBlock->m_iAllocSize = sizeof(SHeader);
		m_pBlock->m_iGranularity = iGranularity;
		m_pBlock->m_iSize = 0;
		}
	}

CArrayBase::~CArrayBase (void)

//	CArrayBase destructor

	{
	if (m_pBlock)
		::HeapFree(m_pBlock->m_hHeap, 0, m_pBlock);
	}

void CArrayBase::CopyOptions (const CArrayBase &Src)

//	CopyOptions
//
//	Copies heap an granularity information from source

	{
	//	If we're changing heaps then we need to reallocate

	if (GetHeap() != Src.GetHeap() 
			|| (m_pBlock == NULL && GetGranularity() != Src.GetGranularity()))
		{
		ASSERT(GetSize() == 0);

		if (m_pBlock)
			::HeapFree(m_pBlock->m_hHeap, 0, m_pBlock);

		m_pBlock = (SHeader *)::HeapAlloc(Src.GetHeap(), 0, sizeof(SHeader));
		m_pBlock->m_hHeap = Src.GetHeap();
		m_pBlock->m_iAllocSize = sizeof(SHeader);
		m_pBlock->m_iGranularity = Src.GetGranularity();
		m_pBlock->m_iSize = 0;
		}

	//	Otherwise we just change the granularity

	else if (GetGranularity() != Src.GetGranularity())
		{
		ASSERT(m_pBlock);
		m_pBlock->m_iGranularity = Src.GetGranularity();
		}
	}

void CArrayBase::DeleteBytes (int iOffset, int iLength)

//	Delete
//
//	Delete iLength bytes in the array at the given offset

	{
	int i;

	if (iLength <= 0)
		return;

	ASSERT(m_pBlock);

	//	Move stuff down

	char *pSource = GetBytes() + iOffset + iLength;
	char *pDest = GetBytes() + iOffset;
	for (i = 0; i < GetSize() - (iOffset + iLength); i++)
		*pDest++ = *pSource++;

	//	Done

	m_pBlock->m_iSize -= iLength;
	}

void CArrayBase::InsertBytes (int iOffset, void *pData, int iLength, int iAllocQuantum)

//	Insert
//
//	Insert the given data at the offset

	{
	int i;

	if (iLength <= 0)
		return;

	if (iOffset == -1)
		iOffset = GetSize();

	ASSERT(iOffset >= 0 && iOffset <= GetSize());

    //	Reallocate if necessary

	Resize(GetSize() + iLength, true, iAllocQuantum);
    
	//	Move the array up
    
	char *pSource = GetBytes() + GetSize()-1;
	char *pDest = pSource + iLength;
    for (i = GetSize()-1; i >= iOffset; i--)
		*pDest-- = *pSource--;

	//	Copy the new values

	if (pData)
		{
		pSource = (char *)pData;
		pDest = GetBytes() + iOffset;
		for (i = 0; i < iLength; i++)
			*pDest++ = *pSource++;
		}

	//	Done
    
	m_pBlock->m_iSize += iLength;
	}

ALERROR CArrayBase::Resize (int iNewSize, bool bPreserve, int iAllocQuantum)

//	Resize
//
//	Resize the array so that it is at least the given new size

	{
	ASSERT(iAllocQuantum > 0);

	//	See if we need to reallocate the block

	if (m_pBlock == NULL || (m_pBlock->m_iAllocSize - (int)sizeof(SHeader) < iNewSize))
		{
		//	Allocate a new block

		int iNewAllocSize = sizeof(SHeader) + AlignUp(iNewSize, iAllocQuantum);
		SHeader *pNewBlock = (SHeader *)::HeapAlloc(GetHeap(), 0, iNewAllocSize);
		if (pNewBlock == NULL)
			{
			::kernelDebugLogMessage("Out of memory allocating array of %d bytes.", iNewAllocSize);
			throw CException(ERR_MEMORY);
			}

		pNewBlock->m_hHeap = GetHeap();
		pNewBlock->m_iAllocSize = iNewAllocSize;
		pNewBlock->m_iGranularity = GetGranularity();
		pNewBlock->m_iSize = GetSize();

		//	Transfer the contents, if necessary

		if (m_pBlock && bPreserve)
			{
			int i;
			char *pSource = GetBytes();
			char *pDest = (char *)(&pNewBlock[1]);

			for (i = 0; i < GetSize(); i++)
				*pDest++ = *pSource++;
			}

		//	Swap blocks

		if (m_pBlock)
			::HeapFree(m_pBlock->m_hHeap, 0, m_pBlock);

		m_pBlock = pNewBlock;
		}

	return NOERROR;
	}

void CArrayBase::TakeHandoffBase (CArrayBase &Src)

//	TakeHandoffBase
//
//	Takes the allocated array from Src

	{
	if (m_pBlock)
		::HeapFree(m_pBlock->m_hHeap, 0, m_pBlock);

	m_pBlock = Src.m_pBlock;
	Src.m_pBlock = NULL;
	}
