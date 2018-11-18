//	CIntroSession.cpp
//
//	CIntroSession class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define ATTRIB_GENERIC_SHIP_CLASS				CONSTLIT("genericClass")

#define CMD_ACCOUNT								CONSTLIT("cmdAccount")
#define CMD_ACCOUNT_EDIT						CONSTLIT("cmdAccountEdit")
#define CMD_CHANGE_PASSWORD						CONSTLIT("cmdChangePassword")
#define CMD_OPEN_NEWS							CONSTLIT("cmdOpenNews")
#define CMD_SHOW_MOD_EXCHANGE					CONSTLIT("cmdShowModExchange")
#define CMD_SHOW_PROFILE						CONSTLIT("cmdShowProfile")
#define CMD_SHOW_SETTINGS						CONSTLIT("cmdShowSettings")
#define CMD_SIGN_OUT							CONSTLIT("cmdSignOut")
#define CMD_SOUNDTRACK_NOW_PLAYING				CONSTLIT("cmdSoundtrackNowPlaying")
#define CMD_TOGGLE_DEBUG						CONSTLIT("cmdToggleDebug")
#define CMD_TOGGLE_MUSIC						CONSTLIT("cmdToggleMusic")
#define CMD_UPDATE_HIGH_SCORE_LIST				CONSTLIT("cmdUpdateHighScoreList")

#define CMD_INTRO_SHIP_DESTROYED				CONSTLIT("introShipDestroyed")

#define CMD_SERVICE_HIGH_SCORE_LIST_ERROR		CONSTLIT("serviceHighScoreListError")
#define CMD_SERVICE_HIGH_SCORE_LIST_LOADED		CONSTLIT("serviceHighScoreListLoaded")
#define CMD_SERVICE_NEWS_LOADED					CONSTLIT("serviceNewsLoaded")

#define CMD_UI_CHANGE_PASSWORD					CONSTLIT("uiChangePassword")
#define CMD_UI_SHOW_LOGIN						CONSTLIT("uiShowLogin")
#define CMD_UI_SHOW_MOD_EXCHANGE				CONSTLIT("uiShowModExchange")
#define CMD_UI_SHOW_PROFILE						CONSTLIT("uiShowProfile")
#define CMD_UI_SHOW_SETTINGS					CONSTLIT("uiShowSettings")
#define CMD_UI_SIGN_OUT							CONSTLIT("uiSignOut")

#define ID_GAME_STAT_PERFORMANCE				CONSTLIT("gameStatPerformance")
#define ID_GAME_STAT_SCROLLER					CONSTLIT("gsScroller")
#define ID_GAME_STAT_FADER						CONSTLIT("gsFader")
#define ID_GAME_STAT_SELECT_RECT				CONSTLIT("gsSelRect")
#define ID_HIGH_SCORES_ANIMATOR					CONSTLIT("hsAnimator")
#define ID_HIGH_SCORES_SELECT_RECT				CONSTLIT("hsSelRect")
#define ID_ACCOUNT_CONTROLS						CONSTLIT("idAccount")
#define ID_CREDITS_PERFORMANCE					CONSTLIT("idCredits")
#define ID_END_GAME_PERFORMANCE					CONSTLIT("idEndGame")
#define ID_HIGH_SCORES_PERFORMANCE				CONSTLIT("idHighScores")
#define ID_INTRO_HELP_PERFORMANCE				CONSTLIT("idIntroHelp")
#define ID_PLAYER_BAR_PERFORMANCE				CONSTLIT("idPlayerBar")
#define ID_SHIP_DESC_PERFORMANCE				CONSTLIT("idShipDescPerformance")
#define ID_TITLES_PERFORMANCE					CONSTLIT("idTitles")
#define ID_NEWS_PERFORMANCE						CONSTLIT("idNews")
#define ID_SOUNDTRACK_TITLE_PERFORMANCE			CONSTLIT("idSoundtrackTitle")

#define OBJ_DATA_INTRO_CONTROLLER				CONSTLIT("IntroController")

#define PROP_POSITION							CONSTLIT("position")

#define PROPERTY_PRIMARY_WEAPON					CONSTLIT("primaryWeapon")

const int INTRO_DISPLAY_HEIGHT =				512;
const int TITLE_BAR_HEIGHT =					128;

const int MAX_INTRO_SHIPS =						500;
const int MAX_TIME_WITH_ONE_SHIP =				(g_TicksPerSecond * 90);
const int EXPAND_ON_IDLE_TIME =                 900;

const CG32bitPixel RGB_FRAME =					CG32bitPixel(80,80,80);

CIntroSession::~CIntroSession (void)

//	CIntroSession destructor

	{
	}

void CIntroSession::CancelCurrentState (void)

//	CancelCurrentState
//
//	Cancels the current state and moves to a new appropriate one.

	{
	switch (GetState())
		{
		case isIntroHelp:
		case isCredits:
		case isHighScores:
		case isBlankThenRandom:
		case isNews:
		case isWaitingForHighScores:
			SetState(isShipStats);
			break;

		case isEndGame:
			if (m_HighScoreDisplay.HasHighScores())
				SetState(isHighScoresEndGame);
			else
				SetState(isCredits);
			break;

		case isEnterCommand:
			SetState(isShipStats);
			break;

		case isHighScoresEndGame:
			SetState(isCredits);
			break;

		case isOpeningTitles:
			SetState(isShipStats);
			break;

		case isShipStats:
			SetState(isBlankThenRandom);
			break;
		}
	}

void CIntroSession::CmdShowHighScoreList (DWORD dwAdventure, const CString &sUsername, int iScore)

//	CmdShowHighScoreList
//
//	Shows the high score list.

	{
	//	If we're already showing high scores, then stop

	if (GetState() == isHighScores)
		{
		SetState(isBlank);
		return;
		}

	//	If we're already waiting for high scores, keep waiting

	if (GetState() == isWaitingForHighScores)
		return;

	//	If adventure is 0, pick a random adventure

	if (dwAdventure == 0)
		dwAdventure = DEFAULT_ADVENTURE_EXTENSION_UNID;

	//	Get high scores from the server (and remember our state so that we know
	//	what to do when we get back).

	CAdventureHighScoreList::SSelect Select;
	Select.dwAdventure = dwAdventure;
	Select.sUsername = sUsername;
	Select.iScore = iScore;

	SetState(isWaitingForHighScores);
	m_HI.HICommand(CMD_UPDATE_HIGH_SCORE_LIST, &Select);

	//	When we get a response from the server, we'll get sent a serviceHighScoreListLoaded
	//	command.
	}

