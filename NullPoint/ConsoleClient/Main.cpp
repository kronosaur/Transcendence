//	Main.cpp
//
//	Implements a console test client for Null Point

#include "Alchemy.h"
#include "NetUtil.h"
#include "NPEngine.h"
#include "NPRPC.h"

#include <stdio.h>

struct CommandData
	{
	char *pszCmd;					//	command
	int iCmd;						//	RPC command ID
	int iParamCount;				//	parameters
	char *pszHelp;					//	Help string
	};

class CConsoleClient : INPEvents
	{
	public:
		CConsoleClient (void);

		ALERROR Connect (const CString &sServer);
		void Disconnect (void);
		bool SendCommand (const CString &sCommand);
		bool WaitForResponse (void);

		static void Run (void);

		//	INPEvents
		virtual void OnConnect (const CNPGameDesc &Desc);
		virtual void OnError (const NPClientError iError, const CString &sError);
		virtual void OnCmdFail (int iCmd, const CString &sError);
		virtual void OnCmdSuccess (int iCmd);
		virtual void OnLogin (DWORD dwSovereignUNID);
		virtual void OnViewFleet (CNPFleetView &Fleet);
		virtual void OnViewNullPoint (CNPNullPointView &NullPoint);
		virtual void OnViewSovereign (CNPSovereignView &Sovereign);
		virtual void OnViewWorld (CNPWorldView &World);

	private:
		enum States
			{
			stateWaitingForInput,
			stateWaitingForConnection,
			};

		CNPClient m_NP;
		States m_iState;
		NPClientError m_iError;

		HANDLE m_hReadyEvent;
		CString m_sResponse;
	};

static CommandData g_CommandData[] =
	{
		{ "createuniverse",		CMD_CREATE_UNIVERSE,	1,	"CREATEUNIVERSE password" },
		{ "fleetdest",			CMD_FLEET_DEST,			2,	"FLEETDEST fleet dest" },
		{ "login",				CMD_LOGIN,				2,	"LOGIN name password" },
		{ "newplayer",			CMD_NEW_PLAYER,			3,	"NEWPLAYER name password empire-name" },
		{ "view",				CMD_VIEW,				1,	"VIEW object" },
		{ "", -1, 0, "" },
	};

int main (int argc, char *argv[ ], char *envp[ ])

//	main
//
//	main entry-point

	{
	ALERROR error;

	if (!kernelInit())
		{
		printf("ERROR: Unable to initialize Alchemy kernel.\n");
		return 1;
		}

	if (error = netBoot())
		{
		printf("ERROR: Unable to initialize network system.\n");
		return 1;
		}

	//	Do it

	CConsoleClient::Run();

	//	Done

	netShutdown();
	kernelCleanUp();
	return 0;
	}

CString UNID2String (DWORD dwUNID)
	{
	CString sName;

	switch (GetUNIDType(dwUNID))
		{
		case unidNullPoint:
			sName = CONSTLIT("n");
			break;

		case unidLink:
			sName = CONSTLIT("l");
			break;

		case unidWorld:
			sName = CONSTLIT("w");
			break;

		case unidFleet:
			sName = CONSTLIT("f");
			break;

		case unidSovereign:
			sName = CONSTLIT("e");
			break;

		case unidResourceClass:
			sName = CONSTLIT("res");
			break;

		case unidUnitClass:
			sName = CONSTLIT("unit");
			break;

		case unidReserved:
			return CONSTLIT("Unknown");

		default:
			sName = strFromInt(GetUNIDType(dwUNID), FALSE);
			sName.Append(CONSTLIT("-"));
		}

	sName.Append(strFromInt(GetUNIDOrdinal(dwUNID), FALSE));
	return sName;
	}

bool String2UNID (const CString &sName, DWORD *retdwUNID)
	{
	char *pPos = sName.GetPointer();
	UNIDType dwType;

	switch (*pPos)
		{
		case 'n':
		case 'N':
			dwType = unidNullPoint;
			break;

		case 'l':
		case 'L':
			dwType = unidLink;
			break;

		case 'w':
		case 'W':
			dwType = unidWorld;
			break;

		case 'f':
		case 'F':
			dwType = unidFleet;
			break;

		case 'e':
		case 'E':
		case 's':
		case 'S':
			dwType = unidSovereign;
			break;

		default:
			{
			printf("Invalid object name: %s\n", pPos);
			return false;
			}
		}

	BOOL bFailed;
	DWORD dwOrdinal = strParseInt(pPos+1, 0, NULL, &bFailed);
	if (bFailed)
		{
		printf("Invalid object name: %s\n", pPos);
		return false;
		}

	*retdwUNID = MakeUNID(dwType, dwOrdinal);
	return true;
	}

