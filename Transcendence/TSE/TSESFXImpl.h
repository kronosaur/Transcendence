//	TSESFXImpl.h
//
//	Transcendence IEffectPainter classes

#ifndef INCL_TSE_SFX
#define INCL_TSE_SFX

class CBeamEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		static CString GetClassTag (void) { return CONSTLIT("Beam"); }
		static BeamTypes ParseBeamType (const CString &sValue);

		virtual CString GetTag (void) { return GetClassTag(); }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void) { return this; }
		virtual int GetLifetime (void) { return 0; }

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual void GetRect (RECT *retRect) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void PaintHit (CG16bitImage &Dest, int x, int y, const CVector &vHitPos, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0) const;

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		struct SLineDesc
			{
			int xFrom;
			int yFrom;
			int xTo;
			int yTo;
			};

		void DrawBeam (CG16bitImage &Dest, SLineDesc &Line, SViewportPaintCtx &Ctx);
		void DrawBeamBlaster (CG16bitImage &Dest, SLineDesc &Line, SViewportPaintCtx &Ctx);
		void DrawBeamHeavyBlaster (CG16bitImage &Dest, SLineDesc &Line, SViewportPaintCtx &Ctx);
		void DrawBeamLaser (CG16bitImage &Dest, SLineDesc &Line, SViewportPaintCtx &Ctx);
		void DrawBeamLightning (CG16bitImage &Dest, SLineDesc &Line, SViewportPaintCtx &Ctx);
		void DrawBeamParticle (CG16bitImage &Dest, SLineDesc &Line, SViewportPaintCtx &Ctx);
		void DrawBeamStarBlaster (CG16bitImage &Dest, SLineDesc &Line, SViewportPaintCtx &Ctx);

		BeamTypes m_iType;
		int m_iIntensity;
		WORD m_wPrimaryColor;
		WORD m_wSecondaryColor;
	};

class CBoltEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		static CString GetClassTag (void) { return CONSTLIT("Bolt"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void) { return this; }
		virtual int GetLifetime (void) { return 0; }

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual void GetRect (RECT *retRect) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0) const;

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		int m_iLength;
		int m_iWidth;
		WORD m_wPrimaryColor;
		WORD m_wSecondaryColor;
	};

class CEffectGroupCreator : public CEffectCreator
	{
	public:
		CEffectGroupCreator (void);

		void ApplyOffsets (SViewportPaintCtx *ioCtx, int *retx, int *rety);
		inline int GetCount (void) { return m_iCount; }
		inline CEffectCreator *GetCreator (int iIndex) { return m_pCreators[iIndex]; }
		static CString GetClassTag (void) { return CONSTLIT("Group"); }
		virtual CString GetTag (void) { return GetClassTag(); }
		inline bool HasOffsets (void) { return m_bHasOffsets; }

		//	Virtuals

		virtual ~CEffectGroupCreator (void);
		virtual ALERROR CreateEffect (CSystem *pSystem,
									  CSpaceObject *pAnchor,
									  const CVector &vPos,
									  const CVector &vVel,
									  int iRotation);
		virtual IEffectPainter *CreatePainter (void);
		virtual int GetLifetime (void);
		virtual void LoadImages (void);
		virtual void MarkImages (void);
		virtual void SetLifetime (int iLifetime);
		virtual void SetVariants (int iVariants);

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		int m_iCount;
		CEffectCreator **m_pCreators;

		bool m_bHasOffsets;
		int m_xOffset;						//	Cartessian coords (pixels)
		int m_yOffset;
		int m_iAngleOffset;
		int m_iRadiusOffset;				//	Pixels
		int m_iRotationAdj;
	};

