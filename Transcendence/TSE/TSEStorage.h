//	Storage.h
//
//	Classes and methods for saving Transcendence files
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CGameFile
	{
	public:
		enum Flags
			{
			FLAG_CHECKPOINT =						0x00000001,
			FLAG_ENTER_GATE =						0x00000002,
			FLAG_EXIT_GATE =						0x00000004,
			FLAG_ACCEPT_MISSION =					0x00000008,
			};

		CGameFile (void);
		~CGameFile (void);

		ALERROR ClearRegistered (void);
		void Close (void);
		ALERROR Create (const CString &sFilename, const CString &sUsername);
		static CString GenerateFilename (const CString &sName);
		inline DWORD GetAdventure (void) const { return m_Header.dwAdventure; }
		inline DWORD GetCreateVersion (void) const { return m_Header.dwCreateVersion; }
		inline CString GetEpitaph (void) const { return CString((char *)m_Header.szEpitaph); }
		inline CString GetFilespec (void) const { return m_pFile->GetFilename(); }
		inline CString GetGameID (void) { return CString(m_Header.szGameID); }
		inline GenomeTypes GetPlayerGenome (void) const { return (GenomeTypes)m_Header.dwGenome; }
		CString GetPlayerName (void) const;
		inline DWORD GetPlayerShip (void) const { return m_Header.dwPlayerShip; }
		inline int GetResurrectCount (void) { return m_Header.dwResurrectCount; }
		inline int GetScore (void) const { return (int)m_Header.dwScore; }
		CString GetSystemName (void) const;
		inline CString GetUsername (void) { return CString(m_Header.szUsername); }
		inline bool IsDebug (void) const { return ((m_Header.dwFlags & GAME_FLAG_DEBUG) ? true : false); }
		inline bool IsGameResurrect (void) { return ((m_Header.dwFlags & GAME_FLAG_RESURRECT) ? true : false); }
		inline bool IsOpen (void) const { return (m_pFile != NULL); }
		inline bool IsRegistered (void) const { return ((m_Header.dwFlags & GAME_FLAG_REGISTERED) ? true : false); }
		inline bool IsUniverseValid (void) { return (m_Header.dwUniverse != INVALID_ENTRY); }
		ALERROR Open (const CString &sFilename);

		ALERROR LoadGameStats (CGameStats *retStats);
		ALERROR LoadSystem (DWORD dwUNID, CSystem **retpSystem, CString *retsError, DWORD dwObjID = OBJID_NULL, CSpaceObject **retpObj = NULL, CSpaceObject *pPlayerShip = NULL);
		ALERROR LoadUniverse (CUniverse &Univ, DWORD *retdwSystemID, DWORD *retdwPlayerID, CString *retsError);
		ALERROR SaveGameStats (const CGameStats &Stats);
		ALERROR SaveSystem (DWORD dwUNID, CSystem *pSystem, DWORD dwFlags = 0);
		ALERROR SaveUniverse (CUniverse &Univ, DWORD dwFlags);
		ALERROR SetGameResurrect (void);
		ALERROR SetGameStatus (int iScore, const CString &sEpitaph);

	private:
		enum GameFlags
			{
			GAME_FLAG_RESURRECT =					0x00000001,	//	If we load game when flag is set, then it counts as resurrect
			GAME_FLAG_DEBUG =						0x00000002,	//	This is a debug game
			GAME_FLAG_IN_STARGATE =					0x00000004,	//	We are in the middle of entering a stargate
			GAME_FLAG_REGISTERED =					0x00000008,	//	This is a registered game
			};

		enum Consts
			{
			GAME_HEADER_MAX_SYSTEM_NAME =			128,
			GAME_ID_MAX =							128,
			PLAYER_NAME_MAX =						128,
			USERNAME_MAX =							256,
			EPITAPH_MAX =							256,
			INVALID_ENTRY =							0xffffffff,
			};

		struct SGameHeader8
			{
			DWORD dwVersion;				//	Game file format version

			DWORD dwUniverse;				//	Location of universe data
			DWORD dwSystemMap;				//	Location of system directory. The system
											//		directory is an array of file IDs
											//		indexed by system UNID and prefixed
											//		by a count.
			char szSystemName[GAME_HEADER_MAX_SYSTEM_NAME];
			DWORD dwFlags;					//	Flags for game
			DWORD dwResurrectCount;			//	Number of times we're been resurrected
			DWORD dwGameStats;				//	Location of game stats
			DWORD dwCreateVersion;			//	Product version that created this save file
			DWORD dwPartialSave;			//	System entry that was partially saved (while entering a gate)

			DWORD dwSpare[4];
			};

		struct SGameHeader
			{
			DWORD dwVersion;				//	Game file format version

			DWORD dwUniverse;				//	Location of universe data
			DWORD dwSystemMap;				//	Location of system directory. The system
											//		directory is an array of file IDs
											//		indexed by system UNID and prefixed
											//		by a count.
			char szSystemName[GAME_HEADER_MAX_SYSTEM_NAME];
			DWORD dwFlags;					//	Flags for game
			DWORD dwResurrectCount;			//	Number of times we're been resurrected
			DWORD dwGameStats;				//	Location of game stats
			DWORD dwCreateVersion;			//	Product version that created this save file
			DWORD dwPartialSave;			//	System entry that was partially saved (while entering a gate)

			//	New in SGameHeader 9
			char szUsername[USERNAME_MAX];	//	Username (may be NULL if not a regulation game)
			char szGameID[GAME_ID_MAX];		//	GameID (may be NULL if not a regulation game)
			DWORD dwAdventure;				//	UNID of adventure (extension)
			char szPlayerName[PLAYER_NAME_MAX];
			DWORD dwGenome;					//	Player genome
			DWORD dwPlayerShip;				//	UNID of player ship
			DWORD dwScore;					//	Current score
			char szEpitaph[EPITAPH_MAX];	//	Epitaph (if dead)
			};

		ALERROR ComposeLoadError (const CString &sError, CString *retsError);
		ALERROR LoadGameHeader (SGameHeader *retHeader);
		void LoadSystemMapFromStream (DWORD dwVersion, const CString &sStream);
		ALERROR SaveGameHeader (SGameHeader &Header);
		void SaveSystemMapToStream (CString *retsStream);

		int m_iRefCount;

		CDataFile *m_pFile;

		int m_iHeaderID;							//	Entry of header
		SGameHeader m_Header;						//	Loaded header
		CIDTable m_SystemMap;						//	Map from system ID to save file ID
	};

