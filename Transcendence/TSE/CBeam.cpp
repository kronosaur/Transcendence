//	CBeam.cpp
//
//	CBeam class

#include "PreComp.h"

struct SBeamDrawCtx
	{
	int xFrom;
	int yFrom;
	int xTo;
	int yTo;

	int iIntensity;
	WORD wBackgroundColor;
	WORD wPrimaryColor;
	WORD wSecondaryColor;
	};

static CObjectClass<CBeam>g_Class(OBJID_CBEAM, NULL);

void DrawLaserBeam (CG16bitImage &Dest, const SBeamDrawCtx &Ctx);

CBeam::CBeam (void) : CSpaceObject(&g_Class),
//		m_xPaintFrom(0),
//		m_yPaintFrom(0),
		m_pHit(NULL),
		m_dwSpareFlags(0),
		m_iBonus(0)

//	CBeam constructor

	{
	}

void CBeam::ComputeOffsets (void)

//	ComputeOffsets
//
//	Computes offsets

	{
	Metric rLength = LIGHT_SPEED * g_SecondsPerUpdate / g_KlicksPerPixel;
	CVector vFrom = PolarToVector(m_iRotation, -rLength);

	m_xFromOffset = (int)(vFrom.GetX() + 0.5);
	m_yFromOffset = -(int)(vFrom.GetY() + 0.5);
	m_xToOffset = 0;
	m_yToOffset = 0;
	}

void CBeam::CreateReflection (const CVector &vPos, int iDirection)

//	CreateReflection
//
//	Creates a new object that is a clone of this one but moving
//	in the given direction.

	{
	}

CString CBeam::GetName (DWORD *retdwFlags)

//	GetName
//
//	This is only used if the source has been destroyed

	{
	if (retdwFlags)
		*retdwFlags = 0;

	switch (m_pDesc->m_Damage.GetDamageType())
		{
		case damageLaser:
			return CONSTLIT("laser blast");

		case damageParticle:
			return CONSTLIT("particle beam");

		case damageIonRadiation:
			return CONSTLIT("ion beam");

		case damagePositron:
			return CONSTLIT("positron beam");

		case damageGravitonBeam:
			return CONSTLIT("graviton beam");

		default:
			return CONSTLIT("beam"); 
		}
	}

void CBeam::OnMove (const CVector &vOldPos, Metric rSeconds)

//	OnMove
//
//	Move our points

	{
	//	See if the beam hit anything after the move

	if (!m_fReflection || m_iTick > 1)
		m_pHit = HitTest(vOldPos, 0.0, m_pDesc->m_Damage, &m_vPaintTo, &m_iHitDir);

	if (m_pHit == NULL)
		m_vPaintTo = GetPos();
	}

void CBeam::ObjectDestroyedHook (const SDestroyCtx &Ctx)

//	ObjectDestroyedHook
//
//	Handle it when another object is destroyed

	{
	//	If our source is destroyed, clear it

	m_Source.OnObjDestroyed(Ctx.pObj);

	if (Ctx.pObj == m_pHit)
		m_pHit = NULL;
	}

void CBeam::OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paint the beam

	{
	//	Setup structure

	SBeamDrawCtx BeamCtx;
	BeamCtx.xFrom = x + m_xFromOffset;
	BeamCtx.yFrom = y + m_yFromOffset;
	if (m_pHit)
		Ctx.XForm.Transform(m_vPaintTo, &BeamCtx.xTo, &BeamCtx.yTo);
	else
		{
		BeamCtx.xTo = x + m_xToOffset;
		BeamCtx.yTo = y + m_yToOffset;
		}
	BeamCtx.iIntensity = 1;
	BeamCtx.wBackgroundColor = Ctx.wSpaceColor;
	BeamCtx.wPrimaryColor = CG16bitImage::RGBValue(0xf1, 0x5f, 0x2a);
	BeamCtx.wSecondaryColor = CG16bitImage::RGBValue(0xff, 0x00, 0x00);

	//	Paint a line

	DrawLaserBeam(Dest, BeamCtx);

	//	Draw the head of the beam if we have an image

	if (m_pDesc->m_Image.IsLoaded())
		{
		m_pDesc->m_Image.PaintImage(Dest,
				BeamCtx.xTo,
				BeamCtx.yTo,
				m_iTick,
				0);
		}
	}

