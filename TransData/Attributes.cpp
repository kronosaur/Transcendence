//	Armor.cpp
//
//	Generate stats about armor

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define CRITERIA_ATTRIB						CONSTLIT("criteria")

struct SAttribData
	{
	CString sAttrib;
	int iCount;
	DWORD dwTypeSet;
	};

void GenerateAttributeList (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i, j;

	//	Parse the criteria

	CString sCriteria = pCmdLine->GetAttribute(CRITERIA_ATTRIB);
	if (sCriteria.IsBlank())
		sCriteria = CONSTLIT("*");

	CDesignTypeCriteria Criteria;
	if (CDesignTypeCriteria::ParseCriteria(sCriteria, &Criteria) != NOERROR)
		{
		printf("TransData: Invalid criteria: %s\n", sCriteria.GetASCIIZPointer());
		return;
		}

	printf("Attribute\tCount\tTypes\n");

	TSortMap<CString, SAttribData> Table;

	//	Generate table

	for (i = 0; i < g_pUniverse->GetDesignTypeCount(); i++)
		{
		CDesignType *pType = g_pUniverse->GetDesignType(i);

		//	See if the type matches the criteria

		if (!pType->MatchesCriteria(Criteria))
			continue;

		//	Parse the attributes

		TArray<CString> Attribs;
		ParseAttributes(pType->GetAttributes(), &Attribs);

		for (j = 0; j < Attribs.GetCount(); j++)
			{
			bool bNew;
			SAttribData *pData = Table.SetAt(Attribs[j], &bNew);
			if (bNew)
				{
				pData->sAttrib = Attribs[j];
				pData->iCount = 0;
				pData->dwTypeSet = 0;
				}

			pData->iCount++;
			pData->dwTypeSet |= (1 << (DWORD)pType->GetType());
			}
		}

	//	Output results

	for (i = 0; i < Table.GetCount(); i++)
		{
		printf("%s\t%d\t", Table[i].sAttrib.GetASCIIZPointer(), Table[i].iCount);

		for (j = 0; j < designCount; j++)
			if (Table[i].dwTypeSet & (1 << j))
				{
				printf("%s", CDesignType::GetTypeChar((DesignTypes)j).GetASCIIZPointer());
				}

		printf("\n");
		}
	}
