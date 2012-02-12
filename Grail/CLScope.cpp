//	CLScope.cpp
//
//	CLScope class

#include "Alchemy.h"
#include "Labyrinth.h"

CLScope::CLScope (CLScope *pParent) : m_pParent(pParent)
	{
	}

CLScope::~CLScope (void)
	{
	//	Delete all sub-entries

	for (int i = 0; i < m_Entries.GetCount(); i++)
		{
		CLEntry *pEntry = (CLEntry *)m_Entries.GetValue(i);
		delete pEntry;
		}
	}

ALERROR CLScope::AddEntry (CLEntry *pEntry)
	{
	return m_Entries.AddEntry(pEntry->GetName(), (CObject *)pEntry);
	}

bool CLScope::FindEntry (const CString &sName, CLEntry **retpEntry)
	{
	//	Look for it in our scope first

	if (m_Entries.Lookup(sName, (CObject **)retpEntry) == NOERROR)
		return true;

	//	Otherwise look for it in our parent's scope

	if (m_pParent)
		return m_pParent->FindEntry(sName, retpEntry);

	//	If we have no parent and we get here then we did not find
	//	the symbol in any scope.

	return false;
	}
