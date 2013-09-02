//	TSEDeviceClasses.h
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
		virtual ItemCategories GetCategory (void) { return itemcatMiscDevice; }
		virtual int GetDamageType (CInstalledDevice *pDevice = NULL, int iVariant = -1);
		virtual int GetPowerRating (CItemCtx &Ctx);
		virtual CString GetReference (CItemCtx &Ctx, int iVariant = -1, DWORD dwFlags = 0);
		virtual bool GetReferenceDamageType (const CItem *pItem, CInstalledDevice *pDevice, CSpaceObject *pInstalled, int iVariant, DamageTypes *retiDamage, CString *retsReference) const;
		virtual bool IsAutomatedWeapon (void) { return true; }
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL);

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
		virtual bool CanBeDisabled (void) { return false; }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual ItemCategories GetCategory (void) { return itemcatCargoHold; }
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
							   int iFireAngle,
							   bool *retbSourceDestroyed,
							   bool *retbConsumedItems = NULL);
		virtual bool CanHitFriends (void) { return false; }
		virtual int GetActivateDelay (CInstalledDevice *pDevice, CSpaceObject *pSource) { return 30; }
		virtual ItemCategories GetCategory (void) { return itemcatWeapon; }
		virtual int GetDamageType (CInstalledDevice *pDevice = NULL, int iVariant = -1) { return damageNone; }
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
		virtual bool SelectNextVariant (CSpaceObject *pSource, CInstalledDevice *pDevice) { return true; }
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

		virtual bool CanBeDisabled (void) { return true; }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual ItemCategories GetCategory (void) { return itemcatDrive; }
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
		virtual ItemCategories GetCategory (void) { return itemcatMiscDevice; }
		virtual bool GetDeviceEnhancementDesc (CInstalledDevice *pDevice, CSpaceObject *pSource, CInstalledDevice *pWeapon, SDeviceEnhancementDesc *retDesc);
		virtual int GetPowerRating (CItemCtx &Ctx) { return m_iPowerUse; }

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
		virtual ItemCategories GetCategory (void) { return itemcatMiscDevice; }
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

		//	CDeviceClass virtuals

		virtual bool CanBeDisabled (void) { return false; }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual ItemCategories GetCategory (void) { return itemcatReactor; }
		virtual const ReactorDesc *GetReactorDesc (CInstalledDevice *pDevice = NULL, CSpaceObject *pSource = NULL);
		virtual bool IsFuelCompatible (CItemCtx &Ctx, const CItem &FuelItem);
		virtual void OnInstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList);

	private:
		CReactorClass (void);

		ReactorDesc m_Desc;
		ReactorDesc m_DamagedDesc;
		ReactorDesc m_EnhancedDesc;

		CItemCriteria m_FuelCriteria;
	};

class CRepairerClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		//	CDeviceClass virtuals

		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual ItemCategories GetCategory (void) { return itemcatMiscDevice; }
		virtual int GetPowerRating (CItemCtx &Ctx) { return 2 * m_iPowerUse; }
		virtual void OnInstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList);
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL);

	private:
		CRepairerClass (void);

		TArray<CRegenDesc> m_Repair;			//	Repair descriptor (by level)
		int m_iPowerUse;						//	Power used for each hp of repair
	};

class CShieldClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpShield);

		//	CDeviceClass virtuals

		virtual bool AbsorbsWeaponFire (CInstalledDevice *pDevice, CSpaceObject *pSource, CInstalledDevice *pWeapon);
		virtual bool AbsorbDamage (CInstalledDevice *pDevice, CSpaceObject *pShip, SDamageCtx &Ctx);
		virtual bool Activate (CInstalledDevice *pDevice, 
							   CSpaceObject *pSource, 
							   CSpaceObject *pTarget,
							   int iFireAngle,
							   bool *retbSourceDestroyed,
							   bool *retbConsumedItems = NULL);
		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual void Deplete (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual ItemCategories GetCategory (void) { return itemcatShields; }
		virtual int GetPowerRating (CItemCtx &Ctx);
		virtual CString GetReference (CItemCtx &Ctx, int iVariant = -1, DWORD dwFlags = 0);
		virtual bool GetReferenceDamageAdj (const CItem *pItem, CSpaceObject *pInstalled, int *retiHP, int *retArray) const;
		virtual void GetStatus (CInstalledDevice *pDevice, CShip *pShip, int *retiStatus, int *retiMaxStatus);
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
		virtual void OnLoadImages (void);
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
		int m_iDamageAdj[damageCount];			//	Adjustments for damage type
		int m_iRegenRate;						//	Ticks between regen
		int m_iRegenHP;							//	HP regenerated each cycle
		int m_iDepletionDelay;					//	Ticks to recover from full depletion
		int m_iPowerUse;						//	Power used during regeneration (1/10 megawatt)
		int m_iIdlePowerUse;					//	Power used to maintain shields
		DamageTypeSet m_WeaponSuppress;			//	Types of weapons suppressed
		DamageTypeSet m_Reflective;				//	Types of damage reflected

		int m_iMaxCharges;						//	Max charges
		int m_iExtraHPPerCharge;				//	Extra HP for each point of charge
		int m_iExtraPowerPerCharge;				//	Extra power use for each point of charge (1/10 megawatt)
		int m_iExtraRegenPerCharge;				//	Extra regen per 10 points of charge

		ICCItem *m_pOnShieldDamage;				//	Code to execute when shield is hit (may be NULL)
		ICCItem *m_pOnShieldDown;				//	Code to execute when shields go down (may be NULL)
		ICCItem *m_pGetMaxHP;					//	Code to compute max hit points (may be NULL)

		CEffectCreatorRef m_pHitEffect;			//	Effect when shield is hit
	};

class CSolarDeviceClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		//	CDeviceClass virtuals

		virtual bool CanBeDisabled (void) { return false; }
		virtual ItemCategories GetCategory (void) { return itemcatMiscDevice; }
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
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpWeapon);
		virtual ~CWeaponClass (void);

		inline int GetVariantCount (void) { return m_iShotVariants; }
		inline CWeaponFireDesc *GetVariant (int iIndex) { return &m_pShotData[iIndex]; }

		//	CDeviceClass virtuals

		virtual bool Activate (CInstalledDevice *pDevice, 
							   CSpaceObject *pSource, 
							   CSpaceObject *pTarget,
							   int iFireAngle,
							   bool *retbSourceDestroyed,
							   bool *retbConsumedItems = NULL);
		virtual CWeaponClass *AsWeaponClass (void) { return this; }
		virtual int CalcFireSolution (CInstalledDevice *pDevice, CSpaceObject *pSource, CSpaceObject *pTarget);
		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual int GetActivateDelay (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual int GetAmmoVariant (const CItemType *pItem) const;
		virtual ItemCategories GetCategory (void);
		virtual int GetCounter (CInstalledDevice *pDevice, CSpaceObject *pSource, CounterTypes *retiType = NULL);
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL);

		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual bool FindDataField (int iVariant, const CString &sField, CString *retsValue);
		virtual const DamageDesc *GetDamageDesc (CInstalledDevice *pDevice);
		virtual int GetDamageType (CInstalledDevice *pDevice = NULL, int iVariant = -1);
		virtual Metric GetMaxEffectiveRange (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget);
		virtual int GetPowerRating (CItemCtx &Ctx);
		virtual CString GetReference (CItemCtx &Ctx, int iVariant = -1, DWORD dwFlags = 0);
		virtual bool GetReferenceDamageType (const CItem *pItem, CInstalledDevice *pDevice, CSpaceObject *pInstalled, int iVariant, DamageTypes *retiDamage, CString *retsReference) const;
		virtual void GetSelectedVariantInfo (CSpaceObject *pSource, 
											 CInstalledDevice *pDevice,
											 CString *retsLabel,
											 int *retiAmmoLeft,
											 CItemType **retpType = NULL);
		virtual int GetValidVariantCount (CSpaceObject *pSource, CInstalledDevice *pDevice);
		virtual int GetWeaponEffectiveness (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget);
		virtual bool IsAreaWeapon (CSpaceObject *pSource, CInstalledDevice *pDevice);
		virtual bool IsVariantSelected (CSpaceObject *pSource, CInstalledDevice *pDevice);
		virtual bool IsWeaponAligned (CSpaceObject *pShip, CInstalledDevice *pDevice, CSpaceObject *pTarget, int *retiAimAngle = NULL, int *retiFireAngle = NULL);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual bool RequiresItems (void);
		virtual bool SelectFirstVariant (CSpaceObject *pSource, CInstalledDevice *pDevice);
		virtual bool SelectNextVariant (CSpaceObject *pSource, CInstalledDevice *pDevice);
		virtual bool ValidateSelectedVariant (CSpaceObject *pSource, CInstalledDevice *pDevice);

	protected:
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);

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

		enum EOnFireWeaponResults
			{
			resDefault,
			resNoShot,
			resShotFired,
			};

		CWeaponClass (void);

		int CalcBalance (int iVariant);
		int CalcConfiguration (CItemCtx &ItemCtx, CWeaponFireDesc *pShot, int iFireAngle, CVector *ShotPosOffset, int *ShotDir);
		int CalcConfigurationMultiplier (CWeaponFireDesc *pShot = NULL) const;
		Metric CalcDamagePerShot (CWeaponFireDesc *pShot) const;
		int CalcFireAngle (CItemCtx &ItemCtx, Metric rSpeed, CSpaceObject *pTarget);
		bool CanRotate (CInstalledDevice *pDevice);
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
						 int iFireAngle,
						 int iRepeatingCount,
						 bool *retbSourceDestroyed,
						 bool *retbConsumedItems);
		CWeaponFireDesc *GetReferenceShotData (CWeaponFireDesc *pShot, int *retiFragments = NULL) const;
		CWeaponFireDesc *GetSelectedShotData (CInstalledDevice *pDevice);
		inline bool IsCapacitorEnabled (void) { return (m_Counter == cntCapacitor); }
		inline bool IsCounterEnabled (void) { return (m_Counter != cntNone); }
		bool IsDirectional (CInstalledDevice *pDevice, int *retiMinFireArc = NULL, int *retiMaxFireArc = NULL);
		bool IsOmniDirectional (CInstalledDevice *pDevice);
		inline bool IsTemperatureEnabled (void) { return (m_Counter == cntTemperature); }
		bool VariantIsValid (CSpaceObject *pSource, CInstalledDevice *pDevice, CWeaponFireDesc &ShotData);

		int GetAlternatingPos (CInstalledDevice *pDevice);
		int GetContinuousFire (CInstalledDevice *pDevice);
		int GetCurrentVariant (CInstalledDevice *pDevice);
		void SetAlternatingPos (CInstalledDevice *pDevice, int iAlternatingPos);
		void SetContinuousFire (CInstalledDevice *pDevice, int iContinuous);
		void SetCurrentVariant (CInstalledDevice *pDevice, int iVariant);

		int m_iFireRate;						//	Ticks between shots
		int m_iFireRateSecs;					//	Game seconds between shots
		int m_iPowerUse;						//	Power use to recharge capacitors (1/10 megawatt)
		int m_iIdlePowerUse;					//	Power use when capacitors fully charged
		int m_iRecoil;							//	0-7 (as per momentum damage)
		int m_iFailureChance;					//	Chance of failure

		bool m_bLauncher;						//	Generic missile launcher
		bool m_bOmnidirectional;				//	Omnidirectional
		bool m_bCharges;						//	TRUE if weapon has charges
		bool m_bMIRV;							//	Each shot seeks an independent target
		int m_iMinFireArc;						//	Min angle of fire arc (degrees)
		int m_iMaxFireArc;						//	Max angle of fire arc (degrees)

		int m_iShotVariants;					//	Number of shot variations
		CWeaponFireDesc *m_pShotData;			//	Desc for each shot variation

		ConfigurationTypes m_Configuration;		//	Shot configuration;
		int m_iConfigCount;						//	Number of shots for custom configurations
		SConfigDesc *m_pConfig;					//	Custom configuration (may be NULL)
		int m_iConfigAimTolerance;				//	Aim tolerance
		bool m_bConfigAlternating;				//	Fire each shot in turn

		CounterTypes m_Counter;					//	Counter type
		int m_iCounterUpdateRate;				//	Ticks to update counter
		int m_iCounterUpdate;					//	Inc/dec value per update
		int m_iCounterActivate;					//	Inc/dec value per shot

		ICCItem *m_pOnFireWeapon;				//	Code to execute weapon fire (may be NULL)

	friend CObjectClass<CWeaponClass>;
	};

#endif

