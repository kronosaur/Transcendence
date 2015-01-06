//	CTradingDesc.cpp
//
//	CTradingDesc class

#include "PreComp.h"

#define ACCEPT_DONATION_TAG						CONSTLIT("AcceptDonation")
#define BUY_TAG									CONSTLIT("Buy")
#define INSTALL_ARMOR_TAG						CONSTLIT("InstallArmor")
#define INSTALL_DEVICE_TAG						CONSTLIT("InstallDevice")
#define REFUEL_TAG								CONSTLIT("Refuel")
#define REMOVE_DEVICE_TAG						CONSTLIT("RemoveDevice")
#define REPAIR_ARMOR_TAG						CONSTLIT("RepairArmor")
#define SELL_TAG								CONSTLIT("Sell")

#define ACTUAL_PRICE_ATTRIB						CONSTLIT("actualPrice")
#define CURRENCY_ATTRIB							CONSTLIT("currency")
#define CREDIT_CONVERSION_ATTRIB				CONSTLIT("creditConversion")
#define CRITERIA_ATTRIB							CONSTLIT("criteria")
#define INVENTORY_ADJ_ATTRIB					CONSTLIT("inventoryAdj")
#define ITEM_ATTRIB								CONSTLIT("item")
#define MAX_ATTRIB								CONSTLIT("max")
#define PRICE_ADJ_ATTRIB						CONSTLIT("priceAdj")
#define REPLENISH_ATTRIB						CONSTLIT("replenish")

#define CONSTANT_PREFIX							CONSTLIT("constant")
#define UNAVAILABLE_PREFIX						CONSTLIT("unavailable")

struct SServiceData
	{
	char *pszName;
	char *pszIDPrefix;
	char *pszTag;
	};

static SServiceData SERVICE_DATA[serviceCount] =
	{
		{	"(serviceNone)",			"",		""				},	//	serviceNone

		{	"priceOfferedToPlayer",		"B",	"Buy"			},	//	serviceBuy
		{	"priceForPlayerToBuy",		"S",	"Sell"			},	//	serviceSell
		{	"valueOfDonation",			"A",	"AcceptDonation"},	//	serviceAcceptDonations
		{	"priceToRefuel",			"F",	"Refuel"		},	//	serviceRefuel
		{	"priceToRepairArmor",		"Ra",	"RepairArmor"	},	//	serviceRepairArmor

		{	"priceToReplaceArmor",		"Ia",	"ReplaceArmor"	},	//	serviceReplaceArmor
		{	"priceToInstallDevice",		"Id",	"InstallDevice"	},	//	serviceInstallDevice
		{	"priceToRemoveDevice",		"Vd",	"RemoveDevice"	},	//	serviceRemoveDevice
		{	"priceToUpgradeDevice",		"Ud",	"UpgradeDevice"	},	//	serviceUpgradeDevice
		{	"priceToEnhanceItem",		"Ei",	"EnhanceItem"	},	//	serviceEnhanceItem

		{	"priceToRepairItem",		"Ri",	"RepairItem"	},	//	serviceRepairItem
		{	"customPrice",				"X",	"Custom"		},	//	serviceCustom
	};

CTradingDesc::CTradingDesc (void) : 
		m_iMaxCurrency(0),
		m_iReplenishCurrency(0)

//	CTradingDesc constructor

	{
	}

CTradingDesc::~CTradingDesc (void)

//	CTradingDesc destructor

	{
	}

void CTradingDesc::AddOrder (CItemType *pItemType, const CString &sCriteria, int iPriceAdj, DWORD dwFlags)

//	AddOrder
//
//	Add a new commodity line

	{
	int i;

	//	Convert to new method

	ETradeServiceTypes iService;
	DWORD dwNewFlags;
	ReadServiceFromFlags(dwFlags, &iService, &dwNewFlags);
	if (iService == serviceNone)
		return;

	//	We always add at the beginning (because new orders take precedence)

	SServiceDesc *pCommodity = m_List.InsertAt(0);
	pCommodity->iService = iService;
	pCommodity->pItemType = pItemType;
	if (pItemType == NULL)
		CItem::ParseCriteria(sCriteria, &pCommodity->ItemCriteria);
	pCommodity->PriceAdj.SetInteger(iPriceAdj);
	pCommodity->dwFlags = dwNewFlags;
	pCommodity->sID = ComputeID(iService, pCommodity->pItemType.GetUNID(), sCriteria, dwNewFlags);

	//	If we find a previous order for the same criteria, then delete it

	for (i = 1; i < m_List.GetCount(); i++)
		if (strEquals(pCommodity->sID, m_List[i].sID))
			{
			m_List.Delete(i);
			break;
			}
	}

