//	TSEDeviceClassesImpl.h
//
//	Transcendence Device Classes

#ifndef INCL_TSE_DEVICE_CLASSES
#define INCL_TSE_DEVICE_CLASSES

class CAutoDefenseClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		//	CDeviceClass virtuals

		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual int GetActivateDelay (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual ItemCategories GetCategory (void) const { return itemcatMiscDevice; }
		virtual int GetDamageType (CInstalledDevice *pDevice = NULL, int iVariant = -1);
		virtual int GetPowerRating (CItemCtx &Ctx);
		virtual CString GetReference (CItemCtx &Ctx, int iVariant = -1, DWORD dwFlags = 0);
		virtual bool GetReferenceDamageType (CItemCtx &Ctx, int iVariant, DamageTypes *retiDamage, CString *retsReference) const;
		virtual bool IsAutomatedWeapon (void) { return true; }
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL);

	protected:
		virtual void OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);

	private:
		enum TargetingSystemTypes
			{
			trgMissiles,				//	Target nearest missile
			trgCriteria,				//	Target nearest object that matches criteria
			trgFunction,				//	Target computed by a function
			};

		CAutoDefenseClass (void);

		inline CDeviceClass *GetWeapon (void) const { return m_pWeapon; }

		TargetingSystemTypes m_iTargeting;
		CSpaceObject::Criteria m_TargetCriteria;

		int m_iRechargeTicks;
		CDeviceClassRef m_pWeapon;
	};

class CCargoSpaceClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		//	CDeviceClass virtuals

		virtual bool CanBeDamaged (void) { return false; }
		virtual bool CanBeDisabled (CItemCtx &Ctx) { return false; }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual ItemCategories GetCategory (void) const { return itemcatCargoHold; }
		virtual int GetCargoSpace (void) { return m_iCargoSpace; }
		virtual void OnInstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList);

	private:
		CCargoSpaceClass (void);

		int m_iCargoSpace;
	};

class CCyberDeckClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		//	CDeviceClass virtuals

		virtual bool Activate (CInstalledDevice *pDevice, 
							   CSpaceObject *pSource, 
							   CSpaceObject *pTarget,
							   bool *retbSourceDestroyed,
							   bool *retbConsumedItems = NULL);
		virtual bool CanHitFriends (void) { return false; }
		virtual int GetActivateDelay (CInstalledDevice *pDevice, CSpaceObject *pSource) { return 30; }
		virtual ItemCategories GetCategory (void) const { return itemcatWeapon; }
		virtual int GetDamageType (CInstalledDevice *pDevice = NULL, int iVariant = -1) { return damageGeneric; }
		virtual Metric GetMaxEffectiveRange (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget);
		virtual CString GetReference (CItemCtx &Ctx, int iVariant = -1, DWORD dwFlags = 0);
		virtual void GetSelectedVariantInfo (CSpaceObject *pSource, 
											 CInstalledDevice *pDevice,
											 CString *retsLabel,
											 int *retiAmmoLeft,
											 CItemType **retpType = NULL);
		virtual int GetValidVariantCount (CSpaceObject *pSource, CInstalledDevice *pDevice) { return 1; }
		virtual int GetWeaponEffectiveness (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget);
		virtual bool IsVariantSelected (CSpaceObject *pSource, CInstalledDevice *pDevice) { return true; }
		virtual bool IsWeaponAligned (CSpaceObject *pShip, CInstalledDevice *pDevice, CSpaceObject *pTarget, int *retiAimAngle = NULL, int *retiFireAngle = NULL);
		virtual bool SelectFirstVariant (CSpaceObject *pSource, CInstalledDevice *pDevice) { return true; }
		virtual bool SelectNextVariant (CSpaceObject *pSource, CInstalledDevice *pDevice, int iDir = 1) { return true; }
		virtual bool ValidateSelectedVariant (CSpaceObject *pSource, CInstalledDevice *pDevice) { return true; }

	private:
		CCyberDeckClass (void);

		int m_iRange;							//	in light-seconds
		int m_iAttackChance;					//	%chance of success per attack

		ProgramDesc m_Program;					//	Program description
	};

class CDriveClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDrive);

		//	CDeviceClass virtuals

		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual ItemCategories GetCategory (void) const { return itemcatDrive; }
		virtual const DriveDesc *GetDriveDesc (CInstalledDevice *pDevice = NULL, CSpaceObject *pSource = NULL);
		virtual int GetPowerRating (CItemCtx &Ctx);
		virtual void OnInstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList);

	private:
		CDriveClass (void);

		DriveDesc m_DriveDesc;
		DriveDesc m_DamagedDriveDesc;
		DriveDesc m_EnhancedDriveDesc;
	};

class CEnhancerClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		//	CDeviceClass virtuals

		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual ItemCategories GetCategory (void) const { return itemcatMiscDevice; }
		virtual bool GetDeviceEnhancementDesc (CInstalledDevice *pDevice, CSpaceObject *pSource, CInstalledDevice *pWeapon, SDeviceEnhancementDesc *retDesc);
		virtual int GetPowerRating (CItemCtx &Ctx) { return m_iPowerUse; }

	protected:
		virtual bool OnAccumulateEnhancements (CItemCtx &Device, CInstalledDevice *pTarget, TArray<CString> &EnhancementIDs, CItemEnhancementStack *pEnhancements);

	private:
		CEnhancerClass (void);

		CString m_sEnhancementType;			//	Type of enhancement
		CItemCriteria m_Criteria;			//	Only enhances items that match criteria

		int m_iDamageAdj;					//	Adjustment to damage

		int m_iActivateAdj;					//	Fire rate adj (100 = none)
		int m_iMinActivateDelay;			//	Minimum rate (0 = no min)
		int m_iMaxActivateDelay;			//	Maximum rate (0 = no max)

		bool m_bUseArray;						//	If TRUE, use this array
		int m_iDamageAdjArray[damageCount];		//	Adjustment to weapons damage

		int m_iPowerUse;
	};

class CMiscellaneousClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		//	CDeviceClass virtuals

		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual int GetActivateDelay (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual ItemCategories GetCategory (void) const { return itemcatMiscDevice; }
		virtual int GetCounter (CInstalledDevice *pDevice, CSpaceObject *pSource, CounterTypes *retiType = NULL);
		virtual int GetPowerRating (CItemCtx &Ctx);
		virtual bool ShowActivationDelayCounter (CSpaceObject *pSource, CInstalledDevice *pDevice);
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL);

	private:
		CMiscellaneousClass (void);

		int GetCapacitorPowerUse (CInstalledDevice *pDevice, CSpaceObject *pSource);

		int m_iPowerRating;					//	If >0, power rating for device
											//		This value is required if
											//		m_iPowerForCapacitor is negative
		int m_iPowerUse;					//	If >0, power used per update cycle
		int m_iPowerToActivate;				//	If >0, power used per activation
		int m_iPowerForCapacitor;			//	Required if m_iPowerToActivate is >0
											//		This is power used per cycle
											//		to charge for activation. If
											//		<0, this is a percent of max
											//		reactor power.
	};

class CReactorClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		static ALERROR InitReactorDesc (SDesignLoadCtx &Ctx, CXMLElement *pDesc, ReactorDesc *retDesc, bool bShipClass = false);
		static bool FindDataField (const ReactorDesc &Desc, const CString &sField, CString *retsValue);

		//	CDeviceClass virtuals

		virtual bool CanBeDisabled (CItemCtx &Ctx) { return false; }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual ItemCategories GetCategory (void) const { return itemcatReactor; }
		virtual const ReactorDesc *GetReactorDesc (CInstalledDevice *pDevice = NULL, CSpaceObject *pSource = NULL);
		virtual bool IsFuelCompatible (CItemCtx &Ctx, const CItem &FuelItem);
		virtual void OnInstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList);

	private:
		CReactorClass (void);

		ReactorDesc m_Desc;
		ReactorDesc m_DamagedDesc;
		ReactorDesc m_EnhancedDesc;
	};

