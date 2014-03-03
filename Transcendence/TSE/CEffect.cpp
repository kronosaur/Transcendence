//	CEffect.cpp
//
//	CEffect class

#include "PreComp.h"

static CObjectClass<CEffect>g_Class(OBJID_CEFFECT, NULL);

CEffect::CEffect (void) : CSpaceObject(&g_Class), m_pPainter(NULL)

//	CEffect constructor

	{
	}

CEffect::~CEffect (void)

//	CEffect destructor

	{
	if (m_pPainter)
		m_pPainter->Delete();
	}

ALERROR CEffect::Create (CEffectCreator *pType,
				CSystem *pSystem,
				CSpaceObject *pAnchor,
				const CVector &vPos,
				const CVector &vVel,
				int iRotation,
				CEffect **retpEffect)

//	Create
//
//	Creates a new effects object

	{
	ALERROR error;
	CEffect *pEffect;

	pEffect = new CEffect;
	if (pEffect == NULL)
		return ERR_MEMORY;

	pEffect->Place(vPos, (pAnchor == NULL ? vVel : CVector()));
	pEffect->SetObjectDestructionHook();

	ASSERT(pType);
	pEffect->m_pPainter = pType->CreatePainter(CCreatePainterCtx());
	pEffect->m_pAnchor = pAnchor;
	pEffect->m_iLifetime = pType->GetLifetime();
	pEffect->m_iRotation = iRotation;
	pEffect->m_iTick = 0;

	//	Add to system

	if (error = pEffect->AddToSystem(pSystem))
		{
		delete pEffect;
		return error;
		}

	//	Set bounds

	if (pEffect->m_pPainter)
		{
		pEffect->m_pPainter->SetPos(vPos);
		pEffect->SetBounds(pEffect->m_pPainter);
		}

	//	Play sound

	pType->PlaySound(pEffect);

	//	Done

	if (retpEffect)
		*retpEffect = pEffect;

	return NOERROR;
	}

CEffectCreator *CEffect::GetEffectCreator (void)

//	GetEffectCreator
//
//	Returns the effect type

	{
	if (m_pPainter)
		return m_pPainter->GetCreator();
	else
		return NULL;
	}

void CEffect::OnMove (const CVector &vOldPos, Metric rSeconds)

//	OnMove
//
//	Move hook

	{
	//	If we've got an anchor then move the effect along with the anchor

	if (m_pAnchor)
		SetPos(GetPos() + (m_pAnchor->GetVel() * rSeconds));

	//	Update the painter motion

	if (m_pPainter)
		{
		SEffectMoveCtx Ctx;
		Ctx.pObj = this;

		//	Tell the painter about the new absolute position

		m_pPainter->SetPos(GetPos());

		//	Let the painter handle a move

		bool bBoundsChanged;
		m_pPainter->OnMove(Ctx, &bBoundsChanged);

		//	Set bounds, if they've changed

		if (bBoundsChanged)
			SetBounds(m_pPainter);
		}
	}

void CEffect::ObjectDestroyedHook (const SDestroyCtx &Ctx)

//	ObjectDestroyedHook
//
//	Handle it when another object is destroyed

	{
	//	If our source is destroyed, clear it

	if (Ctx.pObj == m_pAnchor)
		m_pAnchor = NULL;
	}

void CEffect::OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paints the effect

	{
	if (m_pPainter)
		{
		Ctx.iTick = m_iTick;
		Ctx.iVariant = 0;
		Ctx.iRotation = m_iRotation;
		Ctx.iDestiny = GetDestiny();

		m_pPainter->Paint(Dest, x, y, Ctx);
		}
	}

void CEffect::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read object data from a stream
//
//	DWORD		IEffectPainter
//	DWORD		m_pAnchor (CSpaceObject Ref)
//	DWORD		m_iLifetime
//	DWORD		m_iTick
//	DWORD		m_iRotation

	{
#ifdef DEBUG_LOAD
	::OutputDebugString("CEffect::OnReadFromStream\n");
#endif
	m_pPainter = CEffectCreator::CreatePainterFromStream(Ctx);
	CSystem::ReadObjRefFromStream(Ctx, &m_pAnchor);
	Ctx.pStream->Read((char *)&m_iLifetime, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iTick, sizeof(DWORD));

	if (Ctx.dwVersion >= 51)
		Ctx.pStream->Read((char *)&m_iRotation, sizeof(DWORD));
	else
		m_iRotation = 0;
	}

void CEffect::OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick)

//	OnUpdate
//
//	Update the effect

	{
	bool bDestroyEffect = false;

	//	See if the effect has faded out

	m_iTick++;
	if (m_iLifetime != -1 && m_iTick >= m_iLifetime)
		{
		Destroy(removedFromSystem, CDamageSource());
		return;
		}

	//	If we're moving, slow down

	SetVel(CVector(GetVel().GetX() * g_SpaceDragFactor, GetVel().GetY() * g_SpaceDragFactor));

	//	Give the painter a chance to update

	if (m_pPainter)
		{
		SEffectUpdateCtx Ctx;
		Ctx.pSystem = GetSystem();
		Ctx.pObj = this;

		Ctx.pDamageDesc = m_pPainter->GetCreator()->GetDamageDesc();
		Ctx.iTotalParticleCount = m_pPainter->GetParticleCount();
		Ctx.Attacker = CDamageSource(this);

		//	Update

		m_pPainter->OnUpdate(Ctx);

		//	See if we need to destroy ourselves

		if (Ctx.bDestroy)
			{
			Destroy(removedFromSystem, CDamageSource());
			return;
			}
		}
	}

void CEffect::OnUpdateExtended (const CTimeSpan &ExtraTime)

//	OnUpdateExtended
//
//	Update a prolonged time

	{
	//	Updates don't last for an extended period of time

	if (m_iLifetime != -1)
		Destroy(removedFromSystem, CDamageSource());
	}

void CEffect::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write the object's data to stream
//
//	DWORD		IEffectPainter
//	DWORD		m_pAnchor (CSpaceObject Ref)
//	DWORD		m_iLifetime
//	DWORD		m_iTick
//	DWORD		m_iRotation

	{
	CEffectCreator::WritePainterToStream(pStream, m_pPainter);

	WriteObjRefToStream(m_pAnchor, pStream);
	pStream->Write((char *)&m_iLifetime, sizeof(DWORD));
	pStream->Write((char *)&m_iTick, sizeof(DWORD));
	pStream->Write((char *)&m_iRotation, sizeof(DWORD));
	}

