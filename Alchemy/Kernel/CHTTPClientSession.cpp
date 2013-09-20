//	CHTTPClientSession.cpp
//
//	CHTTPClientSession class

#include "Kernel.h"
#include "KernelObjID.h"

#include "Internets.h"

const int ONE_SECOND =							1000;

CHTTPClientSession::CHTTPClientSession (void) :
		m_iLastError(inetsOK),
		m_bConnected(false),
		m_hStop(INVALID_HANDLE_VALUE),
		m_iStatus(notConnected),
		m_iInternetStatus(internetUnknown),
		m_dwLastActivity(0)

//	CHTTPClientSession constructor

	{
	m_hReadDone = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hWriteDone = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	}

CHTTPClientSession::~CHTTPClientSession (void)

//	CHTTPClientSession destructor

	{
	Disconnect();
	::CloseHandle(m_hReadDone);
	::CloseHandle(m_hWriteDone);
	}

bool CHTTPClientSession::CheckInternetAccess (void)

//	CheckInternetAccess
//
//	Returns TRUE if we can access a well-known Internet server.

	{
	CString sHost = CONSTLIT("google.com");

	//	Set our status so we don't recurse.

	m_iInternetStatus = internetChecking;

	//	Try to connect

	Disconnect();
	if (Connect(sHost, CONSTLIT("80")) != inetsOK)
		{
		m_iInternetStatus = internetDisconnected;
		return false;
		}

	//	Try to send and receive data

	CHTTPMessage Request;
	Request.InitRequest(CONSTLIT("GET"), CONSTLIT("/"));
	Request.AddHeader(CONSTLIT("Host"), sHost);
	Request.AddHeader(CONSTLIT("User-Agent"), CONSTLIT("AlchemyDebug/1.0"));
	Request.AddHeader(CONSTLIT("Accept-Language"), CONSTLIT("en-US"));
	Request.AddHeader(CONSTLIT("Connection"), CONSTLIT("close"));

	CHTTPMessage Response;
	if (Send(Request, &Response) != inetsOK)
		{
		Disconnect();
		m_iInternetStatus = internetDisconnected;
		return false;
		}

	//	If send succeeded it will set m_iInternetStatus.

	Disconnect();
	return true;
	}

EInetsErrors CHTTPClientSession::Connect (const CString &sHost, const CString &sPort)

//	Connect
//
//	Establish a connection.

	{
	CSmartLock Lock(m_cs);

	ASSERT(!m_bConnected);
	m_iStatus = inConnect;

	//	Prepare structure to connect

	SOCKADDR_IN name;
	utlMemSet(&name, sizeof(name), 0);
	name.sin_family = AF_INET;

	//	Map service name to a port number

	if (sPort.IsBlank())
		name.sin_port = ::htons(80);
	else
		name.sin_port = ::htons((WORD)strToInt(sPort, 80));

	//	Now check to see if the host name is a valid IP address.
	//	If it is not then we do a DNS lookup on the host name.

	name.sin_addr.s_addr = ::inet_addr(sHost.GetASCIIZPointer());
	if (name.sin_addr.s_addr == INADDR_NONE)
		{
		//	Set our status because we're about to do a network call

		m_iStatus = dnsLookup;

		//	DNS lookup

		HOSTENT *phe = ::gethostbyname(sHost.GetASCIIZPointer());
		if (phe == NULL)
			{
			::kernelDebugLogMessage("gethostbyname failed: %d.", ::WSAGetLastError());
			m_iStatus = notConnected;
			m_iLastError = inetsDNSError;
			if (m_iInternetStatus != internetChecking)
				m_iInternetStatus = internetUnknown;
			return m_iLastError;
			}

		utlMemCopy((char *)phe->h_addr, (char *)&name.sin_addr, phe->h_length);
		m_iStatus = inConnect;
		}

	//	Map protocol name to protocol number

	PROTOENT *ppe = ::getprotobyname("tcp");
	if (ppe == NULL)
		{
		::kernelDebugLogMessage("getprotobyname failed: %d.", ::WSAGetLastError());
		m_iStatus = notConnected;
		m_iLastError = inetsInvalidProtocol;
		return m_iLastError;
		}

	//	Create the socket

	m_Socket = ::socket(AF_INET, SOCK_STREAM, ppe->p_proto);
	if (m_Socket == INVALID_SOCKET)
		{
		::kernelDebugLogMessage("socket failed: %d.", ::WSAGetLastError());
		m_iStatus = notConnected;
		m_iLastError = inetsUnableToCreateSocket;
		return m_iLastError;
		}

	//	Connect to the server

	m_iStatus = connecting;
	if (connect(m_Socket, (SOCKADDR *)&name, sizeof(name)))
		{
		::kernelDebugLogMessage("connect failed: %d.", ::WSAGetLastError());
		::closesocket(m_Socket);
		m_iStatus = notConnected;
		m_iLastError = inetsCannotConnect;
		if (m_iInternetStatus != internetChecking)
			m_iInternetStatus = internetUnknown;
		return m_iLastError;
		}

	//	Success!

	m_iStatus = connected;
	m_sHost = sHost;
	m_bConnected = true;
	m_iLastError = inetsOK;
	m_dwLastActivity = ::GetTickCount();

	return m_iLastError;
	}

