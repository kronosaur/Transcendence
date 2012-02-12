//	Elements.cpp
//
//	Initialization and Clean Up
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"

#include <process.h>

long g_iGlobalInit = 0;
DWORD g_dwAPIFlags = 0;
CRITICAL_SECTION g_csKernel;

struct THREADCTX
	{
	LPTHREAD_START_ROUTINE pfStart;
	LPVOID pData;
	};

//	Forwards

void InitAPIFlags (void);
DWORD WINAPI elementsThreadProc (LPVOID pData);

bool elementsInit (void)

//	elementsInit
//
//	Must be called before any other routines.
//	elementsInit must be called for each thread that
//	uses the library
//
//	Note that there's no concurrency control between
//	different calls to elementsInit. The caller must
//	guarantee that only one thread gets initialized
//	at a time.

	{
	//	Since this routine must be called at the
	//	beginning for each thread, but globals are shared
	//	between threads, we check to make sure that
	//	we haven't already initialized them

	if (InterlockedIncrement(&g_iGlobalInit) == 1)
		{
		//	Initialize API flags

		InitAPIFlags();

		//	Initialize the critical section

		InitializeCriticalSection(&g_csKernel);
		}

	//	Initialize random number generator. This is
	//	done for each thread because we link with the multi-threaded
	//	runtime library, which keeps separate context for
	//	each thread.

	srand((int)GetTickCount());

	return true;
	}

void elementsCleanUp (void)

//	elementsCleanUp
//
//	Must be called for each thread to clean up

	{
	if (InterlockedDecrement(&g_iGlobalInit) == 0)
		{
		//	Clean up critical section

		DeleteCriticalSection(&g_csKernel);
		}

	ASSERT(g_iGlobalInit >= 0);
	}

DWORD sysGetAPIFlags (void)

//	sysGetAPIFlags
//
//	Returns API flags

	{
	return g_dwAPIFlags;
	}

void InitAPIFlags (void)

//	InitAPIFlags
//
//	Initialize API flags

	{
	//	Check to see if we're running on Windows NT

	OSVERSIONINFO osvi;

	osvi.dwOSVersionInfoSize = sizeof(osvi);
	if (::GetVersionEx(&osvi))
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
			g_dwAPIFlags |= API_FLAG_WINNT;

	//	Check to see if MaskBlt is available

	HDC hSourceDC, hDestDC;
	HBITMAP hDestBmp, hSourceBmp, hMaskBmp;

	hSourceDC = ::CreateCompatibleDC(NULL);
	hDestDC = ::CreateCompatibleDC(NULL);
	hDestBmp = ::CreateCompatibleBitmap(hSourceDC, 10, 10);
	hSourceBmp = ::CreateCompatibleBitmap(hSourceDC, 10, 10);
	hMaskBmp = ::CreateCompatibleBitmap(hSourceDC, 10, 10);

	::SelectObject(hSourceDC, hSourceBmp);
	::SelectObject(hDestDC, hDestBmp);
	if (::MaskBlt(hDestDC, 0, 0, 10, 10, hSourceDC, 0, 0, hMaskBmp, 0, 0, 0xCCAA0000L))
		g_dwAPIFlags |= API_FLAG_MASKBLT;
    
	::DeleteDC(hSourceDC);
	::DeleteDC(hDestDC);
	::DeleteObject(hDestBmp);
	::DeleteObject(hSourceBmp);
	::DeleteObject(hMaskBmp);
	}

HANDLE sysCreateThread (LPTHREAD_START_ROUTINE pfStart, LPVOID pData)

//	sysCreateThread
//
//	Creates a new thread

	{
	HANDLE hThread;
	DWORD dwThreadID;
	THREADCTX *pCtx;

	//	Allocate a context block to pass to the thread

	pCtx = new THREADCTX;
	pCtx->pfStart = pfStart;
	pCtx->pData = pData;

	//	Run the new thread

	hThread = (HANDLE)_beginthreadex(NULL,
			0,
			(unsigned int (__stdcall *)(void *))elementsThreadProc,
			pCtx,
			0,
			(unsigned int *)&dwThreadID);

	return hThread;
	}

DWORD WINAPI elementsThreadProc (LPVOID pData)
	{
	THREADCTX *pCtx = (THREADCTX *)pData;

	LPTHREAD_START_ROUTINE pfStart = pCtx->pfStart;
	LPVOID pUserData = pCtx->pData;
	delete pCtx;

	elementsInit();
	DWORD dwResult = pfStart(pUserData);
	elementsCleanUp();

	return dwResult;
	};

CString CException::GetMessage (void)
	{
	CString sMessage;

	switch (GetCode())
		{
		case errOutOfMemory:
			sMessage = CONSTLIT("Out of memory");
			break;

		case errOutOfDiskspace:
			sMessage = CONSTLIT("Out of diskspace");
			break;

		case errUnableToOpenFile:
			sMessage = strPatternSubst(CONSTLIT("Unable to open file: \"%s\""), GetErrorInfo());
			break;

		case errUnableToSaveFile:
			sMessage = strPatternSubst(CONSTLIT("Unable to save file: \"%s\""), GetErrorInfo());
			break;

		case errPathInvalid:
			sMessage = strPatternSubst(CONSTLIT("Invalid path: \"%s\""), GetErrorInfo());
			break;

		default:
			sMessage = GetErrorInfo();
			break;
		}

	return sMessage;
	}