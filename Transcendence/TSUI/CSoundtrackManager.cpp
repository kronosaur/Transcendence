//	CSoundtrackManager.h
//
//	CSoundtrackManager class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define ATTRIB_COMBAT_SOUNDTRACK				CONSTLIT("combatSoundtrack")
#define ATTRIB_SYSTEM_SOUNDTRACK				CONSTLIT("systemSoundtrack")
#define ATTRIB_TRAVEL_SOUNDTRACK				CONSTLIT("travelSoundtrack")

const DWORD MIN_COMBAT_LENGTH =					300;
const DWORD MIN_TRAVEL_LENGTH =					300;

//	VOLUME_LEVEL
//
//	Exponential volume level:
//
//	Level(X) = 1000 * (1-((POWER(K,X)-1)/(POWER(K,10)-1)))
//
//	K = 1.3

const int VOLUME_LEVEL[11] =
	{
	   0,			//	Vol 0
	  23,			//	Vol 1
	  54,			//	Vol 2
	  94,			//	Vol 3
	 145,			//	Vol 4
	 212,			//	Vol 5
	 299,			//	Vol 6
	 413,			//	Vol 7
	 560,			//	Vol 8
	 751,			//	Vol 9
	1000,			//	Vol 10
	};

const int SEGMENT_BOUNDARY_THRESHOLD =			5000;	//	5 seconds

CSoundtrackManager::CSoundtrackManager (void) :
		m_bEnabled(false),
		m_iGameState(stateNone),
		m_pNowPlaying(NULL),
		m_pLastTravel(NULL),
		m_LastPlayed(10),
		m_bSystemTrackPlayed(false),
		m_bStartCombatWhenUndocked(false),
		m_bInTransition(false),
		m_dwHoldUntil(0),
		m_dwStartedCombat(0),
		m_dwStartedTravel(0)

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

CSoundType *CSoundtrackManager::CalcGameTrackToPlay (CTopologyNode *pNode, const CString &sRequiredAttrib) const

//	CalcGameTrackToPlay
//
//	Calculates a game track to play.

	{
	int i;

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

		int iChance = (pNode ? pNode->CalcMatchStrength(pTrack->GetLocationCriteria()) : (pTrack->GetLocationCriteria().MatchesAll() ? 1000 : 0));
		if (iChance == 0)
			continue;

		//	Adjust probability based on when we last played this tack.

		int iLastPlayedRank = GetLastPlayedRank(pTrack->GetUNID());
		switch (iLastPlayedRank)
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

		//	If we've played this track recently, then set its priority
		//	to the lowest.

		int iPriority = pTrack->GetPriority();
		if (iLastPlayedRank != -1 && iLastPlayedRank <= 10)
			iPriority = 0;

		//	Add to the probability table

		TProbabilityTable<CSoundType *> *pEntry = Table.SetAt(iPriority);
		pEntry->Insert(pTrack, iChance);
		}

	//	If the table is empty, then there is nothing to play.

	if (Table.GetCount() == 0)
		{
#ifdef DEBUG_SOUNDTRACK
		kernelDebugLogMessage("Unable to find soundtrack for state %d.", m_iGameState);
#endif
		return NULL;
		}

	//	Otherwise, roll out of the first table.

	TProbabilityTable<CSoundType *> &Entry = Table[0];
	CSoundType *pResult = Entry.GetAt(Entry.RollPos());

#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogMessage("State: %d: Found %d tracks in priority %d table.", m_iGameState, Table[0].GetCount(), Table.GetKey(0));
	kernelDebugLogMessage("Chose: %s", (pResult ? pResult->GetFilespec() : CONSTLIT("(none)")));
#endif

	return pResult;
	}

CSoundType *CSoundtrackManager::CalcRandomTrackToPlay (void) const

//	CalcRandomTrackToPlay
//
//	Calculates a random track to play. All tracks are treated equally, except 
//	that we decrease probabilities for tracks we've played recently.

	{
	int i;

	//	Create a probability table of tracks to play.

	TProbabilityTable<CSoundType *> Table;
	for (i = 0; i < g_pUniverse->GetSoundTypeCount(); i++)
		{
		CSoundType *pTrack = g_pUniverse->GetSoundType(i);

		//	Adjust probability based on when we last played this tack.

		int iChance = 1000;
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

		Table.Insert(pTrack, iChance);
		}

	//	If the table is empty, then there is nothing to play.

	if (Table.GetCount() == 0)
		{
#ifdef DEBUG_SOUNDTRACK
		kernelDebugLogMessage("Unable to find soundtrack for state %d.", m_iGameState);
#endif
		return NULL;
		}

	//	Otherwise, roll out of the first table.

	CSoundType *pResult = Table.GetAt(Table.RollPos());

#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogMessage("State: %d: Found %d tracks in priority %d table.", m_iGameState, Table[0].GetCount(), Table.GetKey(0));
	kernelDebugLogMessage("Chose: %s", (pResult ? pResult->GetFilespec() : CONSTLIT("(none)")));
#endif

	return pResult;
	}

