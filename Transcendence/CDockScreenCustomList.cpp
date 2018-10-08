//	CDockScreenCustomList.cpp
//
//	CDockScreenCustomList class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

ALERROR CDockScreenCustomList::OnInitList (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError)

//	OnInitList
//
//	Initialize list

	{
	//	See if we define a custom row height

	if (!Options.sRowHeightCode.IsBlank())
		{
		CString sResult;
		if (!EvalString(Options.sRowHeightCode, false, eventNone, &sResult))
			{
			*retsError = sResult;
			return ERR_FAIL;
			}

		int cyRow = strToInt(sResult, -1);
		if (cyRow > 0)
			m_pItemListControl->SetRowHeight(cyRow);
		}

	m_pItemListControl->SetIconHeight(Options.cyIcon);
	m_pItemListControl->SetIconWidth(Options.cxIcon);
	m_pItemListControl->SetIconScale(Options.rIconScale);

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

	m_pItemListControl->SetList(CC, pResult);
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
	}