int CTradingDesc::CalcPriceForService (ETradeServiceTypes iService, CSpaceObject *pProvider, const CItem &Item, int iCount, DWORD dwFlags)

//	CalcPriceForService
//
//	Compute a price for a service (used by objects without a Trade descriptor

	{
	SServiceDesc Default;
	Default.iService = iService;
	Default.PriceAdj.InitFromInteger(100);
	Default.dwFlags = 0;

	return ComputePrice(pProvider, pProvider->GetDefaultEconomy(), Item, iCount, Default, dwFlags);
	}

CString CTradingDesc::ComputeID (ETradeServiceTypes iService, DWORD dwUNID, const CString &sCriteria, DWORD dwFlags)

//	ComputeID
//
//	Generates a string ID for the order. Two identical orders should have the same ID

	{
	CString sService = ((iService >= 0 && iService < serviceCount) ? CString(SERVICE_DATA[iService].pszIDPrefix) : CONSTLIT("?"));

	if (dwUNID)
		return strPatternSubst(CONSTLIT("%s:%x"), sService, dwUNID);
	else
		return strPatternSubst(CONSTLIT("%s:%s"), sService, sCriteria);
	}

int CTradingDesc::ComputeMaxCurrency (CSpaceObject *pObj)

//	ComputeMaxCurrency
//
//	Computes max balance

	{
	return m_iMaxCurrency * (90 + ((pObj->GetDestiny() + 9) / 18)) / 100;
	}

int CTradingDesc::ComputePrice (CSpaceObject *pObj, CEconomyType *pCurrency, const CItem &Item, int iCount, const SServiceDesc &Commodity, DWORD dwFlags)

//	ComputePrice
//
//	Computes the price of the item in the proper currency

	{
	bool bActual = (Commodity.dwFlags & FLAG_ACTUAL_PRICE ? true : false);
	bool bPlayerAdj = !(dwFlags & FLAG_NO_PLAYER_ADJ);

	//	Get the raw price from the item

	int iItemPrice;
	switch (Commodity.iService)
		{
		case serviceRepairArmor:
			{
			CArmorClass *pArmor = Item.GetType()->GetArmorClass();
			if (pArmor == NULL)
				return -1;

			iItemPrice = iCount * pArmor->GetRepairCost();
			break;
			}

		case serviceInstallDevice:
		case serviceReplaceArmor:
			{
			iItemPrice = iCount * Item.GetType()->GetInstallCost();
			break;
			}

		case serviceRemoveDevice:
			{
			iItemPrice = iCount * Item.GetType()->GetInstallCost() / 2;
			break;
			}

		default:
			iItemPrice = iCount * Item.GetTradePrice(pObj, bActual);
			break;
		}

	//	Adjust the price appropriately

	CurrencyValue iPrice;
	CString sPrefix;
	int iPriceAdj = Commodity.PriceAdj.EvalAsInteger(pObj, &sPrefix);

	if (sPrefix.IsBlank())
		iPrice = iPriceAdj * iItemPrice / 100;
	else if (strEquals(sPrefix, CONSTANT_PREFIX))
		iPrice = iPriceAdj;
	else if (strEquals(sPrefix, UNAVAILABLE_PREFIX))
		return -1;
	else
		{
		kernelDebugLogMessage("Unknown priceAdj prefix: %s", sPrefix);
		return -1;
		}

	//	Let global types adjust the price

	int iPlayerPriceAdj;
	if (bPlayerAdj 
			&& g_pUniverse->GetDesignCollection().FireGetGlobalPlayerPriceAdj(Commodity.iService, pObj, Item, NULL, &iPlayerPriceAdj))
		{
		if (iPlayerPriceAdj <= 0)
			return -1;

		iPrice = iPlayerPriceAdj * iPrice / 100;
		}

	//	Convert to proper currency

	return (int)pCurrency->Exchange(Item.GetCurrencyType(), iPrice);
	}

