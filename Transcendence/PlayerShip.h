//	PlayerShip.h
//
//	Player ship classes
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

enum TargetTypes
	{
	targetEnemies,
	targetFriendlies,
	};

enum UIMessageTypes
	{
	uimsgUnknown =					-1,
	uimsgEnabledHints =				-2,

	uimsgAllMessages =				0,
	uimsgAllHints =					1,			//	IsEnabled(uimsgAllHints) returns TRUE if ANY hint is enabled
												//	SetEnabled(uimsgAllHints) enables/disables ALL hints

	uimsgCommsHint =				2,
	uimsgDockHint =					3,
	uimsgMapHint =					4,
	uimsgAutopilotHint =			5,
	uimsgGateHint =					6,
	uimsgUseItemHint =				7,
	uimsgRefuelHint =				8,
	uimsgEnableDeviceHint =			9,
	uimsgSwitchMissileHint =		10,
	uimsgFireMissileHint =			11,
	uimsgGalacticMapHint =			12,

	uimsgCount =					13,
	};

class CUIMessageController
	{
	public:
		CUIMessageController (void);

		UIMessageTypes Find (const CString &sMessageName);
		inline bool IsEnabled (UIMessageTypes iMsg) const { return m_bMsgEnabled[iMsg]; }
		void ReadFromStream (SLoadCtx &Ctx);
		void SetEnabled (UIMessageTypes iMsg, bool bEnabled = true);
		void WriteToStream (IWriteStream *pStream);

	private:
		bool IsHint (UIMessageTypes iMsg);

		bool m_bMsgEnabled[uimsgCount];
	};

class CManeuverController
	{
	public:
		enum ECommands
			{
			cmdNone =				0,

			cmdMouseAim =			1,	//	Turn to m_iAngle (used for mouse aiming)
			cmdMoveTo =				2,	//	Move to m_vPos
			cmdDockWith =			3,	//	Dock with m_pTarget
			};

		CManeuverController (void);

		bool CmdCancel (void);
		bool CmdMouseAim (int iAngle);
		bool CmdMoveTo (const CVector &vPos);
		EManeuverTypes GetManeuver (CShip *pShip) const;
		bool GetThrust (CShip *pShip) const;
		inline bool IsActive (void) const { return m_iCommand != cmdNone; }
		inline bool IsManeuverActive (void) const { return m_iCommand != cmdNone; }
		bool IsThrustActive (void) const;
		void ReadFromStream (SLoadCtx &Ctx);
		void Update (SUpdateCtx &Ctx, CShip *pShip);
		void WriteToStream (IWriteStream &Stream, CSystem *pSystem);

	private:
		void UpdateMoveTo (SUpdateCtx &Ctx, CShip *pShip);

		ECommands m_iCommand;
		int m_iAngle;
		CVector m_vPos;
		CSpaceObject *m_pTarget;

		//	Temporary variables during update (no need to save)

		EManeuverTypes m_iManeuver;
		bool m_bThrust;
	};

