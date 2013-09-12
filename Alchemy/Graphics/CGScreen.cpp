//	CGScreen.cpp
//
//	Implements CGScreen class

#include <windows.h>
#include <MMSystem.h>

#include "Kernel.h"
#include "KernelObjID.h"
#include "Graphics.h"

#ifndef NO_DIRECT_DRAW

#define SHOW_FRAME_COUNT			TRUE	//	Show frame rate
//#define SYNC_FLIP					TRUE	//	::Flip is a synchronous call
//#define LOCK_POINTER				TRUE	//	Output surface pointer on ::LockSurface

#define TASK_NONE					0
#define TASK_FLIP					1
#define TASK_FLIP_OWN				2

typedef struct
	{
	CGScreen *pScreen;					//	Screen object
	int iTask;							//	Task for worker thread
	ALERROR Status;						//	Result of last task

	LPDIRECTDRAWSURFACE7 pPrimary;		//	Primary surface
	LPDIRECTDRAWSURFACE7 pBackBuffer;	//	Back buffer

	HANDLE hWorkToDo;
	HANDLE hWorkDone;
	HANDLE hQuit;
	} WORKTHREADCTX;

WORKTHREADCTX g_Ctx;

//	Forwards

static HRESULT CALLBACK EnumModesCallback (LPDDSURFACEDESC pddsd, LPVOID pCtx);
static HRESULT CALLBACK EnumSurfacesCallback (LPDIRECTDRAWSURFACE7 pSurface, LPDDSURFACEDESC pddsd, LPVOID pCtx);
DWORD WINAPI WorkerThread (LPVOID pvCtx);

CGScreen::CGScreen (void) : m_lpdd(NULL),
		m_pPrimary(NULL),
		m_pBackBuffer(NULL),
		m_p3rdBuffer(NULL),
		m_iStartTick(0),
		m_hWnd(NULL),
		m_pTrans(NULL),
		m_pDepth(NULL),
		m_hWorkerThread(NULL),
		m_fOwnBackBuffer(FALSE)

//	CGScreen constructor

	{
	utlMemSet(&m_Mode, sizeof(m_Mode), 0);
	}

CGScreen::~CGScreen (void)

//	CGScreen destructor

	{
	if (m_lpdd)
		Destroy();
	}

ALERROR CGScreen::Create (void)

//	Create
//
//	Creates the screen object. This function must be called before
//	any other methods

	{
	ALERROR error;
	HRESULT result;

	ASSERT(m_lpdd == NULL);

	//	Create the DirectDraw object

	result = DirectDrawCreate(NULL,
			&m_lpdd,
			NULL);
	if (result != DD_OK)
		return ERR_FAIL;

	//	Initialize the worker thread

	if (error = CreateWorkerThread())
		{
		m_lpdd->Release();
		m_lpdd = NULL;
		return error;
		}

	//	Note that we need to call SetExclusiveMode to get into a
	//	reasonable state.

	m_fExclusive = FALSE;

	return NOERROR;
	}

ALERROR CGScreen::CreateChannel (int cxWidth, int cyHeight, CGChannelStruct **retpChannel)

//	CreateChannel
//
//	Creates an 8-bit channel structure

	{
	int cxWidthBytes;
	CGChannelStruct *pChannel;

	cxWidthBytes = AlignUp(cxWidth, sizeof(DWORD));
	pChannel = (CGChannelStruct *)MemAlloc(sizeof(CGChannelStruct)
			+ cxWidthBytes * cyHeight);
	if (pChannel == NULL)
		return ERR_MEMORY;

	//	Initialize

	pChannel->cxWidthBytes = cxWidthBytes;
	pChannel->cxWidth = cxWidth;
	pChannel->cyHeight = cyHeight;
	pChannel->pMap = (BYTE *)&pChannel[1];

	utlMemSet(pChannel->pMap, cxWidthBytes * cyHeight, 0);

	//	Done

	*retpChannel = pChannel;

	return NOERROR;
	}

ALERROR CGScreen::CreateWorkerThread (void)

