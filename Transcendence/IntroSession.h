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
			isIntroHelp,
			isCredits,
			isHighScores,
			isHighScoresEndGame,
			isOpeningTitles,
			isEndGame,
			isShipStats,
			isBlankThenRandom,
			isEnterCommand,
			isNews,
			isWaitingForHighScores,
			};

		CIntroSession (STranscendenceSessionCtx &CreateCtx, EStates iInitialState) : IHISession(*CreateCtx.pHI),
				m_Model(*CreateCtx.pModel),
				m_DebugConsole(*CreateCtx.pDebugConsole),
				m_Settings(*CreateCtx.pSettings),
				m_iInitialState(iInitialState),
				m_bShowAllShips(false)
			{ }

		~CIntroSession (void);

		//	IHISession virtuals
		virtual CReanimator &GetReanimator (void) override { return g_pTrans->GetReanimator(); }
		virtual void OnAnimate (CG32bitImage &Screen, bool bTopMost) override;
		virtual void OnChar (char chChar, DWORD dwKeyData) override;
		virtual void OnCleanUp (void) override;
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) override;
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnLButtonDblClick (int x, int y, DWORD dwFlags) override;
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture) override;
		virtual void OnLButtonUp (int x, int y, DWORD dwFlags) override;
        virtual void OnMouseMove (int x, int y, DWORD dwFlags) override;
		virtual void OnMove (int x, int y) override { g_pTrans->WMMove(x, y); }
		virtual void OnReportHardCrash (CString *retsMessage) override { *retsMessage = g_pTrans->GetCrashInfo(); }
		virtual void OnSize (int cxWidth, int cyHeight) override { g_pTrans->WMSize(cxWidth, cyHeight, 0); }

	private:
		void CmdShowHighScoreList (DWORD dwAdventure = 0, const CString &sUsername = NULL_STR, int iScore = 0);

		void CreateIntroSystem (void);
		void CreateIntroShips (DWORD dwNewShipClass = 0, DWORD dwSovereign = 0, CSpaceObject *pShipDestroyed = NULL);
		ALERROR CreateRandomShip (CSystem *pSystem, DWORD dwClass, CSovereign *pSovereign, CShip **retpShip);
		void InitShipTable (TSortMap<int, CShipClass *> &List, bool bAll = false);
		void OrderAttack (CShip *pShip, CSpaceObject *pTarget);

		void CancelCurrentState (void);
		void CreateSoundtrackTitleAnimation (CMusicResource *pTrack, IAnimatron **retpAni);
		inline EStates GetState (void) const { return m_iState; }
		bool HandleCommandBoxChar (char chChar, DWORD dwKeyData);
		bool HandleChar (char chChar, DWORD dwKeyData);
		void OnPOVSet (CSpaceObject *pObj);
		void Paint (CG32bitImage &Screen, bool bTopMost);
        void SetExpanded (bool bExpanded = true);
		void SetState (EStates iState);
		void StartSoundtrackTitleAnimation (CMusicResource *pTrack);
		void StopAnimations (void);
		void Update (void);

		CTranscendenceModel &m_Model;
		CCommandLineDisplay &m_DebugConsole;
		CGameSettings &m_Settings;
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
		bool m_bShowAllShips;				//	If FALSE, we only show the lower half (by score)
	};