void CIntroSession::CreateIntroShips (DWORD dwNewShipClass, DWORD dwSovereign, CSpaceObject *pShipDestroyed)

//	CreateIntroShips
//
//	Makes sure that there are enough ships and that everyone has a target

	{
	DEBUG_TRY

	int i;
	CSystem *pSystem = g_pUniverse->GetCurrentSystem();

	//	Make sure each sovereign has a ship

	CSovereign *pSovereign1 = g_pUniverse->FindSovereign(g_PlayerSovereignUNID);
	CSovereign *pSovereign2 = g_pUniverse->FindSovereign(UNID_UNKNOWN_ENEMY);
	CShip *pShip1 = NULL;
	CShip *pShip2 = NULL;

	//	Sovereign of POV

	DWORD dwCurSovereign;
	if (dwSovereign)
		dwCurSovereign = dwSovereign;
	else if (g_pUniverse->GetPOV())
		dwCurSovereign = g_pUniverse->GetPOV()->GetSovereign()->GetUNID();
	else
		dwCurSovereign = g_PlayerSovereignUNID;

	//	Look for the surviving ships

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj
				&& pObj->GetCategory() == CSpaceObject::catShip
				&& !pObj->IsDestroyed()
				&& pObj != pShipDestroyed
				&& pObj->CanAttack()
				&& !pObj->GetData(OBJ_DATA_INTRO_CONTROLLER)->IsNil())
			{
			if (pObj->GetSovereign() == pSovereign1)
				{
				if (pShip1 == NULL)
					pShip1 = pObj->AsShip();
				}
			else if (pObj->GetSovereign() == pSovereign2)
				{
				if (pShip2 == NULL)
					pShip2 = pObj->AsShip();
				}
			}
		}

	ASSERT(pShip1 == NULL || !pShip1->IsDestroyed());
	ASSERT(pShip2 == NULL || !pShip2->IsDestroyed());

	//	Create ships if necessary

	if (pShip1 == NULL || (dwNewShipClass && dwCurSovereign == g_PlayerSovereignUNID))
		{
		if (CreateRandomShip(pSystem, (dwCurSovereign == g_PlayerSovereignUNID ? dwNewShipClass : 0), pSovereign1, &pShip1) != NOERROR)
			return;
		}

	if (pShip2 == NULL || (dwNewShipClass && dwCurSovereign == UNID_UNKNOWN_ENEMY))
		{
		if (CreateRandomShip(pSystem, (dwCurSovereign == UNID_UNKNOWN_ENEMY ? dwNewShipClass : 0), pSovereign2, &pShip2) != NOERROR)
			return;
		}

	//	Make sure every ship has an order to attack someone

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj
				&& pObj->GetCategory() == CSpaceObject::catShip
				&& !pObj->IsDestroyed()
				&& pObj->CanAttack()
				&& pObj != pShipDestroyed)
			{
			CShip *pShip = pObj->AsShip();
			if (pShip)
				{
				IShipController *pController = pShip->GetController();

				CSpaceObject *pTarget;
				IShipController::OrderTypes iOrder = pController->GetCurrentOrderEx(&pTarget);
				if ((pShipDestroyed && pTarget == pShipDestroyed) || iOrder == IShipController::orderNone)
					{
					if (pShip->GetSovereign() == pSovereign1)
						OrderAttack(pShip, pShip2);
					else
						OrderAttack(pShip, pShip1);
					}
				}
			}
		}

	//	Chance the POV if necessary

	if (g_pUniverse->GetPOV() == pShipDestroyed 
			|| g_pUniverse->GetPOV()->IsDestroyed()
			|| g_pUniverse->GetPOV()->AsShip() == NULL
			|| dwNewShipClass != 0)
		{
		//	Pick a POV of the same sovereign

		if (dwCurSovereign == g_PlayerSovereignUNID)
			g_pUniverse->SetPOV(pShip1);
		else
			g_pUniverse->SetPOV(pShip2);

#ifdef DEBUG_COMBAT
		pShip2->SetSelection();
#endif
		OnPOVSet(g_pUniverse->GetPOV());
		}

	//	Mark and sweep, so we don't run out of memory

	g_pUniverse->ClearLibraryBitmapMarks();
	g_pUniverse->MarkLibraryBitmaps();
	g_pUniverse->SweepLibraryBitmaps();

	g_pTrans->m_iLastShipCreated = g_pTrans->m_iTick;

	DEBUG_CATCH
	}

void CIntroSession::CreateIntroSystem (void)

//	CreateIntroSystem
//
//	Creates the intro system.

	{
	ALERROR error;
	int i;

	//	Create an empty system

	if (error = g_pUniverse->CreateEmptyStarSystem(&g_pTrans->m_pIntroSystem))
		{
		ASSERT(false);
		return;
		}

	g_pUniverse->SetCurrentSystem(g_pTrans->m_pIntroSystem);

	//	Let types control what happens on the intro screen

	g_pUniverse->FireOnGlobalIntroStarted();

	//	If the POV was set during OnGlobalIntroStarted, then we are done. Otherwise, initialize as normal
	
	if (!g_pUniverse->GetPOV())
		{
		CSovereign *pSovereign1 = g_pUniverse->FindSovereign(g_PlayerSovereignUNID);
		CSovereign *pSovereign2 = g_pUniverse->FindSovereign(UNID_UNKNOWN_ENEMY);

		//	Create a couple of random enemy ships

		CShip *pShip1;
		CShip *pShip2;
		if (error = CreateRandomShip(g_pTrans->m_pIntroSystem, 0, pSovereign1, &pShip1))
			return;

		if (error = CreateRandomShip(g_pTrans->m_pIntroSystem, 0, pSovereign2, &pShip2))
			return;

		//	Make the ships attack each other

		for (i = 0; i < g_pTrans->m_pIntroSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = g_pTrans->m_pIntroSystem->GetObject(i);

			if (pObj
					&& pObj->GetCategory() == CSpaceObject::catShip
					&& pObj->CanAttack()
					&& !pObj->GetData(OBJ_DATA_INTRO_CONTROLLER)->IsNil())
				{
				CShip *pShip = pObj->AsShip();
				if (pShip)
					{
					if (pShip->GetSovereign() == pSovereign1)
						OrderAttack(pShip, pShip2);
					else
						OrderAttack(pShip, pShip1);
					}
				}
			}

		//	Set the POV to one of them

		g_pUniverse->SetPOV(pShip1);
		}

	//	No sound

	g_pUniverse->SetSound(false);

	g_pTrans->m_iTick = 0;
	g_pTrans->m_iLastShipCreated = g_pTrans->m_iTick;

	//	Initialize the system

	g_pUniverse->MarkLibraryBitmaps();
	}

