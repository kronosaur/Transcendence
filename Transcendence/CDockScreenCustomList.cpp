//	CDockScreenCustomList.cpp
//
//	CDockScreenCustomList class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define LIST_TAG					CONSTLIT("List")

#define INITIAL_ITEM_ATTRIB			CONSTLIT("initialItem")
#define ROW_HEIGHT_ATTRIB			CONSTLIT("rowHeight")

ALERROR CDockScreenCustomList::OnInitList (SInitCtx &Ctx, CString *retsError)

//	OnInitList
//
//	Initialize list

	{
	//	Get the list element

	CXMLElement *pListData = Ctx.pDesc->GetContentElementByTag(LIST_TAG);
	if (pListData == NULL)
		return ERR_FAIL;

	//	See if we define a custom row height

	CString sRowHeight;
	if (pListData->FindAttribute(ROW_HEIGHT_ATTRIB, &sRowHeight))
		{
		CString sResult;
		if (!EvalString(sRowHeight, false, eventNone, &sResult))
			{
			*retsError = sResult;
			return ERR_FAIL;
			}

		int cyRow = strToInt(sResult, -1);
		if (cyRow > 0)
			m_pItemListControl->SetRowHeight(cyRow);
		}

	//	Get the list to show

	CCodeChain &CC = g_pUniverse->GetCC();
	ICCItem *pExp = CC.Link(pListData->GetContentText(0), 0, NULL);

	//	Evaluate the function

	CCodeChainCtx CCCtx;
	CCCtx.SetScreen(m_pDockScreen);
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

	CString sInitialItemFunc = pListData->GetAttribute(INITIAL_ITEM_ATTRIB);
	if (!sInitialItemFunc.IsBlank())
		{
		bool bMore = IsCurrentItemValid();
		while (bMore)
			{
			bool bResult;
			if (!EvalBool(sInitialItemFunc, &bResult, retsError))
				return ERR_FAIL;

			if (bResult)
				break;

			bMore = SelectNextItem();
			}
		}

	return NOERROR;
	}
