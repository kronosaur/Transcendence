//	CRadiusDamage.cpp
//
//	CRadiusDamage class

#include "PreComp.h"

static CObjectClass<CRadiusDamage>g_Class(OBJID_CRADIUSDAMAGE, NULL);

CRadiusDamage::CRadiusDamage (void) : CSpaceObject(&g_Class),
		m_pEnhancements(NULL),
		m_pPainter(NULL)

//	CRadiusDamage constructor

	{
	}

CRadiusDamage::~CRadiusDamage (void)

//	CRadiusDamage destructor

	{
	if (m_pPainter)
		m_pPainter->Delete();

	if (m_pEnhancements)
		m_pEnhancements->Delete();
	}

ALERROR CRadiusDamage::Create (CSystem *pSystem,
							   CWeaponFireDesc *pDesc,
							   CItemEnhancementStack *pEnhancements,
							   DestructionTypes iCause,
							   const CDamageSource &Source,
							   const CVector &vPos,
							   const CVector &vVel,
							   CSpaceObject *pTarget,
							   CRadiusDamage **retpObj)

//	Create
//
//	Create the object

	{
	ALERROR error;

	//	Make sure we have a valid CWeaponFireDesc (otherwise we won't be
	//	able to save the descriptor).
	ASSERT(!pDesc->m_sUNID.IsBlank());

	//	Create the area

	CRadiusDamage *pArea = new CRadiusDamage;
	if (pArea == NULL)
		return ERR_MEMORY;

	pArea->Place(vPos, vVel);

	//	Get notifications when other objects are destroyed
	pArea->SetObjectDestructionHook();

	pArea->m_iLifeLeft = pDesc->GetLifetime();
	pArea->m_pDesc = pDesc;
	pArea->m_pEnhancements = (pEnhancements ? pEnhancements->AddRef() : NULL);
	pArea->m_iCause = iCause;
	pArea->m_Source = Source;
	pArea->m_pTarget = pTarget;
	pArea->m_iTick = 0;

	Metric rRadius = pDesc->GetMaxRadius();

	//	Friendly fire

	if (!pDesc->CanHitFriends())
		pArea->SetNoFriendlyFire();

	//	Remember the sovereign of the source (in case the source is destroyed)

	pArea->m_pSovereign = Source.GetSovereign();

	//	Create a painter instance

	CEffectCreator *pEffect;
	if (pEffect = pDesc->GetEffect())
		{
		CCreatePainterCtx Ctx;
		Ctx.SetWeaponFireDesc(pDesc);

		pArea->m_pPainter = pEffect->CreatePainter(Ctx);

		//	The lifetime of the object is based on the painter

		pArea->m_iLifeLeft = Max(pEffect->GetLifetime(), pArea->m_iLifeLeft);

		//	The radius is also adjusted

		rRadius = Max(pArea->m_pPainter->GetRadius(), rRadius);
		}
	else
		pArea->m_pPainter = NULL;

	//	Our bounds are based on the max radius

	pArea->SetBounds(rRadius);

	//	Add to system

	if (error = pArea->AddToSystem(pSystem))
		{
		delete pArea;
		return error;
		}

	//	Done

	if (retpObj)
		*retpObj = pArea;

	return NOERROR;
	}