ALERROR CTradingDesc::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CTradingDesc **retpTrade)

//	InitFromXML
//
//	Initialize from an XML element

	{
	ALERROR error;
	int i, j;

	//	Allocate the trade structure

	CTradingDesc *pTrade = new CTradingDesc;
	pTrade->m_pCurrency.LoadUNID(pDesc->GetAttribute(CURRENCY_ATTRIB));
	pTrade->m_iMaxCurrency = pDesc->GetAttributeIntegerBounded(MAX_ATTRIB, 0, -1, 0);
	pTrade->m_iReplenishCurrency = pDesc->GetAttributeIntegerBounded(REPLENISH_ATTRIB, 0, -1, 0);

	//	Allocate the array

	int iCount = pDesc->GetContentElementCount();
	if (iCount)
		{
		pTrade->m_List.InsertEmpty(iCount);

		//	Load

		for (i = 0; i < iCount; i++)
			{
			CXMLElement *pLine = pDesc->GetContentElement(i);
			SServiceDesc *pCommodity = &pTrade->m_List[i];

			//	Parse criteria

			CString sCriteria = pLine->GetAttribute(CRITERIA_ATTRIB);
			if (!sCriteria.IsBlank())
				CItem::ParseCriteria(sCriteria, &pCommodity->ItemCriteria);
			else
				CItem::InitCriteriaAll(&pCommodity->ItemCriteria);

			//	Item

			if (error = pCommodity->pItemType.LoadUNID(Ctx, pLine->GetAttribute(ITEM_ATTRIB)))
				return error;

			//	Other

			if (error = pCommodity->PriceAdj.InitFromString(Ctx, pLine->GetAttribute(PRICE_ADJ_ATTRIB)))
				return error;

			if (error = pCommodity->InventoryAdj.InitFromString(Ctx, pLine->GetAttribute(INVENTORY_ADJ_ATTRIB)))
				return error;

			//	Service

			pCommodity->iService = serviceNone;
			for (j = 0; j < serviceCount; j++)
				{
				if (strEquals(pLine->GetTag(), CString(SERVICE_DATA[j].pszTag, -1, true)))
					{
					pCommodity->iService = (ETradeServiceTypes)j;
					break;
					}
				}

			if (pCommodity->iService == serviceNone)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Unknown Trade directive: %s."), pLine->GetTag());
				return ERR_FAIL;
				}

			//	Flags

			pCommodity->dwFlags = 0;
			if (pLine->GetAttributeBool(ACTUAL_PRICE_ATTRIB))
				pCommodity->dwFlags |= FLAG_ACTUAL_PRICE;

			if (!pCommodity->InventoryAdj.IsEmpty())
				pCommodity->dwFlags |= FLAG_INVENTORY_ADJ;

			//	Set ID

			pCommodity->sID = pTrade->ComputeID(pCommodity->iService, pCommodity->pItemType.GetUNID(), sCriteria, pCommodity->dwFlags);
			}
		}

	//	Done

	*retpTrade = pTrade;

	return NOERROR;
	}

bool CTradingDesc::Buys (CSpaceObject *pObj, const CItem &Item, DWORD dwFlags, int *retiPrice, int *retiMaxCount)

//	Buys
//
//	Returns TRUE if the given object buys items of the given type.
//	Optionally returns a price and a max number.
//
//	Note that we always return a price for items we are willing to buy, even if we
//	don't currently have enough to buy it.

	{
	int i;

	//	Loop over the commodity list and find the first entry that matches

	for (i = 0; i < m_List.GetCount(); i++)
		if (((m_List[i].iService == serviceBuy) || (m_List[i].iService == serviceAcceptDonations))
				&& Matches(Item, m_List[i]))
			{
			//	If this is virtual price and we don't want virtual, then skip

			if ((m_List[i].iService == serviceAcceptDonations)
					&& (dwFlags & FLAG_NO_DONATION))
				return false;

			//	Compute price

			int iPrice = ComputePrice(pObj, Item, 1, m_List[i], dwFlags);
			if (iPrice < 0)
				return false;

			//	Compute the maximum number of this item that we are willing
			//	to buy. First we figure out how much money the station has left

			int iBalance = (int)pObj->GetBalance(m_pCurrency->GetUNID());
			int iMaxCount = (iPrice > 0 ? (iBalance / iPrice) : 0);

			//	Done

			if (retiMaxCount)
				*retiMaxCount = iMaxCount;

			if (retiPrice)
				*retiPrice = iPrice;

			return true;
			}

	return false;
	}

