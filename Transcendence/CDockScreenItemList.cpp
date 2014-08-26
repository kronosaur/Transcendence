//	CDockScreenItemList.cpp
//
//	CDockScreenItemList class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define LIST_OPTIONS_TAG			CONSTLIT("ListOptions")

#define DATA_FROM_ATTRIB			CONSTLIT("dataFrom")
#define INITIAL_ITEM_ATTRIB			CONSTLIT("initialItem")
#define LIST_ATTRIB					CONSTLIT("list")

#define DATA_FROM_PLAYER			CONSTLIT("player")
#define DATA_FROM_SOURCE			CONSTLIT("source")
#define DATA_FROM_STATION			CONSTLIT("station")

CSpaceObject *CDockScreenItemList::EvalListSource (const CString &sString, CString *retsError)

//	EvalListSource
//
//	Returns the object from which we should display items

	{
	char *pPos = sString.GetPointer();

	//	See if we need to evaluate

	if (*pPos == '=')
		{
		CCodeChainCtx Ctx;
		Ctx.SetScreen(this);
		Ctx.SaveAndDefineSourceVar(m_pLocation);
		Ctx.SaveAndDefineDataVar(m_pData);

		ICCItem *pExp = Ctx.Link(sString, 1, NULL);

		ICCItem *pResult = Ctx.Run(pExp);	//	LATER:Event
		Ctx.Discard(pExp);

		if (pResult->IsError())
			{
			*retsError = pResult->GetStringValue();
			Ctx.Discard(pResult);
			return NULL;
			}

		//	Convert to an object pointer

		CSpaceObject *pSource;
		if (strEquals(pResult->GetStringValue(), DATA_FROM_PLAYER))
			pSource = m_pPlayer->GetShip();
		else if (strEquals(pResult->GetStringValue(), DATA_FROM_STATION)
				|| strEquals(pResult->GetStringValue(), DATA_FROM_SOURCE))
			pSource = m_pLocation;
		else
			pSource = Ctx.AsSpaceObject(pResult);

		Ctx.Discard(pResult);
		return pSource;
		}

	//	Otherwise, compare to constants

	else if (strEquals(sString, DATA_FROM_PLAYER))
		return m_pPlayer->GetShip();
	else
		return m_pLocation;
	}

ALERROR CDockScreenItemList::OnInitList (SInitCtx &Ctx, CString *retsError)

//	OnInitList
//
//	Initialize list

	{
	CSpaceObject *pListSource;

	//	Get the list options element

	CXMLElement *pOptions = Ctx.pDesc->GetContentElementByTag(LIST_OPTIONS_TAG);
	if (pOptions == NULL)
		{
		*retsError = CONSTLIT("<ListOptions> expected.");
		return ERR_FAIL;
		}

	//	Figure out where to get the data from: either the station
	//	or the player's ship.

	pListSource = EvalListSource(pOptions->GetAttribute(DATA_FROM_ATTRIB), retsError);
	if (pListSource == NULL)
		return ERR_FAIL;

	//	Set the list control

	m_pItemListControl->SetList(pListSource);

	//	Initialize flags that control what items we will show

	CString sCriteria;
	if (!EvalString(pOptions->GetAttribute(LIST_ATTRIB), false, eventNone, &sCriteria))
		{
		*retsError = sCriteria;
		return ERR_FAIL;
		}

	CItem::ParseCriteria(sCriteria, &m_ItemCriteria);
	m_pItemListControl->SetFilter(m_ItemCriteria);

	//	If we have content, then eval the function (note that this might
	//	re-enter and set the filter)

	CString sCode = pOptions->GetContentText(0);
	if (!sCode.IsBlank())
		{
		if (!EvalString(sCode, true, eventInitDockScreenList, retsError))
			return ERR_FAIL;
		}

	//	Position the cursor on the next relevant item

	SelectNextItem();

	//	Give the screen a chance to start at a different item (other
	//	than the first)

	CString sInitialItemFunc = pOptions->GetAttribute(INITIAL_ITEM_ATTRIB);
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

	//	Done

	return NOERROR;
	}