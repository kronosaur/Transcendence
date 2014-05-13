//	CHumanInterface.cpp
//
//	CHumanInterface class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

#define CMD_SOUNDTRACK_DONE						CONSTLIT("cmdSoundtrackDone")

const DWORD ID_BACKGROUND_PROCESSOR =				1;
const DWORD ID_LOW_PRIORITY_BACKGROUND_PROCESSOR =	2;

static CReanimator g_DefaultReanimator;
CHumanInterface *g_pHI = NULL;

CHumanInterface::CHumanInterface (void) : 
		m_pController(NULL),
		m_pCurSession(NULL),
		m_hWnd(NULL),
		m_chKeyDown('\0'),
		m_iShutdownCode(HIShutdownUnknown)

//	CHumanInterface constructor

	{
	}

CHumanInterface::~CHumanInterface (void)

//	CHumanInterface destructor

	{
	CleanUp(HIShutdownDestructor);
	}

void CHumanInterface::AddBackgroundTask (IHITask *pTask, DWORD dwFlags, IHICommand *pListener, const CString &sCmd)

//	AddBackgroundTask
//
//	Adds the task

	{
	if (dwFlags & FLAG_LOW_PRIORITY)
		m_BackgroundLowPriority.AddTask(pTask, pListener, sCmd);
	else
		m_Background.AddTask(pTask, pListener, sCmd);
	}

DWORD CHumanInterface::AddTimer (DWORD dwMilliseconds, IHICommand *pListener, const CString &sCmd, bool bRecurring)

//	AddTimer
//
//	Adds a timer

	{
	return m_Timers.AddTimer(m_hWnd,
			dwMilliseconds,
			(pListener ? pListener : m_pController),
			(!sCmd.IsBlank() ? sCmd : CONSTLIT("cmdTimer")),
			bRecurring);
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

void CHumanInterface::CalcBackgroundSessions (void)

//	CalcBackgroundSessions
//
//	Initializes the m_BackgroundSessions array.

	{
	int i;

	m_BackgroundSessions.DeleteAll();

	//	If the current session is fully opaque, then we don't need to paint
	//	any previous sessions.

	if (!m_pCurSession->IsTransparent())
		return;

	//	Make a list of all the sessions that we need to paint before we paint
	//	the current session (in paint order).

	for (i = m_SavedSessions.GetCount() - 1; i >= 0; i--)
		{
		//	Add the session to the list to paint; we add it at the beginning
		//	because previous sessions should paint on top of us.

		m_BackgroundSessions.Insert(m_SavedSessions[i], 0);

		//	If this session is fully opaque, then we're done. 

		if (!m_SavedSessions[i]->IsTransparent())
			return;
		}
	}

void CHumanInterface::CleanUp (EHIShutdownReasons iShutdownCode)

//	CleanUp
//
//	Clean up interface

	{
	int i;

	//	Wait for all background tasks to end

	m_Background.StopAll();
	m_BackgroundLowPriority.StopAll();

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
		m_pController->HIShutdown(iShutdownCode);
		m_pController->HICleanUp();
		delete m_pController;
		m_pController = NULL;
		}

	m_Visuals.CleanUp();
	m_Background.CleanUp();
	m_BackgroundLowPriority.CleanUp();
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
		if (m_pCurSession->IsCursorShown())
			::ShowCursor(false);

		//	Remove the session from any background tasks and timers

		m_Background.ListenerDestroyed(m_pCurSession);
		m_BackgroundLowPriority.ListenerDestroyed(m_pCurSession);
		m_Timers.ListenerDestroyed(m_hWnd, m_pCurSession);

		//	Clean up

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

	//	Recompute paint order

	CalcBackgroundSessions();
	}

bool CHumanInterface::Create (void)

//	Create
//
//	Creates the global structure.

	{
	ASSERT(g_pHI == NULL);

	g_pHI = new CHumanInterface;
	return true;
	}

void CHumanInterface::Destroy (void)

//	Destroy
//
//	Destroys

	{
	if (g_pHI)
		{
		delete g_pHI;
		g_pHI = NULL;
		}
	}

void CHumanInterface::EndSessionPaint (CG16bitImage &Screen, bool bTopMost)

