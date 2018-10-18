//	ItemTable.cpp
//
//	Generate basic stats about items

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define BY_ATTRIBUTE_ATTRIB					CONSTLIT("byAttribute")
#define BY_COMPONENT_ATTRIB					CONSTLIT("byComponent")
#define BY_SHIP_CLASS_ATTRIB				CONSTLIT("byShipClass")
#define BY_SHIP_CLASS_USAGE_ATTRIB			CONSTLIT("byShipClassUsage")
#define CRITERIA_ATTRIB						CONSTLIT("criteria")
#define GENERATE_ACTUAL_COUNT_ATTRIB		CONSTLIT("generateSimTables")
#define TABLE_ATTRIB						CONSTLIT("table")

#define FIELD_AVERAGE_DAMAGE				CONSTLIT("averageDamage")
#define FIELD_TYPE							CONSTLIT("category")
#define FIELD_FREQUENCY						CONSTLIT("frequency")
#define FIELD_LEVEL							CONSTLIT("level")
#define FIELD_NAME							CONSTLIT("shortName")
#define FIELD_POWER							CONSTLIT("power")
#define FIELD_POWER_PER_SHOT				CONSTLIT("powerPerShot")
#define FIELD_PROBABILITY					CONSTLIT("probability")
#define FIELD_TOTAL_COUNT					CONSTLIT("totalCount")

#define FIELD_BENCHMARK						CONSTLIT("benchmark")
#define FIELD_BALANCE_STATS                 CONSTLIT("balanceStats")

#define TOTAL_COUNT_FILENAME				CONSTLIT("TransData_ItemCount.txt")

static char *g_szTypeCode[] =
	{
	"",
	"Armor",
	"Weapon",
	"Missile",
	"Shield",
	"Device",
	"Useful",
	"Fuel",
	"Misc",
	};

static char *g_szFreqCode[] =
	{
	"",	"C", "UC", "R", "VR", "NR",
	};

struct SItemTableCtx
	{
	CUniverse *pUniverse = NULL;
	CXMLElement *pCmdLine = NULL;

	TArray<CString> Cols;
	CDesignTypeStats TotalCount;

    CWeaponBenchmarkCtx WeaponBenchmarks;
	bool bHasArmor = false;
	bool bHasShields = false;
	bool bArmorBalanceStats = false;
	bool bShieldBalanceStats = false;
	bool bWeaponBalanceStats = false;

	CItemTypeProbabilityTable ProbTable;
	};

typedef TSortMap<CString, CItemType *> SItemTypeList;

struct SAttributeEntry
	{
	CString sAttribute;
	SItemTypeList ItemTable;
	};

typedef TSortMap<CString, SAttributeEntry> SByAttributeTypeList;

struct SShipClassEntry
	{
	CString sShipClassName;
	SItemTypeList ItemTable;
	};

typedef TSortMap<CString, SShipClassEntry> SByShipClassTypeList;

bool CalcColumns (SItemTableCtx &Ctx, CXMLElement *pCmdLine);
int GetItemFreq (CItemType *pType);
int GetItemType (CItemType *pType);
void OutputByAttribute (SItemTableCtx &Ctx, const SItemTypeList &ItemList);
void OutputByComponent (SItemTableCtx &Ctx, const SItemTypeList &ItemList);
void OutputByShipClass (SItemTableCtx &Ctx, const SItemTypeList &ItemList, bool bShowUsage);
void OutputHeader (SItemTableCtx &Ctx);
void OutputTable (SItemTableCtx &Ctx, const SItemTypeList &ItemList);
void SelectByCriteria (SItemTableCtx &Ctx, const CString &sCriteria, TArray<CItemType *> *retList);
bool SelectByItemTable (SItemTableCtx &Ctx, const CString &sTable, int iLevel, TArray<CItemType *> *retList);
void SortTable (SItemTableCtx &Ctx, const TArray<CItemType *> &List, SItemTypeList *retSorted);