//	CreateWorkerThread
//
//	Creates a worker thread

	{
	DWORD dwThreadID;

	ASSERT(m_hWorkerThread == NULL);

	//	Initialize context

	g_Ctx.iTask = TASK_NONE;
	g_Ctx.Status = NOERROR;
	g_Ctx.pScreen = this;
	g_Ctx.pPrimary = NULL;

	g_Ctx.hWorkToDo = CreateEvent(NULL,
			TRUE,
			FALSE,
			NULL);
	if (g_Ctx.hWorkToDo == NULL)
		return ERR_MEMORY;

	g_Ctx.hWorkDone = CreateEvent(NULL,
			TRUE,
			TRUE,
			NULL);
	if (g_Ctx.hWorkDone == NULL)
		{
		CloseHandle(g_Ctx.hWorkToDo);
		return ERR_MEMORY;
		}

	g_Ctx.hQuit = CreateEvent(NULL,
			TRUE,
			FALSE,
			NULL);
	if (g_Ctx.hQuit == NULL)
		{
		CloseHandle(g_Ctx.hWorkDone);
		CloseHandle(g_Ctx.hWorkToDo);
		return ERR_MEMORY;
		}

	//	Create the thread

	m_hWorkerThread = CreateThread(NULL,
			0,
			WorkerThread,
			&g_Ctx,
			0,
			&dwThreadID);
	if (m_hWorkerThread == NULL)
		{
		CloseHandle(g_Ctx.hQuit);
		CloseHandle(g_Ctx.hWorkDone);
		CloseHandle(g_Ctx.hWorkToDo);
		return ERR_MEMORY;
		}

	return NOERROR;
	}

void CGScreen::Destroy (void)

//	Destroy

	{
	ASSERT(m_lpdd);

	//	Get back to windowed mode, if we're not already there

	if (m_fExclusive)
		m_lpdd->RestoreDisplayMode();

	//	Kill the worker thread

	DestroyWorkerThread();

	//	Clean up the buffers

	if (m_pBackBuffer)
		{
		m_pBackBuffer->Release();
		m_pBackBuffer = NULL;
		}

	if (m_pPrimary)
		{
		m_pPrimary->Release();
		m_pPrimary = NULL;
		}

	if (m_pDepth)
		{
		DestroyChannel(m_pDepth);
		m_pDepth = NULL;
		}

	if (m_pTrans)
		{
		DestroyChannel(m_pTrans);
		m_pTrans = NULL;
		}

	//	Clean up

	m_lpdd->Release();
	m_lpdd = NULL;
	}

void CGScreen::DestroyChannel (CGChannelStruct *pChannel)

//	DestroyChannel
//
//	Destroys a channel

	{
	if (pChannel)
		MemFree(pChannel);
	}

void CGScreen::DestroyWorkerThread (void)

//	DestroyWorkerThread
//
//	Destroys the thread

	{
	DWORD dwWait;

	ASSERT(m_hWorkerThread);

	//	Wait for thread to die

	SetEvent(g_Ctx.hQuit);
	dwWait = WaitForSingleObject(m_hWorkerThread, 5000);
	if (dwWait == WAIT_TIMEOUT)
		TerminateThread(m_hWorkerThread, 0);

	CloseHandle(m_hWorkerThread);

	//	Clean up

	CloseHandle(g_Ctx.hWorkToDo);
	CloseHandle(g_Ctx.hWorkDone);
	CloseHandle(g_Ctx.hQuit);
	}

ALERROR CGScreen::InitDrawSurfaces (void)

