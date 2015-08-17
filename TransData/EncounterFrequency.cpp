//	EncounterFrequency.cpp
//
//	Counts encounters in a game

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

struct SEncounterFreqEntry
	{
	SEncounterFreqEntry (void) :
			iCount(0),
			iTotalChance(0),
			iMinChance(0),
			pMinTable(NULL),
			iMaxChance(0),
			pMaxTable(NULL)
		{ }

	int iCount;								//	Chances to create
	int iTotalChance;						//	Sum of probabilities

	int iMinChance;							//	Smallest chance
	const CSystemCreateStats::SEncounterTable *pMinTable;

	int iMaxChance;							//	Largest chance
	const CSystemCreateStats::SEncounterTable *pMaxTable;
	};

void GenerateEncounterFrequency (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	enum ETypes
		{
		outputFreq,
		outputFillLocations,
		};

	int i, j;

	//	Options

	int iSystemSample = pCmdLine->GetAttributeIntegerBounded(CONSTLIT("count"), 1, -1, 1);
	bool bLogo = !pCmdLine->GetAttributeBool(CONSTLIT("noLogo"));
	bool bAll = pCmdLine->GetAttributeBool(CONSTLIT("all"));

	bool bRawData = pCmdLine->GetAttributeBool(CONSTLIT("rawData"));
	
	ETypes iType;
	if (pCmdLine->GetAttributeBool(CONSTLIT("fillLocations")))
		iType = outputFillLocations;
	else
		iType = outputFreq;

	//	Additional columns

	TArray<CString> Cols;
	for (i = 0; i < pCmdLine->GetAttributeCount(); i++)
		{
		CString sAttrib = pCmdLine->GetAttributeName(i);

		if (!IsMainCommandParam(sAttrib)
				&& !strEquals(sAttrib, CONSTLIT("count"))
				&& !strEquals(sAttrib, CONSTLIT("fillLocations"))
				&& !strEquals(sAttrib, CONSTLIT("rawData"))
				&& !strEquals(sAttrib, CONSTLIT("encounterfreq")))
			{
			CString sValue = pCmdLine->GetAttribute(i);
			
			if (!strEquals(sValue, CONSTLIT("true")))
				Cols.Insert(strPatternSubst(CONSTLIT("%s:%s"), sAttrib, sValue));
			else
				Cols.Insert(sAttrib);
			}
		}

	//	Generate systems for multiple games

	CSystemCreateStats Stats;
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
			if (Universe.CreateStarSystem(pNode, &pSystem, NULL, &Stats) != NOERROR)
				{
				printf("ERROR: Unable to create star system.\n");
				return;
				}

			//	Done with old system

			Universe.DestroySystem(pSystem);
			}

		Universe.Reinit();
		}

	//	Output based on type

	if (iType == outputFreq)
		{
		//	Generate a table for each encounter

		TSortMap<CStationType *, SEncounterFreqEntry> EncounterFreq;
		for (i = 0; i < Stats.GetEncounterTableCount(); i++)
			{
			const CSystemCreateStats::SEncounterTable &Table = Stats.GetEncounterTable(i);

			//	Skip if only planets and asteroids

			if (!Table.bHasStation && !bAll)
				continue;

			//	For each entry in the table, add to the frequency table

			for (j = 0; j < Table.Table.GetCount(); j++)
				{
				CStationType *pEncounterType = Table.Table.GetAt(j);
				int iChance = Table.Table.GetChance(j);

				SEncounterFreqEntry *pEntry = EncounterFreq.SetAt(pEncounterType);
				pEntry->iCount += Table.iCount;
				pEntry->iTotalChance += Table.iCount * iChance;

				if (pEntry->pMaxTable == NULL || iChance > pEntry->iMaxChance)
					{
					pEntry->pMaxTable = &Table;
					pEntry->iMaxChance = iChance;
					}

				if (pEntry->pMinTable == NULL || iChance < pEntry->iMinChance)
					{
					pEntry->pMinTable = &Table;
					pEntry->iMinChance = iChance;
					}
				}
			}

		//	Output encounter frequency distribution

		printf("Level\tCategory\tEncounter\tTotal Chances\tAverage Chance\tMin Chance\tMax Chance");
		for (i = 0; i < Cols.GetCount(); i++)
			printf("\t%s", Cols[i].GetASCIIZPointer());
		printf("\n");

		for (i = 0; i < EncounterFreq.GetCount(); i++)
			{
			CStationType *pEncounterType = EncounterFreq.GetKey(i);
			int iAveWhole = EncounterFreq[i].iTotalChance / EncounterFreq[i].iCount;
			int iAveFrac = 1000 * (EncounterFreq[i].iTotalChance % EncounterFreq[i].iCount) / EncounterFreq[i].iCount;

			printf("%d\t%s\t%s\t%d\t%d.%03d\t%d\t%d",
					pEncounterType->GetLevel(),
					pEncounterType->GetDataField(CONSTLIT("category")).GetASCIIZPointer(),
					pEncounterType->GetName().GetASCIIZPointer(),
					EncounterFreq[i].iCount,
					iAveWhole,
					iAveFrac,
					EncounterFreq[i].iMinChance,
					EncounterFreq[i].iMaxChance);

			for (j = 0; j < Cols.GetCount(); j++)
				{
				CString sValue = pEncounterType->GetDataField(Cols[j]);
				printf("\t%s", sValue.GetASCIIZPointer());
				}

			printf("\n");
			}

		//	Output raw data

		if (bRawData)
			{
			//	Loop over stats and output tables

			for (i = 0; i < Stats.GetEncounterTableCount(); i++)
				{
				const CSystemCreateStats::SEncounterTable &Table = Stats.GetEncounterTable(i);

				//	Skip if only planets and asteroids

				if (!Table.bHasStation && !bAll)
					continue;

				//	Header

				printf("LEVEL %d (%d location%s)\nSystem Type: %08x\nStations: %s\n", 
						Table.iLevel,
						Table.iCount,
						(Table.iCount == 1 ? "" : "s"),
						Table.pSystemType->GetUNID(),
						Table.sStationCriteria.GetASCIIZPointer());

				//	Location attributes

				printf("Location: ");
				for (j = 0; j < Table.LabelAttribs.GetCount(); j++)
					{
					if (j != 0)
						printf(", ");
					printf(Table.LabelAttribs[j].GetASCIIZPointer());
					}
				printf("\n");

				//	Entries

				for (j = 0; j < Table.Table.GetCount(); j++)
					printf("%s\t%d\n", Table.Table.GetAt(j)->GetName().GetASCIIZPointer(), Table.Table.GetChance(j));

				printf("\n");
				}
			}
		}
	else if (iType == outputFillLocations)
		{
		printf("Level\tSystem\tEncounter\tChance\n");

		for (i = 0; i < Stats.GetFillLocationsTableCount(); i++)
			{
			const CSystemCreateStats::SFillLocationsTable &Entry = Stats.GetFillLocationsTable(i);

			for (j = 0; j < Entry.Table.GetCount(); j++)
				{
				int iPercent = (int)(1000.0 * Entry.Table.GetChance(j) / (Metric)Entry.Table.GetTotalChance());

				printf("%d\t%s\t%s\t%d.%d\n",
						Entry.iLevel,
						Entry.sSystemName.GetASCIIZPointer(),
						Entry.Table[j]->GetName().GetASCIIZPointer(), 
						iPercent / 10,
						iPercent % 10);
				}
			}
		}
	}
