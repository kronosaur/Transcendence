//	Kernel.cpp
//
//	Kernel boot

#include "Kernel.h"
#include "KernelObjID.h"

#include <process.h>
#include "eh.h"

long g_iGlobalInit = 0;
DWORD g_dwAPIFlags = 0;
CRITICAL_SECTION g_csKernel;
CTextFileLog *g_pDebugLog = NULL;
bool g_bFreeDebugLog = false;
DWORD g_dwKernelFlags = 0;

struct THREADCTX
	{
	LPTHREAD_START_ROUTINE pfStart;
	LPVOID pData;
	};

//	Forwards

void InitAPIFlags (void);
DWORD WINAPI kernelThreadProc (LPVOID pData);
void kernelHandleWin32Exception (unsigned code, EXCEPTION_POINTERS* info);

BOOL kernelInit (DWORD dwFlags)

//	kernelInit
//
//	Must be called before any other routines.
//	KernelInit must be called for each thread that
//	uses the library
//
//	Note that there's no concurrency control between
//	different calls to KernelInit. The caller must
//	guarantee that only one thread gets initialized
//	at a time.

	{
	//	Since this routine must be called at the
	//	beginning for each thread, but globals are shared
	//	between threads, we check to make sure that
	//	we haven't already initialized them

	if (InterlockedIncrement(&g_iGlobalInit) == 1)
		{
		g_dwKernelFlags = dwFlags;

		//	Initialize the strings subsystem

		if (CString::INTStringInit() != NOERROR)
			return FALSE;

		//	Initialize API flags

		InitAPIFlags();

		//	Initialize the critical section

		InitializeCriticalSection(&g_csKernel);

		//	If necessary, initialize winsock system

		if (g_dwKernelFlags & KERNEL_FLAG_INTERNETS)
			{
			WSADATA wsaData;
			::WSAStartup(MAKEWORD(1,1), &wsaData);
			}
		}

	//	Install a Win32 exception handler

	_set_se_translator(kernelHandleWin32Exception);

	//	Initialize random number generator. This is
	//	done for each thread because we link with the multi-threaded
	//	runtime library, which keeps separate context for
	//	each thread.

	srand((int)GetTickCount());

	return TRUE;
	}

void kernelCleanUp (void)

