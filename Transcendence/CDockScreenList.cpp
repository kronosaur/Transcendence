//	CDockScreenList.cpp
//
//	CDockScreenList class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define FIELD_FILTER_SELECTED		CONSTLIT("filterSelected")

const int PICKER_ROW_HEIGHT	=	96;
const int PICKER_ROW_COUNT =	4;

const DWORD FILTER_BUTTON_FIRST_ID =		0xf0000000;

bool CDockScreenList::FindFilter (DWORD dwID, int *retiIndex) const

//	FindFilter
//
//	Find the filter by ID

	{
	int i;

	for (i = 0; i < m_Filters.GetCount(); i++)
		if (dwID == m_Filters[i].dwID)
			{
			if (retiIndex)
				*retiIndex = i;

			return true;
			}

	return false;
	}

bool CDockScreenList::FindFilter (const CString &sID, int *retiIndex) const

//	FindFilter
//
//	Finds the filter by ID

	{
	int i;

	for (i = 0; i < m_Filters.GetCount(); i++)
		if (strEquals(sID, m_Filters[i].sID))
			{
			if (retiIndex)
				*retiIndex = i;

			return true;
			}

	return false;
	}

IDockScreenDisplay::EResults CDockScreenList::OnAddListFilter (const CString &sID, const CString &sLabel, const CItemCriteria &Filter)

//	OnAddListFilter
//
//	Adds a user-selectable list filter.

	{
	SFilter *pNewFilter = m_Filters.Insert();
	pNewFilter->sID = sID;
	pNewFilter->dwID = FILTER_BUTTON_FIRST_ID + m_dwNextFilterID++;
	pNewFilter->sLabel = sLabel;
	pNewFilter->Filter = Filter;

	m_pItemListControl->AddTab(pNewFilter->dwID, pNewFilter->sLabel);

	return resultHandled;
	}

void CDockScreenList::OnDeleteCurrentItem (int iCount)

//	OnDeleteCurrentItem
//
//	Delete the current item.

	{
	m_pItemListControl->DeleteAtCursor(iCount);
	ShowItem();
	}

const CItem &CDockScreenList::OnGetCurrentItem (void) const

//	OnGetCurrentItem
//
//	Returns the currently selected item

	{
	return m_pItemListControl->GetItemAtCursor();
	}

ICCItem *CDockScreenList::OnGetCurrentListEntry (void) const

//	OnGetCurrentListEntry
//
//	Returns the current list entry.

	{
	return m_pItemListControl->GetEntryAtCursor();
	}

IDockScreenDisplay::EResults CDockScreenList::OnHandleAction (DWORD dwTag, DWORD dwData)

//	OnHandleAction
//
//	Handle an action

	{
	if (dwTag == m_dwID)
		{
		int iFilter;

		switch (dwData)
			{
			case ITEM_LIST_AREA_PAGE_DOWN_ACTION:
			case ITEM_LIST_AREA_PAGE_UP_ACTION:
				return resultNone;

			default:
				if (m_bNoListNavigation)
					return resultHandled;

				else if (FindFilter(dwData, &iFilter))
					{
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
					m_pItemListControl->SetFilter(m_Filters[iFilter].Filter);
					m_pItemListControl->MoveCursorForward();
					GetScreenStack().SetDisplayData(FIELD_FILTER_SELECTED, strFromInt(m_Filters[iFilter].dwID));

					ShowItem();
					return resultShowPane;
					}
				else
					{
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
					m_pItemListControl->SetCursor(dwData);
					return resultShowPane;
					}
			}
		}
	else
		return resultNone;
	}

IDockScreenDisplay::EResults CDockScreenList::OnHandleKeyDown (int iVirtKey)

//	OnHandleKeyDown
//
//	Handles key down. If we don't handle the given key, we return resultNone.

	{
	switch (iVirtKey)
		{
		case VK_UP:
		case VK_LEFT:
			if (!m_bNoListNavigation)
				{
				bool bOK = SelectPrevItem();
				if (bOK)
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));

				ShowItem();
				return resultShowPane;
				}
			else
				return resultHandled;

		case VK_DOWN:
		case VK_RIGHT:
			if (!m_bNoListNavigation)
				{
				bool bOK = SelectNextItem();
				if (bOK)
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));

				ShowItem();
				return resultShowPane;
				}
			else
				return resultHandled;

		case VK_PRIOR:
			if (!m_bNoListNavigation)
				{
				bool bOK = SelectPrevItem();
				SelectPrevItem();
				SelectPrevItem();
				if (bOK)
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));

				m_pItemListControl->Invalidate();
				return resultShowPane;
				}
			else
				return resultHandled;

		case VK_NEXT:
			if (!m_bNoListNavigation)
				{
				bool bOK = SelectNextItem();
				SelectNextItem();
				SelectNextItem();
				if (bOK)
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));

				m_pItemListControl->Invalidate();
				return resultShowPane;
				}
			else
				return resultHandled;

		default:
			return resultNone;
		}
	}

