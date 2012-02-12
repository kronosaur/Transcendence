//	CNPClient.cpp
//
//	Implements CNPClient class

#include "Alchemy.h"
#include "NPEngine.h"
#include "NPRPC.h"

CNPClient::CNPClient (void) : m_iStatus(npsDisconnected),
		m_pHandler(NULL),
		m_bConnected(false),
		m_hWritesComplete(NULL),
		m_dwSovereignUNID(0)
	{
	//	Initialize global critical section that will protect
	//	all member data.

	::InitializeCriticalSection(&m_csClient);
	}

CNPClient::~CNPClient (void)
	{
	Disconnect();

	//	Clean up

	if (m_hWritesComplete)
		::CloseHandle(m_hWritesComplete);

	::DeleteCriticalSection(&m_csClient);
	}

ALERROR CNPClient::Connect (const CString &sServer, INPEvents *pEvents)

//	Connect
//
//	Connects to a NullPoint server

	{
	ALERROR error;

	ASSERT(m_iStatus == npsDisconnected);

	//	Create some events (start signalled)

	m_hWritesComplete = ::CreateEvent(NULL, TRUE, TRUE, NULL);

	//	Remember our handler

	m_pHandler = pEvents;

	//	Connect to the server

	m_iStatus = npsWaitingForConnection;
	if (error = m_Connection.Connect(sServer, 
			SERVER_PORT,
			SERVER_PROTOCOL,
			this))
		{
		m_iStatus = npsDisconnected;
		return error;
		}

	//	Wait for OnConnect call...

	return NOERROR;
	}

ALERROR CNPClient::Disconnect (void)

//	Disconnect
//
//	Breaks the connection

	{
	//	Disconnect

	if (m_bConnected || m_iStatus == npsWaitingForConnection)
		{
		//	Wait for all writes to complete

		::WaitForSingleObject(m_hWritesComplete, 5000);

		//	Disconnect

		m_Connection.Disconnect();
		}

	return NOERROR;
	}

void CNPClient::OnResConnect (const CDataPackStruct &Data)

//	OnResConnect
//
//	Server has accepted our connection

	{
	CNPGameDesc Desc;

	Desc.m_dwVersion = (DWORD)Data.GetIntegerItem(1);
	Desc.m_dwUNID = (DWORD)Data.GetIntegerItem(2);
	Desc.m_sName = Data.GetStringItem(3);
	Desc.m_iTurns = Data.GetIntegerItem(4);
	Desc.m_iNullPoints = Data.GetIntegerItem(5);
	Desc.m_iPlayers = Data.GetIntegerItem(6);

	DWORD dwFlags = (DWORD)Data.GetIntegerItem(7);
	Desc.m_bOpen = (dwFlags & GAME_ACCEPT_NEW_PLAYERS ? true : false);

	m_pHandler->OnConnect(Desc);

	Lock();
	m_iStatus = npsConnected;
	Unlock();
	}

void CNPClient::OnResError (const CDataPackStruct &Data)

//	OnResError
//
//	Server could not parse our command

	{
	}

void CNPClient::OnResFail (const CDataPackStruct &Data)

//	OnResFail
//
//	Command failed

	{
	int iCmd = Data.GetIntegerItem(1);
	CString sError = Data.GetStringItem(2);

	m_pHandler->OnCmdFail(iCmd, sError);
	}

void CNPClient::OnResLogin (const CDataPackStruct &Data)

//	OnResLogin
//
//	Handle RES_LOGIN

	{
	m_dwSovereignUNID = Data.GetIntegerItem(1);
	m_iStardate = Data.GetIntegerItem(2);
	m_sEmpireName = Data.GetStringItem(3);

	m_pHandler->OnLogin(m_dwSovereignUNID);
	}

void CNPClient::OnResSuccess (const CDataPackStruct &Data)

//	OnResSuccess
//
//	Command succeeded

	{
	m_pHandler->OnCmdSuccess(Data.GetIntegerItem(1));
	}

void CNPClient::OnResViewFleet (const CDataPackStruct &Data)

//	OnResViewFleet
//
//	Handles RES_VIEW_FLEET

	{
	CNPFleetView Fleet;

	Fleet.InitFromData(Data);
	m_pHandler->OnViewFleet(Fleet);
	}

void CNPClient::OnResViewNullPoint (const CDataPackStruct &Data)

//	OnResViewNullPoint
//
//	Handle RES_VIEW_NULLPOINT

	{
	CNPNullPointView NullPoint;

	NullPoint.InitFromData(Data);
	m_pHandler->OnViewNullPoint(NullPoint);
	}

void CNPClient::OnResViewSovereign (const CDataPackStruct &Data)

//	OnResViewSovereign
//
//	Handle RES_VIEW_SOVEREIGN

	{
	CNPSovereignView Sovereign;

	Sovereign.InitFromData(Data);
	m_pHandler->OnViewSovereign(Sovereign);
	}

void CNPClient::OnResViewWorld (const CDataPackStruct &Data)

//	OnResViewWorld
//
//	Handle RES_VIEW_WORLD

	{
	CNPWorldView World;

	World.InitFromData(Data);
	m_pHandler->OnViewWorld(World);
	}

void CNPClient::SendData (const CDataPackStruct &Data)