//	InitDrawSurfaces
//
//	Initialize the surfaces to draw on

	{
	ALERROR error;
	HRESULT result;

	ASSERT(m_lpdd);
	ASSERT(m_fExclusive);			//	For now, we only understand exclusive mode

	//	Create the buffers, if necessary

	if (m_pPrimary == NULL)
		{
		DDSURFACEDESC ddsd;

		ASSERT(m_Mode.iBackBuffers >= 0);

		utlMemSet(&ddsd, sizeof(ddsd), 0);
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;

		if (m_Mode.iBackBuffers > 0)
			{
			ddsd.dwFlags |= DDSD_BACKBUFFERCOUNT;
			ddsd.ddsCaps.dwCaps |= DDSCAPS_COMPLEX | DDSCAPS_FLIP;
			ddsd.dwBackBufferCount = (DWORD)m_Mode.iBackBuffers;
			}

		result = m_lpdd->CreateSurface(&ddsd, &m_pPrimary, NULL);

		//	If all the buffers fit in video memory, then we continue
		//	Otherwise, we create our own off-screen buffers

		if (result == DD_OK)
			{
			//	If we've got a back-buffer, create that too

			if (m_Mode.iBackBuffers > 0)
				{
				DDSCAPS ddscaps;
				CIntArray AllSurfaces;

				utlMemSet(&ddscaps, sizeof(ddscaps), 0);
				ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

				result = m_pPrimary->GetAttachedSurface(&ddscaps, &m_pBackBuffer);
				if (result != DD_OK)
					{
					m_pPrimary->Release();
					m_pPrimary = NULL;
					return ERR_FAIL;
					}

				//	Get a list of all attached surfaces

#ifdef DEBUG
				result = m_pPrimary->EnumAttachedSurfaces(&AllSurfaces, EnumSurfacesCallback);
				if (result != DD_OK)
					kernelDebugLogMessage("Unable to enumerate attached surfaces: %x", result);
#endif
				}
			}

		//	Create our own buffers

		else
			{
#ifdef DEBUG
			kernelDebugLogMessage("Unable to fit all buffers in video memory--using system memory.");
#endif

			utlMemSet(&ddsd, sizeof(ddsd), 0);
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags = DDSD_CAPS;
			ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

			result = m_lpdd->CreateSurface(&ddsd, &m_pPrimary, NULL);
			if (result != DD_OK)
				return ERR_FAIL;

			//	Create a back buffer

			utlMemSet(&ddsd, sizeof(ddsd), 0);
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			ddsd.dwWidth = m_Mode.cxWidth;
			ddsd.dwHeight = m_Mode.cyHeight;

			result = m_lpdd->CreateSurface(&ddsd, &m_pBackBuffer, NULL);
			if (result != DD_OK)
				{
				m_pPrimary->Release();
				m_pPrimary = NULL;
				return ERR_FAIL;
				}

			//	Create 2nd Back buffer

			utlMemSet(&ddsd, sizeof(ddsd), 0);
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			ddsd.dwWidth = m_Mode.cxWidth;
			ddsd.dwHeight = m_Mode.cyHeight;

			result = m_lpdd->CreateSurface(&ddsd, &m_p3rdBuffer, NULL);
			if (result != DD_OK)
				{
				m_pBackBuffer->Release();
				m_pBackBuffer = NULL;
				m_pPrimary->Release();
				m_pPrimary = NULL;
				return ERR_FAIL;
				}

			//	Set a flag to indicate that we need to handle our own
			//	back buffer management.

			m_fOwnBackBuffer = TRUE;
			}
		}

	//	Create transparency map, if necessary

	if (m_Mode.iTransDepth > 0)
		{
		ASSERT(m_Mode.iTransDepth == 8);
		ASSERT(m_pTrans == NULL);

		if (error = CreateChannel(m_Mode.cxWidth, m_Mode.cyHeight, &m_pTrans))
			{
			m_pPrimary->Release();
			m_pPrimary = NULL;
			return error;
			}
		}

	//	Create depth map, if necessary

	if (m_Mode.iZDepth > 0)
		{
		ASSERT(m_Mode.iZDepth == 8);
		ASSERT(m_pDepth == NULL);

		if (error = CreateChannel(m_Mode.cxWidth, m_Mode.cyHeight, &m_pDepth))
			{
			DestroyChannel(m_pTrans);
			m_pTrans = NULL;
			m_pPrimary->Release();
			m_pPrimary = NULL;
			return error;
			}
		}

	//	Let the worker thread know about the primary buffer

	g_Ctx.pPrimary = m_pPrimary;

	return NOERROR;
	}

ALERROR CGScreen::Flip (void)

//	Flip
//
//	Flips the back and primary buffers

	{
	ALERROR error;

	//	Update the frame count

	if (m_iStartTick == 0)
		{
		m_iStartTick = GetTickCount();
		m_iFrameCount = 0;
		}
	else
		{
		m_iFrameCount++;

#ifdef SHOW_FRAME_COUNT
		HDC hDC;
		char szBuffer[256];
		
		GetGDIDC(&hDC);
		SetTextColor(hDC, RGB(255, 255, 255));
		SetBkColor(hDC, RGB(0, 0, 0));
		wsprintf(szBuffer, "Frame Rate: %d", 1000 / ((GetTickCount() - m_iStartTick) / m_iFrameCount));
		TextOut(hDC, 0, 0, szBuffer, lstrlen(szBuffer));
		ReleaseGDIDC(hDC);
#endif
		}

	//	Flip

	if (m_pBackBuffer)
		{
		//	If we own our own back buffers then we need to swap buffers now

		if (m_fOwnBackBuffer)
			{
			LPDIRECTDRAWSURFACE7 pSwap = m_pBackBuffer;

			m_pBackBuffer = m_p3rdBuffer;
			m_p3rdBuffer = pSwap;

			//	Wait for the worker to be done, so that we don't
			//	clobber its data

			WaitForWorker();

			//	Let the worker flip the current buffer while we draw
			//	on the other one.

			g_Ctx.pBackBuffer = pSwap;

			if (error = SetWorkerTask(TASK_FLIP_OWN))
				return error;
			}

		//	Otherwise we just let DirectDraw flip our buffers

		else
			{
			if (error = SetWorkerTask(TASK_FLIP))
				return error;
			}

		//	Wait for flip to finish

#ifdef SYNC_FLIP
		WaitForWorker();
#endif
		}

	return NOERROR;
	}

