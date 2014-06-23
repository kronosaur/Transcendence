//	SFXParticleJet.cpp
//
//	CParticleJetEffectCreator class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define PARTICLE_EFFECT_TAG						CONSTLIT("ParticleEffect")

#define CUR_DIRECTION_ATTRIB					CONSTLIT("curDirection")
#define EMIT_RATE_ATTRIB						CONSTLIT("emitRate")
#define EMIT_SPEED_ATTRIB						CONSTLIT("emitSpeed")
#define FIXED_POS_ATTRIB						CONSTLIT("fixedPos")
#define IS_TRACKING_OBJECT_ATTRIB				CONSTLIT("isTrackingObject")
#define PARTICLE_LIFETIME_ATTRIB				CONSTLIT("particleLifetime")
#define SPREAD_ANGLE_ATTRIB						CONSTLIT("spreadAngle")
#define LAST_DIRECTION_ATTRIB					CONSTLIT("lastDirection")
#define LAST_EMIT_POS_ATTRIB					CONSTLIT("lastEmitPos")
#define LIFETIME_ATTRIB							CONSTLIT("lifetime")
#define TANGENT_SPEED_ATTRIB					CONSTLIT("tangentSpeed")
#define USE_OBJECT_CENTER_ATTRIB				CONSTLIT("useObjectCenter")
#define XFORM_ROTATION_ATTRIB					CONSTLIT("xformRotation")
#define XFORM_TIME_ATTRIB						CONSTLIT("xformTime")

class CParticleJetEffectPainter : public IEffectPainter
	{
	public:
		CParticleJetEffectPainter (CCreatePainterCtx &Ctx, CParticleJetEffectCreator *pCreator);
		~CParticleJetEffectPainter (void);

		//	IEffectPainter virtuals
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual int GetFadeLifetime (void) { return m_ParticleLifetime.GetMaxValue(); }
		virtual int GetLifetime (void) { return m_iLifetime; }
		virtual void GetParam (const CString &sParam, CEffectParamDesc *retValue);
		virtual bool GetParamList (TArray<CString> *retList) const;
		virtual int GetParticleCount (void) { return m_Particles.GetCount(); }
		virtual void GetRect (RECT *retRect) const;
		virtual void OnMove (SEffectMoveCtx &Ctx, bool *retbBoundsChanged = NULL);
		virtual void OnUpdate (SEffectUpdateCtx &Ctx);
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void PaintFade (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) { bool bOldFade = Ctx.bFade; Ctx.bFade = true; Paint(Dest, x, y, Ctx); Ctx.bFade = bOldFade; }
		virtual void PaintHit (CG16bitImage &Dest, int x, int y, const CVector &vHitPos, SViewportPaintCtx &Ctx) { Paint(Dest, x, y, Ctx); }
		virtual void SetParam (CCreatePainterCtx &Ctx, const CString &sParam, const CEffectParamDesc &Value);
		virtual void SetPos (const CVector &vPos) { m_Particles.SetOrigin(vPos); }

	private:
		CVector CalcInitialVel (CSpaceObject *pObj);
		void CreateFixedParticles (CSpaceObject *pObj, int iCount, const CVector &vInitialPos, const CVector &vInitialVel);
		void CreateInterpolatedParticles (CSpaceObject *pObj, int iCount, const CVector &vInitialPos, const CVector &vInitialVel);
		void CreateLinearParticles (CSpaceObject *pObj, int iCount, const CVector &vInitialPos, const CVector &vInitialVel);
		void CreateNewParticles (CSpaceObject *pObj, int iCount, const CVector &vInitialPos, const CVector &vInitialVel);
		void InitParticles (const CVector &vInitialPos);

		CParticleJetEffectCreator *m_pCreator;

		DiceRange m_EmitRate;
		DiceRange m_EmitSpeed;
		DiceRange m_ParticleLifetime;
		DiceRange m_TangentSpeed;
		DiceRange m_SpreadAngle;
		
		int m_iLifetime;
		int m_iXformRotation;
		Metric m_rXformTime;
		bool m_bUseObjectCenter;
		bool m_bUseObjectMotion;
		bool m_bTrackingObject;

		int m_iCurDirection;
		CVector m_vLastEmitPos;
		int m_iLastDirection;

		CParticleArray m_Particles;
		IEffectPainter *m_pParticlePainter;
	};