void CBeam::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read object data from a stream
//
//	CString		CWeaponFireDesc UNID
//	DWORD		m_iBonus
//	DWORD		m_iCause
//	DWORD		m_iRotation
//	Vector		m_vPaintTo
//	DWORD		m_iTick
//	DWORD		m_iLifeLeft
//	DWORD		m_Source (CSpaceObject Ref)
//	DWORD		m_pSovereign (UNID)
//	DWORD		m_pHit (CSpaceObject Ref)
//	DWORD		m_iHitDir

	{
	DWORD dwLoad;

#ifdef DEBUG_LOAD
	::OutputDebugString("CBeam::OnReadFromStream\n");
#endif
	//	Load descriptor

	CString sDescUNID;
	sDescUNID.ReadFromStream(Ctx.pStream);
	m_pDesc = g_pUniverse->FindWeaponFireDesc(sDescUNID);

	Ctx.pStream->Read((char *)&m_iBonus, sizeof(DWORD));
	if (Ctx.dwVersion >= 18)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_iCause = (DestructionTypes)dwLoad;
		}
	else
		m_iCause = killedByDamage;

	Ctx.pStream->Read((char *)&m_iRotation, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_vPaintTo, sizeof(CVector));
	Ctx.pStream->Read((char *)&m_iTick, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iLifeLeft, sizeof(m_iLifeLeft));
	m_Source.ReadFromStream(Ctx);
	CSystem::ReadSovereignRefFromStream(Ctx, &m_pSovereign);
	CSystem::ReadObjRefFromStream(Ctx, &m_pHit);
	Ctx.pStream->Read((char *)&m_iHitDir, sizeof(DWORD));

	ComputeOffsets();
	}

void CBeam::OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick)

//	OnUpdate
//
//	Update the beam

	{
	bool bDestroy = false;

	m_iTick++;

	//	See if the beam hit anything

	if (m_pHit)
		{
		//	Tell the object hit that it has been damaged

		SDamageCtx Ctx;
		Ctx.pObj = m_pHit;
		Ctx.pDesc = m_pDesc;
		Ctx.Damage = m_pDesc->m_Damage;
		Ctx.Damage.AddBonus(m_iBonus);
		Ctx.Damage.SetCause(m_iCause);
		if (IsAutomatedWeapon())
			Ctx.Damage.SetAutomatedWeapon();
		Ctx.iDirection = (m_iHitDir + 360 + mathRandom(0, 30) - 15) % 360;
		Ctx.vHitPos = m_vPaintTo;
		Ctx.pCause = this;
		Ctx.Attacker = m_Source;

		EDamageResults result = m_pHit->Damage(Ctx);

		//	Set the beam to destroy itself after a hit

		if (m_pDesc->GetPassthrough() == 0
				|| result == damageNoDamage 
				|| result == damageAbsorbedByShields
				|| mathRandom(1, 100) > m_pDesc->GetPassthrough())
			bDestroy = true;
		}

	//	See if the beam has faded out

	if (bDestroy || --m_iLifeLeft <= 0)
		Destroy(removedFromSystem, CDamageSource());
	}

void CBeam::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write the object's data to stream
//
//	CString		CWeaponFireDesc UNID
//	DWORD		m_iBonus
//	DWORD		m_iCause
//	DWORD		m_iRotation
//	Vector		m_vPaintTo
//	DWORD		m_iTick
//	DWORD		m_iLifeLeft
//	DWORD		m_Source (CSpaceObject Ref)
//	DWORD		m_pSovereign (UNID)
//	DWORD		m_pHit (CSpaceObject Ref)
//	DWORD		m_iHitDir

	{
	DWORD dwSave;
	m_pDesc->m_sUNID.WriteToStream(pStream);
	pStream->Write((char *)&m_iBonus, sizeof(DWORD));
	dwSave = m_iCause;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&m_iRotation, sizeof(DWORD));
	pStream->Write((char *)&m_vPaintTo, sizeof(m_vPaintTo));
	pStream->Write((char *)&m_iTick, sizeof(DWORD));
	pStream->Write((char *)&m_iLifeLeft, sizeof(m_iLifeLeft));
	m_Source.WriteToStream(GetSystem(), pStream);
	GetSystem()->WriteSovereignRefToStream(m_pSovereign, pStream);
	WriteObjRefToStream(m_pHit, pStream);
	pStream->Write((char *)&m_iHitDir, sizeof(DWORD));
	}

//	Beam Drawing Routines -----------------------------------------------------

void DrawLaserBeam (CG16bitImage &Dest, const SBeamDrawCtx &Ctx)

//	DrawLaserBeam
//
//	Draws a simple laser beam

	{
	WORD wGlow = CG16bitImage::BlendPixel(Ctx.wBackgroundColor, Ctx.wSecondaryColor, 100);

	Dest.DrawLine(Ctx.xFrom, Ctx.yFrom,
			Ctx.xTo, Ctx.yTo,
			3,
			wGlow);

	Dest.DrawLine(Ctx.xFrom, Ctx.yFrom,
			Ctx.xTo, Ctx.yTo,
			1,
			Ctx.wPrimaryColor);
	}
