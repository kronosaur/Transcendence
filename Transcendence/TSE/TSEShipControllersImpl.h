//	TSEShipControllersImpl.h
//
//	Transcendence IShipController classes

#ifndef INCL_TSE_SHIP_CONTROLLERS
#define INCL_TSE_SHIP_CONTROLLERS

class CBaseShipAI;

class CAIShipControls
	{
	public:
		enum Constants
			{
			constAlwaysThrust = -1,
			constNeverThrust = 1000,
			};

		CAIShipControls (void);

		inline EManeuverTypes GetManeuver (void) const { return m_iManeuver; }
		inline bool GetThrust (CShip *pShip) const;
		inline int GetThrustDir (void) const { return m_iThrustDir; }
		void ReadFromStream (SLoadCtx &Ctx);
		inline void SetManeuver (EManeuverTypes iManeuver) { m_iManeuver = iManeuver; }
		inline void SetThrust (bool bThrust) { m_iThrustDir = (bThrust ? constAlwaysThrust : constNeverThrust); }
		inline void SetThrustDir (int iDir) { m_iThrustDir = iDir; }
		void WriteToStream (CSystem *pSystem, IWriteStream *pStream);

	private:
		EManeuverTypes m_iManeuver;	//	Current maneuver (turn)
		int m_iThrustDir;						//	Thrust if facing given direction
	};

