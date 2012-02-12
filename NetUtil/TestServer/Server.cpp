//	Server.cpp
//
//	Tests a generic server

#include "Alchemy.h"
#include "NetUtil.h"

#include <stdio.h>

class CServer : public INetServerEvents
	{
	public:
		void Run (void);

		virtual CString OnClientConnect (DWORD dwClientID);
		virtual void OnClientError (DWORD dwClientID, NetErrors Error, CString sErrorString);
		virtual void OnError (NetErrors Error, CString sErrorString);
		virtual CString OnDataReceived (DWORD dwClientID, CString sData, LPVOID pData);
		virtual void OnClientDisconnect (DWORD dwClientID);

	private:
		CNetServer m_Server;
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
		CServer myServer;
		myServer.Run();
		}

	//	Done

	netShutdown();
	kernelCleanUp();
	return 0;
	}

void CServer::Run (void)

//	Run
//
//	Runs the server

	{
	ALERROR error;

	if (error = m_Server.Boot(LITERAL("5666"), LITERAL("tcp"), this))
		{
		printf("ERROR: Unable to boot server.\n");
		return;
		}

	//	accept commands until done

	while (true)
		{
		CString sInput;
		gets(sInput.GetWritePointer(1024));
		sInput.Truncate(lstrlen(sInput.GetASCIIZPointer()));

		//	If this is the quit command, then we're done

		if (strCompare(sInput, LITERAL("\\quit")) == 0)
			break;
		}

	m_Server.Shutdown();
	}

CString CServer::OnClientConnect (DWORD dwClientID)
	{
	printf("%d: New client connected\n", dwClientID);

	char szBuffer[1024];
	sprintf(szBuffer, "Welcome, client %d!\n", dwClientID);
	return LITERAL(szBuffer); 
	}

void CServer::OnClientError (DWORD dwClientID, NetErrors Error, CString sErrorString)
	{
	printf("%d: ERROR\n", dwClientID);
	}

void CServer::OnError (NetErrors Error, CString sErrorString)
	{
	printf("Connection error\n");
	}

CString CServer::OnDataReceived (DWORD dwClientID, CString sData, LPVOID pData)
	{
	printf("%d: %s\n", dwClientID, sData.GetASCIIZPointer());

	char szBuffer[1024];
	sprintf(szBuffer, "ECHO: %s\n", sData.GetASCIIZPointer());
	return LITERAL(szBuffer); 
	}

void CServer::OnClientDisconnect (DWORD dwClientID)
	{
	printf("%d: Client diconnected\n", dwClientID);
	}
