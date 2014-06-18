//	CMCIMixer.h
//
//	CMCIMixer class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define CMD_SOUNDTRACK_DONE						CONSTLIT("cmdSoundtrackDone")
#define CMD_SOUNDTRACK_NOW_PLAYING				CONSTLIT("cmdSoundtrackNowPlaying")
#define CMD_SOUNDTRACK_UPDATE_PLAY_POS			CONSTLIT("cmdSoundtrackUpdatePlayPos")

const DWORD CHECK_INTERVAL =					50;
const int FADE_LENGTH =							2000;
const int NORMAL_VOLUME =						1000;
const int FADE_DELAY =							25;

CMCIMixer::CMCIMixer (int iChannels) :
		m_hParent(NULL),
		m_iDefaultVolume(NORMAL_VOLUME),
		m_iCurChannel(0),
		m_pNowPlaying(NULL),
		m_hProcessingThread(INVALID_HANDLE_VALUE),
		m_hWorkEvent(INVALID_HANDLE_VALUE),
		m_hQuitEvent(INVALID_HANDLE_VALUE)

//	CMCIMixer constructor

	{
	int i;

	ASSERT(iChannels > 0);

	//	Create the parent window that will receive notifications.
	//	
	//	NOTE: If we could not create the window then something is wrong and we
	//	disable everything.

	CreateParentWindow();
	if (m_hParent == NULL)
		return;

	//	Create all the MCI windows (one per channel).

	m_Channels.InsertEmpty(iChannels);
	for (i = 0; i < iChannels; i++)
		{
		m_Channels[i].hMCI = ::MCIWndCreate(m_hParent, 
			NULL,
			WS_OVERLAPPED | WS_CHILD | MCIWNDF_NOERRORDLG | MCIWNDF_NOMENU | MCIWNDF_NOPLAYBAR | MCIWNDF_NOTIFYALL,
			NULL);

		//	Abort if we can't do this

		if (m_Channels[i].hMCI == NULL)
			{
			::DestroyWindow(m_hParent);
			m_hParent = NULL;
			return;
			}

		//	Initialize

		m_Channels[i].iState = stateNone;
		}

	//	Start up a processing thread

	m_hQuitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hWorkEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hAbortEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hProcessingThread = ::kernelCreateThread(ProcessingThread, this);
	}

CMCIMixer::~CMCIMixer (void)

//	CMCIMixer destructor

	{
	Shutdown();
	}

void CMCIMixer::CreateParentWindow (void)

//	CreateParentWindow
//
//	Creates the parent window

	{
	if (m_hParent == NULL)
		{
		WNDCLASSEX wc;

		//	Register window

		ZeroMemory(&wc, sizeof(wc));
		wc.cbSize = sizeof(wc);
		wc.style = CS_DBLCLKS;
		wc.lpfnWndProc = ParentWndProc;
		wc.lpszClassName = "TSUI_MCIParent";
		if (!::RegisterClassEx(&wc))
			return;

		//	Create the window

		m_hParent = ::CreateWindowEx(0,	// WS_EX_TOPMOST,
				"TSUI_MCIParent", 
				"",
				WS_POPUP,
				0,
				0, 
				GetSystemMetrics(SM_CXSCREEN),
				GetSystemMetrics(SM_CYSCREEN), 
				NULL,
				NULL,
				NULL,
				this);
		}
	}

void CMCIMixer::EnqueueRequest (ERequestType iType, CSoundType *pTrack, int iPos)

//	EnqueueRequest
//
//	Enqueue a request

	{
	CSmartLock Lock(m_cs);

	SRequest &NewRequest = m_Request.EnqueueAndOverwrite();
	NewRequest.iType = iType;
	NewRequest.pTrack = pTrack;
	NewRequest.iPos = iPos;

	::SetEvent(m_hWorkEvent);
	}

void CMCIMixer::FadeAtPos (int iPos)

//	FadeAtPos
//
//	Fades starting before the given position so that the fade is complete by the
//	time we reach iPos.

	{
	if (m_hParent == NULL)
		return;

	//	Enqueue a wait request

	EnqueueRequest(typeWaitForPos, NULL, iPos - FADE_LENGTH);
	EnqueueRequest(typeFadeOut, NULL, iPos);
	EnqueueRequest(typeStop);
	}

void CMCIMixer::FadeNow (void)

