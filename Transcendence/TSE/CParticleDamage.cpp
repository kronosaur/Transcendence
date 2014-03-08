//	CParticleDamage.cpp
//
//	CParticleDamage class

#include "PreComp.h"

static CObjectClass<CParticleDamage>g_Class(OBJID_CPARTICLEDAMAGE, NULL);

CParticleDamage::CParticleDamage (void) : CSpaceObject(&g_Class),
		m_pEnhancements(NULL),
		m_pPainter(NULL)

//	CParticleDamage constructor

	{
	}

CParticleDamage::~CParticleDamage (void)

//	CParticleDamage destructor

	{
	if (m_pPainter)
		m_pPainter->Delete();

	if (m_pEnhancements)
		m_pEnhancements->Delete();
	}

ALERROR CParticleDamage::Create (CSystem *pSystem,
								 CWeaponFireDesc *pDesc,
								 CItemEnhancementStack *pEnhancements,
								 DestructionTypes iCause,
								 const CDamageSource &Source,
								 const CVector &vPos,
								 const CVector &vVel,
								 int iDirection,
								 CSpaceObject *pTarget,
								 CParticleDamage **retpObj)

//	Create
//
//	Create the object

	{
	ALERROR error;

	//	Make sure we have a valid CWeaponFireDesc (otherwise we won't be
	//	able to save the object).
	ASSERT(!pDesc->m_sUNID.IsBlank());

	//	Create the area

	CParticleDamage *pParticles = new CParticleDamage;
	if (pParticles == NULL)
		return ERR_MEMORY;

	pParticles->Place(vPos, vVel);

	//	Get notifications when other objects are destroyed
	pParticles->SetObjectDestructionHook();

	//	Set non-linear move, meaning that we are responsible for
	//	setting the position and velocity in OnMove
	pParticles->SetNonLinearMove();

	pParticles->m_pDesc = pDesc;
	pParticles->m_pTarget = pTarget;
	pParticles->m_pEnhancements = (pEnhancements ? pEnhancements->AddRef() : NULL);
	pParticles->m_iCause = iCause;
	pParticles->m_iEmitDirection = iDirection;
	pParticles->m_vEmitSourcePos = vPos;
	pParticles->m_vEmitSourceVel = (Source.GetObj() ? Source.GetObj()->GetVel() : CVector());
	pParticles->m_iEmitTime = Max(1, pDesc->GetParticleEmitTime());
	pParticles->m_iLifeLeft = pDesc->GetMaxLifetime() + pParticles->m_iEmitTime;
	pParticles->m_Source = Source;
	pParticles->m_iTick = 0;

	pParticles->m_iDamage = pDesc->m_Damage.RollDamage();

	//	Friendly fire

	if (!pDesc->CanHitFriends())
		pParticles->SetNoFriendlyFire();

	//	Painter

	CEffectCreator *pEffect;
	if (pEffect = pDesc->GetEffect())
		{
		CCreatePainterCtx Ctx;
		Ctx.SetWeaponFireDesc(pDesc);

		pParticles->m_pPainter = pEffect->CreatePainter(Ctx);
		}

	//	Remember the sovereign of the source (in case the source is destroyed)

	if (Source.GetObj())
		pParticles->m_pSovereign = Source.GetObj()->GetSovereign();
	else
		pParticles->m_pSovereign = NULL;

	//	Compute the maximum number of particles that we might have

	int iMaxCount = pParticles->m_iEmitTime * pDesc->GetMaxParticleCount();
	pParticles->m_Particles.Init(iMaxCount, vPos);

	//	Create the initial particles

	int iInitCount = pDesc->GetParticleCount();
	pParticles->InitParticles(iInitCount, CVector(), vVel, iDirection);

	//	Figure out the number of particles that will cause full damage

	if (pParticles->m_iEmitTime > 1)
		pParticles->m_iParticleCount = pParticles->m_iEmitTime * pDesc->GetAveParticleCount();
	else
		pParticles->m_iParticleCount = iInitCount;

	pParticles->m_iParticleCount = Max(1, pParticles->m_iParticleCount);

	//	Add to system

	if (error = pParticles->AddToSystem(pSystem))
		{
		delete pParticles;
		return error;
		}

	//	Done

	if (retpObj)
		*retpObj = pParticles;

	return NOERROR;
	}

