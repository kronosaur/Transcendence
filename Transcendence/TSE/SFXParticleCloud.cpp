//	SFXParticleCloud.cpp
//
//	Particle Cloud SFX

#include "PreComp.h"

#define PARTICLE_EFFECT_TAG						CONSTLIT("ParticleEffect")

#define COHESION_ATTRIB							CONSTLIT("cohesion")
#define EMIT_LIFETIME_ATTRIB					CONSTLIT("emitDuration")
#define NEW_PARTICLES_ATTRIB					CONSTLIT("emitRate")
#define EMIT_SPEED_ATTRIB						CONSTLIT("emitSpeed")
#define LIFETIME_ATTRIB							CONSTLIT("lifetime")
#define MAX_RADIUS_ATTRIB						CONSTLIT("maxRadius")
#define PARTICLE_COUNT_ATTRIB					CONSTLIT("particleCount")
#define PARTICLE_LIFETIME_ATTRIB				CONSTLIT("particleLifetime")
#define RING_RADIUS_ATTRIB						CONSTLIT("ringRadius")
#define RING_WIDTH_ATTRIB						CONSTLIT("ringWidth")
#define VISCOSITY_ATTRIB						CONSTLIT("viscosity")
#define WAKE_POTENTIAL_ATTRIB					CONSTLIT("wakePotential")

const int DEFAULT_PARTICLE_COUNT =				20;
const int DEFAULT_PARTICLE_LIFETIME =			10;
const int DEFAULT_MAX_WIDTH =					8;
const int DEFAULT_EMIT_SPEED =					40;

class CParticleCloudPainter : public IEffectPainter
	{
	public:
		CParticleCloudPainter (CParticleCloudEffectCreator *pCreator);

		//	IEffectPainter virtuals
		virtual ~CParticleCloudPainter (void);
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual int GetFadeLifetime (void) { return m_pCreator->GetParticleLifetimeMax(); }
		virtual int GetParticleCount (void) { return m_Particles.GetCount(); }
		virtual void GetRect (RECT *retRect) const;
		virtual void OnBeginFade (void) { m_iEmitLifetime = 0; }
		virtual void OnMove (bool *retbBoundsChanged = NULL);
		virtual void OnUpdate (SEffectUpdateCtx &Ctx);
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void PaintFade (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) { Paint(Dest, x, y, Ctx); }
		virtual void PaintHit (CG16bitImage &Dest, int x, int y, const CVector &vHitPos, SViewportPaintCtx &Ctx) { Paint(Dest, x, y, Ctx); }
		virtual void SetPos (const CVector &vPos) { m_Particles.SetOrigin(vPos); }

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		void CreateInitialParticles (int iCount);
		void CreateNewParticles (int iCount);

		CParticleCloudEffectCreator *m_pCreator;
		CParticleArray m_Particles;
		int m_iLastDirection;
		int m_iEmitLifetime;
		int m_iTick;

		IEffectPainter *m_pParticlePainter;
	};

//	CParticleCloudEffectCreator object

CParticleCloudEffectCreator::~CParticleCloudEffectCreator (void)

//	CParticleCloudEffectCreator destructor

	{
	if (m_pParticleEffect)
		delete m_pParticleEffect;
	}

IEffectPainter *CParticleCloudEffectCreator::CreatePainter (void)

//	CreatePainter
//
//	Creates a new painter

	{
	return new CParticleCloudPainter(this);
	}

Metric CParticleCloudEffectCreator::GetEmitSpeed (void) const

//	GetEmitSpeed
//
//	Returns the emit speed of a particle

	{
	return (m_InitSpeed.Roll() * LIGHT_SPEED / 100.0f);
	}

