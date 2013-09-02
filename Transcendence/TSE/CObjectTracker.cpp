//	CObjectTracker.cpp
//
//	CObjectTracker class

#include "PreComp.h"

CObjectTracker::~CObjectTracker (void)

//	CObjectTracker destructor

	{
	DeleteAll();
	}

bool CObjectTracker::AccumulateEntries (TArray<SObjList *> &Table, const CDesignTypeCriteria &Criteria, TArray<SObjEntry> *retResult)

//	AccumulateEntries
//
//	Looks for objects that match the criteria and adds them to the result list.

	{
	int i;
	int j;

	for (i = 0; i < Table.GetCount(); i++)
		{
		SObjList *pList = Table[i];

		//	If we don't match, then continue

		if (!pList->pType->MatchesCriteria(Criteria))
			continue;

		//	If all we care about is whether we have any entries, then we're done.

		if (retResult == NULL)
			return true;

		//	Otherwise, add all objects to the results

		for (j = 0; j < pList->ObjectIDs.GetCount(); j++)
			{
			SObjEntry *pEntry = retResult->Insert();
			pEntry->pNode = pList->pNode;
			pEntry->pType = pList->pType;
			pEntry->dwObjID = pList->ObjectIDs[j];

			SObjName *pName = pList->ObjectNames.GetAt(pEntry->dwObjID);
			if (pName)
				{
				pEntry->sName = pName->sName;
				pEntry->dwNameFlags = pName->dwNameFlags;
				}
			else
				pEntry->sName = pList->pType->GetTypeName(&pEntry->dwNameFlags);
			}
		}

	//	Done

	return (retResult && retResult->GetCount() > 0);
	}

void CObjectTracker::Delete (CSpaceObject *pObj)

//	Delete
//
//	Delete an object

	{
	int i;

	SObjList *pList = GetList(pObj);
	if (pList == NULL)
		return;

	DWORD dwID = pObj->GetID();
	for (i = 0; i < pList->ObjectIDs.GetCount(); i++)
		if (pList->ObjectIDs[i] == dwID)
			{
			pList->ObjectIDs.Delete(i);
			break;
			}
	}

void CObjectTracker::DeleteAll (void)

//	DeleteAll
//
//	Delete all entries

	{
	int i;

	for (i = 0; i < m_AllLists.GetCount(); i++)
		delete m_AllLists[i];

	m_AllLists.DeleteAll();
	m_ByNode.DeleteAll();
	}

bool CObjectTracker::Find (const CString &sNodeID, const CDesignTypeCriteria &Criteria, TArray<SObjEntry> *retResult)

//	Find
//
//	Find a set of objects matching the given criteria.

	{
	if (retResult)
		retResult->DeleteAll();

	//	If no node ID, then we look through all nodes

	if (sNodeID.IsBlank())
		return AccumulateEntries(m_AllLists, Criteria, retResult);

	//	Otherwise, check the specific node

	else
		{
		//	Look for the node ID.

		TArray<SObjList *> *pTable = m_ByNode.GetAt(sNodeID);
		if (pTable == NULL)
			return false;

		//	Accumulate entries for this table

		return AccumulateEntries(*pTable, Criteria, retResult);
		}
	}

CObjectTracker::SObjList *CObjectTracker::GetList (CSpaceObject *pObj)

//	GetList
//
//	Returns the list that corresponds to the given object's UNID and NodeID.

	{
	CSystem *pSystem = pObj->GetSystem();
	if (pSystem == NULL)
		return NULL;

	CTopologyNode *pNode = pSystem->GetTopology();
	if (pNode == NULL)
		return NULL;

	CDesignType *pType = pObj->GetType();
	if (pType == NULL)
		return NULL;

	//	Done

	return GetList(pNode, pType);
	}

CObjectTracker::SObjList *CObjectTracker::GetList (CTopologyNode *pNode, CDesignType *pType)

//	GetList
//
//	Returns the list that corresponds to the given object's UNID and NodeID.

	{
	int i;

	//	Look in the index of nodes

	TArray<SObjList *> *pTable = m_ByNode.SetAt(pNode->GetID());

	//	Look for the UNID. If found, return it.

	for (i = 0; i < pTable->GetCount(); i++)
		{
		SObjList *pList = pTable->GetAt(i);
		if (pList->pType == pType)
			return pList;
		}

	//	Otherwise, we need to add it

	SObjList *pNewList = new SObjList;
	pNewList->pNode = pNode;
	pNewList->pType = pType;

	//	Add to the flat list

	m_AllLists.Insert(pNewList);

	//	Add to the index

	pTable->Insert(pNewList);

	//	Done

	return pNewList;
	}

