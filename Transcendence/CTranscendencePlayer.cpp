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
