//	CCLinkedList.cpp
//
//	Implements CCLinkedList class

#include <windows.h>
#include "Kernel.h"
#include "KernelObjID.h"
#include "CodeChain.h"

#define MIN_UNINDEXED_LOOKUP					5

static CObjectClass<CCLinkedList>g_Class(OBJID_CCLINKEDLIST, NULL);

CCLinkedList::CCLinkedList (void) : ICCList(&g_Class),
		m_pFirst(NULL),
		m_pLast(NULL),
		m_iCount(0),
		m_pIndex(NULL)

//	CCInteger constructor

	{
	}

CCLinkedList::~CCLinkedList (void)

//	CCLinkedList destructor

	{
	}

void CCLinkedList::Append (CCodeChain *pCC, ICCItem *pItem, ICCItem **retpError)

//	Append
//
//	Appends the item to the list

	{
	CCons *pCons;

	//	Create a new cons

	pCons = pCC->CreateCons();
	if (pCons == NULL)
		{
		if (retpError)
			*retpError = pCC->CreateMemoryError();
		return;
		}

	pCons->m_pItem = pItem->Reference();
	pCons->m_pNext = NULL;

	//	Link it to the rest of the list

	if (m_pLast)
		m_pLast->m_pNext = pCons;
	m_pLast = pCons;

	if (m_pFirst == NULL)
		m_pFirst = pCons;

	m_iCount++;

	//	Discard index since we've changed things

	if (m_pIndex)
		{
		delete [] m_pIndex;
		m_pIndex = NULL;
		}

	//	Done

	if (retpError)
		*retpError = pCC->CreateTrue();
	}

void CCLinkedList::AppendIntegerValue (CCodeChain *pCC, int iValue, ICCItem **retpError)

//	AppendIntegerValue
//
//	Appends an integer

	{
	ICCItem *pItem = pCC->CreateInteger(iValue);
	Append(pCC, pItem, retpError);
	pItem->Discard(pCC);
	}

void CCLinkedList::AppendStringValue (CCodeChain *pCC, const CString &sString, ICCItem **retpError)

//	AppendStringValue
//
//	Append a string

	{
	ICCItem *pItem = pCC->CreateString(sString);
	Append(pCC, pItem, retpError);
	pItem->Discard(pCC);
	}

ICCItem *CCLinkedList::Clone (CCodeChain *pCC)

//	Clone
//
//	Returns a copy of the item

	{
	ICCItem *pNew;
	CCLinkedList *pClone;
	CCons *pCons;

	pNew = pCC->CreateLinkedList();
	if (pNew->IsError())
		return pNew;

	pClone = dynamic_cast<CCLinkedList *>(pNew);
	pClone->CloneItem(this);

	//	Copy all the items

	pCons = m_pFirst;
	while (pCons)
		{
		pClone->Append(pCC, pCons->m_pItem, NULL);
		pCons = pCons->m_pNext;
		}

	return pClone;
	}

void CCLinkedList::CreateIndex (void)

//	CreateIndex
//
//	Create an index so that we can do lookups by position

	{
	if (m_pIndex)
		return;

	m_pIndex = new CCons *[m_iCount];
	if (m_pIndex)
		{
		int i;
		CCons *pCons;

		pCons = m_pFirst;
		i = 0;
		while (pCons)
			{
			m_pIndex[i] = pCons;
			i++;
			pCons = pCons->m_pNext;
			}
		}
	}

void CCLinkedList::DestroyItem (CCodeChain *pCC)

//	DestroyItem
//
//	Destroys the item

	{
	CCons *pCons;

	//	Discard all items that we contain

	pCons = m_pFirst;
	while (pCons)
		{
		CCons *pNext = pCons->m_pNext;
		pCons->m_pItem->Discard(pCC);
		pCC->DestroyCons(pCons);
		pCons = pNext;
		}

	//	Free the index

	if (m_pIndex)
		delete [] m_pIndex;

	//	Give the item back

	pCC->DestroyLinkedList(this);
	}

ICCItem *CCLinkedList::Enum (CEvalContext *pCtx, ICCItem *pCode)

//	Enum
//
//	Iterate over all the items in the list and execute pCode

	{
	return pCtx->pCC->CreateNil();
	}

ICCItem *CCLinkedList::GetElement (int iIndex)

