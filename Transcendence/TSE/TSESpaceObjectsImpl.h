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
				int iBonus,
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
		virtual bool CanHit (CSpaceObject *pObj) { return MissileCanHitObj(pObj, m_Source.GetObj(), m_pDesc->m_bCanDamageSource); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx) { return damagePassthrough; }
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CAreaDamage (void);

		CWeaponFireDesc *m_pDesc;				//	Weapon descriptor
		int m_iBonus;							//	Bonus
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
		virtual bool CanHit (CSpaceObject *pObj) { return MissileCanHitObj(pObj, m_Source.GetObj(), m_pDesc->m_bCanDamageSource); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx) { return damagePassthrough; }
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
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
		virtual void OnUpdate (Metric rSecondsPerTick);
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
		virtual void OnUpdate (Metric rSecondsPerTick);
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
		virtual void OnUpdate (Metric rSecondsPerTick);
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
			int iShape;						//	Index of shape
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
				int iBonus,
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
		virtual Categories GetCategory (void) const { return (m_pDesc->GetFireType() == ftBeam ? catBeam : catMissile); }
		virtual CString GetDamageCauseNounPhrase (DWORD dwFlags) { return m_Source.GetDamageCauseNounPhrase(dwFlags); }
		virtual DestructionTypes GetDamageCauseType (void) { return m_iCause; }
		virtual int GetInteraction (void) { return m_pDesc->GetInteraction(); }
		virtual CString GetName (DWORD *retdwFlags = NULL);
		virtual CString GetObjClassName (void) { return CONSTLIT("CMissile"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerStations; }
		virtual int GetRotation (void) const { return m_iRotation; }
		virtual CSpaceObject *GetSecondarySource (void) { return m_Source.GetSecondaryObj(); }
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual CSpaceObject *GetSource (void) { return m_Source.GetObj(); }
		virtual int GetStealth (void) const;
		virtual CWeaponFireDesc *GetWeaponFireDesc (void) { return m_pDesc; }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);
		virtual bool PointInObject (const CVector &vObjPos, const CVector &vPointPos);

	protected:

		//	Virtuals to be overridden
		virtual bool CanHit (CSpaceObject *pObj) { return MissileCanHitObj(pObj, m_Source.GetObj(), m_pDesc->m_bCanDamageSource); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
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

		CWeaponFireDesc *m_pDesc;				//	Weapon descriptor
		int m_iBonus;							//	Bonus damage
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
		CG16bitRegion *m_pVaporTrailRegions;	//	Array of vapor trail regions
		int m_iSavedRotationsCount;				//	Number of saved rotations
		int *m_pSavedRotations;					//	Array of saved rotation angles

		DWORD m_fDestroyed:1;					//	TRUE if destroyed (only around to paint effect)
		DWORD m_fReflection:1;					//	TRUE if this is a reflection
		DWORD m_fDetonate:1;					//	TRUE if we should detonate on next update
		DWORD m_dwSpareFlags:29;				//	Flags

	friend CObjectClass<CMissile>;
	};

