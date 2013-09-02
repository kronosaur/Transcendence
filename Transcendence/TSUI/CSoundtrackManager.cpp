//	CSoundtrackManager.h
//
//	CSoundtrackManager class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define ATTRIB_COMBAT_SOUNDTRACK				CONSTLIT("combatSoundtrack")
#define ATTRIB_SYSTEM_SOUNDTRACK				CONSTLIT("systemSoundtrack")
#define ATTRIB_TRAVEL_SOUNDTRACK				CONSTLIT("travelSoundtrack")

CSoundtrackManager::CSoundtrackManager (void) :
		m_bEnabled(false),
		m_iGameState(stateNone),
		m_pNowPlaying(NULL),
		m_LastPlayed(10),
		m_bSystemTrackPlayed(false),
		m_bStartCombatWhenUndocked(false)

//	CSoundtrackManager constructor

	{
	//	Hardcoded intro soundtrack

	m_pIntroTrack = new CSoundType;
	m_pIntroTrack->Init(0xFFFFFFFF, CONSTLIT("TranscendenceIntro.mp3"));
	}

CSoundtrackManager::~CSoundtrackManager (void)

//	CSoundtrackManager destructor

	{
	if (m_pIntroTrack)
		delete m_pIntroTrack;
	}

CSoundType *CSoundtrackManager::CalcGameTrackToPlay (const CString &sRequiredAttrib)

//	CalcGameTrackToPlay
//
//	Calculates a game track to play.

	{
	int i;
	CSystem *pSystem = g_pUniverse->GetCurrentSystem();

	//	Create a probability table of tracks to play.

	TSortMap<int, TProbabilityTable<CSoundType *>> Table(DescendingSort);
	for (i = 0; i < g_pUniverse->GetSoundTypeCount(); i++)
		{
		CSoundType *pTrack = g_pUniverse->GetSoundType(i);

		//	If this is not appropriate music then skip it

		if (!pTrack->HasAttribute(sRequiredAttrib))
			continue;

		//	If this is a system track and we've already played a
		//	system track, then skip it.

		else if (m_bSystemTrackPlayed
				&& pTrack->HasAttribute(ATTRIB_SYSTEM_SOUNDTRACK))
			continue;

		//	Calculate the chance for this track based on location
		//	criteria.

		int iChance = (pSystem ? pSystem->CalcMatchStrength(pTrack->GetLocationCriteria()) : 1000);
		if (iChance == 0)
			continue;

		//	Adjust probability based on when we last played this tack.

		switch (GetLastPlayedRank(pTrack->GetUNID()))
			{
			case 0:
				iChance = 0;
				break;

			case 1:
				iChance = iChance / 10;
				break;

			case 2:
				iChance = iChance / 5;
				break;

			case 3:
			case 4:
				iChance = iChance / 3;
				break;

			case 5:
			case 6:
			case 7:
				iChance = iChance / 2;
				break;

			case 8:
			case 9:
			case 10:
				iChance = 2 * iChance / 3;
				break;
			}

		if (iChance == 0)
			continue;

		//	Add to the probability table

		TProbabilityTable<CSoundType *> *pEntry = Table.SetAt(pTrack->GetPriority());
		pEntry->Insert(pTrack, iChance);
		}

	//	If the table is empty, then there is nothing to play.

	if (Table.GetCount() == 0)
		return NULL;

	//	Otherwise, roll out of the first table.

	TProbabilityTable<CSoundType *> &Entry = Table[0];
	return Entry.GetAt(Entry.RollPos());
	}

CSoundType *CSoundtrackManager::CalcTrackToPlay (EGameStates iNewState)

//	CalcTrackToPlay
//
//	Calculates the current track to play, based on game state. May return NULL 
//	if there is nothing to play.

	{
	switch (iNewState)
		{
		case stateProgramLoad:
		case stateProgramIntro:
			return m_pIntroTrack;

		case stateGameTravel:
			return CalcGameTrackToPlay(ATTRIB_TRAVEL_SOUNDTRACK);

		default:
			//	For other states the caller must set a track explicitly.
			return m_pNowPlaying;
		}
	}

int CSoundtrackManager::GetLastPlayedRank (DWORD dwUNID)

//	GetLastPlayedRank
//
//	Returns the last time the given track was played.
//
//	0 = most recent track played.
//	1 = one track played after this one.
//	2 = two tracks played after this one.
//	...
//	-1 = Played more than 10 track ago (or never played).

	{
	int i;

	for (i = m_LastPlayed.GetCount() - 1; i >= 0; i--)
		{
		if (m_LastPlayed[i] == dwUNID)
			return (m_LastPlayed.GetCount() - 1 - i);
		}

	//	Not on our list, so treat as if it has never played.

	return -1;
	}

void CSoundtrackManager::NotifyEnterSystem (void)

