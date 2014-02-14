//	MainWndProc.cpp
//
//	Implementation of main window proc

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

CTranscendenceWnd *g_pTrans = NULL;

LONG APIENTRY MainWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)

//	MainWndProc
//
//	WndProc for main window

	{
	ALERROR error;

	switch (message)
		{
		case WM_ACTIVATEAPP:
			return g_pHI->WMActivateApp(wParam ? true : false);

		case WM_CHAR:
			return g_pHI->WMChar((char)wParam, (DWORD)lParam);

		case WM_CLOSE:
			return g_pTrans->WMClose();

		case WM_CREATE:
			{
			LPCREATESTRUCT pCreate = (LPCREATESTRUCT)lParam;
			SCreateTrans *pCreateTrans = (SCreateTrans *)pCreate->lpCreateParams;
			long iResult;

			//	Initialize the window

			CHumanInterface::Create();

			//	Create the controller

			CTranscendenceController *pController = new CTranscendenceController(*g_pHI);
			if (pController == NULL)
				{
				CHumanInterface::Destroy();
				return -1;
				}

			//	Create legacy window class

			g_pTrans = new CTranscendenceWnd(hWnd, pController);
			if (g_pTrans == NULL)
				{
				CHumanInterface::Destroy();
				return -1;
				}

			//	Initialize

			if (error = g_pHI->WMCreate(pCreate->hInstance, hWnd, pCreateTrans->pszCommandLine, pController))
				{
				if (error != ERR_CANCEL)
					::MessageBox(NULL, "Unable to initialize Transcendence", "Transcendence", MB_OK);

				CHumanInterface::Destroy();
				return -1;
				}

			//	Legacy

			CString sError;
			iResult = g_pTrans->WMCreate(&sError);
			if (iResult != 0)
				{
				::MessageBox(NULL, sError.GetASCIIZPointer(), "Transcendence", MB_OK);

				delete g_pTrans;
				g_pTrans = NULL;
				CHumanInterface::Destroy();
				return iResult;
				}

			pCreateTrans->bWindowed = pController->GetOptionBoolean(CGameSettings::windowedMode);

			return 0;
			}

		case WM_DESTROY:
			{
			long iResult = 0;

			//	Clean up window

			try
				{
				if (g_pHI)
					{
					g_pHI->WMDestroy();
					CHumanInterface::Destroy();
					}

				//	Clean up

				if (g_pTrans)
					{
					iResult = g_pTrans->WMDestroy();
					delete g_pTrans;
					g_pTrans = NULL;
					}
				}
			catch (...)
				{
				g_pHI = NULL;
				g_pTrans = NULL;
				}

			//	Quit the app 
			//	
			//	NOTE: We do this last because we want to wait until all
			//	windows are done processing. In particular, MCI playback windows
			//	need to process messages while quitting.

			::PostQuitMessage(0);

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
			return (g_pHI ? g_pHI->WMKeyUp((int)wParam, lParam) : 0);

		case WM_LBUTTONDBLCLK:
			return g_pHI->WMLButtonDblClick((int)LOWORD(lParam), (int)HIWORD(lParam), wParam);

		case WM_LBUTTONDOWN:
			return g_pHI->WMLButtonDown((int)LOWORD(lParam), (int)HIWORD(lParam), wParam);

		case WM_LBUTTONUP:
			return (g_pHI ? g_pHI->WMLButtonUp((int)LOWORD(lParam), (int)HIWORD(lParam), wParam) : 0);

		case MCIWNDM_NOTIFYMODE:
			return (g_pHI ? g_pHI->MCINotifyMode((int)lParam) : 0);

		case MCIWNDM_NOTIFYPOS:
			return 0;

		case WM_MOUSEMOVE:
			return g_pHI->WMMouseMove((int)LOWORD(lParam), (int)HIWORD(lParam), wParam);

		case WM_MOUSEWHEEL:
			return g_pHI->WMMouseWheel((int)(short)HIWORD(wParam), (int)LOWORD(lParam), (int)HIWORD(lParam), (DWORD)LOWORD(wParam));

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

		case WM_TIMER:
			return g_pHI->WMTimer((DWORD)wParam);

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