ALERROR CGScreen::GetExclusiveModeList (CStructArray *pList)

//	GetExclusizeModeList
//
//	Appends CGModeDesc structures to the given list representing
//	available modes.

	{
	HRESULT result;

	ASSERT(m_lpdd);
	ASSERT(pList->GetElementSize() == sizeof(CGModeDesc));

	//	Enumerate all modes

	result = m_lpdd->EnumDisplayModes(0,
			NULL,
			pList,
			EnumModesCallback);
	if (result != DD_OK)
		return ERR_FAIL;

	//	Done

	return NOERROR;
	}

ALERROR CGScreen::GetGDIDC (HDC *rethDC)

//	GetGDIDC
//
//	Returns a DC that can be used to draw

	{
	HRESULT result;
	HDC hDC;
	LPDIRECTDRAWSURFACE7 pBuffer = GetDrawSurface();

	ASSERT(pBuffer);

	if (result = pBuffer->GetDC(&hDC))
		{
		kernelDebugLogMessage("Unable to get DC for surface: %x", result);
		return ERR_FAIL;
		}

	//	Done

	*rethDC = hDC;

	return NOERROR;
	}

ALERROR CGScreen::LockSurface (RECT *pRect, CGLockedSurface *pSurface)

//	LockSurface
//
//	Returns a raw DirectDraw surface that can be used to draw

	{
	ALERROR error;

	error = pSurface->Create(GetDrawSurface(), pRect, m_pTrans, m_pDepth);

#ifdef LOCK_POINTER
	kernelDebugLogMessage("DEBUG: CGScreen::LockSurface: pointer (%x)", pSurface->GetPixelPtr(0, 0));
#endif

	return error;
	}

void CGScreen::ReleaseGDIDC (HDC hDC)

//	ReleaseGDIDC
//
//	Releases a DC created by GetGDIDC

	{
	LPDIRECTDRAWSURFACE7 pBuffer = GetDrawSurface();

	ASSERT(pBuffer);
	pBuffer->ReleaseDC(hDC);
	}

ALERROR CGScreen::RestoreWindowedMode (void)

//	RestoreWindowedMode
//
//	Restores the screen to windowed mode

	{
	ASSERT(m_lpdd);

	if (m_fExclusive)
		{
		HDC hDC;

		ASSERT(m_hWnd);
		ASSERT(m_pPrimary == NULL);		//	We cannot yet handle switching after surfaces are created

		m_lpdd->RestoreDisplayMode();
		m_fExclusive = FALSE;

		//	Set mode

		GetClientRect(m_hWnd, &m_rcRect);

		hDC = GetDC(m_hWnd);
		m_Mode.iColorDepth = GetDeviceCaps(hDC, PLANES) * GetDeviceCaps(hDC, BITSPIXEL);
		m_Mode.cxWidth = RectWidth(&m_rcRect);
		m_Mode.cyHeight = RectHeight(&m_rcRect);
		m_Mode.dwFlags = 0;
		ReleaseDC(m_hWnd, hDC);
		}

	return NOERROR;
	}

ALERROR CGScreen::SetExclusiveMode (HWND hWnd, CGModeDesc *pMode)

