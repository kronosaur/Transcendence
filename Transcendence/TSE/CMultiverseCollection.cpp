//	CMultiverseCollection.cpp
//
//	CMultiverseCollection Class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

void CMultiverseCollection::DeleteAll (void)

//	DeleteAll
//
//	Delete all entries in the collection

	{
	int i;

	for (i = 0; i < m_List.GetCount(); i++)
		delete m_List[i];

	m_List.DeleteAll();
	}

bool CMultiverseCollection::HasAllUNIDs (const TArray<DWORD> &UNIDList) const

//	HasAllUNIDs
//
//	Returns TRUE if we have all of the given unids

	{
	int i;

	for (i = 0; i < UNIDList.GetCount(); i++)
		if (!HasUNID(UNIDList[i]))
			return false;

	return true;
	}

bool CMultiverseCollection::HasAnyUNID (const TArray<DWORD> &UNIDList) const

//	HasAnyUNID
//
//	Returns TRUE if we have any of the given UNIDs

	{
	int i;

	for (i = 0; i < UNIDList.GetCount(); i++)
		if (HasUNID(UNIDList[i]))
			return true;

	return false;
	}

bool CMultiverseCollection::HasUNID (DWORD dwUNID) const

//	HasUNID
//
//	Returns TRUE if we have the given UNID

	{
	int i;

	for (i = 0; i < m_List.GetCount(); i++)
		if (m_List[i]->GetUNID() == dwUNID)
			return true;

	return false;
	}
