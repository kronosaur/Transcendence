//	CSpaceObjectList.cpp
//
//	CSpaceObjectList class

#include "PreComp.h"

#define ALLOC_GRANULARITY				64

CSpaceObjectList::CSpaceObjectList (void)

//	CSpaceObjectList constructor

	{
	}

CSpaceObjectList::~CSpaceObjectList (void)

//	CSpaceObjectList destructor

	{
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

void CSpaceObjectList::ReadFromStream (SLoadCtx &Ctx, bool bIgnoreMissing)

//	ReadFromStream
//
//	Read the list from a stream

	{
	int i;
	ASSERT(m_List.GetCount() == 0);

	DWORD dwCount;
	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	if (dwCount == 0)
		return;

	//	If we expect all objects to be resolved, then we create the array and
	//	wait for them to be loaded.

	if (!bIgnoreMissing)
		{
		m_List.InsertEmpty(dwCount);

		for (i = 0; i < (int)dwCount; i++)
			CSystem::ReadObjRefFromStream(Ctx, &m_List[i]);
		}

	//	Otherwise, we use the callback method, which allows us to ignore any
	//	unresolved entries.

	else
		{
		for (i = 0; i < (int)dwCount; i++)
			CSystem::ReadObjRefFromStream(Ctx, this, &ResolveObjProc);
		}
	}

bool CSpaceObjectList::Remove (CSpaceObject *pObj)

//	Remove
//
//	Remove the object. Returns TRUE if we removed the object.

	{
	int iIndex;
	if (FindObj(pObj, &iIndex))
		{
		Remove(iIndex);
		return true;
		}

	return false;
	}

void CSpaceObjectList::ResolveObjProc (void *pCtx, DWORD dwObjID, CSpaceObject *pObj)

//	ResolveObjProc
//
//	Resolve an object reference

	{
	CSpaceObjectList *pList = (CSpaceObjectList *)pCtx;
	if (pObj && dwObjID)
		pList->m_List.Insert(pObj);
	}

void CSpaceObjectList::SetAllocSize (int iNewCount)

//	SetAllocSize
//
//	Sets the size of the buffer in preparation for adding objects.
//	NOTE that this also empties the list.

	{
	int iNeeded = (iNewCount - m_List.GetCount());
	if (iNeeded > 0)
		m_List.InsertEmpty(iNeeded);

	m_List.DeleteAll();
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

	TArray<CSpaceObject *> NewList;
	for (i = 0; i < iCount; i++)
		if (GetObj(i)->IsMarked())
			NewList.Insert(GetObj(i));

	m_List.TakeHandoff(NewList);
	}

void CSpaceObjectList::WriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	WriteToStream
//
//	Write list to stream

	{
	int iCount = GetCount();
	pStream->Write((char *)&iCount, sizeof(int));

	for (int i = 0; i < iCount; i++)
		pSystem->WriteObjRefToStream(m_List[i], pStream);
	}
