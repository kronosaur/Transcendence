//	CScreenMgr.cpp
//
//	CScreenMgr class

#include <windows.h>
#include <dsound.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

const int EXCLUSIVE_BACK_BUFFERS = 2;

CScreenMgr::CScreenMgr (void) :
		m_hWnd(NULL),
		m_pDD(NULL),
		m_pPrimary(NULL),
		m_PrimaryType(CG16bitImage::stUnknown),
		m_pBack(NULL),
		m_pCurrent(NULL),
		m_pClipper(NULL),
		m_bDXReady(false),
		m_bMinimized(false),
		m_hBackgroundThread(INVALID_HANDLE_VALUE),
		m_hWorkEvent(INVALID_HANDLE_VALUE),
		m_hQuitEvent(INVALID_HANDLE_VALUE)

//	CScreenMgr constructor

	{
	}

CScreenMgr::~CScreenMgr (void)

//	CScreenMgr destructor

	{
	CleanUp();
	}

void CScreenMgr::BackgroundCleanUp (void)

//	BackgroundCleanUp
//
//	Clean up

	{
	if (m_hBackgroundThread != INVALID_HANDLE_VALUE)
		{
		::SetEvent(m_hQuitEvent);
		::WaitForSingleObject(m_hBackgroundThread, 5000);

		::CloseHandle(m_hBackgroundThread);
		m_hBackgroundThread = INVALID_HANDLE_VALUE;

		::CloseHandle(m_hWorkEvent);
		m_hWorkEvent = INVALID_HANDLE_VALUE;

		::CloseHandle(m_hQuitEvent);
		m_hQuitEvent = INVALID_HANDLE_VALUE;
		}
	}

void CScreenMgr::BackgroundInit (void)

//	BackgroundInit
//
//	Create a background thread that will do blts

	{
	m_hQuitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hWorkEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hBackgroundThread = ::kernelCreateThread(BackgroundThread, this);
	}

DWORD WINAPI CScreenMgr::BackgroundThread (LPVOID pData)

//	BackgroundThread
//
//	Background thread

	{
	CScreenMgr *pThis = (CScreenMgr *)pData;

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
			pThis->BltToPrimary(pThis->m_Secondary);
			::ResetEvent(pThis->m_hWorkEvent);
			}
		}
	}

void CScreenMgr::Blt (void)

//	Blt
//
//	Blts the given image to the screen. Image must be of the same size as the screen

	{
	CG16bitImage *pBuffer = &m_Screen;

	if (m_pDD)
		{
		//	Make sure the surfaces are OK

		if (m_pPrimary && m_pPrimary->IsLost() == DDERR_SURFACELOST)
			m_pPrimary->Restore();

		if (m_pBack && m_pBack->IsLost() == DDERR_SURFACELOST)
			m_pBack->Restore();

		//	Blt

		if (m_bExclusiveMode)
			m_Screen.BltToSurface(m_pCurrent, m_PrimaryType);
		else if (m_bBackgroundBlt)
			{
			m_Screen.SwapBuffers(m_Secondary);
			Swap(m_rcInvalid, m_rcInvalidSecondary);
			::SetEvent(m_hWorkEvent);
			}
		else
			BltToPrimary(m_Screen);
		}
	else
		{
		HDC hDC = ::GetDC(m_hWnd);
		m_Screen.BltToDC(hDC, m_rcScreen.left, m_rcScreen.top);
		::ReleaseDC(m_hWnd, hDC);
		}
	}

void CScreenMgr::BltToPrimary (CG16bitImage &Image)

