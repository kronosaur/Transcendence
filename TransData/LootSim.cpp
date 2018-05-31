//	LootSim.cpp
//
//	Generate statistics about loot

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define NO_LOGO_SWITCH						CONSTLIT("nologo")

#define FIELD_NAME							CONSTLIT("shortName")

#define LOOT_TYPE_ASTEROIDS					CONSTLIT("asteroids")
#define LOOT_TYPE_ENEMIES					CONSTLIT("enemies")
#define LOOT_TYPE_INVENTORY					CONSTLIT("inventory")

enum ELootTypes
	{
	lootEnemies =			0x00000001,		//	Loot enemy ships and stations
	lootMining =			0x00000002,		//	Asteroid ore
	lootInventory =			0x00000004,		//	Inventory on friendly stations (for trade)
	};

struct SSystemInfo
	{
	CString sName;
	int iLevel;
	DWORD dwSystemType;
	int iCount;								//	Number of times this system instance 
											//	has appeared.

	TSortMap<DWORD, ItemInfo> Items;		//	All items types that have ever appeared in
											//	this system instance.
	int iTotalStations = 0;
	int iTotalLootValue = 0;
	int iTotalDeviceValue = 0;
	int iTotalArmorValue = 0;
	int iTotalOtherValue = 0;
	};

void AddItems (CSpaceObject *pObj, const CItemCriteria &Criteria, SSystemInfo *pSystemEntry);

