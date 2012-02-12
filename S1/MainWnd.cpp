//	MainWnd.cpp
//
//	Main window

#include "stdafx.h"
#include "S1.h"

CHumanInterface *g_pHI = NULL;
CS1Controller *g_pCtrl = NULL;

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

		case WM_CLOSE:
			return g_pHI->WMClose();

		case WM_CREATE:
			{
			LPCREATESTRUCT pCreate = (LPCREATESTRUCT)lParam;
			SCreateS1 *pCreateTrans = (SCreateS1 *)pCreate->lpCreateParams;

			//	Initialize the window

			g_pHI = new CHumanInterface;
			if (g_pHI == NULL)
				return -1;

			//	Create the controller

			g_pCtrl = new CS1Controller(*g_pHI);
			if (g_pCtrl == NULL)
				{
				delete g_pHI;
				g_pHI = NULL;
				return -1;
				}

			//	Initialize

			if (g_pHI->WMCreate(pCreate->hInstance, hWnd, pCreateTrans->pszCommandLine, g_pCtrl) != NOERROR)
				{
				::MessageBox(NULL, "Unable to initialize S1", "S1", MB_OK);

				delete g_pHI;
				g_pHI = NULL;
				delete g_pCtrl;
				g_pCtrl = NULL;
				return -1;
				}

			pCreateTrans->bWindowed = false;//pController->GetOptionBoolean(CGameSettings::windowedMode);

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
				g_pCtrl = NULL;
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
			return 0;//g_pTrans->WMMove((int)LOWORD(lParam), (int)HIWORD(lParam));

		case WM_PAINT:
			{
			PAINTSTRUCT ps;
			HDC hDC = ::BeginPaint(hWnd, &ps);

			::EndPaint(hWnd, &ps);
			return 0;
			}

		case WM_SIZE:
			return 0;//g_pTrans->WMSize((int)LOWORD(lParam), (int)HIWORD(lParam), (int)wParam);

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
