//	CEnergyField.cpp
//
//	CEnergyField class

#include "PreComp.h"

#define ON_CREATE_EVENT							CONSTLIT("OnCreate")
#define ON_DAMAGE_EVENT							CONSTLIT("OnDamage")
#define ON_DESTROY_EVENT						CONSTLIT("OnDestroy")
#define ON_OBJ_DESTROYED_EVENT					CONSTLIT("OnObjDestroyed")
#define ON_UPDATE_EVENT							CONSTLIT("OnUpdate")

#define OVERLAY_RADIUS							(10.0 * g_KlicksPerPixel)
#define OVERLAY_RADIUS2							(OVERLAY_RADIUS * OVERLAY_RADIUS)

#define LIFETIME_ATTRIB							CONSTLIT("lifetime")

CEnergyField::CEnergyField (void) : 
		m_pType(NULL),
		m_pPainter(NULL),
		m_pHitPainter(NULL),
		m_fDestroyed(false),
		m_pNext(NULL)

//	CEnergyField constructor

	{
	}

CEnergyField::~CEnergyField (void)

//	CEnergyField destructor

	{
	if (m_pPainter)
		m_pPainter->Delete();

	if (m_pHitPainter)
		m_pHitPainter->Delete();
	}

bool CEnergyField::AbsorbDamage (CSpaceObject *pSource, SDamageCtx &Ctx)

//	AbsorbDamage
//
//	Absorbs damage and returns TRUE if any damage is absorbed

	{
	//	If we absorb damage, then see how much we absorbed

	int iAbsorb = m_pType->GetDamageAbsorbed(pSource, Ctx);
	if (iAbsorb > 0)
		{
		//	Handle custom damage (this is to give custom damage
		//	a chance to react to hitting an overlay)

		if (Ctx.pDesc->FireOnDamageOverlay(Ctx, this))
			return true;

		//	Absorb damage

		Ctx.iDamage -= iAbsorb;

		//	Hit effect

		CreateHitEffect(pSource, Ctx);

		//	Done

		return true;
		}

	//	Otherwise, see if the damage affects us

	else
		{
		bool bHit = false;

		//	If we have a position, then we only get hit if the shot was near us

		if (m_iPosRadius)
			{
			CVector vPos = GetPos(pSource);
			if ((Ctx.vHitPos - vPos).Length2() < OVERLAY_RADIUS2)
				bHit = true;
			}

		//	Otherwise, we always get hit

		else
			bHit = true;

		//	If we're hit, then fire an event

		if (bHit)
			{
			//	Handle custom damage (this is to give custom damage
			//	a chance to react to hitting an overlay)

			if (Ctx.pDesc->FireOnDamageOverlay(Ctx, this))
				return true;

			//	Handle damage (if we don't do anything on hit, then
			//	we let the damage through)

			if (FireOnDamage(pSource, Ctx))
				{
				CreateHitEffect(pSource, Ctx);
				return true;
				}

			//	If we're a device overlay then there is a chance that we
			//	damage the device.

			if (m_iDevice != -1
					&& mathRandom(1, 100) <= 40)
				pSource->DamageExternalDevice(m_iDevice, Ctx);
			}
		
		return bHit;
		}
	}

void CEnergyField::CreateHitEffect (CSpaceObject *pSource, SDamageCtx &Ctx)

//	CreateHitEffect
//
//	Creates the appropriate hit effect

	{
	CEffectCreator *pHitEffect = m_pType->GetHitEffectCreator();
	if (pHitEffect == NULL)
		return;

	//	If the hit effect just replaces the normal effect, create a painter
	//	and set the state

	if (m_pType->IsHitEffectAlt())
		{
		//	Create a painter

		if (m_pHitPainter)
			m_pHitPainter->Delete();
		m_pHitPainter = pHitEffect->CreatePainter(CCreatePainterCtx());

		//	Initialize

		m_iPaintHit = m_pHitPainter->GetLifetime();
		m_iPaintHitTick = 0;

		m_pHitPainter->PlaySound(pSource);
		}

	//	Otherwise, create an effect object

	else
		{
		pHitEffect->CreateEffect(pSource->GetSystem(),
				NULL,
				Ctx.vHitPos,
				pSource->GetVel(),
				Ctx.iDirection);
		}
	}

