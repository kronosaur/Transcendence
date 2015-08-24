//	CTranscendenceController.cpp
//
//	CTranscendenceController class
//
//	stateLoading
//	----------------------------------------------------------------------------
//		OnInit initializes a background task to load the universe, and shows
//		the loading session.
//
//		modelInitDone:
//
//			When background load is done we get the modelInitDone command. When
//			we get this, we show the legacy session and set our state to 
//			stateIntro.
//
//	stateIntro
//	----------------------------------------------------------------------------
//		The intro is handled by the legacy session.
//
//		gameAdventure:
//
//			This command indicates that the player wants to start a new game and
//			has selected and adventure (the adventure UNID is the command data).
//
//			We switch state to stateNewGame.
//
//	stateNewGame
//	----------------------------------------------------------------------------
//
//		gameCreate:
//
//			pData is a pointer to an SNewGameSettings structure.
//
//		When ship selection is done, the legacy session starts the prologue
//		screen. The StartProlog code calls StartNewGame on the model and then
//		StartNewGameBackground on the controller (which calls the model).
//
//		When the background task is done, we get the modelNewGameCreated command.
//		This sets a flag inside the legacy session. The legacy session then
//		stops the prologue, initializes some stuff and sends the gameStart
//		command.
//
//		The gameStart command triggers the StartGame call on the model and we
//		set our state to stateInGame.

#include "PreComp.h"
#include "Zip.h"
#include "Transcendence.h"

#ifdef STEAM_BUILD
#include "SteamUtil.h"
#endif

#define SERVICES_TAG							CONSTLIT("Services")

#define CMD_NULL								CONSTLIT("null")

#define CMD_POST_CRASH_REPORT					CONSTLIT("cmdPostCrashReport")
#define CMD_SHOW_OK_BUTTON						CONSTLIT("cmdShowOKButton")
#define CMD_SHOW_WAIT_ANIMATION					CONSTLIT("cmdShowWaitAnimation")
#define CMD_SOUNDTRACK_DONE						CONSTLIT("cmdSoundtrackDone")
#define CMD_SOUNDTRACK_NEXT						CONSTLIT("cmdSoundtrackNext")
#define CMD_SOUNDTRACK_NOW_PLAYING				CONSTLIT("cmdSoundtrackNowPlaying")
#define CMD_SOUNDTRACK_PLAY_PAUSE				CONSTLIT("cmdSoundtrackPlayPause")
#define CMD_SOUNDTRACK_PREV						CONSTLIT("cmdSoundtrackPrev")
#define CMD_SOUNDTRACK_STOP						CONSTLIT("cmdSoundtrackStop")
#define CMD_SOUNDTRACK_UPDATE_PLAY_POS			CONSTLIT("cmdSoundtrackUpdatePlayPos")
#define CMD_UPDATE_HIGH_SCORE_LIST				CONSTLIT("cmdUpdateHighScoreList")

#define CMD_GAME_ADVENTURE						CONSTLIT("gameAdventure")
#define CMD_GAME_CREATE							CONSTLIT("gameCreate")
#define CMD_GAME_END_DESTROYED					CONSTLIT("gameEndDestroyed")
#define CMD_GAME_END_GAME						CONSTLIT("gameEndGame")
#define CMD_GAME_END_SAVE						CONSTLIT("gameEndSave")
#define CMD_GAME_ENTER_FINAL_STARGATE			CONSTLIT("gameEnterFinalStargate")
#define CMD_GAME_ENTER_STARGATE					CONSTLIT("gameEnterStargate")
#define CMD_GAME_INSIDE_STARGATE				CONSTLIT("gameInsideStargate")
#define CMD_GAME_LEAVE_STARGATE					CONSTLIT("gameLeaveStargate")
#define CMD_GAME_LOAD							CONSTLIT("gameLoad")
#define CMD_GAME_LOAD_DONE						CONSTLIT("gameLoadDone")
#define CMD_GAME_PAUSE							CONSTLIT("gamePause")
#define CMD_GAME_READY							CONSTLIT("gameReady")
#define CMD_GAME_SELECT_ADVENTURE				CONSTLIT("gameSelectAdventure")
#define CMD_GAME_SELECT_SAVE_FILE				CONSTLIT("gameSelectSaveFile")
#define CMD_GAME_STARGATE_SYSTEM_READY			CONSTLIT("gameStargateSystemReady")
#define CMD_GAME_UNPAUSE						CONSTLIT("gameUnpause")

#define CMD_MODEL_ADVENTURE_INIT_DONE			CONSTLIT("modelAdventureInitDone")
#define CMD_MODEL_INIT_DONE						CONSTLIT("modelInitDone")
#define CMD_MODEL_NEW_GAME_CREATED				CONSTLIT("modelNewGameCreated")

#define CMD_PLAYER_COMBAT_ENDED					CONSTLIT("playerCombatEnded")
#define CMD_PLAYER_COMBAT_MISSION_STARTED		CONSTLIT("playerCombatMisionStarted")
#define CMD_PLAYER_COMBAT_STARTED				CONSTLIT("playerCombatStarted")
#define CMD_PLAYER_UNDOCKED						CONSTLIT("playerUndocked")

#define CMD_SERVICE_ACCOUNT_CHANGED				CONSTLIT("serviceAccountChanged")
#define CMD_SERVICE_COLLECTION_LOADED			CONSTLIT("serviceCollectionLoaded")
#define CMD_SERVICE_DOWNLOAD_RESOURCES			CONSTLIT("serviceDownloadResources")
#define CMD_SERVICE_DOWNLOADS_COMPLETE			CONSTLIT("serviceDownloadsComplete")
#define CMD_SERVICE_DOWNLOADS_IN_PROGRESS		CONSTLIT("serviceDownloadsInProgress")
#define CMD_SERVICE_ERROR						CONSTLIT("serviceError")
#define CMD_SERVICE_EXTENSION_LOADED			CONSTLIT("serviceExtensionLoaded")
#define CMD_SERVICE_FILE_DOWNLOADED				CONSTLIT("serviceFileDownloaded")
#define CMD_SERVICE_HIGH_SCORE_LIST_ERROR		CONSTLIT("serviceHighScoreListError")
#define CMD_SERVICE_HIGH_SCORE_LIST_LOADED		CONSTLIT("serviceHighScoreListLoaded")
#define CMD_SERVICE_HOUSEKEEPING				CONSTLIT("serviceHousekeeping")
#define CMD_SERVICE_NEWS_LOADED					CONSTLIT("serviceNewsLoaded")
#define CMD_SERVICE_STATUS						CONSTLIT("serviceStatus")
#define CMD_SERVICE_UPGRADE_READY				CONSTLIT("serviceUpgradeReady")

#define CMD_SESSION_STATS_DONE					CONSTLIT("sessionStatsDone")
#define CMD_SESSION_EPILOGUE_DONE				CONSTLIT("sessionEpilogueDone")
#define CMD_SESSION_PROLOGUE_DONE				CONSTLIT("sessionPrologueDone")

#define CMD_UI_BACK_TO_INTRO					CONSTLIT("uiBackToIntro")
#define CMD_UI_CHANGE_PASSWORD					CONSTLIT("uiChangePassword")
#define CMD_UI_EXIT								CONSTLIT("uiExit")
#define CMD_UI_MUSIC_VOLUME_DOWN				CONSTLIT("uiMusicVolumeDown")
#define CMD_UI_MUSIC_VOLUME_UP					CONSTLIT("uiMusicVolumeUp")
#define CMD_UI_RESET_PASSWORD					CONSTLIT("uiResetPassword")
#define CMD_UI_SHOW_GAME_STATS					CONSTLIT("uiShowGameStats")
#define CMD_UI_SHOW_GALACTIC_MAP				CONSTLIT("uiShowGalacticMap")
#define CMD_UI_SHOW_HELP						CONSTLIT("uiShowHelp")
#define CMD_UI_SHOW_LOGIN						CONSTLIT("uiShowLogin")
#define CMD_UI_SHOW_MOD_EXCHANGE				CONSTLIT("uiShowModExchange")
#define CMD_UI_SHOW_PROFILE						CONSTLIT("uiShowProfile")
#define CMD_UI_SIGN_OUT							CONSTLIT("uiSignOut")
#define CMD_UI_START_EPILOGUE					CONSTLIT("uiStartEpilogue")
#define CMD_UI_START_GAME						CONSTLIT("uiStartGame")
#define CMD_UI_VOLUME_DOWN						CONSTLIT("uiVolumeDown")
#define CMD_UI_VOLUME_UP						CONSTLIT("uiVolumeUp")

#define FILESPEC_DOWNLOADS_FOLDER				CONSTLIT("Cache")
#define FILESPEC_UPGRADE_FILE					CONSTLIT("Cache\\Upgrade.zip")

#define ID_MULTIVERSE_STATUS_SEQ				CONSTLIT("idMultiverseStatusSeq")
#define ID_MULTIVERSE_STATUS_TEXT				CONSTLIT("idMultiverseStatusText")

#define DEBUG_LOG_FILENAME						CONSTLIT("Debug.log")
#define SETTINGS_FILENAME						CONSTLIT("Settings.xml")
#define FOLDER_COLLECTION						CONSTLIT("Collection")
#define FOLDER_EXTENSIONS						CONSTLIT("Extensions")

#define PROP_COLOR								CONSTLIT("color")
#define PROP_FONT								CONSTLIT("font")
#define PROP_OPACITY							CONSTLIT("opacity")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_TEXT								CONSTLIT("text")

