//	TransSaver.cpp
//
//	Transcendence Screen Saver
//	Copyright (c) 2007-2010 by George Moromisato

#include "PreComp.h"

const int FRAME_DELAY = (1000 / g_TicksPerSecond);

LRESULT WINAPI ScreenSaverProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	static int iTimerID;
	static CPeriodicWaiter Waiter(FRAME_DELAY);

	switch (message)
		{
		case WM_CREATE:
			{
			//	Initialize kernel

			if (!kernelInit())
				return -1;

			//	Initialize the window

			CHumanInterface::Create();

			//	Create the controller

			CTransSaverController *pController = new CTransSaverController(*g_pHI);
			if (pController == NULL)
				{
				CHumanInterface::Destroy();
				return -1;
				}

			//	NULL means the current process module

			HMODULE hModule = NULL;

			//	Initialize

			if (g_pHI->WMCreate(hModule, hWnd, "", pController) != NOERROR)
				{
				CHumanInterface::Destroy();
				return -1;
				}

			//	Initialize timer

			iTimerID = ::SetTimer(hWnd, 1, 10, NULL);
			break;
			}

		case WM_DESTROY:
			{
			::PostQuitMessage(0);

			//	Clean up timer

			if (iTimerID)
				::KillTimer(hWnd, iTimerID);

			//	Clean up window

			if (g_pHI)
				{
				g_pHI->WMDestroy();
				CHumanInterface::Destroy();
				}

			//	Clean up kernel

			kernelCleanUp();
			break;
			}

		case WM_ERASEBKGND:
			{
			HDC hDC = ::GetDC(hWnd);
			RECT rcRect;
			::GetClientRect(hWnd, &rcRect);
			gdiFillRect(hDC, &rcRect, RGB(0, 0, 0));
			::ReleaseDC(hWnd, hDC);
			break;
			}

		case WM_HI_COMMAND:
			g_pHI->OnPostCommand(lParam);
			break;

		case WM_HI_TASK_COMPLETE:
			g_pHI->OnTaskComplete(wParam, lParam);
			break;

		case WM_TIMER:
			{
			Waiter.Wait();
			g_pHI->OnAnimate();
			break;
			}

        default:
			return (::DefScreenSaverProc(hWnd, message, wParam, lParam));
		}

	return 0;
	}

BOOL WINAPI ScreenSaverConfigureDialog (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
	return TRUE;
	}

BOOL WINAPI RegisterDialogClasses (HANDLE hInst)
	{
	return TRUE;
	}