class CEffectSequencerCreator : public CEffectCreator
	{
	public:
		enum TimeTypes
			{
			timePast,
			timeNow,
			timeFuture,
			};

		virtual ~CEffectSequencerCreator (void);

		static CString GetClassTag (void) { return CONSTLIT("Sequencer"); }
		inline int GetCount (void) const { return m_Timeline.GetCount(); }
		inline CEffectCreator *GetCreator (int iIndex) { return m_Timeline[iIndex].pCreator; }
		virtual CString GetTag (void) { return GetClassTag(); }
		TimeTypes GetTime (int iIndex, int iStart, int iEnd = -1);
		inline bool IsTime (int iIndex, int iStart, int iEnd) { return (iStart <= m_Timeline[iIndex].iTime) && (m_Timeline[iIndex].iTime <= iEnd); }

		//	CEffectCreator virtuals
		virtual ALERROR CreateEffect (CSystem *pSystem,
									  CSpaceObject *pAnchor,
									  const CVector &vPos,
									  const CVector &vVel,
									  int iRotation);
		virtual IEffectPainter *CreatePainter (void) { ASSERT(false); return NULL; }
		virtual int GetLifetime (void);
		virtual void LoadImages (void);
		virtual void MarkImages (void);

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		struct SEntry
			{
			int iTime;
			CEffectCreator *pCreator;
			};

		TArray<SEntry> m_Timeline;
	};

class CEllipseEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		static CString GetClassTag (void) { return CONSTLIT("Ellipse"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void) { return this; }
		virtual int GetLifetime (void) { return m_iLifetime; }

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual void GetRect (RECT *retRect) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0) const;

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		CVector GetPoint (Metric rAngle) const;

		Metric m_rSemiMajorAxis;
		Metric m_rEccentricity;
		Metric m_rRotation;

		WORD m_wLineColor;
		int m_iLineWidth;
		CString m_sLineStyle;

		int m_iLifetime;
	};

class CFlareEffectCreator : public CEffectCreator
	{
	public:
		enum Styles
			{
			styleFadingBlast,							//	Starts bright and fades out
			};

		static CString GetClassTag (void) { return CONSTLIT("Flare"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		void CreateFlareSpike (int iAngle, int iLength, int iWidth, SPoint *Poly);
		inline int GetLifetime (void) const { return m_iLifetime; }
		inline WORD GetPrimaryColor (void) const { return m_wPrimaryColor; }
		inline int GetRadius (void) const { return m_iRadius; }
		inline Styles GetStyle (void) const { return m_iStyle; }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void);
		virtual int GetLifetime (void) { return m_iLifetime; }
		virtual void SetLifetime (int iLifetime) { m_iLifetime = iLifetime; }

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		Styles m_iStyle;
		int m_iRadius;									//	Radius in pixels
		int m_iLifetime;								//	Lifetime (ticks)
		WORD m_wPrimaryColor;
	};

class CImageEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		static CString GetClassTag (void) { return CONSTLIT("Image"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		inline CCompositeImageDesc &GetImage (void) { return m_Image; }
		inline bool HasRandomStartFrame (void) const { return m_bRandomStartFrame; }
		inline bool ImageRotationNeeded (void) const { return m_bRotateImage; }
		inline bool IsDirectional (void) const { return m_bDirectional; }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void);
		virtual int GetLifetime (void) { return m_iLifetime; }
		virtual void LoadImages (void) { m_Image.LoadImage(); }
		virtual void MarkImages (void) { m_Image.MarkImage(); }
		virtual void SetVariants (int iVariants);

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual bool GetParticlePaintDesc (SParticlePaintDesc *retDesc);
		virtual void GetRect (RECT *retRect) const;
		virtual int GetVariants (void) const { return m_iVariants; }
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0) const;

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		CCompositeImageDesc m_Image;
		int m_iLifetime;
		int m_iVariants;
		bool m_bRotateImage;
		bool m_bRandomStartFrame;
		bool m_bDirectional;
	};

class CImageAndTailEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		static CString GetClassTag (void) { return CONSTLIT("ImageAndTail"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void) { return this; }
		virtual int GetLifetime (void) { return m_iLifetime; }
		virtual void LoadImages (void) { m_Image.LoadImage(); }
		virtual void MarkImages (void) { m_Image.MarkImage(); }
		virtual void SetVariants (int iVariants) { m_iVariants = iVariants; }

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual void GetRect (RECT *retRect) const { *retRect = m_Image.GetImageRect(); }
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0) const;

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		CObjectImageArray m_Image;
		int m_iLifetime;
		int m_iVariants;
		bool m_bRandomStartFrame;

		int m_iLength;
		int m_iWidth;
		WORD m_wPrimaryColor;
		WORD m_wSecondaryColor;
	};

class CImageFractureEffectCreator : public CEffectCreator
	{
	public:
		static CString GetClassTag (void) { return CONSTLIT("ImageFracture"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		//	CEffectCreator virtuals
		virtual ALERROR CreateEffect (CSystem *pSystem,
									  CSpaceObject *pAnchor,
									  const CVector &vPos,
									  const CVector &vVel,
									  int iRotation);
		virtual IEffectPainter *CreatePainter (void) { ASSERT(false); return NULL; }
		virtual void LoadImages (void) { m_Image.LoadImage(); }
		virtual void MarkImages (void) { m_Image.MarkImage(); }

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		CObjectImageArray m_Image;						//	Images
	};

class CMoltenBoltEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		static CString GetClassTag (void) { return CONSTLIT("MoltenBolt"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void) { return this; }
		virtual int GetLifetime (void) { return 0; }

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual void GetRect (RECT *retRect) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0) const;

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		void CreateBoltShape (int iAngle, int iLength, int iWidth, SPoint *Poly);

		int m_iWidth;
		int m_iLength;
		int m_iGrowth;
		WORD m_wPrimaryColor;
		WORD m_wSecondaryColor;
	};

class CNullEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		static CString GetClassTag (void) { return CONSTLIT("Null"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void) { return this; }
		virtual int GetLifetime (void) { return 0; }

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) { }
	};

