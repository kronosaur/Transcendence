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
#include "Transcendence.h"

#define SERVICES_TAG							CONSTLIT("Services")

#define CMD_NULL								CONSTLIT("null")

#define CMD_POST_CRASH_REPORT					CONSTLIT("cmdPostCrashReport")
#define CMD_SHOW_WAIT_ANIMATION					CONSTLIT("cmdShowWaitAnimation")
#define CMD_SOUNDTRACK_DONE						CONSTLIT("cmdSoundtrackDone")
#define CMD_SOUNDTRACK_NEXT						CONSTLIT("cmdSoundtrackNext")
#define CMD_SOUNDTRACK_PLAY_PAUSE				CONSTLIT("cmdSoundtrackPlayPause")
#define CMD_SOUNDTRACK_PREV						CONSTLIT("cmdSoundtrackPrev")
#define CMD_SOUNDTRACK_STOP						CONSTLIT("cmdSoundtrackStop")

#define CMD_GAME_ADVENTURE						CONSTLIT("gameAdventure")
#define CMD_GAME_CREATE							CONSTLIT("gameCreate")
#define CMD_GAME_END_DESTROYED					CONSTLIT("gameEndDestroyed")
#define CMD_GAME_END_SAVE						CONSTLIT("gameEndSave")
#define CMD_GAME_ENTER_FINAL_STARGATE			CONSTLIT("gameEnterFinalStargate")
#define CMD_GAME_ENTER_STARGATE					CONSTLIT("gameEnterStargate")
#define CMD_GAME_LEAVE_STARGATE					CONSTLIT("gameLeaveStargate")
#define CMD_GAME_LOAD							CONSTLIT("gameLoad")
#define CMD_GAME_LOAD_DONE						CONSTLIT("gameLoadDone")
#define CMD_GAME_SELECT_ADVENTURE				CONSTLIT("gameSelectAdventure")
#define CMD_GAME_SELECT_SAVE_FILE				CONSTLIT("gameSelectSaveFile")
#define CMD_GAME_START_EXISTING					CONSTLIT("gameStartExisting")
#define CMD_GAME_START_NEW						CONSTLIT("gameStartNew")

#define CMD_MODEL_ADVENTURE_INIT_DONE			CONSTLIT("modelAdventureInitDone")
#define CMD_MODEL_INIT_DONE						CONSTLIT("modelInitDone")
#define CMD_MODEL_NEW_GAME_CREATED				CONSTLIT("modelNewGameCreated")

#define CMD_PLAYER_COMBAT_MISSION_STARTED		CONSTLIT("playerCombatMisionStarted")
#define CMD_PLAYER_UNDOCKED						CONSTLIT("playerUndocked")

#define CMD_SERVICE_ACCOUNT_CHANGED				CONSTLIT("serviceAccountChanged")
#define CMD_SERVICE_COLLECTION_LOADED			CONSTLIT("serviceCollectionLoaded")
#define CMD_SERVICE_DOWNLOADS_IN_PROGRESS		CONSTLIT("serviceDownloadsInProgress")
#define CMD_SERVICE_ERROR						CONSTLIT("serviceError")
#define CMD_SERVICE_EXTENSION_DOWNLOADED		CONSTLIT("serviceExtensionDownloaded")
#define CMD_SERVICE_EXTENSION_LOADED			CONSTLIT("serviceExtensionLoaded")
#define CMD_SERVICE_HOUSEKEEPING				CONSTLIT("serviceHousekeeping")
#define CMD_SERVICE_STATUS						CONSTLIT("serviceStatus")

#define CMD_SESSION_STATS_DONE					CONSTLIT("sessionStatsDone")
#define CMD_SESSION_EPILOGUE_DONE				CONSTLIT("sessionEpilogueDone")
#define CMD_SESSION_PROLOGUE_DONE				CONSTLIT("sessionPrologueDone")

#define CMD_UI_BACK_TO_INTRO					CONSTLIT("uiBackToIntro")
#define CMD_UI_CHANGE_PASSWORD					CONSTLIT("uiChangePassword")
#define CMD_UI_EXIT								CONSTLIT("uiExit")
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

#define FILESPEC_DOWNLOADS_FOLDER				CONSTLIT("Downloads")

