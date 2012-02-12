//	MainApp.cpp
//
//	Main loop and main window proc
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"
#include "HIUtil.h"
#include "Utilities.h"

static const int TIMER_RESOLUTION =						1;
static const int FRAME_DELAY =							1000 / 30;
static const char *HI_WINDOW_CLASS =					"hi_main_1";

struct SCreateCtx
	{
	HINSTANCE hInst;
	int nCmdShow;
	LPSTR lpCmdLine;
	IHIController *pController;
	HWND hWnd;

	SHIOptions Options;
	};

ALERROR CreateMainWindow (SCreateCtx &Ctx);
int MainLoop (HWND hWnd);
LONG APIENTRY MainWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);

CHumanInterface *g_pHI = NULL;
HINSTANCE g_hInst = NULL;

int HIRunApplication (HINSTANCE hInst, int nCmdShow, LPSTR lpCmdLine, IHIController *pController)

//	HIRunApplication
//
//	This function should be called inside WinMain after kernelInit.

	{
	g_hInst = hInst;

	SCreateCtx Ctx;
	Ctx.hInst = hInst;
	Ctx.nCmdShow = nCmdShow;
	Ctx.lpCmdLine = lpCmdLine;
	Ctx.pController = pController;
	Ctx.hWnd = NULL;

	//	Boot the controller to get options

	ASSERT(pController);
	if (pController->HIBoot(Ctx.lpCmdLine, Ctx.Options) != NOERROR)
		return -1;

	//	Create the main window
	
	if (CreateMainWindow(Ctx) != NOERROR)
		return -1;

	//	Event loop

	return MainLoop(Ctx.hWnd);
	}

ALERROR CreateMainWindow (SCreateCtx &Ctx)

//	CreateMainWindow
//
//	Registers and creates the main window

	{
    WNDCLASSEX wc;

	//	Register window

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = Ctx.hInst;
	wc.hIcon = ::LoadIcon(Ctx.hInst, "AppIcon");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = HI_WINDOW_CLASS;
	if (!RegisterClassEx(&wc))
		return ERR_FAIL;

	//	Create the window

    Ctx.hWnd = CreateWindowEx(0,
			HI_WINDOW_CLASS, 
			Ctx.Options.m_sAppTitle.GetASCIIZPointer(),
			WS_POPUP,
			0,
			0, 
			GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN), 
			NULL,
			NULL,
			Ctx.hInst,
			&Ctx);

	if (Ctx.hWnd == NULL)
		return ERR_FAIL;

	//	Show the window

	if (Ctx.Options.m_bWindowedMode)
		ShowWindow(Ctx.hWnd, Ctx.nCmdShow);
	else
		ShowWindow(Ctx.hWnd, SW_SHOWMAXIMIZED);
    UpdateWindow(Ctx.hWnd);

	return NOERROR;
	}

int MainLoop (HWND hWnd)

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

		//g_pHI->OnAnimate();

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
		DWORD dwNextFrame = dwStartTime + FRAME_DELAY;
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

#if 0
		case WM_CLOSE:
			return g_pTrans->WMClose();
#endif

		case WM_CREATE:
			{
			LPCREATESTRUCT pCreate = (LPCREATESTRUCT)lParam;
			SCreateCtx *pCtx = (SCreateCtx *)pCreate->lpCreateParams;

			//	Initialize the window

			g_pHI = new CHumanInterface;
			if (g_pHI == NULL)
				return -1;

			//	Initialize

			if (g_pHI->WMCreate(pCtx->hInst, hWnd, pCtx->Options, pCtx->pController) != NOERROR)
				{
				::MessageBox(NULL, "Unable to initialize application", pCtx->Options.m_sAppTitle.GetASCIIZPointer(), MB_OK);

				delete g_pHI;
				g_pHI = NULL;
				return -1;
				}

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
				delete g_pHI;
				g_pHI = NULL;
				}

			return iResult;
			}

		case WM_DISPLAYCHANGE:
			return g_pHI->WMDisplayChange((int)wParam, (int)LOWORD(lParam), (int)HIWORD(lParam));

		case WM_HI_TASK_COMPLETE:
			g_pHI->OnTaskComplete(lParam);
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
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

