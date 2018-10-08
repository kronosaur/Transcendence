//	CDockScreenDetailsPane.cpp
//
//	CDockScreenDetailsPane class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

ICCItem *CDockScreenDetailsPane::OnGetCurrentListEntry (void) const

//	OnGetCurrentListEntry
//
//	Returns the current list entry.

	{
	return m_pControl->GetData();
	}

ALERROR CDockScreenDetailsPane::OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError)

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

	m_pControl = new CGDetailsArea(g_pHI->GetVisuals());
	if (m_pControl == NULL)
		{
		*retsError = CONSTLIT("Out of memory.");
		return ERR_MEMORY;
		}

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

	m_pControl->SetData(CC, pResult);
	CCCtx.Discard(pResult);

	return NOERROR;

	DEBUG_CATCH
	}

void CDockScreenDetailsPane::OnShowItem (void)

//	OnShowItem
//
//	Show the current item

	{
	}

void CDockScreenDetailsPane::OnShowPane (bool bNoListNavigation)

//	OnShowPane
//
//	Handle case where the pane is shown

	{
	ShowItem();
	}