//	FadeNow
//
//	Fades right now.

	{
	if (m_hParent == NULL)
		return;

	//	Enqueue a wait request

	EnqueueRequest(typeFadeOut, NULL, Min(GetCurrentPlayPos() + FADE_LENGTH, GetCurrentPlayLength()));
	EnqueueRequest(typeStop);
	}

bool CMCIMixer::FindChannel (HWND hMCI, SChannel **retpChannel)

//	FindChannel
//
//	Finds the channel by child window

	{
	int i;

	for (i = 0; i < m_Channels.GetCount(); i++)
		if (m_Channels[i].hMCI == hMCI)
			{
			if (retpChannel)
				*retpChannel = &m_Channels[i];
			return true;
			}

	return false;
	}

int CMCIMixer::GetCurrentPlayLength (void) const

//	GetCurrentPlayLength
//
//	Returns the length of the current track.

	{
	HWND hMCI = m_Channels[m_iCurChannel].hMCI;
	return MCIWndGetLength(hMCI);
	}

int CMCIMixer::GetCurrentPlayPos (void) const

//	GetCurrentPlayPos
//
//	Returns the position of the current track.

	{
	HWND hMCI = m_Channels[m_iCurChannel].hMCI;
	return MCIWndGetPosition(hMCI);
	}

void CMCIMixer::LogError (HWND hMCI, const CString &sFilespec)

//	LogError
//
//	Log an error

	{
	CString sError;
	char *pDest = sError.GetWritePointer(1024);
	MCIWndGetError(hMCI, pDest, sError.GetLength());
	sError.Truncate(lstrlen(pDest));

	if (!sFilespec.IsBlank())
		::kernelDebugLogMessage("MCI ERROR [%s]: %s", sFilespec, sError);
	else
		::kernelDebugLogMessage("MCI ERROR: %s", sError);
	}

LONG CMCIMixer::OnNotifyMode (HWND hWnd, int iMode)

//	OnNotifyMode
//
//	Handle MCIWNDM_NOTIFYMODE

	{
	CSmartLock Lock(m_cs);

	SChannel *pChannel;
	if (!FindChannel(hWnd, &pChannel))
		return 0;

	switch (iMode)
		{
		case MCI_MODE_NOT_READY:
			break;

		case MCI_MODE_OPEN:
			break;

		case MCI_MODE_PAUSE:
			break;

		case MCI_MODE_PLAY:
			pChannel->iState = statePlaying;

			//	Notify that we're playing

			if (g_pHI)
				g_pHI->HICommand(CMD_SOUNDTRACK_NOW_PLAYING, m_pNowPlaying);
			break;

		case MCI_MODE_RECORD:
			break;

		case MCI_MODE_SEEK:
			break;

		case MCI_MODE_STOP:
			pChannel->iState = stateNone;

			//	Notify that we're done with this track

			if (g_pHI)
				g_pHI->HICommand(CMD_SOUNDTRACK_DONE);
			break;
		}

	return 0;
	}

LONG CMCIMixer::OnNotifyPos (HWND hWnd, int iPos)

//	OnNotifyPos
//
//	Notify that we've played to a certain position.

	{
	CSmartLock Lock(m_cs);

	SChannel *pChannel;
	if (!FindChannel(hWnd, &pChannel))
		return 0;

	if (g_pHI)
		g_pHI->HICommand(CMD_SOUNDTRACK_UPDATE_PLAY_POS, (void *)iPos);

	return 0;
	}

LONG APIENTRY CMCIMixer::ParentWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)