CParticleJetEffectCreator::CParticleJetEffectCreator (void) :
			m_pSingleton(NULL)

//	CRayEffectCreator constructor

	{
	}

CParticleJetEffectCreator::~CParticleJetEffectCreator (void)

//	CRayEffectCreator destructor

	{
	if (m_pSingleton)
		delete m_pSingleton;
	}

IEffectPainter *CParticleJetEffectCreator::CreatePainter (CCreatePainterCtx &Ctx)

//	CreatePainter
//
//	Creates a new painter
	
	{
	//	If we have a singleton, return that

	if (m_pSingleton)
		return m_pSingleton;

	//	Otherwise we need to create a painter with the actual
	//	parameters.

	IEffectPainter *pPainter = new CParticleJetEffectPainter(Ctx, this);

	//	Initialize the painter parameters

	pPainter->SetParam(Ctx, EMIT_RATE_ATTRIB, m_EmitRate);

	pPainter->SetParam(Ctx, FIXED_POS_ATTRIB, m_FixedPos);
	pPainter->SetParam(Ctx, PARTICLE_LIFETIME_ATTRIB, m_ParticleLifetime);
	pPainter->SetParam(Ctx, LIFETIME_ATTRIB, m_Lifetime);
	pPainter->SetParam(Ctx, XFORM_ROTATION_ATTRIB, m_XformRotation);
	pPainter->SetParam(Ctx, XFORM_TIME_ATTRIB, m_XformTime);

	if (!m_FixedPos.EvalBool(Ctx) || m_EmitSpeed.GetType() != CEffectParamDesc::typeNull)
		pPainter->SetParam(Ctx, EMIT_SPEED_ATTRIB, m_EmitSpeed);

	if (m_SpreadAngle.GetType() != CEffectParamDesc::typeNull)
		pPainter->SetParam(Ctx, SPREAD_ANGLE_ATTRIB, m_SpreadAngle);
	else
		pPainter->SetParam(Ctx, TANGENT_SPEED_ATTRIB, m_TangentSpeed);

	//	Initialize via GetParameters, if necessary

	InitPainterParameters(Ctx, pPainter);

	//	If we are a singleton, then we only need to create this once.

	if (GetInstance() == instGame)
		{
		pPainter->SetSingleton(true);
		m_pSingleton = pPainter;
		}

	return pPainter;
	}

