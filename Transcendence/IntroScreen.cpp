//	IntroScreen.cpp
//
//	Show intro screen

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

#include "Credits.h"

#define INTRO_DISPLAY_WIDTH						1024
#define INTRO_DISPLAY_HEIGHT					512

#define HIGHSCORE_DISPLAY_X						0
#define HIGHSCORE_DISPLAY_Y						200
#define HIGHSCORE_DISPLAY_WIDTH					500
#define HIGHSCORE_DISPLAY_HEIGHT				250

#define RGB_VERSION_COLOR						CG16bitImage::RGBValue(128,128,128)
#define RGB_COPYRIGHT_COLOR						CG16bitImage::RGBValue(80,80,80)
#define RGB_FRAME								CG16bitImage::RGBValue(80,80,80)

#define MAX_TIME_WITH_ONE_SHIP					(g_TicksPerSecond * 90)
#define MAX_INTRO_SHIPS							500

#define DIALOG_WIDTH							500
#define DIALOG_HEIGHT							250
#define DIALOG_SPACING_X						8
#define DIALOG_SPACING_Y						8
#define DIALOG_BUTTON_HEIGHT					25
#define DIALOG_BUTTON_WIDTH						100
#define RGB_DIALOG_BACKGROUND					CG16bitImage::RGBValue(48,48,64)
#define RGB_DIALOG_TEXT							CG16bitImage::RGBValue(144,144,192)
#define RGB_DIALOG_BUTTON_BACKGROUND			CG16bitImage::RGBValue(144,144,192)
#define RGB_DIALOG_BUTTON_TEXT					CG16bitImage::RGBValue(0, 0, 0)

#define RGB_NEWS_PANE_BACKGROUND				CG16bitImage::RGBValue(128, 213, 255)

#define STR_OVERWRITE							CONSTLIT("Overwrite")
#define STR_CANCEL								CONSTLIT("Cancel")
#define STR_OVERWRITE_GAME						CONSTLIT("Overwrite Saved Game?")
#define STR_TEXT1								CONSTLIT("You currently have a saved game in progress.")
#define STR_TEXT2								CONSTLIT("If you start a new game your will overwrite your saved game.")

#define ATTRIB_GENERIC_SHIP_CLASS				CONSTLIT("genericClass")

const int SCORE_COLUMN_WIDTH =					360;
const Metric SCORE_SPACING_Y =					4.0;
const Metric SCORE_SELECTION_WIDTH =			480.0;
const Metric SCORE_SELECTION_X =				-120.0;
const DWORD SCORE_SELECTION_OPACITY =			64;
const int HIGH_SCORES_DURATION =				300;

const Metric GAME_STAT_SELECTION_WIDTH =		480.0;
const Metric GAME_STAT_SELECTION_X =			-20.0;
const DWORD GAME_STAT_SELECTION_OPACITY =		64;
const int GAME_STAT_DURATION =					300;

const int BUTTON_HEIGHT =						96;
const int BUTTON_WIDTH =						96;
const int ICON_CORNER_RADIUS =					8;
const int ICON_HEIGHT =							96;
const int ICON_WIDTH =							96;
const int PADDING_LEFT =						20;
const int SMALL_LINK_SPACING =					20;
const int TITLE_BAR_HEIGHT =					128;

const int NEWS_PANE_WIDTH =						400;
const int NEWS_PANE_PADDING_X =					20;
const int NEWS_PANE_PADDING_Y =					8;
const int NEWS_PANE_CORNER_RADIUS =				8;
const int NEWS_PANE_INNER_SPACING_Y =			8;

#define ALIGN_CENTER							CONSTLIT("center")

#define CMD_ACCOUNT								CONSTLIT("cmdAccount")
#define CMD_ACCOUNT_EDIT						CONSTLIT("cmdAccountEdit")
#define CMD_CHANGE_PASSWORD						CONSTLIT("cmdChangePassword")
#define CMD_OPEN_NEWS							CONSTLIT("cmdOpenNews")
#define CMD_SHOW_MOD_EXCHANGE					CONSTLIT("cmdShowModExchange")
#define CMD_SHOW_PROFILE						CONSTLIT("cmdShowProfile")
#define CMD_SIGN_OUT							CONSTLIT("cmdSignOut")
#define CMD_TOGGLE_DEBUG						CONSTLIT("cmdToggleDebug")
#define CMD_TOGGLE_MUSIC						CONSTLIT("cmdToggleMusic")

#define CMD_SERVICE_NEWS_LOADED					CONSTLIT("serviceNewsLoaded")

//	These are commands that we send to the Controller
#define CMD_UI_CHANGE_PASSWORD					CONSTLIT("uiChangePassword")
#define CMD_UI_SHOW_LOGIN						CONSTLIT("uiShowLogin")
#define CMD_UI_SHOW_MOD_EXCHANGE				CONSTLIT("uiShowModExchange")
#define CMD_UI_SHOW_PROFILE						CONSTLIT("uiShowProfile")
#define CMD_UI_SIGN_OUT							CONSTLIT("uiSignOut")

#define EVENT_ON_CLICK							CONSTLIT("onClick")

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
#define ID_PLAYER_BAR_PERFORMANCE				CONSTLIT("idPlayerBar")
#define ID_SHIP_DESC_PERFORMANCE				CONSTLIT("idShipDescPerformance")
#define ID_TITLES_PERFORMANCE					CONSTLIT("idTitles")
#define ID_NEWS_PERFORMANCE						CONSTLIT("idNews")

#define PROP_COLOR								CONSTLIT("color")
#define PROP_FONT								CONSTLIT("font")
#define PROP_LL_RADIUS							CONSTLIT("llRadius")
#define PROP_LR_RADIUS							CONSTLIT("lrRadius")
#define PROP_OPACITY							CONSTLIT("opacity")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_SCROLL_POS							CONSTLIT("scrollPos")
#define PROP_TEXT								CONSTLIT("text")
#define PROP_TEXT_ALIGN_HORZ					CONSTLIT("textAlignHorz")
#define PROP_UL_RADIUS							CONSTLIT("ulRadius")
#define PROP_UR_RADIUS							CONSTLIT("urRadius")
#define PROP_VIEWPORT_HEIGHT					CONSTLIT("viewportHeight")

#define STYLE_CHECK								CONSTLIT("check")
#define STYLE_DISABLED							CONSTLIT("disabled")
#define STYLE_DOWN								CONSTLIT("down")
#define STYLE_FRAME								CONSTLIT("frame")
#define STYLE_FRAME_FOCUS						CONSTLIT("frameFocus")
#define STYLE_FRAME_DISABLED					CONSTLIT("frameDisabled")
#define STYLE_HOVER								CONSTLIT("hover")
#define STYLE_IMAGE								CONSTLIT("image")
#define STYLE_NORMAL							CONSTLIT("normal")
#define STYLE_TEXT								CONSTLIT("text")

void CTranscendenceWnd::AnimateIntro (bool bTopMost)

