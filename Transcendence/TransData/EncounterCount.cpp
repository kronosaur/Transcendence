//	EncounterCount.cpp
//
//	Counts encounters in a game

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

typedef TSortMap<DWORD, int> CCountTable;
typedef TSortMap<DWORD, CCountTable> CSystemTypeTable;

struct SNodeDesc
	{
	CString sNodeID;
	int iLevel;
	CSystemTypeTable Table;					//	Counts for each system type
	};

void AddSystemData (CSystem *pSystem, bool bAll, SNodeDesc *retResult);

void GenerateEncounterCount (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i, j, k;

	//	Options

	int iSystemSample = pCmdLine->GetAttributeIntegerBounded(CONSTLIT("count"), 1, -1, 1);
	bool bLogo = !pCmdLine->GetAttributeBool(CONSTLIT("noLogo"));
	bool bAll = pCmdLine->GetAttributeBool(CONSTLIT("all"));

	//	Generate systems for multiple games

	TSortMap<CString, SNodeDesc> NodeTable;

	for (i = 0; i < iSystemSample; i++)
		{
		if (bLogo)
			printf("pass %d...\n", i+1);

		for (j = 0; j < Universe.GetTopologyNodeCount(); j++)
			{
			CTopologyNode *pNode = Universe.GetTopologyNode(j);
			if (pNode->IsEndGame())
				continue;

			//	Create the system

			CSystem *pSystem;
			if (Universe.CreateStarSystem(pNode, &pSystem) != NOERROR)
				{
				printf("ERROR: Unable to create star system.\n");
				return;
				}

			//	Create a sort string for this system

			CString sSort = strPatternSubst(CONSTLIT("%02d-%s"), pSystem->GetLevel(), pNode->GetID());

			//	Get the table

			bool bNew;
			SNodeDesc *pResult = NodeTable.SetAt(sSort, &bNew);
			if (bNew)
				{
				pResult->iLevel = pNode->GetLevel();
				pResult->sNodeID = pNode->GetID();
				}

			//	Accumulate data

			AddSystemData(pSystem, bAll, pResult);

			//	Done with old system

			Universe.DestroySystem(pSystem);
			}

		Universe.Reinit();
		}

	//	Output all rows

	printf("Level\tNode\tSystemType\tCategory\tSovereign\tEncounter\tCount\n");
	for (i = 0; i < NodeTable.GetCount(); i++)
		{
		for (j = 0; j < NodeTable[i].Table.GetCount(); j++)
			{
			CCountTable &Encounters = NodeTable[i].Table[j];

			for (k = 0; k < Encounters.GetCount(); k++)
				{
				int iCount = Encounters[k] / iSystemSample;
				int iCountFrac = 1000 * (Encounters[k] % iSystemSample) / iSystemSample;
				CStationType *pEncounterType = Universe.FindStationType(Encounters.GetKey(k));
				if (pEncounterType == NULL)
					continue;

				CSovereign *pSovereign = pEncounterType->GetSovereign();
				CString sSovereign = (pSovereign ? pSovereign->GetTypeName() : CONSTLIT("(Unknown)"));

				printf("%d\t%s\t%08x\t%s\t%s\t%s\t%d.%03d\n",
						NodeTable[i].iLevel,
						NodeTable[i].sNodeID.GetASCIIZPointer(),
						NodeTable[i].Table.GetKey(j),
						pEncounterType->GetDataField(CONSTLIT("category")).GetASCIIZPointer(),
						sSovereign.GetASCIIZPointer(),
						pEncounterType->GetName().GetASCIIZPointer(),
						iCount,
						iCountFrac);
				}
			}
		}
	}

void AddSystemData (CSystem *pSystem, bool bAll, SNodeDesc *retResult)
	{
	int i;

	//	Loop over all objects

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CStationType *pType;
		CSpaceObject *pObj = pSystem->GetObject(i);
		if (pObj == NULL 
				|| pObj->IsDestroyed()
				|| (pType = pObj->GetEncounterInfo()) == NULL)
			continue;

		//	Skip if we're not interested in this encounter

		if (!bAll && !pType->CanBeEncounteredRandomly())
			continue;

		//	Get table

		CCountTable *pTable = retResult->Table.SetAt(pSystem->GetType()->GetUNID());

		//	Add to count

		bool bNew;
		int *pCount = pTable->SetAt(pType->GetUNID(), &bNew);
		if (bNew)
			*pCount = 1;
		else
			*pCount += 1;
		}
	}