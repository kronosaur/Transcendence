//	TSESpaceObjectsImpl.h
//
//	Transcendence CSpaceObject classes

#ifndef INCL_TSE_SPACE_OBJECTS
#define INCL_TSE_SPACE_OBJECTS

class CAreaDamage : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
				CWeaponFireDesc *pDesc,
				CItemEnhancementStack *pEnhancements,
				DestructionTypes iCause,
				const CDamageSource &Source,
				const CVector &vPos,
				const CVector &vVel,
				CAreaDamage **retpObj);
		~CAreaDamage (void);

		//	CSpaceObject virtuals
		virtual bool CanMove (void) { return true; }
		virtual CString GetDamageCauseNounPhrase (DWORD dwFlags) { return m_Source.GetDamageCauseNounPhrase(dwFlags); }
		virtual DestructionTypes GetDamageCauseType (void) { return m_iCause; }
		virtual CString GetName (DWORD *retdwFlags = NULL);
		virtual CString GetObjClassName (void) { return CONSTLIT("CAreaDamage"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerEffects; }
		virtual CSpaceObject *GetSecondarySource (void) { return m_Source.GetSecondaryObj(); }
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual CSpaceObject *GetSource (void) { return m_Source.GetObj(); }
		virtual CWeaponFireDesc *GetWeaponFireDesc (void) { return m_pDesc; }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual void OnSystemLoaded (void);
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);
		virtual bool PointInObject (const CVector &vObjPos, const CVector &vPointPos);

	protected:
		//	Virtuals to be overridden
		virtual bool CanHit (CSpaceObject *pObj) { return MissileCanHitObj(pObj, m_Source.GetObj(), m_pDesc); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx) { return damagePassthrough; }
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CAreaDamage (void);

		CWeaponFireDesc *m_pDesc;				//	Weapon descriptor
		CItemEnhancementStack *m_pEnhancements;	//	Stack of enhancements
		DestructionTypes m_iCause;				//	Cause of damage
		IEffectPainter *m_pPainter;				//	Effect painter
		int m_iInitialDelay;					//	Delay before start
		int m_iTick;							//	Counter
		int m_iLifeLeft;						//	Ticks left
		CDamageSource m_Source;					//	Object that fired the beam
		CSovereign *m_pSovereign;				//	Sovereign

	friend CObjectClass<CAreaDamage>;
	};

class CBeam : public CSpaceObject
	{
	public:
		CBeam (void);

		//	CSpaceObject virtuals
		virtual bool CanMove (void) { return true; }
		virtual void CreateReflection (const CVector &vPos, int iDirection);
		virtual Categories GetCategory (void) const { return catBeam; }
		virtual CString GetDamageCauseNounPhrase (DWORD dwFlags) { return m_Source.GetDamageCauseNounPhrase(dwFlags); }
		virtual DestructionTypes GetDamageCauseType (void) { return m_iCause; }
		virtual int GetInteraction (void) { return 0; }
		virtual CString GetName (DWORD *retdwFlags = NULL);
		virtual CString GetObjClassName (void) { return CONSTLIT("CBeam"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerStations; }
		virtual CSpaceObject *GetSecondarySource (void) { return m_Source.GetSecondaryObj(); }
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual CSpaceObject *GetSource (void) { return m_Source.GetObj(); }
		virtual CWeaponFireDesc *GetWeaponFireDesc (void) { return m_pDesc; }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);

	protected:
		//	Virtuals to be overridden
		virtual bool CanHit (CSpaceObject *pObj) { return MissileCanHitObj(pObj, m_Source.GetObj(), m_pDesc); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx) { return damagePassthrough; }
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		void ComputeOffsets (void);

		CWeaponFireDesc *m_pDesc;				//	Weapon descriptor
		int m_iBonus;							//	Bonus damage
		DestructionTypes m_iCause;				//	Cause of damage
		int m_iRotation;						//	Direction
		CVector m_vPaintTo;						//	Paint from old position to this location
		int m_iTick;							//	Ticks that we have been alive
		int m_iLifeLeft;						//	Ticks left
		CDamageSource m_Source;					//	Object that fired the beam
		CSovereign *m_pSovereign;				//	Sovereign
		CSpaceObject *m_pHit;					//	Object that was hit
		int m_iHitDir;							//	Direction from which object was hit

		int m_xFromOffset;						//	Offsets for painting (volatile)
		int m_yFromOffset;
		int m_xToOffset;
		int m_yToOffset;

		DWORD m_fReflection:1;					//	TRUE if this is a reflection
		DWORD m_dwSpareFlags:31;				//	Flags

	friend CObjectClass<CBeam>;
	};

class CBoundaryMarker : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
							   const CVector &vStartPos,
							   CBoundaryMarker **retpMarker);

		void AddSegment (const CVector &vEndpoint);
		void CloseBoundary (void);
		bool FindIntersectSegment (const CVector &vStart, const CVector &vEnd, CVector *retvSegInt, CVector *retvSegEnd, int *retiSeg);
		void GetSegment (int iSeg, CVector *retvStart, CVector *retvEnd);

		//	CSpaceObject virtuals
		virtual CBoundaryMarker *AsBoundaryMarker (void) { return this; }

	protected:
		//	CSpaceObject virtuals
		virtual bool CanHit (CSpaceObject *pObj) { return false; }
		virtual CString GetObjClassName (void) { return CONSTLIT("CBoundaryMarker"); }
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans) { }

	private:
		CBoundaryMarker (void);

		TArray<CVector> m_Path;

	friend CObjectClass<CBoundaryMarker>;
	};

class CDisintegrationEffect : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
				const CVector &vPos,
				const CVector &vVel,
				const CObjectImageArray &MaskImage,
				int iMaskTick,
				int iMaskRotation,
				CDisintegrationEffect **retpEffect);
		virtual ~CDisintegrationEffect (void);

		//	CSpaceObject virtuals
		virtual bool CanMove (void) { return true; }
		virtual CString GetObjClassName (void) { return CONSTLIT("CDisintegrationEffect"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerEffects; }

	protected:

		//	CSpaceObject virtuals
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx) { return damagePassthrough; }
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick);
		virtual void OnUpdateExtended (const CTimeSpan &ExtraTime);
		virtual void OnWriteToStream (IWriteStream *pStream);
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);

	private:
		struct SParticle
			{
			int x;							//	Position in 256th of a pixel
			int y;
			int xV;							//	Velocity in 256th of a pixel
			int yV;							//	per tick
			int iTicksLeft;
			WORD wColor;
			WORD wSpare;
			};

		CDisintegrationEffect (void);
		void InitParticle (SParticle *pParticle);

		int m_iTick;
		CObjectImageArray m_MaskImage;
		int m_iMaskTick;
		int m_iMaskRotation;

		//	particles
		int m_iParticleCount;
		SParticle *m_pParticles;

	friend CObjectClass<CDisintegrationEffect>;
	};

class CEffect : public CSpaceObject
	{
	public:
		static ALERROR Create (CEffectCreator *pType,
				CSystem *pSystem,
				CSpaceObject *pAnchor,
				const CVector &vPos,
				const CVector &vVel,
				int iRotation,
				CEffect **retpEffect);
		virtual ~CEffect (void);

		//	CSpaceObject virtuals
		virtual bool CanMove (void) { return true; }
		virtual CString GetObjClassName (void) { return CONSTLIT("CEffect"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerEffects; }

	protected:

		//	CSpaceObject virtuals
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx) { return damagePassthrough; }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick);
		virtual void OnUpdateExtended (const CTimeSpan &ExtraTime);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CEffect (void);

		CEffectCreator *GetEffectCreator (void);

		IEffectPainter *m_pPainter;
		CSpaceObject *m_pAnchor;
		int m_iLifetime;
		int m_iRotation;
		int m_iTick;

	friend CObjectClass<CEffect>;
	};

class CFractureEffect : public CSpaceObject
	{
	public:
		enum Style
			{
			styleExplosion,
			};

		static ALERROR Create (CSystem *pSystem,
				const CVector &vPos,
				const CVector &vVel,
				const CObjectImageArray &Image,
				int iImageTick,
				int iImageRotation,
				int iStyle,
				CFractureEffect **retpEffect);
		virtual ~CFractureEffect (void);

		//	CSpaceObject virtuals
		virtual bool CanMove (void) { return true; }
		virtual Categories GetCategory (void) const { return catFractureEffect; }
		virtual CString GetObjClassName (void) { return CONSTLIT("CFractureEffect"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerEffects; }
		virtual void SetAttractor (CSpaceObject *pObj);

	protected:

		//	CSpaceObject virtuals
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx) { return damagePassthrough; }
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick);
		virtual void OnUpdateExtended (const CTimeSpan &ExtraTime);
		virtual void OnWriteToStream (IWriteStream *pStream);
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);

	private:
		struct SParticle
			{
			int x;							//	Position in 256th of a pixel
			int y;
			int xV;							//	Velocity in 256th of a pixel
			int yV;							//	per tick
			int iTicks;

			int xSrc;						//	Position in source
			int ySrc;
			int iShape;						//	Index of shape (-1 = destroyed)
			};

		CFractureEffect (void);
		void InitParticleArray (void);

		int m_iTick;
		int m_iLifeTime;
		CObjectImageArray m_Image;
		int m_iImageTick;
		int m_iImageRotation;
		int m_iStyle;
		int m_iCellSize;

		int m_iParticleCount;
		SParticle *m_pParticles;

		CSpaceObject *m_pAttractor;

	friend CObjectClass<CFractureEffect>;
	};

