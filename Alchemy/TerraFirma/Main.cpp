//	Main.cpp
//
//	Main program file for Terra Firma

#include "Alchemy.h"
#include "TerraFirma.h"

#define FRAME_DELAY								33	//	30-frames per second

char g_szAppName[] = "TerraFirma";

//	Forwards

HWND CreateMainWindow (HINSTANCE hInstance);
int MainLoop (void);
LONG APIENTRY MainWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);

HWND CreateMainWindow (HINSTANCE hInstance)

//	CreateMainWindow
//
//	Registers and creates the main window

	{
	WNDCLASS  wc;
	HWND hWnd;
	DWORD dwStyle;
	RECT rcRect;

	//	Register the class

	utlMemSet(&wc, sizeof(wc), 0);
	wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)MainWndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject(BLACK_BRUSH);
	wc.lpszClassName = g_szAppName;

	if (!RegisterClass(&wc))
		return NULL;

	//	Figure out paramenters

	dwStyle = WS_POPUP | WS_CLIPSIBLINGS;

	//	Make the window as big as the screen

	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = GetSystemMetrics(SM_CXSCREEN);
	rcRect.bottom = GetSystemMetrics(SM_CYSCREEN);

	//	Create the window

	hWnd = CreateWindow(
			g_szAppName,
			"",
			dwStyle,
			rcRect.left, rcRect.top, RectWidth(&rcRect), RectHeight(&rcRect),
			NULL,
			NULL,
			hInstance,
			NULL);
	if (hWnd == NULL)
		return NULL;

	//	Show the window

	ShowWindow(hWnd, SW_SHOWMAXIMIZED);

	return hWnd;
	}

int MainLoop (void)

//	MainLoop
//
//	Main loop for the program

	{
	MSG msg;
	int iLastAnimate;
	int iDelay;

	iLastAnimate = GetTickCount();

	while (TRUE)
		{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE|PM_NOYIELD))
			{
			BOOL bHandled = FALSE;

			if (msg.message == WM_QUIT)
				return msg.wParam;

			//	If any of our windows need dialog key handling,
			//	do it now

#ifdef LATER
			for (i = 0; i < m_WindowList.GetCount(); i++)
				{
				GAWindow *pWindow = (GAWindow *)m_WindowList.GetObject(i);

				if (pWindow->NeedsDialogHandling())
					if (IsDialogMessage(pWindow->GetHWND(), &msg))
						{
						bHandled = TRUE;
						break;
						}
				}
#endif

			//	If we haven't handled it yet, handle it now

			if (!bHandled)
				{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				}
			}
		else
			{
			//	Animate, if necessary

#ifdef LATER
			for (i = 0; i < m_WindowList.GetCount(); i++)
				{
				GAWindow *pWindow = (GAWindow *)m_WindowList.GetObject(i);

				if (pWindow->NeedsAnimation())
					pWindow->Animate();
				}
#endif

			//	Figure out how long to sleep to maintain the proper frame rate

			iDelay = FRAME_DELAY - (GetTickCount() - iLastAnimate);
			iDelay = min(FRAME_DELAY, max(0, iDelay));
			Sleep(iDelay);
			iLastAnimate = GetTickCount();
			}
		}

	return 0;
	}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevIntance, LPSTR lpCmdLine, int nCmdShow)

//	WinMain
//
//	Main entry-point for GUI programs

	{
	HWND hMainWnd;
	int iResult;

	//	Initialize the kernel

	if (!kernelInit())
		{
		MessageBox(NULL, "Alchemy: Unable to initialize kernel", g_szAppName, MB_OK | MB_ICONSTOP);
		return 0;
		}

	//	Create the main window

	hMainWnd = CreateMainWindow(hInstance);
	if (hMainWnd == NULL)
		{
		MessageBox(NULL, "TerraFirma: Unable to create main window", g_szAppName, MB_OK | MB_ICONSTOP);
		kernelCleanUp();
		return 0;
		}

	//	Main loop

	iResult = MainLoop();

	//	Done

	kernelCleanUp();
	return iResult;
	}
