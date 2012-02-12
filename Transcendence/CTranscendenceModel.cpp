//	CTranscendenceModel.cpp
//
//	CTranscendenceModel class
//
//	DOCK SCREENS
//
//	To enter screen mode, call one of the following:
//
//		OnPlayerDocked (automatically called after CPlayerShipController::Dock
//			is called)
//		ShowShipScreen (with no parameters to bring up internal ship screen)
//		ShowShipScreen (with appropriate parameters, if using an item, e.g.)
//
//	Once in screen mode, you may call:
//
//		ShowScreen (to switch screens)
//		ShowPane (to switch screens)
//		ExitScreenSession (to exit a session)
//
//	The functions to enter screen mode must not be called if already
//	in screen mode. [And the screen mode functions must not be called
//	if not already in screen mode.]
//
//	PLAYER DOCK
//
//	CTranscendenceWnd::WMKeyDown
//		CPlayerShipController::Dock
//			Figure out station to dock with
//			Ask station (RequestDock)
//			CPlayerShipController::m_pStation set
//
//	...wait for dock...
//
//	CPlayerShipController::OnDocked
//		CTranscendenceModel::OnPlayerDocked
//			Call FireOnDockObjAdj (to change m_pDock)
//			m_pDock->GetDockScreen
//			Set default screens root to m_pDock type
//			CTranscendenceModel::EnterScreenSession
//				SetPlayerDocked
//				ResetFirstOnInit
//				CTranscendenceModel::ShowScreen
//					Set info for frame
//					m_CurrentDock.InitScreen
//				CTranscendenceWnd::ShowDockScreen(true)
//
//	PLAYER SHIP SCREEN
//
//	CTranscendenceWnd::WMKeyDown
//		CTranscendenceModel::ShowShipScreen()
//			Get screen from CPlayerSettings
//			CTranscendenceModel::ShowShipScreen(screen)
//				Set default screens root to ship class
//				CTranscendenceModel::EnterScreenSession
//					SetPlayerDocked
//					ResetFirstOnInit
//					CTranscendenceModel::ShowScreen
//						Set info for frame
//						m_CurrentDock.InitScreen
//					CTranscendenceWnd::ShowDockScreen(true)
//
//	PLAYER ENTERS A STARGATE
//
//	CTranscendenceWnd::WMKeyDown
//	CPlayerShipController::Gate
//	CStation::RequestGate
//		Create gate effect
//	CSpaceObject::EnterGate
//		Calls to OnObjEnteredGate for all objs in system
//	CShip::GateHook
//		FireOnEnteredGate
//	CPlayerShipController::OnEnterGate
//		CTranscendenceModel::OnPlayerEnteredGate
//			FireOnGlobalPlayerLeftSystem
//			TransferObjEventsOut
//			Player ship removed from system
//			Gate followers inserted in list
//			gPlayerShip set to Nil
//			CTranscendenceWnd::PlayerEnteredGate
//				state set to gsEnteringStargate
//	
//	...wait for timer...
//
//	CTranscendenceWnd::EnterStargate
//		CTranscendenceModel::OnPlayerTraveledThroughGate
//			Create or load system
//			UpdateExtended
//			TransferGateFollowers
//			Set POV to new system
//
//	...wait for timer...
//
//	CTranscendenceWnd::LeaveStargate
//		CTranscendenceModel::OnPlayerExitedGate
//			Player added to new system
//			gPlayerShip defined
//			"Welcome to the xyz system"
//			FireOnGlobalPlayerEnteredSystem
//			CSystem::PlayerEntered
//			CPlayerShipController::OnSystemEntered
//			System saved

#include "PreComp.h"
#include "Transcendence.h"

#define HIGH_SCORES_FILENAME				CONSTLIT("HighScores.xml")

#define STR_G_PLAYER						CONSTLIT("gPlayer")
#define STR_G_PLAYER_SHIP					CONSTLIT("gPlayerShip")

#define NESTED_SCREEN_ATTRIB				CONSTLIT("nestedScreen")

#define DEFAULT_SCREEN_NAME					CONSTLIT("[DefaultScreen]")

CTranscendenceModel::CTranscendenceModel (CHumanInterface &HI) : 
		m_HI(HI), 
		m_iState(stateUnknown),
		m_bDebugMode(false),
		m_bForceTDB(false),
		m_bNoSound(false),
		m_pPlayer(NULL),
		m_pResurrectType(NULL),
		m_dwCrawlImage(0),
		m_iLastHighScore(-1),
		m_iPlayerGenome(genomeUnknown),
		m_dwAdventure(INVALID_UNID),
		m_dwPlayerShip(INVALID_UNID),
		m_pDock(NULL),
		m_pDestNode(NULL)

//	CTranscendenceModel constructor

	{
	}

int CTranscendenceModel::AddHighScore (const CGameRecord &Score)

//	AddHighScore
//
//	Adds a high score record

	{
	int iPos = m_HighScoreList.AddEntry(Score);
	SaveHighScoreList();
	return iPos;
	}

DWORD CTranscendenceModel::CalcDefaultPlayerShipClass (void)

//	CalcDefaultPlayerShipClass
//
//	Returns the default player ship class

	{
	//	Return the freighter (if it exists)

	if (m_Universe.FindShipClass(PLAYER_EI500_FREIGHTER_UNID))
		return PLAYER_EI500_FREIGHTER_UNID;

	//	Otherwise, return the first ship available to the player

	else
		{
		int i;

		//	Returns the first ship available to the player

		for (i = 0; i < m_Universe.GetShipClassCount(); i++)
			{
			CShipClass *pClass = m_Universe.GetShipClass(i);
			if (pClass->IsShownAtNewGame())
				return pClass->GetUNID();
			}

		ASSERT(false);
		return 0;
		}
	}

CString CTranscendenceModel::CalcEpitaph (SDestroyCtx &Ctx)

//	CalcEpitaph
//
//	Generates an epitaph based on destruction cause

	{
	CShip *pShip = m_pPlayer->GetShip();
	ASSERT(Ctx.pObj == (CSpaceObject *)pShip);
	ASSERT(pShip->GetSystem());
	CString sSystemName = pShip->GetSystem()->GetName();

	//	Figure out who killed us

	CString sCause;
	CSpaceObject *pAttacker = Ctx.Attacker.GetObj();
	if (pAttacker)
		{
		if (pAttacker->IsAngryAt(pShip) || pAttacker->GetSovereign() == NULL)
			sCause = strPatternSubst(CONSTLIT("by %s"), Ctx.Attacker.GetDamageCauseNounPhrase(nounArticle));
		else
			{
			if (pAttacker == pShip)
				sCause = g_pTrans->ComposePlayerNameString(CONSTLIT("accidentally by %his% own weapon"));
			else
				sCause = strPatternSubst(CONSTLIT("accidentally by %s"), Ctx.Attacker.GetDamageCauseNounPhrase(nounArticle));
			}
		}
	else if (Ctx.iCause == killedByOther)
		sCause = Ctx.Attacker.GetDamageCauseNounPhrase(0);
	else if (Ctx.Attacker.HasDamageCause())
		sCause = strPatternSubst(CONSTLIT("by %s"), Ctx.Attacker.GetDamageCauseNounPhrase(nounArticle));
	else
		sCause = CONSTLIT("by unknown forces");

	//	Generat based on cause

	CString sText;
	switch (Ctx.iCause)
		{
		case killedByDamage:
			sText = strPatternSubst(CONSTLIT("was destroyed %s"), sCause);
			break;

		case killedByDisintegration:
			sText = strPatternSubst(CONSTLIT("was disintegrated %s"), sCause);
			break;

		case killedByRunningOutOfFuel:
			sText = CONSTLIT("ran out of fuel");
			break;

		case killedByRadiationPoisoning:
			sText = CONSTLIT("was killed by radiation poisoning");
			break;

		case killedBySelf:
			sText = CONSTLIT("self-destructed");
			break;

		case killedByWeaponMalfunction:
			sText = CONSTLIT("was destroyed by a malfunctioning weapon");
			break;

		case killedByEjecta:
			if (Ctx.Attacker.HasDamageCause())
				sText = strPatternSubst(CONSTLIT("was destroyed by exploding debris from %s"), Ctx.Attacker.GetDamageCauseNounPhrase(nounArticle));
			else
				sText = CONSTLIT("was destroyed by exploding debris");
			break;

		case killedByExplosion:
		case killedByPlayerCreatedExplosion:
			if (Ctx.Attacker.HasDamageCause())
				sText = strPatternSubst(CONSTLIT("was destroyed by the explosion of %s"), Ctx.Attacker.GetDamageCauseNounPhrase(nounArticle));
			else
				sText = CONSTLIT("was destroyed by exploding debris");
			break;

		case killedByShatter:
			sText = strPatternSubst(CONSTLIT("was shattered %s"), sCause);
			break;

		case killedByOther:
			sText = sCause;
			break;

		default:
			sText = CONSTLIT("was destroyed by unknown forces");
			break;
		}

	//	Append the location of death

	//	Check to see if the epitaph is of the pattern "abc for xyz"
	//	If it is, convert from "abc for xyz" to "abc in the blah system for xyz"

	int iFound = strFind(sText, CONSTLIT(" for "));
	if (iFound != -1)
		sText = strPatternSubst(CONSTLIT("%s in the %s System for %s"),
				strSubString(sText, 0, iFound),
				sSystemName,
				strSubString(sText, iFound + 5, -1));

	//	Otherwise, just add " in the blah system" at the end

	else
		sText.Append(strPatternSubst(CONSTLIT(" in the %s System"), sSystemName));

	//	Done

	return sText;
	}