void GenerateItemTable (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	//	Create the context

	SItemTableCtx Ctx;
	Ctx.pUniverse = &Universe;
	Ctx.pCmdLine = pCmdLine;

	//	Compute the list of items in the table

	TArray<CItemType *> Selection;

	CString sValue;
	if (pCmdLine->FindAttribute(TABLE_ATTRIB, &sValue))
		{
		int iLevel = pCmdLine->GetAttributeIntegerBounded(FIELD_LEVEL, 1, MAX_SYSTEM_LEVEL, 1);
		if (!SelectByItemTable(Ctx, sValue, iLevel, &Selection))
			return;

		if (Selection.GetCount() == 0)
			{
			printf("No entries in table.\n");
			return;
			}
		}
	else
		{
		SelectByCriteria(Ctx, pCmdLine->GetAttribute(CRITERIA_ATTRIB), &Selection);
		if (Selection.GetCount() == 0)
			{
			printf("No entries match criteria.\n");
			return;
			}
		}

	//	Figure out if we're showing balance stats

	if (pCmdLine->GetAttributeBool(FIELD_BALANCE_STATS))
		{
		if (Ctx.bHasArmor)
			Ctx.bArmorBalanceStats = true;
		else if (Ctx.bHasShields)
			Ctx.bShieldBalanceStats = true;
		else
			Ctx.bWeaponBalanceStats = true;
		}

	//	Compute columns

	if (!CalcColumns(Ctx, pCmdLine))
		return;

	//	Sort the list

	SItemTypeList ItemList;
	SortTable(Ctx, Selection, &ItemList);

	//	If by attribute, output categorized list

	if (pCmdLine->GetAttributeBool(BY_ATTRIBUTE_ATTRIB))
		OutputByAttribute(Ctx, ItemList);

	else if (pCmdLine->GetAttributeBool(BY_COMPONENT_ATTRIB))
		OutputByComponent(Ctx, ItemList);

	else if (pCmdLine->GetAttributeBool(BY_SHIP_CLASS_ATTRIB))
		OutputByShipClass(Ctx, ItemList, false);

	else if (pCmdLine->GetAttributeBool(BY_SHIP_CLASS_USAGE_ATTRIB))
		OutputByShipClass(Ctx, ItemList, true);

	//	Otherwise, just output a full table

	else
		{
		OutputHeader(Ctx);
		OutputTable(Ctx, ItemList);
		}

	printf("\n");
	}

bool CalcColumns (SItemTableCtx &Ctx, CXMLElement *pCmdLine)
	{
	int i;

	Ctx.Cols.DeleteAll();

	//	Generate a list of columns to display

	Ctx.Cols.Insert(FIELD_LEVEL);
	Ctx.Cols.Insert(FIELD_TYPE);
	Ctx.Cols.Insert(FIELD_FREQUENCY);
	Ctx.Cols.Insert(FIELD_NAME);

	//	If we have a probability table, the add probability

	if (Ctx.ProbTable.GetCount() > 0)
		Ctx.Cols.Insert(FIELD_PROBABILITY);

	//	Add fields as columns

	for (i = 0; i < pCmdLine->GetAttributeCount(); i++)
		{
		CString sAttrib = pCmdLine->GetAttributeName(i);

		if (!IsMainCommandParam(sAttrib)
				&& !strEquals(sAttrib, TABLE_ATTRIB)
				&& !strEquals(sAttrib, FIELD_LEVEL)
				&& !strEquals(sAttrib, BY_ATTRIBUTE_ATTRIB)
				&& !strEquals(sAttrib, BY_SHIP_CLASS_ATTRIB)
				&& !strEquals(sAttrib, BY_SHIP_CLASS_USAGE_ATTRIB)
				&& !strEquals(sAttrib, CONSTLIT("itemtable")))
			{
			CString sValue = pCmdLine->GetAttribute(i);
				
			if (!strEquals(sValue, CONSTLIT("true")))
				Ctx.Cols.Insert(strPatternSubst(CONSTLIT("%s:%s"), sAttrib, sValue));
			else
				Ctx.Cols.Insert(sAttrib);
			}
		}

	//	If we need to output total count, then load the table

	if (pCmdLine->GetAttributeBool(FIELD_TOTAL_COUNT))
		{
		if (LoadDesignTypeStats(Ctx.pUniverse->GetDesignCollection().GetAdventureUNID(), &Ctx.TotalCount) != NOERROR)
			return false;
		}

	return true;
	}

