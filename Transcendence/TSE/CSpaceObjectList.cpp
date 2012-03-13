//	CSpaceObjectList.cpp
//
//	CSpaceObjectList class

#include "PreComp.h"

#define ALLOC_GRANULARITY				64

CSpaceObjectList::CSpaceObjectList (void) : m_pList(NULL)

//	CSpaceObjectList constructor

	{
	}

CSpaceObjectList::~CSpaceObjectList (void)

//	CSpaceObjectList destructor

	{
	CleanUp();
	}

void CSpaceObjectList::Add (CSpaceObject *pObj, int *retiIndex)

//	Add
//
//	Add object to the list. If the object is already in the list, we
//	don't add it.

	{
	if (FindObj(pObj))
		return;

	FastAdd(pObj, retiIndex);
	}

void CSpaceObjectList::FastAdd (CSpaceObject *pObj, int *retiIndex)

//	FastAdd
//
//	Adds the object without checking to see if it is already
//	in the list.

	{
	int iAlloc = GetAllocation();
	int iCount = GetCount();

	if (iAlloc <= iCount)
		{
		iAlloc += ALLOC_GRANULARITY;
		CSpaceObject **pNewList = new CSpaceObject * [iAlloc + 1];

		if (m_pList)
			{
			for (int i = 1; i <= iCount; i++)
				pNewList[i] = m_pList[i];

			delete [] m_pList;
			}

		m_pList = pNewList;
		}

	if (retiIndex)
		*retiIndex = iCount;

	m_pList[++iCount] = pObj;
	SetCountAndAllocation(iCount, iAlloc);
	}

bool CSpaceObjectList::FindObj (CSpaceObject *pObj, int *retiIndex) const

//	FindObj
//
//	Find the object in the list

	{
	int iCount = GetCount();

	for (int i = 0; i < iCount; i++)
		if (m_pList[i+1] == pObj)
			{
			if (retiIndex)
				*retiIndex = i;

			return true;
			}

	return false;
	}

int CSpaceObjectList::GetAllocation (void) const

//	GetAllocation
//
//	Get allocation size (in elements)

	{
	if (m_pList)
		return (int)HIWORD(((DWORD)m_pList[0]));
	else
		return 0;
	}

int CSpaceObjectList::GetCount (void) const

//	GetCount
//
//	Returns the number of objects in the list

	{
	if (m_pList)
		return (int)LOWORD(((DWORD)m_pList[0]));
	else
		return 0;
	}

void CSpaceObjectList::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read the list from a stream

	{
	ASSERT(m_pList == NULL);

	DWORD dwCount;
	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	if (dwCount)
		{
		int iAlloc = AlignUp(dwCount, ALLOC_GRANULARITY);
		m_pList = new CSpaceObject * [iAlloc + 1];

		for (int i = 0; i < (int)dwCount; i++)
			Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pList[i + 1]);

		SetCountAndAllocation(dwCount, iAlloc);
		}
	}

void CSpaceObjectList::Remove (int iIndex)

//	Remove
//
//	Remove the object

	{
	int iCount = GetCount();
	ASSERT(iCount > 0);

	for (int i = iIndex + 1; i < iCount; i++)
		m_pList[i] = m_pList[i+1];

	iCount--;
	SetCountAndAllocation(iCount, GetAllocation());
	}

bool CSpaceObjectList::Remove (CSpaceObject *pObj)

//	Remove
//
//	Remove the object

	{
	int iCount = GetCount();

	for (int i = 0; i < iCount; i++)
		if (m_pList[i+1] == pObj)
			{
			Remove(i);
			return true;
			}

	return false;
	}

void CSpaceObjectList::RemoveAll (void)

//	RemoveAll
//
//	Remove all objects

	{
	if (m_pList)
		SetCountAndAllocation(0, GetAllocation());
	}

void CSpaceObjectList::SetAllocSize (int iNewCount)

//	SetAllocSize
//
//	Sets the allocation size to hold at least the given number of objects

	{
	int iAlloc = GetAllocation();
	int iCount = GetCount();

	if (iAlloc < iNewCount)
		{
		iAlloc = AlignUp(iNewCount, ALLOC_GRANULARITY);
		CSpaceObject **pNewList = new CSpaceObject * [iAlloc + 1];

		if (m_pList)
			{
			for (int i = 1; i <= iCount; i++)
				pNewList[i] = m_pList[i];

			delete [] m_pList;
			}

		m_pList = pNewList;

		SetCountAndAllocation(iCount, iAlloc);
		}
	}

void CSpaceObjectList::SetCountAndAllocation (int iCount, int iAllocation)

//	SetCountAndAllocation
//
//	Set the count and allocation (which we store in the first element)

	{
	ASSERT(m_pList);
	m_pList[0] = (CSpaceObject *)MAKELONG((WORD)(DWORD)iCount, (WORD)(DWORD)iAllocation);
	}

void CSpaceObjectList::Subtract (const CSpaceObjectList &List)

//	Subtract
//
//	Removes all objects in List from the current list

	{
	int i;

	//	Mark all current objects

	int iCount = GetCount();
	for (i = 0; i < iCount; i++)
		GetObj(i)->SetMarked(true);

	//	Clear marks on all objects to remove

	for (i = 0; i < List.GetCount(); i++)
		List.GetObj(i)->SetMarked(false);

	//	Create a new list with the remaining objects

	CSpaceObject **pOldList = m_pList;
	m_pList = NULL;

	for (i = 0; i < iCount; i++)
		if (pOldList[i + 1]->IsMarked())
			FastAdd(pOldList[i + 1]);

	if (pOldList)
		delete [] pOldList;
	}

void CSpaceObjectList::WriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	WriteToStream
//
//	Write list to stream

	{
	int iCount = GetCount();
	pStream->Write((char *)&iCount, sizeof(int));

	for (int i = 0; i < iCount; i++)
		pSystem->WriteObjRefToStream(m_pList[i+1], pStream);
	}
