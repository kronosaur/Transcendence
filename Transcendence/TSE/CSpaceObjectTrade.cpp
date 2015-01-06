//	CSpaceObjectTrade.cpp
//
//	CSpaceObject class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

const int DAYS_TO_REFRESH_INVENTORY =	5;

void CSpaceObject::AddBuyOrder (CItemType *pType, const CString &sCriteria, int iPriceAdj)

//	AddBuyOrder
//
//	Adds an override buy order

	{
	CTradingDesc *pTrade = AllocTradeDescOverride();
	pTrade->AddBuyOrder(pType, sCriteria, iPriceAdj);
	}

void CSpaceObject::AddSellOrder (CItemType *pType, const CString &sCriteria, int iPriceAdj)

//	AddSellOrder
//
//	Adds an override sell order

	{
	CTradingDesc *pTrade = AllocTradeDescOverride();
	pTrade->AddSellOrder(pType, sCriteria, iPriceAdj);
	}

bool CSpaceObject::GetArmorInstallPrice (const CItem &Item, DWORD dwFlags, int *retiPrice)

//	GetArmorInstallPrice
//
//	Returns the price to install the given armor

	{
	//	See if we have an override

	CTradingDesc *pTradeOverride = GetTradeDescOverride();
	if (pTradeOverride && pTradeOverride->GetArmorInstallPrice(this, Item, dwFlags, retiPrice))
		return true;

	//	Otherwise, ask our design type

	CDesignType *pType = GetType();
	if (pType == NULL)
		return false;

	CTradingDesc *pTrade = pType->GetTradingDesc();
	if (pTrade && pTrade->GetArmorInstallPrice(this, Item, dwFlags, retiPrice))
		return true;

	//	For compatibility, any ship prior to version 23 has a default.
	//	[For API Version 23 and above, ships must have a <Trade> descriptor.]

	if (pType->GetAPIVersion() < 23
			&& pType->GetType() == designShipClass)
		{
		if (retiPrice)
			*retiPrice = CTradingDesc::CalcPriceForService(serviceReplaceArmor, this, Item, 1, dwFlags);

		return true;
		}

	//	Otherwise, we do not repair

	return false;
	}

bool CSpaceObject::GetArmorRepairPrice (const CItem &Item, int iHPToRepair, DWORD dwFlags, int *retiPrice)

//	GetArmorRepairPrice
//
//	Returns the price to repair the given number of HP for the given armor item.

	{
	//	See if we have an override

	CTradingDesc *pTradeOverride = GetTradeDescOverride();
	if (pTradeOverride && pTradeOverride->GetArmorRepairPrice(this, Item, iHPToRepair, dwFlags, retiPrice))
		return true;

	//	Otherwise, ask our design type

	CDesignType *pType = GetType();
	if (pType == NULL)
		return false;

	CTradingDesc *pTrade = pType->GetTradingDesc();
	if (pTrade && pTrade->GetArmorRepairPrice(this, Item, iHPToRepair, dwFlags, retiPrice))
		return true;

	//	For compatibility, any ship prior to version 23 has a default.
	//	[For API Version 23 and above, ships must have a <Trade> descriptor.]

	if (pType->GetAPIVersion() < 23
			&& pType->GetType() == designShipClass)
		{
		if (retiPrice)
			*retiPrice = CTradingDesc::CalcPriceForService(serviceRepairArmor, this, Item, iHPToRepair, dwFlags);

		return true;
		}

	//	Otherwise, we do not repair

	return false;
	}

int CSpaceObject::GetBuyPrice (const CItem &Item, DWORD dwFlags, int *retiMaxCount)

//	GetBuyPrice
//
//	Returns the price at which the station will buy the given
//	item. Also returns the max number of items that the station
//	will buy at that price.
//
//	Returns -1 if the station will not buy the item.

	{
	int iPrice;

	//	First see if we have an override

	CTradingDesc *pTradeOverride = GetTradeDescOverride();
	if (pTradeOverride && pTradeOverride->Buys(this, Item, dwFlags, &iPrice, retiMaxCount))
		return iPrice;

	//	See if our type has a price

	CDesignType *pType = GetType();
	if (pType == NULL)
		return -1;

	CTradingDesc *pTrade = pType->GetTradingDesc();
	if (pTrade && pTrade->Buys(this, Item, dwFlags, &iPrice, retiMaxCount))
		return iPrice;

	//	Otherwise, we will not buy the item

	return -1;
	}

CEconomyType *CSpaceObject::GetDefaultEconomy (void)

//	GetDefaultEconomy
//
//	Returns the default economy

	{
	CTradingDesc *pTradeOverride = GetTradeDescOverride();
	if (pTradeOverride)
		return pTradeOverride->GetEconomyType();

	CDesignType *pType = GetType();
	CTradingDesc *pTrade = (pType ? pType->GetTradingDesc() : NULL);
	if (pTrade)
		return pTrade->GetEconomyType();

	return CEconomyType::AsType(g_pUniverse->FindDesignType(DEFAULT_ECONOMY_UNID));
	}