class CAIBehaviorCtx
	{
	public:
		CAIBehaviorCtx (void);

		inline bool AscendOnGate (void) const { return m_AISettings.AscendOnGate(); }
		inline bool AvoidsExplodingStations (void) const { return m_fAvoidExplodingStations; }
		inline void ClearBestWeapon (void) { m_fRecalcBestWeapon = true; }
		inline void ClearNavPath (void) { m_pNavPath = NULL; m_iNavPathPos = -1; }
		void DebugPaintInfo (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		inline CString GetAISetting (const CString &sSetting) { return m_AISettings.GetValue(sSetting); }
		inline const CAISettings &GetAISettings (void) const { return m_AISettings; }
		inline CInstalledDevice *GetBestWeapon (void) const { return (m_iBestWeapon != devNone ? m_pBestWeapon : NULL); }
		inline DeviceNames GetBestWeaponIndex (void) const { return m_iBestWeapon; }
		inline Metric GetBestWeaponRange (void) const { return m_rBestWeaponRange; }
		inline Metric GetBestWeaponRange2 (void) const { return m_rBestWeaponRange * m_rBestWeaponRange; }
		inline AICombatStyles GetCombatStyle (void) const { return m_AISettings.GetCombatStyle(); }
		inline int GetFireAccuracy (void) const { return m_AISettings.GetFireAccuracy(); }
		inline int GetFireRangeAdj (void) const { return m_AISettings.GetFireRangeAdj(); }
		inline int GetFireRateAdj (void) const { return m_AISettings.GetFireRateAdj(); }
		inline Metric GetFlankDist (void) const { return m_rFlankDist; }
		inline int GetLastAttack (void) const { return m_iLastAttack; }
		inline EManeuverTypes GetLastTurn (void) const { return m_iLastTurn; }
		inline int GetLastTurnCount (void) const { return m_iLastTurnCount; }
		inline EManeuverTypes GetManeuver (void) const { return m_ShipControls.GetManeuver(); }
		inline int GetMaxTurnCount (void) const { return m_iMaxTurnCount; }
		inline Metric GetMaxWeaponRange (void) const { return m_rMaxWeaponRange; }
		inline Metric GetMinCombatSeparation (void) const { return m_AISettings.GetMinCombatSeparation(); }
		inline CNavigationPath *GetNavPath (void) const { return m_pNavPath; }
		inline int GetPerception (void) const { return m_AISettings.GetPerception(); }
		inline const CVector &GetPotential (void) const { return m_vPotential; }
		inline Metric GetPrimaryAimRange2 (void) const { return m_rPrimaryAimRange2; }
		inline bool GetThrust (CShip *pShip) const { return m_ShipControls.GetThrust(pShip); }
		inline int GetThrustDir (void) const { return m_ShipControls.GetThrustDir(); }
		inline bool HasEscorts (void) const { return m_fHasEscorts; }
		inline bool HasMultipleWeapons (void) const { return m_fHasMultipleWeapons; }
		inline bool HasSecondaryWeapons (void) const { return m_fHasSecondaryWeapons; }
		inline bool HasSuperconductingShields (void) const { return m_fSuperconductingShields; }
		inline bool IsAggressor (void) const { return m_AISettings.IsAggressor(); }
		inline bool IsBeingAttacked (int iTick, int iThreshold = 150) const { return (iTick - m_iLastAttack) > iThreshold; }
		inline bool IsDockingRequested (void) const { return m_fDockingRequested; }
		inline bool IsImmobile (void) const { return m_fImmobile; }
		inline bool IsNonCombatant (void) const { return m_AISettings.IsNonCombatant(); }
		inline bool IsWaitingForShieldsToRegen (void) const { return m_fWaitForShieldsToRegen; }
		inline bool NoAttackOnThreat (void) const { return m_AISettings.NoAttackOnThreat(); }
		inline bool NoDogfights (void) const { return m_AISettings.NoDogfights(); }
		inline bool NoFriendlyFire (void) const { return m_AISettings.NoFriendlyFire(); }
		inline bool NoFriendlyFireCheck (void) const { return m_AISettings.NoFriendlyFireCheck(); }
		inline bool NoNavPaths (void) const { return m_AISettings.NoNavPaths(); }
		inline bool NoOrderGiver (void) const { return m_AISettings.NoOrderGiver(); }
		inline bool NoShieldRetreat (void) const { return m_AISettings.NoShieldRetreat(); }
		void ReadFromStream (SLoadCtx &Ctx);
		inline void ReadFromStreamAISettings (SLoadCtx &Ctx) { m_AISettings.ReadFromStream(Ctx); }
		inline CString SetAISetting (const CString &sSetting, const CString &sValue) { return m_AISettings.SetValue(sSetting, sValue); }
		inline void SetAISettings (const CAISettings &Source) { m_AISettings = Source; }
		inline void SetDockingRequested (bool bValue = true) { m_fDockingRequested = bValue; }
		inline void SetHasEscorts (bool bValue = true) { m_fHasEscorts = bValue; }
		void SetLastAttack (int iTick);
		inline void SetLastTurn (EManeuverTypes iTurn) { m_iLastTurn = iTurn; }
		inline void SetLastTurnCount (int iCount) { m_iLastTurnCount = iCount; }
		inline void SetManeuver (EManeuverTypes iManeuver) { m_ShipControls.SetManeuver(iManeuver); }
		inline void SetManeuverCounter (int iCount) { m_iManeuverCounter = iCount; }
		inline void SetNavPath (CNavigationPath *pNavPath, int iNavPathPos) { m_pNavPath = pNavPath; m_iNavPathPos = iNavPathPos; }
		inline void SetPotential (const CVector &vVec) { m_vPotential = vVec; }
		inline void SetThrust (bool bThrust) { m_ShipControls.SetThrust(bThrust); }
		inline void SetThrustDir (int iDir) { m_ShipControls.SetThrustDir(iDir); }
		inline void SetWaitingForShieldsToRegen (bool bValue = true) { m_fWaitForShieldsToRegen = bValue; }
		inline bool ThrustsThroughTurn (void) const { return m_fThrustThroughTurn; }
		void WriteToStream (CSystem *pSystem, IWriteStream *pStream);

		//	Maneuvers
		CVector CalcManeuverCloseOnTarget (CShip *pShip, CSpaceObject *pTarget, const CVector &vTarget, Metric rTargetDist2, bool bFlank = false);
		CVector CalcManeuverFormation (CShip *pShip, const CVector vDest, const CVector vDestVel, int iDestFacing);
		CVector CalcManeuverSpiralIn (CShip *pShip, const CVector &vTarget, int iTrajectory = 30);
		CVector CalcManeuverSpiralOut (CShip *pShip, const CVector &vTarget, int iTrajectory = 30);
		void ImplementAttackNearestTarget (CShip *pShip, Metric rMaxRange, CSpaceObject **iopTarget, CSpaceObject *pExcludeObj = NULL);
		void ImplementAttackTarget (CShip *pShip, CSpaceObject *pTarget, bool bMaintainCourse = false, bool bDoNotShoot = false);
		void ImplementCloseOnImmobileTarget (CShip *pShip, CSpaceObject *pTarget, const CVector &vTarget, Metric rTargetDist2, Metric rTargetSpeed = 0.0);
		void ImplementCloseOnTarget (CShip *pShip, CSpaceObject *pTarget, const CVector &vTarget, Metric rTargetDist2, bool bFlank = false);
		void ImplementDocking (CShip *pShip, CSpaceObject *pTarget);
		void ImplementEscort (CShip *pShip, CSpaceObject *pBase, CSpaceObject **iopTarget);
		void ImplementEscortManeuvers (CShip *pShip, CSpaceObject *pTarget, const CVector &vTarget);
		void ImplementEvasiveManeuvers (CShip *pShip, CSpaceObject *pTarget);
		void ImplementFireOnTarget (CShip *pShip, CSpaceObject *pTarget, bool *retbOutOfRange = NULL);
		void ImplementFireOnTargetsOfOpportunity (CShip *pShip, CSpaceObject *pTarget = NULL, CSpaceObject *pExcludeObj = NULL);
		void ImplementFireWeapon (CShip *pShip, DeviceNames iDev = devNone);
		void ImplementFireWeaponOnTarget (CShip *pShip, int iWeapon, int iWeaponVariant, CSpaceObject *pTarget, const CVector &vTarget, Metric rTargetDist2, int *retiFireDir = NULL, bool bDoNotShoot = false);
		bool ImplementFlockingManeuver (CShip *pShip, CSpaceObject *pLeader);
		void ImplementFollowNavPath (CShip *pShip, bool *retbAtDestination = NULL);
		void ImplementFormationManeuver (CShip *pShip, const CVector vDest, const CVector vDestVel, int iDestFacing, bool *retbInFormation = NULL);
		void ImplementGating (CShip *pShip, CSpaceObject *pTarget);
		void ImplementHold (CShip *pShip, bool *retbInPlace = NULL);
		void ImplementOrbit (CShip *pShip, CSpaceObject *pBase, Metric rDistance);
		void ImplementManeuver (CShip *pShip, int iDir, bool bThrust, bool bNoThrustThroughTurn = false);
		void ImplementSpiralIn (CShip *pShip, const CVector &vTarget);
		void ImplementSpiralOut (CShip *pShip, const CVector &vTarget, int iTrajectory = 30);
		void ImplementStop (CShip *pShip);
		void ImplementTurnTo (CShip *pShip, int iRotation);

		//	Helpers
		void CalcAvoidPotential (CShip *pShip, CSpaceObject *pTarget);
		void CalcBestWeapon (CShip *pShip, CSpaceObject *pTarget, Metric rTargetDist2);
		bool CalcFlockingFormation (CShip *pShip, CSpaceObject *pLeader, Metric rFOVRange, Metric rSeparationRange, CVector *retvPos, CVector *retvVel, int *retiFacing);
		void CalcInvariants (CShip *pShip);
		bool CalcIsBetterTarget (CShip *pShip, CSpaceObject *pCurTarget, CSpaceObject *pNewTarget) const;
		bool CalcNavPath (CShip *pShip, CSpaceObject *pTo);
		void CalcNavPath (CShip *pShip, CSpaceObject *pFrom, CSpaceObject *pTo);
		void CalcNavPath (CShip *pShip, CNavigationPath *pPath);
		void CalcShieldState (CShip *pShip);
		int CalcWeaponScore (CShip *pShip, CSpaceObject *pTarget, CInstalledDevice *pWeapon, Metric rTargetDist2);
		void CancelDocking (CShip *pShip, CSpaceObject *pBase);
		inline bool CheckForFriendsInLineOfFire (CShip *pShip, CInstalledDevice *pDevice, CSpaceObject *pTarget, int iFireAngle, Metric rMaxRange)
			{ return (NoFriendlyFireCheck() || pShip->IsLineOfFireClear(pDevice, pTarget, iFireAngle, rMaxRange)); }
		inline CVector CombinePotential (const CVector &vDir)
			{ return GetPotential() + (vDir.Normal() * 100.0 * g_KlicksPerPixel);	}
		void CommunicateWithEscorts (CShip *pShip, MessageTypes iMessage, CSpaceObject *pParam1 = NULL, DWORD dwParam2 = 0);
		void Undock (CShip *pShip);

	private:
		bool ImplementAttackTargetManeuver (CShip *pShip, CSpaceObject *pTarget, const CVector &vTarget, Metric rTargetDist2);

		CAISettings m_AISettings;				//	Settings

		//	Ship control
		CAIShipControls m_ShipControls;			//	Current ship control state

		//	State
		EManeuverTypes m_iLastTurn;				//	Last turn direction
		int m_iLastTurnCount;					//	Number of updates turning
		int m_iManeuverCounter;					//	Counter used by maneuvers
		int m_iLastAttack;						//	Tick of last attack on us
		CVector m_vPotential;					//	Avoid potential
		CNavigationPath *m_pNavPath;			//	Current navigation path
		int m_iNavPathPos;						//	-1 = not in nav path

		DWORD m_fDockingRequested:1;			//	TRUE if we've requested docking
		DWORD m_fWaitForShieldsToRegen:1;		//	TRUE if ship is waiting for shields to regen
		DWORD m_dwSpare1:30;

		//	Cached values
		CInstalledDevice *m_pShields;			//	Shields (NULL if none)
		CInstalledDevice *m_pBestWeapon;		//	Best weapon
		DeviceNames m_iBestWeapon;
		Metric m_rBestWeaponRange;				//	Range of best weapon
		Metric m_rPrimaryAimRange2;				//	If further than this, close on target
		Metric m_rFlankDist;					//	Flank distance
		int m_iMaxTurnCount;					//	Max ticks turning in the same direction
		Metric m_rMaxWeaponRange;				//	Range of longest range primary weapon
		int m_iBestNonLauncherWeaponLevel;		//	Level of best non-launcher weapon
		int m_iPrematureFireChance;				//	Chance of firing prematurely

		DWORD m_fImmobile:1;					//	TRUE if ship does not move
		DWORD m_fSuperconductingShields:1;		//	TRUE if ship has superconducting shields
		DWORD m_fHasMultipleWeapons:1;			//	TRUE if ship has more than 1 primary
		DWORD m_fThrustThroughTurn:1;			//	TRUE if ship thrusts through a turn
		DWORD m_fAvoidExplodingStations:1;		//	TRUE if ship avoids exploding stations
		DWORD m_fRecalcBestWeapon:1;			//	TRUE if we need to recalc best weapon
		DWORD m_fHasSecondaryWeapons:1;			//	TRUE if ship has secondary weapons
		DWORD m_fHasEscorts:1;					//	TRUE if ship has escorts

		DWORD m_fHasMultiplePrimaries:1;		//	TRUE if ship has multiple primary weapons (non-launchers)
		DWORD m_fSpare2:1;
		DWORD m_fSpare3:1;
		DWORD m_fSpare4:1;
		DWORD m_fSpare5:1;
		DWORD m_fSpare6:1;
		DWORD m_fSpare7:1;
		DWORD m_fSpare8:1;

		DWORD m_dwSpare:16;
	};

const int MAX_OBJS =	4;

class IOrderModule
	{
	public:
		IOrderModule (int iObjCount);
		virtual ~IOrderModule (void);

		void Attacked (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pAttacker, const DamageDesc &Damage, bool bFriendlyFire);
		inline void Behavior (CShip *pShip, CAIBehaviorCtx &Ctx) { OnBehavior(pShip, Ctx); }
		inline void BehaviorStart (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pOrderTarget, const IShipController::SData &Data) { OnBehaviorStart(pShip, Ctx, pOrderTarget, Data); }
		DWORD Communicate (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);
		static IOrderModule *Create (IShipController::OrderTypes iOrder);
		CString DebugCrashInfo (void);
		inline CSpaceObject *GetBase (void) { return OnGetBase(); }
		inline IShipController::OrderTypes GetOrder (void) { return OnGetOrder(); }
		inline CSpaceObject *GetTarget (void) { return OnGetTarget(); }
		void ObjDestroyed (CShip *pShip, const SDestroyCtx &Ctx);
		void ReadFromStream (SLoadCtx &Ctx);
		void WriteToStream (CSystem *pSystem, IWriteStream *pStream);

	protected:
		//	IOrderModule virtuals
		virtual bool IsAttacking (void) { return false; }
		virtual bool IsTarget (int iObj) { return false; }
		virtual bool IsTarget (CSpaceObject *pObj) { return false; }
		virtual void OnAttacked (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pAttacker, const DamageDesc &Damage, bool bFriendlyFire) { }
		virtual void OnBehavior (CShip *pShip, CAIBehaviorCtx &Ctx) = 0;
		virtual void OnBehaviorStart (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pOrderTarget, const IShipController::SData &Data) { }
		virtual DWORD OnCommunicate (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2) { return resNoAnswer; }
		virtual CString OnDebugCrashInfo (void) { return NULL_STR; }
		virtual CSpaceObject *OnGetBase (void) { return NULL; }
		virtual IShipController::OrderTypes OnGetOrder (void) = 0;
		virtual CSpaceObject *OnGetTarget (void) { return NULL; }
		virtual void OnObjDestroyed (CShip *pShip, const SDestroyCtx &Ctx, int iObj, bool *retbCancelOrder) { }
		virtual void OnReadFromStream (SLoadCtx &Ctx) { }
		virtual void OnWriteToStream (CSystem *pSystem, IWriteStream *pStream) { }

		int m_iObjCount;
		CSpaceObject *m_Objs[MAX_OBJS];
	};

class COrderList
	{
	public:
		~COrderList (void);

		void DeleteAll (void);
		void DeleteCurrent (void);
		inline int GetCount (void) const { return m_List.GetCount(); }
		inline IShipController::OrderTypes GetCurrentOrder (void) const { return (IShipController::OrderTypes)(GetCurrentEntry().dwOrderType); }
		IShipController::OrderTypes GetCurrentOrder (CSpaceObject **retpTarget, IShipController::SData *retData = NULL) const;
		inline DWORD GetCurrentOrderData (void) const { return GetCurrentEntry().dwData; }
		inline CSpaceObject *GetCurrentOrderTarget (void) const { return GetCurrentEntry().pTarget; }
		void Insert (IShipController::OrderTypes iOrder, CSpaceObject *pTarget, const IShipController::SData &Data, bool bAddBefore = false);
		void OnNewSystem (CSystem *pNewSystem, bool *retbCurrentChanged);
		void OnObjDestroyed (CSpaceObject *pObj, bool *retbCurrentChanged);
		void OnPlayerChangedShips (CSpaceObject *pOldShip, CSpaceObject *pNewShip, bool *retbCurrentChanged);
		void OnStationDestroyed (CSpaceObject *pObj, bool *retbCurrentChanged);
		void ReadFromStream (SLoadCtx &Ctx);
		void SetCurrentOrderData (const IShipController::SData &Data);
		void WriteToStream (IWriteStream *pStream, CSystem *pSystem);

	private:
		struct SOrderEntry
			{
			SOrderEntry (void) :
					dwOrderType(0),
					dwDataType(IShipController::dataNone),
					pTarget(NULL),
					dwData(0)
				{ }

			DWORD dwOrderType:8;			//	IShipController::OrderTypes
			DWORD dwDataType:8;				//	EDataTypes
			DWORD dwSpare:16;

			CSpaceObject *pTarget;
			DWORD dwData;
			};

		void CleanUp (SOrderEntry *pEntry);
		const SOrderEntry &GetCurrentEntry (void) const { return (m_List.GetCount() > 0 ? m_List[0] : m_NullOrder); }
		SOrderEntry &GetCurrentEntryActual (void) { return (m_List.GetCount() > 0 ? m_List[0] : m_NullOrder); }
		void SetEntryData (SOrderEntry *pEntry, const IShipController::SData &Data);

		TArray<SOrderEntry> m_List;

		static SOrderEntry m_NullOrder;
	};

class CBaseShipAI : public CObject, public IShipController
	{
	public:
		CBaseShipAI (IObjectClass *pClass);
		virtual ~CBaseShipAI (void);

		//	IShipController virtuals
		virtual void Behavior (void);
		virtual CString DebugCrashInfo (void);
		virtual void DebugPaintInfo (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool FollowsObjThroughGate (CSpaceObject *pLeader);
		virtual CString GetAISetting (const CString &sSetting) { return m_AICtx.GetAISetting(sSetting); }
		virtual const CAISettings *GetAISettings (void) { return &m_AICtx.GetAISettings(); }
		virtual CSpaceObject *GetBase (void) const;
		virtual int GetCombatPower (void);
		virtual bool GetDeviceActivate (void) { return m_fDeviceActivate; }
		virtual CSpaceObject *GetEscortPrincipal (void) const;
		virtual int GetFireRateAdj (void) { return m_AICtx.GetFireRateAdj(); }
		virtual EManeuverTypes GetManeuver (void) { return m_AICtx.GetManeuver(); }
		virtual CSpaceObject *GetOrderGiver (void);
		virtual bool GetReverseThrust (void) { return false; }
		virtual CSpaceObject *GetShip (void) { return m_pShip; }
		virtual bool GetStopThrust (void) { return false; }
		virtual CSpaceObject *GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget = false) const;
		virtual bool GetThrust (void) { return m_AICtx.GetThrust(m_pShip); }
		virtual void GetWeaponTarget (STargetingCtx &TargetingCtx, CItemCtx &ItemCtx, CSpaceObject **retpTarget, int *retiFireSolution);
		virtual bool IsAngryAt (CSpaceObject *pObj) const;
		virtual bool IsPlayerWingman (void) const { return (m_fIsPlayerWingman ? true : false); }
		virtual void OnAttacked (CSpaceObject *pAttacker, const DamageDesc &Damage);
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);
		virtual void OnDocked (CSpaceObject *pObj);
		virtual void OnEnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate, bool bAscend);
		virtual void OnNewSystem (CSystem *pSystem);
		virtual void OnWeaponStatusChanged (void) { m_AICtx.ClearBestWeapon(); }
		virtual void OnObjEnteredGate (CSpaceObject *pObj, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate);
		virtual void OnObjDestroyed (const SDestroyCtx &Ctx);
		virtual void OnPlayerChangedShips (CSpaceObject *pOldShip);
		virtual void OnPlayerObj (CSpaceObject *pPlayer);
		virtual void OnStationDestroyed (const SDestroyCtx &Ctx);
		virtual void OnStatsChanged (void) { m_AICtx.CalcInvariants(m_pShip); }
		virtual void OnSystemLoaded (void) { m_AICtx.CalcInvariants(m_pShip); OnSystemLoadedNotify(); }
		virtual CString SetAISetting (const CString &sSetting, const CString &sValue) { CString sNew = m_AICtx.SetAISetting(sSetting, sValue); m_AICtx.CalcInvariants(m_pShip); return sNew; }
		virtual void SetCommandCode (ICCItem *pCode);
		virtual void SetManeuver (EManeuverTypes iManeuver) { m_AICtx.SetManeuver(iManeuver); }
		virtual void SetShipToControl (CShip *pShip);
		virtual void SetThrust (bool bThrust) { m_AICtx.SetThrust(bThrust); }
		virtual void SetPlayerWingman (bool bIsWingman) { m_fIsPlayerWingman = bIsWingman; }
		virtual void ReadFromStream (SLoadCtx &Ctx, CShip *pShip);
		virtual void WriteToStream (IWriteStream *pStream);

		virtual void AddOrder (IShipController::OrderTypes Order, CSpaceObject *pTarget, const IShipController::SData &Data, bool bAddBefore = false);
		virtual void CancelAllOrders (void);
		virtual void CancelCurrentOrder (void);
		virtual DWORD GetCurrentOrderData (void) { return m_Orders.GetCurrentOrderData(); }
		virtual OrderTypes GetCurrentOrderEx (CSpaceObject **retpTarget = NULL, IShipController::SData *retData = NULL);

	protected:
		CSpaceObject *CalcEnemyShipInRange (CSpaceObject *pCenter, Metric rRange, CSpaceObject *pExcludeObj = NULL);
		Metric CalcShipIntercept (const CVector &vRelPos, const CVector &vAbsVel, Metric rMaxSpeed);
		int CalcWeaponScore (CSpaceObject *pTarget, CInstalledDevice *pWeapon, Metric rTargetDist2);
		void CancelDocking (CSpaceObject *pTarget);
		bool CheckForEnemiesInRange (CSpaceObject *pCenter, Metric rRange, int iInterval, CSpaceObject **retpTarget);
		bool CheckOutOfRange (CSpaceObject *pTarget, Metric rRange, int iInterval);
		bool CheckOutOfZone (CSpaceObject *pBase, Metric rInnerRadius, Metric rOuterRadius, int iInterval);
		void FireOnOrderChanged (void);
		void FireOnOrdersCompleted (void);
		IShipController::OrderTypes GetCurrentOrder (void) const;
		inline CSpaceObject *GetCurrentOrderTarget (void) const { return m_Orders.GetCurrentOrderTarget(); }
		inline Metric GetDistance (CSpaceObject *pObj) const { return (pObj->GetPos() - m_pShip->GetPos()).Length(); }
		inline Metric GetDistance2 (CSpaceObject *pObj) const { return (pObj->GetPos() - m_pShip->GetPos()).Length2(); }
		inline int GetOrderCount (void) const { return m_Orders.GetCount(); }
		CSpaceObject *GetPlayerOrderGiver (void) const;
		inline bool IsImmobile (void) const { return m_AICtx.IsImmobile(); }
		bool IsPlayerOrPlayerFollower (CSpaceObject *pObj, int iRecursions = 0);
		bool IsWaitingForShieldsToRegen (void) { return m_AICtx.IsWaitingForShieldsToRegen(); }
		void HandleFriendlyFire (CSpaceObject *pOrderGiver);
		inline bool IsDockingRequested (void) { return m_AICtx.IsDockingRequested(); }
		void ResetBehavior (void);
		void SetCurrentOrderData (const SData &Data);
		void UpgradeShieldBehavior (void);
		void UpgradeWeaponBehavior (void);
		void UseItemsBehavior (void);

		//	CBaseShipAI virtuals
		virtual void OnAttackedNotify (CSpaceObject *pAttacker, const DamageDesc &Damage) { }
		virtual void OnBehavior (void) { }
		virtual void OnCleanUp (void) { }
		virtual DWORD OnCommunicateNotify (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2) { return resNoAnswer; }
		virtual CString OnDebugCrashInfo (void) { return NULL_STR; }
		virtual void OnDockedEvent (CSpaceObject *pObj) { }
		virtual CSpaceObject *OnGetBase (void) const { return NULL; }
		virtual CSpaceObject *OnGetTarget (bool bNoAutoTarget = false) const { return NULL; }
		virtual void OnNewSystemNotify (void) { }
		virtual void OnOrderChanged (void) { }
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx) { }
		virtual void OnReadFromStream (SLoadCtx &Ctx) { }
		virtual void OnSystemLoadedNotify (void) { }
		virtual void OnWriteToStream (IWriteStream *pStream) { }

		CShip *m_pShip;							//	Ship that we're controlling
		COrderList m_Orders;					//	Ordered list of orders
		ICCItem *m_pCommandCode;				//	Code to generate orders (may be NULL)

		CAIBehaviorCtx m_AICtx;					//	Ctx for behavior
		IOrderModule *m_pOrderModule;			//	Current order module (may be NULL)

		CAttackDetector m_Blacklist;			//	Player blacklisted

		//	Flags
		DWORD m_fDeviceActivate:1;
		DWORD m_fInOnOrderChanged:1;
		DWORD m_fInOnOrdersCompleted:1;
		DWORD m_fCheckedForWalls:1;				//	TRUE if we searched for walls in the system
		DWORD m_fAvoidWalls:1;					//	TRUE if we need to avoid walls
		DWORD m_fIsPlayerWingman:1;				//	TRUE if this is a wingman for the player
		DWORD m_fOldStyleBehaviors:1;			//	TRUE if we're not using m_pOrderModule
		DWORD m_fSpare:25;
	};