//	AnimateIntro
//
//	Paint intro screen

	{
	CG16bitImage &TheScreen = g_pHI->GetScreen();
	const CVisualPalette &VI = g_pHI->GetVisuals();
	WORD wBackgroundColor = VI.GetColor(colorAreaDeep);

	DWORD dwStartTimer;
	if (m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
		dwStartTimer = ::GetTickCount();

	//	Tell the universe to paint

	g_pUniverse->PaintPOV(TheScreen, m_rcIntroMain, 0);

	//	Paint displays

	m_ButtonBarDisplay.Update();
	m_ButtonBarDisplay.Paint(TheScreen);

	//	Paint a frame around viewscreen

	TheScreen.FillLine(m_rcIntroMain.left, m_rcIntroMain.top - 1, RectWidth(m_rcIntroMain), RGB_FRAME);
	TheScreen.FillLine(m_rcIntroMain.left, m_rcIntroMain.bottom, RectWidth(m_rcIntroMain), RGB_FRAME);
	TheScreen.Fill(0, 0, RectWidth(m_rcIntroMain), m_rcIntroMain.top - 1, wBackgroundColor);

	//	Paint reanimator

	m_Reanimator.PaintFrame(TheScreen);

	//	Paint based on state

	switch (m_iIntroState)
		{
		case isEnterShipClass:
			{
			int xMidCenter = m_rcIntroMain.left + RectWidth(m_rcIntroMain) / 2;
			int yMidCenter = m_rcIntroMain.bottom - RectHeight(m_rcIntroMain) / 3;

			int cyHeight;
			int cxWidth = Max(300, m_Fonts.SubTitle.MeasureText(m_sCommand, &cyHeight));

			WORD wBorderColor = CG16bitImage::BlendPixel(CG16bitImage::RGBValue(0, 0, 0), m_Fonts.wTitleColor, 128);
			DrawRectDotted(TheScreen, 
					xMidCenter - cxWidth / 2 - 2, 
					yMidCenter - 2, 
					cxWidth + 4, 
					cyHeight + 4, 
					wBorderColor);

			m_Fonts.SubTitle.DrawText(TheScreen,
					xMidCenter,
					yMidCenter,
					m_Fonts.wTitleColor,
					m_sCommand,
					CG16bitFont::AlignCenter);
			break;
			}
		}

	//	If we've got a dialog box up, paint it

	if (m_bOverwriteGameDlg)
		PaintOverwriteGameDlg();

	//	Figure out how long it took to paint

	if (m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
		{
		DWORD dwNow = ::GetTickCount();
		m_iPaintTime[m_iFrameCount % FRAME_RATE_COUNT] = dwNow - dwStartTimer;
		dwStartTimer = dwNow;
		}

	//	Debug information

	if (m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
		PaintFrameRate();

#ifdef DEBUG
	PaintDebugLines();
#endif

	//	Update the screen

	if (bTopMost)
		g_pHI->GetScreenMgr().Blt();

	//	Figure out how long it took to blt

	if (m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
		{
		DWORD dwNow = ::GetTickCount();
		m_iBltTime[m_iFrameCount % FRAME_RATE_COUNT] = dwNow - dwStartTimer;
		dwStartTimer = dwNow;
		}

	//	If the same ship has been here for a while, then create a new ship

	if (m_iTick - m_iLastShipCreated > MAX_TIME_WITH_ONE_SHIP)
		{
		CShip *pShip = g_pUniverse->GetPOV()->AsShip();
		if (pShip)
			{
			pShip->Destroy(removedFromSystem, CDamageSource());
			CreateIntroShips(0, 0, pShip);
			}
		}

	//	Update the universe

	if (!m_bPaused)
		{
		g_pUniverse->Update(g_SecondsPerUpdate, true);
		m_iTick++;
		}

	//	Slight HACK: If the current POV is not a ship, then create a new one

	if (g_pUniverse->GetPOV()->GetCategory() != CSpaceObject::catShip)
		CreateIntroShips();

	//	Advance the intro animation

	switch (m_iIntroState)
		{
		case isBlankThenRandom:
			if (--m_iIntroCounter == 0)
				{
				int iRoll = mathRandom(1, 100);
				if (iRoll <= 50)
					SetIntroState(isCredits);
				else
					SetIntroState(isHighScores);
				}
			break;

		case isCredits:
			if (!m_Reanimator.IsPerformanceRunning(ID_CREDITS_PERFORMANCE))
				SetIntroState(isShipStats);
			break;

		case isShipStats:
			if (!m_Reanimator.IsPerformanceRunning(ID_SHIP_DESC_PERFORMANCE))
				SetIntroState(isBlankThenRandom);
			break;

		case isNews:
			if (!m_Reanimator.IsPerformanceRunning(ID_NEWS_PERFORMANCE))
				SetIntroState(isShipStats);
			break;

		case isEndGame:
			if (!m_Reanimator.IsPerformanceRunning(ID_END_GAME_PERFORMANCE))
				SetIntroState(isHighScoresEndGame);
			break;

		case isHighScores:
			if (!m_Reanimator.IsPerformanceRunning(ID_HIGH_SCORES_PERFORMANCE))
				SetIntroState(isShipStats);
			break;

		case isHighScoresEndGame:
			if (!m_Reanimator.IsPerformanceRunning(ID_HIGH_SCORES_PERFORMANCE))
				SetIntroState(isCredits);
			break;

		case isOpeningTitles:
			if (!m_Reanimator.IsPerformanceRunning(ID_TITLES_PERFORMANCE))
				SetIntroState(isNews);
			break;
		}

	//	Figure out how long it took to update

	if (m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
		{
		DWORD dwNow = ::GetTickCount();
		m_iUpdateTime[m_iFrameCount % FRAME_RATE_COUNT] = dwNow - dwStartTimer;
		dwStartTimer = dwNow;
		}
	}

void CTranscendenceWnd::CancelCurrentIntroState (void)

//	CancelCurrentIntroState
//
//	Cancels the current animation state

	{
	switch (m_iIntroState)
		{
		case isCredits:
		case isHighScores:
		case isBlankThenRandom:
		case isNews:
			SetIntroState(isShipStats);
			break;

		case isEndGame:
			SetIntroState(isHighScoresEndGame);
			break;

		case isEnterShipClass:
			SetIntroState(isShipStats);
			break;

		case isHighScoresEndGame:
			SetIntroState(isCredits);
			break;

		case isOpeningTitles:
			SetIntroState(isShipStats);
			break;

		case isShipStats:
			SetIntroState(isBlankThenRandom);
			break;
		}
	}

void CTranscendenceWnd::CreateCreditsAnimation (IAnimatron **retpAnimatron)

//	CreateCreditsAnimation
//
//	Creates full end credits

	{
	int i;
	CAniSequencer *pSeq = new CAniSequencer;
	int iTime = 0;

	//	Figure out the position

	int xMidCenter = m_rcIntroMain.left + RectWidth(m_rcIntroMain) / 2;
	int yMidCenter = m_rcIntroMain.bottom - RectHeight(m_rcIntroMain) / 3;

	IAnimatron *pAnimation;

	//	Create credits

	TArray<CString> Names;
	Names.Insert(CONSTLIT("George Moromisato"));
	m_UIRes.CreateMediumCredit(CONSTLIT("designed & created by"), 
			Names,
			xMidCenter,
			yMidCenter,
			150,
			&pAnimation);
	pSeq->AddTrack(pAnimation, iTime);
	iTime += 150;

	Names.DeleteAll();
	Names.Insert(CONSTLIT("Michael Tangent"));
	m_UIRes.CreateMediumCredit(CONSTLIT("music by"), 
			Names,
			xMidCenter,
			yMidCenter,
			150,
			&pAnimation);
	pSeq->AddTrack(pAnimation, iTime);
	iTime += 150;

	//	Linux port

	Names.DeleteAll();
	Names.Insert(CONSTLIT("Benn Bollay"));
	m_UIRes.CreateMediumCredit(CONSTLIT("linux port by"),
			Names,
			xMidCenter,
			yMidCenter,
			150,
			&pAnimation);
	pSeq->AddTrack(pAnimation, iTime);
	iTime += 150;

	//	Special thanks

	Names.DeleteAll();
	for (i = 0; i < SPECIAL_THANKS_COUNT; i++)
		Names.Insert(CString(SPECIAL_THANKS[i]));

	m_UIRes.CreateMediumCredit(CONSTLIT("special thanks to"),
			Names,
			xMidCenter,
			yMidCenter,
			150,
			&pAnimation);
	pSeq->AddTrack(pAnimation, iTime);
	iTime += SPECIAL_THANKS_COUNT * 150;

	//	Thanks to

	int yStart = m_rcIntroMain.top;
	int yEnd = m_rcIntroMain.bottom - m_Fonts.Header.GetHeight();
	CreateLongCreditsAnimation(xMidCenter + RectWidth(m_rcIntroMain) / 6,
			yStart,
			yEnd - yStart, 
			&pAnimation);
	pSeq->AddTrack(pAnimation, iTime);
	iTime += pAnimation->GetDuration();

	//	Done

	*retpAnimatron = pSeq;
	}

void CTranscendenceWnd::CreateHighScoresAnimation (IAnimatron **retpAnimatron)

//	CreateHighScoresAnimation
//
//	Creates scrolling high-scores

	{
	int i, j;
	CHighScoreList *pHighScoreList = GetHighScoreList();

	//	Figure out the position

	int x = m_rcIntroMain.left + 2 * RectWidth(m_rcIntroMain) / 3;
	int y = m_rcIntroMain.top;
	int yEnd = m_rcIntroMain.bottom - m_Fonts.Header.GetHeight();
	int cyHeight = yEnd - y;

	//	We keep track of all the score positions in an array

	m_pHighScorePos = new int [pHighScoreList->GetCount() + 1];

	//	Create a scroller

	CAniVScroller *pAni = new CAniVScroller;
	pAni->SetPropertyVector(CONSTLIT("position"), CVector((Metric)x, (Metric)y));
	pAni->SetPropertyMetric(CONSTLIT("viewportHeight"), (Metric)cyHeight);
	pAni->SetPropertyMetric(CONSTLIT("fadeEdgeHeight"), (Metric)(cyHeight / 8));

	//	Add a rect that we use to highlight selection.
	//	We add it first because it paints behind everything

	IAnimatron *pRect;
	CAniRect::Create(CVector(0.0, 0.0), CVector(100.0, 10.0), m_Fonts.wLightTitleColor, 0, &pRect);
	pRect->SetID(ID_HIGH_SCORES_SELECT_RECT);
	pAni->AddLine(pRect);

	//	Loop over all scores

	for (i = 0; i < pHighScoreList->GetCount(); i++)
		{
		const CGameRecord &Score = pHighScoreList->GetEntry(i);

		//	Position

		Metric yPos = pAni->GetHeight() + (i > 0 ? SCORE_SPACING_Y : 0);
		m_pHighScorePos[i] = (int)(yPos - (cyHeight / 3));

		//	Score

		IAnimatron *pText;
		CAniText::Create(strFromInt(Score.GetScore()),
				CVector((Metric)-m_Fonts.SubTitle.GetAverageWidth(), yPos),
				&m_Fonts.SubTitle,
				CG16bitFont::AlignRight,
				m_Fonts.wTitleColor,
				&pText);
		pAni->AddLine(pText);

		//	Name

		CAniText::Create(Score.GetPlayerName(),
				CVector(0.0, yPos),
				&m_Fonts.SubTitle,
				0,
				m_Fonts.wTitleColor,
				&pText);
		pAni->AddLine(pText);

		//	Ship class

		CString sShipClass = Score.GetShipClass();
		if (Score.IsDebug())
			sShipClass.Append(CONSTLIT(" [debug]"));

		CAniText::Create(sShipClass,
				CVector(0.0, pAni->GetHeight()),
				&m_Fonts.Medium,
				0,
				m_Fonts.wLightTitleColor,
				&pText);
		pAni->AddLine(pText);

		//	Epitaph

		CString sEpitaph = Score.GetDescription(CGameRecord::descEpitaph | CGameRecord::descPlayTime | CGameRecord::descResurrectCount);

		TArray<CString> Lines;
		m_Fonts.Medium.BreakText(sEpitaph, SCORE_COLUMN_WIDTH, &Lines);

		for (j = 0; j < Lines.GetCount(); j++)
			{
			CAniText::Create(Lines[j],
					CVector(0.0, pAni->GetHeight()),
					&m_Fonts.Medium,
					0,
					m_Fonts.wLightTitleColor,
					&pText);
			pAni->AddLine(pText);
			}
		}

	//	Set the end pos

	m_pHighScorePos[i] = (int)pAni->GetHeight() - (cyHeight / 3);

	//	Done

	*retpAnimatron = pAni;
	}

void CTranscendenceWnd::CreateIntroShips (DWORD dwNewShipClass, DWORD dwSovereign, CSpaceObject *pShipDestroyed)

//	CreateIntroShips
//
//	Makes sure that there are enough ships and that everyone has a target

	{
	int i;
	CSystem *pSystem = g_pUniverse->GetCurrentSystem();

	//	Make sure each sovereign has a ship

	CSovereign *pSovereign1 = g_pUniverse->FindSovereign(g_PlayerSovereignUNID);
	CSovereign *pSovereign2 = g_pUniverse->FindSovereign(g_PirateSovereignUNID);
	CShip *pShip1 = NULL;
	CShip *pShip2 = NULL;

	//	Sovereign of POV

	DWORD dwCurSovereign = (dwSovereign ? dwSovereign : g_pUniverse->GetPOV()->GetSovereign()->GetUNID());

	//	Look for the surviving ships

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj
				&& pObj->GetCategory() == CSpaceObject::catShip
				&& !pObj->IsDestroyed()
				&& pObj != pShipDestroyed
				&& !pObj->IsInactive()
				&& !pObj->IsVirtual()
				&& !pObj->GetData(CONSTLIT("IntroController")).IsBlank())
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
		CreateRandomShip(pSystem, (dwCurSovereign == g_PlayerSovereignUNID ? dwNewShipClass : 0), pSovereign1, &pShip1);

	if (pShip2 == NULL || (dwNewShipClass && dwCurSovereign == g_PirateSovereignUNID))
		CreateRandomShip(pSystem, (dwCurSovereign == g_PirateSovereignUNID ? dwNewShipClass : 0), pSovereign2, &pShip2);

	//	Make sure every ship has an order to attack someone

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj
				&& pObj->GetCategory() == CSpaceObject::catShip
				&& !pObj->IsDestroyed()
				&& !pObj->IsInactive()
				&& !pObj->IsVirtual()
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
					pController->CancelAllOrders();
					if (pShip->GetSovereign() == pSovereign1)
						pController->AddOrder(IShipController::orderDestroyTarget, pShip2, IShipController::SData());
					else
						pController->AddOrder(IShipController::orderDestroyTarget, pShip1, IShipController::SData());
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
		OnIntroPOVSet(g_pUniverse->GetPOV());
		}

	m_iLastShipCreated = m_iTick;
	}

void CTranscendenceWnd::CreateLongCreditsAnimation (int x, int y, int cyHeight, IAnimatron **retpAnimatron)

//	CreateLongCreditsAnimation
//
//	Creates scrolling credits

	{
	int i, j;
	CAniVScroller *pAni = new CAniVScroller;
	pAni->SetPropertyVector(CONSTLIT("position"), CVector((Metric)x, (Metric)y));
	pAni->SetPropertyMetric(CONSTLIT("viewportHeight"), (Metric)cyHeight);
	pAni->SetPropertyMetric(CONSTLIT("fadeEdgeHeight"), (Metric)(cyHeight / 8));

	//	Inspiration, Ideas, and Testing

	pAni->AddTextLine(CONSTLIT("ideas & testing"), &m_Fonts.SubTitle, m_Fonts.wLightTitleColor, CG16bitFont::AlignCenter);

	//	Add names

	for (i = 0; i < FEEDBACK_COUNT; i++)
		pAni->AddTextLine(CString(FEEDBACK[i]), 
				&m_Fonts.Header, 
				m_Fonts.wTitleColor, 
				CG16bitFont::AlignCenter,
				(i == 0 ? m_Fonts.Header.GetHeight() : 0));

	//	Software

	pAni->AddTextLine(CONSTLIT("created using"), &m_Fonts.SubTitle, m_Fonts.wLightTitleColor, CG16bitFont::AlignCenter, m_Fonts.Title.GetHeight());
	for (i = 0; i < SOFTWARE_COUNT; i++)
		pAni->AddTextLine(CString(SOFTWARE[i]), 
				&m_Fonts.Header, 
				m_Fonts.wTitleColor, 
				CG16bitFont::AlignCenter,
				(i == 0 ? m_Fonts.Header.GetHeight() : 0));

	//	Inspiration

	pAni->AddTextLine(CONSTLIT("with inspiration from"), &m_Fonts.SubTitle, m_Fonts.wLightTitleColor, CG16bitFont::AlignCenter, m_Fonts.Title.GetHeight());
	for (i = 0; i < INSPIRATION_COUNT; i++)
		pAni->AddTextLine(CString(INSPIRATION[i]), 
				&m_Fonts.Header, 
				m_Fonts.wTitleColor, 
				CG16bitFont::AlignCenter,
				(i == 0 ? m_Fonts.Header.GetHeight() : 0));

	//	Extensions credit. Build a list of all extensions 
	//	that need to be credited.

	struct SExtCredit
		{
		TArray<CString> Extensions;
		TArray<CString> Credits;
		};

	TArray<SExtCredit> ExtCredit;

	for (i = 0; i < g_pUniverse->GetExtensionDescCount(); i++)
		{
		const CExtension *pExt = g_pUniverse->GetExtensionDesc(i);
		if (!pExt->GetName().IsBlank() && pExt->GetCredits().GetCount() > 0)
			{
			//	See if we already have an extension with the same credits

			SExtCredit *pFound = NULL;
			for (j = 0; j < ExtCredit.GetCount() && pFound == NULL; j++)
				{
				if (ExtCredit[j].Credits.GetCount() == pExt->GetCredits().GetCount())
					{
					int k;

					bool bSame = true;
					for (k = 0; k < pExt->GetCredits().GetCount() && bSame; k++)
						if (!strEquals(ExtCredit[j].Credits[k], pExt->GetCredits()[k]))
							bSame = false;

					if (bSame)
						pFound = &ExtCredit[j];
					}
				}

			//	If we found a duplicate, then add the extension
			//	to the struct

			if (pFound)
				{
				//	Make sure we don't already have an extension of the same name listed

				bool bDuplicate = false;
				for (j = 0; j < pFound->Extensions.GetCount() && !bDuplicate; j++)
					if (strEquals(pFound->Extensions[j], pExt->GetName()))
						bDuplicate = true;

				if (!bDuplicate)
					pFound->Extensions.Insert(strToLower(pExt->GetName()));
				}

			//	Otherwise, add the new entry

			else
				{
				SExtCredit *pNew = ExtCredit.Insert();
				pNew->Extensions.Insert(strToLower(pExt->GetName()));
				pNew->Credits = pExt->GetCredits();
				}
			}
		}

	//	Add all the credits

	for (i = 0; i < ExtCredit.GetCount(); i++)
		{
		SExtCredit *pExt = &ExtCredit[i];

		//	Add the names of the extensions that are credited
		//	to the given people

		for (j = 0; j < pExt->Extensions.GetCount(); j++)
			if (j == pExt->Extensions.GetCount() - 1)
				{
				CString sCredit = (j == 0 ?
						pExt->Extensions[j]
						: strPatternSubst(CONSTLIT("and %s"), pExt->Extensions[j]));

				pAni->AddTextLine(sCredit,
						&m_Fonts.SubTitle, 
						m_Fonts.wLightTitleColor, 
						CG16bitFont::AlignCenter,
						(j == 0 ? m_Fonts.Title.GetHeight() : 0));
				}
			else
				{
				pAni->AddTextLine(strPatternSubst(CONSTLIT("%s,"), pExt->Extensions[j]),
						&m_Fonts.SubTitle, 
						m_Fonts.wLightTitleColor, 
						CG16bitFont::AlignCenter,
						(j == 0 ? m_Fonts.Title.GetHeight() : 0));
				}

		//	Add the people names

		for (j = 0; j < pExt->Credits.GetCount(); j++)
			pAni->AddTextLine(pExt->Credits[j], 
					&m_Fonts.Header, 
					m_Fonts.wTitleColor, 
					CG16bitFont::AlignCenter,
					(j == 0 ? m_Fonts.Header.GetHeight() : 0));
		}

	//	Copyright

	pAni->AddTextLine(m_sVersion, &m_Fonts.SubTitle, m_Fonts.wTitleColor, CG16bitFont::AlignCenter, m_Fonts.Title.GetHeight());
	pAni->AddTextLine(m_sCopyright, &m_Fonts.Medium, m_Fonts.wTextColor, CG16bitFont::AlignCenter);
	pAni->AddTextLine(CONSTLIT("Transcendence is a registered trademark"), &m_Fonts.Medium, m_Fonts.wTextColor, CG16bitFont::AlignCenter);
	pAni->AddTextLine(CONSTLIT("http://transcendence-game.com"), &m_Fonts.Medium, m_Fonts.wTextColor, CG16bitFont::AlignCenter);

	//	Animate

	pAni->AnimateLinearScroll(1.0);

	//	Done

	*retpAnimatron = pAni;
	}

void CTranscendenceWnd::CreateNewsAnimation (CMultiverseNewsEntry *pEntry, IAnimatron **retpAnimatron)

//	CreateNewsAnimation
//
//	Creates animation of a Multiverse news entry.

	{
	int iDuration = 600;
	int iInitialFade = 30;
	int iEndFade = 30;

	//	Compute some metrics for the pane based on the entry information

	int cxInnerPane = NEWS_PANE_WIDTH - (2 * NEWS_PANE_PADDING_X);

	CG16bitImage *pImage = pEntry->LoadImageHandoff();
	int cyImage = (pImage ? pImage->GetHeight() : 0);

	TArray<CString> TitleLines;
	m_Fonts.SubTitle.BreakText(pEntry->GetTitle(), cxInnerPane, &TitleLines);
	int cyTitle = m_Fonts.SubTitle.GetHeight() * TitleLines.GetCount();

	TArray<CString> BodyLines;
	m_Fonts.Medium.BreakText(pEntry->GetBody(), cxInnerPane, &BodyLines);
	int cyBody = m_Fonts.Medium.GetHeight() * BodyLines.GetCount();

	TArray<CString> FooterLines;
	m_Fonts.MediumHeavyBold.BreakText(pEntry->GetCallToActionText(), cxInnerPane, &FooterLines);
	int cyFooter = m_Fonts.MediumHeavyBold.GetHeight() * FooterLines.GetCount();

	int cyPane = cyImage
			+ cyTitle
			+ NEWS_PANE_INNER_SPACING_Y
			+ cyBody
			+ NEWS_PANE_INNER_SPACING_Y
			+ cyFooter
			+ NEWS_PANE_PADDING_Y;

	int xPane = m_rcIntroMain.left + (RectWidth(m_rcIntroMain) / 2) + (RectWidth(m_rcIntroMain) / 6);
	int yPane = m_rcIntroMain.top + ((RectHeight(m_rcIntroMain) - cyPane) / 2);

	//	Create sequencer to hold everything The origin of the sequencer is
	//	at the top-center of the pane.

	CAniSequencer *pSeq = new CAniSequencer;
	pSeq->SetPropertyVector(PROP_POSITION, CVector(xPane, yPane));

	int xLeft = -NEWS_PANE_WIDTH / 2;

	//	Create a button that will respond to clicks on the news pane

	CAniButton *pButton = new CAniButton;
	pButton->SetPropertyVector(PROP_POSITION, CVector(xLeft, 0));
	pButton->SetPropertyVector(PROP_SCALE, CVector(NEWS_PANE_WIDTH, cyPane));
	pButton->SetStyle(STYLE_DOWN, NULL);
	pButton->SetStyle(STYLE_HOVER, NULL);
	pButton->SetStyle(STYLE_NORMAL, NULL);
	pButton->SetStyle(STYLE_DISABLED, NULL);
	pButton->SetStyle(STYLE_TEXT, NULL);
	pButton->AddListener(EVENT_ON_CLICK, this, CMD_OPEN_NEWS);

	pSeq->AddTrack(pButton, 0);

	//	Create the background

	CAniRoundedRect *pPane = new CAniRoundedRect;
	pPane->SetPropertyVector(PROP_POSITION, CVector(xLeft, 0));
	pPane->SetPropertyVector(PROP_SCALE, CVector(NEWS_PANE_WIDTH, cyPane));
	pPane->SetPropertyColor(PROP_COLOR, RGB_NEWS_PANE_BACKGROUND);
	pPane->SetPropertyOpacity(PROP_OPACITY, 64);
	pPane->SetPropertyInteger(PROP_UL_RADIUS, NEWS_PANE_CORNER_RADIUS);
	pPane->SetPropertyInteger(PROP_UR_RADIUS, NEWS_PANE_CORNER_RADIUS);
	pPane->SetPropertyInteger(PROP_LL_RADIUS, NEWS_PANE_CORNER_RADIUS);
	pPane->SetPropertyInteger(PROP_LR_RADIUS, NEWS_PANE_CORNER_RADIUS);
	pPane->AnimateLinearFade(iDuration, iInitialFade, iEndFade, 64);

	pSeq->AddTrack(pPane, 0);

	//	Add the content

	int yPos = 0;
	int xInnerLeft = -(cxInnerPane / 2);

	//	Create the image

	if (pImage)
		{
		//	If the image is wide enough to hit the rounded corners, then we
		//	need to mask it out.

		if (pImage->GetWidth() > (NEWS_PANE_WIDTH - 2 * NEWS_PANE_CORNER_RADIUS))
			{
			//	Create a mask the size of the pane and apply it to the image
			//	(We own the image so we can modify it).

			CG16bitImage PaneMask;
			CreateRoundedRectAlpha(NEWS_PANE_WIDTH, cyPane, NEWS_PANE_CORNER_RADIUS, &PaneMask);
			pImage->IntersectMask(0, 
					0, 
					PaneMask.GetWidth(), 
					PaneMask.GetHeight(), 
					PaneMask,
					(pImage->GetWidth() - PaneMask.GetWidth()) / 2,
					0);
			}

		//	Create an animatron

		CAniRect *pRect = new CAniRect;
		pRect->SetPropertyVector(PROP_POSITION, CVector(-pImage->GetWidth() / 2, yPos));
		pRect->SetPropertyVector(PROP_SCALE, CVector(pImage->GetWidth(), pImage->GetHeight()));
		pRect->SetFillMethod(new CAniImageFill(pImage, true));
		pRect->AnimateLinearFade(iDuration, iInitialFade, iEndFade);

		pSeq->AddTrack(pRect, 0);

		yPos += cyImage;
		}

	//	Create the title

	IAnimatron *pText;
	CAniText::Create(pEntry->GetTitle(),
			CVector(xInnerLeft, yPos),
			&m_Fonts.SubTitle,
			CG16bitFont::AlignCenter,
			m_Fonts.wTitleColor,
			&pText);
	pText->SetPropertyVector(PROP_SCALE, CVector(cxInnerPane, cyPane));
	pText->AnimateLinearFade(iDuration, iInitialFade, iEndFade);
	pSeq->AddTrack(pText, 0);

	yPos += cyTitle + NEWS_PANE_INNER_SPACING_Y;

	//	Create the text

	CAniText::Create(pEntry->GetBody(),
			CVector(xInnerLeft, yPos),
			&m_Fonts.Medium,
			CG16bitFont::AlignCenter,
			m_Fonts.wTitleColor,
			&pText);
	pText->SetPropertyVector(PROP_SCALE, CVector(cxInnerPane, cyPane));
	pText->AnimateLinearFade(iDuration, iInitialFade, iEndFade);
	pSeq->AddTrack(pText, 0);

	yPos += cyBody + NEWS_PANE_INNER_SPACING_Y;

	//	Create the call to action

	CAniText::Create(pEntry->GetCallToActionText(),
			CVector(xInnerLeft, yPos),
			&m_Fonts.MediumHeavyBold,
			CG16bitFont::AlignCenter,
			m_Fonts.wTitleColor,
			&pText);
	pText->SetPropertyVector(PROP_SCALE, CVector(cxInnerPane, cyPane));
	pText->AnimateLinearFade(iDuration, iInitialFade, iEndFade);
	pSeq->AddTrack(pText, 0);

	//	Remember the URL to open when the user clicks

	m_sNewsURL = pEntry->GetCallToActionURL();

	//	Done

	*retpAnimatron = pSeq;
	}

void CTranscendenceWnd::CreatePlayerBarAnimation (IAnimatron **retpAni)

//	CreatePlayerBarAnimation
//
//	Creates an animation for the top bar.

	{
	const CVisualPalette &VI = g_pHI->GetVisuals();
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);
	CCloudService &Service = m_pTC->GetService();

	RECT rcRect;
	VI.GetWidescreenRect(g_pHI->GetScreen(), &rcRect);

	//	Create a sequencer to hold all the controls

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(rcRect.left, rcRect.top - TITLE_BAR_HEIGHT), &pRoot);

	//	Create buttons (from right to left)

	int x = RectWidth(rcRect) - BUTTON_WIDTH;
	IAnimatron *pButton;

	//	Debug icon

	if (m_pTC->GetOptionBoolean(CGameSettings::debugMode))
		{
		VI.CreateImageButton(pRoot, CMD_TOGGLE_DEBUG, x, (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2, &VI.GetImage(imageDebugIcon), CONSTLIT("Debug"), 0, &pButton);
		pButton->AddListener(EVENT_ON_CLICK, this, CMD_TOGGLE_DEBUG);

		x -= (BUTTON_WIDTH + PADDING_LEFT);
		}

	//	Music toggle

	VI.CreateImageButton(pRoot, CMD_TOGGLE_MUSIC, x, (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2, &VI.GetImage(imageMusicIconOn), CONSTLIT("Music On"), 0, &pButton);
	pButton->AddListener(EVENT_ON_CLICK, this, CMD_TOGGLE_MUSIC);

	x -= (BUTTON_WIDTH + PADDING_LEFT);

	//	Profile

	if (Service.HasCapability(ICIService::userProfile))
		{
		VI.CreateImageButton(pRoot, CMD_SHOW_PROFILE, x, (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2, &VI.GetImage(imageProfileIcon), CONSTLIT("Records"), 0, &pButton);
		pButton->AddListener(EVENT_ON_CLICK, this, CMD_SHOW_PROFILE);

		x -= (BUTTON_WIDTH + PADDING_LEFT);
		}

	//	Mod Exchange

	if (Service.HasCapability(ICIService::modExchange))
		{
		VI.CreateImageButton(pRoot, CMD_SHOW_MOD_EXCHANGE, x, (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2, &VI.GetImage(imageModExchangeIcon), CONSTLIT("Mod Collection"), 0, &pButton);
		pButton->AddListener(EVENT_ON_CLICK, this, CMD_SHOW_MOD_EXCHANGE);
		}

	//	Done

	*retpAni = pRoot;
	}

ALERROR CTranscendenceWnd::CreateRandomShip (CSystem *pSystem, DWORD dwClass, CSovereign *pSovereign, CShip **retpShip)

//	CreateRandomShip
//
//	Creates a random ship

	{
	ALERROR error;
	int i;

	//	Figure out the class

	int iTimeOut = 100;
	CShipClass *pShipClass;
	if (dwClass == 0
			|| (pShipClass = g_pUniverse->FindShipClass(dwClass)) == NULL)
		{
		do
			pShipClass = g_pUniverse->GetShipClass(mathRandom(0, g_pUniverse->GetShipClassCount()-1));
		while (iTimeOut-- > 0
				&&	(pShipClass->GetScore() > 1000 
					|| pShipClass->IsPlayerShip()
					|| pShipClass->IsVirtual()
					|| !pShipClass->HasLiteralAttribute(ATTRIB_GENERIC_SHIP_CLASS)));
		}

	//	Normally we create a single ship, but sometimes we create lots

	int iCount;
	int iRoll = mathRandom(1, 100);

	//	Adjust the roll for capital ships

	if (pShipClass->GetHullMass() >= 10000)
		iRoll -= 9;
	else if (pShipClass->GetHullMass() >= 1000)
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

		CIntroShipController *pNewController = new CIntroShipController(this, pShip->GetController());
		pShip->SetController(pNewController, false);
		pNewController->SetShip(pShip);
		pShip->SetData(CONSTLIT("IntroController"), CONSTLIT("True"));

		*retpShip = pShip;
		}

	g_pUniverse->SetLogImageLoad(true);

	return NOERROR;
	}

void CTranscendenceWnd::CreateScoreAnimation (const CGameRecord &Stats, IAnimatron **retpAnimatron)

//	CreateScoreAnimation
//
//	Creates an animation of the given score

	{
	int i;
	int iDuration = 300;
	int x = m_rcIntroMain.left + RectWidth(m_rcIntroMain) / 2;
	int y = m_rcIntroMain.bottom - RectHeight(m_rcIntroMain) / 3;

	//	Create sequencer to hold everything

	CAniSequencer *pSeq = new CAniSequencer;

	//	Create the score

	CAniText *pCredit = new CAniText;
	pCredit->SetPropertyVector(CONSTLIT("position"), CVector((Metric)x, (Metric)y));
	pCredit->SetPropertyColor(CONSTLIT("color"), m_Fonts.wTitleColor);
	pCredit->SetPropertyString(CONSTLIT("text"), strFromInt(Stats.GetScore()));

	pCredit->SetPropertyFont(CONSTLIT("font"), &m_Fonts.Title);
	pCredit->SetFontFlags(CG16bitFont::AlignCenter);

	pCredit->AnimateLinearFade(iDuration, 30, 30);

	pSeq->AddTrack(pCredit, 0);
	y += m_Fonts.Title.GetHeight();

	//	Player name

	CAniText *pName = new CAniText;
	pName->SetPropertyVector(CONSTLIT("position"), CVector((Metric)x, (Metric)y));
	pName->SetPropertyColor(CONSTLIT("color"), m_Fonts.wTextColor);
	pName->SetPropertyString(CONSTLIT("text"), Stats.GetPlayerName());

	pName->SetPropertyFont(CONSTLIT("font"), &m_Fonts.SubTitle);
	pName->SetFontFlags(CG16bitFont::AlignCenter);

	pName->AnimateLinearFade(iDuration, 30, 30);

	pSeq->AddTrack(pName, 5);
	y += m_Fonts.SubTitle.GetHeight();

	//	Epitaph

	TArray<CString> EpitaphLines;
	m_Fonts.Header.BreakText(strCapitalize(Stats.GetEndGameEpitaph()), 512, &EpitaphLines);

	for (i = 0; i < EpitaphLines.GetCount(); i++)
		{
		CAniText *pLine = new CAniText;
		pLine->SetPropertyVector(CONSTLIT("position"), CVector((Metric)x, (Metric)y));
		pLine->SetPropertyColor(CONSTLIT("color"), m_Fonts.wTextColor);
		pLine->SetPropertyString(CONSTLIT("text"), EpitaphLines[i]);

		pLine->SetPropertyFont(CONSTLIT("font"), &m_Fonts.Header);
		pLine->SetFontFlags(CG16bitFont::AlignCenter);

		pLine->AnimateLinearFade(iDuration, 30, 30);

		pSeq->AddTrack(pLine, 5);
		y += m_Fonts.Header.GetHeight();
		}

	//	Done

	*retpAnimatron = pSeq;
	}

void CTranscendenceWnd::CreateShipDescAnimation (CShip *pShip, IAnimatron **retpAnimatron)

//	CreateShipDescAnimation
//
//	Creates animation describing the given ship

	{
	int i, j;
	int iDuration = 600;
	int iInterLineDelay = 1;
	int iDelay = 0;
	int x = m_rcIntroMain.left + (RectWidth(m_rcIntroMain) / 2) + (RectWidth(m_rcIntroMain) / 6);
	int y = m_rcIntroMain.bottom - RectHeight(m_rcIntroMain) / 3;

	//	Create sequencer to hold everything.

	CAniSequencer *pSeq = new CAniSequencer;

	//	Show the ship class

	CString sClassName = strToLower(pShip->GetName());
	int cyClassName;
	int cxClassName = m_Fonts.SubTitle.MeasureText(sClassName, &cyClassName);
	int cySectionSpacing = cyClassName / 6;

	IAnimatron *pText;
	CAniText::Create(sClassName,
			CVector((Metric)x, (Metric)y),
			&m_Fonts.SubTitle,
			CG16bitFont::AlignCenter,
			m_Fonts.wTitleColor,
			&pText);
	pText->AnimateLinearFade(iDuration, 0, 30);
	pSeq->AddTrack(pText, 0);

	y += cyClassName + cySectionSpacing;
	iDelay += iInterLineDelay * 3;

	//	Weapons label

	CAniText::Create(CONSTLIT("WEAPONS:"),
			CVector((Metric)x - cyClassName / 4, (Metric)(y + m_Fonts.Medium.GetAscent() - m_Fonts.Small.GetAscent())),
			&m_Fonts.Small,
			CG16bitFont::AlignRight,
			m_Fonts.wLightTitleColor,
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	//	Collect duplicate weapons

	struct SWeaponDesc
		{
		CString sWeaponName;
		int iCount;
		};

	TArray<SWeaponDesc> WeaponList;

	for (i = 0; i < pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = pShip->GetDevice(i);
		if (pDevice->IsEmpty())
			continue;

		if (pDevice->GetCategory() == itemcatWeapon || pDevice->GetCategory() == itemcatLauncher)
			{
			CString sName = pDevice->GetClass()->GetItemType()->GetNounPhrase(nounActual | nounCapitalize);

			//	Look for the weapon in the list

			bool bFound = false;
			for (j = 0; j < WeaponList.GetCount() && !bFound; j++)
				if (strEquals(WeaponList[j].sWeaponName, sName))
					{
					WeaponList[j].iCount++;
					bFound = true;
					}

			//	Add if necessary

			if (!bFound)
				{
				SWeaponDesc *pWeapon = WeaponList.Insert();
				pWeapon->sWeaponName = sName;
				pWeapon->iCount = 1;
				}
			}
		}

	//	Output weapon list

	if (WeaponList.GetCount() == 0)
		{
		CAniText::Create(CONSTLIT("None"),
				CVector((Metric)x + cyClassName / 4, (Metric)y),
				&m_Fonts.Medium,
				0,
				m_Fonts.wTitleColor,
				&pText);
		pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
		pSeq->AddTrack(pText, iDelay);

		y += m_Fonts.Medium.GetHeight();
		iDelay += iInterLineDelay;
		}
	else
		{
		for (i = 0; i < WeaponList.GetCount() && i < 6; i++)
			{
			CAniText::Create((WeaponList[i].iCount == 1 ? WeaponList[i].sWeaponName
						: strPatternSubst(CONSTLIT("%s (x%d)"), WeaponList[i].sWeaponName, WeaponList[i].iCount)),
					CVector((Metric)x + cyClassName / 4, (Metric)y),
					&m_Fonts.Medium,
					0,
					m_Fonts.wTitleColor,
					&pText);
			pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
			pSeq->AddTrack(pText, iDelay);

			y += m_Fonts.Medium.GetHeight();
			iDelay += iInterLineDelay;
			}
		}

	//	Shields

	y += cySectionSpacing;

	CAniText::Create(CONSTLIT("SHIELDS:"),
			CVector((Metric)x - cyClassName / 4, (Metric)(y + m_Fonts.Medium.GetAscent() - m_Fonts.Small.GetAscent())),
			&m_Fonts.Small,
			CG16bitFont::AlignRight,
			m_Fonts.wLightTitleColor,
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	CInstalledDevice *pShields = pShip->GetNamedDevice(devShields);

	CAniText::Create((pShields ? pShields->GetClass()->GetItemType()->GetNounPhrase(nounActual | nounCapitalize) : CONSTLIT("None")),
			CVector((Metric)x + cyClassName / 4, (Metric)y),
			&m_Fonts.Medium,
			0,
			m_Fonts.wTitleColor,
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	y += m_Fonts.Medium.GetHeight();
	iDelay += iInterLineDelay;

	//	Armor

	y += cySectionSpacing;

	CAniText::Create(CONSTLIT("ARMOR:"),
			CVector((Metric)x - cyClassName / 4, (Metric)(y + m_Fonts.Medium.GetAscent() - m_Fonts.Small.GetAscent())),
			&m_Fonts.Small,
			CG16bitFont::AlignRight,
			m_Fonts.wLightTitleColor,
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	int iCount = pShip->GetArmorSectionCount();
	CInstalledArmor *pArmor = (iCount > 0 ? pShip->GetArmorSection(0) : NULL);
	CString sArmor = (pArmor ? pArmor->GetClass()->GetItemType()->GetNounPhrase(nounActual | nounCapitalize | nounShort) : NULL_STR);

	CAniText::Create((pArmor ? strPatternSubst(CONSTLIT("%s (x%d)"), sArmor, iCount) : CONSTLIT("None")),
			CVector((Metric)x + cyClassName / 4, (Metric)y),
			&m_Fonts.Medium,
			0,
			m_Fonts.wTitleColor,
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	y += m_Fonts.Medium.GetHeight();
	iDelay += iInterLineDelay;

	//	Done

	*retpAnimatron = pSeq;
	}

void CTranscendenceWnd::CreateTitleAnimation (IAnimatron **retpAnimatron)

//	CreateTitleAnimation
//
//	Creates opening titles animation

	{
	CAniSequencer *pSeq = new CAniSequencer;
	int iTime = 0;

	//	Figure out the position

	int xMidCenter = m_rcIntroMain.left + RectWidth(m_rcIntroMain) / 2;
	int yMidCenter = m_rcIntroMain.bottom - RectHeight(m_rcIntroMain) / 3;
	IAnimatron *pAnimation;

	//	Create Transcendence title text

	m_UIRes.CreateTitleAnimation(xMidCenter, yMidCenter, 150, &pAnimation);
	pSeq->AddTrack(pAnimation, iTime);

	//	Create version

	int y = m_rcIntroMain.bottom - (m_Fonts.MediumHeavyBold.GetHeight() + 2 * m_Fonts.Medium.GetHeight());
	CAniText::Create(m_sVersion,
			CVector(xMidCenter, y),
			&m_Fonts.MediumHeavyBold,
			CG16bitFont::AlignCenter,
			RGB_VERSION_COLOR,
			&pAnimation);
	pAnimation->AnimateLinearFade(150, 30, 30);
	pSeq->AddTrack(pAnimation, iTime);

	y += m_Fonts.MediumHeavyBold.GetHeight();

	//	Copyright

	CAniText::Create(m_sCopyright,
			CVector(xMidCenter, y),
			&m_Fonts.Medium,
			CG16bitFont::AlignCenter,
			RGB_COPYRIGHT_COLOR,
			&pAnimation);
	pAnimation->AnimateLinearFade(150, 30, 30);
	pSeq->AddTrack(pAnimation, iTime);

	iTime += 150;

	//	Done

	*retpAnimatron = pSeq;
	}

void CTranscendenceWnd::DestroyIntroShips (void)

//	DestroyIntroShips
//
//	Destroys all ships of the same class as the POV

	{
	int i;

	CShip *pShip = g_pUniverse->GetPOV()->AsShip();
	if (pShip == NULL)
		return;

	//	Destroy all ships of the current class

	CSystem *pSystem = pShip->GetSystem();
	CShipClass *pClassToDestroy = pShip->GetClass();
	TArray<CSpaceObject *> ShipsToDestroy;
	CSpaceObject *pOtherShip = NULL;
	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);
		CShip *pShip;
		if (pObj 
				&& !pObj->IsInactive()
				&& !pObj->IsVirtual()
				&& (pShip = pObj->AsShip()))
			{
			if (pShip->GetClass() == pClassToDestroy)
				ShipsToDestroy.Insert(pObj);
			else if (pOtherShip == NULL)
				pOtherShip = pObj;
			}
		}

	//	Destroy ships

	for (i = 0; i < ShipsToDestroy.GetCount(); i++)
		ShipsToDestroy[i]->Destroy(removedFromSystem, CDamageSource());
	}

int CTranscendenceWnd::GetHighScoresPos (void)

//	GetHighScoresPos
//
//	Returns the current position of the high scores performance

	{
	int i;
	CHighScoreList *pHighScoreList = GetHighScoreList();

	IAnimatron *pAni = m_Reanimator.GetPerformance(m_dwHighScoresPerformance);
	if (pAni == NULL)
		return -1;

	int yPos = (int)pAni->GetPropertyMetric(PROP_SCROLL_POS);

	for (i = 0; i < pHighScoreList->GetCount(); i++)
		if (m_pHighScorePos[i] >= yPos)
			return Max(0, i);

	return -1;
	}

void CTranscendenceWnd::OnAccountChanged (const CMultiverseModel &Multiverse)

//	OnAccountChanged
//
//	The user account has changed

	{
	switch (m_State)
		{
		case gsIntro:
			SetAccountControls(Multiverse);
			break;
		}
	}

void CTranscendenceWnd::OnCommandIntro (const CString &sCmd, void *pData)

//	OnCommandIntro
//
//	Handle commands from Reanimator, etc.

	{
	if (strEquals(sCmd, CMD_SHOW_PROFILE))
		g_pHI->HICommand(CMD_UI_SHOW_PROFILE);

	else if (strEquals(sCmd, CMD_TOGGLE_MUSIC))
		{
		m_pTC->SetOptionBoolean(CGameSettings::noMusic, !m_pTC->GetOptionBoolean(CGameSettings::noMusic));
		SetMusicOption();
		}

	else if (strEquals(sCmd, CMD_TOGGLE_DEBUG))
		{
		m_pTC->GetModel().SetDebugMode(!m_pTC->GetModel().GetDebugMode());
		SetDebugOption();
		}

	else if (strEquals(sCmd, CMD_ACCOUNT))
		g_pHI->HICommand(CMD_UI_SHOW_LOGIN);

	else if (strEquals(sCmd, CMD_SIGN_OUT))
		g_pHI->HICommand(CMD_UI_SIGN_OUT);

	else if (strEquals(sCmd, CMD_SHOW_MOD_EXCHANGE))
		g_pHI->HICommand(CMD_UI_SHOW_MOD_EXCHANGE);

	else if (strEquals(sCmd, CMD_CHANGE_PASSWORD))
		g_pHI->HICommand(CMD_UI_CHANGE_PASSWORD);

	else if (strEquals(sCmd, CMD_OPEN_NEWS))
		{
		if (!m_sNewsURL.IsBlank())
			sysOpenURL(m_sNewsURL);
		}

	else if (strEquals(sCmd, CMD_SERVICE_NEWS_LOADED))
		SetIntroState(isNews);
	}

void CTranscendenceWnd::OnDblClickIntro (int x, int y, DWORD dwFlags)

//	OnDblClickIntro
//
//	Handle WM_LBUTTONDBLCLK

	{
	if (m_bOverwriteGameDlg)
		return;

	//	See if the animator will handle it

	bool bCapture = false;
	if (m_Reanimator.HandleLButtonDblClick(x, y, dwFlags, &bCapture))
		{
		if (bCapture)
			::SetCapture(g_pHI->GetHWND());
		return;
		}

	if (m_ButtonBarDisplay.OnLButtonDoubleClick(x, y))
		return;
	}

void CTranscendenceWnd::OnCharIntro (char chChar, DWORD dwKeyData)

//	OnCharIntro
//
//	Handle WM_CHAR

	{
	int i;

	if (m_bOverwriteGameDlg)
		NULL;

	//	See if the animator will handle it

	else if (m_Reanimator.HandleChar(chChar, dwKeyData))
		NULL;

	else if (m_ButtonBarDisplay.OnChar(chChar))
		NULL;

	else if (m_iIntroState == isEnterShipClass)
		{
		switch (chChar)
			{
			//	VK_BACKSPACE

			case '\010':
				if (!m_sCommand.IsBlank())
					m_sCommand = strSubString(m_sCommand, 0, m_sCommand.GetLength() - 1);
				break;

			//	VK_RETURN

			case '\015':
				{
				CShip *pShip = g_pUniverse->GetPOV()->AsShip();
				DWORD dwSovereign = (pShip ? pShip->GetSovereign()->GetUNID() : 0);

				//	Parse the string into a ship class

				CShipClass *pClass = g_pUniverse->FindShipClassByName(m_sCommand);
				if (pClass == NULL)
					{
					SetIntroState(isShipStats);
					break;
					}

				//	Destroy and create

				DestroyIntroShips();
				CreateIntroShips(pClass->GetUNID(), dwSovereign);
				CancelCurrentIntroState();
				break;
				}

			//	VK_ESCAPE

			case '\033':
				CancelCurrentIntroState();
				break;

			default:
				if (chChar >= ' ')
					m_sCommand.Append(CString(&chChar, 1));
				break;
			}
		}

	else
		{
		switch (chChar)
			{
			case ' ':
				CancelCurrentIntroState();
				break;

			case '!':
				SetIntroState(isEnterShipClass);
				break;

			case 'C':
			case 'c':
				SetIntroState(isCredits);
				break;

			case 'D':
			case 'd':
				{
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
							&& !pObj->IsVirtual()
							&& !pObj->IsInactive())
						iCount++;
					}

				//	If we already have too many, we don't do anything

				if (iCount > MAX_INTRO_SHIPS)
					break;

				//	Create a duplicate

				CreateIntroShips(pShip->GetClassUNID(), pShip->GetSovereign()->GetUNID());
				break;
				}

			case 'H':
			case 'h':
				if (m_iIntroState == isHighScores)
					SetIntroState(isBlank);
				else
					SetIntroState(isHighScores);
				break;

			case 'K':
			case 'k':
				DestroyIntroShips();
				CreateIntroShips();
				break;

			case 'L':
			case 'l':
				DoCommand(CMD_CONTINUE_OLD_GAME);
				break;

			case 'N':
			case 'n':
				{
				CShip *pShip = g_pUniverse->GetPOV()->AsShip();
				if (pShip == NULL)
					break;

				//	Get the UNID of the next ship class in order

				DWORD dwNewShipClass = 0;
				if (chChar == 'n' || chChar == 'N')
					{
					DWORD dwClass = pShip->GetClassUNID();
					int iIndex = -1;
					for (i = 0; i < g_pUniverse->GetShipClassCount(); i++)
						if (g_pUniverse->GetShipClass(i)->GetUNID() == dwClass)
							{
							iIndex = i;
							break;
							}

					CShipClass *pShipClass;
					do
						{
						if (iIndex == -1 || (iIndex + 1) == g_pUniverse->GetShipClassCount())
							iIndex = 0;
						else
							iIndex++;

						pShipClass = g_pUniverse->GetShipClass(iIndex);
						}
					while (pShipClass->IsVirtual());

					//	Set the variable so that the next ship created will
					//	have the given class

					dwNewShipClass = pShipClass->GetUNID();
					}

				//	Destroy all ships of the current class

				DestroyIntroShips();

				//	Create a new ship

				CreateIntroShips(dwNewShipClass, pShip->GetSovereign()->GetUNID());
				break;
				}

			case 'O':
			case 'o':
				{
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
							&& !pObj->IsInactive()
							&& !pObj->IsVirtual())
						Opponents.Insert(pObj);
					}

				//	Pick a random opponent and set the POV

				if (Opponents.GetCount() > 0)
					{
					g_pUniverse->SetPOV(Opponents[mathRandom(0, Opponents.GetCount() - 1)]);
					SetIntroState(isShipStats);
					}

				break;
				}

			case 'P':
				{
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
							&& !pObj->IsVirtual()
							&& !pObj->IsInactive())
						{
						g_pUniverse->SetPOV(pObj);
						SetIntroState(isShipStats);
						break;
						}
					}

				break;
				}

			case 'p':
				{
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
							&& !pObj->IsVirtual()
							&& !pObj->IsInactive())
						{
						g_pUniverse->SetPOV(pObj);
						SetIntroState(isShipStats);
						break;
						}
					}

				break;
				}

			case 'Q':
			case 'q':
				DoCommand(CMD_QUIT_GAME);
				break;

			case 'S':
			case 's':
				if (m_iIntroState == isShipStats)
					SetIntroState(isBlank);
				else
					SetIntroState(isShipStats);
				break;

			case 'V':
			case 'v':
				SetIntroState(isOpeningTitles);
				break;
			}
		}
	}

