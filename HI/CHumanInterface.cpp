//	CHumanInterface.cpp
//
//	CHumanInterface class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

static CReanimator g_DefaultReanimator;

CHumanInterface::CHumanInterface (void) : 
		m_bDebugVideo(false),
		m_pController(NULL),
		m_pCurSession(NULL),
		m_hWnd(NULL),
		m_chKeyDown('\0')

//	CHumanInterface constructor

	{
	}

CHumanInterface::~CHumanInterface (void)

//	CHumanInterface destructor

	{
	CleanUp();
	}

void CHumanInterface::AddBackgroundTask (IHITask *pTask, IHICommand *pListener, const CString &sCmd)

//	AddBackgroundTask
//
//	Adds the task

	{
	m_Background.AddTask(pTask, pListener, sCmd);
	}

void CHumanInterface::BeginSessionPaint (CG16bitImage &Screen)

//	BeginSessionPaint
//
//	Called in DefaultOnAnimate

	{
	}

void CHumanInterface::BeginSessionUpdate (void)

//	BeginSessionUpdate
//
//	Called in DefaultOnAnimate

	{
	}

void CHumanInterface::CleanUp (void)

//	CleanUp
//
//	Clean up interface

	{
	int i;

	//	Wait for all background tasks to end

	m_Background.StopAll();

	//	Clean up session

	if (m_pCurSession)
		{
		m_pCurSession->HICleanUp();
		delete m_pCurSession;
		m_pCurSession = NULL;
		}

	//	Clean up any saved session

	for (i = 0; i < m_SavedSessions.GetCount(); i++)
		{
		m_SavedSessions[i]->HICleanUp();
		delete m_SavedSessions[i];
		}
	m_SavedSessions.DeleteAll();

	//	Clean up controller

	if (m_pController)
		{
		m_pController->HICleanUp();
		delete m_pController;
		m_pController = NULL;
		}

	m_Visuals.CleanUp();
	m_Background.CleanUp();
	m_ScreenMgr.CleanUp();
	m_SoundMgr.CleanUp();
	}

void CHumanInterface::ClosePopupSession (void)

//	ClosePopupSession
//
//	Closes the current session and restore the most
//	recent saved session.

	{
	//	Close the current session

	if (m_pCurSession)
		{
		m_pCurSession->HICleanUp();
		delete m_pCurSession;
		m_pCurSession = NULL;
		}

	//	Restore the previous session

	int iIndex = m_SavedSessions.GetCount() - 1;
	if (iIndex >= 0)
		{
		m_pCurSession = m_SavedSessions[iIndex];

		//	Send size message

		m_pCurSession->HISize(GetScreen().GetWidth(), GetScreen().GetHeight());

		//	Remove from saved array

		m_SavedSessions.Delete(iIndex);
		}
	}

void CHumanInterface::EndSessionPaint (CG16bitImage &Screen)

//	EndSessionPatin
//
//	Called in DefaultOnAnimate

	{
	//	Paint frame rate

	if (m_bDebugVideo)
		{
		m_FrameRate.MarkFrame();
		PaintFrameRate();
		}

	//	Blt

	BltScreen();
	}

void CHumanInterface::EndSessionUpdate (void)

//	EndSessionUpdate
//
//	Called in DefaultOnAnimate

	{
	//	If necessary, flip

	FlipScreen();
	}

CReanimator &CHumanInterface::GetReanimator (void)

//	GetReanimator
//
//	Returns the current session's reanimator

	{
	if (m_pCurSession)
		return m_pCurSession->HIGetReanimator();
	else
		return g_DefaultReanimator;
	}

void CHumanInterface::HardCrash (const CString &sProgramState)

//	HardCrash
//
//	Report an error

	{
	CString sSessionMessage;
	try
		{
		if (m_pCurSession)
			m_pCurSession->HIReportHardCrash(&sSessionMessage);
		}
	catch (...)
		{
		sSessionMessage = CONSTLIT("Unable to obtain crash report from session.");
		}

	CString sMessage = strPatternSubst(CONSTLIT(
			"Unable to continue due to program error.\r\n\r\n"
			"program state: %s\r\n"
			"%s"
			"\r\n\r\nPlease contact transcendence@neurohack.com with a copy of Debug.log and your save file. "
			"We are sorry for the inconvenience.\r\n"),
			sProgramState,
			sSessionMessage
			);

	kernelDebugLogMessage(sMessage.GetASCIIZPointer());
	ShowHardCrashSession(CONSTLIT("Transcendence System Crash"), sMessage);
	}

void CHumanInterface::OnAnimate (void)