EInetsErrors CHTTPClientSession::Disconnect (void)

//	Disconnect
//
//	Disconnects the session

	{
	CSmartLock Lock(m_cs);
	if (!m_bConnected)
		return inetsOK;

	::shutdown(m_Socket, SD_BOTH);
	::closesocket(m_Socket);
	m_iStatus = notConnected;
	m_bConnected = false;
	m_iLastError = inetsOK;
	m_dwLastActivity = ::GetTickCount();

	return m_iLastError;
	}

bool CHTTPClientSession::IsConnected (void)

//	IsConnected
//
//	Returns TRUE if we're connected

	{
	CSmartLock Lock(m_cs);
	return (m_iStatus == connected);
	}

bool CHTTPClientSession::IsInternetAvailable (void)

//	IsInternetAvailable
//
//	Returns TRUE if we believe that the Internet is available (via check on
//	well-known reliable host).

	{
	switch (m_iInternetStatus)
		{
		case internetDisconnected:
			return false;

		case internetUnknown:
			return CheckInternetAccess();

		default:
			return true;
		}
	}

bool CHTTPClientSession::ReadBuffer (void *pBuffer, DWORD dwLen, DWORD *retdwRead)

//	ReadBuffer
//
//	Reads into the given buffer. Returns TRUE if successful; FALSE if failed.

	{
	//	Set up overlapped IO

	OVERLAPPED oRead;
	oRead.Offset = 0;
	oRead.OffsetHigh = 0;
	oRead.hEvent = m_hReadDone;
	::ResetEvent(m_hReadDone);

	//	Read into the buffer

	DWORD dwBytesRead;
	DWORD lasterror;
	if (!::ReadFile((HANDLE)m_Socket,
			pBuffer,
			dwLen,
			&dwBytesRead,
			&oRead)
			&& (lasterror = GetLastError()) != ERROR_IO_PENDING)
		{
		::kernelDebugLogMessage("ReadFile failed: %x", lasterror);
		return false;
		}

	//	Wait for data

	if (!WaitForTransfer(oRead, &dwBytesRead) || dwBytesRead == 0)
		return false;

	//	Done

	if (retdwRead)
		*retdwRead = dwBytesRead;

	m_dwLastActivity = ::GetTickCount();

	return true;
	}

EInetsErrors CHTTPClientSession::Send (const CHTTPMessage &Request, CHTTPMessage *retResponse)

//	Send
//
//	Blocking call that sends the given data and returns the result.
//	A connection must already be established.

	{
	CSmartLock Lock(m_cs);

	ASSERT(m_bConnected);

	//	Serialize the request

	CMemoryWriteStream RequestBuff;
	if (RequestBuff.Create() != NOERROR)
		{
		::kernelDebugLogMessage("Out of memory: Unable to create request buffer.");
		m_iLastError = inetsOutOfMemory;
		return m_iLastError;
		}

	if (Request.WriteToBuffer(&RequestBuff) != NOERROR)
		{
		::kernelDebugLogMessage("Unable to stream request buffer.");
		m_iLastError = inetsInvalidMessage;
		return m_iLastError;
		}

	//	Write it out

	DWORD dwBytesWritten;
	if (!WriteBuffer(RequestBuff.GetPointer(), RequestBuff.GetLength(), &dwBytesWritten))
		{
		::kernelDebugLogMessage("Unable to send request to server.");
		Disconnect();
		m_iLastError = inetsUnableToWrite;
		if (m_iInternetStatus != internetChecking)
			m_iInternetStatus = internetUnknown;
		return m_iLastError;
		}

	//	Now read the response. We build up a buffer to hold it.

	CMemoryWriteStream ResponseBuff;
	int iResponseBuffSize = 0;
	if (ResponseBuff.Create() != NOERROR)
		{
		::kernelDebugLogMessage("Out of memory: Unable to create response buffer.");
		m_iLastError = inetsOutOfMemory;
		return m_iLastError;
		}

	//	As we read the buffer we parse into the response

	retResponse->InitFromBuffer(CString(ResponseBuff.GetPointer(), iResponseBuffSize, true));

	//	Keep reading until we've got enough (or until the connection drops)

	while (!retResponse->IsMessageComplete())
		{
		//	Grow the buffer so that we can read into it

		int iReadSize = 4096;
		if (ResponseBuff.Write(NULL, iReadSize - (ResponseBuff.GetLength() - iResponseBuffSize)) != NOERROR)
			{
			::kernelDebugLogMessage("Out of memory: Unable to write to response buffer.");
			m_iLastError = inetsOutOfMemory;
			return m_iLastError;
			}

		//	Read

		DWORD dwBytesRead;
		if (!ReadBuffer(ResponseBuff.GetPointer() + iResponseBuffSize, iReadSize, &dwBytesRead))
			{
			::kernelDebugLogMessage("Unable to read from server. Reponse:\r\n%s", CString(ResponseBuff.GetPointer(), iResponseBuffSize, true));
			Disconnect();
			m_iLastError = inetsUnableToRead;
			if (m_iInternetStatus != internetChecking)
				m_iInternetStatus = internetUnknown;
			return m_iLastError;
			}

		//	Increment our read position

		iResponseBuffSize += dwBytesRead;

		//	Parse some more

		retResponse->InitFromBuffer(CString(ResponseBuff.GetPointer(), iResponseBuffSize, true));
		}

	//	Done

	m_dwLastActivity = ::GetTickCount();
	m_iInternetStatus = internetConnected;

	return inetsOK;
	}

