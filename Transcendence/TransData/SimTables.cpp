//	SimTables.cpp
//
//	Generate stats based on simulated games

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define ITEM_COUNT_FILENAME				CONSTLIT("TransData_ItemCount.txt")
#define ENCOUNTER_COUNT_FILENAME		CONSTLIT("TransData_EncounterCount.txt")

#define DEFAULT_SYSTEM_SAMPLE				100

class SystemInfo : public CObject
	{
	public:
		SystemInfo (void) : 
				CObject(NULL),
				Items(TRUE, TRUE) { }

		CString sName;
		int iLevel;
		DWORD dwSystemType;
		int iCount;									//	Number of times this system instance 
													//	has appeared.

		CSymbolTable Items;							//	All items types that have ever appeared in
													//	this system instace.

		CSymbolTable Stations;						//	Stations of a given type
	};

ALERROR OutputItemTable (CSymbolTable &AllSystems, int iSystemSample);
ALERROR OutputEncounterTable (CSymbolTable &AllSystems, int iSystemSample);

void GenerateSimTables (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	ALERROR error;
	int i, j;

	int iSystemSample = pCmdLine->GetAttributeInteger(CONSTLIT("count"));
	if (iSystemSample == 0)
		iSystemSample = DEFAULT_SYSTEM_SAMPLE;

	//	Generate systems for multiple games

	CSymbolTable AllSystems(TRUE, TRUE);
	for (i = 0; i < iSystemSample; i++)
		{
		printf("sample %d...\n", i+1);

		CTopologyNode *pNode = Universe.GetFirstTopologyNode();

		while (true)
			{
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
				pSystemEntry->dwSystemType = pNode->GetSystemDescUNID();
				pSystemEntry->iCount = 1;

				AllSystems.AddEntry(pSystemEntry->sName, pSystemEntry);
				}
			else
				pSystemEntry->iCount++;

			//	Add the encounters to the appropriate tables

			for (j = 0; j < pSystem->GetObjectCount(); j++)
				{
				CSpaceObject *pObj = pSystem->GetObject(j);

				if (pObj)
					{
					//	Add this encounter to the table

					CStationType *pType;
					if (pType = pObj->GetEncounterInfo())
						{
						CString sKey = strFromInt(pType->GetUNID(), false);

						//	See if we have this type in the table

						StationInfo *pEntry;
						if (error = pSystemEntry->Stations.Lookup(sKey, (CObject **)&pEntry))
							{
							pEntry = new StationInfo;
							pEntry->pType = pType;
							pEntry->iSystemCount = 0;
							pEntry->iTotalCount = 1;

							pSystemEntry->Stations.AddEntry(sKey, pEntry);
							}
						else
							pEntry->iTotalCount++;
						}

					//	Enumerate the items in this object

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
							}
						}
					}
				}

			//	Get the next node

			CString sEntryPoint;
			pNode = pSystem->GetStargateDestination(CONSTLIT("Outbound"), &sEntryPoint);
			if (pNode == NULL || pNode->IsEndGame())
				break;

			//	Done with old system

			Universe.DestroySystem(pSystem);
			}

		Universe.Reinit();
		}

	//	Output

	if (error = OutputItemTable(AllSystems, iSystemSample))
		return;

	if (error = OutputEncounterTable(AllSystems, iSystemSample))
		return;

	//	Create a table with the sum of all items for the game

	printf("Total count statistic computed.\n");
	}