//	NotifyEnterSystem
//
//	Player has entered a new system.

	{
	m_bSystemTrackPlayed = false;
	m_bStartCombatWhenUndocked = false;

	//	We're not already in travel mode, we set us there.

	if (m_iGameState != stateGameTravel)
		{
		//	Unless we're transitioning from the prologue,
		//	stop playing current track

		if (m_iGameState != stateGamePrologue)
			{
			g_pHI->GetSoundMgr().StopMusic();
			m_pNowPlaying = NULL;
			}

		m_iGameState = stateGameTravel;
		}
	
	//	If we're not playing anything, try to play something

	if (m_pNowPlaying == NULL)
		Play(CalcTrackToPlay(m_iGameState));
	}

void CSoundtrackManager::NotifyStartCombatMission (void)

//	NotifyStartCombatMission
//
//	Player has just started a combat mission.

	{
	//	If we're already playing a combat track then nothing to do.

	if (m_pNowPlaying
			&& m_pNowPlaying->HasAttribute(ATTRIB_COMBAT_SOUNDTRACK))
		return;

	//	If the player is currently docked, then wait until we undock to play
	//	the appropriate track.

	if (g_pUniverse->GetPlayer()
			&& g_pUniverse->GetPlayer()->GetDockedObj())
		{
		m_bStartCombatWhenUndocked = true;
		return;
		}

	//	Start playing a combat track

	Play(CalcGameTrackToPlay(ATTRIB_COMBAT_SOUNDTRACK));
	}

void CSoundtrackManager::NotifyTrackDone (void)

//	NotifyTrackDone
//
//	Done playing a track

	{
	//	Play another appropriate track

	Play(CalcTrackToPlay(m_iGameState));
	}

void CSoundtrackManager::NotifyUndocked (void)

//	NotifyUndocked
//
//	Player has undocked.

	{
	if (m_bStartCombatWhenUndocked)
		{
		m_bStartCombatWhenUndocked = false;
		Play(CalcGameTrackToPlay(ATTRIB_COMBAT_SOUNDTRACK));
		}
	}

void CSoundtrackManager::Play (CSoundType *pTrack)

//	Play
//
//	Plays the given track.

	{
	//	If we're already playing this track, then nothing to do.

	if (pTrack == m_pNowPlaying)
		return;

	//	Play

	if (m_bEnabled
			&& pTrack)
		{
		CString sFilespec = pTrack->GetFilespec();
		if (sFilespec.IsBlank())
			{
			::kernelDebugLogMessage("Unable to find soundtrack: %x", pTrack->GetUNID());
			return;
			}

		CString sError;
		if (!g_pHI->GetSoundMgr().PlayMusic(sFilespec, 0, &sError))
			{
			::kernelDebugLogMessage("Unable to play soundtrack %s: %s", sFilespec, sError);
			return;
			}
		}

	//	Remember what we're playing

	if (m_pNowPlaying)
		m_LastPlayed.EnqueueAndOverwrite(m_pNowPlaying->GetUNID());

	if (pTrack && pTrack->HasAttribute(ATTRIB_SYSTEM_SOUNDTRACK))
		m_bSystemTrackPlayed = true;

	m_pNowPlaying = pTrack;
	}

void CSoundtrackManager::Reinit (void)

//	Reinit
//
//	Reinitialize now playing state, etc.

	{
	m_LastPlayed.DeleteAll();
	m_bSystemTrackPlayed = false;
	m_bStartCombatWhenUndocked = false;
	}

void CSoundtrackManager::SetGameState (EGameStates iNewState)

//	SetGameState
//
//	Sets the current game state and determines which track to play.

	{
	//	If our state has not changed, then nothing to do

	if (iNewState == m_iGameState)
		return;

	//	Set new state

	SetGameState(iNewState, CalcTrackToPlay(iNewState));
	}

void CSoundtrackManager::SetGameState (EGameStates iNewState, CSoundType *pTrack)

//	SetGameState
//
//	Sets the current game state and starts playing the given track.

	{
	//	If our state has not changed, then nothing to do

	if (iNewState == m_iGameState
			&& pTrack == m_pNowPlaying)
		return;

	//	Set our state

	m_iGameState = iNewState;

	//	If we're back at program intro then reinit our playlist.

	if (iNewState == stateProgramIntro)
		Reinit();

	//	Play the soundtrack

	Play(pTrack);
	}

void CSoundtrackManager::SetMusicEnabled (bool bEnabled)

//	SetMusicEnabled
//
//	Enable/disable playing music

	{
	if (m_bEnabled == bEnabled)
		return;

	m_bEnabled = bEnabled;

	//	If we're enabling music, play the current track

	if (m_bEnabled)
		Play(CalcTrackToPlay(m_iGameState));

	//	Otherwise, stop playing

	else
		{
		g_pHI->GetSoundMgr().StopMusic();
		m_pNowPlaying = NULL;
		}
	}

void CSoundtrackManager::TogglePlayPaused (void)

//	TogglePlayPaused
//
//	Pause/unpause

	{
	g_pHI->GetSoundMgr().TogglePlayPaused();
	}
