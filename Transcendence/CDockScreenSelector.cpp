//	CDockScreenSelector.cpp
//
//	CDockScreenSelector class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

void CDockScreenSelector::OnDeleteCurrentItem (int iCount)

//	OnDeleteCurrentItem
//
//	Delete item.

	{
	}

const CItem &CDockScreenSelector::OnGetCurrentItem (void) const

//	OnGetCurrentItem
//
//	Returns the current item

	{
	return m_pControl->GetItemAtCursor();
	}

ICCItem *CDockScreenSelector::OnGetCurrentListEntry (void) const

//	OnGetCurrentListEntry
//
//	Returns the current entry

	{
	return m_pControl->GetEntryAtCursor();
	}

bool CDockScreenSelector::OnGetDefaultBackground (SBackgroundDesc *retDesc)

//	OnGetDefaultBackground
//
//	Returns the object that we should use as a background.

	{
	CSpaceObject *pSource = m_pControl->GetSource();
	if (pSource == NULL)
		return false;

	retDesc->iType = backgroundObj;
	retDesc->pObj = pSource;

	return true;
	}

IDockScreenDisplay::EResults CDockScreenSelector::OnHandleAction (DWORD dwTag, DWORD dwData)

//	OnHandleAction
//
//	Handle an action from our control

	{
	if (dwTag == m_dwID)
		{
		if (!m_bNoListNavigation)
			{
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			m_pControl->SetCursor(dwData);
			return resultShowPane;
			}
		else
			return resultHandled;
		}
	else
		return resultNone;
	}

IDockScreenDisplay::EResults CDockScreenSelector::OnHandleKeyDown (int iVirtKey)

//	OnHandleKeyDown
//
//	Handle key down

	{
	switch (iVirtKey)
		{
		case VK_UP:
			if (!m_bNoListNavigation
					&& m_pControl->MoveCursor(CGSelectorArea::moveUp))
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				ShowItem();
				return resultShowPane;
				}
			else
				return resultHandled;

		case VK_LEFT:
			if (!m_bNoListNavigation
					&& m_pControl->MoveCursor(CGSelectorArea::moveLeft))
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				ShowItem();
				return resultShowPane;
				}
			else
				return resultHandled;

		case VK_DOWN:
			if (!m_bNoListNavigation
					&& m_pControl->MoveCursor(CGSelectorArea::moveDown))
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				ShowItem();
				return resultShowPane;
				}
			else
				return resultHandled;

		case VK_RIGHT:
			if (!m_bNoListNavigation
					&& m_pControl->MoveCursor(CGSelectorArea::moveRight))
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				ShowItem();
				return resultShowPane;
				}
			else
				return resultHandled;

		default:
			return resultNone;
		}
	}

ALERROR CDockScreenSelector::OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError)

//	OnInit
//
//	Initialize

	{
	m_dwID = Ctx.dwFirstID;

	//	Figure out where to get the data from: either the station
	//	or the player's ship.

	CSpaceObject *pListSource = EvalListSource(Options.sDataFrom, retsError);
	if (pListSource == NULL)
		return ERR_FAIL;

	//	Create the selector control

	m_pControl = new CGSelectorArea(*Ctx.pVI);
	if (m_pControl == NULL)
		{
		*retsError = CONSTLIT("Out of memory.");
		return ERR_MEMORY;
		}

	//	Create. NOTE: Once we add it to the screen, it takes ownership of it. 
	//	We do not have to free it.

	Ctx.pScreen->AddArea(m_pControl, Ctx.rcRect, m_dwID);

	//	Initialize the control

	m_pControl->SetRegions(pListSource, m_iConfig);

	//	Position the cursor

	SelectNextItem();

	//	Give the screen a chance to start at a different item (other
	//	than the first)

	if (!Options.sInitialItemCode.IsBlank())
		{
		bool bMore = IsCurrentItemValid();
		while (bMore)
			{
			bool bResult;
			if (!EvalBool(Options.sInitialItemCode, &bResult, retsError))
				return ERR_FAIL;

			if (bResult)
				break;

			bMore = SelectNextItem();
			}
		}

	return NOERROR;
	}

bool CDockScreenSelector::OnIsCurrentItemValid (void) const

//	OnIsCurrentItemValid
//
//	Returns TRUE if we've selected a valid item.

	{
	return m_pControl->IsCursorValid();
	}

IDockScreenDisplay::EResults CDockScreenSelector::OnResetList (CSpaceObject *pLocation)

//	OnResetList
//
//	Reset the list

	{
	if (m_pControl->GetSource() == pLocation)
		{
		m_pControl->ResetCursor();
		m_pControl->MoveCursor(CGSelectorArea::moveNext);
		ShowItem();
		return resultShowPane;
		}
	else
		return resultNone;
	}

IDockScreenDisplay::EResults CDockScreenSelector::OnSetListCursor (int iCursor)

//	OnSetListCursor
//
//	Set the cursor at the given entry.

	{
	m_pControl->SetCursor(iCursor);
	ShowItem();
	return resultShowPane;
	}

IDockScreenDisplay::EResults CDockScreenSelector::OnSetListFilter (const CItemCriteria &Filter)

//	OnSetListFilter
//
//	Sets the filter

	{
	return resultNone;
	}

bool CDockScreenSelector::OnSelectNextItem (void)

//	OnSelectNextItem
//
//	Select the next item

	{
	return m_pControl->MoveCursor(CGSelectorArea::moveNext);
	}

bool CDockScreenSelector::OnSelectPrevItem (void)

//	OnSelectPrevItem
//
//	Select the previous item

	{
	return m_pControl->MoveCursor(CGSelectorArea::movePrev);
	}

void CDockScreenSelector::OnShowItem (void)

//	OnShowItem
//
//	Item is shown

	{
	m_pControl->SyncCursor();

	//	If we've got an installed armor segment selected, then highlight
	//	it on the armor display

	if (m_pControl->IsCursorValid())
		{
		const CItem &Item = m_pControl->GetItemAtCursor();
		if (Item.IsInstalled() && Item.GetType()->IsArmor())
			{
			int iSeg = Item.GetInstalled();
			g_pTrans->SelectArmor(iSeg);
			}
		else
			g_pTrans->SelectArmor(-1);
		}
	else
		g_pTrans->SelectArmor(-1);
	}

void CDockScreenSelector::OnShowPane (bool bNoListNavigation)

//	OnShowPane
//
//	Pane is shown

	{
	//	Update the item list

	ShowItem();

	//	If this is set, don't allow the list selection to change

	m_bNoListNavigation = bNoListNavigation;
	}