int GetItemFreq (CItemType *pType)
	{
	int iFreq = pType->GetFrequency();
	if (iFreq == ftCommon)
		return 1;
	else if (iFreq == ftUncommon)
		return 2;
	else if (iFreq == ftRare)
		return 3;
	else if (iFreq == ftVeryRare)
		return 4;
	else
		return 5;
	}

int GetItemType (CItemType *pType)
	{
	switch (pType->GetCategory())
		{
		case itemcatArmor:
			return 1;

		case itemcatLauncher:
		case itemcatWeapon:
			return 2;

		case itemcatMissile:
			return 3;

		case itemcatShields:
			return 4;

		case itemcatMiscDevice:
		case itemcatCargoHold:
		case itemcatReactor:
		case itemcatDrive:
			return 5;

		case itemcatUseful:
			return 6;

		case itemcatFuel:
			return 7;

		default:
			return 8;
		}
	}

void OutputByAttribute (SItemTableCtx &Ctx, const SItemTypeList &ItemList)
	{
	int i, j;

	//	Make a categorized list by attribute

	SByAttributeTypeList ByAttributeTable;
	for (i = 0; i < ItemList.GetCount(); i++)
		{
		const CString &sKey = ItemList.GetKey(i);
		CItemType *pType = ItemList[i];

		//	Loop over all attributes

		TArray<CString> Attribs;
		ParseAttributes(pType->GetAttributes(), &Attribs);
		for (j = 0; j < Attribs.GetCount(); j++)
			{
			bool bNew;
			SAttributeEntry *pEntry = ByAttributeTable.SetAt(Attribs[j], &bNew);
			if (bNew)
				pEntry->sAttribute = Attribs[j];

			pEntry->ItemTable.Insert(sKey, pType);
			}

		//	If no attribute

		if (Attribs.GetCount() == 0)
			{
			bool bNew;
			SAttributeEntry *pEntry = ByAttributeTable.SetAt(CONSTLIT("(none)"), &bNew);
			if (bNew)
				pEntry->sAttribute = CONSTLIT("(none)");

			pEntry->ItemTable.Insert(sKey, pType);
			}
		}

	//	Now loop over all attributes

	for (i = 0; i < ByAttributeTable.GetCount(); i++)
		{
		const SAttributeEntry &Entry = ByAttributeTable[i];
		printf("%s\n\n", Entry.sAttribute.GetASCIIZPointer());

		OutputHeader(Ctx);
		OutputTable(Ctx, Entry.ItemTable);
		printf("\n");
		}
	}