class CAutonAI : public CBaseShipAI
	{
	public:
		CAutonAI (void);

		//	IShipController virtuals
		virtual void Behavior (void);
		virtual CString DebugCrashInfo (void);
		virtual CSpaceObject *GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget = false) const;
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);

	protected:
		//	CBaseShipAI overrides
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx);
		virtual void OnOrderChanged (void);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnSystemLoadedNotify (void);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum StateTypes
			{
			stateNone,
			stateEscorting,
			stateAttackingTarget,
			stateOnCourseForStargate,
			stateFollowing,
			stateWaiting,
			};

		void BehaviorStart (void);
		void SetState (StateTypes State);

		StateTypes m_State;
		CSpaceObject *m_pDest;
		CSpaceObject *m_pTarget;

	friend CObjectClass<CAutonAI>;
	};

class CFerianShipAI : public CBaseShipAI
	{
	public:
		CFerianShipAI (void);

		//	IShipController virtuals
		virtual void Behavior (void);
		virtual CString DebugCrashInfo (void);
		virtual CSpaceObject *GetBase (void) const { return m_pBase; }
		virtual CSpaceObject *GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget = false) const { return m_pTarget; }
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);

	protected:
		//	CBaseShipAI overrides
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx);
		virtual void OnOrderChanged (void);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum StateTypes
			{
			stateNone,
			stateOnCourseForMine,
			stateMining,
			stateAttackingThreat,
			stateOnCourseForStargate,
			stateAvoidThreat,
			stateAttackingTarget,
			stateWaitForPlayerAtGate,
			};

		void BehaviorStart (void);
		CSpaceObject *FindRandomAsteroid (void);
		bool InRangeOfThreat (CSpaceObject **retpThreat);
		void SetState (StateTypes State);

		StateTypes m_State;						//	Ship's current state
		CSpaceObject *m_pBase;
		CSpaceObject *m_pTarget;

	friend CObjectClass<CFerianShipAI>;
	};

