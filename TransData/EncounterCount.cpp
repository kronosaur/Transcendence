//	EncounterCount.cpp
//
//	Counts encounters in a game

#include <stdio.h>

#include <windows.h>
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
	int i, j, k, l;

	//	Options

	int iSystemSample = pCmdLine->GetAttributeIntegerBounded(CONSTLIT("count"), 1, -1, 1);
	bool bLogo = !pCmdLine->GetAttributeBool(CONSTLIT("noLogo"));
	bool bAll = pCmdLine->GetAttributeBool(CONSTLIT("all"));

	//	Additional columns

	TArray<CString> Cols;
	for (i = 0; i < pCmdLine->GetAttributeCount(); i++)
		{
		CString sAttrib = pCmdLine->GetAttributeName(i);

		if (!IsMainCommandParam(sAttrib)
				&& !strEquals(sAttrib, CONSTLIT("count"))
				&& !strEquals(sAttrib, CONSTLIT("encountercount")))
			{
			CString sValue = pCmdLine->GetAttribute(i);
			
			if (!strEquals(sValue, CONSTLIT("true")))
				Cols.Insert(strPatternSubst(CONSTLIT("%s:%s"), sAttrib, sValue));
			else
				Cols.Insert(sAttrib);
			}
		}

	//	Generate systems for multiple games

	TSortMap<CString, SNodeDesc> NodeTable;

	for (i = 0; i < iSystemSample; i++)
		{
		if (bLogo)
			printf("pass %d...\n", i+1);

		//	Initialize the game

		CString sError;
		if (Universe.InitGame(0, &sError) != NOERROR)
			{
			printf("%s\n", sError.GetASCIIZPointer());
			return;
			}

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

	//	Header

	printf("Level\tNode\tSystemType\tCategory\tSovereign\tEncounter\tCount");
	for (i = 0; i < Cols.GetCount(); i++)
		printf("\t%s", Cols[i].GetASCIIZPointer());
	printf("\n");

	//	Output all rows

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

				CSovereign *pSovereign = pEncounterType->GetControllingSovereign();
				CString sSovereign = (pSovereign ? pSovereign->GetNounPhrase() : CONSTLIT("(Unknown)"));

				printf("%d\t%s\t0x%08x\t%s\t%s\t%s\t%d.%03d",
						NodeTable[i].iLevel,
						NodeTable[i].sNodeID.GetASCIIZPointer(),
						NodeTable[i].Table.GetKey(j),
						pEncounterType->GetDataField(CONSTLIT("category")).GetASCIIZPointer(),
						sSovereign.GetASCIIZPointer(),
						pEncounterType->GetNounPhrase().GetASCIIZPointer(),
						iCount,
						iCountFrac);

				for (l = 0; l < Cols.GetCount(); l++)
					{
					CString sValue = pEncounterType->GetDataField(Cols[l]);
					printf("\t%s", sValue.GetASCIIZPointer());
					}

				printf("\n");
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