void CTranscendenceWnd::OnIntroPOVSet (CSpaceObject *pObj)

//	OnIntroPOVSet
//
//	POV has changed

	{
	switch (m_iIntroState)
		{
		case isBlank:
		case isBlankThenRandom:
//		case isEnterShipClass:
		case isShipStats:
			SetIntroState(isShipStats);
			break;
		}
	}

void CTranscendenceWnd::OnKeyDownIntro (int iVirtKey, DWORD dwKeyData)

//	OnKeyDownIntro
//
//	Handle WM_KEYDOWN

	{
	if (m_bOverwriteGameDlg)
		NULL;

	else if (m_ButtonBarDisplay.OnKeyDown(iVirtKey))
		NULL;

	else if (m_iIntroState == isEnterShipClass)
		NULL;

	else if (m_Reanimator.IsPaused())
		m_Reanimator.Resume();

	else
		{
		switch (iVirtKey)
			{
			case VK_ESCAPE:
				CancelCurrentIntroState();
				break;

			case VK_RETURN:
				DoCommand(CMD_START_NEW_GAME);
				break;

			case VK_UP:
				if (m_iIntroState == isHighScores || m_iIntroState == isHighScoresEndGame)
					SetHighScoresPrev();
				break;

			case VK_DOWN:
				if (m_iIntroState == isHighScores || m_iIntroState == isHighScoresEndGame)
					SetHighScoresNext();
				break;

			case VK_PRIOR:
				if (m_iIntroState == isHighScores || m_iIntroState == isHighScoresEndGame)
					SetHighScoresPrev();
				else
					m_Reanimator.FFBurst(-32, 15);
				break;

			case VK_NEXT:
				if (m_iIntroState == isHighScores || m_iIntroState == isHighScoresEndGame)
					SetHighScoresNext();
				else
					m_Reanimator.FFBurst(32, 15);
				break;

			case VK_PAUSE:
				m_Reanimator.Pause();
				break;

			case VK_F1:
				g_pHI->HICommand(CONSTLIT("uiShowHelp"));
				break;

			case VK_F2:
				g_pHI->HICommand(CONSTLIT("uiShowGameStats"));
				break;
			}
		}
	}

