//	IntroSession.h
//
//	Transcendence session classes
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CHighScoreDisplay
	{
	public:
		CHighScoreDisplay (void);
		~CHighScoreDisplay (void);

		inline bool HasHighScores (void) const { return (m_pHighScoreList && m_pHighScoreList->GetCount() > 0); }
		bool IsPerformanceRunning (void);
		void SelectNext (void);
		void SelectPrev (void);
		void SetHighScoreList (CAdventureHighScoreList *pList);
		bool StartPerformance (CReanimator &Reanimator, const CString &sPerformanceID, const RECT &rcScreen);
		void StopPerformance (void);

	private:
		void CreatePerformance (CReanimator &Reanimator, const CString &sPerformanceID, const RECT &rcRect, CAdventureHighScoreList *pHighScoreList, IAnimatron **retpAnimatron);
		void DeletePerformance (void);
		int GetCurrentScrollPos (void);
		inline bool IsPerformanceCreated (void) const { return (m_dwPerformance != 0); }
		void ScrollToPos (int iPos);

		CAdventureHighScoreList *m_pHighScoreList;
		RECT m_rcScreen;

		//	State data for performance

		CReanimator *m_pReanimator;
		IAnimatron *m_pPerformance;
		CString m_sPerformance;
		DWORD m_dwPerformance;
		TArray<int> m_ScrollPos;			//	Scroll position for each entry in high scores
		int m_iSelection;					//	Currently selected entry
	};

//	CIntroSession --------------------------------------------------------------

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
			isWaitingForHighScores,
			};

		CIntroSession (CHumanInterface &HI, CTranscendenceModel &Model, EStates iInitialState) : IHISession(HI),
				m_Model(Model),
				m_iInitialState(iInitialState)
			{ }

		~CIntroSession (void);

		//	IHISession virtuals
		virtual CReanimator &GetReanimator (void) override { return g_pTrans->GetReanimator(); }
		virtual void OnAnimate (CG32bitImage &Screen, bool bTopMost) override;
		virtual void OnChar (char chChar, DWORD dwKeyData) override;
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) override;
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnLButtonDblClick (int x, int y, DWORD dwFlags) override { g_pTrans->WMLButtonDblClick(x, y, dwFlags); }
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture) override { m_iIdleTicks = 0;  SetExpanded(false); g_pTrans->WMLButtonDown(x, y, dwFlags); }
		virtual void OnLButtonUp (int x, int y, DWORD dwFlags) override { g_pTrans->WMLButtonUp(x, y, dwFlags); }
        virtual void OnMouseMove (int x, int y, DWORD dwFlags) override;
		virtual void OnMove (int x, int y) override { g_pTrans->WMMove(x, y); }
		virtual void OnReportHardCrash (CString *retsMessage) override { *retsMessage = g_pTrans->GetCrashInfo(); }
		virtual void OnSize (int cxWidth, int cyHeight) override { g_pTrans->WMSize(cxWidth, cyHeight, 0); }

	private:
		void CmdShowHighScoreList (DWORD dwAdventure = 0, const CString &sUsername = NULL_STR, int iScore = 0);

		void CreateIntroSystem (void);
		void CreateIntroShips (DWORD dwNewShipClass = 0, DWORD dwSovereign = 0, CSpaceObject *pShipDestroyed = NULL);
		ALERROR CreateRandomShip (CSystem *pSystem, DWORD dwClass, CSovereign *pSovereign, CShip **retpShip);

		void CancelCurrentState (void);
		void CreateSoundtrackTitleAnimation (CSoundType *pTrack, IAnimatron **retpAni);
		inline EStates GetState (void) const { return m_iState; }
		bool HandleCommandBoxChar (char chChar, DWORD dwKeyData);
		bool HandleChar (char chChar, DWORD dwKeyData);
		void OnPOVSet (CSpaceObject *pObj);
		void Paint (CG32bitImage &Screen, bool bTopMost);
        void SetExpanded (bool bExpanded = true);
		void SetState (EStates iState);
		void StartSoundtrackTitleAnimation (CSoundType *pTrack);
		void StopAnimations (void);
		void Update (void);

		CTranscendenceModel &m_Model;
		EStates m_iInitialState;
		EStates m_iState;					//	Current state

		RECT m_rcMain;						//	Main animation RECT (where system is painted)
        RECT m_rcCenter;                    //  Center RECT
		RECT m_rcTop;						//	Top area (sign in controls, etc.)
		RECT m_rcBottom;					//	Bottom area (buttons)

        bool m_bExpanded;                   //  TRUE if the main screen is fully expanded
        bool m_bExpandedDesired;            //  Desired setting for expanded/collapsed
        int m_iIdleTicks;                   //  Number of ticks idle
        RECT m_rcMainNormal;                //  Center RECT with button area
        RECT m_rcMainExpanded;              //  Full screen

		CHighScoreDisplay m_HighScoreDisplay;

		TSortMap<int, CShipClass *> m_ShipList;
	};

