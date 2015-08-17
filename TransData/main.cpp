//	TransData
//
//	TransData is used to report information out of a Transcendence
//	datafile

#include <stdio.h>

#include <windows.h>
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
#define ENCOUNTER_COUNT_SWITCH				CONSTLIT("encountercount")
#define ENCOUNTER_FREQ_SWITCH				CONSTLIT("encounterfreq")
#define ENCOUNTER_SIM_SWITCH				CONSTLIT("encountersim")
#define ENCOUNTER_TABLE_SWITCH				CONSTLIT("encountertable")
#define ENTITIES_SWITCH						CONSTLIT("entitiesReference")
#define EXTENSION_FOLDER_ATTRIB				CONSTLIT("extensionFolder")
#define EXTENSIONS_ATTRIB					CONSTLIT("extensions")
#define HEXARC_TEST_SWITCH					CONSTLIT("hexarcTest")
#define ITEM_FREQUENCY_SWITCH				CONSTLIT("itemsim")
#define ITEM_TABLE_SWITCH					CONSTLIT("itemtable")
#define LOOT_SIM_SWITCH						CONSTLIT("lootsim")
#define PERF_TEST_SWITCH					CONSTLIT("perftest")
#define RANDOM_ITEMS_SWITCH					CONSTLIT("randomitems")
#define RANDOM_NUMBER_TEST					CONSTLIT("randomnumbertest")
#define RUN_SWITCH							CONSTLIT("run")
#define RUN_FILE_SWITCH						CONSTLIT("runFile")
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
#define SYSTEM_COUNT_SWITCH					CONSTLIT("systemcount")
#define SYSTEM_IMAGES_SWITCH				CONSTLIT("systemImages")
#define SYSTEM_LABELS_SWITCH				CONSTLIT("systemlabels")
#define SYSTEM_TEST_SWITCH					CONSTLIT("systemtest")
#define TOPOLOGY_SWITCH						CONSTLIT("topology")
#define TOPOLOGY_MAP_SWITCH					CONSTLIT("topologyMap")
#define TRADE_SIM_SWITCH					CONSTLIT("tradeSim")
#define TYPE_DEPENDENCIES_SWITCH			CONSTLIT("typeDependencies")
#define IMAGES_SWITCH						CONSTLIT("typeImages")
#define TYPE_ISLANDS_SWITCH					CONSTLIT("typeIslands")
#define TYPE_TABLE_SWITCH					CONSTLIT("typeTable")
#define WEAPON_IMAGES_SWITCH				CONSTLIT("weaponImages")
#define WORD_GENERATOR_SWITCH				CONSTLIT("wordgenerator")
#define WORD_LIST_SWITCH					CONSTLIT("wordlist")
#define WORLD_IMAGES_SWITCH					CONSTLIT("worldImages")

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
ALERROR CreateXMLElementFromDataFile (const CString &sFilespec, CXMLElement **retpDataFile, CString *retsError);
ALERROR InitUniverse (CUniverse &Universe, CHost &Host, const CString &sFilespec, CXMLElement *pCmdLine, CString *retsError);

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
	CString sError;

	//	Look for a TransData.xml file. If we find it, load it.

	CXMLElement *pOptions = NULL;
	CString sDefaultOptionsFile = CONSTLIT("TransData.xml");
	if (pathExists(sDefaultOptionsFile))
		{
		if (error = CreateXMLElementFromDataFile(sDefaultOptionsFile, &pOptions, &sError))
			{
			printf("ERROR: Unable to parse %s: %s.\n", sDefaultOptionsFile.GetASCIIZPointer(), sError.GetASCIIZPointer());
			return 1;
			}
		}

	//	Otherwise we just create an empty element

	else
		pOptions = new CXMLElement(CONSTLIT("TransData"), NULL);

	//	Load the command line and merge it into pOptions

	CXMLElement *pCmdLine;
	if (error = CreateXMLElementFromCommandLine(argc, argv, &pCmdLine))
		{
		printf("ERROR: Unable to parse command line.\n");
		return 1;
		}

	pOptions->MergeFrom(pCmdLine);
	delete pCmdLine;

	//	If we have a text argument, then use it as a command-line file:

	const CString &sArg = pOptions->GetContentText(0);
	if (!sArg.IsBlank())
		{
		CXMLElement *pDataFile;
		if (error = CreateXMLElementFromDataFile(sArg, &pDataFile, &sError))
			{
			printf("ERROR: %s\n", sError.GetASCIIZPointer());
			return 1;
			}

		//	Merge into options

		pOptions->MergeFrom(pDataFile);
		delete pDataFile;
		}

	//	Now run

	AlchemyMain(pOptions);

	//	Done

	delete pOptions;
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

	//	Welcome message

	bool bLogo = !pCmdLine->GetAttributeBool(NO_LOGO_SWITCH);
	if (bLogo)
		{
		SFileVersionInfo VersionInfo;
		if (fileGetVersionInfo(NULL_STR, &VersionInfo) != NOERROR)
			{
			printf("ERROR: Unable to get version info.\n");
			return;
			}

		if (pCmdLine->FindAttribute(RUN_SWITCH)
				|| pCmdLine->FindAttribute(RUN_FILE_SWITCH))
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

	bool bDefaultDataFile = false;
	CString sDataFile = pCmdLine->GetAttribute(CONSTLIT("dataFile"));
	if (sDataFile.IsBlank())
		{
		bDefaultDataFile = true;
		sDataFile = CONSTLIT("Transcendence");
		}

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

		printf("Cleared registered bit on %s.\n", (char *)sSaveFile);
		::kernelSetDebugLog(NULL);
		return;
		}

	//	Set to data file directory

