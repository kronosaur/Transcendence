//	TradeTable.cpp
//
//	Generate basic stats about trading

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define CRITERIA_ATTRIB						CONSTLIT("criteria")

#define FIELD_LEVEL							CONSTLIT("level")
#define FIELD_NAME							CONSTLIT("name")

void PrintTradeDesc (CDesignType *pType, CTradingDesc *pTrade);

void GenerateTradeTable (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i;

	//	Get the criteria from the command line.

	CString sCriteria;
	if (!pCmdLine->FindAttribute(CRITERIA_ATTRIB, &sCriteria))
		sCriteria = CONSTLIT("*");

	//	Parse it

	CDesignTypeCriteria Criteria;
	if (CDesignTypeCriteria::ParseCriteria(sCriteria, &Criteria) != NOERROR)
		{
		printf("ERROR: Unable to parse criteria.\n");
		return;
		}

	//	Generate a table of all matching types

	TSortMap<CString, CDesignType *> TypeTable;

	//	Loop over all items for this level and add them to
	//	a sorted table.

	for (i = 0; i < Universe.GetDesignTypeCount(); i++)
		{
		CDesignType *pType = Universe.GetDesignType(i);
		int iLevel = pType->GetLevel();

		if (!pType->MatchesCriteria(Criteria))
			continue;

		//	If we have no tradedesc, then skip

		if (pType->GetTradingDesc() == NULL)
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
		TypeTable.Insert(CString(szBuffer), pType);
		}

	//	Header

	printf("Level\tName\tService\tCriteria\tPriceAdj\n");

	//	Now loop in sorted order and output the trading descriptors

	for (i = 0; i < TypeTable.GetCount(); i++)
		{
		CDesignType *pType = TypeTable[i];
		CTradingDesc *pTrade = pType->GetTradingDesc();

		PrintTradeDesc(pType, pTrade);
		}
	}

void PrintTradeDesc (CDesignType *pType, CTradingDesc *pTrade)
	{
	int i;

	for (i = 0; i < pTrade->GetServiceCount(); i++)
		{
		CTradingDesc::SServiceInfo Info;
		pTrade->GetServiceInfo(i, Info);

		//	Figure what to show as a criteria

		CString sCriteria;
		if (Info.pItemType)
			sCriteria = Info.pItemType->GetNounPhrase();
		else if (Info.sItemCriteria)
			sCriteria = Info.sItemCriteria;
		else
			sCriteria = Info.sTypeCriteria;

		//	Print line

		printf("%d\t%s\t%s\t%s\t%d\n",
			pType->GetLevel(),
			(LPSTR)pType->GetDataField(FIELD_NAME),
			(LPSTR)CTradingDesc::ServiceToString(Info.iService),
			(LPSTR)sCriteria,
			Info.iPriceAdj
			);
		}
	}
