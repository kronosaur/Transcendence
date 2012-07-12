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