ALERROR CParticleCloudEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnEffectCreateFromXML
//
//	Initializes from XML

	{
	ALERROR error;

	if (error = m_Lifetime.LoadFromXML(pDesc->GetAttribute(LIFETIME_ATTRIB), -1))
		return error;

	if (error = m_ParticleCount.LoadFromXML(pDesc->GetAttribute(PARTICLE_COUNT_ATTRIB), 0))
		return error;

	if (error = m_ParticleLifetime.LoadFromXML(pDesc->GetAttribute(PARTICLE_LIFETIME_ATTRIB), -1))
		return error;

	if (error = m_NewParticles.LoadFromXML(pDesc->GetAttribute(NEW_PARTICLES_ATTRIB), 0))
		return error;

	if (error = m_InitSpeed.LoadFromXML(pDesc->GetAttribute(EMIT_SPEED_ATTRIB), DEFAULT_EMIT_SPEED))
		return error;

	m_iEmitLifetime = pDesc->GetAttributeIntegerBounded(EMIT_LIFETIME_ATTRIB, 0, -1, 100);
	m_iCohesion = pDesc->GetAttributeIntegerBounded(COHESION_ATTRIB, 0, 100, 0);
	m_iViscosity = pDesc->GetAttributeIntegerBounded(VISCOSITY_ATTRIB, 0, 100, 0);
	m_iWakePotential = pDesc->GetAttributeIntegerBounded(WAKE_POTENTIAL_ATTRIB, 0, 100, 0);

	//	Size

	int iRingRadius = pDesc->GetAttributeIntegerBounded(RING_RADIUS_ATTRIB, 0, -1, 0);
	if (iRingRadius > 0)
		{
		m_rRingRadius = iRingRadius * g_KlicksPerPixel;
		Metric rRingWidth = pDesc->GetAttributeIntegerBounded(RING_WIDTH_ATTRIB, 0, -1, 1) * g_KlicksPerPixel;
		m_rMaxRadius = m_rRingRadius + rRingWidth / 2.0f;
		m_rMinRadius = Max((Metric)0.0f, (Metric)(m_rRingRadius - rRingWidth / 2.0f));
		}
	else
		{
		m_rRingRadius = 0.0;
		m_rMaxRadius = pDesc->GetAttributeIntegerBounded(MAX_RADIUS_ATTRIB, 0, -1, 0) * g_KlicksPerPixel;
		m_rMinRadius = 0.0;
		}

	//	If both initial and new particles are 0, then set better defaults

	if (m_ParticleCount.GetMaxValue() <= 0 && m_NewParticles.GetMaxValue() <= 0)
		m_ParticleCount.SetConstant(DEFAULT_PARTICLE_COUNT);

	//	Load the effect to use for particles

	CXMLElement *pEffect = pDesc->GetContentElementByTag(PARTICLE_EFFECT_TAG);
	if (pEffect)
		{
		if (error = CEffectCreator::CreateFromXML(Ctx, pEffect, NULL_STR, &m_pParticleEffect))
			return error;
		}
	else
		m_pParticleEffect = NULL;

	return NOERROR;
	}

ALERROR CParticleCloudEffectCreator::OnEffectBindDesign (SDesignLoadCtx &Ctx)

//	OnEffectBindDesign
//
//	Resolve loading

	{
	ALERROR error;

	if (m_pParticleEffect)
		{
		if (error = m_pParticleEffect->BindDesign(Ctx))
			return error;
		}

	return NOERROR;
	}

//	CParticleCloudPainter object -----------------------------------------------

CParticleCloudPainter::CParticleCloudPainter (CParticleCloudEffectCreator *pCreator) : 
		m_pCreator(pCreator),
		m_iLastDirection(-1),
		m_iTick(0)