void CTranscendenceWnd::OnLButtonDownIntro (int x, int y, DWORD dwFlags)

//	OnLButtonDownIntro
//
//	Handle WM_LBUTTONDOWN

	{
	if (m_bOverwriteGameDlg)
		{
		POINT pt;
		pt.x = x;
		pt.y = y;

		if (::PtInRect(&m_rcOverwriteGameOK, pt))
			{
			m_bSavedGame = false;
			m_bOverwriteGameDlg = false;
			DoCommand(CMD_START_NEW_GAME);
			}
		else if (::PtInRect(&m_rcOverwriteGameCancel, pt))
			m_bOverwriteGameDlg = false;

		return;
		}

	//	See if the animator will handle it

	bool bCapture = false;
	if (m_Reanimator.HandleLButtonDown(x, y, dwFlags, &bCapture))
		{
		if (bCapture)
			::SetCapture(g_pHI->GetHWND());
		return;
		}

	if (m_ButtonBarDisplay.OnLButtonDown(x, y))
		return;
	}

void CTranscendenceWnd::OnLButtonUpIntro (int x, int y, DWORD dwFlags)

//	OnLButtonUpIntro
//
//	Handle WM_LBUTTONUP

	{
	if (::GetCapture() == g_pHI->GetHWND())
		::ReleaseCapture();

	if (m_bOverwriteGameDlg)
		return;

	//	See if the animator will handle it

	if (m_Reanimator.HandleLButtonUp(x, y, dwFlags))
		return;
	}