int CTradingDesc::Charge (CSpaceObject *pObj, int iCharge)

//	Charge
//
//	Charge out of the station's balance

	{
	if (m_iMaxCurrency)
		return (int)pObj->ChargeMoney(m_pCurrency->GetUNID(), iCharge);
	else
		return 0;
	}

bool CTradingDesc::FindService (ETradeServiceTypes iService, const CItem &Item, const SServiceDesc **retpDesc)

//	FindService
//
//	Finds the given service for the given item.

	{
	int i;

	//	Loop over the commodity list and find the first entry that matches

	for (i = 0; i < m_List.GetCount(); i++)
		if (m_List[i].iService == iService 
				&& Matches(Item, m_List[i]))
			{
			if (retpDesc)
				*retpDesc = &m_List[i];

			return true;
			}
	
	return false;
	}

bool CTradingDesc::GetArmorInstallPrice (CSpaceObject *pObj, const CItem &Item, DWORD dwFlags, int *retiPrice) const

//	GetArmorInstallPrice
//
//	Returns the price to install the given armor

	{
	int i;

	//	Loop over the commodity list and find the first entry that matches

	for (i = 0; i < m_List.GetCount(); i++)
		if (m_List[i].iService == serviceReplaceArmor
				&& Matches(Item, m_List[i]))
			{
			//	Compute price

			int iPrice = ComputePrice(pObj, Item, 1, m_List[i], dwFlags);
			if (iPrice < 0)
				return false;

			//	Done

			if (retiPrice)
				*retiPrice = iPrice;

			return true;
			}

	return false;
	}

bool CTradingDesc::GetArmorRepairPrice (CSpaceObject *pObj, const CItem &Item, int iHPToRepair, DWORD dwFlags, int *retiPrice) const

//	GetArmorRepairPrice
//
//	Returns the price for repairing the given armor.

	{
	int i;

	//	Loop over the commodity list and find the first entry that matches

	for (i = 0; i < m_List.GetCount(); i++)
		if (m_List[i].iService == serviceRepairArmor
				&& Matches(Item, m_List[i]))
			{
			//	Compute price

			int iPrice = ComputePrice(pObj, Item, iHPToRepair, m_List[i], dwFlags);
			if (iPrice < 0)
				return false;

			//	Done

			if (retiPrice)
				*retiPrice = iPrice;

			return true;
			}

	return false;
	}

bool CTradingDesc::GetDeviceInstallPrice (CSpaceObject *pObj, const CItem &Item, DWORD dwFlags, int *retiPrice) const

//	GetDeviceInstallPrice
//
//	Returns the price to install the given device

	{
	int i;

	//	Loop over the commodity list and find the first entry that matches

	for (i = 0; i < m_List.GetCount(); i++)
		if (m_List[i].iService == serviceInstallDevice
				&& Matches(Item, m_List[i]))
			{
			//	Compute price

			int iPrice = ComputePrice(pObj, Item, 1, m_List[i], dwFlags);
			if (iPrice < 0)
				return false;

			//	Done

			if (retiPrice)
				*retiPrice = iPrice;

			return true;
			}

	return false;
	}

bool CTradingDesc::GetDeviceRemovePrice (CSpaceObject *pObj, const CItem &Item, DWORD dwFlags, int *retiPrice) const

//	GetDeviceRemovePrice
//
//	Returns the price to remove the given device

	{
	int i;

	//	Loop over the commodity list and find the first entry that matches

	for (i = 0; i < m_List.GetCount(); i++)
		if (m_List[i].iService == serviceRemoveDevice
				&& Matches(Item, m_List[i]))
			{
			//	Compute price

			int iPrice = ComputePrice(pObj, Item, 1, m_List[i], dwFlags);
			if (iPrice < 0)
				return false;

			//	Done

			if (retiPrice)
				*retiPrice = iPrice;

			return true;
			}

	return false;
	}