class CIntroShipController : public IShipController
	{
	public:
		CIntroShipController (void);
		CIntroShipController (IShipController *pDelegate);
		virtual ~CIntroShipController (void);

		inline void SetShip (CShip *pShip) { m_pShip = pShip; }

		virtual void Behavior (SUpdateCtx &Ctx) override { m_pDelegate->Behavior(Ctx); }
		virtual CString DebugCrashInfo (void) override { return m_pDelegate->DebugCrashInfo(); }
		virtual int GetCombatPower (void) override { return m_pDelegate->GetCombatPower(); }
		virtual EManeuverTypes GetManeuver (void) override { return m_pDelegate->GetManeuver(); }
		virtual bool GetThrust (void) override { return m_pDelegate->GetThrust(); }
		virtual bool GetReverseThrust (void) override { return m_pDelegate->GetReverseThrust(); }
		virtual bool GetStopThrust (void) override { return m_pDelegate->GetStopThrust(); }
		virtual bool GetDeviceActivate (void) override { return m_pDelegate->GetDeviceActivate(); }
		virtual int GetFireDelay (void) override { return m_pDelegate->GetFireDelay(); }
		virtual int GetFireRateAdj (void) override { return m_pDelegate->GetFireRateAdj(); }
		virtual CSpaceObject *GetBase (void) const override { return m_pDelegate->GetBase(); }
		virtual CSpaceObject *GetEscortPrincipal (void) const override { return m_pDelegate->GetEscortPrincipal(); }
		virtual CSpaceObject *GetOrderGiver (void) override { return m_pShip; }
		virtual CSpaceObject *GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget = false) const override { return m_pDelegate->GetTarget(ItemCtx, bNoAutoTarget); }
		virtual void GetWeaponTarget (STargetingCtx &TargetingCtx, CItemCtx &ItemCtx, CSpaceObject **retpTarget, int *retiFireSolution) override { m_pDelegate->GetWeaponTarget(TargetingCtx, ItemCtx, retpTarget, retiFireSolution); }

		virtual void AddOrder (OrderTypes Order, CSpaceObject *pTarget, const IShipController::SData &Data, bool bAddBefore = false) override { m_pDelegate->AddOrder(Order, pTarget, Data, bAddBefore); }
		virtual void CancelAllOrders (void) override { m_pDelegate->CancelAllOrders(); }
		virtual void CancelCurrentOrder (void) override { m_pDelegate->CancelCurrentOrder(); }
		virtual OrderTypes GetCurrentOrderEx (CSpaceObject **retpTarget = NULL, IShipController::SData *retData = NULL) override { return m_pDelegate->GetCurrentOrderEx(retpTarget, retData); }

		//	Events

		virtual void OnAttacked (CSpaceObject *pAttacker, const SDamageCtx &Damage) override { m_pDelegate->OnAttacked(pAttacker, Damage); }
		virtual void OnDamaged (const CDamageSource &Cause, CInstalledArmor *pArmor, const DamageDesc &Damage, int iDamage) override { m_pDelegate->OnDamaged(Cause, pArmor, Damage, iDamage); }
		virtual void OnDestroyed (SDestroyCtx &Ctx) override;
		virtual void OnDocked (CSpaceObject *pObj) override { m_pDelegate->OnDocked(pObj); }
		virtual void OnDockedObjChanged (CSpaceObject *pLocation) override { m_pDelegate->OnDockedObjChanged(pLocation); }
		virtual void OnEnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate, bool bAscend) override { m_pDelegate->OnEnterGate(pDestNode, sDestEntryPoint, pStargate, bAscend); }
		virtual void OnObjDestroyed (const SDestroyCtx &Ctx) override { m_pDelegate->OnObjDestroyed(Ctx); }
		virtual void OnShipStatus (EShipStatusNotifications iEvent, DWORD dwData = 0) override { m_pDelegate->OnShipStatus(iEvent, dwData); }
		virtual void OnWeaponStatusChanged (void) override { m_pDelegate->OnWeaponStatusChanged(); }

	private:
		IShipController *m_pDelegate;
		CShip *m_pShip;
	};

