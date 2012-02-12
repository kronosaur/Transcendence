// S1.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "S1.h"

const int TIMER_RESOLUTION =					1;
const int TICKS_PER_SECOND =					30;
const int FRAME_DELAY =							(1000 / TICKS_PER_SECOND);

//	Globals

static const char *g_pszClassName = "s1_class";
HINSTANCE g_hInst = NULL;

//	Forwards

ALERROR CreateMainWindow (HINSTANCE hInstance, int nCmdShow, LPSTR lpCmdLine, HWND *rethWnd);
int MainLoop (HWND hWnd, int iFrameDelay);

int WINAPI WinMain (HINSTANCE hInstance, 
					HINSTANCE hPrevInstance, 
					LPSTR lpCmdLine, 
                    int nCmdShow)

//	WinMain
//
//	Main Windows entry-point

	{
	ALERROR error;
	HWND hWnd;
	int iResult;

	g_hInst = hInstance;

	//	Initialize alchemy

	if (!kernelInit())
		return 0;

	//	Create the main window

	if (error = CreateMainWindow(hInstance, nCmdShow, lpCmdLine, &hWnd))
		{
		kernelCleanUp();
		return 0;
		}

	//	Event loop

	iResult = MainLoop(hWnd, FRAME_DELAY);

	//	Done

	kernelCleanUp();

	return iResult;
	}

ALERROR CreateMainWindow (HINSTANCE hInstance, int nCmdShow, LPSTR lpCmdLine, HWND *rethWnd)

//	CreateMainWindow
//
//	Registers and creates the main window

	{
    WNDCLASSEX wc;

	//	Struct

	SCreateS1 CreateData;
	CreateData.hInst = hInstance;
	CreateData.pszCommandLine = lpCmdLine;

	//	Register window

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
	wc.hIcon = ::LoadIcon(hInstance, "AppIcon");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = g_pszClassName;
	if (!RegisterClassEx(&wc))
		return ERR_FAIL;

	//	Create the window

    *rethWnd = CreateWindowEx(0,	// WS_EX_TOPMOST,
			g_pszClassName, 
			"S1: Guide Hierarchy",
			WS_POPUP,
			0,
			0, 
			GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN), 
			NULL,
			NULL,
			hInstance,
			&CreateData);

	if (*rethWnd == NULL)
		return ERR_FAIL;

	//	Show the window

	if (CreateData.bWindowed)
		ShowWindow(*rethWnd, nCmdShow);
	else
		ShowWindow(*rethWnd, SW_SHOWMAXIMIZED);
    UpdateWindow(*rethWnd);

	return NOERROR;
	}

int MainLoop (HWND hWnd, int iFrameDelay)

//	MainLoop
//
//	Runs until the program terminates

	{
	MSG msg;

	timeBeginPeriod(TIMER_RESOLUTION);

	//	Start main loop

	DWORD dwStartTime = timeGetTime();

	while (TRUE)
		{
		//	Tell the main window that we're animating

		g_pHI->OnAnimate();

		//	Process all events

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE|PM_NOYIELD))
			{
			BOOL bHandled = FALSE;

			//	Check to see if we're quitting

			if (msg.message == WM_QUIT)
				return (int)msg.wParam;

			//	If we haven't handled it yet, handle it now

			if (!bHandled)
				{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				}
			}

		//	Figure out how long until our next animation

		DWORD dwNow = timeGetTime();
		DWORD dwNextFrame = dwStartTime + iFrameDelay;
		if (dwNextFrame > dwNow)
			{
			::Sleep(dwNextFrame - dwNow);
			dwStartTime = dwNextFrame;
			}
		else
			dwStartTime = dwNow;
		}

	timeEndPeriod(TIMER_RESOLUTION);

	return 0;
	}