#define ERR_CANT_LOAD_GAME						CONSTLIT("Unable to load game")
#define ERR_CANT_START_GAME						CONSTLIT("Unable to start game")
#define ERR_CANT_SHOW_ADVENTURES				CONSTLIT("Unable to show list of adventures")
#define ERR_CANT_SHOW_MOD_EXCHANGE				CONSTLIT("Unable to show Mod Collection screen")
#define ERR_CONTACT_KP							CONSTLIT("Contact Kronosaur Productions")
#define ERR_LOAD_ERROR							CONSTLIT("Error loading extensions")
#define ERR_RESET_PASSWORD_DESC					CONSTLIT("Automated password reset is not yet implemented. Please contact Kronosaur Productions at:\n\ntranscendence@kronosaur.com\n\nPlease provide your username.")

const DWORD SERVICE_HOUSEKEEPING_INTERVAL =		1000 * 60;

CTranscendenceWnd *g_pTrans = NULL;

void CTranscendenceController::CleanUpUpgrade (void)

//	CleanUpUpgrade
//
//	Clean up files from an upgrade.

	{
	int i;

	//	Delete any previous upgrade files

	TArray<CString> FilesToDelete;
	if (!fileGetFileList(m_Settings.GetAppDataFolder(), NULL_STR, CONSTLIT("Delete_*.*"), 0, &FilesToDelete))
		::kernelDebugLogMessage("Unable to list files to delete from previous upgrade.");

	for (i = 0; i < FilesToDelete.GetCount(); i++)
		if (!fileDelete(FilesToDelete[i]))
			::kernelDebugLogMessage("Unable to delete file: %s.", FilesToDelete[i]);
	}

bool CTranscendenceController::CheckAndRunUpgrade (void)

//	CheckAndRunUpgrade
//
//	If we upgrades to the AppData directory, then we should run that file. 

	{
	if (m_Settings.GetAppDataFolder().IsBlank())
		return false;

	CString sExe = pathAddComponent(m_Settings.GetAppDataFolder(), CONSTLIT("Transcendence.exe"));
	if (!pathExists(sExe))
		return false;

	//	Close the log file, otherwise we'll have trouble launching

	::kernelSetDebugLog(NULL);

	//	Run
	//
	//	LATER: Since we're running in a different directory, we should add this
	//	Extension directory as a command-line parameter so that the EXE in AppData
	//	knowns about our Extensions.

	if (!fileOpen(sExe))
		{
		::kernelDebugLogMessage("Unable to run upgraded Transcendence.exe");
		return false;
		}

	//	TRUE means we exit this process.

	return true;
	}

void CTranscendenceController::DisplayMultiverseStatus (const CString &sStatus, bool bError)

//	DisplayMultiverseStatus
//
//	Displays a line of status on the current session (if appropriate). If 
//	sStatus is NULL then we clear the status.

	{
	//	We don't display status in all states

	switch (m_iState)
		{
		case stateIntro:
		case stateEndGameStats:
			break;

		default:
			return;
		}

	//	Get a hold of the Reanimator object

	IHISession *pSession = m_HI.GetTopSession();
	if (pSession == NULL)
		return;

	CReanimator &Reanimator = pSession->HIGetReanimator();

	//	Prepare some visuals

	const CVisualPalette &VI = g_pHI->GetVisuals();
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);

	//	Get the appropriate performance

	IAnimatron *pAni = Reanimator.GetPerformance(ID_MULTIVERSE_STATUS_SEQ);
	if (pAni == NULL)
		{
		//	Start by creating a sequencer to use as a container

		const int TOP_SPACING = 8;

		RECT rcRect;
		VI.GetWidescreenRect(m_HI.GetScreen(), &rcRect);

		CAniSequencer *pRoot;
		CAniSequencer::Create(CVector(rcRect.left, rcRect.bottom - (MediumFont.GetHeight() + TOP_SPACING)), &pRoot);

		Reanimator.AddPerformance(pRoot, ID_MULTIVERSE_STATUS_SEQ);
		Reanimator.StartPerformance(ID_MULTIVERSE_STATUS_SEQ);

		pAni = pRoot;

		//	Now create a text element

		IAnimatron *pStatus = new CAniText;
		pStatus->SetID(ID_MULTIVERSE_STATUS_TEXT);
		pStatus->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), RectHeight(rcRect)));
		pStatus->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogLabel));
		pStatus->SetPropertyFont(PROP_FONT, &MediumFont);

		pRoot->AddTrack(pStatus, 0);
		}

	//	Find the appropriate control

	IAnimatron *pStatus;
	if (!pAni->FindElement(ID_MULTIVERSE_STATUS_TEXT, &pStatus))
		return;

	//	Set the text element

	pStatus->SetPropertyString(PROP_TEXT, sStatus);
	}

bool CTranscendenceController::InstallUpgrade (CString *retsError)

//	InstallUpgrade
//
//	Installs an upgrade

	{
	int i;

	//	Upgrade file?

	CString sUpgradeFile = pathAddComponent(m_Settings.GetAppDataFolder(), FILESPEC_UPGRADE_FILE);
	if (!pathExists(sUpgradeFile))
		{
		if (retsError) *retsError = CONSTLIT("No upgrade file found.");
		return false;
		}

	//	Make a list of all files in the archive

	TArray<CString> FileList;
	CString sError;
	if (!arcList(sUpgradeFile, &FileList, retsError))
		{
		fileDelete(sUpgradeFile);
		::kernelDebugLogMessage(sError.GetASCIIZPointer());
		return false;
		}

	//	Figure out where to write out the new files.

	CString sDestFolder = m_Settings.GetAppDataFolder();

	//	Rename the files that we're going to replace, keeping track of what
	//	we've renamed so we can undo it.

	bool bAbort = false;
	TArray<CString> FilesToReplace;
	TArray<CString> RenamedFiles;
	for (i = 0; i < FileList.GetCount(); i++)
		{
		CString sFileToReplace = pathAddComponent(sDestFolder, FileList[i]);

		CString sRenameTo;
		if (pathExists(sFileToReplace))
			{
			sRenameTo = pathAddComponent(sDestFolder, strPatternSubst(CONSTLIT("Delete_%s"), FileList[i]));
			if (!fileMove(sFileToReplace, sRenameTo))
				{
				::kernelDebugLogMessage("Unable to rename %s to %s.", sFileToReplace, sRenameTo);
				bAbort = true;
				break;
				}
			}

		FilesToReplace.Insert(sFileToReplace);
		RenamedFiles.Insert(sRenameTo);
		}

	//	If we failed, move everything back

	if (bAbort)
		{
		for (i = 0; i < RenamedFiles.GetCount(); i++)
			fileMove(RenamedFiles[i], FilesToReplace[i]);
		if (retsError) *retsError = CONSTLIT("Unable to replace existing files.");
		return false;
		}

	//	Now extract all the files to the proper place

	for (i = 0; i < FileList.GetCount(); i++)
		{
		if (!arcDecompressFile(sUpgradeFile, FileList[i], FilesToReplace[i], &sError))
			{
			::kernelDebugLogMessage(sError.GetASCIIZPointer());
			bAbort = true;
			break;
			}
		}

	//	If we failed, then we need to delete the files and move everything back.

	if (bAbort)
		{
		for (i = 0; i < FileList.GetCount(); i++)
			fileDelete(FilesToReplace[i]);

		for (i = 0; i < RenamedFiles.GetCount(); i++)
			fileMove(RenamedFiles[i], FilesToReplace[i]);

		//	Delete the archive because if we failed to decompress then the archive
		//	is probably bad.

		fileDelete(sUpgradeFile);
		if (retsError) *retsError = CONSTLIT("Unable to extract files from archive.");
		return false;
		}

	//	Delete the original zip archive because we don't need it anymore.

	if (!fileDelete(sUpgradeFile))
		::kernelDebugLogMessage("Unable to delete upgrade file: %s.", sUpgradeFile);

	return true;
	}

bool CTranscendenceController::IsUpgradeReady (void)

//	IsUpgradeReady
//
//	Returns TRUE if we have an upgrade file in the DOwnloads directory

	{
	return pathExists(pathAddComponent(m_Settings.GetAppDataFolder(), FILESPEC_UPGRADE_FILE));
	}

ALERROR CTranscendenceController::OnBoot (char *pszCommandLine, SHIOptions *retOptions, CString *retsError)

