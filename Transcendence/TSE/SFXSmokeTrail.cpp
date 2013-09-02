//	SFXSmokeTrail.cpp
//
//	Smoke Trail SFX

#include "PreComp.h"

#define PARTICLE_EFFECT_TAG						CONSTLIT("ParticleEffect")

#define LIFETIME_ATTRIB							CONSTLIT("lifetime")
#define SPREAD_ATTRIB							CONSTLIT("spread")
#define EMIT_LIFETIME_ATTRIB					CONSTLIT("emitDuration")
#define PARTICLE_LIFETIME_ATTRIB				CONSTLIT("particleLifetime")
#define NEW_PARTICLES_ATTRIB					CONSTLIT("emitRate")
#define EMIT_SPEED_ATTRIB						CONSTLIT("emitSpeed")
#define ROTATION_ATTRIB							CONSTLIT("rotation")

const int DEFAULT_PARTICLE_LIFETIME =			10;
const int DEFAULT_NEW_PARTICLES_PER_TICK =		20;
const int DEFAULT_MAX_WIDTH =					8;
const int DEFAULT_EMIT_SPEED =					20;

class CSmokeTrailPainter : public IEffectPainter
	{
	public:
		CSmokeTrailPainter (CCreatePainterCtx &Ctx, CSmokeTrailEffectCreator *pCreator);

		//	IEffectPainter virtuals
		virtual ~CSmokeTrailPainter (void);
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual int GetFadeLifetime (void) { return m_pCreator->GetParticleLifetimeMax(); }
		virtual void GetRect (RECT *retRect) const;
		virtual void OnBeginFade (void) { m_iEmitLifetime = 0; }
		virtual void OnUpdate (SEffectUpdateCtx &Ctx);
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void PaintFade (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) { Paint(Dest, x, y, Ctx); }
		virtual void PaintHit (CG16bitImage &Dest, int x, int y, const CVector &vHitPos, SViewportPaintCtx &Ctx) { Paint(Dest, x, y, Ctx); }

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		void CreateNewParticles (int iCount, int iDirection);

		CSmokeTrailEffectCreator *m_pCreator;
		CParticleArray m_Particles;
		int m_iLastDirection;
		int m_iEmitLifetime;
		int m_iTick;

		IEffectPainter *m_pParticlePainter;
	};

//	CSmokeTrailEffectCreator object

CSmokeTrailEffectCreator::~CSmokeTrailEffectCreator (void)

//	CSmokeTrailEffectCreator destructor

	{
	if (m_pParticleEffect)
		delete m_pParticleEffect;
	}

IEffectPainter *CSmokeTrailEffectCreator::CreatePainter (CCreatePainterCtx &Ctx)

//	CreatePainter
//
//	Creates a new painter

	{
	return new CSmokeTrailPainter(Ctx, this);
	}

Metric CSmokeTrailEffectCreator::GetEmitSpeed (void) const

//	GetEmitSpeed
//
//	Returns the emit speed of a particle

	{
	return (m_InitSpeed.Roll() * LIGHT_SPEED / 100.0f);
	}