DWORD CSpaceObject::GetDefaultEconomyUNID (void)

//	GetDefaultEconomyUNID
//
//	Returns the default economy
	
	{
	CTradingDesc *pTradeOverride = GetTradeDescOverride();
	if (pTradeOverride)
		return pTradeOverride->GetEconomyType()->GetUNID();

	CDesignType *pType = GetType();
	CTradingDesc *pTrade = (pType ? pType->GetTradingDesc() : NULL);
	if (pTrade)
		return pTrade->GetEconomyType()->GetUNID();

	return DEFAULT_ECONOMY_UNID;
	}

bool CSpaceObject::GetDeviceInstallPrice (const CItem &Item, DWORD dwFlags, int *retiPrice)

//	GetDeviceInstallPrice
//
//	Returns the price to install the given device

	{
	//	See if we have an override

	CTradingDesc *pTradeOverride = GetTradeDescOverride();
	if (pTradeOverride && pTradeOverride->GetDeviceInstallPrice(this, Item, dwFlags, retiPrice))
		return true;

	//	Otherwise, ask our design type

	CDesignType *pType = GetType();
	if (pType == NULL)
		return false;

	CTradingDesc *pTrade = pType->GetTradingDesc();
	if (pTrade && pTrade->GetDeviceInstallPrice(this, Item, dwFlags, retiPrice))
		return true;

	//	Otherwise, we do not install

	return false;
	}

bool CSpaceObject::GetDeviceRemovePrice (const CItem &Item, DWORD dwFlags, int *retiPrice)

//	GetDeviceRemovePrice
//
//	Returns the price to install the given device

	{
	//	See if we have an override

	CTradingDesc *pTradeOverride = GetTradeDescOverride();
	if (pTradeOverride && pTradeOverride->GetDeviceRemovePrice(this, Item, dwFlags, retiPrice))
		return true;

	//	Otherwise, ask our design type

	CDesignType *pType = GetType();
	if (pType == NULL)
		return false;

	CTradingDesc *pTrade = pType->GetTradingDesc();
	if (pTrade && pTrade->GetDeviceRemovePrice(this, Item, dwFlags, retiPrice))
		return true;

	//	Otherwise, we do not remove

	return false;
	}

bool CSpaceObject::GetRefuelItemAndPrice (CSpaceObject *pObjToRefuel, CItemType **retpItemType, int *retiPrice)