ALERROR CDockScreenList::OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError)

//	OnInit
//
//	Initialize

	{
	ALERROR error;
    const CDockScreenVisuals &DockScreenVisuals = Ctx.pDockScreen->GetVisuals();

	m_dwID = Ctx.dwFirstID;

	//	Calculate some basic metrics

	RECT rcList = Ctx.rcRect;
	rcList.left += Options.rcControl.left;
	rcList.right = rcList.left + RectWidth(Options.rcControl);
	rcList.top += Options.rcControl.top;
	rcList.bottom = rcList.top + RectHeight(Options.rcControl);

	//	Create the picker control

	m_pItemListControl = new CGItemListArea(g_pHI->GetVisuals());
	if (m_pItemListControl == NULL)
		{
		*retsError = CONSTLIT("Out of memory.");
		return ERR_MEMORY;
		}

	m_pItemListControl->SetUIRes(&g_pTrans->GetUIRes());
	m_pItemListControl->SetFontTable(Ctx.pFontTable);
    m_pItemListControl->SetColor(DockScreenVisuals.GetTitleTextColor());
    m_pItemListControl->SetBackColor(DockScreenVisuals.GetTextBackgroundColor());

	//	Create. NOTE: Once we add it to the screen, it takes ownership of it. 
	//	We do not have to free it.

	Ctx.pScreen->AddArea(m_pItemListControl, rcList, m_dwID);

	//	Let our subclass initialize

	if (error = OnInitList(Ctx, Options, retsError))
		return error;

	//	If we have tabs, select the first one

	if (m_Filters.GetCount() > 0)
		{
		CString sID = GetScreenStack().GetDisplayData(FIELD_FILTER_SELECTED);
		int iFilter;
		if (sID.IsBlank() || !FindFilter(strToInt(sID, 0), &iFilter))
			{
			m_pItemListControl->SetFilter(m_Filters[0].Filter);
			m_pItemListControl->MoveCursorForward();
			}
		else
			{
			m_pItemListControl->SelectTab(m_Filters[iFilter].dwID);
			m_pItemListControl->SetFilter(m_Filters[iFilter].Filter);
			m_pItemListControl->MoveCursorForward();
			}
		}

	return NOERROR;
	}

bool CDockScreenList::OnIsCurrentItemValid (void) const

//	OnIsCurrentItemValid
//
//	Returns TRUE if current item is valid

	{
	return m_pItemListControl->IsCursorValid();
	}

IDockScreenDisplay::EResults CDockScreenList::OnResetList (CSpaceObject *pLocation)

//	OnResetList
//
//	Reset the list

	{
	if (m_pItemListControl->GetSource() == pLocation)
		{
		m_pItemListControl->ResetCursor();
		m_pItemListControl->MoveCursorForward();
		ShowItem();
		return resultShowPane;
		}
	else
		return resultNone;
	}

IDockScreenDisplay::EResults CDockScreenList::OnSetListCursor (int iCursor)

//	OnSetListCursor
//
//	Sets the list cursor

	{
	m_pItemListControl->SetCursor(iCursor);
	ShowItem();
	return resultShowPane;
	}

IDockScreenDisplay::EResults CDockScreenList::OnSetListFilter (const CItemCriteria &Filter)

//	OnSetListFilter
//
//	Sets the list filter

	{
	m_pItemListControl->SetFilter(Filter);
	ShowItem();
	return resultShowPane;
	}

IDockScreenDisplay::EResults CDockScreenList::OnSetLocation (CSpaceObject *pLocation)

//	OnSetLocation
//
//	The location has changed

	{
	//	LATER: Deal with changing location
	return resultShowPane;
	}

bool CDockScreenList::OnSelectNextItem (void)

//	OnSelectNextItem
//
//	Selects the next item

	{
	return m_pItemListControl->MoveCursorForward();
	}

bool CDockScreenList::OnSelectPrevItem (void)

//	OnSelectPrevItem
//
//	Selects the previous item

	{
	return m_pItemListControl->MoveCursorBack();
	}

void CDockScreenList::OnShowItem (void)

//	OnShowItem
//
//	Show the current item

	{
	m_pItemListControl->SyncCursor();

	//	If we've got an installed armor segment selected, then highlight
	//	it on the armor display

	if (m_pItemListControl->IsCursorValid())
		{
		const CItem &Item = m_pItemListControl->GetItemAtCursor();
		if (Item.IsInstalled() && Item.GetType()->IsArmor())
			{
			int iSeg = Item.GetInstalled();
			SelectArmor(iSeg);
			}
		else
			SelectArmor(-1);
		}
	else
		SelectArmor(-1);
	}

void CDockScreenList::OnShowPane (bool bNoListNavigation)

//	OnShowPane
//
//	Handle case where the pane is shown

	{
	//	Update the item list

	ShowItem();

	//	If this is set, don't allow the list selection to change

	m_bNoListNavigation = bNoListNavigation;
	}
