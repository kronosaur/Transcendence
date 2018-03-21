//	CTranscendencePlayer.cpp
//
//	CTranscendencePlayer class
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

CTranscendencePlayer::CTranscendencePlayer (void) : m_pPlayer(NULL)

//	CTranscendencePlayer constructor

	{
	}

GenomeTypes CTranscendencePlayer::GetGenome (void) const

//	GetGenome
//
//	Returns the player genome

	{
	return m_pPlayer->GetPlayerGenome();
	}

CString CTranscendencePlayer::GetName (void) const

//	GetName
//
//	Returns the player's name

	{
	return m_pPlayer->GetPlayerName();
	}

IPlayerController::EUIMode CTranscendencePlayer::GetUIMode (void) const

//	GetUIMode
//
//	Returns the current UI mode.

	{
	CGameSession *pSession = m_pPlayer->GetGameSession();
	if (pSession == NULL)
		return uimodeUnknown;

	return pSession->GetUIMode();
	}

void CTranscendencePlayer::OnMessageFromObj (CSpaceObject *pSender, const CString &sMessage)

//	OnMessageFromObj
//
//	Receive a message.

	{
	if (pSender 
			&& !pSender->IsVirtual() 
			&& !pSender->IsMission()
			&& !pSender->IsDestroyed())
		pSender->Highlight(sMessage);
	else
		g_pTrans->DisplayMessage(sMessage);
	}
