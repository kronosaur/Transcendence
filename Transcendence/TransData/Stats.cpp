//	Stats.cpp
//
//	Generate basic stats about the data file

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

static char g_DamageTypeChar[] = "lkpbitPmangsASLF";

void GenerateArmorTable (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i;

	printf("ARMOR TABLE\n\n");

	printf("Level\tArmor\tCost\tWeight\tHP\tLasr\tImpc\tPart\tBlst\tIon\tThrm\tPosi\tPlsm\tAnti\tNano\tGrav\tSing\tDacd\tDstl\tDlgt\tDfir\n");
	for (i = 0; i < Universe.GetItemTypeCount(); i++)
		{
		CItemType *pItem = Universe.GetItemType(i);
		CArmorClass *pArmor = pItem->GetArmorClass();
		if (pArmor == NULL)
			continue;

		CItem Item(pItem, 1);
		CString sName = pItem->GetName(NULL);
		int iHP = pArmor->GetMaxHP(CItemCtx(&Item));

		printf("%d\t%s\t%d\t%d\t%d\t", 
				pItem->GetLevel(), 
				sName.GetASCIIZPointer(), 
				pItem->GetValue(), 
				pItem->GetMassKg(),
				iHP);

		//	For each damage type, compute the adjusted hit points of the armor

		int iDamage;
		for (iDamage = damageLaser; iDamage < damageCount; iDamage++)
			{
			printf("%d", pArmor->GetDamageAdj((DamageTypes)iDamage));
			if (iDamage != damageCount - 1)
				printf("\t");
			}

		printf("\n");
		}

	printf("ARMOR TABLE DAMAGE ADJUSTMENTS\n\n");

	printf("Level\tArmor\tHP\tLasr\tImpc\tPart\tBlst\tIon\tThrm\tPosi\tPlsm\tAnti\tNano\tGrav\tSing\tDacd\tDstl\tDlgt\tDfir\n");
	for (i = 0; i < Universe.GetItemTypeCount(); i++)
		{
		CItemType *pItem = Universe.GetItemType(i);
		CArmorClass *pArmor = pItem->GetArmorClass();
		if (pArmor == NULL)
			continue;

		CString sName = pItem->GetName(NULL);
		CItem Item(pItem, 1);
		int iHP = pArmor->GetMaxHP(CItemCtx(&Item));

		printf("%d\t%s\t%d\t", 
				pItem->GetLevel(), 
				sName.GetASCIIZPointer(), 
				iHP);

		//	For each damage type, compute the adjusted hit points of the armor

		int iDamage;
		for (iDamage = damageLaser; iDamage < damageCount; iDamage++)
			{
			int iAdj = pArmor->GetDamageAdj((DamageTypes)iDamage);
			if (iAdj == 0)
				printf("----");
			else
				printf("%d", iHP * 100 / iAdj);

			if (iDamage != damageCount - 1)
				printf("\t");
			}

		printf("\n");
		}
	}

void GenerateStationFrequencyTable (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	printf("STATION FREQUENCY TABLE\n\n");

	//	For all levels, 1-25

	int iLevel;
	for (iLevel = 1; iLevel <= MAX_ITEM_LEVEL; iLevel++)
		{
		int i;
		CSymbolTable Sort(FALSE, TRUE);

		//	Find all stations for this level and add them to
		//	a sorting table

		for (i = 0; i < Universe.GetStationTypeCount(); i++)
			{
			CStationType *pType = Universe.GetStationType(i);

			if (pType->GetFrequencyByLevel(iLevel) > 0)
				{
				char szBuffer[256];
				wsprintf(szBuffer, "%02d %s", 
						ftCommon - pType->GetFrequencyByLevel(iLevel),
						pType->GetName().GetASCIIZPointer());
						
				Sort.AddEntry(CString(szBuffer), (CObject *)pType);
				}
			}

		//	Print out the stations for this level

		if (Sort.GetCount() > 0)
			{
			printf("Level %s\n\n", strLevel(iLevel));

			for (i = 0; i < Sort.GetCount(); i++)
				{
				CStationType *pType = (CStationType *)Sort.GetValue(i);

				printf("%s\t%s\t%s\t%s\n",
						FrequencyChar(pType->GetFrequencyByLevel(iLevel)),
						pType->GetName().GetASCIIZPointer(),
						pType->GetAttributes().GetASCIIZPointer(),
						pType->GetLocationCriteria().GetASCIIZPointer());
				}

			printf("\n");
			}
		}
	}

void GenerateStats (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i;

	printf("STATS\n\n");

	int iItemTypes = 0;
	for (i = 0; i < Universe.GetItemTypeCount(); i++)
		{
		CItemType *pType = Universe.GetItemType(i);

		//	Do not count virtual items

		if (pType->IsVirtual())
			continue;

		iItemTypes++;
		}

	printf("Item types:\t\t%d\n", iItemTypes);
	printf("Ship classes:\t\t%d\n", Universe.GetShipClassCount());
	printf("Station types:\t\t%d\n", Universe.GetStationTypeCount());
	}

char *FrequencyChar (int iFreq)
	{
	if (iFreq >= ftCommon)
		return "c";
	else if (iFreq >= ftUncommon)
		return "u";
	else if (iFreq >= ftRare)
		return "r";
	else if (iFreq >= ftVeryRare)
		return "v";
	else
		return "-";
	}

