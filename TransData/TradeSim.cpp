//	TradeSim.cpp
//
//	Generate statistics about buy/sell prices.
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define NO_LOGO_SWITCH						CONSTLIT("nologo")

struct SItemTradeInfo
	{
	CItemType *pItem;
	int iMaxPrice;
	int iMinPrice;
	int iAveragePrice;
	TArray<int> Records;
	};

typedef TSortMap<CItemType *, SItemTradeInfo> SItemTradeArray;

struct SStationTypeInfo
	{
	CStationType *pType;
	SItemTradeArray Sells;
	SItemTradeArray Buys;
	};

struct SStationTradeInfo
	{
	CStationType *pStation;
	int iMaxPrice;
	int iMinPrice;
	int iAveragePrice;
	TArray<int> Records;
	};

typedef TSortMap<CStationType *, SStationTradeInfo> SStationTradeArray;

struct SItemTypeInfo
	{
	CItemType *pItem;
	SStationTradeArray SoldAt;
	SStationTradeArray BoughtBy;
	};

typedef TSortMap<CStationType *, SStationTypeInfo> SStationData;
typedef TSortMap<CItemType *, SItemTypeInfo> SItemData;

void CompileTradeData (CSpaceObject *pObj, SStationData *retAllStations, SItemData *retAllItems)
	{
	int i;

	CDesignType *pType = pObj->GetType();
	CStationType *pStationType = CStationType::AsType(pType);
	if (pStationType == NULL || pStationType->GetTradingDesc() == NULL)
		return;

	//	All prices in default currency (credits)

	const CEconomyType *pCurrency = CEconomyType::AsType(g_pUniverse->FindDesignType(DEFAULT_ECONOMY_UNID));

	//	Get the entry

	bool bNew;
	SStationTypeInfo *pStationInfo = retAllStations->SetAt(pStationType, &bNew);
	if (bNew)
		pStationInfo->pType = pStationType;

	//	Loop over all item types and see if that station buys or sells.

	for (i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
		{
		CItemType *pItemType = g_pUniverse->GetItemType(i);
		CItem AnItem(pItemType, 1);
		SItemTypeInfo *pItemInfo = NULL;

		//	See if the stations buys this

		int iBuyPrice = (int)pCurrency->Exchange(pObj->GetDefaultEconomy(), pObj->GetBuyPrice(AnItem, CTradingDesc::FLAG_NO_DONATION));
		if (iBuyPrice > 0)
			{
			//	Add an entry for the station type recording that we buy this 
			//	item type at the given price.

			SItemTradeInfo *pItemEntry = pStationInfo->Buys.SetAt(pItemType, &bNew);
			if (bNew)
				pItemEntry->pItem = pItemType;

			pItemEntry->Records.Insert(iBuyPrice);

			//	Add an entry for the item type recording that this station type
			//	buys the given item.

			pItemInfo = retAllItems->SetAt(pItemType, &bNew);
			if (bNew)
				pItemInfo->pItem = pItemType;

			SStationTradeInfo *pStationEntry = pItemInfo->BoughtBy.SetAt(pStationType, &bNew);
			if (bNew)
				pStationEntry->pStation = pStationType;

			pStationEntry->Records.Insert(iBuyPrice);
			}

		//	See if the station sells this

		int iSellPrice = (int)pCurrency->Exchange(pObj->GetDefaultEconomy(), pObj->GetSellPrice(AnItem, 0));
		if (iSellPrice > 0)
			{
			//	Add an entry for the station type recording that we sell this 
			//	item type at the given price.

			SItemTradeInfo *pItemEntry = pStationInfo->Sells.SetAt(pItemType, &bNew);
			if (bNew)
				pItemEntry->pItem = pItemType;

			pItemEntry->Records.Insert(iSellPrice);

			//	Add an entry for the item type recording that this station type
			//	buys the given item.

			if (pItemInfo == NULL)
				{
				pItemInfo = retAllItems->SetAt(pItemType, &bNew);
				if (bNew)
					pItemInfo->pItem = pItemType;
				}

			SStationTradeInfo *pStationEntry = pItemInfo->SoldAt.SetAt(pStationType, &bNew);
			if (bNew)
				pStationEntry->pStation = pStationType;

			pStationEntry->Records.Insert(iSellPrice);
			}
		}
	}

void ComputeAverages (const TArray<int> &Records, int *retiMax, int *retiAverage, int *retiMin)
	{
	int i;
	if (Records.GetCount() == 0)
		{
		*retiMax = 0;
		*retiAverage = 0;
		*retiMin = 0;
		return;
		}

	int iMax = Records[0];
	int iMin = Records[0];
	int iTotal = Records[0];
	for (i = 1; i < Records.GetCount(); i++)
		{
		if (Records[i] < iMin)
			iMin = Records[i];
		else if (Records[i] > iMax)
			iMax = Records[i];

		iTotal += Records[i];
		}

	*retiMax = iMax;
	*retiAverage = iTotal / Records.GetCount();
	*retiMin = iMin;
	}

void ComputeAverages (SStationTradeArray *retTradeArray)
	{
	int i;

	for (i = 0; i < retTradeArray->GetCount(); i++)
		{
		SStationTradeInfo &Info = (*retTradeArray)[i];
		ComputeAverages(Info.Records, &Info.iMaxPrice, &Info.iAveragePrice, &Info.iMinPrice);
		}
	}

void ComputeAverages (SItemTypeInfo *retItemInfo)
	{
	ComputeAverages(&retItemInfo->BoughtBy);
	ComputeAverages(&retItemInfo->SoldAt);
	}

void OutputTradingStrategy (SItemTypeInfo &ItemInfo)
	{
	int i;

	//	Loop over all sellers and get the best price.

	CStationType *pBestSeller = NULL;
	int iBestSellerPrice;
	for (i = 0; i < ItemInfo.SoldAt.GetCount(); i++)
		{
		SStationTradeInfo &Info = ItemInfo.SoldAt[i];
		if (pBestSeller == NULL || Info.iMinPrice < iBestSellerPrice)
			{
			pBestSeller = Info.pStation;
			iBestSellerPrice = Info.iMinPrice;
			}
		}

	//	Now get the best buyer's price

	CStationType *pBestBuyer = NULL;
	int iBestBuyerPrice;
	for (i = 0; i < ItemInfo.BoughtBy.GetCount(); i++)
		{
		SStationTradeInfo &Info = ItemInfo.BoughtBy[i];
		if (pBestBuyer == NULL || Info.iMaxPrice > iBestBuyerPrice)
			{
			pBestBuyer = Info.pStation;
			iBestBuyerPrice = Info.iMaxPrice;
			}
		}

	//	Output

	if (pBestSeller == NULL && pBestBuyer == NULL)
		;

	else if (pBestSeller == NULL)
		printf("%s\t\t\t%s\t%d\t0.0%%\n", 
				ItemInfo.pItem->GetNounPhrase(nounActual).GetASCIIZPointer(), 
				pBestBuyer->GetNounPhrase().GetASCIIZPointer(), 
				iBestBuyerPrice);

	else if (pBestBuyer == NULL)
		printf("%s\t%s\t%d\t\t\t0.0%%\n", 
				ItemInfo.pItem->GetNounPhrase(nounActual).GetASCIIZPointer(), 
				pBestSeller->GetNounPhrase().GetASCIIZPointer(), 
				iBestSellerPrice);

	else
		printf("%s\t%s\t%d\t%s\t%d\t%2.2f%%\n",
				ItemInfo.pItem->GetNounPhrase(nounActual).GetASCIIZPointer(), 
				pBestSeller->GetNounPhrase().GetASCIIZPointer(),
				iBestSellerPrice,
				pBestBuyer->GetNounPhrase().GetASCIIZPointer(),
				iBestBuyerPrice,
				100.0 * (double)(iBestBuyerPrice - iBestSellerPrice) / (double)iBestSellerPrice);
	}

void DoTradeSim (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	ALERROR error;
	int i, j;
	CSovereign *pPlayer = Universe.FindSovereign(g_PlayerSovereignUNID);

	int iSystemSample = pCmdLine->GetAttributeIntegerBounded(CONSTLIT("count"), 1, -1, 1);
	bool bLogo = !pCmdLine->GetAttributeBool(NO_LOGO_SWITCH);

	//	For each station type we keep track of the items that it sells and the
	//	various prices that each instances charges.

	SStationData AllStations;

	//	For each item type we keep track of the stations that sell and/or
	//	buy it.

	SItemData AllItems;

	//	Generate systems for multiple games

	for (i = 0; i < iSystemSample; i++)
		{
		if (bLogo)
			printf("pass %d...\n", i+1);

		CTopologyNode *pNode = Universe.GetFirstTopologyNode();

		while (true)
			{
			//	Create the system

			CSystem *pSystem;
			if (error = Universe.CreateStarSystem(pNode, &pSystem))
				{
				printf("ERROR: Unable to create star system.\n");
				return;
				}

			//	For all active stations in the system, get their trading information

			for (j = 0; j < pSystem->GetObjectCount(); j++)
				{
				CSpaceObject *pObj = pSystem->GetObject(j);

				if (pObj && pObj->GetCategory() == CSpaceObject::catStation)
					CompileTradeData(pObj, &AllStations, &AllItems);
				}

			//	Get the next node

			CString sEntryPoint;
			pNode = pSystem->GetStargateDestination(CONSTLIT("Outbound"), &sEntryPoint);
			if (pNode == NULL || pNode->IsEndGame())
				break;

			//	Done with old system

			Universe.DestroySystem(pSystem);
			}

		Universe.Reinit();
		}

	if (bLogo)
		printf("FINAL STATISTICS\n\n");

	//	Loop over each item and output the best trading strategy

	printf("Item\tSeller\tSell Price\tBuyer\tBuy Price\tProfit\n");
	for (i = 0; i < AllItems.GetCount(); i++)
		{
		ComputeAverages(&AllItems[i]);
		OutputTradingStrategy(AllItems[i]);
		}
	}

