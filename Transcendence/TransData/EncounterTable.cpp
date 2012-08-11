//	EncounterTable.cpp
//
//	Generate basic stats about encounters

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define CRITERIA_ATTRIB						CONSTLIT("criteria")
#define ALL_ATTRIB							CONSTLIT("all")

#define FIELD_LEVEL							CONSTLIT("level")
#define FIELD_CATEGORY						CONSTLIT("category")
#define FIELD_NAME							CONSTLIT("name")
#define FIELD_ARMOR_CLASS					CONSTLIT("armorClass")
#define FIELD_HP							CONSTLIT("hp")
#define FIELD_FIRE_RATE_ADJ					CONSTLIT("fireRateAdj")
#define FIELD_TOTAL_COUNT					CONSTLIT("totalCount")
#define FIELD_CAN_ATTACK					CONSTLIT("canAttack")
#define FIELD_EXPLOSION_TYPE				CONSTLIT("explosionType")

#define TOTAL_COUNT_FILENAME				CONSTLIT("TransData_EncounterCount.txt")

void GenerateEncounterTable (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	ALERROR error;
	int i, j;

	//	Get the criteria from the command line. Always append 't' because we
	//	want station types.

	CString sCriteria = strPatternSubst(CONSTLIT("%s t"), pCmdLine->GetAttribute(CRITERIA_ATTRIB));

	//	Parse it

	CDesignTypeCriteria Criteria;
	if (CDesignTypeCriteria::ParseCriteria(sCriteria, &Criteria) != NOERROR)
		{
		printf("ERROR: Unable to parse criteria.\n");
		return;
		}

	bool bAll = pCmdLine->GetAttributeBool(ALL_ATTRIB);

	//	Generate a table of all matching encounters

	CSymbolTable Table(FALSE, TRUE);

	//	Loop over all items for this level and add them to
	//	a sorted table.

	for (i = 0; i < Universe.GetStationTypeCount(); i++)
		{
		CStationType *pType = Universe.GetStationType(i);
		int iLevel = pType->GetLevel();
		if (iLevel == 0 && !bAll)
			continue;

		//	If we don't match the criteria, then continue

		if (!pType->MatchesCriteria(Criteria))
			continue;

		//	Get the category and name

		CString sCategory = pType->GetDataField(FIELD_CATEGORY);
		CString sName = pType->GetDataField(FIELD_NAME);
		if (*sName.GetASCIIZPointer() == '(')
			sName = strSubString(sName, 1, -1);

		//	Figure out the sort order

		char szBuffer[1024];
		wsprintf(szBuffer, "%02d%s%s", 
				pType->GetLevel(),
				sCategory.GetASCIIZPointer(), 
				sName.GetASCIIZPointer());
		Table.AddEntry(CString(szBuffer), (CObject *)pType);
		}

	//	Generate a list of columns to display

	TArray<CString> Cols;
	Cols.Insert(FIELD_LEVEL);
	Cols.Insert(FIELD_CATEGORY);
	Cols.Insert(FIELD_NAME);

	for (i = 0; i < pCmdLine->GetAttributeCount(); i++)
		{
		CString sAttrib = pCmdLine->GetAttributeName(i);

		if (!strEquals(sAttrib, CONSTLIT("all"))
				&& !strEquals(sAttrib, CONSTLIT("criteria"))
				&& !strEquals(sAttrib, CONSTLIT("encountertable"))
				&& !strEquals(sAttrib, CONSTLIT("nologo")))
			{
			CString sValue = pCmdLine->GetAttribute(i);
			
			if (!strEquals(sValue, CONSTLIT("true")))
				Cols.Insert(strPatternSubst(CONSTLIT("%s:%s"), sAttrib, sValue));
			else
				Cols.Insert(sAttrib);
			}
		}

#if 0
	if (pCmdLine->GetAttributeBool(FIELD_ARMOR_CLASS))
		Cols.Insert(FIELD_ARMOR_CLASS);
	if (pCmdLine->GetAttributeBool(FIELD_HP))
		Cols.Insert(FIELD_HP);
	if (pCmdLine->GetAttributeBool(FIELD_FIRE_RATE_ADJ))
		Cols.Insert(FIELD_FIRE_RATE_ADJ);
	if (pCmdLine->GetAttributeBool(FIELD_TOTAL_COUNT))
		Cols.Insert(FIELD_TOTAL_COUNT);
	if (pCmdLine->GetAttributeBool(FIELD_CAN_ATTACK))
		Cols.Insert(FIELD_CAN_ATTACK);
	if (pCmdLine->GetAttributeBool(FIELD_EXPLOSION_TYPE))
		Cols.Insert(FIELD_EXPLOSION_TYPE);
#endif

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
			CStationType *pType = (CStationType *)Table.GetValue(i);

			for (j = 0; j < Cols.GetCount(); j++)
				{
				if (j != 0)
					printf("\t");

				const CString &sField = Cols[j];
				CString sValue = pType->GetDataField(sField);

				if (strEquals(sField, FIELD_FIRE_RATE_ADJ))
					printf("%.2f", strToInt(sValue, 0, NULL) / 1000.0);
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

