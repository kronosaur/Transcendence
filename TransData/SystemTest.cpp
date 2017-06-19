//	SystemTest.cpp
//
//	Generate statistics about station frequency by system

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

const int MAX_LEVEL = 26;

#define NO_LOGO_SWITCH						CONSTLIT("nologo")
#define FIELD_NAME							CONSTLIT("shortName")

class SSystemInfo
	{
	public:
		CString sName;
		int iLevel;
		DWORD dwSystemType;
		int iCount;									//	Number of times this system instance 
													//	has appeared.

		TSortMap<CString, StationInfo> Stations;	//	All station types that have ever appeared
													//	in this system instance.

		TSortMap<DWORD, ItemInfo> Items;			//	All items types that have ever appeared in
													//	this system instace.
	};

CString GenerateStationKey (CStationType *pType, CSovereign *pPlayer, CString *retsCategory = NULL)
	{
	//	Generate sort order

	char *pCat;
	if (pType->HasLiteralAttribute(CONSTLIT("debris")))
		pCat = "debris";
	else if (pType->GetControllingSovereign()->IsEnemy(pPlayer))
		pCat = "enemy";
	else if (pType->GetScale() == scaleStar || pType->GetScale() == scaleWorld)
		pCat = "-";
	else
		pCat = "friend";

	char szBuffer[1024];
	wsprintf(szBuffer, "%s %s", pCat, pType->GetNounPhrase().GetASCIIZPointer());

	if (retsCategory)
		*retsCategory = CString(pCat);

	return CString(szBuffer);
	}

void OutputSystemStats (SSystemInfo *pSystemEntry)
	{
	int j;

	//	Print

	printf("Level %s: %s (%x) %d\n\n", 
			(LPSTR)strLevel(pSystemEntry->iLevel),
			pSystemEntry->sName.GetASCIIZPointer(),
			pSystemEntry->dwSystemType,
			pSystemEntry->iCount);

	//	Output stations in sorted order

	for (j = 0; j < pSystemEntry->Stations.GetCount(); j++)
		{
		StationInfo *pEntry = &pSystemEntry->Stations[j];

		printf("%s\t%s\t", 
				pEntry->sCategory.GetASCIIZPointer(),
				pEntry->pType->GetNounPhrase().GetASCIIZPointer());

		for (int k = 0; k < MAX_FREQUENCY_COUNT; k++)
			{
			int iPercent = 100 * pEntry->iFreqCount[k] / pSystemEntry->iCount;

			if (pEntry->iFreqCount[k])
				printf("%d (%d%%); ", k, iPercent);
			}

		printf("%d average\n", pEntry->iTotalCount / pSystemEntry->iCount);
		}

	printf("\n");
	}