void CObjectTracker::Insert (CSpaceObject *pObj)

//	Insert
//
//	Insert a new object.
//
//	NOTE: We rely on our caller to NOT insert the same object twice.

	{
	CDesignType *pType = pObj->GetType();
	if (pType == NULL)
		return;

	SObjList *pList = GetList(pObj);
	if (pList == NULL)
		return;

	pList->ObjectIDs.Insert(pObj->GetID());

	//	If the name of this object does not match the type, then we store it.

	DWORD dwNameFlags;
	CString sName = pObj->GetName(&dwNameFlags);
	if (!strEquals(sName, pType->GetTypeName()))
		{
		SObjName *pName = pList->ObjectNames.Insert(pObj->GetID());
		pName->sName = sName;
		pName->dwNameFlags = dwNameFlags;
		}
	}

void CObjectTracker::ReadFromStream (SUniverseLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads all objects
//
//	DWORD			No. of lists
//
//	For each list:
//	CString			NodeID
//	DWORD			UNID
//	DWORD			No. of objects
//
//	For each object
//	DWORD			ObjID
//
//	DWORD			No. of names
//
//	For each name
//	DWORD			ObjID
//	DWORD			Name Flags
//	CString			Name

	{
	int i;
	int j;
	DWORD dwLoad;

	DeleteAll();

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	int iListCount = (int)dwLoad;

	for (i = 0; i < iListCount; i++)
		{
		CString sNodeID;

		sNodeID.ReadFromStream(Ctx.pStream);
		CTopologyNode *pNode = g_pUniverse->FindTopologyNode(sNodeID);

		DWORD dwUNID;
		Ctx.pStream->Read((char *)&dwUNID, sizeof(DWORD));
		CDesignType *pType = g_pUniverse->FindDesignType(dwUNID);

		int iObjCount;
		Ctx.pStream->Read((char *)&iObjCount, sizeof(DWORD));

		//	Get the appropriate table

		SObjList *pList = (pNode && pType ? GetList(pNode, pType) : NULL);

		//	Read all the objects

		for (j = 0; j < iObjCount; j++)
			{
			Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
			if (pList)
				pList->ObjectIDs.Insert(dwLoad);
			}

		//	Read names

		if (Ctx.dwVersion >= 21)
			{
			Ctx.pStream->Read((char *)&iObjCount, sizeof(DWORD));

			for (j = 0; j < iObjCount; j++)
				{
				DWORD dwObjID;
				Ctx.pStream->Read((char *)&dwObjID, sizeof(DWORD));

				if (pList)
					{
					SObjName *pName = pList->ObjectNames.Insert(dwObjID);
			
					Ctx.pStream->Read((char *)&pName->dwNameFlags, sizeof(DWORD));
					pName->sName.ReadFromStream(Ctx.pStream);
					}
				else
					{
					CString sDummy;
					Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
					sDummy.ReadFromStream(Ctx.pStream);
					}
				}
			}
		}
	}

void CObjectTracker::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write out all objects
//
//	DWORD			No. of lists
//
//	For each list:
//	CString			NodeID
//	DWORD			UNID
//	DWORD			No. of objects
//
//	For each object
//	DWORD			ObjID
//
//	DWORD			No. of names
//
//	For each name
//	DWORD			ObjID
//	DWORD			Name Flags
//	CString			Name

	{
	int i;
	int j;
	DWORD dwSave;

	dwSave = m_AllLists.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < m_AllLists.GetCount(); i++)
		{
		SObjList *pList = m_AllLists[i];

		pList->pNode->GetID().WriteToStream(pStream);

		dwSave = pList->pType->GetUNID();
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		//	Write out all object IDs

		dwSave = pList->ObjectIDs.GetCount();
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		for (j = 0; j < pList->ObjectIDs.GetCount(); j++)
			{
			dwSave = pList->ObjectIDs[j];
			pStream->Write((char *)&dwSave, sizeof(DWORD));
			}

		//	Write out all object names

		dwSave = pList->ObjectNames.GetCount();
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		for (j = 0; j < pList->ObjectNames.GetCount(); j++)
			{
			dwSave = pList->ObjectNames.GetKey(j);
			pStream->Write((char *)&dwSave, sizeof(DWORD));

			const SObjName &Name = pList->ObjectNames[j];
			pStream->Write((char *)&Name.dwNameFlags, sizeof(DWORD));
			Name.sName.WriteToStream(pStream);
			}
		}
	}