//	ParentWndProc
//
//	Message handler

	{
	switch (message)
		{
		case WM_CREATE:
			{
			LPCREATESTRUCT pCreate = (LPCREATESTRUCT)lParam;
			CMCIMixer *pThis = (CMCIMixer *)pCreate->lpCreateParams;
			::SetWindowLong(hWnd, GWL_USERDATA, (LONG)pThis);
			return 0;
			}

		case MCIWNDM_NOTIFYMODE:
			{
			CMCIMixer *pThis = (CMCIMixer *)::GetWindowLong(hWnd, GWL_USERDATA);
			return pThis->OnNotifyMode((HWND)wParam, (int)lParam);
			}

		case MCIWNDM_NOTIFYPOS:
			{
			CMCIMixer *pThis = (CMCIMixer *)::GetWindowLong(hWnd, GWL_USERDATA);
			return pThis->OnNotifyPos((HWND)wParam, (int)lParam);
			}

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

bool CMCIMixer::Play (CSoundType *pTrack, int iPos)

//	Play
//
//	Stops all channels and begins playing the given track.

	{
	if (m_hParent == NULL)
		return false;

	//	Enqueue

	EnqueueRequest(typePlay, pTrack, iPos);
	return true;
	}

bool CMCIMixer::PlayFadeIn (CSoundType *pTrack, int iPos)

//	Play
//
//	Stops all channels and begins playing the given track.

	{
	if (m_hParent == NULL)
		return false;

	//	Enqueue

	EnqueueRequest(typeFadeIn, pTrack, iPos);
	return true;
	}

void CMCIMixer::ProcessFadeIn (const SRequest &Request)

//	ProcessFadeIn
//
//	Fade in the given track starting at the given position.

	{
#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogMessage("ProcessFadeIn");
#endif

	//	Stop all channels

	ProcessStop(Request);

	//	Play on some channel

	HWND hMCI = m_Channels[m_iCurChannel].hMCI;

	//	Open new file

	CString sFilespec = Request.pTrack->GetFilespec();
	if (MCIWndOpen(hMCI, sFilespec.GetASCIIZPointer(), 0) != 0)
		{
		LogError(hMCI, sFilespec);
		return;
		}

	//	Set state (we need to do this before we play because the callback inside
	//	MCIWndPlay needs m_pNowPlaying to be valid).

	m_pNowPlaying = Request.pTrack;

	//	Seek to the proper position

	MCIWndSeek(hMCI, Request.iPos);
	
	//	Play it

	MCIWndSetVolume(hMCI, 0);
	if (MCIWndPlay(hMCI) != 0)
		{
		LogError(hMCI, sFilespec);
		return;
		}

	//	Fade in util we reach this position

	int iFullVolPos = Request.iPos + FADE_LENGTH;
	while (true)
		{
		//	Wait a little bit

		if (!Wait(FADE_DELAY))
			return;

		//	How far into the fade

		int iCurPos = GetCurrentPlayPos();
		int iPlaying = iCurPos - Request.iPos;
		if (iPlaying <= 0 || iPlaying >= FADE_LENGTH)
			{
			MCIWndSetVolume(hMCI, m_iDefaultVolume);
			return;
			}

		//	The volume should be proportional to how much we have left

		int iVolume = m_iDefaultVolume * iPlaying / FADE_LENGTH;
		MCIWndSetVolume(hMCI, iVolume);
		}
	}

void CMCIMixer::ProcessFadeOut (const SRequest &Request)

//	ProcessFadeOut
//
//	Fade out

	{
#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogMessage("ProcessFadeOut");
#endif

	HWND hMCI = m_Channels[m_iCurChannel].hMCI;

	int iStartPos = GetCurrentPlayPos();
	int iFadeLen = Request.iPos - iStartPos;
	if (iFadeLen <= 0)
		return;

	//	Loop until we've reached out desired position (or until the end of the
	//	track).

	while (true)
		{
		//	Wait a little bit

		if (!Wait(FADE_DELAY))
			return;

		//	How much longer until we fade completely?

		int iCurPos = GetCurrentPlayPos();
		int iLeft = Request.iPos - iCurPos;
		if (iLeft <= 0)
			{
			MCIWndSetVolume(hMCI, 0);
			return;
			}

		//	The volume should be proportional to how much we have left

		int iVolume = m_iDefaultVolume * iLeft / iFadeLen;
		MCIWndSetVolume(hMCI, iVolume);
		}
	}

void CMCIMixer::ProcessPlay (const SRequest &Request)

//	ProcessPlay
//
//	Play the file

	{
#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogMessage("ProcessPlay: %s", Request.pTrack->GetFilespec());
#endif

	//	Stop all channels

	ProcessStop(Request);

	//	Play on some channel

	HWND hMCI = m_Channels[m_iCurChannel].hMCI;

	//	Open new file

	CString sFilespec = Request.pTrack->GetFilespec();
	if (MCIWndOpen(hMCI, sFilespec.GetASCIIZPointer(), 0) != 0)
		{
		LogError(hMCI, sFilespec);
		return;
		}

	//	Set state (we need to do this before we play because the callback inside
	//	MCIWndPlay needs m_pNowPlaying to be valid).

	m_pNowPlaying = Request.pTrack;

	//	Seek to the proper position

	MCIWndSeek(hMCI, Request.iPos);
	
	//	Play it

	MCIWndSetVolume(hMCI, m_iDefaultVolume);
	if (MCIWndPlay(hMCI) != 0)
		{
		LogError(hMCI, sFilespec);
		return;
		}
	}

void CMCIMixer::ProcessPlayPause (const SRequest &Request)

//	ProcessPlayPause
//
//	Pause and play current track.

	{
#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogMessage("ProcessPlayPause");
#endif

	HWND hMCI = m_Channels[m_iCurChannel].hMCI;

	int iMode = MCIWndGetMode(hMCI, 0, NULL);
	if (iMode == MCI_MODE_PLAY)
		MCIWndPause(hMCI);
	else if (iMode == MCI_MODE_PAUSE)
		MCIWndPlay(hMCI);
	}

bool CMCIMixer::ProcessRequest (void)

//	ProcessRequest
//
//	Process a single request off the queue. If there are no more requests, then
//	we reset the worker event.

	{
	bool bMoreEvents;
	SRequest Request;

	//	Lock while we pull something off the queue

	m_cs.Lock();
	if (m_Request.GetCount() > 0)
		{
		Request = m_Request.Head();
		m_Request.Dequeue();
		}
	else
		Request.iType = typeNone;

	//	If our queue is empty, then reset the work event

	if (m_Request.GetCount() == 0)
		{
		::ResetEvent(m_hWorkEvent);
		bMoreEvents = false;
		}
	else
		bMoreEvents = true;

	//	Unlock so we can process the event

	m_cs.Unlock();

	//	Now process the event based on the type

	switch (Request.iType)
		{
		case typeFadeIn:
			ProcessFadeIn(Request);
			break;

		case typeFadeOut:
			ProcessFadeOut(Request);
			break;

		case typePlay:
			ProcessPlay(Request);
			break;

		case typePlayPause:
			ProcessPlayPause(Request);
			break;

		case typeStop:
			ProcessStop(Request);
			break;

		case typeWaitForPos:
			ProcessWaitForPos(Request);
			break;

		case typeSetPaused:
		case typeSetUnpaused:
			ProcessSetPlayPaused(Request);
			break;
		}

	//	Return whether we think there are more events in the queue. This is a
	//	heuristic, because someone might have changed the queue from under us,
	//	but callers just use this as a hint. No harm if we're wrong.

	return bMoreEvents;
	}

void CMCIMixer::ProcessSetPlayPaused (const SRequest &Request)

//	ProcessSetPlayPaused
//
//	Set to play or paused

	{
	HWND hMCI = m_Channels[m_iCurChannel].hMCI;

	int iMode = MCIWndGetMode(hMCI, 0, NULL);

	if (Request.iType == typeSetPaused)
		{
		if (iMode == MCI_MODE_PLAY)
			MCIWndPause(hMCI);
		}
	else if (Request.iType == typeSetUnpaused)
		{
		if (iMode == MCI_MODE_PAUSE)
			MCIWndPlay(hMCI);
		}
	}

void CMCIMixer::ProcessStop (const SRequest &Request)

//	ProcessStop
//
//	Stop playing

	{
	int i;

	for (i = 0; i < m_Channels.GetCount(); i++)
		if (m_Channels[i].iState == statePlaying)
			{
#ifdef DEBUG_SOUNDTRACK
			kernelDebugLogMessage("ProcessStop");
#endif

			MCIWndStop(m_Channels[i].hMCI);
			}

	m_pNowPlaying = NULL;
	}

void CMCIMixer::ProcessWaitForPos (const SRequest &Request)

//	ProcessWaitForPos
//
//	Waits until the current playback position is at the given position.

	{
#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogMessage("ProcessWaitForPos");
#endif

	HWND hMCI = m_Channels[m_iCurChannel].hMCI;

	while (true)
		{
		//	Keep looping until we reach the given position or until we ask asked
		//	to stop.
		//
		//	We must be in play mode.

		int iMode = MCIWndGetMode(hMCI, 0, NULL);
		if (iMode != MCI_MODE_PLAY)
			return;

		//	Get our current position. If we reached the position, then we're done.

		int iCurPos = MCIWndGetPosition(hMCI);
		if (iCurPos >= Request.iPos)
			return;

		//	Otherwise, we wait for a little bit (and we check to see if someone
		//	wants us to quit.

		if (!Wait(CHECK_INTERVAL))
			return;

		//	Loop and check again
		}
	}

DWORD WINAPI CMCIMixer::ProcessingThread (LPVOID pData)

//	ProcessingThread
//
//	Processing thread

	{
	CMCIMixer *pThis = (CMCIMixer *)pData;

	while (true)
		{
		HANDLE Handles[2];
		Handles[0] = pThis->m_hQuitEvent;
		const DWORD WAIT_QUIT_EVENT = (WAIT_OBJECT_0);
		Handles[1] = pThis->m_hWorkEvent;
		const DWORD WAIT_WORK_EVENT = (WAIT_OBJECT_0 + 1);

		//	Wait for work to do

		DWORD dwWait = ::WaitForMultipleObjects(2, Handles, FALSE, INFINITE);

		//	Do the work

		if (dwWait == WAIT_QUIT_EVENT)
			return 0;
		else if (dwWait == WAIT_WORK_EVENT)
			{
			//	Process requests from the queue until we're done. NOTE: This
			//	will also reset the work event.

			while (pThis->ProcessRequest())
				{
				//	Quit, if necessary

				if (::WaitForSingleObject(pThis->m_hQuitEvent, 0) == WAIT_OBJECT_0)
					return 0;
				}

			//	Keep looping
			}
		}

	return 0;
	}

void CMCIMixer::SetPlayPaused (bool bPlay)

//	SetPlayPaused
//
//	Pause/Play

	{
	if (m_hParent == NULL)
		return;

	EnqueueRequest(bPlay ? typeSetUnpaused : typeSetPaused);
	}

void CMCIMixer::SetVolume (int iVolume)

//	SetVolume
//
//	Sets the volume, where 1000 is normal volume and 0 is no volume.

	{
	m_iDefaultVolume = Max(0, iVolume);

	HWND hMCI = m_Channels[m_iCurChannel].hMCI;
	MCIWndSetVolume(hMCI, m_iDefaultVolume);
	}

void CMCIMixer::Shutdown (void)

//	Shutdown
//
//	Program is quitting, so clean up.

	{
	if (m_hProcessingThread != INVALID_HANDLE_VALUE)
		{
		::SetEvent(m_hQuitEvent);
		::kernelDispatchUntilEventSet(m_hProcessingThread, 5000);

		::CloseHandle(m_hProcessingThread);
		m_hProcessingThread = INVALID_HANDLE_VALUE;

		::CloseHandle(m_hWorkEvent);
		m_hWorkEvent = INVALID_HANDLE_VALUE;

		::CloseHandle(m_hQuitEvent);
		m_hQuitEvent = INVALID_HANDLE_VALUE;

		::CloseHandle(m_hAbortEvent);
		m_hAbortEvent = INVALID_HANDLE_VALUE;
		}

	if (m_hParent)
		{
		::DestroyWindow(m_hParent);
		m_hParent = NULL;
		}

	m_Channels.DeleteAll();
	m_pNowPlaying = NULL;
	}

void CMCIMixer::Stop (void)

//	Stop
//
//	Stops playing all.

	{
	if (m_hParent == NULL)
		return;

	//	Enqueue

	EnqueueRequest(typeStop);
	}

void CMCIMixer::TogglePausePlay (void)

//	TogglePausePlay
//
//	Pause/Play

	{
	if (m_hParent == NULL)
		return;

	//	Enqueue

	EnqueueRequest(typePlayPause);
	}

bool CMCIMixer::Wait (DWORD dwTimeout)

//	Wait
//
//	Waits until the timeout or until we are asked to quit or abort. Returns 
//	FALSE we we need to quit or abort.

	{
	HANDLE Handles[2];
	Handles[0] = m_hQuitEvent;
	const DWORD WAIT_QUIT_EVENT = (WAIT_OBJECT_0);
	Handles[1] = m_hAbortEvent;
	const DWORD WAIT_ABORT_EVENT = (WAIT_OBJECT_0 + 1);

	DWORD dwWait = ::WaitForMultipleObjects(2, Handles, FALSE, dwTimeout);

	//	Quit or abort?

	if (dwWait == WAIT_QUIT_EVENT
			|| dwWait == WAIT_ABORT_EVENT)
		return false;

	//	Done

	return true;
	}
