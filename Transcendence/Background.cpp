//	Background.cpp
//
//	Background thread functions

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

DWORD WINAPI CTranscendenceWnd::BackgroundThread (LPVOID pData)

//	BackgroundThread
//
//	Background thread

	{
	CTranscendenceWnd *pThis = (CTranscendenceWnd *)pData;

	while (true)
		{
		//	Wait for work to do

		::WaitForSingleObject(pThis->m_hWorkAvailableEvent, INFINITE);

		//	Do the work

		switch (pThis->m_iBackgroundState)
			{
			case bsQuit:
				return 0;

			case bsLoadUniverse:
				pThis->BackgroundLoadUniverse();
				pThis->m_bLoadComplete = true;
				break;

			case bsCreateGame:
				pThis->BackgroundNewGame();
				pThis->m_bGameCreated = true;
				break;

			default:
				ASSERT(false);
			}

		//	Reset

		::ResetEvent(pThis->m_hWorkAvailableEvent);
		pThis->m_iBackgroundState = bsNone;
		}
	}

void CTranscendenceWnd::BackgroundLoadUniverse (void)

//	BackgroundLoadUniverse
//
//	Load the universe

	{
	//	Load the universe definition

	if (LoadUniverseDefinition() != NOERROR)
		return;

	//	Load high-score list

	ASSERT(m_pHighScoreList == NULL);
	m_pHighScoreList = new CHighScoreList;
	m_pHighScoreList->Load(CONSTLIT("HighScores.xml"));

	//	Set the default name of the player

	CString sPlayerName = m_pHighScoreList->GetMostRecentPlayerName();
	if (sPlayerName.IsBlank())
		sPlayerName = sysGetUserName();
	m_pTC->GetModel().SetPlayerName(sPlayerName);

	m_iPlayerGenome = m_pHighScoreList->GetMostRecentPlayerGenome();
	if (m_iPlayerGenome == genomeUnknown)
		m_iPlayerGenome = (mathRandom(1, 2) == 2 ? genomeHumanMale : genomeHumanFemale);

	//	Set the default adventure

	m_dwAdventure = 0;

	//	Set the default player ship

	m_dwPlayerShip = GetDefaultPlayerShip();
	if (m_dwPlayerShip == 0)
		m_sBackgroundError = CONSTLIT("No valid player ship class");
	}

void CTranscendenceWnd::BackgroundNewGame (void)

//	BackgroundNewGame
//
//	Complete creating a new game

	{
	if (StartNewGameBackground(&m_sBackgroundError) != NOERROR)
		{
		g_pUniverse->Reinit();
		GetGameFile().Close();
		return;
		}
	}

void CTranscendenceWnd::CreateBackgroundThread (void)

//	CreateBackgroundThread
//
//	Creates the background thread

	{
	m_hWorkAvailableEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hBackgroundThread = ::kernelCreateThread(BackgroundThread, this);
	}

void CTranscendenceWnd::DestroyBackgroundThread (void)

//	DestroyBackgroundThread
//
//	Destroy the background thread

	{
	SetBackgroundState(bsQuit);
	::WaitForSingleObject(m_hBackgroundThread, 5000);
	::CloseHandle(m_hWorkAvailableEvent);
	::CloseHandle(m_hBackgroundThread);
	}

void CTranscendenceWnd::SetBackgroundState (BackgroundState iState)

//	SetBackgroundState
//
//	Called by foreground thread when we want to do work

	{
	m_iBackgroundState = iState;
	::SetEvent(m_hWorkAvailableEvent);
	}
