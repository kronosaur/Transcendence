//	CNPHumanPlayer.cpp
//
//	Implements CNPHumanPlayer class

#include "Alchemy.h"
#include "NPEngine.h"
#include "NPRPC.h"

CNPHumanPlayer::CNPHumanPlayer (void) : CObject(NULL)
	{
	}

ALERROR CNPHumanPlayer::Create (CNPUniverse *pUniv, 
								CString sName, 
								CString sPassword, 
								CString sEmpireName, 
								CNPWorld *pCapital, 
								CNPHumanPlayer **retpPlayer)

//	Create
//
//	Creates a new human player and its associated sovereign

	{
	ALERROR error;
	CNPHumanPlayer *pPlayer;
	CNPSovereign *pSovereign;

	//	Create the player

	pPlayer = new CNPHumanPlayer;
	if (pPlayer == NULL)
		return ERR_MEMORY;

	//	Create the sovereign

	if (error = pUniv->CreateSovereign(sEmpireName, pCapital, pUniv->GetPlayerTemplate(), &pSovereign))
		{
		delete pPlayer;
		return error;
		}

	//	Init player

	pPlayer->m_sName = sName;
	pPlayer->m_sPassword = sPassword;
	pPlayer->m_dwSovereignUNID = pSovereign->GetUNID();

	//	Done

	if (retpPlayer)
		*retpPlayer = pPlayer;

	return NOERROR;
	}
