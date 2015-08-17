//	CSimViewer.cpp
//
//	CSimViewer class

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

const int FRAMES_PER_SECOND = 30;
const int DELAY_PER_FRAME = (1000 / FRAMES_PER_SECOND);

static char WND_CLASS_NAME[] = "TransData_Viewer";

void CSimViewer::Create (void)

//	Create
//
//	Create the viewer window

	{
    WNDCLASSEX wc;

	//	Register window

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = CSimViewer::WndProc;
    wc.hInstance = NULL;
	wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = WND_CLASS_NAME;
	if (!RegisterClassEx(&wc))
		{
		printf("ERROR: Unable to register viewer window.\n");
		return;
		}

	//	Create the window

    m_hWnd = CreateWindowEx(0,	// WS_EX_TOPMOST,
			WND_CLASS_NAME, 
			"TransData Viewer",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			0, 
			CW_USEDEFAULT,
			0, 
			NULL,
			NULL,
			NULL,
			this);

	if (m_hWnd == NULL)
		{
		printf("ERROR: Unable to create viewer window.\n");
		return;
		}

	//	Show the window

	ShowWindow(m_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hWnd);

	//	Create the buffer

	m_Buffer.Create(1024, 768);

	m_dwLastUpdate = 0;
	}

void CSimViewer::Destroy (void)

//	Destroy
//
//	Destroy the viewer window

	{
	if (m_hWnd)
		{
		::DestroyWindow(m_hWnd);
		m_hWnd = NULL;
		}
	}

void CSimViewer::PaintViewport (CUniverse &Universe)

//	PaintViewport
//
//	Paint the viewport on the window

	{
	if (m_hWnd)
		ProcessMessages();

	//	Wait an appropriate amount of time to paint 30 frames per second

	DWORD dwNow = ::GetTickCount();
	if (m_dwLastUpdate)
		{
		DWORD dwDelay = Max(0, DELAY_PER_FRAME - (int)(dwNow - m_dwLastUpdate));
		::Sleep(dwDelay);
		}

	//	Paint frame

	if (m_hWnd)
		{
		RECT rcView;
		rcView.left = 0;
		rcView.top = 0;
		rcView.right = m_Buffer.GetWidth();
		rcView.bottom = m_Buffer.GetHeight();

		Universe.PaintPOV(m_Buffer, rcView, 0);

		HDC hDC = ::GetDC(m_hWnd);
		if (hDC)
			{
			RECT rcClient;
			::GetClientRect(m_hWnd, &rcClient);
			int x = (RectWidth(rcClient) - m_Buffer.GetWidth()) / 2;
			int y = (RectHeight(rcClient) - m_Buffer.GetHeight()) / 2;

			m_Buffer.BltToDC(hDC, x, y);
			::ReleaseDC(m_hWnd, hDC);
			}
		}

	m_dwLastUpdate = dwNow;
	}

void CSimViewer::ProcessMessages (void)

//	ProcessMessages
//
//	Handles the message loop

	{
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE|PM_NOYIELD))
		{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		}
	}

LONG APIENTRY CSimViewer::WndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)
	{
	switch (message)
		{
		case WM_CREATE:
			{
			LPCREATESTRUCT pCreate = (LPCREATESTRUCT)lParam;
			CSimViewer *pViewer = (CSimViewer *)pCreate->lpCreateParams;
			::SetWindowLong(hWnd, GWL_USERDATA, (LONG)pViewer);
			return 0;
			}

		case WM_DESTROY:
			{
			CSimViewer *pViewer = (CSimViewer *)::GetWindowLong(hWnd, GWL_USERDATA);
			pViewer->WMDestroy();
			return 0;
			}

		case WM_PAINT:
			{
			PAINTSTRUCT ps;
			HDC hDC = ::BeginPaint(hWnd, &ps);
			RECT rcClient;
			::GetClientRect(hWnd, &rcClient);
			gdiFillRect(hDC, &rcClient, RGB(0, 0, 0));
			::EndPaint(hWnd, &ps);
			return 0;
			}

		case WM_SIZE:
			{
			::InvalidateRect(hWnd, NULL, TRUE);
			return 0;
			}

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