class CIntroShipController : public IShipController
	{
	public:
		CIntroShipController (void);
		CIntroShipController (IShipController *pDelegate);
		virtual ~CIntroShipController (void);

		inline void SetShip (CShip *pShip) { m_pShip = pShip; }

		virtual void AccumulateCrewMetrics (SCrewMetrics &Metrics) override { m_pDelegate->AccumulateCrewMetrics(Metrics); }
		virtual void Behavior (SUpdateCtx &Ctx) override { m_pDelegate->Behavior(Ctx); }
		virtual void CancelDocking (void) override { m_pDelegate->CancelDocking(); }
		virtual CString DebugCrashInfo (void) override { return m_pDelegate->DebugCrashInfo(); }
        virtual ICCItem *FindProperty (const CString &sProperty) override { return m_pDelegate->FindProperty(sProperty); }
		virtual bool FollowsObjThroughGate (CSpaceObject *pLeader = NULL) override { return m_pDelegate->FollowsObjThroughGate(pLeader); }
		virtual int GetAISettingInteger (const CString &sSetting) override { return m_pDelegate->GetAISettingInteger(sSetting); }
		virtual CString GetAISettingString (const CString &sSetting) override { return m_pDelegate->GetAISettingString(sSetting); }
		virtual const CAISettings *GetAISettings (void) override { return m_pDelegate->GetAISettings(); }
		virtual CSpaceObject *GetBase (void) const override { return m_pDelegate->GetBase(); }
		virtual CString GetClass (void) override { return m_pDelegate->GetClass(); }
		virtual int GetCombatPower (void) override { return m_pDelegate->GetCombatPower(); }
		virtual CCurrencyBlock *GetCurrencyBlock (void) override { return m_pDelegate->GetCurrencyBlock(); }
		virtual CSpaceObject *GetDestination (void) const override { return m_pDelegate->GetDestination(); }
		virtual bool GetDeviceActivate (void) override { return m_pDelegate->GetDeviceActivate(); }
		virtual CSpaceObject *GetEscortPrincipal (void) const override { return m_pDelegate->GetEscortPrincipal(); }
		virtual int GetFireDelay (void) override { return m_pDelegate->GetFireDelay(); }
		virtual int GetFireRateAdj (void) override { return m_pDelegate->GetFireRateAdj(); }
		virtual EManeuverTypes GetManeuver (void) override { return m_pDelegate->GetManeuver(); }
		virtual CSpaceObject *GetOrderGiver (void) override { return m_pShip; }
		virtual bool GetReverseThrust (void) override { return m_pDelegate->GetReverseThrust(); }
		virtual CSpaceObject *GetShip (void) override { return m_pShip; }
		virtual bool GetStopThrust (void) override { return m_pDelegate->GetStopThrust(); }
		virtual CSpaceObject *GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget = false) const override { return m_pDelegate->GetTarget(ItemCtx, bNoAutoTarget); }
		virtual bool GetThrust (void) override { return m_pDelegate->GetThrust(); }
		virtual void GetWeaponTarget (STargetingCtx &TargetingCtx, CItemCtx &ItemCtx, CSpaceObject **retpTarget, int *retiFireSolution) override { m_pDelegate->GetWeaponTarget(TargetingCtx, ItemCtx, retpTarget, retiFireSolution); }
		virtual bool IsAngryAt (CSpaceObject *pObj) const override { return m_pDelegate->IsAngryAt(pObj); }
		virtual int SetAISettingInteger (const CString &sSetting, int iValue) override { return m_pDelegate->SetAISettingInteger(sSetting, iValue); }
		virtual CString SetAISettingString (const CString &sSetting, const CString &sValue) override { return m_pDelegate->SetAISettingString(sSetting, sValue); }
		virtual void SetCommandCode (ICCItem *pCode) override { m_pDelegate->SetCommandCode(pCode); }
		virtual void SetManeuver (EManeuverTypes iManeuver) override { m_pDelegate->SetManeuver(iManeuver); }
		virtual void SetThrust (bool bThrust) override { m_pDelegate->SetThrust(bThrust); }

		virtual void AddOrder (OrderTypes Order, CSpaceObject *pTarget, const IShipController::SData &Data, bool bAddBefore = false) override { m_pDelegate->AddOrder(Order, pTarget, Data, bAddBefore); }
		virtual void CancelAllOrders (void) override { m_pDelegate->CancelAllOrders(); }
		virtual void CancelCurrentOrder (void) override { m_pDelegate->CancelCurrentOrder(); }
		virtual bool CancelOrder (int iIndex) override { return m_pDelegate->CancelOrder(iIndex); }
		virtual OrderTypes GetCurrentOrderEx (CSpaceObject **retpTarget = NULL, IShipController::SData *retData = NULL) override { return m_pDelegate->GetCurrentOrderEx(retpTarget, retData); }
		virtual OrderTypes GetOrder (int iIndex, CSpaceObject **retpTarget = NULL, IShipController::SData *retData = NULL) const override { return m_pDelegate->GetOrder(iIndex, retpTarget, retData); }
		virtual int GetOrderCount (void) const override { return m_pDelegate->GetOrderCount(); }

		//	Events

		virtual void OnAbilityChanged (Abilities iAbility, AbilityModifications iChange, bool bNoMessage = false) override { m_pDelegate->OnAbilityChanged(iAbility, iChange, bNoMessage); }
		virtual void OnAttacked (CSpaceObject *pAttacker, const SDamageCtx &Damage) override { m_pDelegate->OnAttacked(pAttacker, Damage); }
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2) override { return m_pDelegate->OnCommunicate(pSender, iMessage, pParam1, dwParam2); }
		virtual void OnComponentChanged (ObjectComponentTypes iComponent) override { m_pDelegate->OnComponentChanged(iComponent); }
		virtual void OnDamaged (const CDamageSource &Cause, CInstalledArmor *pArmor, const DamageDesc &Damage, int iDamage) override { m_pDelegate->OnDamaged(Cause, pArmor, Damage, iDamage); }
		virtual void OnDeviceEnabledDisabled (int iDev, bool bEnabled, bool bSilent = false) override { m_pDelegate->OnDeviceEnabledDisabled(iDev, bEnabled, bSilent); }
		virtual void OnDeviceStatus (CInstalledDevice *pDev, CDeviceClass::DeviceNotificationTypes iEvent) override { m_pDelegate->OnDeviceStatus(pDev, iEvent); }
		virtual bool OnDestroyCheck (DestructionTypes iCause, const CDamageSource &Attacker) override { return m_pDelegate->OnDestroyCheck(iCause, Attacker); }
		virtual void OnDestroyed (SDestroyCtx &Ctx) override;
		virtual void OnDocked (CSpaceObject *pObj) override { m_pDelegate->OnDocked(pObj); }
		virtual void OnEnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate, bool bAscend) override { m_pDelegate->OnEnterGate(pDestNode, sDestEntryPoint, pStargate, bAscend); }
        virtual void OnFuelConsumed (Metric rFuel, CReactorDesc::EFuelUseTypes iUse) override { m_pDelegate->OnFuelConsumed(rFuel, iUse); }
		virtual void OnHitBarrier (CSpaceObject *pBarrierObj, const CVector &vPos) override { m_pDelegate->OnHitBarrier(pBarrierObj, vPos); }
		virtual void OnMissionCompleted (CMission *pMission, bool bSuccess) override { m_pDelegate->OnMissionCompleted(pMission, bSuccess); }
		virtual void OnObjDamaged (const SDamageCtx &Ctx) override { m_pDelegate->OnObjDamaged(Ctx); }
		virtual void OnObjDestroyed (const SDestroyCtx &Ctx) override { m_pDelegate->OnObjDestroyed(Ctx); }
		virtual void OnObjEnteredGate (CSpaceObject *pObj, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate) override { m_pDelegate->OnObjEnteredGate(pObj, pDestNode, sDestEntryPoint, pStargate); }
		virtual void OnPaintSRSEnhancements (CG32bitImage &Dest, SViewportPaintCtx &Ctx) override { m_pDelegate->OnPaintSRSEnhancements(Dest, Ctx); }
		virtual void OnProgramDamage (CSpaceObject *pHacker, const ProgramDesc &Program) override { m_pDelegate->OnProgramDamage(pHacker, Program); }
		virtual void OnShipStatus (EShipStatusNotifications iEvent, DWORD dwData = 0) override { m_pDelegate->OnShipStatus(iEvent, dwData); }
		virtual void OnStatsChanged (void) override { m_pDelegate->OnStatsChanged(); }
		virtual void OnStationDestroyed (const SDestroyCtx &Ctx) override { m_pDelegate->OnStationDestroyed(Ctx); }
		virtual void OnWeaponStatusChanged (void) override { m_pDelegate->OnWeaponStatusChanged(); }
		virtual void OnWreckCreated (CSpaceObject *pWreck) override { m_pDelegate->OnWreckCreated(pWreck); }

	private:
		IShipController *m_pDelegate;
		CShip *m_pShip;
	};

