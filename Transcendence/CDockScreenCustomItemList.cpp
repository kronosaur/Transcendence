//	CDockScreenCustomItemList.cpp
//
//	CDockScreenCustomItemList class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

ALERROR CDockScreenCustomItemList::OnInitList (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError)

//	OnInitList
//
//	Initialize list

	{
	int i;

	//	Get the list to show

	CCodeChain &CC = g_pUniverse->GetCC();
	ICCItem *pExp = CC.Link(Options.sCode);

	//	Evaluate the function

	CCodeChainCtx CCCtx;
	CCCtx.SetScreen(Ctx.pDockScreen);
	CCCtx.SaveAndDefineSourceVar(Ctx.pLocation);
	CCCtx.SaveAndDefineDataVar(Ctx.pData);

	ICCItem *pResult = CCCtx.Run(pExp);	//	LATER:Event
	CCCtx.Discard(pExp);

	if (pResult->IsError())
		{
		*retsError = pResult->GetStringValue();
		return ERR_FAIL;
		}

	//	We expect a list of item structures. Load them into an item list

	m_CustomItems.DeleteAll();
	for (i = 0; i < pResult->GetCount(); i++)
		{
		ICCItem *pItem = pResult->GetElement(i);

		CItem NewItem = CreateItemFromList(CC, pItem);
		if (NewItem.GetType() != NULL)
			m_CustomItems.AddItem(NewItem);
		}

	//	Done with result

	CCCtx.Discard(pResult);

	//	Make sure items are sorted

	m_CustomItems.SortItems();

	//	Set the list control

	m_pItemListControl->SetList(m_CustomItems);

	//	Position the cursor on the next relevant item

	SelectNextItem();

	return NOERROR;
	}