#if 0
	if (!sDataFile.IsBlank())
		{
		CString sFolder = pathGetPath(sDataFile);
		if (!sFolder.IsBlank())
			::SetCurrentDirectory(sFolder.GetASCIIZPointer());
		}
#endif

	//	Initialize the Universe

	CHost Host;
	CUniverse Universe;
	CString sError;

	if (bLogo)
		printf("Loading...");

	if (error = InitUniverse(Universe, Host, (bDefaultDataFile ? NULL_STR : sDataFile), pCmdLine, &sError))
		{
		if (bLogo)
			printf("\n");

		printf("%s\n", sError.GetASCIIZPointer());
		::kernelSetDebugLog(NULL);
		return;
		}

	if (bLogo)
		printf("done.\n");

	//	Figure out what to do

	if (pCmdLine->GetAttributeBool(ARMOR_TABLE_SWITCH))
		GenerateArmorTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(ENCOUNTER_COUNT_SWITCH))
		GenerateEncounterCount(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(ENCOUNTER_FREQ_SWITCH))
		GenerateEncounterFrequency(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(ENCOUNTER_TABLE_SWITCH))
		GenerateEncounterTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(IMAGES_SWITCH))
		GenerateImageChart(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(ITEM_FREQUENCY_SWITCH))
		GenerateItemFrequencyTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(ITEM_TABLE_SWITCH))
		GenerateItemTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(LOOT_SIM_SWITCH))
		GenerateLootSim(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(RANDOM_ITEMS_SWITCH))
		GenerateRandomItemTables(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(RANDOM_NUMBER_TEST))
		DoRandomNumberTest();
	else if (pCmdLine->FindAttribute(RUN_SWITCH)
				|| pCmdLine->FindAttribute(RUN_FILE_SWITCH))
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
		GenerateShipTable(Universe, pCmdLine);
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
	else if (pCmdLine->GetAttributeBool(SYSTEM_COUNT_SWITCH))
		GenerateSystemCount(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SYSTEM_IMAGES_SWITCH))
		GenerateSystemImages(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SYSTEM_LABELS_SWITCH))
		GenerateSystemLabelCount(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SYSTEM_TEST_SWITCH))
		GenerateSystemTest(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(TOPOLOGY_SWITCH))
		GenerateTopology(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(TOPOLOGY_MAP_SWITCH))
		GenerateTopologyMap(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(ATTRIBUTE_LIST_SWITCH))
		GenerateAttributeList(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(PERF_TEST_SWITCH))
		PerformanceTest(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(ENCOUNTER_SIM_SWITCH))
		RunEncounterSim(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(TYPE_DEPENDENCIES_SWITCH))
		GenerateTypeDependencies(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(TYPE_TABLE_SWITCH))
		GenerateTypeTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(TYPE_ISLANDS_SWITCH))
		GenerateTypeIslands(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(HEXARC_TEST_SWITCH))
		HexarcTest(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(TRADE_SIM_SWITCH))
		DoTradeSim(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(WEAPON_IMAGES_SWITCH))
		GenerateWeaponEffectChart(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(WORLD_IMAGES_SWITCH))
		GenerateWorldImageChart(Universe, pCmdLine);
	else
		GenerateStats(Universe, pCmdLine);

	//	Done

	::kernelSetDebugLog(NULL);
	}

