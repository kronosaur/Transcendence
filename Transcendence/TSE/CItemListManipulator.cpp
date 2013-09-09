//	CItemListManipulator.cpp
//
//	CItemListManipulator object

#include "PreComp.h"


#define LOOKUP_TAG							CONSTLIT("Lookup")
#define ITEM_TAG							CONSTLIT("Item")
#define NULL_TAG							CONSTLIT("Null")

#define TABLE_ATTRIB						CONSTLIT("table")
#define ITEM_ATTRIB							CONSTLIT("item")
#define CATEGORIES_ATTRIB					CONSTLIT("categories")
#define LEVEL_ATTRIB						CONSTLIT("level")
#define DAMAGED_ATTRIB						CONSTLIT("damaged")

CItemListManipulator::CItemListManipulator (CItemList &ItemList) : m_ItemList(ItemList),
		m_iCursor(-1),
		m_bUseFilter(false)

//	CItemListManipulator constructor

	{
	GenerateViewMap();
	CItem::InitCriteriaAll(&m_Filter);
	}

CItemListManipulator::~CItemListManipulator (void)

//	CItemListManipulator destructor

	{
	}

void CItemListManipulator::AddItem (const CItem &Item)

//	AddItem
//
//	Adds an item to the list and puts the cursor on the newly added item. Note
//	that this will fold similar items together, so the item at the cursor is not
//	guaranteed to be the same count.

	{
	int iIndex = FindItem(Item);
	if (iIndex != -1)
		{
		CItem &ThisItem = m_ItemList.GetItem(m_ViewMap.GetElement(iIndex));
		ThisItem.SetCount(Item.GetCount() + ThisItem.GetCount());
		m_iCursor = iIndex;
		}
	else
		{
		m_iCursor = m_ViewMap.GetCount();
		m_ViewMap.AppendElement(m_ItemList.GetCount(), NULL);
		m_ItemList.AddItem(Item);
		}
	}

DWORD CItemListManipulator::AddItemEnhancementAtCursor (const CItemEnhancement &Mods, int iCount)

//	AddItemEnhancementAtCursor
//
//	Adds the given item enhancement and returns the ID

	{
	ASSERT(m_iCursor != -1);
	CItem &OldItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	CItem NewItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));

	NewItem.AddEnhancement(Mods);
	if (iCount != -1)
		NewItem.SetCount(Min(iCount, OldItem.GetCount()));

	MoveItemTo(NewItem, OldItem);

	return GetItemAtCursor().GetMods().GetID();
	}

void CItemListManipulator::AddItems (const CItemList &ItemList)

//	AddItems
//
//	Adds all the items in the list

	{
	for (int i = 0; i < ItemList.GetCount(); i++)
		AddItem(ItemList.GetItem(i));
	}

void CItemListManipulator::ClearDisruptedAtCursor (int iCount)

//	ClearDisruptedAtCursor
//
//	Clears the disrupted time

	{
	ASSERT(m_iCursor != -1);

	CItem &OldItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));

	//	Generate a new item

	CItem NewItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	if (iCount != -1)
		NewItem.SetCount(Min(iCount, OldItem.GetCount()));
	NewItem.ClearDisrupted();

	MoveItemTo(NewItem, OldItem);
	}

void CItemListManipulator::DeleteAtCursor (int iCount)

//	DeleteAtCursor
//
//	Deletes a certain number of copies at the cursor. If
//	All the copies of the item are deleted, the entire item
//	is deleted.

	{
	ASSERT(m_iCursor != -1);

	CItem &ThisItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));

	if (ThisItem.GetCount() > iCount)
		ThisItem.SetCount(ThisItem.GetCount() - iCount);
	else
		{
		m_ItemList.DeleteItem(m_ViewMap.GetElement(m_iCursor));
		GenerateViewMap();
		if (m_iCursor >= m_ViewMap.GetCount())
			m_iCursor = m_ViewMap.GetCount() - 1;
		}
	}

void CItemListManipulator::DeleteMarkedItems (void)

//	DeleteMarkedItems
//
//	Deletes all items marked for deletion

	{
	for (int i = 0; i < m_ItemList.GetCount(); i++)
		{
		CItem &ThisItem = m_ItemList.GetItem(i);

		if (ThisItem.IsMarkedForDelete())
			{
			m_ItemList.DeleteItem(i);
			i--;
			}
		}
	}

int CItemListManipulator::FindItem (const CItem &Item)

//	FindItem
//
//	Finds an item in the list that matches the given one. Returns
//	the index in the list of the item or -1 if not found.

	{
	for (int i = 0; i < m_ViewMap.GetCount(); i++)
		{
		CItem &ThisItem = m_ItemList.GetItem(m_ViewMap.GetElement(i));

		if (ThisItem.IsEqual(Item))
			return i;
		}

	return -1;
	}

void CItemListManipulator::GenerateViewMap (void)