bool CHTTPClientSession::WaitForTransfer (OVERLAPPED &oOp, DWORD *retdwBytesTransfered)

//	WaitForTransfer
//
//	Waits for an overlapped read or write operation to complete. Returns TRUE
//	if the operation completed successfully; FALSE otherwise.
//
//	Assumes that m_Socket is valid.

	{
	int iTimeOutLoops = 0;
	DWORD dwLastBytes = 0;

	while (true)
		{
		int iEventCount = 1;
		HANDLE hEvents[2];
		hEvents[0] = oOp.hEvent;
		if (m_hStop != INVALID_HANDLE_VALUE)
			{
			hEvents[1] = m_hStop;
			iEventCount++;
			}

		DWORD dwWait = ::WaitForMultipleObjects(iEventCount, hEvents, FALSE, ONE_SECOND);
		if (dwWait == WAIT_OBJECT_0 + 1)
			return false;

		//	See how much we've written. If we're done, then we've
		//	succeeded.

		DWORD dwBytesTransfered;
		if (::GetOverlappedResult((HANDLE)m_Socket,
				&oOp,
				&dwBytesTransfered,
				FALSE))
			{
			if (retdwBytesTransfered)
				*retdwBytesTransfered = dwBytesTransfered;
			return true;
			}

		//	Check for error

		DWORD error = ::GetLastError();
		if (error != ERROR_IO_INCOMPLETE
				&& error != ERROR_IO_PENDING)
			//	Any error means that we should stop.
			{
			::kernelDebugLogMessage("GetOverlappedResult failed: %x", error);
			return false;
			}

		//	Do progress notification

		//m_pNotify->OnWriteProgress(dwTotal, dwBytesWritten);

		//	If we're not making progress, then keep track of
		//	how many times we loop so that we can time out

		if (dwBytesTransfered == dwLastBytes)
			{
			if (++iTimeOutLoops >= 30)
				{
				::kernelDebugLogMessage("WaitForTransfer timed out");
				return false;
				}
			}

		//	If we are making progress, reset our counters

		else
			{
			iTimeOutLoops = 0;
			dwLastBytes = dwBytesTransfered;
			}
		}

	//	We can't here

	::kernelDebugLogMessage("This cannot happen.");
	return false;
	}

bool CHTTPClientSession::WriteBuffer (void *pBuffer, DWORD dwLen, DWORD *retdwWritten)

//	WriteBuffer
//
//	Writes a buffer to the socket and returns TRUE if sucessful and FALSE if failed.

	{
	//	Setup overlapped IO

	OVERLAPPED oWrite;
	oWrite.Offset = 0;
	oWrite.OffsetHigh = 0;
	oWrite.hEvent = m_hWriteDone;
	::ResetEvent(m_hWriteDone);

	//	Start an overlapped I/O

	DWORD dwBytesWritten;
	DWORD lasterror;
	if (!::WriteFile((HANDLE)m_Socket,
			pBuffer,
			dwLen,
			&dwBytesWritten,
			&oWrite)
			&& (lasterror = GetLastError()) != ERROR_IO_PENDING)
		{
		::kernelDebugLogMessage("WriteFile failed: %x", lasterror);
		return false;
		}

	//	Keep looping until we write all the data

	if (!WaitForTransfer(oWrite, &dwBytesWritten))
		return false;

	//	Done

	if (retdwWritten)
		*retdwWritten = dwBytesWritten;

	m_dwLastActivity = ::GetTickCount();
	return true;
	}
