//	CLStructure.cpp
//
//	CLStructure class

#include "Alchemy.h"
#include "Labyrinth.h"

CLStructure::CLStructure (void) : m_Fields(FALSE, TRUE)
	{
	}

CLStructure::~CLStructure (void)
	{
	for (int i = 0; i < GetCount(); i++)
		{
		CLValue *pValue = GetValue(i);
		delete pValue;
		}
	}

ALERROR CLStructure::AddField (const CString &sFieldName, CLValue *pValue)
	{
	return m_Fields.AddEntry(sFieldName, (CObject *)pValue);
	}

bool CLStructure::FindField (const CString &sFieldName, CLValue **retpValue)
	{
	//	Look for it in our scope first

	if (m_Fields.Lookup(sFieldName, (CObject **)retpValue) == NOERROR)
		return true;
	else
		return false;
	}
