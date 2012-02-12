//	NetUtil.cpp
//
//	Global functions

#include "Alchemy.h"
#include "NetUtil.h"

static long g_iInitialized = 0;

ALERROR netBoot (void)

//	NetBoot
//
//	Must be called before all other functions

	{
	//	Note that this is not entirely thread-safe.
	//	If two threads try to initialize at the same time, one
	//	of them will return from NetBoot before the other is
	//	done initializing.
	//
	//	For now this call should only be made by the main
	//	thread.

	if (InterlockedIncrement(&g_iInitialized) == 1)
		{
	    WORD wVersionRequested;
		WSADATA wsaData;

		wVersionRequested = 0x0101;		//	Version 1.1
		if (WSAStartup (wVersionRequested, &wsaData))
			return ERR_FAIL;
		}

	return NOERROR;
	}

void netShutdown (void)

//	NetShutdown
//
//	Must be called when done

	{
	if (InterlockedDecrement(&g_iInitialized) == 0)
	    WSACleanup ();
	}