void GenerateSystemTest (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	ALERROR error;
	int i, j, iNode;
	CSovereign *pPlayer = Universe.FindSovereign(g_PlayerSovereignUNID);

	int iSystemSample = pCmdLine->GetAttributeInteger(CONSTLIT("count"));
	if (iSystemSample == 0)
		iSystemSample = 1;

	//	Generate systems for multiple games

	TSortMap<CString, SSystemInfo> AllSystems;
	for (i = 0; i < iSystemSample; i++)
		{
		printf("pass %d...\n", i+1);

		for (iNode = 0; iNode < Universe.GetTopologyNodeCount(); iNode++)
			{
			CTopologyNode *pNode = Universe.GetTopologyNode(iNode);
			if (pNode->IsEndGame())
				continue;

			//	Create the system

			CSystem *pSystem;
			CString sError;
			if (error = Universe.CreateStarSystem(pNode, &pSystem, &sError))
				{
				printf("ERROR: %s\n", sError.GetASCIIZPointer());
				return;
				}

			//	Find this system in the table.

			bool bNew;
			SSystemInfo *pSystemEntry = AllSystems.SetAt(pNode->GetSystemName(), &bNew);
			if (bNew)
				{
				pSystemEntry->sName = pNode->GetSystemName();
				pSystemEntry->iLevel = pNode->GetLevel();
				pSystemEntry->dwSystemType = pNode->GetSystemTypeUNID();
				pSystemEntry->iCount = 1;
				}
			else
				pSystemEntry->iCount++;

			//	Reset stations counts for this system

			for (j = 0; j < pSystemEntry->Stations.GetCount(); j++)
				{
				StationInfo *pEntry = &pSystemEntry->Stations[j];
				pEntry->iTempCount = 0;
				}

			//	Create a table of all stations

			for (j = 0; j < pSystem->GetObjectCount(); j++)
				{
				CSpaceObject *pObj = pSystem->GetObject(j);
				CStationType *pType;

				if (pObj 
						&& (pType = pObj->GetEncounterInfo()))
					{
					//	Generate sort order

					CString sCategory;
					CString sKey = GenerateStationKey(pType, pPlayer, &sCategory);

					//	See if we have this type in the table

					bool bNew;
					StationInfo *pEntry = pSystemEntry->Stations.SetAt(sKey, &bNew);
					if (bNew)
						{
						pEntry->sCategory = sCategory;
						pEntry->pType = pType;
						pEntry->iSystemCount = 0;
						pEntry->iTotalCount = 0;

						for (int k = 0; k < MAX_FREQUENCY_COUNT; k++)
							pEntry->iFreqCount[k] = 0;

						pEntry->iTempCount = 1;
						}
					else
						pEntry->iTempCount++;
					}
				}

			//	Output in sorted order

			for (j = 0; j < pSystemEntry->Stations.GetCount(); j++)
				{
				StationInfo *pEntry = &pSystemEntry->Stations[j];

				if (pEntry->iTempCount)
					{
					pEntry->iSystemCount++;
					pEntry->iTotalCount += pEntry->iTempCount;

					if (pEntry->iTempCount < MAX_FREQUENCY_COUNT)
						pEntry->iFreqCount[pEntry->iTempCount]++;
					}
				}

			//	Output stats for the system

			//OutputSystemStats(pSystemEntry);

			//	Done with old system

			Universe.DestroySystem(pSystem);
			}

		Universe.Reinit();
		}

	printf("FINAL SYSTEM STATISTICS\n\n");

	//	Output stats for all systems

	for (i = 0; i < AllSystems.GetCount(); i++)
		{
		SSystemInfo *pSystemEntry = &AllSystems[i];
		OutputSystemStats(pSystemEntry);
		}

	//	Compute stats for stations by level

	TSortMap<CString, StationInfo> AllStations[MAX_LEVEL];

	for (i = 0; i < AllSystems.GetCount(); i++)
		{
		SSystemInfo *pSystemEntry = &AllSystems[i];

		if (pSystemEntry->iCount > 0)
			{
			//	Get the destination

			TSortMap<CString, StationInfo> *pDest = &AllStations[pSystemEntry->iLevel];

			//	Figure out the probability that this system will appear

			int iSystemProb = pSystemEntry->iCount * 100 / iSystemSample;

			//	Add stations from this system

			for (j = 0; j < pSystemEntry->Stations.GetCount(); j++)
				{
				StationInfo *pEntry = &pSystemEntry->Stations[j];

				//	Figure out the average number of stations of this type
				//	in this system per game (in hundredths)

				int iCount = pEntry->iTotalCount * 100 / pSystemEntry->iCount;
				iCount = iCount * iSystemProb / 100;

				//	Add it

				CString sCategory;
				CString sKey = GenerateStationKey(pEntry->pType, pPlayer, &sCategory);

				bool bNew;
				StationInfo *pDestEntry = pDest->SetAt(sKey, &bNew);
				if (bNew)
					{
					pDestEntry->sCategory = sCategory;
					pDestEntry->pType = pEntry->pType;
					pDestEntry->iTotalCount = iCount;
					}
				else
					pDestEntry->iTotalCount += iCount;
				}
			}
		}

	//	Output stats for stations by level

	printf("AVERAGE STATION ENCOUNTERS BY LEVEL\n\n");

	printf("Level\tFOF\tEncounter\tCount\tFreq\n");

	for (i = 0; i < MAX_LEVEL; i++)
		{
		if (AllStations[i].GetCount())
			{
			for (j = 0; j < AllStations[i].GetCount(); j++)
				{
				StationInfo *pEntry = &AllStations[i].GetValue(j);

				printf("%d\t%s\t%s\t%d.%02d\t%d\n", 
						i,
						pEntry->sCategory.GetASCIIZPointer(),
						pEntry->pType->GetNounPhrase().GetASCIIZPointer(),
						pEntry->iTotalCount / 100,
						pEntry->iTotalCount % 100,
						pEntry->pType->GetFrequencyByLevel(i));
				}
			}
		}
	}

void GenerateItemFrequencyTable (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	ALERROR error;
	int i, j;
	CSovereign *pPlayer = Universe.FindSovereign(g_PlayerSovereignUNID);

	int iSystemSample = pCmdLine->GetAttributeInteger(CONSTLIT("count"));
	if (iSystemSample == 0)
		iSystemSample = 1;

	bool bLogo = !pCmdLine->GetAttributeBool(NO_LOGO_SWITCH);

	//	Generate systems for multiple games

	TSortMap<CString, SSystemInfo> AllSystems;
	for (i = 0; i < iSystemSample; i++)
		{
		if (bLogo)
			printf("pass %d...\n", i+1);

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

			bool bNew;
			SSystemInfo *pSystemEntry = AllSystems.SetAt(pNode->GetSystemName(), &bNew);
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

			for (j = 0; j < pSystem->GetObjectCount(); j++)
				{
				CSpaceObject *pObj = pSystem->GetObject(j);

				if (pObj)
					{
					//	Enumerate the items in this object

					CItemListManipulator ItemList(pObj->GetItemList());
					ItemList.ResetCursor();
					while (ItemList.MoveCursorForward())
						{
						const CItem &Item(ItemList.GetItemAtCursor());

						if (!Item.IsInstalled() && !Item.IsDamaged())
							{
							bool bNew;
							ItemInfo *pEntry = pSystemEntry->Items.SetAt(Item.GetType()->GetUNID(), &bNew);
							if (bNew)
								{
								pEntry->pType = Item.GetType();
								pEntry->iTotalCount = Item.GetCount();
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

	if (bLogo)
		printf("FINAL SYSTEM STATISTICS\n\n");

	printf("Level\tSystem\tItem\tCount\n");

	//	Output all items

	MarkItemsKnown(Universe);

	for (i = 0; i < AllSystems.GetCount(); i++)
		{
		SSystemInfo *pSystemEntry = &AllSystems[i];

		for (j = 0; j < pSystemEntry->Items.GetCount(); j++)
			{
			ItemInfo *pEntry = &pSystemEntry->Items[j];

			printf("%d\t%s\t%s\t%.2f\n",
					pSystemEntry->iLevel,
					pSystemEntry->sName.GetASCIIZPointer(),
					pEntry->pType->GetDataField(FIELD_NAME).GetASCIIZPointer(),
					(double)pEntry->iTotalCount / (double)iSystemSample);
			}
		}
	}

