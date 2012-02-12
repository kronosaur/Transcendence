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

			case bsBlt:
				pThis->BackgroundBlt(&pThis->m_Secondary);
				break;

			default:
				ASSERT(false);
			}

		//	Reset

		::ResetEvent(pThis->m_hWorkAvailableEvent);
		pThis->m_iBackgroundState = bsNone;
		}
	}

void CTranscendenceWnd::BackgroundBlt (CG16bitImage *pScreen)

//	BackgroundBlt
//
//	Blt the screen

	{
	//	First blt to a surface that conforms to the primary screen res

	pScreen->BltToSurface(m_pBack, m_PrimaryType);

	//	Now blt the surface

	RECT    rcSrc;  // source blit rectangle
	RECT    rcDest; // destination blit rectangle
	POINT   p;

	// find out where on the primary surface our window lives
	p.x = 0; p.y = 0;
	::ClientToScreen(m_hWnd, &p);
	::GetClientRect(m_hWnd, &rcDest);
	OffsetRect(&rcDest, p.x, p.y);
	SetRect(&rcSrc, 0, 0, g_cxScreen, g_cyScreen);

	m_pPrimary->Blt(&rcDest, m_pBack, &rcSrc, DDBLT_ASYNC, NULL);
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

	m_sPlayerName = m_pHighScoreList->GetMostRecentPlayerName();
	if (m_sPlayerName.IsBlank())
		m_sPlayerName = sysGetUserName();

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
		m_Universe.Reinit();
		m_GameFile.Close();
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
