//	CNetServer.cpp
//
//	Implementation of CNetServer class
//
//	Parts based on iMatix SFL <http://www.imatix.com>

#include "Alchemy.h"
#include "NetUtil.h"

#define INVALID_CLIENT			0xFFFFFFFF

CNetServer::CNetServer (void) : m_pNotify(NULL),
		m_Clients(sizeof(ClientStruct), 10)

//	CNetServer constructor

	{
	}

CNetServer::~CNetServer (void)

//	CNetServer destructor

	{
	Shutdown();
	}

ALERROR CNetServer::Boot (const CString &sService, const CString &sProtocol, INetServerEvents *pNotify)

//	Boot
//
//	Initializes the server

	{
	ASSERT(m_pNotify == NULL);

	//	Remember our connection info

	m_pNotify = pNotify;
	m_sService = sService;
	m_sProtocol = sProtocol;

	//	Create some events

	m_hQuit = CreateEvent(NULL, TRUE, FALSE, NULL);
	InitializeCriticalSection(&m_csClients);

	//	Start the listener thread

	m_hListener = kernelCreateThread(CNetServer::ListenerThreadThunk, this);
	if (m_hListener == NULL)
		{
		m_pNotify = NULL;
		return ERR_FAIL;
		}

	return NOERROR;
	}

DWORD WINAPI CNetServer::ClientThreadThunk (LPVOID pvClient)

//	ClientThreadThunk
//
//	Set up this pointer

	{
	ClientStruct *pClient = (ClientStruct *)pvClient;
	return pClient->pServer->ClientThread(pClient);
	}

DWORD CNetServer::ClientThread (ClientStruct *pClient)

//	ClientThread
//
//	Handles the given client

	{
	bool bQuit = false;

	//	Prepare the read buffers

	CString sReadBuffer;
	DWORD dwBytesRead;

	OVERLAPPED oRead;
	HANDLE hReadDone = CreateEvent(NULL, TRUE, FALSE, NULL);
	oRead.hEvent = hReadDone;

	//	Prepare the quit event

	pClient->hQuit = CreateEvent(NULL, TRUE, FALSE, NULL);

	//	Prepare a set of events to wait on

	HANDLE hEvents[2];
	hEvents[0] = pClient->hQuit;
	hEvents[1] = hReadDone;

	//	Tell our user that a client has connected. If there
	//	is a reply, we send it. Note that it is legal for the
	//	user to call DropClient inside of OnClientConnect.

	CString sReply = m_pNotify->OnClientConnect(pClient->dwID);

	//	Send (synchronous) reply to client. NOTE: We must
	//	use overlapped I/O; otherwise we get error 57.
	//	Note also that we reuse hReadDone because we're
	//	not going to be reading simultaneously.

	if (!sReply.IsBlank())
		{
		DWORD dwBytesWritten;
		ResetEvent(hReadDone);
		if (!WriteFile((HANDLE)pClient->Socket,
				sReply.GetPointer(),
				sReply.GetLength(),
				&dwBytesWritten,
				&oRead)
				&& GetLastError() != ERROR_IO_PENDING)
			{
			DWORD dwError = GetLastError();
			m_pNotify->OnClientError(pClient->dwID, ConnectionLost, strFromInt(dwError, FALSE));
			bQuit = true;
			}

		GetOverlappedResult((HANDLE)pClient->Socket,
				&oRead,
				&dwBytesWritten,
				TRUE);
		}

	//	Wait for some message from the client

	while (!bQuit)
		{
		ResetEvent(hReadDone);
		if (!ReadFile((HANDLE)pClient->Socket,
				sReadBuffer.GetWritePointer(4096),
				4096,
				&dwBytesRead,
				&oRead)
				&& GetLastError() != ERROR_IO_PENDING)
			{
			m_pNotify->OnClientError(pClient->dwID, ConnectionLost, LITERAL(""));
			break;
			}

		//	Wait for something to happen

		DWORD dwWait = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

		if (dwWait == WAIT_OBJECT_0)
			{
			CancelIo((HANDLE)pClient->Socket);
			break;
			}
		else if (dwWait == WAIT_OBJECT_0 + 1)
			{
			GetOverlappedResult((HANDLE)pClient->Socket,
					&oRead,
					&dwBytesRead,
					TRUE);
			sReadBuffer.Truncate(dwBytesRead);

			//	If we read 0 bytes then it is because the client dropped the
			//	connection

			if (dwBytesRead == 0)
				{
				m_pNotify->OnClientError(pClient->dwID, ConnectionLost, LITERAL(""));
				break;
				}

			//	Otherwise, process the client's message

			else
				{
				CString sReply = m_pNotify->OnDataReceived(pClient->dwID, sReadBuffer, pClient->pData);

				//	Send (synchronous) reply to client

				if (!sReply.IsBlank())
					{
					DWORD dwBytesWritten;
					ResetEvent(hReadDone);
					if (!WriteFile((HANDLE)pClient->Socket,
							sReply.GetPointer(),
							sReply.GetLength(),
							&dwBytesWritten,
							&oRead)
							&& GetLastError() != ERROR_IO_PENDING)
						{
						m_pNotify->OnClientError(pClient->dwID, ConnectionLost, LITERAL(""));
						break;
						}

					GetOverlappedResult((HANDLE)pClient->Socket,
							&oRead,
							&dwBytesWritten,
							TRUE);
					}
				}
			}
		}

	m_pNotify->OnClientDisconnect(pClient->dwID, pClient->pData);

	//	Clean up

	CloseHandle(hReadDone);
	CloseHandle(pClient->hQuit);
	closesocket(pClient->Socket);
	
	EnterCriticalSection(&m_csClients);
	pClient->dwID = INVALID_CLIENT;
	LeaveCriticalSection(&m_csClients);

	return 0;
	}