void CTranscendenceModel::CalcStartingPos (CShipClass *pStartingShip, CString *retsNodeID, CString *retsPos)

//	CalcStartingPos
//
//	Calculates the starting position of the player in a new game

	{
	//	See if the player ship specifies it.

	const CPlayerSettings *pPlayerSettings = pStartingShip->GetPlayerSettings();
	CString sNodeID = pPlayerSettings->GetStartNode();
	CString sPos = pPlayerSettings->GetStartPos();

	//	If not, see if the adventure does

	if (sNodeID.IsBlank() || sPos.IsBlank())
		{
		CAdventureDesc *pAdventure = m_Universe.GetCurrentAdventureDesc();
		sNodeID = pAdventure->GetStartingNodeID();
		sPos = pAdventure->GetStartingPos();
		}

	//	If not, come up with a reasonable default

	if (sNodeID.IsBlank() || sPos.IsBlank())
		{
		sNodeID = m_Universe.GetDesignCollection().GetTopologyDesc()->GetFirstNodeID();
		CTopologyNode *pNode = m_Universe.FindTopologyNode(sNodeID);
		if (pNode && pNode->GetStargateCount() > 0)
			sPos = pNode->GetStargate(0);
		}

	//	Done

	if (retsNodeID)
		*retsNodeID = sNodeID;

	if (retsPos)
		*retsPos = sPos;
	}

void CTranscendenceModel::CleanUp (void)

//	CleanUp
//
//	Clean up the model

	{
	}

ALERROR CTranscendenceModel::EndGameClose (CString *retsError)

//	EndGameClose
//
//	Player has closed the app and we need to save

	{
	switch (m_iState)
		{
		case stateInGame:
			{
			int iMaxLoops = 100;

			//	If we have dock screens up, keep hitting the cancel action
			//	until we're done.

			while (m_DockFrames.GetCount() > 0 && iMaxLoops-- > 0)
				g_pTrans->m_CurrentDock.ExecuteCancelAction();

			if (m_DockFrames.GetCount() > 0)
				ExitScreenSession(true);

			//	If we're dead after this, then recurse

			if (m_iState != stateInGame)
				return EndGameClose(retsError);

			//	Otherwise, save the game

			return EndGameSave(retsError);
			}

		case statePlayerInResurrect:
			EndGameDestroyed();
			return EndGameSave(retsError);

		case statePlayerDestroyed:
			return EndGameDestroyed();

		//	LATER: Handle the case were we close while in a stargate
		//	or while resurrecting.
		}

	return NOERROR;
	}

ALERROR CTranscendenceModel::EndGameDestroyed (bool *retbResurrected)

//	EndGameDestroyed
//
//	Ends the game after ship has been destroyed (or resurrects the ship)

	{
	ALERROR error;

	//	See if we are resurrecting

	if (m_iState == statePlayerInResurrect)
		{
		//	Call the player controller. This will place the player
		//	ship back in the system and empty out any wrecks.

		m_pPlayer->InsuranceClaim();

		//	Call our resurrector

		if (m_pResurrectType)
			{
			CString sError;
			if (m_pResurrectType->FireOnGlobalResurrect(&sError) != NOERROR)
				{
				g_pTrans->DisplayMessage(sError);
				kernelDebugLogMessage(sError.GetASCIIZPointer());
				}
			}

		//	Continue

		m_Universe.SetPOV(m_pPlayer->GetShip());

		//	Save the game (checkpoint means that we increase resurrect
		//	count if we load the game).

		SaveGame(CGameFile::FLAG_CHECKPOINT);

		//	Done

		m_iState = stateInGame;

		if (retbResurrected)
			*retbResurrected = true;
		}

	//	If we are destroyed

	else if (m_iState == statePlayerDestroyed)
		{
		//	Generate stats and save to file

		if (error = SaveGameStats(true))
			return error;

		//	Clean up (deleting the ship also deletes its controller)

		ASSERT(m_pPlayer->GetShip()->GetSystem() == NULL);
		delete m_pPlayer->GetShip();
		m_pPlayer = NULL;

		//	Close the game file

		m_GameFile.Close();
		m_Universe.Reinit();

		//	Done

		m_iState = stateGameOver;

		if (retbResurrected)
			*retbResurrected = false;
		}
	else
		ASSERT(false);

	return NOERROR;
	}

ALERROR CTranscendenceModel::EndGameSave (CString *retsError)

//	EndGameSave
//
//	Saves and ends the current game

	{
	ALERROR error;

	ASSERT(m_iState == stateInGame);

	//	Save the game

	if (error = SaveGame(0, retsError))
		return error;

	//	Clean up

	m_pPlayer = NULL;

	//	Close the game file

	m_GameFile.Close();
	m_Universe.Reinit();

	//	Done

	m_iState = stateGameOver;

	return NOERROR;
	}

ALERROR CTranscendenceModel::EndGameStargate (void)

//	EndGameStargate
//
//	Ends game when player enters final stargate

	{
	ALERROR error;

	ASSERT(m_iState == statePlayerInGateOldSystem);

	//	Bonus for escaping Human Space

	m_pPlayer->IncScore(10000);

	//	Game over (compile stats)

	m_pPlayer->OnGameEnd();

	//	Remember the epitaph

	m_sEpitaph = m_pDestNode->GetEpitaph();

	//	Record final score

	RecordFinalScore(m_sEpitaph, m_pDestNode->GetEndGameReason(), false);

	//	We need to define gPlayerShip so that game stats 
	//	are generated properly. (It gets cleared out in CTranscendenceWnd::PlayerEnteredGate)

	CCodeChain &CC = g_pUniverse->GetCC();
	CC.DefineGlobal(CONSTLIT("gPlayerShip"), CC.CreateInteger((int)m_pPlayer->GetShip()));

	//	Generate stats and save to file

	if (error = SaveGameStats(true))
		return error;

	//	Clean up

	m_pPlayer = NULL;

	//	Close the game file

	m_GameFile.Close();
	m_Universe.Reinit();

	//	Done

	m_iState = stateGameOver;

	return NOERROR;
	}

ALERROR CTranscendenceModel::EnterScreenSession (CSpaceObject *pLocation, CDesignType *pRoot, const CString &sScreen, const CString &sPane)

