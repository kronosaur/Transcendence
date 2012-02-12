//	ClientCommands.cpp
//
//	Implements command handlers for CNPClient class

#include "Alchemy.h"
#include "NPEngine.h"
#include "NPRPC.h"

ALERROR CNPClient::CreateUniverse (CString sPassword)

//	CreateUniverse
//
//	If a game is not currently running on this server then this command
//	will create a new universe

	{
	CDataPackStruct Msg;

	Msg.AppendInteger(CMD_CREATE_UNIVERSE);
	Msg.AppendString(sPassword);
	SendData(Msg);

	return NOERROR;
	}

ALERROR CNPClient::FleetDest (DWORD dwFleet, DWORD dwDest)

//	FleetDest
//
//	Sets a new destination for the fleet

	{
	CDataPackStruct Msg;

	Msg.AppendInteger(CMD_FLEET_DEST);
	Msg.AppendInteger(dwFleet);
	Msg.AppendInteger(dwDest);
	SendData(Msg);

	return NOERROR;
	}

ALERROR CNPClient::Login (CString sName, CString sPassword)

//	Login
//
//	Logs in as a player

	{
	CDataPackStruct Msg;

	m_sEmperorName = sName;

	Msg.AppendInteger(CMD_LOGIN);
	Msg.AppendString(sName);
	Msg.AppendString(sPassword);
	SendData(Msg);

	return NOERROR;
	}

ALERROR CNPClient::NewPlayer (CString sEmperorName, CString sPassword, CString sEmpireName)

//	NewPlayer
//
//	Create a player account

	{
	CDataPackStruct Msg;

	Msg.AppendInteger(CMD_NEW_PLAYER);
	Msg.AppendString(sEmperorName);
	Msg.AppendString(sPassword);
	Msg.AppendString(sEmpireName);
	SendData(Msg);

	return NOERROR;
	}

ALERROR CNPClient::ViewObject (DWORD dwUNID)

//	ViewObject
//
//	CMD_VIEW

	{
	CDataPackStruct Msg;

	Msg.AppendInteger(CMD_VIEW);
	Msg.AppendInteger(dwUNID);
	SendData(Msg);

	return NOERROR;
	}
