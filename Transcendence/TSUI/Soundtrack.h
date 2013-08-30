//	Soundtrack.h
//
//	Classes and methods for playing music
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CSoundtrackManager
	{
	public:
		enum EGameStates
			{
			stateNone,

			stateProgramLoad,				//	Loading the program
			stateProgramIntro,				//	In intro screen
			stateGamePrologue,				//	Creating a new game
			stateGameTravel,				//	Normal play; travel music
			stateGameCombat,				//	Combat music
			stateGameEpitaph,				//	Game over
			};

		CSoundtrackManager (void);
		~CSoundtrackManager (void);

		void NotifyEnterSystem (void);
		void NotifyStartCombatMission (void);
		void NotifyTrackDone (void);
		void NotifyUndocked (void);
		void SetGameState (EGameStates iNewState);
		void SetGameState (EGameStates iNewState, CSoundType *pTrack);
		void SetMusicEnabled (bool bEnabled = true);
		void TogglePlayPaused (void);

	private:
		CSoundType *CalcGameTrackToPlay (const CString &sRequiredAttrib);
		CSoundType *CalcTrackToPlay (EGameStates iNewState);
		int GetLastPlayedRank (DWORD dwUNID);
		void Play (CSoundType *pTrack);
		void Reinit (void);

		bool m_bEnabled;					//	Music is enabled
		EGameStates m_iGameState;			//	Current soundtrack state
		CSoundType *m_pNowPlaying;			//	Now playing

		TQueue<DWORD> m_LastPlayed;			//	UNID of tracks played.
		bool m_bSystemTrackPlayed;			//	systemSoundtrack already played in system.
		bool m_bStartCombatWhenUndocked;	//	If TRUE, we play combat music when we undock

		CSoundType *m_pIntroTrack;			//	Track to play for intro.
	};