//	SetExclusiveMode
//
//	Sets the screen to full-screen mode at the given resolution and
//	color depth. This function assumes that pMode was
//	set up by a call to GetExclusiveModeList

	{
	ALERROR error;
	HRESULT result;
	CGModeDesc ActualMode;
	CStructArray ModeList(sizeof(CGModeDesc), 10);
	int i;

	ASSERT(m_lpdd);
	ASSERT(m_pPrimary == NULL);		//	We cannot yet handle switching after surfaces are created
	ASSERT(m_hWnd == NULL);
	ASSERT(hWnd);

	//	Get a list of modes available

	if (error = GetExclusiveModeList(&ModeList))
		return error;

	//	Make sure that the requested mode is available

	ActualMode.cxWidth = 0;
	ActualMode.cyHeight = 0;
	for (i = 0; i < ModeList.GetCount(); i++)
		{
		CGModeDesc *pAvailMode = (CGModeDesc *)ModeList.GetStruct(i);

		if (pAvailMode->iColorDepth == pMode->iColorDepth
				&& pAvailMode->cxWidth >= pMode->cxWidth
				&& pAvailMode->cyHeight >= pMode->cyHeight)
			{
			if ((pAvailMode->cxWidth - pMode->cxWidth < ActualMode.cxWidth - pMode->cxWidth)
					|| (pAvailMode->cyHeight - pMode->cyHeight < ActualMode.cyHeight - pMode->cyHeight)
					|| (ActualMode.cxWidth == 0)
					|| (ActualMode.cyHeight == 0))
				ActualMode = *pAvailMode;
			}
		}

	//	If we could not find an appropriate mode, we're done

	if (ActualMode.cxWidth + ActualMode.cyHeight == 0)
		return ERR_FAIL;

	//	If we're not already in exclusive mode, go to it now

	if (!m_fExclusive)
		{
		result = m_lpdd->SetCooperativeLevel(hWnd,
				DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
		if (result != DD_OK)
			return ERR_FAIL;

		m_fExclusive = TRUE;
		}

	//	Now switch modes

	result = m_lpdd->SetDisplayMode(ActualMode.cxWidth,
			ActualMode.cyHeight,
			ActualMode.iColorDepth);
	if (result != DD_OK)
		return ERR_FAIL;

	//	Remember our mode

	m_Mode = ActualMode;
	m_Mode.iZDepth = pMode->iZDepth;
	m_Mode.iTransDepth = pMode->iTransDepth;
	m_Mode.iBackBuffers = pMode->iBackBuffers;
	m_Mode.dwFlags = pMode->dwFlags;
	m_hWnd = hWnd;

	m_rcRect.left = 0;
	m_rcRect.top = 0;
	m_rcRect.right = ActualMode.cxWidth;
	m_rcRect.bottom = ActualMode.cyHeight;

	//	Done

	return NOERROR;
	}

ALERROR CGScreen::SetWorkerTask (int iTask)

//	SetWorkerTask
//
//	Adds a task for the worker thread

	{
	WaitForWorker();

	//	Set the task

	g_Ctx.iTask = iTask;
	ResetEvent(g_Ctx.hWorkDone);
	SetEvent(g_Ctx.hWorkToDo);

	return NOERROR;
	}

void CGScreen::UnlockSurface (CGLockedSurface *pSurface)

//	UnlockSurface
//
//	Releases a surfaces created by LockSurface

	{
	pSurface->Destroy();
	}

ALERROR CGScreen::WaitForWorker (void)

//	WaitForWorker
//
//	Waits until the worker thread is done

	{
	DWORD dwWait;

	//	Wait until the thread is done with the
	//	previous task. (If we're not done in 5 seconds then
	//	something has gone horribly wrong and we fail.)

	dwWait = WaitForSingleObject(g_Ctx.hWorkDone, 5000);
	if (dwWait == WAIT_TIMEOUT)
		return ERR_FAIL;

	return NOERROR;
	}

HRESULT CALLBACK EnumModesCallback (LPDDSURFACEDESC pddsd, LPVOID pCtx)

//	EnumModesCallback
//
//	Called for each DirectDraw mode

	{
	CStructArray *pList = (CStructArray *)pCtx;
	CGModeDesc Mode;

	//	Create a structure

	utlMemSet(&Mode, sizeof(Mode), 0);
	Mode.cxWidth = pddsd->dwWidth;
	Mode.cyHeight = pddsd->dwHeight;
	Mode.iColorDepth = pddsd->ddpfPixelFormat.dwRGBBitCount;
	Mode.iZDepth = 8;
	Mode.iTransDepth = 8;
	Mode.iBackBuffers = 1;
	Mode.dwFlags = CGMODE_FLAG_EXCLUSIVE;

	//	Add the struct

	if (pList->AppendStruct(&Mode, NULL) != NOERROR)
		return DDENUMRET_CANCEL;

	return DDENUMRET_OK;
	}

HRESULT CALLBACK EnumSurfacesCallback (LPDIRECTDRAWSURFACE7 pSurface, LPDDSURFACEDESC pddsd, LPVOID pCtx)

//	EnumSurfacesCallback
//
//	Called for each attached surface

	{
	CIntArray *pList = (CIntArray *)pCtx;

	//	Add the surface to the list

	if (pList->AppendElement((int)pSurface, NULL) != NOERROR)
		return DDENUMRET_CANCEL;

	//	Print out data

#ifdef DEBUG
	kernelDebugLogMessage("Surface %d Caps: %x", pList->GetCount(), pddsd->ddsCaps.dwCaps);
	if (pddsd->ddsCaps.dwCaps & DDSCAPS_BACKBUFFER)
		kernelDebugLogMessage("   BACKBUFFER");
	if (pddsd->ddsCaps.dwCaps & DDSCAPS_FLIP)
		kernelDebugLogMessage("   FLIP");
	if (pddsd->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
		kernelDebugLogMessage("   SYSTEMMEMORY");
	if (pddsd->ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER)
		kernelDebugLogMessage("   FRONTBUFFER");
#endif

	return DDENUMRET_OK;
	}

DWORD WINAPI WorkerThread (LPVOID pvCtx)

//	WorkerThread
//
//	This thread will do work in the background

	{
	HRESULT result;
	WORKTHREADCTX *pCtx = (WORKTHREADCTX *)pvCtx;
	BOOL bQuit = FALSE;

	while (!bQuit)
		{
		HANDLE Waiting[4];
		DWORD dwWait;

		//	Wait for work to do (or for a signal to quit)

		Waiting[0] = pCtx->hQuit;
		Waiting[1] = pCtx->hWorkToDo;
		dwWait = WaitForMultipleObjects(2, Waiting, FALSE, INFINITE);
		if (dwWait == WAIT_OBJECT_0)
			bQuit = TRUE;
		else
			{
			//	Now do the task

			switch (pCtx->iTask)
				{
				case TASK_FLIP:
					{
					BOOL bFlipped = FALSE;
#ifdef FLIP_TIME
					int iTicks = GetTickCount();
#endif

					pCtx->Status = NOERROR;

					//	Keep looping until we successfully flip or
					//	until we get an error

					while (!bFlipped && (pCtx->Status == NOERROR))
						{
						result = pCtx->pPrimary->Flip(NULL, 0);
						if (result == DD_OK)
							bFlipped = TRUE;
						else if (result == DDERR_SURFACELOST)
							{
							result = pCtx->pPrimary->Restore();
							if (result != DD_OK)
								pCtx->Status = ERR_FAIL;
							}
						else if (result == DDERR_WASSTILLDRAWING)
							{
							//	If we need to wait, check to see
							//	if we have to quit

							dwWait = WaitForSingleObject(pCtx->hQuit, 5);
							if (dwWait != WAIT_TIMEOUT)
								pCtx->Status = ERR_FAIL;
							}
						else
							pCtx->Status = ERR_FAIL;
						}

#ifdef FLIP_TIME
					kernelDebugLogMessage("Flip took %d milliseconds", GetTickCount() - iTicks);
#endif
					break;
					}

				case TASK_FLIP_OWN:
					{
					RECT rcRect;
					CGModeDesc Mode;

					pCtx->Status = NOERROR;
					pCtx->pScreen->GetMode(&Mode);

					rcRect.left = 0;
					rcRect.top = 0;
					rcRect.right = Mode.cxWidth;
					rcRect.bottom = Mode.cyHeight;

					result = pCtx->pPrimary->BltFast(0, 0,
							pCtx->pBackBuffer,
							&rcRect,
							DDBLTFAST_WAIT);

					if (result != DD_OK)
						{
						kernelDebugLogMessage("Unable to blt back buffer to primary surface: %x", result);
						pCtx->Status = ERR_FAIL;
						}

					break;
					}

				default:
					ASSERT(FALSE);
				}

			//	Tell the main thread that we're done

			ResetEvent(pCtx->hWorkToDo);
			SetEvent(pCtx->hWorkDone);
			}
		}

	return 0;
	}

#endif