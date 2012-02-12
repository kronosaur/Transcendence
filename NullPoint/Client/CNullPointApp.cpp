//	CNullPointApp
//
//	CNullPointApp object

#include "Alchemy.h"
#include "NetUtil.h"
#include "Flatland.h"
#include "NullPointClient.h"
#include "ViewCode.h"

DEFINE_FLATLAND_APP(CNullPointApp)

#define NPSERVER_ID							1	//	ID for events from NP server

#define MAINWINDOW_ID						100
#define GAMESCREEN_ID						100

#define MESSAGETEXT_ID						101
#define LOGINPANE_ID						102
#define MAPPANE_ID							201

enum NPServerEvents
	{
	srvUnableToConnect =	1000,
	srvLostConnection =		1001,
	srvConnect =			1002,
	srvLogin =				1003,
	srvCmdFail =			1004,
	};

CNullPointApp::CNullPointApp (void) : m_pMainWindow(NULL),
		m_iState(Start)
	{
	}

//	IFlatlandEvents

void CNullPointApp::OnShutdown (void)

//	OnShutdown
//
//	Called when we're shutting down the application

	{
	m_NP.Disconnect();
	netShutdown();
	}

ALERROR CNullPointApp::OnStartup (void)

//	OnStartup
//
//	Called before we enter the main loop. We need to put up a window.

	{
	ALERROR error;
	CString sServer = CONSTLIT("leibowitz.neurohack.com");

	//	Initialize the network subsystem

	if (error = netBoot())
		{
		g_pFlatland->DisplayErrorMessage(CONSTLIT("Unable to initialize network system."));
		return ErrorSetDisplayed(error);
		}

	//	Register some code components

	if (error = RegisterCode())
		{
		m_NP.Disconnect();
		netShutdown();
		return error;
		}

	g_pFlatland->RegisterView(CONSTLIT("NPMap"), CMapView::Create);

	//	Create the main window

	if (error = g_pFlatland->CreateNewWindow(CONSTLIT("Main"),
			100,
			this,
			&m_pMainWindow))
		{
		m_NP.Disconnect();
		netShutdown();
		return error;
		}

	//	Connect to the server. Note that we must do this after we've
	//	created the main window because we main need to post some
	//	messages to the main window in our callback.

	if (error = m_NP.Connect(sServer, this))
		{
		netShutdown();
		g_pFlatland->DisplayErrorMessage(CONSTLIT("Unable to connect to NullPoint server."));
		return ErrorSetDisplayed(error);
		}

	return NOERROR;
	}

//	Helper code for the OnEvent state machine

bool CNullPointApp::CheckUniverseExistence (CNPGameDesc *pGameDesc)
	{
	if (pGameDesc->m_dwUNID == 0)
		{
		ShowMessage(CONSTLIT("The Universe has not been created."));
		m_iState = ShowFatalError;
		return false;
		}

	return true;
	}

void CNullPointApp::ExitNP (void)
	{
	m_pMainWindow->Close();
	}

void CNullPointApp::GetUserNameAndPassword (CFView *pLoginPane)
	{
	m_sUserName.INTTakeStorage((void *)pLoginPane->Message(CLoginPaneCode::GetUserName, 0));
	m_sPassword.INTTakeStorage((void *)pLoginPane->Message(CLoginPaneCode::GetPassword, 0));
	}

void CNullPointApp::HandleMapPaneEvents(CFView *pView, int iEvent, DWORD dwData1, const CString &sData2)
	{
	switch (iEvent)
		{
		case CMapView::NeedNPClient:
			pView->Message(CMapView::SetNPClient, (DWORD)&m_NP);
			break;
		}
	}

void CNullPointApp::Login (void)
	{
	m_NP.Login(m_sUserName, m_sPassword);
	m_sPassword = CONSTLIT("");
	ShowMessage(strPatternSubst(CONSTLIT("Login as %s..."), m_sUserName));
	}

void CNullPointApp::ShowMessage (const CString &sString)
	{
	CFView *pMsg = m_pMainWindow->FindView(MESSAGETEXT_ID);
	pMsg->Message(CFTextView::SetText, 0, sString);
	}

//	IFController