ALERROR OutputEncounterTable (CSymbolTable &AllSystems, int iSystemSample)
	{
	ALERROR error;
	int i, j;
	CSymbolTable AllStations(TRUE, TRUE);

	for (i = 0; i < AllSystems.GetCount(); i++)
		{
		SystemInfo *pSystemEntry = (SystemInfo *)AllSystems.GetValue(i);

		for (j = 0; j < pSystemEntry->Stations.GetCount(); j++)
			{
			StationInfo *pEntry = (StationInfo *)pSystemEntry->Stations.GetValue(j);

			CString sKey = strFromInt(pEntry->pType->GetUNID(), false);

			StationInfo *pDestEntry;
			if (error = AllStations.Lookup(sKey, (CObject **)&pDestEntry))
				{
				pDestEntry = new StationInfo;
				pDestEntry->pType = pEntry->pType;
				pDestEntry->rTotalCount = ((double)pEntry->iTotalCount / (double)iSystemSample);

				AllStations.AddEntry(sKey, pDestEntry);
				}
			else
				pDestEntry->rTotalCount += ((double)pEntry->iTotalCount / (double)iSystemSample);
			}
		}

	//	Output all items to a well-known file

	CTextFileLog Output(ENCOUNTER_COUNT_FILENAME);
	if (error = Output.Create(FALSE))
		{
		printf("ERROR: Unable to create output file: %s\n", ENCOUNTER_COUNT_FILENAME.GetASCIIZPointer());
		return error;
		}

	for (i = 0; i < AllStations.GetCount(); i++)
		{
		StationInfo *pEntry = (StationInfo *)AllStations.GetValue(i);
		Output.LogOutput(0, "0x%x\t%d", pEntry->pType->GetUNID(), (int)((pEntry->rTotalCount * 1000) + 0.5));
		}

	if (error = Output.Close())
		{
		printf("ERROR: Unable to create output file: %s\n", ENCOUNTER_COUNT_FILENAME.GetASCIIZPointer());
		return error;
		}

	return NOERROR;
	}

ALERROR OutputItemTable (CSymbolTable &AllSystems, int iSystemSample)
	{
	ALERROR error;
	int i, j;
	CSymbolTable AllItems(TRUE, TRUE);

	for (i = 0; i < AllSystems.GetCount(); i++)
		{
		SystemInfo *pSystemEntry = (SystemInfo *)AllSystems.GetValue(i);

		for (j = 0; j < pSystemEntry->Items.GetCount(); j++)
			{
			ItemInfo *pEntry = (ItemInfo *)pSystemEntry->Items.GetValue(j);

			CString sKey = strFromInt(pEntry->pType->GetUNID(), false);

			ItemInfo *pDestEntry;
			if (error = AllItems.Lookup(sKey, (CObject **)&pDestEntry))
				{
				pDestEntry = new ItemInfo;
				pDestEntry->pType = pEntry->pType;
				pDestEntry->rTotalCount = ((double)pEntry->iTotalCount / (double)iSystemSample);

				AllItems.AddEntry(sKey, pDestEntry);
				}
			else
				pDestEntry->rTotalCount += ((double)pEntry->iTotalCount / (double)iSystemSample);
			}
		}

	//	Output all items to a well-known file

	CTextFileLog Output(ITEM_COUNT_FILENAME);
	if (error = Output.Create(FALSE))
		{
		printf("ERROR: Unable to create output file: %s\n", ITEM_COUNT_FILENAME.GetASCIIZPointer());
		return error;
		}

	for (i = 0; i < AllItems.GetCount(); i++)
		{
		ItemInfo *pEntry = (ItemInfo *)AllItems.GetValue(i);
		Output.LogOutput(0, "0x%x\t%d", pEntry->pType->GetUNID(), (int)((pEntry->rTotalCount * 1000) + 0.5));
		}

	if (error = Output.Close())
		{
		printf("ERROR: Unable to create output file: %s\n", ITEM_COUNT_FILENAME.GetASCIIZPointer());
		return error;
		}

	return NOERROR;
	}

ALERROR LoadTotalCount (const CString &sFilename, CSymbolTable &TotalCount)
	{
	ALERROR error;

	CFileReadBlock Input(sFilename);
	if (error = Input.Open())
		{
		printf("ERROR: Unable to open total count file. Use /generateSimTables.");
		return error;
		}

	char *pPos = Input.GetPointer(0, -1);
	char *pEndPos = pPos + Input.GetLength();

	while (pPos < pEndPos)
		{
		//	Read an UNID

		DWORD dwUNID = (DWORD)strParseInt(pPos, 0, &pPos);
		if (dwUNID == 0)
			break;

		//	Read an count

		if (pPos >= pEndPos)
			{
			printf("ERROR: Unexpected end of file.");
			return ERR_FAIL;
			}

		int iCount = strParseInt(pPos, 0, &pPos);

		//	Add the entry

		CString sKey = strFromInt(dwUNID, false);
		EntryInfo *pEntry = new EntryInfo;
		pEntry->dwUNID = dwUNID;
		pEntry->rTotalCount = (double)iCount / 1000.0;

		if (error = TotalCount.AddEntry(sKey, pEntry))
			{
			printf("ERROR: Invalid UNID.");
			return error;
			}
		}

	return NOERROR;
	}
