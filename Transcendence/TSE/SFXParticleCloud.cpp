//	SFXParticleCloud.cpp
//
//	Particle Cloud SFX
//
//	LATER: <SmokeTrail> should just be a style of this effect.

#include "PreComp.h"

#define PARTICLE_EFFECT_TAG						CONSTLIT("ParticleEffect")

#define DRAG_ATTRIB								CONSTLIT("drag")
#define COHESION_ATTRIB							CONSTLIT("cohesion")
#define EMIT_LIFETIME_ATTRIB					CONSTLIT("emitDuration")
#define NEW_PARTICLES_ATTRIB					CONSTLIT("emitRate")
#define EMIT_ROTATION_ATTRIB					CONSTLIT("emitRotation")
#define EMIT_SPEED_ATTRIB						CONSTLIT("emitSpeed")
#define LIFETIME_ATTRIB							CONSTLIT("lifetime")
#define PARTICLE_COUNT_ATTRIB					CONSTLIT("particleCount")
#define PARTICLE_LIFETIME_ATTRIB				CONSTLIT("particleLifetime")
#define RADIUS_ATTRIB							CONSTLIT("radius")
#define RING_WIDTH_ATTRIB						CONSTLIT("ringWidth")
#define SLOW_MOTION_ATTRIB						CONSTLIT("slowMotion")
#define SPREAD_ANGLE_ATTRIB						CONSTLIT("spreadAngle")
#define STYLE_ATTRIB							CONSTLIT("style")
#define VISCOSITY_ATTRIB						CONSTLIT("viscosity")
#define WAKE_POTENTIAL_ATTRIB					CONSTLIT("wakePotential")

#define STYLE_CLOUD								CONSTLIT("cloud")
#define STYLE_EXHAUST							CONSTLIT("exhaust")
#define STYLE_JET								CONSTLIT("jet")
#define STYLE_RING								CONSTLIT("ring")
#define STYLE_SPLASH							CONSTLIT("splash")

const int DEFAULT_PARTICLE_COUNT =				20;
const int DEFAULT_PARTICLE_LIFETIME =			10;
const int DEFAULT_MAX_WIDTH =					8;
const int DEFAULT_EMIT_SPEED =					40;

class CParticleCloudPainter : public IEffectPainter
	{
	public:
		CParticleCloudPainter (CCreatePainterCtx &Ctx, CParticleCloudEffectCreator *pCreator);

		//	IEffectPainter virtuals
		virtual ~CParticleCloudPainter (void);
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual int GetFadeLifetime (void) { return m_pCreator->GetParticleLifetimeMax(); }
		virtual int GetParticleCount (void) { return m_Particles.GetCount(); }
		virtual void GetRect (RECT *retRect) const;
		virtual void OnBeginFade (void) { m_iEmitLifetime = 0; }
		virtual void OnMove (SEffectMoveCtx &Ctx, bool *retbBoundsChanged = NULL);
		virtual void OnUpdate (SEffectUpdateCtx &Ctx);
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void PaintFade (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) { bool bOldFade = Ctx.bFade; Ctx.bFade = true; Paint(Dest, x, y, Ctx); Ctx.bFade = bOldFade; }
		virtual void PaintHit (CG16bitImage &Dest, int x, int y, const CVector &vHitPos, SViewportPaintCtx &Ctx) { Paint(Dest, x, y, Ctx); }
		virtual void SetParam (CCreatePainterCtx &Ctx, const CString &sParam, const CEffectParamDesc &Value);
		virtual void SetPos (const CVector &vPos) { m_Particles.SetOrigin(vPos); }

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		void CreateInitialParticles (int iCount, const CVector &vInitialPos);
		void CreateNewParticles (int iCount, const CVector &vInitialPos, const CVector &vInitialVel);
		void InitParticles (const CVector &vInitialPos);
		void SetLifetime (int iLifetime);

		CParticleCloudEffectCreator *m_pCreator;

		CParticleArray m_Particles;
		int m_iLastDirection;
		int m_iEmitLifetime;
		int m_iTick;

		bool m_bUseObjectCenter;

		IEffectPainter *m_pParticlePainter;
	};