CString CParticleDamage::GetName (DWORD *retdwFlags)

//	GetName
//
//	Returns the name of the object

	{
	//	This name is used only if the source has been destroyed

	if (retdwFlags)
		*retdwFlags = 0;
	return CONSTLIT("enemy weapon");
	}

void CParticleDamage::InitParticles (int iCount, const CVector &vSource, const CVector &vInitVel, int iDirection)

//	InitParticles
//
//	Initialize particles

	{
	int i;

	//	Generate the number of particles

	if (iCount > 0)
		{
		//	Calculate a few temporaries

		Metric rRadius = (6.0 * m_pDesc->GetRatedSpeed());

		int iSpreadAngle = m_pDesc->GetParticleSpreadAngle();
		if (iSpreadAngle > 0)
			iSpreadAngle = (iSpreadAngle / 2) + 1;
		bool bSpreadAngle = (iSpreadAngle > 0);

		CVector vTemp = PolarToVector(iSpreadAngle, m_pDesc->GetRatedSpeed());
		Metric rTangentV = (3.0 * vTemp.GetY());
		int iTangentAngle = (iDirection + 90) % 360;

		int iSpreadWidth = m_pDesc->GetParticleSpreadWidth();
		Metric rSpreadWidth = iSpreadWidth * g_KlicksPerPixel;
		bool bSpreadWidth = (iSpreadWidth > 0);

		//	Create the particles with appropriate velocity

		for (i = 0; i < iCount; i++)
			{
			Metric rPlace = ((mathRandom(0, 25) + mathRandom(0, 25) + mathRandom(0, 25) + mathRandom(0, 25)) - 50.0) / 100.0;
			Metric rTangentPlace = ((mathRandom(0, 25) + mathRandom(0, 25) + mathRandom(0, 25) + mathRandom(0, 25)) - 50.0) / 100.0;
	
			CVector vPlace = PolarToVector(iDirection, rRadius * rPlace);
			CVector vVel = vInitVel
					+ (0.05 * vPlace)
					+ PolarToVector(iTangentAngle, rTangentV * rTangentPlace);

			//	Compute the spread width

			CVector vPos = vSource + vPlace;
			if (bSpreadWidth)
				vPos = vPos + PolarToVector(iTangentAngle, rSpreadWidth * rTangentPlace);

			//	Compute the travel rotation for these particles

			int iRotation = (bSpreadAngle ? VectorToPolar(GetVel() + vVel) : iDirection);

			//	Create the particle

			m_Particles.AddParticle(vPos, vVel, m_pDesc->GetLifetime(), iRotation);
			}
		}
	}

void CParticleDamage::OnMove (const CVector &vOldPos, Metric rSeconds)

//	OnMove
//
//	Handle moving

	{
	//	Update the single particle painter

	if (m_pPainter)
		{
		SEffectMoveCtx Ctx;
		Ctx.pObj = this;

		m_pPainter->OnMove(Ctx);
		}

	//	Update particle motion

	bool bAlive;
	CVector vNewPos;
	m_Particles.UpdateMotionLinear(&bAlive, &vNewPos);

	//	If no particles are left alive, then we destroy the object

	if (!bAlive)
		{
		Destroy(removedFromSystem, CDamageSource());
		return;
		}

	//	Set the position of the object base on the average particle position

	SetPos(m_Particles.GetOrigin() + vNewPos);

	//	Set the bounds (note, we make the bounds twice as large to deal
	//	with the fact that we're moving).

	RECT rcBounds = m_Particles.GetBounds();
	SetBounds(g_KlicksPerPixel * Max(RectWidth(rcBounds), RectHeight(rcBounds)));

	//	Update emit source position

	m_vEmitSourcePos = m_vEmitSourcePos + m_vEmitSourceVel;
	}