void GenerateLootSim (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	ALERROR error;
	CString sError;
	int i, j;
	CSovereign *pPlayer = Universe.FindSovereign(g_PlayerSovereignUNID);

	int iSystemSample = pCmdLine->GetAttributeIntegerBounded(CONSTLIT("count"), 1, -1, 1);
	bool bLogo = !pCmdLine->GetAttributeBool(NO_LOGO_SWITCH);

	//	Criteria

	CString sCriteria = pCmdLine->GetAttribute(CONSTLIT("criteria"));
	if (sCriteria.IsBlank())
		sCriteria = CONSTLIT("*");

	CSpaceObjectCriteria Criteria(sCriteria);

	CItemCriteria ItemCriteria;
	CString sItemCriteria = pCmdLine->GetAttribute(CONSTLIT("itemCriteria"));
	if (!sItemCriteria.IsBlank())
		CItem::ParseCriteria(sItemCriteria, &ItemCriteria);
	else
		CItem::InitCriteriaAll(&ItemCriteria);

	//	Figure out what we're looting

	DWORD dwLootType = 0;
	if (pCmdLine->GetAttributeBool(LOOT_TYPE_ENEMIES))
		dwLootType |= lootEnemies;

	if (pCmdLine->GetAttributeBool(LOOT_TYPE_ASTEROIDS))
		dwLootType |= lootMining;

	if (pCmdLine->GetAttributeBool(LOOT_TYPE_INVENTORY))
		dwLootType |= lootInventory;

	//	Default to enemies

	if (dwLootType == 0)
		dwLootType = lootEnemies;

	DWORD dwTotalTime = 0;

	//	Generate systems for multiple games

	TSortMap<CString, SSystemInfo> AllSystems;
	for (i = 0; i < iSystemSample; i++)
		{
		if (bLogo)
			printf("pass %d...\n", i+1);

		int iNode;
		for (iNode = 0; iNode < Universe.GetTopologyNodeCount(); iNode++)
			{
			CTopologyNode *pNode = Universe.GetTopologyNode(iNode);
			if (pNode == NULL || pNode->IsEndGame())
				continue;

			//	Create the system

			DWORD dwStartTime = ::GetTickCount();

			CSystem *pSystem;
			if (error = Universe.CreateStarSystem(pNode, &pSystem))
				{
				printf("ERROR: Unable to create star system.\n");
				return;
				}

			dwTotalTime += ::sysGetTicksElapsed(dwStartTime);

			//	Find this system in the table.

			bool bNew;
			SSystemInfo *pSystemEntry = AllSystems.SetAt(pNode->GetID(), &bNew);
			if (bNew)
				{
				pSystemEntry->sName = pNode->GetSystemName();
				pSystemEntry->iLevel = pNode->GetLevel();
				pSystemEntry->dwSystemType = pNode->GetSystemTypeUNID();
				pSystemEntry->iCount = 1;
				}
			else
				pSystemEntry->iCount++;

			//	Create a table of all items

			CSpaceObjectCriteria::SCtx Ctx(Criteria);
			for (j = 0; j < pSystem->GetObjectCount(); j++)
				{
				CSpaceObject *pObj = pSystem->GetObject(j);
				if (pObj == NULL
						|| !pObj->MatchesCriteria(Ctx, Criteria))
					continue;

				//	Asteroid mining

				if ((dwLootType & lootMining)
						&& pObj->GetScale() == scaleWorld
						&& pObj->GetItemList().GetCount() != 0)
					{
					pSystemEntry->iTotalStations++;
					AddItems(pObj, ItemCriteria, pSystemEntry);
					}

				//	Find any objects that are lootable by the player

				else if ((dwLootType & lootEnemies)
						&& pObj->GetSovereign()
						&& pObj->GetSovereign()->IsEnemy(pPlayer))
					{
					//	Destroy the object and get the loot from the wreckage

					CSpaceObject *pWreck;
					if (pObj->GetCategory() == CSpaceObject::catShip)
						pObj->Destroy(killedByDamage, CDamageSource(NULL, killedByDamage), NULL, &pWreck);
					else
						pWreck = pObj;

					//	Get the loot

					if (pWreck)
						{
						pSystemEntry->iTotalStations++;
						AddItems(pWreck, ItemCriteria, pSystemEntry);
						}
					}

				//	Inventory

				else if ((dwLootType & lootInventory)
						&& pObj->GetSovereign()
						&& !pObj->GetSovereign()->IsEnemy(pPlayer)
						&& pObj->HasTradeService(serviceSell))
					{
					pSystemEntry->iTotalStations++;
					AddItems(pObj, ItemCriteria, pSystemEntry);
					}
				}

			//	Done with old system

			Universe.DestroySystem(pSystem);
			}

		//	Reinitialize

		Universe.Reinit();
		if (Universe.InitGame(0, &sError) != NOERROR)
			{
			printf("ERROR: %s\n", sError.GetASCIIZPointer());
			return;
			}
		}

	if (bLogo)
		printf("FINAL SYSTEM STATISTICS\n\n");

	MarkItemsKnown(Universe);

	//	Sort by level then system name

	TSortMap<CString, int> Sorted;
	for (i = 0; i < AllSystems.GetCount(); i++)
		{
		Sorted.Insert(strPatternSubst(CONSTLIT("%04d-%s"), AllSystems[i].iLevel, AllSystems[i].sName), i);
		}

	//	Output total value stats

	printf("Level\tSystem\tObjects\tLoot\tDevices\tArmor\tTreasure\n");

	for (i = 0; i < Sorted.GetCount(); i++)
		{
		const SSystemInfo &SystemEntry = AllSystems[Sorted[i]];

		printf("%d\t%s\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n",
				SystemEntry.iLevel,
				SystemEntry.sName.GetASCIIZPointer(),
				(double)SystemEntry.iTotalStations / (double)iSystemSample,
				(double)SystemEntry.iTotalLootValue / (double)iSystemSample,
				(double)SystemEntry.iTotalDeviceValue / (double)iSystemSample,
				(double)SystemEntry.iTotalArmorValue / (double)iSystemSample,
				(double)SystemEntry.iTotalOtherValue / (double)iSystemSample
				);
		}

	printf("\n");

	//	Output all items

	printf("Level\tSystem\tItem\tCount\tValue\n");

	CItem NULL_ITEM;
	CItemCtx ItemCtx(NULL_ITEM);

	for (i = 0; i < Sorted.GetCount(); i++)
		{
		const SSystemInfo &SystemEntry = AllSystems[Sorted[i]];

		for (j = 0; j < SystemEntry.Items.GetCount(); j++)
			{
			const ItemInfo *pEntry = &SystemEntry.Items[j];

			printf("%d\t%s\t%s\t%.2f\t%.2f\n",
					SystemEntry.iLevel,
					SystemEntry.sName.GetASCIIZPointer(),
					pEntry->pType->GetDataField(FIELD_NAME).GetASCIIZPointer(),
					(double)pEntry->iTotalCount / (double)iSystemSample,
					(double)pEntry->pType->GetValue(ItemCtx, true) * pEntry->iTotalCount / (double)iSystemSample);
			}
		}

	printf("Average time to create systems: %.2f seconds.\n", (double)dwTotalTime / (1000.0 * iSystemSample));
	}

void AddItems (CSpaceObject *pObj, const CItemCriteria &Criteria, SSystemInfo *pSystemEntry)
	{
	CItemListManipulator ItemList(pObj->GetItemList());
	ItemList.ResetCursor();
	while (ItemList.MoveCursorForward())
		{
		const CItem &Item(ItemList.GetItemAtCursor());

		//	Skip items we're not interested in.

		if (!Item.MatchesCriteria(Criteria)
				|| Item.IsInstalled()
				|| Item.IsDamaged())
			continue;

		//	Add the item

		bool bNew;
		ItemInfo *pEntry = pSystemEntry->Items.SetAt(Item.GetType()->GetUNID(), &bNew);
		if (bNew)
			{
			pEntry->pType = Item.GetType();
			pEntry->iTotalCount = Item.GetCount();
			}
		else
			pEntry->iTotalCount += Item.GetCount();

		//	Increment value

		int iValue = Item.GetTradePrice(NULL, true) * Item.GetCount();
		pSystemEntry->iTotalLootValue += iValue;

		if (Item.IsDevice())
			pSystemEntry->iTotalDeviceValue += iValue;
		else if (Item.IsArmor())
			pSystemEntry->iTotalArmorValue += iValue;
		else
			pSystemEntry->iTotalOtherValue += iValue;
		}
	}