void OutputByComponent (SItemTableCtx &Ctx, const SItemTypeList &ItemList)
	{
	int i, j;

	//	For each component, make a list of which items require that component.

	SByAttributeTypeList ByComponentTable;
	for (i = 0; i < ItemList.GetCount(); i++)
		{
		const CString &sKey = ItemList.GetKey(i);
		CItemType *pType = ItemList[i];

		//	Loop over all components

		const CItemList &Components = pType->GetComponents();
		for (j = 0; j < Components.GetCount(); j++)
			{
			const CItem &ComponentItem = Components.GetItem(j);
			CString sUNID = strPatternSubst(CONSTLIT("%08x: %s"), ComponentItem.GetType()->GetUNID(), ComponentItem.GetNounPhrase(CItemCtx()));
			bool bNew;
			SAttributeEntry *pEntry = ByComponentTable.SetAt(sUNID, &bNew);
			if (bNew)
				pEntry->sAttribute = sUNID;

			pEntry->ItemTable.Insert(sKey, pType);
			}

		//	If no components

		if (Components.GetCount() == 0)
			{
			bool bNew;
			SAttributeEntry *pEntry = ByComponentTable.SetAt(CONSTLIT("(none)"), &bNew);
			if (bNew)
				pEntry->sAttribute = CONSTLIT("(none)");

			pEntry->ItemTable.Insert(sKey, pType);
			}
		}

	//	Now loop over all attributes

	for (i = 0; i < ByComponentTable.GetCount(); i++)
		{
		const SAttributeEntry &Entry = ByComponentTable[i];
		printf("%s\n\n", Entry.sAttribute.GetASCIIZPointer());

		OutputHeader(Ctx);
		OutputTable(Ctx, Entry.ItemTable);
		printf("\n");
		}
	}

void OutputByShipClass (SItemTableCtx &Ctx, const SItemTypeList &ItemList, bool bShowUsage)
	{
	int i, j;

	//	Make a map of ship classes for each item

	TSortMap<DWORD, TArray<CShipClass *>> ItemToShipClass;
	for (i = 0; i < g_pUniverse->GetShipClassCount(); i++)
		{
		CShipClass *pClass = g_pUniverse->GetShipClass(i);

		//	Skip non-generic ones

		if (!pClass->HasLiteralAttribute(CONSTLIT("genericClass")))
			continue;

		//	Add the list of types used by the ship

		TSortMap<DWORD, bool> TypesUsed;
		pClass->AddTypesUsed(&TypesUsed);

		//	For each item type, add it to the map

		for (j = 0; j < TypesUsed.GetCount(); j++)
			{
			CDesignType *pType = g_pUniverse->FindDesignType(TypesUsed.GetKey(j));
			if (pType && pType->GetType() == designItemType)
				{
				TArray<CShipClass *> *pList = ItemToShipClass.SetAt(pType->GetUNID());
				pList->Insert(pClass);
				}
			}
		}

	//	If we want to show usage, then we print each item along with the 
	//	ship classes using each item.

	if (bShowUsage)
		{
		for (i = 0; i < ItemList.GetCount(); i++)
			{
			CItemType *pType = ItemList[i];
			printf("%s\n", (LPSTR)pType->GetNounPhrase());

			TArray<CShipClass *> *pList = ItemToShipClass.SetAt(pType->GetUNID());
			for (j = 0; j < pList->GetCount(); j++)
				printf("\t%s\n", (LPSTR)pList->GetAt(j)->GetNounPhrase());

			if (pList->GetCount() == 0)
				printf("\t(none)\n");

			printf("\n");
			}
		}

	//	Otherwise we categorize by ship class

	else
		{
		//	Now make a list of all ship classes that have our items

		SByShipClassTypeList ByShipClassTable;
		for (i = 0; i < ItemList.GetCount(); i++)
			{
			const CString &sKey = ItemList.GetKey(i);
			CItemType *pType = ItemList[i];

			//	Loop over all ship classes

			TArray<CShipClass *> *pList = ItemToShipClass.SetAt(pType->GetUNID());
			for (j = 0; j < pList->GetCount(); j++)
				{
				CString sClassName = pList->GetAt(j)->GetNounPhrase();

				bool bNew;
				SShipClassEntry *pEntry = ByShipClassTable.SetAt(sClassName, &bNew);
				if (bNew)
					pEntry->sShipClassName = sClassName;

				pEntry->ItemTable.Insert(sKey, pType);
				}

			//	If no ship class

			if (pList->GetCount() == 0)
				{
				bool bNew;
				SShipClassEntry *pEntry = ByShipClassTable.SetAt(CONSTLIT("(none)"), &bNew);
				if (bNew)
					pEntry->sShipClassName = CONSTLIT("(none)");

				pEntry->ItemTable.Insert(sKey, pType);
				}
			}

		//	Now loop over all attributes

		for (i = 0; i < ByShipClassTable.GetCount(); i++)
			{
			const SShipClassEntry &Entry = ByShipClassTable[i];
			printf("%s\n\n", Entry.sShipClassName.GetASCIIZPointer());

			OutputHeader(Ctx);
			OutputTable(Ctx, Entry.ItemTable);
			printf("\n");
			}
		}
	}

