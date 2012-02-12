//	CUApplication.cpp
//
//	CUApplication object

#include "Alchemy.h"
#include "ALGUI.h"

#define FRAME_DELAY							50	//	50 mSec frame delay (20 frames per sec)

static CObjectClass<CUApplication>g_Class(OBJID_CUAPPLICATION, NULL);

CUApplication::CUApplication (void) : CObject(&g_Class)

//	CUApplication constructor

	{
	}

CUApplication::CUApplication (HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow) : CObject(&g_Class),
		m_hInstance(hInstance),
		m_sCommandLine(LITERAL(lpCmdLine)),
		m_iInitialShow(nCmdShow),
		m_Resources(hInstance),
		m_WindowList(TRUE),
		m_iFrameDelay(FRAME_DELAY),
		m_pfTimerMsg(NULL)

//	CUApplication constructor

	{
	}

CUApplication::~CUApplication (void)

//	CUApplication destructor

	{
	}

ALERROR CUApplication::CreateNewWindow (IUController *pController, int iID, DWORD dwFlags, CUWindow **retpWindow)

//	CreateWindow
//
//	Create a new window

	{
	ALERROR error;
	CUWindow *pWindow;

	pWindow = new CUWindow(this, pController, iID, dwFlags);
	if (pWindow == NULL)
		return ERR_MEMORY;

	//	Do the actual creation

	if (error = pWindow->Boot())
		{
		delete pWindow;
		return error;
		}

	//	Add to our list

	if (error = m_WindowList.AppendObject(pWindow, NULL))
		{
		delete pWindow;
		return error;
		}

	//	Done

	*retpWindow = pWindow;

	return NOERROR;
	}

void CUApplication::DestroyWindow (CUWindow *pWindow)

//	DestroyWindow
//
//	Destroy the window

	{
	int iIndex;

	//	Look for the window in the window list

	iIndex = m_WindowList.FindObject(pWindow);
	ASSERT(iIndex != -1);

	//	Remove it

	m_WindowList.RemoveObject(iIndex);

	//	If we are left with no more windows, post a quit message
	//	so that we exit the application

	if (m_WindowList.GetCount() == 0)
		PostQuitMessage(1);
	}

void CUApplication::Quit (void)

//	Quit
//
//	Quits the app

	{
	int i;

	//	Send a WM_CLOSE to all our windows

	for (i = 0; i < m_WindowList.GetCount(); i++)
		{
		CUWindow *pWindow = (CUWindow *)m_WindowList.GetObject(i);

		SendMessage(pWindow->GetHWND(), WM_CLOSE, 0, 0);
		}
	}

ALERROR CUApplication::Run (IUController *pController, DWORD dwFlags)

//	Run
//
//	Run the main loop

	{
	MSG msg;
	int i;
	int iNextAnimate;

	//	If the main window is not yet visible, show it now

	GetMainWindow()->ShowMainWindow();

	//	Start main loop

	iNextAnimate = GetTickCount() + m_iFrameDelay;
	kernelDebugLogMessage("Starting main loop");

	while (TRUE)
		{
		int iTimeout;
		DWORD dwWait;

		//	Figure out how long until our next animation

		iTimeout = iNextAnimate - GetTickCount();

		//	If it is not time yet, wait until it is time or
		//	until we get an event

		if (iTimeout > 0)
			{
			dwWait = MsgWaitForMultipleObjects(0,
					NULL,
					FALSE,
					iTimeout,
					QS_ALLINPUT);
			}
		else
			dwWait = WAIT_TIMEOUT;

		//	If it is time to animate, do it now

		if (dwWait == WAIT_TIMEOUT)
			{
			iNextAnimate = GetTickCount() + m_iFrameDelay;

			//	Tell the controller that we're animating

			CallNotifyProc(pController, m_pfTimerMsg, 0, 0);

			//	Animate, if necessary

			for (i = 0; i < m_WindowList.GetCount(); i++)
				{
				CUWindow *pWindow = (CUWindow *)m_WindowList.GetObject(i);

				if (pWindow->NeedsAnimation())
					pWindow->Animate();
				}
			}

		//	Process all events

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE|PM_NOYIELD))
			{
			BOOL bHandled = FALSE;

			//	Check to see if we're quitting

			if (msg.message == WM_QUIT)
				return NOERROR;

			//	If we haven't handled it yet, handle it now

			if (!bHandled)
				{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				}
			}
		}

	return NOERROR;
	}