//	GetElement
//
//	Returns the nth element in the list. iIndex is 0-based.
//	If iIndex is out of range, we return NULL.

	{
	CCons *pCons;

	//	If iIndex is pretty large and we don't have an
	//	index, then create one

	if (iIndex > MIN_UNINDEXED_LOOKUP && m_pIndex == NULL)
		CreateIndex();

	//	If we've got an index, just look it up

	if (m_pIndex)
		{
		if (iIndex <= 0)
			pCons = m_pFirst;
		else if (iIndex < m_iCount)
			pCons = m_pIndex[iIndex];
		else
			pCons = NULL;
		}

	//	Otherwise we need to iterate

	else
		{
		pCons = m_pFirst;
		while (pCons && iIndex > 0)
			{
			pCons = pCons->m_pNext;
			iIndex--;
			}
		}

	//	Done

	if (pCons)
		return pCons->m_pItem;
	else
		return NULL;
	}

CString CCLinkedList::Print (CCodeChain *pCC, DWORD dwFlags)

//	Print
//
//	Returns a text representation of this item

	{
	//	If we have a lot of items in the list, then we choose
	//	a different method for generating the string

	if (GetCount() > 16)
		{
		CMemoryWriteStream Stream;
		if (Stream.Create() != NOERROR)
			return CONSTLIT("ERROR-OUT-OF-MEMORY");

		CCons *pNext = m_pFirst;

		//	Open paren

		if (IsQuoted())
			Stream.Write("'(", 2);
		else
			Stream.Write("(", 1);

		//	Items

		while (pNext)
			{
			CString sItem = pNext->m_pItem->Print(pCC);
			Stream.Write(sItem.GetASCIIZPointer(), sItem.GetLength());

			pNext = pNext->m_pNext;

			if (pNext)
				Stream.Write(" ", 1);
			}

		//	Close paren

		Stream.Write(")", 1);
		return CString(Stream.GetPointer(), Stream.GetLength());
		}
	else
		{
		CCons *pNext = m_pFirst;
		CString sString;

		//	Open paren

		if (IsQuoted())
			sString = LITERAL("'(");
		else
			sString = LITERAL("(");

		//	Items

		while (pNext)
			{
			sString.Append(pNext->m_pItem->Print(pCC));

			pNext = pNext->m_pNext;

			if (pNext)
				sString.Append(LITERAL(" "));
			}

		//	Close paren

		sString.Append(LITERAL(")"));
		return sString;
		}
	}

void CCLinkedList::QuickSort (int iLeft, int iRight, int iOrder)

//	QuickSort
//
//	QuickSort algorithm on elements of the list
//
//	iOrder 1 = descending
//	iOrder -1 = ascending

	{
	int i;

	ASSERT(m_pIndex);
	ASSERT(iLeft < iRight);
	ASSERT(iLeft >= 0 && iRight < m_iCount);

	//	Deal with small lists

	if (iLeft + 1 == iRight)
		{
		if (iOrder * HelperCompareItems(m_pIndex[iLeft]->m_pItem, m_pIndex[iRight]->m_pItem) < 0)
			Swap(m_pIndex[iLeft], m_pIndex[iRight]);
		}
	else
		{
		//	Pick a pivot

		int iPivot = (iLeft + iRight) / 2;

		//	Order left, right, and pivot.

		if (iOrder * HelperCompareItems(m_pIndex[iLeft]->m_pItem, m_pIndex[iRight]->m_pItem) < 0)
			Swap(m_pIndex[iLeft], m_pIndex[iRight]);

		if (iOrder * HelperCompareItems(m_pIndex[iLeft]->m_pItem, m_pIndex[iPivot]->m_pItem) < 0)
			Swap(m_pIndex[iLeft], m_pIndex[iPivot]);
		else if (iOrder * HelperCompareItems(m_pIndex[iPivot]->m_pItem, m_pIndex[iRight]->m_pItem) < 0)
			Swap(m_pIndex[iPivot], m_pIndex[iRight]);

		//	We take the value of the pivot (because we can't guarantee that the pivot will
		//	end up in the middle after partition).

		ICCItem *pPivot = m_pIndex[iPivot]->m_pItem;

		//	Swap the pivot to the end so we don't have to partition it

		Swap(m_pIndex[iPivot], m_pIndex[iRight]);

		//	Partition around the pivot

		int iStoreIndex = iLeft;
		for (i = iLeft; i < iRight; i++)
			if (iOrder * HelperCompareItems(m_pIndex[i]->m_pItem, pPivot) > 0)
				{
				Swap(m_pIndex[iStoreIndex], m_pIndex[i]);
				iStoreIndex++;
				}

		//	Move pivot to final place

		Swap(m_pIndex[iStoreIndex], m_pIndex[iRight]);

		//	Recurse

		if (iLeft < iStoreIndex - 1)
			QuickSort(iLeft, iStoreIndex - 1, iOrder);

		if (iStoreIndex + 1 < iRight)
			QuickSort(iStoreIndex + 1, iRight, iOrder);
		}
	}