ALERROR CParticleJetEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Initialize type-specific data

	{
	ALERROR error;

	m_FixedPos.InitBool(pDesc->GetAttributeBool(FIXED_POS_ATTRIB));

	if (error = m_EmitRate.InitIntegerFromXML(Ctx, pDesc->GetAttribute(EMIT_RATE_ATTRIB)))
		return error;

	if (error = m_EmitSpeed.InitIntegerFromXML(Ctx, pDesc->GetAttribute(EMIT_SPEED_ATTRIB)))
		return error;

	if (error = m_ParticleLifetime.InitIntegerFromXML(Ctx, pDesc->GetAttribute(PARTICLE_LIFETIME_ATTRIB)))
		return error;

	if (error = m_SpreadAngle.InitIntegerFromXML(Ctx, pDesc->GetAttribute(SPREAD_ANGLE_ATTRIB)))
		return error;

	if (error = m_Lifetime.InitIntegerFromXML(Ctx, pDesc->GetAttribute(LIFETIME_ATTRIB)))
		return error;

	if (error = m_TangentSpeed.InitIntegerFromXML(Ctx, pDesc->GetAttribute(TANGENT_SPEED_ATTRIB)))
		return error;

	if (error = m_XformRotation.InitIntegerFromXML(Ctx, pDesc->GetAttribute(XFORM_ROTATION_ATTRIB)))
		return error;

	if (error = m_XformTime.InitIntegerFromXML(Ctx, pDesc->GetAttribute(XFORM_TIME_ATTRIB)))
		return error;

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

ALERROR CParticleJetEffectCreator::OnEffectBindDesign (SDesignLoadCtx &Ctx)

//	OnEffectBindDesign
//
//	Resolve loading

	{
	ALERROR error;

	//	Clean up, because we might want to recompute for next time.

	if (m_pSingleton)
		{
		delete m_pSingleton;
		m_pSingleton = NULL;
		}

	if (m_pParticleEffect)
		{
		if (error = m_pParticleEffect->BindDesign(Ctx))
			return error;
		}

	return NOERROR;
	}

//	CParticleJetEffectPainter --------------------------------------------------

CParticleJetEffectPainter::CParticleJetEffectPainter (CCreatePainterCtx &Ctx, CParticleJetEffectCreator *pCreator) : 
		m_pCreator(pCreator),
		m_iXformRotation(0),
		m_rXformTime(1.0),
		m_iLifetime(0),
		m_iCurDirection(-1),
		m_iLastDirection(-1),
		m_bUseObjectCenter(Ctx.UseObjectCenter()),
		m_bUseObjectMotion(false),
		m_bTrackingObject(Ctx.IsTracking())

//	CParticleJetEffectPainter constructor

	{
	//	Initialize the single particle painter

	CEffectCreator *pEffect = m_pCreator->GetParticleEffect();
	if (pEffect)
		m_pParticlePainter = pEffect->CreatePainter(Ctx);
	else
		m_pParticlePainter = NULL;
	}

CParticleJetEffectPainter::~CParticleJetEffectPainter (void)

//	CParticleJetEffectPainter destructor

	{
	if (m_pParticlePainter)
		m_pParticlePainter->Delete();
	}

CVector CParticleJetEffectPainter::CalcInitialVel (CSpaceObject *pObj)

//	CalcInitialVel
//
//	Calculates the initial particle velocity

	{
#if 0
	return (pObj ? m_pCreator->GetDrag() * pObj->GetVel() : CVector());
#else
	return CVector();
#endif
	}

void CParticleJetEffectPainter::CreateFixedParticles (CSpaceObject *pObj, int iCount, const CVector &vInitialPos, const CVector &vInitialVel)

//	CreateFixedParticles
//
//	Creates particles along the objects path (e.g., missile exhaust).

	{
	int i;

	//	Calculate a vector to our previous position
	//
	//	NOTE: In this mode m_vLastEmitPos is the last position of the object.

	CVector vCurPos = (pObj ? pObj->GetPos() : CVector());
	CVector vToOldPos;
	if (m_bTrackingObject)
		{
		Metric rAveSpeed = m_rXformTime * m_EmitSpeed.GetAveValue() * LIGHT_SPEED / 100.0;
		vToOldPos = m_vLastEmitPos - (vCurPos + vInitialPos) + ::PolarToVector(180 + m_iLastDirection, rAveSpeed * g_SecondsPerUpdate);
		}
	else
		{
		Metric rSpeed = (pObj ? pObj->GetVel().Length() : 0.0);
		vToOldPos = ::PolarToVector(180 + m_iLastDirection, rSpeed * g_SecondsPerUpdate);
		}

	//	Compute two orthogonal coordinates

	CVector vAxis = ::PolarToVector(m_iCurDirection + 180, 1.0);
	CVector vTangent = ::PolarToVector(m_iCurDirection + 90, 1.0);

	//	Create particles

	for (i = 0; i < iCount; i++)
		{
		Metric rSlide = mathRandom(0, 9999) / 10000.0;

		//	Compute a position randomly along the line between the current and
		//	last emit positions.

		CVector vPos = vInitialPos + rSlide * vToOldPos;

		//	Generate a random velocity along the tangent

		Metric rTangentSlide = mathRandom(-9999, 9999) / 10000.0;
		Metric rAxisJitter = mathRandom(-50, 50) / 100.0;
		CVector vVel = (vTangent * rTangentSlide * m_rXformTime * m_TangentSpeed.Roll() * LIGHT_SPEED / 100.0)
				+ (vAxis * (m_EmitSpeed.Roll() + rAxisJitter) * LIGHT_SPEED / 100.0);

		//	Lifetime

		int iLifeLeft = m_ParticleLifetime.Roll();

		//	Add the particle

		m_Particles.AddParticle(vPos, vVel, iLifeLeft, m_iCurDirection);
		}

	//	Remember the last position

	m_iLastDirection = m_iCurDirection;
	m_vLastEmitPos = vCurPos + vInitialPos;
	}

void CParticleJetEffectPainter::CreateInterpolatedParticles (CSpaceObject *pObj, int iCount, const CVector &vInitialPos, const CVector &vInitialVel)

//	CreateInterpolatedParticles
//
//	Creates particles interpolated between to directions.

	{
	int i;

	//	Compute some basic stuff

	const Metric rJitterFactor = LIGHT_SPEED / 100000.0;
	Metric rLastRotation = AngleToRadians(180 + m_iXformRotation + m_iLastDirection);
	Metric rCurRotation = AngleToRadians(180 + m_iXformRotation + m_iCurDirection);

	//	Compute the spread angle, in radians

	Metric rSpread = AngleToRadians(Max(0, m_SpreadAngle.Roll()));
	Metric rHalfSpread = 0.5 * rSpread;

	//	Calculate where last tick's particles would be based on the last rotation.

	Metric rAveSpeed = m_EmitSpeed.GetAveValue() * LIGHT_SPEED / 100.0;
	CVector vLastStart = (m_vLastEmitPos + (m_rXformTime * ::PolarToVectorRadians(rLastRotation, rAveSpeed * g_SecondsPerUpdate))) - vInitialPos;

	//	Calculate where last tick's particles would be IF we have used the current
	//	rotation. This allows us to interpolate a turn.

	CVector vCurStart = (m_vLastEmitPos + (m_rXformTime * ::PolarToVectorRadians(rCurRotation, rAveSpeed * g_SecondsPerUpdate))) - vInitialPos;

	//	Create particles

	for (i = 0; i < iCount; i++)
		{
		Metric rSlide = mathRandom(0, 9999) / 10000.0;

		//	Compute two points along the two slide vectors (last and current)

		CVector vSlide1 = rSlide * vLastStart;
		CVector vSlide2 = rSlide * vCurStart;
		CVector vAdj = (rSlide * vSlide1) + ((1.0 - rSlide) * vSlide2);

		//	We place the particle along the line betwen the current
		//	and last emit positions

		CVector vPos = vInitialPos + vAdj;

		//	We blend the rotation as well

		if (Absolute(rCurRotation - rLastRotation) > g_Pi)
			{
			if (rLastRotation < rCurRotation)
				rLastRotation += g_Pi * 2.0;
			else
				rCurRotation += g_Pi * 2.0;
			}

		Metric rSlideRotation = (rSlide * rLastRotation) + ((1.0 - rSlide) * rCurRotation);

		//	Generate a random velocity backwards

		Metric rRotation = rSlideRotation + (rHalfSpread * mathRandom(-1000, 1000) / 1000.0);
		Metric rSpeed = m_EmitSpeed.Roll() * LIGHT_SPEED / 100.0;
		CVector vVel = m_rXformTime * (vInitialVel + ::PolarToVectorRadians(rRotation, rSpeed + rJitterFactor * mathRandom(-500, 500)));

		//	Lifetime

		int iLifeLeft = m_ParticleLifetime.Roll();

		//	Add the particle

		m_Particles.AddParticle(vPos, vVel, iLifeLeft, AngleToDegrees(rRotation));
		}

	//	Remember the last position

	m_iLastDirection = m_iCurDirection;
	m_vLastEmitPos = vInitialPos;
	}

void CParticleJetEffectPainter::CreateLinearParticles (CSpaceObject *pObj, int iCount, const CVector &vInitialPos, const CVector &vInitialVel)

//	CreateLinearParticles
//
//	Creates new particles on a straight line

	{
	int i;

	//	Compute some basic stuff

	const Metric rJitterFactor = LIGHT_SPEED / 100000.0;
	Metric rCurRotation = AngleToRadians(180 + m_iXformRotation + m_iCurDirection);

	//	Compute the spread angle, in radians

	Metric rSpread = AngleToRadians(Max(0, m_SpreadAngle.Roll()));
	Metric rHalfSpread = 0.5 * rSpread;

	//	Calculate where last tick's particles would be based on the last rotation.

	Metric rAveSpeed = m_EmitSpeed.GetAveValue() * LIGHT_SPEED / 100.0;
	CVector vCurStart = (m_vLastEmitPos + (m_rXformTime * ::PolarToVectorRadians(rCurRotation, rAveSpeed * g_SecondsPerUpdate))) - vInitialPos;

	//	Create particles

	for (i = 0; i < iCount; i++)
		{
		Metric rSlide = mathRandom(0, 9999) / 10000.0;

		//	We place the particle along the line betwen the current
		//	and last emit positions

		CVector vPos = vInitialPos + rSlide * vCurStart;

		//	Generate a random velocity backwards

		Metric rRotation = rCurRotation + (rHalfSpread * mathRandom(-1000, 1000) / 1000.0);
		Metric rSpeed = m_EmitSpeed.Roll() * LIGHT_SPEED / 100.0;
		CVector vVel = m_rXformTime * (vInitialVel + ::PolarToVectorRadians(rRotation, rSpeed + rJitterFactor * mathRandom(-500, 500)));

		//	Lifetime

		int iLifeLeft = m_ParticleLifetime.Roll();

		//	Add the particle

		m_Particles.AddParticle(vPos, vVel, iLifeLeft, AngleToDegrees(rRotation));
		}

	//	Remember the last position

	m_iLastDirection = m_iCurDirection;
	m_vLastEmitPos = vInitialPos;
	}

void CParticleJetEffectPainter::CreateNewParticles (CSpaceObject *pObj, int iCount, const CVector &vInitialPos, const CVector &vInitialVel)

//	CreateNewParticles
//
//	Create the new particles for a tick

	{
	//	Make sure we're initialized

	InitParticles(vInitialPos);

	//	If we haven't yet figured out which direction to emit, then we wait.
	//	(This doesn't happen until paint time).

	if (m_iCurDirection == -1)
		return;

	//	If we using object motion (e.g., for missile exhaust, then have a 
	//	completely different algorithm.

	if (m_bUseObjectMotion)
		CreateFixedParticles(pObj, iCount, vInitialPos, vInitialVel);

	//	If our emit direction has changed then we need to interpolate between the two

	else if (m_iCurDirection != m_iLastDirection)
		CreateInterpolatedParticles(pObj, iCount, vInitialPos, vInitialVel);

	//	Otherwise, just linear creation

	else
		CreateLinearParticles(pObj, iCount, vInitialPos, vInitialVel);
	}

void CParticleJetEffectPainter::GetParam (const CString &sParam, CEffectParamDesc *retValue)

//	GetParam
//
//	Returns the parameter

	{
	if (strEquals(sParam, CUR_DIRECTION_ATTRIB))
		retValue->InitInteger(m_iCurDirection);

	else if (strEquals(sParam, EMIT_RATE_ATTRIB))
		retValue->InitDiceRange(m_EmitRate);

	else if (strEquals(sParam, EMIT_SPEED_ATTRIB))
		retValue->InitDiceRange(m_EmitSpeed);

	else if (strEquals(sParam, FIXED_POS_ATTRIB))
		retValue->InitBool(m_bUseObjectMotion);

	else if (strEquals(sParam, IS_TRACKING_OBJECT_ATTRIB))
		retValue->InitBool(m_bTrackingObject);

	else if (strEquals(sParam, PARTICLE_LIFETIME_ATTRIB))
		retValue->InitDiceRange(m_ParticleLifetime);

	else if (strEquals(sParam, LAST_DIRECTION_ATTRIB))
		retValue->InitInteger(m_iLastDirection);

	else if (strEquals(sParam, LAST_EMIT_POS_ATTRIB))
		retValue->InitVector(m_vLastEmitPos);

	else if (strEquals(sParam, LIFETIME_ATTRIB))
		retValue->InitInteger(m_iLifetime);

	else if (strEquals(sParam, SPREAD_ANGLE_ATTRIB))
		retValue->InitDiceRange(m_SpreadAngle);

	else if (strEquals(sParam, TANGENT_SPEED_ATTRIB))
		retValue->InitDiceRange(m_TangentSpeed);

	else if (strEquals(sParam, USE_OBJECT_CENTER_ATTRIB))
		retValue->InitBool(m_bUseObjectCenter);

	else if (strEquals(sParam, XFORM_ROTATION_ATTRIB))
		retValue->InitInteger(m_iXformRotation);

	else if (strEquals(sParam, XFORM_TIME_ATTRIB))
		retValue->InitInteger((int)(m_rXformTime * 100.0));

	else
		retValue->InitNull();
	}

bool CParticleJetEffectPainter::GetParamList (TArray<CString> *retList) const

//	GetParamList
//
//	Returns a list of value parameter names

	{
	retList->DeleteAll();
	retList->InsertEmpty(14);
	retList->GetAt(0) = CUR_DIRECTION_ATTRIB;
	retList->GetAt(1) = EMIT_RATE_ATTRIB;
	retList->GetAt(2) = EMIT_SPEED_ATTRIB;
	retList->GetAt(3) = FIXED_POS_ATTRIB;
	retList->GetAt(4) = IS_TRACKING_OBJECT_ATTRIB;
	retList->GetAt(5) = PARTICLE_LIFETIME_ATTRIB;
	retList->GetAt(6) = LAST_DIRECTION_ATTRIB;
	retList->GetAt(7) = LAST_EMIT_POS_ATTRIB;
	retList->GetAt(8) = LIFETIME_ATTRIB;
	retList->GetAt(9) = SPREAD_ANGLE_ATTRIB;
	retList->GetAt(10) = TANGENT_SPEED_ATTRIB;
	retList->GetAt(11) = USE_OBJECT_CENTER_ATTRIB;
	retList->GetAt(12) = XFORM_ROTATION_ATTRIB;
	retList->GetAt(13) = XFORM_TIME_ATTRIB;

	return true;
	}

void CParticleJetEffectPainter::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the rect bounds of the image

	{
	*retRect = m_Particles.GetBounds();
	}

void CParticleJetEffectPainter::InitParticles (const CVector &vInitialPos)

//	Init
//
//	Make sure particle array is initialized

	{
	if (m_Particles.GetCount() == 0)
		{
		//	Compute the maximum number of particles that we could ever have

		int iNewParticleRate = m_EmitRate.GetMaxValue();
		int iParticleLifetime = m_ParticleLifetime.GetMaxValue();

		int iMaxParticleCount = Max(0, iParticleLifetime * iNewParticleRate);

		//	Initialize the array

		m_Particles.Init(iMaxParticleCount);
		}
	}

void CParticleJetEffectPainter::OnMove (SEffectMoveCtx &Ctx, bool *retbBoundsChanged)

//	OnMove
//
//	Move the particles

	{
	//	Update the single-particle painter

	if (m_pParticlePainter)
		m_pParticlePainter->OnMove(Ctx);

	//	Update particle motion

	m_Particles.UpdateMotionLinear();

	//	If we're using the object's motion, adjust now

	if (m_bUseObjectMotion && Ctx.pObj)
		{
		CVector vToOldPos;
		if (m_bTrackingObject)
			{
			CVector vCurPos = Ctx.pObj->GetPos();
			vToOldPos = Ctx.vOldPos - vCurPos;
			}
		else
			{
			Metric rSpeed = Ctx.pObj->GetVel().Length();
			vToOldPos = ::PolarToVector(180 + m_iLastDirection, rSpeed * g_SecondsPerUpdate);
			}

		//	Move all particles by the given amount

		m_Particles.Move(vToOldPos);
		}

	//	Bounds are always changing

	if (retbBoundsChanged)
		*retbBoundsChanged = true;
	}

void CParticleJetEffectPainter::OnUpdate (SEffectUpdateCtx &Ctx)

//	OnUpdate
//
//	Update the painter

	{
	//	Update the single-particle painter

	if (m_pParticlePainter)
		m_pParticlePainter->OnUpdate(Ctx);

	//	LATER: Support damage

#if 0
	//	If we have a wake potential or if the particles do damage
	//	then we need to hit test against all objects in the system.

	Ctx.pDamageDesc = m_pCreator->GetDamageDesc();
	Ctx.iWakePotential = m_pCreator->GetWakePotential();

	if ((Ctx.pDamageDesc || Ctx.iWakePotential > 0) && Ctx.pSystem)
		{
		//	Update

		m_Particles.Update(Ctx);
		}
#endif

	//	Create new particles

	if (!Ctx.bFade)
		CreateNewParticles(Ctx.pObj, m_EmitRate.Roll(), Ctx.vEmitPos, CalcInitialVel(Ctx.pObj));
	else if (m_bUseObjectMotion && Ctx.pObj)
		m_vLastEmitPos = Ctx.pObj->GetPos();

	//	If we're fading, reset direction (otherwise, when painting thruster 
	//	effects we'll try to interpolate between stale directions).

	if (Ctx.bFade)
		m_iCurDirection = -1;
	}

void CParticleJetEffectPainter::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint the effect

	{
	int iParticleLifetime = m_ParticleLifetime.GetMaxValue();

	//	Particles move the opposite direction from the shot

	int iTrailDirection = Ctx.iRotation;

	//	If we're using the object center then we paint at the object center.
	//	Otherwise we paint at the given position.

	int xPaint;
	int yPaint;
	if (m_bUseObjectCenter)
		{
		if (Ctx.pObj)
			Ctx.XForm.Transform(Ctx.pObj->GetPos(), &xPaint, &yPaint);
		else
			{
			//	If we don't have an object then we use the viewport center. This
			//	handles the case where we paint in TransData (where there is
			//	no object).

			xPaint = Ctx.xCenter;
			yPaint = Ctx.yCenter;
			}
		}
	else
		{
		xPaint = x;
		yPaint = y;
		}

	//	If we haven't created any particles yet, do it now

	if (m_iCurDirection == -1
			&& !Ctx.bFade)
		{
		m_iLastDirection = iTrailDirection;
		m_iCurDirection = iTrailDirection;

		//	Figure out the position where we create particles

		CVector vPos;

		//	If we're using the object center then it means that x,y is where
		//	we emit particles from. We need to convert from screen coordinates
		//	to object-relative coordinates.

		if (m_bUseObjectCenter)
			vPos = CVector((x - xPaint) * g_KlicksPerPixel, (yPaint - y) * g_KlicksPerPixel);

		//	Initialize last emit position

		m_vLastEmitPos = (m_bUseObjectMotion && Ctx.pObj ? Ctx.pObj->GetPos() + vPos : vPos);

		//	Create particles

		CreateNewParticles(Ctx.pObj, m_EmitRate.Roll(), vPos, CalcInitialVel(Ctx.pObj));
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

	m_iCurDirection = iTrailDirection;
	}

void CParticleJetEffectPainter::SetParam (CCreatePainterCtx &Ctx, const CString &sParam, const CEffectParamDesc &Value)

//	SetParam
//
//	Sets parameters

	{
	if (strEquals(sParam, CUR_DIRECTION_ATTRIB))
		m_iCurDirection = Value.EvalIntegerBounded(Ctx, 0, -1, -1);

	else if (strEquals(sParam, EMIT_RATE_ATTRIB))
		m_EmitRate = Value.EvalDiceRange(Ctx, 10);

	else if (strEquals(sParam, EMIT_SPEED_ATTRIB))
		m_EmitSpeed = Value.EvalDiceRange(Ctx, 50);

	else if (strEquals(sParam, FIXED_POS_ATTRIB))
		m_bUseObjectMotion = Value.EvalBool(Ctx);

	else if (strEquals(sParam, IS_TRACKING_OBJECT_ATTRIB))
		m_bTrackingObject = Value.EvalBool(Ctx);

	else if (strEquals(sParam, PARTICLE_LIFETIME_ATTRIB))
		m_ParticleLifetime = Value.EvalDiceRange(Ctx, 10);

	else if (strEquals(sParam, LAST_DIRECTION_ATTRIB))
		m_iLastDirection = Value.EvalIntegerBounded(Ctx, 0, -1, -1);

	else if (strEquals(sParam, LAST_EMIT_POS_ATTRIB))
		m_vLastEmitPos = Value.EvalVector(Ctx);

	else if (strEquals(sParam, LIFETIME_ATTRIB))
		m_iLifetime = Value.EvalIntegerBounded(Ctx, 0, -1, 0);

	else if (strEquals(sParam, SPREAD_ANGLE_ATTRIB))
		m_SpreadAngle = Value.EvalDiceRange(Ctx, 5);

	else if (strEquals(sParam, TANGENT_SPEED_ATTRIB))
		m_TangentSpeed = Value.EvalDiceRange(Ctx, 0);

	else if (strEquals(sParam, USE_OBJECT_CENTER_ATTRIB))
		m_bUseObjectCenter = Value.EvalBool(Ctx);

	else if (strEquals(sParam, XFORM_ROTATION_ATTRIB))
		m_iXformRotation = Value.EvalIntegerBounded(Ctx, -359, 359, 0);

	else if (strEquals(sParam, XFORM_TIME_ATTRIB))
		m_rXformTime = Value.EvalIntegerBounded(Ctx, 0, -1, 100) / 100.0;
	}
