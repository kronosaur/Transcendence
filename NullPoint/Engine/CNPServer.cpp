//	CNPServer.cpp
//
//	Implements CNPServer class

#include "Alchemy.h"
#include "NPEngine.h"
#include "NPRPC.h"

#include <stdio.h>

const int GAME_UPDATE_DELAY	=				1000;	//	In milliseconds

CNPServer::CNPServer (void) : m_pUniv(NULL),
		m_pHandler(NULL),
		m_iState(ssNotStarted),
		m_bLog(true),
		m_bOpenAccess(false)
	{
	}

void CNPServer::Run (void)

//	Run
//
//	Continues until Stop is called on some other thread

	{
	Log("Listening for connection requests.");

	while (true)
		{
		HANDLE hEvents[1];
		hEvents[0] = m_hQuitEvent;

		DWORD dwWait = ::WaitForMultipleObjects(1, hEvents, FALSE, GAME_UPDATE_DELAY);
		if (dwWait == WAIT_OBJECT_0)
			break;

		//	If a game is in progress, update it

		Lock();

		if (m_pUniv)
			{
			if (m_pUniv->Update())
				Log("Universe updated: %s", m_pUniv->GetStardate().GetASCIIZPointer());
			}

		Unlock();
		}

	//	We can clean up now

	Log("Server shutdown complete.");
	m_Server.Shutdown();

	::CloseHandle(m_hQuitEvent);
	::DeleteCriticalSection(&m_csServer);
	}

ALERROR CNPServer::Start (INPServerEvents *pHandler)

//	Start
//
//	Starts the server. Caller must call Run

	{
	ALERROR error;

	ASSERT(m_iState == ssNotStarted);

	InitializeCriticalSection(&m_csServer);
	m_iState = ssStart;
	m_pHandler = pHandler;

	//	Create some events

	m_hQuitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	//	Initialize listener, etc.

	if (error = m_Server.Boot(SERVER_PORT, SERVER_PROTOCOL, this))
		{
		DeleteCriticalSection(&m_csServer);
		return error;
		}

	Log("NullPoint Server v%d.%d", SERVER_MAJOR_VERSION, SERVER_MINOR_VERSION);
	Log("(c) Copyright 1999 by NeuroHack, Inc. All Rights Reserved.");

	return NOERROR;
	}

void CNPServer::Stop (void)

//	Stop
//
//	Stops the server

	{
	if (m_iState != ssNotStarted)
		::SetEvent(m_hQuitEvent);
	}

CString CNPServer::OnClientConnect (DWORD dwClientID)

//	OnClientConnect

	{
	Log("[%d] Connected.", dwClientID);

	//	Allocate context for this client

	ClientCtx *pCtx = new ClientCtx;
	pCtx->sName = LITERAL("(Unknown)");
	pCtx->dwClientID = dwClientID;
	pCtx->iRPCVersion = 0;
	pCtx->ACL = aclNoLogin;
	pCtx->pSovereign = NULL;

	m_Server.SetClientData(dwClientID, pCtx);

	return CString("");
	}

void CNPServer::OnClientError (DWORD dwClientID, NetErrors Error, const CString &sErrorString)

//	OnClientError

	{
	}

void CNPServer::OnError (NetErrors Error, const CString &sErrorString)

//	OnError

	{
	}

CString CNPServer::OnDataReceived (DWORD dwClientID, const CString &sData, LPVOID pCtx)

//	OnDataReceived

	{
	ALERROR error;
	CString sReply;
	ClientCtx *pClient = (ClientCtx *)pCtx;

	//	Parse data

	CBufferReadBlock Block(sData);
	CDataPackStruct *pData;
	if (error = CDataPackStruct::CreateFromBuffer(&Block, &pData))
		{
		Log("[%d] ERROR: Error creating CDataPackStruct from buffer", dwClientID);

		CDataPackStruct Msg;
		Msg.AppendInteger(RES_CMD_ERROR);
		return Msg.SerializeToString();
		}

	//	The first integer is the command code

	int iCmdCode = pData->GetIntegerItem(0);

	//	Most commands do not work if the universe has not been created yet

	switch (iCmdCode)
		{
		case CMD_CREATE_UNIVERSE:
		case CMD_REQUEST_CONNECT:
			break;

		default:
			{
			if (m_pUniv == NULL)
				{
				sReply = ComposeCmdFail(pClient, iCmdCode,
						LITERAL("The Universe has not yet been created."));
				delete pData;
				return sReply;
				}
			}
		}

	//	Dispatch

	switch (iCmdCode)
		{
		case CMD_CREATE_UNIVERSE:
			sReply = CmdCreateUniverse(pClient, *pData);
			break;

		case CMD_FLEET_DEST:
			sReply = CmdFleetDest(pClient, *pData);
			break;

		case CMD_LOGIN:
			sReply = CmdLogin(pClient, *pData);
			break;

		case CMD_NEW_PLAYER:
			sReply = CmdNewPlayer(pClient, *pData);
			break;

		case CMD_REQUEST_CONNECT:
			sReply = CmdRequestConnect(pClient, *pData);
			break;

		case CMD_VIEW:
			sReply = CmdView(pClient, *pData);
			break;

		default:
			{
			Log("[%d] ERROR: Invalid command from client (%d)", dwClientID, pData->GetIntegerItem(0));

			CDataPackStruct Msg;
			Msg.AppendInteger(RES_CMD_ERROR);
			sReply = Msg.SerializeToString();
			}
		}

	//	Done

	delete pData;

	return sReply;
	}