void CTranscendenceWnd::OnMouseMoveIntro (int x, int y, DWORD dwFlags)

//	OnMouseMoveIntro
//
//	Handle WM_MOUSEMOVE

	{
	if (m_bOverwriteGameDlg)
		return;

	//	See if the animator will handle it

	if (m_Reanimator.HandleMouseMove(x, y, dwFlags))
		return;

	m_ButtonBarDisplay.OnMouseMove(x, y);
	}

void CTranscendenceWnd::PaintDlgButton (const RECT &rcRect, const CString &sText)

//	PaintDlgButton
//
//	Paint button

	{
	CG16bitImage &TheScreen = g_pHI->GetScreen();

	TheScreen.Fill(rcRect.left, 
			rcRect.top,
			RectWidth(rcRect),
			RectHeight(rcRect),
			RGB_DIALOG_BUTTON_BACKGROUND);

	int cy;
	int cx = m_Fonts.MediumHeavyBold.MeasureText(sText, &cy);

	TheScreen.DrawText(rcRect.left + (RectWidth(rcRect) - cx) / 2,
			rcRect.top + (RectHeight(rcRect) - cy) / 2,
			m_Fonts.MediumHeavyBold,
			RGB_DIALOG_BUTTON_TEXT,
			sText);
	}

void CTranscendenceWnd::PaintOverwriteGameDlg (void)