//	BltToPrimary
//
//	Blt the image to primary DX surface (assumes we are using DX)

	{
	//	First blt to a surface that conforms to the primary screen res

	Image.BltToSurface(m_pBack, m_PrimaryType);

	//	Now blt the surface

	RECT    rcSrc;  // source blit rectangle
	RECT    rcDest; // destination blit rectangle
	POINT   p;

	// find out where on the primary surface our window lives
	p.x = 0; p.y = 0;
	::ClientToScreen(m_hWnd, &p);
	::GetClientRect(m_hWnd, &rcDest);
	OffsetRect(&rcDest, p.x, p.y);
	SetRect(&rcSrc, 0, 0, m_cxScreen, m_cyScreen);

	m_pPrimary->Blt(&rcDest, m_pBack, &rcSrc, DDBLT_ASYNC, NULL);
	}

void CScreenMgr::BltToScreen (CG16bitImage &Image)

//	BltToScreen
//
//	Blt the image to screen

	{
	if (m_pDD)
		{
		//	Make sure the surfaces are OK

		if (m_pPrimary && m_pPrimary->IsLost() == DDERR_SURFACELOST)
			m_pPrimary->Restore();

		if (m_pBack && m_pBack->IsLost() == DDERR_SURFACELOST)
			m_pBack->Restore();

		//	Blt

		if (m_bExclusiveMode)
			Image.BltToSurface(m_pCurrent, m_PrimaryType);
		else
			BltToPrimary(Image);
		}
	else
		{
		HDC hDC = ::GetDC(m_hWnd);
		Image.BltToDC(hDC, m_rcScreen.left, m_rcScreen.top);
		::ReleaseDC(m_hWnd, hDC);
		}
	}

bool CScreenMgr::CheckIsReady (void)

//	CheckIsRead
//
//	Make sure that directdraw is ready. Returns FALSE if not ready.

	{
	if (m_pDD && !m_bDXReady)
		{
		HRESULT hr = m_pDD->TestCooperativeLevel();
		if (hr != DD_OK)
			return false;

		m_bDXReady = true;
		}

	return true;
	}

void CScreenMgr::CleanUp (void)

//	CleanUp
//
//	Terminates the manager

	{
	BackgroundCleanUp();

	if (m_pDD)
		{
		//	Release back
		if (m_pBack)
			{
			m_pBack->Release();
			m_pBack = NULL;
			}

		//	Release primary surface
		m_pPrimary->Release();
		m_pPrimary = NULL;
		m_pCurrent = NULL;

		//	Back to normal mode
		m_pDD->RestoreDisplayMode();
		m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL);

		//	Release clipper
		if (m_pClipper)
			{
			m_pClipper->Release();
			m_pClipper = NULL;
			}

		//	Release DirectDraw
		m_pDD->Release();
		m_pDD = NULL;
		}
	}

void CScreenMgr::ClientToScreen (int x, int y, int *retx, int *rety)

//	ClientToScreen
//
//	Converts from window client coordinates to screen manager coordinates

	{
	if (m_pDD && m_bWindowedMode)
		{
		RECT rcClient;
		::GetClientRect(m_hWnd, &rcClient);

		int cxClient = RectWidth(rcClient);
		int cyClient = RectHeight(rcClient);

		//	In DX we stretch the screen to fit into the client area.

		if (cxClient > 0 && cyClient > 0)
			{
			*retx = m_cxScreen * x / cxClient;
			*rety = m_cyScreen * y / cyClient;
			}
		else
			{
			*retx = x;
			*rety = y;
			}
		}
	else
		{
		//	Compensate for the fact that we center the screen rect
		//	in the window client area.

		*retx = x - m_rcScreen.left;
		*rety = y - m_rcScreen.top;
		}
	}

void CScreenMgr::DebugOutputStats (void)