void CIntroSession::InitShipTable (TSortMap<int, CShipClass *> &List, bool bAll)

//	InitShipTable
//
//	Initializes the table with ships

	{
	int i;

	if (List.GetCount() == 0)
		{
		for (i = 0; i < g_pUniverse->GetShipClassCount(); i++)
			{
			CShipClass *pClass = g_pUniverse->GetShipClass(i);

			//	Skip classes that we don't want in the intro

			if (pClass->IsVirtual()
					|| (!bAll && !pClass->HasLiteralAttribute(ATTRIB_GENERIC_SHIP_CLASS)))
				continue;

			//	Skip unarmed ships

			if (pClass->GetPropertyString(PROPERTY_PRIMARY_WEAPON).IsBlank())
				continue;

			//	Add to our list, sorted by score

			List.Insert(pClass->GetScore(), pClass);
			}
		}
	}

ALERROR CIntroSession::CreateRandomShip (CSystem *pSystem, DWORD dwClass, CSovereign *pSovereign, CShip **retpShip)

//	CreateRandomShip
//
//	Creates a random ship

	{
	DEBUG_TRY

	ALERROR error;
	int i;

	//	Figure out the class. If we've got a class passed in, use that.

	CShipClass *pShipClass = NULL;
	if (dwClass != 0
			&& (pShipClass = g_pUniverse->FindShipClass(dwClass)) != NULL)
		{ }

	//	Otherwise, we pick a random ship

	else
		{
		//	If necessary, build our table of ships.

		if (m_ShipList.GetCount() == 0)
			{
			InitShipTable(m_ShipList);

			//	If no ships in our list, then try again but include all ships (not 
			//	just the ones with genericClass).

			if (m_ShipList.GetCount() == 0)
				{
				InitShipTable(m_ShipList, true);

				//	If we still don't have any ships, then we can't do anything. This 
				//	should never happen because we have ships in CoreTypes.

				if (m_ShipList.GetCount() == 0)
					return ERR_FAIL;
				}
			}

		//	We only pick from the bottom half of the list, on the assumption that
		//	we want the higher level ships to be a surprise for the player.

		int iMax = (m_bShowAllShips ? m_ShipList.GetCount() - 1 : m_ShipList.GetCount() / 2);
		pShipClass = m_ShipList[mathRandom(0, iMax)];
		}

	//	Normally we create a single ship, but sometimes we create lots

	int iCount;
	int iRoll = mathRandom(1, 100);

	//	Adjust the roll for capital ships

	if (pShipClass->GetHullDesc().GetMass() >= 10000)
		iRoll -= 9;
	else if (pShipClass->GetHullDesc().GetMass() >= 1000)
		iRoll -= 6;

	if (iRoll == 100)
		iCount = mathRandom(30, 60);
	else if (iRoll >= 98)
		iCount = mathRandom(10, 20);
	else if (iRoll >= 95)
		iCount = mathRandom(5, 10);
	else if (iRoll >= 90)
		iCount = mathRandom(2, 5);
	else
		iCount = 1;

	//	Create the ships

	g_pUniverse->SetLogImageLoad(false);

	for (i = 0; i < iCount; i++)
		{
		CShip *pShip;

		if (error = pSystem->CreateShip(pShipClass->GetUNID(),
				NULL,
				NULL,
				pSovereign,
				PolarToVector(mathRandom(0, 359), mathRandom(250, 2500) * g_KlicksPerPixel),
				NullVector,
				mathRandom(0, 359),
				NULL,
				NULL,
				&pShip))
			{
			g_pUniverse->SetLogImageLoad(true);
			return error;
			}

		//	Override the controller

		CIntroShipController *pNewController = new CIntroShipController(pShip->GetController());
		pShip->SetController(pNewController, false);
		pNewController->SetShip(pShip);
		pShip->SetData(OBJ_DATA_INTRO_CONTROLLER, ICCItemPtr(g_pUniverse->GetCC().CreateTrue()));

		*retpShip = pShip;
		}

	g_pUniverse->SetLogImageLoad(true);

	return NOERROR;

	DEBUG_CATCH
	}

void CIntroSession::CreateSoundtrackTitleAnimation (CMusicResource *pTrack, IAnimatron **retpAni)