class CMarker : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
							   CSovereign *pSovereign,
							   const CVector &vPos,
							   const CVector &vVel,
							   const CString &sName,
							   CMarker **retpMarker);

		//	CSpaceObject virtuals
		virtual CString GetName (DWORD *retdwFlags = NULL) { if (retdwFlags) *retdwFlags = 0; return m_sName; }
		virtual bool IsMarker (void) { return true; }
		virtual void OnObjLeaveGate (CSpaceObject *pObj);

	protected:
		virtual bool CanHit (CSpaceObject *pObj) { return false; }
		virtual CSovereign *GetSovereign (void) const;
		virtual CString GetObjClassName (void) { return CONSTLIT("CMarker"); }
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans) { }

	private:
		CMarker (void);

		CString m_sName;						//	Name
		CSovereign *m_pSovereign;				//	Sovereign

	friend CObjectClass<CMarker>;
	};

class CMissile : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
				CWeaponFireDesc *pDesc,
				CItemEnhancementStack *pEnhancements,
				DestructionTypes iCause,
				const CDamageSource &Source,
				const CVector &vPos,
				const CVector &vVel,
				int iRotation,
				CSpaceObject *pTarget,
				CMissile **retpMissile);
		~CMissile (void);

		//	CSpaceObject virtuals
		virtual CMissile *AsMissile (void) { return this; }
		virtual bool CanMove (void) { return true; }
		virtual void CreateReflection (const CVector &vPos, int iDirection);
		virtual CString DebugCrashInfo (void);
		virtual void DetonateNow (CSpaceObject *pHit);
		virtual Categories GetCategory (void) const;
		virtual CString GetDamageCauseNounPhrase (DWORD dwFlags) { return m_Source.GetDamageCauseNounPhrase(dwFlags); }
		virtual DestructionTypes GetDamageCauseType (void) { return m_iCause; }
		virtual int GetInteraction (void) { return m_pDesc->GetInteraction(); }
		virtual int GetLevel (void) const { CItemType *pType = m_pDesc->GetWeaponType(); return (pType ? pType->GetLevel() : 1); }
		virtual CString GetName (DWORD *retdwFlags = NULL);
		virtual CString GetObjClassName (void) { return CONSTLIT("CMissile"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return (m_pDesc->GetPassthrough() > 0 ? CSystem::layerEffects : CSystem::layerStations); }
		virtual ICCItem *GetProperty (const CString &sName);
		virtual int GetRotation (void) const { return m_iRotation; }
		virtual CSpaceObject *GetSecondarySource (void) { return m_Source.GetSecondaryObj(); }
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual CSpaceObject *GetSource (void) { return m_Source.GetObj(); }
		virtual int GetStealth (void) const;
		virtual CDesignType *GetType (void) const { return m_pDesc->GetWeaponType(); }
		virtual CWeaponFireDesc *GetWeaponFireDesc (void) { return m_pDesc; }
		virtual bool HasAttribute (const CString &sAttribute) const;
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);
		virtual bool PointInObject (const CVector &vObjPos, const CVector &vPointPos);
		virtual bool SetProperty (const CString &sName, ICCItem *pValue, CString *retsError);

	protected:

		//	Virtuals to be overridden
		virtual bool CanHit (CSpaceObject *pObj) { return MissileCanHitObj(pObj, m_Source.GetObj(), m_pDesc); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx);
		virtual void OnDestroyed (SDestroyCtx &Ctx);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		struct SExhaustParticle
			{
			CVector vPos;				//	Position of particle
			CVector vVel;				//	Velocity of particle
			};

		CMissile (void);

		int ComputeVaporTrail (void);
		void CreateFragments (const CVector &vPos);
		bool SetMissileFade (void);

		CWeaponFireDesc *m_pDesc;				//	Weapon descriptor
		CItemEnhancementStack *m_pEnhancements;	//	Stack of enhancements
		DestructionTypes m_iCause;				//	Cause of damage
		int m_iLifeLeft;						//	Ticks left
		int m_iHitPoints;						//	HP left
		IEffectPainter *m_pPainter;				//	Effect painter
		CDamageSource m_Source;					//	Object that fired missile
		CSovereign *m_pSovereign;				//	Sovereign
		CSpaceObject *m_pHit;					//	Object hit
		CVector m_vHitPos;						//	Position hit
		int m_iHitDir;							//	Direction hit
		int m_iRotation;						//	Current rotation (degrees)
		CSpaceObject *m_pTarget;				//	Target
		int m_iTick;							//	Number of ticks of life so far
		TQueue<SExhaustParticle> *m_pExhaust;	//	Array of exhaust particles
		CG16bitBinaryRegion *m_pVaporTrailRegions;	//	Array of vapor trail regions
		int m_iSavedRotationsCount;				//	Number of saved rotations
		int *m_pSavedRotations;					//	Array of saved rotation angles

		DWORD m_fDestroyed:1;					//	TRUE if destroyed (only around to paint effect)
		DWORD m_fReflection:1;					//	TRUE if this is a reflection
		DWORD m_fDetonate:1;					//	TRUE if we should detonate on next update
		DWORD m_fPassthrough:1;					//	TRUE if shot passed through
		DWORD m_fPainterFade:1;					//	TRUE if we need to paint a fading painter
		DWORD m_dwSpareFlags:27;				//	Flags

	friend CObjectClass<CMissile>;
	};

class CParticleDamage : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
				CWeaponFireDesc *pDesc,
				CItemEnhancementStack *pEnhancements,
				DestructionTypes iCause,
				const CDamageSource &Source,
				const CVector &vPos,
				const CVector &vVel,
				int iDirection,
				CSpaceObject *pTarget,
				CParticleDamage **retpObj);
		~CParticleDamage (void);

		//	CSpaceObject virtuals
		virtual bool CanMove (void) { return true; }
		virtual CString GetDamageCauseNounPhrase (DWORD dwFlags) { return m_Source.GetDamageCauseNounPhrase(dwFlags); }
		virtual DestructionTypes GetDamageCauseType (void) { return m_iCause; }
		virtual CString GetName (DWORD *retdwFlags = NULL);
		virtual CString GetObjClassName (void) { return CONSTLIT("CParticleDamage"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerEffects; }
		virtual CSpaceObject *GetSecondarySource (void) { return m_Source.GetSecondaryObj(); }
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual CSpaceObject *GetSource (void) { return m_Source.GetObj(); }
		virtual CWeaponFireDesc *GetWeaponFireDesc (void) { return m_pDesc; }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual bool PointInObject (const CVector &vObjPos, const CVector &vPointPos);

	protected:
		//	Virtuals to be overridden
		virtual bool CanHit (CSpaceObject *pObj) { return MissileCanHitObj(pObj, m_Source.GetObj(), m_pDesc); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx) { return damagePassthrough; }
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CParticleDamage (void);

		void InitParticles (int iCount, const CVector &vPos, const CVector &vInitVel, int iDirection);

		CWeaponFireDesc *m_pDesc;				//	Weapon descriptor
		CItemEnhancementStack *m_pEnhancements;	//	Stack of enhancements
		CSpaceObject *m_pTarget;				//	Target
		DestructionTypes m_iCause;				//	Cause of damage
		int m_iTick;							//	Counter
		int m_iLifeLeft;						//	Ticks left
		int m_iEmitTime;						//	Ticks to emit new particles
		CDamageSource m_Source;					//	Object that fired the beam
		CSovereign *m_pSovereign;				//	Sovereign
		int m_iDamage;							//	Damage in points
		int m_iParticleCount;					//	Number of particles generated
		CVector m_vEmitSourcePos;				//	Emit source position
		CVector m_vEmitSourceVel;				//	Emit source velocity
		int m_iEmitDirection;					//	Emit direction

		IEffectPainter *m_pPainter;				//	Painter to use for each particle
		CParticleArray m_Particles;

	friend CObjectClass<CParticleDamage>;
	friend struct SParticle;
	};