class CParticleCloudEffectCreator : public CEffectCreator
	{
	public:
		static CString GetClassTag (void) { return CONSTLIT("ParticleCloud"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		inline int GetCohesion (void) const { return m_iCohesion; }
		inline int GetEmitLifetime (void) const { return m_iEmitLifetime; }
		Metric GetEmitSpeed (void) const;
		inline Metric GetMaxRadius (void) const { return m_rMaxRadius; }
		inline Metric GetMinRadius (void) const { return m_rMinRadius; }
		inline int GetNewParticleCount (void) const { return m_NewParticles.Roll(); }
		inline int GetNewParticleMax (void) const { return m_NewParticles.GetMaxValue(); }
		inline int GetParticleCount (void) const { return m_ParticleCount.Roll(); }
		inline int GetParticleCountMax (void) const { return m_ParticleCount.GetMaxValue(); }
		inline CEffectCreator *GetParticleEffect (void) const { return m_pParticleEffect; }
		inline int GetParticleLifetime (void) const { return m_ParticleLifetime.Roll(); }
		inline int GetParticleLifetimeMax (void) const { return Max(1, m_ParticleLifetime.GetMaxValue()); }
		inline Metric GetRingRadius (void) const { return m_rRingRadius; }
		inline int GetViscosity (void) const { return m_iViscosity; }
		inline int GetWakePotential (void) const { return m_iWakePotential; }

		//	CEffectCreator virtuals
		virtual ~CParticleCloudEffectCreator (void);
		virtual IEffectPainter *CreatePainter (void);
		virtual int GetLifetime (void) { return m_Lifetime.Roll(); }
		virtual void SetLifetime (int iLifetime) { m_Lifetime.SetConstant(iLifetime); }

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		DiceRange m_Lifetime;							//	Lifetime of effect
		DiceRange m_ParticleCount;						//	Initial number of particles
		DiceRange m_ParticleLifetime;					//	Lifetime of each particle

		int m_iEmitLifetime;							//	% time that it emits particles
		DiceRange m_NewParticles;						//	Number of new particles per tick
		DiceRange m_InitSpeed;							//	Initial speed of each particle

		Metric m_rRingRadius;							//	If non-zero, particles form a ring at this radius
		Metric m_rMaxRadius;							//	If RingRadius is non-zero, this is the outer edge of ring
														//		Otherwise, it is the max radius of a sphere
		Metric m_rMinRadius;							//	If RingRadius is non-zero, this is the inner edge of ring
														//		Otherwise, it is ignored

		int m_iCohesion;								//	Strength of force keeping particles together (0-100)
		int m_iViscosity;								//	Drag on particles while inside bounds (0-100)
		int m_iWakePotential;							//	Influence of moving objects (0-100)

		CEffectCreator *m_pParticleEffect;				//	Effect to use to paint particles
	};

class CParticleCometEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		CParticleCometEffectCreator (void);
		virtual ~CParticleCometEffectCreator (void);
		static CString GetClassTag (void) { return CONSTLIT("ParticleComet"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		CVector GetParticlePos (int iParticle, int iTick, int iDirection, int *retiAge = NULL);
		inline int GetParticleCount (void) { return m_iParticleCount; }
		inline WORD GetPrimaryColor (void) { return m_wPrimaryColor; }
		inline int GetMaxAge (void) { return POINT_COUNT-1; }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void) { return this; }
		virtual int GetLifetime (void) { return -1; }

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual void GetRect (RECT *retRect) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		enum Constants
			{
			POINT_COUNT = 100,
			};

		struct SParticle
			{
			int iPos;
			CVector vScale;
			};

		void ComputeSplinePoints (void);
		void CreateParticles (void);

		int m_iParticleCount;
		int m_iWidth;
		int m_iLength;
		WORD m_wPrimaryColor;

		CVector *m_vPoints;
		SParticle *m_Particles;
	};

class CParticleExplosionEffectCreator : public CEffectCreator
	{
	public:
		static CString GetClassTag (void) { return CONSTLIT("ParticleExplosion"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		//	CEffectCreator virtuals
		virtual ALERROR CreateEffect (CSystem *pSystem,
									  CSpaceObject *pAnchor,
									  const CVector &vPos,
									  const CVector &vVel,
									  int iRotation);
		virtual IEffectPainter *CreatePainter (void) { ASSERT(false); return NULL; }
		virtual void LoadImages (void) { m_Image.LoadImage(); }
		virtual void MarkImages (void) { m_Image.MarkImage(); }

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		DiceRange m_Lifetime;							//	Total lifetime
		DiceRange m_ParticleCount;						//	Number of particles
		Metric m_rParticleSpeed;						//	Speed of particles
		int m_iParticleLifetime;						//	Particle lifespan
		CObjectImageArray m_Image;						//	Images
	};

class CPlasmaSphereEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		static CString GetClassTag (void) { return CONSTLIT("PlasmaSphere"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void) { return this; }
		virtual int GetLifetime (void) { return 0; }

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual void GetRect (RECT *retRect) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0) const;

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		DiceRange m_Radius;
		DiceRange m_SpikeCount;
		DiceRange m_SpikeLength;
		WORD m_wPrimaryColor;
		WORD m_wSecondaryColor;
	};

class CPolyflashEffectCreator : public CEffectCreator
	{
	public:
		static CString GetClassTag (void) { return CONSTLIT("Polyflash"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void);
		virtual int GetLifetime (void) { return 1; }

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) { return NOERROR; }
	};

class CShapeEffectCreator : public CEffectCreator
	{
	public:
		CShapeEffectCreator (void) : m_Points(NULL), m_TransBuffer(NULL) { }
		static CString GetClassTag (void) { return CONSTLIT("Shape"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		void CreateShapeRegion (int iAngle, int iLength, int iWidth, CG16bitRegion *pRegion);
		inline WORD GetColor (void) const { return m_wColor; }
		inline int GetLength (void) const { return m_iLength; }
		inline int GetLengthInc (void) const { return m_iLengthInc; }
		inline DWORD GetOpacity (void) const { return m_byOpacity; }
		inline int GetWidth (void) const { return m_iWidth; }
		inline int GetWidthInc (void) const { return m_iWidthInc; }
		inline bool IsDirectional (void) const { return m_bDirectional; }

		//	CEffectCreator virtuals
		virtual ~CShapeEffectCreator (void);
		virtual IEffectPainter *CreatePainter (void);
		virtual int GetLifetime (void) { return 0; }

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		int m_iWidth;								//	Scale factor: pixel width of 100 units
		int m_iLength;								//	Scale factor: pixel length of 100 units

		bool m_bDirectional;						//	If TRUE, rotate shape based on Ctx.iRotation
		int m_iWidthInc;							//	Increment width every tick
		int m_iLengthInc;							//	Increment length every tick

		WORD m_wColor;								//	Shape color
		DWORD m_byOpacity;							//	Shape opacity

		int m_iPointCount;
		SPoint *m_Points;
		bool m_bConvexPolygon;						//	TRUE if simple polygon

		SPoint *m_TransBuffer;						//	Buffer used for transforms
	};

class CShockwaveEffectCreator : public CEffectCreator
	{
	public:
		enum Styles
			{
			styleImage,									//	Use an image to paint shockwave
			styleGlowRing,								//	Glowing right
			};

		CShockwaveEffectCreator (void);
		~CShockwaveEffectCreator (void);
		static CString GetClassTag (void) { return CONSTLIT("Shockwave"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		inline WORD *GetColorGradient (void) { return m_wColorGradient; }
		inline int GetFadeStart (void) { return m_iFadeStart; }
		inline const CObjectImageArray &GetImage (void) { return m_Image; }
		inline DWORD *GetOpacityGradient (void) { return m_byOpacityGradient; }
		inline WORD GetPrimaryColor (void) const { return m_wPrimaryColor; }
		inline WORD GetSecondaryColor (void) const { return m_wSecondaryColor; }
		inline int GetRingThickness (void) { return m_iGradientCount; }
		inline int GetSpeed (void) { return Max(1, (int)((m_Speed.Roll() * LIGHT_SPEED * g_SecondsPerUpdate / (g_KlicksPerPixel * 100.0)) + 0.5)); }
		inline Styles GetStyle (void) const { return m_iStyle; }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void);
		virtual int GetLifetime (void) { return m_iLifetime; }
		virtual void LoadImages (void) { m_Image.LoadImage(); }
		virtual void MarkImages (void) { m_Image.MarkImage(); }
		virtual void SetLifetime (int iLifetime) { m_iLifetime = iLifetime; }

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		void CreateGlowGradient (int iSolidWidth, int iGlowWidth, WORD wSolidColor, WORD wGlowColor);

		Styles m_iStyle;								//	Style of effect
		CObjectImageArray m_Image;						//	Shockwave image
		DiceRange m_Speed;								//	Expansion speed (% of lightspeed)
		int m_iLifetime;								//	Lifetime (ticks)
		int m_iFadeStart;								//	% of lifetime at which we start to fade

		int m_iWidth;									//	Width of central ring
		int m_iGlowWidth;								//	Glow width

		WORD m_wPrimaryColor;
		WORD m_wSecondaryColor;

		int m_iGradientCount;
		WORD *m_wColorGradient;
		DWORD *m_byOpacityGradient;
	};

class CSingleParticleEffectCreator : public CEffectCreator
	{
	public:
		static CString GetClassTag (void) { return CONSTLIT("Particle"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		inline ParticlePaintStyles GetStyle (void) const { return m_iStyle; }
		inline int GetMaxWidth (void) const { return m_MaxWidth.Roll(); }
		inline int GetMinWidth (void) const { return m_MinWidth.Roll(); }
		inline WORD GetPrimaryColor (void) const { return m_wPrimaryColor; }
		inline WORD GetSecondaryColor (void) const { return m_wSecondaryColor; }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void);

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		ParticlePaintStyles m_iStyle;
		DiceRange m_MinWidth;
		DiceRange m_MaxWidth;
		WORD m_wPrimaryColor;
		WORD m_wSecondaryColor;
	};

class CSmokeTrailEffectCreator : public CEffectCreator
	{
	public:
		static CString GetClassTag (void) { return CONSTLIT("SmokeTrail"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		inline int GetEmitLifetime (void) const { return m_iEmitLifetime; }
		Metric GetEmitSpeed (void) const;
		inline int GetNewParticleCount (void) const { return m_NewParticles.Roll(); }
		inline int GetNewParticleMax (void) const { return m_NewParticles.GetMaxValue(); }
		inline CEffectCreator *GetParticleEffect (void) const { return m_pParticleEffect; }
		inline int GetParticleLifetime (void) const { return Max(1, m_ParticleLifetime.Roll()); }
		inline int GetParticleLifetimeMax (void) const { return Max(1, m_ParticleLifetime.GetMaxValue()); }
		inline int GetRotation (void) const { return m_iRotation; }
		inline int GetSpread (void) const { return m_Spread.Roll(); }

		//	CEffectCreator virtuals
		virtual ~CSmokeTrailEffectCreator (void);
		virtual IEffectPainter *CreatePainter (void);
		virtual int GetLifetime (void) { return m_iLifetime; }
		virtual void SetLifetime (int iLifetime) { m_iLifetime = iLifetime; }

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		DiceRange m_Spread;								//	Random spread as % of particle speed (per particle)
		DiceRange m_NewParticles;						//	Number of new particles per tick
		DiceRange m_InitSpeed;							//	Initial speed of each particle
		DiceRange m_ParticleLifetime;					//	Lifetime of each particle

		int m_iLifetime;								//	Lifetime of effect
		int m_iEmitLifetime;							//	% time that it emits particles
		int m_iRotation;								//	Direction of emission (defaults to 180)

		CEffectCreator *m_pParticleEffect;				//	Effect to use to paint particles
	};

class CStarburstEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		static CString GetClassTag (void) { return CONSTLIT("Starburst"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void) { return this; }
		virtual int GetLifetime (void) { return m_iLifetime; }

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual void GetRect (RECT *retRect) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0) const;

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		enum Styles
			{
			stylePlain,									//	Random spikes from center point
			styleMorningStar,							//	Sphere with random triangular spikes
			styleLightningStar,							//	Sphere with tail of lightning
			styleFlare,									//	Constant glow plus spikes
			};

		void CreateDiamondSpike (int iAngle, int iLength, int iWidthAngle, SPoint *Poly);

		Styles m_iStyle;
		DiceRange m_SpikeCount;
		DiceRange m_SpikeLength;
		WORD m_wPrimaryColor;
		WORD m_wSecondaryColor;
		int m_iLifetime;
	};

class CTextEffectCreator : public CEffectCreator
	{
	public:
		static CString GetClassTag (void) { return CONSTLIT("Text"); }
		virtual CString GetTag (void) { return GetClassTag(); }

		inline const CG16bitFont *GetFont (void) { return m_pFont; }
		inline DWORD GetFontFlags (void) { return m_dwAlignment; }
		inline DWORD GetOpacity (void) { return m_byOpacity; }
		inline WORD GetPrimaryColor (void) { return m_wPrimaryColor; }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void);
		virtual int GetLifetime (void) { return m_iLifetime; }

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		CString m_sDefaultText;

		const CG16bitFont *m_pFont;
		WORD m_wPrimaryColor;
		DWORD m_byOpacity;
		DWORD m_dwAlignment;

		int m_iLifetime;
	};

#endif