//	CParticleCloudEffectCreator object

CParticleCloudEffectCreator::~CParticleCloudEffectCreator (void)

//	CParticleCloudEffectCreator destructor

	{
	if (m_pParticleEffect)
		delete m_pParticleEffect;
	}

IEffectPainter *CParticleCloudEffectCreator::CreatePainter (CCreatePainterCtx &Ctx)

//	CreatePainter
//
//	Creates a new painter

	{
	IEffectPainter *pPainter = new CParticleCloudPainter(Ctx, this);

	//	Honor the lifetime passed in.

	if (Ctx.GetLifetime() != 0)
		{
		CEffectParamDesc Value;
		Value.InitInteger(Ctx.GetLifetime());
		pPainter->SetParam(Ctx, LIFETIME_ATTRIB, Value);
		}

	//	Done

	return pPainter;
	}

Metric CParticleCloudEffectCreator::GetEmitSpeed (void) const

//	GetEmitSpeed
//
//	Returns the emit speed of a particle

	{
	return (m_InitSpeed.Roll() * LIGHT_SPEED / 100.0f);
	}

ALERROR CParticleCloudEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Initializes from XML

	{
	ALERROR error;

	CString sStyle = pDesc->GetAttribute(STYLE_ATTRIB);
	if (strEquals(sStyle, STYLE_EXHAUST))
		m_iStyle = styleExhaust;
	else if (strEquals(sStyle, STYLE_JET))
		m_iStyle = styleJet;
	else if (strEquals(sStyle, STYLE_RING))
		m_iStyle = styleRing;
	else if (strEquals(sStyle, STYLE_SPLASH))
		m_iStyle = styleSplash;
	else if (strEquals(sStyle, STYLE_CLOUD) || sStyle.IsBlank())
		m_iStyle = styleCloud;
	else
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unknown <ParticleCloud> style: %s"), sStyle);
		return ERR_FAIL;
		}

	int iSlowMotion;
	if (pDesc->FindAttributeInteger(SLOW_MOTION_ATTRIB, &iSlowMotion)
			&& iSlowMotion > 0)
		m_rSlowMotionFactor = iSlowMotion / 100.0;
	else
		m_rSlowMotionFactor = 1.0;

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

	if (error = m_Spread.LoadFromXML(pDesc->GetAttribute(SPREAD_ANGLE_ATTRIB), -1))
		return error;

	int iDefaultEmitLifetime = (m_ParticleLifetime.GetAveValue() == -1 ? 100 : -1);
	m_iEmitLifetime = pDesc->GetAttributeIntegerBounded(EMIT_LIFETIME_ATTRIB, 0, -1, iDefaultEmitLifetime);
	m_iEmitRotation = pDesc->GetAttributeInteger(EMIT_ROTATION_ATTRIB);
	m_iCohesion = pDesc->GetAttributeIntegerBounded(COHESION_ATTRIB, 0, 100, 0);
	m_iViscosity = pDesc->GetAttributeIntegerBounded(VISCOSITY_ATTRIB, 0, 100, 0);
	m_iWakePotential = pDesc->GetAttributeIntegerBounded(WAKE_POTENTIAL_ATTRIB, 0, 100, 0);

	int iDrag;
	if (pDesc->FindAttributeInteger(DRAG_ATTRIB, &iDrag))
		m_rDrag = -(Metric)(iDrag / 100.0);
	else
		m_rDrag = -0.5;

	//	Size and shape

	switch (m_iStyle)
		{
		case styleCloud:
			{
			m_rRingRadius = 0.0;
			m_rMaxRadius = pDesc->GetAttributeIntegerBounded(RADIUS_ATTRIB, 0, -1, 64) * g_KlicksPerPixel;
			m_rMinRadius = 0.0;
			break;
			}

		case styleRing:
			{
			m_rRingRadius = pDesc->GetAttributeIntegerBounded(RADIUS_ATTRIB, 1, -1, 64) * g_KlicksPerPixel;
			Metric rRingWidth = pDesc->GetAttributeIntegerBounded(RING_WIDTH_ATTRIB, 0, -1, 1) * g_KlicksPerPixel;
			m_rMaxRadius = m_rRingRadius + rRingWidth / 2.0f;
			m_rMinRadius = Max((Metric)0.0f, (Metric)(m_rRingRadius - rRingWidth / 2.0f));
			break;
			}

		default:
			{
			m_rRingRadius = 0.0;
			m_rMaxRadius = g_InfiniteDistance;
			m_rMinRadius = 0.0;
			}
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

CParticleCloudPainter::CParticleCloudPainter (CCreatePainterCtx &Ctx, CParticleCloudEffectCreator *pCreator) : 
		m_pCreator(pCreator),
		m_iLastDirection(-1),
		m_iTick(0),
		m_bUseObjectCenter(Ctx.UseObjectCenter())

//	CParticleCloudPainter constructor

	{
	//	Get the total lifetime of the cloud (this could be -1, meaning immortal)

	SetLifetime(m_pCreator->GetLifetime());

	//	Initialize the single particle painter

	CEffectCreator *pEffect = m_pCreator->GetParticleEffect();
	if (pEffect)
		m_pParticlePainter = pEffect->CreatePainter(Ctx);
	else
		m_pParticlePainter = NULL;
	}

CParticleCloudPainter::~CParticleCloudPainter (void)

//	CParticleCloudPainter destructor

	{
	if (m_pParticlePainter)
		m_pParticlePainter->Delete();
	}

void CParticleCloudPainter::CreateInitialParticles (int iCount, const CVector &vInitialPos)

//	CreateInitialParticles
//
//	Create the initial particles

	{
	int i;

	//	Slightly different algorithms depending on style

	switch (m_pCreator->GetStyle())
		{
		case CParticleCloudEffectCreator::styleRing:
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
					vPos = vInitialPos + ::PolarToVector(mathRandom(0, 359), rRadius + pNormalDist[iDestiny] * rOuterRange);
				else
					vPos = vInitialPos + ::PolarToVector(mathRandom(0, 359), rRadius + pNormalDist[iDestiny] * rInnerRange);

				//	Generate a random velocity

				int iRotation = mathRandom(0, 359);
				CVector vVel = ::PolarToVector(iRotation, mathRandom(5, 25) * LIGHT_SPEED / 1000.0f);

				//	Lifetime

				int iLifeLeft = m_pCreator->GetParticleLifetime();

				//	Add the particle

				m_Particles.AddParticle(vPos, vVel, iLifeLeft, iRotation, iDestiny);
				}
			break;
			}

		default:
			{
			int iRadius = (int)(m_pCreator->GetMaxRadius() / g_KlicksPerPixel);

			for (i = 0; i < iCount; i++)
				{
				//	Generate a random position

				CVector vPos = vInitialPos + ::PolarToVector(mathRandom(0, 359), mathRandom(0, iRadius) * g_KlicksPerPixel);

				//	Generate a random velocity

				int iRotation = mathRandom(0, 359);
				CVector vVel = ::PolarToVector(iRotation, mathRandom(5, 25) * LIGHT_SPEED / 1000.0f);

				//	Lifetime

				int iLifeLeft = m_pCreator->GetParticleLifetime();

				//	Add the particle

				m_Particles.AddParticle(vPos, vVel, iLifeLeft, iRotation);
				}
			}
		}
	}

