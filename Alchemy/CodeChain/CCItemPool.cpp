//	CCItemPool.cpp
//
//	Implements CCItemPool template class

#include <windows.h>
#include "Kernel.h"
#include "KernelObjID.h"
#include "CodeChain.h"

#define SEGMENT_SIZE					4096	//	Size of each backbone segment
#define BACKBONE_SIZE					1024	//	Size of the backbone

#pragma warning (disable : 4660)

template class CCItemPool<CCInteger>;
template class CCItemPool<CCString>;
template class CCItemPool<CCLinkedList>;
template class CCItemPool<CCPrimitive>;
template class CCItemPool<CCNil>;
template class CCItemPool<CCTrue>;
template class CCItemPool<CCSymbolTable>;
template class CCItemPool<CCLambda>;
template class CCItemPool<CCAtomTable>;

template <class ItemClass>CCItemPool<ItemClass>::CCItemPool (void) :
		m_pFreeList(NULL),
		m_pBackbone(NULL),
		m_iBackboneSize(0),
		m_iCount(0)

//	CCItemPool constructor

	{
	}

template <class ItemClass>CCItemPool<ItemClass>::~CCItemPool (void)

//	CCItemPool destructor

	{
	}

template <class ItemClass> ICCItem *CCItemPool<ItemClass>::CreateItem (CCodeChain *pCC)

//	CreateItem
//
//	Creates an item in the pool

	{
	int i;
	ICCItem *pItem;

	//	If we've got no backbone, allocate it now

	if (m_pBackbone == NULL)
		{
		m_pBackbone = new ItemClass *[BACKBONE_SIZE];
		if (m_pBackbone == NULL)
			return pCC->CreateMemoryError();

		m_iBackboneSize = 0;
		}

	//	If we've got no more free items, allocate another segment
	//	of backbone

	if (m_pFreeList == NULL)
		{
		ItemClass *pSegment;

		if (m_iBackboneSize == BACKBONE_SIZE)
			return pCC->CreateMemoryError();

		pSegment = new ItemClass[SEGMENT_SIZE];
		if (pSegment == NULL)
			return pCC->CreateMemoryError();

		m_pBackbone[m_iBackboneSize] = pSegment;
		m_iBackboneSize++;

		//	Add all the entries to the free list

		for (i = 0; i < SEGMENT_SIZE; i++)
			{
			ICCItem *pItem = dynamic_cast<ICCItem *>(&pSegment[i]);

			pItem->SetNextFree(m_pFreeList);
			m_pFreeList = pItem;
			}
		}

	//	Return the next free item

	pItem = m_pFreeList;
	m_pFreeList = pItem->GetNextFree();

	pItem->ResetItem();
	m_iCount++;
	return pItem;
	}

template <class ItemClass> void CCItemPool<ItemClass>::DestroyItem (CCodeChain *pCC, ICCItem *pItem)

//	DestroyItem
//
//	Destroys an item in the pool

	{
#ifdef DEBUG
	ItemClass *pClass = dynamic_cast<ItemClass *>(pItem);
	ASSERT(pClass);
#endif

	//	Add the item back to the free list

	pItem->SetNextFree(m_pFreeList);
	m_pFreeList = pItem;
	m_iCount--;
	}