#define ID_MULTIVERSE_STATUS_SEQ				CONSTLIT("idMultiverseStatusSeq")
#define ID_MULTIVERSE_STATUS_TEXT				CONSTLIT("idMultiverseStatusText")

#define DEBUG_LOG_FILENAME						CONSTLIT("Debug.log")
#define SETTINGS_FILENAME						CONSTLIT("Settings.xml")

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

ALERROR CTranscendenceController::OnBoot (char *pszCommandLine, SHIOptions &Options)

//	OnBoot
//
//	Boot up controller and return options for HI

	{
	ALERROR error;

	//	Set our default directory

	CString sCurDir = pathGetExecutablePath(g_hInst);
	::SetCurrentDirectory(sCurDir.GetASCIIZPointer());

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
		return error;

	//	Set the debug log

	if (!bLogFileOpened && !m_Settings.GetBoolean(CGameSettings::noDebugLog))
		if (error = kernelSetDebugLog(DEBUG_LOG_FILENAME))
			return error;

	//	If we're windowed, figure out the size that we want.

	Options.m_bWindowedMode = m_Settings.GetBoolean(CGameSettings::windowedMode);
	if (Options.m_bWindowedMode)
		{
		//	If we're forcing 1024, then we know the size

		if (m_Settings.GetBoolean(CGameSettings::force1024Res))
			{
			Options.m_cxScreenDesired = 1024;
			Options.m_cyScreenDesired = 768;
			}
		else if (m_Settings.GetBoolean(CGameSettings::force600Res))
			{
			Options.m_cxScreenDesired = 1024;
			Options.m_cyScreenDesired = 600;
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
				Options.m_cxScreenDesired = 1280;
				Options.m_cyScreenDesired = 1024;
				}
			else
				{
				Options.m_cyScreenDesired = Max(768, cyAvail - cyExtra);
				Options.m_cxScreenDesired = Options.m_cyScreenDesired * 1024 / 768;
				}
			}
		}

	//	Now set other options for HI

	Options.m_iColorDepthDesired = 16;
	Options.m_bForceDX = (m_Settings.GetBoolean(CGameSettings::forceDirectX) || m_Settings.GetBoolean(CGameSettings::forceExclusive) || m_Settings.GetBoolean(CGameSettings::force1024Res));
	Options.m_bForceNonDX = (m_Settings.GetBoolean(CGameSettings::forceNonDirectX) && !Options.m_bForceDX);
	Options.m_bForceExclusiveMode = (m_Settings.GetBoolean(CGameSettings::forceExclusive) || m_Settings.GetBoolean(CGameSettings::force1024Res));
	Options.m_bForceNonExclusiveMode = (m_Settings.GetBoolean(CGameSettings::forceNonExclusive) && !Options.m_bForceExclusiveMode);
	Options.m_bForceScreenSize = m_Settings.GetBoolean(CGameSettings::force1024Res);
	Options.m_iSoundVolume = m_Settings.GetInteger(CGameSettings::soundVolume);
	Options.m_sMusicFolder = m_Settings.GetString(CGameSettings::musicPath);
	Options.m_bDebugVideo = m_Settings.GetBoolean(CGameSettings::debugVideo);

	return NOERROR;
	}

void CTranscendenceController::OnCleanUp (void)