//	PaintOverwriteGameDlg
//
//	Paint dialog box

	{
	CG16bitImage &TheScreen = g_pHI->GetScreen();

	//	Fade the background

	TheScreen.FillTrans(0, 0, TheScreen.GetWidth(), TheScreen.GetHeight(), 0, 128);

	//	Paint the dialog box frame

	TheScreen.Fill(m_rcOverwriteGameDlg.left, m_rcOverwriteGameDlg.top, DIALOG_WIDTH, DIALOG_HEIGHT, RGB_DIALOG_BACKGROUND);

	//	Paint the text

	int y = m_rcOverwriteGameDlg.top + DIALOG_SPACING_Y;
	int cy;
	int cx = m_Fonts.SubTitle.MeasureText(STR_OVERWRITE_GAME, &cy);
	TheScreen.DrawText(m_rcOverwriteGameDlg.left + (DIALOG_WIDTH - cx) / 2,
			y,
			m_Fonts.SubTitle,
			RGB_DIALOG_TEXT,
			STR_OVERWRITE_GAME);
	y += cy + DIALOG_SPACING_Y;

	cx = m_Fonts.Medium.MeasureText(STR_TEXT1, &cy);
	TheScreen.DrawText(m_rcOverwriteGameDlg.left + (DIALOG_WIDTH - cx) / 2,
			y,
			m_Fonts.Medium,
			RGB_DIALOG_TEXT,
			STR_TEXT1);
	y += cy;

	cx = m_Fonts.Medium.MeasureText(STR_TEXT2, &cy);
	TheScreen.DrawText(m_rcOverwriteGameDlg.left + (DIALOG_WIDTH - cx) / 2,
			y,
			m_Fonts.Medium,
			RGB_DIALOG_TEXT,
			STR_TEXT2);

	//	Paint the buttons

	PaintDlgButton(m_rcOverwriteGameOK, STR_OVERWRITE);
	PaintDlgButton(m_rcOverwriteGameCancel, STR_CANCEL);
	}

void CTranscendenceWnd::SetAccountControls (const CMultiverseModel &Multiverse)