CString CRadiusDamage::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Dump debug information

	{
	CString sResult;

	sResult.Append(strPatternSubst(CONSTLIT("m_pDesc: %s\r\n"), m_pDesc->m_sUNID));
	sResult.Append(strPatternSubst(CONSTLIT("m_Source: %s\r\n"), CSpaceObject::DebugDescribe(m_Source.GetObj())));
	sResult.Append(strPatternSubst(CONSTLIT("m_pTarget: %s\r\n"), CSpaceObject::DebugDescribe(m_pTarget)));
	sResult.Append(strPatternSubst(CONSTLIT("m_iDestiny: %d\r\n"), GetDestiny()));
	sResult.Append(strPatternSubst(CONSTLIT("m_iTick: %d\r\n"), m_iTick));
	sResult.Append(strPatternSubst(CONSTLIT("m_iLifeLeft: %d\r\n"), m_iLifeLeft));

	//	m_pEnhancements

	try
		{
		if (m_pEnhancements)
			sResult.Append(strPatternSubst(CONSTLIT("m_pEnhancements: %d\r\n"), m_pEnhancements->GetCount()));
		else
			sResult.Append(CONSTLIT("m_pEnhancements: none\r\n"));
		}
	catch (...)
		{
		sResult.Append(strPatternSubst(CONSTLIT("m_pEnhancements: %x [invalid]\r\n"), (DWORD)m_pEnhancements));
		}

	//	m_pPainter

	try
		{
		if (m_pPainter)
			sResult.Append(strPatternSubst(CONSTLIT("m_pPainter: %s\r\n"), m_pPainter->GetCreator()->GetTypeClassName()));
		else
			sResult.Append(CONSTLIT("m_pPainter: none\r\n"));
		}
	catch (...)
		{
		sResult.Append(strPatternSubst(CONSTLIT("m_pPainter: %x [invalid]\r\n"), (DWORD)m_pPainter));
		}

	return sResult;
	}

CString CRadiusDamage::GetName (DWORD *retdwFlags)

//	GetName
//
//	Returns the name of the object

	{
	//	This name is used only if the source has been destroyed

	if (retdwFlags)
		*retdwFlags = 0;
	return CONSTLIT("enemy weapon");
	}

void CRadiusDamage::OnMove (const CVector &vOldPos, Metric rSeconds)

//	OnMove
//
//	Handle moving

	{
	//	Update the painter motion

	if (m_pPainter)
		{
		SEffectMoveCtx Ctx;
		Ctx.pObj = this;

		bool bBoundsChanged;
		m_pPainter->OnMove(Ctx, &bBoundsChanged);

		//	Set bounds, if they've changed

		if (bBoundsChanged)
			SetBounds(m_pPainter);
		}
	}

void CRadiusDamage::ObjectDestroyedHook (const SDestroyCtx &Ctx)

//	ObjectDestroyedHook
//
//	Called when another object is destroyed

	{
	m_Source.OnObjDestroyed(Ctx.pObj);

	if (Ctx.pObj == m_pTarget)
		m_pTarget = NULL;
	}

void CRadiusDamage::OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paint

	{
	if (m_pPainter)
		{
		Ctx.iTick = m_iTick;
		Ctx.iVariant = 0;
		Ctx.iDestiny = GetDestiny();
		m_pPainter->Paint(Dest, x, y, Ctx);
		}
	}

void CRadiusDamage::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Restore from stream

	{
	DWORD dwLoad;

#ifdef DEBUG_LOAD
	::OutputDebugString("CRadiusDamage::OnReadFromStream\n");
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
	if (Ctx.dwVersion >= 19)
		CSystem::ReadObjRefFromStream(Ctx, &m_pTarget);
	else
		m_pTarget = NULL;

	CSystem::ReadSovereignRefFromStream(Ctx, &m_pSovereign);
	Ctx.pStream->Read((char *)&m_iTick, sizeof(m_iTick));

	//	Load painter

	m_pPainter = CEffectCreator::CreatePainterFromStreamAndCreator(Ctx, m_pDesc->GetEffect());

	//	Enhancements

	if (Ctx.dwVersion >= 92)
		CItemEnhancementStack::ReadFromStream(Ctx, &m_pEnhancements);
	}

void CRadiusDamage::OnSystemLoaded (void)

//	OnSystemLoaded
//
//	System done loading

	{
	}

void CRadiusDamage::OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick)

