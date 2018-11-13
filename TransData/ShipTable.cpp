//	ShipTable.cpp
//
//	Generate basic stats about ship classes

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define MAX_LEVEL			25

#define FIELD_BALANCE							CONSTLIT("balance")
#define FIELD_HULL_POINTS_STATS					CONSTLIT("hullPointsStats")
#define FIELD_LEVEL								CONSTLIT("level")
#define FIELD_MANEUVER							CONSTLIT("maneuver")
#define FIELD_NAME								CONSTLIT("name")
#define FIELD_SCORE								CONSTLIT("score")
#define FIELD_SCORE_CALC						CONSTLIT("scoreCalc")
#define FIELD_THRUST_TO_WEIGHT					CONSTLIT("thrustToWeight")

void GenerateShipTable (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i, j;

	//	Some options

	bool bAllClasses = (pCmdLine->GetAttributeBool(CONSTLIT("allClasses")) || pCmdLine->GetAttributeBool(CONSTLIT("all")));

	//	Get the criteria from the command line. Always append 's' because we
	//	want ship classes.

	CString sCriteria = strPatternSubst(CONSTLIT("%s; s"), pCmdLine->GetAttribute(CONSTLIT("criteria")));
	CDesignTypeCriteria Criteria;
	if (CDesignTypeCriteria::ParseCriteria(sCriteria, &Criteria) != NOERROR)
		{
		printf("ERROR: Unable to parse criteria.\n");
		return;
		}

	//	Generate a list of columns to display

	TArray<CString> Cols;
	Cols.Insert(FIELD_LEVEL);
	Cols.Insert(FIELD_NAME);

	for (i = 0; i < pCmdLine->GetAttributeCount(); i++)
		{
		CString sAttrib = pCmdLine->GetAttributeName(i);

		if (strEquals(sAttrib, FIELD_BALANCE))
			{
			Cols.Insert(CONSTLIT("balanceType"));
			Cols.Insert(CONSTLIT("combatStrength"));
			Cols.Insert(CONSTLIT("damage"));
			Cols.Insert(CONSTLIT("defenseStrength"));
			}
		else if (!IsMainCommandParam(sAttrib)
				&& !strEquals(sAttrib, CONSTLIT("shiptable")))
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

		if (strEquals(Cols[j], FIELD_HULL_POINTS_STATS))
			{
			for (int k = 0; k < CHullPointsCalculator::fieldCount; k++)
				{
				if (k != 0)
					printf("\t");

				printf((LPSTR)CHullPointsCalculator::GetFieldName(k));
				}
			}
		else
			printf(Cols[j].GetASCIIZPointer());
		}

	printf("\n");

	//	Generate a table

	CSymbolTable Table(FALSE, TRUE);

	//	Loop over all items that match and add them to
	//	a sorted table.

	for (i = 0; i < Universe.GetShipClassCount(); i++)
		{
		CShipClass *pClass = Universe.GetShipClass(i);

		//	Only include generic classes unless otherwise specified

		if (!bAllClasses && !pClass->HasLiteralAttribute(CONSTLIT("genericClass")))
			continue;

		if (!pClass->MatchesCriteria(Criteria))
			continue;

		//	Figure out the sort order

		char szBuffer[1024];
		wsprintf(szBuffer, "%04d%s%d", 
				pClass->GetLevel(),
				pClass->GetNounPhrase(nounGeneric).GetASCIIZPointer(),
				pClass->GetUNID());
		Table.AddEntry(CString(szBuffer), (CObject *)pClass);
		}

	//	Output table

	CCodeChainCtx CCCtx;
	for (i = 0; i < Table.GetCount(); i++)
		{
		CShipClass *pClass = (CShipClass *)Table.GetValue(i);

		//	Output each row

		for (j = 0; j < Cols.GetCount(); j++)
			{
			if (j != 0)
				printf("\t");

			const CString &sField = Cols[j];
			ICCItem *pResult = pClass->GetProperty(CCCtx, sField);

			if (strEquals(sField, FIELD_MANEUVER) 
					|| strEquals(sField, FIELD_THRUST_TO_WEIGHT))
				printf("%.1f", pResult->GetIntegerValue() / 1000.0);
			else if (strEquals(sField, FIELD_SCORE_CALC))
				printf("%d", pClass->CalcScore());
			else if (strEquals(sField, FIELD_HULL_POINTS_STATS))
				{
				CHullPointsCalculator Calc(*pClass);

				for (int k = 0; k < Calc.GetFieldCount(); k++)
					{
					if (k != 0)
						printf("\t");

					printf("%.1f", Calc.GetField(k) * 10.0);
					}
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
