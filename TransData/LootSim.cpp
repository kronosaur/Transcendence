//	LootSim.cpp
//
//	Generate statistics about loot

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define NO_LOGO_SWITCH						CONSTLIT("nologo")

#define FIELD_NAME							CONSTLIT("shortName")

#define LOOT_TYPE_ASTEROIDS					CONSTLIT("asteroids")
#define LOOT_TYPE_ENEMIES					CONSTLIT("enemies")

enum ELootTypes
	{
	lootEnemies =			0x00000001,		//	Loot enemy ships and stations
	lootMining =			0x00000002,		//	Asteroid ore
	};

class SystemInfo : public CObject
	{
	public:
		SystemInfo (void) : 
				CObject(NULL),
				Stations(TRUE, TRUE),
				Items(TRUE, TRUE) { }

		CString sName;
		int iLevel;
		DWORD dwSystemType;
		int iCount;									//	Number of times this system instance 
													//	has appeared.

		CSymbolTable Stations;						//	All station types that have ever appeared
													//	in this system instance.

		CSymbolTable Items;							//	All items types that have ever appeared in
													//	this system instance.
		int iTotalLootValue;
	};

void AddItems (CSpaceObject *pObj, SystemInfo *pSystemEntry);

void GenerateLootSim (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	ALERROR error;
	CString sError;
	int i, j;
	CSovereign *pPlayer = Universe.FindSovereign(g_PlayerSovereignUNID);

	int iSystemSample = pCmdLine->GetAttributeIntegerBounded(CONSTLIT("count"), 1, -1, 1);
	bool bLogo = !pCmdLine->GetAttributeBool(NO_LOGO_SWITCH);

	//	Figure out what we're looting

	DWORD dwLootType = 0;
	if (pCmdLine->GetAttributeBool(LOOT_TYPE_ENEMIES))
		dwLootType |= lootEnemies;

	if (pCmdLine->GetAttributeBool(LOOT_TYPE_ASTEROIDS))
		dwLootType |= lootMining;

	//	Default to enemies

	if (dwLootType == 0)
		dwLootType = lootEnemies;

	//	Generate systems for multiple games

	CSymbolTable AllSystems(TRUE, TRUE);
	for (i = 0; i < iSystemSample; i++)
		{
		if (bLogo)
			printf("pass %d...\n", i+1);

		CTopologyNode *pNode = Universe.GetFirstTopologyNode();

		int iNode;
		for (iNode = 0; iNode < Universe.GetTopologyNodeCount(); iNode++)
			{
			CTopologyNode *pNode = Universe.GetTopologyNode(iNode);
			if (pNode == NULL || pNode->IsEndGame())
				continue;

			//	Create the system

			CSystem *pSystem;
			if (error = Universe.CreateStarSystem(pNode, &pSystem))
				{
				printf("ERROR: Unable to create star system.\n");
				return;
				}

			//	Find this system in the table.

			SystemInfo *pSystemEntry;
			if (error = AllSystems.Lookup(pNode->GetSystemName(), (CObject **)&pSystemEntry))
				{
				pSystemEntry = new SystemInfo;
				pSystemEntry->sName = pNode->GetSystemName();
				pSystemEntry->iLevel = pNode->GetLevel();
				pSystemEntry->dwSystemType = pNode->GetSystemTypeUNID();
				pSystemEntry->iCount = 1;
				pSystemEntry->iTotalLootValue = 0;

				AllSystems.AddEntry(pSystemEntry->sName, pSystemEntry);
				}
			else
				pSystemEntry->iCount++;

			//	Create a table of all items

			for (j = 0; j < pSystem->GetObjectCount(); j++)
				{
				CSpaceObject *pObj = pSystem->GetObject(j);
				if (pObj == NULL)
					continue;

				//	Asteroid mining

				if ((dwLootType & lootMining)
						&& pObj->GetScale() == scaleWorld
						&& pObj->GetItemList().GetCount() != 0)
					{
					AddItems(pObj, pSystemEntry);
					continue;
					}

				//	Find any objects that are lootable by the player

				if ((dwLootType & lootEnemies)
						&& pObj->GetSovereign()
						&& pObj->GetSovereign()->IsEnemy(pPlayer))
					{
					//	Destroy the object and get the loot from the wreckage

					CSpaceObject *pWreck;
					if (pObj->GetCategory() == CSpaceObject::catShip)
						pObj->Destroy(killedByDamage, CDamageSource(NULL, killedByDamage), &pWreck);
					else
						pWreck = pObj;

					//	Get the loot

					if (pWreck)
						{
						AddItems(pWreck, pSystemEntry);
						continue;
						}
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

	//	Output total value stats

	printf("Level\tSystem\tLoot\n");

	for (i = 0; i < AllSystems.GetCount(); i++)
		{
		SystemInfo *pSystemEntry = (SystemInfo *)AllSystems.GetValue(i);

		printf("%d\t%s\t%.2f\n",
				pSystemEntry->iLevel,
				pSystemEntry->sName.GetASCIIZPointer(),
				(double)pSystemEntry->iTotalLootValue / (double)iSystemSample);
		}

	printf("\n");

	//	Output all items

	printf("Level\tSystem\tItem\tCount\tValue\n");

	CItem NULL_ITEM;
	CItemCtx ItemCtx(NULL_ITEM);

	for (i = 0; i < AllSystems.GetCount(); i++)
		{
		SystemInfo *pSystemEntry = (SystemInfo *)AllSystems.GetValue(i);

		for (j = 0; j < pSystemEntry->Items.GetCount(); j++)
			{
			ItemInfo *pEntry = (ItemInfo *)pSystemEntry->Items.GetValue(j);

			printf("%d\t%s\t%s\t%.2f\t%.2f\n",
					pSystemEntry->iLevel,
					pSystemEntry->sName.GetASCIIZPointer(),
					pEntry->pType->GetDataField(FIELD_NAME).GetASCIIZPointer(),
					(double)pEntry->iTotalCount / (double)iSystemSample,
					(double)pEntry->pType->GetValue(ItemCtx, true) * pEntry->iTotalCount / (double)iSystemSample);
			}
		}
	}

void AddItems (CSpaceObject *pObj, SystemInfo *pSystemEntry)
	{
	ALERROR error;

	CItemListManipulator ItemList(pObj->GetItemList());
	ItemList.ResetCursor();
	while (ItemList.MoveCursorForward())
		{
		const CItem &Item(ItemList.GetItemAtCursor());

		if (!Item.IsInstalled() && !Item.IsDamaged())
			{
			CString sKey = strFromInt(Item.GetType()->GetUNID(), false);

			//	Find the item type in the table

			ItemInfo *pEntry;
			if (error = pSystemEntry->Items.Lookup(sKey, (CObject **)&pEntry))
				{
				pEntry = new ItemInfo;
				pEntry->pType = Item.GetType();
				pEntry->iTotalCount = Item.GetCount();

				pSystemEntry->Items.AddEntry(sKey, pEntry);
				}
			else
				pEntry->iTotalCount += Item.GetCount();

			//	Increment value

			pSystemEntry->iTotalLootValue += Item.GetValue(true) * Item.GetCount();
			}
		}
	}