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

enum ETypes
	{
	outputFreq,
	outputFillLocations,
	};

struct SOptions
	{
	ETypes iType = outputFreq;
	int iSystemSample = 1;
	TArray<CString> Cols;

	bool bLogo = false;
	bool bAll = false;
	bool bRawData = false;
	};

void CalcSystemStats (CUniverse &Universe, const SOptions &Options, CSystemCreateStats &Stats);
void OutputFillLocations (const CSystemCreateStats &Stats, const SOptions &Options);
void OutputFreq (const CSystemCreateStats &Stats, const SOptions &Options);

void GenerateEncounterFrequency (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	//	Options

	SOptions Options;
	if (pCmdLine->GetAttributeBool(CONSTLIT("fillLocations")))
		Options.iType = outputFillLocations;
	else
		Options.iType = outputFreq;

	Options.iSystemSample = pCmdLine->GetAttributeIntegerBounded(CONSTLIT("count"), 1, -1, 1);
	Options.bLogo = !pCmdLine->GetAttributeBool(CONSTLIT("noLogo"));
	Options.bAll = pCmdLine->GetAttributeBool(CONSTLIT("all"));
	Options.bRawData = pCmdLine->GetAttributeBool(CONSTLIT("rawData"));
	
	//	Additional columns

	for (int i = 0; i < pCmdLine->GetAttributeCount(); i++)
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
				Options.Cols.Insert(strPatternSubst(CONSTLIT("%s:%s"), sAttrib, sValue));
			else
				Options.Cols.Insert(sAttrib);
			}
		}

	//	Different options based on type

	switch (Options.iType)
		{
		case outputFillLocations:
			{
			CSystemCreateStats Stats;
			CalcSystemStats(Universe, Options, Stats);
			OutputFillLocations(Stats, Options);
			break;
			}

		case outputFreq:
			{
			CSystemCreateStats Stats;
			CalcSystemStats(Universe, Options, Stats);
			OutputFreq(Stats, Options);
			break;
			}
		}
	}

void CalcSystemStats (CUniverse &Universe, const SOptions &Options, CSystemCreateStats &Stats)
	{
	for (int i = 0; i < Options.iSystemSample; i++)
		{
		if (Options.bLogo)
			printf("pass %d...\n", i+1);

		//	Initialize the game

		CString sError;
		if (Universe.InitGame(0, &sError) != NOERROR)
			{
			printf("%s\n", sError.GetASCIIZPointer());
			return;
			}

		for (int j = 0; j < Universe.GetTopologyNodeCount(); j++)
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
	}

void OutputFillLocations (const CSystemCreateStats &Stats, const SOptions &Options)
	{
	printf("Level\tNode\tSystem\tAttribs\tCount\tEncounter\tSystem Chance\tFill Table Chance\tLocation Chance\n");

	for (int i = 0; i < Stats.GetFillLocationsTableCount(); i++)
		{
		const CSystemCreateStats::SFillLocationsTable &Entry = Stats.GetFillLocationsTable(i);

		for (int j = 0; j < Entry.SystemProb.GetCount(); j++)
			{
			CStationType *pType = Entry.SystemProb.GetKey(j);

			int iSystemPercent = mathRound(Entry.SystemProb.GetScaledChance(j, 1000));
			int iFillPercent = mathRound(Entry.FillProb.GetScaledChanceByValue(pType, 1000));
			int iLocationPercent = mathRound(Entry.LocationProb.GetScaledChanceByValue(pType, 1000));

			printf("%d\t%s\t%s\t%s\t%d\t%s\t%d.%d\t%d.%d\t%d.%d\n",
					Entry.iLevel,
					Entry.sNodeID.GetASCIIZPointer(),
					Entry.sSystemName.GetASCIIZPointer(),
					Entry.sSystemAttribs.GetASCIIZPointer(),
					Entry.iCount,
					pType->GetNounPhrase().GetASCIIZPointer(), 
					iSystemPercent / 10,
					iSystemPercent % 10,
					iFillPercent / 10,
					iFillPercent % 10,
					iLocationPercent / 10,
					iLocationPercent % 10);
			}
		}
	}

void OutputFreq (const CSystemCreateStats &Stats, const SOptions &Options)
	{
	//	Generate a table for each encounter

	TSortMap<CStationType *, SEncounterFreqEntry> EncounterFreq;
	for (int i = 0; i < Stats.GetEncounterTableCount(); i++)
		{
		const CSystemCreateStats::SEncounterTable &Table = Stats.GetEncounterTable(i);

		//	Skip if only planets and asteroids

		if (!Table.bHasStation && !Options.bAll)
			continue;

		//	For each entry in the table, add to the frequency table

		for (int j = 0; j < Table.Table.GetCount(); j++)
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
	for (int i = 0; i < Options.Cols.GetCount(); i++)
		printf("\t%s", Options.Cols[i].GetASCIIZPointer());
	printf("\n");

	for (int i = 0; i < EncounterFreq.GetCount(); i++)
		{
		CStationType *pEncounterType = EncounterFreq.GetKey(i);
		int iAveWhole = EncounterFreq[i].iTotalChance / EncounterFreq[i].iCount;
		int iAveFrac = 1000 * (EncounterFreq[i].iTotalChance % EncounterFreq[i].iCount) / EncounterFreq[i].iCount;

		printf("%d\t%s\t%s\t%d\t%d.%03d\t%d\t%d",
				pEncounterType->GetLevel(),
				pEncounterType->GetDataField(CONSTLIT("category")).GetASCIIZPointer(),
				pEncounterType->GetNounPhrase().GetASCIIZPointer(),
				EncounterFreq[i].iCount,
				iAveWhole,
				iAveFrac,
				EncounterFreq[i].iMinChance,
				EncounterFreq[i].iMaxChance);

		for (int j = 0; j < Options.Cols.GetCount(); j++)
			{
			CString sValue = pEncounterType->GetDataField(Options.Cols[j]);
			printf("\t%s", sValue.GetASCIIZPointer());
			}

		printf("\n");
		}

	//	Output raw data

	if (Options.bRawData)
		{
		//	Loop over stats and output tables

		for (int i = 0; i < Stats.GetEncounterTableCount(); i++)
			{
			const CSystemCreateStats::SEncounterTable &Table = Stats.GetEncounterTable(i);

			//	Skip if only planets and asteroids

			if (!Table.bHasStation && !Options.bAll)
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
			for (int j = 0; j < Table.LabelAttribs.GetCount(); j++)
				{
				if (j != 0)
					printf(", ");
				printf(Table.LabelAttribs[j].GetASCIIZPointer());
				}
			printf("\n");

			//	Entries

			for (int j = 0; j < Table.Table.GetCount(); j++)
				printf("%s\t%d\n", Table.Table.GetAt(j)->GetNounPhrase().GetASCIIZPointer(), Table.Table.GetChance(j));

			printf("\n");
			}
		}
	}
