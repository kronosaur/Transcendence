//	CDockScreenList.cpp
//
//	CDockScreenList class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define FIELD_FILTER_ALL			CONSTLIT("filterAll")
#define FIELD_FILTER_SELECTED		CONSTLIT("filterSelected")

const int PICKER_ROW_HEIGHT	=	96;
const int PICKER_ROW_COUNT =	4;

const DWORD FILTER_BUTTON_FIRST_ID =		0xf0000000;

bool CDockScreenList::FilterHasItems (const CItemCriteria &Filter) const

//	FilterHasItems
//
//	Returns TRUE if the filter selects any item in this object.

	{
	//	Get the source.

	CSpaceObject *pSource = m_pItemListControl->GetSource();
	if (pSource == NULL)
		return false;

	//	Get an item list manipulator

	CItemListManipulator ItemList(pSource->GetItemList());
	ItemList.SetFilter(Filter);
	return ItemList.MoveCursorForward();
	}

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

	pNewFilter->bAllFilter = strEquals(sID, FIELD_FILTER_ALL);

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
					if (!SelectTab(dwData, iFilter))
						return resultHandled;

					//	If ctrl-key is down, the make this the default tab

					if (uiIsControlDown())
						{
						CString sID = m_Filters[iFilter].sID;
						SetDefaultTab(sID);

						//	Remember the setting

						m_DockScreen.GetGameSession().GetGameSettings().SetString(CGameSettings::defaultBuyTab, sID);
						}

					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
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
	DWORD dwTab;

	switch (iVirtKey)
		{
		case VK_UP:
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

		case VK_LEFT:
			if (m_bNoListNavigation)
				return resultHandled;

			else if (m_pItemListControl->GetPrevTab(&dwTab))
				{
				if (!SelectTab(dwTab))
					return resultHandled;

				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				return resultShowPane;
				}
			else
				return OnHandleKeyDown(VK_UP);

		case VK_RIGHT:
			if (m_bNoListNavigation)
				return resultHandled;

			else if (m_pItemListControl->GetNextTab(&dwTab))
				{
				if (!SelectTab(dwTab))
					return resultHandled;

				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				return resultShowPane;
				}
			else
				return OnHandleKeyDown(VK_DOWN);

		case VK_TAB:
			if (!m_bNoListNavigation)
				{
				DWORD dwNextTab;
				if (!m_pItemListControl->GetNextTab(&dwNextTab))
					return resultHandled;

				if (!SelectTab(dwNextTab))
					return resultHandled;

				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
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
	DEBUG_TRY

	int i;
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
	m_pItemListControl->SetNoArmorSpeedDisplay(Options.bNoArmorSpeedDisplay);
	m_pItemListControl->SetDisplayAsKnown(Options.bActualItems);

	//	Create. NOTE: Once we add it to the screen, it takes ownership of it. 
	//	We do not have to free it.

	Ctx.pScreen->AddArea(m_pItemListControl, rcList, m_dwID);

	//	Let our subclass initialize

	if (error = OnInitList(Ctx, Options, retsError))
		return error;

	//	If we have tabs, select the first one

	if (m_Filters.GetCount() > 0)
		{
		//	If we have a default tab, then move it to the beginning.

		const CString &sDefaultTab = m_DockScreen.GetGameSession().GetGameSettings().GetString(CGameSettings::defaultBuyTab);
		if (!sDefaultTab.IsBlank())
			SetDefaultTab(sDefaultTab);

		//	First see if any of our filters should be disabled and figure out
		//	our first non-disabled filter (so we can use it as a default).

		int iAllFilter = -1;
		int iFilterCount = 0;
		int iFirstValidFilter = -1;
		for (i = 0; i < m_Filters.GetCount(); i++)
			{
			m_Filters[i].bDisabled = !FilterHasItems(m_Filters[i].Filter);
			m_pItemListControl->EnableTab(m_Filters[i].dwID, !m_Filters[i].bDisabled);

			if (iFirstValidFilter == -1 && !m_Filters[i].bDisabled)
				iFirstValidFilter = i;

			if (!m_Filters[i].bDisabled)
				{
				if (m_Filters[i].bAllFilter)
					iAllFilter = i;
				else
					iFilterCount++;
				}
			}

		//	If there's only one tab other than the all filter, then we disable that
		//	tab too.

		if (iFilterCount == 1 && iAllFilter != -1)
			{
			for (i = 0; i < m_Filters.GetCount(); i++)
				{
				if (!m_Filters[i].bAllFilter)
					{
					m_Filters[i].bDisabled = true;
					m_pItemListControl->EnableTab(m_Filters[i].dwID, false);
					}
				}

			iFirstValidFilter = iAllFilter;
			}

		//	See if we've saved a specific filter.

		CString sID = GetScreenStack().GetDisplayData(FIELD_FILTER_SELECTED);
		int iFilter;
		if (sID.IsBlank() || !FindFilter(strToInt(sID, 0), &iFilter) || m_Filters[iFilter].bDisabled)
			iFilter = iFirstValidFilter;

		//	If we still don't have a valid filter, default to the first filter

		if (iFilter == -1)
			iFilter = 0;

		//	Select the filter

		m_pItemListControl->SelectTab(m_Filters[iFilter].dwID);
		m_pItemListControl->SetFilter(m_Filters[iFilter].Filter);
		m_pItemListControl->MoveCursorForward();
		}

	return NOERROR;

	DEBUG_CATCH
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
		//	HACK: We need to define these variables because any filters will
		//	expect them. Sometimes we get called here in response to some other
		//	event. E.g., someone might add an item in <OnUpdate> and we end up
		//	getting called here. So we need to always set up the same environment
		//	that the filter code expects.

		CCodeChainCtx Ctx;
		Ctx.SetScreen(&m_DockScreen);
		Ctx.SaveAndDefineSourceVar(m_pLocation);
		Ctx.SaveAndDefineDataVar(m_pData);

		m_pItemListControl->ResetCursor();
		m_pItemListControl->MoveCursorForward();
		ShowItem();
		return resultShowPane;
		}
	else
		return resultNone;
	}