void OutputHeader (SItemTableCtx &Ctx)
	{
	int i;

	for (i = 0; i < Ctx.Cols.GetCount(); i++)
		{
		if (i != 0)
			printf("\t");

        if (strEquals(Ctx.Cols[i], FIELD_BENCHMARK))
            printf("averageTime\tbestArmor\tbestArmorTime\tworstArmor\tworstArmorTime");
		else if (strEquals(Ctx.Cols[i], FIELD_BALANCE_STATS))
			{
			if (Ctx.bArmorBalanceStats)
				printf("balance\t"
					"balanceExcludeCost\t"
					"balHP\t"
					"balDamageAdj\t"
					"balDamageEffectAdj\t"
					"balRegen\t"
					"balPower\t"
					"balRepair\t"
					"balArmorComplete\t"
					"balStealth\t"
					"balSpeedAdj\t"
					"balDeviceBonus\t"
					"balMass\t"
					"balCost");
			else if (Ctx.bShieldBalanceStats)
				printf("balance\t"
					"balanceExcludeCost\t"
					"balHP\t"
					"balRegen\t"
					"balDamageAdj\t"
					"balRecovery\t"
					"balPower\t"
					"balSlots\t"
					"balCost");
			else
				printf("balance\t"
					"balanceExcludeCost\t"
					"balDamage\t"
					"balDamageType\t"
					"balAmmo\t"
					"balOmni\t"
					"balTracking\t"
					"balRange\t"
					"balSpeed\t"
					"balWMD\t"
					"balRadiation\t"
					"balMining\t"
					"balShatter\t"
					"balDeviceDisrupt\t"
					"balDeviceDamage\t"
					"balDisintegrate\t"
					"balShieldPenetrate\t"
					"balArmorDamage\t"
					"balShieldDamage\t"
					"balProjectileHP\t"
					"balPower\t"
					"balCost\t"
					"balSlots\t"
					"balExternal\t"
					"balLinkedFire\t"
					"balRecoil");
			}
        else
		    printf(Ctx.Cols[i].GetASCIIZPointer());
		}

	printf("\n");
	}