//	OnBoot
//
//	Boot up controller and return options for HI

	{
	ALERROR error;

	//	Set our basic application settings

	retOptions->sAppName = CONSTLIT("Transcendence");
	retOptions->sClassName = CONSTLIT("transcendence_class");
	retOptions->sAppData = CONSTLIT("Kronosaur\\Transcendence");
	retOptions->hIcon = ::LoadIcon(NULL, "AppIcon");

	//	Set our default directory

	CString sCurDir = pathGetExecutablePath(NULL);
	::SetCurrentDirectory(sCurDir.GetASCIIZPointer());

	//	Add the services that we want (we need to do this before we load settings
	//	because we will get called back to initialize services).

#ifdef STEAM_BUILD
	m_Service.AddService(new CSteamService(m_HI));
#else
	CHexarcServiceFactory HexarcService;
	m_Service.AddService(HexarcService.Create(m_HI));

	CXelerusServiceFactory XelerusService;
	m_Service.AddService(XelerusService.Create(m_HI));
#endif

	//	Load the settings from a file

	CString sError;
	bool bLogFileOpened = false;
	m_Settings.SetSettingsHandler(this);
	if (error = m_Settings.Load(SETTINGS_FILENAME, &sError))
		{
		//	We can't open the log until see if we want a log file, but if we get an error
		//	loading settings then we have to open the log file.

		kernelSetDebugLog(DEBUG_LOG_FILENAME);
		bLogFileOpened = true;

		//	Report to Debug.log but otherwise continue

		kernelDebugLogMessage("Error loading %s: %s", SETTINGS_FILENAME, sError);
		}

	//	Allow the command line to override some options

	if (error = m_Settings.ParseCommandLine(pszCommandLine))
		{
		if (retsError) *retsError = CONSTLIT("Unable to parse command line.");
		return error;
		}

	//	Set the debug log

	if (!bLogFileOpened && !m_Settings.GetBoolean(CGameSettings::noDebugLog))
		if (error = kernelSetDebugLog(pathAddComponent(m_Settings.GetAppDataFolder(), DEBUG_LOG_FILENAME)))
			{
			if (retsError) *retsError = CONSTLIT("Unable to set debug log file.");
			return error;
			}

	//	If we're running a new copy, do it now

	if (CheckAndRunUpgrade())
		return ERR_CANCEL;

	//	Clean up upgrade files

	CleanUpUpgrade();

	//	If we're windowed, figure out the size that we want.

	retOptions->m_bWindowedMode = m_Settings.GetBoolean(CGameSettings::windowedMode);
	if (retOptions->m_bWindowedMode)
		{
		//	If we're forcing 1024, then we know the size

		if (m_Settings.GetBoolean(CGameSettings::force1024Res))
			{
			retOptions->m_cxScreenDesired = 1024;
			retOptions->m_cyScreenDesired = 768;
			}
		else if (m_Settings.GetBoolean(CGameSettings::force600Res))
			{
			retOptions->m_cxScreenDesired = 1024;
			retOptions->m_cyScreenDesired = 600;
			}

		//	Otherwise compute the window size based on the screen

		else
			{
			//	Get the size of the desktop on the primary monitor

			RECT rcWorkArea;
			::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);
			int cyAvail = RectHeight(rcWorkArea);

			//	Figure out the extra space that we need to title bar, etc.

			RECT rcRect;
			rcRect.left = 0;
			rcRect.top = 0;
			rcRect.right = 100;
			rcRect.bottom = 100;
			::AdjustWindowRect(&rcRect, WS_OVERLAPPEDWINDOW, FALSE);

			int cyExtra = RectHeight(rcRect) - 100;

			//	Now compute the desired size

			if (cyAvail - cyExtra >= 1024)
				{
				retOptions->m_cxScreenDesired = 1280;
				retOptions->m_cyScreenDesired = 1024;
				}
			else
				{
				retOptions->m_cyScreenDesired = Max(768, cyAvail - cyExtra);
				retOptions->m_cxScreenDesired = retOptions->m_cyScreenDesired * 1024 / 768;
				}
			}
		}

	//	Now set other options for HI

	retOptions->m_iColorDepthDesired = 16;
	retOptions->m_bForceDX = (m_Settings.GetBoolean(CGameSettings::forceDirectX) || m_Settings.GetBoolean(CGameSettings::forceExclusive) || m_Settings.GetBoolean(CGameSettings::force1024Res));
	retOptions->m_bForceNonDX = (m_Settings.GetBoolean(CGameSettings::forceNonDirectX) && !retOptions->m_bForceDX);
	retOptions->m_bForceExclusiveMode = (m_Settings.GetBoolean(CGameSettings::forceExclusive) || m_Settings.GetBoolean(CGameSettings::force1024Res));
	retOptions->m_bForceNonExclusiveMode = (m_Settings.GetBoolean(CGameSettings::forceNonExclusive) && !retOptions->m_bForceExclusiveMode);
	retOptions->m_bForceScreenSize = m_Settings.GetBoolean(CGameSettings::force1024Res);
	retOptions->m_bNoGPUAcceleration = m_Settings.GetBoolean(CGameSettings::noGPUAcceleration);
	retOptions->m_iSoundVolume = m_Settings.GetInteger(CGameSettings::soundVolume);
	retOptions->m_sMusicFolder = m_Settings.GetString(CGameSettings::musicPath);
	retOptions->m_bDebugVideo = m_Settings.GetBoolean(CGameSettings::debugVideo);
	retOptions->m_cyMaxScreen = (m_Settings.GetInteger(CGameSettings::maxResolution) <= 0 ? -1 : Max(600, m_Settings.GetInteger(CGameSettings::maxResolution)));

	return NOERROR;
	}

void CTranscendenceController::OnCleanUp (void)

//	OnCleanUp
//
//	Clean up

	{
	m_Soundtrack.SetGameState(CSoundtrackManager::stateProgramQuit);

	//	Clean up

	m_Model.CleanUp();
	kernelClearDebugLog();

	//	Clean up legacy window

	if (g_pTrans)
		{
		g_pTrans->WMDestroy();
		delete g_pTrans;
		g_pTrans = NULL;
		}
	}

bool CTranscendenceController::OnClose (void)

//	OnClose
//
//	User wants to close the app.

	{
	//	Do not allow the user to close if we're entering or leaving
	//	a stargate (because we are too lazy to handle saving a game in the
	//	middle of a gate).

	if (g_pTrans->m_State == CTranscendenceWnd::gsEnteringStargate || g_pTrans->m_State == CTranscendenceWnd::gsLeavingStargate)
		return false;

	//	If we have a loading error, display it now

	if (!g_pTrans->m_sBackgroundError.IsBlank())
		{
		g_pHI->GetScreenMgr().StopDX();

		::MessageBox(m_HI.GetHWND(), 
				g_pTrans->m_sBackgroundError.GetASCIIZPointer(), 
				"Transcendence", 
				MB_OK | MB_ICONSTOP);
		}

	//	OK to close

	return true;
	}

ALERROR CTranscendenceController::OnCommand (const CString &sCmd, void *pData)

