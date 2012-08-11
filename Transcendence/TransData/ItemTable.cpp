//	ItemTable.cpp
//
//	Generate basic stats about items

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

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

int GetItemFreq (CItemType *pType);
int GetItemType (CItemType *pType);

void GenerateItemTable (CUniverse &Universe, CXMLElement *pCmdLine, CIDTable &EntityTable)
	{
	ALERROR error;
	int i, j;

	//	Compute the criteria

	CItemCriteria Crit;
	CString sCriteria;
	if (pCmdLine->FindAttribute(CRITERIA_ATTRIB, &sCriteria))
		CItem::ParseCriteria(sCriteria, &Crit);
	else
		CItem::InitCriteriaAll(&Crit);

	//	Generate a table

	CSymbolTable Table(FALSE, TRUE);

	//	Loop over all items that match and add them to
	//	a sorted table.

	for (j = 0; j < Universe.GetItemTypeCount(); j++)
		{
		CItemType *pType = Universe.GetItemType(j);
		CItem Item(pType, 1);

		if (!Item.MatchesCriteria(Crit))
			continue;

		//	Figure out the sort order

		char szBuffer[1024];
		wsprintf(szBuffer, "%02d%s%02d%s", 
				pType->GetLevel(),
				g_szTypeCode[GetItemType(pType)], 
				GetItemFreq(pType), 
				pType->GetNounPhrase().GetASCIIZPointer());
		Table.AddEntry(CString(szBuffer), (CObject *)pType);
		}

	//	If we need to output total count, then load the table

	CSymbolTable TotalCount(TRUE, TRUE);
	if (pCmdLine->GetAttributeBool(FIELD_TOTAL_COUNT))
		{
		if (error = LoadTotalCount(TOTAL_COUNT_FILENAME, TotalCount))
			return;
		}

	//	If we've got any entries in the table, output now

	if (Table.GetCount())
		{
		//	Generate a list of columns to display

		TArray<CString> Cols;
		Cols.Insert(FIELD_LEVEL);
		Cols.Insert(FIELD_TYPE);
		Cols.Insert(FIELD_FREQUENCY);
		Cols.Insert(FIELD_NAME);

		//	Add fields as columns

		for (i = 0; i < pCmdLine->GetAttributeCount(); i++)
			{
			CString sAttrib = pCmdLine->GetAttributeName(i);

			if (!strEquals(sAttrib, CONSTLIT("adventure"))
					&& !strEquals(sAttrib, CONSTLIT("itemtable"))
					&& !strEquals(sAttrib, CONSTLIT("criteria"))
					&& !strEquals(sAttrib, CONSTLIT("nologo")))
				{
				CString sValue = pCmdLine->GetAttribute(i);
				
				if (!strEquals(sValue, CONSTLIT("true")))
					Cols.Insert(strPatternSubst(CONSTLIT("%s:%s"), sAttrib, sValue));
				else
					Cols.Insert(sAttrib);
				}
			}

		//	Output the header

		for (j = 0; j < Cols.GetCount(); j++)
			{
			if (j != 0)
				printf("\t");

			printf(Cols[j].GetASCIIZPointer());
			}

		printf("\n");

		//	Output each row

		for (i = 0; i < Table.GetCount(); i++)
			{
			CItemType *pType = (CItemType *)Table.GetValue(i);

			for (j = 0; j < Cols.GetCount(); j++)
				{
				if (j != 0)
					printf("\t");

				const CString &sField = Cols[j];

				//	Get the field value

				CString sValue;
				if (strEquals(sField, FIELD_ENTITY))
					{
					CString *pValue;
					if (EntityTable.Lookup(pType->GetUNID(), (CObject **)&pValue) == NOERROR)
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
					double rCount = 0.0;

					CString sKey = strFromInt(pType->GetUNID(), false);
					EntryInfo *pEntry;
					if (TotalCount.Lookup(sKey, (CObject **)&pEntry) == NOERROR)
						rCount = pEntry->rTotalCount;

					printf("%.2f", rCount);
					}
				else
					printf(sValue.GetASCIIZPointer());
				}

			printf("\n");
			}

		printf("\n");
		}
	else
		printf("No entries match criteria.\n");
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