class CRepairerClass : public CDeviceClass
	{
	public:
		enum ECachedHandlers
			{
			evtGetArmorRegen			= 0,

			evtCount					= 1,
			};

		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);
		inline bool FindEventHandlerRepairerClass (ECachedHandlers iEvent, SEventHandlerDesc *retEvent = NULL) const { if (retEvent) *retEvent = m_CachedEvents[iEvent]; return (m_CachedEvents[iEvent].pCode != NULL); }

		//	CDeviceClass virtuals

		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual ItemCategories GetCategory (void) const { return itemcatMiscDevice; }
		virtual int GetPowerRating (CItemCtx &Ctx) { return 2 * m_iPowerUse; }
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual void OnInstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList);
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL);

	private:
		CRepairerClass (void);

		void CalcRegen (CInstalledDevice *pDevice, CShip *pShip, int iSegment, int iTick, int *retiHP, int *retiPower);

		TArray<CRegenDesc> m_Repair;			//	Repair descriptor (by level)
		int m_iPowerUse;						//	Power used for each hp of repair

		SEventHandlerDesc m_CachedEvents[evtCount];		//	Cached events
	};

class CShieldClass : public CDeviceClass
	{
	public:
		enum ECachedHandlers
			{
			evtGetMaxHP					= 0,
			evtOnShieldDamage			= 1,
			evtOnShieldDown				= 2,

			evtCount					= 3,
			};

		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpShield);
		inline bool FindEventHandlerShieldClass (ECachedHandlers iEvent, SEventHandlerDesc *retEvent = NULL) const { if (retEvent) *retEvent = m_CachedEvents[iEvent]; return (m_CachedEvents[iEvent].pCode != NULL); }

		//	CDeviceClass virtuals

		virtual bool AbsorbsWeaponFire (CInstalledDevice *pDevice, CSpaceObject *pSource, CInstalledDevice *pWeapon);
		virtual bool AbsorbDamage (CInstalledDevice *pDevice, CSpaceObject *pShip, SDamageCtx &Ctx);
		virtual bool Activate (CInstalledDevice *pDevice, 
							   CSpaceObject *pSource, 
							   CSpaceObject *pTarget,
							   bool *retbSourceDestroyed,
							   bool *retbConsumedItems = NULL);
		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual void Deplete (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual ItemCategories GetCategory (void) const { return itemcatShields; }
		virtual int GetDamageEffectiveness (CSpaceObject *pAttacker, CInstalledDevice *pWeapon);
		virtual int GetPowerRating (CItemCtx &Ctx);
		virtual CString GetReference (CItemCtx &Ctx, int iVariant = -1, DWORD dwFlags = 0);
		virtual bool GetReferenceDamageAdj (const CItem *pItem, CSpaceObject *pInstalled, int *retiHP, int *retArray) const;
		virtual void GetStatus (CInstalledDevice *pDevice, CSpaceObject *pSource, int *retiStatus, int *retiMaxStatus);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);
		virtual void OnInstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList);
		virtual void Recharge (CInstalledDevice *pDevice, CShip *pShip, int iStatus);
		virtual bool RequiresItems (void);
		virtual void Reset (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL);

		static int GetStdCost (int iLevel);
		static int GetStdEffectiveHP (int iLevel);
		static int GetStdHP (int iLevel);
		static int GetStdPower (int iLevel);
		static int GetStdRegen (int iLevel);


	protected:
		virtual void OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual void OnMarkImages (void);

	private:
		CShieldClass (void);

		int CalcBalance (void);
		void CalcMinMaxHP (CItemCtx &Ctx, int iCharges, int iArmorSegs, int iTotalHP, int *retiMin, int *retiMax) const;
		bool IsDepleted (CInstalledDevice *pDevice);
		int FireGetMaxHP (CInstalledDevice *pDevice, CSpaceObject *pSource, int iMaxHP) const;
		void FireOnShieldDamage (CItemCtx &ItemCtx, SDamageCtx &Ctx);
		void FireOnShieldDown (CInstalledDevice *pDevice, CSpaceObject *pSource);
		int GetDamageAdj (CItemEnhancement Mods, const DamageDesc &Damage) const;
		int GetHPLeft (CInstalledDevice *pDevice, CSpaceObject *pSource);
		int GetMaxHP (CInstalledDevice *pDevice, CSpaceObject *pSource);
		bool UpdateDepleted (CInstalledDevice *pDevice);
		void SetDepleted (CInstalledDevice *pDevice, CSpaceObject *pSource);
		void SetHPLeft (CInstalledDevice *pDevice, int iHP);

		int m_iHitPoints;						//	Max HP
		int m_iArmorShield;						//	If non-zero then this is the
												//		This is the number of shield
												//		HPs that are generated by
												//		10 HP of average armor
		int m_iAbsorbAdj[damageCount];			//	Absorbtion adjustments
		int m_iDamageAdjLevel;					//	Level to use for intrinsic damage adj
		CDamageAdjDesc m_DamageAdj;				//	Adjustments for damage type
		CRegenDesc m_Regen;						//	Regeneration rate
		int m_iDepletionTicks;					//	Ticks to recover from full depletion
		int m_iPowerUse;						//	Power used during regeneration (1/10 megawatt)
		int m_iIdlePowerUse;					//	Power used to maintain shields
		DamageTypeSet m_WeaponSuppress;			//	Types of weapons suppressed
		DamageTypeSet m_Reflective;				//	Types of damage reflected

		int m_iMaxCharges;						//	Max charges
		int m_iExtraHPPerCharge;				//	Extra HP for each point of charge
		int m_iExtraPowerPerCharge;				//	Extra power use for each point of charge (1/10 megawatt)
		int m_iExtraRegenPerCharge;				//	Extra regen/180 ticks per point of charge

		SEventHandlerDesc m_CachedEvents[evtCount];		//	Cached events

		CEffectCreatorRef m_pHitEffect;			//	Effect when shield is hit
	};

class CSolarDeviceClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		//	CDeviceClass virtuals

		virtual bool CanBeDisabled (CItemCtx &Ctx) { return false; }
		virtual ItemCategories GetCategory (void) const { return itemcatMiscDevice; }
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL);
		virtual void OnInstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList);

	private:
		CSolarDeviceClass (void);

		int m_iRefuel;							//	Units of fuel recharged per 10 ticks
												//	at 100% intensity.
	};

class CWeaponClass : public CDeviceClass
	{
	public:
		enum ECachedHandlers
			{
			evtOnFireWeapon				= 0,

			evtCount					= 1,
			};

		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpWeapon);
		virtual ~CWeaponClass (void);

		inline bool FindEventHandlerWeaponClass (ECachedHandlers iEvent, SEventHandlerDesc *retEvent = NULL) const { if (retEvent) *retEvent = m_CachedEvents[iEvent]; return (m_CachedEvents[iEvent].pCode != NULL); }
		CWeaponFireDesc *GetSelectedShotData (CItemCtx &Ctx);
		inline int GetVariantCount (void) { return m_ShotData.GetCount(); }
		inline CWeaponFireDesc *GetVariant (int iIndex) const { return m_ShotData[iIndex].pDesc; }

		static int GetStdDamage (int iLevel);
		static bool IsStdDamageType (DamageTypes iDamageType, int iLevel);

		//	CDeviceClass virtuals

		virtual bool Activate (CInstalledDevice *pDevice, 
							   CSpaceObject *pSource, 
							   CSpaceObject *pTarget,
							   bool *retbSourceDestroyed,
							   bool *retbConsumedItems = NULL);
		virtual CWeaponClass *AsWeaponClass (void) { return this; }
		virtual int CalcFireSolution (CInstalledDevice *pDevice, CSpaceObject *pSource, CSpaceObject *pTarget);
		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual bool CanRotate (CItemCtx &Ctx, int *retiMinFireArc = NULL, int *retiMaxFireArc = NULL);
		virtual int GetActivateDelay (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual int GetAmmoVariant (const CItemType *pItem) const;
		virtual ItemCategories GetCategory (void) const;
		virtual int GetCounter (CInstalledDevice *pDevice, CSpaceObject *pSource, CounterTypes *retiType = NULL);
		virtual int GetDefaultFireAngle (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL);

		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual bool FindDataField (int iVariant, const CString &sField, CString *retsValue);
		virtual const DamageDesc *GetDamageDesc (CItemCtx &Ctx);
		virtual int GetDamageType (CInstalledDevice *pDevice = NULL, int iVariant = -1);
		virtual ICCItem *GetItemProperty (CItemCtx &Ctx, const CString &sName);
		virtual DWORD GetLinkedFireOptions (CItemCtx &Ctx);
		virtual Metric GetMaxEffectiveRange (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget);
		virtual int GetPowerRating (CItemCtx &Ctx);
		virtual CString GetReference (CItemCtx &Ctx, int iVariant = -1, DWORD dwFlags = 0);
		virtual bool GetReferenceDamageType (CItemCtx &Ctx, int iVariant, DamageTypes *retiDamage, CString *retsReference) const;
		virtual void GetSelectedVariantInfo (CSpaceObject *pSource, 
											 CInstalledDevice *pDevice,
											 CString *retsLabel,
											 int *retiAmmoLeft,
											 CItemType **retpType = NULL);
		virtual int GetValidVariantCount (CSpaceObject *pSource, CInstalledDevice *pDevice);
		virtual int GetWeaponEffectiveness (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget);
		virtual bool IsAmmoWeapon (void);
		virtual bool IsAreaWeapon (CSpaceObject *pSource, CInstalledDevice *pDevice);
		virtual bool IsTrackingWeapon (CItemCtx &Ctx);
		virtual bool IsVariantSelected (CSpaceObject *pSource, CInstalledDevice *pDevice);
		virtual bool IsWeaponAligned (CSpaceObject *pShip, CInstalledDevice *pDevice, CSpaceObject *pTarget, int *retiAimAngle = NULL, int *retiFireAngle = NULL);
		virtual bool NeedsAutoTarget (CItemCtx &Ctx, int *retiMinFireArc = NULL, int *retiMaxFireArc = NULL);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual bool RequiresItems (void);
		virtual bool SelectFirstVariant (CSpaceObject *pSource, CInstalledDevice *pDevice);
		virtual bool SelectNextVariant (CSpaceObject *pSource, CInstalledDevice *pDevice, int iDir = 1);
		virtual bool ValidateSelectedVariant (CSpaceObject *pSource, CInstalledDevice *pDevice);

	protected:
		virtual void OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);
		virtual void OnMarkImages (void);

	private:
		enum ConfigurationTypes
			{
			ctSingle,				//	single shot
			ctDual,					//	dual, parallel shots
			ctWall,					//	five parallel shots
			ctSpread2,				//	dual, spread shots
			ctSpread3,				//	three, spread shots
			ctSpread5,				//	five, spread shots
			ctDualAlternating,		//	alternate barrels
			ctCustom,				//	custom configuration
			};

		struct SConfigDesc
			{
			DiceRange Angle;		//	Offset from fire angle
			int iPosAngle;			//	Origin of shot
			Metric rPosRadius;		//	Origin of shot
			};

		struct SShotDesc
			{
			CItemTypeRef pAmmoType;	//	ItemType for ammo (may be NULL)
			CWeaponFireDesc *pDesc;	//	Pointer to descriptor (may be external)
			bool bOwned;			//	TRUE if we own the descriptor
			};

		enum EOnFireWeaponResults
			{
			resDefault,
			resNoShot,
			resShotFired,
			};

		enum Constants
			{
			CONTINUOUS_START = 0xff,
			};

		CWeaponClass (void);

		int CalcBalance (int iVariant);
		int CalcConfiguration (CItemCtx &ItemCtx, CWeaponFireDesc *pShot, int iFireAngle, CVector *ShotPosOffset, int *ShotDir, bool bSetAlternating);
		Metric CalcConfigurationMultiplier (CWeaponFireDesc *pShot = NULL, bool bIncludeFragments = true) const;
		Metric CalcDamage (CWeaponFireDesc *pShot) const;
		Metric CalcDamagePerShot (CWeaponFireDesc *pShot) const;
		int CalcFireAngle (CItemCtx &ItemCtx, Metric rSpeed, CSpaceObject *pTarget, bool *retbOutOfArc);
		EOnFireWeaponResults FireOnFireWeapon (CItemCtx &ItemCtx, 
											   CWeaponFireDesc *pShot,
											   const CVector &vSource,
											   CSpaceObject *pTarget,
											   int iFireAngle,
											   int iRepeatingCount);
		bool FireWeapon (CInstalledDevice *pDevice, 
						 CWeaponFireDesc *pShot, 
						 CSpaceObject *pSource, 
						 CSpaceObject *pTarget,
						 int iRepeatingCount,
						 bool *retbSourceDestroyed,
						 bool *retbConsumedItems);
		CWeaponFireDesc *GetReferenceShotData (CWeaponFireDesc *pShot, int *retiFragments = NULL) const;
		inline bool IsCapacitorEnabled (void) { return (m_Counter == cntCapacitor); }
		inline bool IsCounterEnabled (void) { return (m_Counter != cntNone); }
		bool IsDirectional (CInstalledDevice *pDevice, int *retiMinFireArc = NULL, int *retiMaxFireArc = NULL);
		bool IsOmniDirectional (CInstalledDevice *pDevice);
		inline bool IsTemperatureEnabled (void) { return (m_Counter == cntTemperature); }
		bool VariantIsValid (CSpaceObject *pSource, CInstalledDevice *pDevice, CWeaponFireDesc &ShotData);

		int GetAlternatingPos (CInstalledDevice *pDevice);
		DWORD GetContinuousFire (CInstalledDevice *pDevice);
		int GetCurrentVariant (CInstalledDevice *pDevice);
		void SetAlternatingPos (CInstalledDevice *pDevice, int iAlternatingPos);
		void SetContinuousFire (CInstalledDevice *pDevice, DWORD dwContinuous);
		void SetCurrentVariant (CInstalledDevice *pDevice, int iVariant);

		int m_iFireRate;						//	Ticks between shots
		int m_iFireRateSecs;					//	Game seconds between shots
		int m_iPowerUse;						//	Power use to recharge capacitors (1/10 megawatt)
		int m_iIdlePowerUse;					//	Power use when capacitors fully charged
		int m_iRecoil;							//	0-7 (as per momentum damage)
		int m_iFailureChance;					//	Chance of failure

		bool m_bLauncher;						//	Generic missile launcher
		bool m_bOmnidirectional;				//	Omnidirectional
		bool m_bMIRV;							//	Each shot seeks an independent target
		bool m_bReportAmmo;						//	Report count of ammo shot even if not a launcher
		int m_iMinFireArc;						//	Min angle of fire arc (degrees)
		int m_iMaxFireArc;						//	Max angle of fire arc (degrees)
		DWORD m_dwLinkedFireOptions;			//	Linked fire options

		TArray<SShotDesc> m_ShotData;			//	Desc for each shot variation

		ConfigurationTypes m_Configuration;		//	Shot configuration;
		int m_iConfigCount;						//	Number of shots for custom configurations
		SConfigDesc *m_pConfig;					//	Custom configuration (may be NULL)
		int m_iConfigAimTolerance;				//	Aim tolerance
		bool m_bConfigAlternating;				//	Fire each shot in turn

		bool m_bCharges;						//	TRUE if weapon has charges instead of ammo

		CounterTypes m_Counter;					//	Counter type
		int m_iCounterUpdateRate;				//	Ticks to update counter
		int m_iCounterUpdate;					//	Inc/dec value per update
		int m_iCounterActivate;					//	Inc/dec value per shot

		bool m_bTargetStationsOnly;				//	Do not target ships

		SEventHandlerDesc m_CachedEvents[evtCount];	//	Cached events

	friend CObjectClass<CWeaponClass>;
	};

#endif