//	DebugOutputStats
//
//	Output debug stats

	{
	if (m_pDD)
		{
		//	DirectX info

		kernelDebugLogMessage("DIRECT X");
		if (m_bExclusiveMode)
			kernelDebugLogMessage("exclusive mode");
		kernelDebugLogMessage("Screen: %d x %d (%d-bit color)", m_cxScreen, m_cyScreen, m_iColorDepth);
		if (m_PrimaryType == CG16bitImage::r5g5b5)
			kernelDebugLogMessage("Pixels: 5-5-5");
		else
			kernelDebugLogMessage("Pixels: 5-6-5");
		//kernelDebugLogMessage("Video Memory: %d", caps.dwVidMemTotal);

		//	Primary surface

#if 0
		kernelDebugLogMessage("PRIMARY SURFACE");

		if (!FAILED(hr))
			DebugOutputSurfaceDesc(desc);
		else
			kernelDebugLogMessage("GetSurfaceDesc failed: %x", hr);

		//	Back buffer

		if (m_pBack)
			{
			kernelDebugLogMessage("SECONDARY SURFACE");

			DDSURFACEDESC2 desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.dwSize = sizeof(desc);
			hr = m_pBack->GetSurfaceDesc(&desc);

			if (!FAILED(hr))
				DebugOutputSurfaceDesc(desc);
			else
				kernelDebugLogMessage("GetSurfaceDesc failed: %x", hr);
			}
#endif
		}
	}

void CScreenMgr::FlipInt (void)

//	FlipInt
//
//	Flips DX surfaces in exclusive mode

	{
	ASSERT(m_pDD);
	ASSERT(m_bExclusiveMode);
	ASSERT(m_pPrimary);

	HRESULT hr = m_pPrimary->Flip(NULL, DDFLIP_WAIT);
	if (FAILED(hr))
		kernelDebugLogMessage("Flip failed: %x", hr);
	}

void CScreenMgr::GlobalToLocal (int x, int y, int *retx, int *rety)

//	GlobalToLocal
//
//	Converts global (screen) coordinates to local (screen manager)

	{
	POINT pt;
	pt.x = x;
	pt.y = y;

	::ScreenToClient(m_hWnd, &pt);

	ClientToScreen(pt.x, pt.y, retx, rety);
	}

ALERROR CScreenMgr::Init (SScreenMgrOptions &Options, CString *retsError)