//	CreateSoundtrackTitleAnimation
//
//	Creates an animation for soundtrack titles

	{
	ASSERT(pTrack);

	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &TitleFont = VI.GetFont(fontSubTitle);
	const CG16bitFont &BodyFont = VI.GetFont(fontHeader);

	//	Position to the bottom-left of the main rect.

	RECT rcCenter;
	VI.GetWidescreenRect(&rcCenter);

	int x = rcCenter.left;
	int cy = TitleFont.GetHeight() + 2 * BodyFont.GetHeight();
	int cySpacing = 2 * BodyFont.GetHeight();
	int y = m_rcMain.bottom - (cy + cySpacing);

	//	Animate

	int iDuration = 300;
	int iInterLineDelay = 1;
	int iDelay = 0;

	//	Create sequencer to hold everything.

	CAniSequencer *pSeq = new CAniSequencer;

	//	Show the title

	IAnimatron *pText;
	CAniText::Create(pTrack->GetTitle(),
			CVector((Metric)x, (Metric)y),
			&TitleFont,
			0,
			VI.GetColor(colorTextHighlight),
			&pText);
	pText->AnimateLinearFade(iDuration, 0, 30);
	pSeq->AddTrack(pText, iDelay);

	y += TitleFont.GetHeight();
	iDelay += iInterLineDelay * 3;

	//	Show the performer

	CAniText::Create(pTrack->GetPerformedBy(),
			CVector((Metric)x, (Metric)y),
			&BodyFont,
			0,
			VI.GetColor(colorTextDialogLabel),
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	y += BodyFont.GetHeight();
	iDelay += iInterLineDelay;

	//	Show the extension name

	CAniText::Create(pTrack->GetAlbum(),
			CVector((Metric)x, (Metric)y),
			&BodyFont,
			0,
			VI.GetColor(colorTextDialogLabel),
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	//	Done

	*retpAni = pSeq;
	}

bool CIntroSession::HandleCommandBoxChar (char chChar, DWORD dwKeyData)

//	HandleCommandBoxChar
//
//	Handle key commands when inputting commands

	{
	//	If not in command mode, then we're done

	if (GetState() != isEnterCommand)
		return false;

	//	Handle it

	switch (chChar)
		{
		//	VK_BACKSPACE

		case '\010':
			if (!g_pTrans->m_sCommand.IsBlank())
				g_pTrans->m_sCommand = strSubString(g_pTrans->m_sCommand, 0, g_pTrans->m_sCommand.GetLength() - 1);
			break;

		//	VK_RETURN

		case '\015':
			{
			if (strStartsWith(g_pTrans->m_sCommand, CONSTLIT("~")))
				{
				g_pUniverse->FireOnGlobalIntroCommand(strSubString(g_pTrans->m_sCommand, 1));
				CancelCurrentState();
				break;
				}

			CShip *pShip = g_pUniverse->GetPOV()->AsShip();
			DWORD dwSovereign = (pShip ? pShip->GetSovereign()->GetUNID() : 0);

			//	Parse the string into a ship class

			CShipClass *pClass = g_pUniverse->FindShipClassByName(g_pTrans->m_sCommand);
			if (pClass == NULL)
				{
				SetState(isShipStats);
				break;
				}

			//	Destroy and create

			g_pTrans->DestroyPOVIntroShips();
			CreateIntroShips(pClass->GetUNID(), dwSovereign);
			CancelCurrentState();
			break;
			}

		//	VK_ESCAPE

		case '\033':
			CancelCurrentState();
			break;

		default:
			if (chChar >= ' ')
				g_pTrans->m_sCommand.Append(CString(&chChar, 1));
			break;
		}

	return true;
	}

bool CIntroSession::HandleChar (char chChar, DWORD dwKeyData)

//	HandleChar
//
//	Handle key commands

	{
	switch (chChar)
		{
		case ' ':
			CancelCurrentState();
			break;

		case '!':
			SetState(isEnterCommand);
			break;

		case 'C':
		case 'c':
			SetState(isCredits);
			break;

		case 'D':
		case 'd':
			{
			int i;

			//	Get the UNID of the current ship

			CShip *pShip = g_pUniverse->GetPOV()->AsShip();
			if (pShip == NULL)
				break;

			//	Count the number of ships in the system

			CSystem *pSystem = pShip->GetSystem();
			int iCount = 0;
			for (i = 0; i < pSystem->GetObjectCount(); i++)
				{
				CSpaceObject *pObj = pSystem->GetObject(i);
				if (pObj 
						&& pObj->GetCategory() == CSpaceObject::catShip
						&& pObj->CanAttack())
					iCount++;
				}

			//	If we already have too many, we don't do anything

			if (iCount > MAX_INTRO_SHIPS)
				break;

			//	Create a duplicate

			CreateIntroShips(pShip->GetClassUNID(), pShip->GetSovereign()->GetUNID());
			break;
			}

		case 'h':
		case 'H':
			CmdShowHighScoreList();
			break;

		case 'I':
		case 'i':
			SetState(isIntroHelp);
			break;

		case 'K':
			g_pTrans->DestroyAllIntroShips();
			CreateIntroShips();
			break;
		case 'k':
			g_pTrans->DestroyPOVIntroShips();
			CreateIntroShips();
			break;

		case 'L':
		case 'l':
			g_pTrans->DoCommand(CMD_CONTINUE_OLD_GAME);
			break;

		case 'N':
		case 'n':
			{
			int i;

			CShip *pShip = g_pUniverse->GetPOV()->AsShip();
			if (pShip == NULL)
				break;

			DWORD dwNewShipClass = 0;

			//	Find the index of the current ship class

			DWORD dwClass = pShip->GetClassUNID();
			int iIndex = -1;
			for (i = 0; i < g_pUniverse->GetShipClassCount(); i++)
				if (g_pUniverse->GetShipClass(i)->GetUNID() == dwClass)
				{
					iIndex = i;
					break;
				}

			CShipClass *pNewShipClass;

			if (chChar == 'n')
				{
				//	Get the UNID of the next ship class in order

				do
					{
					if (iIndex == -1 || (iIndex + 1) == g_pUniverse->GetShipClassCount())
						iIndex = 0;
					else
						iIndex++;

					pNewShipClass = g_pUniverse->GetShipClass(iIndex);
					}
				while (pNewShipClass->IsVirtual());

				//	Set the variable so that the next ship created will
				//	have the given class

				dwNewShipClass = pNewShipClass->GetUNID();
				}
			else if(chChar == 'N')
				{
				//	Get the UNID of the previous ship class in order

				do
				{
					if ((iIndex - 1) < 0)
						iIndex = g_pUniverse->GetShipClassCount()-1;
					else
						iIndex--;

					pNewShipClass = g_pUniverse->GetShipClass(iIndex);
				} while (pNewShipClass->IsVirtual());

				//	Set the variable so that the next ship created will
				//	have the given class

				dwNewShipClass = pNewShipClass->GetUNID();
				}
			//	Destroy all ships of the current class

			g_pTrans->DestroyPOVIntroShips();

			//	Create a new ship

			CreateIntroShips(dwNewShipClass, pShip->GetSovereign()->GetUNID());
			break;
			}

		case 'O':
		case 'o':
			{
			int i;

			CSpaceObject *pPOV = g_pUniverse->GetPOV();
			if (pPOV->GetCategory() != CSpaceObject::catShip)
				break;

			CSystem *pSystem = pPOV->GetSystem();
			CSovereign *pCurSovereign = pPOV->GetSovereign();

			//	Make a list of all opponents

			TArray<CSpaceObject *> Opponents;
			for (i = 0; i < pSystem->GetObjectCount(); i++)
				{
				CSpaceObject *pObj = pSystem->GetObject(i);
				if (pObj 
						&& pObj->GetCategory() == CSpaceObject::catShip
						&& pObj->GetSovereign() != pCurSovereign
						&& pObj->CanAttack())
					Opponents.Insert(pObj);
				}

			//	Pick a random opponent and set the POV

			if (Opponents.GetCount() > 0)
				{
				g_pUniverse->SetPOV(Opponents[mathRandom(0, Opponents.GetCount() - 1)]);
				SetState(isShipStats);
				}

			break;
			}

		case 'P':
			{
			int i;

			CSpaceObject *pPOV = g_pUniverse->GetPOV();
			if (pPOV->GetCategory() != CSpaceObject::catShip)
				break;

			CSystem *pSystem = pPOV->GetSystem();
			CSovereign *pCurSovereign = pPOV->GetSovereign();

			//	Find the next POV in the list

			int iTotalCount = pSystem->GetObjectCount();
			for (i = 0; i < iTotalCount; i++)
				{
				CSpaceObject *pObj = pSystem->GetObject((pPOV->GetIndex() + iTotalCount - (i + 1)) % iTotalCount);
				if (pObj 
						&& pObj->GetCategory() == CSpaceObject::catShip
						&& pObj->CanAttack())
					{
					g_pUniverse->SetPOV(pObj);
					SetState(isShipStats);
					break;
					}
				}

			break;
			}

		case 'p':
			{
			int i;

			CSpaceObject *pPOV = g_pUniverse->GetPOV();
			if (pPOV->GetCategory() != CSpaceObject::catShip)
				break;

			CSystem *pSystem = pPOV->GetSystem();
			CSovereign *pCurSovereign = pPOV->GetSovereign();

			//	Find the next POV in the list

			int iTotalCount = pSystem->GetObjectCount();
			for (i = 0; i < iTotalCount; i++)
				{
				CSpaceObject *pObj = pSystem->GetObject((pPOV->GetIndex() + i + 1) % iTotalCount);
				if (pObj 
						&& pObj->GetCategory() == CSpaceObject::catShip
						&& pObj->CanAttack())
					{
					g_pUniverse->SetPOV(pObj);
					SetState(isShipStats);
					break;
					}
				}

			break;
			}

		case 'Q':
		case 'q':
			g_pTrans->DoCommand(CMD_QUIT_GAME);
			break;

		case 'S':
			g_pUniverse->SetSound(!g_pUniverse->GetSound());
			break;
		case 's':
			if (GetState() == isShipStats)
				SetState(isBlank);
			else
				SetState(isShipStats);
			break;

		case 'V':
		case 'v':
			SetState(isOpeningTitles);
			break;

		//	Not handled

		default:
			return false;
		}

	return true;
	}

void CIntroSession::OnAnimate (CG32bitImage &Screen, bool bTopMost)

//	OnAnimate
//
//	Animate the session

	{
	CSmartLock Lock(g_pUniverse->GetSem());

	bool bFailed = false;

	//	Update context

	SSystemUpdateCtx UpdateCtx;

	try
		{
		SetProgramState(psAnimating);

		//	Paint

		Paint(Screen, bTopMost);
		Update();

		SetProgramState(psUnknown);
		}
	catch (...)
		{
		bFailed = true;
		}

	//	Deal with errors/crashes

	if (bFailed)
		{
		m_HI.GetScreenMgr().StopDX();
		g_pTrans->ReportCrash();
		}
	}

void CIntroSession::OnChar (char chChar, DWORD dwKeyData)

//	OnChar
//
//	Handle keystrokes
	
	{
	CReanimator &Reanimator = GetReanimator();

    m_iIdleTicks = 0;

	//	Handle debug console

	if (m_DebugConsole.OnChar(chChar, dwKeyData))
		NULL;

	//	If we're in command mode, then handle it

	else if (HandleCommandBoxChar(chChar, dwKeyData))
		NULL;

	//	See if the animator will handle it

	else if (Reanimator.HandleChar(chChar, dwKeyData))
		NULL;

	//	Handle the button bar

	else if (g_pTrans->m_ButtonBarDisplay.OnChar(chChar))
		NULL;

	//	Otherwise, we handle the key code

	else
		HandleChar(chChar, dwKeyData);
	}

void CIntroSession::OnCleanUp (void)

//	OnCleanUp
//
//	Clean up

	{
	if (m_bExpandedDesired)
		::ShowCursor(true);
	}

ALERROR CIntroSession::OnCommand (const CString &sCmd, void *pData)

//	OnCommand
//
//	Handle commands
	
	{
	if (strEquals(sCmd, CMD_SHOW_PROFILE))
		m_HI.HICommand(CMD_UI_SHOW_PROFILE);

	else if (strEquals(sCmd, CMD_ACCOUNT))
		m_HI.HICommand(CMD_UI_SHOW_LOGIN);

	else if (strEquals(sCmd, CMD_SIGN_OUT))
		m_HI.HICommand(CMD_UI_SIGN_OUT);

	else if (strEquals(sCmd, CMD_SHOW_MOD_EXCHANGE))
		m_HI.HICommand(CMD_UI_SHOW_MOD_EXCHANGE);

	else if (strEquals(sCmd, CMD_SHOW_SETTINGS))
		m_HI.HICommand(CMD_UI_SHOW_SETTINGS);

	else if (strEquals(sCmd, CMD_CHANGE_PASSWORD))
		m_HI.HICommand(CMD_UI_CHANGE_PASSWORD);

	else if (strEquals(sCmd, CMD_SERVICE_HIGH_SCORE_LIST_LOADED))
		{
		m_HighScoreDisplay.SetHighScoreList((CAdventureHighScoreList *)pData);

		//	If we were waiting for the high score list, then show it.

		if (GetState() == isWaitingForHighScores)
			{
			if (m_HighScoreDisplay.HasHighScores())
				SetState(isHighScores);
			else
				SetState(isShipStats);
			}
		}
	else if (strEquals(sCmd, CMD_SERVICE_HIGH_SCORE_LIST_ERROR))
		{
		if (GetState() == isWaitingForHighScores)
			SetState(isShipStats);
		}

	else if (strEquals(sCmd, CMD_SERVICE_NEWS_LOADED))
		SetState(isNews);

	else if (strEquals(sCmd, CMD_SOUNDTRACK_NOW_PLAYING))
		StartSoundtrackTitleAnimation((CMusicResource *)pData);

	else if (strEquals(sCmd, CMD_INTRO_SHIP_DESTROYED))
		{
		CShip *pShip = (CShip *)pData;
		CreateIntroShips(0, 0, pShip);
		}

	else
		g_pTrans->OnCommandIntro(sCmd, pData);

	return NOERROR;
	}

ALERROR CIntroSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize the session
	
	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	SetNoCursor(true);

	//	Options

	m_bShowAllShips = m_Settings.GetBoolean(CGameSettings::introSpoilers);

	//	Metrics

	VI.GetWidescreenRect(&m_rcCenter);

	int cyBarHeight = Max(128, (g_cyScreen - INTRO_DISPLAY_HEIGHT) / 2);
	m_rcTop.top = 0;
	m_rcTop.left = 0;
	m_rcTop.bottom = cyBarHeight;
	m_rcTop.right = g_cxScreen;

	m_rcMain.top = cyBarHeight;
	m_rcMain.left = 0;
	m_rcMain.bottom = g_cyScreen - cyBarHeight;
	m_rcMain.right = g_cxScreen;

	m_rcBottom.top = g_cyScreen - cyBarHeight;
	m_rcBottom.left = 0;
	m_rcBottom.bottom = g_cyScreen;
	m_rcBottom.right = g_cxScreen;

    m_bExpanded = false;
    m_bExpandedDesired = false;
    m_iIdleTicks = 0;
    m_rcMainNormal = m_rcMain;
    m_rcMainExpanded.left = 0;
    m_rcMainExpanded.top = 0;
    m_rcMainExpanded.right = g_cxScreen;
    m_rcMainExpanded.bottom = g_cyScreen;

	//	Initialize the system

	CreateIntroSystem();

	//	Initialize

	g_pTrans->StartIntro(this);

	//	Set state

	SetState(m_iInitialState);

	//	Show the cursor

	ShowCursor(true);

	return NOERROR; 
	}

void CIntroSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//	OnKeyDown
//
//	Handle key down
	
	{
	CReanimator &Reanimator = GetReanimator();
	DWORD dwTVirtKey = CVirtualKeyData::TranslateVirtKey(iVirtKey, dwKeyData);
	CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(dwTVirtKey);

    m_iIdleTicks = 0;

	if (g_pTrans->m_ButtonBarDisplay.OnKeyDown(iVirtKey))
		NULL;

	else if (GetState() == isEnterCommand)
		NULL;

	else if (m_DebugConsole.IsEnabled())
		{
		if (iVirtKey == VK_ESCAPE || iCommand == CGameKeys::keyShowConsole)
			{
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			m_DebugConsole.SetEnabled(false);
			}
		else
			m_DebugConsole.OnKeyDown(iVirtKey, dwKeyData);
		}

	else if (Reanimator.IsPaused())
		Reanimator.Resume();

	else if (iCommand == CGameKeys::keyShowConsole)
		{
		if (m_Settings.GetBoolean(CGameSettings::debugMode))
			{
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			m_DebugConsole.SetEnabled(true);
			}
		}

	else
		{
		switch (iVirtKey)
			{
			case VK_ESCAPE:
                SetExpanded(false);
				CancelCurrentState();
				break;

			case VK_RETURN:
				g_pTrans->DoCommand(CMD_START_NEW_GAME);
				break;

			case VK_UP:
				if (GetState() == isHighScores || GetState() == isHighScoresEndGame)
					m_HighScoreDisplay.SelectPrev();
				break;

			case VK_DOWN:
				if (GetState() == isHighScores || GetState() == isHighScoresEndGame)
					m_HighScoreDisplay.SelectNext();
				break;

			case VK_PRIOR:
				Reanimator.FFBurst(-32, 15);
				break;

			case VK_NEXT:
				Reanimator.FFBurst(32, 15);
				break;

			case VK_PAUSE:
				Reanimator.Pause();
				break;

			case VK_F1:
				m_HI.HICommand(CONSTLIT("uiShowHelp"));
				break;

			case VK_F2:
				m_HI.HICommand(CONSTLIT("uiShowGameStats"));
				break;

            case VK_F11:
                SetExpanded(!m_bExpanded);
                break;
			}
		}
	}

void CIntroSession::OnLButtonDblClick (int x, int y, DWORD dwFlags) 

//	OnLButtonDblClick
//
//	Handle left double click.

	{
	if (m_DebugConsole.IsEnabled())
		;
	else
		g_pTrans->WMLButtonDblClick(x, y, dwFlags); 
	}

void CIntroSession::OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture)