int CTradingDesc::GetMaxLevelMatched (ETradeServiceTypes iService) const

//	GetMaxLevelMatched
//
//	Returns the max item level matched for the given service. Returns -1 if the
//	service is not available.

	{
	int i;
	int iMaxLevel = -1;

	//	Loop over the commodity list and find the first entry that matches

	for (i = 0; i < m_List.GetCount(); i++)
		if (m_List[i].iService == iService)
			{
			int iLevel;
			if (m_List[i].pItemType)
				iLevel = m_List[i].pItemType->GetLevel();
			else
				iLevel = m_List[i].ItemCriteria.GetMaxLevelMatched();

			if (iLevel > iMaxLevel)
				iMaxLevel = iLevel;
			}
	
	return iMaxLevel;
	}

bool CTradingDesc::GetRefuelItemAndPrice (CSpaceObject *pObj, CSpaceObject *pObjToRefuel, DWORD dwFlags, CItemType **retpItemType, int *retiPrice) const

//	GetRefuelItemAndPrice
//
//	Returns the appropriate fuel and price to refuel the given object.

	{
	int i, j;
	CShip *pShipToRefuel = pObjToRefuel->AsShip();
	if (pShipToRefuel == NULL)
		return false;

	for (i = 0; i < m_List.GetCount(); i++)
		if (m_List[i].iService == serviceRefuel)
			{
			int iBestLevel = 0;
			int iBestPrice = 0;
			CItemType *pBestItem = NULL;

			//	Find the highest-level item that matches the given criteria.
			//	If we find it, then we use it.

			for (j = 0; j < g_pUniverse->GetItemTypeCount(); j++)
				{
				CItemType *pType = g_pUniverse->GetItemType(j);
				CItem Item(pType, 1);

				if (Item.MatchesCriteria(m_List[i].ItemCriteria)
						&& pShipToRefuel->IsFuelCompatible(Item))
					{
					if (pBestItem == NULL || pType->GetLevel() > iBestLevel)
						{
						//	Compute the price, because if we don't sell it, then we
						//	skip it.
						//
						//	NOTE: Unlike selling, we allow 0 prices because some 
						//	stations give fuel for free.

						int iPrice = ComputePrice(pObj, Item, 1, m_List[i], dwFlags);
						if (iPrice >= 0)
							{
							pBestItem = pType;
							iBestLevel = pType->GetLevel();
							iBestPrice = iPrice;
							}
						}
					}
				}

			//	If found, then return it.

			if (pBestItem)
				{
				if (retpItemType)
					*retpItemType = pBestItem;

				if (retiPrice)
					*retiPrice = iBestPrice;

				return true;
				}
			}

	//	Otherwise, not found

	return false;
	}

bool CTradingDesc::Matches (const CItem &Item, const SServiceDesc &Commodity) const

//	Matches
//
//	Returns TRUE if the given item matches the commodity

	{
	if (Commodity.pItemType)
		return (Commodity.pItemType == Item.GetType());
	else
		return Item.MatchesCriteria(Commodity.ItemCriteria);
	}

void CTradingDesc::OnCreate (CSpaceObject *pObj)

//	OnCreate
//
//	Station is created

	{
	//	Give the station a limited amount of money

	if (m_iMaxCurrency)
		{
		int iMaxCurrency = ComputeMaxCurrency(pObj);
		pObj->CreditMoney(m_pCurrency->GetUNID(), iMaxCurrency);
		}
	}

ALERROR CTradingDesc::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Design loaded

	{
	ALERROR error;
	int i;

	if (error = m_pCurrency.Bind(Ctx))
		return error;

	for (i = 0; i < m_List.GetCount(); i++)
		if (error = m_List[i].pItemType.Bind(Ctx))
			return error;

	return NOERROR;
	}

void CTradingDesc::OnUpdate (CSpaceObject *pObj)

