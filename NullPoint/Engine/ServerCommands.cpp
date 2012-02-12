//	ServerCommands.cpp
//
//	Implements command handlers for CNPServer class

#include "Alchemy.h"
#include "NPEngine.h"
#include "NPRPC.h"

CString CNPServer::ComposeCmdError (ClientCtx *pClient, int iCmd, ALERROR error)

//	ComposeCmdError
//
//	Composes a RES_CMD_FAIL message with an error code

	{
	CString sError;

	//	Get the error string

	switch (error)
		{
		case NPERROR_FLEET_IN_TRANSIT:
			sError = CONSTLIT("Fleet is in transit");
			break;

		case NPERROR_FLEET_NO_HYPERDRIVE:
			sError = CONSTLIT("Fleet cannot enter hyperspace");
			break;

		default:
			sError = CONSTLIT("Unexpected server error.");
		}

	//	Log the error

	char szBuffer[1024];
	wsprintf(szBuffer, "[%d] CMDFAIL(%d): %s: %d", pClient->dwClientID, iCmd, sError.GetASCIIZPointer(), error);
	Log(szBuffer);

	//	Send reply

	CDataPackStruct Reply;
	Reply.AppendInteger(RES_CMD_FAIL);
	Reply.AppendInteger(iCmd);
	Reply.AppendString(sError);

	return Reply.SerializeToString();
	}

CString CNPServer::ComposeCmdFail (ClientCtx *pClient, int iCmd, CString sMessage)

//	ComposeCmdFail
//
//	Composes a RES_CMD_FAIL message

	{
	//	Report as error

	char szBuffer[1024];
	wsprintf(szBuffer, "[%d] CMDFAIL(%d): %s", pClient->dwClientID, iCmd, sMessage.GetASCIIZPointer());
	Log(szBuffer);

	//	Send reply

	CDataPackStruct Reply;
	Reply.AppendInteger(RES_CMD_FAIL);
	Reply.AppendInteger(iCmd);
	Reply.AppendString(sMessage);

	return Reply.SerializeToString();
	}

CString CNPServer::ComposeCmdSuccess (ClientCtx *pClient, int iCmd)

//	ComposeCmdSuccess
//
//	Composes a RES_CMD_SUCCESS message

	{
	CDataPackStruct Reply;
	Reply.AppendInteger(RES_CMD_SUCCESS);
	Reply.AppendInteger(iCmd);

	return Reply.SerializeToString();
	}

CString CNPServer::CmdCreateUniverse (ClientCtx *pClient, const CDataPackStruct &Data)
	{
	ALERROR error;
	CString sReply;

	//	Get parameters

	CString sPassword = Data.GetStringItem(1);
	CString sTemplate = LITERAL("NPUniverse.xml");
	Log("[%d] CreateUniverse using '%s'", pClient->dwClientID, sTemplate.GetASCIIZPointer());

	Lock();

	//	If the universe already exists then we're done

	if (m_pUniv != NULL)
		{
		Unlock();
		return ComposeCmdFail(pClient, CMD_CREATE_UNIVERSE, LITERAL("The Universe already exists."));
		}

	//	Create the universe

	CString sError;
	if (error = CNPUniverse::CreateUniverse(sTemplate, &m_pUniv, &sError))
		{
		Unlock();
		return ComposeCmdFail(pClient, CMD_CREATE_UNIVERSE, sError);
		}

	//	Initialize some stuff

	m_sOverlordPassword = sPassword;
	m_dwGameUNID = 666;
	m_bOpenAccess = true;

	Unlock();
	return ComposeCmdSuccess(pClient, CMD_CREATE_UNIVERSE);
	}