//	EndSessionPatin
//
//	Called in DefaultOnAnimate

	{
	//	If this is the top-most session, then do some housekeeping

	if (bTopMost)
		{
		//	Paint frame rate

		if (m_Options.m_bDebugVideo)
			{
			m_FrameRate.MarkFrame();
			PaintFrameRate();
			}

		//	Blt

		BltScreen();
		}
	}

void CHumanInterface::EndSessionUpdate (bool bTopMost)

//	EndSessionUpdate
//
//	Called in DefaultOnAnimate

	{
	//	If necessary, flip

	if (bTopMost)
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

IHISession *CHumanInterface::GetTopSession (bool bNonTransparentOnly)

//	GetTopSession
//
//	Returns the top-most session.

	{
	int i;

	//	If we allow transparent sessions then just return the top-most session

	if (!bNonTransparentOnly || (m_pCurSession == NULL) || !m_pCurSession->IsTransparent())
		return m_pCurSession;

	//	Otherwise, look for the session

	else
		{
		for (i = m_SavedSessions.GetCount() - 1; i >= 0; i--)
			{
			//	If this session is fully opaque, then we're done. 

			if (!m_SavedSessions[i]->IsTransparent())
				return m_SavedSessions[i];
			}

		//	Otherwise, not found

		return NULL;
		}
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
			"\r\n\r\nPlease contact transcendence@kronosaur.com with a copy of Debug.log and your save file. "
			"We are sorry for the inconvenience.\r\n"),
			sProgramState,
			sSessionMessage
			);

	kernelDebugLogMessage(sMessage);
	ShowHardCrashSession(CONSTLIT("Transcendence System Crash"), sMessage);

	//	Ask the controller to post a crash report

	CString *pCrashReport = new CString(::kernelGetSessionDebugLog());
	if (HICommand(CONSTLIT("cmdPostCrashReport"), pCrashReport) == ERR_NOTFOUND)
		delete pCrashReport;
	}

void CHumanInterface::HIPostCommand (const CString &sCmd, void *pData)

//	HIPostCommand
//
//	This may be called by a different thread to post a command to the main thread.

	{
	SPostCommand *pMsg = new SPostCommand;
	pMsg->sCmd = sCmd;
	pMsg->pData = pData;

	if (!::PostMessage(m_hWnd, WM_HI_COMMAND, 0, (LPARAM)pMsg))
		::kernelDebugLogMessage("Unable to send post HICommand (PostMessage failed): %s.", sCmd);
	}

LONG CHumanInterface::MCINotifyMode (int iMode)

//	MCIWNDM_NOTIFYMODE
//
//	Notification that MCI playback mode has changed.

	{
	switch (iMode)
		{
		case MCI_MODE_NOT_READY:
			break;

		case MCI_MODE_OPEN:
			break;

		case MCI_MODE_PAUSE:
			break;

		case MCI_MODE_PLAY:
			break;

		case MCI_MODE_RECORD:
			break;

		case MCI_MODE_SEEK:
			break;

		case MCI_MODE_STOP:
			HICommand(CMD_SOUNDTRACK_DONE);
			break;
		}

	return 0;
	}

void CHumanInterface::OnAnimate (void)

//	OnAnimate
//
//	Paint an animation frame

	{
	int i;

	//	If minimized, bail out

	if (m_ScreenMgr.IsMinimized())
		return;

	//	DirectDraw

	if (!m_ScreenMgr.CheckIsReady())
		return;

	//	Paint the current session

	if (m_pCurSession)
		{
		bool bCrash = false;

		try
			{
			//	Paint the background sessions

			for (i = 0; i < m_BackgroundSessions.GetCount(); i++)
				m_BackgroundSessions[i]->HIAnimate(GetScreen(), false);

			//	Paint the current session

			m_pCurSession->HIAnimate(GetScreen(), true);
			}
		catch (...)
			{
			bCrash = true;
			}

		//	If we crashed, report it

		if (bCrash)
			HardCrash(CONSTLIT("OnAnimate"));
		}
	else
		{
		GetScreen().Fill(0, 0, GetScreen().GetWidth(), GetScreen().GetHeight(), CG16bitImage::RGBValue(0, 0, 0));
		BltScreen();
		FlipScreen();
		}
	}

void CHumanInterface::OnPostCommand (LPARAM pData)

//	OnPostCommand
//
//	Handle a posted command

	{
	SPostCommand *pMsg = (SPostCommand *)pData;
	HICommand(pMsg->sCmd, pMsg->pData);
	delete pMsg;
	}