//	OnCommand
//
//	Handle commands from sessions, etc.

	{
	ALERROR error;
	CString sError;

	//	Background initialization of model is done.
	//	Go to intro screen.

	if (strEquals(sCmd, CMD_MODEL_INIT_DONE))
		{
		CInitModelTask *pTask = (CInitModelTask *)pData;

		//	Check for error

		if (pTask->GetResult(&sError))
			{
			kernelDebugLogMessage(sError);
			m_HI.OpenPopupSession(new CMessageSession(m_HI, ERR_LOAD_ERROR, sError, CMD_UI_EXIT));
			return NOERROR;
			}

		//	If we need to open a save file, then put up a waiting screen.

		const CString &sSaveFile = m_Settings.GetInitialSaveFile();
		if (!sSaveFile.IsBlank())
			{
			//	Kick-off background thread to load the game

			m_HI.AddBackgroundTask(new CLoadGameWithSignInTask(m_HI, m_Service, m_Model, sSaveFile), 0, this, CMD_GAME_LOAD_DONE);

			//	Show transition session while we load

			m_HI.ShowSession(new CWaitSession(m_HI, m_Service, CONSTLIT("Loading Game")));
			return NOERROR;
			}

		//	If we can sign in automatically, do so now.

		if (m_Service.HasCapability(ICIService::autoLoginUser))
			m_HI.AddBackgroundTask(new CSignInUserTask(m_HI, m_Service, NULL_STR, NULL_STR, true), 0);

		//	If we can, download news

		else if (m_Service.HasCapability(ICIService::canLoadNews)
				&& m_Multiverse.IsLoadNewsNeeded())
			{
			//	Figure out the path for the news image cache

			CString sDownloadsFolder = pathAddComponent(m_Settings.GetAppDataFolder(), FILESPEC_DOWNLOADS_FOLDER);

			//	Make sure it exists

			if (!pathExists(sDownloadsFolder))
				pathCreate(sDownloadsFolder);

			//	Start a task to load the news (we pass in Multiverse so
			//	that the collection is placed there).

			m_HI.AddBackgroundTask(new CLoadNewsTask(m_HI, m_Service, m_Multiverse, m_Model.GetProgramVersion(), sDownloadsFolder), 0, this, CMD_SERVICE_NEWS_LOADED);
			}

		//	Legacy CTranscendenceWnd takes over

		m_HI.ShowSession(new CIntroSession(m_HI, m_Model, CIntroSession::isOpeningTitles));

		//	Start the intro

		m_iState = stateIntro;
		DisplayMultiverseStatus(m_Multiverse.GetServiceStatus());
		m_Soundtrack.SetGameState(CSoundtrackManager::stateProgramIntro);
		}

	//	Bring up screen to choose an adventure

	else if (strEquals(sCmd, CMD_GAME_SELECT_ADVENTURE))
		{
		DisplayMultiverseStatus(NULL_STR);
		if (error = m_HI.OpenPopupSession(new CChooseAdventureSession(m_HI, m_Service, m_Settings)))
			{
			m_HI.OpenPopupSession(new CMessageSession(m_HI, ERR_CANT_SHOW_ADVENTURES, NULL_STR, CMD_NULL));
			return NOERROR;
			}
		}

	//	New game started; adventure selected

	else if (strEquals(sCmd, CMD_GAME_ADVENTURE))
		{
		SAdventureSettings *pNewAdventure = (SAdventureSettings *)pData;
		if (pNewAdventure == NULL || pNewAdventure->pAdventure == NULL)
			{
			m_HI.OpenPopupSession(new CMessageSession(m_HI, ERR_CANT_START_GAME, CONSTLIT("Unknown adventure"), CMD_UI_BACK_TO_INTRO));
			return NOERROR;
			}

		//	Done with intro

		ASSERT(m_iState == stateIntro);
		g_pTrans->StopIntro();

		//	Remember the adventure name

		CString sAdventureName = pNewAdventure->pAdventure->GetName();

		//	Kick-off background thread to initialize the adventure

		m_HI.AddBackgroundTask(new CInitAdventureTask(m_HI, m_Model, *pNewAdventure), 0, this, CMD_MODEL_ADVENTURE_INIT_DONE);

		//	Show transition session while we load

		m_HI.ShowSession(new CAdventureIntroSession(m_HI, m_Service, sAdventureName));
		}

	//	Adventure init done

	else if (strEquals(sCmd, CMD_MODEL_ADVENTURE_INIT_DONE))
		{
		CInitAdventureTask *pTask = (CInitAdventureTask *)pData;

		//	Check for error

		if (pTask->GetResult(&sError))
			{
			kernelDebugLogMessage(sError);
			m_HI.OpenPopupSession(new CMessageSession(m_HI, ERR_CANT_START_GAME, sError, CMD_UI_BACK_TO_INTRO));
			return NOERROR;
			}

		//	Select the ship

		SNewGameSettings Defaults;
		Defaults.sPlayerName = m_Settings.GetString(CGameSettings::playerName);
		Defaults.iPlayerGenome = ParseGenomeID(m_Settings.GetString(CGameSettings::playerGenome));
		Defaults.dwPlayerShip = (DWORD)m_Settings.GetInteger(CGameSettings::playerShipClass);

		//	If the player name is NULL then we come up with a better idea

		if (Defaults.sPlayerName.IsBlank())
			{
			Defaults.sPlayerName = m_Service.GetDefaultUsername();
			Defaults.bDefaultPlayerName = true;
			}

		if (Defaults.sPlayerName.IsBlank())
			{
			Defaults.sPlayerName = ::sysGetUserName();
			Defaults.bDefaultPlayerName = true;
			}

		Defaults.sPlayerName = CUniverse::ValidatePlayerName(Defaults.sPlayerName);

		//	Validate genome

		if (Defaults.iPlayerGenome == genomeUnknown)
			Defaults.iPlayerGenome = (mathRandom(1, 2) == 1 ? genomeHumanFemale : genomeHumanMale);

		//	New game screen

		if (error = m_HI.ShowSession(new CNewGameSession(m_HI, m_Service, Defaults), &sError))
			{
			kernelDebugLogMessage(sError);
			m_HI.OpenPopupSession(new CMessageSession(m_HI, ERR_CANT_START_GAME, sError, CMD_UI_BACK_TO_INTRO));
			return NOERROR;
			}

		m_iState = stateNewGame;
		}

	//	Back to intro if we get an error previously

	else if (strEquals(sCmd, CMD_UI_BACK_TO_INTRO))
		{
		m_HI.ShowSession(new CIntroSession(m_HI, m_Model, CIntroSession::isBlank));
		m_iState = stateIntro;
		DisplayMultiverseStatus(m_Multiverse.GetServiceStatus());
		m_Soundtrack.SetGameState(CSoundtrackManager::stateProgramIntro);
		}

	//	New game

	else if (strEquals(sCmd, CMD_GAME_CREATE))
		{
		SNewGameSettings *pNewGame = (SNewGameSettings *)pData;
		pNewGame->bFullCreate = !g_pUniverse->InDebugMode() || !m_Settings.GetBoolean(CGameSettings::noFullCreate);

		//	Let the model begin the creation

		if (error = m_Model.StartNewGame(m_Service.GetUsername(), *pNewGame, &sError))
			{
			sError = strPatternSubst(CONSTLIT("Unable to begin new game: %s"), sError);
			kernelDebugLogMessage(sError);

			m_HI.OpenPopupSession(new CMessageSession(m_HI, ERR_CANT_START_GAME, sError, CMD_UI_BACK_TO_INTRO));
			return NOERROR;
			}

		//	Remember the player settings
		//
		//	NOTE: We don't save the player name if it is a default name. This 
		//	allows us to pick a different name if we two different people sign in.
		//	Also, it prevents from us from posting the computer's username to
		//	the Multiverse (which some people see as a privacy issue).

		if (!pNewGame->bDefaultPlayerName)
			m_Settings.SetString(CGameSettings::playerName, pNewGame->sPlayerName);

		m_Settings.SetString(CGameSettings::playerGenome, GetGenomeID(pNewGame->iPlayerGenome));
		m_Settings.SetInteger(CGameSettings::playerShipClass, (int)pNewGame->dwPlayerShip);
		m_Settings.SetInteger(CGameSettings::lastAdventure, (int)g_pUniverse->GetCurrentAdventureDesc()->GetExtensionUNID());

		//	Report creation

		kernelDebugLogMessage("Created new game");

		//	Kick-off background thread to finish up creating the game

		g_pTrans->SetGameCreated(false);
		m_HI.AddBackgroundTask(new CStartNewGameTask(m_HI, m_Model, *pNewGame), 0, this, CMD_MODEL_NEW_GAME_CREATED);

		//	Start the prologue

		CG32bitImage *pCrawlImage = m_Model.GetCrawlImage();
		const CString &sCrawlText = m_Model.GetCrawlText();

		m_HI.ShowSession(new CTextCrawlSession(m_HI, m_Service, pCrawlImage, sCrawlText, CMD_SESSION_PROLOGUE_DONE));
		m_iState = statePrologue;
		m_Soundtrack.SetGameState(CSoundtrackManager::stateGamePrologue, m_Model.GetCrawlSoundtrack());
		}

	//	Player has stopped the prologue

	else if (strEquals(sCmd, CMD_SESSION_PROLOGUE_DONE))
		{
		//	If we're already done, then ignore this message

		if (m_iState == statePrologueDone)
			NULL;

		//	If we're done create the new game then we can continue

		else if (g_pTrans->IsGameCreated())
			HICommand(CMD_UI_START_GAME);

		//	Otherwise start wait animation

		else
			m_HI.HISessionCommand(CMD_SHOW_WAIT_ANIMATION);

		//	We set state so we don't repeat any of the actions above. [This 
		//	could happen if the client sends us this message twice, e.g.,
		//	if the player clicks the button multiple times.]

		m_iState = statePrologueDone;
		}

	//	Background creation of game is done

	else if (strEquals(sCmd, CMD_MODEL_NEW_GAME_CREATED))
		{
		CStartNewGameTask *pTask = (CStartNewGameTask *)pData;

		//	Check for error

		if (pTask->GetResult(&sError))
			{
			m_Model.StartNewGameAbort();
			kernelDebugLogMessage(sError);
			m_HI.OpenPopupSession(new CMessageSession(m_HI, ERR_CANT_START_GAME, sError, CMD_UI_BACK_TO_INTRO));
			return NOERROR;
			}

		g_pTrans->SetGameCreated();

		//	If the prologue is done, then we can start the game (otherwise we
		//	wait for the player to dismiss the prologue).

		if (m_iState == statePrologueDone)
			HICommand(CMD_UI_START_GAME);
		}

	//	Start the game

	else if (strEquals(sCmd, CMD_UI_START_GAME))
		{
		g_pTrans->StartGame();
		m_HI.AddBackgroundTask(new CStartGameTask(m_HI, m_Model, true), 0, this, CMD_GAME_READY);
		}

	//	Choose a save file to load

	else if (strEquals(sCmd, CMD_GAME_SELECT_SAVE_FILE))
		{
		DisplayMultiverseStatus(NULL_STR);
		if (error = m_HI.OpenPopupSession(new CLoadGameSession(m_HI, m_Service, m_Model.GetSaveFileFolders())))
			{
			m_HI.OpenPopupSession(new CMessageSession(m_HI, ERR_CANT_LOAD_GAME, NULL_STR, CMD_NULL));
			return NOERROR;
			}
		}

	//	Load an old game

	else if (strEquals(sCmd, CMD_GAME_LOAD))
		{
		CString *pFilespec = (CString *)pData;

		//	Done with intro

		ASSERT(m_iState == stateIntro);
		g_pTrans->StopIntro();

		//	Kick-off background thread to load the game

		m_HI.AddBackgroundTask(new CLoadGameTask(m_HI, m_Model, m_Service.GetUsername(), *pFilespec), 0, this, CMD_GAME_LOAD_DONE);

		//	Show transition session while we load

		m_HI.ShowSession(new CWaitSession(m_HI, m_Service, CONSTLIT("Loading Game")));
		}

		//	Background load of game is done

	else if (strEquals(sCmd, CMD_GAME_LOAD_DONE))
		{
		CLoadGameTask *pTask = (CLoadGameTask *)pData;

		//	Check for error

		if (error = pTask->GetResult(&sError))
			{
			//	On ERR_CANCEL it means that the game has ended and cannot be 
			//	continued, but that we loaded game stats.

			if (error == ERR_CANCEL)
				{
				CGameStats Stats;

				//	Get the stats

				if ((error = m_Model.GetGameStats(&Stats))
						|| Stats.GetCount() == 0)
					//	If we get an error (or stats come back blank, ignore)
					{
					m_HI.HICommand(CMD_UI_BACK_TO_INTRO);
					return NOERROR;
					}

				//	The session takes handoff of the stats

				m_HI.ShowSession(new CStatsSession(m_HI, m_Service, Stats));
				m_iState = stateEndGameStats;
				return NOERROR;
				}

			//	Otherwise, this is a real error

			else
				{
				kernelDebugLogMessage(sError);
				m_HI.OpenPopupSession(new CMessageSession(m_HI, ERR_CANT_LOAD_GAME, sError, CMD_UI_BACK_TO_INTRO));
				return NOERROR;
				}
			}

		//	Start game (this does some stuff and then calls cmdGameStart)

		g_pTrans->StartGame();
		m_HI.AddBackgroundTask(new CStartGameTask(m_HI, m_Model, false), 0, this, CMD_GAME_READY);
		}

	//	The game has ended with the player still alive. We assume that we've
	//	already called m_Model.EndGame with the proper epitaph, etc.

	else if (strEquals(sCmd, CMD_GAME_END_GAME))
		{
		m_Model.EndGame();

		//	Clean up game state

		g_pTrans->CleanUpDisplays();

		//	Epilogue

		HICommand(CMD_UI_START_EPILOGUE);
		}

	//	End destroyed state. We either go to the epilog or we
	//	resurrect back to the game.

	else if (strEquals(sCmd, CMD_GAME_END_DESTROYED))
		{
		bool bResurrected;
		m_Model.EndGameDestroyed(&bResurrected);

		if (bResurrected)
			g_pTrans->m_State = CTranscendenceWnd::gsInGame;
		else
			{
			//	Clean up game state

			g_pTrans->CleanUpDisplays();

			//	Epilogue

			HICommand(CMD_UI_START_EPILOGUE);
			}
		}

	//	Save and exit

	else if (strEquals(sCmd, CMD_GAME_END_SAVE))
		{
		if (error = m_Model.EndGameSave(&sError))
			g_pTrans->DisplayMessage(sError);

		//	Clean up game state

		g_pTrans->CleanUpDisplays();

		//	Back to intro screen

		m_HI.ShowSession(new CIntroSession(m_HI, m_Model, CIntroSession::isShipStats));
		m_iState = stateIntro;
		DisplayMultiverseStatus(m_Multiverse.GetServiceStatus());
		m_Soundtrack.SetGameState(CSoundtrackManager::stateProgramIntro);
		}

	//	Start the game

	else if (strEquals(sCmd, CMD_GAME_READY))
		{
		m_HI.ShowSession(new CGameSession(m_HI, m_Settings, m_Soundtrack));
		m_iState = stateInGame;

		//	NOTE: It's OK to leave the default param (firstTime = true) for
		//	NotifyEnterSystem. We want the soundtrack to change to the system
		//	track.

		m_Soundtrack.NotifyGameStart();
		m_Soundtrack.NotifyEnterSystem();
		}

#if 0
	else if (strEquals(sCmd, CMD_GAME_START_EXISTING))
		{
		m_Model.StartGame(false);
		m_iState = stateInGame;

		//	NOTE: It's OK to leave the default param (firstTime = true) for
		//	NotifyEnterSystem. We want the soundtrack to change to the system
		//	track.

		m_Soundtrack.NotifyGameStart();
		m_Soundtrack.NotifyEnterSystem();
		}
	else if (strEquals(sCmd, CMD_GAME_START_NEW))
		{
		m_Model.StartGame(true);
		m_iState = stateInGame;

		m_Soundtrack.NotifyGameStart();
		m_Soundtrack.NotifyEnterSystem();
		}
#endif

	//	Player notifications

	else if (strEquals(sCmd, CMD_PLAYER_COMBAT_MISSION_STARTED))
		m_Soundtrack.NotifyStartCombatMission();

	else if (strEquals(sCmd, CMD_PLAYER_COMBAT_ENDED))
		m_Soundtrack.NotifyEndCombat();

	else if (strEquals(sCmd, CMD_PLAYER_COMBAT_STARTED))
		m_Soundtrack.NotifyStartCombat();

	else if (strEquals(sCmd, CMD_PLAYER_UNDOCKED))
		m_Soundtrack.NotifyUndocked();

	//	Player hit 'G' to enter stargate

	else if (strEquals(sCmd, CMD_GAME_ENTER_STARGATE))
		{
		CTopologyNode *pDestNode = (CTopologyNode *)pData;
		if (pDestNode && !pDestNode->IsEndGame())
			{
			//	Figure out if we've entered this system before

			bool bFirstTime = m_Model.GetPlayer()->GetSystemEnteredTime(pDestNode->GetID()) == 0xffffffff;

			//	Tell the soundtrack

			m_Soundtrack.NotifyEnterSystem(pDestNode, bFirstTime);
			}
		}

	//	Left system; now inside the stargate

	else if (strEquals(sCmd, CMD_GAME_INSIDE_STARGATE))
		{
		m_HI.AddBackgroundTask(new CTravelThroughStargate(m_HI, m_Model), 0);
		}

	//	The new system is ready

	else if (strEquals(sCmd, CMD_GAME_STARGATE_SYSTEM_READY))
		{
		g_pTrans->OnStargateSystemReady();
		}

	//	Leave a stargate into a new system

	else if (strEquals(sCmd, CMD_GAME_LEAVE_STARGATE))
		{
		m_Model.OnPlayerExitedGate();
		}

	//	Player has entered final stargate and the game is over

	else if (strEquals(sCmd, CMD_GAME_ENTER_FINAL_STARGATE))
		{
		m_Model.EndGameStargate();

		//	Clean up game state

		g_pTrans->CleanUpDisplays();

		//	Epilogue

		HICommand(CMD_UI_START_EPILOGUE);
		}

	//	Start epilogue

	else if (strEquals(sCmd, CMD_UI_START_EPILOGUE))
		{
		//	If we have no crawl image, then go straight to intro

		CG32bitImage *pCrawlImage = m_Model.GetCrawlImage();
		const CString &sCrawlText = m_Model.GetCrawlText();
		if (pCrawlImage == NULL)
			{
			m_HI.HICommand(CMD_SESSION_EPILOGUE_DONE);
			return NOERROR;
			}

		//	Otherwise, start epilog session

		m_HI.ShowSession(new CTextCrawlSession(m_HI, m_Service, pCrawlImage, sCrawlText, CMD_SESSION_EPILOGUE_DONE));
		m_iState = stateEpilogue;
		m_Soundtrack.SetGameState(CSoundtrackManager::stateGameEpitaph, m_Model.GetCrawlSoundtrack());
		}

	//	Epilogue is done

	else if (strEquals(sCmd, CMD_SESSION_EPILOGUE_DONE))
		{
		//	Get stats

		CGameStats Stats;
		m_Model.GetGameStats(&Stats);

		//	Publish game record. We always post and allow the service 
		//	implementation to decide whether to post or not (based on whether
		//	the game is registered).

		DWORD dwFlags = 0;
		if (m_Service.HasCapability(ICIService::canPostGameRecord))
			{
			m_HI.AddBackgroundTask(new CPostRecordTask(m_HI, m_Service, m_Model.GetGameRecord(), Stats), 0);

			dwFlags |= CStatsSession::SHOW_TASK_PROGRESS;
			}

		//	Show stats

		if (Stats.GetCount() > 0)
			{
			//	The session takes handoff of the stats
			m_HI.ShowSession(new CStatsSession(m_HI, m_Service, Stats, dwFlags));
			m_iState = stateEndGameStats;
			}
		else
			{
			m_HI.ShowSession(new CIntroSession(m_HI, m_Model, CIntroSession::isEndGame));
			m_iState = stateIntro;
			DisplayMultiverseStatus(m_Multiverse.GetServiceStatus());
			m_Soundtrack.SetGameState(CSoundtrackManager::stateProgramIntro);
			}
		}

	//	Close game stats session

	else if (strEquals(sCmd, CMD_SESSION_STATS_DONE))
		{
		//	If we're showing end game stats, then we transition
		//	back to the intro.

		if (m_iState == stateEndGameStats)
			{
			m_HI.ShowSession(new CIntroSession(m_HI, m_Model, CIntroSession::isEndGame));
			m_iState = stateIntro;
			DisplayMultiverseStatus(m_Multiverse.GetServiceStatus());
			m_Soundtrack.SetGameState(CSoundtrackManager::stateProgramIntro);
			}

		//	Otherwise, we're in a popup that we can dismiss

		else
			m_HI.ClosePopupSession();
		}

	//	Show galactic map

	else if (strEquals(sCmd, CMD_UI_SHOW_GALACTIC_MAP))
		{
		//	Do we have a map installed?

		if (!m_Model.IsGalacticMapAvailable(&sError))
			{
			g_pTrans->DisplayMessage(sError);
			return NOERROR;
			}

		//	Show the map session

		CGalacticMapSession *pSession = new CGalacticMapSession(m_HI);
		if (error = m_HI.OpenPopupSession(pSession))
			return error;

		//	Close the system map, if it is open

		g_pTrans->ShowSystemMap(false);
		}

	//	Show game stats session

	else if (strEquals(sCmd, CMD_UI_SHOW_GAME_STATS))
		{
		CGameStats Stats;

		//	Get the stats

		if ((error = m_Model.GetGameStats(&Stats))
				|| Stats.GetCount() == 0)
			//	If we get an error (or stats come back blank, ignore)
			return NOERROR;

		//	The session takes handoff of the stats
		CStatsSession *pSession = new CStatsSession(m_HI, m_Service, Stats);
		if (error = m_HI.OpenPopupSession(pSession))
			return error;
		}

	//	Show help session

	else if (strEquals(sCmd, CMD_UI_SHOW_HELP))
		{
		DisplayMultiverseStatus(NULL_STR);
		CHelpSession *pSession = new CHelpSession(m_HI);
		if (error = m_HI.OpenPopupSession(pSession))
			return error;
		}

	//	Volume controls

	else if (strEquals(sCmd, CMD_UI_MUSIC_VOLUME_DOWN))
		{
		int iVolume = GetOptionInteger(CGameSettings::musicVolume);
		if (--iVolume >= 0)
			{
			SetOptionInteger(CGameSettings::musicVolume, iVolume);
			g_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("Volume %d"), iVolume));
			}
		}
	else if (strEquals(sCmd, CMD_UI_MUSIC_VOLUME_UP))
		{
		int iVolume = GetOptionInteger(CGameSettings::musicVolume);
		if (++iVolume <= 10)
			{
			SetOptionInteger(CGameSettings::musicVolume, iVolume);
			g_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("Volume %d"), iVolume));
			}
		}
	else if (strEquals(sCmd, CMD_UI_VOLUME_DOWN))
		{
		int iVolume = GetOptionInteger(CGameSettings::soundVolume);
		if (--iVolume >= 0)
			{
			SetOptionInteger(CGameSettings::soundVolume, iVolume);
			g_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("Volume %d"), iVolume));
			}
		}
	else if (strEquals(sCmd, CMD_UI_VOLUME_UP))
		{
		int iVolume = GetOptionInteger(CGameSettings::soundVolume);
		if (++iVolume <= 10)
			{
			SetOptionInteger(CGameSettings::soundVolume, iVolume);
			g_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("Volume %d"), iVolume));
			}
		}

	//	Pause/unpause

	else if (strEquals(sCmd, CMD_GAME_PAUSE))
		{
		m_Soundtrack.SetPlayPaused(false);
		}
	else if (strEquals(sCmd, CMD_GAME_UNPAUSE))
		{
		m_Soundtrack.SetPlayPaused(true);
		}

	//	Show login session

	else if (strEquals(sCmd, CMD_UI_SHOW_LOGIN))
		{
		CLoginSession *pSession = new CLoginSession(m_HI, m_Service);
		if (error = m_HI.OpenPopupSession(pSession))
			return error;
		}

	//	Show player profile session

	else if (strEquals(sCmd, CMD_UI_SHOW_PROFILE))
		{
		//	If we're not logged in, log in first.

		if (!m_Service.HasCapability(ICIService::canGetUserProfile))
			{
			//	We pass the uiShowProfile command to the login session. On
			//	success, it will fire the command.

			CLoginSession *pSession = new CLoginSession(m_HI, m_Service, CMD_UI_SHOW_PROFILE);
			if (error = m_HI.OpenPopupSession(pSession))
				return error;
			}

		//	Otherwise, show the session now

		else
			{
			DisplayMultiverseStatus(NULL_STR);
			CProfileSession *pSession = new CProfileSession(m_HI, m_Service);
			if (error = m_HI.OpenPopupSession(pSession))
				return error;
			}
		}

	//	Mod exchange

	else if (strEquals(sCmd, CMD_UI_SHOW_MOD_EXCHANGE))
		{
		//	If we're not logged in, log in first.

		if (!m_Service.HasCapability(ICIService::canGetUserProfile))
			{
			//	We pass the uiShowModExchange command to the login session. On
			//	success, it will fire the command.

			CLoginSession *pSession = new CLoginSession(m_HI, m_Service, CMD_UI_SHOW_MOD_EXCHANGE);
			if (error = m_HI.OpenPopupSession(pSession))
				return error;
			}

		//	Otherwise, show the session now

		else
			{
			DisplayMultiverseStatus(NULL_STR);
			if (error = m_HI.OpenPopupSession(new CModExchangeSession(m_HI, m_Service, m_Multiverse, g_pUniverse->GetExtensionCollection(), m_Settings.GetBoolean(CGameSettings::debugMode))))
				{
				m_HI.OpenPopupSession(new CMessageSession(m_HI, ERR_CANT_SHOW_MOD_EXCHANGE, NULL_STR, CMD_NULL));
				return NOERROR;
				}
			}
		}

	//	Service housekeeping

	else if (strEquals(sCmd, CMD_SERVICE_HOUSEKEEPING))
		{
		m_HI.AddBackgroundTask(new CServiceHousekeepingTask(m_HI, m_Service), 0);
		}

	//	Account has changed (user signed in or signed out, etc)

	else if (strEquals(sCmd, CMD_SERVICE_ACCOUNT_CHANGED))
		{
		//	Modify Multiverse state

		if (m_Service.HasCapability(ICIService::canGetUserProfile))
			{
			m_Multiverse.OnUserSignedIn(m_Service.GetUsername());
			m_iBackgroundState = stateSignedIn;
			}
		else
			{
			m_Multiverse.OnUserSignedOut();
			m_iBackgroundState = stateIdle;
			}

		//	If we're signed in then we should load the user's collection from
		//	the service.

		if (m_Multiverse.IsLoadCollectionNeeded() && m_Service.HasCapability(ICIService::canLoadUserCollection))
			{
			//	Start a task to load the collection (we pass in Multiverse so
			//	that the collection is placed there).

			m_iBackgroundState = stateLoadingCollection;
			m_HI.AddBackgroundTask(new CLoadUserCollectionTask(m_HI, m_Service, m_Multiverse), 0, this, CMD_SERVICE_COLLECTION_LOADED);
			}

		//	Now change the UI

		g_pTrans->OnAccountChanged(m_Multiverse);
		}

	//	Collection has been loaded

	else if (strEquals(sCmd, CMD_SERVICE_COLLECTION_LOADED))
		{
		m_iBackgroundState = stateIdle;

		//	Set registration status

		CMultiverseCollection Collection;
		if (m_Multiverse.GetCollection(&Collection) != NOERROR)
			{
			::kernelDebugLogMessage("Failed to GetCollection from Multiverse model.");
			return NOERROR;
			}

		TArray<CMultiverseCatalogEntry *> Download;
		g_pUniverse->SetRegisteredExtensions(Collection, &Download);

		//	Let the Mod Collection session refresh

		m_HI.HISessionCommand(CMD_SERVICE_EXTENSION_LOADED);

		//	If we need to download a new version, do so now.

#ifdef DEBUG_PROGRAM_UPGRADE
		if (true
#else
		if (m_Multiverse.GetUpgradeVersion() > m_Model.GetProgramVersion().dwProductVersion
#endif
				&& !m_Settings.GetBoolean(CGameSettings::noAutoUpdate)
				&& !IsUpgradeReady()
				&& !m_bUpgradeDownloaded)
			{
			m_iBackgroundState = stateDownloadingUpgrade;
			m_HI.AddBackgroundTask(new CUpgradeProgram(m_HI, m_Service, m_Multiverse.GetUpgradeURL()), CHumanInterface::FLAG_LOW_PRIORITY);

			//	Remember that we already did this so we don't try again later (e.g., if
			//	we refresh the collection).

			m_bUpgradeDownloaded = true;
			}

		//	Request download. If a request was made then begin a background
		//	task to process the download.

		else if (!m_Settings.GetBoolean(CGameSettings::noCollectionDownload)
				&& RequestCatalogDownload(Download))
			{
			m_iBackgroundState = stateDownloadingCatalogEntry;
			m_HI.AddBackgroundTask(new CProcessDownloadsTask(m_HI, m_Service), 0);
			}

		//	Otherwise we're done downloading

		else
			m_HI.HIPostCommand(CMD_SERVICE_DOWNLOADS_COMPLETE);
		}

	//	Upgrade ready

	else if (strEquals(sCmd, CMD_SERVICE_UPGRADE_READY))
		{
		DisplayMultiverseStatus(CONSTLIT("Saving install files..."));

		CString sError;
		error = WriteUpgradeFile((IMediaType *)pData, &sError);
		delete (IMediaType *)pData;
		if (error)
			{
			m_Multiverse.SetServiceStatus(NULL_STR);
			DisplayMultiverseStatus(sError, true);
			return error;
			}

		//	Install

		DisplayMultiverseStatus(CONSTLIT("Installing upgrade..."));
		if (!InstallUpgrade(&sError))
			{
			m_Multiverse.SetServiceStatus(NULL_STR);
			DisplayMultiverseStatus(sError, true);
			return ERR_FAIL;
			}

		//	Success

		m_Multiverse.SetServiceStatus(CONSTLIT("Please restart to upgrade."));
		DisplayMultiverseStatus(CONSTLIT("Please restart to upgrade."));
		}

	//	Service status

	else if (strEquals(sCmd, CMD_SERVICE_ERROR))
		{
		CString *pState = (CString *)pData;

		if (pState)
			{
			::kernelDebugLogMessage(pState->GetASCIIZPointer());
			m_Multiverse.SetServiceStatus(*pState);
			DisplayMultiverseStatus(*pState, true);
			delete pState;
			}
		else
			{
			::kernelDebugLogMessage("Unknown service error");
			m_Multiverse.SetServiceStatus(NULL_STR);
			DisplayMultiverseStatus(NULL_STR, true);
			}
		}

	//	Need to continue downloading

	else if (strEquals(sCmd, CMD_SERVICE_DOWNLOADS_IN_PROGRESS))
		m_HI.AddBackgroundTask(new CProcessDownloadsTask(m_HI, m_Service), 0);

	//	Extension file downloaded

	else if (strEquals(sCmd, CMD_SERVICE_FILE_DOWNLOADED))
		{
		if (m_iBackgroundState == stateDownloadingCatalogEntry)
			{
			//	Add a task to load the newly downloaded extension

			CHexarcDownloader::SStatus *pStatus = (CHexarcDownloader::SStatus *)pData;
			m_HI.AddBackgroundTask(new CLoadExtensionTask(m_HI, *pStatus), 0);
			delete pStatus;

			//	When done, CLoadExtensionTask will send us a cmdServiceExtensionLoaded 
			//	message.
			}
		else if (m_iBackgroundState == stateDownloadingResource)
			{
			CHexarcDownloader::SStatus *pStatus = (CHexarcDownloader::SStatus *)pData;

			//	Rename the file

			CString sPath = pathGetPath(pStatus->sFilespec);
			CString sFilename = pathGetFilename(pStatus->sFilespec);
			if (*sFilename.GetASCIIZPointer() == '~')
				sFilename = strSubString(sFilename, 1);

			CString sNewFilespec = pathAddComponent(sPath, sFilename);
			fileMove(pStatus->sFilespec, sNewFilespec);

			//	Continue loading resources

			m_HI.AddBackgroundTask(new CProcessDownloadsTask(m_HI, m_Service), 0);
			}
		}

	//	Extension finished loaded

	else if (strEquals(sCmd, CMD_SERVICE_EXTENSION_LOADED))
		{
		//	Let the current session know. For example, the Mod Collection session
		//	uses this command to reload its list.

		m_HI.HISessionCommand(CMD_SERVICE_EXTENSION_LOADED);

		//	Continue downloading

		m_HI.AddBackgroundTask(new CProcessDownloadsTask(m_HI, m_Service), 0);
		}

	//	Download external resource

	else if (strEquals(sCmd, CMD_SERVICE_DOWNLOAD_RESOURCES))
		{
		if (!m_Settings.GetBoolean(CGameSettings::noCollectionDownload))
			{
			TArray<CString> LocalFilenames;
			g_pUniverse->GetExtensionCollection().GetRequiredResources(&LocalFilenames);

			//	Ask the multiverse to map to a list of cloud filepaths

			TArray<CMultiverseFileRef> CloudFileRefs;
			m_Multiverse.GetResourceFileRefs(LocalFilenames, &CloudFileRefs);

			//	If necessary, request downloads

			if (RequestResourceDownload(CloudFileRefs))
				{
				m_iBackgroundState = stateDownloadingResource;
				m_HI.AddBackgroundTask(new CProcessDownloadsTask(m_HI, m_Service), 0);
				}
			else
				{
				m_iBackgroundState = stateIdle;
				::kernelDebugLogMessage("All resources downloaded.");
				}
			}
		}

	//	Done downloading extensions.

	else if (strEquals(sCmd, CMD_SERVICE_DOWNLOADS_COMPLETE))
		{
		//	If we're done loading catalog entries, then download news

		if ((m_iBackgroundState == stateIdle || m_iBackgroundState == stateDownloadingCatalogEntry)
				&& m_Service.HasCapability(ICIService::canLoadNews)
				&& m_Multiverse.IsLoadNewsNeeded())
			{
			//	Figure out the path for the news image cache

			CString sDownloadsFolder = pathAddComponent(m_Settings.GetAppDataFolder(), FILESPEC_DOWNLOADS_FOLDER);

			//	Make sure it exists

			if (!pathExists(sDownloadsFolder))
				pathCreate(sDownloadsFolder);

			//	Start a task to load the news (we pass in Multiverse so
			//	that the collection is placed there).

			m_iBackgroundState = stateLoadingNews;
			m_HI.AddBackgroundTask(new CLoadNewsTask(m_HI, m_Service, m_Multiverse, m_Model.GetProgramVersion(), sDownloadsFolder), 0, this, CMD_SERVICE_NEWS_LOADED);
			}

		//	Otherwise, if we're loading loading resources, then handle them

		else if (m_iBackgroundState == stateDownloadingResource)
			{
			m_iBackgroundState = stateIdle;
			}

		//	Ask the extension collection for a list of missing resource files.

		else
			HICommand(CMD_SERVICE_DOWNLOAD_RESOURCES);
		}

	//	News loaded

	else if (strEquals(sCmd, CMD_SERVICE_NEWS_LOADED))
		{
		//	Tell the current session that we loaded news.

		m_HI.HISessionCommand(CMD_SERVICE_NEWS_LOADED);

		//	Ask the extension collection for a list of missing resource files.

		HICommand(CMD_SERVICE_DOWNLOAD_RESOURCES);
		}

	//	High score list

	else if (strEquals(sCmd, CMD_SERVICE_HIGH_SCORE_LIST_LOADED))
		{
		CAdventureHighScoreList *pHighScoreList = (CAdventureHighScoreList *)pData;

		//	If we're still in the intro, pass it on to it (it will take 
		//	ownership of it).

		if (m_iState == stateIntro)
			m_HI.HISessionCommand(CMD_SERVICE_HIGH_SCORE_LIST_LOADED, pHighScoreList);

		//	Otherwise, we free the list

		else
			delete pHighScoreList;
		}

	else if (strEquals(sCmd, CMD_SERVICE_HIGH_SCORE_LIST_ERROR))
		m_HI.HISessionCommand(sCmd);

	else if (strEquals(sCmd, CMD_SERVICE_STATUS))
		{
		CString *pState = (CString *)pData;

		if (pState)
			{
			m_Multiverse.SetServiceStatus(*pState);
			DisplayMultiverseStatus(*pState);
			delete pState;
			}
		else
			{
			m_Multiverse.SetServiceStatus(NULL_STR);
			DisplayMultiverseStatus(NULL_STR);
			}
		}

	//	Sign out

	else if (strEquals(sCmd, CMD_UI_SIGN_OUT))
		{
		//	We don't need a special notification when sign out is complete, 
		//	since the service will send out a serviceAccountChanged command.
		m_HI.AddBackgroundTask(new CSignOutUserTask(m_HI, m_Service), 0, this, CMD_NULL);
		}

	//	Change password
	
	else if (strEquals(sCmd, CMD_UI_CHANGE_PASSWORD))
		{
		m_HI.OpenPopupSession(new CChangePasswordSession(m_HI, m_Service));
		}

	//	Reset password

	else if (strEquals(sCmd, CMD_UI_RESET_PASSWORD))
		{
		m_HI.OpenPopupSession(new CMessageSession(m_HI, ERR_CONTACT_KP, ERR_RESET_PASSWORD_DESC, CMD_NULL));
		}

	//	Soundtrack control

	else if (strEquals(sCmd, CMD_SOUNDTRACK_DONE))
		m_Soundtrack.NotifyTrackDone();

	else if (strEquals(sCmd, CMD_SOUNDTRACK_NEXT))
		m_Soundtrack.NextTrack();

	else if (strEquals(sCmd, CMD_SOUNDTRACK_NOW_PLAYING))
		{
		m_Soundtrack.NotifyTrackPlaying((CSoundType *)pData);
		m_HI.HISessionCommand(sCmd, pData);
		}

	else if (strEquals(sCmd, CMD_SOUNDTRACK_UPDATE_PLAY_POS))
		m_Soundtrack.NotifyUpdatePlayPos((int)pData);

	else if (strEquals(sCmd, CMD_SOUNDTRACK_PLAY_PAUSE))
		m_Soundtrack.TogglePlayPaused();

	//	Update high score list

	else if (strEquals(sCmd, CMD_UPDATE_HIGH_SCORE_LIST))
		{
		CAdventureHighScoreList::SSelect *pSelect = (CAdventureHighScoreList::SSelect *)pData;
		m_HI.AddBackgroundTask(new CReadHighScoreListTask(m_HI, m_Service, *pSelect), 0);
		}

	//	Exit

	else if (strEquals(sCmd, CMD_UI_EXIT))
		m_HI.Exit();

	//	Crash report

	else if (strEquals(sCmd, CMD_POST_CRASH_REPORT))
		{
		if (!m_Settings.GetBoolean(CGameSettings::noCrashPost))
			{
			CString *pCrashReport = (CString *)pData;
			m_HI.AddBackgroundTask(new CPostCrashReportTask(m_HI, m_Service, *pCrashReport), 0);
			delete pCrashReport;
			}
		}

	//	Else we did not handle it

	else
		return ERR_NOTFOUND;

	return NOERROR;
	}