CString CNPServer::CmdFleetDest (ClientCtx *pClient, const CDataPackStruct &Data)
	{
	ALERROR error;
	CString sError;

	//	Get parameters

	DWORD dwFleet = Data.GetIntegerItem(1);
	DWORD dwDest = Data.GetIntegerItem(2);

	Lock();

	//	This better be a fleet that we can command

	CNPFleet *pFleet;
	if (!ValidFleetAccess(pClient, dwFleet, true, &pFleet, &sError))
		{
		Unlock();
		return sError;
		}

	//	The null point better be known or adjacent to a known
	//	null point

	CNPNullPoint *pNP;
	if (!ValidNullPointAccess(pClient, dwDest, true, &pNP, &sError))
		{
		Unlock();
		return sError;
		}

	//	Do it

	if (error = m_pUniv->EnterHyperspace(pFleet, pNP))
		{
		Unlock();
		return ComposeCmdError(pClient, CMD_FLEET_DEST, error);
		}

	Unlock();
	return ComposeCmdSuccess(pClient, CMD_FLEET_DEST);
	}

CString CNPServer::CmdLogin (ClientCtx *pClient, const CDataPackStruct &Data)
	{
	//	If we're already logged in, then fail (for now)

	if (pClient->ACL != aclNoLogin)
		return ComposeCmdFail(pClient, CMD_LOGIN, LITERAL("You are already logged in."));

	//	Get the data

	CString sName = Data.GetStringItem(1);
	CString sPassword = Data.GetStringItem(2);

	Lock();

	//	Check for overlord

	if (strCompare(sName, LITERAL("overlord")) == 0)
		{
		//	Check the password

		if (strCompare(sPassword, m_sOverlordPassword) != 0)
			{
			Unlock();
			return ComposeCmdFail(pClient, CMD_LOGIN, LITERAL("Invalid name or password."));
			}

		//	Complete login

		pClient->sName = LITERAL("Overlord");
		pClient->ACL = aclOverlord;
		pClient->pSovereign = m_pUniv->GetSovereign(m_pUniv->GetOverlordUNID());

		Log("[%d] Login as Overlord", pClient->dwClientID);
		}

	//	Check for guest

	else if (strCompare(sName, LITERAL("guest")) == 0)
		{
		//	Not allowed, for now

		Unlock();
		return ComposeCmdFail(pClient, CMD_LOGIN, LITERAL("Guest access not allowed in this universe."));
		}

	//	A player

	else
		{
		//	Look-up the player

		CNPHumanPlayer *pPlayer;
		if (m_Players.Lookup(sName, (CObject **)&pPlayer) != NOERROR)
			{
			Unlock();
			return ComposeCmdFail(pClient, CMD_LOGIN, LITERAL("Invalid name or password."));
			}

		//	Check the password

		if (strCompare(sPassword, pPlayer->GetPassword()) != 0)
			{
			Unlock();
			return ComposeCmdFail(pClient, CMD_LOGIN, LITERAL("Invalid name or password."));
			}

		//	Complete login

		pClient->sName = sName;
		pClient->ACL = aclPlayer;
		pClient->pSovereign = m_pUniv->GetSovereign(pPlayer->GetSovereignID());

		Log("[%d] Login as %s", pClient->dwClientID, sName.GetASCIIZPointer());
		}

	//	Compose reply

	CDataPackStruct Reply;
	Reply.AppendInteger(RES_LOGIN);
	Reply.AppendInteger(pClient->pSovereign->GetUNID());
	Reply.AppendInteger(m_pUniv->GetTurn());
	Reply.AppendString(pClient->pSovereign->GetName());

	Unlock();

	return Reply.SerializeToString();
	}

