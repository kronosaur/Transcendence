//	RandomItemTables.cpp
//
//	Generates random item tables for each level

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

enum TableTypes
	{
	tableArmorAndWeapons	= 0,
	tableMiscItems			= 1,
	tableUsefulItems		= 2,

	tableTypesFirst			= 0,
	tableTypesCount			= 2,
	};

struct ItemEntry
	{
	CItemType *pType;
	int iScore;
	int iChance;
	int iRemainder;
	};

bool ItemInTable (CItemType *pType, TableTypes iTable, int iLevel, int *retiScore)
	{
	//	If this item is not near our level then bail out.

	int iLevelDelta = pType->GetLevel() - iLevel;
	if (pType->GetLevel() == 0 || iLevelDelta > 3 || iLevelDelta < -2)
		return false;

	//	Compute a score based on the level

	int iScore;
	switch (iLevelDelta)
		{
		case 0:
		case 1:
			iScore = 1000;
			break;

		case -1:
		case 2:
			iScore = 400;
			break;

		case 3:
		case -2:
			iScore = 150;
			break;

		default:
			ASSERT(false);
		}

	//	Now adjust for the type of table

	switch (iTable)
		{
		case tableArmorAndWeapons:
			{
			if (pType->GetCategory() != itemcatArmor
					&& pType->GetCategory() != itemcatLauncher
					&& pType->GetCategory() != itemcatWeapon
					&& pType->GetCategory() != itemcatShields
					&& pType->GetCategory() != itemcatMiscDevice)
				return false;
			break;
			}

		case tableUsefulItems:
			{
			if (pType->GetCategory() != itemcatFuel
					&& pType->GetCategory() != itemcatMissile
					&& !pType->IsUsable()
					&& pType->GetCategory() != itemcatDrive
					&& pType->GetCategory() != itemcatReactor)
				return false;
			break;
			}

		case tableMiscItems:
			{
			if (pType->GetCategory() != itemcatMisc
					|| pType->IsUsable())
				return false;
			break;
			}
		}

	//	Adjust for frequency

	iScore = iScore * pType->GetFrequency() * 10 / (ftCommon * 10);
	if (iScore == 0)
		return false;

	//	Done

	*retiScore = iScore;
	return true;
	}

void GenerateRandomItemTables (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i;
	printf("RANDOM ITEM TABLES\n\n");

	//	Create a sorted list of item types

	CSymbolTable SortedList(FALSE, TRUE);
	for (i = 0; i < Universe.GetItemTypeCount(); i++)
		{
		CItemType *pType = Universe.GetItemType(i);
		SortedList.AddEntry(pType->GetSortName(), (CObject *)pType);
		}

	//	Create the tables

	for (int iLevel = 1; iLevel <= MAX_ITEM_LEVEL; iLevel += 2)
		{
		for (TableTypes iTable = tableTypesFirst; iTable <= tableTypesCount; iTable = (enum TableTypes)((int)iTable + 1))
			{
			int iCount = 0;
			ItemEntry Table[100];

			//	Fill the table with entries for each 
			//	item at this level

			for (i = 0; i < SortedList.GetCount(); i++)
				{
				CItemType *pType = (CItemType *)SortedList.GetValue(i);
				int iScore;
				if (iCount < 100 && ItemInTable(pType, iTable, iLevel, &iScore))
					{
					Table[iCount].pType = pType;
					Table[iCount].iScore = iScore;
					iCount++;
					}
				}

			//	Compute total score

			int iTotalScore = 0;
			for (i = 0; i < iCount; i++)
				iTotalScore += Table[i].iScore;

			//	Compute chance

			for (i = 0; i < iCount; i++)
				{
				Table[i].iChance = (Table[i].iScore * 100) / iTotalScore;
				Table[i].iRemainder = (Table[i].iScore * 100) % iTotalScore;
				}

			//	Compute remainder

			int iTotalChance = 0;
			for (i = 0; i < iCount; i++)
				iTotalChance += Table[i].iChance;

			while (iTotalChance < 100)
				{
				int iBestRemainder = 0;
				int iBestEntry = -1;

				for (i = 0; i < iCount; i++)
					if (Table[i].iRemainder > iBestRemainder)
						{
						iBestRemainder = Table[i].iRemainder;
						iBestEntry = i;
						}

				Table[iBestEntry].iChance++;
				Table[iBestEntry].iRemainder = 0;
				iTotalChance++;
				}

			//	Title

			if (iCount > 0)
				{
				switch (iTable)
					{
					case tableArmorAndWeapons:
						printf("LEVEL %d: ARMOR AND WEAPONS\n\n", iLevel);
						break;

					case tableMiscItems:
						printf("LEVEL %d: MISCELLANEOUS ITEMS\n\n", iLevel);
						break;

					case tableUsefulItems:
						printf("LEVEL %d: USEFUL ITEMS\n\n", iLevel);
						break;
					}

				//	Print table

				for (i = 0; i < iCount; i++)
					if (Table[i].iChance > 0)
						{
						//DWORD dwFlags;
						printf("%d\t%s\n", 
								Table[i].iChance,
								//Table[i].pType->GetName(&dwFlags, true).GetASCIIZPointer());
								Table[i].pType->GetSortName().GetASCIIZPointer());
						}

				printf("\n");
				}
			}
		}
	}