ALERROR CTranscendenceController::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	ALERROR error;

	//	Initialize our legacy window

	g_pTrans = new CTranscendenceWnd(m_HI.GetHWND(), this);

	//	Initialize the model

	if (error = m_Model.Init(m_Settings))
		return error;

	//	Figure out where the Collection folder is and where the Extension
	//	folders are.

	CString sCollectionFolder = pathAddComponent(m_Settings.GetAppDataFolder(), FOLDER_COLLECTION);
	TArray<CString> ExtensionFolders;
	ExtensionFolders.Insert(pathAddComponent(m_Settings.GetAppDataFolder(), FOLDER_EXTENSIONS));

	//	Add additional folders from our settings

	ExtensionFolders.Insert(m_Settings.GetExtensionFolders());

	//	If our AppData is elsewhere, then add an Extensions folder under the
	//	current folder. [This allows the player to manually place extensions in Program Files.]

	if (!m_Settings.GetAppDataFolder().IsBlank())
		ExtensionFolders.Insert(FOLDER_EXTENSIONS);

	//	If we're using the current directory for AppData, then also look for
	//	save files in the current directory (for backwards compatibility)

	if (m_Settings.GetAppDataFolder().IsBlank())
		m_Model.AddSaveFileFolder(CONSTLIT(""));

	//	Kick off a background initialization of the model
	//	(this will load the universe)

	m_HI.AddBackgroundTask(new CInitModelTask(m_HI, m_Model, m_Settings, sCollectionFolder, ExtensionFolders), 0, this, CMD_MODEL_INIT_DONE);

	//	If the clouds services have not been initialized yet (because there was no
	//	<Services> tag in the settings file) then initialize to defaults here.

	if (m_Service.IsEmpty())
		{
		bool bModified;
		if (error = m_Service.InitFromXML(m_HI, NULL, &bModified))
			return error;

		if (bModified)
			m_Settings.SetModified();
		}

	//	Allow the service to load private info

	if (error = m_Service.InitPrivateData())
		return error;

	//	Initialize the Multiverse model based on current service states.

	if (!m_Service.HasCapability(ICIService::loginUser))
		m_Multiverse.SetDisabled();
	else if (m_Service.HasCapability(ICIService::cachedUser))
		m_Multiverse.SetUsername(m_Service.GetUsername());

	//	Add a timer so that services can do some background processing.

	m_HI.AddTimer(SERVICE_HOUSEKEEPING_INTERVAL, this, CMD_SERVICE_HOUSEKEEPING);

	//	Show the loading screen

	m_HI.ShowSession(new CLoadingSession(m_HI, m_Model.GetCopyright()));
	m_iState = stateLoading;

	//	Play Intro Music

	m_Soundtrack.SetMusicEnabled(!GetOptionBoolean(CGameSettings::noMusic));
	m_Soundtrack.SetVolume(GetOptionInteger(CGameSettings::musicVolume));
	m_Soundtrack.SetGameState(CSoundtrackManager::stateProgramLoad);

	//	Initialize legacy window

	if (g_pTrans->WMCreate(retsError) != 0)
		{
		delete g_pTrans;
		g_pTrans = NULL;
		return ERR_FAIL;
		}

	return NOERROR;
	}

