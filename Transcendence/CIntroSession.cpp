//	CIntroSession.cpp
//
//	CIntroSession class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define CMD_ACCOUNT								CONSTLIT("cmdAccount")
#define CMD_ACCOUNT_EDIT						CONSTLIT("cmdAccountEdit")
#define CMD_CHANGE_PASSWORD						CONSTLIT("cmdChangePassword")
#define CMD_OPEN_NEWS							CONSTLIT("cmdOpenNews")
#define CMD_SHOW_MOD_EXCHANGE					CONSTLIT("cmdShowModExchange")
#define CMD_SHOW_PROFILE						CONSTLIT("cmdShowProfile")
#define CMD_SIGN_OUT							CONSTLIT("cmdSignOut")
#define CMD_SOUNDTRACK_NOW_PLAYING				CONSTLIT("cmdSoundtrackNowPlaying")
#define CMD_TOGGLE_DEBUG						CONSTLIT("cmdToggleDebug")
#define CMD_TOGGLE_MUSIC						CONSTLIT("cmdToggleMusic")

#define CMD_SERVICE_NEWS_LOADED					CONSTLIT("serviceNewsLoaded")

#define CMD_UI_CHANGE_PASSWORD					CONSTLIT("uiChangePassword")
#define CMD_UI_SHOW_LOGIN						CONSTLIT("uiShowLogin")
#define CMD_UI_SHOW_MOD_EXCHANGE				CONSTLIT("uiShowModExchange")
#define CMD_UI_SHOW_PROFILE						CONSTLIT("uiShowProfile")
#define CMD_UI_SIGN_OUT							CONSTLIT("uiSignOut")

#define ID_SOUNDTRACK_TITLE_PERFORMANCE			CONSTLIT("idSoundtrackTitle")

const int INTRO_DISPLAY_HEIGHT =				512;

void CIntroSession::CreateSoundtrackTitleAnimation (CSoundType *pTrack, IAnimatron **retpAni)

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
	VI.GetWidescreenRect(m_HI.GetScreen(), &rcCenter);

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

void CIntroSession::OnAnimate (CG32bitImage &Screen, bool bTopMost)

//	OnAnimate
//
//	Animate the session

	{
	bool bFailed = false;

	//	Update context

	SSystemUpdateCtx UpdateCtx;

	try
		{
		//	Paint

		SetProgramState(psAnimating);
		g_pTrans->AnimateIntro(bTopMost);

		//	Flip

		if (bTopMost)
			m_HI.GetScreenMgr().Flip();

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

	else if (strEquals(sCmd, CMD_CHANGE_PASSWORD))
		m_HI.HICommand(CMD_UI_CHANGE_PASSWORD);

	else if (strEquals(sCmd, CMD_SERVICE_NEWS_LOADED))
		g_pTrans->SetIntroState(CTranscendenceWnd::isNews);

	else if (strEquals(sCmd, CMD_SOUNDTRACK_NOW_PLAYING))
		StartSoundtrackTitleAnimation((CSoundType *)pData);

	else
		g_pTrans->OnCommandIntro(sCmd, pData);

	return NOERROR;
	}

ALERROR CIntroSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize the session
	
	{
	SetNoCursor(true);

	//	Metrics

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

	//	Initialize

	g_pTrans->StartIntro(this, m_iInitialState);

	return NOERROR; 
	}

void CIntroSession::StartSoundtrackTitleAnimation (CSoundType *pTrack)

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