void CParticleDamage::ObjectDestroyedHook (const SDestroyCtx &Ctx)

//	ObjectDestroyedHook
//
//	Called when another object is destroyed

	{
	m_Source.OnObjDestroyed(Ctx.pObj);

	if (Ctx.pObj == m_pTarget)
		m_pTarget = NULL;
	}

void CParticleDamage::OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paint

	{
	if (m_pPainter)
		{
		Ctx.iTick = m_iTick;
		Ctx.iMaxLength = Max(10, (int)((g_SecondsPerUpdate * (m_iTick - 1) * m_pDesc->GetRatedSpeed()) / g_KlicksPerPixel));

		//	Painting is relative to the origin

		int xOrigin, yOrigin;
		Ctx.XForm.Transform(m_Particles.GetOrigin(), &xOrigin, &yOrigin);

		//	If we can get a paint descriptor, use that because it is faster

		SParticlePaintDesc Desc;
		if (m_pPainter->GetParticlePaintDesc(&Desc))
			{
			Desc.iMaxLifetime = m_pDesc->GetMaxLifetime();
			m_Particles.Paint(Dest, xOrigin, yOrigin, Ctx, Desc);
			}

		//	Otherwise, we use the painter for each particle

		else
			m_Particles.Paint(Dest, xOrigin, yOrigin, Ctx, m_pPainter);
		}
	}

void CParticleDamage::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Restore from stream

	{
	DWORD dwLoad;

#ifdef DEBUG_LOAD
	::OutputDebugString("CParticleDamage::OnReadFromStream\n");
#endif

	//	Load descriptor

	CString sDescUNID;
	sDescUNID.ReadFromStream(Ctx.pStream);
	m_pDesc = g_pUniverse->FindWeaponFireDesc(sDescUNID);

	//	Old style bonus

	if (Ctx.dwVersion < 92)
		{
		int iBonus;
		Ctx.pStream->Read((char *)&iBonus, sizeof(DWORD));
		if (iBonus != 0)
			{
			m_pEnhancements = new CItemEnhancementStack;
			m_pEnhancements->InsertHPBonus(iBonus);
			}
		}

	//	Load other stuff

	if (Ctx.dwVersion >= 18)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_iCause = (DestructionTypes)dwLoad;
		}
	else
		m_iCause = killedByDamage;

	Ctx.pStream->Read((char *)&m_iLifeLeft, sizeof(m_iLifeLeft));
	m_Source.ReadFromStream(Ctx);
	CSystem::ReadSovereignRefFromStream(Ctx, &m_pSovereign);
	Ctx.pStream->Read((char *)&m_iTick, sizeof(m_iTick));
	Ctx.pStream->Read((char *)&m_iDamage, sizeof(m_iDamage));
	if (Ctx.dwVersion >= 3 && Ctx.dwVersion < 67)
		{
		CVector vDummy;
		Ctx.pStream->Read((char *)&vDummy, sizeof(CVector));
		}

	//	The newer version uses a different particle array

	if (Ctx.dwVersion >= 21)
		{
		Ctx.pStream->Read((char *)&m_vEmitSourcePos, sizeof(CVector));
		Ctx.pStream->Read((char *)&m_vEmitSourceVel, sizeof(CVector));
		Ctx.pStream->Read((char *)&m_iEmitDirection, sizeof(DWORD));
		Ctx.pStream->Read((char *)&m_iEmitTime, sizeof(DWORD));
		Ctx.pStream->Read((char *)&m_iParticleCount, sizeof(DWORD));

		//	Load painter

		m_pPainter = CEffectCreator::CreatePainterFromStreamAndCreator(Ctx, m_pDesc->GetEffect());

		m_Particles.ReadFromStream(Ctx);
		}

	//	Read the previous version, but no need to convert

	else
		{
		DWORD dwCount;
		Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
		if (dwCount > 0)
			{
			char *pDummy = new char [5 * sizeof(DWORD) * dwCount];
			Ctx.pStream->Read(pDummy, 5 * sizeof(DWORD) * dwCount);
			delete pDummy;
			}

		m_iEmitTime = 0;
		m_iEmitDirection = -1;
		}

	//	Read the target

	if (Ctx.dwVersion >= 67)
		CSystem::ReadObjRefFromStream(Ctx, &m_pTarget);
	else
		m_pTarget = NULL;

	//	Enhancements

	if (Ctx.dwVersion >= 92)
		CItemEnhancementStack::ReadFromStream(Ctx, &m_pEnhancements);
	}