ALERROR CTranscendenceController::OnLoadSettings (CXMLElement *pDesc, bool *retbModified)

//	OnLoadSettings
//
//	This is called to load any settings that CGameSettings doesn't understand
//	(Note: This may be called inside of OnBoot, before OnInit is called)

	{
	ALERROR error;

	if (strEquals(pDesc->GetTag(), SERVICES_TAG))
		{
		if (error = m_Service.InitFromXML(m_HI, pDesc, retbModified))
			return error;
		}

	return NOERROR;
	}

ALERROR CTranscendenceController::OnSaveSettings (IWriteStream *pOutput)

//	OnSaveSettings
//
//	This is called to save any settings that CGameSettings doesn't understand
//	(e.g., m_Services)

	{
	ALERROR error;

	//	Write out cloud services settings

	if (error = m_Service.WriteAsXML(pOutput))
		return error;

	return NOERROR;
	}

void CTranscendenceController::OnShutdown (EHIShutdownReasons iShutdownCode)

//	OnShutdown
//
//	The game is shutting down

	{
	//	Stop music

	m_HI.GetSoundMgr().StopMusic();

	//	Clean up displays (or else we'll be holding on to bad pointers after the
	//	universe gets cleaned up)

	g_pTrans->CleanUpDisplays();

	//	If we're still in the game and we're exiting, make sure
	//	to save the game first
	//
	//	(But only if we're not crashing...)

	if (iShutdownCode != HIShutdownByHardCrash)
		{
		m_Model.EndGameClose();

		//	Save stuff

		if (m_Service.IsModified())
			{
			m_Settings.SetModified();

			//	Write private data

			m_Service.WritePrivateData();
			}

		m_Model.SaveHighScoreList();
		m_Multiverse.Save(pathAddComponent(m_Settings.GetAppDataFolder(), FILESPEC_DOWNLOADS_FOLDER));
		m_Settings.Save(SETTINGS_FILENAME);
		}
	}

