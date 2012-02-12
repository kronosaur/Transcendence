//	CNetClient.cpp
//
//	Implementation of CNetClient class
//
//	Parts based on iMatix SFL <http://www.imatix.com>

#include "Alchemy.h"
#include "NetUtil.h"

const int MAXMESSAGESIZE = 4096;

//	Constants for shutdown()
#ifndef SD_BOTH
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02
#endif

CNetClient::CNetClient (void) : m_pNotify(NULL)

//	CNetClient constructor

	{
	}

CNetClient::~CNetClient (void)

//	CNetClient destructor

	{
	Disconnect();
	}

ALERROR CNetClient::Connect (const CString &sHost, const CString &sService, const CString &sProtocol, INetClientEvents *pNotify)

//	Connect
//
//	Connects to the server and establishes the callback
//	notification. The caller must wait until the OnConnect call
//	to do anything except a Disconnect.

	{
	ASSERT(m_pNotify == NULL);

	//	Remember our connection info

	m_pNotify = pNotify;
	m_sHost = sHost;
	m_sService = sService;
	m_sProtocol = sProtocol;
	m_State = stateDisconnected;

	//	Create some events

	m_hQuit = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hReadDone = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hWriteDone = CreateEvent(NULL, TRUE, FALSE, NULL);

	//	Start our thread running.

	m_hThread = kernelCreateThread(CNetClient::ClientThreadThunk, this);
	if (m_hThread == NULL)
		{
		m_pNotify = NULL;
		return ERR_FAIL;
		}

	return NOERROR;
	}

void CNetClient::Disconnect (void)

//	Disconnect
//
//	Disconnect may be called at any time after Connect has
//	succeeded.

	{
	if (m_pNotify)
		{
		SetEvent(m_hQuit);

		//	Wait for the thread to die

		if (WaitForSingleObject(m_hThread, 5000) == WAIT_TIMEOUT)
			TerminateThread(m_hThread, 1);

		//	Release resources

		CloseHandle(m_hThread);
		CloseHandle(m_hReadDone);
		CloseHandle(m_hWriteDone);
		CloseHandle(m_hQuit);

		m_pNotify = NULL;
		}
	}

ALERROR CNetClient::SendData (const CString &sMessage)

//	SendData
//
//	Sends a data to the server. OnWriteComplete is called when
//	the data has been successfully sent.

	{
	ALERROR error;
	WorkStruct *pWork = new WorkStruct;

	pWork->Type = workSend;
	pWork->sData = sMessage;

	if (error = m_Queue.EnqueueObject((CObject *)pWork))
		{
		delete pWork;
		return error;
		}

	return NOERROR;
	}

DWORD CNetClient::ClientThread (void)

