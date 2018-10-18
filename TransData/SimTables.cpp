//	SimTables.cpp
//
//	Generate stats based on simulated games

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define TYPE_COUNT_PATTERN				CONSTLIT("TransData_TypeCount_%08x.txt")

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

struct STypeStats
	{
	TNumberSeries<double> PerGame;			//	Number appearing in the entire game
	TNumberSeries<double> PerLevel[MAX_TECH_LEVEL];	//	Number appearing at each level
	};

struct STypeInfo
	{
	STypeInfo (void)
		{
		int i;

		iTotalCount = 0;
		for (i = 0; i < MAX_TECH_LEVEL; i++)
			PerLevel[i] = 0;
		}

	int iTotalCount;						//	Total count so far
	int PerLevel[MAX_TECH_LEVEL];			//	Total count per level
	};

void AccumulateSystem (CTopologyNode *pNode, CSystem *pSystem, TSortMap<DWORD, STypeInfo> &AllTypes);
ALERROR OutputTypeTable (DWORD dwAdventureUNID, TSortMap<DWORD, STypeStats> &AllStats, int iSystemSample);

void GenerateSimTables (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	ALERROR error;
	int i, j, k;

	int iSystemSample = pCmdLine->GetAttributeInteger(CONSTLIT("count"));
	if (iSystemSample == 0)
		iSystemSample = DEFAULT_SYSTEM_SAMPLE;

	//	Keep track of stats for each type

	TSortMap<DWORD, STypeStats> AllStats;
	for (i = 0; i < iSystemSample; i++)
		{
		TSortMap<DWORD, STypeInfo> AllTypes;

		printf("sample %d...\n", i+1);

		//	Initialize the game

		CString sError;
		if (error = Universe.InitGame(0, &sError))
			{
			printf("%s\n", sError.GetASCIIZPointer());
			return;
			}

		//	Loop over all nodes

		for (j = 0; j < Universe.GetTopologyNodeCount(); j++)
			{
			CTopologyNode *pNode = Universe.GetTopologyNode(j);

			//	Skip end game nodes

			if (pNode->IsEndGame())
				continue;

			//	Create the system

			CSystem *pSystem;
			if (error = Universe.CreateStarSystem(pNode, &pSystem, &sError))
				{
				printf("ERROR: Unable to create star system: %s\n", sError.GetASCIIZPointer());
				return;
				}

			//	Accumulate

			AccumulateSystem(pNode, pSystem, AllTypes);

			//	Done with old system

			Universe.DestroySystem(pSystem);
			}

		//	Now accumulate all stats

		for (j = 0; j < Universe.GetDesignTypeCount(); j++)
			{
			CDesignType *pType = Universe.GetDesignType(j);
			STypeStats *pStats = AllStats.SetAt(pType->GetUNID());

			STypeInfo *pTypeInfo = AllTypes.GetAt(pType->GetUNID());
			if (pTypeInfo)
				{
				pStats->PerGame.Insert(pTypeInfo->iTotalCount);

				for (k = 0; k < MAX_TECH_LEVEL; k++)
					pStats->PerLevel[k].Insert(pTypeInfo->PerLevel[k]);
				}
			else
				{
				pStats->PerGame.Insert(0);

				for (k = 0; k < MAX_TECH_LEVEL; k++)
					pStats->PerLevel[k].Insert(0);
				}
			}

		Universe.Reinit();
		}

	//	Output

	if (error = OutputTypeTable(Universe.GetDesignCollection().GetAdventureUNID(), AllStats, iSystemSample))
		return;

	//	Create a table with the sum of all items for the game

	printf("Total count statistic computed.\n");
	}

void AccumulateSystem (CTopologyNode *pNode, CSystem *pSystem, TSortMap<DWORD, STypeInfo> &AllTypes)
	{
	int j;

	int iSystemLevel = pSystem->GetLevel();

	//	Add the encounters to the appropriate tables

	for (j = 0; j < pSystem->GetObjectCount(); j++)
		{
		CSpaceObject *pObj = pSystem->GetObject(j);

		if (pObj)
			{
			//	Add this encounter to the table

			CDesignType *pEncounterType;
			if (pEncounterType = pObj->GetEncounterInfo())
				{
				STypeInfo *pInfo = AllTypes.SetAt(pEncounterType->GetUNID());
				pInfo->iTotalCount++;
				pInfo->PerLevel[iSystemLevel]++;
				}

			//	Add the object itself, if different from the encounter

			CDesignType *pType;
			if ((pType = pObj->GetType()) && pType != pEncounterType)
				{
				STypeInfo *pInfo = AllTypes.SetAt(pType->GetUNID());
				pInfo->iTotalCount++;
				pInfo->PerLevel[iSystemLevel]++;
				}

			//	Enumerate the items in this object

			CItemListManipulator ItemList(pObj->GetItemList());
			ItemList.ResetCursor();
			while (ItemList.MoveCursorForward())
				{
				const CItem &Item(ItemList.GetItemAtCursor());

				if (!Item.IsInstalled() && !Item.IsDamaged())
					{
					STypeInfo *pInfo = AllTypes.SetAt(Item.GetType()->GetUNID());
					pInfo->iTotalCount += Item.GetCount();
					pInfo->PerLevel[iSystemLevel] += Item.GetCount();
					}
				}
			}
		}
	}