void CParticleCloudPainter::CreateNewParticles (int iCount, const CVector &vInitialPos, const CVector &vInitialVel)

//	CreateNewParticles
//
//	Create the new particles for a tick

	{
	int i;
	const Metric rJitterFactor = LIGHT_SPEED / 100000.0;

	switch (m_pCreator->GetStyle())
		{
		case CParticleCloudEffectCreator::styleExhaust:
		case CParticleCloudEffectCreator::styleJet:
			{
			if (m_iLastDirection == -1)
				break;

			//	Compute the standard direction

			int iBaseRotation = m_pCreator->GetEmitRotation() + m_iLastDirection;
			if (m_pCreator->GetStyle() == CParticleCloudEffectCreator::styleExhaust)
				iBaseRotation += 180;

			Metric rBaseRotation = AngleToRadians(iBaseRotation);

			//	Compute the spread angle, in radians

			Metric rSpread;
			int iSpreadAngle = m_pCreator->GetSpreadAngle();
			if (iSpreadAngle >= 0)
				rSpread = AngleToRadians(iSpreadAngle);
			else if (m_pCreator->GetStyle() == CParticleCloudEffectCreator::styleExhaust)
				rSpread = AngleToRadians(4);
			else
				rSpread = AngleToRadians(20);

			Metric rHalfSpread = 0.5 * rSpread;

			//	Create particles

			for (i = 0; i < iCount; i++)
				{
				//	Position

				CVector vPos = vInitialPos;

				//	Generate a random velocity backwards

				Metric rRotation = rBaseRotation + (rHalfSpread * mathRandom(-1000, 1000) / 1000.0);
				CVector vVel = m_pCreator->GetSlowMotionFactor() * (vInitialVel + ::PolarToVectorRadians(rRotation, m_pCreator->GetEmitSpeed() + rJitterFactor * mathRandom(-500, 500)));

				//	Lifetime

				int iLifeLeft = m_pCreator->GetParticleLifetime();

				//	Add the particle

				m_Particles.AddParticle(vPos, vVel, iLifeLeft, AngleToDegrees(rRotation));
				}

			break;
			}

		default:
			{
			for (i = 0; i < iCount; i++)
				{
				//	Position

				CVector vPos = vInitialPos;

				//	Generate a random velocity

				int iRotation = mathRandom(0, 359);
				CVector vVel = m_pCreator->GetSlowMotionFactor() * (vInitialVel + ::PolarToVector(iRotation, m_pCreator->GetEmitSpeed()));

				//	Lifetime

				int iLifeLeft = m_pCreator->GetParticleLifetime();

				//	Add the particle

				m_Particles.AddParticle(vPos, vVel, iLifeLeft, iRotation);
				}
			}
		}
	}