//	ClientThread
//
//	Thread to handle all asynchronous communications

	{
	int rc;
	SOCKADDR_IN name;
	DWORD lasterror;

	//	Resolve the host, service and protocol into an IP address
	//	and a port number.

	m_State = stateHostLookup;
	ZeroMemory(&name, sizeof(name));
	name.sin_family = AF_INET;

	//	Map service name to a port number

    SERVENT *pse = getservbyname(m_sService.GetASCIIZPointer(),
			m_sProtocol.GetASCIIZPointer());

	//	If we got back a valid entry then get the port number
	//	from the structure. m_sService is an ASCII port number.

	if (pse)
		name.sin_port = pse->s_port;
	else
		name.sin_port = htons((WORD)strToInt(m_sService, 0, NULL));

	//	Now check to see if the host name is a valid IP address.
	//	If it is not then we do a DNS lookup on the host name.

	name.sin_addr.s_addr = inet_addr(m_sHost.GetASCIIZPointer());
	if (name.sin_addr.s_addr == INADDR_NONE)
		{
		HOSTENT *phe = gethostbyname(m_sHost.GetASCIIZPointer());
		if (phe)
			memcpy(&name.sin_addr, phe->h_addr, phe->h_length);
		else
			{
			m_pNotify->OnError(HostNotFound, m_sHost);
			return 1;
			}
		}

	//	Map protocol name to protocol number

	PROTOENT *ppe = getprotobyname(m_sProtocol.GetASCIIZPointer());
	if (ppe == NULL)
		{
		m_pNotify->OnError(BadProtocol, m_sProtocol);
		return 1;
		}

	//	Choose socket type

	int SockType;
	if (strCompare(m_sProtocol, LITERAL("udp")) == 0)
		SockType = SOCK_DGRAM;
	else
		SockType = SOCK_STREAM;

	//	Create a socket

	m_Socket = socket(AF_INET, SockType, ppe->p_proto);
	if (m_Socket == INVALID_SOCKET)
		{
		m_pNotify->OnError(GeneralError, m_sHost);
		return 1;
		}

	//	Connect to the server

	m_State = stateConnecting;
	if (rc = connect(m_Socket, (SOCKADDR *)&name, sizeof(name)))
		{
		closesocket(m_Socket);
		m_pNotify->OnError(HostNotFound, m_sHost);
		return 1;
		}

	//	Tell the client that we connected successfully

	m_pNotify->OnConnect();

	//	Start reading on the socket

	CString sReadBuffer;
	DWORD dwBytesRead;

	OVERLAPPED oRead;
	oRead.hEvent = m_hReadDone;

	if (!ReadFile((HANDLE)m_Socket,
			sReadBuffer.GetWritePointer(4096),
			4096,
			&dwBytesRead,
			&oRead)
			&& (lasterror = GetLastError()) != ERROR_IO_PENDING)
		{
		closesocket(m_Socket);
		m_pNotify->OnError(ConnectionLost, strFromInt(lasterror, FALSE));
		return 1;
		}

	//	Remember if we are currently writing something out.
	//	We need to know this so that we don't try to do overlapped
	//	I/O while one is in progress.

	bool bWritingData = false;
	CString sWriteBuffer;
	DWORD dwBytesWritten;

	OVERLAPPED oWrite;
	oWrite.hEvent = m_hWriteDone;

	//	Set up the events

	HANDLE hEvents[4];

	const int EVENT_QUIT = 0;				//	Time to quit
	const int EVENT_SENDDATA = 1;			//	Data ready to send out
	const int EVENT_READDONE = 2;			//	Read complete
	const int EVENT_WRITEDONE = 3;			//	Write complete

	hEvents[EVENT_QUIT] = m_hQuit;
	hEvents[EVENT_SENDDATA] = m_Queue.GetQueueEvent();
	hEvents[EVENT_READDONE] = m_hReadDone;
	hEvents[EVENT_WRITEDONE] = m_hWriteDone;

	//	Keep working until we're done

	bool bQuit = false;
	while (!bQuit)
		{
		m_State = stateConnected;

		DWORD dwWait = WaitForMultipleObjects(4, hEvents, FALSE, INFINITE);

		//	If we're quitting, shut down everything

		switch (dwWait - WAIT_OBJECT_0)
			{
			case EVENT_QUIT:
				{
				//	Cancel any IO

				CancelIo((HANDLE)m_Socket);

				bQuit = true;
				break;
				}

			case EVENT_SENDDATA:
			case EVENT_WRITEDONE:
				{
				//	If our write is done then inform our client

				if (dwWait - WAIT_OBJECT_0 == EVENT_WRITEDONE)
					{
					ASSERT(bWritingData);

					m_pNotify->OnWriteComplete();
					ResetEvent(m_hWriteDone);
					bWritingData = false;
#ifdef LATER
					//	check to make sure that all the data was written
#endif
					}

				//	If we're not currently writing data and we've
				//	got stuff to write out then begin an overlapped I/O

				CObject *pObj;
				if (!bWritingData && (pObj = m_Queue.DequeueObject()))
					{
					WorkStruct *pWork = (WorkStruct *)pObj;
					sWriteBuffer = pWork->sData;
					delete pWork;

					if (!WriteFile((HANDLE)m_Socket,
							sWriteBuffer.GetPointer(),
							sWriteBuffer.GetLength(),
							&dwBytesWritten,
							&oWrite)
							&& (lasterror = GetLastError()) != ERROR_IO_PENDING)
						{
						m_pNotify->OnError(ConnectionLost, strFromInt(lasterror, FALSE));
						bQuit = true;
						break;
						}

					bWritingData = true;
					}

				//	HACK: If we're busy writing data then we need
				//	to reset the SENDDATA event because we don't want
				//	to handle it until were done writing

				else if (bWritingData)
					{
					ResetEvent(m_Queue.GetQueueEvent());
					}

				break;
				}

			case EVENT_READDONE:
				{
				//	Tell the client that we've got some data

				GetOverlappedResult((HANDLE)m_Socket,
						&oRead,
						&dwBytesRead,
						TRUE);

				//	If we got no data then it means that we lost
				//	the connection

				if (dwBytesRead == 0)
					{
					m_pNotify->OnError(ConnectionLost, LITERAL(""));
					bQuit = true;
					break;
					}

				sReadBuffer.Truncate(dwBytesRead);
				m_pNotify->OnDataReady(sReadBuffer);

				//	Continue reading some more

				ResetEvent(m_hReadDone);
				if (!ReadFile((HANDLE)m_Socket,
						sReadBuffer.GetWritePointer(4096),
						4096,
						&dwBytesRead,
						&oRead)
						&& (lasterror = GetLastError()) != ERROR_IO_PENDING)
					{
					m_pNotify->OnError(ConnectionLost, strFromInt(lasterror, FALSE));
					bQuit = true;
					break;
					}

				break;
				}
			}
		}

	//	Shutdown

	shutdown(m_Socket, SD_BOTH);
	closesocket(m_Socket);
	m_State = stateDisconnected;
	m_pNotify->OnDisconnect();

	return 0;
	}

DWORD WINAPI CNetClient::ClientThreadThunk (LPVOID pThis)

//	ClientThreadThunk
//
//	Sets up this pointer

	{
	return ((CNetClient *)pThis)->ClientThread();
	}