//	SetAccountControls
//
//	Sets the user account controls

	{
	const CVisualPalette &VI = g_pHI->GetVisuals();
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	//	Get the account state

	CString sUsername;
	CMultiverseModel::EOnlineStates iState = Multiverse.GetOnlineState(&sUsername);

	CString sStatus;
	WORD wUsernameColor;
	switch (iState)
		{
		case CMultiverseModel::stateDisabled:
			sUsername = CONSTLIT("Offline");
			sStatus = CONSTLIT("Multiverse disabled");
			wUsernameColor = VI.GetColor(colorTextDialogLabel);
			break;

		case CMultiverseModel::stateNoUser:
			sUsername = CONSTLIT("Offline");
			sStatus = CONSTLIT("Click to register a new account");
			wUsernameColor = VI.GetColor(colorTextDialogLabel);
			break;

		case CMultiverseModel::stateOffline:
			sUsername = CONSTLIT("Offline");
			sStatus = CONSTLIT("Click to sign in");
			wUsernameColor = VI.GetColor(colorTextDialogLabel);
			break;

		case CMultiverseModel::stateOnline:
			sStatus = CONSTLIT("Signed in to the Multiverse");
			wUsernameColor = VI.GetColor(colorTextDialogInput);
			break;

		default:
			ASSERT(false);
		}

	//	Metrics

	int cxText = Max(SubTitleFont.MeasureText(sUsername), MediumFont.MeasureText(sStatus));

	//	Compute metrics

	RECT rcRect;
	VI.GetWidescreenRect(g_pHI->GetScreen(), &rcRect);

	//	Delete any existing controls

	m_Reanimator.DeleteElement(ID_ACCOUNT_CONTROLS);

	//	Create a sequencer to hold all the controls. We will be a child of the
	//	player bar animation, so we are relative to that.

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(0, 0), &pRoot);
	pRoot->SetID(ID_ACCOUNT_CONTROLS);

	//	The user icon is centered

	CAniRoundedRect *pIcon = new CAniRoundedRect;
	pIcon->SetPropertyVector(PROP_POSITION, CVector(0, (TITLE_BAR_HEIGHT - ICON_HEIGHT) / 2));
	pIcon->SetPropertyVector(PROP_SCALE, CVector(ICON_HEIGHT, ICON_WIDTH));
	pIcon->SetPropertyColor(PROP_COLOR, CG16bitImage::RGBValue(128, 128, 128));
	pIcon->SetPropertyOpacity(PROP_OPACITY, 255);
	pIcon->SetPropertyInteger(PROP_UL_RADIUS, ICON_CORNER_RADIUS);
	pIcon->SetPropertyInteger(PROP_UR_RADIUS, ICON_CORNER_RADIUS);
	pIcon->SetPropertyInteger(PROP_LL_RADIUS, ICON_CORNER_RADIUS);
	pIcon->SetPropertyInteger(PROP_LR_RADIUS, ICON_CORNER_RADIUS);

	pRoot->AddTrack(pIcon, 0);

	//	The user name baseline is centered.

	int y = (TITLE_BAR_HEIGHT / 2) - SubTitleFont.GetAscent();

	//	Create a hot spot over the entire text region (so that the user can 
	//	click on the username to sign in).

	if (iState == CMultiverseModel::stateNoUser || iState == CMultiverseModel::stateOffline)
		{
		IAnimatron *pButton;
		VI.CreateHiddenButton(pRoot, CMD_ACCOUNT,
				ICON_WIDTH + (PADDING_LEFT / 2),
				y - (PADDING_LEFT / 2),
				cxText + PADDING_LEFT,
				SubTitleFont.GetHeight() + 2 * MediumFont.GetHeight() + PADDING_LEFT,
				0,
				&pButton);
		pButton->AddListener(EVENT_ON_CLICK, this, CMD_ACCOUNT);
		}

	//	Username

	IAnimatron *pName = new CAniText;
	pName->SetPropertyVector(PROP_POSITION, CVector(ICON_WIDTH + PADDING_LEFT, y));
	pName->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), RectHeight(rcRect)));
	pName->SetPropertyColor(PROP_COLOR, wUsernameColor);
	pName->SetPropertyFont(PROP_FONT, &SubTitleFont);
	pName->SetPropertyString(PROP_TEXT, sUsername);

	pRoot->AddTrack(pName, 0);
	y += SubTitleFont.GetHeight();

	//	Status

	IAnimatron *pStatus = new CAniText;
	pStatus->SetPropertyVector(PROP_POSITION, CVector(ICON_WIDTH + PADDING_LEFT, y));
	pStatus->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), RectHeight(rcRect)));
	pStatus->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogLabel));
	pStatus->SetPropertyFont(PROP_FONT, &MediumFont);
	pStatus->SetPropertyString(PROP_TEXT, sStatus);

	pRoot->AddTrack(pStatus, 0);
	y += MediumFont.GetHeight();

	//	If the user is signed in, add buttons to edit account and sign out.

	if (iState == CMultiverseModel::stateOnline)
		{
		int x = ICON_WIDTH + PADDING_LEFT;

		//	Edit account

		IAnimatron *pButton;
		int cxLink;
#ifdef EDIT_ACCOUNT
		VI.CreateLink(pRoot, CMD_ACCOUNT_EDIT, x, y, CONSTLIT("edit account"), CVisualPalette::OPTION_LINK_MEDIUM_FONT, &pButton, &cxLink);
		pButton->AddListener(EVENT_ON_CLICK, this, CMD_ACCOUNT_EDIT);
#else
		VI.CreateLink(pRoot, CMD_CHANGE_PASSWORD, x, y, CONSTLIT("change password"), CVisualPalette::OPTION_LINK_MEDIUM_FONT, &pButton, &cxLink);
		pButton->AddListener(EVENT_ON_CLICK, this, CMD_CHANGE_PASSWORD);
#endif

		x += cxLink;

		//	Separator

		IAnimatron *pSep = new CAniText;
		pSep->SetPropertyVector(PROP_POSITION, CVector(x, y));
		pSep->SetPropertyVector(PROP_SCALE, CVector(SMALL_LINK_SPACING, RectHeight(rcRect)));
		pSep->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogLabel));
		pSep->SetPropertyFont(PROP_FONT, &MediumFont);
		pSep->SetPropertyString(PROP_TEXT, CONSTLIT("•"));
		pSep->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_CENTER);

		pRoot->AddTrack(pSep, 0);
		x += SMALL_LINK_SPACING;

		//	Sign out

		VI.CreateLink(pRoot, CMD_SIGN_OUT, x, y, CONSTLIT("sign out"), CVisualPalette::OPTION_LINK_MEDIUM_FONT, &pButton, &cxLink);
		pButton->AddListener(EVENT_ON_CLICK, this, CMD_SIGN_OUT);

		x += cxLink;
		}

	//	Add it to the existing sequencer

	IAnimatron *pPlayerBar = m_Reanimator.GetPerformance(ID_PLAYER_BAR_PERFORMANCE);
	if (pPlayerBar == NULL)
		return;

	CAniSequencer *pSeq = (CAniSequencer *)pPlayerBar;
	pSeq->AddTrack(pRoot, 0);
	}

void CTranscendenceWnd::SetDebugOption (void)

//	SetDebugOption
//
//	Sets the current state of debug mode

	{
	const CVisualPalette &VI = g_pHI->GetVisuals();

	IAnimatron *pButton = m_Reanimator.GetElement(CMD_TOGGLE_DEBUG);
	if (pButton == NULL)
		return;

	IAnimatron *pStyle = pButton->GetStyle(STYLE_IMAGE);
	if (pStyle == NULL)
		return;

	bool bDebug = m_pTC->GetModel().GetDebugMode();
	if (bDebug)
		{
		pStyle->SetFillMethod(new CAniImageFill(&VI.GetImage(imageDebugIcon), false));
		pButton->SetPropertyString(PROP_TEXT, CONSTLIT("Debug"));
		}
	else
		{
		pStyle->SetFillMethod(new CAniImageFill(&VI.GetImage(imagePlayIcon), false));
		pButton->SetPropertyString(PROP_TEXT, CONSTLIT("Play"));
		}
	}

void CTranscendenceWnd::SetHighScoresNext (void)

//	SetHighScoresNext
//
//	Select next high score

	{
	CHighScoreList *pHighScoreList = GetHighScoreList();

	if (m_iHighScoreSelection == -1)
		{
		int iPos = GetHighScoresPos();
		if (iPos != -1)
			SetHighScoresPos(iPos);
		}
	else
		SetHighScoresPos(Min(m_iHighScoreSelection + 1, pHighScoreList->GetCount() - 1));
	}

void CTranscendenceWnd::SetHighScoresPrev (void)

//	SetHighScoresPrev
//
//	Select previous high score

	{
	if (m_iHighScoreSelection == -1)
		{
		int iPos = GetHighScoresPos();
		if (iPos != -1)
			SetHighScoresPos(iPos);
		}
	else
		SetHighScoresPos(Max(0, m_iHighScoreSelection - 1));
	}

void CTranscendenceWnd::SetHighScoresPos (int iPos)

//	SetHighScoresPos
//
//	Animates to the given position

	{
	CHighScoreList *pHighScoreList = GetHighScoreList();

	if (iPos < 0 || iPos >= pHighScoreList->GetCount())
		return;

	IAnimatron *pAni = m_Reanimator.GetPerformance(m_dwHighScoresPerformance);
	if (pAni == NULL)
		return;

	//	Get the current scroll position

	int yCurrent = (int)pAni->GetPropertyMetric(PROP_SCROLL_POS);
	int yDest = m_pHighScorePos[iPos];

	//	Delete any current animation

	pAni->RemoveAnimation(ID_HIGH_SCORES_ANIMATOR);

	//	Create animation that goes from the current position
	//	to the desired position

	CLinearMetric *pScroller = new CLinearMetric;
	pScroller->SetParams(yCurrent, yDest, (yDest - yCurrent) / 16.0f);
	pAni->AnimateProperty(PROP_SCROLL_POS, pScroller, 0, ID_HIGH_SCORES_ANIMATOR);

	//	Create animation that fades out the whole list after a while

	CLinearFade *pFader = new CLinearFade;
	pFader->SetParams(pScroller->GetDuration() + HIGH_SCORES_DURATION, 0, 30);
	pAni->AnimateProperty(PROP_OPACITY, pFader, 0, ID_HIGH_SCORES_ANIMATOR);

	//	Set the selection to the proper spot

	IAnimatron *pSelRect;
	if (pAni->FindElement(ID_HIGH_SCORES_SELECT_RECT, &pSelRect))
		{
		int cyOffset = (m_rcIntroMain.bottom - m_Fonts.Header.GetHeight() - m_rcIntroMain.top) / 3;

		pSelRect->SetPropertyVector(PROP_POSITION, CVector(SCORE_SELECTION_X, yDest + cyOffset));
		pSelRect->SetPropertyVector(PROP_SCALE, CVector(SCORE_SELECTION_WIDTH, m_pHighScorePos[iPos + 1] - yDest));
		pSelRect->SetPropertyOpacity(PROP_OPACITY, SCORE_SELECTION_OPACITY);
		}

	//	Restart animation

	m_Reanimator.StartPerformance(m_dwHighScoresPerformance);

	//	Remember the position

	m_iHighScoreSelection = iPos;
	}

void CTranscendenceWnd::SetHighScoresScroll (void)

//	SetHighScoresScroll
//
//	Animates high scores to scrolling mode

	{
	IAnimatron *pAni = m_Reanimator.GetPerformance(m_dwHighScoresPerformance);
	if (pAni == NULL)
		return;

	//	Delete any current animations

	pAni->RemoveAnimation(ID_HIGH_SCORES_ANIMATOR);

	//	Scroller

	CLinearMetric *pScroller = new CLinearMetric;
	Metric cyViewport = pAni->GetPropertyMetric(PROP_VIEWPORT_HEIGHT);
	RECT rcRect;
	pAni->GetSpacingRect(&rcRect);
	pScroller->SetParams(-cyViewport, (Metric)RectHeight(rcRect), 2.0);
	pAni->AnimateProperty(PROP_SCROLL_POS, pScroller, 0, ID_HIGH_SCORES_ANIMATOR);

	//	Clear the selection

	IAnimatron *pSelRect;
	if (pAni->FindElement(ID_HIGH_SCORES_SELECT_RECT, &pSelRect))
		pSelRect->SetPropertyOpacity(PROP_OPACITY, 0);

	//	Restart

	m_Reanimator.StartPerformance(m_dwHighScoresPerformance);

	//	No selection

	m_iHighScoreSelection = -1;
	}

void CTranscendenceWnd::SetIntroState (IntroState iState)

