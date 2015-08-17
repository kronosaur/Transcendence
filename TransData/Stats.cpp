//	Stats.cpp
//
//	Generate basic stats about the data file

#include <stdio.h>

#include <windows.h>
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
				Item.GetValue(true), 
				Item.GetMassKg(),
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
	int i, j;

	printf("STATION FREQUENCY TABLE\n\n");

	//	Figure out which columns to show

	TArray<CString> Cols;
	Cols.Insert(CONSTLIT("Level"));
	Cols.Insert(CONSTLIT("Freq"));
	Cols.Insert(CONSTLIT("Name"));
	Cols.Insert(CONSTLIT("Type"));
	Cols.Insert(CONSTLIT("Environment"));

	//	Print the header

	for (i = 0; i < Cols.GetCount(); i++)
		{
		if (i != 0)
			printf("\t");
		printf("%s", (LPSTR)Cols[i]);
		}

	printf("\n");

	//	For all levels, 1-25

	int iLevel;
	for (iLevel = 1; iLevel <= MAX_ITEM_LEVEL; iLevel++)
		{
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
			for (i = 0; i < Sort.GetCount(); i++)
				{
				CStationType *pType = (CStationType *)Sort.GetValue(i);

				for (j = 0; j < Cols.GetCount(); j++)
					{
					if (j != 0)
						printf("\t");

					if (strEquals(Cols[j], CONSTLIT("Level")))
						printf("%d", iLevel);
					else if (strEquals(Cols[j], CONSTLIT("Freq")))
						printf((LPSTR)FrequencyChar(pType->GetFrequencyByLevel(iLevel)));
					else if (strEquals(Cols[j], CONSTLIT("Name")))
						printf((LPSTR)pType->GetName());
					else if (strEquals(Cols[j], CONSTLIT("Type")))
						{
						if (pType->HasAttribute(CONSTLIT("enemy")))
							printf("Enemy");
						else if (pType->HasAttribute(CONSTLIT("debris")))
							printf("Debris");
						else
							printf("Friend");
						}
					else if (strEquals(Cols[j], CONSTLIT("Environment")))
						{
						bool bElements = false;

						if (pType->HasAttribute(CONSTLIT("envAir")))
							{
							if (bElements)
								printf(", ");

							printf("envAir");
							bElements = true;
							}

						if (pType->HasAttribute(CONSTLIT("envEarth")))
							{
							if (bElements)
								printf(", ");

							printf("envEarth");
							bElements = true;
							}

						if (pType->HasAttribute(CONSTLIT("envFire")))
							{
							if (bElements)
								printf(", ");

							printf("envFire");
							bElements = true;
							}

						if (pType->HasAttribute(CONSTLIT("envWater")))
							{
							if (bElements)
								printf(", ");

							printf("envWater");
							bElements = true;
							}

						if (!bElements)
							printf("None");
						}
					}

				printf("\n");
				}
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

CString GetTypeDesc (CDesignType *pType)
	{
	CString sName = pType->GetTypeName();
	if (sName.IsBlank())
		return strPatternSubst(CONSTLIT("%08x: [%s]"), pType->GetUNID(), pType->GetTypeClassName());
	else
		return strPatternSubst(CONSTLIT("%08x: %s [%s]"), pType->GetUNID(), sName, pType->GetTypeClassName());
	}

void AddTypesUsedRecursive (CUniverse &Universe, DWORD dwUNID, TSortMap<DWORD, bool> *retTypesUsed)
	{
	int i;

	//	If already added, don't bother

	if (retTypesUsed->Find(dwUNID))
		return;

	CDesignType *pType = Universe.FindDesignType(dwUNID);
	if (pType == NULL)
		return;

	retTypesUsed->SetAt(dwUNID, true);

	//	Recurse

	TSortMap<DWORD, bool> TypesUsed;
	pType->AddTypesUsed(&TypesUsed);
	for (i = 0; i < TypesUsed.GetCount(); i++)
		AddTypesUsedRecursive(Universe, TypesUsed.GetKey(i), retTypesUsed);
	}

void GenerateTypeDependencies (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i, j;

	bool bRecursive = pCmdLine->GetAttributeBool(CONSTLIT("recursive"));
	bool bReverse = pCmdLine->GetAttributeBool(CONSTLIT("reverse"));

	//	Create a reverse index of all type dependencies

	TSortMap<DWORD, TArray<DWORD> > ReverseIndex;

	//	Types and what they use

	if (!bReverse)
		{
		printf("TYPES AND WHAT THEY USE\n");
		printf("-----------------------\n\n");
		}

	for (i = 0; i < Universe.GetDesignTypeCount(); i++)
		{
		CDesignType *pType = Universe.GetDesignType(i);

		if (!bReverse)
			printf("%s\n", (char *)GetTypeDesc(pType));

		//	Get the list of UNIDs that this type uses

		TSortMap<DWORD, bool> TypesUsed;
		if (bRecursive)
			AddTypesUsedRecursive(Universe, pType->GetUNID(), &TypesUsed);
		else
			pType->AddTypesUsed(&TypesUsed);

		//	Output the list

		for (j = 0; j < TypesUsed.GetCount(); j++)
			{
			CDesignType *pRequired = Universe.FindDesignType(TypesUsed.GetKey(j));
			if (pRequired == NULL)
				continue;

			if (!bReverse)
				printf("\t%s\n", (char *)GetTypeDesc(pRequired));

			//	Add to reverse index

			TArray<DWORD> *pList = ReverseIndex.SetAt(pRequired->GetUNID());
			pList->Insert(pType->GetUNID());
			}
		}

	//	Types and what depends on them

	if (bReverse)
		{
		printf("\nTYPES AND WHAT USES THEM\n");
		printf(  "------------------------\n\n");

		for (i = 0; i < ReverseIndex.GetCount(); i++)
			{
			CDesignType *pType = Universe.FindDesignType(ReverseIndex.GetKey(i));
			if (pType == NULL)
				continue;

			printf("%s\n", (char *)GetTypeDesc(pType));

			TArray<DWORD> &List = ReverseIndex.GetValue(i);
			for (j = 0; j < List.GetCount(); j++)
				{
				CDesignType *pRequiredBy = Universe.FindDesignType(List[j]);
				if (pRequiredBy)
					printf("\t%s\n", (char *)GetTypeDesc(pRequiredBy));
				}
			}
		}
	}

typedef TSortMap<DWORD, bool> IslandMap;
typedef TSortMap<DWORD, TArray<DWORD> > ReverseIndexMap;

IslandMap g_DefaultTypes;

void AddTypeToIsland (CUniverse &Universe, ReverseIndexMap &ReverseIndex, IslandMap *pIsland, CDesignType *pType)
	{
	int i;

	//	Skip images

	if (pType->GetType() == designImage)
		return;

	if (pType->GetType() == designShipTable)
		return;

	//	Exclude default types

	if (g_DefaultTypes.Find(pType->GetUNID()))
		return;

	//	If this type is already on the island, then there is nothing to do.

	if (pIsland->Find(pType->GetUNID()))
		return;

	//	Add the type to the island (we do this first because we want it to be
	//	here in case we recurse below).

	pIsland->Insert(pType->GetUNID(), true);

	//	Now get the list of all types that this type uses and add them to the
	//	island.

	IslandMap TypesUsed;
	pType->AddTypesUsed(&TypesUsed);
	for (i = 0; i < TypesUsed.GetCount(); i++)
		{
		CDesignType *pTypeUsed = Universe.FindDesignType(TypesUsed.GetKey(i));
		if (pTypeUsed == NULL)
			continue;

		AddTypeToIsland(Universe, ReverseIndex, pIsland, pTypeUsed);
		}

	//	Now get the lists that use this type and add them to the island.

#if 0
	TArray<DWORD> *pList = ReverseIndex.GetAt(pType->GetUNID());
	if (pList)
		{
		for (i = 0; i < pList->GetCount(); i++)
			{
			CDesignType *pTypeUsing = Universe.FindDesignType(pList->GetAt(i));
			if (pTypeUsing == NULL)
				continue;

			AddTypeToIsland(Universe, ReverseIndex, pIsland, pTypeUsing);
			}
		}
#endif
	}

void GenerateTypeIslands (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i, j;

	bool bExcludeImages = true;

	printf("TYPE ISLANDS\n");
	printf("------------\n\n");

	//	Make a list of default types

#if 0
	g_DefaultTypes.SetAt(0x00001001, true);	//	independent sovereign
	g_DefaultTypes.SetAt(0x00001002, true);	//	Commonwealth sovereign
	g_DefaultTypes.SetAt(0x00001003, true);	//	independent sovereign
	g_DefaultTypes.SetAt(0x00001007, true);	//	ares sovereign
	g_DefaultTypes.SetAt(0x0000100c, true);	//	sung slavers sovereign
	g_DefaultTypes.SetAt(0x0000100f, true);	//	auton sovereign
	g_DefaultTypes.SetAt(0x00001011, true);	//	corporate hierarchy
	g_DefaultTypes.SetAt(0x00004027, true);	//	container of frozen supplies
	g_DefaultTypes.SetAt(0x0000402c, true);	//	pteracnium ore
	g_DefaultTypes.SetAt(0x000040ae, true);	//	helium3 reactor assembly
	g_DefaultTypes.SetAt(0x000040af, true);	//	pteracnium fuel
	g_DefaultTypes.SetAt(0x000040ca, true);	//	lancer cannon
	g_DefaultTypes.SetAt(0x000040e2, true);	//	worldship armor
	g_DefaultTypes.SetAt(0x00004100, true);	//	xenotite ore
	g_DefaultTypes.SetAt(0x00004109, true);	//	SN2500 reactor
	g_DefaultTypes.SetAt(0x00004167, true);	//	tetramite ore
	g_DefaultTypes.SetAt(0x00005004, true);	//	wreck ejecta
	g_DefaultTypes.SetAt(0x0000500c, true);	//	blast explosion 2
	g_DefaultTypes.SetAt(0x0000500d, true);	//	blast explosion 3
	g_DefaultTypes.SetAt(0x0000500e, true);	//	blast explosion 4
	g_DefaultTypes.SetAt(0x0000500f, true);	//	thermo explosion 1
	g_DefaultTypes.SetAt(0x00005011, true);	//	thermo explosion 3
	g_DefaultTypes.SetAt(0x00005012, true);	//	thermo explosion 4
	g_DefaultTypes.SetAt(0x00009004, true);	//	shield effect
	g_DefaultTypes.SetAt(0x00009007, true);	//	explosion effect
	g_DefaultTypes.SetAt(0x0000900a, true);	//	fire effect
	g_DefaultTypes.SetAt(0x0000A003, true);	//	dsAbandonedStation
	g_DefaultTypes.SetAt(0x0000a017, true);	//	dock screen?

	g_DefaultTypes.SetAt(0x001a200c, true);	//	wreck of the CSC Europa
	g_DefaultTypes.SetAt(0x001a200e, true);	//	sandstorm wreck
	g_DefaultTypes.SetAt(0x001c1002, true);	//	ares sect in Heretic
	g_DefaultTypes.SetAt(0x08020102, true);	//	huari empire sovereign
	g_DefaultTypes.SetAt(0x08040140, true);	//	gaian processor station
#endif

	//	Create a reverse index of all type dependencies

	ReverseIndexMap ReverseIndex;
	for (i = 0; i < Universe.GetDesignTypeCount(); i++)
		{
		CDesignType *pType = Universe.GetDesignType(i);

		//	Get the list of UNIDs that this type uses

		TSortMap<DWORD, bool> TypesUsed;
		pType->AddTypesUsed(&TypesUsed);
		for (j = 0; j < TypesUsed.GetCount(); j++)
			{
			CDesignType *pRequired = Universe.FindDesignType(TypesUsed.GetKey(j));
			if (pRequired == NULL)
				continue;

			//	Add to reverse index

			TArray<DWORD> *pList = ReverseIndex.SetAt(pRequired->GetUNID());
			pList->Insert(pType->GetUNID());
			}
		}

	//	We create a list of islands. In each island, all the types refer to
	//	each other and don't refer to types on any other island.

	TArray<IslandMap> AllIslands;

	//	Loop over all types and add them to an island.

	for (i = 0; i < Universe.GetDesignTypeCount(); i++)
		{
		CDesignType *pType = Universe.GetDesignType(i);

		//	Exclude images

		if (bExcludeImages && pType->GetType() == designImage)
			continue;

		if (pType->GetType() == designShipTable)
			continue;

		//	Exclude default types

		if (g_DefaultTypes.Find(pType->GetUNID()))
			continue;

		//	If this type is already on one of the islands, then we skip it.

#if 0
		bool bFound = false;
		for (j = 0; j < AllIslands.GetCount(); j++)
			if (AllIslands[j].Find(pType->GetUNID()))
				{
				bFound = true;
				break;
				}

		if (bFound)
			continue;
#endif 

		//	Since this type is not on any island, we add start a new island and
		//	add all related types to it.

		IslandMap *pIsland = AllIslands.Insert();
		AddTypeToIsland(Universe, ReverseIndex, pIsland, pType);

		//	Print

		printf("ISLAND %s\n", (char *)GetTypeDesc(pType));

		for (j = 0; j < pIsland->GetCount(); j++)
			{
			CDesignType *pType = Universe.FindDesignType(pIsland->GetKey(j));
			if (pType == NULL)
				continue;

			printf("%s\n", (char *)GetTypeDesc(pType));
			}

		printf("\n");
		}
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

