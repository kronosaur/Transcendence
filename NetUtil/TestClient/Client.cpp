//	Client.cpp
//
//	Tests a generic client

#include "Alchemy.h"
#include "NetUtil.h"

#include <stdio.h>

class CClient : public INetClientEvents
	{
	public:
		void Run (void);
		virtual void OnConnect (void) { SetEvent(m_hEvent); }
		virtual void OnError (NetErrors Error, CString sErrorString) { SetEvent(m_hEvent); }
		virtual void OnWriteComplete (void) { SetEvent(m_hEvent); }
		virtual void OnDataReady (CString sData) { printf(sData.GetASCIIZPointer()); }
		virtual void OnDisconnect (void) { SetEvent(m_hEvent); }

	private:
		CNetClient m_Connection;
		HANDLE m_hEvent;
		CString m_sReply;
	};

int main (int argc, char *argv[ ], char *envp[ ])

//	main
//
//	main entry-point

	{
	ALERROR error;

	if (!kernelInit())
		{
		printf("ERROR: Unable to initialize Alchemy.\n");
		return 1;
		}

	if (error = netBoot())
		{
		printf("ERROR: Unable to initialize network system.\n");
		return 1;
		}

	//	Do it

	if (true)
		{
		CClient myClient;
		myClient.Run();
		}

	//	Done

	netShutdown();
	kernelCleanUp();
	return 0;
	}

void CClient::Run (void)
	{
	ALERROR error;

	//	Create an event to wait on

	m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//	Connect

	CString sHost = LITERAL("www.neurohack.com");

	printf("Connecting to %s...\n", sHost.GetASCIIZPointer());
	if (error = m_Connection.Connect(sHost,
			LITERAL("5666"),
			LITERAL("tcp"),
			this))
		{
		printf("ERROR: Unable to connect to %s.\n", sHost.GetASCIIZPointer());
		return;
		}

	WaitForSingleObject(m_hEvent, INFINITE);
	ResetEvent(m_hEvent);
	printf("Connected to %s.\n", sHost.GetASCIIZPointer());

	//	accept commands until done

	while (true)
		{
		CString sInput;
		gets(sInput.GetWritePointer(1024));
		sInput.Truncate(lstrlen(sInput.GetASCIIZPointer()));

		//	If this is the quit command, then we're done

		if (strCompare(sInput, LITERAL("\\quit")) == 0)
			break;

		//	Tack-on \r\n to indicate end of line

		sInput.Append(LITERAL("\r\n"));

		//	Send the message and wait for it to be sent
		
		m_Connection.SendData(sInput);
		WaitForSingleObject(m_hEvent, INFINITE);
		ResetEvent(m_hEvent);
		}

	//	Done

	printf("Terminating connection to %s...\n", sHost.GetASCIIZPointer());
	m_Connection.Disconnect();
	WaitForSingleObject(m_hEvent, INFINITE);
	}