class CFleetCommandAI : public CBaseShipAI
	{
	public:
		CFleetCommandAI (void);
		virtual ~CFleetCommandAI (void);

		//	IShipController virtuals
		virtual void Behavior (void);
		virtual CString DebugCrashInfo (void);
		virtual CSpaceObject *GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget = false) const { return m_pTarget; }
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);

	protected:
		//	CBaseShipAI overrides
		virtual void OnDockedEvent (CSpaceObject *pObj);
		virtual void OnNewSystemNotify (void);
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx);
		virtual void OnOrderChanged (void);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum StateTypes
			{
			stateNone,
			stateAttackAtWill,
			stateChargeInFormation,
			stateFormAtRallyPoint,
			stateAttackFromRallyPoint,
			stateOnCourseForStargate,
			stateWaiting,
			stateWaitingForThreat,
			};

		struct STarget
			{
			CSpaceObject *pTarget;
			int iAssignedTo;					//	Power points assigned to destroy this target
			int iKilled;						//	Power points killed by this asset
			};

		struct SAsset
			{
			CSpaceObject *pAsset;
			int iFormationPos;
			CSpaceObject *pTarget;				//	Assigned target (NULL = none)
			};

		SAsset *AddAsset (CSpaceObject *pAsset);
		void AddTarget (CSpaceObject *pTarget);
		void BehaviorStart (void);
		void ComputeCombatPower (int *retiAssetPower, int *retiTargetPower);
		bool ComputeFormationReady (void);
		CVector ComputeRallyPoint (CSpaceObject *pTarget, int *retiFacing = NULL);
		CVector ComputeRallyPointEx (int iBearing, CSpaceObject *pTarget, int *retiFacing = NULL);
		SAsset *FindAsset (CSpaceObject *pAsset, int *retiIndex = NULL);
		STarget *FindTarget (CSpaceObject *pTarget, int *retiIndex = NULL);
		void ImplementAttackAtWill (void);
		void ImplementAttackFromRallyPoint (void);
		void ImplementChargeInFormation (void);
		void ImplementFormAtRallyPoint (void);
		void ImplementWaitAtPosition (void);
		void OrderAllFormUp (bool bResetFormation = false);
		void OrderAllOpenFire (void);
		void OrderAttackTarget (CSpaceObject *pTarget);
		void RemoveAsset (int iIndex);
		bool RemoveAssetObj (CSpaceObject *pObj);
		void RemoveTarget (int iIndex);
		void SetState (StateTypes State);
		void UpdateAttackTargets (void);
		void UpdateTargetList (void);
		void UpdateTargetListAndPotentials (void);

		StateTypes m_State;

		CSpaceObject *m_pTarget;
		CSpaceObject *m_pObjective;
		int m_iCounter;

		int m_iStartingAssetCount;
		int m_iAssetCount;
		int m_iAssetAlloc;
		SAsset *m_pAssets;

		int m_iTargetCount;
		int m_iTargetAlloc;
		STarget *m_pTargets;

		CVector m_vThreatPotential;
		CVector m_vRallyPoint;
		int m_iRallyFacing;

		DWORD m_fOpenFireOrdered:1;
		DWORD m_dwSpare:31;

	friend CObjectClass<CFleetCommandAI>;
	};

