//	EncounterTable.cpp
//
//	Generate basic stats about encounters

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define CRITERIA_ATTRIB						CONSTLIT("criteria")
#define ALL_ATTRIB							CONSTLIT("all")
#define ONLY_NOT_RANDOM_ATTRIB				CONSTLIT("onlyNotRandom")

#define FIELD_ARMOR_CLASS					CONSTLIT("armorClass")
#define FIELD_CAN_ATTACK					CONSTLIT("canAttack")
#define FIELD_CATEGORY						CONSTLIT("category")
#define FIELD_COUNT_DISTRIBUTION			CONSTLIT("countDistribution")
#define FIELD_EXPLOSION_TYPE				CONSTLIT("explosionType")
#define FIELD_FIRE_RATE_ADJ					CONSTLIT("fireRateAdj")
#define FIELD_HP							CONSTLIT("hp")
#define FIELD_LEVEL							CONSTLIT("level")
#define FIELD_NAME							CONSTLIT("name")
#define FIELD_TOTAL_COUNT					CONSTLIT("totalCount")

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
	bool bOnlyNotRandom = pCmdLine->GetAttributeBool(ONLY_NOT_RANDOM_ATTRIB);

	//	Generate a table of all matching encounters

	CSymbolTable Table(FALSE, TRUE);

	//	Loop over all items for this level and add them to
	//	a sorted table.

	for (i = 0; i < Universe.GetStationTypeCount(); i++)
		{
		CStationType *pType = Universe.GetStationType(i);
		int iLevel = pType->GetLevel();

		//	If we're selecting all types, then do it

		if (bAll)
			;

		//	If we only want non-random types, then skip any random encounters.

		else if (bOnlyNotRandom)
			{
			if (pType->CanBeEncounteredRandomly())
				continue;
			}

		//	Otherwise we skip anything except random encounters.

		else
			{
			if (!pType->CanBeEncounteredRandomly())
				continue;
			}

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
		wsprintf(szBuffer, "%02d%s%s%08x", 
				iLevel,
				sCategory.GetASCIIZPointer(), 
				sName.GetASCIIZPointer(),
				pType->GetUNID());
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

		if (!IsMainCommandParam(sAttrib)
				&& !strEquals(sAttrib, ONLY_NOT_RANDOM_ATTRIB)
				&& !strEquals(sAttrib, CONSTLIT("encountertable")))
			{
			CString sValue = pCmdLine->GetAttribute(i);
			
			if (!strEquals(sValue, CONSTLIT("true")))
				Cols.Insert(strPatternSubst(CONSTLIT("%s:%s"), sAttrib, sValue));
			else
				Cols.Insert(sAttrib);
			}
		}

	//	If we need to output total count, then load the table

	CDesignTypeStats TotalCount;
	if (pCmdLine->GetAttributeBool(FIELD_TOTAL_COUNT)
			|| pCmdLine->GetAttributeBool(FIELD_COUNT_DISTRIBUTION))
		{
		if (error = LoadDesignTypeStats(Universe.GetDesignCollection().GetAdventureUNID(), &TotalCount))
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

    	CCodeChainCtx CCCtx;
		for (i = 0; i < Table.GetCount(); i++)
			{
			CStationType *pType = (CStationType *)Table.GetValue(i);

			for (j = 0; j < Cols.GetCount(); j++)
				{
				if (j != 0)
					printf("\t");


				//	Get the value

				const CString &sField = Cols[j];
    			ICCItem *pResult = pType->GetProperty(CCCtx, sField);

				//	Format and output

				if (strEquals(sField, FIELD_FIRE_RATE_ADJ))
					printf("%.2f", pResult->GetIntegerValue() / 1000.0);
				else if (strEquals(sField, FIELD_TOTAL_COUNT))
					{
					SDesignTypeInfo *pInfo = TotalCount.GetAt(pType->GetUNID());
					double rCount = (pInfo ? pInfo->rPerGameMeanCount : 0.0);
					printf("%.2f", rCount);
					}
				else if (strEquals(sField, FIELD_COUNT_DISTRIBUTION))
					{
					SDesignTypeInfo *pInfo = TotalCount.GetAt(pType->GetUNID());
					printf("%s", (pInfo ? pInfo->sDistribution : NULL_STR).GetASCIIZPointer());
					}
				else
				    {
				    CString sValue = pResult->Print(&g_pUniverse->GetCC(), PRFLAG_NO_QUOTES | PRFLAG_ENCODE_FOR_DISPLAY);
				    printf(sValue.GetASCIIZPointer());
				    }

                pResult->Discard(&g_pUniverse->GetCC());
				}

			printf("\n");
			}

		printf("\n");
		}
	else
		printf("No entries match criteria.\n");
	}

