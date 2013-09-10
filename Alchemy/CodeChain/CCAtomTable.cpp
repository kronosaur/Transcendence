//	CCAtomTable.cpp
//
//	Implements CCAtomTable class

#include <windows.h>
#include "Kernel.h"
#include "KernelObjID.h"
#include "CodeChain.h"

static CObjectClass<CCAtomTable>g_Class(OBJID_CCATOMTABLE, NULL);

CCAtomTable::CCAtomTable (void) : ICCAtom(&g_Class)

//	CCAtomTable constructor

	{
	}

ICCItem *CCAtomTable::AddEntry (CCodeChain *pCC, ICCItem *pAtom, ICCItem *pEntry, bool bForceLocalAdd)

//	AddEntry
//
//	Adds an entry to the symbol table and returns
//	True for success.

	{
	ICCItem *pPrevEntry = NULL;
	int iOldEntry;
	BOOL bAdded;

	if (m_Table.ReplaceEntry(pAtom->GetIntegerValue(), (int)pEntry->Reference(), TRUE, &bAdded, &iOldEntry) != NOERROR)
		return pCC->CreateMemoryError();

	//	If we have a previous entry, decrement its refcount since we're
	//	throwing it away

	pPrevEntry = (ICCItem *)iOldEntry;
	if (!bAdded && pPrevEntry)
		pPrevEntry->Discard(pCC);

	SetModified();

	return pCC->CreateTrue();
	}

ICCItem *CCAtomTable::Clone (CCodeChain *pCC)

//	Clone
//
//	Clone this item

	{
	ASSERT(FALSE);
	return pCC->CreateNil();
	}

void CCAtomTable::DestroyItem (CCodeChain *pCC)

//	DestroyItem
//
//	Destroy this item

	{
	int i;

	//	Release all the entries

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		int iKey, iValue;
		ICCItem *pItem;

		m_Table.GetEntry(i, &iKey, &iValue);
		pItem = (ICCItem *)iValue;
		pItem->Discard(pCC);
		}

	//	Remove all symbols

	m_Table.RemoveAll();

	//	Destroy this item

	pCC->DestroyAtomTable(this);
	}

ICCItem *CCAtomTable::ListSymbols (CCodeChain *pCC)

//	ListSymbols
//
//	Returns a list of all the atoms in the table

	{
	//	If there are no symbols, return Nil

	if (m_Table.GetCount() == 0)
		return pCC->CreateNil();

	//	Otherwise, make a list

	else
		{
		int i;
		ICCItem *pResult;
		CCLinkedList *pList;

		pResult = pCC->CreateLinkedList();
		if (pResult->IsError())
			return pResult;

		pList = (CCLinkedList *)pResult;

		for (i = 0; i < m_Table.GetCount(); i++)
			{
			ICCItem *pItem;
			int iKey;

			m_Table.GetEntry(i, &iKey, NULL);

			//	Make an item for the symbol

			pItem = pCC->CreateInteger(iKey);

			//	Add the item to the list

			pList->Append(pCC, pItem, NULL);
			pItem->Discard(pCC);
			}

		return pList;
		}
	}

ICCItem *CCAtomTable::Lookup (CCodeChain *pCC, ICCItem *pAtom)

//	Lookup
//
//	Looks up the key and returns the association. If no
//	Association is found, returns Nil

	{
	return LookupEx(pCC, pAtom, NULL);
	}

ICCItem *CCAtomTable::LookupEx (CCodeChain *pCC, ICCItem *pAtom, BOOL *retbFound)

//	LookupEx
//
//	Looks up the key and returns the association. If no
//	Association is found, returns an error

	{
	ALERROR error;
	int iValue;
	ICCItem *pBinding;

	if (error = m_Table.Find(pAtom->GetIntegerValue(), &iValue))
		{
		if (error == ERR_NOTFOUND)
			{
			if (retbFound)
				*retbFound = FALSE;

			return pCC->CreateErrorCode(CCRESULT_NOTFOUND);
			}
		else
			return pCC->CreateMemoryError();
		}

	pBinding = (ICCItem *)iValue;
	ASSERT(pBinding);

	if (retbFound)
		*retbFound = TRUE;

	return pBinding->Reference();
	}

CString CCAtomTable::Print (CCodeChain *pCC, DWORD dwFlags)

//	Print
//
//	Render as text

	{
	return LITERAL("[atom table]");
	}

void CCAtomTable::Reset (void)

//	Reset
//
//	Reset the internal variables

	{
	m_Table.RemoveAll();
	}

ICCItem *CCAtomTable::StreamItem (CCodeChain *pCC, IWriteStream *pStream)

//	StreamItem
//
//	Stream the sub-class specific data

	{
	ALERROR error;
	int iCount;
	int i;

	//	Write out the count

	iCount = m_Table.GetCount();
	if (error = pStream->Write((char *)&iCount, sizeof(iCount), NULL))
		return pCC->CreateSystemError(error);

	//	Write out each of the items in the list

	for (i = 0; i < iCount; i++)
		{
		int iValue, iKey;
		ICCItem *pItem;
		ICCItem *pKey;
		ICCItem *pError;

		m_Table.GetEntry(i, &iKey, &iValue);
		pItem = (ICCItem *)iValue;

		//	Write out the key

		pKey = pCC->CreateInteger(iKey);
		if (pKey->IsError())
			return pKey;

		pError = pCC->StreamItem(pKey, pStream);
		pKey->Discard(pCC);
		if (pError->IsError())
			return pError;

		pError->Discard(pCC);

		//	Write out the value

		pError = pCC->StreamItem(pItem, pStream);
		if (pError->IsError())
			return pError;

		pError->Discard(pCC);

		//	Note that there is no need to discard pItem
		//	since we did not increase its refcount
		}

	return pCC->CreateTrue();
	}

ICCItem *CCAtomTable::UnstreamItem (CCodeChain *pCC, IReadStream *pStream)

//	UnstreamItem
//
//	Unstream the sub-class specific data

	{
	ALERROR error;
	int i, iCount;

	//	Read the count

	if (error = pStream->Read((char *)&iCount, sizeof(iCount), NULL))
		return pCC->CreateSystemError(error);

	//	Read all the items

	for (i = 0; i < iCount; i++)
		{
		ICCItem *pItem;
		int iKey, iValue;
		BOOL bAdded;

		//	Load the key

		pItem = pCC->UnstreamItem(pStream);
		if (pItem->IsError())
			return pItem;

		iKey = pItem->GetIntegerValue();
		pItem->Discard(pCC);

		//	Now load the value

		pItem = pCC->UnstreamItem(pStream);

		//	Note that we don't abort in case of an error
		//	because the list might contain errors

		//	Append the item to the symbol table

		if (m_Table.ReplaceEntry(iKey, (int)pItem, TRUE, &bAdded, &iValue) != NOERROR)
			return pCC->CreateMemoryError();

		//	No need to discard pItem because we're adding it to the
		//	symbol table.

		//	If we have a previous entry, decrement its refcount since we're
		//	throwing it away

		ASSERT(bAdded);
		}

	return pCC->CreateTrue();
	}
