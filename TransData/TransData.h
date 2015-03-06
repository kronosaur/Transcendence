//	TransData.h
//
//	Include file for TranData app

#ifndef INCL_TRANSDATA
#define INCL_TRANSDATA

#ifndef INCL_TSE
#include "TSE.h"
#endif

#include "Utilities.h"

//	Used by sim tables

class EntryInfo : public CObject
	{
	public:
		EntryInfo (void) : CObject(NULL) { }

		DWORD dwUNID;
		double rTotalCount;
	};

class ItemInfo : public CObject
	{
	public:
		ItemInfo (void) : CObject(NULL) { }

		CItemType *pType;							//	Item type
		int iTotalCount;							//	Total times this item type has appeared
		double rTotalCount;
	};

const int MAX_FREQUENCY_COUNT = 12;

class StationInfo : public CObject
	{
	public:
		StationInfo (void) : CObject(NULL) { }

		CStationType *pType;
		CString sCategory;

		int iFreqCount[MAX_FREQUENCY_COUNT];		//	For each count, then number of times that
													//	number of stations have appeared in the
													//	system instance.

		int iTotalCount;							//	Total times this station type has appeared
													//	the system instance.
		double rTotalCount;

		int iSystemCount;							//	Number of system instances with this
													//	station type

		int iTempCount;								//	Temp count for a specific system instance
	};

struct SDesignTypeInfo
	{
	SDesignTypeInfo (void) :
			rPerGameMedianCount(0.0),
			rPerGameMeanCount(0.0)
		{ }

	double rPerGameMedianCount;				//	Median encountered per game
	double rPerGameMeanCount;				//	Mean encountered per game
	CString sDistribution;					//	"1 (5%); 2 (10%); 3 (70%); 4 (10%); 5 (5%)"
	};

typedef TSortMap<DWORD, SDesignTypeInfo> CDesignTypeStats;

//	Functions

char *FrequencyChar (int iFreq);
void ShowHelp (CXMLElement *pCmdLine);
void MarkItemsKnown (CUniverse &Universe);
bool OutputImage (CG32bitImage &Image, const CString &sFilespec);
ALERROR LoadDesignTypeStats (CDesignTypeStats *retStats);

void Decompile (const CString &sDataFile, CXMLElement *pCmdLine);
void DoRandomNumberTest (void);
void DoSmokeTest (CUniverse &Universe, CXMLElement *pCmdLine);
void DoTradeSim (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateArmorTable (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateAttributeList (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateEffectImage (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateEncounterCount (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateEncounterFrequency (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateEncounterTable (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateEntitiesTable (const CString &sDataFile, CXMLElement *pCmdLine);
void GenerateImageChart (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateItemFrequencyTable (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateItemTable (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateLootSim (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateRandomItemTables (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateShieldStats (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateShipImage (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateShipImageChart (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateShipTable (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateSimTables (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateSnapshot (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateStationFrequencyTable (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateStationPlaceSim (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateStats (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateStdArmorTable (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateStdShieldTable (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateSystemCount (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateSystemImages (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateSystemLabelCount (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateSystemTest (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateTopology (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateTopologyMap (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateTypeDependencies (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateTypeIslands (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateTypeTable (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateWeaponEffectChart (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateWordList (const CString &sDataFile, CXMLElement *pCmdLine);
void GenerateWorldImageChart (CUniverse &Universe, CXMLElement *pCmdLine);
void HexarcTest (CUniverse &Universe, CXMLElement *pCmdLine);
void PerformanceTest (CUniverse &Universe, CXMLElement *pCmdLine);
void Run (CUniverse &Universe, CXMLElement *pCmdLine);
void RunEncounterSim (CUniverse &Universe, CXMLElement *pCmdLine);
void WordGenerator (CXMLElement *pCmdLine);

//	Don't care about warning of depecrated functions (e.g., sprintf)
#pragma warning(disable: 4996)

#endif