//	SetIntroState
//
//	Sets the given state

	{
	//	Note: It is OK to call this with iState already
	//	equal to the current state. We still need to go through this code

	switch (iState)
		{
		case isBlank:
			StopAnimations();
			break;

		case isBlankThenRandom:
			StopAnimations();
			m_iIntroCounter = 3600;
			break;

		case isCredits:
			StopAnimations();
			m_Reanimator.StartPerformance(m_dwCreditsPerformance);
			break;

		case isEndGame:
			{
			CHighScoreList *pHighScoreList = GetHighScoreList();
			StopAnimations();

			int iLastHighScore = m_pTC->GetModel().GetLastHighScore();
			const CGameRecord *pScore = (iLastHighScore != -1 ? &pHighScoreList->GetEntry(iLastHighScore) : NULL);
			if (pScore == NULL)
				return SetIntroState(isHighScoresEndGame);

			IAnimatron *pAni;
			CreateScoreAnimation(*pScore, &pAni);
			DWORD dwScorePerformance = m_Reanimator.AddPerformance(pAni, ID_END_GAME_PERFORMANCE);
			m_Reanimator.StartPerformance(dwScorePerformance, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
			break;
			}

		case isEnterShipClass:
			StopAnimations();
			m_sCommand = NULL_STR;
			break;

		case isHighScores:
			StopAnimations();
			SetHighScoresScroll();
			break;

		case isHighScoresEndGame:
			{
			StopAnimations();
			int iLastHighScore = m_pTC->GetModel().GetLastHighScore();
			if (iLastHighScore != -1)
				SetHighScoresPos(iLastHighScore);
			else
				SetHighScoresScroll();
			break;
			}

		case isNews:
			{
			CMultiverseNewsEntry *pNews = m_pTC->GetMultiverse().GetNextNewsEntry();
			if (pNews == NULL)
				{
				SetIntroState(isShipStats);
				return;
				}

			StopAnimations();

			IAnimatron *pAni;
			CreateNewsAnimation(pNews, &pAni);
			delete pNews;

			DWORD dwPerformance = m_Reanimator.AddPerformance(pAni, ID_NEWS_PERFORMANCE);
			m_Reanimator.StartPerformance(dwPerformance, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
			break;
			}

		case isOpeningTitles:
			StopAnimations();
			m_Reanimator.StartPerformance(m_dwTitlesPerformance);
			break;

		case isShipStats:
			{
			CShip *pShip = g_pUniverse->GetPOV()->AsShip();
			if (pShip == NULL)
				return;

			StopAnimations();

			IAnimatron *pAni;
			CreateShipDescAnimation(pShip, &pAni);
			DWORD dwPerformance = m_Reanimator.AddPerformance(pAni, ID_SHIP_DESC_PERFORMANCE);
			m_Reanimator.StartPerformance(dwPerformance, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
			break;
			}

		default:
			ASSERT(false);
			return;
		}

	m_iIntroState = iState;
	}

void CTranscendenceWnd::SetMusicOption (void)

//	SetMusicOption
//
//	Sets the current state of the music toggle icon

	{
	const CVisualPalette &VI = g_pHI->GetVisuals();

	IAnimatron *pButton = m_Reanimator.GetElement(CMD_TOGGLE_MUSIC);
	if (pButton == NULL)
		return;

	IAnimatron *pStyle = pButton->GetStyle(STYLE_IMAGE);
	if (pStyle == NULL)
		return;

	bool bMusicOn = !m_pTC->GetOptionBoolean(CGameSettings::noMusic);
	if (bMusicOn)
		{
		pStyle->SetFillMethod(new CAniImageFill(&VI.GetImage(imageMusicIconOn), false));
		pButton->SetPropertyString(PROP_TEXT, CONSTLIT("Music On"));
		}
	else
		{
		pStyle->SetFillMethod(new CAniImageFill(&VI.GetImage(imageMusicIconOff), false));
		pButton->SetPropertyString(PROP_TEXT, CONSTLIT("Music Off"));
		}
	}

ALERROR CTranscendenceWnd::StartIntro (IntroState iState)

//	StartIntro
//
//	Start introduction

	{
	ALERROR error;
	int i;

	ClearDebugLines();

	//	Use widescreen topology

	int cyBarHeight = Max(128, (g_cyScreen - INTRO_DISPLAY_HEIGHT) / 2);
	m_rcIntroTop.top = 0;
	m_rcIntroTop.left = 0;
	m_rcIntroTop.bottom = cyBarHeight;
	m_rcIntroTop.right = g_cxScreen;

	m_rcIntroMain.top = cyBarHeight;
	m_rcIntroMain.left = 0;
	m_rcIntroMain.bottom = g_cyScreen - cyBarHeight;
	m_rcIntroMain.right = g_cxScreen;

	m_rcIntroBottom.top = g_cyScreen - cyBarHeight;
	m_rcIntroBottom.left = 0;
	m_rcIntroBottom.bottom = g_cyScreen;
	m_rcIntroBottom.right = g_cxScreen;

	//	Create the buttons

	m_ButtonBar.Init();

	m_bSavedGame = false;
	m_ButtonBar.AddButton(CMD_CONTINUE_OLD_GAME,
			CONSTLIT("Load Game"),
			CONSTLIT("Continue a Previous Game"),
			CONSTLIT("C"),
			0,
			CButtonBarData::alignLeft);

	m_ButtonBar.AddButton(CMD_START_NEW_GAME,
			CONSTLIT("New Game"),
			CONSTLIT("Begin a New Game"),
			CONSTLIT("N"),
			1,
			CButtonBarData::alignCenter);

	m_ButtonBar.AddButton(CMD_QUIT_GAME,
			CONSTLIT("Quit"),
			CONSTLIT("Exit Transcendence"),
			CONSTLIT("Q"),
			2,
			CButtonBarData::alignRight);

	m_ButtonBarDisplay.SetFontTable(&m_Fonts);
	m_ButtonBarDisplay.Init(this, &m_ButtonBar, m_rcIntroBottom);

	//	Create an empty system

	if (error = g_pUniverse->CreateEmptyStarSystem(&m_pIntroSystem))
		{
		ASSERT(false);
		return error;
		}

	g_pUniverse->SetCurrentSystem(m_pIntroSystem);

	CSovereign *pSovereign1 = g_pUniverse->FindSovereign(g_PlayerSovereignUNID);
	CSovereign *pSovereign2 = g_pUniverse->FindSovereign(g_PirateSovereignUNID);

	//	Create a couple of random enemy ships

	CShip *pShip1;
	CShip *pShip2;
	if (error = CreateRandomShip(m_pIntroSystem, 0, pSovereign1, &pShip1))
		{
		ASSERT(false);
		return error;
		}

	if (error = CreateRandomShip(m_pIntroSystem, 0, pSovereign2, &pShip2))
		{
		ASSERT(false);
		return error;
		}

	//	Make the ships attack each other

	for (i = 0; i < m_pIntroSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = m_pIntroSystem->GetObject(i);

		if (pObj
				&& pObj->GetCategory() == CSpaceObject::catShip
				&& !pObj->IsVirtual()
				&& !pObj->IsInactive()
				&& !pObj->GetData(CONSTLIT("IntroController")).IsBlank())
			{
			CShip *pShip = pObj->AsShip();
			if (pShip)
				{
				IShipController *pController = pShip->GetController();
				if (pShip->GetSovereign() == pSovereign1)
					pController->AddOrder(IShipController::orderDestroyTarget, pShip2, IShipController::SData());
				else
					pController->AddOrder(IShipController::orderDestroyTarget, pShip1, IShipController::SData());
				}
			}
		}

	//	Other initialization

	m_bOverwriteGameDlg = false;
	m_rcOverwriteGameDlg.left = m_rcIntroMain.left + (RectWidth(m_rcIntroMain) - DIALOG_WIDTH) / 2;
	m_rcOverwriteGameDlg.top = m_rcIntroMain.top + (RectHeight(m_rcIntroMain) - DIALOG_HEIGHT) / 2;
	m_rcOverwriteGameDlg.right = m_rcOverwriteGameDlg.left + DIALOG_WIDTH;
	m_rcOverwriteGameDlg.bottom = m_rcOverwriteGameDlg.top + DIALOG_HEIGHT;

	int cxMid = m_rcOverwriteGameDlg.left + (DIALOG_WIDTH / 2);
	m_rcOverwriteGameOK.left = cxMid - DIALOG_SPACING_X - DIALOG_BUTTON_WIDTH;
	m_rcOverwriteGameOK.top = m_rcOverwriteGameDlg.bottom - DIALOG_SPACING_Y - DIALOG_BUTTON_HEIGHT;
	m_rcOverwriteGameOK.right = m_rcOverwriteGameOK.left + DIALOG_BUTTON_WIDTH;
	m_rcOverwriteGameOK.bottom = m_rcOverwriteGameOK.top + DIALOG_BUTTON_HEIGHT;

	m_rcOverwriteGameCancel = m_rcOverwriteGameOK;
	m_rcOverwriteGameCancel.left = cxMid + DIALOG_SPACING_X;
	m_rcOverwriteGameCancel.right = m_rcOverwriteGameCancel.left + DIALOG_BUTTON_WIDTH;

	//	No sound

	g_pUniverse->SetSound(false);

	//	Set the POV to one of them

	g_pUniverse->SetPOV(pShip1);
	m_iTick = 0;
	m_iLastShipCreated = m_iTick;

	//	Create the credits performance

	IAnimatron *pAnimation;
	CreateCreditsAnimation(&pAnimation);
	m_dwCreditsPerformance = m_Reanimator.AddPerformance(pAnimation, ID_CREDITS_PERFORMANCE);

	//	Create the titles performance

	CreateTitleAnimation(&pAnimation);
	m_dwTitlesPerformance = m_Reanimator.AddPerformance(pAnimation, ID_TITLES_PERFORMANCE);

	//	Create the high scores performance

	CreateHighScoresAnimation(&pAnimation);
	m_dwHighScoresPerformance = m_Reanimator.AddPerformance(pAnimation, ID_HIGH_SCORES_PERFORMANCE);

	//	Create the top bar

	CreatePlayerBarAnimation(&pAnimation);
	m_dwPlayerBarPerformance = m_Reanimator.AddPerformance(pAnimation, ID_PLAYER_BAR_PERFORMANCE);
	m_Reanimator.StartPerformance(m_dwPlayerBarPerformance);

	SetMusicOption();
	SetDebugOption();
	SetAccountControls(m_pTC->GetMultiverse());

	//	Start

	SetIntroState(iState);
	m_State = gsIntro;

	//	Show the cursor

	ShowCursor(true);

	return NOERROR;
	}

void CTranscendenceWnd::StopAnimations (void)

//	StopAnimations
//
//	Stops all intro animations (but not UI element animations)

	{
	m_Reanimator.StopPerformance(ID_CREDITS_PERFORMANCE);
	m_Reanimator.StopPerformance(ID_END_GAME_PERFORMANCE);
	m_Reanimator.StopPerformance(ID_HIGH_SCORES_PERFORMANCE);
	m_Reanimator.StopPerformance(ID_SHIP_DESC_PERFORMANCE);
	m_Reanimator.StopPerformance(ID_TITLES_PERFORMANCE);
	m_Reanimator.StopPerformance(ID_NEWS_PERFORMANCE);
	}

void CTranscendenceWnd::StopIntro (void)

//	StopIntro
//
//	Stop introduction screen

	{
	ASSERT(m_State == gsIntro);

	m_ButtonBarDisplay.CleanUp();
	m_ButtonBar.CleanUp();

	//	Clean up reanimator (so that we don't keep memory allocated)

	if (m_dwCreditsPerformance)
		m_Reanimator.DeletePerformance(m_dwCreditsPerformance);

	if (m_dwTitlesPerformance)
		m_Reanimator.DeletePerformance(m_dwTitlesPerformance);

	if (m_dwHighScoresPerformance)
		m_Reanimator.DeletePerformance(m_dwHighScoresPerformance);

	if (m_dwPlayerBarPerformance)
		m_Reanimator.DeletePerformance(m_dwPlayerBarPerformance);

	if (m_pHighScorePos)
		{
		delete [] m_pHighScorePos;
		m_pHighScorePos = NULL;
		}

	//	Destroy system

	g_pUniverse->DestroySystem(m_pIntroSystem);
	m_pIntroSystem = NULL;

	//	Enable sound

	g_pUniverse->SetSound(true);

	//	Hide cursor

	ShowCursor(false);
	}