void CEnergyField::CreateFromType (COverlayType *pType, 
								   int iPosAngle,
								   int iPosRadius,
								   int iRotation,
								   int iLifeLeft, 
								   CEnergyField **retpField)

//	CreateFromType
//
//	Create field from type

	{
	ASSERT(pType);

	CEnergyField *pField = new CEnergyField;

	pField->m_pType = pType;
	pField->m_dwID = g_pUniverse->CreateGlobalID();
	pField->m_iDevice = -1;
	pField->m_iLifeLeft = iLifeLeft;
	pField->m_iPosAngle = iPosAngle;
	pField->m_iPosRadius = iPosRadius;
	pField->m_iRotation = iRotation;
	pField->m_iPaintHit = 0;

	//	Create painters

	CEffectCreator *pCreator = pType->GetEffectCreator();
	if (pCreator)
		{
		CCreatePainterCtx CreateCtx;
		CreateCtx.SetLifetime(pField->m_iLifeLeft);

		pField->m_pPainter = pCreator->CreatePainter(CreateCtx);
		}

	pField->m_pHitPainter = NULL;

	*retpField = pField;
	}

void CEnergyField::Destroy (CSpaceObject *pSource)

//	Destroy
//
//	Destroy the field

	{
	if (!m_fDestroyed)
		{
		FireOnDestroy(pSource);

		//	Mark the field as destroyed.
		//	It will be deleted in CEnergyFieldList::Update

		m_fDestroyed = true;
		}
	}

void CEnergyField::FireCustomEvent (CSpaceObject *pSource, const CString &sEvent, ICCItem **retpResult)

//	FireCustomEvent
//
//	Fires a custom event

	{
	SEventHandlerDesc Event;
	if (m_pType->FindEventHandler(sEvent, &Event))
		{
		CCodeChainCtx Ctx;

		//	Setup 

		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.DefineInteger(CONSTLIT("aOverlayID"), m_dwID);

		//	Execute

		ICCItem *pResult = Ctx.Run(Event);

		//	Done

		if (retpResult)
			*retpResult = pResult;
		else
			Ctx.Discard(pResult);
		}
	else if (retpResult)
		*retpResult = g_pUniverse->GetCC().CreateNil();
	}

void CEnergyField::FireOnCreate (CSpaceObject *pSource)

