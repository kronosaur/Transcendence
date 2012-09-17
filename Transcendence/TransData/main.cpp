//	TransData
//
//	TransData is used to report information out of a Transcendence
//	datafile

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define NOARGS								CONSTLIT("noArgs")
#define QUESTION_MARK_SWITCH				CONSTLIT("?")
#define HELP_SWITCH							CONSTLIT("help")
#define H_SWITCH							CONSTLIT("h")
#define NO_LOGO_SWITCH						CONSTLIT("nologo")

#define ADVENTURE_SWITCH					CONSTLIT("adventure")
#define ARMOR_TABLE_SWITCH					CONSTLIT("armortable")
#define ATTRIBUTE_LIST_SWITCH				CONSTLIT("attributelist")
#define DEBUG_SWITCH						CONSTLIT("debug")
#define DECOMPILE_SWITCH					CONSTLIT("decompile")
#define EFFECT_IMAGE_SWITCH					CONSTLIT("effectImage")
#define ENCOUNTER_SIM_SWITCH				CONSTLIT("encountersim")
#define ENCOUNTER_TABLE_SWITCH				CONSTLIT("encountertable")
#define ENTITIES_SWITCH						CONSTLIT("entitiesReference")
#define HEXARC_TEST_SWITCH					CONSTLIT("hexarcTest")
#define ITEM_FREQUENCY_SWITCH				CONSTLIT("itemsim")
#define ITEM_TABLE_SWITCH					CONSTLIT("itemtable")
#define LOOT_SIM_SWITCH						CONSTLIT("lootsim")
#define PERF_TEST_SWITCH					CONSTLIT("perftest")
#define RANDOM_ITEMS_SWITCH					CONSTLIT("randomitems")
#define RANDOM_NUMBER_TEST					CONSTLIT("randomnumbertest")
#define RUN_SWITCH							CONSTLIT("run")
#define SHIELD_TEST_SWITCH					CONSTLIT("shieldtest")
#define SHIP_IMAGE_SWITCH					CONSTLIT("shipimage")
#define SHIP_IMAGES_SWITCH					CONSTLIT("shipimages")
#define SHIP_TABLE_SWITCH					CONSTLIT("shiptable")
#define SIM_TABLES_SWITCH					CONSTLIT("simTables")
#define SMOKE_TEST_SWITCH					CONSTLIT("smoketest")
#define SNAPSHOT_SWITCH						CONSTLIT("snapshot")
#define STATION_FREQUENCY_SWITCH			CONSTLIT("stationfrequency")
#define STATION_PLACE_SIM_SWITCH			CONSTLIT("stationSeparationSim")
#define STATS_SWITCH						CONSTLIT("stats")
#define STD_ARMOR_SWITCH					CONSTLIT("stdarmor")
#define STD_SHIELD_SWITCH					CONSTLIT("stdshield")
#define SYSTEM_LABELS_SWITCH				CONSTLIT("systemlabels")
#define SYSTEM_TEST_SWITCH					CONSTLIT("systemtest")
#define TOPOLOGY_SWITCH						CONSTLIT("topology")
#define TYPE_DEPENDENCIES_SWITCH			CONSTLIT("typeDependencies")
#define TYPE_ISLANDS_SWITCH					CONSTLIT("typeIslands")
#define WORD_GENERATOR_SWITCH				CONSTLIT("wordgenerator")
#define WORD_LIST_SWITCH					CONSTLIT("wordlist")

#define CLEAR_REGISTERED_SWITCH				CONSTLIT("clearRegistered")
#define GAME_FILE_SWITCH					CONSTLIT("gameFile")

class CHost : public CUniverse::IHost
	{
	public:
		CHost (void) { m_DefaultFont.Create(CONSTLIT("Tahoma"), -13); }

		virtual void ConsoleOutput (const CString &sLine) { printf("%s\n", sLine.GetASCIIZPointer()); }
		virtual void DebugOutput (const CString &sLine) { printf("%s\n", sLine.GetASCIIZPointer()); }
		virtual const CG16bitFont *GetFont (const CString &sFont) { return &m_DefaultFont; }

	private:
		CG16bitFont m_DefaultFont;
	};

void AlchemyMain (CXMLElement *pCmdLine);

int main (int argc, char *argv[ ], char *envp[ ])