//	EnterScreenSession
//
//	Begins a new frame on the screen stack. If we are not yet in dock mode
//	we switch to dock mode.
//
//	If pRoot is NULL then we attempt to resolve it using sScreen
//	and m_pDefaultScreensRoot

	{
	ALERROR error;
	ASSERT(pLocation);

	bool bFirstFrame = (m_DockFrames.GetCount() == 0);

	//	Mark the object so that it knows that the player is docked with it.
	//	We need this so that the object can tell us if its items change.

	bool bOldPlayerDocked = pLocation->IsPlayerDocked();
	pLocation->SetPlayerDocked();

	//	If this is our first frame, then this is the first OnInit

	if (bFirstFrame)
		g_pTrans->m_CurrentDock.ResetFirstOnInit();

	//	Add a new frame.
	//	Note that pRoot might be NULL and sScreen might be [DefaultScreen] at
	//	this point.

	SDockFrame *pFrame = m_DockFrames.Insert();
	pFrame->pLocation = pLocation;
	pFrame->pRoot = pRoot;
	pFrame->sScreen = sScreen;
	pFrame->sPane = sPane;

	//	From this point forward we are considered in a screen session.
	//	[We use this to determine whether a call to scrShowScreen switches
	//	screens or enters a new session.]
	//
	//	Initialize the current screen object

	if (error = ShowScreen(pRoot, sScreen, sPane))
		{
		//	Undo

		m_DockFrames.Delete(m_DockFrames.GetCount() - 1);

		if (!bOldPlayerDocked)
			pLocation->ClearPlayerDocked();

		return error;
		}

	//	If we don't have any frames then we called exit from inside of ShowScreen

	if (m_DockFrames.GetCount() == 0)
		return NOERROR;

	//	Switch state

	if (bFirstFrame)
		g_pTrans->ShowDockScreen(true);

	return NOERROR;
	}

void CTranscendenceModel::ExitScreenSession (bool bForceUndock)

//	ExitScreenSession
//
//	Exits docking

	{
	ASSERT(m_DockFrames.GetCount() > 0);

	//	If we have another frame, then switch back to that screen

	if (m_DockFrames.GetCount() > 1 && !bForceUndock)
		{
		//	Remove the current frame

		m_DockFrames.Delete(m_DockFrames.GetCount() - 1);

		//	Switch
		//	bReturn = true (on ShowScreen) so that we don't restack a
		//	nested screen that we are returning to.

		SDockFrame *pFrame = &m_DockFrames[m_DockFrames.GetCount() - 1];
		ShowScreen(pFrame->pRoot, pFrame->sScreen, pFrame->sPane, true);
		}

	//	Otherwise, exit

	else
		{
		//	Unmark the object

		m_DockFrames[0].pLocation->ClearPlayerDocked();

		//	If we're docked with something (came from OnPlayerDocked)
		//	then we need to undock

		if (m_pDock)
			{
			m_pPlayer->Undock();
			m_pDock = NULL;
			}

		//	Clean up

		m_pDefaultScreensRoot = NULL;
		g_pTrans->ShowDockScreen(false);
		g_pTrans->m_CurrentDock.CleanUpScreen();

		m_DockFrames.DeleteAll();
		}
	}

bool CTranscendenceModel::FindScreenRoot (const CString &sScreen, CDesignType **retpRoot, CString *retsScreen)

//	FindScreenRoot
//
//	Finds the appropriate root given a string name

	{
	//	If the screen is an UNID, then expect a stand-alone screen

	bool bNotANumber;
	DWORD dwUNID = (DWORD)strToInt(sScreen, 0, &bNotANumber);
	if (!bNotANumber)
		{
		CDesignType *pRoot = g_pUniverse->FindSharedDockScreen(dwUNID);
		if (pRoot == NULL)
			return false;

		if (retpRoot)
			*retpRoot = pRoot;

		if (retsScreen)
			*retsScreen = sScreen;

		return true;
		}

	//	Otherwise, if this is the special name [DefaultScreen] then
	//	we ask the object for the proper screen

	else if (strEquals(sScreen, DEFAULT_SCREEN_NAME))
		{
		CString sScreenActual;

		ASSERT(m_DockFrames.GetCount() > 0);
		SDockFrame *pFrame = &m_DockFrames[m_DockFrames.GetCount() - 1];
		ASSERT(pFrame->pLocation);

		CDesignType *pRoot = pFrame->pLocation->GetFirstDockScreen(&sScreenActual);
		if (pRoot == NULL)
			return false;

		if (retpRoot)
			*retpRoot = pRoot;

		if (retsScreen)
			*retsScreen = sScreenActual;

		return true;
		}

	//	Otherwise, this is a local screen, so we use the default root

	else if (m_pDefaultScreensRoot)
		{
		if (retpRoot)
			*retpRoot = m_pDefaultScreensRoot;

		if (retsScreen)
			*retsScreen = sScreen;

		return true;
		}

	//	Otherwise, use the player ship

	else if (m_pPlayer->GetShip())
		{
		if (retpRoot)
			*retpRoot = m_pPlayer->GetShip()->GetType();

		if (retsScreen)
			*retsScreen = sScreen;

		return true;
		}

	//	Otherwise, we could not find the root

	else
		return false;
	}

void CTranscendenceModel::GenerateGameStats (CGameStats *retStats, bool bGameOver)

//	GenerateGameStats
//
//	Generates current game stats

	{
	retStats->DeleteAll();

	CShip *pPlayerShip;
	if (m_pPlayer == NULL || (pPlayerShip = m_pPlayer->GetShip()) == NULL)
		return;

	retStats->SetDefaultSectionName(m_pPlayer->GetPlayerName());

	//	If the player isn't set, then we need to temporarily set it
	//	so that the calls inside of GenerateGameStats can work properly

	bool bCleanUp = false;
	if (m_Universe.GetPlayer() == NULL)
		{
		ASSERT(pPlayerShip);
		m_Universe.SetPlayer(pPlayerShip);
		bCleanUp = true;
		}

	//	Add the game version

	retStats->Insert(CONSTLIT("Version"), m_Version.sProductVersion);

	//	Some stats we only add at the end of the game

	if (bGameOver)
		{
		//	Add the epitaph, if necessary

		if (!m_sEpitaph.IsBlank())
			{
			CString sFate;
			if (strEquals(strWord(m_sEpitaph, 0), CONSTLIT("was")))
				sFate = strSubString(m_sEpitaph, 4, -1);
			else
				sFate = m_sEpitaph;

			retStats->Insert(CONSTLIT("Fate"), ::strCapitalize(sFate));

			//	We no longer need the epitaph after this, since it is
			//	in the game stats, so we clear it so that later games
			//	don't get confused.

			m_sEpitaph = NULL_STR;
			}
		}

	//	Generate

	m_Universe.GenerateGameStats(*retStats);
	m_pPlayer->GenerateGameStats(*retStats, bGameOver);
	retStats->Sort();

	//	Clean up

	if (bCleanUp)
		m_Universe.SetPlayer(NULL);
	}

ALERROR CTranscendenceModel::GetGameStats (CGameStats *retStats)

//	GetGameStats
//
//	Gets the game stats for the current player

	{
	//	If we're in the middle of a game, then generate the stats now

	if (m_iState == stateInGame)
		{
		GenerateGameStats(retStats);
		}

	//	Otherwise, we can't get them

	else
		return ERR_FAIL;

	return NOERROR;
	}

void CTranscendenceModel::GetScreenSession (CSpaceObject **retpObj, CDesignType **retpRoot, CString *retsScreen, CString *retsPane)

//	GetScreenSession
//
//	Returns info about the current screen session

	{
	ASSERT(m_DockFrames.GetCount() > 0);
	SDockFrame *pFrame = &m_DockFrames[m_DockFrames.GetCount() - 1];

	if (retpObj)
		*retpObj = pFrame->pLocation;

	if (retpRoot)
		*retpRoot = pFrame->pRoot;

	if (retsScreen)
		*retsScreen = pFrame->sScreen;

	if (retsPane)
		*retsPane = pFrame->sPane;
	}

ALERROR CTranscendenceModel::Init (void)

//	Init
//
//	Initialize

	{
	::fileGetVersionInfo(NULL_STR, &m_Version);

	return NOERROR;
	}

ALERROR CTranscendenceModel::InitBackground (CString *retsError)

