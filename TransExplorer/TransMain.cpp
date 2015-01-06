//	TransMain.cpp
//
//	Main application entry-point for Transcendence UI applications
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"
#include <mmsystem.h>

const int TIMER_RESOLUTION =					1;
const int FRAME_DELAY =							(1000 / TICKS_PER_SECOND);		//	Delay in milliseconds

struct SCreateTrans
	{
	HINSTANCE hInst;
	char *pszCommandLine;
	bool bWindowed;
	};

HINSTANCE g_hInst = NULL;

ALERROR CreateMainWindow (HINSTANCE hInstance, int nCmdShow, LPSTR lpCmdLine, HWND *rethWnd);
int MainLoop (HWND hWnd, int iFrameDelay);
LONG APIENTRY MainWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);

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

	if (!::kernelInit(KERNEL_FLAG_INTERNETS))
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

	SCreateTrans CreateData;
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
    wc.lpszClassName = TRANS_APP_WINDOW_CLASS;
	if (!RegisterClassEx(&wc))
		return ERR_FAIL;

	//	Create the window

    *rethWnd = CreateWindowEx(0,	// WS_EX_TOPMOST,
			TRANS_APP_WINDOW_CLASS, 
			TRANS_APP_TITLE,
			WS_POPUP | WS_CLIPCHILDREN,
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

LONG APIENTRY MainWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)

//	MainWndProc
//
//	WndProc for main window

	{
	switch (message)
		{
		case WM_ACTIVATEAPP:
			return g_pHI->WMActivateApp(wParam ? true : false);

		case WM_CHAR:
			return g_pHI->WMChar((char)wParam, (DWORD)lParam);

		//case WM_CLOSE:
		//	return g_pTrans->WMClose();

		case WM_CREATE:
			{
			LPCREATESTRUCT pCreate = (LPCREATESTRUCT)lParam;
			SCreateTrans *pCreateTrans = (SCreateTrans *)pCreate->lpCreateParams;

			//	Initialize the window

			CHumanInterface::Create();

			//	Create the controller

			CExplorerController *pController = new CExplorerController(*g_pHI);
			if (pController == NULL)
				{
				CHumanInterface::Destroy();
				return -1;
				}

			//	Initialize

			if (g_pHI->WMCreate(pCreate->hInstance, hWnd, pCreateTrans->pszCommandLine, pController) != NOERROR)
				{
				::MessageBox(NULL, "Unable to initialize Transcendence", TRANS_APP_TITLE, MB_OK);

				CHumanInterface::Destroy();
				return -1;
				}

			//	Legacy

			pCreateTrans->bWindowed = g_pHI->IsWindowedMode();

			return 0;
			}

		case WM_DESTROY:
			{
			long iResult = 0;

			//	Quit the app (we do this first in case we crash in clean up)

			PostQuitMessage(0);

			//	Clean up window

			if (g_pHI)
				{
				g_pHI->WMDestroy();
				CHumanInterface::Destroy();
				}

			return iResult;
			}

		case WM_DISPLAYCHANGE:
			return g_pHI->WMDisplayChange((int)wParam, (int)LOWORD(lParam), (int)HIWORD(lParam));

		case WM_HI_COMMAND:
			g_pHI->OnPostCommand(lParam);
			return 0;

		case WM_HI_TASK_COMPLETE:
			g_pHI->OnTaskComplete(wParam, lParam);
			return 0;

		case WM_KEYDOWN:
			return g_pHI->WMKeyDown((int)wParam, lParam);

		case WM_KEYUP:
			return g_pHI->WMKeyUp((int)wParam, lParam);

		case WM_LBUTTONDBLCLK:
			return g_pHI->WMLButtonDblClick((int)LOWORD(lParam), (int)HIWORD(lParam), wParam);

		case WM_LBUTTONDOWN:
			return g_pHI->WMLButtonDown((int)LOWORD(lParam), (int)HIWORD(lParam), wParam);

		case WM_LBUTTONUP:
			return g_pHI->WMLButtonUp((int)LOWORD(lParam), (int)HIWORD(lParam), wParam);

		case WM_MOUSEMOVE:
			return g_pHI->WMMouseMove((int)LOWORD(lParam), (int)HIWORD(lParam), wParam);

		case WM_MOVE:
			return g_pHI->WMMove((int)LOWORD(lParam), (int)HIWORD(lParam));

		case WM_PAINT:
			{
			PAINTSTRUCT ps;
			HDC hDC = ::BeginPaint(hWnd, &ps);

			::EndPaint(hWnd, &ps);
			return 0;
			}

		case WM_SIZE:
			return g_pHI->WMSize((int)LOWORD(lParam), (int)HIWORD(lParam), (int)wParam);

		default:
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
