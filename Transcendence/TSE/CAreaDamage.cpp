//	CAreaDamage.cpp
//
//	CAreaDamage class

#include "PreComp.h"

#define SPEED_PARAM								CONSTLIT("speed")

static CObjectClass<CAreaDamage>g_Class(OBJID_CAREADAMAGE, NULL);

CAreaDamage::CAreaDamage (void) : CSpaceObject(&g_Class),
		m_pEnhancements(NULL),
		m_pPainter(NULL)

//	CAreaDamage constructor

	{
	}

CAreaDamage::~CAreaDamage (void)

//	CAreaDamage destructor

	{
	if (m_pPainter)
		m_pPainter->Delete();

	if (m_pEnhancements)
		m_pEnhancements->Delete();
	}

ALERROR CAreaDamage::Create (CSystem *pSystem,
							 CWeaponFireDesc *pDesc,
							 CItemEnhancementStack *pEnhancements,
							 DestructionTypes iCause,
							 const CDamageSource &Source,
							 const CVector &vPos,
							 const CVector &vVel,
							 CAreaDamage **retpObj)

//	Create
//
//	Create the object

	{
	ALERROR error;

	//	Make sure we have a valid CWeaponFireDesc (otherwise we won't be
	//	able to save it the area of effect).
	ASSERT(!pDesc->m_sUNID.IsBlank());

	//	Create the area

	CAreaDamage *pArea = new CAreaDamage;
	if (pArea == NULL)
		return ERR_MEMORY;

	pArea->Place(vPos, vVel);

	//	Get notifications when other objects are destroyed
	pArea->SetObjectDestructionHook();

	pArea->m_pDesc = pDesc;
	pArea->m_pEnhancements = (pEnhancements ? pEnhancements->AddRef() : NULL);
	pArea->m_iCause = iCause;
	pArea->m_iLifeLeft = pDesc->GetLifetime();
	pArea->m_Source = Source;
	pArea->m_iInitialDelay = pDesc->GetInitialDelay();
	pArea->m_iTick = 0;

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

		//	Set the expansion speed appropriately

		if (pArea->m_pPainter)
			pArea->m_pPainter->SetParamMetric(SPEED_PARAM, pDesc->GetExpansionSpeed());
		}

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

CString CAreaDamage::GetName (DWORD *retdwFlags)

//	GetName
//
//	Returns the name of the object

	{
	//	This name is used only if the source has been destroyed

	if (retdwFlags)
		*retdwFlags = 0;
	return CONSTLIT("enemy weapon");
	}

void CAreaDamage::OnMove (const CVector &vOldPos, Metric rSeconds)

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

void CAreaDamage::ObjectDestroyedHook (const SDestroyCtx &Ctx)

//	ObjectDestroyedHook
//
//	Called when another object is destroyed

	{
	m_Source.OnObjDestroyed(Ctx.pObj);
	}

void CAreaDamage::OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paint

	{
	if (m_iInitialDelay > 0)
		return;

	if (m_pPainter)
		{
		Ctx.iTick = m_iTick;
		Ctx.iVariant = 0;
		Ctx.iDestiny = GetDestiny();
		m_pPainter->Paint(Dest, x, y, Ctx);
		}
	}

void CAreaDamage::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Restore from stream

	{
	DWORD dwLoad;

#ifdef DEBUG_LOAD
	::OutputDebugString("CAreaDamage::OnReadFromStream\n");
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

	if (Ctx.dwVersion >= 21)
		Ctx.pStream->Read((char *)&m_iInitialDelay, sizeof(DWORD));
	else
		m_iInitialDelay = 0;

	Ctx.pStream->Read((char *)&m_iLifeLeft, sizeof(m_iLifeLeft));
	m_Source.ReadFromStream(Ctx);
	CSystem::ReadSovereignRefFromStream(Ctx, &m_pSovereign);
	Ctx.pStream->Read((char *)&m_iTick, sizeof(m_iTick));

	//	Load painter

	m_pPainter = CEffectCreator::CreatePainterFromStreamAndCreator(Ctx, m_pDesc->GetEffect());

	//	Previous versions stored an m_Hit array

	if (Ctx.dwVersion < 70)
		{
		int iCount;
		Ctx.pStream->Read((char *)&iCount, sizeof(DWORD));
		if (iCount)
			Ctx.pStream->Read(NULL, iCount * 2 * sizeof(DWORD));
		}

	//	Enhancements

	if (Ctx.dwVersion >= 92)
		CItemEnhancementStack::ReadFromStream(Ctx, &m_pEnhancements);
	}