//	CParticleCloudPainter constructor

	{
	//	Get the total lifetime of the cloud (this could be -1, meaning immortal)

	int iTotalLifetime = m_pCreator->GetLifetime();

	//	Figure out how many ticks we will be emitting for. For mortal clouds, this
	//	is the % of total lifetime. For immortal clouds, this is an absolute number of ticks

	m_iEmitLifetime = (iTotalLifetime == -1 ? m_pCreator->GetEmitLifetime() : m_pCreator->GetEmitLifetime() * iTotalLifetime / 100);

	//	Compute the maximum number of particles that we could ever have

	int iNewParticleRate = m_pCreator->GetNewParticleMax();
	int iParticleLifetime = m_pCreator->GetParticleLifetimeMax();

	int iMaxParticleCount = m_pCreator->GetParticleCountMax()
			+ (iParticleLifetime == -1 ? m_iEmitLifetime * iNewParticleRate : iParticleLifetime * iNewParticleRate);

	//	Initialize the array

	m_Particles.Init(iMaxParticleCount);
	CreateInitialParticles(m_pCreator->GetParticleCount());

	//	Initialize the single particle painter

	CEffectCreator *pEffect = pCreator->GetParticleEffect();
	if (pEffect)
		m_pParticlePainter = pEffect->CreatePainter();
	else
		m_pParticlePainter = NULL;
	}

CParticleCloudPainter::~CParticleCloudPainter (void)

//	CParticleCloudPainter destructor

	{
	if (m_pParticlePainter)
		m_pParticlePainter->Delete();
	}

void CParticleCloudPainter::CreateInitialParticles (int iCount)

//	CreateInitialParticles
//
//	Create the initial particles

	{
	int i;

	//	Slightly different algorithms depending on whether we are a ring or not

	if (m_pCreator->GetRingRadius() > 0.0)
		{
		Metric rRadius = m_pCreator->GetRingRadius();
		Metric rOuterRange = m_pCreator->GetMaxRadius() - rRadius;
		Metric rInnerRange = rRadius - m_pCreator->GetMinRadius();
		Metric *pNormalDist = GetDestinyToBellCurveArray();

		for (i = 0; i < iCount; i++)
			{
			//	Generate a destiny, because we use it to set a position on the ring

			int iDestiny = mathRandom(0, g_DestinyRange - 1);

			//	Generate a random position around the ring

			CVector vPos;
			if (pNormalDist[iDestiny] > 0.0)
				vPos = ::PolarToVector(mathRandom(0, 359), rRadius + pNormalDist[iDestiny] * rOuterRange);
			else
				vPos = ::PolarToVector(mathRandom(0, 359), rRadius + pNormalDist[iDestiny] * rInnerRange);

			//	Generate a random velocity

			CVector vVel = ::PolarToVector(mathRandom(0, 359), mathRandom(5, 25) * LIGHT_SPEED / 1000.0f);

			//	Lifetime

			int iLifeLeft = m_pCreator->GetParticleLifetime();

			//	Add the particle

			m_Particles.AddParticle(vPos, vVel, iLifeLeft, -1, iDestiny);
			}
		}
	else
		{
		int iRadius = (int)(m_pCreator->GetMaxRadius() / g_KlicksPerPixel);

		for (i = 0; i < iCount; i++)
			{
			//	Generate a random position

			CVector vPos = ::PolarToVector(mathRandom(0, 359), mathRandom(0, iRadius) * g_KlicksPerPixel);

			//	Generate a random velocity

			CVector vVel = ::PolarToVector(mathRandom(0, 359), mathRandom(5, 25) * LIGHT_SPEED / 1000.0f);

			//	Lifetime

			int iLifeLeft = m_pCreator->GetParticleLifetime();

			//	Add the particle

			m_Particles.AddParticle(vPos, vVel, iLifeLeft);
			}
		}
	}

void CParticleCloudPainter::CreateNewParticles (int iCount)

//	CreateNewParticles
//
//	Create the new particles for a tick

	{
	int i;
	
	for (i = 0; i < iCount; i++)
		{
		//	Position (start at the center)

		CVector vPos;

		//	Generate a random velocity

		CVector vVel = ::PolarToVector(mathRandom(0, 359), mathRandom(5, 25) * LIGHT_SPEED / 1000.0f);

		//	Lifetime

		int iLifeLeft = m_pCreator->GetParticleLifetime();

		//	Add the particle

		m_Particles.AddParticle(vPos, vVel, iLifeLeft);
		}
	}