class CParticleEffect : public CSpaceObject
	{
	public:
		enum ParticlePainting
			{
			paintDot,
			paintImage,
			paintSmoke,
			paintFlame,
			};

		struct SParticleType
			{
			SParticleType (void) :
					iPaintStyle(paintDot),
					wColor(0xffff),
					iRegenerationTimer(0),
					iLifespan(0),
					rAveSpeed(0.0),
					iDirection(-1),
					iDirRange(0),
					rRadius(0.0),
					rHoleRadius(0.0),
					rDampening(1.0f),
					pDamageDesc(NULL),
					m_fMaxRadius(false),
					m_fLifespan(false),
					m_fWake(false),
					m_fRegenerate(false),
					m_fDrag(false),
					m_fFreeDesc(false)
				{ }

			~SParticleType (void)
				{
				if (m_fFreeDesc && pDamageDesc)
					delete pDamageDesc;
				}

			int iPaintStyle;			//	Painting style
			CObjectImageArray Image;	//	Image for each particle
			WORD wColor;				//	Color of particle

			int iRegenerationTimer;		//	Max lifespan of entire group
			int iLifespan;				//	Max lifespan (in ticks)

			Metric rAveSpeed;			//	Average speed of particles
			int iDirection;				//	Direction (-1 == all directions)
			int iDirRange;				//	Span to either side (in degrees)

			Metric rRadius;				//	Radius of particle field
			Metric rHoleRadius;			//	Keep particles out of the center
			Metric rDampening;			//	Velocity dampening constant

			CWeaponFireDesc *pDamageDesc;	//	Damage caused to objects in field

			DWORD m_fMaxRadius:1;		//	TRUE if we should keep particles in radius
			DWORD m_fLifespan:1;		//	TRUE if particles have a lifespan
			DWORD m_fWake:1;			//	TRUE if particles are affected by passing objects
			DWORD m_fRegenerate:1;		//	TRUE if particles regenerate when they die
			DWORD m_fDrag:1;			//	TRUE if particles are subject to drag
			DWORD m_fFreeDesc:1;		//	TRUE if we own the pDesc structure
			};

		static ALERROR Create (CSystem *pSystem,
							   CXMLElement *pDesc,
							   const CVector &vPos,
							   const CVector &vVel,
							   CParticleEffect **retpEffect);
		static ALERROR CreateEmpty (CSystem *pSystem,
									const CVector &vPos,
									const CVector &vVel,
									CParticleEffect **retpEffect);
		static ALERROR CreateExplosion (CSystem *pSystem,
										CSpaceObject *pAnchor,
										const CVector &vPos,
										const CVector &vVel,
										int iCount,
										Metric rAveSpeed,
										int iTotalLifespan,
										int iParticleLifespan,
										const CObjectImageArray &Image,
										CParticleEffect **retpEffect);
		static ALERROR CreateGeyser (CSystem *pSystem,
									 CSpaceObject *pAnchor,
									 const CVector &vPos,
									 const CVector &vVel,
									 int iTotalLifespan,
									 int iParticleCount,
									 ParticlePainting iParticleStyle,
									 int iParticleLifespan,
									 Metric rAveSpeed,
									 int iDirection,
									 int iAngleWidth,
									 CParticleEffect **retpEffect);

		~CParticleEffect (void);

		void AddGroup (SParticleType *pType, int iCount);

		//	CSpaceObject virtuals
		virtual bool CanBeHitBy (const DamageDesc &Damage);
		virtual CString GetName (DWORD *retdwFlags = NULL) { if (retdwFlags) *retdwFlags = 0; return m_sName; }
		virtual CString GetObjClassName (void) { return CONSTLIT("CParticleEffect"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerSpace; }
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);

	protected:

		//	Virtuals to be overridden
		virtual bool CanHit (CSpaceObject *pObj) { return false; }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual bool PointInObject (const CVector &vObjPos, const CVector &vPointPos);

	private:

		enum Constants
			{
			ctMaxParticleGroups =	8,
			ctMaxObjsInField =		16,
			};

		struct SParticle
			{
			inline bool IsValid (void) { return iLifeLeft != -1; }

			int iDestiny;
			int iLifeLeft;
			CVector vPos;
			CVector vVel;
			};

		struct SParticleArray
			{
			~SParticleArray (void)
				{
				if (pType)
					delete pType;

				if (pParticles)
					delete [] pParticles;
				}

			SParticleType *pType;
			int iAlive;
			int iCount;
			SParticle *pParticles;

			SParticleArray *pNext;
			};

		CParticleEffect (void);

		void CreateGroup (SParticleType *pType, int iCount, SParticleArray **retpGroup);
		void PaintFlameParticles (SParticleArray *pGroup, CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		void PaintSmokeParticles (SParticleArray *pGroup, CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		void SetParticleSpeed (SParticleType *pType, SParticle *pParticle);

		CString m_sName;
		SParticleArray *m_pFirstGroup;
		CSpaceObject *m_pAnchor;

	friend CObjectClass<CParticleEffect>;
	};

class CPOVMarker : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
							   const CVector &vPos,
							   const CVector &vVel,
							   CPOVMarker **retpMarker);

	protected:
		virtual bool CanHit (CSpaceObject *pObj) { return false; }
		virtual CSovereign *GetSovereign (void) const;
		virtual void OnLosePOV (void);
		virtual CString GetObjClassName (void) { return CONSTLIT("CPOVMarker"); }
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans) { }

	private:
		CPOVMarker (void);

	friend CObjectClass<CPOVMarker>;
	};

class CRadiusDamage : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
				CWeaponFireDesc *pDesc,
				CItemEnhancementStack *pEnhancements,
				DestructionTypes iCause,
				const CDamageSource &Source,
				const CVector &vPos,
				const CVector &vVel,
				CSpaceObject *pTarget,
				CRadiusDamage **retpObj);
		~CRadiusDamage (void);

		//	CSpaceObject virtuals
		virtual bool CanMove (void) { return true; }
		virtual CString DebugCrashInfo (void);
		virtual CString GetDamageCauseNounPhrase (DWORD dwFlags) { return m_Source.GetDamageCauseNounPhrase(dwFlags); }
		virtual DestructionTypes GetDamageCauseType (void) { return m_iCause; }
		virtual CString GetName (DWORD *retdwFlags = NULL);
		virtual CString GetObjClassName (void) { return CONSTLIT("CRadiusDamage"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerEffects; }
		virtual CSpaceObject *GetSecondarySource (void) { return m_Source.GetSecondaryObj(); }
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual CSpaceObject *GetSource (void) { return m_Source.GetObj(); }
		virtual CWeaponFireDesc *GetWeaponFireDesc (void) { return m_pDesc; }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual void OnSystemLoaded (void);
		virtual bool PointInObject (const CVector &vObjPos, const CVector &vPointPos);

	protected:
		//	Virtuals to be overridden
		virtual bool CanHit (CSpaceObject *pObj) { return MissileCanHitObj(pObj, m_Source.GetObj(), m_pDesc); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx) { return damagePassthrough; }
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CRadiusDamage (void);

		CWeaponFireDesc *m_pDesc;				//	Weapon descriptor
		CItemEnhancementStack *m_pEnhancements;	//	Stack of enhancements
		DestructionTypes m_iCause;				//	Cause of damage
		IEffectPainter *m_pPainter;				//	Effect painter
		int m_iTick;							//	Counter
		int m_iLifeLeft;						//	Ticks left
		CDamageSource m_Source;					//	Object that fired the shot
		CSpaceObject *m_pTarget;				//	Target
		CSovereign *m_pSovereign;				//	Sovereign

	friend CObjectClass<CRadiusDamage>;
	};

class CSequencerEffect : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
							   CEffectSequencerCreator *pType,
							   CSpaceObject *pAnchor,
							   const CVector &vPos,
							   const CVector &vVel,
							   CSequencerEffect **retpEffect);

	protected:
		//	CSpaceObject virtuals
		virtual bool CanHit (CSpaceObject *pObj) { return false; }
		virtual CString GetObjClassName (void) { return CONSTLIT("CSequencerEffect"); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);
		virtual void OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick);
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans) { }

	private:
		CSequencerEffect (void);

		CEffectSequencerCreator *m_pType;
		CSpaceObject *m_pAnchor;
		CVector m_vAnchorOffset;
		int m_iStartTime;
		int m_iTimeCursor;

	friend CObjectClass<CSequencerEffect>;
	};