//	GenerateViewMap
//
//	Generates a mapping from index to an index into the item list

	{
	m_ViewMap.RemoveAll();

	for (int i = 0; i < m_ItemList.GetCount(); i++)
		{
		CItem &ThisItem = m_ItemList.GetItem(i);

		if (!m_bUseFilter || ThisItem.MatchesCriteria(m_Filter))
			m_ViewMap.AppendElement(i, NULL);
		}
	}

const CItem &CItemListManipulator::GetItemAtCursor (void)

//	GetItemAtCursor
//
//	Returns the item at the cursor

	{
	ASSERT(m_iCursor != -1);
	return m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	}

CItem *CItemListManipulator::GetItemPointerAtCursor (void)

//	GetItemPointerAtCursor
//
//	Returns a pointer to the item

	{
	ASSERT(m_iCursor != -1);
	return &m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	}

void CItemListManipulator::MarkDeleteAtCursor (int iCount)

//	MarkDeleteAtCursor
//
//	Deletes a certain number of copies at the cursor. If
//	All the copies of the item are deleted, the entire item
//	is marked for deletion (and is deleted when we call
//	DeleteMarkedItems)

	{
	ASSERT(m_iCursor != -1);

	CItem &ThisItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));

	if (ThisItem.GetCount() > iCount)
		ThisItem.SetCount(ThisItem.GetCount() - iCount);
	else
		ThisItem.MarkForDelete();
	}

bool CItemListManipulator::MoveCursorBack (void)

//	MoveCursorBack
//
//	Moves the cursor to the previous item. If there are no more item
//	returns false

	{
	if (m_iCursor == -1 && m_ViewMap.GetCount() > 0)
		{
		m_iCursor = m_ViewMap.GetCount() - 1;
		return true;
		}
	else if (m_iCursor <= 0)
		return false;
	else
		{
		m_iCursor--;
		return true;
		}
	}

bool CItemListManipulator::MoveCursorForward (void)

//	MoveCursorForward
//
//	Moves the cursor to the next item. If there are no more items
//	returns false

	{
	if (m_iCursor + 1 == m_ViewMap.GetCount())
		return false;
	else
		{
		m_iCursor++;
		return true;
		}
	}

void CItemListManipulator::MoveItemTo (const CItem &NewItem, const CItem &OldItem)

//	MoveItemTo
//
//	Move NewItem.GetCount() item from OldItem to NewItem, adding and removing items
//	as appropriate. The cursor is left on NewItem

	{
	if (SetCursorAtItem(OldItem))
		{
		//	If new and old items are the same, change in place

		if (OldItem.IsEqual(NewItem))
			{
			CItem &Item = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
			Item = NewItem;
			}

		//	If we're replacing all the old items (and we can't coalesce)
		//	then change in place

		else if (OldItem.GetCount() <= NewItem.GetCount()
				&& FindItem(NewItem) == -1)
			{
			CItem &Item = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
			Item = NewItem;
			}

		//	Otherwise, we delete and add

		else
			{
			//	The device mechanism relies on the fact that pointers
			//	to installed items do not change until uninstalled.

			ASSERT(!OldItem.IsInstalled() || !NewItem.IsInstalled());

			DeleteAtCursor(NewItem.GetCount());
			AddItem(NewItem);
			}
		}
	else
		AddItem(NewItem);
	}

bool CItemListManipulator::Refresh (const CItem &Item)

//	Refresh
//
//	Refreshes the manipulator from the list and selects
//	the given item.
//
//	Returns TRUE if selection succeeded

	{
	GenerateViewMap();

	if (Item.GetType() == NULL)
		{
		ResetCursor();
		return true;
		}

	return SetCursorAtItem(Item);
	}

void CItemListManipulator::RemoveItemEnhancementAtCursor (DWORD dwID, int iCount)

//	RemoveItemEnhancementAtCursor
//
//	Removes the item enhancement by ID

	{
	ASSERT(m_iCursor != -1);
	CItem &OldItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	CItem NewItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));

	if (iCount != -1)
		NewItem.SetCount(Min(iCount, OldItem.GetCount()));

	if (NewItem.RemoveEnhancement(dwID))
		MoveItemTo(NewItem, OldItem);
	}

void CItemListManipulator::ResetCursor (void)

//	ResetCursor
//
//	Resets the cursor to the beginning

	{
	m_iCursor = -1;
	}

void CItemListManipulator::SetChargesAtCursor (int iCharges, int iCount)

//	SetChargesAtCursor
//
//	Sets the data field

	{
	ASSERT(m_iCursor != -1);
	CItem &OldItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	CItem NewItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	if (iCount != -1)
		NewItem.SetCount(Max(0, Min(iCount, OldItem.GetCount())));
	NewItem.SetCharges(iCharges);

	MoveItemTo(NewItem, OldItem);
	}

void CItemListManipulator::SetCountAtCursor (int iCount)

//	SetCountAtCursor
//
//	Sets the number of items

	{
	ASSERT(m_iCursor != -1);
	CItem &OldItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));

	if (iCount <= 0)
		DeleteAtCursor(OldItem.GetCount());
	else
		OldItem.SetCount(iCount);
	}