void CHumanInterface::OnTaskComplete (DWORD dwID, LPARAM pData)

//	OnTaskComplete
//
//	Process a message that a given task is complete.
	
	{
	switch (dwID)
		{
		case ID_BACKGROUND_PROCESSOR:
			m_Background.OnTaskComplete(pData); 
			break;

		case ID_LOW_PRIORITY_BACKGROUND_PROCESSOR:
			m_BackgroundLowPriority.OnTaskComplete(pData);
			break;

		default:
			ASSERT(false);
		}
	}

ALERROR CHumanInterface::OpenPopupSession (IHISession *pSession, CString *retsError)

//	OpenPopupSession
//
//	Pushes the current session on the stack and makes the given
//	session public

	{
	ALERROR error;
	CString sError;

	ASSERT(pSession);

	//	Save the current session

	IHISession *pOldSession = m_pCurSession;

	//	Show the new session

	m_pCurSession = pSession;
	if (error = m_pCurSession->HIInit(&sError))
		{
		if (retsError)
			*retsError = sError;

		//	Restore the previous session

		m_pCurSession = pOldSession;
		return error;
		}

	//	Now that we know that the new session will be shown, add the old 
	//	session to the stack.

	if (pOldSession)
		m_SavedSessions.Insert(pOldSession);

	//	Send size message

	m_pCurSession->HISize(GetScreen().GetWidth(), GetScreen().GetHeight());

	//	Recompute paint order

	CalcBackgroundSessions();

	//	Show the cursor

	if (m_pCurSession->IsCursorShown())
		::ShowCursor(true);

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

ALERROR CHumanInterface::ShowSession (IHISession *pSession, CString *retsError)

//	ShowSession
//
//	Takes ownership of the session object and gives it control
//	of the screen.

	{
	ALERROR error;
	CString sError;

	//	Initialize the new session

	if (pSession)
		{
		if (error = pSession->HIInit(&sError))
			{
			if (retsError)
				*retsError = sError;
			return error;
			}
		}

	//	We always replace the bottom most session

	if (m_SavedSessions.GetCount() > 0)
		{
		//	Clean up previous session

		m_SavedSessions[0]->HICleanUp();
		delete m_SavedSessions[0];

		//	Replace

		m_SavedSessions[0] = pSession;

		//	Recompute paint order

		CalcBackgroundSessions();
		}
	else
		{
		//	Clean up previous session

		if (m_pCurSession)
			{
			if (m_pCurSession->IsCursorShown())
				::ShowCursor(false);

			m_pCurSession->HICleanUp();
			delete m_pCurSession;
			}

		m_pCurSession = pSession;
		if (m_pCurSession)
			{
			//	Send size message

			m_pCurSession->HISize(GetScreen().GetWidth(), GetScreen().GetHeight());

			//	Show the cursor

			if (m_pCurSession->IsCursorShown())
				::ShowCursor(true);
			}
		}

	return NOERROR;
	}

int CHumanInterface::SetSoundVolume (int iVolume)

//	SetSoundVolume
//
//	Sets the volume

	{
	if (iVolume < 0 || iVolume > 10 || iVolume == m_Options.m_iSoundVolume)
		return m_Options.m_iSoundVolume;

	//	Set it

	m_Options.m_iSoundVolume = iVolume;
	m_SoundMgr.SetWaveVolume(iVolume);
	m_SoundMgr.SetMusicVolume(iVolume);

	//	Done

	return m_Options.m_iSoundVolume;
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

ALERROR CHumanInterface::WMCreate (HMODULE hModule, HWND hWnd, char *pszCommandLine, IHIController *pController)

//	WMCreate
//
//	Initializes the class.
//
//	pController will be owned by this object and will be freed when on WMDestroy

	{
	ALERROR error;
	CString sError;

	//	Init

	ASSERT(hWnd);
	m_hWnd = hWnd;

	ASSERT(pController);
	m_pController = pController;

	//	Always start with a hidden cursor. Sessions will show it if necessary.
	//	(Remember that the Windows cursor is refcounted. Each call to 
	//	ShowCursor(true) must be paired with a ShowCursor(false).

	::ShowCursor(false);

	//	Boot the controller

	if (error = pController->HIBoot(pszCommandLine, m_Options))
		return error;

	//	Initialize sound manager

	if (error = m_SoundMgr.Init(m_hWnd))
		{
		//	Report error, but don't abort

		kernelDebugLogMessage("Unable to initialize sound manager.");
		}

	m_SoundMgr.SetWaveVolume(m_Options.m_iSoundVolume);

	//	Initialize screen

	SScreenMgrOptions ScreenOptions;
	ScreenOptions.m_hWnd = m_hWnd;
	ScreenOptions.m_cxScreen = m_Options.m_cxScreenDesired;
	ScreenOptions.m_cyScreen = m_Options.m_cyScreenDesired;
	ScreenOptions.m_iColorDepth = m_Options.m_iColorDepthDesired;
	ScreenOptions.m_bWindowedMode = m_Options.m_bWindowedMode;
	ScreenOptions.m_bMultiMonitorMode = m_Options.m_bMultiMonitorMode;
	ScreenOptions.m_bForceDX = m_Options.m_bForceDX;
	ScreenOptions.m_bForceNonDX = m_Options.m_bForceNonDX;
	ScreenOptions.m_bForceExclusiveMode = m_Options.m_bForceExclusiveMode;
	ScreenOptions.m_bForceNonExclusiveMode = m_Options.m_bForceNonExclusiveMode;
	ScreenOptions.m_bForceScreenSize = m_Options.m_bForceScreenSize;

	if (error = m_ScreenMgr.Init(ScreenOptions, &sError))
		goto Fail;

	//	Initialize the background processors

	if (error = m_Background.Init(m_hWnd, ID_BACKGROUND_PROCESSOR))
		{
		sError = CONSTLIT("Unable to initialize background processor.");
		goto Fail;
		}

	if (error = m_BackgroundLowPriority.Init(m_hWnd, ID_LOW_PRIORITY_BACKGROUND_PROCESSOR))
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

	if (error = m_pController->HIInit(&sError))
		goto Fail;

	return NOERROR;

Fail:

	::kernelDebugLogMessage(sError);
	::MessageBox(NULL, sError.GetASCIIZPointer(), "Transcendence", MB_OK);

	return ERR_FAIL;
	}

void CHumanInterface::WMDestroy (void)

//	WMDestroy
//
//	Shut down

	{
	//	If we don't know why we're shutting down, then it was probably
	//	because of the user

	if (m_iShutdownCode == HIShutdownUnknown)
		m_iShutdownCode = HIShutdownByUser;

	//	Clean up

	CleanUp(m_iShutdownCode);
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

		//	If we're dead, then exit

		if (g_pHI == NULL)
			return 0;

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

		m_ScreenMgr.ClientToScreen(x, y, &xLocal, &yLocal);
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

		m_ScreenMgr.ClientToScreen(x, y, &xLocal, &yLocal);
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

		m_ScreenMgr.ClientToScreen(x, y, &xLocal, &yLocal);
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

		m_ScreenMgr.ClientToScreen(x, y, &xLocal, &yLocal);
		m_pCurSession->HIMouseMove(xLocal, yLocal, dwFlags);
		}

	return 0;
	}

LONG CHumanInterface::WMMouseWheel (int iDelta, int x, int y, DWORD dwFlags)

//	WMMouseWheel
//
//	Handle WM_MOUSEWHEEL message

	{
	if (m_pCurSession)
		{
		int xLocal, yLocal;

		m_ScreenMgr.ClientToScreen(x, y, &xLocal, &yLocal);
		m_pCurSession->HIMouseWheel(iDelta, xLocal, yLocal, dwFlags);
		}

	return 0;
	}

LONG CHumanInterface::WMMove (int x, int y)

//	WMMove
//
//	Handle WM_MOVE

	{
	m_ScreenMgr.OnWMMove(x, y);

	if (m_pCurSession)
		m_pCurSession->HISize(GetScreen().GetWidth(), GetScreen().GetHeight());

	return 0;
	}

LONG CHumanInterface::WMSize (int cxWidth, int cyHeight, int iSize)

//	WMSize
//
//	Handle WM_SIZE

	{
	m_ScreenMgr.OnWMSize(cxWidth, cyHeight, iSize);

	if (m_pCurSession)
		m_pCurSession->HISize(GetScreen().GetWidth(), GetScreen().GetHeight());

	return 0;
	}

LONG CHumanInterface::WMTimer (DWORD dwID)

//	WMTimer
//
//	Handle WM_TIMER

	{
	m_Timers.FireTimer(m_hWnd, dwID);

	return 0;
	}