//	InitBackground
//
//	Initializes the model, including loading the universe. This function
//	is designed to be called in a background thread.

	{
	ALERROR error;

	//	Load the universe

	if (error = LoadUniverse(retsError))
		return error;

	//	Load the high scores list

	if (error = LoadHighScoreList(retsError))
		return error;

	//	Load player defaults

	if (error = LoadPlayerDefaults(retsError))
		return error;

	return NOERROR;
	}

ALERROR CTranscendenceModel::InitAdventure (DWORD dwAdventure, CString *retsError)

//	InitAdventure
//
//	Initializes the adventure in preparation for creating a new game.

	{
	return m_Universe.InitAdventure(dwAdventure, NULL, retsError);
	}

bool CTranscendenceModel::IsGalacticMapAvailable (CString *retsError)

//	IsGalacticMapAvailable
//
//	Returns TRUE if we can show a galactic map

	{
	//	Make sure we have a player ship

	if (m_pPlayer == NULL)
		return false;

	//	If not installed, then it is not available

	if (!m_pPlayer->IsGalacticMapAvailable())
		{
		if (retsError)
			*retsError = CONSTLIT("No galactic map installed");

		return false;
		}

	//	Installed

	return true;
	}

ALERROR CTranscendenceModel::LoadHighScoreList (CString *retsError)

//	LoadHighScoreList
//
//	Loads the high score list

	{
	ALERROR error;

	if (error = m_HighScoreList.Load(HIGH_SCORES_FILENAME))
		{
		if (retsError)
			*retsError = strPatternSubst(CONSTLIT("Unable to open high scores file: %s"), HIGH_SCORES_FILENAME);
		return error;
		}

	return NOERROR;
	}

ALERROR CTranscendenceModel::LoadGame (const CString &sFilespec, CString *retsError)

//	LoadGame
//
//	Load a previously saved game and keeps the game file open

	{
	ALERROR error;

	//	Old game

	if (error = m_GameFile.Open(sFilespec))
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to open save file: %s"), sFilespec);
		return error;
		}

	//	Load the universe

	DWORD dwSystemID, dwPlayerID;
	if (error = m_GameFile.LoadUniverse(*g_pUniverse, &dwSystemID, &dwPlayerID, retsError))
		{
		m_GameFile.Close();
		return error;
		}

	//	Load the POV system

	CSystem *pSystem;
	CShip *pPlayerShip;
	if (error = m_GameFile.LoadSystem(dwSystemID, 
			&pSystem, 
			dwPlayerID, 
			(CSpaceObject **)&pPlayerShip))
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to load system"));
		m_GameFile.Close();
		return error;
		}

	//	Set the player ship

	ASSERT(m_pPlayer == NULL);
	m_pPlayer = dynamic_cast<CPlayerShipController *>(pPlayerShip->GetController());
	m_pPlayer->SetTrans(g_pTrans);

	//	If we didn't save the player name then it means that we have an older
	//	version.

	if (m_pPlayer->GetPlayerName().IsBlank())
		m_pPlayer->SetName(m_GameFile.GetPlayerName());

	//	We only need to do this for backwards compatibility (pre 0.97 this flag
	//	was not set)

	pPlayerShip->TrackMass();

	//	Set the resurrect flag (this will be cleared if we save the game
	//	properly later)

	if (error = m_GameFile.SetGameResurrect())
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to set resurrect flag"));
		m_pPlayer = NULL;
		m_GameFile.Close();
		return error;
		}

	//	Set the resurrect count

	m_pPlayer->SetResurrectCount(m_GameFile.GetResurrectCount());

	//	Set debug mode appropriately

	m_bDebugMode = m_Universe.InDebugMode();

	//	Log that we loaded a game

	kernelDebugLogMessage("Loaded game file version: %x", m_GameFile.GetCreateVersion());

	return NOERROR;
	}

ALERROR CTranscendenceModel::LoadGameStats (const CString &sFilespec, CGameStats *retStats)

//	LoadGameStats
//
//	Loads game stats from the given game file

	{
	ALERROR error;

	//	Open the game file

	CGameFile GameFile;
	if (error = GameFile.Open(sFilespec))
		return error;

	//	Load the stats

	if (error = GameFile.LoadGameStats(retStats))
		return error;

	//	Done

	GameFile.Close();
	return NOERROR;
	}

ALERROR CTranscendenceModel::LoadPlayerDefaults (CString *retsError)

//	LoadPlayerDefaults
//
//	Loads the defaults for player name, etc.
//	This can only be called after the universe and high-score lists are loaded

	{
	//	Load default player name

	CString sPlayerName = m_HighScoreList.GetMostRecentPlayerName();
	if (sPlayerName.IsBlank())
		sPlayerName = sysGetUserName();
	SetPlayerName(sPlayerName);

	//	Default genome

	int iPlayerGenome = m_HighScoreList.GetMostRecentPlayerGenome();
	if (iPlayerGenome == genomeUnknown)
		iPlayerGenome = (mathRandom(1, 2) == 2 ? genomeHumanMale : genomeHumanFemale);
	SetPlayerGenome((GenomeTypes)iPlayerGenome);

	//	Default adventure

	m_dwAdventure = 0;

	//	Default ship class

	m_dwPlayerShip = CalcDefaultPlayerShipClass();
	if (m_dwPlayerShip == 0)
		{
		if (retsError)
			*retsError = CONSTLIT("No valid player ship class");
		return ERR_FAIL;
		}

	return NOERROR;
	}

ALERROR CTranscendenceModel::LoadUniverse (CString *retsError)

//	LoadUniverse
//
//	Loads the universe

	{
	ALERROR error;

	//	Make sure the universe know about our various managers

	m_Universe.SetDebugMode(m_bDebugMode);
	m_Universe.SetSoundMgr(&m_HI.GetSoundMgr());
	m_Universe.SetHost(g_pTrans);

	//	Figure out what to load. If no extension is supplied, we check for an XML
	//	file first. Otherwise, we load the .tdb

	CString sGameFile;
	if (m_bForceTDB)
		sGameFile = CONSTLIT("Transcendence.tdb");
	else
		sGameFile = CONSTLIT("Transcendence");

	//	Load the Transcendence Data Definition file that describes the universe.

	if (error = m_Universe.Init(TransPath(sGameFile), retsError))
		return error;

	//	Initialize TSUI CodeChain primitives

	if (error = m_HI.InitCodeChainPrimitives(m_Universe.GetCC()))
		return error;

	//	Initialize Transcendence application primitives

	if (error = InitCodeChainExtensions(m_Universe.GetCC()))
		return error;

	return NOERROR;
	}

void CTranscendenceModel::MarkGateFollowers (CSystem *pSystem)

//	MarkGateFollowers
//
//	Makes a list of all objects that should follow the player

	{
	int i;

	CShip *pShip = m_pPlayer->GetShip();

	//	Make a list of all our henchmen. When ships get a notification that
	//	the player has entered a gate, any followers of the player will get
	//	an order to follow the player through the gate. This loop finds all the
	//	ships that have that order.

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj 
				&& pObj->FollowsObjThroughGate(pShip)
				&& pObj != pShip
				&& !pObj->IsDestroyed())
			m_GateFollowers.Insert(pObj->GetID());
		}
	}

void CTranscendenceModel::OnDockedObjChanged (CSpaceObject *pObj)

//	OnDockedObjChanged
//
//	An object that was marked with SetPlayerDocked has changed

	{
	g_pTrans->m_CurrentDock.ResetList(pObj);
	}

void CTranscendenceModel::OnPlayerDestroyed (SDestroyCtx &Ctx, CString *retsEpitaph)

//	OnPlayerDestroyed
//
//	Called from inside the ship's OnDestroyed

	{
	//	Undock, if necessary

	if (InScreenSession())
		ExitScreenSession(true);

	//	See if are going to be resurrected

	if (!m_Universe.IsGlobalResurrectPending(&m_pResurrectType))
		m_pResurrectType = NULL;

	Ctx.bResurrectPending = (m_pResurrectType != NULL);

	//	Generate epitaph

	CString sText = CalcEpitaph(Ctx);
	if (retsEpitaph)
		*retsEpitaph = sText;

	//	If we're not resurrecting, then end game

	if (!Ctx.bResurrectPending)
		{
		//	Game over (compile stats)

		m_pPlayer->OnGameEnd();

		//	Record final score

		RecordFinalScore(sText, CONSTLIT("destroyed"), false);

		//	Remember the epitaph for later (we only need it until we record game stats)

		m_sEpitaph = sText;

		//	Done

		m_iState = statePlayerDestroyed;
		}
	else
		m_iState = statePlayerInResurrect;
	}

