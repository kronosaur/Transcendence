//	CDaimonList.cpp
//
//	CDaimonList class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

CDaimonList::CDaimonList (void) :
		m_dwNextID(1),
		m_iSelection(0)

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
	}