void CNullPointApp::OnEvent (CFView *pView, int iID, int iEvent, DWORD dwData1, const CString &sData2)
	{
	switch (m_iState)
		{
		//	Start: The game has just been launched and we need two
		//	things to initiate a login: 1) the user must type in
		//	a username and password, and 2) we must have a connection
		//	to the NullPoint server.

		case Start:
			{
			//	User cancelled

			if (iID == LOGINPANE_ID && iEvent == CLoginPaneCode::Close)
				ExitNP();
			
			//	Got password from user

			else if (iID == LOGINPANE_ID && iEvent == CLoginPaneCode::OK)
				{
				GetUserNameAndPassword(pView);
				ShowMessage(CONSTLIT("Connecting to server..."));
				m_iState = PasswordOK_WaitingForConnect;
				}

			//	Connection to server successful

			else if (iID == NPSERVER_ID && iEvent == srvConnect)
				{
				CNPGameDesc *pGameDesc = (CNPGameDesc *)dwData1;

				if (!CheckUniverseExistence(pGameDesc))
					break;

				ShowMessage(CONSTLIT("Connected to server."));
				m_iState = ConnectOK_WaitingForPassword;
				}

			//	Connection failed

			else if (iID == NPSERVER_ID && iEvent == srvUnableToConnect)
				{
				ShowMessage(CONSTLIT("Unable to connect to server."));
				m_iState = ShowFatalError;
				}

			else
				ASSERT(FALSE);

			break;
			}

		//	ShowFatalError: If we get here then the game can not
		//	continue, most likely because we could not make a
		//	connection to the NullPoint server (or perhaps because
		//	the Universe is not created.

		case ShowFatalError:
			{
			//	User cancelled

			if (iID == LOGINPANE_ID && iEvent == CLoginPaneCode::Close)
				ExitNP();
			
			break;
			}

		//	PasswordOK_WaitingForConnect: The user has entered a
		//	username and password but we still have not established
		//	a connection to the server.

		case PasswordOK_WaitingForConnect:
			{
			//	User cancelled

			if (iID == LOGINPANE_ID && iEvent == CLoginPaneCode::Close)
				ExitNP();
			
			//	Connection to server successful

			else if (iID == NPSERVER_ID && iEvent == srvConnect)
				{
				CNPGameDesc *pGameDesc = (CNPGameDesc *)dwData1;

				if (!CheckUniverseExistence(pGameDesc))
					break;

				Login();
				m_iState = WaitingForLogin;
				}

			//	Connection failed

			else if (iID == NPSERVER_ID && iEvent == srvUnableToConnect)
				{
				ShowMessage(CONSTLIT("Unable to connect to server."));
				m_iState = ShowFatalError;
				}

			break;
			}

		//	ConnectOK_WaitingForPassword: We have a good connection
		//	to the server and the Universe exists properly, but the
		//	user still needs to enter a username and password.

		case ConnectOK_WaitingForPassword:
			{
			//	User cancelled

			if (iID == LOGINPANE_ID && iEvent == CLoginPaneCode::Close)
				ExitNP();
			
			//	Got password from user

			else if (iID == LOGINPANE_ID && iEvent == CLoginPaneCode::OK)
				{
				GetUserNameAndPassword(pView);
				Login();
				m_iState = WaitingForLogin;
				}

			break;
			}

		//	WaitingForLogin: We've submitted both a username and password
		//	to the server and are expecting a reply back. We may be let
		//	in or we may fail login because the password is incorrect.

		case WaitingForLogin:
			{
			//	User cancelled

			if (iID == LOGINPANE_ID && iEvent == CLoginPaneCode::Close)
				ExitNP();

			//	If login failed, go back to getting a password

			else if (iID == NPSERVER_ID && iEvent == srvCmdFail)
				{
				ShowMessage(sData2);
				m_iState = ConnectOK_WaitingForPassword;
				}

			//	If success

			else if (iID == NPSERVER_ID && iEvent == srvLogin)
				{
				m_iState = PlayingGame;

				m_pMainWindow->SetView(CONSTLIT("GameScreen"));
				}
			
			break;
			}

		case PlayingGame:
			{
			break;
			}
		}
	}

void CNullPointApp::OnViewLoad (CFView *pView, int iID)
	{
	switch (m_iState)
		{
		case PlayingGame:
			{
			//	If we're initializing the game view, set up some
			//	parameters

			if (iID == GAMESCREEN_ID)
				{
				//	Give the NP client to the map

				CFView *pMap = pView->FindSubView(MAPPANE_ID);
				pMap->Message(CMapView::SetNPClient, (DWORD)&m_NP);
				}

			break;
			}
		}
	}

//	INPEvents

void CNullPointApp::OnCmdFail (int iCmd, const CString &sError)
	{
	m_pMainWindow->RaiseExternalEvent(NPSERVER_ID, srvCmdFail, (DWORD)iCmd, sError);
	}

void CNullPointApp::OnCmdSuccess (int iCmd)
	{
	}

void CNullPointApp::OnConnect (const CNPGameDesc &Desc)
	{
	m_pMainWindow->RaiseExternalEvent(NPSERVER_ID, srvConnect, (DWORD)&Desc);
	}

void CNullPointApp::OnError (const NPClientError iError, const CString &sError)
	{
	if (iError == npeUnableToConnect)
		m_pMainWindow->RaiseExternalEvent(NPSERVER_ID, srvUnableToConnect, (DWORD)iError, sError);
	else if (iError == npeLostConnection)
		m_pMainWindow->RaiseExternalEvent(NPSERVER_ID, srvLostConnection, (DWORD)iError, sError);
	}

void CNullPointApp::OnLogin (DWORD dwSovereignUNID)
	{
	m_pMainWindow->RaiseExternalEvent(NPSERVER_ID, srvLogin, dwSovereignUNID);
	}

void CNullPointApp::OnViewFleet (CNPFleetView &Fleet)
	{
	}

void CNullPointApp::OnViewNullPoint (CNPNullPointView &NullPoint)
	{
	}

void CNullPointApp::OnViewSovereign (CNPSovereignView &Sovereign)
	{
	}

void CNullPointApp::OnViewWorld (CNPWorldView &World)
	{
	}