class CShip : public CSpaceObject
	{
	public:
		enum RemoveDeviceStatus
			{
			remOK					= 0,	//	Can remove this device
			remTooMuchCargo			= 1,	//	Can't remove cargo expansion (because too much cargo)
			remNotInstalled			= 2,	//	Device is not installed
			remCannotRemove			= 3,	//	Custom reason
			};

		static ALERROR CreateFromClass (CSystem *pSystem, 
										CShipClass *pClass, 
										IShipController *pController,
										CDesignType *pOverride,
										CSovereign *pSovereign,
										const CVector &vPos, 
										const CVector &vVel, 
										int iRotation, 
										SShipGeneratorCtx *pCtx,
										CShip **retpShip);
		virtual ~CShip (void);

		//	Orders
		inline void CancelCurrentOrder (void) { m_pController->CancelCurrentOrder(); }
		inline IShipController::OrderTypes GetCurrentOrder (CSpaceObject **retpTarget, IShipController::SData *retData = NULL) { return m_pController->GetCurrentOrderEx(retpTarget, retData); }
		inline DWORD GetCurrentOrderData (void) { return m_pController->GetCurrentOrderData(); }

		//	Armor methods
		inline CInstalledArmor *GetArmorSection (int iSect) { return &m_Armor[iSect]; }
		inline int GetArmorSectionCount (void) { return m_Armor.GetCount(); }
		int DamageArmor (int iSect, DamageDesc &Damage);
		void InstallItemAsArmor (CItemListManipulator &ItemList, int iSect);
		bool IsArmorDamaged (int iSect);
		bool IsArmorRepairable (int iSect);
		void RepairAllArmor (void);
		void RepairArmor (int iSect, int iHitPoints, int *retiHPRepaired = NULL);
		void SetCursorAtArmor (CItemListManipulator &ItemList, int iSect);
		void UninstallArmor (CItemListManipulator &ItemList);

		//	Device methods
		RemoveDeviceStatus CanRemoveDevice (const CItem &Item, CString *retsResult);
		void ClearAllTriggered (void);
		void DamageCargo (SDamageCtx &Ctx);
		void DamageDevice (CInstalledDevice *pDevice, SDamageCtx &Ctx);
		void DamageDrive (SDamageCtx &Ctx);
		void EnableDevice (int iDev, bool bEnable = true, bool bSilent = false);
		bool FindDeviceAtPos (const CVector &vPos, CInstalledDevice **retpDevice);
		DeviceNames GetDeviceNameForCategory (ItemCategories iCategory);
		int GetItemDeviceName (const CItem &Item) const;
		CItem GetNamedDeviceItem (DeviceNames iDev);
		bool HasNamedDevice (DeviceNames iDev) const;
		void InstallItemAsDevice (CItemListManipulator &ItemList, int iDeviceSlot = -1);
		bool IsDeviceSlotAvailable (ItemCategories iItemCat = itemcatNone, int *retiSlot = NULL);
		void ReadyFirstWeapon (void);
		void ReadyNextWeapon (int iDir = 1);
		void ReadyFirstMissile (void);
		void ReadyNextMissile (int iDir = 1);
		void RechargeItem (CItemListManipulator &ItemList, int iCharges);
		int GetMissileCount (void);
		ALERROR RemoveItemAsDevice (CItemListManipulator &ItemList);
		DeviceNames SelectWeapon (int iDev, int iVariant);
		void SetCursorAtDevice (CItemListManipulator &ItemList, int iDev);
		void SetCursorAtNamedDevice (CItemListManipulator &ItemList, DeviceNames iDev);
		void SetWeaponTriggered (DeviceNames iDev, bool bTriggered = true);
		void SetWeaponTriggered (CInstalledDevice *pWeapon, bool bTriggered = true);
		CDeviceClass *GetNamedDeviceClass (DeviceNames iDev);
		CString GetReactorName (void);
		inline int GetWeaponAimTolerance (DeviceNames iDev) { return 3; }
		bool GetWeaponIsReady (DeviceNames iDev);
		Metric GetWeaponRange (DeviceNames iDev) { return LIGHT_SPEED * 60.0; }
		Metric GetWeaponSpeed (DeviceNames iDev) { return LIGHT_SPEED; }
		bool IsWeaponAligned (DeviceNames iDev, CSpaceObject *pTarget, int *retiAimAngle = NULL, int *retiFireAngle = NULL, int *retiFacingAngle = NULL);

		//	Settings
		inline bool HasAutopilot (void) { return m_fHasAutopilot; }

		void ClearBlindness (bool bNoMessage = false);

		void ClearDisarmed (void);
		void MakeDisarmed (int iTickCount = -1);

		void ClearLRSBlindness (void);
		inline bool IsLRSBlind (void) { return m_iLRSBlindnessTimer != 0; }
		void MakeLRSBlind (int iTickCount = -1);

		void ClearParalyzed (void);

		bool IsRadiationImmune (void);

		inline bool IsInGate (void) const { return m_iExitGateTimer > 0; }
		void SetInGate (CSpaceObject *pGate, int iTickCount);

		inline bool HasTargetingComputer (void) { return m_fHasTargetingComputer; }

		inline void ClearSRSEnhanced (void) { SetAbility(ablExtendedScanner, ablRemove, -1, 0); }
		inline bool IsSRSEnhanced (void) { return (m_fSRSEnhanced ? true : false); }
		inline void SetSRSEnhanced (void) { SetAbility(ablExtendedScanner, ablInstall, -1, 0); }

		//	Reactor methods
		inline int GetFuelLeft (void) { return (m_fOutOfFuel ? 0 : m_iFuelLeft); }
		int GetMaxFuel (void);
		inline const ReactorDesc *GetReactorDesc (void) { return m_pReactorDesc; }
		inline void TrackFuel (bool bTrack = true) { m_fTrackFuel = bTrack; }
		inline void TrackMass (bool bTrack = true) { m_fTrackMass = bTrack; }
		int GetPowerConsumption (void);
		bool IsFuelCompatible (const CItem &Item);

		//	Drive methods
		inline int Angle2Direction (int iAngle) const { return m_pClass->Angle2Direction(iAngle); }
		inline int AlignToRotationAngle (int iAngle) const { return m_pClass->AlignToRotationAngle(iAngle); }
		inline int GetManeuverability (void) { return m_pClass->GetManeuverability(); }
		inline int GetRotationAngle (void) { return m_pClass->GetRotationAngle(); }
		inline int GetRotationRange (void) { return m_pClass->GetRotationRange(); }
		inline EManeuverTypes GetManeuverToFace (int iAngle) const { return m_Rotation.GetManeuverToFace(m_pClass->GetRotationDesc(), iAngle); }
		inline Metric GetThrust (void) { return (IsMainDriveDamaged() ? (m_iThrust / 2) : m_iThrust); }
		inline bool IsInertialess (void) { return (m_pDriveDesc->fInertialess ? true : false); }
		inline bool IsMainDriveDamaged (void) const { return m_iDriveDamagedTimer != 0; }
		bool IsPointingTo (int iAngle);
		inline void SetMaxSpeedHalf (void) { m_fHalfSpeed = true; }
		inline void ResetMaxSpeed (void) { m_fHalfSpeed = false; }

		//	Miscellaneous
		inline IShipController *GetController (void) { return m_pController; }
		inline CShipClass *GetClass (void) { return m_pClass; }
		void SetController (IShipController *pController, bool bFreeOldController = true);
		inline void SetControllerEnabled (bool bEnabled = true) { m_fControllerDisabled = !bEnabled; }
		void SetCommandCode (ICCItem *pCode);
		inline void SetDestroyInGate (void) { m_fDestroyInGate = true; }
		inline void SetEncounterInfo (CStationType *pEncounterInfo) { m_pEncounterInfo = pEncounterInfo; }
		inline void SetPlayerWingman (bool bIsWingman) const { m_pController->SetPlayerWingman(bIsWingman); }
		inline void SetRotation (int iAngle) { m_Rotation.SetRotationAngle(m_pClass->GetRotationDesc(), iAngle); }
		void Undock (void);

		//	CSpaceObject virtuals
		virtual bool AbsorbWeaponFire (CInstalledDevice *pWeapon);
		virtual void AddOverlay (COverlayType *pType, int iPosAngle, int iPosRadius, int iRotation, int iLifetime, DWORD *retdwID = NULL);
		virtual CTradingDesc *AllocTradeDescOverride (void);
		virtual CShip *AsShip (void) { return this; }
		virtual void Behavior (SUpdateCtx &Ctx);
		virtual bool CanAttack (void) const;
		virtual bool CanInstallItem (const CItem &Item, int iSlot = -1, InstallItemResults *retiResult = NULL, CString *retsResult = NULL, CItem *retItemToReplace = NULL);
		virtual bool CanMove (void) { return true; }
		virtual CurrencyValue ChargeMoney (DWORD dwEconomyUNID, CurrencyValue iValue);
		virtual bool ClassCanAttack (void) { return true; }
		virtual void ConsumeFuel (int iFuel);
		virtual CurrencyValue CreditMoney (DWORD dwEconomyUNID, CurrencyValue iValue);
		virtual void DamageExternalDevice (int iDev, SDamageCtx &Ctx);
		virtual void DeactivateShields (void);
		virtual CString DebugCrashInfo (void);
		virtual void Decontaminate (void);
		virtual void DepleteShields (void);
		virtual void DisableDevice (CInstalledDevice *pDevice);
		virtual CInstalledArmor *FindArmor (const CItem &Item);
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual CInstalledDevice *FindDevice (const CItem &Item);
		virtual bool FindDeviceSlotDesc (const CItem &Item, SDeviceDesc *retDesc) { return m_pClass->FindDeviceSlotDesc(Item, retDesc); }
		virtual bool FollowsObjThroughGate (CSpaceObject *pLeader);
		virtual AbilityStatus GetAbility (Abilities iAbility);
		virtual CurrencyValue GetBalance (DWORD dwEconomyUNID);
		virtual CSpaceObject *GetBase (void) const;
		virtual Metric GetCargoSpaceLeft (void);
		virtual Categories GetCategory (void) const { return catShip; }
		virtual DWORD GetClassUNID (void) { return m_pClass->GetUNID(); }
		virtual int GetCombatPower (void);
		virtual int GetCyberDefenseLevel (void) { return m_pClass->GetCyberDefenseLevel(); }
		virtual int GetDamageEffectiveness (CSpaceObject *pAttacker, CInstalledDevice *pWeapon);
		virtual DamageTypes GetDamageType (void);
		virtual DWORD GetDefaultBkgnd (void) { return m_pClass->GetDefaultBkgnd(); }
		virtual CSpaceObject *GetDestination (void) const { return m_pController->GetDestination(); }
		virtual CSpaceObject *GetDockedObj (void) { return m_pDocked; }
		virtual int GetDockingPortCount (void) { return m_DockingPorts.GetPortCount(this); }
		virtual CDesignType *GetDefaultDockScreen (CString *retsName = NULL);
		virtual CInstalledDevice *GetDevice (int iDev) const { return &m_Devices[iDev]; }
		virtual int GetDeviceCount (void) const { return m_iDeviceCount; }
		virtual CVector GetDockingPortOffset (int iRotation) { return m_pClass->GetDockingPortOffset(iRotation); }
		virtual CStationType *GetEncounterInfo (void) { return m_pEncounterInfo; }
		virtual CSpaceObject *GetEscortPrincipal (void) const;
		virtual const CString &GetGlobalData (const CString &sAttribute) { return m_pClass->GetGlobalData(sAttribute); }
		virtual const CObjectImageArray &GetImage (void) const { return m_pClass->GetImage(); }
		virtual CString GetInstallationPhrase (const CItem &Item) const;
		virtual int GetLastFireTime (void) const { return m_iLastFireTime; }
		virtual int GetLastHitTime (void) const { return m_iLastHitTime; }
		virtual int GetLevel (void) const { return m_pClass->GetLevel(); }
		virtual Metric GetMass (void);
		virtual int GetMaxPower (void) const;
		virtual CString GetName (DWORD *retdwFlags = NULL);
		virtual CInstalledDevice *GetNamedDevice (DeviceNames iDev);
		virtual int GetNearestDockPort (CSpaceObject *pRequestingObj, CVector *retvPort = NULL);
		virtual CString GetObjClassName (void) { return CONSTLIT("CShip"); }
		virtual int GetOpenDockingPortCount (void) { return m_DockingPorts.GetPortCount(this) - m_DockingPorts.GetPortsInUseCount(this); }
		virtual CEnergyField *GetOverlay (DWORD dwID) const { return m_EnergyFields.GetOverlay(dwID); }
		virtual const CString &GetOverlayData (DWORD dwID, const CString &sAttrib) { return m_EnergyFields.GetData(dwID, sAttrib); }
		virtual void GetOverlayImpact (CEnergyFieldList::SImpactDesc *retImpact) { m_EnergyFields.GetImpact(this, retImpact); }
		virtual void GetOverlayList (TArray<CEnergyField *> &List) { m_EnergyFields.GetList(List); }
		virtual CVector GetOverlayPos (DWORD dwID) { return m_EnergyFields.GetPos(this, dwID); }
		virtual ICCItem *GetOverlayProperty (CCodeChainCtx *pCCCtx, DWORD dwID, const CString &sName) { return m_EnergyFields.GetProperty(pCCCtx, this, dwID, sName); }
		virtual int GetOverlayRotation (DWORD dwID) { return m_EnergyFields.GetRotation(dwID); }
		virtual COverlayType *GetOverlayType (DWORD dwID) { return m_EnergyFields.GetType(dwID); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerShips; }
		virtual int GetPerception (void);
		virtual ICCItem *GetProperty (const CString &sName);
		virtual int GetRotation (void) const { return m_Rotation.GetRotationAngle(m_pClass->GetRotationDesc()); }
		virtual ScaleTypes GetScale (void) const { return scaleShip; }
		virtual int GetScore (void) { return m_pClass->GetScore(); }
		virtual CXMLElement *GetScreen (const CString &sName) { return m_pClass->GetScreen(sName); }
		virtual int GetShieldLevel (void);
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual int GetStealth (void) const;
		virtual Metric GetMaxSpeed (void) { return ((m_fHalfSpeed || IsMainDriveDamaged()) ? (m_rMaxSpeed / 2.0) : m_rMaxSpeed); }
		virtual CSpaceObject *GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget = false) const;
		virtual CTradingDesc *GetTradeDescOverride (void) { return m_pTrade; }
		virtual CDesignType *GetType (void) const { return m_pClass; }
		virtual int GetVisibleDamage (void);
		virtual bool HasAttribute (const CString &sAttribute) const;
		virtual bool ImageInObject (const CVector &vObjPos, const CObjectImageArray &Image, int iTick, int iRotation, const CVector &vImagePos);
		virtual bool IsAngryAt (CSpaceObject *pObj);
		virtual bool IsBlind (void) { return m_iBlindnessTimer != 0; }
		virtual bool IsDisarmed (void) { return m_fDisarmedByOverlay || m_iDisarmedTimer != 0; }
		virtual bool IsIdentified (void) { return m_fIdentified; }
		virtual bool IsInactive (void) const { return (m_fManualSuspended || m_iExitGateTimer > 0); }
		virtual bool IsKnown (void) { return m_fKnown; }
		virtual bool IsMultiHull (void) { return !m_Interior.IsEmpty(); }
		virtual bool IsObjDocked (CSpaceObject *pObj) { return m_DockingPorts.IsObjDocked(pObj); }
		virtual bool IsObjDockedOrDocking (CSpaceObject *pObj) { return m_DockingPorts.IsObjDockedOrDocking(pObj); }
		virtual bool IsOutOfFuel (void) { return m_fOutOfFuel; }
		virtual bool IsParalyzed (void) { return m_fParalyzedByOverlay || m_iParalysisTimer != 0; }
		virtual bool IsPlayer (void) const;
		virtual bool IsRadioactive (void) { return (m_fRadioactive ? true : false); }
		virtual bool IsSuspended (void) const { return m_fManualSuspended; }
		virtual bool IsTimeStopImmune (void) { return m_pClass->IsTimeStopImmune(); }
		virtual bool IsVirtual (void) const { return m_pClass->IsVirtual(); }
		virtual void MarkImages (void);
		virtual bool ObjectInObject (const CVector &vObj1Pos, CSpaceObject *pObj2, const CVector &vObj2Pos);
		virtual void OnAscended (void);
		virtual void OnBounce (CSpaceObject *pBarrierObj, const CVector &vPos);
		virtual void OnComponentChanged (ObjectComponentTypes iComponent);
		virtual void OnDeviceStatus (CInstalledDevice *pDev, int iEvent);
		virtual bool OnDestroyCheck (DestructionTypes iCause, const CDamageSource &Attacker);
		virtual void OnDocked (CSpaceObject *pObj);
		virtual void OnDockedObjChanged (CSpaceObject *pLocation);
		virtual void OnItemEnhanced (CItemListManipulator &ItemList);
		virtual void OnHitByDeviceDamage (void);
		virtual void OnHitByDeviceDisruptDamage (DWORD dwDuration);
		virtual void OnHitByRadioactiveDamage (SDamageCtx &Ctx);
		virtual void OnMissionCompleted (CMission *pMission, bool bSuccess) { m_pController->OnMissionCompleted(pMission, bSuccess); }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual void OnNewSystem (CSystem *pSystem);
		virtual void OnObjDamaged (SDamageCtx &Ctx) { m_pController->OnObjDamaged(Ctx); }
		virtual void OnPlayerChangedShips (CSpaceObject *pOldShip);
		virtual void OnPlayerObj (CSpaceObject *pPlayer);
		virtual void OnStationDestroyed (const SDestroyCtx &Ctx);
		virtual void OnSystemCreated (void);
		virtual void OnSystemLoaded (void);
		virtual void MakeBlind (int iTickCount = -1);
		virtual void MakeParalyzed (int iTickCount = -1);
		virtual void MakeRadioactive (void);
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);
		virtual bool PointInObject (const CVector &vObjPos, const CVector &vPointPos);
		virtual bool PointInObject (SPointInObjectCtx &Ctx, const CVector &vObjPos, const CVector &vPointPos);
		virtual void PointInObjectInit (SPointInObjectCtx &Ctx);
		virtual void ProgramDamage (CSpaceObject *pHacker, const ProgramDesc &Program);
		virtual void Refuel (int iFuel);
		virtual void Refuel (const CItem &Fuel);
		virtual void RemoveOverlay (DWORD dwID);
		virtual void RepairDamage (int iHitPoints);
		virtual bool RequestDock (CSpaceObject *pObj, int iPort = -1);
		virtual void Resume (void) { m_fManualSuspended = false; if (!IsInGate()) ClearCannotBeHit(); m_pController->OnStatsChanged(); }
		virtual void SendMessage (CSpaceObject *pSender, const CString &sMsg);
		virtual bool SetAbility (Abilities iAbility, AbilityModifications iModification, int iDuration, DWORD dwOptions);
		virtual void SetFireDelay (CInstalledDevice *pWeapon, int iDelay = -1);
		virtual void SetGlobalData (const CString &sAttribute, const CString &sData) { m_pClass->SetGlobalData(sAttribute, sData); }
		virtual void SetIdentified (bool bIdentified = true) { m_fIdentified = bIdentified; }
		virtual void SetKnown (bool bKnown = true) { m_fKnown = bKnown; }
		virtual void SetName (const CString &sName, DWORD dwFlags = 0) { m_sName = sName; m_dwNameFlags = dwFlags; }
		virtual void SetOverlayData (DWORD dwID, const CString &sAttribute, const CString &sData) { m_EnergyFields.SetData(dwID, sAttribute, sData); }
		virtual bool SetOverlayEffectProperty (DWORD dwID, const CString &sProperty, ICCItem *pValue) { return m_EnergyFields.SetEffectProperty(dwID, sProperty, pValue); }
		virtual void SetOverlayPos (DWORD dwID, const CVector &vPos) { m_EnergyFields.SetPos(this, dwID, vPos); }
		virtual bool SetOverlayProperty (DWORD dwID, const CString &sName, ICCItem *pValue, CString *retsError) { return m_EnergyFields.SetProperty(this, dwID, sName, pValue); }
		virtual void SetOverlayRotation (DWORD dwID, int iRotation) { m_EnergyFields.SetRotation(dwID, iRotation); }
		virtual bool SetProperty (const CString &sName, ICCItem *pValue, CString *retsError);
		virtual void SetSovereign (CSovereign *pSovereign) { m_pSovereign = pSovereign; }
		virtual void Suspend (void) { m_fManualSuspended = true; SetCannotBeHit(); }
		virtual void Undock (CSpaceObject *pObj);
		virtual void UpdateArmorItems (void);
		virtual void UpdateDockingManeuver(const CVector &vDest, const CVector &vDestVel, int iDestFacing);

	protected:

		//	CSpaceObject virtuals
		virtual bool CanFireOn (CSpaceObject *pObj) { return CanFireOnObjHelper(pObj); }
		virtual void GateHook (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate, bool bAscend);
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx);
		virtual void OnDestroyed (SDestroyCtx &Ctx);
		virtual CSpaceObject *OnGetOrderGiver (void);
		virtual void OnObjEnteredGate (CSpaceObject *pObj, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnPaintAnnotations (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnPaintMap (CMapViewportCtx &Ctx, CG16bitImage &Dest, int x, int y);
		virtual void OnPaintSRSEnhancements (CG16bitImage &Dest, SViewportPaintCtx &Ctx) { m_pController->OnPaintSRSEnhancements(Dest, Ctx); }
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnSetEventFlags (void);
		virtual void OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick);
		virtual void OnUpdatePlayer (SUpdateCtx &Ctx) { m_pController->OnUpdatePlayer(Ctx); }
		virtual void OnWriteToStream (IWriteStream *pStream);
		virtual bool OrientationChanged (void);
		virtual void RevertOrientationChange (void);

	private:
		CShip (void);

		void CalcArmorBonus (void);
		void CalcBounds (void);
		int CalcMaxCargoSpace (void) const;
		void CalcDeviceBonus (void);
		int CalcDeviceSlotsInUse (int *retiWeaponSlots = NULL, int *retiNonWeapon = NULL) const;
		bool CalcDeviceTarget (STargetingCtx &Ctx, CItemCtx &ItemCtx, CSpaceObject **retpTarget, int *retiFireSolution);
		InstallItemResults CalcDeviceToReplace (const CItem &Item, int *retiSlot = NULL);
		DWORD CalcEffectsMask (void);
		void CalcOverlayImpact (void);
		void CalcReactorStats (void);
		int FindDeviceIndex (CInstalledDevice *pDevice) const;
		int FindFreeDeviceSlot (void);
		bool FindInstalledDeviceSlot (const CItem &Item, int *retiDev = NULL);
		int FindNextDevice (int iStart, ItemCategories Category, int iDir = 1);
		int FindRandomDevice (bool bEnabledOnly = false);
		void FinishCreation (SShipGeneratorCtx *pCtx = NULL);
		Metric GetCargoMass (void);
		ItemCategories GetCategoryForNamedDevice (DeviceNames iDev);
		Metric GetItemMass (void);
		bool IsSingletonDevice (ItemCategories iItemCat);
		void ReactorOverload (void);
		bool ShieldsAbsorbFire (CInstalledDevice *pWeapon);
		void SetDriveDesc (const DriveDesc *pDesc);
		void SetOrdersFromGenerator (SShipGeneratorCtx &Ctx);
		inline bool ShowParalyzedEffect (void) const { return (m_iParalysisTimer != 0 || m_iDisarmedTimer > 0 || m_fDeviceDisrupted); }

		CShipClass *m_pClass;					//	Ship class
		IShipController *m_pController;			//	Controller
		CSovereign *m_pSovereign;				//	Allegiance
		CString m_sName;						//	Ship's name
		DWORD m_dwNameFlags;					//	Name flags
		CString m_sMapLabel;					//	Map label

		TArray<CInstalledArmor> m_Armor;		//	Array of CInstalledArmor
		int m_iDeviceCount;						//	Number of devices
		CInstalledDevice *m_Devices;			//	Array of devices
		int m_NamedDevices[devNamesCount];
		const DriveDesc *m_pDriveDesc;			//	Drive descriptor
		const ReactorDesc *m_pReactorDesc;		//	Reactor descriptor
		CIntegralRotation m_Rotation;			//	Ship rotation
		CObjectEffectList m_Effects;			//	List of effects to paint
		CShipInterior m_Interior;				//	Interior decks and compartments (optionally)
		CEnergyFieldList m_EnergyFields;		//	List of energy fields
		CDockingPorts m_DockingPorts;			//	Docking ports (optionally)
		CStationType *m_pEncounterInfo;			//	Pointer back to encounter type (generally NULL)
		CTradingDesc *m_pTrade;					//	Override of trading desc (may be NULL)

		int m_iFireDelay:16;					//	Ticks until next fire
		int m_iMissileFireDelay:16;				//	Ticks until next missile fire
		int m_iSpare:16;
		int m_iContaminationTimer:16;			//	Ticks left to live
		int m_iBlindnessTimer:16;				//	Ticks until blindness wears off
												//	(-1 = permanent)
		int m_iParalysisTimer:16;				//	Ticks until paralysis wears off
												//	(-1 = permanent)
		int m_iExitGateTimer:16;				//	Ticks until ship exits gate (in gate until then)
		int m_iDisarmedTimer:16;				//	Ticks until ship can use weapons
												//	(-1 = permanent)
		int m_iLRSBlindnessTimer:16;			//	Ticks until LRS blindness wears off
												//	(-1 = permanent)
		int m_iDriveDamagedTimer:16;			//	Ticks until drive repaired
												//	(-1 = permanent)
		int m_iLastFireTime;					//	Tick when we last fired a weapon
		int m_iLastHitTime;						//	Tick when we last got hit by something

		int m_iFuelLeft;						//	Fuel left (kilos)
		Metric m_rItemMass;						//	Total mass of all items (including installed)
		Metric m_rCargoMass;					//	Mass of cargo items (not including installed)
		int m_iPowerDrain;						//	(temp) power consumed (1/10 megawatt)
		int m_iMaxPower;						//	(temp) max power (1/10 megawatt)
		int m_iThrust;							//	Computed thrust
		Metric m_rMaxSpeed;						//	Computed max speed
		int m_iStealth;							//	Computed stealth

		CSpaceObject *m_pDocked;				//	If not NULL, object we are docked to.
		CSpaceObject *m_pExitGate;				//	If not NULL, gate we are about to exit.
		CDamageSource *m_pIrradiatedBy;			//	If not NULL, object that irradiated us
		SShipGeneratorCtx *m_pDeferredOrders;	//	Defer orders until system done being created.

		DWORD m_fOutOfFuel:1;					//	TRUE if ship is out of fuel
		DWORD m_fRadioactive:1;					//	TRUE if radioactive
		DWORD m_fHasAutopilot:1;				//	TRUE if ship has autopilot
		DWORD m_fDestroyInGate:1;				//	TRUE if ship has entered a gate
		DWORD m_fHalfSpeed:1;					//	TRUE if ship is at half speed
		DWORD m_fHasTargetingComputer:1;		//	TRUE if ship has targeting computer
		DWORD m_fTrackFuel:1;					//	TRUE if ship keeps track of fuel (only player ship does)
		DWORD m_fSRSEnhanced:1;					//	TRUE if ship's SRS is enhanced
		
		DWORD m_fDeviceDisrupted:1;				//	TRUE if at least one device is disrupted
		DWORD m_fKnown:1;						//	TRUE if we know about this ship
		DWORD m_fHiddenByNebula:1;				//	TRUE if ship is hidden by nebula
		DWORD m_fTrackMass:1;					//	TRUE if ship keeps track of mass to compute performance
		DWORD m_fIdentified:1;					//	TRUE if player can see ship class, etc.
		DWORD m_fManualSuspended:1;				//	TRUE if ship is suspended
		DWORD m_fGalacticMap:1;					//	TRUE if ship has galactic map installed
		DWORD m_fRecalcItemMass:1;				//	TRUE if we need to recalculate m_rImageMass

		DWORD m_fDockingDisabled:1;				//	TRUE if docking is disabled
		DWORD m_fControllerDisabled:1;			//	TRUE if we want to disable controller
		DWORD m_fRecalcRotationAccel:1;			//	TRUE if we need to recalc rotation acceleration
		DWORD m_fParalyzedByOverlay:1;			//	TRUE if one or more overlays paralyze the ship.
		DWORD m_fDisarmedByOverlay:1;			//	TRUE if one or more overlays disarmed the ship.
		DWORD m_fSpinningByOverlay:1;			//	TRUE if we should spin wildly
		DWORD m_fDragByOverlay:1;				//	TRUE if overlay imposes drag
		DWORD m_fSpare8:1;

		DWORD m_dwSpare:8;

	friend CObjectClass<CShip>;
	};

