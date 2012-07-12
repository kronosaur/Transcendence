//	CDesignTable.cpp
//
//	CDesignTable class

#include "PreComp.h"

ALERROR CDesignTable::AddEntry (CDesignType *pEntry)

//	AddEntry
//
//	Adds an entry to the table

	{
	ASSERT(pEntry);

	m_Table.Insert(pEntry->GetUNID(), pEntry);
	return NOERROR;
	}

ALERROR CDesignTable::AddOrReplaceEntry (CDesignType *pEntry, CDesignType **retpOldEntry)

//	AddOrReplaceEntry
//
//	Adds or replaces an entry

	{
	bool bAdded;
	CDesignType **pSlot = m_Table.SetAt(pEntry->GetUNID(), &bAdded);

	if (retpOldEntry)
		*retpOldEntry = (!bAdded ? *pSlot : NULL);

	*pSlot = pEntry;

	return NOERROR;
	}

void CDesignTable::Delete (DWORD dwUNID)

//	Delete
//
//	Delete by UNID

	{
	int iIndex;
	if (m_Table.FindPos(dwUNID, &iIndex))
		{
		if (m_bFreeTypes)
			delete m_Table[iIndex];

		m_Table.Delete(iIndex);
		}
	}

void CDesignTable::DeleteAll (void)

//	DeleteAll
//
//	Removes all entries and deletes the object that they point to

	{
	int i;

	if (m_bFreeTypes)
		{
		for (i = 0; i < GetCount(); i++)
			delete GetEntry(i);
		}

	m_Table.DeleteAll();
	}

CDesignType *CDesignTable::FindByUNID (DWORD dwUNID) const

//	FindByUNID
//
//	Returns a pointer to the given entry or NULL

	{
	CDesignType **pObj = m_Table.GetAt(dwUNID);
	return (pObj ? *pObj : NULL);
	}

ALERROR CDesignTable::Merge (const CDesignTable &Table)

//	Merge
//
//	Merge the given table into ours. Entries in Table override our entries
//	if they have the same UNID.

	{
	int i;

	TArray<CDesignType *> Replaced;

	//	Merge. If we need to free replaced entries then get a list.

	m_Table.Merge(Table.m_Table, (m_bFreeTypes ? &Replaced : NULL));

	//	Delete replaced entries

	if (m_bFreeTypes)
		{
		for (i = 0; i < Replaced.GetCount(); i++)
			delete Replaced[i];
		}

	return NOERROR;
	}
