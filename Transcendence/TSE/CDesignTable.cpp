//	CDesignTable.cpp
//
//	CDesignTable class

#include "PreComp.h"

ALERROR CDesignTable::AddEntry (CDesignType *pEntry)

//	AddEntry
//
//	Adds an entry to the table. Returns ERR_OUTOFROOM if we already have an
//	entry with that UNID.

	{
	ASSERT(pEntry);

	bool bNew;
	m_Table.SetAt(pEntry->GetUNID(), pEntry, &bNew);
	if (!bNew)
		return ERR_OUTOFROOM;

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

ALERROR CDesignTable::Merge (const CDynamicDesignTable &Source, CDesignList *ioOverride)

//	Merge
//
//	Merge the given table into ours.

	{
	DEBUG_TRY

	int i;

	for (i = 0; i < Source.GetCount(); i++)
		{
		CDesignType *pNewType = Source.GetType(i);

		//	If this is an override then we put it on a different table and
		//	leave the type alone.

		if (pNewType->IsModification())
			{
			if (ioOverride)
				ioOverride->AddEntry(pNewType);
			}

		//	Otherwise, add or replace

		else
			AddOrReplaceEntry(pNewType);
		}

	return NOERROR;

	DEBUG_CATCH
	}

ALERROR CDesignTable::Merge (const CDesignTable &Source, CDesignList *ioOverride)

//	Merge
//
//	Merge the given table into ours. Entries in Table override our entries
//	if they have the same UNID.

	{
	int i;

	TArray<CDesignType *> Replaced;

	//	We move through both tables in order and handle when we get an 
	//	addition or overlap.

	int iSrcPos = 0;
	int iDestPos = 0;

	//	Merge

	while (iSrcPos < Source.GetCount())
		{
		//	If we're at the end of the destination then just insert

		if (iDestPos == m_Table.GetCount())
			{
			m_Table.InsertSorted(Source.m_Table.GetKey(iSrcPos), Source.m_Table.GetValue(iSrcPos));

			//	Advance

			iDestPos++;
			iSrcPos++;
			}

		//	Otherwise, see if we need to insert or replace

		else
			{
			int iCompare = AscendingSort * KeyCompare(Source.m_Table.GetKey(iSrcPos), m_Table.GetKey(iDestPos));

			//	If the same key then we replace

			if (iCompare == 0)
				{
				CDesignType *pNewType = Source.m_Table.GetValue(iSrcPos);

				//	If this is an override then we put it on a different table and
				//	leave the type alone.
				//
				//	NOTE: It is OK if we add multiple types of the same UNID. As long
				//	as we add them in order, we're OK.

				if (pNewType->IsModification())
					{
					if (ioOverride)
						ioOverride->AddEntry(pNewType);
					}

				//	Otherwise we just replace the type

				else
					{
					//	If we have to free our originals, then remember them here.

					if (m_bFreeTypes)
						{
						CDesignType *pOriginalType = m_Table.GetValue(iDestPos);
						Replaced.Insert(pOriginalType);
						}

					//	Replace

					m_Table.GetValue(iDestPos) = pNewType;
					}

				//	Advance

				iDestPos++;
				iSrcPos++;
				}

			//	If the source is less than dest then we insert at this
			//	position.

			else if (iCompare == 1)
				{
				m_Table.InsertSorted(Source.m_Table.GetKey(iSrcPos), Source.m_Table.GetValue(iSrcPos), iDestPos);

				//	Advance

				iDestPos++;
				iSrcPos++;
				}

			//	Otherwise, go to the next destination slot

			else
				iDestPos++;
			}
		}

	//	Delete replaced entries

	for (i = 0; i < Replaced.GetCount(); i++)
		delete Replaced[i];

	return NOERROR;
	}