CString CNPServer::CmdNewPlayer (ClientCtx *pClient, const CDataPackStruct &Data)
	{
	ALERROR error;

	//	Unless we're overlord or the universe has open access
	//	we cannot create a new player

	if (pClient->ACL != aclOverlord	&& !m_bOpenAccess)
		return ComposeCmdFail(pClient, CMD_NEW_PLAYER, LITERAL("You are not authorized to create new player accounts."));

	//	Create the new player

	CString sEmperorName = Data.GetStringItem(1);
	CString sPassword = Data.GetStringItem(2);
	CString sEmpireName = Data.GetStringItem(3);

	Lock();

	//	Make sure there isn't a player with that name already

	if (m_Players.Lookup(sEmperorName, NULL) == NOERROR)
		{
		Unlock();
		return ComposeCmdFail(pClient, CMD_NEW_PLAYER, LITERAL("A player with that name already exists."));
		}

	//	Pick a capital for this player

	int i;
	CNPWorld *pCapital = NULL;
	for (i = 0; i < m_pUniv->GetNullPointCount(); i++)
		{
		CNPNullPoint *pNP = m_pUniv->GetNullPointByIndex(i);

		int j;
		for (j = 0; j < pNP->GetWorldCount(); j++)
			{
			CNPWorld *pWorld = pNP->GetWorld(j);

			if (pWorld->HasTrait(traitReservedCapital)
					&& pWorld->GetSovereign()->GetUNID() == m_pUniv->GetIndependentUNID())
				{
				pCapital = pWorld;
				break;
				}
			}

		if (pCapital)
			break;
		}

	//	If we could not find a capital then we cannot create a new empire

	if (pCapital == NULL)
		{
		Unlock();
		return ComposeCmdFail(pClient, CMD_NEW_PLAYER, LITERAL("A world suitable for an empire's capital could not be found."));
		}

	//	Create the player

	CNPHumanPlayer *pPlayer;
	if (error = CNPHumanPlayer::Create(m_pUniv, sEmperorName, sPassword, sEmpireName, pCapital, &pPlayer))
		{
		Unlock();
		return ComposeCmdError(pClient, CMD_NEW_PLAYER, error);
		}

	//	Add it to our list

	if (error = m_Players.AddEntry(sEmperorName, pPlayer))
		{
#ifdef LATER
		//	delete the sovereign and the player
#endif
		Unlock();
		return ComposeCmdError(pClient, CMD_NEW_PLAYER, error);
		}

	Unlock();

	Log("[%d] New account created for %s", pClient->dwClientID, sEmperorName.GetASCIIZPointer());
	return ComposeCmdSuccess(pClient, CMD_NEW_PLAYER);
	}

CString CNPServer::CmdRequestConnect (ClientCtx *pClient, const CDataPackStruct &Data)
	{
	CDataPackStruct Reply;

	//	Get parameters

	pClient->iRPCVersion = Data.GetIntegerItem(1);
	Log("[%d] RequestConnect: RPCVersion = %d", pClient->dwClientID, pClient->iRPCVersion);

	//	If we don't have a universe then the game UNID is 0

	Lock();
	Reply.AppendInteger(RES_CONNECT);
	Reply.AppendInteger((DWORD)(SERVER_MAJOR_VERSION << 16) | (DWORD)SERVER_MINOR_VERSION);	//	Universe version
	if (m_pUniv == NULL)
		{
		Reply.AppendInteger(0);	//	Game UNID
		Reply.AppendString(LITERAL(""));	//	Name
		Reply.AppendInteger(0);	//	Turns
		Reply.AppendInteger(0);	//	Null points
		Reply.AppendInteger(0);	//	Players
		Reply.AppendInteger(0);	//	Flags
		}
	else
		{
		Reply.AppendInteger(m_dwGameUNID);	//	Game UNID
		Reply.AppendString(LITERAL("Reconstruction: 4021"));	//	Name
		Reply.AppendInteger(m_pUniv->GetTurn());	//	Turns
		Reply.AppendInteger(m_pUniv->GetNullPointCount());	//	Null points
		Reply.AppendInteger(m_pUniv->GetSovereignCount());	//	Players
		Reply.AppendInteger(0);	//	Flags
		}
	Unlock();

	//	Done

	return Reply.SerializeToString();
	}

