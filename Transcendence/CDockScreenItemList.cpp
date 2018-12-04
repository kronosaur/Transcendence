//	CDockScreenItemList.cpp
//
//	CDockScreenItemList class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

ALERROR CDockScreenItemList::OnInitList (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError)

//	OnInitList
//
//	Initialize list

	{
	DEBUG_TRY

	//	Figure out where to get the data from: either the station
	//	or the player's ship.

	CSpaceObject *pListSource = EvalListSource(Options.sDataFrom, retsError);
	if (pListSource == NULL)
		return ERR_FAIL;

	//	Set the list control

	m_pItemListControl->SetList(pListSource);

	//	Initialize flags that control what items we will show

	CString sCriteria;
	if (!EvalString(Options.sItemCriteria, false, eventNone, &sCriteria))
		{
		*retsError = sCriteria;
		return ERR_FAIL;
		}

	CItem::ParseCriteria(sCriteria, &m_ItemCriteria);
	m_pItemListControl->SetFilter(m_ItemCriteria);

	//	If we have content, then eval the function (note that this might
	//	re-enter and set the filter)

	if (!Options.sCode.IsBlank())
		{
		if (!EvalString(Options.sCode, true, eventInitDockScreenList, retsError))
			return ERR_FAIL;
		}

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

	//	Done

	return NOERROR;

	DEBUG_CATCH
	}