void CNPServer::OnClientDisconnect (DWORD dwClientID, LPVOID pCtx)

//	OnClientDisconnect

	{
	Log("[%d] Disconnect.", dwClientID);

	//	Free context

	ClientCtx *pClient = (ClientCtx *)pCtx;
	delete pClient;
	}

void CNPServer::Log (char *pszLine, ...)

//	Log
//
//	Logs a message to the debug log

	{
	if (m_bLog)
		{
		char *pArgs;
		char szBuffer[1024];

		pArgs = (char *)&pszLine + sizeof(pszLine);
		wvsprintf(szBuffer, pszLine, pArgs);

		kernelDebugLogMessage(szBuffer);
#ifdef _DEBUG
		printf("%s\n", szBuffer);
#endif
		}
	}

bool CNPServer::ValidAccess (ClientCtx *pClient, DWORD dwUNID)

//	ValidAccess
//
//	Returns TRUE if the client is allowed to see info for
//	the given object

	{
	switch (pClient->ACL)
		{
		case aclOverlord:
			return true;

		case aclPlayer:
			return pClient->pSovereign->KnowsAbout(dwUNID);

		default:
			return false;
		}
	}

bool CNPServer::ValidFleetAccess (ClientCtx *pClient, DWORD dwFleet, bool bOrders, CNPFleet **retpFleet, CString *retsError)

//	ValidFleetAccess
//
//	Return TRUE if the client is allowed to access the given fleet
//	If FALSE, then retsError is the error return.

	{
	//	Is this a valid fleet?

	CNPFleet *pFleet = m_pUniv->GetFleet(dwFleet);
	if (pFleet == NULL)
		{
		*retsError = ComposeCmdFail(pClient, CMD_VIEW, CONSTLIT("Unknown object."));
		return false;
		}

	//	Do we know about this fleet?

	if (!ValidAccess(pClient, dwFleet))
		{
		*retsError = ComposeCmdFail(pClient, CMD_VIEW, CONSTLIT("Unknown object."));
		return false;
		}

	//	If we're giving orders, Does the client own this fleet?

	if (bOrders)
		{
		if (pClient->pSovereign != pFleet->GetSovereign()
				&& pClient->ACL != aclOverlord)
			{
			*retsError = ComposeCmdFail(pClient, CMD_VIEW, CONSTLIT("Fleet rejects orders."));
			return false;
			}
		}

	//	Done

	*retpFleet = pFleet;

	return true;
	}

bool CNPServer::ValidNullPointAccess (ClientCtx *pClient, DWORD dwNullPoint, bool bAdjacent, CNPNullPoint **retpNP, CString *retsError)

//	ValidNullPointAccess
//
//	Return TRUE if the client is allowed to see this null point

	{
	//	Is this a valid null point?

	CNPNullPoint *pNP = m_pUniv->GetNullPoint(dwNullPoint);
	if (pNP == NULL)
		{
		*retsError = ComposeCmdFail(pClient, CMD_VIEW, CONSTLIT("Unknown object."));
		return false;
		}

	//	Do we know about this null point?

	if (!ValidAccess(pClient, dwNullPoint))
		{
		//	If we're looking for adjacent null points, see if this
		//	is adjacent to any null point that we do know about

		if (bAdjacent)
			{
			int i;
			for (i = 0; i < pNP->GetLinkCount(); i++)
				{
				CNPNullPoint *pDest = pNP->GetLinkDest(i);
				if (ValidAccess(pClient, pDest->GetUNID()))
					break;
				}

			if (i == pNP->GetLinkCount())
				{
				*retsError = ComposeCmdFail(pClient, CMD_VIEW, CONSTLIT("Unknown object."));
				return false;
				}
			}
		else
			{
			*retsError = ComposeCmdFail(pClient, CMD_VIEW, CONSTLIT("Unknown object."));
			return false;
			}
		}

	//	Done

	*retpNP = pNP;

	return true;
	}
