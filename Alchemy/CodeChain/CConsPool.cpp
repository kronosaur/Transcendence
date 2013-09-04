//	CConsPool.cpp
//
//	Implements CConsPool class

#include <windows.h>
#include "Kernel.h"
#include "KernelObjID.h"
#include "CodeChain.h"

#define SEGMENT_SIZE					4096	//	Size of each backbone segment
#define BACKBONE_SIZE					1024	//	Size of the backbone

CConsPool::CConsPool (void) :
		m_pFreeList(NULL),
		m_pBackbone(NULL),
		m_iBackboneSize(0),
		m_iCount(0)

//	CConsPool constructor

	{
	}

CConsPool::~CConsPool (void)

//	CConsPool destructor

	{
	}

CCons *CConsPool::CreateCons (void)

//	CreateCons
//
//	Creates an item in the pool

	{
	int i;
	CCons *pCons;

	//	If we've got no backbone, allocate it now

	if (m_pBackbone == NULL)
		{
		m_pBackbone = new CCons *[BACKBONE_SIZE];
		if (m_pBackbone == NULL)
			return NULL;

		m_iBackboneSize = 0;
		}

	//	If we've got no more free items, allocate another segment
	//	of backbone

	if (m_pFreeList == NULL)
		{
		CCons *pSegment;

		if (m_iBackboneSize == BACKBONE_SIZE)
			return NULL;

		pSegment = new CCons[SEGMENT_SIZE];
		if (pSegment == NULL)
			return NULL;

		m_pBackbone[m_iBackboneSize] = pSegment;
		m_iBackboneSize++;

		//	Add all the entries to the free list

		for (i = 0; i < SEGMENT_SIZE; i++)
			{
			CCons *pCons = &pSegment[i];

			pCons->m_pNext = m_pFreeList;
			m_pFreeList = pCons;
			}
		}

	//	Return the next free item

	pCons = m_pFreeList;
	m_pFreeList = pCons->m_pNext;

	m_iCount++;
	return pCons;
	}

void CConsPool::DestroyCons (CCons *pCons)

//	DestroyItem
//
//	Destroys an item in the pool

	{
	//	Add the item back to the free list

	pCons->m_pNext = m_pFreeList;
	m_pFreeList = pCons;
	m_iCount--;
	}