void CCLinkedList::QuickSortLists (int iKeyIndex, int iLeft, int iRight, int iOrder)

//	QuickSortLists
//
//	Assumes that this is a list of lists and sorts based on the given key index
//	(0-based)

	{
	int i;

	ASSERT(m_pIndex);
	ASSERT(iLeft < iRight);
	ASSERT(iLeft >= 0 && iRight < m_iCount);

	//	Deal with small lists

	if (iLeft + 1 == iRight)
		{
		if (iOrder * HelperCompareItemsLists(m_pIndex[iLeft]->m_pItem, m_pIndex[iRight]->m_pItem, iKeyIndex) < 0)
			Swap(m_pIndex[iLeft], m_pIndex[iRight]);
		}
	else
		{
		//	Pick a pivot

		int iPivot = (iLeft + iRight) / 2;

		//	Order left, right, and pivot.

		if (iOrder * HelperCompareItemsLists(m_pIndex[iLeft]->m_pItem, m_pIndex[iRight]->m_pItem, iKeyIndex) < 0)
			Swap(m_pIndex[iLeft], m_pIndex[iRight]);

		if (iOrder * HelperCompareItemsLists(m_pIndex[iLeft]->m_pItem, m_pIndex[iPivot]->m_pItem, iKeyIndex) < 0)
			Swap(m_pIndex[iLeft], m_pIndex[iPivot]);
		else if (iOrder * HelperCompareItemsLists(m_pIndex[iPivot]->m_pItem, m_pIndex[iRight]->m_pItem, iKeyIndex) < 0)
			Swap(m_pIndex[iPivot], m_pIndex[iRight]);

		//	We take the value of the pivot (because we can't guarantee that the pivot will
		//	end up in the middle after partition).

		ICCItem *pPivot = m_pIndex[iPivot]->m_pItem;

		//	Swap the pivot to the end so we don't have to partition it

		Swap(m_pIndex[iPivot], m_pIndex[iRight]);

		//	Partition around the pivot

		int iStoreIndex = iLeft;
		for (i = iLeft; i < iRight; i++)
			if (iOrder * HelperCompareItemsLists(m_pIndex[i]->m_pItem, pPivot, iKeyIndex) > 0)
				{
				Swap(m_pIndex[iStoreIndex], m_pIndex[i]);
				iStoreIndex++;
				}

		//	Move pivot to final place

		Swap(m_pIndex[iStoreIndex], m_pIndex[iRight]);

		//	Recurse

		if (iLeft < iStoreIndex - 1)
			QuickSortLists(iKeyIndex, iLeft, iStoreIndex - 1, iOrder);

		if (iStoreIndex + 1 < iRight)
			QuickSortLists(iKeyIndex, iStoreIndex + 1, iRight, iOrder);
		}
	}

void CCLinkedList::RemoveElement (CCodeChain *pCC, int iIndex)

//	RemoveElement
//
//	Removes the nth entry

	{
	//	Check the range

	if (iIndex < 0 || iIndex >= m_iCount)
		return;

	//	Look for the cons entry

	CCons *pPrevCons;
	CCons *pCons;

	if (m_pIndex)
		{
		if (iIndex == 0)
			pPrevCons = NULL;
		else
			pPrevCons = m_pIndex[iIndex - 1];
		pCons = m_pIndex[iIndex];
		}
	else
		{
		pPrevCons = NULL;
		pCons = m_pFirst;
		while (pCons && iIndex > 0)
			{
			pPrevCons = pCons;
			pCons = pCons->m_pNext;
			iIndex--;
			}
		}

	//	Relink

	if (pPrevCons)
		pPrevCons->m_pNext = pCons->m_pNext;
	else
		{
		ASSERT(m_pFirst == pCons);
		m_pFirst = pCons->m_pNext;
		}

	if (pCons->m_pNext == NULL)
		{
		ASSERT(m_pLast == pCons);
		m_pLast = pPrevCons;
		}

	//	Count

	m_iCount--;
	ASSERT(m_iCount >= 0);

	//	Discard cons

	pCons->m_pItem->Discard(pCC);
	pCC->DestroyCons(pCons);

	//	Discard index since we've changed things

	if (m_pIndex)
		{
		delete [] m_pIndex;
		m_pIndex = NULL;
		}
	}

void CCLinkedList::ReplaceElement (CCodeChain *pCC, int iIndex, ICCItem *pNewItem)

