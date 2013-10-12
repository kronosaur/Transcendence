//	C3DConversion.cpp
//
//	C3DConversion class

#include "PreComp.h"

const int ICON_WIDTH =						96;
const int ICON_HEIGHT =						96;

const CItem g_DummyItem;
CItemList g_DummyItemList;
CItemListManipulator g_DummyItemListManipulator(g_DummyItemList);

//	CItemListWrapper -----------------------------------------------------------

CItemListWrapper::CItemListWrapper (CSpaceObject *pSource) :
		m_pSource(pSource),
		m_ItemList(pSource->GetItemList())

//	CItemListWrapper constructor

	{
	}

CItemListWrapper::CItemListWrapper (CItemList &ItemList) :
		m_pSource(NULL),
		m_ItemList(ItemList)

//	CItemListWrapper constructor

	{
	}

//	CListWrapper ---------------------------------------------------------------

const int TITLE_INDEX =			0;
const int ICON_INDEX =			1;
const int DESC_INDEX =			2;

const int IMAGE_UNID_INDEX =	0;
const int IMAGE_X_INDEX =		1;
const int IMAGE_Y_INDEX =		2;
const int IMAGE_ELEMENTS =		3;

CListWrapper::CListWrapper (CCodeChain *pCC, ICCItem *pList) :
		m_pCC(pCC),
		m_pList(pList->Reference()),
		m_iCursor(-1)

//	CListWrapper constructor

	{
	}

CString CListWrapper::GetDescAtCursor (void)

//	GetDescAtCursor
//
//	Returns the description of the list element

	{
	if (IsCursorValid())
		{
		ICCItem *pItem = m_pList->GetElement(m_iCursor);
		if (DESC_INDEX < pItem->GetCount())
			{
			ICCItem *pDesc = pItem->GetElement(DESC_INDEX);
			if (pDesc->IsNil())
				return NULL_STR;

			return pDesc->GetStringValue();
			}
		}

	return NULL_STR;
	}

ICCItem *CListWrapper::GetEntryAtCursor (CCodeChain &CC)

//	GetEntryAtCursor
//
//	Returns the entry at the cursor

	{
	if (!IsCursorValid())
		return CC.CreateNil();

	ICCItem *pItem = m_pList->GetElement(m_iCursor);
	return pItem->Reference();
	}

CString CListWrapper::GetTitleAtCursor (void)

//	GetTitleAtCursor
//
//	Returns the title of the list element

	{
	if (IsCursorValid())
		{
		ICCItem *pItem = m_pList->GetElement(m_iCursor);
		if (TITLE_INDEX < pItem->GetCount())
			{
			ICCItem *pTitle = pItem->GetElement(TITLE_INDEX);
			if (pTitle->IsNil())
				return NULL_STR;

			return pTitle->GetStringValue();
			}
		}

	return NULL_STR;
	}

bool CListWrapper::MoveCursorBack (void)

//	MoveCursorBack
//
//	Move cursor back

	{
	if (m_iCursor <= 0)
		return false;
	else
		{
		m_iCursor--;
		return true;
		}
	}

bool CListWrapper::MoveCursorForward (void)

//	MoveCursorForward
//
//	Moves the cursor forward

	{
	if (m_iCursor + 1 == GetCount())
		return false;
	else
		{
		m_iCursor++;
		return true;
		}
	}

void CListWrapper::PaintImageAtCursor (CG16bitImage &Dest, int x, int y)

//	PaintImageAtCursor
//
//	Paints the image for the current element

	{
	if (!IsCursorValid())
		return;

	ICCItem *pItem = m_pList->GetElement(m_iCursor);
	if (pItem->GetCount() <= ICON_INDEX)
		return;

	ICCItem *pIcon = pItem->GetElement(ICON_INDEX);
	if (pIcon->GetCount() < IMAGE_ELEMENTS)
		return;

	CG16bitImage *pImage = g_pUniverse->GetLibraryBitmap(pIcon->GetElement(IMAGE_UNID_INDEX)->GetIntegerValue());
	if (pImage == NULL)
		return;

	Dest.ColorTransBlt(pIcon->GetElement(IMAGE_X_INDEX)->GetIntegerValue(),
			pIcon->GetElement(IMAGE_Y_INDEX)->GetIntegerValue(),
			ICON_WIDTH,
			ICON_HEIGHT,
			255,
			*pImage,
			x,
			y);
	}

void CListWrapper::SyncCursor (void)

//	SyncCursor
//
//	Make sure the cursor is inbounds

	{
	if (m_iCursor != -1
			&& m_iCursor >= m_pList->GetCount())
		m_iCursor = m_pList->GetCount() - 1;
	}