//	OnCleanUp
//
//	Clean up

	{
	//	Clean up

	m_Model.CleanUp();
	kernelClearDebugLog();
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

			m_HI.AddBackgroundTask(new CLoadGameWithSignInTask(m_HI, m_Service, m_Model, sSaveFile), this, CMD_GAME_LOAD_DONE);

			//	Show transition session while we load

			m_HI.ShowSession(new CWaitSession(m_HI, m_Service, CONSTLIT("Loading Game")));
			return NOERROR;
			}

		//	If we can sign in automatically, do so now.

		if (m_Service.HasCapability(ICIService::autoLoginUser))
			m_HI.AddBackgroundTask(new CSignInUserTask(m_HI, m_Service, NULL_STR, NULL_STR, true));

		//	Legacy CTranscendenceWnd takes over

		m_HI.ShowSession(new CLegacySession(m_HI));

		//	Start the intro

		g_pTrans->StartIntro(CTranscendenceWnd::isOpeningTitles);
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

		m_HI.AddBackgroundTask(new CInitAdventureTask(m_HI, m_Model, *pNewAdventure), this, CMD_MODEL_ADVENTURE_INIT_DONE);

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
			Defaults.sPlayerName = m_Service.GetDefaultUsername();

		if (Defaults.sPlayerName.IsBlank())
			Defaults.sPlayerName = ::sysGetUserName();

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
		m_HI.ShowSession(new CLegacySession(m_HI));
		g_pTrans->StartIntro(CTranscendenceWnd::isBlank);
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

		m_Settings.SetString(CGameSettings::playerName, pNewGame->sPlayerName);
		m_Settings.SetString(CGameSettings::playerGenome, GetGenomeID(pNewGame->iPlayerGenome));
		m_Settings.SetInteger(CGameSettings::playerShipClass, (int)pNewGame->dwPlayerShip);

		//	Report creation

		kernelDebugLogMessage("Created new game");

		//	Kick-off background thread to finish up creating the game

		g_pTrans->SetGameCreated(false);
		m_HI.AddBackgroundTask(new CStartNewGameTask(m_HI, m_Model, *pNewGame), this, CMD_MODEL_NEW_GAME_CREATED);

		//	Start the prologue

		CG16bitImage *pCrawlImage = m_Model.GetCrawlImage();
		const CString &sCrawlText = m_Model.GetCrawlText();

		m_HI.ShowSession(new CTextCrawlSession(m_HI, pCrawlImage, sCrawlText, CMD_SESSION_PROLOGUE_DONE));
		m_iState = statePrologue;
		m_Soundtrack.SetGameState(CSoundtrackManager::stateGamePrologue, m_Model.GetCrawlSoundtrack());
		}

	//	Player has stopped the prologue

	else if (strEquals(sCmd, CMD_SESSION_PROLOGUE_DONE))
		{
		m_iState = statePrologueDone;

		//	If we're done create the new game then we can continue

		if (g_pTrans->IsGameCreated())
			HICommand(CMD_UI_START_GAME);

		//	Otherwise start wait animation

		else
			m_HI.GetSession()->HICommand(CMD_SHOW_WAIT_ANIMATION);
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

		//	If the prologue is done, then we can start the game

		if (m_iState == statePrologueDone)
			HICommand(CMD_UI_START_GAME);
		}

	//	Start the game

	else if (strEquals(sCmd, CMD_UI_START_GAME))
		{
		m_HI.ShowSession(new CLegacySession(m_HI));
		g_pTrans->StartGame(true);
		}

	//	Choose a save file to load

	else if (strEquals(sCmd, CMD_GAME_SELECT_SAVE_FILE))
		{
		DisplayMultiverseStatus(NULL_STR);
		if (error = m_HI.OpenPopupSession(new CLoadGameSession(m_HI, m_Service)))
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

		m_HI.AddBackgroundTask(new CLoadGameTask(m_HI, m_Model, m_Service.GetUsername(), *pFilespec), this, CMD_GAME_LOAD_DONE);

		//	Show transition session while we load

		m_HI.ShowSession(new CWaitSession(m_HI, m_Service, CONSTLIT("Loading Game")));
		}

		//	Background load of game is done

	else if (strEquals(sCmd, CMD_GAME_LOAD_DONE))
		{
		CLoadGameTask *pTask = (CLoadGameTask *)pData;

		//	Check for error

		if (pTask->GetResult(&sError))
			{
			kernelDebugLogMessage(sError);
			m_HI.OpenPopupSession(new CMessageSession(m_HI, ERR_CANT_LOAD_GAME, sError, CMD_UI_BACK_TO_INTRO));
			return NOERROR;
			}

		//	Start game (this does some stuff and then calls cmdGameStart)

		m_HI.ShowSession(new CLegacySession(m_HI));
		g_pTrans->StartGame();
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

		m_HI.ShowSession(new CLegacySession(m_HI));
		g_pTrans->StartIntro(CTranscendenceWnd::isShipStats);
		m_iState = stateIntro;
		DisplayMultiverseStatus(m_Multiverse.GetServiceStatus());
		m_Soundtrack.SetGameState(CSoundtrackManager::stateProgramIntro);
		}

	//	Start the game

	else if (strEquals(sCmd, CMD_GAME_START_EXISTING))
		{
		m_Model.StartGame(false);
		m_iState = stateInGame;
		m_Soundtrack.NotifyEnterSystem();
		}
	else if (strEquals(sCmd, CMD_GAME_START_NEW))
		{
		m_Model.StartGame(true);
		m_iState = stateInGame;
		m_Soundtrack.NotifyEnterSystem();
		}

	//	Player notifications

	else if (strEquals(sCmd, CMD_PLAYER_COMBAT_MISSION_STARTED))
		m_Soundtrack.NotifyStartCombatMission();

	else if (strEquals(sCmd, CMD_PLAYER_UNDOCKED))
		m_Soundtrack.NotifyUndocked();

	//	Entering a stargate

	else if (strEquals(sCmd, CMD_GAME_ENTER_STARGATE))
		{
		m_Model.OnPlayerTraveledThroughGate();
		}

	//	Leave a stargate into a new system

	else if (strEquals(sCmd, CMD_GAME_LEAVE_STARGATE))
		{
		m_Model.OnPlayerExitedGate();
		m_Soundtrack.NotifyEnterSystem();
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

		CG16bitImage *pCrawlImage = m_Model.GetCrawlImage();
		const CString &sCrawlText = m_Model.GetCrawlText();
		if (pCrawlImage == NULL)
			{
			m_HI.HICommand(CMD_SESSION_EPILOGUE_DONE);
			return NOERROR;
			}

		//	Otherwise, start epilog session

		m_HI.ShowSession(new CTextCrawlSession(m_HI, pCrawlImage, sCrawlText, CMD_SESSION_EPILOGUE_DONE));
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
			m_HI.AddBackgroundTask(new CPostRecordTask(m_HI, m_Service, m_Model.GetGameRecord(), Stats));

			dwFlags |= CStatsSession::SHOW_TASK_PROGRESS;
			}

		//	Show stats

		if (Stats.GetCount() > 0)
			{
			//	The session takes handoff of the stats
			m_HI.ShowSession(new CStatsSession(m_HI, Stats, dwFlags));
			m_iState = stateEndGameStats;
			}
		else
			{
			m_HI.ShowSession(new CLegacySession(m_HI));
			g_pTrans->StartIntro(CTranscendenceWnd::isEndGame);
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
			m_HI.ShowSession(new CLegacySession(m_HI));
			g_pTrans->StartIntro(CTranscendenceWnd::isEndGame);
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
		CStatsSession *pSession = new CStatsSession(m_HI, Stats);
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
		m_HI.AddBackgroundTask(new CServiceHousekeepingTask(m_HI, m_Service));
		}

	//	Account has changed (user signed in or signed out, etc)

	else if (strEquals(sCmd, CMD_SERVICE_ACCOUNT_CHANGED))
		{
		//	Modify Multiverse state

		if (m_Service.HasCapability(ICIService::canGetUserProfile))
			m_Multiverse.OnUserSignedIn(m_Service.GetUsername());
		else
			m_Multiverse.OnUserSignedOut();

		//	If we're signed in then we should load the user's collection from
		//	the service.

		if (m_Multiverse.IsLoadCollectionNeeded() && m_Service.HasCapability(ICIService::canLoadUserCollection))
			{
			//	Start a task to load the collection (we pass in Multiverse so
			//	that the collection is placed there).

			m_HI.AddBackgroundTask(new CLoadUserCollectionTask(m_HI, m_Service, m_Multiverse), this, CMD_SERVICE_COLLECTION_LOADED);
			}

		//	Now change the UI

		g_pTrans->OnAccountChanged(m_Multiverse);
		}

	//	Collection has been loaded

	else if (strEquals(sCmd, CMD_SERVICE_COLLECTION_LOADED))
		{
		//	Set registration status

		CMultiverseCollection Collection;
		if (m_Multiverse.GetCollection(&Collection) != NOERROR)
			return NOERROR;

		TArray<CMultiverseCatalogEntry *> Download;
		g_pUniverse->SetRegisteredExtensions(Collection, &Download);

		//	Request download. If a request was made then begin a background
		//	task to process the download.

		if (RequestCatalogDownload(Download))
			m_HI.AddBackgroundTask(new CProcessDownloadsTask(m_HI, m_Service));
		}

	//	Service status

	else if (strEquals(sCmd, CMD_SERVICE_ERROR))
		{
		CString *pState = (CString *)pData;

		if (pState)
			{
			m_Multiverse.SetServiceStatus(*pState);
			DisplayMultiverseStatus(*pState, true);
			delete pState;
			}
		else
			{
			m_Multiverse.SetServiceStatus(NULL_STR);
			DisplayMultiverseStatus(NULL_STR, true);
			}
		}

	//	Need to continue downloading

	else if (strEquals(sCmd, CMD_SERVICE_DOWNLOADS_IN_PROGRESS))
		m_HI.AddBackgroundTask(new CProcessDownloadsTask(m_HI, m_Service));

	//	Extension file downloaded

	else if (strEquals(sCmd, CMD_SERVICE_EXTENSION_DOWNLOADED))
		{
		//	Add a task to load the newly downloaded extension

		CHexarcDownloader::SStatus *pStatus = (CHexarcDownloader::SStatus *)pData;
		m_HI.AddBackgroundTask(new CLoadExtensionTask(m_HI, *pStatus));
		delete pStatus;

		//	When done, CLoadExtensionTask will send us a cmdServiceExtensionLoaded 
		//	message.
		}

	//	Extension finished loaded

	else if (strEquals(sCmd, CMD_SERVICE_EXTENSION_LOADED))
		{
		//	Let the current session know. For example, the Mod Collection session
		//	uses this command to reload its list.

		m_HI.GetSession()->HICommand(CMD_SERVICE_EXTENSION_LOADED);

		//	Continue downloading

		m_HI.AddBackgroundTask(new CProcessDownloadsTask(m_HI, m_Service));
		}

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
		m_HI.AddBackgroundTask(new CSignOutUserTask(m_HI, m_Service), this, CMD_NULL);
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
		m_Soundtrack.NotifyTrackDone();

	else if (strEquals(sCmd, CMD_SOUNDTRACK_PLAY_PAUSE))
		m_Soundtrack.TogglePlayPaused();

	//	Exit

	else if (strEquals(sCmd, CMD_UI_EXIT))
		{
		//	LATER: This should be an m_HI method
		::SendMessage(m_HI.GetHWND(), WM_CLOSE, 0, 0);
		}

	//	Crash report

	else if (strEquals(sCmd, CMD_POST_CRASH_REPORT))
		{
		if (!m_Settings.GetBoolean(CGameSettings::noCrashPost))
			{
			CString *pCrashReport = (CString *)pData;
			m_HI.AddBackgroundTask(new CPostCrashReportTask(m_HI, m_Service, *pCrashReport));
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

	//	Initialize the model

	if (error = m_Model.Init())
		return error;

	//	Set some options

	m_Model.SetDebugMode(m_Settings.GetBoolean(CGameSettings::debugGame));
	m_Model.SetForceTDB(m_Settings.GetBoolean(CGameSettings::useTDB));
	m_Model.SetNoSound(m_Settings.GetBoolean(CGameSettings::noSound));

	//	Kick off a background initialization of the model
	//	(this will load the universe)

	m_HI.AddBackgroundTask(new CInitModelTask(m_HI, m_Model), this, CMD_MODEL_INIT_DONE);

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
	m_Soundtrack.SetGameState(CSoundtrackManager::stateProgramLoad);

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
		m_Settings.Save(SETTINGS_FILENAME);
		}
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

	//	Make sure that we have the Downloads folder

	if (!pathExists(FILESPEC_DOWNLOADS_FOLDER))
		pathCreate(FILESPEC_DOWNLOADS_FOLDER);

	//	If we have to download extensions, do it now.

	for (i = 0; i < Downloads.GetCount(); i++)
		{
		//	Get the name of the filePath of the file to download

		const CString &sFilePath = Downloads[i]->GetTDBFileRef().GetFilePath();
		const CIntegerIP &FileDigest = Downloads[i]->GetTDBFileRef().GetDigest();

		//	Generate a path to download to

		CString sFilespec = pathAddComponent(FILESPEC_DOWNLOADS_FOLDER, CHexarc::GetFilenameFromFilePath(sFilePath));

		//	Request a download. (We can do this synchronously because it
		//	doesn't take long and the call is thread-safe).

		m_Service.RequestExtensionDownload(sFilePath, sFilespec, FileDigest);
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
		case CGameSettings::soundVolume:
			ASSERT(iValue >= 0 && iValue <= 10);
			m_HI.SetSoundVolume(iValue);
			break;
		}

	m_Settings.SetInteger(iOption, iValue, bModifySettings);
	}