ALERROR OutputTypeTable (DWORD dwAdventureUNID, TSortMap<DWORD, STypeStats> &AllStats, int iSystemSample)
	{
	ALERROR error;
	int i, j;

	CString sFilespec = strPatternSubst(TYPE_COUNT_PATTERN, dwAdventureUNID);

	//	Output all items to a well-known file

	CTextFileLog Output(sFilespec);
	if (error = Output.Create(FALSE))
		{
		printf("ERROR: Unable to create output file: %s\n", sFilespec.GetASCIIZPointer());
		return error;
		}

	for (i = 0; i < AllStats.GetCount(); i++)
		{
		DWORD dwUNID = AllStats.GetKey(i);
		const STypeStats &Stats = AllStats[i];

		TArray<TNumberSeries<double>::SHistogramPoint> Histogram;
		Stats.PerGame.CalcHistogram(&Histogram);

		CString sLine;
		sLine = strPatternSubst("0x%x\t%d\t%d", dwUNID, (int)((Stats.PerGame.GetMean() * 1000.0) + 0.5), Histogram.GetCount());

		for (j = 0; j < Histogram.GetCount(); j++)
			sLine.Append(strPatternSubst("\t%d\t%d", Histogram[j].iValue, (int)(Histogram[j].rPercent * 100.0 + 0.5)));

		Output.LogOutput(0, sLine);
		}

	if (error = Output.Close())
		{
		printf("ERROR: Unable to create output file: %s\n", sFilespec.GetASCIIZPointer());
		return error;
		}

	return NOERROR;
	}

void ReadLine (char *pPos, char *pEndPos, TArray<int> *retValues, char **retpPos)
	{
	while (true)
		{
		//	Skip to the next number.

		while (pPos < pEndPos && !strIsDigit(pPos) && *pPos != '-' && *pPos != '+' && *pPos != '\n' && *pPos != '\r')
			pPos++;

		if (pPos >= pEndPos || *pPos == '\n' || *pPos == '\r')
			{
			while (pPos < pEndPos && (*pPos == '\n' || *pPos == '\r'))
				pPos++;

			break;
			}

		//	Parse the number

		int iValue = strParseInt(pPos, 0, &pPos);
		retValues->Insert(iValue);
		}

	if (retpPos)
		*retpPos = pPos;
	}

ALERROR LoadDesignTypeStats (DWORD dwAdventureUNID, CDesignTypeStats *retStats)
	{
	ALERROR error;
	int i;

	CString sFilespec = strPatternSubst(TYPE_COUNT_PATTERN, dwAdventureUNID);

	CFileReadBlock Input(sFilespec);
	if (error = Input.Open())
		{
		printf("ERROR: Unable to open total count file. Use /generateSimTables.");
		return error;
		}

	char *pPos = Input.GetPointer(0, -1);
	char *pEndPos = pPos + Input.GetLength();

	while (pPos < pEndPos)
		{
		int iArg = 0;

		//	Read all values

		TArray<int> Values;
		ReadLine(pPos, pEndPos, &Values, &pPos);

		//	Get the entry

		if (Values.GetCount() == 0)
			continue;

		DWORD dwUNID = (DWORD)Values[iArg++];
		SDesignTypeInfo *pInfo = retStats->SetAt(dwUNID);

		//	Get the total count

		if (iArg < Values.GetCount())
			pInfo->rPerGameMeanCount = (Values[iArg++] / 1000.0);

		//	Get the distribution

		if (iArg < Values.GetCount())
			{
			CString sDist;
			int iDistCount = Values[iArg++];

			for (i = 0; i < iDistCount; i++)
				{
				int iValue;
				int iPercent;

				if (iArg < Values.GetCount())
					iValue = Values[iArg++];
				else
					iValue = 0;

				if (iArg < Values.GetCount())
					iPercent = Values[iArg++];
				else
					iPercent = 0;

				sDist.Append(strPatternSubst(CONSTLIT("%d (%d%%) "), iValue, iPercent));
				}

			pInfo->sDistribution = sDist;
			}
		}

	return NOERROR;
	}
