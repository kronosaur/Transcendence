//	StationPlaceSim.cpp
//
//	Generate statistics about station placement

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define NO_LOGO_SWITCH						CONSTLIT("nologo")
#define FIELD_NAME							CONSTLIT("shortName")

const int DIST_BUCKET_COUNT =				4;

const Metric DIST_BUCKET[DIST_BUCKET_COUNT] =
	{
	10.0 * LIGHT_SECOND,
	25.0 * LIGHT_SECOND,
	50.0 * LIGHT_SECOND,
	100.0 * LIGHT_SECOND
	};

class SPSimSystemInfo : public CObject
	{
	public:
		SPSimSystemInfo (void) : 
				CObject(NULL)
			{ }

		CString sName;
		int iLevel;
		DWORD dwSystemType;
		int iCount;									//	Number of times this system instance 
													//	has appeared.

		int iEnemies[DIST_BUCKET_COUNT];			//	Number of enemies at each distance
	};

int DistToBucketIndex (Metric rDist)
	{
	for (int i = 0; i < DIST_BUCKET_COUNT; i++)
		if (rDist <= DIST_BUCKET[i])
			return i;

	return -1;
	}

void GenerateStationPlaceSim (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	ALERROR error;
	int i, j, k;
	CSovereign *pPlayer = Universe.FindSovereign(g_PlayerSovereignUNID);

	int iSystemSample = pCmdLine->GetAttributeIntegerBounded(CONSTLIT("count"), 1, -1, 1);
	bool bLogo = !pCmdLine->GetAttributeBool(NO_LOGO_SWITCH);

	//	Generate systems for multiple games

	CSymbolTable AllSystems(TRUE, TRUE);
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

			SPSimSystemInfo *pSystemEntry;
			if (error = AllSystems.Lookup(pNode->GetSystemName(), (CObject **)&pSystemEntry))
				{
				pSystemEntry = new SPSimSystemInfo;
				pSystemEntry->sName = pNode->GetSystemName();
				pSystemEntry->iLevel = pNode->GetLevel();
				pSystemEntry->dwSystemType = pNode->GetSystemTypeUNID();
				pSystemEntry->iCount = 1;

				for (j = 0; j < DIST_BUCKET_COUNT; j++)
					pSystemEntry->iEnemies[j] = 0;

				AllSystems.AddEntry(pSystemEntry->sName, pSystemEntry);
				}
			else
				pSystemEntry->iCount++;

			//	For all active stations in the system, count the number of enemy stations
			//	within certain distance buckets

			for (j = 0; j < pSystem->GetObjectCount(); j++)
				{
				CSpaceObject *pObj = pSystem->GetObject(j);

				//	Find any objects that are lootable by the player

				if (pObj
						&& pObj->GetCategory() == CSpaceObject::catStation
						&& pObj->CanAttack())
					{
					//	Count to see how many enemy stations are in range

					for (k = 0; k < pSystem->GetObjectCount(); k++)
						{
						CSpaceObject *pEnemy = pSystem->GetObject(k);
						if (pEnemy
								&& pEnemy->GetCategory() == CSpaceObject::catStation
								&& pEnemy->CanAttack()
								&& (pEnemy->IsEnemy(pObj) || pObj->IsEnemy(pEnemy)))
							{
							Metric rDist = pObj->GetDistance(pEnemy);
							int iDist = DistToBucketIndex(rDist);
							if (iDist != -1)
								{
								ASSERT(iDist < DIST_BUCKET_COUNT && iDist >= 0);
								pSystemEntry->iEnemies[iDist]++;

								int iLSDist = (int)((rDist / LIGHT_SECOND) + 0.5);
								if (iLSDist < 30)
									{
									printf("%s: %s (%x) and %s (%x) within %d ls\n",
											pSystem->GetName().GetASCIIZPointer(),
											pObj->GetNounPhrase().GetASCIIZPointer(),
											pObj->GetID(),
											pEnemy->GetNounPhrase().GetASCIIZPointer(),
											pEnemy->GetID(),
											iLSDist);
									}
								}
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

	//	Output total value stats

	printf("Level\tSystem\t<10 ls\t<25 ls\t<50ls\t<100 ls\n");

	int iTotals[DIST_BUCKET_COUNT];
	for (i = 0; i < DIST_BUCKET_COUNT; i++)
		iTotals[i] = 0;

	for (i = 0; i < AllSystems.GetCount(); i++)
		{
		SPSimSystemInfo *pSystemEntry = (SPSimSystemInfo *)AllSystems.GetValue(i);

		printf("%d\t%s",
				pSystemEntry->iLevel,
				pSystemEntry->sName.GetASCIIZPointer());

		for (j = 0; j < DIST_BUCKET_COUNT; j++)
			{
			printf("\t%.2f", (double)pSystemEntry->iEnemies[j] / (double)pSystemEntry->iCount);
			iTotals[j] += pSystemEntry->iEnemies[j];
			}

		printf("\n");
		}

	//	Totals

	printf("\n");
	printf("Within  10 ls: %.2f\n", iTotals[0] / (double)iSystemSample);
	printf("Within  25 ls: %.2f\n", iTotals[1] / (double)iSystemSample);
	printf("Within  50 ls: %.2f\n", iTotals[2] / (double)iSystemSample);
	printf("Within 100 ls: %.2f\n", iTotals[3] / (double)iSystemSample);

	printf("\n");
	}