//	Init
//
//	Initializes the manager

	{
	//	Figure out if we should try to use multiple monitors (this is needed
	//	for screen savers)

	int cxScreen = (Options.m_bMultiMonitorMode ? ::GetSystemMetrics(SM_CXVIRTUALSCREEN) : ::GetSystemMetrics(SM_CXSCREEN));
	int cyScreen = (Options.m_bMultiMonitorMode ? ::GetSystemMetrics(SM_CYVIRTUALSCREEN) : ::GetSystemMetrics(SM_CYSCREEN));

	//	Compute some settings

	m_hWnd = Options.m_hWnd;
	m_bWindowedMode = Options.m_bWindowedMode;
	m_cxScreen = ((!m_bWindowedMode && !Options.m_bForceScreenSize) ? cxScreen : Options.m_cxScreen);
	m_cyScreen = ((!m_bWindowedMode && !Options.m_bForceScreenSize) ? cyScreen : Options.m_cyScreen);
	m_iColorDepth = Options.m_iColorDepth;
	m_bMinimized = false;
	m_bDebugVideo = Options.m_bDebugVideo;

	m_rcScreen.left = 0;
	m_rcScreen.right = m_cxScreen;
	m_rcScreen.top = 0;
	m_rcScreen.bottom = m_cyScreen;

	//	In windowed mode, make sure we have the right styles
	//	(We don't do anything if the window is a child so that we can handle screen savers
	//	in preview window mode).

	DWORD dwStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	if (!(dwStyle & WS_CHILD))
		{
		if (m_bWindowedMode)
			::SetWindowLong(m_hWnd, GWL_STYLE, (dwStyle & ~WS_POPUP) | WS_OVERLAPPEDWINDOW);
		else
			::SetWindowLong(m_hWnd, GWL_STYLE, (dwStyle & ~WS_OVERLAPPEDWINDOW) | WS_POPUP);
		}

	//	Use DX?

	bool bUsingDWM = ((sysGetAPIFlags() & API_FLAG_DWM) ? true : false);
	m_bDX = (bUsingDWM || Options.m_bForceDX) && !Options.m_bForceNonDX;

	//	Exclusive mode?

	m_bExclusiveMode = (m_bDX && (!bUsingDWM || Options.m_bForceExclusiveMode)) 
			&& !m_bWindowedMode
			&& !Options.m_bForceNonExclusiveMode;

	//	Background blt?

	m_bBackgroundBlt = (::sysGetProcessorCount() > 1);

	//	Initialize DirectDraw, if necessary

	if (m_bDX)
		{
		HRESULT hr;
		DDSURFACEDESC2 ddsd;
		hr = ::DirectDrawCreateEx(NULL, (VOID**)&m_pDD, IID_IDirectDraw7, NULL);
		if (FAILED(hr))
			{
			*retsError = strPatternSubst(CONSTLIT("DirectDrawCreate failed: %x"), hr);
			return ERR_FAIL;
			}

		//	Get the capabilities of the device

		DDCAPS caps;
		caps.dwSize = sizeof(caps);
		hr = m_pDD->GetCaps(&caps, NULL);
		if (FAILED(hr))
			{
			*retsError = strPatternSubst(CONSTLIT("GetCaps failed: %x"), hr);
			return ERR_FAIL;
			}

		//	Figure out the current display resolution.

		ddsd.dwSize = sizeof(ddsd);
		hr = m_pDD->GetDisplayMode(&ddsd);
		if (FAILED(hr))
			{
			*retsError = strPatternSubst(CONSTLIT("GetDisplayMode failed: %x"), hr);
			return ERR_FAIL;
			}

		//	Make sure we have enough memory for a back buffer
		//	2 buffers, 2 Bytes per pixel

		if (m_bExclusiveMode)
			{
			DWORD dwMemoryNeeded = EXCLUSIVE_BACK_BUFFERS * m_cxScreen * m_cyScreen * 2;
			if (dwMemoryNeeded > caps.dwVidMemTotal)
				m_bExclusiveMode = false;
			}

		//	Set cooperative level

		DWORD dwFlags;
		if (m_bExclusiveMode)
			dwFlags = DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN;
		else
			dwFlags = DDSCL_NORMAL;

		hr = m_pDD->SetCooperativeLevel(m_hWnd, dwFlags);
		if (FAILED(hr))
			{
			*retsError = strPatternSubst(CONSTLIT("SetCooperativeLevel failed: %x"), hr);
			m_pDD->Release();
			m_pDD = NULL;
			return ERR_FAIL;
			}

		//	Set the display mode

		if (m_bExclusiveMode)
			{
			hr = m_pDD->SetDisplayMode(m_cxScreen, m_cyScreen, m_iColorDepth, 0, 0);
			if (FAILED(hr))
				{
				*retsError = strPatternSubst(CONSTLIT("SetDisplayMode(%dx%dx%d) failed: %x"), m_cxScreen, m_cyScreen, m_iColorDepth, hr);
				m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL);
				m_pDD->Release();
				m_pDD = NULL;
				return ERR_FAIL;
				}
			}

		//	Create the primary surface

		ZeroMemory(&ddsd, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		if (m_bExclusiveMode)
			{
			ddsd.dwFlags |= DDSD_BACKBUFFERCOUNT;
			ddsd.ddsCaps.dwCaps |= DDSCAPS_FLIP | DDSCAPS_COMPLEX;
			ddsd.dwBackBufferCount = EXCLUSIVE_BACK_BUFFERS;
			}

		hr = m_pDD->CreateSurface(&ddsd, &m_pPrimary, NULL);
		if (FAILED(hr))
			{
			*retsError = strPatternSubst(CONSTLIT("CreateSurface failed: %x"), hr);
			m_pDD->RestoreDisplayMode();
			m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL);
			m_pDD->Release();
			m_pDD = NULL;
			return ERR_FAIL;
			}

		//	If we've got a backbuffer then get that

		if (m_bExclusiveMode)
			{
			DDSCAPS2 ddscaps;
			::ZeroMemory(&ddscaps, sizeof(ddscaps));
			ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

			hr = m_pPrimary->GetAttachedSurface(&ddscaps, &m_pBack);
			if (FAILED(hr))
				{
				*retsError = strPatternSubst(CONSTLIT("GetAttachedSurface failed: %x"), hr);
				m_pPrimary->Release();
				m_pDD->RestoreDisplayMode();
				m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL);
				m_pDD->Release();
				m_pDD = NULL;
				return ERR_FAIL;
				}

			m_pCurrent = m_pBack;
			m_PrimaryType = CG16bitImage::GetSurfaceType(m_pPrimary);
			}

		//	Otherwise, create a secondary surface

		else
			{
			::ZeroMemory(&ddsd, sizeof(ddsd));
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
			ddsd.dwWidth = m_cxScreen;
			ddsd.dwHeight = m_cyScreen;
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

			hr = m_pDD->CreateSurface(&ddsd, &m_pBack, NULL);
			if (FAILED(hr))
				{
				*retsError = strPatternSubst(CONSTLIT("CreateSurface failed: %x"), hr);
				m_pPrimary->Release();
				m_pDD->RestoreDisplayMode();
				m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL);
				m_pDD->Release();
				m_pDD = NULL;
				return ERR_FAIL;
				}

			m_pCurrent = m_pBack;
			m_PrimaryType = CG16bitImage::GetSurfaceType(m_pCurrent);
			}

		//	Figure out whether our surface is 5-5-5 or 5-6-5.

		if (m_PrimaryType == CG16bitImage::stUnknown)
			{
			kernelDebugLogMessage("Unknown pixel format");
			m_PrimaryType = CG16bitImage::r5g5b5;
			}

		//	If we're windowed, create a clipper object

		if (!m_bExclusiveMode)
			{
			hr = m_pDD->CreateClipper(0, &m_pClipper, NULL);
			if (FAILED(hr))
				{
				*retsError = strPatternSubst(CONSTLIT("CreateClipper failed: %x"), hr);
				m_pPrimary->Release();
				m_pDD->RestoreDisplayMode();
				m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL);
				m_pDD->Release();
				m_pDD = NULL;
				return ERR_FAIL;
				}

			hr = m_pClipper->SetHWnd(0, m_hWnd);
			hr = m_pPrimary->SetClipper(m_pClipper);
			}
		else
			m_pClipper = NULL;

		//	Done

		m_bDXReady = true;
		}
	else
		m_bDXReady = false;

	//	If in windowed mode, position the window properly

	if (m_bWindowedMode)
		{
		DWORD dwStyles = WS_OVERLAPPEDWINDOW;

		//	Compute how big the window needs to be (this grows the rect
		//	to account for title bar, etc.)

		RECT rcRect;
		rcRect.left = 0;
		rcRect.top = 0;
		rcRect.right = m_cxScreen;
		rcRect.bottom = m_cyScreen;
		::AdjustWindowRect(&rcRect, dwStyles, FALSE);

		//	Center the window on the screen

		uiGetCenteredWindowRect(RectWidth(rcRect), RectHeight(rcRect), &rcRect);

		//	Move/resize the window

		::MoveWindow(m_hWnd, rcRect.left, rcRect.top, RectWidth(rcRect), RectHeight(rcRect), FALSE);
		}

	//	Create an off-screen bitmap

	if (m_Screen.CreateBlank(m_cxScreen, m_cyScreen, false) != NOERROR)
		{
		*retsError = CONSTLIT("Unable to create off-screen bitmap.");
		CleanUp();
		return ERR_FAIL;
		}

	SetInvalidAll(&m_rcInvalid);

	//	If we're doing background blts, set up  some stuff

	if (m_bBackgroundBlt)
		{
		//	We need a secondary window

		if (m_Secondary.CreateBlank(m_cxScreen, m_cyScreen, false) != NOERROR)
			{
			*retsError = CONSTLIT("Unable to create off-screen bitmap.");
			CleanUp();
			return ERR_FAIL;
			}

		SetInvalidAll(&m_rcInvalidSecondary);

		//	We need a new thread

		BackgroundInit();
		}

	//	Done

	if (m_bDebugVideo)
		DebugOutputStats();

	return NOERROR;
	}

