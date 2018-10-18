//	TypeTable.cpp
//
//	Generate basic stats about any type

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define CRITERIA_ATTRIB						CONSTLIT("criteria")

#define FIELD_COUNT_DISTRIBUTION			CONSTLIT("countDistribution")
#define FIELD_LEVEL							CONSTLIT("level")
#define FIELD_NAME							CONSTLIT("name")
#define FIELD_TOTAL_COUNT					CONSTLIT("totalCount")

void GenerateTypeTable (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i, j;

	//	Get the criteria from the command line.

	CString sCriteria = pCmdLine->GetAttribute(CRITERIA_ATTRIB);

	//	Parse it

	CDesignTypeCriteria Criteria;
	if (CDesignTypeCriteria::ParseCriteria(sCriteria, &Criteria) != NOERROR)
		{
		printf("ERROR: Unable to parse criteria.\n");
		return;
		}

	//	Generate a table of all matching types

	TSortMap<CString, CDesignType *> Table;

	//	Loop over all items for this level and add them to
	//	a sorted table.

	for (i = 0; i < Universe.GetDesignTypeCount(); i++)
		{
		CDesignType *pType = Universe.GetDesignType(i);
		int iLevel = pType->GetLevel();

		if (!pType->MatchesCriteria(Criteria))
			continue;

		//	Get the name

		CString sName = pType->GetDataField(FIELD_NAME);
		if (*sName.GetASCIIZPointer() == '(')
			sName = strSubString(sName, 1, -1);

		//	Figure out the sort order

		char szBuffer[1024];
		wsprintf(szBuffer, "%02d%s%08x", 
				iLevel,
				sName.GetASCIIZPointer(),
				pType->GetUNID());
		Table.Insert(CString(szBuffer), pType);
		}

	//	Generate a list of columns to display

	TArray<CString> Cols;
	Cols.Insert(FIELD_LEVEL);
	Cols.Insert(FIELD_NAME);

	for (i = 0; i < pCmdLine->GetAttributeCount(); i++)
		{
		CString sAttrib = pCmdLine->GetAttributeName(i);

		if (!IsMainCommandParam(sAttrib)
				&& !strEquals(sAttrib, CONSTLIT("typetable")))
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
		if (LoadDesignTypeStats(Universe.GetDesignCollection().GetAdventureUNID(), &TotalCount) != NOERROR)
			{
			printf("ERROR: Unable to load type count table.\n");
			return;
			}
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
			CDesignType *pType = Table[i];

			for (j = 0; j < Cols.GetCount(); j++)
				{
				if (j != 0)
					printf("\t");

				const CString &sField = Cols[j];

				//	Format and output

				if (strEquals(sField, FIELD_TOTAL_COUNT))
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
					ICCItem *pResult = pType->GetProperty(CCCtx, sField);
					CString sValue = pResult->Print(&g_pUniverse->GetCC(), PRFLAG_NO_QUOTES | PRFLAG_ENCODE_FOR_DISPLAY);
					pResult->Discard(&g_pUniverse->GetCC());

					printf(sValue.GetASCIIZPointer());
					}
				}

			printf("\n");
			}

		printf("\n");
		}
	else
		printf("No entries match criteria.\n");
	}