void CTranscendenceModel::OnPlayerDocked (CSpaceObject *pObj)

//	OnPlayerDocked
//
//	Player has docked with the given object.

	{
	g_pTrans->ClearMessage();

	//	See if the object wants to redirect us. In that case, m_pStation
	//	will remain the original object, but gSource and m_pLocation in the
	//	dock screen will be set to the new object

	if (!pObj->FireOnDockObjAdj(&m_pDock))
		m_pDock = pObj;

	//	Remember the default screens root.
	//	Note that this can be different from the root obtained above (from
	//	GetDockScreen) because GetDockScreen sometimes returns a top-level
	//	dock screen (in those cases, the default screens root is still
	//	the original object type).

	m_pDefaultScreensRoot = m_pDock->GetType();

	//	Show screen

	if (EnterScreenSession(m_pDock, NULL, DEFAULT_SCREEN_NAME, CString()) != NOERROR)
		{
		m_pPlayer->Undock();
		m_pDock = NULL;

		CString sError = strPatternSubst(CONSTLIT("[%s]: Unable to show dock screen"), m_pDock->GetType()->GetTypeName());
		g_pTrans->DisplayMessage(sError);
		::kernelDebugLogMessage(sError.GetASCIIZPointer());
		return;
		}
	}

void CTranscendenceModel::OnPlayerEnteredGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate)

//	OnPlayerEnteredGate
//
//	This is called the instant that the player enters a stargate.
//	(But the current system will continue to update after the player disappears)

	{
	ASSERT(m_iState == stateInGame);

	//	Remember some state

	CShip *pShip = m_pPlayer->GetShip();
	m_pOldSystem = pShip->GetSystem();
	m_pDestNode = pDestNode;
	m_sDestEntryPoint = sDestEntryPoint;

	//	Update stats

	m_pPlayer->OnSystemLeft(m_pOldSystem);

	//	Fire global event

	m_Universe.FireOnGlobalPlayerLeftSystem();

	//	Preserve any timer events

	m_pOldSystem->TransferObjEventsOut(pShip, m_TimerEvents);

	//	Create a marker that will hold our place for the POV (the marker
	//	object will be destroyed when the POV changes)

	CPOVMarker *pMarker;
	CPOVMarker::Create(m_pOldSystem, pShip->GetPos(), NullVector, &pMarker);
	m_Universe.SetPOV(pMarker);

	//	Remove the ship from the system
	//	(Note: this will remove us from the system. There are cases
	//	where we will lose our escorts when we leave the system, so we don't
	//	ask about henchmen until after we gate).

	pShip->Remove(enteredStargate, CDamageSource(pStargate, enteredStargate));

	//	Make a list of all our henchmen. When ships get a notification that
	//	the player has entered a gate, any followers of the player will get
	//	an order to follow the player through the gate. This loop finds all the
	//	ships that have that order.

	MarkGateFollowers(m_pOldSystem);

	//	Clear out these globals so that events don't try to send us
	//	orders (Otherwise, an event could set a target for the player. If the
	//	target is destroyed while we are out of the system, we will
	//	never get an OnObjDestroyed message).
	//	Note: We need gPlayer for OnGameEnd event

	CCodeChain &CC = m_Universe.GetCC();
	CC.DefineGlobal(STR_G_PLAYER_SHIP, CC.CreateNil());

	//	Tell the display

	g_pTrans->PlayerEnteredGate(m_pOldSystem, m_pDestNode, m_sDestEntryPoint);

	//	Done

	m_iState = statePlayerInGateOldSystem;
	}

void CTranscendenceModel::OnPlayerExitedGate (void)

//	OnPlayerExitedGate
//
//	This is called as soon as the player emerges from the stargate.

	{
	ASSERT(m_iState == statePlayerInGateNewSystem);

	CShip *pShip = m_pPlayer->GetShip();
	CSpaceObject *pStargate = m_Universe.GetPOV();
	CSystem *pNewSystem = pStargate->GetSystem();

	//	Clear the time-stop flag if necessary

	if (pShip->IsTimeStopped())
		pShip->RestartTime();

	//	Place the player in the new system

	pShip->Place(pStargate->GetPos());
	pShip->AddToSystem(pNewSystem);
	pShip->OnNewSystem();

	//	Restore timer events

	pNewSystem->TransferObjEventsIn(pShip, m_TimerEvents);

	//	Set globals

	CCodeChain &CC = m_Universe.GetCC();
	CC.DefineGlobalInteger(STR_G_PLAYER_SHIP, (int)pShip);

	//	Welcome message

	g_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("Welcome to the %s system!"), pNewSystem->GetName()));

	//	POV

	m_Universe.SetPOV(pShip);

	//	Fire global event

	m_Universe.FireOnGlobalPlayerEnteredSystem();

	//	Tell all objects that the player has entered the system

	pNewSystem->PlayerEntered(pShip);

	//	Update our stats, etc.

	m_pPlayer->OnSystemEntered(pNewSystem);

	//	Save the game because we're at a checkpoint

	SaveGame(CGameFile::FLAG_CHECKPOINT | CGameFile::FLAG_EXIT_GATE);

	//	Done

	m_iState = stateInGame;
	}

void CTranscendenceModel::OnPlayerTraveledThroughGate (void)

//	OnPlayerTraveledThroughGate
//
//	This is called to switch systems as the player travels through the
//	gate. We assume that OnPlayerEnteredGate has already been called and
//	that OnPlayerExitedGate will be called later.

	{
	ASSERT(m_iState == statePlayerInGateOldSystem);
	ASSERT(m_pDestNode);

	SetProgramState(psStargateEnter);

	//	If we've reached the end game node then the game is over

	if (m_pDestNode->IsEndGame())
		{
		SetProgramState(psStargateEndGame);

		//	Done with ship controller

		g_pTrans->CleanUpPlayerShip();

		//	Done

		m_iState = stateInGame;
		m_HI.HICommand(CONSTLIT("gameEnterFinalStargate"));
		return;
		}

	//	Get the player ship

	CShip *pShip = m_pPlayer->GetShip();

	//	Load or create the system

	CSystem *pNewSystem = m_pDestNode->GetSystem();
	if (pNewSystem == NULL)
		{
		SetProgramState(psStargateLoadingSystem);

		//	See if we need to create the system

		DWORD dwSystemID = m_pDestNode->GetSystemID();
		if (dwSystemID == 0xffffffff)
			{
			SetProgramState(psStargateCreatingSystem);

			//	If we failed to load, then we need to create a new system

			CString sError;
			if (m_Universe.CreateStarSystem(m_pDestNode, &pNewSystem, &sError) != NOERROR)
				{
				SetProgramError(sError);
				//g_pTrans->DisplayMessage(sError);
				//kernelDebugLogMessage(sError.GetASCIIZPointer());
				throw CException(1);
				}
			}

		//	Otherwise, load the system

		else
			{
			if (m_GameFile.LoadSystem(dwSystemID, &pNewSystem, OBJID_NULL, NULL, pShip) != NOERROR)
				{
				g_pTrans->DisplayMessage(CONSTLIT("ERROR: Unable to load system"));
				kernelDebugLogMessage("Error loading system: %s (%x)", m_pDestNode->GetSystemName().GetASCIIZPointer(), dwSystemID);
				throw CException(1);
				}
			}
		}

	//	Get the entry point. Note: We set the POV right away because Update
	//	cannot be called with a POV in a different system.

	CSpaceObject *pStart = pNewSystem->GetNamedObject(m_sDestEntryPoint);
	if (pStart == NULL)
		{
		kernelDebugLogMessage("Unable to find destination stargate %s in destination system.", m_sDestEntryPoint.GetASCIIZPointer());
		throw CException(1);
		}

	m_Universe.SetPOV(pStart);

	//	Add a discontinuity to reflect the amount of time spent
	//	in the stargate

	m_Universe.AddTimeDiscontinuity(CTimeSpan(0, mathRandom(0, (SECONDS_PER_DAY - 1) * 1000)));

	//	Time passes

	SetProgramState(psStargateUpdateExtended);
	m_Universe.UpdateExtended();

	//	Gate effect

	pStart->OnObjLeaveGate(pShip);

	//	Clear the POVLRS flag for all objects (so that we don't get the
	//	"Enemy Ships Detected" message when entering a system

	pNewSystem->SetPOVLRS(pStart);

	//	Move any henchmen through the stargate (note: we do this here because
	//	we need to remove the henchmen out of the old system before we save).

	SetProgramState(psStargateTransferringGateFollowers);
	TransferGateFollowers(m_pOldSystem, pNewSystem, pStart);

	//	Garbage-collect images and load those for the new system

	SetProgramState(psStargateGarbageCollecting);
	m_Universe.GarbageCollectLibraryBitmaps();

	SetProgramState(psStargateLoadingBitmaps);
	m_Universe.LoadLibraryBitmaps();

	//	Set the time that we stopped updating the system

	m_pOldSystem->SetLastUpdated();

	//	Save the old system.
	//	NOTE: From this point on the save file is invalid until we save the
	//	game at the end of LeaveStargate().
	//	FLAG_ENTER_GATE sets the save file in a state such that we can recover in case
	//	we crash before we save the rest of the file.

	SetProgramState(psStargateSavingSystem);
	if (m_GameFile.SaveSystem(m_pOldSystem->GetID(), m_pOldSystem, CGameFile::FLAG_ENTER_GATE) != NOERROR)
		kernelDebugLogMessage("Error saving system '%s' to game file", m_pOldSystem->GetName().GetASCIIZPointer());

	//	Remove the old system

	SetProgramState(psStargateFlushingSystem);
	m_Universe.FlushStarSystem(m_pOldSystem->GetTopology());

	SetProgramState(psStargateEnterDone);

	//	Done

	m_iState = statePlayerInGateNewSystem;
	}