//	OnLButtonDown
//
//	Handle left click

	{
	if (m_DebugConsole.IsEnabled())
		;
	else
		{
		m_iIdleTicks = 0;
		SetExpanded(false);
		g_pTrans->WMLButtonDown(x, y, dwFlags);
		}
	}

void CIntroSession::OnLButtonUp (int x, int y, DWORD dwFlags) 

//	OnLButtonUp
//
//	Handle left click

	{
	if (m_DebugConsole.IsEnabled())
		;
	else
		g_pTrans->WMLButtonUp(x, y, dwFlags); 
	}

void CIntroSession::OnMouseMove (int x, int y, DWORD dwFlags)

//  OnMouseMove
//
//  Handle mouse move
    
    {
	if (m_DebugConsole.IsEnabled())
		;

    //  If the mouse moves, show the controls (exit expanded mode).

    else 
		{
		if (m_HI.HasMouseMoved(x, y))
			{
			m_iIdleTicks = 0;
			SetExpanded(false);
			}
    
		g_pTrans->WMMouseMove(x, y, dwFlags);
		}
    }

void CIntroSession::OnPOVSet (CSpaceObject *pObj)

//	OnPOVSet
//
//	A new ship has been set for the POV

	{
	switch (GetState())
		{
		case isBlank:
		case isBlankThenRandom:
		case isShipStats:
			SetState(isShipStats);
			break;
		}
	}

