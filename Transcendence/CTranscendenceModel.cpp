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
//	CTranscendenceController::HICommand("gameInsideStargate")
//		CTranscendenceModel::OnPlayerTraveledThroughGate
//			Create or load system
//			UpdateExtended
//			TransferGateFollowers
//			Set POV to new system
//
//	...wait for timer...
//
//	CTranscendenceController::HICommand("cmdGameLeaveStargate")
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

#define CMD_GAME_ENTER_FINAL_STARGATE			CONSTLIT("gameEnterFinalStargate")
#define CMD_GAME_ENTER_STARGATE					CONSTLIT("gameEnterStargate")
#define CMD_GAME_STARGATE_SYSTEM_READY			CONSTLIT("gameStargateSystemReady")
#define CMD_PLAYER_UNDOCKED						CONSTLIT("playerUndocked")

#define FOLDER_SAVE_FILES						CONSTLIT("Games")

#define GRAPHICS_AUTO							CONSTLIT("auto")
#define GRAPHICS_MINIMUM						CONSTLIT("minimum")
#define GRAPHICS_STANDARD						CONSTLIT("standard")
#define GRAPHICS_MAXIMUM						CONSTLIT("maximum")

#define HIGH_SCORES_FILENAME					CONSTLIT("HighScores.xml")

#define STR_G_PLAYER_SHIP						CONSTLIT("gPlayerShip")

#define NESTED_SCREEN_ATTRIB					CONSTLIT("nestedScreen")

#define DEFAULT_SCREEN_NAME						CONSTLIT("[DefaultScreen]")

CTranscendenceModel::CTranscendenceModel (CHumanInterface &HI) : 
		m_HI(HI), 
		m_iState(stateUnknown),
		m_bDebugMode(false),
		m_bForceTDB(false),
		m_bNoSound(false),
		m_bNoMissionCheckpoint(false),
		m_pPlayer(NULL),
		m_pResurrectType(NULL),
		m_pCrawlImage(NULL),
		m_pCrawlSoundtrack(NULL),
		m_iLastHighScore(-1),
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

void CTranscendenceModel::AddSaveFileFolder (const CString &sFilespec)

