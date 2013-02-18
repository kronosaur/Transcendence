//	ItemTable.cpp
//
//	Generate basic stats about items

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define BY_ATTRIBUTE_ATTRIB					CONSTLIT("byAttribute")
#define CRITERIA_ATTRIB						CONSTLIT("criteria")
#define GENERATE_ACTUAL_COUNT_ATTRIB		CONSTLIT("generateSimTables")

#define FIELD_AVERAGE_DAMAGE				CONSTLIT("averageDamage")
#define FIELD_ENTITY						CONSTLIT("entity")
#define FIELD_FREQUENCY						CONSTLIT("frequency")
#define FIELD_LEVEL							CONSTLIT("level")
#define FIELD_NAME							CONSTLIT("shortName")
#define FIELD_POWER							CONSTLIT("power")
#define FIELD_POWER_PER_SHOT				CONSTLIT("powerPerShot")
#define FIELD_TOTAL_COUNT					CONSTLIT("totalCount")
#define FIELD_TYPE							CONSTLIT("category")

#define TOTAL_COUNT_FILENAME				CONSTLIT("TransData_ItemCount.txt")

char *g_szTypeCode[] =
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

char *g_szFreqCode[] =
	{
	"",	"C", "UC", "R", "VR", "NR",
	};

struct SItemTableCtx
	{
	SItemTableCtx (void) :
			pUniverse(NULL),
			pCmdLine(NULL),
			pEntityTable(NULL)
		{ }

	CUniverse *pUniverse;
	CXMLElement *pCmdLine;
	CIDTable *pEntityTable;

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

bool CalcColumns (SItemTableCtx &Ctx, CXMLElement *pCmdLine);
int GetItemFreq (CItemType *pType);
int GetItemType (CItemType *pType);
void OutputByAttribute (SItemTableCtx &Ctx, const SItemTypeList &ItemList);
void OutputHeader (SItemTableCtx &Ctx);
void OutputTable (SItemTableCtx &Ctx, const SItemTypeList &ItemList);
void SelectByCriteria (SItemTableCtx &Ctx, const CString &sCriteria, TArray<CItemType *> *retList);
void SortTable (SItemTableCtx &Ctx, const TArray<CItemType *> &List, SItemTypeList *retSorted);

void GenerateItemTable (CUniverse &Universe, CXMLElement *pCmdLine, CIDTable &EntityTable)
	{
	//	Create the context

	SItemTableCtx Ctx;
	Ctx.pUniverse = &Universe;
	Ctx.pCmdLine = pCmdLine;
	Ctx.pEntityTable = &EntityTable;

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

		if (!strEquals(sAttrib, CONSTLIT("adventure"))
				&& !strEquals(sAttrib, BY_ATTRIBUTE_ATTRIB)
				&& !strEquals(sAttrib, CONSTLIT("itemtable"))
				&& !strEquals(sAttrib, CONSTLIT("criteria"))
				&& !strEquals(sAttrib, CONSTLIT("nologo")))
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
			if (strEquals(sField, FIELD_ENTITY))
				{
				CString *pValue;
				if (Ctx.pEntityTable->Lookup(pType->GetUNID(), (CObject **)&pValue) == NOERROR)
					sValue = *pValue;
				else
					sValue = CONSTLIT("?");
				}
			else
				sValue = pType->GetDataField(sField);

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