CSoundType *CSoundtrackManager::CalcTrackToPlay (CTopologyNode *pNode, EGameStates iNewState) const

//	CalcTrackToPlay
//
//	Calculates the current track to play, based on game state. May return NULL 
//	if there is nothing to play.

	{
	switch (iNewState)
		{
		case stateProgramLoad:
			return m_pIntroTrack;

		case stateProgramIntro:
			return CalcRandomTrackToPlay();

		case stateGameCombat:
			return CalcGameTrackToPlay(pNode, ATTRIB_COMBAT_SOUNDTRACK);

		case stateGameTravel:
			return CalcGameTrackToPlay(pNode, ATTRIB_TRAVEL_SOUNDTRACK);

		default:
			//	For other states the caller must set a track explicitly.
			return NULL;
		}
	}

CSoundType *CSoundtrackManager::GetCurrentTrack (int *retiPos) const

//	GetCurrentTrack
//
//	Returns the current track playing
	
	{
	if (m_pNowPlaying == NULL)
		return NULL;

	if (retiPos)
		*retiPos = m_Mixer.GetCurrentPlayPos();

	return m_pNowPlaying; 
	}

int CSoundtrackManager::GetLastPlayedRank (DWORD dwUNID) const

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

bool CSoundtrackManager::IsPlayingCombatTrack (void) const

//	IsPlayingCombatTrack
//
//	Returns TRUE if we're currently playing a combat track.

	{
	return (m_pNowPlaying && m_pNowPlaying->HasAttribute(ATTRIB_COMBAT_SOUNDTRACK));
	}

void CSoundtrackManager::NextTrack (void)

//	NextTrack
//
//	Play the next track

	{
	if (m_bInTransition)
		return;

	if (m_bEnabled)
		{
		CSoundType *pTrack = CalcTrackToPlay(g_pUniverse->GetCurrentTopologyNode(), m_iGameState);
		if (pTrack == NULL)
			return;

		//	Transition

		TransitionTo(pTrack, pTrack->GetNextPlayPos());
		m_bInTransition = true;
		}
	}

void CSoundtrackManager::NotifyEndCombat (void)

//	NotifyEndCombat
//
//	Player is out of combat

	{
#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogMessage("Combat done.");
#endif

	//	If we're not in combat, then nothing to do

	if (m_iGameState != stateGameCombat)
		return;

	//	If we've been in combat for longer than the minimum time, then
	//	switch to travel. Otherwise, we stay in combat.

	if (m_bEnabled)
		{
		DWORD dwTimeInCombat = g_pUniverse->GetTicks() - m_dwStartedCombat;
		if (dwTimeInCombat < MIN_COMBAT_LENGTH)
			m_dwHoldUntil = g_pUniverse->GetTicks() + (MIN_COMBAT_LENGTH - dwTimeInCombat);
		else
			{
			m_dwHoldUntil = 0;

			if (!m_bInTransition)
				TransitionToTravel();
			}
		}

	//	Remember our state

	m_iGameState = stateGameTravel;
	}

void CSoundtrackManager::NotifyEnterSystem (CTopologyNode *pNode, bool bFirstTime)

//	NotifyEnterSystem
//
//	Player has entered a new system. At the beginning of the game this is called
//	when we appear in the system. When passing through a gate, this is called
//	just as the player hits 'G' to enter the gate (to give us a chance to
//	transition music).

	{
#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogMessage("Entered system.");
#endif

	//	If it's not the first time we've entered the system, then continue 
	//	playing.

	if (!bFirstTime)
		return;

	//	If no node passed in then assume the current node. Generally, when
	//	passing through a stargate we get an explicit node because we're still 
	//	in the old system.

	if (pNode == NULL)
		pNode = g_pUniverse->GetCurrentTopologyNode();

	//	Reset some variables.

	m_bSystemTrackPlayed = false;
	m_bStartCombatWhenUndocked = false;

	//	If we're still in the prologue then let the music continue to play until
	//	the track is done (but switch our state).

	if (m_iGameState == stateGamePrologue && m_pNowPlaying)
		{
		m_iGameState = stateGameTravel;
		return;
		}

	//	Transition out of our current track and start a new one.

	if (m_bEnabled)
		{
		CSoundType *pTrack = CalcTrackToPlay(pNode, stateGameTravel);
		if (pTrack == NULL)
			return;

		TransitionTo(pTrack, 0);
		m_bInTransition = true;
		}

	//	Remember our state

	m_iGameState = stateGameTravel;
	}