//	GetRefuelItemAndPrice
//
//	Returns the appropriate item to use for refueling (based on the trading
//	directives).

	{
	int i;

	//	See if we have an override

	CTradingDesc *pTradeOverride = GetTradeDescOverride();
	if (pTradeOverride && pTradeOverride->GetRefuelItemAndPrice(this, pObjToRefuel, 0, retpItemType, retiPrice))
		return true;

	//	Otherwise, ask our design type

	CDesignType *pType = GetType();
	if (pType == NULL)
		return false;

	CTradingDesc *pTrade = pType->GetTradingDesc();
	if (pTrade && pTrade->GetRefuelItemAndPrice(this, pObjToRefuel, 0, retpItemType, retiPrice))
		return true;

	//	For compatibility, any ship prior to version 23 has a default.
	//	[For API Version 23 and above, ships must have a <Trade> descriptor.]

	if (pType->GetAPIVersion() < 23
			&& pType->GetType() == designShipClass)
		{
		//	Get the ship

		CShip *pShipToRefuel = pObjToRefuel->AsShip();
		if (pShipToRefuel == NULL)
			return false;

		//	Find the highest-level item that can be used by the ship

		int iBestLevel = 0;
		int iBestPrice = 0;
		CItemType *pBestItem = NULL;

		for (i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
			{
			CItemType *pType = g_pUniverse->GetItemType(i);
			CItem Item(pType, 1);

			if (pShipToRefuel->IsFuelCompatible(Item))
				{
				if (pBestItem == NULL || pType->GetLevel() > iBestPrice)
					{
					//	Compute the price, because if we don't sell it, then we
					//	skip it.
					//
					//	NOTE: Unlike selling, we allow 0 prices because some 
					//	stations give fuel for free.

					int iPrice = CTradingDesc::CalcPriceForService(serviceRefuel, this, Item, 1, 0);
					if (iPrice >= 0)
						{
						pBestItem = pType;
						iBestLevel = pType->GetLevel();
						iBestPrice = iPrice;
						}
					}
				}
			}

		if (pBestItem == NULL)
			return false;

		//	Done

		if (retpItemType)
			*retpItemType = pBestItem;

		if (retiPrice)
			*retiPrice = iBestPrice;

		return true;
		}

	//	Otherwise, we do not refuel

	return false;
	}

int CSpaceObject::GetSellPrice (const CItem &Item, DWORD dwFlags)

//	GetSellPrice
//
//	Returns the price at which the station will sell the given
//	item. Returns 0 if the station cannot or will not sell the
//	item.

	{
	bool bHasTradeDirective = false;
	int iPrice = -1;

	//	See if we have an override price

	CTradingDesc *pTradeOverride = GetTradeDescOverride();
	if (pTradeOverride)
		{
		pTradeOverride->Sells(this, Item, dwFlags, &iPrice);
		bHasTradeDirective = true;
		}

	//	See if our type has a price

	CDesignType *pType = GetType();
	if (iPrice == -1)
		{
		CTradingDesc *pTrade = (pType ? pType->GetTradingDesc() : NULL);
		if (pTrade)
			{
			pTrade->Sells(this, Item, dwFlags, &iPrice);
			bHasTradeDirective = true;
			}
		}

	//	If we still have no price, then try to figure out a default.

	if (iPrice == -1)
		{
		//	If we have Trade directives and they specify no price, then we can't
		//	sell any.

		if (bHasTradeDirective)
			return 0;

		//	For compatibility, any ship prior to version 23 has a default.
		//	[For API Version 23 and above, ships must have a <Trade> descriptor.]

		else if (pType 
				&& pType->GetAPIVersion() < 23
				&& pType->GetType() == designShipClass)
			iPrice = CTradingDesc::CalcPriceForService(serviceSell, this, Item, 1, dwFlags);

		//	Otherwise, get the price from the item itself

		else
			iPrice = (int)GetDefaultEconomy()->Exchange(Item.GetCurrencyType(), Item.GetTradePrice(this));
		}

	//	If we don't have any of the item, then we don't sell any

	if (!(dwFlags & CTradingDesc::FLAG_NO_INVENTORY_CHECK))
		{
		CItemListManipulator ItemList(GetItemList());
		if (!ItemList.SetCursorAtItem(Item))
			return 0;
		}

	//	Return the price

	return iPrice;
	}

void CSpaceObject::SetTradeDesc (CEconomyType *pCurrency, int iMaxCurrency, int iReplenishCurrency)

//	SetTradeDesc
//
//	Overrides trade desc

	{
	CTradingDesc *pTradeOverride = AllocTradeDescOverride();
	if (pTradeOverride == NULL)
		return;

	pTradeOverride->SetEconomyType(pCurrency);
	pTradeOverride->SetMaxCurrency(iMaxCurrency);
	pTradeOverride->SetReplenishCurrency(iReplenishCurrency);

	//	This call will set up the currency.

	pTradeOverride->OnCreate(this);
	}

void CSpaceObject::UpdateTrade (SUpdateCtx &Ctx, int iInventoryRefreshed)

//	UpdateTrade
//
//	Updates trading directives. This creates new inventory, if necessary.

	{
	//	Update override first

	CTradingDesc *pTradeOverride = GetTradeDescOverride();
	if (pTradeOverride)
		{
		pTradeOverride->OnUpdate(this);

		if (!IsPlayerDocked())
			pTradeOverride->RefreshInventory(this, iInventoryRefreshed);
		}

	//	Handle base

	CDesignType *pType = GetType();
	CTradingDesc *pTrade = (pType ? pType->GetTradingDesc() : NULL);
	if (pTrade)
		{
		//	If we have a trade desc override, then don't update. [Otherwise
		//	we will replenish currency at double the rate.]

		if (pTradeOverride == NULL)
			pTrade->OnUpdate(this);

		//	But we still need to refresh inventory, since the base 
		//	may contain items not in the override.
		//
		//	LATER: Note that this doesn't handle the case where we try
		//	to override a specific item. The fix is to add the concept
		//	of overriding directly into the class.

		if (!IsPlayerDocked())
			pTrade->RefreshInventory(this, iInventoryRefreshed);
		}
	}

void CSpaceObject::UpdateTradeExtended (const CTimeSpan &ExtraTime)

//	UpdateTradeExtended
//
//	Update trade after a long time.

	{
	//	Refresh inventory, if necessary

	CTradingDesc *pTradeOverride = GetTradeDescOverride();
	CDesignType *pType = GetType();
	CTradingDesc *pTrade = (pType ? pType->GetTradingDesc() : NULL);
	if ((pTrade || pTradeOverride) && ExtraTime.Days() > 0 && !IsAbandoned())
		{
		//	Compute the percent of the inventory that need to refresh

		int iRefreshPercent;
		if (ExtraTime.Days() >= DAYS_TO_REFRESH_INVENTORY)
			iRefreshPercent = 100;
		else
			iRefreshPercent = 100 * ExtraTime.Days() / DAYS_TO_REFRESH_INVENTORY;

		//	Do it

		if (pTradeOverride)
			pTradeOverride->RefreshInventory(this, iRefreshPercent);

		if (pTrade)
			pTrade->RefreshInventory(this, iRefreshPercent);
		}
	}
