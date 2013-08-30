//	CSpaceObjectAddressResolver.cpp
//
//	CSpaceObjectAddressResolver class

#include "PreComp.h"

bool CSpaceObjectAddressResolver::HasUnresolved (void)

//	HasUnresolved
//
//	For all callback reference, we call them with NULL. If there are any address
//	references left, then we return TRUE.

	{
	int i, j;

	bool bUnresolved = false;

	for (i = 0; i < m_List.GetCount(); i++)
		{
		TArray<SEntry> *pList = &m_List[i];

		for (j = 0; j < pList->GetCount(); j++)
			{
			SEntry *pEntry = &pList->GetAt(j);

			//	If we have a callback function, invoke it now.
			//	The function may throw if it does not want to leave an object
			//	unresolved.

			if (pEntry->pfnResolveProc)
				(pEntry->pfnResolveProc)(pEntry->pCtx, m_List.GetKey(i), NULL);

			//	Otherwise this is unresolved

			else
				{
				kernelDebugLogMessage("Unresolved object: %x", m_List.GetKey(i));
				bUnresolved = true;
				}
			}
		}

	return bUnresolved;
	}

void CSpaceObjectAddressResolver::InsertRef (DWORD dwObjID, void *pCtx, PRESOLVEOBJIDPROC pfnResolveProc)

//	InsertRef
//
//	Insert a reference

	{
	TArray<SEntry> *pList = m_List.SetAt(dwObjID);
	SEntry *pEntry = pList->Insert();
	pEntry->pCtx = pCtx;
	pEntry->pfnResolveProc = pfnResolveProc;
	}

void CSpaceObjectAddressResolver::InsertRef (DWORD dwObjID, CSpaceObject **ppAddr)

//	InsertRef
//
//	Insert a reference

	{
	TArray<SEntry> *pList = m_List.SetAt(dwObjID);
	SEntry *pEntry = pList->Insert();
	pEntry->pCtx = ppAddr;
	pEntry->pfnResolveProc = NULL;
	}

void CSpaceObjectAddressResolver::ResolveRefs (DWORD dwObjID, CSpaceObject *pObj)

//	ResolveRefs
//
//	Resolve a reference

	{
	int i;

	int iPos;
	if (!m_List.FindPos(dwObjID, &iPos))
		return;

	TArray<SEntry> *pList = &m_List[iPos];

	for (i = 0; i < pList->GetCount(); i++)
		{
		SEntry *pEntry = &pList->GetAt(i);

		//	If we have a callback function, invoke it now.

		if (pEntry->pfnResolveProc)
			(pEntry->pfnResolveProc)(pEntry->pCtx, dwObjID, pObj);

		//	Otherwise we fix up the address

		else
			(*(CSpaceObject **)pEntry->pCtx) = pObj;
		}

	//	Remove the entry (since we've resolved all entries)

	m_List.Delete(iPos);
	}