bool CItemListManipulator::SetCursorAtItem (const CItem &Item)

//	SetCursorAtItem
//
//	Positions the cursor at the given item
//	Return true if the item was found

	{
	int iPos = FindItem(Item);
	if (iPos == -1)
		return false;

	m_iCursor = iPos;
	return true;
	}

void CItemListManipulator::SetDamagedAtCursor (bool bDamaged, int iCount)

//	SetDamagedAtCursor
//
//	Sets the damage flag

	{
	ASSERT(m_iCursor != -1);

	//	Get the item at the cursor. Abort if there is nothing to do

	CItem &OldItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	if (OldItem.IsDamaged() == bDamaged)
		return;

	//	Generate a new item

	CItem NewItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	if (iCount != -1)
		NewItem.SetCount(Min(iCount, OldItem.GetCount()));
	NewItem.SetDamaged(bDamaged);

	MoveItemTo(NewItem, OldItem);
	}

void CItemListManipulator::SetDataAtCursor (const CString &sAttrib, const CString &sData, int iCount)

//	SetDataAtCursor
//
//	Sets data

	{
	ASSERT(m_iCursor != -1);
	CItem &OldItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	CItem NewItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	if (iCount != -1)
		NewItem.SetCount(Min(iCount, OldItem.GetCount()));
	NewItem.SetData(sAttrib, sData);

	MoveItemTo(NewItem, OldItem);
	}

void CItemListManipulator::SetDisruptedAtCursor (DWORD dwDuration, int iCount)

//	SetDisruptedAtCursor
//
//	Sets the disrupted time

	{
	ASSERT(m_iCursor != -1);

	CItem &OldItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));

	//	Generate a new item

	CItem NewItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	if (iCount != -1)
		NewItem.SetCount(Min(iCount, OldItem.GetCount()));
	NewItem.SetDisrupted(dwDuration);

	MoveItemTo(NewItem, OldItem);
	}

void CItemListManipulator::SetEnhancedAtCursor (bool bEnhanced)

//	SetEnhancedAtCursor
//
//	Sets the enhanced flag

	{
	ASSERT(m_iCursor != -1);

	//	Get the item at the cursor. Abort if there is nothing to do

	CItem &OldItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	if (OldItem.IsEnhanced() == bEnhanced)
		return;

	//	Generate a new item

	CItem NewItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	NewItem.SetCount(1);
	NewItem.SetEnhanced(bEnhanced);

	//	If we've got a single item, and the new item does not exist, then
	//	we can do it in place

	if (OldItem.GetCount() == 1 && FindItem(NewItem) == -1)
		OldItem.SetEnhanced(bEnhanced);

	//	Otherwise, we delete and add

	else
		{
		DeleteAtCursor(1);
		AddItem(NewItem);
		}
	}

void CItemListManipulator::SetFilter (const CItemCriteria &Filter)

//	SetFilter
//
//	Set filter for item list

	{
	m_Filter = Filter;
	m_bUseFilter = true;
	m_iCursor = -1;
	GenerateViewMap();
	}

void CItemListManipulator::SetInstalledAtCursor (int iInstalled)

//	SetInstalledAtCursor
//
//	Installs the selected item

	{
	ASSERT(m_iCursor != -1);
	CItem &OldItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	CItem NewItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	NewItem.SetCount(1);
	NewItem.SetInstalled(iInstalled);

	MoveItemTo(NewItem, OldItem);
	}

bool CItemListManipulator::SetPropertyAtCursor (CSpaceObject *pSource, const CString &sName, ICCItem *pValue, int iCount, CString *retsError)

//	SetPropertyAtCursor
//
//	Sets the property

	{
	ASSERT(m_iCursor != -1);
	CItem &OldItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	CItem NewItem = m_ItemList.GetItem(m_ViewMap.GetElement(m_iCursor));
	if (iCount != -1)
		NewItem.SetCount(Max(0, Min(iCount, OldItem.GetCount())));

	CItemCtx ItemCtx(&NewItem, pSource);
	if (!NewItem.SetProperty(ItemCtx, sName, pValue, retsError))
		return false;

	MoveItemTo(NewItem, OldItem);

	return true;
	}

void CItemListManipulator::SyncCursor (void)

//	SyncCursor
//
//	Make sure the cursor is in bounds.

	{
	if (m_iCursor != -1
			&& m_iCursor >= m_ItemList.GetCount())
		m_iCursor = m_ItemList.GetCount() - 1;
	}

void CItemListManipulator::TransferAtCursor (int iCount, CItemList &DestList)

//	TransferAtCursor
//
//	Transfers copies from the cursor to the given destination list

	{
	ASSERT(m_iCursor != -1);

	CItemListManipulator DestListManipulator(DestList);
	CItem ThisItem = GetItemAtCursor();
	ThisItem.SetCount(iCount);

	DestListManipulator.AddItem(ThisItem);
	DeleteAtCursor(iCount);
	}