//	OnUpdate
//
//	Station updates (call roughly every 1800 ticks)

	{
	DEBUG_TRY

	if (m_iMaxCurrency && m_iReplenishCurrency)
		{
		int iBalance = (int)pObj->GetBalance(m_pCurrency->GetUNID());
		int iMaxCurrency = ComputeMaxCurrency(pObj);

		if (iBalance < iMaxCurrency)
			pObj->CreditMoney(m_pCurrency->GetUNID(), m_iReplenishCurrency);
		}

	DEBUG_CATCH
	}

void CTradingDesc::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read from a stream
//
//	DWORD			m_pCurrency UNID
//	DWORD			m_iMaxCurrency
//	DWORD			m_iReplenishCurrency
//	DWORD			No of orders
//
//	DWORD			iService
//	CString			sID
//	DWORD			pItemType
//	CString			ItemCriteria
//	CFormulaText	PriceAdj
//	CFormulaText	InventoryAdj
//	DWORD			dwFlags

	{
	int i;
	DWORD dwLoad;

	if (Ctx.dwVersion >= 62)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_pCurrency.Set(dwLoad);
		if (m_pCurrency == NULL)
			m_pCurrency.Set(DEFAULT_ECONOMY_UNID);
		}
	else
		{
		CString sDummy;
		sDummy.ReadFromStream(Ctx.pStream);
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));

		//	Previous versions are always credits

		m_pCurrency.Set(DEFAULT_ECONOMY_UNID);
		}

	Ctx.pStream->Read((char *)&m_iMaxCurrency, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iReplenishCurrency, sizeof(DWORD));

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad > 0)
		{
		m_List.InsertEmpty(dwLoad);

		for (i = 0; i < m_List.GetCount(); i++)
			{
			SServiceDesc &Commodity = m_List[i];

			if (Ctx.dwVersion >= 83)
				Ctx.pStream->Read((char *)&Commodity.iService, sizeof(DWORD));

			Commodity.sID.ReadFromStream(Ctx.pStream);

			Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
			Commodity.pItemType = g_pUniverse->FindItemType(dwLoad);

			CString sCriteria;
			sCriteria.ReadFromStream(Ctx.pStream);
			CItem::ParseCriteria(sCriteria, &Commodity.ItemCriteria);

			if (Ctx.dwVersion >= 62)
				{
				Commodity.PriceAdj.ReadFromStream(Ctx);
				Commodity.InventoryAdj.ReadFromStream(Ctx);
				}
			else
				{
				Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
				Commodity.PriceAdj.SetInteger(dwLoad);
				}

			Ctx.pStream->Read((char *)&Commodity.dwFlags, sizeof(DWORD));

			//	If necessary we need to load backwards-compatible service

			if (Ctx.dwVersion < 83)
				ReadServiceFromFlags(Commodity.dwFlags, &Commodity.iService, &Commodity.dwFlags);

			//	For now we don't support inventory adj in dynamic trade descs
			
			Commodity.dwFlags &= ~FLAG_INVENTORY_ADJ;
			}
		}
	}

void CTradingDesc::ReadServiceFromFlags (DWORD dwFlags, ETradeServiceTypes *retiService, DWORD *retdwFlags)

//	ReadServiceFromFlags
//
//	Before we had service types we encoded info in flags. This function converts
//	to the new method.

	{
	if (retiService)
		{
		if (dwFlags & FLAG_SELLS)
			*retiService = serviceSell;
		else if (dwFlags & FLAG_BUYS)
			*retiService = serviceBuy;
		else if (dwFlags & FLAG_ACCEPTS_DONATIONS)
			*retiService = serviceAcceptDonations;
		else
			{
			ASSERT(false);
			*retiService = serviceNone;
			}
		}

	if (retdwFlags)
		*retdwFlags = (dwFlags & ~(FLAG_SELLS | FLAG_BUYS | FLAG_ACCEPTS_DONATIONS));
	}

void CTradingDesc::RefreshInventory (CSpaceObject *pObj, int iPercent)

