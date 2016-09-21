//	CDaimonList.cpp
//
//	CDaimonList class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

CDaimonList::CDaimonList (void) :
		m_dwNextID(1),
		m_iSelection(-1)

//	CDaimonList constructor

	{
	}

CDaimonList::~CDaimonList (void)

//	CDaimonList destructor

	{
	DeleteAll();
	}

void CDaimonList::Add (CItemType *pItem)

//	Add
//
//	Adds a Daimon to the list.

	{
	ASSERT(pItem);

	//	Allocate a new entry

	SDaimonEntry *pEntry = new SDaimonEntry;
	pEntry->dwID = m_dwNextID++;
	pEntry->pDaimon = pItem;

	//	Add it to our main list, sorted by ID

	m_List.Insert(pEntry->dwID, pEntry);

	//	Add it to our sorted list.

	m_Sorted.Insert(pItem->GetNounPhrase(), pEntry);

	//	Set a selection if we don't already have one.

	if (m_iSelection == -1)
		m_iSelection = 0;
	}

void CDaimonList::DeleteAll (void)

//	DeleteAll
//
//	Delete all daimons

	{
	int i;

	for (i = 0; i < m_List.GetCount(); i++)
		delete m_List[i];

	m_List.DeleteAll();
	m_Sorted.DeleteAll();
	m_iSelection = -1;
	}

int CDaimonList::DeleteSelectedDaimon (void)

//	DeleteSelectedDaimon
//
//	Deletes the selected daimon and returns the new selection.

	{
	if (m_iSelection == -1 || GetCount() == 0)
		return -1;

	SDaimonEntry *pToDelete = m_Sorted[GetSelection()];
	m_List.DeleteAt(pToDelete->dwID);
	m_Sorted.Delete(GetSelection());
	delete pToDelete;

	//	Set the selection

	if (m_iSelection >= m_Sorted.GetCount())
		m_iSelection = m_Sorted.GetCount() - 1;

	return m_iSelection;
	}