void OutputTable (SItemTableCtx &Ctx, const SItemTypeList &ItemList)
	{
	int i, j;

	if (ItemList.GetCount() == 0)
		return;

	//	Output each row

	for (i = 0; i < ItemList.GetCount(); i++)
		{
		CItemType *pType = ItemList[i];
		CItem Item(pType, 1);
		CItemCtx ItemCtx(Item);

		for (j = 0; j < Ctx.Cols.GetCount(); j++)
			{
			if (j != 0)
				printf("\t");

			const CString &sField = Ctx.Cols[j];

            //  Handle some special fields

            if (strEquals(sField, FIELD_BENCHMARK))
                {
                CWeaponBenchmarkCtx::SStats Stats;
                if (!Ctx.WeaponBenchmarks.GetStats(pType, Stats))
                    {
                    printf("\t\t\t\t");
                    }
                else
                    {
                    CString sBestArmor;
                    if (Stats.pBestArmor)
                        {
                        CItem BestArmor(Stats.pBestArmor, 1);
                        sBestArmor = BestArmor.GetNounPhrase(ItemCtx, nounShort);
                        }

                    CString sWorstArmor;
                    if (Stats.pWorstArmor)
                        {
                        CItem WorstArmor(Stats.pWorstArmor, 1);
                        sWorstArmor = WorstArmor.GetNounPhrase(ItemCtx, nounShort);
                        }

                    printf("%d\t%s\t%d\t%s\t%d",
                            Stats.iAverageTime,
                            (LPSTR)sBestArmor,
                            Stats.iBestTime,
                            (LPSTR)sWorstArmor,
                            Stats.iWorstTime);
                    }
                }

            else if (strEquals(sField, FIELD_BALANCE_STATS))
                {
				if (Ctx.bArmorBalanceStats)
					{
					CArmorClass *pArmor = pType->GetArmorClass();
					if (pArmor)
						{
						CArmorClass::SBalance Balance;
						pArmor->CalcBalance(ItemCtx, Balance);
						printf("%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f",
								Balance.rBalance,
								Balance.rBalance - Balance.rCost,
								Balance.rHPBalance,
								Balance.rDamageAdj,
								Balance.rDamageEffectAdj,
								Balance.rRegen,
								Balance.rPowerUse,
								Balance.rRepairAdj,
								Balance.rArmorComplete,
								Balance.rStealth,
								Balance.rSpeedAdj,
								Balance.rDeviceBonus,
								Balance.rMass,
								Balance.rCost
								);
						}
					else
						printf("\t\t\t\t\t\t\t\t\t\t\t\t\t");
					}
				else if (Ctx.bShieldBalanceStats)
					{
					CDeviceClass *pDevice = pType->GetDeviceClass();
					CShieldClass *pShields = (pDevice ? pDevice->AsShieldClass() : NULL);

					if (pShields)
						{
						CShieldClass::SBalance Balance;
						pShields->CalcBalance(ItemCtx, Balance);
						printf("%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f",
							Balance.rBalance,
							Balance.rBalance - Balance.rCost,
							Balance.rHPBalance,
							Balance.rRegen,
							Balance.rDamageAdj,
							Balance.rRecoveryAdj,
							Balance.rPowerUse,
							Balance.rSlots,
							Balance.rCost
						);
						}
					else
						printf("\t\t\t\t\t\t\t\t\t");
					}
				else
					{
					CDeviceClass *pDevice = pType->GetDeviceClass();
					CWeaponClass *pWeapon = NULL;

					if (pDevice)
						pWeapon = pDevice->AsWeaponClass();
					else if (pType->IsMissile() && ItemCtx.ResolveVariant())
						{
						pDevice = ItemCtx.GetVariantDevice();
						pWeapon = pDevice->AsWeaponClass();
						}

					if (pWeapon)
						{
						CWeaponClass::SBalance Balance;
						pWeapon->CalcBalance(ItemCtx, Balance);
						printf("%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f",
								Balance.rBalance,
								Balance.rBalance - Balance.rCost,
								Balance.rDamage,
								Balance.rDamageType,
								Balance.rAmmo,
								Balance.rOmni,
								Balance.rTracking,
								Balance.rRange,
								Balance.rSpeed,
								Balance.rWMD,
								Balance.rRadiation,
								Balance.rMining,
								Balance.rShatter,
								Balance.rDeviceDisrupt,
								Balance.rDeviceDamage,
								Balance.rDisintegration,
								Balance.rShieldPenetrate,
								Balance.rArmor,
								Balance.rShield,
								Balance.rProjectileHP,
								Balance.rPower,
								Balance.rCost,
								Balance.rSlots,
								Balance.rExternal,
								Balance.rLinkedFire,
								Balance.rRecoil
								);
						}
					else
						printf("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");
					}
                }

			else if (strEquals(sField, FIELD_PROBABILITY))
				{
				Metric rProb = 100.0 * Ctx.ProbTable.GetProbability(pType);
				printf("%.2f%%", rProb);
				}

			//	Get the field value

            else
                {
			    CCodeChainCtx CCCtx;
			    CString sValue = Item.GetItemPropertyString(CCCtx, ItemCtx, sField);

			    //	Format the value

			    if (strEquals(sField, FIELD_POWER_PER_SHOT))
				    printf("%.2f", strToInt(sValue, 0, NULL) / 1000.0);
			    else if (strEquals(sField, FIELD_POWER))
				    printf("%.1f", strToInt(sValue, 0, NULL) / 1000.0);
			    else if (strEquals(sField, FIELD_TOTAL_COUNT))
				    {
				    SDesignTypeInfo *pInfo = Ctx.TotalCount.GetAt(pType->GetUNID());
				    double rCount = (pInfo ? pInfo->rPerGameMeanCount : 0.0);
				    printf("%.2f", rCount);
				    }
			    else
				    printf(sValue.GetASCIIZPointer());
                }
			}

		printf("\n");
		}
	}