void CAreaDamage::OnSystemLoaded (void)

//	OnSystemLoaded
//
//	System done loading

	{
	}

void CAreaDamage::OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick)

//	OnUpdate
//
//	Update

	{
	if (m_iInitialDelay > 0)
		{
		m_iInitialDelay--;
		return;
		}

	if (m_pPainter == NULL)
		{
		Destroy(removedFromSystem, CDamageSource());
		return;
		}

	//	Next

	m_iTick++;

	//	Set up context block for particle array update

	SEffectUpdateCtx EffectCtx;
	EffectCtx.pSystem = GetSystem();
	EffectCtx.pObj = this;

	EffectCtx.pDamageDesc = m_pDesc;
	EffectCtx.pEnhancements = m_pEnhancements;
	EffectCtx.iCause = m_iCause;
	EffectCtx.bAutomatedWeapon = IsAutomatedWeapon();
	EffectCtx.Attacker = m_Source;

	m_pPainter->OnUpdate(EffectCtx);

	//	Destroy

	if (EffectCtx.bDestroy || --m_iLifeLeft <= 0)
		Destroy(removedFromSystem, CDamageSource());
	}

void CAreaDamage::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write out to stream
//
//	CString			CWeaponFireDesc UNID
//	DWORD			m_iCause
//	DWORD			m_iInitialDelay
//	DWORD			m_iLifeLeft
//	DWORD			m_Source (CDamageSource)
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
	pStream->Write((char *)&m_iInitialDelay, sizeof(DWORD));
	pStream->Write((char *)&m_iLifeLeft, sizeof(m_iLifeLeft));
	m_Source.WriteToStream(GetSystem(), pStream);
	GetSystem()->WriteSovereignRefToStream(m_pSovereign, pStream);
	pStream->Write((char *)&m_iTick, sizeof(m_iTick));

	CEffectCreator::WritePainterToStream(pStream, m_pPainter);

	//	Enhancements

	CItemEnhancementStack::WriteToStream(m_pEnhancements, pStream);
	}

void CAreaDamage::PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans)

//	PaintLRS
//
//	Paints the object on an LRS

	{
	if (m_pPainter)
		{
		int i;
		Metric rRadius = m_pPainter->GetRadius();
		int iRadius = (int)((rRadius / g_KlicksPerPixel) + 0.5);
		int iCount = Min(64, 3 * iRadius);

		for (i = 0; i < iCount; i++)
			{
			CVector vPos = GetPos() + PolarToVector(mathRandom(0, 359), rRadius);
			Trans.Transform(vPos, &x, &y);

			int iColor = mathRandom(128, 255);
			Dest.DrawDot(x, y, 
					CG16bitImage::RGBValue(iColor, iColor, 0), 
					CG16bitImage::markerSmallRound);
			}
		}
	}

bool CAreaDamage::PointInObject (const CVector &vObjPos, const CVector &vPointPos)

//	PointInObject
//
//	Returns TRUE if the given point is in the object

	{
	if (m_pPainter)
		{
		RECT rcBounds;
		m_pPainter->GetRect(&rcBounds);
		Metric rRadius = (RectWidth(rcBounds) / 2) * g_KlicksPerPixel;
		Metric rRadius2 = rRadius * rRadius;
		Metric rInnerRadius = rRadius * 0.8;
		Metric rInnerRadius2 = rInnerRadius * rInnerRadius;
		CVector vDist = vObjPos - vPointPos;
		Metric rDist2 = vDist.Length2();
		return (rDist2 < rRadius2 && rDist2 > rInnerRadius2);
		}
	else
		return false;
	}