//	main
//
//	main entry-point

	{
	//	Output Windows Western codepage
	::SetConsoleOutputCP(1252);

#ifdef DEBUG_MEMORY_LEAKS
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 
#endif

	if (!kernelInit(KERNEL_FLAG_INTERNETS))
		{
		printf("ERROR: Unable to initialize Alchemy kernel.\n");
		return 1;
		}

	//	Do it

	{
	ALERROR error;
	CXMLElement *pCmdLine;
	if (error = CreateXMLElementFromCommandLine(argc, argv, &pCmdLine))
		{
		printf("ERROR: Unable to parse command line.\n");
		return 1;
		}

	AlchemyMain(pCmdLine);

	delete pCmdLine;
	}

	//	Done

	kernelCleanUp();
	return 0;
	}

void AlchemyMain (CXMLElement *pCmdLine)

//	AlchemyMain
//
//	Main entry-point after kernel initialization

	{
	ALERROR error;
	bool bLogo = !pCmdLine->GetAttributeBool(NO_LOGO_SWITCH);

	//	Welcome message

	if (bLogo)
		{
		SFileVersionInfo VersionInfo;
		if (fileGetVersionInfo(NULL_STR, &VersionInfo) != NOERROR)
			{
			printf("ERROR: Unable to get version info.\n");
			return;
			}

		if (pCmdLine->FindAttribute(RUN_SWITCH))
			{
			printf("TLisp Shell %s\n", (LPSTR)VersionInfo.sProductVersion);
			printf("%s\n\n", (LPSTR)VersionInfo.sCopyright);
			}
		else
			{
			printf("TransData %s\n", (LPSTR)VersionInfo.sProductVersion);
			printf("%s\n\n", (LPSTR)VersionInfo.sCopyright);
			}
		}

	if (pCmdLine->GetAttributeBool(NOARGS)
			|| pCmdLine->GetAttributeBool(QUESTION_MARK_SWITCH)
			|| pCmdLine->GetAttributeBool(HELP_SWITCH)
			|| pCmdLine->GetAttributeBool(H_SWITCH))
		{
		ShowHelp(pCmdLine);
		return;
		}

	//	Set debug log

#ifdef DEBUG
	CTextFileLog LogFile(CONSTLIT("TransData.log"));
	::kernelSetDebugLog(&LogFile, FALSE);
#endif

	//	Figure out the data file that we're working on

	CString sDataFile = CONSTLIT("Transcendence");

	//	See if we are doing a command that does not require parsing

	if (pCmdLine->GetAttributeBool(WORD_LIST_SWITCH))
		{
		GenerateWordList(sDataFile, pCmdLine);
		::kernelSetDebugLog(NULL);
		return;
		}
	else if (pCmdLine->GetAttributeBool(WORD_GENERATOR_SWITCH))
		{
		WordGenerator(pCmdLine);
		::kernelSetDebugLog(NULL);
		return;
		}
	else if (pCmdLine->GetAttributeBool(ENTITIES_SWITCH))
		{
		GenerateEntitiesTable(sDataFile, pCmdLine);
		::kernelSetDebugLog(NULL);
		return;
		}
	else if (pCmdLine->GetAttributeBool(DECOMPILE_SWITCH))
		{
		Decompile(sDataFile, pCmdLine);
		::kernelSetDebugLog(NULL);
		return;
		}
	else if (pCmdLine->GetAttributeBool(CLEAR_REGISTERED_SWITCH))
		{
		const CString sSaveFile = pCmdLine->GetAttribute(GAME_FILE_SWITCH);
		
		CGameFile Game;
		if (error = Game.Open(sSaveFile))
			{
			printf("ERROR: Can't open %s.\n", (char *)sSaveFile);
			::kernelSetDebugLog(NULL);
			return;
			}

		if (error = Game.ClearRegistered())
			{
			printf("ERROR: Unable to clear registered bit.\n");
			::kernelSetDebugLog(NULL);
			return;
			}

		::kernelSetDebugLog(NULL);
		return;
		}

	//	Figure out what adventure we need

	DWORD dwAdventureUNID;
	if (!pCmdLine->FindAttributeInteger(ADVENTURE_SWITCH, (int *)&dwAdventureUNID))
		dwAdventureUNID = DEFAULT_ADVENTURE_EXTENSION_UNID;

	//	See if we need to load images

	DWORD dwInitFlags = 0;
	if (pCmdLine->GetAttributeBool(EFFECT_IMAGE_SWITCH)
			|| pCmdLine->GetAttributeBool(SHIP_IMAGE_SWITCH) 
			|| pCmdLine->GetAttributeBool(SHIP_IMAGES_SWITCH)
			|| pCmdLine->GetAttributeBool(SMOKE_TEST_SWITCH)
			|| pCmdLine->GetAttributeBool(SNAPSHOT_SWITCH))
		;
	else
		dwInitFlags |= flagNoResources;

	//	We don't need a version check

	dwInitFlags |= flagNoVersionCheck;

	//	We're not loading the game

	dwInitFlags |= flagNewGame;

	//	Open the universe

	if (bLogo)
		printf("Loading...");

	CHost Host;
	CUniverse Universe;
	Universe.SetHost(&Host);
	CString sError;

	CUniverse::SInitDesc Ctx;
	Ctx.bDebugMode = pCmdLine->GetAttributeBool(DEBUG_SWITCH);
	Ctx.dwAdventure = dwAdventureUNID;
	Ctx.bNoResources = ((dwInitFlags & flagNoResources) == flagNoResources);
	Ctx.bDefaultExtensions = true;

	if (error = Universe.Init(Ctx, &sError))
		{
		printf("\n%s\n", sError.GetASCIIZPointer());
		::kernelSetDebugLog(NULL);
		return;
		}

	if (error = Universe.InitGame(&sError))
		{
		printf("\n%s\n", sError.GetASCIIZPointer());
		::kernelSetDebugLog(NULL);
		return;
		}

	if (bLogo)
		printf("done.\n");

	//	Mark everything as known

	MarkItemsKnown(Universe);

	//	Figure out what to do

	if (pCmdLine->GetAttributeBool(ARMOR_TABLE_SWITCH))
		GenerateArmorTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(ENCOUNTER_TABLE_SWITCH))
		GenerateEncounterTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(ITEM_FREQUENCY_SWITCH))
		GenerateItemFrequencyTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(ITEM_TABLE_SWITCH))
		{
		CIDTable EntityTable(TRUE, TRUE);
		ComputeUNID2EntityTable(sDataFile, EntityTable);
		GenerateItemTable(Universe, pCmdLine, EntityTable);
		}
	else if (pCmdLine->GetAttributeBool(LOOT_SIM_SWITCH))
		GenerateLootSim(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(RANDOM_ITEMS_SWITCH))
		GenerateRandomItemTables(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(RANDOM_NUMBER_TEST))
		DoRandomNumberTest();
	else if (pCmdLine->FindAttribute(RUN_SWITCH))
		Run(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SHIELD_TEST_SWITCH))
		GenerateShieldStats(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(EFFECT_IMAGE_SWITCH))
		GenerateEffectImage(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SHIP_IMAGE_SWITCH))
		GenerateShipImage(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SHIP_IMAGES_SWITCH))
		GenerateShipImageChart(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SHIP_TABLE_SWITCH))
		{
		CIDTable EntityTable(TRUE, TRUE);
		ComputeUNID2EntityTable(sDataFile, EntityTable);
		GenerateShipTable(Universe, pCmdLine, EntityTable);
		}
	else if (pCmdLine->GetAttributeBool(SIM_TABLES_SWITCH))
		GenerateSimTables(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SMOKE_TEST_SWITCH))
		DoSmokeTest(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SNAPSHOT_SWITCH))
		GenerateSnapshot(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(STATION_FREQUENCY_SWITCH))
		GenerateStationFrequencyTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(STATION_PLACE_SIM_SWITCH))
		GenerateStationPlaceSim(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(STATS_SWITCH))
		GenerateStats(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(STD_ARMOR_SWITCH))
		GenerateStdArmorTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(STD_SHIELD_SWITCH))
		GenerateStdShieldTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SYSTEM_LABELS_SWITCH))
		GenerateSystemLabelCount(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SYSTEM_TEST_SWITCH))
		GenerateSystemTest(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(TOPOLOGY_SWITCH))
		GenerateTopology(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(ATTRIBUTE_LIST_SWITCH))
		GenerateAttributeList(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(PERF_TEST_SWITCH))
		PerformanceTest(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(ENCOUNTER_SIM_SWITCH))
		RunEncounterSim(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(TYPE_DEPENDENCIES_SWITCH))
		GenerateTypeDependencies(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(TYPE_ISLANDS_SWITCH))
		GenerateTypeIslands(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(HEXARC_TEST_SWITCH))
		HexarcTest(Universe, pCmdLine);
	else
		GenerateStats(Universe, pCmdLine);

	//	Done

	::kernelSetDebugLog(NULL);
	}