class CParticleDamage : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
				CWeaponFireDesc *pDesc,
				int iBonus,
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
		virtual bool CanHit (CSpaceObject *pObj) { return MissileCanHitObj(pObj, m_Source.GetObj(), m_pDesc->m_bCanDamageSource); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx) { return damagePassthrough; }
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CParticleDamage (void);

		void InitParticles (int iCount, const CVector &vPos, const CVector &vInitVel, int iDirection);

		CWeaponFireDesc *m_pDesc;				//	Weapon descriptor
		CSpaceObject *m_pTarget;				//	Target
		int m_iBonus;							//	Bonus
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
		virtual void OnUpdate (Metric rSecondsPerTick);
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
				int iBonus,
				DestructionTypes iCause,
				const CDamageSource &Source,
				const CVector &vPos,
				const CVector &vVel,
				CSpaceObject *pTarget,
				CRadiusDamage **retpObj);
		~CRadiusDamage (void);

		//	CSpaceObject virtuals
		virtual bool CanMove (void) { return true; }
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
		virtual bool CanHit (CSpaceObject *pObj) { return MissileCanHitObj(pObj, m_Source.GetObj(), m_pDesc->m_bCanDamageSource); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx) { return damagePassthrough; }
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CRadiusDamage (void);

		CWeaponFireDesc *m_pDesc;				//	Weapon descriptor
		int m_iBonus;							//	Bonus
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
		virtual void OnUpdate (Metric rSecondsPerTick);
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
		enum InstallArmorStatus
			{
			insArmorOK				= 0,
			insArmorTooHeavy		= 1,	//	Armor is too heavy for ship class
			};

		enum InstallDeviceStatus
			{
			insOK					= 0,	//	Can install this device
			insNotADevice			= 1,	//	Item is not a device
			insNoSlots				= 2,	//	No slots available for any device
			insAlreadyInstalled		= 3,	//	Device is already installed
			insShieldsInstalled		= 4,	//	Shield generator is already installed
			insDriveInstalled		= 5,	//	Drive is already installed
			insLauncherInstalled	= 6,	//	Missile launcher already installed
			insEngineCoreTooWeak	= 7,	//	Engine core is too weak
			insCargoInstalled		= 8,	//	Cargo expansion is already installed
			insReactorInstalled		= 9,	//	Reactor upgrade is already installed
			insCargoMaxExpansion	= 10,	//	Cargo expansion does not fit on ship
			insReactorMaxPower		= 11,	//	Reactor is too powerful for ship
			insNoWeaponSlots		= 12,	//	No slots available for more weapons
			insNoGeneralSlots		= 13,	//	No slots available for non-weapons
			};

		enum RemoveDeviceStatus
			{
			remOK					= 0,	//	Can remove this device
			remTooMuchCargo			= 1,	//	Can't remove cargo expansion (because too much cargo)
			remNotInstalled			= 2,	//	Device is not installed
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
		inline DWORD GetCurrentOrderData (void) { return m_pController->GetCurrentOrderData(); }

		//	Armor methods
		inline CInstalledArmor *GetArmorSection (int iSect) { return (CInstalledArmor *)m_Armor.GetStruct(iSect); }
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
		InstallArmorStatus CanInstallArmor (const CItem &Item) const;
		InstallArmorStatus CanInstallArmor (CItemListManipulator &ItemList) const;
		InstallDeviceStatus CanInstallDevice (const CItem &Item, bool bReplace = false);
		InstallDeviceStatus CanInstallDevice (CItemListManipulator &ItemList);
		RemoveDeviceStatus CanRemoveDevice (const CItem &Item);
		void ClearAllTriggered (void);
		void DamageExternalDevice (int iDev, SDamageCtx &Ctx);
		void EnableDevice (int iDev, bool bEnable = true);
		DeviceNames GetDeviceNameForCategory (ItemCategories iCategory);
		int GetItemDeviceName (const CItem &Item) const;
		CItem GetNamedDeviceItem (DeviceNames iDev);
		bool HasNamedDevice (DeviceNames iDev) const;
//		inline bool HasSecondaryWeapons (void) { return m_fHasSecondaryWeapons; }
		void InstallItemAsDevice (CItemListManipulator &ItemList, int iDeviceSlot = -1);
		bool IsDeviceSlotAvailable (void);
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
		CInstalledDevice *GetNamedDevice (DeviceNames iDev);
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
		int GetManeuverDelay (void);
		inline int GetRotationAngle (void) { return m_pClass->GetRotationAngle(); }
		inline int GetRotationRange (void) { return m_pClass->GetRotationRange(); }
		inline Metric GetThrust (void) { return m_iThrust; }
		inline bool IsInertialess (void) { return (m_pDriveDesc->fInertialess ? true : false); }
		inline void SetMaxSpeedHalf (void) { m_fHalfSpeed = true; }
		inline void ResetMaxSpeed (void) { m_fHalfSpeed = false; }

		//	Miscellaneous
		inline IShipController *GetController (void) { return m_pController; }
		inline CShipClass *GetClass (void) { return m_pClass; }
		void SetController (IShipController *pController, bool bFreeOldController = true);
		void SetCommandCode (ICCItem *pCode);
		inline void SetDestroyInGate (void) { m_fDestroyInGate = true; }
		inline void SetEncounterInfo (CStationType *pEncounterInfo) { m_pEncounterInfo = pEncounterInfo; }
		inline void SetPlayerWingman (bool bIsWingman) const { m_pController->SetPlayerWingman(bIsWingman); }
		inline void SetRotation (int iAngle) { m_iRotation = AlignToRotationAngle(iAngle); }
		void Undock (void);

		//	CSpaceObject virtuals
		virtual bool AbsorbWeaponFire (CInstalledDevice *pWeapon);
		virtual void AddOverlay (CEnergyFieldType *pType, int iPosAngle, int iPosRadius, int iRotation, int iLifetime, DWORD *retdwID = NULL);
		virtual CShip *AsShip (void) { return this; }
		virtual void Behavior (void);
		virtual bool CanAttack (void) const;
		virtual bool CanMove (void) { return true; }
		virtual CurrencyValue ChargeMoney (DWORD dwEconomyUNID, CurrencyValue iValue);
		virtual bool ClassCanAttack (void) { return true; }
		virtual void ConsumeFuel (int iFuel);
		virtual CurrencyValue CreditMoney (DWORD dwEconomyUNID, CurrencyValue iValue);
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
		virtual DamageTypes GetDamageType (void);
		virtual DWORD GetDefaultBkgnd (void) { return m_pClass->GetDefaultBkgnd(); }
		virtual CSpaceObject *GetDockedObj (void) { return m_pDocked; }
		virtual int GetDockingPortCount (void) { return m_DockingPorts.GetPortCount(this); }
		virtual CDesignType *GetDefaultDockScreen (CString *retsName = NULL);
		virtual CInstalledDevice *GetDevice (int iDev) const { return &m_Devices[iDev]; }
		virtual int GetDeviceCount (void) const { return m_iDeviceCount; }
		virtual CStationType *GetEncounterInfo (void) { return m_pEncounterInfo; }
		virtual CSpaceObject *GetEscortPrincipal (void) const;
		virtual const CString &GetGlobalData (const CString &sAttribute) { return m_pClass->GetGlobalData(sAttribute); }
		virtual const CObjectImageArray &GetImage (void) { return m_pClass->GetImage(); }
		virtual CString GetInstallationPhrase (const CItem &Item) const;
		virtual int GetLastFireTime (void) const { return m_iLastFireTime; }
		virtual int GetLevel (void) const { return m_pClass->GetLevel(); }
		virtual Metric GetMass (void);
		virtual int GetMaxPower (void) const;
		virtual CString GetName (DWORD *retdwFlags = NULL);
		virtual CVector GetNearestDockVector (CSpaceObject *pRequestingObj);
		virtual CString GetObjClassName (void) { return CONSTLIT("CShip"); }
		virtual int GetOpenDockingPortCount (void) { return m_DockingPorts.GetPortCount(this) - m_DockingPorts.GetPortsInUseCount(this); }
		virtual CEnergyField *GetOverlay (DWORD dwID) const { return m_EnergyFields.GetOverlay(dwID); }
		virtual const CString &GetOverlayData (DWORD dwID, const CString &sAttrib) { return m_EnergyFields.GetData(dwID, sAttrib); }
		virtual void GetOverlayList (TArray<CEnergyField *> &List) { m_EnergyFields.GetList(List); }
		virtual CVector GetOverlayPos (DWORD dwID) { return m_EnergyFields.GetPos(this, dwID); }
		virtual int GetOverlayRotation (DWORD dwID) { return m_EnergyFields.GetRotation(dwID); }
		virtual CEnergyFieldType *GetOverlayType (DWORD dwID) { return m_EnergyFields.GetType(dwID); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerShips; }
		virtual int GetPerception (void);
		virtual ICCItem *GetProperty (const CString &sName);
		virtual int GetRotation (void) const { return AlignToRotationAngle(m_iRotation); }
		virtual ScaleTypes GetScale (void) const { return scaleShip; }
		virtual int GetScore (void) { return m_pClass->GetScore(); }
		virtual CXMLElement *GetScreen (const CString &sName) { return m_pClass->GetScreen(sName); }
		virtual int GetSellPrice (const CItem &Item, bool bNoInventoryCheck = false);
		virtual int GetShieldLevel (void);
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual int GetStealth (void) const;
		virtual Metric GetMaxSpeed (void) { return (m_fHalfSpeed ? (m_rMaxSpeed / 2.0) : m_rMaxSpeed); }
		virtual CSpaceObject *GetTarget (bool bNoAutoTarget = false) const;
		virtual CDesignType *GetType (void) const { return m_pClass; }
		virtual int GetVisibleDamage (void);
		virtual bool HasAttribute (const CString &sAttribute) const;
		virtual bool ImageInObject (const CVector &vObjPos, const CObjectImageArray &Image, int iTick, int iRotation, const CVector &vImagePos);
		virtual bool IsAngryAt (CSpaceObject *pObj);
		virtual bool IsBlind (void) { return m_iBlindnessTimer != 0; }
		virtual bool IsDisarmed (void) { return m_iDisarmedTimer != 0; }
		virtual bool IsIdentified (void) { return m_fIdentified; }
		virtual bool IsInactive (void) const { return (m_fManualSuspended || m_iExitGateTimer > 0); }
		virtual bool IsObjDocked (CSpaceObject *pObj) { return m_DockingPorts.IsObjDocked(pObj); }
		virtual bool IsObjDockedOrDocking (CSpaceObject *pObj) { return m_DockingPorts.IsObjDockedOrDocking(pObj); }
		virtual bool IsOutOfFuel (void) { return m_fOutOfFuel; }
		virtual bool IsParalyzed (void) { return m_iParalysisTimer != 0; }
		virtual bool IsPlayer (void) const;
		virtual bool IsRadioactive (void) { return (m_fRadioactive ? true : false); }
		virtual bool IsSuspended (void) const { return m_fManualSuspended; }
		virtual bool IsTimeStopImmune (void) { return m_pClass->IsTimeStopImmune(); }
		virtual bool IsVirtual (void) const { return m_pClass->IsVirtual(); }
		virtual void MarkImages (void);
		virtual bool ObjectInObject (const CVector &vObj1Pos, CSpaceObject *pObj2, const CVector &vObj2Pos);
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
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual void OnNewSystem (CSystem *pSystem);
		virtual void OnPlayerChangedShips (CSpaceObject *pOldShip);
		virtual void OnPlayerObj (CSpaceObject *pPlayer);
		virtual void OnStationDestroyed (const SDestroyCtx &Ctx);
		virtual void OnSystemCreated (void);
		virtual void OnSystemLoaded (void);
		virtual void MakeBlind (int iTickCount = -1);
		virtual void MakeParalyzed (int iTickCount = -1);
		virtual void MakeRadioactive (void);
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);
		virtual void PaintMap (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);
		virtual bool PointInObject (const CVector &vObjPos, const CVector &vPointPos);
		virtual bool PointInObject (SPointInObjectCtx &Ctx, const CVector &vObjPos, const CVector &vPointPos);
		virtual void PointInObjectInit (SPointInObjectCtx &Ctx);
		virtual void ProgramDamage (CSpaceObject *pHacker, const ProgramDesc &Program);
		virtual void Refuel (int iFuel);
		virtual void Refuel (const CItem &Fuel);
		virtual void RemoveOverlay (DWORD dwID) { m_EnergyFields.RemoveField(this, dwID); }
		virtual void RepairDamage (int iHitPoints);
		virtual bool RequestDock (CSpaceObject *pObj);
		virtual void Resume (void) { m_fManualSuspended = false; if (!IsInGate()) ClearCannotBeHit(); }
		virtual void SendMessage (CSpaceObject *pSender, const CString &sMsg);
		virtual bool SetAbility (Abilities iAbility, AbilityModifications iModification, int iDuration, DWORD dwOptions);
		virtual void SetEventFlags (void);
		virtual void SetFireDelay (CInstalledDevice *pWeapon, int iDelay = -1);
		virtual void SetGlobalData (const CString &sAttribute, const CString &sData) { m_pClass->SetGlobalData(sAttribute, sData); }
		virtual void SetIdentified (bool bIdentified = true) { m_fIdentified = bIdentified; }
		virtual void SetKnown (bool bKnown = true) { m_fKnown = bKnown; }
		virtual void SetName (const CString &sName, DWORD dwFlags = 0) { m_sName = sName; m_dwNameFlags = dwFlags; }
		virtual void SetOverlayData (DWORD dwID, const CString &sAttribute, const CString &sData) { m_EnergyFields.SetData(dwID, sAttribute, sData); }
		virtual void SetOverlayPos (DWORD dwID, const CVector &vPos) { m_EnergyFields.SetPos(this, dwID, vPos); }
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
		virtual void GateHook (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate);
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx);
		virtual void OnDestroyed (SDestroyCtx &Ctx);
		virtual CSpaceObject *OnGetOrderGiver (void);
		virtual void OnObjEnteredGate (CSpaceObject *pObj, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);
		virtual bool OrientationChanged (void);
		virtual void RevertOrientationChange (void);

	private:
		CShip (void);

		void CalcArmorBonus (void);
		int CalcMaxCargoSpace (void) const;
		void CalcDeviceBonus (void);
		bool CalcDeviceTarget (STargetingCtx &Ctx, CItemCtx &ItemCtx, CSpaceObject **retpTarget, int *retiFireSolution);
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
		void ReactorOverload (void);
		bool ShieldsAbsorbFire (CInstalledDevice *pWeapon);
		void SetDriveDesc (const DriveDesc *pDesc);
		void SetOrdersFromGenerator (SShipGeneratorCtx &Ctx);

		CShipClass *m_pClass;					//	Ship class
		IShipController *m_pController;			//	Controller
		CSovereign *m_pSovereign;				//	Allegiance
		CString m_sName;						//	Ship's name
		DWORD m_dwNameFlags;					//	Name flags
		CString m_sMapLabel;					//	Map label

		int m_iRotation:16;						//	Current rotation (in degrees)
		int m_iPrevRotation:16;					//	Previous rotation

		CStructArray m_Armor;					//	Array of CInstalledArmor
		int m_iDeviceCount;						//	Number of devices
		CInstalledDevice *m_Devices;			//	Array of devices
		int m_NamedDevices[devNamesCount];
		const DriveDesc *m_pDriveDesc;			//	Drive descriptor
		const ReactorDesc *m_pReactorDesc;		//	Reactor descriptor
		CEnergyFieldList m_EnergyFields;		//	List of energy fields
		CDockingPorts m_DockingPorts;			//	Docking ports (optionally)
		CStationType *m_pEncounterInfo;			//	Pointer back to encounter type (generally NULL)

		int m_iFireDelay:16;					//	Ticks until next fire
		int m_iMissileFireDelay:16;				//	Ticks until next missile fire
		int m_iManeuverDelay:16;				//	Ticks until next rotation
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
		int m_iSpare:16;
		int m_iLastFireTime;					//	Tick when we last fired a weapon

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
		
		DWORD m_dwSpare:16;

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
				const CVector &vPos,
				const CVector &vVel,
				CXMLElement *pExtraData,
				CStation **retpStation);
		virtual ~CStation (void);

		inline void ClearFireReconEvent (void) { m_fFireReconEvent = false; }
		inline void ClearReconned (void) { m_fReconned = false; }
		inline CTradingDesc *GetDefaultTradingDesc (void) { return (m_pTrade ? m_pTrade : m_pType->GetTradingDesc()); }
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
		void SetImageVariant (int iVariant);
		inline void SetInactive (void) { m_fActive = false; }
		void SetMapOrbit (const COrbit &oOrbit);
		inline void SetMass (Metric rMass) { m_rMass = rMass; }
		inline void SetMaxStructuralHitPoints (int iHP) { m_iMaxStructuralHP = iHP; }
		inline void SetNoMapLabel (void) { m_fNoMapLabel = true; }
		inline void SetNoReinforcements (void) { m_fNoReinforcements = true; }
		inline void SetReconned (void) { m_fReconned = true; }
		inline void SetShowMapLabel (bool bShow = true) { m_fNoMapLabel = !bShow; }
		void SetStargate (const CString &sDestNode, const CString &sDestEntryPoint);
		inline void SetStructuralHitPoints (int iHP) { m_iStructuralHP = iHP; }
		void SetWreckImage (CShipClass *pWreckClass);
		void SetWreckParams (CShipClass *pWreckClass, CShip *pShip = NULL);

		//	CSpaceObject virtuals
		virtual void AddBuyOrder (CItemType *pType, const CString &sCriteria, int iPriceAdj);
		virtual void AddOverlay (CEnergyFieldType *pType, int iPosAngle, int iPosRadius, int iRotation, int iLifetime, DWORD *retdwID = NULL);
		virtual void AddSellOrder (CItemType *pType, const CString &sCriteria, int iPriceAdj);
		virtual void AddSubordinate (CSpaceObject *pSubordinate);
		virtual CStation *AsStation (void) { return this; }
		virtual bool CanAttack (void) const;
		virtual bool CanBeDestroyed (void) { return (m_iStructuralHP > 0); }
		virtual bool CanBlock (CSpaceObject *pObj);
		virtual bool CanBlockShips (void) { return m_pType->IsWall(); }
		virtual bool CanMove (void) { return IsMobile(); }
		virtual CurrencyValue ChargeMoney (DWORD dwEconomyUNID, CurrencyValue iValue);
		virtual bool ClassCanAttack (void);
		virtual void CreateRandomDockedShips (IShipGenerator *pGenerator);
		virtual CurrencyValue CreditMoney (DWORD dwEconomyUNID, CurrencyValue iValue);
		virtual CString DebugCrashInfo (void);
		virtual void Decontaminate (void) { m_fRadioactive = false; }
		virtual CurrencyValue GetBalance (DWORD dwEconomyUNID);
		virtual int GetBuyPrice (const CItem &Item, int *retiMaxCount = NULL);
		virtual Categories GetCategory (void) const { return catStation; }
		virtual DWORD GetClassUNID (void) { return m_pType->GetUNID(); }
		virtual DWORD GetDefaultBkgnd (void) { return m_pType->GetDefaultBkgnd(); }
		virtual CEconomyType *GetDefaultEconomy (void);
		virtual DWORD GetDefaultEconomyUNID (void);
		virtual CInstalledDevice *GetDevice (int iDev) const { return &m_pDevices[iDev]; }
		virtual int GetDeviceCount (void) const { return (m_pDevices ? maxDevices : 0); }
		virtual int GetDockingPortCount (void) { return m_DockingPorts.GetPortCount(this); }
		virtual CDesignType *GetDefaultDockScreen (CString *retsName = NULL);
		virtual CStationType *GetEncounterInfo (void) { return m_pType; }
		virtual const CString &GetGlobalData (const CString &sAttribute) { return m_pType->GetGlobalData(sAttribute); }
		virtual const CObjectImageArray &GetImage (void) { return m_pType->GetImage(m_ImageSelector, 0); }
		virtual int GetLevel (void) const { return m_pType->GetLevel(); }
		virtual const COrbit *GetMapOrbit (void) { return m_pMapOrbit; }
		virtual Metric GetMass (void) { return m_rMass; }
		virtual CString GetName (DWORD *retdwFlags = NULL);
		virtual CVector GetNearestDockVector (CSpaceObject *pRequestingObj);
		virtual CString GetObjClassName (void) { return CONSTLIT("CStation"); }
		virtual int GetOpenDockingPortCount (void) { return m_DockingPorts.GetPortCount(this) - m_DockingPorts.GetPortsInUseCount(this); }
		virtual CEnergyField *GetOverlay (DWORD dwID) const { return m_Overlays.GetOverlay(dwID); }
		virtual const CString &GetOverlayData (DWORD dwID, const CString &sAttrib) { return m_Overlays.GetData(dwID, sAttrib); }
		virtual void GetOverlayList (TArray<CEnergyField *> &List) { m_Overlays.GetList(List); }
		virtual CVector GetOverlayPos (DWORD dwID) { return m_Overlays.GetPos(this, dwID); }
		virtual int GetOverlayRotation (DWORD dwID) { return m_Overlays.GetRotation(dwID); }
		virtual CEnergyFieldType *GetOverlayType (DWORD dwID) { return m_Overlays.GetType(dwID); }
		virtual CSystem::LayerEnum GetPaintLayer (void);
		virtual Metric GetParallaxDist (void) { return m_pType->GetParallaxDist(); }
		virtual ICCItem *GetProperty (const CString &sName);
		virtual IShipGenerator *GetRandomEncounterTable (int *retiFrequency = NULL) const;
		virtual ScaleTypes GetScale (void) const { return m_Scale; }
		virtual CXMLElement *GetScreen (const CString &sName) { return m_pType->GetScreen(sName); }
		virtual int GetSellPrice (const CItem &Item, bool bNoInventoryCheck = false);
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual COLORREF GetSpaceColor (void) { return m_pType->GetSpaceColor(); }
		virtual CString GetStargateID (void) const;
		virtual int GetStealth (void) const { return ((m_fKnown && !IsMobile()) ? stealthMin : m_pType->GetStealth()); }
		virtual CDesignType *GetType (void) const { return m_pType; }
		virtual int GetVisibleDamage (void);
		virtual CDesignType *GetWreckType (void) const;
		virtual bool HasAttribute (const CString &sAttribute) const;
		virtual bool HasMapLabel (void);
		virtual bool ImageInObject (const CVector &vObjPos, const CObjectImageArray &Image, int iTick, int iRotation, const CVector &vImagePos);
		virtual bool IsAbandoned (void) const { return (m_iHitPoints == 0 && !IsImmutable()); }
		virtual bool IsActiveStargate (void) const { return !m_sStargateDestNode.IsBlank() && m_fActive; }
		virtual bool IsAngryAt (CSpaceObject *pObj) { return (IsEnemy(pObj) || IsBlacklisted(pObj)); }
		virtual bool IsBackgroundObj (void) { return m_pType->IsBackgroundObject(); }
		virtual bool IsKnown (void) { return m_fKnown; }
		virtual bool IsObjDocked (CSpaceObject *pObj) { return m_DockingPorts.IsObjDocked(pObj); }
		virtual bool IsObjDockedOrDocking (CSpaceObject *pObj) { return m_DockingPorts.IsObjDockedOrDocking(pObj); }
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
		virtual void PaintMap (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);
		virtual void PlaceAtRandomDockPort (CSpaceObject *pObj) { m_DockingPorts.DockAtRandomPort(this, pObj); }
		virtual bool PointInObject (const CVector &vObjPos, const CVector &vPointPos);
		virtual bool PointInObject (SPointInObjectCtx &Ctx, const CVector &vObjPos, const CVector &vPointPos);
		virtual void PointInObjectInit (SPointInObjectCtx &Ctx);
		virtual void RemoveOverlay (DWORD dwID) { m_Overlays.RemoveField(this, dwID); }
		virtual bool RemoveSubordinate (CSpaceObject *pSubordinate);
		virtual bool RequestDock (CSpaceObject *pObj);
		virtual bool RequestGate (CSpaceObject *pObj);
		virtual void SetEventFlags (void);
		virtual void SetGlobalData (const CString &sAttribute, const CString &sData) { m_pType->SetGlobalData(sAttribute, sData); }
		virtual void SetKnown (bool bKnown = true) { m_fKnown = bKnown; }
		virtual void SetMapLabelPos (int x, int y) { m_xMapLabel = x; m_yMapLabel = y; }
		virtual void SetName (const CString &sName, DWORD dwFlags = 0);
		virtual void SetOverlayData (DWORD dwID, const CString &sAttribute, const CString &sData) { m_Overlays.SetData(dwID, sAttribute, sData); }
		virtual void SetOverlayPos (DWORD dwID, const CVector &vPos) { m_Overlays.SetPos(this, dwID, vPos); }
		virtual void SetOverlayRotation (DWORD dwID, int iRotation) { m_Overlays.SetRotation(dwID, iRotation); }
		virtual bool SetProperty (const CString &sName, ICCItem *pValue, CString *retsError);
		virtual void SetSovereign (CSovereign *pSovereign) { m_pSovereign = pSovereign; }
		virtual void SetTradeDesc (CEconomyType *pCurrency, int iMaxCurrency, int iReplenishCurrency);
		virtual bool SupportsGating (void) { return IsActiveStargate(); }
		virtual void Undock (CSpaceObject *pObj);

	protected:

		//	CSpaceObject virtuals
		virtual bool CanFireOn (CSpaceObject *pObj) { return CanFireOnObjHelper(pObj); }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);
		virtual EDamageResults OnDamage (SDamageCtx &Ctx);
		virtual void OnObjEnteredGate (CSpaceObject *pObj, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
		virtual void OnUpdateExtended (const CTimeSpan &ExtraTime);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum Constants
			{
			maxRegisteredObjects	= 12,
			maxDevices				= 8,
			};

		CStation (void);

		void AllocTradeOverride (void);
		void Blacklist (CSpaceObject *pObj);
		int CalcNumberOfShips (void);
		void CreateDestructionEffect (void);
		void CreateEjectaFromDamage (int iDamage, const CVector &vHitPos, int iDirection, const DamageDesc &Damage);
		void CreateStructuralDestructionEffect (SDestroyCtx &Ctx);
		ALERROR CreateMapImage (void);
		void FinishCreation (void);
		void FriendlyFire (CSpaceObject *pAttacker);
		const CObjectImageArray &GetImage (int *retiTick, int *retiRotation);
		bool IsBlacklisted (CSpaceObject *pObj);
		inline bool IsImmutable (void) const { return m_fImmutable; }
		void RaiseAlert (CSpaceObject *pTarget);
		void SetAngry (void);
		inline bool ShowWreckImage (void) { return (IsAbandoned() && m_iMaxHitPoints > 0); }
		void UpdateAttacking (int iTick);
		void UpdateReinforcements (int iTick);

		CStationType *m_pType;					//	Station type
		CString m_sName;						//	Station name
		DWORD m_dwNameFlags;					//	Name flags
		CSovereign *m_pSovereign;				//	Allegiance
		ScaleTypes m_Scale;						//	Scale of station
		Metric m_rMass;							//	Mass of station

		CCompositeImageSelector m_ImageSelector;//	Image variant to display
		int m_iDestroyedAnimation;				//	Frames left of destroyed animation
		COrbit *m_pMapOrbit;					//	Orbit to draw on map
		int m_xMapLabel;						//	Name label in map view
		int m_yMapLabel;

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
		DWORD m_fSpare2:1;
		DWORD m_fSpare3:1;
		DWORD m_fSpare4:1;
		DWORD m_fSpare5:1;
		DWORD m_fSpare6:1;
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