void SelectByCriteria (SItemTableCtx &Ctx, const CString &sCriteria, TArray<CItemType *> *retList)
	{
	int i;

	Ctx.bHasArmor = false;
	Ctx.bHasShields = false;

	//	Compute the criteria

	CItemCriteria Crit;
	if (!sCriteria.IsBlank())
		CItem::ParseCriteria(sCriteria, &Crit);
	else
		CItem::InitCriteriaAll(&Crit);

	//	Loop over all items that match and add them to
	//	a sorted table.

	retList->DeleteAll();
	for (i = 0; i < Ctx.pUniverse->GetItemTypeCount(); i++)
		{
		CItemType *pType = Ctx.pUniverse->GetItemType(i);
		CItem Item(pType, 1);

		if (!Item.MatchesCriteria(Crit))
			continue;

		//	Keep track of whether we've selected any armor or shields.
		//	[We need this for balance stats.]

		if (pType->GetArmorClass())
			Ctx.bHasArmor = true;
		else if (pType->GetDeviceClass() && pType->GetDeviceClass()->GetCategory() == itemcatShields)
			Ctx.bHasShields = true;

		//	Add

		retList->Insert(pType);
		}
	}

bool SelectByItemTable (SItemTableCtx &Ctx, const CString &sTable, int iLevel, TArray<CItemType *> *retList)
	{
	int i;

	CItemTable *pTable;
	DWORD dwUNID = strToInt(sTable, 0);
	if (dwUNID == 0)
		{
		dwUNID = Ctx.pUniverse->GetExtensionCollection().GetEntityValue(sTable);
		if (dwUNID == 0)
			{
			printf("Enter UNID or entity: %s\n", (LPSTR)sTable);
			return false;
			}

		pTable = Ctx.pUniverse->FindItemTable(dwUNID);
		if (pTable == NULL)
			{
			printf("Unknown item table: %s\n", (LPSTR)sTable);
			return false;
			}
		}

	//	Get the probability table

	CItemList Dummy1;
	CItemListManipulator Dummy2(Dummy1);
	SItemAddCtx ItemCtx(Dummy2);
	ItemCtx.iLevel = iLevel;

	Ctx.ProbTable = pTable->GetProbabilityTable(ItemCtx);

	//	Add items

	retList->DeleteAll();
	retList->InsertEmpty(Ctx.ProbTable.GetCount());

	for (i = 0; i < retList->GetCount(); i++)
		retList->GetAt(i) = Ctx.ProbTable.GetType(i);

	return true;
	}

void SortTable (SItemTableCtx &Ctx, const TArray<CItemType *> &List, SItemTypeList *retSorted)
	{
	int i;

	//	Loop over all items that match and add them to
	//	a sorted table.

	retSorted->DeleteAll();
	for (i = 0; i < List.GetCount(); i++)
		{
		CItemType *pType = List[i];

		//	Add with sort key

		char szBuffer[1024];
		wsprintf(szBuffer, "%02d%s%02d%s", 
				pType->GetLevel(),
				g_szTypeCode[GetItemType(pType)], 
				GetItemFreq(pType), 
				pType->GetNounPhrase().GetASCIIZPointer());
		retSorted->Insert(CString(szBuffer), pType);
		}
	}