void CParticleCloudPainter::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the RECT of the effect centered on 0,0

	{
	*retRect = m_Particles.GetBounds();
	}

void CParticleCloudPainter::InitParticles (const CVector &vInitialPos)

//	Init
//
//	Make sure particle array is initialized

	{
	if (m_Particles.GetCount() == 0)
		{
		//	Compute the maximum number of particles that we could ever have

		int iNewParticleRate = m_pCreator->GetNewParticleMax();
		int iParticleLifetime = m_pCreator->GetParticleLifetimeMax();

		int iMaxParticleCount = m_pCreator->GetParticleCountMax()
				+ (iParticleLifetime == -1 ? m_iEmitLifetime * iNewParticleRate : iParticleLifetime * iNewParticleRate);

		//	Initialize the array

		m_Particles.Init(iMaxParticleCount);
		CreateInitialParticles(m_pCreator->GetParticleCount(), vInitialPos);
		}
	}

void CParticleCloudPainter::OnMove (SEffectMoveCtx &Ctx, bool *retbBoundsChanged)

//	OnMove
//
//	Move the particles

	{
	//	Make sure particles are initialized

	InitParticles(CVector());

	//	Update the single-particle painter

	if (m_pParticlePainter)
		m_pParticlePainter->OnMove(Ctx);

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
	DWORD dwLoad;

	m_Particles.ReadFromStream(Ctx);

	Ctx.pStream->Read((char *)&m_iLastDirection, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iEmitLifetime, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iTick, sizeof(DWORD));

	if (Ctx.dwVersion >= 98)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_bUseObjectCenter = ((dwLoad & 0x00000001) ? true : false);
		}

	if (m_pParticlePainter)
		m_pParticlePainter->Delete();

	m_pParticlePainter = CEffectCreator::CreatePainterFromStreamAndCreator(Ctx, m_pCreator->GetParticleEffect());
	}