bool CDockScreenList::OnSelectItem (const CItem &Item)

//	OnSelectItem
//
//	Selects the given item.

	{
	IListData *pList = GetListData();
	if (pList == NULL)
		return false;

	int iCursor;
	if (!pList->FindItem(Item, &iCursor))
		return false;

	SetListCursor(iCursor);
	return true;
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

bool CDockScreenList::SelectTab (DWORD dwID, int iFilter)

//	SelectTab
//
//	Selects the given tab by ID.

	{
	//	Find the filter if we don't know it.

	if (iFilter == -1)
		{
		if (!FindFilter(dwID, &iFilter))
			return false;
		}

	//	Select the tab and filter

	m_pItemListControl->SelectTab(dwID);
	m_pItemListControl->SetFilter(m_Filters[iFilter].Filter);
	m_pItemListControl->MoveCursorForward();
	GetScreenStack().SetDisplayData(FIELD_FILTER_SELECTED, strFromInt(m_Filters[iFilter].dwID));

	ShowItem();

	return true;
	}

bool CDockScreenList::SetDefaultTab (const CString &sID)

//	SetDefaultTab
//
//	Moves the given tab (by ID) to the front (so that it is selected by default).
//	If the tab is not found, we return FALSE.

	{
	int iTab;
	if (!FindFilter(sID, &iTab))
		return false;

	//	If we're already at the front, then we're done

	if (iTab == 0)
		return true;

	//	Re-order our list

	SFilter Tab = m_Filters[iTab];
	m_Filters.Delete(iTab);
	m_Filters.Insert(Tab, 0);

	//	Tell our control to move the tab to the front

	m_pItemListControl->MoveTabToFront(Tab.dwID);

	//	Done

	return true;
	}