void CTranscendenceModel::RecordFinalScore (const CString &sEpitaph, const CString &sEndGameReason, bool bEscaped)

//	RecordFinalScore
//
//	Adds the score to the high-score list and fires OnGameEnd for the adventure.

	{
	ASSERT(m_pPlayer);
	CShip *pPlayerShip = m_pPlayer->GetShip();

	CAdventureDesc *pAdventure = m_Universe.GetCurrentAdventureDesc();
	ASSERT(pAdventure);

	//	Add to high score list

	m_GameRecord.SetUsername(m_GameFile.GetUsername());
	m_GameRecord.SetGameID(m_GameFile.GetGameID());
	m_GameRecord.SetAdventureUNID(pAdventure->GetUNID());

	m_GameRecord.SetPlayerName(m_pPlayer->GetPlayerName());
	m_GameRecord.SetPlayerGenome(m_pPlayer->GetPlayerGenome());

	m_GameRecord.SetShipClass(pPlayerShip ? pPlayerShip->GetType()->GetUNID() : 0);
	m_GameRecord.SetSystem(pPlayerShip ? pPlayerShip->GetSystem() : NULL);

	m_GameRecord.SetEndGameReason(sEndGameReason);
	m_GameRecord.SetEndGameEpitaph(sEpitaph);
	m_GameRecord.SetPlayTime(m_Universe.StopGameTime());

	m_GameRecord.SetDebug(m_Universe.InDebugMode());

	//	Pass in the score before end game adjustment

	m_GameRecord.SetResurrectCount(m_pPlayer->GetResurrectCount());
	m_GameRecord.SetScore(m_pPlayer->GetScore());

	//	Collect some basic stats that are generally used in the epilogue.

	SBasicGameStats BasicStats;
	BasicStats.iSystemsVisited = m_pPlayer->GetSystemsVisited();
	BasicStats.iEnemiesDestroyed = m_pPlayer->GetEnemiesDestroyed();
	BasicStats.iBestEnemyDestroyedCount = m_pPlayer->GetBestEnemyShipsDestroyed(&BasicStats.dwBestEnemyDestroyed);

	//	Fire events

	SetCrawlImage(0);
	SetCrawlText(NULL_STR);
	pAdventure->FireOnGameEnd(m_GameRecord, BasicStats);

	//	Update the score in case it was changed inside OnGameEnd

	m_GameRecord.SetScore(m_pPlayer->GetEndGameScore());

	//	Add to high score if this is the default adventure

	if (pAdventure->GetUNID() == DEFAULT_ADVENTURE_UNID)
		m_iLastHighScore = AddHighScore(m_GameRecord);
	else
		m_iLastHighScore = -1;
	}

ALERROR CTranscendenceModel::SaveGame (DWORD dwFlags, CString *retsError)

//	SaveGame
//
//	Saves the game to a file

	{
	ALERROR error;

	ASSERT(m_GameFile.IsOpen());

	//	Fire and event to give global types a chance to save any volatiles

	m_Universe.FireOnGlobalUniverseSave();

	//	Generate and save game stats

	if (error = SaveGameStats())
		{
		if (retsError)
			*retsError = CONSTLIT("Error saving game stats to game file");
		kernelDebugLogMessage("Error saving game stats to game file");
		return error;
		}

	//	Save the current system

	CSystem *pSystem = m_pPlayer->GetShip()->GetSystem();
	if (error = m_GameFile.SaveSystem(pSystem->GetID(), pSystem))
		{
		if (retsError)
			*retsError = strPatternSubst(CONSTLIT("Error saving system '%s' to game file"), pSystem->GetName());
		kernelDebugLogMessage("Error saving system '%s' to game file", pSystem->GetName().GetASCIIZPointer());
		return error;
		}

	//	Save the universe

	if (error = m_GameFile.SaveUniverse(m_Universe, dwFlags))
		{
		if (retsError)
			*retsError = CONSTLIT("Error saving universe to game file");
		kernelDebugLogMessage("Error saving universe to game file");
		return error;
		}

	return NOERROR;
	}

ALERROR CTranscendenceModel::SaveHighScoreList (CString *retsError)

//	SaveHighScoreList
//
//	Saves the list

	{
	ALERROR error;

	//	Set the default player name and genome

	m_HighScoreList.SetMostRecentPlayerName(m_sPlayerName);
	m_HighScoreList.SetMostRecentPlayerGenome(m_iPlayerGenome);

	//	Save

	if (error = m_HighScoreList.Save(HIGH_SCORES_FILENAME))
		{
		if (retsError)
			*retsError = strPatternSubst(CONSTLIT("Unable to save high scores file: %s"), HIGH_SCORES_FILENAME);
		return error;
		}

	return NOERROR;
	}

void CTranscendenceModel::SetDebugMode (bool bDebugMode)

//	SetDebugMode
//
//	Sets debug mode

	{
	m_bDebugMode = bDebugMode;
	m_Universe.SetDebugMode(bDebugMode);
	}

const CString &CTranscendenceModel::SetPlayerName (const CString &sName)

//	SetPlayerName
//
//	Sets the player's name

	{
	m_sPlayerName = sName;

	//	Strip out any invalid characters. In particular, we don't like back-slash
	//	and % because they are escape characters

	char *pPos = m_sPlayerName.GetASCIIZPointer();
	while (*pPos != '\0')
		{
		if (*pPos < ' ' || *pPos == '\\' || *pPos == '%')
			*pPos = ' ';

		pPos++;
		}

	m_sPlayerName = strTrimWhitespace(m_sPlayerName);
	if (m_sPlayerName.IsBlank())
		m_sPlayerName = CONSTLIT("Shofixti");

	//	Return the actual name

	return m_sPlayerName;
	}

ALERROR CTranscendenceModel::SaveGameStats (bool bGameOver)