void CScreenMgr::LocalToGlobal (int x, int y, int *retx, int *rety)

//	LocalToGlobal
//
//	Converts local (client window) coordinates to global (screen)

	{
	POINT pt;
	ScreenToClient(x, y, (int *)&pt.x, (int *)&pt.y);

	::ClientToScreen(m_hWnd, &pt);

	*retx = pt.x;
	*rety = pt.y;
	}

void CScreenMgr::OnWMActivateApp (bool bActivate)

//	OnWMActivateApp
//
//	Handle app activation

	{
	if (m_pDD)
		{
		if (bActivate)
			{
			if (m_pPrimary)
				m_pPrimary->Restore();

			if (m_pBack)
				m_pBack->Restore();
			}
		else
			{
			m_bDXReady = false;
			}
		}
	}

void CScreenMgr::OnWMDisplayChange (int iBitDepth, int cxWidth, int cyHeight)

//	OnWMDisplayChange
//
//	Handle display change

	{
	if (m_pDD)
		{
		HRESULT hr;

		if (m_bDXReady)
			{
			hr = m_pDD->TestCooperativeLevel();
			if (hr != DD_OK)
				m_bDXReady = false;
			}
		}
	}

void CScreenMgr::OnWMMove (int x, int y)

//	OnWMMove
//
//	Handle WM_MOVE

	{
	}