//	KernelCleanUp
//
//	Must be called for each thread to clean up

	{
	if (InterlockedDecrement(&g_iGlobalInit) == 0)
		{
		if (g_dwKernelFlags & KERNEL_FLAG_INTERNETS)
			::WSACleanup();

		//	Clean up strings

		CString::INTStringCleanUp();

		//	Done logging

		kernelSetDebugLog(NULL, FALSE);

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
	if (GetVersionEx(&osvi))
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
			g_dwAPIFlags |= API_FLAG_WINNT;

	//	DWM runs on Vista and above

	if (osvi.dwMajorVersion >= 6)
		g_dwAPIFlags |= API_FLAG_DWM;

	//	Check to see if MaskBlt is available

	HDC hSourceDC, hDestDC;
	HBITMAP hDestBmp, hSourceBmp, hMaskBmp;

	hSourceDC = CreateCompatibleDC(NULL);
	hDestDC = CreateCompatibleDC(NULL);
	hDestBmp = CreateCompatibleBitmap(hSourceDC, 10, 10);
	hSourceBmp = CreateCompatibleBitmap(hSourceDC, 10, 10);
	hMaskBmp = CreateCompatibleBitmap(hSourceDC, 10, 10);

	SelectObject(hSourceDC, hSourceBmp);
	SelectObject(hDestDC, hDestBmp);
	if (MaskBlt(hDestDC, 0, 0, 10, 10, hSourceDC, 0, 0, hMaskBmp, 0, 0, 0xCCAA0000L))
		g_dwAPIFlags |= API_FLAG_MASKBLT;
    
	DeleteDC(hSourceDC);
	DeleteDC(hDestDC);
	DeleteObject(hDestBmp);
	DeleteObject(hSourceBmp);
	DeleteObject(hMaskBmp);
	}

void kernelClearDebugLog (void)

//	kernerClearDebugLog
//
//	Stops the debug log

	{
	kernelSetDebugLog(NULL);
	}

CString kernelGetSessionDebugLog (void)

//	kernetGetSessionDebugLog
//
//	Returns all of the logged entries since the start of the session.

	{
	if (g_pDebugLog == NULL)
		return NULL_STR;

	return g_pDebugLog->GetSessionLog();
	}

ALERROR kernelSetDebugLog (const CString &sFilespec, bool bAppend)

//	kernelSetDebugLog
//
//	Sets the debug log file

	{
	CTextFileLog *pLog = new CTextFileLog(sFilespec);
	return kernelSetDebugLog(pLog, bAppend, true);
	}

ALERROR kernelSetDebugLog (CTextFileLog *pLog, bool bAppend, bool bFreeLog)

//	kernelSetDebugLog
//
//	Sets the debug log file

	{
	ALERROR error;

	EnterCriticalSection(&g_csKernel);

	//	Close previous log

	if (g_pDebugLog)
		{
		g_pDebugLog->LogOutput(ILOG_FLAG_TIMEDATE, "End logging session");
		g_pDebugLog->Close();
		if (g_bFreeDebugLog)
			{
			delete g_pDebugLog;
			g_bFreeDebugLog = false;
			}
		}

	g_pDebugLog = pLog;
	g_bFreeDebugLog = bFreeLog;

	//	Start new one

	if (pLog)
		{
		if (error = pLog->Create(bAppend))
			{
			LeaveCriticalSection(&g_csKernel);
			return error;
			}

		//	Output start of logging

		pLog->LogOutput(ILOG_FLAG_TIMEDATE, "--------------------------------------------------------------------------------");
		pLog->LogOutput(ILOG_FLAG_TIMEDATE, "Start logging session");

		//	Skip the above marker when getting the session content

		pLog->SetSessionStart();
		}

	LeaveCriticalSection(&g_csKernel);

	return NOERROR;
	}

void kernelDebugLogMessage (char *pszLine, ...)

//	kernelDebugLogMessage
//
//	Log debug output

	{
	EnterCriticalSection(&g_csKernel);

	if (g_pDebugLog)
		{
		CString sParsedLine;

		char *pArgs = (char *)&pszLine + sizeof(pszLine);
		sParsedLine = strPattern(CString(pszLine, ::strlen(pszLine), TRUE), (void **)pArgs);

		g_pDebugLog->LogOutput(ILOG_FLAG_TIMEDATE, sParsedLine);
		}

	LeaveCriticalSection(&g_csKernel);
	}

HANDLE kernelCreateThread (LPTHREAD_START_ROUTINE pfStart, LPVOID pData)

//	kernelCreateThread
//
//	Creates a new thread

	{
	HANDLE hThread;
	DWORD dwThreadID;
	THREADCTX *pCtx;

	//	Allocate a context block to pass to the thread

	pCtx = (THREADCTX *)MemAlloc(sizeof(THREADCTX));
	if (pCtx == NULL)
		return NULL;

	pCtx->pfStart = pfStart;
	pCtx->pData = pData;

	//	Run the new thread

	hThread = (HANDLE)_beginthreadex(NULL,
			0,
			(unsigned int (__stdcall *)(void *))kernelThreadProc,
			pCtx,
			0,
			(unsigned int *)&dwThreadID);

	return hThread;
	}

bool kernelDispatchUntilEventSet (HANDLE hEvent, DWORD dwTimeout)

//	kernelDispatchUntilEventSet
//
//	Waits on the given event while dispatching messages. We return TRUE if the 
//	event is set (FALSE if we hit the timeout).

	{
	//	If the event is already set, we're done--no need to wait.

	if (::WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
		return true;

	//	Keep looping until we're done

	while (true)
		{
		//	Dispatch messages until there are no more messages or until our
		//	event is set.

		MSG msg;
		while (::PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
			{
			//	If this is a PM_QUIT, then we need to repost it and exit

			if (msg.message == WM_QUIT)
				{
				::PostQuitMessage((int)msg.wParam);
				return false; // abandoned due to WM_QUIT
				}

			//	Otherwise, dispatch

			::TranslateMessage(&msg); 
			::DispatchMessage(&msg); 

			//	If we're signalled now, we're done.

			if (::WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
				return true;
			}

		//	Wait for more messages or until our event is set.

		DWORD dwResult = ::MsgWaitForMultipleObjects(1, &hEvent, FALSE, dwTimeout, QS_ALLINPUT); 
		switch (dwResult)
			{
			//	Event was signalled

			case WAIT_OBJECT_0:
				return true;

			//	Timeout

			case WAIT_TIMEOUT:
				return false;

			//	Otherwise, more messages in the queue

			default:
				if (dwResult == WAIT_OBJECT_0 + 1)
					continue;
				else
					return false;
			}
		}
	}

DWORD WINAPI kernelThreadProc (LPVOID pData)
	{
	THREADCTX *pCtx = (THREADCTX *)pData;

	LPTHREAD_START_ROUTINE pfStart = pCtx->pfStart;
	LPVOID pUserData = pCtx->pData;
	MemFree(pCtx);

	kernelInit();
	DWORD dwResult = 0;
	dwResult = pfStart(pUserData);
	kernelCleanUp();

	return dwResult;
	};

DWORD sysGetVersion (void)
	{ return 0x00060002; }

void kernelHandleWin32Exception (unsigned code, EXCEPTION_POINTERS* info)
	{
	throw CException(ERR_WIN32_EXCEPTION);
	}