void CIntroSession::OrderAttack (CShip *pShip, CSpaceObject *pTarget)

//	OrderAttack
//
//	Orders the given ship to attack the target

	{
	IShipController *pController = pShip->GetController();
	pController->CancelAllOrders();

	//	If this is a turret, then we use the sentinel order

	if (pShip->IsAnchored())
		pController->AddOrder(IShipController::orderSentry, NULL, IShipController::SData());

	//	Otherwise, we attack

	else
		pController->AddOrder(IShipController::orderDestroyTarget, pTarget, IShipController::SData());
	}

void CIntroSession::Paint (CG32bitImage &Screen, bool bTopMost)

//	Paint
//
//	Paint the intro screen

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	CG32bitPixel rgbBackgroundColor = VI.GetColor(colorAreaDeep);
	CReanimator &Reanimator = GetReanimator();

	DWORD dwStartTimer;
	if (g_pTrans->m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
		dwStartTimer = ::GetTickCount();

	//	Tell the universe to paint

	g_pUniverse->PaintPOV(Screen, m_rcMain, 0);

	//	Paint displays

	g_pTrans->m_ButtonBarDisplay.Update();
	g_pTrans->m_ButtonBarDisplay.Paint(Screen);

	//	Paint a frame around viewscreen

	Screen.FillLine(m_rcMain.left, m_rcMain.top - 1, RectWidth(m_rcMain), RGB_FRAME);
	Screen.FillLine(m_rcMain.left, m_rcMain.bottom, RectWidth(m_rcMain), RGB_FRAME);
	Screen.Fill(0, 0, RectWidth(m_rcMain), m_rcMain.top - 1, rgbBackgroundColor);

	//	Paint reanimator

	Reanimator.PaintFrame(Screen);

	//	Paint based on state

	switch (GetState())
		{
		case isEnterCommand:
			{
			int xMidCenter = g_pTrans->m_rcIntroMain.left + RectWidth(g_pTrans->m_rcIntroMain) / 2;
			int yMidCenter = g_pTrans->m_rcIntroMain.bottom - RectHeight(g_pTrans->m_rcIntroMain) / 3;

			int cyHeight;
			int cxWidth = Max(300, g_pTrans->m_Fonts.SubTitle.MeasureText(g_pTrans->m_sCommand, &cyHeight));

			CG32bitPixel rgbBorderColor = CG32bitPixel::Blend(CG32bitPixel(0, 0, 0), g_pTrans->m_Fonts.rgbTitleColor, (BYTE)128);
			CGDraw::RectOutlineDotted(Screen, 
					xMidCenter - cxWidth / 2 - 2, 
					yMidCenter - 2, 
					cxWidth + 4, 
					cyHeight + 4, 
					rgbBorderColor);

			g_pTrans->m_Fonts.SubTitle.DrawText(Screen,
					xMidCenter,
					yMidCenter,
					g_pTrans->m_Fonts.rgbTitleColor,
					g_pTrans->m_sCommand,
					CG16bitFont::AlignCenter);
			break;
			}
		}

	//	Figure out how long it took to paint

	if (g_pTrans->m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
		{
		DWORD dwNow = ::GetTickCount();
		g_pTrans->m_iPaintTime[g_pTrans->m_iFrameCount % FRAME_RATE_COUNT] = dwNow - dwStartTimer;
		dwStartTimer = dwNow;
		}

	//	Debug information

	if (g_pTrans->m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
		g_pTrans->PaintFrameRate();

#ifdef DEBUG
	g_pTrans->PaintDebugLines();
#endif
	g_pTrans->m_pTC->PaintDebugInfo(Screen, m_rcMain);

	//	Debug console

	m_DebugConsole.Paint(Screen);

	//	Update the screen

	if (bTopMost)
		m_HI.GetScreenMgr().Render();

	//	Figure out how long it took to blt

	if (g_pTrans->m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
		{
		DWORD dwNow = ::GetTickCount();
		g_pTrans->m_iBltTime[g_pTrans->m_iFrameCount % FRAME_RATE_COUNT] = dwNow - dwStartTimer;
		}
	}

void CIntroSession::SetExpanded (bool bExpanded)

    //  SetExpanded
    //
    //  Sets the main view to be expanded or not.

    {
    //  If we're not yet done switching from one state to another, then ignore
    //  this command

    if (m_bExpandedDesired != m_bExpanded)
        return;

    //  Change state

    if (m_bExpandedDesired != bExpanded)
        {
        m_bExpandedDesired = bExpanded;
        ShowCursor(!bExpanded);
        }
    }

void CIntroSession::SetState (EStates iState)

//	SetState
//
//	Sets our current state

	{
	CReanimator &Reanimator = GetReanimator();

	//	Note: It is OK to call this with iState already
	//	equal to the current state. We still need to go through this code

	switch (iState)
		{
		case isBlank:
			StopAnimations();
			break;

		case isBlankThenRandom:
			StopAnimations();
			g_pTrans->m_iIntroCounter = 3600;
			break;

		case isCredits:
			StopAnimations();
			Reanimator.StartPerformance(g_pTrans->m_dwCreditsPerformance);
			break;

		case isEndGame:
			{
			StopAnimations();

			const CGameRecord &Score = m_Model.GetGameRecord();
			if (Score.GetScore() == 0)
				{
				CmdShowHighScoreList();
				return;
				}

			IAnimatron *pAni;
			g_pTrans->CreateScoreAnimation(Score, &pAni);
			DWORD dwScorePerformance = Reanimator.AddPerformance(pAni, ID_END_GAME_PERFORMANCE);
			Reanimator.StartPerformance(dwScorePerformance, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

			//	While we're showing the performance, get the latest score

			CAdventureHighScoreList::SSelect Select;
			Select.dwAdventure = Score.GetAdventureUNID();
			Select.sUsername = Score.GetUsername();
			Select.iScore = Score.GetScore();
			m_HI.HICommand(CMD_UPDATE_HIGH_SCORE_LIST, &Select);
			break;
			}

		case isEnterCommand:
			StopAnimations();
			g_pTrans->m_sCommand = NULL_STR;
			break;

		case isHighScores:
		case isHighScoresEndGame:
			StopAnimations();
			m_HighScoreDisplay.StartPerformance(Reanimator, ID_HIGH_SCORES_PERFORMANCE, g_pTrans->m_rcIntroMain);
			break;

		case isIntroHelp:
			{
			StopAnimations();

			IAnimatron *pAni;
			g_pTrans->CreateIntroHelpAnimation(&pAni);
			DWORD dwPerformance = Reanimator.AddPerformance(pAni, ID_INTRO_HELP_PERFORMANCE);
			Reanimator.StartPerformance(dwPerformance, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
			break;
			}

		case isNews:
			{
			CMultiverseNewsEntry *pNews = g_pTrans->m_pTC->GetMultiverse().GetNextNewsEntry();
			if (pNews == NULL)
				{
				SetState(isShipStats);
				return;
				}

			StopAnimations();

			IAnimatron *pAni;
			g_pTrans->CreateNewsAnimation(pNews, &pAni);
			delete pNews;

			DWORD dwPerformance = Reanimator.AddPerformance(pAni, ID_NEWS_PERFORMANCE);
			Reanimator.StartPerformance(dwPerformance, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
			break;
			}

		case isOpeningTitles:
			StopAnimations();
			Reanimator.StartPerformance(g_pTrans->m_dwTitlesPerformance);
			break;

		case isShipStats:
			{
			CSpaceObject *pPOV = g_pUniverse->GetPOV();
			if (pPOV == NULL)
				{
				SetState(isBlank);
				return;
				}

			CShip *pShip = pPOV->AsShip();
			if (pShip == NULL)
				{
				SetState(isBlank);
				return;
				}

			StopAnimations();

			IAnimatron *pAni;
			g_pTrans->CreateShipDescAnimation(pShip, &pAni);
			DWORD dwPerformance = Reanimator.AddPerformance(pAni, ID_SHIP_DESC_PERFORMANCE);
			Reanimator.StartPerformance(dwPerformance, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
			break;
			}

		case isWaitingForHighScores:
			StopAnimations();
			break;

		default:
			ASSERT(false);
			return;
		}

	m_iState = iState;
	}

void CIntroSession::StartSoundtrackTitleAnimation (CMusicResource *pTrack)

//	StartSoundtrackTitleAnimation
//
//	Display titles for the current soundtrack.

	{
	CReanimator &Reanimator = GetReanimator();

	//	Stop any existing performance

	Reanimator.StopPerformance(ID_SOUNDTRACK_TITLE_PERFORMANCE);

	//	Create an animation, if we have a track

	if (pTrack 
			&& !pTrack->GetTitle().IsBlank())
		{
		IAnimatron *pAni;
		CreateSoundtrackTitleAnimation(pTrack, &pAni);
		DWORD dwPerformance = Reanimator.AddPerformance(pAni, ID_SOUNDTRACK_TITLE_PERFORMANCE);
		Reanimator.StartPerformance(dwPerformance, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
		}
	}

void CIntroSession::StopAnimations (void)

//	StopAnimations
//
//	Stops all intro animations (but not UI element animations)

	{
	CReanimator &Reanimator = GetReanimator();

	Reanimator.StopPerformance(ID_CREDITS_PERFORMANCE);
	Reanimator.StopPerformance(ID_END_GAME_PERFORMANCE);
	Reanimator.StopPerformance(ID_INTRO_HELP_PERFORMANCE);
	Reanimator.StopPerformance(ID_SHIP_DESC_PERFORMANCE);
	Reanimator.StopPerformance(ID_TITLES_PERFORMANCE);
	Reanimator.StopPerformance(ID_NEWS_PERFORMANCE);
	m_HighScoreDisplay.StopPerformance();
	}

void CIntroSession::Update (void)

//	Update
//
//	Update the intro state

	{
	CReanimator &Reanimator = GetReanimator();

    //  If we're expanding/collapsing the main screen, then we need to update 
    //  some metrics

    if (m_bExpanded != m_bExpandedDesired)
        {
        //  Figure out the desired state

        RECT rcMainDesired = (m_bExpandedDesired ? m_rcMainExpanded : m_rcMainNormal);

        //  Grow or shrink the main rect

        m_rcMain.top = IntProportionalTransition(m_rcMain.top, rcMainDesired.top, 30);
        m_rcMain.bottom = IntProportionalTransition(m_rcMain.bottom, rcMainDesired.bottom, 30);

        //  Move the top bar

        IAnimatron *pPlayerBar = Reanimator.GetPerformance(ID_PLAYER_BAR_PERFORMANCE);
        if (pPlayerBar)
            pPlayerBar->SetPropertyVector(PROP_POSITION, CVector(m_rcCenter.left, m_rcMain.top - TITLE_BAR_HEIGHT));

        //  Move the bottom bar

        RECT rcBottomBar;
        rcBottomBar.left = 0;
        rcBottomBar.top = m_rcMain.bottom;
        rcBottomBar.right = g_cxScreen;
        rcBottomBar.bottom = g_cyScreen;
        g_pTrans->m_ButtonBarDisplay.SetRect(rcBottomBar);

        //  Are we done?

        if (m_rcMain.top == rcMainDesired.top && m_rcMain.bottom == rcMainDesired.bottom)
            m_bExpanded = m_bExpandedDesired;
        }

    //  Otherwise, if we've been idle for a while, expand

    else if (m_iIdleTicks > EXPAND_ON_IDLE_TIME)
        SetExpanded(true);

	//	Time update, if necessary

	DWORD dwStartTimer;
	if (g_pTrans->m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
		dwStartTimer = ::GetTickCount();

	//	If the same ship has been here for a while, then create a new ship

	if (g_pTrans->m_iTick - g_pTrans->m_iLastShipCreated > MAX_TIME_WITH_ONE_SHIP)
		{
		CShip *pShip = g_pUniverse->GetPOV()->AsShip();
		if (pShip)
			{
			pShip->Destroy(removedFromSystem, CDamageSource());
			CreateIntroShips(0, 0, pShip);
			}
		}

	//	Update the universe

	SSystemUpdateCtx Ctx;
	Ctx.bForceEventFiring = true;
	if (!g_pTrans->m_bPaused)
		{
		g_pUniverse->Update(Ctx);
		g_pTrans->m_iTick++;
		}

	//	Slight HACK: If the current POV is not a ship, then create a new one

	if (g_pUniverse->GetPOV() == NULL
			|| g_pUniverse->GetPOV()->GetCategory() != CSpaceObject::catShip)
		CreateIntroShips();

	//	Figure out how long it took to update

	if (g_pTrans->m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
		{
		DWORD dwNow = ::GetTickCount();
		g_pTrans->m_iUpdateTime[g_pTrans->m_iFrameCount % FRAME_RATE_COUNT] = dwNow - dwStartTimer;
		}

	//	Update intro state

	switch (GetState())
		{
		case isBlankThenRandom:
			if (--g_pTrans->m_iIntroCounter == 0)
				{
				int iRoll = mathRandom(1, 100);
				if (iRoll <= 50)
					SetState(isCredits);
				else if (m_HighScoreDisplay.HasHighScores())
					SetState(isHighScores);
				else
					SetState(isShipStats);
				}
			break;

		case isCredits:
			if (!Reanimator.IsPerformanceRunning(ID_CREDITS_PERFORMANCE))
				SetState(isShipStats);
			break;

		case isShipStats:
			if (!Reanimator.IsPerformanceRunning(ID_SHIP_DESC_PERFORMANCE))
				SetState(isBlankThenRandom);
			break;

		case isNews:
			if (!Reanimator.IsPerformanceRunning(ID_NEWS_PERFORMANCE))
				SetState(isShipStats);
			break;

		case isEndGame:
			if (!Reanimator.IsPerformanceRunning(ID_END_GAME_PERFORMANCE))
				{
				if (m_HighScoreDisplay.HasHighScores())
					SetState(isHighScoresEndGame);
				else
					SetState(isCredits);
				}
			break;

		case isHighScores:
			if (!m_HighScoreDisplay.IsPerformanceRunning())
				SetState(isShipStats);
			break;

		case isHighScoresEndGame:
			if (!m_HighScoreDisplay.IsPerformanceRunning())
				SetState(isCredits);
			break;

		case isIntroHelp:
			if (!Reanimator.IsPerformanceRunning(ID_INTRO_HELP_PERFORMANCE))
				SetState(isShipStats);
			break;

		case isOpeningTitles:
			if (!Reanimator.IsPerformanceRunning(ID_TITLES_PERFORMANCE))
				SetState(isNews);
			break;
		}

    //  Track idle

    m_iIdleTicks++;
	}