void CScreenMgr::OnWMSize (int cxWidth, int cyHeight, int iSize)

//	OnWMSize
//
//	Handle WM_SIZE

	{
	RECT rcClient;

	//	Compute the RECT where we draw the screen (in client coordinate)
	//	(We only care about this in non-DX cases because in DX we stretch
	//	the screen to fit the window).

	if (m_pDD == NULL)
		{
		::GetClientRect(m_hWnd, &rcClient);
		m_rcScreen.left = (RectWidth(rcClient) - m_cxScreen) / 2;
		m_rcScreen.right = m_rcScreen.left + m_cxScreen;
		m_rcScreen.top = (RectHeight(rcClient) - m_cyScreen) / 2;
		m_rcScreen.bottom = m_rcScreen.top + m_cyScreen;
		}

	//	Are we minimized?

	m_bMinimized = (iSize == SIZE_MINIMIZED);
	}

void CScreenMgr::ScreenToClient (int x, int y, int *retx, int *rety)

//	ScreenToClient
//
//	Converts from screen manager coordinates to client coordinate

	{
	if (m_pDD && m_bWindowedMode)
		{
		RECT rcClient;
		::GetClientRect(m_hWnd, &rcClient);

		int cxClient = RectWidth(rcClient);
		int cyClient = RectHeight(rcClient);

		//	In DX we stretch the screen to fit into the client area.

		if (m_cxScreen > 0 && m_cyScreen > 0)
			{
			*retx = cxClient * x / m_cxScreen;
			*rety = cyClient * y / m_cyScreen;
			}
		else
			{
			*retx = x;
			*rety = y;
			}
		}
	else
		{
		//	Compensate for the fact that we center the screen rect
		//	in the window client area.

		*retx = x + m_rcScreen.left;
		*rety = y + m_rcScreen.top;
		}
	}

void CScreenMgr::StopDX (void)

//	StopDX
//
//	Exits exclusive mode, to show an error

	{
	CleanUp();
	}

