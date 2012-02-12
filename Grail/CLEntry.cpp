//	CLEntry.cpp
//
//	CLEntry class

#include "Alchemy.h"
#include "Labyrinth.h"

CLEntry::CLEntry (const CString &sName, CLScope *pParentScope) : m_sName(sName),
		m_Scope(pParentScope)
	{
	}

void CLEntry::AddCategory (const CString &sCategory)
	{
	m_Categories.AppendString(sCategory, NULL);
	}