class CFleetShipAI : public CBaseShipAI
	{
	public:
		CFleetShipAI (void);

		//	IShipController virtuals
		virtual void Behavior (void);
		virtual CString DebugCrashInfo (void);
		virtual CSpaceObject *GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget = false) const;
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);

	protected:
		//	CBaseShipAI overrides
		virtual void OnDockedEvent (CSpaceObject *pObj);
		virtual CSpaceObject *OnGetBase (void) const;
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx);
		virtual void OnOrderChanged (void);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum StateTypes
			{
			stateNone,
			stateKeepFormation,
			stateAttackAtWill,
			stateAttackInFormation,
			stateAttackTarget,
			stateOnCourseForStargate,				//	Heading for m_pDest to gate out
			stateOnCourseForDocking,				//	Heading for m_pDest to dock
			stateWaitForPlayerAtGate,
			stateOnPatrolOrbit,
			stateAttackOnPatrol,
			stateWaiting,
			};

		void BehaviorStart (void);
		CVector ComputeAttackPos (CSpaceObject *pTarget, Metric rDist, int *retiFacing);
		void ImplementFireOnNearestTarget (void);
		void ImplementKeepFormation (bool *retbInFormation = NULL);
		void SetState (StateTypes State);

		StateTypes m_State;						//	Ship's current state
		CSpaceObject *m_pLeader;				//	Group leader
		int m_iFormation;						//	Formation to use
		int m_iPlace;							//	Place in formation
		int m_iCounter;							//	Misc counter
		int m_iAngle;							//	Misc angle
		CVector m_vVector;				//	Misc vector

		CSpaceObject *m_pTarget;
		CSpaceObject *m_pDest;

	friend CObjectClass<CFleetShipAI>;
	};