//	SaveGameStats
//
//	Saves stats to the current save file

	{
	ALERROR error;

	CGameStats Stats;
	GenerateGameStats(&Stats, bGameOver);
	if (error = m_GameFile.SaveGameStats(Stats))
		return error;

	//	Save the score and epitaph

	int iScore;
	CString sEpitaph;
	if (bGameOver)
		{
		iScore = m_GameRecord.GetScore();
		sEpitaph = m_GameRecord.GetEndGameEpitaph();
		}
	else
		{
		iScore = m_pPlayer->GetScore();
		sEpitaph = NULL_STR;
		}

	if (error = m_GameFile.SetGameStatus(iScore, sEpitaph))
		return error;

	//	Done

	return NOERROR;
	}

ALERROR CTranscendenceModel::ShowPane (const CString &sPane)

//	ShowPane
//
//	Shows the given pane

	{
	ASSERT(m_DockFrames.GetCount() > 0);
	SDockFrame *pFrame = &m_DockFrames[m_DockFrames.GetCount() - 1];

	//	Update the frame

	pFrame->sPane = sPane;

	//	Show it

	g_pTrans->m_CurrentDock.ShowPane(sPane);

	//	Done

	return NOERROR;
	}

ALERROR CTranscendenceModel::ShowScreen (CDesignType *pRoot, const CString &sScreen, const CString &sPane, bool bReturn)

//	ShowScreen
//
//	Shows the given screen
//
//	If pRoot is NULL then we attempt to resolve it using sScreen
//	and m_pDefaultScreensRoot

	{
	ALERROR error;

	ASSERT(m_DockFrames.GetCount() > 0);

	//	If pRoot is NULL, then we have to look it up based on sScreen (and the local
	//	screens root)

	CString sScreenActual;
	if (pRoot)
		sScreenActual = sScreen;
	else
		{
		if (!FindScreenRoot(sScreen, &pRoot, &sScreenActual))
			return ERR_FAIL;
		}

	//	If the root is a screen then use that

	CXMLElement *pScreen;
	CXMLElement *pLocalScreens = NULL;
	if (pRoot->GetType() == designDockScreen)
		pScreen = CDockScreenType::AsType(pRoot)->GetDesc();
	else
		{
		pLocalScreens = pRoot->GetLocalScreens();
		if (pLocalScreens == NULL)
			return ERR_FAIL;

		pScreen = pLocalScreens->GetContentElementByTag(sScreenActual);
		if (pScreen == NULL)
			return ERR_FAIL;
		}

	//	Some screens pop us into a new frame

	bool bNewFrame;
	SDockFrame *pFrame;
	if (bNewFrame = (!bReturn && pScreen->GetAttributeBool(NESTED_SCREEN_ATTRIB)))
		{
		SDockFrame *pCurFrame = &m_DockFrames[m_DockFrames.GetCount() - 1];
		pFrame = m_DockFrames.Insert();

		pFrame->pLocation = pCurFrame->pLocation;
		pFrame->pRoot = NULL;
		}
	else
		pFrame = &m_DockFrames[m_DockFrames.GetCount() - 1];

	//	Update the frame

	CDesignType *pOldRoot = pFrame->pRoot;
	CString sOldScreen = pFrame->sScreen;
	CString sOldPane = pFrame->sPane;

	pFrame->pRoot = pRoot;
	pFrame->sScreen = sScreenActual;
	pFrame->sPane = sPane;

	//	Show the screen
	//
	//	NOTE: This call can recurse (because we can call ShowScreen
	//	again from inside <OnScreenInit>). In case of recursion, the
	//	deepest call will set up sNewPane while the others will
	//	return NULL_STR.

	CString sNewPane;
	if (error = g_pTrans->m_CurrentDock.InitScreen(m_HI.GetHWND(),
			g_pTrans->m_rcMainScreen,
			pFrame->pLocation,
			pLocalScreens,
			pScreen,
			sPane,
			&sNewPane,
			&g_pTrans->m_pCurrentScreen))
		{
		//	Undo

		if (bNewFrame)
			m_DockFrames.Delete(m_DockFrames.GetCount() - 1);
		else
			{
			pFrame->pRoot = pOldRoot;
			pFrame->sScreen = sOldScreen;
			pFrame->sPane = sOldPane;
			}

		return error;
		}

	//	If no frames then we exited inside of InitScreen

	if (m_DockFrames.GetCount() == 0)
		return NOERROR;

	//	Update frame

	if (!sNewPane.IsBlank())
		pFrame->sPane = sNewPane;

	return NOERROR;
	}

void CTranscendenceModel::ShowShipScreen (void)

//	ShowShipScreen
//
//	Show the ship screen specified in CPlayerSettings

	{
	CShip *pShip = m_pPlayer->GetShip();
	if (pShip == NULL)
		return;

	const CPlayerSettings *pSettings = pShip->GetClass()->GetPlayerSettings();
	if (pSettings->GetShipScreen() == NULL)
		return;

	CString sScreen;
	CDesignType *pRoot = pSettings->GetShipScreen().GetDockScreen(pShip->GetClass(), &sScreen);

	ShowShipScreen(NULL, pRoot, sScreen, NULL_STR);

	pShip->OnComponentChanged(comCargo);
	}

void CTranscendenceModel::ShowShipScreen (CDesignType *pDefaultScreensRoot, CDesignType *pRoot, const CString &sScreen, const CString &sPane)

//	ShowShipScreen
//
//	Show a ship dock screen
//
//	If pRoot is NULL then we attempt to resolve sScreen using pDefaultScreensRoot
//	if necessary.

	{
	ASSERT(pRoot);
	CShip *pShip = m_pPlayer->GetShip();
	if (pShip == NULL)
		return;

	//	If the default root is passed in, use that. Otherwise, we pull local
	//	screens from the ship class.

	m_pDefaultScreensRoot = pDefaultScreensRoot;
	if (m_pDefaultScreensRoot == NULL)
		m_pDefaultScreensRoot = pShip->GetType();

	//	Show screen

	if (EnterScreenSession(pShip, pRoot, sScreen, sPane) != NOERROR)
		{
		CString sError = strPatternSubst(CONSTLIT("[%s]: Unable to show screen %s"), pRoot->GetTypeName(), sScreen);
		g_pTrans->DisplayMessage(sError);
		::kernelDebugLogMessage(sError.GetASCIIZPointer());
		return;
		}
	}

ALERROR CTranscendenceModel::StartGame (void)

//	StartGame
//
//	This is called after either creating a new game or loading an old one

	{
	//	Tell the universe to focus on the ship

	m_Universe.SetPlayer(m_pPlayer->GetShip());
	m_Universe.SetPOV(m_pPlayer->GetShip());

	//	Set sound

	if (m_bNoSound)
		m_Universe.SetSound(false);

	//	Tell the controller that we're starting

	m_pPlayer->OnStartGame();

	//	Tell the universe to start the game

	m_Universe.StartGame(m_iState == stateCreatingNewGame);

	//	Update stats

	if (m_iState == stateCreatingNewGame)
		m_pPlayer->OnSystemEntered(m_Universe.GetCurrentSystem());

	//	Done

	m_iState = stateInGame;

	return NOERROR;
	}

ALERROR CTranscendenceModel::StartNewGame (const CString &sUsername, const SNewGameSettings &NewGame, CString *retsError)

//	StartNewGame
//
//	Initializes:
//		m_GameFile
//		m_Universe
//		m_pPlayer

	{
	//	New game

	CString sFilename = m_GameFile.GenerateFilename(NewGame.sPlayerName);
	m_GameFile.Create(sFilename, sUsername);

	//	Create a controller for the player's ship (this is owned
	//	by the ship once we pass it to CreateShip)

	ASSERT(m_pPlayer == NULL);
	m_pPlayer = new CPlayerShipController;
	if (m_pPlayer == NULL)
		{
		*retsError = CONSTLIT("Unable to create CPlayerShipController");
		g_pUniverse->Reinit();
		m_GameFile.Close();
		return ERR_MEMORY;
		}

	m_pPlayer->SetTrans(g_pTrans);
	m_pPlayer->SetName(NewGame.sPlayerName);
	m_pPlayer->SetGenome(NewGame.iPlayerGenome);
	m_pPlayer->SetStartingShipClass(NewGame.dwPlayerShip);

	//	Define globals for OnGameStart (only gPlayer is defined)
	//	We need this because script may want to reference gPlayer
	//	to get genome information.
	//
	//	The rest of the variables will be set in g_pUniverse->SetPlayer

	CCodeChain &CC = m_Universe.GetCC();
	CC.DefineGlobal(CONSTLIT("gPlayer"), CC.CreateInteger((int)m_pPlayer));

	//	Invoke Adventure OnGameStart
	//	NOTE: The proper adventure is set by a call to InitAdventure,
	//	when the adventure is chosen.

	CAdventureDesc *pAdventure = m_Universe.GetCurrentAdventureDesc();
	ASSERT(pAdventure);

	m_dwCrawlImage = 0;
	m_sCrawlText = NULL_STR;
	pAdventure->FireOnGameStart();

	//	The remainder of new game start happens in the background thread
	//	in StartNewGamebackground

	m_iState = stateCreatingNewGame;

	return NOERROR;
	}

