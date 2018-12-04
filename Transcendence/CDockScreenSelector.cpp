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

	retDesc->iType = backgroundObjSchematicImage;
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
    const CDockScreenVisuals &DockScreenVisuals = Ctx.pDockScreen->GetVisuals();

	m_dwID = Ctx.dwFirstID;

	//	Figure out where to get the data from: either the station
	//	or the player's ship.

	CSpaceObject *pListSource = EvalListSource(Options.sDataFrom, retsError);
	if (pListSource == NULL)
		return ERR_FAIL;

    //  Generate options for selector control

    CGSelectorArea::SOptions SelOptions;
    SelOptions.iConfig = m_iConfig;
    CItem::ParseCriteria(Options.sItemCriteria, &SelOptions.ItemCriteria);
    SelOptions.bNoEmptySlots = Options.bNoEmptySlots;

    //  If we're on API < 30, then we always show shields on armor selectors
    //  (for backwards compatibility). Otherwise, we will rely on the item
    //  criteria.

    if (Ctx.pRoot && Ctx.pRoot->GetAPIVersion() < 30)
        SelOptions.bAlwaysShowShields = true;

	//	Create the selector control

	m_pControl = new CGSelectorArea(*Ctx.pVI);
	if (m_pControl == NULL)
		{
		*retsError = CONSTLIT("Out of memory.");
		return ERR_MEMORY;
		}

    m_pControl->SetColor(DockScreenVisuals.GetTitleTextColor());
    m_pControl->SetBackColor(DockScreenVisuals.GetTextBackgroundColor());

	//	Create. NOTE: Once we add it to the screen, it takes ownership of it. 
	//	We do not have to free it.

	Ctx.pScreen->AddArea(m_pControl, Ctx.rcRect, m_dwID);

	//	Initialize the control

	m_pControl->SetRegions(pListSource, SelOptions);

    //  If we have code to set slot names, then execute now.

    if (!Options.sSlotNameCode.IsBlank())
        {
        m_pControl->ResetCursor();
        while (SelectNextItem())
            {
            //  We only care about empty slots

            if (m_pControl->GetItemAtCursor().IsEmpty())
                {
                CString sName;
                if (!EvalString(Options.sSlotNameCode, false, eventNone, &sName))
                    {
                    if (retsError) *retsError = sName;
                    return ERR_FAIL;
                    }

                m_pControl->SetSlotNameAtCursor(sName);
                }
            }

        m_pControl->ResetCursor();
        }

	//	Give the screen a chance to start at a different item (other
	//	than the first)

    if (!Options.sInitialItemCode.IsBlank())
        {
        while (SelectNextItem())
            {
            bool bResult;
            if (!EvalBool(Options.sInitialItemCode, &bResult, retsError))
                return ERR_FAIL;

            if (bResult)
                break;
            }
        }

    //  Otherwise, we start at the first item

    else
        SelectNextItem();

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
		m_pControl->Refresh();
		ShowItem();
		return resultShowPane;
		}
	else
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

IDockScreenDisplay::EResults CDockScreenSelector::OnSetLocation (CSpaceObject *pLocation)

//	OnSetLocation
//
//	The location has changed

	{
	//	LATER: Deal with changing location
	return resultShowPane;
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
			SelectArmor(iSeg);
			}
		else
			SelectArmor(-1);
		}
	else
		SelectArmor(-1);
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