class CGaianProcessorAI : public CBaseShipAI
	{
	public:
		CGaianProcessorAI (void);

		//	IShipController virtuals
		virtual void Behavior (void);
		virtual CString DebugCrashInfo (void);
		virtual CSpaceObject *GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget = false) const;
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);

	protected:
		//	CBaseShipAI overrides
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx);
		virtual void OnOrderChanged (void);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnSystemLoadedNotify (void);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum StateTypes
			{
			stateNone,
			stateLookingForWreck,
			statePulverizingWreck,
			stateConsumingWreck
			};

		void BehaviorStart (void);
		void CalcDevices (void);
		void SetState (StateTypes State);

		StateTypes m_State;						//	Ship's current state
		Metric m_rOrbitRadius;					//	Current orbital radius
		CSpaceObject *m_pTarget;
		CSpaceObject *m_pDest;					//	Scrap to eat
		int m_iStartTime;						//	Start time for various actions

		//	Volatiles
		int m_iDestructorDev;					//	Destructor weapon
		int m_iDisablerDev;						//	Disabling weapon
		DWORD m_dwAmmo;							//	Ammo to manufacture

	friend CObjectClass<CGaianProcessorAI>;
	};

class CGladiatorAI : public CBaseShipAI
	{
	public:
		CGladiatorAI (void);

		//	IShipController virtuals
		virtual void Behavior (void);
		virtual CString DebugCrashInfo (void);
		virtual CSpaceObject *GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget = false) const;

	protected:
		//	CBaseShipAI overrides
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx);
		virtual void OnOrderChanged (void);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum StateTypes
			{
			stateNone,
			stateAttackingTarget,
			stateOnCourseForDocking,
			stateOnCourseForStargate,
			};

		void BehaviorStart (void);
		CVector ComputePotential (void);
		void SetState (StateTypes State);

		StateTypes m_State;
		CSpaceObject *m_pDest;
		CSpaceObject *m_pTarget;
		CVector m_vPotential;

	friend CObjectClass<CGladiatorAI>;
	};