void CSoundtrackManager::NotifyStartCombat (void)

//	NotifyStartCombat
//
//	Player has just entered combat.

	{
#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogMessage("Combat started.");
#endif

	//	If we're already in combat, then nothing to do

	if (m_iGameState == stateGameCombat)
		return;

	//	Figure out how long we've been in travel mode. If long enough,
	//	then switch to combat immediately. Otherwise, we wait.

	if (m_bEnabled)
		{
		DWORD dwTimeInTravel = g_pUniverse->GetTicks() - m_dwStartedTravel;
		if (dwTimeInTravel < MIN_TRAVEL_LENGTH)
			m_dwHoldUntil = g_pUniverse->GetTicks() + (MIN_TRAVEL_LENGTH - dwTimeInTravel);
		else
			{
			m_dwHoldUntil = 0;

			if (!m_bInTransition)
				TransitionToCombat();
			}
		}

	//	Set state

	m_iGameState = stateGameCombat;
	}

void CSoundtrackManager::NotifyStartCombatMission (void)

//	NotifyStartCombatMission
//
//	Player has just started a combat mission.

	{
	}

void CSoundtrackManager::NotifyTrackDone (void)

//	NotifyTrackDone
//
//	Done playing a track

	{
#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogMessage("Track done: %s", (m_pNowPlaying ? m_pNowPlaying->GetFilespec() : CONSTLIT("(none)")));
#endif

	//	If we're transitioning then we wait for a subsequent play.

	if (m_bInTransition)
		return;

	//	Play another appropriate track

	if (m_bEnabled)
		Play(CalcTrackToPlay(g_pUniverse->GetCurrentTopologyNode(), m_iGameState));
	}

void CSoundtrackManager::NotifyTrackPlaying (CSoundType *pTrack)

//	NotifyTrackPlaying
//
//	We're now playing this track

	{
#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogMessage("Track playing: %s", (pTrack ? pTrack->GetFilespec() : CONSTLIT("(none)")));
#endif

	if (pTrack)
		{
		//	Remember that we're playing

		m_pNowPlaying = pTrack;

		//	Remember that we played this.

		m_LastPlayed.EnqueueAndOverwrite(m_pNowPlaying->GetUNID());

		//	Remember if we played the system track

		if (m_iGameState != stateProgramIntro
				&& pTrack->HasAttribute(ATTRIB_SYSTEM_SOUNDTRACK))
			m_bSystemTrackPlayed = true;

		//	Done with transition

		m_bInTransition = false;
		}
	}

void CSoundtrackManager::NotifyUndocked (void)

//	NotifyUndocked
//
//	Player has undocked.

	{
	if (m_bStartCombatWhenUndocked)
		{
		m_bStartCombatWhenUndocked = false;
		Play(CalcGameTrackToPlay(g_pUniverse->GetCurrentTopologyNode(), ATTRIB_COMBAT_SOUNDTRACK));
		}
	}

void CSoundtrackManager::NotifyUpdatePlayPos (int iPos)

//	NotifyUpdatePlayPos
//
//	Notifies the soundtrack manager that the current track has reached the
//	given play position.

	{
	//	Sometimes, if we transition in/out of combat too quickly, we get out
	//	of synch because we have to wait until we're done transitioning. Here we
	//	take the opportunity to make sure we're in the right mode.

	if (!m_bInTransition 
			&& m_bEnabled
			&& (m_dwHoldUntil == 0 || (DWORD)g_pUniverse->GetTicks() > m_dwHoldUntil))
		{
		if (IsPlayingCombatTrack())
			{
			//	If we're playing a combat track and we are not in combat, then
			//	transition to travel music

			if (m_iGameState == stateGameTravel)
				TransitionToTravel();
			}
		else
			{
			//	If we're playing a travel track and we're in combat, then 
			//	transition to combat.

			if (m_iGameState == stateGameCombat)
				TransitionToCombat();
			}

		m_dwHoldUntil = 0;
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

		m_bInTransition = true;
		m_Mixer.Play(pTrack);
		}
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
		NULL;

	//	If we're quitting, then clean up

	else if (iNewState == stateProgramQuit)
		{
		m_iGameState = stateProgramQuit;
		m_Mixer.Shutdown();
		}

	//	If we're transitioning from loading to intro, then nothing to do (other
	//	than set state)

	else if (iNewState == stateProgramIntro && m_iGameState == stateProgramLoad)
		m_iGameState = iNewState;

	//	Otherwise, set the new state

	else
		SetGameState(iNewState, CalcTrackToPlay(g_pUniverse->GetCurrentTopologyNode(), iNewState));
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
		Play(CalcTrackToPlay(g_pUniverse->GetCurrentTopologyNode(), m_iGameState));

	//	Otherwise, stop playing

	else
		m_Mixer.Stop();
	}