CString CNPServer::CmdView (ClientCtx *pClient, const CDataPackStruct &Data)
	{
	CDataPackStruct Reply;
	CString sError;

	//	Get parameters

	DWORD dwUNID = Data.GetIntegerItem(1);

	Lock();

	//	If we don't know about this object then we can't view it

	if (!ValidAccess(pClient, dwUNID))
		{
		Unlock();
		return ComposeCmdFail(pClient, CMD_VIEW, LITERAL("Unknown object."));
		}

	//	Return data

	switch (GetUNIDType(dwUNID))
		{
		case unidFleet:
			{
			CNPFleet *pFleet;
			if (!ValidFleetAccess(pClient, dwUNID, false, &pFleet, &sError))
				{
				Unlock();
				return sError;
				}

			Reply.AppendInteger(RES_VIEW_FLEET);
			Reply.AppendInteger(dwUNID);
			Reply.AppendInteger(pFleet->GetSovereign()->GetUNID());
			Reply.AppendInteger(pFleet->GetLocation());
			Reply.AppendInteger(pFleet->GetDest());
			Reply.AppendInteger(pFleet->GetInTransit());

			CDataPackStruct *pAssets;
			Reply.AppendNewStruct(&pAssets);
			pFleet->GetAssetList().Save(pAssets);
			break;
			}

		case unidNullPoint:
			{
			int i;

			CNPNullPoint *pNP;
			if (!ValidNullPointAccess(pClient, dwUNID, false, &pNP, &sError))
				{
				Unlock();
				return sError;
				}

			Reply.AppendInteger(RES_VIEW_NULLPOINT);
			Reply.AppendInteger(dwUNID);
			Reply.AppendInteger(pNP->GetX());
			Reply.AppendInteger(pNP->GetY());
			Reply.AppendInteger(pNP->GetWorld(0)->GetUNID());

			//	Links
			CDataPackStruct *pLinks;
			Reply.AppendNewStruct(&pLinks);
			for (i = 0; i < pNP->GetLinkCount(); i++)
				{
				CNPNullPoint *pDest = pNP->GetLinkDest(i);
				pLinks->AppendInteger(pDest->GetUNID());
				}

			//	Fleets
			CDataPackStruct *pFleets;
			Reply.AppendNewStruct(&pFleets);
			CNPFleet *pNextFleet = pNP->GetFirstFleet();
			while (pNextFleet)
				{
				if (ValidAccess(pClient, pNextFleet->GetUNID()))
					pFleets->AppendInteger(pNextFleet->GetUNID());

				pNextFleet = pNextFleet->GetNextFleet();
				}
			break;
			}

		case unidSovereign:
			{
			CNPSovereign *pSovereign = m_pUniv->GetSovereign(dwUNID);
			if (pSovereign == NULL)
				{
				Unlock();
				return ComposeCmdFail(pClient, CMD_VIEW, LITERAL("Unexpected error: object not found."));
				}

			Reply.AppendInteger(RES_VIEW_SOVEREIGN);
			Reply.AppendInteger(dwUNID);
			Reply.AppendString(pSovereign->GetName());
			Reply.AppendInteger(pSovereign->GetCapital());
			break;
			}

		case unidWorld:
			{
			CNPWorld *pWorld = m_pUniv->GetWorld(dwUNID);
			if (pWorld == NULL)
				{
				Unlock();
				return ComposeCmdFail(pClient, CMD_VIEW, LITERAL("Unexpected error: object not found."));
				}

			Reply.AppendInteger(RES_VIEW_WORLD);
			Reply.AppendInteger(dwUNID);
			Reply.AppendInteger(pWorld->GetSovereign()->GetUNID());
			Reply.AppendInteger(pWorld->GetLocation());
			Reply.AppendInteger(pWorld->GetPopulation());
			Reply.AppendInteger(pWorld->GetTechLevel());
			Reply.AppendInteger(pWorld->GetEfficiency());

			CIntArray Traits;
			pWorld->EnumTraits(&Traits);
			Reply.AppendIntArray(Traits);
			break;
			}

		default:
			Unlock();
			return ComposeCmdFail(pClient, CMD_VIEW, LITERAL("Unknown object."));
		}

	Unlock();

	return Reply.SerializeToString();
	}
