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

#define CMD_GAME_ADVENTURE						CONSTLIT("gameAdventure")
#define CMD_GAME_CREATE							CONSTLIT("gameCreate")
#define CMD_GAME_END_DESTROYED					CONSTLIT("gameEndDestroyed")
#define CMD_GAME_END_SAVE						CONSTLIT("gameEndSave")
#define CMD_GAME_ENTER_FINAL_STARGATE			CONSTLIT("gameEnterFinalStargate")
#define CMD_GAME_LOAD							CONSTLIT("gameLoad")
#define CMD_GAME_LOAD_DONE						CONSTLIT("gameLoadDone")
#define CMD_GAME_SELECT_SAVE_FILE				CONSTLIT("gameSelectSaveFile")
#define CMD_GAME_START_EXISTING					CONSTLIT("gameStartExisting")
#define CMD_GAME_START_NEW						CONSTLIT("gameStartNew")

#define CMD_MODEL_ADVENTURE_INIT_DONE			CONSTLIT("modelAdventureInitDone")
#define CMD_MODEL_INIT_DONE						CONSTLIT("modelInitDone")
#define CMD_MODEL_NEW_GAME_CREATED				CONSTLIT("modelNewGameCreated")

#define CMD_SERVICE_ACCOUNT_CHANGED				CONSTLIT("serviceAccountChanged")
#define CMD_SERVICE_HOUSEKEEPING				CONSTLIT("serviceHousekeeping")

#define CMD_SESSION_STATS_DONE					CONSTLIT("sessionStatsDone")
#define CMD_SESSION_EPILOGUE_DONE				CONSTLIT("sessionEpilogueDone")

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

#define DEBUG_LOG_FILENAME						CONSTLIT("Debug.log")
#define SETTINGS_FILENAME						CONSTLIT("Settings.xml")

#define ERR_CANT_LOAD_GAME						CONSTLIT("Unable to load game")
#define ERR_CANT_START_GAME						CONSTLIT("Unable to start game")
#define ERR_CANT_SHOW_MOD_EXCHANGE				CONSTLIT("Unable to show Mod Exchange screen")
#define ERR_CONTACT_KP							CONSTLIT("Contact Kronosaur Productions")
#define ERR_LOAD_ERROR							CONSTLIT("Error loading extensions")
#define ERR_RESET_PASSWORD_DESC					CONSTLIT("Automated password reset is not yet implemented. Please contact Kronosaur Productions at:\n\ntranscendence@kronosaur.com\n\nPlease provide your username.")