ALERROR CSmokeTrailEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Initializes from XML

	{
	ALERROR error;

	//	In previous versions there was a bug in emitSpeed. It was interpreted
	//	as half its actual value. If this is loading from a previous version,
	//	we divide by two.

	bool bEmitSpeedFix = (Ctx.GetAPIVersion() < 3);

	//	Load some stuff

	m_iEmitLifetime = pDesc->GetAttributeIntegerBounded(EMIT_LIFETIME_ATTRIB, 0, -1, 100);
	m_iLifetime = pDesc->GetAttributeIntegerBounded(LIFETIME_ATTRIB, 0, -1, -1);

	if (!pDesc->FindAttributeInteger(ROTATION_ATTRIB, &m_iRotation))
		m_iRotation = 180;

	if (error = m_Spread.LoadFromXML(pDesc->GetAttribute(SPREAD_ATTRIB)))
		return error;

	CString sRange;
	if (pDesc->FindAttribute(PARTICLE_LIFETIME_ATTRIB, &sRange))
		{
		if (error = m_ParticleLifetime.LoadFromXML(sRange))
			return error;
		}
	else
		m_ParticleLifetime = DiceRange(0, 0, DEFAULT_PARTICLE_LIFETIME);

	if (pDesc->FindAttribute(NEW_PARTICLES_ATTRIB, &sRange))
		{
		if (error = m_NewParticles.LoadFromXML(sRange))
			return error;
		}
	else
		m_NewParticles = DiceRange(0, 0, DEFAULT_NEW_PARTICLES_PER_TICK);

	if (pDesc->FindAttribute(EMIT_SPEED_ATTRIB, &sRange))
		{
		if (error = m_InitSpeed.LoadFromXML(sRange))
			return error;

		if (bEmitSpeedFix)
			m_InitSpeed.Scale(0.5);
		}
	else
		m_InitSpeed = DiceRange(0, 0, DEFAULT_EMIT_SPEED);

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

ALERROR CSmokeTrailEffectCreator::OnEffectBindDesign (SDesignLoadCtx &Ctx)

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

//	CSmokeTrailPainter object

CSmokeTrailPainter::CSmokeTrailPainter (CCreatePainterCtx &Ctx, CSmokeTrailEffectCreator *pCreator) : 
		m_pCreator(pCreator),
		m_iLastDirection(-1),
		m_iTick(0)

//	CSmokeTrailPainter constructor

	{
	int iMaxParticleCount = m_pCreator->GetParticleLifetimeMax() * m_pCreator->GetNewParticleMax();

	m_Particles.Init(iMaxParticleCount);

	int iTotalLifetime = m_pCreator->GetLifetime();
	m_iEmitLifetime = (iTotalLifetime == -1 ? -1 : m_pCreator->GetEmitLifetime() * iTotalLifetime / 100);

	CEffectCreator *pEffect = pCreator->GetParticleEffect();
	if (pEffect)
		m_pParticlePainter = pEffect->CreatePainter(Ctx);
	else
		m_pParticlePainter = NULL;
	}

CSmokeTrailPainter::~CSmokeTrailPainter (void)

//	CSmokeTrailPainter destructor

	{
	if (m_pParticlePainter)
		m_pParticlePainter->Delete();
	}

void CSmokeTrailPainter::CreateNewParticles (int iCount, int iDirection)

//	CreateNewParticles
//
//	Create the new particles for a tick

	{
	int i;
	int iAngleRandomness = m_pCreator->GetSpread() / 7;
	
	for (i = 0; i < iCount; i++)
		{
		//	Generate a random velocity for the particle in a cone around the direction

		Metric rInitialSpeed = m_pCreator->GetEmitSpeed();
		CVector vVel = PolarToVector(
				(iDirection + 360 + mathRandom(-iAngleRandomness, iAngleRandomness)),
				rInitialSpeed);

		//	Add a random little kick

		vVel = vVel + PolarToVector(mathRandom(0, 359), rInitialSpeed * m_pCreator->GetSpread() / 100.0);

		//	Position

		CVector vPos = vVel * (mathRandom(0, 100) / 100.0);

		//	Lifetime

		int iLifeLeft = m_pCreator->GetParticleLifetime();

		//	Add the particle

		m_Particles.AddParticle(vPos, vVel, iLifeLeft);
		}
	}

void CSmokeTrailPainter::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the RECT of the effect centered on 0,0

	{
	*retRect = m_Particles.GetBounds();
	}

void CSmokeTrailPainter::OnReadFromStream (SLoadCtx &Ctx)

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

void CSmokeTrailPainter::OnUpdate (SEffectUpdateCtx &Ctx)

//	OnUpdate
//
//	Update the painter

	{
	if (m_iLastDirection != -1)
		{
		//	Update particle motion

		m_Particles.UpdateMotionLinear();

		//	Create new particles (we create the particles after we update
		//	the motion because we start out with some particles)

		if (m_iEmitLifetime == -1 || m_iTick < m_iEmitLifetime)
			{
			CreateNewParticles(m_pCreator->GetNewParticleCount(),
					m_iLastDirection);
			}

		//	Update

		m_iTick++;
		}
	}

void CSmokeTrailPainter::OnWriteToStream (IWriteStream *pStream)

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

void CSmokeTrailPainter::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint

	{
	int iParticleLifetime = m_pCreator->GetParticleLifetimeMax();

	//	Particles move the opposite direction from the shot

	int iTrailDirection = (Ctx.iRotation + m_pCreator->GetRotation()) % 360;

	//	If we haven't created any particles yet, do it now

	if (m_iLastDirection == -1)
		{
		CreateNewParticles(m_pCreator->GetNewParticleCount(), iTrailDirection);
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