//	FireOnCreate
//
//	Fire OnCreate event

	{
	SEventHandlerDesc Event;
	if (m_pType->FindEventHandler(ON_CREATE_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		//	Setup 

		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.DefineInteger(CONSTLIT("aOverlayID"), m_dwID);

		//	Execute

		ICCItem *pResult = Ctx.Run(Event);

		//	Done

		if (pResult->IsError())
			pSource->ReportEventError(strPatternSubst(CONSTLIT("Overlay OnCreate: %s"), pResult->GetStringValue()), pResult);

		Ctx.Discard(pResult);
		}
	}

bool CEnergyField::FireOnDamage (CSpaceObject *pSource, SDamageCtx &Ctx)

//	FireOnDamage
//
//	Fire OnDamage event

	{
	SEventHandlerDesc Event;
	if (m_pType->FindEventHandler(ON_DAMAGE_EVENT, &Event))
		{
		CCodeChainCtx CCCtx;

		//	Setup 

		CCCtx.SaveAndDefineSourceVar(pSource);
		CCCtx.DefineInteger(CONSTLIT("aOverlayID"), m_dwID);
		CCCtx.DefineInteger(CONSTLIT("aArmorSeg"), Ctx.iSectHit);
		CCCtx.DefineSpaceObject(CONSTLIT("aCause"), Ctx.pCause);
		CCCtx.DefineSpaceObject(CONSTLIT("aAttacker"), Ctx.Attacker.GetObj());
		CCCtx.DefineSpaceObject(CONSTLIT("aOrderGiver"), (Ctx.Attacker.GetObj() ? Ctx.Attacker.GetObj()->GetOrderGiver(Ctx.Attacker.GetCause()) : NULL));
		CCCtx.DefineVector(CONSTLIT("aHitPos"), Ctx.vHitPos);
		CCCtx.DefineInteger(CONSTLIT("aHitDir"), Ctx.iDirection);
		CCCtx.DefineInteger(CONSTLIT("aDamageHP"), Ctx.iDamage);
		CCCtx.DefineString(CONSTLIT("aDamageType"), GetDamageShortName(Ctx.Damage.GetDamageType()));
		CCCtx.DefineItemType(CONSTLIT("aWeaponType"), Ctx.pDesc->GetWeaponType());

		//	Execute

		ICCItem *pResult = CCCtx.Run(Event);

		//	Check for error

		if (pResult->IsError())
			pSource->ReportEventError(strPatternSubst(CONSTLIT("Overlay OnDamage: %s"), pResult->GetStringValue()), pResult);

		//	If we don't return Nil, then the result is the damage that passes to the source

		bool bHandled = false;
		if (!pResult->IsNil())
			{
			Ctx.iDamage = pResult->GetIntegerValue();
			bHandled = true;
			}

		//	Done

		CCCtx.Discard(pResult);
		return bHandled;
		}
	else
		return false;
	}

void CEnergyField::FireOnDestroy (CSpaceObject *pSource)

//	FireOnDestroy
//
//	Fire OnDestroy event

	{
	SEventHandlerDesc Event;
	if (m_pType->FindEventHandler(ON_DESTROY_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		//	Setup 

		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.DefineInteger(CONSTLIT("aOverlayID"), m_dwID);

		//	Execute

		ICCItem *pResult = Ctx.Run(Event);

		//	Done

		if (pResult->IsError())
			pSource->ReportEventError(strPatternSubst(CONSTLIT("Overlay OnDestroy: %s"), pResult->GetStringValue()), pResult);

		Ctx.Discard(pResult);
		}
	}

void CEnergyField::FireOnObjDestroyed (CSpaceObject *pSource, const SDestroyCtx &Ctx) const

//	FireOnObjDestroyed
//
//	OnObjDestroyed event

	{
	SEventHandlerDesc Event;
	if (m_pType->FindEventHandler(ON_OBJ_DESTROYED_EVENT, &Event))
		{
		CCodeChainCtx CCCtx;

		//	Setup

		CCCtx.SaveAndDefineSourceVar(pSource);
		CCCtx.DefineInteger(CONSTLIT("aOverlayID"), m_dwID);
		CCCtx.DefineSpaceObject(CONSTLIT("aObjDestroyed"), Ctx.pObj);
		CCCtx.DefineSpaceObject(CONSTLIT("aDestroyer"), Ctx.Attacker.GetObj());
		CCCtx.DefineSpaceObject(CONSTLIT("aOrderGiver"), (Ctx.Attacker.GetObj() ? Ctx.Attacker.GetObj()->GetOrderGiver(Ctx.iCause) : NULL));
		CCCtx.DefineSpaceObject(CONSTLIT("aWreckObj"), Ctx.pWreck);
		CCCtx.DefineString(CONSTLIT("aDestroyReason"), GetDestructionName(Ctx.iCause));

		//	Execute

		ICCItem *pResult = CCCtx.Run(Event);

		//	Done

		if (pResult->IsError())
			pSource->ReportEventError(strPatternSubst(CONSTLIT("Overlay OnObjDestroyed: %s"), pResult->GetStringValue()), pResult);

		CCCtx.Discard(pResult);
		}
	}

void CEnergyField::FireOnUpdate (CSpaceObject *pSource)

//	FireOnUpdate
//
//	Fire OnUpdate event (once per 15 ticks)

	{
	SEventHandlerDesc Event;
	if (m_pType->FindEventHandler(ON_UPDATE_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		//	Setup 

		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.DefineInteger(CONSTLIT("aOverlayID"), m_dwID);

		//	Execute

		ICCItem *pResult = Ctx.Run(Event);

		//	Done

		if (pResult->IsError())
			pSource->ReportEventError(strPatternSubst(CONSTLIT("Overlay OnUpdate: %s"), pResult->GetStringValue()), pResult);

		Ctx.Discard(pResult);
		}
	}

CVector CEnergyField::GetPos (CSpaceObject *pSource)

//	GetPos
//
//	Returns the current absolute position of the overlay

	{
	if (m_iPosRadius)
		{
		int iRotationOrigin = (m_pType->RotatesWithShip() ? pSource->GetRotation() : 0);
		return pSource->GetPos() + PolarToVector(iRotationOrigin + m_iPosAngle, (Metric)m_iPosRadius * g_KlicksPerPixel);
		}
	else
		return pSource->GetPos();
	}

void CEnergyField::Paint (CG16bitImage &Dest, int iScale, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint the field

	{
	//	Do we rotate with the source?

	int iRotationOrigin = (m_pType->RotatesWithShip() ? Ctx.iRotation : 0);

	//	Adjust position, if necessary

	if (m_iPosRadius)
		{
		int xOffset, yOffset;
		C3DConversion::CalcCoord(iScale, iRotationOrigin + m_iPosAngle, m_iPosRadius, 0, &xOffset, &yOffset);

		x = x + xOffset;
		y = y + yOffset;
		}

	//	Adjust rotation

	int iSavedRotation = Ctx.iRotation;
	Ctx.iRotation = (iRotationOrigin + m_iRotation) % 360;

	//	Paint

	if (m_iPaintHit > 0 && m_pHitPainter)
		{
		//	Paint hit effect

		int iSavedTick = Ctx.iTick;
		Ctx.iTick = m_iPaintHitTick;

		m_pHitPainter->Paint(Dest, x, y, Ctx);

		Ctx.iTick = iSavedTick;
		}
	else if (m_pPainter)
		m_pPainter->Paint(Dest, x, y, Ctx);

	//	Done

	Ctx.iRotation = iSavedRotation;
	}

void CEnergyField::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	DWORD	UNID of energy field type
//	DWORD	m_dwID
//	DWORD	m_iPosAngle
//	DWORD	m_iPosRadius
//	DWORD	m_iRotation
//	DWORD	m_iDevice
//	DWORD	Life left
//	CAttributeDataBlock m_Data
//	IPainter	m_pPaint
//	IPainter	m_pHitPainter
//	DWORD	Flags

	{
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_pType = g_pUniverse->FindShipEnergyFieldType(dwLoad);
	if (m_pType == NULL)
		{
		kernelDebugLogMessage("Unable to find overlay type: %x", dwLoad);
		throw CException(ERR_FAIL);
		}

	if (Ctx.dwVersion >= 38)
		{
		Ctx.pStream->Read((char *)&m_dwID, sizeof(DWORD));
		Ctx.pStream->Read((char *)&m_iPosAngle, sizeof(DWORD));
		Ctx.pStream->Read((char *)&m_iPosRadius, sizeof(DWORD));
		Ctx.pStream->Read((char *)&m_iRotation, sizeof(DWORD));
		}
	else
		{
		m_dwID = g_pUniverse->CreateGlobalID();
		m_iPosAngle = 0;
		m_iPosRadius = 0;
		m_iRotation = 0;
		}

	if (Ctx.dwVersion >= 63)
		Ctx.pStream->Read((char *)&m_iDevice, sizeof(DWORD));
	else
		m_iDevice = -1;

	Ctx.pStream->Read((char *)&m_iLifeLeft, sizeof(DWORD));

	if (Ctx.dwVersion >= 39)
		m_Data.ReadFromStream(Ctx);

	m_pPainter = CEffectCreator::CreatePainterFromStreamAndCreator(Ctx, m_pType->GetEffectCreator());
	m_pHitPainter = CEffectCreator::CreatePainterFromStreamAndCreator(Ctx, m_pType->GetHitEffectCreator());

	m_iPaintHit = 0;

	//	Flags

	DWORD dwFlags = 0;
	if (Ctx.dwVersion >= 101)
		Ctx.pStream->Read((char *)&dwFlags, sizeof(DWORD));

	m_fDestroyed = ((dwFlags & 0x00000001) ? true : false);
	}

bool CEnergyField::SetEffectProperty (const CString &sProperty, ICCItem *pValue)

//	SetEffectProperty
//
//	Sets a property on the effect

	{
	if (m_pPainter)
		return m_pPainter->SetProperty(sProperty, pValue);

	return false;
	}

void CEnergyField::SetPos (CSpaceObject *pSource, const CVector &vPos)

//	SetPos
//
//	Sets the position of the overlay

	{
	Metric rRadius;
	int iDirection = VectorToPolar(vPos - pSource->GetPos(), &rRadius);
	int iRotationOrigin = (m_pType->RotatesWithShip() ? pSource->GetRotation() : 0);
	m_iPosAngle = (iDirection + 360 - iRotationOrigin) % 360;
	m_iPosRadius = (int)(rRadius / g_KlicksPerPixel);
	}

void CEnergyField::Update (CSpaceObject *pSource)

//	Update
//
//	Update the field

	{
	//	See if our lifetime has expired

	if (m_iLifeLeft != -1 && --m_iLifeLeft == 0)
		{
		Destroy(pSource);
		return;
		}

	//	Update paint hit counter

	if (m_iPaintHit > 0)
		{
		m_iPaintHit--;
		m_iPaintHitTick++;

		if (m_iPaintHit == 0 && m_pHitPainter)
			{
			m_pHitPainter->Delete();
			m_pHitPainter = NULL;
			}
		}

	//	Update the painters

	SEffectUpdateCtx UpdateCtx;
	UpdateCtx.pSystem = pSource->GetSystem();
	UpdateCtx.pObj = pSource;

	SEffectMoveCtx MoveCtx;
	MoveCtx.pObj = pSource;
	
	if (m_pPainter)
		{
		m_pPainter->OnUpdate(UpdateCtx);
		m_pPainter->OnMove(MoveCtx);
		}

	if (m_pHitPainter)
		{
		m_pHitPainter->OnUpdate(UpdateCtx);
		m_pHitPainter->OnMove(MoveCtx);
		}

	//	Call OnUpdate

	if (m_pType->HasOnUpdateEvent() 
			&& pSource->IsDestinyTime(OVERLAY_ON_UPDATE_CYCLE, OVERLAY_ON_UPDATE_OFFSET))
		{
		FireOnUpdate(pSource);
		}
	}

void CEnergyField::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	DWORD	UNID of energy field type
//	DWORD	m_dwID
//	DWORD	m_iPosAngle
//	DWORD	m_iPosRadius
//	DWORD	m_iRotation
//	DWORD	m_iDevice
//	DWORD	Life left
//	CAttributeDataBlock m_Data
//	IPainter	m_pPaint
//	IPainter	m_pHitPainter
//	DWORD	Flags

	{
	DWORD dwSave = m_pType->GetUNID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&m_dwID, sizeof(DWORD));
	pStream->Write((char *)&m_iPosAngle, sizeof(DWORD));
	pStream->Write((char *)&m_iPosRadius, sizeof(DWORD));
	pStream->Write((char *)&m_iRotation, sizeof(DWORD));
	pStream->Write((char *)&m_iDevice, sizeof(DWORD));
	pStream->Write((char *)&m_iLifeLeft, sizeof(DWORD));

	m_Data.WriteToStream(pStream);

	CEffectCreator::WritePainterToStream(pStream, m_pPainter);
	CEffectCreator::WritePainterToStream(pStream, m_pHitPainter);

	DWORD dwFlags = 0;
	dwFlags |= (m_fDestroyed ? 0x00000001 : 0);
	pStream->Write((char *)&dwFlags, sizeof(DWORD));
	}