void CSoundtrackManager::SetPlayPaused (bool bPlay)

//	SetPlayPaused
//
//	Plays or pauses

	{
	m_Mixer.SetPlayPaused(bPlay);
	}

void CSoundtrackManager::SetVolume (int iVolume)

//	SetVolume
//
//	Sets the volume (0 to 10)

	{
	iVolume = Max(0, Min(iVolume, 10));
	m_Mixer.SetVolume(VOLUME_LEVEL[iVolume]);
	}

void CSoundtrackManager::TogglePlayPaused (void)

//	TogglePlayPaused
//
//	Pause/unpause

	{
	m_Mixer.TogglePausePlay();
	}

void CSoundtrackManager::TransitionTo (CSoundType *pTrack, int iPos, bool bFadeIn)

//	TransitionTo
//
//	Transition to play the given track, waiting for a segment boundary, if
//	necessary.

	{
	//	If we've got a current track, then we need to fade it out

	if (m_pNowPlaying)
		{
		int iCurPos = m_Mixer.GetCurrentPlayPos();
		int iEndPos = m_pNowPlaying->GetNextFadePos(iCurPos);
		if (iEndPos == -1)
			iEndPos = m_Mixer.GetCurrentPlayLength();

		//	Remember the current segment so that next time we start at the
		//	next segment.

		m_pNowPlaying->SetLastPlayPos(iCurPos);

		//	If we're more than 5 seconds away from the end, then just fade
		//	away now.

		int iTimeToEnd = iEndPos - iCurPos;
		if (iTimeToEnd > SEGMENT_BOUNDARY_THRESHOLD)
			m_Mixer.FadeNow();

		//	Otherwise, fade at the next segment boundary

		else
			m_Mixer.FadeAtPos(iEndPos);
		}

	//	Now queue up the next track

	if (bFadeIn)
		m_Mixer.PlayFadeIn(pTrack, iPos);
	else
		m_Mixer.Play(pTrack, iPos);
	}

void CSoundtrackManager::TransitionToCombat (void)

//	TransitionToCombat
//
//	Transition to a combat track.

	{
	//	If we're in a travel bed track, remember it so we can go back to it later.

	if (m_pNowPlaying 
			&& !m_pNowPlaying->HasAttribute(ATTRIB_COMBAT_SOUNDTRACK))
		m_pLastTravel = m_pNowPlaying;
	else
		m_pLastTravel = NULL;

	//	Pick a combat track

	CSoundType *pCombatTrack = CalcTrackToPlay(g_pUniverse->GetCurrentTopologyNode(), stateGameCombat);
	if (pCombatTrack == NULL)
		return;

	//	Transition

	TransitionTo(pCombatTrack, pCombatTrack->GetNextPlayPos());
	m_bInTransition = true;
	m_dwStartedCombat = g_pUniverse->GetTicks();
	}

void CSoundtrackManager::TransitionToTravel (void)

//	TransitionToTravel
//
//	Transition to a travel bed track.

	{
	CSoundType *pTrack;

	//	If we interrupted a travel track, see if we can go back to it.

	if (m_pLastTravel
			&& m_pLastTravel->GetNextPlayPos() != 0)
		pTrack = m_pLastTravel;

	//	Otherwise, calc a new track

	else
		{
		pTrack = CalcTrackToPlay(g_pUniverse->GetCurrentTopologyNode(), stateGameTravel);
		if (pTrack == NULL)
			return;
		}

	//	Transition

	TransitionTo(pTrack, pTrack->GetNextPlayPos(), true);
	m_bInTransition = true;
	m_dwStartedTravel = g_pUniverse->GetTicks();
	}

