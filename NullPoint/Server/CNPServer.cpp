//	CNPServer.cpp
//
//	Implements CNPServer object

#include "Alchemy.h"
#include "NetUtil.h"
#include "Service.h"
#include "NPEngine.h"
#include "NPRPC.h"

static CNPServer *g_pServer = NULL;

void Run (void);

//	Entry-points for NT Service

void ServiceStart (DWORD dwArgc, LPTSTR *lpszArgv)

//	ServiceStart
//
//	This is called to start the service

	{
	ALERROR error;

	//	Report

    if (!ReportStatusToSCMgr(SERVICE_START_PENDING,	NO_ERROR, 3000))
        return;

	//	Initialize kernel

	if (!kernelInit())
		{
        AddToMessageLog(TEXT("Unable to initialize Alchemy kernel"));
		return;
		}

	//	Initialize network subsystem

	if (error = netBoot())
		{
		AddToMessageLog(TEXT("Unable to boot NetUtil"));
		kernelCleanUp();
		return;
		}

	//	Do it

	Run();

	//	Clean up

	netShutdown();
	kernelCleanUp();
	}

void ServiceStop (void)

//	ServiceStop
//
//	Stops the service

	{
	if (g_pServer)
		g_pServer->Stop();
	}

void Run (void)

//	Run
//
//	Runs the service until we're done

	{
	ALERROR error;
	CNPServer Server;
	CTextFileLog DebugLog(LITERAL("NPLog.txt"));

	//	Begin loggin

	kernelSetDebugLog(&DebugLog, TRUE);

	//	Start the server

	if (error = Server.Start(NULL))
		{
		AddToMessageLog(TEXT("Unable to boot NullPoint server"));
		return;
		}

	//	Tell the controller that we're cool

    if (!ReportStatusToSCMgr(SERVICE_RUNNING, NO_ERROR, 0))
        return;

	//	Assign to a global variable so that we can stop it

	g_pServer = &Server;

	//	Run until we quit

	Server.Run();

	//	Restore

	g_pServer = NULL;
	kernelSetDebugLog(NULL, FALSE);
	}