//	SendData
//
//	Sends structured data to the server

	{
	//	Prepase serialization

	CMemoryWriteStream Output(100000);

	Output.Create();
	Data.SerializeToBuffer(&Output);
	Output.Close();

	//	We're about to have a write outstanding

	Lock();

	if (m_bConnected)
		{
		if (m_iPendingWrites++ == 0)
			::ResetEvent(m_hWritesComplete);

		//	Send it

		CString sOutput(Output.GetPointer(), Output.GetLength());
		m_Connection.SendData(sOutput);
		}

	Unlock();
	}

//	INetClientEvents

void CNPClient::OnConnect (void)

//	OnConnect
//
//	Handle connection

	{
	Lock();

	ASSERT(m_iStatus == npsWaitingForConnection);
	m_iStatus = npsWaitingForGameDesc;
	m_bConnected = true;

	Unlock();

	//	Send our version information

	CDataPackStruct Msg;
	Msg.AppendInteger(CMD_REQUEST_CONNECT);
	Msg.AppendInteger(NPRPC_VERSION);
	SendData(Msg);
	}

void CNPClient::OnError (NetErrors Error, const CString &sErrorString)

//	OnConnect
//
//	Handle errors

	{
	Lock();
	NPClientStatus iStatus = m_iStatus;
	Unlock();

	switch (iStatus)
		{
		case npsWaitingForConnection:
			m_pHandler->OnError(npeUnableToConnect, LITERAL("Unable to connect to server"));
			break;

		default:
			m_pHandler->OnError(npeLostConnection, sErrorString);
		}
	}

void CNPClient::OnWriteComplete (void)

//	OnWriteComplete
//
//	Handle sending

	{
	Lock();
	if (m_iPendingWrites-- == 0)
		::SetEvent(m_hWritesComplete);
	Unlock();
	}

void CNPClient::OnDataReady (const CString &sData)

//	OnDataReady
//
//	Handle reading

	{
	//	Parse data

	CBufferReadBlock Block(sData);
	CDataPackStruct *pData;
	CDataPackStruct::CreateFromBuffer(&Block, &pData);

	//	Dispatch. The first integer is the response code

	switch (pData->GetIntegerItem(0))
		{
		case RES_CMD_ERROR:
			OnResError(*pData);
			break;

		case RES_CMD_FAIL:
			OnResFail(*pData);
			break;

		case RES_CMD_SUCCESS:
			OnResSuccess(*pData);
			break;

		case RES_CONNECT:
			OnResConnect(*pData);
			break;

		case RES_LOGIN:
			OnResLogin(*pData);
			break;

		case RES_VIEW_FLEET:
			OnResViewFleet(*pData);
			break;

		case RES_VIEW_NULLPOINT:
			OnResViewNullPoint(*pData);
			break;

		case RES_VIEW_SOVEREIGN:
			OnResViewSovereign(*pData);
			break;

		case RES_VIEW_WORLD:
			OnResViewWorld(*pData);
			break;

		default:
			ASSERT(FALSE);
		}

	//	Done

	delete pData;
	}

void CNPClient::OnDisconnect (void)

//	OnConnect
//
//	Handle disconnection

	{
	m_pHandler->OnDisconnect();

	Lock();

	m_bConnected = false;
	m_iStatus = npsDisconnected;
	m_pHandler = NULL;

	Unlock();
	}

//	Views ---------------------------------------------------------------------

ALERROR CNPFleetView::InitFromData (const CDataPackStruct &Data)
	{
	m_dwUNID = Data.GetIntegerItem(1);
	m_dwSovereign = Data.GetIntegerItem(2);
	m_dwLocation = Data.GetIntegerItem(3);
	m_dwDest = Data.GetIntegerItem(4);
	m_iInTransit = Data.GetIntegerItem(5);
	m_Assets.Load(*Data.GetStructItem(6));

	return NOERROR;
	}

ALERROR CNPNullPointView::InitFromData (const CDataPackStruct &Data)
	{
	int i;

	m_dwUNID = Data.GetIntegerItem(1);
	m_x = Data.GetIntegerItem(2);
	m_y = Data.GetIntegerItem(3);
	m_dwWorld = Data.GetIntegerItem(4);

	CDataPackStruct *pLinks = Data.GetStructItem(5);
	for (i = 0; i < pLinks->GetCount(); i++)
		m_Links.AppendElement(pLinks->GetIntegerItem(i), NULL);

	CDataPackStruct *pFleets = Data.GetStructItem(6);
	for (i = 0; i < pFleets->GetCount(); i++)
		m_Fleets.AppendElement(pFleets->GetIntegerItem(i), NULL);

	return NOERROR;
	}

ALERROR CNPSovereignView::InitFromData (const CDataPackStruct &Data)
	{
	m_dwUNID = Data.GetIntegerItem(1);
	m_sName = Data.GetStringItem(2);
	m_dwCapital = Data.GetIntegerItem(3);

	return NOERROR;
	}

ALERROR CNPWorldView::InitFromData (const CDataPackStruct &Data)
	{
	m_dwUNID = Data.GetIntegerItem(1);
	m_dwSovereign = Data.GetIntegerItem(2);
	m_dwLocation = Data.GetIntegerItem(3);
	m_iPopulation = Data.GetIntegerItem(4);
	m_iTechLevel = (NPTechLevels)Data.GetIntegerItem(5);
	m_iEfficiency = Data.GetIntegerItem(6);

	CDataPackStruct *pTraits = Data.GetStructItem(7);
	for (int i = 0; i < pTraits->GetCount(); i++)
		m_Traits.AppendElement(pTraits->GetIntegerItem(i), NULL);

	return NOERROR;
	}
