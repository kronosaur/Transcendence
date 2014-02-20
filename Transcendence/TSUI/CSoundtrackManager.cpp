//	CSoundtrackManager.h
//
//	CSoundtrackManager class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define ATTRIB_COMBAT_SOUNDTRACK				CONSTLIT("combatSoundtrack")
#define ATTRIB_SYSTEM_SOUNDTRACK				CONSTLIT("systemSoundtrack")
#define ATTRIB_TRAVEL_SOUNDTRACK				CONSTLIT("travelSoundtrack")

const int SEGMENT_BOUNDARY_THRESHOLD =			5000;	//	5 seconds

CSoundtrackManager::CSoundtrackManager (void) :
		m_bEnabled(false),
		m_iGameState(stateNone),
		m_pNowPlaying(NULL),
		m_LastPlayed(10),
		m_bSystemTrackPlayed(false),
		m_bStartCombatWhenUndocked(false),
		m_bInTransition(false)

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

CSoundType *CSoundtrackManager::CalcTrackToPlay (CTopologyNode *pNode, EGameStates iNewState) const

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

		case stateGameCombat:
			return CalcGameTrackToPlay(pNode, ATTRIB_COMBAT_SOUNDTRACK);

		case stateGameTravel:
			return CalcGameTrackToPlay(pNode, ATTRIB_TRAVEL_SOUNDTRACK);

		default:
			//	For other states the caller must set a track explicitly.
			return NULL;
		}
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

void CSoundtrackManager::NotifyEndCombat (void)

//	NotifyEndCombat
//
//	Player is out of combat

	{
#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogMessage("Combat done.");
#endif

	//	If we're not in combat, then nothing to do

	if (m_iGameState != stateGameCombat
			|| m_bInTransition)
		return;

	//	Figure out which track to play. If there is nothing to play then we're
	//	done.

	if (m_bEnabled)
		{
		CSoundType *pTrack = CalcTrackToPlay(g_pUniverse->GetCurrentTopologyNode(), stateGameTravel);
		if (pTrack == NULL)
			return;

		//	Transition

		TransitionTo(pTrack, pTrack->GetNextPlayPos());
		m_bInTransition = true;
		}

	//	Remember our state

	m_iGameState = stateGameTravel;
	}

void CSoundtrackManager::NotifyEnterSystem (CTopologyNode *pNode)

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

		TransitionTo(pTrack, pTrack->GetNextPlayPos());
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

	if (m_iGameState == stateGameCombat
			|| m_bInTransition)
		return;

	//	Figure out which track to play. If there is nothing to play then we're
	//	done.

	if (m_bEnabled)
		{
		CSoundType *pCombatTrack = CalcTrackToPlay(g_pUniverse->GetCurrentTopologyNode(), stateGameCombat);
		if (pCombatTrack == NULL)
			return;

		//	Transition

		TransitionTo(pCombatTrack, pCombatTrack->GetNextPlayPos());
		m_bInTransition = true;
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

		if (pTrack->HasAttribute(ATTRIB_SYSTEM_SOUNDTRACK))
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
		return;

	//	Set new state

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
		Play(CalcTrackToPlay(g_pUniverse->GetCurrentTopologyNode(), m_iGameState));

	//	Otherwise, stop playing

	else
		m_Mixer.Stop();
	}

void CSoundtrackManager::TogglePlayPaused (void)

//	TogglePlayPaused
//
//	Pause/unpause

	{
	m_Mixer.TogglePausePlay();
	}

void CSoundtrackManager::TransitionTo (CSoundType *pTrack, int iPos)

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

	m_Mixer.Play(pTrack, iPos);
	}