void CTranscendenceController::PaintDebugInfo (CG32bitImage &Dest, const RECT &rcScreen)

//	PaintDebugInfo
//
//	Paints debug info on top of the screen

	{
#ifdef DEBUG_SOUNDTRACK_STATE
	m_Soundtrack.PaintDebugInfo(Dest, rcScreen);
#endif
	}

bool CTranscendenceController::RequestCatalogDownload (const TArray<CMultiverseCatalogEntry *> &Downloads)

//	RequestCatalogDownload
//
//	Initiates a request to download file. We return TRUE if we made the request.
//	FALSE otherwise.

	{
	int i;

	//	If nothing to do then we're done.

	if (Downloads.GetCount() == 0)
		return false;

	//	Figure out the path

	CString sDownloadsFolder = pathAddComponent(m_Settings.GetAppDataFolder(), FILESPEC_DOWNLOADS_FOLDER);

	//	Make sure that we have the Downloads folder

	if (!pathExists(sDownloadsFolder))
		pathCreate(sDownloadsFolder);

	//	If we have to download extensions, do it now.

	for (i = 0; i < Downloads.GetCount(); i++)
		{
		//	Get the name of the filePath of the file to download

		const CString &sFilePath = Downloads[i]->GetTDBFileRef().GetFilePath();
		const CIntegerIP &FileDigest = Downloads[i]->GetTDBFileRef().GetDigest();

		//	Generate a path to download to

		CString sFilespec = pathAddComponent(sDownloadsFolder, CHexarc::GetFilenameFromFilePath(sFilePath));

		//	Request a download. (We can do this synchronously because it
		//	doesn't take long and the call is thread-safe).

		m_Service.RequestExtensionDownload(sFilePath, sFilespec, FileDigest);
		}

	//	Done

	return true;
	}