void ComputeUNID2EntityTable (const CString &sDataFile, CIDTable &EntityTable)
	{
	ALERROR error;
	int i;
	CString sError;

	//	Open the XML file

	CResourceDb Resources(sDataFile);
	if (error = Resources.Open(0, &sError))
		{
		printf("%s\n", (LPSTR)sError);
		return;
		}

	CExternalEntityTable *pEntities;
	if (error = Resources.LoadEntities(&sError, &pEntities))
		{
		printf("%s\n", sError.GetASCIIZPointer());
		return;
		}

	//	Add the entities to an index of UNID to entity name

	for (i = 0; i < pEntities->GetCount(); i++)
		{
		CString sEntity, sValue;
		pEntities->GetEntity(i, &sEntity, &sValue);

		//	Add to the list

		DWORD dwUNID = strToInt(sValue, 0);
		EntityTable.AddEntry(dwUNID, new CString(sEntity));
		}
	}

void MarkItemsKnown (CUniverse &Universe)
	{
	int i;

	for (i = 0; i < Universe.GetItemTypeCount(); i++)
		{
		CItemType *pItem = Universe.GetItemType(i);
		pItem->SetKnown();
		pItem->SetShowReference();
		}
	}

void Run (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	ALERROR error;
	CCodeChain &CC = g_pUniverse->GetCC();

	//	Prepare the universe

	CTopologyNode *pNode = g_pUniverse->GetFirstTopologyNode();
	if (pNode == NULL)
		{
		printf("ERROR: No topology node found.\n");
		return;
		}

	CSystem *pSystem;
	if (error = g_pUniverse->CreateStarSystem(pNode, &pSystem))
		{
		printf("ERROR: Unable to create star system.\n");
		return;
		}

	//	Set the POV

	CSpaceObject *pPOV = pSystem->GetObject(0);
	g_pUniverse->SetPOV(pPOV);
	pSystem->SetPOVLRS(pPOV);

	//	Prepare system

	g_pUniverse->UpdateExtended();
	g_pUniverse->GarbageCollectLibraryBitmaps();

	//	If we have a command, invoke it

	CString sCommand = pCmdLine->GetAttribute(RUN_SWITCH);
	if (!sCommand.IsBlank() && !strEquals(sCommand, CONSTLIT("True")))
		{
		CCodeChainCtx Ctx;
		ICCItem *pCode = Ctx.Link(sCommand, 0, NULL);
		ICCItem *pResult = Ctx.Run(pCode);

		CString sOutput;
		if (pResult->IsIdentifier())
			sOutput = pResult->Print(&CC, PRFLAG_NO_QUOTES | PRFLAG_ENCODE_FOR_DISPLAY);
		else
			sOutput = CC.Unlink(pResult);

		Ctx.Discard(pResult);
		Ctx.Discard(pCode);

		//	Output result

		printf("%s\n", sOutput.GetASCIIZPointer());
		}

	//	Otherwise, we enter a command loop

	else
		{
		//	Welcome

		printf("(help) for function help.\n");
		printf("\\q to quit.\n\n");

		//	Loop

		while (true)
			{
			char szBuffer[1024];
			printf(": ");
			gets_s(szBuffer, sizeof(szBuffer)-1);
			CString sCommand(szBuffer);

			//	Escape codes

			if (*sCommand.GetASCIIZPointer() == '\\')
				{
				//	Quit command

				if (strStartsWith(sCommand, CONSTLIT("\\q")))
					break;
				else if (strStartsWith(sCommand, CONSTLIT("\\?"))
						|| strStartsWith(sCommand, CONSTLIT("\\h")))
					{
					printf("TLisp Shell\n\n");
					printf("\\h      Show this help\n");
					printf("\\q      Quit\n");

					printf("\n(help) for function help.\n");
					}
				}

			//	Null command

			else if (sCommand.IsBlank())
				NULL;

			//	Command

			else
				{
				CCodeChainCtx Ctx;
				ICCItem *pCode = Ctx.Link(sCommand, 0, NULL);
				ICCItem *pResult = Ctx.Run(pCode);

				CString sOutput;
				if (pResult->IsIdentifier())
					sOutput = pResult->Print(&CC, PRFLAG_NO_QUOTES | PRFLAG_ENCODE_FOR_DISPLAY);
				else
					sOutput = CC.Unlink(pResult);

				Ctx.Discard(pResult);
				Ctx.Discard(pCode);

				//	Output result

				printf("%s\n", sOutput.GetASCIIZPointer());
				}
			}
		}
	}