//	OnAnimate
//
//	Paint an animation frame

	{
	if (m_pCurSession)
		{
		bool bCrash = false;

		try
			{
			m_pCurSession->HIAnimate(GetScreen());
			}
		catch (...)
			{
			bCrash = true;
			}

		//	If we crashed, report it

		if (bCrash)
			HardCrash(CONSTLIT("animating"));
		}
	else
		{
		GetScreen().Fill(0, 0, GetScreen().GetWidth(), GetScreen().GetHeight(), CG16bitImage::RGBValue(0, 0, 0));
		BltScreen();
		FlipScreen();
		}
	}

ALERROR CHumanInterface::OpenPopupSession (IHISession *pSession)

//	OpenPopupSession
//
//	Pushes the current session on the stack and makes the given
//	session public

	{
	ALERROR error;

	ASSERT(pSession);

	//	Save the current session

	if (m_pCurSession)
		m_SavedSessions.Insert(m_pCurSession);

	//	Show the new session

	m_pCurSession = pSession;
	if (error = m_pCurSession->HIInit())
		{
		m_pCurSession = NULL;
		return error;
		}

	//	Send size message

	m_pCurSession->HISize(GetScreen().GetWidth(), GetScreen().GetHeight());

	return NOERROR;
	}

void CHumanInterface::PaintFrameRate (void)

//	PaintFrameRate
//
//	Paints the current frame rate

	{
	int iRate = (int)(m_FrameRate.GetFrameRate() + 0.5f);
	CString sText = strPatternSubst(CONSTLIT("Frame rate: %d"), iRate);

	const CG16bitFont &TitleFont = m_Visuals.GetFont(fontSubTitle);
	RECT rcRect;
	rcRect.left = 0;
	rcRect.right = GetScreen().GetWidth();
	rcRect.top = 0;
	rcRect.bottom = TitleFont.GetHeight();
	TitleFont.DrawText(GetScreen(), rcRect, CG16bitImage::RGBValue(128,128,128), sText);

	//	LATER: This should move to EndSessionUpdate
	if (m_pCurSession)
		m_pCurSession->HIInvalidate(rcRect);
	}

ALERROR CHumanInterface::ShowSession (IHISession *pSession)

//	ShowSession
//
//	Takes ownership of the session object and gives it control
//	of the screen.

	{
	ALERROR error;

	if (m_pCurSession)
		{
		m_pCurSession->HICleanUp();
		delete m_pCurSession;
		m_pCurSession = NULL;
		}

	if (pSession)
		{
		m_pCurSession = pSession;
		if (error = m_pCurSession->HIInit())
			{
			m_pCurSession = NULL;
			return error;
			}

		//	Send size message

		m_pCurSession->HISize(GetScreen().GetWidth(), GetScreen().GetHeight());
		}

	return NOERROR;
	}

LONG CHumanInterface::WMActivateApp (bool bActivate)

//	WMActivateApp
//
//	Handle WM_ACTIVATEAPP message

	{
	m_ScreenMgr.OnWMActivateApp(bActivate);

	return 0;
	}

LONG CHumanInterface::WMChar (char chChar, DWORD dwKeyData)

//	WMChar
//
//	Handle WM_CHAR message

	{
	//	We may need to ignore a character

	if (m_chKeyDown && (m_chKeyDown == chChar || m_chKeyDown == (chChar - ('a' - 'A'))))
		{
		m_chKeyDown = '\0';
		return 0;
		}
	else
		m_chKeyDown = '\0';

	//	Pass to session

	if (m_pCurSession)
		m_pCurSession->HIChar(chChar, dwKeyData);

	return 0;
	}

LONG CHumanInterface::WMClose (void)

//	WMClose
//
//	Close the app

	{
	::DestroyWindow(m_hWnd);
	return 0;
	}

ALERROR CHumanInterface::WMCreate (HMODULE hModule, HWND hWnd, char *pszCommandLine, IHICommand *pController)