//	RefreshInventory
//
//	Refresh the inventory for all entries that have an inventory
//	adjustment factor.

	{
	DEBUG_TRY

	int i, j;
	bool bCargoChanged = false;

	for (i = 0; i < m_List.GetCount(); i++)
		if (m_List[i].dwFlags & FLAG_INVENTORY_ADJ)
			{
			//	Make a list of all item types that match the given
			//	criteria.

			TArray<CItemType *> ItemTable;
			for (j = 0; j < g_pUniverse->GetItemTypeCount(); j++)
				{
				CItemType *pType = g_pUniverse->GetItemType(j);
				CItem theItem(pType, 1);
				if (theItem.MatchesCriteria(m_List[i].ItemCriteria))
					ItemTable.Insert(pType);
				}

			//	Loop over the count

			if (ItemTable.GetCount() == 0)
				continue;

			//	Loop over all items refreshing them

			for (j = 0; j < ItemTable.GetCount(); j++)
				if (iPercent == 100 || mathRandom(1, 100) <= iPercent)
					{
					if (SetInventoryCount(pObj, m_List[i], ItemTable[j]))
						bCargoChanged = true;
					}
			}

	if (bCargoChanged)
		pObj->ItemsModified();

	DEBUG_CATCH
	}

bool CTradingDesc::Sells (CSpaceObject *pObj, const CItem &Item, DWORD dwFlags, int *retiPrice)

//	Sells
//
//	Returns TRUE if the given object can currently sell the given item type.
//	Optionally returns a price

	{
	const SServiceDesc *pDesc;
	if (!FindService(serviceSell, Item, &pDesc))
		return false;

	//	Compute price

	int iPrice = ComputePrice(pObj, Item, 1, *pDesc, dwFlags);
	if (iPrice <= 0)
		return false;

	//	Done

	if (retiPrice)
		*retiPrice = iPrice;

	return true;
	}

CString CTradingDesc::ServiceToString (ETradeServiceTypes iService)

//	ServiceToString
//
//	Returns a string representing the service

	{
	if (iService >= 0 && iService < serviceCount)
		return CString(SERVICE_DATA[iService].pszName);
	else
		return CONSTLIT("unknown");
	}

bool CTradingDesc::SetInventoryCount (CSpaceObject *pObj, SServiceDesc &Desc, CItemType *pItemType)

//	SetInventoryCount
//
//	Sets the count for the given item
//	Returns TRUE if the count was changed; FALSE otherwise.

	{
	bool bCargoChanged = false;
	CItemListManipulator ItemList(pObj->GetItemList());

	//	Roll number appearing

	int iItemCount = pItemType->GetNumberAppearing().Roll();

	//	Adjust based on inventory adjustment

	iItemCount = iItemCount * Desc.InventoryAdj.EvalAsInteger(pObj) / 100;

	//	If the item exists, set the count

	if (ItemList.SetCursorAtItem(CItem(pItemType, 1)))
		{
		ItemList.SetCountAtCursor(iItemCount);
		bCargoChanged = true;
		}

	//	Otherwise, add the appropriate number of items

	else if (iItemCount > 0)
		{
		ItemList.AddItem(CItem(pItemType, iItemCount));
		bCargoChanged = true;
		}

	//	Done

	return bCargoChanged;
	}

void CTradingDesc::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Writes to a stream
//
//	DWORD			m_pCurrency UNID
//	DWORD			m_iMaxCurrency
//	DWORD			m_iReplenishCurrency
//	DWORD			No of orders
//
//	DWORD			iService
//	CString			sID
//	DWORD			pItemType
//	CString			ItemCriteria
//	CFormulaText	PriceAdj
//	CFormulaText	InventoryAdj
//	DWORD			dwFlags

	{
	int i;
	DWORD dwSave;

	dwSave = (m_pCurrency ? m_pCurrency->GetUNID() : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_iMaxCurrency, sizeof(DWORD));
	pStream->Write((char *)&m_iReplenishCurrency, sizeof(DWORD));

	dwSave = m_List.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < m_List.GetCount(); i++)
		{
		const SServiceDesc &Commodity = m_List[i];

		dwSave = Commodity.iService;
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		Commodity.sID.WriteToStream(pStream);

		dwSave = (Commodity.pItemType ? Commodity.pItemType->GetUNID() : 0);
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		CString sCriteria = CItem::GenerateCriteria(Commodity.ItemCriteria);
		sCriteria.WriteToStream(pStream);

		Commodity.PriceAdj.WriteToStream(pStream);
		Commodity.InventoryAdj.WriteToStream(pStream);

		pStream->Write((char *)&Commodity.dwFlags, sizeof(DWORD));
		}
	}