ALERROR CreateXMLElementFromDataFile (const CString &sFilespec, CXMLElement **retpDataFile, CString *retsError)
	{
	ALERROR error;
	int i;

	CFileReadBlock File(sFilespec);
	if (error = File.Open())
		{
		if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to open data file: %s."), sFilespec);
		return error;
		}

	CXMLElement *pFile;
	if (error = CXMLElement::ParseXML(&File, NULL, &pFile, retsError))
		return error;

	//	Create a new element that looks like a command-line

	CXMLElement *pDataFile = new CXMLElement(CONSTLIT("TransData"), NULL);

	//	Parse

	CString sExtensionList;
	for (i = 0; i < pFile->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pFile->GetContentElement(i);
		if (strEquals(pItem->GetTag(), CONSTLIT("Extension")))
			{
			DWORD dwUNID = pItem->GetAttributeInteger(CONSTLIT("unid"));
			if (dwUNID == 0)
				{
				printf("Warning: Invalid extension UNID: %x.\n", dwUNID);
				continue;
				}

			if (sExtensionList.IsBlank())
				sExtensionList = strPatternSubst(CONSTLIT("0x%x"), dwUNID);
			else
				sExtensionList = strPatternSubst(CONSTLIT("%s, 0x%x"), sExtensionList, dwUNID);
			}
		else if (strEquals(pItem->GetTag(), CONSTLIT("Option")))
			{
			CString sName = pItem->GetAttribute(CONSTLIT("name"));
			CString sValue = pItem->GetAttribute(CONSTLIT("value"));
			if (sValue.IsBlank())
				sValue = CONSTLIT("true");

			pDataFile->AddAttribute(sName, sValue);
			}
		else
			printf("Warning: Unknown directive: %s.\n", pItem->GetTag().GetASCIIZPointer());
		}

	//	Additional parameters

	if (!sExtensionList.IsBlank())
		pDataFile->AddAttribute(CONSTLIT("extensions"), sExtensionList);

	//	Done

	*retpDataFile = pDataFile;
	delete pFile;
	return NOERROR;
	}

ALERROR InitUniverse (CUniverse &Universe, CHost &Host, const CString &sFilespec, CXMLElement *pCmdLine, CString *retsError)
	{
	ALERROR error;

	CUniverse::SInitDesc Ctx;
	Ctx.sFilespec = sFilespec;
	Ctx.pHost = &Host;
	Ctx.bDebugMode = pCmdLine->GetAttributeBool(DEBUG_SWITCH);

	//	Figure out what adventure we need

	if (!pCmdLine->FindAttributeInteger(ADVENTURE_SWITCH, (int *)&Ctx.dwAdventure))
		Ctx.dwAdventure = DEFAULT_ADVENTURE_EXTENSION_UNID;

	//	See if we need to load images

	DWORD dwInitFlags = 0;
	if (pCmdLine->GetAttributeBool(EFFECT_IMAGE_SWITCH)
			|| pCmdLine->GetAttributeBool(IMAGES_SWITCH)
			|| pCmdLine->GetAttributeBool(SHIP_IMAGE_SWITCH) 
			|| pCmdLine->GetAttributeBool(SHIP_IMAGES_SWITCH)
			|| pCmdLine->GetAttributeBool(SMOKE_TEST_SWITCH)
			|| pCmdLine->GetAttributeBool(SNAPSHOT_SWITCH)
			|| pCmdLine->GetAttributeBool(TOPOLOGY_MAP_SWITCH)
			|| pCmdLine->GetAttributeBool(WEAPON_IMAGES_SWITCH) 
			|| pCmdLine->GetAttributeBool(WORLD_IMAGES_SWITCH))
		;
	else
		dwInitFlags |= flagNoResources;

	//	We don't need a version check

	dwInitFlags |= flagNoVersionCheck;

	//	We're not loading the game

	dwInitFlags |= flagNewGame;

	//	Extension

	CString sExtensionFolder = pCmdLine->GetAttribute(EXTENSION_FOLDER_ATTRIB);
	if (!sExtensionFolder.IsBlank())
		Ctx.ExtensionFolders.Insert(sExtensionFolder);

	CString sExtensionList;
	if (pCmdLine->FindAttribute(EXTENSIONS_ATTRIB, &sExtensionList))
		{
		if (strEquals(sExtensionList, CONSTLIT("none")))
			NULL;
		else if (strEquals(sExtensionList, CONSTLIT("all")))
			Ctx.bDefaultExtensions = true;
		else
			{
			TArray<DWORD> Extensions;
			ParseUNIDList(sExtensionList, PUL_FLAG_HEX, &Ctx.ExtensionUNIDs);
			}
		}
	else
		Ctx.bDefaultExtensions = true;

	//	Open the universe

	Ctx.bNoResources = ((dwInitFlags & flagNoResources) == flagNoResources);

	if (error = Universe.Init(Ctx, retsError))
		return error;

	if (error = Universe.InitGame(0, retsError))
		return error;

	//	Mark everything as known

	MarkItemsKnown(Universe);

	return NOERROR;
	}