const DWORD SERVICE_HOUSEKEEPING_INTERVAL =		1000 * 60; 

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

		kernelDebugLogMessage("Error loading %s: %s", SETTINGS_FILENAME.GetASCIIZPointer(), sError.GetASCIIZPointer());
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
		}

	//	New game started; adventure selected

	else if (strEquals(sCmd, CMD_GAME_ADVENTURE))
		{
		DWORD dwAdventure = (DWORD)pData;

		//	Get some information about the adventure before we kick off the 
		//	thread (we don't want to touch m_Universe while a background thread
		//	is accessing it).

		CString sAdventureName;
		CAdventureDesc *pAdventure = g_pUniverse->FindAdventureDesc(dwAdventure);
		if (pAdventure)
			{
			sAdventureName = pAdventure->GetName();
			}
		else
			{
			sAdventureName = CONSTLIT("Unknown Adventure");
			}

		//	Kick-off background thread to initialize the adventure

		m_HI.AddBackgroundTask(new CInitAdventureTask(m_HI, m_Model, dwAdventure), this, CMD_MODEL_ADVENTURE_INIT_DONE);

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
			m_HI.OpenPopupSession(new CMessageSession(m_HI, ERR_CANT_START_GAME, sError, CMD_UI_BACK_TO_INTRO));
			return NOERROR;
			}

		//	Select the ship

		SNewGameSettings Defaults;
		Defaults.sPlayerName = m_Model.GetDefaultPlayerName();
		Defaults.iPlayerGenome = m_Model.GetDefaultPlayerGenome();
		Defaults.dwPlayerShip = m_Model.GetDefaultPlayerShipClass();

		//	If the player name is NULL then we come up with a better idea

		if (Defaults.sPlayerName.IsBlank())
			Defaults.sPlayerName = m_Service.GetDefaultUsername();

		if (Defaults.sPlayerName.IsBlank())
			Defaults.sPlayerName = ::sysGetUserName();

		Defaults.sPlayerName = CUniverse::ValidatePlayerName(Defaults.sPlayerName);

		//	New game screen

		if (error = m_HI.ShowSession(new CNewGameSession(m_HI, m_Service, Defaults), &sError))
			{
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
		}

	//	New game

	else if (strEquals(sCmd, CMD_GAME_CREATE))
		{
		SNewGameSettings *pNewGame = (SNewGameSettings *)pData;

		//	Let the model begin the creation

		if (error = m_Model.StartNewGame(m_Service.GetUsername(), *pNewGame, &sError))
			{
			sError = strPatternSubst(CONSTLIT("Unable to begin new game: %s"), sError);
			kernelDebugLogMessage(sError.GetASCIIZPointer());

			m_HI.OpenPopupSession(new CMessageSession(m_HI, ERR_CANT_START_GAME, sError, CMD_UI_BACK_TO_INTRO));
			return NOERROR;
			}

		//	Report creation

		kernelDebugLogMessage("Created new game");

		//	Kick-off background thread to finish up creating the game

		g_pTrans->SetGameCreated(false);
		m_HI.AddBackgroundTask(new CStartNewGameTask(m_HI, m_Model), this, CMD_MODEL_NEW_GAME_CREATED);

		//	Start the prologue

		m_HI.ShowSession(new CLegacySession(m_HI));
		g_pTrans->StartProlog();
		}

	//	Background creation of game is done

	else if (strEquals(sCmd, CMD_MODEL_NEW_GAME_CREATED))
		{
		CStartNewGameTask *pTask = (CStartNewGameTask *)pData;

		//	Check for error

		if (pTask->GetResult(&sError))
			{
			m_Model.GetGameFile().Close();
			m_HI.OpenPopupSession(new CMessageSession(m_HI, ERR_CANT_START_GAME, sError, CMD_UI_BACK_TO_INTRO));
			return NOERROR;
			}

		g_pTrans->SetGameCreated();
		}

	//	Choose a save file to load

	else if (strEquals(sCmd, CMD_GAME_SELECT_SAVE_FILE))
		{
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
			HICommand(CMD_UI_START_EPILOGUE);
		}

	//	Save and exit

	else if (strEquals(sCmd, CMD_GAME_END_SAVE))
		{
		if (error = m_Model.EndGameSave(&sError))
			g_pTrans->DisplayMessage(sError);

		//	Back to intro screen

		m_HI.ShowSession(new CLegacySession(m_HI));
		g_pTrans->StartIntro(CTranscendenceWnd::isShipStats);
		m_iState = stateIntro;
		}

	//	Player has entered final stargate and the game is over

	else if (strEquals(sCmd, CMD_GAME_ENTER_FINAL_STARGATE))
		{
		m_Model.EndGameStargate();
		HICommand(CMD_UI_START_EPILOGUE);
		}

	//	Start the game

	else if (strEquals(sCmd, CMD_GAME_START_EXISTING))
		{
		m_Model.StartGame(false);
		m_iState = stateInGame;
		}
	else if (strEquals(sCmd, CMD_GAME_START_NEW))
		{
		m_Model.StartGame(true);
		m_iState = stateInGame;
		}

	//	Start epilogue

	else if (strEquals(sCmd, CMD_UI_START_EPILOGUE))
		{
		//	Play End March

		if (!GetOptionBoolean(CGameSettings::noMusic))
			m_HI.GetSoundMgr().PlayMusic(CONSTLIT("TranscendenceMarch.mp3"));

		//	If we have no crawl image, then go straight to intro

		CG16bitImage *pCrawlImage = g_pUniverse->GetLibraryBitmap(m_Model.GetCrawlImage());
		const CString &sCrawlText = m_Model.GetCrawlText();
		if (pCrawlImage == NULL)
			{
			m_HI.HICommand(CMD_SESSION_EPILOGUE_DONE);
			return NOERROR;
			}

		//	Otherwise, start epilog session

		m_HI.ShowSession(new CTextCrawlSession(m_HI, *pCrawlImage, sCrawlText, CMD_SESSION_EPILOGUE_DONE));
		m_iState = stateEpilogue;
		}

	//	Epilogue is done

	else if (strEquals(sCmd, CMD_SESSION_EPILOGUE_DONE))
		{
		//	Get stats

		CGameStats Stats;
		m_Model.GetGameStats(&Stats);

		//	Publish game record

		if (m_Service.HasCapability(ICIService::postGameRecord)
				&& g_pUniverse->IsStatsPostingEnabled())
			{
			m_HI.AddBackgroundTask(new CPostRecordTask(m_HI, m_Service, m_Model.GetGameRecord(), Stats));
			}

		//	Show stats

		if (Stats.GetCount() > 0)
			{
			DWORD dwFlags = 0;

			//	Post stats to service (in the background)
			//	The tasks takes a copy of the stats
			if (m_Service.HasCapability(ICIService::postGameStats) 
					&& g_pUniverse->IsStatsPostingEnabled())
				{
				m_HI.AddBackgroundTask(new CPostStatsTask(m_HI, m_Service, Stats));
				dwFlags |= CStatsSession::SHOW_TASK_PROGRESS;
				}

			//	The session takes handoff of the stats
			m_HI.ShowSession(new CStatsSession(m_HI, Stats, dwFlags));
			m_iState = stateEndGameStats;
			}
		else
			{
			m_HI.ShowSession(new CLegacySession(m_HI));
			g_pTrans->StartIntro(CTranscendenceWnd::isEndGame);
			m_iState = stateIntro;
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

		//	Get the current node

		CTopologyNode *pNode = g_pUniverse->GetCurrentTopologyNode();
		if (pNode == NULL || pNode->GetDisplayPos() == NULL)
			return NOERROR;

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

		if (!m_Service.HasCapability(ICIService::getUserProfile))
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
			CProfileSession *pSession = new CProfileSession(m_HI, m_Service);
			if (error = m_HI.OpenPopupSession(pSession))
				return error;
			}
		}

	//	Mod exchange

	else if (strEquals(sCmd, CMD_UI_SHOW_MOD_EXCHANGE))
		{
		if (error = m_HI.OpenPopupSession(new CModExchangeSession(m_HI, m_Service)))
			{
			m_HI.OpenPopupSession(new CMessageSession(m_HI, ERR_CANT_SHOW_MOD_EXCHANGE, NULL_STR, CMD_NULL));
			return NOERROR;
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
		g_pTrans->OnAccountChanged();
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

	//	Exit

	else if (strEquals(sCmd, CMD_UI_EXIT))
		{
		//	LATER: This should be an m_HI method
		::SendMessage(m_HI.GetHWND(), WM_CLOSE, 0, 0);
		}

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

	//	Add a timer so that services can do some background processing.

	m_HI.AddTimer(SERVICE_HOUSEKEEPING_INTERVAL, this, CMD_SERVICE_HOUSEKEEPING);

	//	Show the loading screen

	m_HI.ShowSession(new CLoadingSession(m_HI, m_Model.GetCopyright()));
	m_iState = stateLoading;

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
			if (bValue)
				m_HI.GetSoundMgr().StopMusic();
			else
				m_HI.GetSoundMgr().PlayMusic(CONSTLIT("TranscendenceMarch.mp3"));
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

