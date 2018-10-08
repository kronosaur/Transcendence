//	CDockScreenCarousel.cpp
//
//	CDockScreenCarousel class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define STYLE_ATTRIB						CONSTLIT("style")

ICCItem *CDockScreenCarousel::OnGetCurrentListEntry (void) const

//	OnGetCurrentListEntry
//
//	Returns the current list entry.

	{
	return m_pControl->GetEntryAtCursor();
	}

IDockScreenDisplay::EResults CDockScreenCarousel::OnHandleAction (DWORD dwTag, DWORD dwData)

//	OnHandleAction
//
//	Handle an action

	{
	if (dwTag == m_dwID)
		{
		switch (dwData)
			{
			case ITEM_LIST_AREA_PAGE_DOWN_ACTION:
			case ITEM_LIST_AREA_PAGE_UP_ACTION:
				return resultNone;

			default:
				if (m_bNoListNavigation)
					return resultHandled;

				else
					{
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
					m_pControl->SetCursor(dwData);
					return resultShowPane;
					}
			}
		}
	else
		return resultNone;
	}

IDockScreenDisplay::EResults CDockScreenCarousel::OnHandleKeyDown (int iVirtKey)

//	OnHandleKeyDown
//
//	Handles key down. If we don't handle the given key, we return resultNone.

	{
	switch (iVirtKey)
		{
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

				m_pControl->Invalidate();
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

				m_pControl->Invalidate();
				return resultShowPane;
				}
			else
				return resultHandled;

		default:
			return resultNone;
		}
	}

ALERROR CDockScreenCarousel::OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError)

//	OnInit
//
//	Initialize

	{
	DEBUG_TRY

    const CDockScreenVisuals &DockScreenVisuals = Ctx.pDockScreen->GetVisuals();

	m_dwID = Ctx.dwFirstID;

	//	Calculate some basic metrics

	RECT rcList = Ctx.rcRect;
	rcList.left += Options.rcControl.left;
	rcList.right = rcList.left + RectWidth(Options.rcControl);
	rcList.top += Options.rcControl.top;
	rcList.bottom = rcList.top + RectHeight(Options.rcControl);

	//	Create the picker control

	m_pControl = new CGCarouselArea(g_pHI->GetVisuals());
	if (m_pControl == NULL)
		{
		*retsError = CONSTLIT("Out of memory.");
		return ERR_MEMORY;
		}

	//	Style

	CString sStyle = Options.pOptions->GetAttribute(STYLE_ATTRIB);
	if (!sStyle.IsBlank())
		{
		if (!m_pControl->SetStyle(sStyle))
			{
			*retsError = strPatternSubst(CONSTLIT("Unknown style: %s"), sStyle);
			return ERR_FAIL;
			}
		}

	//	Other options

    m_pControl->SetColor(DockScreenVisuals.GetTitleTextColor());
    m_pControl->SetBackColor(DockScreenVisuals.GetTextBackgroundColor());

	//	Create. NOTE: Once we add it to the screen, it takes ownership of it. 
	//	We do not have to free it.

	Ctx.pScreen->AddArea(m_pControl, rcList, m_dwID);

	//	Get the list to show

	CCodeChain &CC = g_pUniverse->GetCC();
	ICCItem *pExp = CC.Link(Options.sCode);

	//	Evaluate the function

	CCodeChainCtx CCCtx;
	CCCtx.SetScreen(&m_DockScreen);
	CCCtx.SaveAndDefineSourceVar(m_pLocation);
	CCCtx.SaveAndDefineDataVar(m_pData);

	ICCItem *pResult = CCCtx.Run(pExp);	//	LATER:Event
	CCCtx.Discard(pExp);

	if (pResult->IsError())
		{
		*retsError = pResult->GetStringValue();
		return ERR_FAIL;
		}

	//	Set this expression as the list

	m_pControl->SetList(CC, pResult);
	CCCtx.Discard(pResult);

	//	Position the cursor on the next relevant item

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

	DEBUG_CATCH
	}

bool CDockScreenCarousel::OnIsCurrentItemValid (void) const

//	OnIsCurrentItemValid
//
//	Returns TRUE if current item is valid

	{
	return m_pControl->IsCursorValid();
	}

IDockScreenDisplay::EResults CDockScreenCarousel::OnResetList (CSpaceObject *pLocation)

//	OnResetList
//
//	Reset the list

	{
	if (m_pControl->GetSource() == pLocation)
		{
		m_pControl->ResetCursor();
		m_pControl->MoveCursorForward();
		ShowItem();
		return resultShowPane;
		}
	else
		return resultNone;
	}

IDockScreenDisplay::EResults CDockScreenCarousel::OnSetListCursor (int iCursor)

//	OnSetListCursor
//
//	Sets the list cursor

	{
	m_pControl->SetCursor(iCursor);
	ShowItem();
	return resultShowPane;
	}

IDockScreenDisplay::EResults CDockScreenCarousel::OnSetLocation (CSpaceObject *pLocation)

//	OnSetLocation
//
//	The location has changed

	{
	//	LATER: Deal with changing location
	return resultShowPane;
	}

bool CDockScreenCarousel::OnSelectNextItem (void)

//	OnSelectNextItem
//
//	Selects the next item

	{
	return m_pControl->MoveCursorForward();
	}

bool CDockScreenCarousel::OnSelectPrevItem (void)

//	OnSelectPrevItem
//
//	Selects the previous item

	{
	return m_pControl->MoveCursorBack();
	}

void CDockScreenCarousel::OnShowItem (void)

//	OnShowItem
//
//	Show the current item

	{
	m_pControl->SyncCursor();
	}

void CDockScreenCarousel::OnShowPane (bool bNoListNavigation)

//	OnShowPane
//
//	Handle case where the pane is shown

	{
	//	Update the item list

	ShowItem();

	//	If this is set, don't allow the list selection to change

	m_bNoListNavigation = bNoListNavigation;
	}