void CNetServer::DropClient (DWORD dwClientID)

//	DropClient
//
//	Drops the client of the given ID

	{
	EnterCriticalSection(&m_csClients);

	for (int i = 0; i < m_Clients.GetCount(); i++)
		{
		ClientStruct *pClient = (ClientStruct *)m_Clients.GetStruct(i);

		if (pClient->dwID == dwClientID)
			SetEvent(pClient->hQuit);
		}

	LeaveCriticalSection(&m_csClients);
	}

DWORD CNetServer::ListenerThread (void)

//	ClientThread
//
//	Thread to handle all asynchronous communications

	{
	SOCKADDR_IN name;

	//	Create a socket

	int SockType;
	if (strCompare(m_sProtocol, LITERAL("udp")) == 0)
		SockType = SOCK_DGRAM;
	else
		SockType = SOCK_STREAM;

	m_ListenerSocket = socket(AF_INET, SockType, 0);
	if (m_ListenerSocket == INVALID_SOCKET)
		{
		return 1;
		}

	//	Bind to a port

	ZeroMemory(&name, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons((WORD)strToInt(m_sService, 0, NULL));
	name.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(m_ListenerSocket, (SOCKADDR *)&name, sizeof(name)) == SOCKET_ERROR)
		{
		return 1;
		}

	//	Start listening

	if (listen(m_ListenerSocket, SOMAXCONN) == SOCKET_ERROR)
		{
		return 1;
		}

	//	Keep handling clients

	while (true)
		{
		SOCKET NewSocket;

		if ((NewSocket = accept(m_ListenerSocket, NULL, NULL)) == INVALID_SOCKET)
			{
			return 1;
			}

		//	Find an empty slot for the new client
		//	and add it to the list.

		EnterCriticalSection(&m_csClients);

		int iIndex;
		for (iIndex = 0; iIndex < m_Clients.GetCount(); iIndex++)
			if (((ClientStruct *)m_Clients.GetStruct(iIndex))->dwID == INVALID_CLIENT)
				break;

		if (iIndex == m_Clients.GetCount())
			m_Clients.AppendStruct(NULL, &iIndex);

		ClientStruct *pNewClient = (ClientStruct *)m_Clients.GetStruct(iIndex);
		pNewClient->pServer = this;
		pNewClient->dwID = (DWORD)iIndex;
		pNewClient->hThread = NULL;
		pNewClient->Socket = NewSocket;
		pNewClient->pData = NULL;

		LeaveCriticalSection(&m_csClients);

		//	Start a new thread to handle this client

		pNewClient->hThread = kernelCreateThread(CNetServer::ClientThreadThunk, pNewClient);
		if (pNewClient->hThread == NULL)
			{
			return 1;
			}
		}

	return 0;
	}

DWORD WINAPI CNetServer::ListenerThreadThunk (LPVOID pThis)

//	ListenerThreadThunk
//
//	Sets up this pointer

	{
	return ((CNetServer *)pThis)->ListenerThread();
	}

void CNetServer::SetClientData (DWORD dwClientID, LPVOID pData)

//	SetClientData
//
//	Associates data with the client ID

	{
	EnterCriticalSection(&m_csClients);

	for (int i = 0; i < m_Clients.GetCount(); i++)
		{
		ClientStruct *pClient = (ClientStruct *)m_Clients.GetStruct(i);

		if (pClient->dwID == dwClientID)
			pClient->pData = pData;
		}

	LeaveCriticalSection(&m_csClients);
	}

void CNetServer::Shutdown (void)

//	Shutdown
//
//	Shuts down the server

	{
	if (m_pNotify)
		{
		m_pNotify = NULL;

		DeleteCriticalSection(&m_csClients);
		CloseHandle(m_hListener);
		CloseHandle(m_hQuit);
		}
	}