//	OnUpdate
//
//	Update

	{
	int i;
	bool bDestroy = false;

	//	Do damage right away

	if (m_iTick == 0)
		{
		Metric rMinRadius = m_pDesc->GetMinRadius();
		Metric rMaxRadius = m_pDesc->GetMaxRadius();
		Metric rRadiusRange = rMaxRadius - rMinRadius;

		if (rMaxRadius > 0.0)
			{
			CVector vUR, vLL;
			GetBoundingRect(&vUR, &vLL);

			for (i = 0; i < GetSystem()->GetObjectCount(); i++)
				{
				CSpaceObject *pObj = GetSystem()->GetObject(i);
				if (pObj 
						&& !pObj->IsDestroyed()
						&& CanHit(pObj)
						&& pObj->CanBeHit()
						&& pObj->InBox(vUR, vLL)
						&& pObj->CanBeHitBy(m_pDesc->m_Damage)
						&& pObj != this)
					{
					//	Compute the distance between this object and the center
					//	of the blast

					CVector vDist = (pObj->GetPos() - GetPos());
					Metric rDist;
					int iAngle = VectorToPolar(vDist, &rDist);

					//	Adjust damage for distance

					if (rDist < rMaxRadius)
						{
						//	Find the point where we hit the object

						Metric rObjRadius = 0.5 * pObj->GetHitSize();
						CVector vHitPos = pObj->GetPos() - PolarToVector(iAngle, rObjRadius);
						CVector vInc = PolarToVector(iAngle, 2.0 * g_KlicksPerPixel);
						int iMax = (int)((2.0 * rObjRadius / (2.0 * g_KlicksPerPixel)) + 0.5);
						while (!pObj->PointInObject(pObj->GetPos(), vHitPos) && iMax-- > 0)
							vHitPos = vHitPos + vInc;

						//	Setup context

						SDamageCtx Ctx;
						Ctx.pObj = pObj;
						Ctx.pDesc = m_pDesc;
						Ctx.Damage = m_pDesc->m_Damage;
						Ctx.Damage.AddEnhancements(m_pEnhancements);
						Ctx.Damage.SetCause(m_iCause);
						if (IsAutomatedWeapon())
							Ctx.Damage.SetAutomatedWeapon();
						Ctx.iDirection = (iAngle + 180) % 360;
						Ctx.vHitPos = vHitPos;
						Ctx.pCause = this;
						Ctx.Attacker = m_Source;

						//	If we're beyond the minimum radius, then decrease the damage
						//	to account for distance

						if (rDist > rMinRadius && rRadiusRange > 0.0)
							{
							Metric rMult = (rRadiusRange - (rDist - rMinRadius)) / rRadiusRange;

							int iDamage = (int)(rMult * (Metric)Ctx.Damage.RollDamage() + 0.5);
							Ctx.Damage.SetDamage(iDamage);
							}

						//	Do damage

						pObj->Damage(Ctx);
						}
					}
				}
			}

		//	Spawn fragments, if necessary

		if (m_pDesc->HasFragments())
			{
			GetSystem()->CreateWeaponFragments(m_pDesc,
					m_pEnhancements,
					m_iCause,
					m_Source,
					m_pTarget,
					GetPos(),
					GetVel(),
					this);
			}
		}

	//	Update the object

	m_iTick++;

	if (m_pPainter)
		m_pPainter->OnUpdate();

	//	Destroy

	if (bDestroy || --m_iLifeLeft <= 0)
		Destroy(removedFromSystem, CDamageSource());
	}

void CRadiusDamage::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write out to stream
//
//	CString			CWeaponFireDesc UNID
//	DWORD			m_iCause
//	DWORD			m_iLifeLeft
//	DWORD			m_Source (CSpaceObject ref)
//	DWORD			m_pTarget (CSpaceObject ref)
//	DWORD			m_pSovereign (CSovereign ref)
//	DWORD			m_iTick
//	IEffectPainter	m_pPainter
//
//	CItemEnhancementStack	m_pEnhancements

	{
	DWORD dwSave;

	m_pDesc->m_sUNID.WriteToStream(pStream);
	dwSave = m_iCause;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&m_iLifeLeft, sizeof(m_iLifeLeft));
	m_Source.WriteToStream(GetSystem(), pStream);
	WriteObjRefToStream(m_pTarget, pStream);
	GetSystem()->WriteSovereignRefToStream(m_pSovereign, pStream);
	pStream->Write((char *)&m_iTick, sizeof(m_iTick));

	CEffectCreator::WritePainterToStream(pStream, m_pPainter);

	//	Enhancements

	CItemEnhancementStack::WriteToStream(m_pEnhancements, pStream);
	}

bool CRadiusDamage::PointInObject (const CVector &vObjPos, const CVector &vPointPos)

//	PointInObject
//
//	Returns TRUE if the given point is in the object

	{
	return false;
	}