class CStandardShipAI : public CBaseShipAI
	{
	public:
		CStandardShipAI (void);
		virtual ~CStandardShipAI (void);

		static void SetDebugShip (CShip *pShip);

	protected:
		//	CBaseShipAI overrides
		virtual void OnAttackedNotify (CSpaceObject *pAttacker, const DamageDesc &Damage);
		virtual void OnBehavior (void);
		virtual void OnCleanUp (void) { m_State = stateNone; m_pTarget = NULL; m_pDest = NULL; }
		virtual DWORD OnCommunicateNotify (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);
		virtual CString OnDebugCrashInfo (void);
		virtual void OnDockedEvent (CSpaceObject *pObj);
		virtual CSpaceObject *OnGetBase (void) const;
		virtual CSpaceObject *OnGetTarget (bool bNoAutoTarget = false) const;
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx);
		virtual void OnOrderChanged (void);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum StateTypes
			{
			stateAttackingOnPatrol,					//	Attacking m_pTarget while on patrol

			stateAttackingTarget,					//	Attacking m_pTarget
			stateAttackingThreat,					//	Attacking m_pTarget to protect principal
			stateDeterTarget,						//	Attack m_pTarget, but stop if target doesn't attack back
			stateDeterTargetNoChase,				//	Attack m_pTarget, but don't chase it
			stateEscorting,							//	Escorting m_pDest
			stateHolding,							//	Doing nothing indefinitely
			stateLookingForLoot,					//	Looking for loot in the system
			stateLooting,							//	Looting stuff from station
			stateLootingOnce,						//	Looting stuff from station
			stateMaintainBearing,					//	Move in m_rDistance (angle) direction
													//		until we run into enemies.
			stateNone,								//	Ship needs to consult orders
			stateOnCourseForDocking,				//	Heading for m_pDest to dock
			stateOnCourseForStargate,				//	Heading for m_pDest to gate out
			stateOnPatrolOrbit,						//	Orbiting m_pDest at m_rDistance
			stateReturningFromThreat,				//	Dock with m_pDest
			stateWaiting,							//	Waiting for countdown (m_iCountdown)
			stateWaitingForThreat,					//	Waiting for a threat to m_pDest
			stateWaitingUnarmed,					//	If threat shows up, we gate out
			stateWaitForPlayerAtGate,				//	Waiting for player at gate
			stateFollowing,

			stateOnCourseForLootDocking,			//	Going to dock with loot
			stateDeterTargetWhileLootDocking,		//	Attack the target, but continue towards loot
			stateFollowNavPath,						//	Follow current nav path
			stateReturningViaNavPath,				//	Follow current nav path
			stateOnCourseForDockingViaNavPath,		//	Follow current nav path
			stateOnCourseForPatrolViaNavPath,		//	Follow current nav path
			stateOnCourseForPoint,					//	Going to a given point
			stateWaitingForTarget,					//	Waiting for m_pTarget
			stateWaitingForEnemy,					//	Waiting for an enemy
			stateAttackingTargetAndAvoiding,		//	Attacking m_pTarget and avoiding enemy stations
			
			stateAvoidingEnemyStation,				//	Attacking m_pTarget and avoiding m_pDest
			stateOnCourseForPointViaNavPath,		//	Follow current nav path
			stateBombarding,						//	Attacking m_pTarget while holding
			stateApproaching,						//	Get within m_rDistance of m_pDest
			stateAimingAtTarget,					//	Aiming at m_pTarget
			stateOnCourseForOrbitViaNavPath,		//	Follow current nav path
			stateOrbit,								//	Orbit m_pTarget
			stateReturningToEscort,					//	Return to m_pDest from an attack
			stateHoldingCourse,						//	Hold course
			stateAttackingPrincipalThreat,			//	Attacking m_pTarget, which attacked principal

			stateTurningTo,							//	Turning to angle
			stateAttackingTargetAndHolding,			//	Attacking m_pTarget while holding position
			};

		struct TacticsSet
			{
			DWORD fCloseOnTarget:1;					//	Header for target
			DWORD fFireAtTarget:1;					//	Fire at target
			DWORD fFireOnOpportunity:1;				//	Fire at any target of opportunity
			DWORD fFlankTarget:1;					//	When closing on a target, aim a little to the side
			DWORD fSpiralToTarget:1;				//	Spiral towards target
			DWORD fSpiralAwayFromTarget:1;			//	Spiral away from target
			DWORD fEscortTarget:1;					//	Follow target
			};

		void AttackShipInRange (CSpaceObject *pCenter, Metric rRange);
		void BehaviorStart (void);
		void CommunicateWithEscorts (MessageTypes iMessage, CSpaceObject *pParam1 = NULL, DWORD dwParam2 = 0);
		void ImplementTactics (TacticsSet &Tactics, CVector &vTarget, Metric rTargetDist2);
		bool IsBeingAttacked (void);
		void SetState (StateTypes State);

		//	Orders and state
		StateTypes m_State;						//	Ship's current state
		CSpaceObject *m_pDest;					//	Destination
		CSpaceObject *m_pTarget;				//	Ship's target
		Metric m_rDistance;						//	Distance
		int m_iCountdown;						//	Timer

		//	Flags
		DWORD m_fHasEscorts:1;					//	TRUE if ship has escorts

	friend CObjectClass<CStandardShipAI>;
	};

