//	ItemTable.cpp
//
//	Generate basic stats about items

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define BY_ATTRIBUTE_ATTRIB					CONSTLIT("byAttribute")
#define BY_SHIP_CLASS_ATTRIB				CONSTLIT("byShipClass")
#define BY_SHIP_CLASS_USAGE_ATTRIB			CONSTLIT("byShipClassUsage")
#define CRITERIA_ATTRIB						CONSTLIT("criteria")
#define GENERATE_ACTUAL_COUNT_ATTRIB		CONSTLIT("generateSimTables")

#define FIELD_AVERAGE_DAMAGE				CONSTLIT("averageDamage")
#define FIELD_TYPE							CONSTLIT("category")
#define FIELD_FREQUENCY						CONSTLIT("frequency")
#define FIELD_LEVEL							CONSTLIT("level")
#define FIELD_NAME							CONSTLIT("shortName")
#define FIELD_POWER							CONSTLIT("power")
#define FIELD_POWER_PER_SHOT				CONSTLIT("powerPerShot")
#define FIELD_TOTAL_COUNT					CONSTLIT("totalCount")

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
	SItemTableCtx (void) :
			pUniverse(NULL),
			pCmdLine(NULL)
		{ }

	CUniverse *pUniverse;
	CXMLElement *pCmdLine;

	TArray<CString> Cols;
	CDesignTypeStats TotalCount;
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
void OutputByShipClass (SItemTableCtx &Ctx, const SItemTypeList &ItemList, bool bShowUsage);
void OutputHeader (SItemTableCtx &Ctx);
void OutputTable (SItemTableCtx &Ctx, const SItemTypeList &ItemList);
void SelectByCriteria (SItemTableCtx &Ctx, const CString &sCriteria, TArray<CItemType *> *retList);
void SortTable (SItemTableCtx &Ctx, const TArray<CItemType *> &List, SItemTypeList *retSorted);

void GenerateItemTable (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	//	Create the context

	SItemTableCtx Ctx;
	Ctx.pUniverse = &Universe;
	Ctx.pCmdLine = pCmdLine;

	//	Compute the criteria

	TArray<CItemType *> Selection;
	SelectByCriteria(Ctx, pCmdLine->GetAttribute(CRITERIA_ATTRIB), &Selection);
	if (Selection.GetCount() == 0)
		{
		printf("No entries match criteria.\n");
		return;
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

	//	Add fields as columns

	for (i = 0; i < pCmdLine->GetAttributeCount(); i++)
		{
		CString sAttrib = pCmdLine->GetAttributeName(i);

		if (!IsMainCommandParam(sAttrib)
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
		if (LoadDesignTypeStats(&Ctx.TotalCount) != NOERROR)
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
				printf("\t%s\n", (LPSTR)pList->GetAt(j)->GetName());

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
				CString sClassName = pList->GetAt(j)->GetName();

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

		for (j = 0; j < Ctx.Cols.GetCount(); j++)
			{
			if (j != 0)
				printf("\t");

			const CString &sField = Ctx.Cols[j];

			//	Get the field value

			CString sValue;
			CItem Item(pType, 1);
			CItemCtx ItemCtx(Item);
			CCodeChainCtx CCCtx;

			ICCItem *pResult = Item.GetProperty(&CCCtx, ItemCtx, sField);

			if (pResult->IsNil())
				sValue = NULL_STR;
			else
				sValue = pResult->Print(&g_pUniverse->GetCC(), PRFLAG_NO_QUOTES | PRFLAG_ENCODE_FOR_DISPLAY);

			pResult->Discard(&g_pUniverse->GetCC());

			//	Format the value

			if (strEquals(sField, FIELD_AVERAGE_DAMAGE) || strEquals(sField, FIELD_POWER_PER_SHOT))
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

		printf("\n");
		}
	}

void SelectByCriteria (SItemTableCtx &Ctx, const CString &sCriteria, TArray<CItemType *> *retList)
	{
	int i;

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

		retList->Insert(pType);
		}
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