//	ReplaceElement
//
//	Replaces then nth entry in the list with the given
//	item. If the nth item does not exist, nothing gets
//	added.

	{
	CCons *pCons;

	//	Check the range

	if (iIndex < 0 || iIndex >= m_iCount)
		return;

	//	Look for the cons entry

	if (m_pIndex)
		pCons = m_pIndex[iIndex];
	else
		{
		pCons = m_pFirst;
		while (pCons && iIndex > 0)
			{
			pCons = pCons->m_pNext;
			iIndex--;
			}
		}

	//	Change the link

	pCons->m_pItem->Discard(pCC);
	pCons->m_pItem = pNewItem->Reference();
	}

void CCLinkedList::Reset (void)

//	Reset
//
//	Reset to initial conditions

	{
	ASSERT(m_dwRefCount == 0);
	m_pFirst = NULL;
	m_pLast = NULL;
	m_iCount = 0;
	m_pIndex = NULL;
	}

void CCLinkedList::Shuffle (CCodeChain *pCC)

//	Shuffle
//
//	Shuffles the elements in the list

	{
	if (m_iCount < 2)
		return;

	//	We need an index for this

	CreateIndex();

	//	Fisher-Yates algorithm

	int i = m_iCount - 1;
	while (i > 0)
		{
		int x = mathRandom(0, i);

		CCons *pTemp = m_pIndex[x];
		m_pIndex[x] = m_pIndex[i];
		m_pIndex[i] = pTemp;

		i--;
		}

	//	Fixup all the pointers

	UpdateLinksFromIndex();
	}

void CCLinkedList::Sort (CCodeChain *pCC, int iOrder, int iIndex)

//	Sort
//
//	Sorts the list. If iIndex is -1, then we sort by comparing
//	each item in the list. Otherwise, we assume that this is
//	a list of lists and use iIndex as the sort key index.

	{
	if (m_iCount < 2)
		return;

	//	We need an index for this

	CreateIndex();

	//	Do a quick sort

	if (iIndex != -1)
		QuickSortLists(iIndex, 0, m_iCount - 1, iOrder);
	else
		QuickSort(0, m_iCount - 1, iOrder);

	//	Fixup all the pointers

	UpdateLinksFromIndex();
	}

ICCItem *CCLinkedList::StreamItem (CCodeChain *pCC, IWriteStream *pStream)

//	StreamItem
//
//	Stream the sub-class specific data

	{
	ALERROR error;
	CCons *pCons;

	//	Write out the count

	if (error = pStream->Write((char *)&m_iCount, sizeof(m_iCount), NULL))
		return pCC->CreateSystemError(error);

	//	Write out each of the items in the list

	pCons = m_pFirst;
	while (pCons)
		{
		ICCItem *pError;

		pError = pCC->StreamItem(pCons->m_pItem, pStream);
		if (pError->IsError())
			return pError;

		pError->Discard(pCC);

		pCons = pCons->m_pNext;
		}

	return pCC->CreateTrue();
	}

ICCItem *CCLinkedList::Tail (CCodeChain *pCC)

//	Tail
//
//	Returns a list that includes all items in this list
//	after the head. If there are no more items, it retuns
//	Nil

	{
	CCons *pNext = m_pFirst->m_pNext;

	if (pNext == NULL)
		return pCC->CreateNil();
	else
		{
		ICCItem *pNew;
		CCLinkedList *pTail;

		pNew = pCC->CreateLinkedList();
		if (pNew->IsError())
			return pNew;

		pTail = dynamic_cast<CCLinkedList *>(pNew);

		//	Add all but the first item to the list

		while (pNext)
			{
			pTail->Append(pCC, pNext->m_pItem, NULL);
			pNext = pNext->m_pNext;
			}

		//	Done

		return pTail;
		}
	}

ICCItem *CCLinkedList::UnstreamItem (CCodeChain *pCC, IReadStream *pStream)

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

		pItem = pCC->UnstreamItem(pStream);

		//	Note that we don't abort in case of an error
		//	because the list might contain errors

		//	Append the item to the list

		Append(pCC, pItem, NULL);
		pItem->Discard(pCC);
		}

	return pCC->CreateTrue();
	}

void CCLinkedList::UpdateLinksFromIndex (void)

//	UpdateLinksFromIndex
//
//	Updates the links in the list from the index
//	(The caller has modified the index)

	{
	int i;

	ASSERT(m_pIndex);

	//	Fixup all the pointers

	for (i = 0; i < m_iCount - 1; i++)
		m_pIndex[i]->m_pNext = m_pIndex[i + 1];

	m_pIndex[m_iCount - 1]->m_pNext = NULL;

	//	Update first and last

	m_pFirst = m_pIndex[0];
	m_pLast = m_pIndex[m_iCount - 1];
	}