class CZoanthropeAI : public CBaseShipAI
	{
	public:
		CZoanthropeAI (void);

		//	IShipController virtuals
		virtual void Behavior (void);
		virtual CString DebugCrashInfo (void);
		virtual CSpaceObject *GetBase (void) const;
		virtual CSpaceObject *GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget = false) const;
		virtual void OnAttacked (CSpaceObject *pAttacker, const DamageDesc &Damage);
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);

	protected:
		//	CBaseShipAI overrides
		virtual void OnDockedEvent (CSpaceObject *pObj);
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx);
		virtual void OnOrderChanged (void);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum StateTypes
			{
			stateNone,
			stateAttackingThreat,
			stateOnCourseForStargate,
			stateReturningFromThreat,
			stateWaitingForThreat,
			stateOnPatrolOrbit,
			stateAttackingOnPatrol,
			stateOnEscortCourse,
			stateWaiting,
			};

		void BehaviorStart (void);
		void ImplementCombatManeuvers (CSpaceObject *pTarget);
		void SetState (StateTypes State, CSpaceObject *pBase = NULL, CSpaceObject *pTarget = NULL);

		StateTypes m_State;						//	Ship's current state
		CSpaceObject *m_pBase;
		CSpaceObject *m_pTarget;

		CVector m_vPotential;

	friend CObjectClass<CZoanthropeAI>;
	};

IShipController *CreateShipController (const CString &sAI);

//	Inlines --------------------------------------------------------------------

inline bool CAIShipControls::GetThrust (CShip *pShip) const 
	{
	switch (m_iThrustDir)
		{
		case constNeverThrust:
			return false;

		case constAlwaysThrust:
			return true;

		default:
			return pShip->IsPointingTo(m_iThrustDir);
		}
	}

#endif