bool IsMainCommandParam (const CString &sAttrib)
	{
	return (strEquals(sAttrib, CONSTLIT("adventure"))
			|| strEquals(sAttrib, CONSTLIT("all"))
			|| strEquals(sAttrib, CONSTLIT("allClasses"))
			|| strEquals(sAttrib, CONSTLIT("criteria"))
			|| strEquals(sAttrib, CONSTLIT("debug"))
			|| strEquals(sAttrib, CONSTLIT("extensionFolder"))
			|| strEquals(sAttrib, CONSTLIT("extensions"))
			|| strEquals(sAttrib, CONSTLIT("noLogo")));
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
	bool bNoLogo = pCmdLine->GetAttributeBool(NO_LOGO_SWITCH);

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

	CString sCommand = pCmdLine->GetAttribute(RUN_SWITCH);
	CString sRunFile = pCmdLine->GetAttribute(RUN_FILE_SWITCH);

	//	If this is a run file, then we parse it and run it

	if (!sRunFile.IsBlank() && !strEquals(sRunFile, CONSTLIT("true")))
		{
		CCodeChainCtx Ctx;

		//	Verify the file

		if (!strEndsWith(sRunFile, CONSTLIT("."))
				&& pathGetExtension(sRunFile).IsBlank())
			sRunFile.Append(CONSTLIT(".tlisp"));

		//	Open the file

		CFileReadBlock InputFile(sRunFile);
		if (error = InputFile.Open())
			{
			printf("error : Unable to open file '%s'.\n", sRunFile.GetASCIIZPointer());
			return;
			}

		if (!bNoLogo)
			printf("%s\n", sRunFile.GetASCIIZPointer());

		//	Parse

		CString sInputFile(InputFile.GetPointer(0), InputFile.GetLength(), TRUE);
		CString sOutput;
		int iOffset = 0;

		while (true)
			{
			int iCharCount;
			ICCItem *pCode = Ctx.Link(sInputFile, iOffset, &iCharCount);
			if (pCode->IsNil())
				break;
			else if (pCode->IsError())
				{
				printf("error : %s\n", pCode->GetStringValue().GetASCIIZPointer());
				Ctx.Discard(pCode);
				return;
				}

			iOffset += iCharCount;

			//	Execute

			ICCItem *pResult = Ctx.Run(pCode);

			//	Compose output

			if (pResult->IsIdentifier())
				sOutput = pResult->Print(&CC, PRFLAG_NO_QUOTES | PRFLAG_ENCODE_FOR_DISPLAY);
			else
				sOutput = CC.Unlink(pResult);

			//	Free

			Ctx.Discard(pResult);
			Ctx.Discard(pCode);
			}

		//	Output result

		printf("%s\n", sOutput.GetASCIIZPointer());
		}

	//	If we have a command, invoke it

	else if (!sCommand.IsBlank() && !strEquals(sCommand, CONSTLIT("True")))
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

		if (!bNoLogo)
			{
			printf("(help) for function help.\n");
			printf("\\q to quit.\n\n");
			}

		//	Loop

		while (true)
			{
			char szBuffer[1024];
			if (!bNoLogo)
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