void CParticleCloudPainter::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the RECT of the effect centered on 0,0

	{
	*retRect = m_Particles.GetBounds();
	}

void CParticleCloudPainter::OnMove (bool *retbBoundsChanged)

//	OnMove
//
//	Move the particles

	{
	//	Update the single-particle painter

	if (m_pParticlePainter)
		m_pParticlePainter->OnMove();

	//	Update particle motion

	m_Particles.UpdateMotionLinear();

	//	Bounds are always changing

	if (retbBoundsChanged)
		*retbBoundsChanged = true;
	}

void CParticleCloudPainter::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Load from stream

	{
	m_Particles.ReadFromStream(Ctx);

	Ctx.pStream->Read((char *)&m_iLastDirection, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iEmitLifetime, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iTick, sizeof(DWORD));

	if (m_pParticlePainter)
		m_pParticlePainter->Delete();

	m_pParticlePainter = CEffectCreator::CreatePainterFromStreamAndCreator(Ctx, m_pCreator->GetParticleEffect());
	}

void CParticleCloudPainter::OnUpdate (SEffectUpdateCtx &Ctx)

//	OnUpdate
//
//	Update the painter

	{
	//	Update the single-particle painter

	if (m_pParticlePainter)
		m_pParticlePainter->OnUpdate();

	//	Update cohesion

	if (m_pCreator->GetCohesion() > 0 || m_pCreator->GetViscosity() > 0)
		{
		if (m_pCreator->GetRingRadius() > 0.0)
			m_Particles.UpdateRingCohesion(m_pCreator->GetRingRadius(), 
					m_pCreator->GetMinRadius(), 
					m_pCreator->GetMaxRadius(), 
					m_pCreator->GetCohesion(),
					m_pCreator->GetViscosity());
		}

	//	If we have a wake potential or if the particles do damage
	//	then we need to hit test against all objects in the system.

	Ctx.pDamageDesc = m_pCreator->GetDamageDesc();
	Ctx.iWakePotential = m_pCreator->GetWakePotential();

	if ((Ctx.pDamageDesc || Ctx.iWakePotential > 0) && Ctx.pSystem)
		{
		//	Update

		m_Particles.Update(Ctx);
		}

	//	Create new particles

	if (m_iEmitLifetime == -1 || m_iTick < m_iEmitLifetime)
		CreateNewParticles(m_pCreator->GetNewParticleCount());

	//	Update

	m_iTick++;
	}

void CParticleCloudPainter::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write to stream

	{
	m_Particles.WriteToStream(pStream);

	pStream->Write((char *)&m_iLastDirection, sizeof(DWORD));
	pStream->Write((char *)&m_iEmitLifetime, sizeof(DWORD));
	pStream->Write((char *)&m_iTick, sizeof(DWORD));

	CEffectCreator::WritePainterToStream(pStream, m_pParticlePainter);
	}

void CParticleCloudPainter::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint

	{
	int iParticleLifetime = m_pCreator->GetParticleLifetimeMax();

	//	Particles move the opposite direction from the shot

	int iTrailDirection = Ctx.iRotation;

	//	If we haven't created any particles yet, do it now

	if (m_iLastDirection == -1)
		{
		CreateNewParticles(m_pCreator->GetNewParticleCount());
		}

	//	Paint with the painter

	if (m_pParticlePainter)
		{
		//	If we can get a paint descriptor, use that because it is faster

		SParticlePaintDesc Desc;
		if (m_pParticlePainter->GetParticlePaintDesc(&Desc))
			{
			Desc.iMaxLifetime = iParticleLifetime;
			m_Particles.Paint(Dest, x, y, Ctx, Desc);
			}

		//	Otherwise, we use the painter for each particle

		else
			m_Particles.Paint(Dest, x, y, Ctx, m_pParticlePainter);
		}

	//	Update

	m_iLastDirection = iTrailDirection;
	}