CConsoleClient::CConsoleClient (void) : m_iError(npeSuccess)
	{
	}

ALERROR CConsoleClient::Connect (const CString &sServer)

//	Connect
//
//	Connects to the server

	{
	ALERROR error;

	//	Prepare the response semaphore

	m_hReadyEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	//	Connect

	m_iState = stateWaitingForConnection;
	if (error = m_NP.Connect(sServer, this))
		return error;

	return NOERROR;
	}

void CConsoleClient::Disconnect (void)

//	Disconnect
//
//	Disconnects from the server

	{
	m_NP.Disconnect();
	}

bool CConsoleClient::SendCommand (const CString &sCommand)

//	SendCommand
//
//	Parses the command and sends it to the server. Returns true
//	if it sent a command to the server.

	{
	CStringArray Tokens;

	//	First we separate the string into space-separated tokens

	if (strDelimit(sCommand, ' ', 1, &Tokens))
		{
		printf("Unable to tokenize command line\n");
		return false;
		}

	//	Look for the command in the table

	int i = 0;
	while (g_CommandData[i].iCmd != -1 
			&& strCompare(Tokens.GetStringValue(0), LITERAL(g_CommandData[i].pszCmd)) != 0)
		i++;

	//	If we could not find it, then we have an error

	if (g_CommandData[i].iCmd == -1)
		{
		printf("Unknown command: %s\n", Tokens.GetStringValue(0).GetASCIIZPointer());
		return false;
		}

	//	Make sure we have the right number of parameters

	if (Tokens.GetCount() < g_CommandData[i].iParamCount + 1)
		{
		printf("Not enough parameters for %s command\n", g_CommandData[i].pszCmd);
		return false;
		}

	//	Reset the event before we tell the server anything

	::ResetEvent(m_hReadyEvent);
	m_sResponse.Truncate(0);

	//	Send the command

	switch (g_CommandData[i].iCmd)
		{
		case CMD_CREATE_UNIVERSE:
			m_NP.CreateUniverse(Tokens.GetStringValue(1));
			break;

		case CMD_FLEET_DEST:
			{
			DWORD dwFleet;
			DWORD dwDest;

			if (!String2UNID(Tokens.GetStringValue(1), &dwFleet))
				return false;
			if (!String2UNID(Tokens.GetStringValue(2), &dwDest))
				return false;

			m_NP.FleetDest(dwFleet, dwDest);
			break;
			}

		case CMD_LOGIN:
			m_NP.Login(Tokens.GetStringValue(1), Tokens.GetStringValue(2));
			break;

		case CMD_NEW_PLAYER:
			m_NP.NewPlayer(Tokens.GetStringValue(1),
					Tokens.GetStringValue(2),
					Tokens.GetStringValue(3));
			break;

		case CMD_VIEW:
			{
			DWORD dwUNID;
			if (!String2UNID(Tokens.GetStringValue(1), &dwUNID))
				return false;

			m_NP.ViewObject(dwUNID);
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return true;
	}

bool CConsoleClient::WaitForResponse (void)

//	WaitForResponse
//
//	Waits for a response from the server and prints out the result.
//	Returns false if we get an error while waiting

	{
	DWORD dwWait;

	dwWait = ::WaitForSingleObject(m_hReadyEvent, INFINITE);
	printf(m_sResponse.GetASCIIZPointer());

	return (m_iError == npeSuccess ? true : false);
	}

void CConsoleClient::Run (void)

//	Run
//
//	Runs the client

	{
	ALERROR error;
	CConsoleClient Client;
	CString sServer = CONSTLIT("leibowitz.neurohack.com");

	//	Connect to the server

	printf("Connecting to %s...", sServer.GetASCIIZPointer());
	if (error = Client.Connect(sServer))
		{
		printf(" Unable to connect to server.\n");
		return;
		}

	//	Wait for the connection

	if (!Client.WaitForResponse())
		{
		Client.Disconnect();
		return;
		};

	//	Wait for response until we're done

	while (true)
		{
		//	Get user input

		CString sInput;
		Client.m_iState = stateWaitingForInput;
		printf(": ");
		gets(sInput.GetWritePointer(1024));
		sInput.Truncate(lstrlen(sInput.GetASCIIZPointer()));

		//	If this is the quit command, we're done
		//	Otherwise we send the command to the server

		if (strCompare(sInput, CONSTLIT("quit")) == 0)
			break;

		//	If this is a help command then print out some help

		if (strCompare(sInput, CONSTLIT("help")) == 0
				|| strCompare(sInput, CONSTLIT("?")) == 0)
			{
			int i = 0;
			while (g_CommandData[i].iCmd != -1)
				printf("%s\n", g_CommandData[i++].pszHelp);

			continue;
			}

		//	Send the command to the server

		if (!Client.SendCommand(sInput))
			continue;

		//	Wait for a response

		if (!Client.WaitForResponse())
			break;
		}

	//	Disconnect

	Client.Disconnect();
	}

void CConsoleClient::OnConnect (const CNPGameDesc &Desc)

//	OnConnect
//
//	Connect successful

	{
	if (m_iState != stateWaitingForConnection)
		printf("ERROR: Unexpected OnConnect()\n");

	m_sResponse = CONSTLIT("\n\nWelcome to NullPoint!\n");
	m_sResponse.Append(strPatternSubst(CONSTLIT("Version %d.%d\n"), HIWORD(Desc.m_dwVersion), LOWORD(Desc.m_dwVersion)));
	m_sResponse.Append(CONSTLIT("(c) Copyright 1999 by NeuroHack, Inc. All Rights Reserved.\n\n"));

	//	Compose a response depending on whether the universe has
	//	been created or not

	if (Desc.m_dwUNID == 0)
		{
		m_sResponse.Append(CONSTLIT("The universe does not exist on this server.\n"));
		}
	else
		{
		m_sResponse.Append(strPatternSubst(CONSTLIT("%s (turn: %d)\n"), Desc.m_sName, Desc.m_iTurns));
		m_sResponse.Append(strPatternSubst(CONSTLIT("%d NullPoints\n"), Desc.m_iNullPoints));
		m_sResponse.Append(strPatternSubst(CONSTLIT("%d Players\n"), Desc.m_iPlayers));
		}

	//	Done

	::SetEvent(m_hReadyEvent);
	}

void CConsoleClient::OnError (const NPClientError iError, const CString &sError)

//	OnError
//
//	Error of some kind

	{
	m_iError = iError;

	switch (iError)
		{
		case npeUnableToConnect:
			m_sResponse = sError;
			m_sResponse.Append(CONSTLIT("\n"));
			break;

		case npeLostConnection:
			m_sResponse = strPatternSubst(CONSTLIT("ERROR: Connection lost (%s)\n"), sError);
			break;
		}

	::SetEvent(m_hReadyEvent);
	}

void CConsoleClient::OnCmdFail (int iCmd, const CString &sError)

//	OnCmdFail
//
//	Previous command failed

	{
	//	Find the command in the table

	int i = 0;
	while (g_CommandData[i].iCmd != -1 && g_CommandData[i].iCmd != iCmd)
		i++;

	ASSERT(g_CommandData[i].iCmd != -1);

	//	Compose response

	m_sResponse = strPatternSubst(CONSTLIT("ERROR[%s]: %s\n"),
			LITERAL(g_CommandData[i].pszCmd),
			sError);

	::SetEvent(m_hReadyEvent);
	}

void CConsoleClient::OnCmdSuccess (int iCmd)

//	OnCmdSuccess
//
//	Previous command succeeded

	{
	//	Find the command in the table

	int i = 0;
	while (g_CommandData[i].iCmd != -1 && g_CommandData[i].iCmd != iCmd)
		i++;

	ASSERT(g_CommandData[i].iCmd != -1);

	//	Compose response

	m_sResponse = CONSTLIT("OK\n");

	::SetEvent(m_hReadyEvent);
	}

void CConsoleClient::OnLogin (DWORD dwSovereignUNID)

//	OnLogin
//
//	Login succeeded

	{
	//	Compose response

	m_sResponse = strPatternSubst(CONSTLIT("\nWelcome, %s, ruler of %s!\n\nYear: %d\nEmpireID: %s\n"),
			m_NP.GetEmperorName(),
			m_NP.GetEmpireName(),
			m_NP.GetStardate(),
			UNID2String(dwSovereignUNID));

	::SetEvent(m_hReadyEvent);
	}

void CConsoleClient::OnViewFleet (CNPFleetView &Fleet)

//	OnViewFleet
//
//	Shows fleet info

	{
	m_sResponse = strPatternSubst(CONSTLIT("%s\nLocation: %s\nSovereign: %s\nDest: %s (%d)\n"),
			UNID2String(Fleet.GetUNID()),
			UNID2String(Fleet.GetLocation()),
			UNID2String(Fleet.GetSovereign()),
			UNID2String(Fleet.GetDest()),
			Fleet.GetInTransit());

	m_sResponse.Append(CONSTLIT("Assets:"));
	for (int i = 0; i < Fleet.GetAssetList().GetCount(); i++)
		{
		m_sResponse.Append(CONSTLIT(" "));
		m_sResponse.Append(UNID2String(Fleet.GetAssetList().GetAssetUNID(i)));
		}
	m_sResponse.Append(CONSTLIT("\n"));

	::SetEvent(m_hReadyEvent);
	}

void CConsoleClient::OnViewNullPoint (CNPNullPointView &NullPoint)

//	OnViewNullPoint
//
//	Show null point info

	{
	int i;
	m_sResponse = strPatternSubst(CONSTLIT("%s\nXY: %d,%d\nWorld: %s\n"),
			UNID2String(NullPoint.GetUNID()),
			NullPoint.GetX(),
			NullPoint.GetY(),
			UNID2String(NullPoint.GetWorld()));

	m_sResponse.Append(CONSTLIT("Links:"));
	for (i = 0; i < NullPoint.GetLinkCount(); i++)
		{
		m_sResponse.Append(CONSTLIT(" "));
		m_sResponse.Append(UNID2String(NullPoint.GetLinkDest(i)));
		}
	m_sResponse.Append(CONSTLIT("\n"));

	m_sResponse.Append(CONSTLIT("Fleets:"));
	for (i = 0; i < NullPoint.GetFleetCount(); i++)
		{
		m_sResponse.Append(CONSTLIT(" "));
		m_sResponse.Append(UNID2String(NullPoint.GetFleet(i)));
		}
	m_sResponse.Append(CONSTLIT("\n"));

	::SetEvent(m_hReadyEvent);
	}

void CConsoleClient::OnViewSovereign (CNPSovereignView &Sovereign)

//	OnViewSovereign
//
//	Show sovereign info

	{
	m_sResponse = strPatternSubst(CONSTLIT("%s (%s)\nCapital: %s\n"),
			Sovereign.GetName(),
			UNID2String(Sovereign.GetUNID()),
			UNID2String(Sovereign.GetCapital()));

	::SetEvent(m_hReadyEvent);
	}

void CConsoleClient::OnViewWorld (CNPWorldView &World)

//	OnViewWorld
//
//	Show world info

	{
	m_sResponse = strPatternSubst(CONSTLIT("%s\nLocation: %s\nSovereign: %s\nPopulation: %d\nTech Level: %d\nEfficiency: %d\nTraits: "),
			UNID2String(World.GetUNID()),
			UNID2String(World.GetLocation()),
			UNID2String(World.GetSovereign()),
			World.GetPopulation(),
			World.GetTechLevel(),
			World.GetEfficiency());

	if (World.GetTraitCount() == 0)
		m_sResponse.Append(CONSTLIT("None\n"));
	else
		{
		for (int i = 0; i < World.GetTraitCount(); i++)
			{
			m_sResponse.Append(strFromInt(World.GetTrait(i), false));
			m_sResponse.Append(CONSTLIT(" "));
			}

		m_sResponse.Append(CONSTLIT("\n"));
		}

	::SetEvent(m_hReadyEvent);
	}
