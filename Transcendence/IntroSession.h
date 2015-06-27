//	IntroSession.h
//
//	Transcendence session classes
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CIntroSession : public IHISession
	{
	public:
		enum EStates
			{
			isBlank,
			isCredits,
			isHighScores,
			isHighScoresEndGame,
			isOpeningTitles,
			isEndGame,
			isShipStats,
			isBlankThenRandom,
			isEnterShipClass,
			isNews,
			};

		CIntroSession (CHumanInterface &HI, EStates iInitialState) : IHISession(HI),
				m_iInitialState(iInitialState),
				m_pHighScoreList(NULL)
			{ }

		~CIntroSession (void);

		//	IHISession virtuals
		virtual CReanimator &GetReanimator (void) { return g_pTrans->GetReanimator(); }
		virtual void OnAnimate (CG32bitImage &Screen, bool bTopMost);
		virtual void OnChar (char chChar, DWORD dwKeyData);
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void OnKeyUp (int iVirtKey, DWORD dwKeyData) { g_pTrans->WMKeyUp(iVirtKey, dwKeyData); }
		virtual void OnLButtonDblClick (int x, int y, DWORD dwFlags) { g_pTrans->WMLButtonDblClick(x, y, dwFlags); }
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture) { g_pTrans->WMLButtonDown(x, y, dwFlags); }
		virtual void OnLButtonUp (int x, int y, DWORD dwFlags) { g_pTrans->WMLButtonUp(x, y, dwFlags); }
		virtual void OnMouseMove (int x, int y, DWORD dwFlags) { g_pTrans->WMMouseMove(x, y, dwFlags); }
		virtual void OnMove (int x, int y) { g_pTrans->WMMove(x, y); }
		virtual void OnReportHardCrash (CString *retsMessage) { *retsMessage = g_pTrans->GetCrashInfo(); }
		virtual void OnSize (int cxWidth, int cyHeight) { g_pTrans->WMSize(cxWidth, cyHeight, 0); }

	private:
		void CmdShowHighScoreList (void);

		void CancelCurrentState (void);
		void CreateIntroShips (DWORD dwNewShipClass = 0, DWORD dwSovereign = 0, CSpaceObject *pShipDestroyed = NULL);
		void CreateSoundtrackTitleAnimation (CSoundType *pTrack, IAnimatron **retpAni);
		inline EStates GetState (void) const { return m_iState; }
		bool HandleCommandBoxChar (char chChar, DWORD dwKeyData);
		bool HandleChar (char chChar, DWORD dwKeyData);
		void OnPOVSet (CSpaceObject *pObj);
		void Paint (CG32bitImage &Screen, bool bTopMost);
		void SetState (EStates iState);
		void StartSoundtrackTitleAnimation (CSoundType *pTrack);
		void Update (void);

		EStates m_iInitialState;
		EStates m_iState;					//	Current state

		RECT m_rcMain;						//	Center RECT
		RECT m_rcTop;						//	Top area (sign in controls, etc.)
		RECT m_rcBottom;					//	Bottom area (buttons)

		CAdventureHighScoreList *m_pHighScoreList;
	};