//	WMCreate
//
//	Initializes the class.
//
//	pController will be owned by this object and will be freed when on WMDestroy

	{
	ALERROR error;
	CString sError;

	//	Boot the controller

	SHIOptions Options;
	if (error = pController->HIBoot(pszCommandLine, Options))
		return error;

	//	Options

	m_bDebugVideo = Options.m_bDebugVideo;

	//	Init

	ASSERT(hWnd);
	m_hWnd = hWnd;

	ASSERT(pController);
	m_pController = pController;

	//	Initialize sound manager

	if (error = m_SoundMgr.Init(m_hWnd))
		{
		//	Report error, but don't abort

		kernelDebugLogMessage("Unable to initialize sound manager.");
		}

	m_SoundMgr.SetWaveVolume(Options.m_iSoundVolume);

	//	Initialize screen

	SScreenMgrOptions ScreenOptions;
	ScreenOptions.m_hWnd = m_hWnd;
	ScreenOptions.m_cxScreen = Options.m_cxScreenDesired;
	ScreenOptions.m_cyScreen = Options.m_cyScreenDesired;
	ScreenOptions.m_iColorDepth = Options.m_iColorDepthDesired;
	ScreenOptions.m_bWindowedMode = Options.m_bWindowedMode;
	ScreenOptions.m_bForceDX = Options.m_bForceDX;
	ScreenOptions.m_bForceNonDX = Options.m_bForceNonDX;
	ScreenOptions.m_bForceExclusiveMode = Options.m_bForceExclusiveMode;
	ScreenOptions.m_bForceNonExclusiveMode = Options.m_bForceNonExclusiveMode;
	ScreenOptions.m_bForceScreenSize = Options.m_bForceScreenSize;

	if (error = m_ScreenMgr.Init(ScreenOptions, &sError))
		goto Fail;

	//	Initialize the background processor

	if (error = m_Background.Init(m_hWnd))
		{
		sError = CONSTLIT("Unable to initialize background processor.");
		goto Fail;
		}

	//	Initialize our visual styles

	if (error = m_Visuals.Init(hModule, &sError))
		goto Fail;

	//	Initialize controller (Note: At this point, we need to be
	//	fully initialized because the controller is free to call us back
	//	to set up sessions, tasks, etc.)

	if (error = m_pController->HIInit())
		{
		sError = CONSTLIT("Controller failed");
		goto Fail;
		}

	return NOERROR;

Fail:

	::kernelDebugLogMessage(sError.GetASCIIZPointer());
	::MessageBox(NULL, sError.GetASCIIZPointer(), "Transcendence", MB_OK);

	return ERR_FAIL;
	}

void CHumanInterface::WMDestroy (void)

//	WMDestroy
//
//	Shut down

	{
	CleanUp();
	}

LONG CHumanInterface::WMDisplayChange (int iBitDepth, int cxWidth, int cyHeight)

//	WMDisplayChange
//
//	Handle WM_DISPLAYCHANGE message

	{
	m_ScreenMgr.OnWMDisplayChange(iBitDepth, cxWidth, cyHeight);

	return 0;
	}

LONG CHumanInterface::WMKeyDown (int iVirtKey, DWORD dwKeyData)

//	WMKeyDown
//
//	Handle WM_KEYDOWN message

	{
	if (m_pCurSession)
		{
		//	Remember our current session

		IHISession *pOldSession = m_pCurSession;

		//	Pass it to the session

		m_pCurSession->HIKeyDown(iVirtKey, dwKeyData);

		//	If the session changed, then we need to ignore a
		//	subsequent WM_CHAR

		if (m_pCurSession != pOldSession)
			m_chKeyDown = iVirtKey;
		}

	return 0;
	}

LONG CHumanInterface::WMKeyUp (int iVirtKey, DWORD dwKeyData)

//	WMKeyUp
//
//	Handle WM_KEYUP message

	{
	if (m_pCurSession)
		m_pCurSession->HIKeyUp(iVirtKey, dwKeyData);

	return 0;
	}

LONG CHumanInterface::WMLButtonDblClick (int x, int y, DWORD dwFlags)

//	WMLButtonDblClick
//
//	Handle WM_LBUTTONDBLCLICK message

	{
	if (m_pCurSession)
		{
		int xLocal, yLocal;

		m_ScreenMgr.GlobalToLocal(x, y, &xLocal, &yLocal);
		m_pCurSession->HILButtonDblClick(xLocal, yLocal, dwFlags);
		}

	return 0;
	}
LONG CHumanInterface::WMLButtonDown (int x, int y, DWORD dwFlags)

//	WMLButtonDown
//
//	Handle WM_LBUTTONDOWN message

	{
	if (m_pCurSession)
		{
		int xLocal, yLocal;

		m_ScreenMgr.GlobalToLocal(x, y, &xLocal, &yLocal);
		m_pCurSession->HILButtonDown(xLocal, yLocal, dwFlags);
		}

	return 0;
	}

LONG CHumanInterface::WMLButtonUp (int x, int y, DWORD dwFlags)

//	WMLButtonUp
//
//	Handle WM_LBUTTONUP message

	{
	if (m_pCurSession)
		{
		int xLocal, yLocal;

		m_ScreenMgr.GlobalToLocal(x, y, &xLocal, &yLocal);
		m_pCurSession->HILButtonUp(xLocal, yLocal, dwFlags);
		}

	return 0;
	}

LONG CHumanInterface::WMMouseMove (int x, int y, DWORD dwFlags)

//	WMMouseMove
//
//	Handle WM_MOUSEMOVE message

	{
	if (m_pCurSession)
		{
		int xLocal, yLocal;

		m_ScreenMgr.GlobalToLocal(x, y, &xLocal, &yLocal);
		m_pCurSession->HIMouseMove(xLocal, yLocal, dwFlags);
		}

	return 0;
	}