class CPlayerShipController : public IShipController
	{
	public:
		CPlayerShipController (void);
		~CPlayerShipController (void);

		void Cargo (void);
		inline CurrencyValue Charge (DWORD dwEconUNID, CurrencyValue iCredits) { return m_Credits.IncCredits(dwEconUNID, -iCredits); }

		bool CanShowShipStatus (void);
		void Communications (CSpaceObject *pObj, MessageTypes iMsg, DWORD dwData = 0, DWORD *iodwFormationPlace = NULL);
		void Dock (void);
		inline bool DockingInProgress (void) { return m_pStation != NULL; }
		inline UIMessageTypes FindUIMessage (const CString &sName) { return m_UIMsgs.Find(sName); }
		void Gate (void);
		void GenerateGameStats (CGameStats &Stats, bool bGameOver);
		inline int GetBestEnemyShipsDestroyed (DWORD *retdwUNID = NULL) { return m_Stats.GetBestEnemyShipsDestroyed(retdwUNID); }
		inline CurrencyValue GetCredits (DWORD dwEconUNID) { return m_Credits.GetCredits(dwEconUNID); }
		inline int GetCargoSpace (void) { return (int)(m_pShip->GetCargoSpaceLeft() + 0.5); }
		inline int GetEndGameScore (void) { return m_Stats.CalcEndGameScore(); }
		inline int GetEnemiesDestroyed (void) { return ::strToInt(m_Stats.GetStat(CONSTLIT("enemyShipsDestroyed")), 0); }
        inline CGameSession *GetGameSession (void) { return m_pSession; }
        inline CPlayerGameStats &GetGameStats (void) { return m_Stats; }
		inline CString GetItemStat (const CString &sStat, ICCItem *pItemCriteria) const { return m_Stats.GetItemStat(sStat, pItemCriteria); }
		inline CString GetKeyEventStat (const CString &sStat, const CString &sNodeID, const CDesignTypeCriteria &Crit) const { return m_Stats.GetKeyEventStat(sStat, sNodeID, Crit); }
		inline GenomeTypes GetPlayerGenome (void) const { return m_iGenome; }
		inline CString GetPlayerName (void) const { return m_sName; }
		inline int GetResurrectCount (void) const { return ::strToInt(m_Stats.GetStat(CONSTLIT("resurrectCount")), 0); }
		inline int GetScore (void) { return ::strToInt(m_Stats.GetStat(CONSTLIT("score")), 0); }
		inline CSpaceObject *GetSelectedTarget (void) { return m_pTarget; }
		inline CShip *GetShip (void) { return m_pShip; }
		inline DWORD GetStartingShipClass (void) const { return m_dwStartingShipClass; }
		inline CString GetStat (const CString &sStat) { return m_Stats.GetStat(sStat); }
		inline DWORD GetSystemEnteredTime (const CString &sNodeID) { return m_Stats.GetSystemEnteredTime(sNodeID); }
		inline int GetSystemsVisited (void) { return ::strToInt(m_Stats.GetStat(CONSTLIT("systemsVisited")), 0); }
		inline CTranscendenceWnd *GetTrans (void) { return m_pTrans; }
		inline void IncScore (int iBonus) { m_Stats.IncStat(CONSTLIT("score"), iBonus); }
		void Init (CTranscendenceWnd *pTrans);
		void InsuranceClaim (void);
		inline bool IsGalacticMapAvailable (void) { return (m_pShip && (m_pShip->GetAbility(::ablGalacticMap) > ::ablUninstalled)); }
		inline bool IsMapHUDActive (void) { return m_bMapHUD; }
		inline bool IsUIMessageEnabled (UIMessageTypes iMsg) { return m_UIMsgs.IsEnabled(iMsg); }
		void OnEnemyShipsDetected (void);
		inline void OnGameEnd (void) { m_Stats.OnGameEnd(m_pShip); }
		inline void OnMouseAimSetting (bool bEnabled) { if (!bEnabled) m_ManeuverController.CmdCancel(); }
        void OnStartGame (void);
		void OnSystemEntered (CSystem *pSystem, int *retiLastVisit = NULL) { m_Stats.OnSystemEntered(pSystem, retiLastVisit); }
		void OnSystemLeft (CSystem *pSystem) { m_Stats.OnSystemLeft(pSystem); }
		inline CurrencyValue Payment (DWORD dwEconUNID, CurrencyValue iCredits) { return m_Credits.IncCredits(dwEconUNID, iCredits); }
		void ReadyNextWeapon (int iDir = 1);
		void ReadyNextMissile (int iDir = 1);
		inline void SetCharacterClass (CGenericType *pClass) { m_pCharacterClass = pClass; }
		void SetDestination (CSpaceObject *pTarget);
        inline void SetGameSession (CGameSession *pSession) { m_pSession = pSession; }
		inline void SetGenome (GenomeTypes iGenome) { m_iGenome = iGenome; }
		inline void SetMapHUD (bool bActive) { m_bMapHUD = bActive; }
        inline void SetMouseAimAngle (int iAngle) { m_ManeuverController.CmdMouseAim(iAngle); }
		inline void SetName (const CString &sName) { m_sName = sName; }
		inline void SetResurrectCount (int iCount) { m_Stats.SetStat(CONSTLIT("resurrectCount"), ::strFromInt(iCount)); }
		inline void SetStartingShipClass (DWORD dwUNID) { m_dwStartingShipClass = dwUNID; }
		void SetTarget (CSpaceObject *pTarget);
		void SelectNearestTarget (void);
		void SelectNextFriendly (int iDir = 1);
		void SelectNextTarget (int iDir = 1);
		inline void SetActivate (bool bActivate) { m_bActivate = bActivate; }
		void SetFireMain (bool bFire);
		inline void SetFireMissile (bool bFire);
		inline void SetShip (CShip *pShip) { m_pShip = pShip; }
		inline void SetStopThrust (bool bStop) { m_bStopThrust = bStop; }
		inline void SetUIMessageEnabled (UIMessageTypes iMsg, bool bEnabled = true) { m_UIMsgs.SetEnabled(iMsg, bEnabled); }
		ALERROR SwitchShips (CShip *pNewShip, SPlayerChangedShipsCtx &Options);
		void Undock (void);
		void Update (int iTick);

		//	Device methods
		bool AreAllDevicesEnabled (void);
		void EnableAllDevices (bool bEnable = true);
		bool ToggleEnableDevice (int iDeviceIndex);

		//	Fleet formation methods
		DWORD GetCommsStatus (void);
		bool HasFleet (void);

		//	IShipController virtuals

		virtual void AddOrder (OrderTypes Order, CSpaceObject *pTarget, const IShipController::SData &Data, bool bAddBefore = false) override;
		virtual void Behavior (SUpdateCtx &Ctx) override;
		virtual void CancelAllOrders (void) override;
		virtual void CancelCurrentOrder (void) override;
		virtual void CancelDocking (void) override;
		virtual bool CancelOrder (int iIndex) override;
		virtual CString DebugCrashInfo (void) override;
        virtual ICCItem *FindProperty (const CString &sProperty) override;
		virtual CString GetAISettingString (const CString &sSetting) override;
		virtual CString GetClass (void) override { return CONSTLIT("player"); }
		virtual int GetCombatPower (void) override;
		virtual CCurrencyBlock *GetCurrencyBlock (void) override { return &m_Credits; }
		virtual OrderTypes GetCurrentOrderEx (CSpaceObject **retpTarget = NULL, IShipController::SData *retData = NULL) override;
		virtual CSpaceObject *GetDestination (void) const override { return m_pDestination; }
		virtual EManeuverTypes GetManeuver (void) override;
		virtual bool GetThrust (void) override;
		virtual CSpaceObject *GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget = false) const override;
		virtual bool GetReverseThrust (void) override;
		virtual bool GetStopThrust (void) override;
		virtual OrderTypes GetOrder (int iIndex, CSpaceObject **retpTarget = NULL, IShipController::SData *retData = NULL) const override;
		virtual CSpaceObject *GetOrderGiver (void) override { return m_pShip; }
		virtual int GetOrderCount (void) const override { return (m_iOrder == IShipController::orderNone ? 0 : 1); }
		virtual bool GetDeviceActivate (void) override;
		virtual int GetFireDelay (void) override { return (int)((5.0 / STD_SECONDS_PER_UPDATE) + 0.5); }
		virtual void GetWeaponTarget (STargetingCtx &TargetingCtx, CItemCtx &ItemCtx, CSpaceObject **retpTarget, int *retiFireSolution) override;
		virtual bool IsPlayer (void) const override { return true; }
		virtual void ReadFromStream (SLoadCtx &Ctx, CShip *pShip) override;
		virtual void SetManeuver (EManeuverTypes iManeuver) override { m_iManeuver = iManeuver; }
		virtual void SetThrust (bool bThrust) override { m_bThrust = bThrust; }
		virtual void WriteToStream (IWriteStream *pStream) override;

		//	Events

		virtual void OnAbilityChanged (Abilities iAbility, AbilityModifications iChange, bool bNoMessage = false) override;
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2) override;
		virtual void OnComponentChanged (ObjectComponentTypes iComponent) override;
		virtual void OnDamaged (const CDamageSource &Cause, CInstalledArmor *pArmor, const DamageDesc &Damage, int iDamage) override;
		virtual bool OnDestroyCheck (DestructionTypes iCause, const CDamageSource &Attacker) override;
		virtual void OnDestroyed (SDestroyCtx &Ctx) override;
		virtual void OnDeviceEnabledDisabled (int iDev, bool bEnable, bool bSilent = false) override;
		virtual void OnDeviceStatus (CInstalledDevice *pDev, CDeviceClass::DeviceNotificationTypes iEvent) override;
		virtual void OnDocked (CSpaceObject *pObj) override;
		virtual void OnEnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate, bool bAscend) override;
        virtual void OnFuelConsumed (Metric rFuel, CReactorDesc::EFuelUseTypes iUse) override;
		virtual void OnItemBought (const CItem &Item, CurrencyValue iTotalPrice) override { m_Stats.OnItemBought(Item, iTotalPrice); }
		virtual void OnItemDamaged (const CItem &Item, int iHP) override { m_Stats.OnItemDamaged(Item, iHP); }
		virtual void OnItemFired (const CItem &Item) override { m_Stats.OnItemFired(Item); }
		virtual void OnItemInstalled (const CItem &Item) override { m_Stats.OnItemInstalled(Item); }
		virtual void OnItemSold (const CItem &Item, CurrencyValue iTotalPrice) override { m_Stats.OnItemSold(Item, iTotalPrice); }
		virtual void OnItemUninstalled (const CItem &Item) override { m_Stats.OnItemUninstalled(Item); }
		virtual void OnMissionCompleted (CMission *pMission, bool bSuccess) override;
		virtual void OnNewSystem (CSystem *pSystem) override;
		virtual void OnObjDamaged (const SDamageCtx &Ctx) override;
		virtual void OnObjDestroyed (const SDestroyCtx &Ctx) override;
		virtual void OnOverlayConditionChanged (CConditionSet::ETypes iCondition, CConditionSet::EModifications iChange) override;
		virtual void OnPaintSRSEnhancements (CG32bitImage &Dest, SViewportPaintCtx &Ctx) override;
		virtual void OnProgramDamage (CSpaceObject *pHacker, const ProgramDesc &Program) override;
		virtual void OnShipStatus (EShipStatusNotifications iEvent, DWORD dwData = 0) override;
		virtual void OnStationDestroyed (const SDestroyCtx &Ctx) override;
		virtual void OnUpdatePlayer (SUpdateCtx &Ctx) override;
		virtual void OnWeaponStatusChanged (void) override;
		virtual void OnWreckCreated (CSpaceObject *pWreck) override;

	private:
		void ClearFireAngle (void);
		CSpaceObject *FindAutoTarget (CItemCtx &ItemCtx) const;
		CSpaceObject *FindDockTarget (void);
		bool HasCommsTarget (void);
		void InitTargetList (TargetTypes iTargetType, bool bUpdate = false);
		void PaintDebugLineOfFire (SViewportPaintCtx &Ctx, CG32bitImage &Dest) const;
		void PaintDebugLineOfFire (SViewportPaintCtx &Ctx, CG32bitImage &Dest, CSpaceObject *pTarget) const;
		void PaintDockingPortIndicators (SViewportPaintCtx &Ctx, CG32bitImage &Dest) const;
		void PaintTargetingReticle (SViewportPaintCtx &Ctx, CG32bitImage &Dest, CSpaceObject *pTarget);
		void Reset (void);
		void UpdateHelp (int iTick);

		CTranscendenceWnd *m_pTrans;
        CGameSession *m_pSession;               //  Game session
		CShip *m_pShip;

		OrderTypes m_iOrder;					//	Last order
		CSpaceObject *m_pTarget;
		CSpaceObject *m_pDestination;
		CSpaceObjectTable m_TargetList;

		CSpaceObject *m_pStation;				//	Station that player is docked with
		bool m_bSignalDock;						//	Tell the model to switch to dock screen

		DWORD m_dwWreckObjID;					//	WreckObjID (temp while we resurrect)

		int m_iLastHelpTick;
		int m_iLastHelpUseTick;
		int m_iLastHelpFireMissileTick;

		CManeuverController m_ManeuverController;
		EManeuverTypes m_iManeuver;
		bool m_bThrust;
		bool m_bActivate;
		bool m_bStopThrust;

		bool m_bMapHUD;							//	Show HUD on map
		bool m_bDockPortIndicators;				//	Dock ports light up when near by

        int m_iMouseAimAngle;                   //  Angle to aim towards

		CCurrencyBlock m_Credits;				//	Money available to player
		CPlayerGameStats m_Stats;				//	Player stats, including score
		CUIMessageController m_UIMsgs;			//	Status of various UI messages, such as hints

		CString m_sName;						//	Player name
		GenomeTypes m_iGenome;					//	Player genome
		DWORD m_dwStartingShipClass;			//	Starting ship class
		CGenericType *m_pCharacterClass;		//	Character class

		bool m_bUnderAttack;					//	TRUE if we're currently under attack

		CSpaceObject *m_pAutoDock;				//	The current station to dock with if we were to 
												//		press 'D' right now. NULL means no station
												//		to dock with.
		int m_iAutoDockPort;					//	The current dock port.
		CVector m_vAutoDockPort;				//	The current dock port position;

		bool m_bShowAutoTarget;					//	If TRUE, we show the autotarget
		bool m_bTargetOutOfRange;				//	If TRUE, m_pTarget is out of weapon range
		CSpaceObject *m_pAutoTarget;			//	Saved autotarget.
		mutable int m_iAutoTargetTick;

		CSpaceObject *m_pAutoDamage;			//	Show damage bar for this object
		DWORD m_dwAutoDamageExpire;				//	Stop showing on this tick

	friend CObjectClass<CPlayerShipController>;
	};

