//	CLList.cpp
//
//	CLList class

#include "Alchemy.h"
#include "Labyrinth.h"

CLList::CLList (void) : m_List(FALSE)
	{
	}

CLList::~CLList (void)
	{
	for (int i = 0; i < GetCount(); i++)
		{
		CLValue *pValue = GetValue(i);
		delete pValue;
		}
	}

ALERROR CLList::AddValue (CLValue *pValue)
	{
	return m_List.AppendObject((CObject *)pValue, NULL);
	}