bool CTranscendenceController::RequestResourceDownload (const TArray<CMultiverseFileRef> &Downloads)

//	RequestResourceDownload
//
//	Initiates a request to download the given files.

	{
	int i;

	//	If nothing to do then we're done.

	if (Downloads.GetCount() == 0)
		return false;

	//	Figure out the path

	CString sDownloadsFolder = pathAddComponent(m_Settings.GetAppDataFolder(), FILESPEC_DOWNLOADS_FOLDER);

	//	Make sure that we have the Downloads folder

	if (!pathExists(sDownloadsFolder))
		pathCreate(sDownloadsFolder);

	//	Loop over all files to download

	for (i = 0; i < Downloads.GetCount(); i++)
		{
		//	We download straight to the final destination (Collections folder) 
		//	but we add a tilde so it doesn't get used yet.

		CString sDestFolder = pathGetPath(Downloads[i].GetFilespec());
		CString sDestFilename = strPatternSubst(CONSTLIT("~%s"), pathGetFilename(Downloads[i].GetFilespec()));

		//	Make sure the folder exists

		if (!pathExists(sDestFolder))
			pathCreate(sDestFolder);

		//	Request a download. (We can do this synchronously because it
		//	doesn't take long and the call is thread-safe).

		m_Service.RequestExtensionDownload(Downloads[i].GetFilePath(), pathAddComponent(sDestFolder, sDestFilename), CIntegerIP());
		}

	//	Done

	return true;
	}

void CTranscendenceController::SetOptionBoolean (int iOption, bool bValue)

//	SetOptionBoolean
//
//	Set the option

	{
	if (bValue == GetOptionBoolean(iOption))
		return;

	bool bModifySettings = true;
	switch (iOption)
		{
		case CGameSettings::noMusic:
			m_Soundtrack.SetMusicEnabled(!bValue);
			break;
		}

	m_Settings.SetBoolean(iOption, bValue, bModifySettings);
	}

void CTranscendenceController::SetOptionInteger (int iOption, int iValue)

//	SetOptionInteger
//
//	Set the option

	{
	if (iValue == GetOptionInteger(iOption))
		return;

	bool bModifySettings = true;
	switch (iOption)
		{
		case CGameSettings::musicVolume:
			ASSERT(iValue >= 0 && iValue <= 10);
			m_Soundtrack.SetVolume(iValue);
			break;

		case CGameSettings::soundVolume:
			ASSERT(iValue >= 0 && iValue <= 10);
			m_HI.SetSoundVolume(iValue);
			break;
		}

	m_Settings.SetInteger(iOption, iValue, bModifySettings);
	}

ALERROR CTranscendenceController::WriteUpgradeFile (IMediaType *pData, CString *retsError)

//	WriteUpgradeFile
//
//	Writes out an upgrade zip file to the Downloads directory

	{
	const CString &sBuffer = pData->GetMediaBuffer();

	//	Write out to Downloads folder

	CString sOutputFile = pathAddComponent(m_Settings.GetAppDataFolder(), FILESPEC_UPGRADE_FILE);
	CFileWriteStream File(sOutputFile);
	if (File.Create() != NOERROR)
		{
		*retsError = CONSTLIT("Unable to create Upgrade.zip");
		return ERR_FAIL;
		}

	File.Write(sBuffer.GetASCIIZPointer(), sBuffer.GetLength());
	File.Close();

	//	Done

	return NOERROR;
	}