void CParticleDamage::OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick)

//	OnUpdate
//
//	Update

	{
	m_iTick++;

	//	Update the single particle painter

	if (m_pPainter)
		m_pPainter->OnUpdate();

	//	Set up context block for particle array update

	SEffectUpdateCtx EffectCtx;
	EffectCtx.pSystem = GetSystem();
	EffectCtx.pObj = this;

	EffectCtx.pDamageDesc = m_pDesc;
	EffectCtx.iTotalParticleCount = m_iParticleCount;
	EffectCtx.pEnhancements = m_pEnhancements;
	EffectCtx.iCause = m_iCause;
	EffectCtx.bAutomatedWeapon = IsAutomatedWeapon();
	EffectCtx.Attacker = m_Source;

	//	Update (includes doing damage)

	m_Particles.Update(EffectCtx);

	//	If we're tracking, change velocity to follow target

	if (m_pTarget && m_pDesc->IsTrackingTime(m_iTick))
		m_Particles.UpdateTrackTarget(m_pTarget, m_pDesc->GetManeuverRate(), m_pDesc->GetRatedSpeed());

	//	Expired?

	if (--m_iLifeLeft <= 0)
		{
		Destroy(removedFromSystem, CDamageSource());
		return;
		}

	//	Emit new particles

	if (m_iTick < m_iEmitTime && !m_Source.IsEmpty())
		{
		InitParticles(m_pDesc->GetParticleCount(),
				m_vEmitSourcePos - GetPos(),
				GetVel(),
				m_iEmitDirection);
		}
	}

void CParticleDamage::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write out to stream
//
//	CString			CWeaponFireDesc UNID
//	DWORD			m_iLifeLeft
//	DWORD			m_Source (CSpaceObject ref)
//	DWORD			m_pSovereign (CSovereign ref)
//	DWORD			m_iTick
//	DWORD			m_iDamage
//
//	CVector			m_vEmitSourcePos
//	CVector			m_vEmitSourceVel
//	DWORD			m_iEmitDirection
//	DWORD			m_iEmitTime
//	DWORD			m_iParticleCount
//	IEffectPainter
//	CParticleArray
//
//	CSpaceObject	m_pTarget
//
//	CItemEnhancementStack	m_pEnhancements

	{
	DWORD dwSave;
	m_pDesc->m_sUNID.WriteToStream(pStream);
	dwSave = m_iCause;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&m_iLifeLeft, sizeof(m_iLifeLeft));
	m_Source.WriteToStream(GetSystem(), pStream);
	GetSystem()->WriteSovereignRefToStream(m_pSovereign, pStream);
	pStream->Write((char *)&m_iTick, sizeof(m_iTick));
	pStream->Write((char *)&m_iDamage, sizeof(m_iDamage));
	pStream->Write((char *)&m_vEmitSourcePos, sizeof(CVector));
	pStream->Write((char *)&m_vEmitSourceVel, sizeof(CVector));
	pStream->Write((char *)&m_iEmitDirection, sizeof(DWORD));
	pStream->Write((char *)&m_iEmitTime, sizeof(DWORD));
	pStream->Write((char *)&m_iParticleCount, sizeof(DWORD));

	CEffectCreator::WritePainterToStream(pStream, m_pPainter);

	m_Particles.WriteToStream(pStream);

	WriteObjRefToStream(m_pTarget, pStream);

	//	Enhancements

	CItemEnhancementStack::WriteToStream(m_pEnhancements, pStream);
	}

bool CParticleDamage::PointInObject (const CVector &vObjPos, const CVector &vPointPos)

//	PointInObject
//
//	Returns TRUE if the given point is in the object

	{
	return false;
	}