class CStaticEffect : public CSpaceObject
	{
	public:
		static ALERROR Create (CEffectCreator *pType,
							   CSystem *pSystem,
							   const CVector &vPos,
							   CStaticEffect **retpEffect);
		virtual ~CStaticEffect (void);

		//	CSpaceObject virtuals
		virtual CString GetObjClassName (void) { return CONSTLIT("CStaticEffect"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerEffects; }
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans) { }

	protected:

		//	CSpaceObject virtuals
		virtual bool CanHit (CSpaceObject *pObj) { return false; }
		virtual EDamageResults OnDamage (SDamageCtx &Ctx) { return damagePassthrough; }
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CStaticEffect (void);

		IEffectPainter *m_pPainter;

	friend CObjectClass<CStaticEffect>;
	};

class CStation : public CSpaceObject
	{
	public:
		static ALERROR CreateFromType (CSystem *pSystem,
				CStationType *pType,
				SObjCreateCtx &CreateCtx,
				CStation **retpStation,
				CString *retsError = NULL);
		virtual ~CStation (void);

		inline void ClearFireReconEvent (void) { m_fFireReconEvent = false; }
		inline void ClearReconned (void) { m_fReconned = false; }
		int GetImageVariant (void);
		inline int GetImageVariantCount (void) { return m_pType->GetImageVariants(); }
		inline int GetMaxStructuralHitPoints (void) { return m_iMaxStructuralHP; }
		inline CSpaceObject *GetShipAtDockingPort (int iPort) { return m_DockingPorts.GetPortObj(this, iPort); }
		inline int GetStructuralHitPoints (void) { return m_iStructuralHP; }
		inline int GetSubordinateCount (void) { return m_Subordinates.GetCount(); }
		inline CSpaceObject *GetSubordinate (int iIndex) { return m_Subordinates.GetObj(iIndex); }
		inline bool IsNameSet (void) { return !m_sName.IsBlank(); }
		inline bool IsReconned (void) { return (m_fReconned ? true : false); }
		inline void SetActive (void) { m_fActive = true; }
		inline void SetBase (CSpaceObject *pBase) { m_pBase = pBase; }
		inline void SetFireReconEvent (void) { m_fFireReconEvent = true; }
		void SetFlotsamImage (CItemType *pItemType);
		void SetImageVariant (int iVariant);
		inline void SetInactive (void) { m_fActive = false; }
		void SetMapOrbit (const COrbit &oOrbit);
		inline void SetMass (Metric rMass) { m_rMass = rMass; }
		inline void SetMaxStructuralHitPoints (int iHP) { m_iMaxStructuralHP = iHP; }
		inline void SetNoConstruction (void) { m_fNoConstruction = true; }
		inline void SetNoMapLabel (void) { m_fNoMapLabel = true; }
		inline void SetNoReinforcements (void) { m_fNoReinforcements = true; }
		inline void SetReconned (void) { m_fReconned = true; }
		inline void SetShowMapLabel (bool bShow = true) { m_fNoMapLabel = !bShow; }
		void SetStargate (const CString &sDestNode, const CString &sDestEntryPoint);
		inline void SetStructuralHitPoints (int iHP) { m_iStructuralHP = iHP; }
		void SetWreckImage (CShipClass *pWreckClass);
		void SetWreckParams (CShipClass *pWreckClass, CShip *pShip = NULL);

		//	CSpaceObject virtuals
		virtual void AddOverlay (COverlayType *pType, int iPosAngle, int iPosRadius, int iRotation, int iLifetime, DWORD *retdwID = NULL);
		virtual void AddSubordinate (CSpaceObject *pSubordinate);
		virtual CTradingDesc *AllocTradeDescOverride (void);
		virtual CStation *AsStation (void) { return this; }
		virtual bool CanAttack (void) const;
		virtual bool CanBeDestroyed (void) { return (m_iStructuralHP > 0); }
		virtual bool CanBlock (CSpaceObject *pObj);
		virtual bool CanBlockShips (void) { return m_pType->IsWall(); }
		virtual bool CanMove (void) { return IsMobile(); }
		virtual CurrencyValue ChargeMoney (DWORD dwEconomyUNID, CurrencyValue iValue);
		virtual bool ClassCanAttack (void);
		virtual void CreateRandomDockedShips (IShipGenerator *pGenerator, int iCount = 1);
		virtual CurrencyValue CreditMoney (DWORD dwEconomyUNID, CurrencyValue iValue);
		virtual CString DebugCrashInfo (void);
		virtual void Decontaminate (void) { m_fRadioactive = false; }
		virtual CurrencyValue GetBalance (DWORD dwEconomyUNID);
		virtual Categories GetCategory (void) const { return catStation; }
		virtual DWORD GetClassUNID (void) { return m_pType->GetUNID(); }
		virtual int GetDamageEffectiveness (CSpaceObject *pAttacker, CInstalledDevice *pWeapon);
		virtual DWORD GetDefaultBkgnd (void) { return m_pType->GetDefaultBkgnd(); }
		virtual CInstalledDevice *GetDevice (int iDev) const { return &m_pDevices[iDev]; }
		virtual int GetDeviceCount (void) const { return (m_pDevices ? maxDevices : 0); }
		virtual int GetDockingPortCount (void) { return m_DockingPorts.GetPortCount(this); }
		virtual CDesignType *GetDefaultDockScreen (CString *retsName = NULL);
		virtual CStationType *GetEncounterInfo (void) { return m_pType; }
		virtual const CString &GetGlobalData (const CString &sAttribute) { return m_pType->GetGlobalData(sAttribute); }
		virtual Metric GetGravity (Metric *retrRadius) const;
		virtual const CObjectImageArray &GetImage (void) const { return m_pType->GetImage(m_ImageSelector, CCompositeImageModifiers()); }
		virtual int GetLevel (void) const { return m_pType->GetLevel(); }
		virtual const COrbit *GetMapOrbit (void) const { return m_pMapOrbit; }
		virtual Metric GetMass (void) { return m_rMass; }
		virtual int GetMaxLightDistance (void) { return m_pType->GetMaxLightDistance(); }
		virtual CString GetName (DWORD *retdwFlags = NULL);
		virtual int GetNearestDockPort (CSpaceObject *pRequestingObj, CVector *retvPort = NULL);
		virtual CString GetObjClassName (void) { return CONSTLIT("CStation"); }
		virtual int GetOpenDockingPortCount (void) { return m_DockingPorts.GetPortCount(this) - m_DockingPorts.GetPortsInUseCount(this); }
		virtual CEnergyField *GetOverlay (DWORD dwID) const { return m_Overlays.GetOverlay(dwID); }
		virtual const CString &GetOverlayData (DWORD dwID, const CString &sAttrib) { return m_Overlays.GetData(dwID, sAttrib); }
		virtual void GetOverlayImpact (CEnergyFieldList::SImpactDesc *retImpact) { m_Overlays.GetImpact(this, retImpact); }
		virtual void GetOverlayList (TArray<CEnergyField *> &List) { m_Overlays.GetList(List); }
		virtual CVector GetOverlayPos (DWORD dwID) { return m_Overlays.GetPos(this, dwID); }
		virtual ICCItem *GetOverlayProperty (CCodeChainCtx *pCCCtx, DWORD dwID, const CString &sName) { return m_Overlays.GetProperty(pCCCtx, this, dwID, sName); }
		virtual int GetOverlayRotation (DWORD dwID) { return m_Overlays.GetRotation(dwID); }
		virtual COverlayType *GetOverlayType (DWORD dwID) { return m_Overlays.GetType(dwID); }
		virtual CSystem::LayerEnum GetPaintLayer (void);
		virtual Metric GetParallaxDist (void) { return m_rParallaxDist; }
		virtual EDamageResults GetPassthroughDefault (void);
		virtual int GetPlanetarySize (void) const { return (GetScale() == scaleWorld ? m_pType->GetSize() : 0); }
		virtual ICCItem *GetProperty (const CString &sName);
		virtual IShipGenerator *GetRandomEncounterTable (int *retiFrequency = NULL) const;
		virtual ScaleTypes GetScale (void) const { return m_Scale; }
		virtual CXMLElement *GetScreen (const CString &sName) { return m_pType->GetScreen(sName); }
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual COLORREF GetSpaceColor (void) { return m_pType->GetSpaceColor(); }
		virtual CString GetStargateID (void) const;
		virtual int GetStealth (void) const { return ((m_fKnown && !IsMobile()) ? stealthMin : m_pType->GetStealth()); }
		virtual Metric GetStellarMass (void) const { return (GetScale() == scaleStar ? m_rMass : 0.0); }
		virtual CSpaceObject *GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget = false) const;
		virtual CTradingDesc *GetTradeDescOverride (void) { return m_pTrade; }
		virtual CDesignType *GetType (void) const { return m_pType; }
		virtual int GetVisibleDamage (void);
		virtual CDesignType *GetWreckType (void) const;
		virtual bool HasAttribute (const CString &sAttribute) const;
		virtual bool HasMapLabel (void);
		virtual bool ImageInObject (const CVector &vObjPos, const CObjectImageArray &Image, int iTick, int iRotation, const CVector &vImagePos);
		virtual bool IsAbandoned (void) const { return (m_iHitPoints == 0 && !IsImmutable()); }
		virtual bool IsActiveStargate (void) const { return !m_sStargateDestNode.IsBlank() && m_fActive; }
		virtual bool IsAngryAt (CSpaceObject *pObj) { return (IsEnemy(pObj) || IsBlacklisted(pObj)); }
		virtual bool IsDisarmed (void) { return m_fDisarmedByOverlay; }
		virtual bool IsExplored (void) { return m_fExplored; }
		virtual bool IsIdentified (void) { return m_fKnown; }
		virtual bool IsImmutable (void) const { return m_fImmutable; }
		virtual bool IsKnown (void) { return m_fKnown; }
		virtual bool IsMultiHull (void) { return m_pType->IsMultiHull(); }
		virtual bool IsObjDocked (CSpaceObject *pObj) { return m_DockingPorts.IsObjDocked(pObj); }
		virtual bool IsObjDockedOrDocking (CSpaceObject *pObj) { return m_DockingPorts.IsObjDockedOrDocking(pObj); }
		virtual bool IsParalyzed (void) { return m_fParalyzedByOverlay; }
		virtual bool IsRadioactive (void) { return (m_fRadioactive ? true : false); }
		virtual bool IsStargate (void) const { return !m_sStargateDestNode.IsBlank(); }
		virtual bool IsTimeStopImmune (void) { return m_pType->IsTimeStopImmune(); }
		virtual bool IsVirtual (void) const { return m_pType->IsVirtual(); }
		virtual bool IsWreck (void) const { return (m_dwWreckUNID != 0); }
		virtual void MakeRadioactive (void) { m_fRadioactive = true; }
		virtual void MarkImages (void) { m_pType->MarkImages(m_ImageSelector); }
		virtual bool ObjectInObject (const CVector &vObj1Pos, CSpaceObject *pObj2, const CVector &vObj2Pos);
		virtual void OnDestroyed (SDestroyCtx &Ctx);
		virtual void OnObjBounce (CSpaceObject *pObj, const CVector &vPos);
		virtual void OnObjLeaveGate (CSpaceObject *pObj);
		virtual void OnPlayerObj (CSpaceObject *pPlayer);
		virtual void OnStationDestroyed (const SDestroyCtx &Ctx);
		virtual void OnSystemCreated (void);
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);
		virtual void PlaceAtRandomDockPort (CSpaceObject *pObj) { m_DockingPorts.DockAtRandomPort(this, pObj); }
		virtual bool PointInObject (const CVector &vObjPos, const CVector &vPointPos);
		virtual bool PointInObject (SPointInObjectCtx &Ctx, const CVector &vObjPos, const CVector &vPointPos);
		virtual void PointInObjectInit (SPointInObjectCtx &Ctx);
		virtual void RemoveOverlay (DWORD dwID);
		virtual bool RemoveSubordinate (CSpaceObject *pSubordinate);
		virtual bool RequestDock (CSpaceObject *pObj, int iPort = -1);
		virtual bool RequestGate (CSpaceObject *pObj);
		virtual void SetExplored (bool bExplored = true) { m_fExplored = bExplored; }
		virtual void SetGlobalData (const CString &sAttribute, const CString &sData) { m_pType->SetGlobalData(sAttribute, sData); }
		virtual void SetIdentified (bool bIdentified = true) { m_fKnown = bIdentified; }
		virtual void SetKnown (bool bKnown = true);
		virtual void SetMapLabelPos (int x, int y) { m_xMapLabel = x; m_yMapLabel = y; }
		virtual void SetName (const CString &sName, DWORD dwFlags = 0);
		virtual void SetOverlayData (DWORD dwID, const CString &sAttribute, const CString &sData) { m_Overlays.SetData(dwID, sAttribute, sData); }
		virtual bool SetOverlayEffectProperty (DWORD dwID, const CString &sProperty, ICCItem *pValue) { return m_Overlays.SetEffectProperty(dwID, sProperty, pValue); }
		virtual void SetOverlayPos (DWORD dwID, const CVector &vPos) { m_Overlays.SetPos(this, dwID, vPos); }
		virtual bool SetOverlayProperty (DWORD dwID, const CString &sName, ICCItem *pValue, CString *retsError) { return m_Overlays.SetProperty(this, dwID, sName, pValue); }
		virtual void SetOverlayRotation (DWORD dwID, int iRotation) { m_Overlays.SetRotation(dwID, iRotation); }
		virtual bool SetProperty (const CString &sName, ICCItem *pValue, CString *retsError);
		virtual void SetSovereign (CSovereign *pSovereign) { m_pSovereign = pSovereign; }
		virtual bool SupportsGating (void) { return IsActiveStargate(); }
		virtual void Undock (CSpaceObject *pObj);

	protected:

		//	CSpaceObject virtuals
		virtual bool CanFireOn (CSpaceObject *pObj) { return CanFireOnObjHelper(pObj); }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);
		virtual void OnComponentChanged (ObjectComponentTypes iComponent);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx);
		virtual void OnObjEnteredGate (CSpaceObject *pObj, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnPaintAnnotations (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnPaintMap (CMapViewportCtx &Ctx, CG16bitImage &Dest, int x, int y);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnSetEventFlags (void);
		virtual void OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick);
		virtual void OnUpdateExtended (const CTimeSpan &ExtraTime);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum Constants
			{
			maxRegisteredObjects	= 12,
			maxDevices				= 8,
			};

		CStation (void);

		void Blacklist (CSpaceObject *pObj);
		void CalcBounds (void);
		int CalcNumberOfShips (void);
		void CalcOverlayImpact (void);
		void ClearBlacklist (CSpaceObject *pObj);
		void CreateDestructionEffect (void);
		void CreateEjectaFromDamage (int iDamage, const CVector &vHitPos, int iDirection, const DamageDesc &Damage);
		void CreateStructuralDestructionEffect (SDestroyCtx &Ctx);
		ALERROR CreateMapImage (void);
		void FinishCreation (void);
		void FriendlyFire (CSpaceObject *pAttacker);
		Metric GetAttackDistance (void) const;
		const CObjectImageArray &GetImage (bool bFade, int *retiTick, int *retiRotation);
		bool IsBlacklisted (CSpaceObject *pObj = NULL);
		void RaiseAlert (CSpaceObject *pTarget);
		void SetAngry (void);
		inline bool ShowWreckImage (void) { return (IsAbandoned() && m_iMaxHitPoints > 0); }
		void UpdateAttacking (SUpdateCtx &Ctx, int iTick);
		void UpdateReinforcements (int iTick);

		CStationType *m_pType;					//	Station type
		CString m_sName;						//	Station name
		DWORD m_dwNameFlags;					//	Name flags
		CSovereign *m_pSovereign;				//	Allegiance
		ScaleTypes m_Scale;						//	Scale of station
		Metric m_rMass;							//	Mass of station (depends on scale)

		CCompositeImageSelector m_ImageSelector;//	Image variant to display
		int m_iDestroyedAnimation;				//	Frames left of destroyed animation
		COrbit *m_pMapOrbit;					//	Orbit to draw on map
		int m_xMapLabel;						//	Name label in map view
		int m_yMapLabel;
		Metric m_rParallaxDist;					//	Parallax distance (1.0 = normal; > 1.0 = background; < 1.0 = foreground)

		CString m_sStargateDestNode;			//	Destination node
		CString m_sStargateDestEntryPoint;		//	Destination entry point

		CArmorClass *m_pArmorClass;				//	Armor class
		int m_iHitPoints;						//	Total hit points (0 = station abandoned)
		int m_iMaxHitPoints;					//	Max hit points (0 = station cannot be abandoned)
		int m_iStructuralHP;					//	Structural hp (0 = station cannot be destroyed)
		int m_iMaxStructuralHP;					//	Max structural hp

		CInstalledDevice *m_pDevices;			//	Array of devices
		CEnergyFieldList m_Overlays;			//	List of overlays
		CDockingPorts m_DockingPorts;			//	Docking ports

		CSpaceObject *m_pTarget;				//	Target to hit (by our weapons)
		CSpaceObject *m_pBase;					//	If we're a subordinate, this points to our base
		CSpaceObjectList m_Subordinates;		//	List of subordinates
		CSpaceObjectList m_Targets;				//	Targets to destroy (by our ships)

		CAttackDetector m_Blacklist;			//	Player blacklisted
		int m_iAngryCounter;					//	Attack cycles until station is not angry
		int m_iReinforceRequestCount;			//	Number of times we've requested reinforcements

		CCurrencyBlock *m_pMoney;				//	Money left (may be NULL)
		CTradingDesc *m_pTrade;					//	Override of trading desc (may be NULL)

		DWORD m_fArmed:1;						//	TRUE if station has weapons
		DWORD m_fKnown:1;						//	TRUE if known to the player
		DWORD m_fNoMapLabel:1;					//	Do not show map label
		DWORD m_fActive:1;						//	TRUE if stargate is active
		DWORD m_fNoReinforcements:1;			//	Do not send reinforcements
		DWORD m_fRadioactive:1;					//	TRUE if radioactive
		DWORD m_fReconned:1;					//	TRUE if reconned by player
		DWORD m_fFireReconEvent:1;				//	If TRUE, fire OnReconned

		DWORD m_fImmutable:1;					//	If TRUE, station is immutable
		DWORD m_fExplored:1;					//	If TRUE, player has docked at least once
		DWORD m_fDisarmedByOverlay:1;			//	If TRUE, an overlay has disarmed us
		DWORD m_fParalyzedByOverlay:1;			//	If TRUE, an overlay has paralyzed us
		DWORD m_fNoBlacklist:1;					//	If TRUE, do not blacklist player on friendly fire
		DWORD m_fNoConstruction:1;				//	Do not build new ships
		DWORD m_fSpare7:1;
		DWORD m_fSpare8:1;

		DWORD m_dwSpare:16;

		//	Wreck image
		DWORD m_dwWreckUNID;					//	UNID of wreck class (0 if none)

		CG16bitImage m_MapImage;				//	Image for the map (if star or world)
		CString m_sMapLabel;					//	Label for map

	friend CObjectClass<CStation>;
	};

#endif