void CParticleCloudPainter::OnUpdate (SEffectUpdateCtx &Ctx)

//	OnUpdate
//
//	Update the painter

	{
	//	Make sure particles are initialized

	InitParticles(Ctx.vEmitPos);

	//	Update the single-particle painter

	if (m_pParticlePainter)
		m_pParticlePainter->OnUpdate(Ctx);

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

	if (!Ctx.bFade
			&& (m_iEmitLifetime == -1 || m_iTick < m_iEmitLifetime))
		{
		CVector vVel = (Ctx.pObj ? m_pCreator->GetDrag() * Ctx.pObj->GetVel() : CVector());
		CreateNewParticles(m_pCreator->GetNewParticleCount(), Ctx.vEmitPos, vVel);
		}

	//	Update

	m_iTick++;
	}

void CParticleCloudPainter::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write to stream

	{
	DWORD dwSave = 0;

	m_Particles.WriteToStream(pStream);

	pStream->Write((char *)&m_iLastDirection, sizeof(DWORD));
	pStream->Write((char *)&m_iEmitLifetime, sizeof(DWORD));
	pStream->Write((char *)&m_iTick, sizeof(DWORD));

	dwSave |= (m_bUseObjectCenter ? 0x00000001 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

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

	//	If we're using the object center then we paint at the object center.
	//	Otherwise we paint at the given position.

	int xPaint;
	int yPaint;
	if (m_bUseObjectCenter && Ctx.pObj)
		Ctx.XForm.Transform(Ctx.pObj->GetPos(), &xPaint, &yPaint);
	else
		{
		xPaint = x;
		yPaint = y;
		}

	//	If we haven't created any particles yet, do it now

	if (m_iLastDirection == -1
			&& !Ctx.bFade)
		{
		m_iLastDirection = iTrailDirection;

		//	Figure out the position where we create particles

		CVector vPos;

		//	If we're using the object center then it means that x,y is where
		//	we emit particles from. We need to convert from screen coordinates
		//	to object-relative coordinates.

		if (m_bUseObjectCenter)
			vPos = CVector((x - xPaint) * g_KlicksPerPixel, (yPaint - y) * g_KlicksPerPixel);

		//	Initialize

		InitParticles(vPos);

		CVector vVel = (Ctx.pObj ? m_pCreator->GetDrag() * Ctx.pObj->GetVel() : CVector());
		CreateNewParticles(m_pCreator->GetNewParticleCount(), vPos, vVel);
		}

	//	Paint with the painter

	if (m_pParticlePainter)
		{
		//	If we can get a paint descriptor, use that because it is faster

		SParticlePaintDesc Desc;
		if (m_pParticlePainter->GetParticlePaintDesc(&Desc))
			{
			Desc.iMaxLifetime = iParticleLifetime;
			m_Particles.Paint(Dest, xPaint, yPaint, Ctx, Desc);
			}

		//	Otherwise, we use the painter for each particle

		else
			m_Particles.Paint(Dest, xPaint, yPaint, Ctx, m_pParticlePainter);
		}

	//	Update

	m_iLastDirection = iTrailDirection;
	}

void CParticleCloudPainter::SetLifetime (int iLifetime)

//	SetLifetime
//
//	Sets the cloud lifetime

	{
	//	Figure out how many ticks we will be emitting for. For mortal clouds, this
	//	is the % of total lifetime. For immortal clouds, this is an absolute number of ticks

	if (iLifetime == -1)
		m_iEmitLifetime = m_pCreator->GetEmitLifetime();
	else
		m_iEmitLifetime = (m_pCreator->GetEmitLifetime() == -1 ? 100 : m_pCreator->GetEmitLifetime()) * iLifetime / 100;
	}

void CParticleCloudPainter::SetParam (CCreatePainterCtx &Ctx, const CString &sParam, const CEffectParamDesc &Value)

//	SetParam
//
//	Sets parameters

	{
	if (strEquals(sParam, LIFETIME_ATTRIB))
		SetLifetime(Value.EvalInteger(Ctx));
	}