//	AddSaveFileFolder
//
//	Adds a save file folder to the list. The first one becomes the default folder.
	
	{
	m_SaveFileFolders.Insert(sFilespec);

	//	If it doesn't exist, create it

	if (!pathExists(sFilespec))
		{
		if (!pathCreate(sFilespec))
			::kernelDebugLogMessage("Unable to create save file folder: %s", sFilespec);
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
		if (pAttacker->IsAngryAt(pShip) 
				|| pAttacker->GetSovereign() == NULL
				|| (pAttacker->GetScale() == scaleWorld || pAttacker->GetScale() == scaleStar))
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

		case killedByGravity:
			sText = strPatternSubst(CONSTLIT("was ripped apart %s"), sCause);
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

void CTranscendenceModel::CalcStartingPos (CShipClass *pStartingShip, DWORD *retdwMap, CString *retsNodeID, CString *retsPos)

//	CalcStartingPos
//
//	Calculates the starting position of the player in a new game

	{
	//	See if the player ship specifies it.

	const CPlayerSettings *pPlayerSettings = pStartingShip->GetPlayerSettings();
	DWORD dwMap = pPlayerSettings->GetStartingMap();
	CString sNodeID = pPlayerSettings->GetStartingNode();
	CString sPos = pPlayerSettings->GetStartingPos();

	//	If not, see if the adventure does

	if (dwMap == 0 || sNodeID.IsBlank() || sPos.IsBlank())
		{
		CAdventureDesc *pAdventure = m_Universe.GetCurrentAdventureDesc();
		if (dwMap == 0)
			dwMap = pAdventure->GetStartingMapUNID();

		if (sNodeID.IsBlank())
			sNodeID = pAdventure->GetStartingNodeID();

		if (sPos.IsBlank())
			sPos = pAdventure->GetStartingPos();
		}

	//	If not, come up with a reasonable default
	//	(It's OK if dwMap is 0.)

	if (sNodeID.IsBlank() || sPos.IsBlank())
		{
		sNodeID = m_Universe.GetDesignCollection().GetStartingNodeID();
		CTopologyNode *pNode = m_Universe.FindTopologyNode(sNodeID);
		if (pNode && pNode->GetStargateCount() > 0)
			sPos = pNode->GetStargate(0);
		}

	//	Done

	if (retdwMap)
		*retdwMap = dwMap;

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

ALERROR CTranscendenceModel::CreateAllSystems (const CString &sStartNode, CSystem **retpStartingSystem, CString *retsError)

//	CreateAllSystems
//
//	Create all systems for the game.

	{
	ALERROR error;
	int i;
	CString sError;

	CTopologyNode *pStartingNode = NULL;

	//	Loop over all topology nodes and create the system.
	//	(Except for the starting system, which we create at the end.)

	for (i = 0; i < m_Universe.GetTopologyNodeCount(); i++)
		{
		CTopologyNode *pNode = m_Universe.GetTopologyNode(i);

		//	Do not try to create end game nodes

		if (pNode->IsEndGame())
			continue;

		//	If this is the starting node, then skip it (we create it later).

		if (strEquals(pNode->GetID(), sStartNode))
			{
			pStartingNode = pNode;
			continue;
			}

		//	Otherwise, create this system

		CSystem *pNewSystem;
		if (error = m_Universe.CreateStarSystem(pNode, &pNewSystem, &sError))
			{
			kernelDebugLogMessage("Error creating system %s: %s", pNode->GetSystemName(), sError);
			if (retsError)
				*retsError = sError;
			return error;
			}

		//	Save the system to disk

		if (error = m_GameFile.SaveSystem(pNewSystem->GetID(), pNewSystem, 0))
			{
			kernelDebugLogMessage("Error saving system '%s' to game file", pNewSystem->GetName());
			if (retsError)
				*retsError = CONSTLIT("Unable to save system.");
			return error;
			}

		//	Delete the system

		m_Universe.FlushStarSystem(pNode);
		}

	//	Create the starting system

	if (pStartingNode == NULL)
		{
		if (retsError)
			*retsError = strPatternSubst(CONSTLIT("Unable to find starting system: %s"), sStartNode);
		return ERR_FAIL;
		}

	if (error = m_Universe.CreateStarSystem(pStartingNode, retpStartingSystem, retsError))
		return error;

	//	Output some counts

#ifdef DEBUG_ENCOUNTER_COUNTS
	for (i = 0; i < m_Universe.GetStationTypeCount(); i++)
		{
		CStationType *pType = m_Universe.GetStationType(i);
		if (!pType->GetLocationCriteria().IsBlank())
			{
			CStationEncounterCtx &EncounterRecord = pType->GetEncounterRecord();
			if (EncounterRecord.GetTotalLimit() != -1 && EncounterRecord.GetTotalCount() > EncounterRecord.GetTotalLimit())
				::kernelDebugLogMessage("%s (%08x): %d created. WARNING: Limit is %d", pType->GetName(), pType->GetUNID(), EncounterRecord.GetTotalCount(), EncounterRecord.GetTotalLimit());
			else if (EncounterRecord.GetTotalMinimum() > 0 && EncounterRecord.GetTotalCount() < EncounterRecord.GetTotalMinimum())
				::kernelDebugLogMessage("%s (%08x): %d created. WARNING: Minimum is %d", pType->GetName(), pType->GetUNID(), EncounterRecord.GetTotalCount(), EncounterRecord.GetTotalMinimum());
			else
				::kernelDebugLogMessage("%s (%08x): %d created.", pType->GetName(), pType->GetUNID(), EncounterRecord.GetTotalCount());
			}
		}
#endif

	//	Done

	return NOERROR;
	}

ALERROR CTranscendenceModel::EndGame (void)

//	EndGame
//
//	Ends the game. We've stored all the state in our temporaries.

	{
	ALERROR error;

	ASSERT(m_iState == statePlayerInEndGame);

	//	Bonus for escaping Human Space

	m_pPlayer->IncScore(m_iScoreBonus);

	//	Game over (compile stats)

	m_pPlayer->OnGameEnd();

	//	Record final score

	RecordFinalScore(m_sEpitaph, m_sEndGameReason, false);

	//	We need to define gPlayerShip so that game stats 
	//	are generated properly. (It gets cleared out in CTranscendenceWnd::PlayerEnteredGate)

	CCodeChain &CC = m_Universe.GetCC();
	CC.DefineGlobal(CONSTLIT("gPlayerShip"), CC.CreateInteger((int)m_pPlayer->GetShip()));

	//	Generate stats and save to file

	GenerateGameStats(&m_GameStats, true);
	if (error = SaveGameStats(m_GameStats, true, true))
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

ALERROR CTranscendenceModel::EndGame (const CString &sReason, const CString &sEpitaph, int iScoreChange)

//	EndGame
//
//	Sets the end game condition.

	{
	m_iScoreBonus = iScoreChange;
	m_sEndGameReason = sReason;
	m_sEpitaph = sEpitaph;
	m_iState = statePlayerInEndGame;

	//	We wait to get called at EndGame to complete the end game.

	return NOERROR;
	}

ALERROR CTranscendenceModel::EndGameClose (CString *retsError)

//	EndGameClose
//
//	Player has closed the app and we need to save

	{
	ALERROR error;

	switch (m_iState)
		{
		case stateInGame:
			{
			int iMaxLoops = 100;

			//	If we have dock screens up, keep hitting the cancel action
			//	until we're done.

			while (!m_DockFrames.IsEmpty() && iMaxLoops-- > 0)
				g_pTrans->m_CurrentDock.ExecuteCancelAction();

			if (!m_DockFrames.IsEmpty())
				ExitScreenSession(true);

			//	If we're dead after this, then recurse

			if (m_iState != stateInGame)
				return EndGameClose(retsError);

			//	Otherwise, save the game

			if (error = EndGameSave(retsError))
				return error;

			break;
			}

		case statePlayerInResurrect:
			{
			EndGameDestroyed();

			if (error = EndGameSave(retsError))
				return error;

			break;
			}

		case statePlayerDestroyed:
			{
			if (error = EndGameDestroyed())
				return error;

			break;
			}

		//	LATER: Handle the case were we close while in a stargate
		//	or while resurrecting.
		}

	//	Tell the universe to save local storage

	if (error = m_Universe.SaveDeviceStorage())
		return error;

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
				kernelDebugLogMessage(sError);
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

		GenerateGameStats(&m_GameStats, true);
		if (error = SaveGameStats(m_GameStats, true))
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
	m_iScoreBonus = 10000;
	m_sEndGameReason = m_pDestNode->GetEndGameReason();
	m_sEpitaph = m_pDestNode->GetEpitaph();
	m_iState = statePlayerInEndGame;
	return EndGame();
	}

ALERROR CTranscendenceModel::EnterScreenSession (CSpaceObject *pLocation, CDesignType *pRoot, const CString &sScreen, const CString &sPane, ICCItem *pData)

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

	bool bFirstFrame = m_DockFrames.IsEmpty();

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

	SDockFrame NewFrame;
	NewFrame.pLocation = pLocation;
	NewFrame.pRoot = pRoot;
	NewFrame.sScreen = sScreen;
	NewFrame.sPane = sPane;
	NewFrame.pInitialData = pData;
	NewFrame.pResolvedRoot = pRoot;
	NewFrame.sResolvedScreen = (pRoot ? sScreen : NULL_STR);
	m_DockFrames.Push(NewFrame);

	//	From this point forward we are considered in a screen session.
	//	[We use this to determine whether a call to scrShowScreen switches
	//	screens or enters a new session.]
	//
	//	Initialize the current screen object

	CString sError;
	if (error = ShowScreen(pRoot, sScreen, sPane, pData, &sError, false, true))
		{
		//	Undo

		m_DockFrames.Pop();

		if (!bOldPlayerDocked)
			pLocation->ClearPlayerDocked();

		return error;
		}

	//	If we don't have any frames then we called exit from inside of ShowScreen

	if (m_DockFrames.IsEmpty())
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
	DEBUG_TRY

	ASSERT(!m_DockFrames.IsEmpty());

	//	If we have another frame, then switch back to that screen

	if (m_DockFrames.GetCount() > 1 && !bForceUndock)
		{
		//	Remove the current frame

		m_DockFrames.Pop();

		//	Switch
		//	bReturn = true (on ShowScreen) so that we don't restack a
		//	nested screen that we are returning to.

		CString sError;
		const SDockFrame &Frame = m_DockFrames.GetCurrent();
		ShowScreen(Frame.pRoot, Frame.sScreen, Frame.sPane, Frame.pInitialData, &sError, true);
		}

	//	Otherwise, exit

	else
		{
		const SDockFrame &Frame = m_DockFrames.GetCurrent();

		//	Unmark the object

		Frame.pLocation->ClearPlayerDocked();

		//	If we're docked with something (came from OnPlayerDocked)
		//	then we need to undock

		if (m_pDock)
			{
			m_pPlayer->Undock();
			m_pDock = NULL;
			}

		//	Tell controller that we're undocked

		m_HI.HICommand(CMD_PLAYER_UNDOCKED);

		//	Clean up

		m_pDefaultScreensRoot = NULL;
		g_pTrans->ShowDockScreen(false);
		g_pTrans->m_CurrentDock.CleanUpScreen();

		m_DockFrames.DeleteAll();
		}

	DEBUG_CATCH
	}

bool CTranscendenceModel::FindScreenRoot (const CString &sScreen, CDesignType **retpRoot, CString *retsScreen, ICCItem **retpData)

//	FindScreenRoot
//
//	Finds the appropriate root given a string name.
//	NOTE: If we return TRUE, caller must discard *retpData.

	{
	CCodeChain &CC = m_Universe.GetCC();

	//	If the screen is an UNID, then expect a stand-alone screen

	bool bNotANumber;
	DWORD dwUNID = (DWORD)strToInt(sScreen, 0, &bNotANumber);
	if (!bNotANumber)
		{
		CDesignType *pRoot = m_Universe.FindSharedDockScreen(dwUNID);
		if (pRoot == NULL)
			return false;

		if (retpRoot)
			*retpRoot = pRoot;

		if (retsScreen)
			*retsScreen = sScreen;

		if (retpData)
			*retpData = NULL;

		return true;
		}

	//	Otherwise, if this is the special name [DefaultScreen] then
	//	we ask the object for the proper screen

	else if (strEquals(sScreen, DEFAULT_SCREEN_NAME))
		{
		CString sScreenActual;
		ICCItem *pData;

		ASSERT(!m_DockFrames.IsEmpty());
		const SDockFrame &Frame = m_DockFrames.GetCurrent();
		ASSERT(Frame.pLocation);

		CDesignType *pRoot = Frame.pLocation->GetFirstDockScreen(&sScreenActual, &pData);
		if (pRoot == NULL)
			{
			if (pData)
				pData->Discard(&CC);
			return false;
			}

		if (retpRoot)
			*retpRoot = pRoot;

		if (retsScreen)
			*retsScreen = sScreenActual;

		if (retpData)
			*retpData = pData;
		else
			pData->Discard(&CC);

		return true;
		}

	//	Otherwise, this is a local screen, so we use the default root

	else if (m_pDefaultScreensRoot)
		{
		if (retpRoot)
			*retpRoot = m_pDefaultScreensRoot;

		if (retsScreen)
			*retsScreen = sScreen;

		if (retpData)
			*retpData = NULL;

		return true;
		}

	//	Otherwise, use the player ship

	else if (m_pPlayer->GetShip())
		{
		if (retpRoot)
			*retpRoot = m_pPlayer->GetShip()->GetType();

		if (retsScreen)
			*retsScreen = sScreen;

		if (retpData)
			*retpData = NULL;

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
	if (m_Universe.GetPlayerShip() == NULL)
		{
		ASSERT(pPlayerShip);
		m_Universe.SetPlayerShip(pPlayerShip);
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
		m_Universe.SetPlayerShip(NULL);
	}

ALERROR CTranscendenceModel::GetGameStats (CGameStats *retStats)

//	GetGameStats
//
//	Gets the game stats for the current player

	{
	//	If we're in the middle of a game, then generate the stats now

	if (m_iState == stateInGame)
		GenerateGameStats(retStats);

	//	Otherwise, return the most recent stats

	else
		*retStats = m_GameStats;

	return NOERROR;
	}

void CTranscendenceModel::GetScreenSession (SDockFrame *retFrame)

//	GetScreenSession
//
//	Returns info about the current screen session

	{
	ASSERT(!m_DockFrames.IsEmpty());
	*retFrame = m_DockFrames.GetCurrent();
	}

ALERROR CTranscendenceModel::Init (const CGameSettings &Settings)

//	Init
//
//	Initialize

	{
	::fileGetVersionInfo(NULL_STR, &m_Version);

	//	Set some options

	AddSaveFileFolder(pathAddComponent(Settings.GetAppDataFolder(), FOLDER_SAVE_FILES));
	SetDebugMode(Settings.GetBoolean(CGameSettings::debugGame));
	m_bForceTDB = Settings.GetBoolean(CGameSettings::useTDB);
	m_bNoMissionCheckpoint = Settings.GetBoolean(CGameSettings::noMissionCheckpoint);
	m_bNoSound = Settings.GetBoolean(CGameSettings::noSound);

	return NOERROR;
	}

ALERROR CTranscendenceModel::InitBackground (const CGameSettings &Settings, const CString &sCollectionFolder, const TArray<CString> &ExtensionFolders, CString *retsError)

//	InitBackground
//
//	Initializes the model, including loading the universe. This function
//	is designed to be called in a background thread.

	{
	ALERROR error;

	//	Set the graphics quality

	CString sGraphics = Settings.GetString(CGameSettings::graphicsQuality);
	if (sGraphics.IsBlank() || strEquals(sGraphics, GRAPHICS_AUTO))
		m_Universe.GetSFXOptions().SetSFXQualityAuto();
	else if (strEquals(sGraphics, GRAPHICS_MINIMUM))
		m_Universe.GetSFXOptions().SetSFXQuality(CSFXOptions::sfxMinimum);
	else if (strEquals(sGraphics, GRAPHICS_STANDARD))
		m_Universe.GetSFXOptions().SetSFXQuality(CSFXOptions::sfxStandard);
	else if (strEquals(sGraphics, GRAPHICS_MAXIMUM))
		m_Universe.GetSFXOptions().SetSFXQuality(CSFXOptions::sfxMaximum);
	else
		{
		m_Universe.GetSFXOptions().SetSFXQualityAuto();
		::kernelDebugLogMessage("Unknown graphics quality: %s.", sGraphics);
		}

	//	Now set some additional options that may override the default graphics
	//	quality settings.

	if (Settings.GetBoolean(CGameSettings::showManeuverEffects))
		m_Universe.GetSFXOptions().SetManeuveringEffectEnabled();

	if (Settings.GetBoolean(CGameSettings::no3DSystemMap))
		m_Universe.GetSFXOptions().Set3DSystemMapEnabled(false);

	//	Load the universe

	if (error = LoadUniverse(sCollectionFolder, ExtensionFolders, retsError))
		return error;

	//	Load the high scores list

	if (error = LoadHighScoreList(retsError))
		return error;

	return NOERROR;
	}

ALERROR CTranscendenceModel::InitAdventure (const SAdventureSettings &Settings, CString *retsError)

//	InitAdventure
//
//	Initializes the adventure in preparation for creating a new game.

	{
	CUniverse::SInitDesc Ctx;
	Ctx.bDebugMode = m_Universe.InDebugMode();
	Ctx.dwAdventure = Settings.pAdventure->GetUNID();
	Ctx.Extensions = Settings.Extensions;

	::kernelDebugLogMessage("Initializing adventure: %s", Settings.pAdventure->GetFilespec());

#ifdef DEBUG_RANDOM_SEED
	mathSetSeed(100);
#endif

	return m_Universe.Init(Ctx, retsError);
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

	//	See if a map is available for this node

	CTopologyNode *pNode = m_Universe.GetCurrentTopologyNode();
	if (pNode == NULL)
		{
		if (retsError)
			*retsError = CONSTLIT("You are in the Twilight Zone");

		return false;
		}
		
	if (pNode->GetDisplayPos() == NULL)
		{
		if (retsError)
			*retsError = CONSTLIT("No galactic map available for your location.");

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

ALERROR CTranscendenceModel::LoadGame (const CString &sSignedInUsername, const CString &sFilespec, CString *retsError)

//	LoadGame
//
//	Load a previously saved game and keeps the game file open

	{
	try
		{
		ALERROR error;

		m_Universe.Reinit();

		//	Old game

		if (error = m_GameFile.Open(sFilespec))
			{
			*retsError = strPatternSubst(CONSTLIT("Unable to open save file: %s"), sFilespec);
			return error;
			}

		//	If this is a registered game and we're not signed in with that player
		//	then we can't continue.

		if (m_GameFile.IsRegistered() && !strEquals(m_GameFile.GetUsername(), sSignedInUsername))
			{
			*retsError = strPatternSubst(CONSTLIT("Save file %s may only be opened by %s."), sFilespec, m_GameFile.GetUsername());
			m_GameFile.Close();
			return ERR_FAIL;
			}

		//	If this game is in end game state then we just load the stats.

		if (m_GameFile.IsEndGame())
			{
			error = m_GameFile.LoadGameStats(&m_GameStats);
			m_GameFile.Close();
			if (error)
				{
				*retsError = strPatternSubst(CONSTLIT("Unable to load stats from save file: %s"), sFilespec);
				return error;
				}

			return ERR_CANCEL;
			}

		//	Load the universe

		DWORD dwSystemID, dwPlayerID;
		if (error = m_GameFile.LoadUniverse(m_Universe, &dwSystemID, &dwPlayerID, retsError))
			{
			m_GameFile.Close();
			m_Universe.Reinit();
			return error;
			}

		//	Load the POV system

		CSystem *pSystem;
		CSpaceObject *pPlayerObj;
		if (error = m_GameFile.LoadSystem(dwSystemID, 
				&pSystem, 
				retsError,
				dwPlayerID, 
				&pPlayerObj))
			{
			m_GameFile.Close();
			m_Universe.Reinit();
			return error;
			}

		CShip *pPlayerShip = pPlayerObj->AsShip();
		if (pPlayerShip == NULL)
			{
			*retsError = CONSTLIT("Save file corruption: Player ship is invalid.");
			m_GameFile.Close();
			m_Universe.Reinit();
			return ERR_FAIL;
			}

		//	Set the player ship

		ASSERT(m_pPlayer == NULL);
		m_pPlayer = dynamic_cast<CPlayerShipController *>(pPlayerShip->GetController());
		if (m_pPlayer == NULL)
			{
			*retsError = CONSTLIT("Save file corruption: Player ship is invalid.");
			m_GameFile.Close();
			m_Universe.Reinit();
			return ERR_FAIL;
			}

		m_pPlayer->Init(g_pTrans);

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
			m_Universe.Reinit();
			return error;
			}

		//	Set the resurrect count

		m_pPlayer->SetResurrectCount(m_GameFile.GetResurrectCount());

		//	Connect the player ship controller to the controller

		CTranscendencePlayer *pPlayerController = dynamic_cast<CTranscendencePlayer *>(m_Universe.GetPlayer());
		if (pPlayerController == NULL)
			{
			*retsError = CONSTLIT("Save file corruption: No player controller found.");
			m_GameFile.Close();
			m_Universe.Reinit();
			return ERR_FAIL;
			}

		pPlayerController->SetPlayer(m_pPlayer);

		//	Set debug mode appropriately

		m_bDebugMode = m_Universe.InDebugMode();

		//	Log that we loaded a game

		kernelDebugLogMessage("Loaded game file version: %x", m_GameFile.GetCreateVersion());

		return NOERROR;
		}
	catch (...)
		{
		m_pPlayer = NULL;
		m_GameFile.Close();
		*retsError = CONSTLIT("Crash loading game.");

		return ERR_FAIL;
		}
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

ALERROR CTranscendenceModel::LoadUniverse (const CString &sCollectionFolder, const TArray<CString> &ExtensionFolders, CString *retsError)

//	LoadUniverse
//
//	Loads the universe

	{
	try
		{
		ALERROR error;

		//	Make sure the universe know about our various managers

		m_Universe.SetDebugMode(m_bDebugMode);
		m_Universe.SetSoundMgr(&m_HI.GetSoundMgr());

		//	Load the Transcendence Data Definition file that describes the universe.

		CUniverse::SInitDesc Ctx;
		Ctx.sCollectionFolder = sCollectionFolder;
		Ctx.ExtensionFolders = ExtensionFolders;
		Ctx.pHost = g_pTrans;
		Ctx.bDebugMode = m_bDebugMode;
		Ctx.dwAdventure = DEFAULT_ADVENTURE_EXTENSION_UNID;
		Ctx.bDefaultExtensions = true;
		Ctx.bForceTDB = m_bForceTDB;

		//	Add additional CodeChain primitives

		SPrimitiveDefTable *pNewTable = Ctx.CCPrimitives.Insert();
		m_HI.GetCodeChainPrimitives(pNewTable);

		pNewTable = Ctx.CCPrimitives.Insert();
		GetCodeChainExtensions(pNewTable);

		//	Initialize the universe

		if (error = m_Universe.Init(Ctx, retsError))
			return error;

		return NOERROR;
		}
	catch (...)
		{
		if (retsError)
			*retsError = CONSTLIT("Crash loading universe.");
		return ERR_FAIL;
		}
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
	DEBUG_TRY

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

	DEBUG_CATCH
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

	if (EnterScreenSession(m_pDock, NULL, DEFAULT_SCREEN_NAME, CString(), NULL) != NOERROR)
		{
		m_pPlayer->Undock();
		m_pDock = NULL;

		CString sError = strPatternSubst(CONSTLIT("[%s]: Unable to show dock screen"), m_pDock->GetType()->GetTypeName());
		g_pTrans->DisplayMessage(sError);
		::kernelDebugLogMessage(sError);
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

	//	Undock, if necessary

	if (InScreenSession())
		ExitScreenSession(true);

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

	//	Notify the controller

	m_HI.HICommand(CMD_GAME_ENTER_STARGATE, pDestNode);
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

	//	Add the player to the system

	m_Universe.PutPlayerInSystem(pShip, pStargate->GetPos(), m_TimerEvents);

	//	Welcome message

	g_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("Welcome to the %s system!"), pNewSystem->GetName()));

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
//
//	The function is designed to be called in the background and it will either
//	fire gameEnterFinalStargate or gameStargateSystemReady

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
		m_HI.HIPostCommand(CMD_GAME_ENTER_FINAL_STARGATE);
		return;
		}

	//	Clear all marks so that we only mark the new entries that we're about to
	//	load or create.

	m_Universe.ClearLibraryBitmapMarks();

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
				//kernelDebugLogMessage(sError);
				throw CException(ERR_FAIL);
				}
			}

		//	Otherwise, load the system

		else
			{
			CString sError;
			if (m_GameFile.LoadSystem(dwSystemID, &pNewSystem, &sError, OBJID_NULL, NULL, pShip) != NOERROR)
				{
				sError = strPatternSubst(CONSTLIT("%s [%s (%x)]"), sError, m_pDestNode->GetSystemName(), dwSystemID);

				g_pTrans->DisplayMessage(sError);
				kernelDebugLogMessage(sError);
				throw CException(ERR_FAIL);
				}
			}
		}

	//	Get the entry point. Note: We set the POV right away because Update
	//	cannot be called with a POV in a different system.

	CSpaceObject *pStart = pNewSystem->GetNamedObject(m_sDestEntryPoint);
	if (pStart == NULL)
		{
		kernelDebugLogMessage("Unable to find destination stargate %s in destination system.", m_sDestEntryPoint);
		throw CException(ERR_FAIL);
		}

	//	Let all types know that the current system is going away
	//	(Obviously, we need to call this before we change the current system.
	//	Note also that at this point the player is already gone.)

	m_Universe.GetDesignCollection().FireOnGlobalSystemStopped();

	//	Set the new system

	m_Universe.SetNewSystem(pNewSystem, pShip, pStart);

	//	Move any henchmen through the stargate (note: we do this here because
	//	we need to remove the henchmen out of the old system before we save).

	SetProgramState(psStargateTransferringGateFollowers);
	TransferGateFollowers(m_pOldSystem, pNewSystem, pStart);

	//	Let all types know that we have a new system. Again, this is called 
	//	before the player has entered the system.

	m_Universe.GetDesignCollection().FireOnGlobalSystemStarted();

	//	Garbage-collect images and load those for the new system

	SetProgramState(psStargateGarbageCollecting);
	m_Universe.MarkLibraryBitmaps();
	m_Universe.SweepLibraryBitmaps();

	//	Set the time that we stopped updating the system

	m_pOldSystem->SetLastUpdated();

	//	Save the old system.
	//	NOTE: From this point on the save file is invalid until we save the
	//	game at the end of OnPlayerExitedGate().
	//	FLAG_ENTER_GATE sets the save file in a state such that we can recover in case
	//	we crash before we save the rest of the file.

	SetProgramState(psStargateSavingSystem);
	if (m_GameFile.SaveSystem(m_pOldSystem->GetID(), m_pOldSystem, CGameFile::FLAG_ENTER_GATE) != NOERROR)
		kernelDebugLogMessage("Error saving system '%s' to game file", m_pOldSystem->GetName());

	//	Remove the old system

	SetProgramState(psStargateFlushingSystem);
	m_Universe.FlushStarSystem(m_pOldSystem->GetTopology());

	SetProgramState(psStargateEnterDone);

	//	Stargate effect

	pStart->OnObjLeaveGate(pShip);

	//	Done

	m_iState = statePlayerInGateNewSystem;
	m_HI.HIPostCommand(CMD_GAME_STARGATE_SYSTEM_READY);
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
	m_GameRecord.SetAdventureUNID(pAdventure->GetExtensionUNID());

	TArray<DWORD> Extensions;
	m_Universe.GetCurrentAdventureExtensions(&Extensions);
	m_GameRecord.SetExtensions(Extensions);

	m_GameRecord.SetPlayerName(m_pPlayer->GetPlayerName());
	m_GameRecord.SetPlayerGenome(m_pPlayer->GetPlayerGenome());

	m_GameRecord.SetShipClass(pPlayerShip ? pPlayerShip->GetType()->GetUNID() : 0);
	m_GameRecord.SetSystem(pPlayerShip ? pPlayerShip->GetSystem() : NULL);

	m_GameRecord.SetEndGameReason(sEndGameReason);
	m_GameRecord.SetEndGameEpitaph(sEpitaph);
	m_GameRecord.SetPlayTime(m_Universe.StopGameTime());

	m_GameRecord.SetRegistered(m_Universe.IsRegistered());
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

	m_Universe.SetLogImageLoad(false);
	pAdventure->FireOnGameEnd(m_GameRecord, BasicStats);
	m_Universe.SetLogImageLoad(true);

	//	Update the score in case it was changed inside OnGameEnd

	m_GameRecord.SetScore(m_pPlayer->GetEndGameScore());

	//	Add to high score if this is the default adventure

	if (pAdventure->GetExtensionUNID() == DEFAULT_ADVENTURE_EXTENSION_UNID)
		m_iLastHighScore = AddHighScore(m_GameRecord);
	else
		m_iLastHighScore = -1;
	}

void CTranscendenceModel::RefreshScreenSession (void)

//	RefreshScreenSession
//
//	Refreshes the current screen (running through OnScreenInit again).

	{
	ASSERT(!m_DockFrames.IsEmpty());

	//	Reload the screen (without stacking)

	//	bReturn = true (on ShowScreen) so that we don't restack a
	//	nested screen that we are returning to.
	//
	//	NOTE: We do not pass the current pane in because we want the screen to
	//	recalculate it (via InitialPane).

	const SDockFrame &Frame = m_DockFrames.GetCurrent();
	CString sError;
	ShowScreen(Frame.pRoot, Frame.sScreen, NULL_STR, Frame.pInitialData, &sError, true);
	}

ALERROR CTranscendenceModel::SaveGame (DWORD dwFlags, CString *retsError)

//	SaveGame
//
//	Saves the game to a file

	{
	ALERROR error;

	//	If this is a mission check point and we've already quit the game, then
	//	we can stop. This can happen if we end the game inside of <OnAcceptedUndock>

	if ((dwFlags & CGameFile::FLAG_ACCEPT_MISSION) 
			&& m_iState == statePlayerInEndGame)
		return NOERROR;

	//	If we're saving a mission, check the option and exit if we're not 
	//	supposed to save on mission accept.

	if ((dwFlags & CGameFile::FLAG_ACCEPT_MISSION) 
			&& m_bNoMissionCheckpoint)
		return NOERROR;

	//	Fire and event to give global types a chance to save any volatiles

	ASSERT(m_GameFile.IsOpen());
	m_Universe.FireOnGlobalUniverseSave();

	//	Generate and save game stats

	GenerateGameStats(&m_GameStats, false);
	if (error = SaveGameStats(m_GameStats))
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
		kernelDebugLogMessage("Error saving system '%s' to game file", pSystem->GetName());
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

ALERROR CTranscendenceModel::SaveGameStats (const CGameStats &Stats, bool bGameOver, bool bEndGame)

//	SaveGameStats
//
//	Saves stats to the current save file

	{
	ALERROR error;

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

	if (error = m_GameFile.SetGameStatus(iScore, sEpitaph, bEndGame))
		return error;

	//	Done

	return NOERROR;
	}

ALERROR CTranscendenceModel::ShowPane (const CString &sPane)

//	ShowPane
//
//	Shows the given pane

	{
	ASSERT(!m_DockFrames.IsEmpty());

	//	Update the frame

	m_DockFrames.SetCurrentPane(sPane);

	//	Show it

	g_pTrans->m_CurrentDock.ShowPane(sPane);

	//	Done

	return NOERROR;
	}

ALERROR CTranscendenceModel::ShowScreen (CDesignType *pRoot, const CString &sScreen, const CString &sPane, ICCItem *pData, CString *retsError, bool bReturn, bool bFirstFrame)

//	ShowScreen
//
//	Shows the given screen
//
//	If pRoot is NULL then we attempt to resolve it using sScreen
//	and m_pDefaultScreensRoot

	{
	ALERROR error;
	CCodeChain &CC = m_Universe.GetCC();

	ASSERT(!m_DockFrames.IsEmpty());

	//	If pRoot is NULL, then we have to look it up based on sScreen (and the local
	//	screens root)

	ICCItem *pDefaultData = NULL;
	CString sScreenActual;
	if (pRoot)
		sScreenActual = sScreen;
	else
		{
		if (!FindScreenRoot(sScreen, &pRoot, &sScreenActual, &pDefaultData))
			{
			*retsError = strPatternSubst(CONSTLIT("Unable to find global screen: %s"), sScreen);
			return ERR_FAIL;
			}

		if (pDefaultData && pData == NULL)
			pData = pDefaultData;

		//	NOTE: We have to discard pDefaultData regardless of whether we use 
		//	it or not.
		}

	//	The extension that the screen comes from is determined by where the root
	//	comes from.

	CExtension *pExtension = pRoot->GetExtension();

	//	If the root is a screen then use that

	CXMLElement *pScreen;
	CXMLElement *pLocalScreens = NULL;
	if (pRoot->GetType() == designDockScreen)
		pScreen = CDockScreenType::AsType(pRoot)->GetDesc();
	else
		{
		pLocalScreens = pRoot->GetLocalScreens();
		if (pLocalScreens == NULL)
			{
			if (pDefaultData)
				pDefaultData->Discard(&CC);
			*retsError = CONSTLIT("No local screens");
			return ERR_FAIL;
			}

		pScreen = pLocalScreens->GetContentElementByTag(sScreenActual);
		if (pScreen == NULL)
			{
			if (pDefaultData)
				pDefaultData->Discard(&CC);
			*retsError = strPatternSubst(CONSTLIT("Unable to find local screen: %s"), sScreenActual);
			return ERR_FAIL;
			}
		}

	const SDockFrame &CurFrame = m_DockFrames.GetCurrent();

	//	See if we need to nest this screen. Sometimes we rely on the fact that 
	//	a redirected screen (via <GetGlobalDockScreen>) is nested so that when
	//	we're done we can go back to the station's original screen.
	//
	//	But other times, if we bring up a nested screen directly via a command
	//	(e.g., from invoking or something) then we want to ignore the nesting
	//	directive (otherwise we just return to the same screen).
	//
	//	This code figures out whether we're about to nest on ourselves.

	bool bNestedScreen = pScreen->GetAttributeBool(NESTED_SCREEN_ATTRIB);
	if (bNestedScreen 
			&& bFirstFrame
			&& CurFrame.sScreen == sScreenActual
			&& CurFrame.sPane == sPane)
		{
		bNestedScreen = false;
		}

	//	Compose the new frame

	SDockFrame NewFrame;
	NewFrame.pLocation = CurFrame.pLocation;
	NewFrame.pRoot = pRoot;
	NewFrame.sScreen = sScreenActual;
	NewFrame.sPane = sPane;
	NewFrame.pInitialData = pData;
	NewFrame.pResolvedRoot = pRoot;
	NewFrame.sResolvedScreen = sScreenActual;

	//	Some screens pop us into a new frame

	bool bNewFrame;
	SDockFrame OldFrame;
	if (bNewFrame = (!bReturn && bNestedScreen))
		m_DockFrames.Push(NewFrame);
	else if (!bReturn)
		m_DockFrames.SetCurrent(NewFrame, &OldFrame);
	else
		m_DockFrames.ResolveCurrent(NewFrame);

	//	Show the screen
	//
	//	NOTE: This call can recurse (because we can call ShowScreen
	//	again from inside <OnScreenInit>). In case of recursion, the
	//	deepest call will set up sNewPane while the others will
	//	return NULL_STR.

	CString sNewPane;

	m_Universe.SetLogImageLoad(false);
	error = g_pTrans->m_CurrentDock.InitScreen(m_HI.GetHWND(),
			g_pTrans->m_rcMainScreen,
			NewFrame,
			pExtension,
			pScreen,
			sPane,
			pData,
			&sNewPane,
			&g_pTrans->m_pCurrentScreen);
	m_Universe.SetLogImageLoad(true);

	//	We no longer need pDefaultData. Anyone who needed it took out a 
	//	reference.

	if (pDefaultData)
		{
		pDefaultData->Discard(&CC);
		pDefaultData = NULL;
		}

	//	Handle errors

	if (error)
		{
		//	Undo

		if (bNewFrame)
			m_DockFrames.Pop();
		else if (!bReturn)
			{
			m_DockFrames.SetCurrent(OldFrame);
			m_DockFrames.DiscardOldFrame(OldFrame);
			}

		*retsError = CONSTLIT("InitScreen failed.");
		return error;
		}

	//	Clean up the old frames (which may contain a CodeChain item)

	if (!bNewFrame && !bReturn)
		m_DockFrames.DiscardOldFrame(OldFrame);

	//	If no frames then we exited inside of InitScreen

	if (m_DockFrames.IsEmpty())
		return NOERROR;

	//	Update frame

	if (!sNewPane.IsBlank())
		m_DockFrames.SetCurrentPane(sNewPane);

	return NOERROR;
	}

void CTranscendenceModel::ShowShipScreen (void)

//	ShowShipScreen
//
//	Show the ship screen specified in CPlayerSettings

	{
	//	Make sure we have a ship.

	CShip *pShip = m_pPlayer->GetShip();
	if (pShip == NULL)
		return;

	const CPlayerSettings *pSettings = pShip->GetClass()->GetPlayerSettings();
	if (pSettings->GetShipScreen() == NULL)
		return;

	CString sScreen;
	CDesignType *pRoot = pSettings->GetShipScreen().GetDockScreen(pShip->GetClass(), &sScreen);

	CString sError;
	if (!ShowShipScreen(NULL, pRoot, sScreen, NULL_STR, NULL, &sError))
		{
		g_pTrans->DisplayMessage(sError);
		::kernelDebugLogMessage(sError);
		return;
		}

	pShip->OnComponentChanged(comCargo);
	}

bool CTranscendenceModel::ShowShipScreen (CDesignType *pDefaultScreensRoot, CDesignType *pRoot, const CString &sScreen, const CString &sPane, ICCItem *pData, CString *retsError)

//	ShowShipScreen
//
//	Show a ship dock screen
//
//	If pRoot is NULL then we attempt to resolve sScreen using pDefaultScreensRoot
//	if necessary.

	{
	//	If we're not in the middle of a game, then we fail (this can happen if
	//	we try to bring up a dock screen while in the middle of gating).

	if (m_iState != stateInGame)
		{
		*retsError = CONSTLIT("Unable to display screen while not in system.");
		return false;
		}

	CShip *pShip = m_pPlayer->GetShip();
	if (pShip == NULL)
		{
		*retsError = CONSTLIT("Unable to find player ship.");
		return false;
		}

	//	If the default root is passed in, use that. Otherwise, we pull local
	//	screens from the ship class.

	m_pDefaultScreensRoot = pDefaultScreensRoot;
	if (m_pDefaultScreensRoot == NULL)
		m_pDefaultScreensRoot = pShip->GetType();

	//	Show screen

	if (EnterScreenSession(pShip, pRoot, sScreen, sPane, pData) != NOERROR)
		{
		*retsError = strPatternSubst(CONSTLIT("[%s]: Unable to show screen %s"), pRoot->GetTypeName(), sScreen);
		return false;
		}

	return true;
	}

ALERROR CTranscendenceModel::StartGame (bool bNewGame)

//	StartGame
//
//	This is called after either creating a new game or loading an old one

	{
	//	Tell the universe to focus on the ship

	m_Universe.SetPlayerShip(m_pPlayer->GetShip());
	m_Universe.SetPOV(m_pPlayer->GetShip());

	//	Set sound

	if (m_bNoSound)
		m_Universe.SetSound(false);

	//	Tell the controller that we're starting

	m_pPlayer->OnStartGame();

	//	Clear out m_pCrawImage since the call to StartGame is going to clobber
	//	it (when it sweeps unused images).

	m_pCrawlImage = NULL;
	m_pCrawlSoundtrack = NULL;

	//	Tell the universe to start the game

	m_Universe.StartGame(m_iState == stateCreatingNewGame);

	//	Update stats

	if (m_iState == stateCreatingNewGame)
		m_pPlayer->OnSystemEntered(m_Universe.GetCurrentSystem());

	//	Done

	m_iState = stateInGame;

	//	If this is a new game, save it so that we can go back to the start.

	if (bNewGame)
		SaveGame(CGameFile::FLAG_CHECKPOINT);

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

	CString sFilename = pathAddComponent(GetSaveFilePath(), m_GameFile.GenerateFilename(NewGame.sPlayerName));
	m_GameFile.Create(sFilename, sUsername);

	//	If we're signed in and if the adventure/extension combination is
	//	registered, then we have a registered game.

	if (!sUsername.IsBlank() && !m_Universe.InDebugMode() && m_Universe.GetDesignCollection().IsRegisteredGame())
		m_Universe.SetRegistered(true);

	//	Create a controller for the player's ship (this is owned
	//	by the ship once we pass it to CreateShip)

	ASSERT(m_pPlayer == NULL);
	m_pPlayer = new CPlayerShipController;
	if (m_pPlayer == NULL)
		{
		*retsError = CONSTLIT("Unable to create CPlayerShipController");
		m_Universe.Reinit();
		m_GameFile.Close();
		return ERR_MEMORY;
		}

	m_pPlayer->Init(g_pTrans);
	m_pPlayer->SetName(NewGame.sPlayerName);
	m_pPlayer->SetGenome(NewGame.iPlayerGenome);
	m_pPlayer->SetStartingShipClass(NewGame.dwPlayerShip);

	//	Inside of InitAdventure we may get called back to set the crawl image
	//	and text.

	m_pCrawlImage = NULL;
	m_pCrawlSoundtrack = NULL;
	m_sCrawlText = NULL_STR;

	//	Create the player controller

	CTranscendencePlayer *pPlayerController = new CTranscendencePlayer;
	pPlayerController->SetPlayer(m_pPlayer);

	//	Initialize the adventure and hand it the player controller.

	if (m_Universe.InitAdventure(pPlayerController, retsError) != NOERROR)
		return ERR_FAIL;

	//	The remainder of new game start happens in the background thread
	//	in StartNewGamebackground

	m_iState = stateCreatingNewGame;

	return NOERROR;
	}

void CTranscendenceModel::StartNewGameAbort (void)

//	StartNewGameAbort
//
//	Handle error after StartNewGame.

	{
	if (m_pPlayer)
		{
		delete m_pPlayer;
		m_pPlayer = NULL;
		}

	m_GameFile.Close();
	}

ALERROR CTranscendenceModel::StartNewGameBackground (const SNewGameSettings &NewGame, CString *retsError)

//	StartNewGameBackground
//
//	Starts a new game.
//
//	NOTE: We need to be very careful about what we do in the main thread while
//	this is running.

	{
	ALERROR error;

	ASSERT(m_iState == stateCreatingNewGame);
	ASSERT(m_pPlayer);

	::kernelDebugLogMessage("Starting new game.");

	//	Figure out the ship class that we want

	CShipClass *pStartingShip = m_Universe.FindShipClass(m_pPlayer->GetStartingShipClass());
	if (pStartingShip == NULL)
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to find player ship class: %x."), m_pPlayer->GetStartingShipClass());
		return ERR_FAIL;
		}

	const CPlayerSettings *pPlayerSettings = pStartingShip->GetPlayerSettings();
	if (pPlayerSettings == NULL)
		{
		*retsError = CONSTLIT("Missing <PlayerSettings> for ship class");
		return ERR_FAIL;
		}

	DWORD dwStartMap;
	CString sStartNode;
	CString sStartPos;
	CalcStartingPos(pStartingShip, &dwStartMap, &sStartNode, &sStartPos);

	//	Initialize topology, etc.

	if (error = m_Universe.InitGame(dwStartMap, retsError))
		return error;

	//	Get the starting system

	CSystem *pStartingSystem;
	if (NewGame.bFullCreate)
		{
		if (error = CreateAllSystems(sStartNode, &pStartingSystem, retsError))
			return error;
		}
	else
		{
		if (error = m_Universe.CreateStarSystem(sStartNode, &pStartingSystem, retsError))
			return error;
		}

	//	Figure out where in the system we want to start

	CVector vStartPos;
	CSpaceObject *pStart = pStartingSystem->GetNamedObject(sStartPos);
	if (pStart)
		vStartPos = pStart->GetPos();

	//	Set some credits

	const CCurrencyAndRange &StartingCredits = pPlayerSettings->GetStartingCredits();
	m_pPlayer->Payment(StartingCredits.GetCurrencyType()->GetUNID(), StartingCredits.Roll());

	//	Create the player's ship

	CShip *pPlayerShip;

	m_Universe.SetLogImageLoad(false);
	error = pStartingSystem->CreateShip(m_pPlayer->GetStartingShipClass(),
			m_pPlayer,
			NULL,
			m_Universe.FindSovereign(g_PlayerSovereignUNID),
			vStartPos,
			NullVector,
			90,
			NULL,
			NULL,
			&pPlayerShip);
	m_Universe.SetLogImageLoad(true);

	if (error)
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
		for (int i = 0; i < m_Universe.GetItemTypeCount(); i++)
			{
			CItemType *pType = m_Universe.GetItemType(i);

			//	Do not count unknown items

			if (pType->HasLiteralAttribute(CONSTLIT("unknown")))
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
			if (error = pStartingSystem->CreateShip(g_DebugHenchmenShipUNID,
						pController,
						NULL,
						m_Universe.FindSovereign(g_PlayerSovereignUNID),
						vStartPos + PolarToVector(0, g_KlicksPerPixel * 100.0),
						NullVector,
						0,
						NULL,
						&pHenchman))
				{
				*retsError = CONSTLIT("Unable to create henchmen");
				return error;
				}

			pHenchman->GetController()->AddOrder(IShipController::orderEscort, pPlayerShip, IShipController::SData(i));
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
		GateFollowerObjs[i]->NotifyOnNewSystem(pSystem);

	//	Done

	m_GateFollowers.DeleteAll();
	}

void CTranscendenceModel::UseItem (CItem &Item)

//	UseItem
//
//	Use item

	{
	CShip *pShip = m_pPlayer->GetShip();
	CItemList &ItemList = pShip->GetItemList();
	CItemType *pType = Item.GetType();

	CItemType::SUseDesc UseDesc;
	if (!pType->GetUseDesc(&UseDesc))
		return;

	//	Use in cockpit

	if (UseDesc.bUsableInCockpit)
		{
		CString sError;

		//	Run the invoke script

		pShip->UseItem(Item, &sError);

		//	Done

		if (!sError.IsBlank())
			{
			pShip->SendMessage(NULL, sError);
			::kernelDebugLogMessage(sError);
			}

		pShip->OnComponentChanged(comCargo);
		}

	//	Use screen

	else if (UseDesc.pScreenRoot)
		{
		CCodeChain &CC = m_Universe.GetCC();

		ICCItem *pItem = CreateListFromItem(CC, Item);
		CC.DefineGlobal(CONSTLIT("gItem"), pItem);
		pItem->Discard(&CC);

		//	Show the dock screen

		CString sError;
		if (!ShowShipScreen(pType, UseDesc.pScreenRoot, UseDesc.sScreenName, NULL_STR, NULL, &sError))
			{
			pShip->SendMessage(NULL, sError);
			::kernelDebugLogMessage(sError);
			}

		pShip->OnComponentChanged(comCargo);
		}
	}