ALERROR CTranscendenceModel::StartNewGameBackground (CString *retsError)

//	StartNewGameBackground
//
//	Starts a new game

	{
	ALERROR error;

	ASSERT(m_iState == stateCreatingNewGame);
	ASSERT(m_pPlayer);

	//	Initialize topology, etc.

	if (error = m_Universe.InitGame(retsError))
		return error;

	//	Figure out the ship class that we want

	CShipClass *pStartingShip = m_Universe.FindShipClass(m_pPlayer->GetStartingShipClass());
	const CPlayerSettings *pPlayerSettings = pStartingShip->GetPlayerSettings();
	if (pPlayerSettings == NULL)
		{
		*retsError = CONSTLIT("Missing <PlayerSettings> for ship class");
		return ERR_FAIL;
		}

	CString sStartNode;
	CString sStartPos;
	CalcStartingPos(pStartingShip, &sStartNode, &sStartPos);

	//	Get the starting system

	CSystem *pSystem;
	if (error = m_Universe.CreateStarSystem(sStartNode, &pSystem, retsError))
		return error;

	//	Figure out where in the system we want to start

	CVector vStartPos;
	CSpaceObject *pStart = pSystem->GetNamedObject(sStartPos);
	if (pStart)
		vStartPos = pStart->GetPos();

	//	Set some credits

	const CCurrencyAndRange &StartingCredits = pPlayerSettings->GetStartingCredits();
	m_pPlayer->Payment(StartingCredits.GetCurrencyType()->GetUNID(), StartingCredits.Roll());

	//	Create the player's ship

	CShip *pPlayerShip;
	if (error = pSystem->CreateShip(m_pPlayer->GetStartingShipClass(),
			m_pPlayer,
			NULL,
			m_Universe.FindSovereign(g_PlayerSovereignUNID),
			vStartPos,
			NullVector,
			90,
			NULL,
			NULL,
			&pPlayerShip))
		{
		*retsError = CONSTLIT("Unable to create player ship");
		ResetPlayer();
		return error;
		}

	//	Ship needs to track fuel and mass

	pPlayerShip->TrackFuel();
	pPlayerShip->TrackMass();
#ifdef DEBUG_SHIP
	pPlayerShip->InstallTargetingComputer();
	pPlayerShip->SetSRSEnhanced();
#endif

	//	Associate with the controller

	m_pPlayer->SetShip(pPlayerShip);

	//	All items on the ship are automatically 
	//	known to the player

	CItemListManipulator ItemList(pPlayerShip->GetItemList());
	while (ItemList.MoveCursorForward())
		{
		CItemType *pType = ItemList.GetItemAtCursor().GetType();
		pType->SetKnown();
		pType->SetShowReference();
		}

#ifdef DEBUG_ALL_ITEMS
	if (m_bDebugMode)
		{
		for (int i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
			{
			CItemType *pType = g_pUniverse->GetItemType(i);

			//	Do not count unknown items

			if (pType->HasAttribute(CONSTLIT("unknown")))
				continue;

			//	Do not count virtual items

			if (pType->IsVirtual())
				continue;

			//	Add item

			CItem Item(pType, 1);
			ItemList.AddItem(Item);

			pType->SetKnown();
			pType->SetShowReference();
			}
		}
#endif

#ifdef DEBUG_HENCHMAN
	if (m_bDebugMode)
		{
		CShip *pHenchman;

		for (int i = 0; i < 14; i++)
			{
			CFleetShipAI *pController = new CFleetShipAI;
			if (error = pSystem->CreateShip(g_DebugHenchmenShipUNID,
						pController,
						NULL,
						g_pUniverse->FindSovereign(g_PlayerSovereignUNID),
						vStartPos + PolarToVector(0, g_KlicksPerPixel * 100.0),
						NullVector,
						0,
						NULL,
						&pHenchman))
				{
				*retsError = CONSTLIT("Unable to create henchmen");
				return error;
				}

			pHenchman->GetController()->AddOrder(IShipController::orderEscort, pPlayerShip, i);
			}
		}
#endif

	//	Clock is running

	m_Universe.StartGameTime();

	//	When we're done, CTranscendenceController gets the CMD_MODEL_NEW_GAME_CREATED
	//	message, which eventually calls CTranscendenceWnd::StartGame.

	return NOERROR;
	}

void CTranscendenceModel::TransferGateFollowers (CSystem *pOldSystem, CSystem *pSystem, CSpaceObject *pStargate)

//	TransferGateFollowers
//
//	Move henchmen to the new system

	{
	int i;

	//	Convert all the gate followers from ObjIDs to pointers. We wait until now because
	//	the gate followers could have gotten destroyed in the mean time

	TArray<CSpaceObject *> GateFollowerObjs;
	for (i = 0; i < m_GateFollowers.GetCount(); i++)
		{
		CSpaceObject *pObj = pOldSystem->FindObject(m_GateFollowers[i]);
		if (pObj)
			GateFollowerObjs.Insert(pObj);
		}

	//	The first follower shouldn't appear until after the player has left the gate
	//	(so that the player is in the system before the first follower is unsuspended).

	int iTimer = TICKS_AFTER_GATE + mathRandom(5, 15);

	//	Deal with all followers

	for (i = 0; i < GateFollowerObjs.GetCount(); i++)
		{
		CSpaceObject *pFollower = GateFollowerObjs[i];
		CShip *pShip = pFollower->AsShip();

		//	Make sure the ship is ready to transfer

		if (pShip)
			{
			//	If we're docked, undock
			pShip->Undock();
			}

		//	Randomize the position a bit so that ships don't end up being on top
		//	of each other (this can happen to zoanthropes who have very deterministic
		//	motion).

		CVector vPos = pStargate->GetPos() + PolarToVector(mathRandom(0, 359), g_KlicksPerPixel);

		//	If this obj has any events, move them to the new system
		//	(We do this before we remove the follower from the system because otherwise
		//	the system will automatically remove the events)

		CTimedEventList ObjEvents;
		pFollower->GetSystem()->TransferObjEventsOut(pFollower, ObjEvents);

		//	Remove the ship from the old system

		pFollower->Remove(enteredStargate, CDamageSource(pStargate, enteredStargate));

		//	Clear the time-stop flag if necessary

		if (pFollower->IsTimeStopped())
			pFollower->RestartTime();

		//	Place the ship at the gate in the new system

		pFollower->Resume();
		pFollower->Place(vPos);
		pFollower->AddToSystem(pSystem);

		//	Move obj events to the new system

		pSystem->TransferObjEventsIn(pFollower, ObjEvents);

		//	Deal with ship stuff
		
		if (pShip)
			{
			//	Cancel the follow through gate order
			pShip->GetController()->CancelCurrentOrder();

			//	Ship appears in gate
			pShip->SetInGate(pStargate, iTimer);
			iTimer += mathRandom(20, 40);

			//	Misc clean up
			pShip->ResetMaxSpeed();
			}
		}

	//	Let each follower know that it has been moved.
	//
	//	Note that both old and new systems need to be loaded at this point because
	//	OnNewSystem may refer to objects in the old system.

	for (i = 0; i < GateFollowerObjs.GetCount(); i++)
		GateFollowerObjs[i]->OnNewSystem();

	//	Done

	m_GateFollowers.DeleteAll();
	}

void CTranscendenceModel::UseItem (CItem &Item)

//	UseItem
//
//	Use item

	{
	}
