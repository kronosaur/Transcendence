//	COverlayType.cpp
//
//	COverlayType class

#include "PreComp.h"

#define COUNTER_TAG								CONSTLIT("Counter")
#define EFFECT_WHEN_HIT_TAG						CONSTLIT("EffectWhenHit")
#define EFFECT_TAG								CONSTLIT("Effect")
#define HIT_EFFECT_TAG							CONSTLIT("HitEffect")
#define SHIP_ENERGY_FIELD_TYPE_TAG				CONSTLIT("ShipEnergyFieldType")

#define ABSORB_ADJ_ATTRIB						CONSTLIT("absorbAdj")
#define ALT_EFFECT_ATTRIB						CONSTLIT("altEffect")
#define COLOR_ATTRIB							CONSTLIT("color")
#define DISABLE_SHIP_SCREEN_ATTRIB				CONSTLIT("disableShipScreen")
#define DISARM_ATTRIB							CONSTLIT("disarm")
#define DRAG_ATTRIB								CONSTLIT("drag")
#define IGNORE_SHIP_ROTATION_ATTRIB				CONSTLIT("ignoreSourceRotation")
#define LABEL_ATTRIB							CONSTLIT("label")
#define MAX_ATTRIB								CONSTLIT("max")
#define PARALYZE_ATTRIB							CONSTLIT("paralyze")
#define SHIELD_OVERLAY_ATTRIB					CONSTLIT("shieldOverlay")
#define SPIN_ATTRIB								CONSTLIT("spin")
#define STYLE_ATTRIB							CONSTLIT("style")
#define UNID_ATTRIB								CONSTLIT("UNID")
#define BONUS_ADJ_ATTRIB						CONSTLIT("weaponBonusAdj")
#define WEAPON_SUPPRESS_ATTRIB					CONSTLIT("weaponSuppress")

#define COUNTER_PROGRESS						CONSTLIT("progress")
#define COUNTER_RADIUS							CONSTLIT("radius")

#define FIELD_WEAPON_SUPPRESS					CONSTLIT("weaponSuppress")

#define ON_UPDATE_EVENT							CONSTLIT("OnUpdate")

#define SUPPRESS_ALL							CONSTLIT("*")

COverlayType::COverlayType (void) :
		m_pEffect(NULL),
		m_pHitEffect(NULL)

//	COverlayType constructor

	{
	}

COverlayType::~COverlayType (void)

//	COverlayType destructor

	{
	if (m_pEffect)
		delete m_pEffect;

	if (m_pHitEffect)
		delete m_pHitEffect;
	}

bool COverlayType::AbsorbsWeaponFire (CInstalledDevice *pWeapon)

//	AbsorbsWeaponFire
//
//	Absorbs weapon fire from the ship

	{
	int iType = pWeapon->GetClass()->GetDamageType(pWeapon);
	if (iType != -1 && m_WeaponSuppress.InSet(iType))
		return true;
	else
		return false;
	}

bool COverlayType::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	int i;

	if (strEquals(sField, FIELD_WEAPON_SUPPRESS))
		{
		if (m_WeaponSuppress.IsEmpty())
			*retsValue = NULL_STR;
		else
			{
			*retsValue = CONSTLIT("=(");

			bool bNeedSeparator = false;
			for (i = 0; i < damageCount; i++)
				if (m_WeaponSuppress.InSet(i))
					{
					if (bNeedSeparator)
						retsValue->Append(CONSTLIT(" "));

					retsValue->Append(::GetDamageType((DamageTypes)i));
					bNeedSeparator = true;
					}

			retsValue->Append(CONSTLIT(")"));
			}
		}
	else
		return CDesignType::FindDataField(sField, retsValue);

	return true;
	}

int COverlayType::GetDamageAbsorbed (CSpaceObject *pSource, SDamageCtx &Ctx)

//	GetDamageAbsorbed
//
//	Returns the amount of damage absorbed

	{
	int i;

	if (Ctx.Damage.GetDamageType() == damageGeneric)
		{
		//	For generic damage, we absorb the min of all other damage types

		int iMin = 100;
		for (i = 0; i < damageCount; i++)
			if (m_iAbsorbAdj[i] < iMin)
				iMin = m_iAbsorbAdj[i];

		return (Ctx.iDamage * iMin) / 100;
		}

	return (Ctx.iDamage * m_iAbsorbAdj[Ctx.Damage.GetDamageType()]) / 100;
	}

int COverlayType::GetWeaponBonus (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetWeaponBonus
//
//	Returns the bonus for this weapon

	{
	int iType = pDevice->GetDamageType();
	if (iType == damageGeneric)
		return 0;

	return m_iBonusAdj[iType];
	}

void COverlayType::OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	OnAddTypesUsed
//
//	Add types used to the list.

	{
	//	LATER: m_pEffect and m_pHitEffect are always local; we should allow
	//	them to be an design type.
	}

ALERROR COverlayType::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	ALERROR error;

	if (m_pEffect)
		if (error = m_pEffect->BindDesign(Ctx))
			return error;

	if (m_pHitEffect)
		if (error = m_pHitEffect->BindDesign(Ctx))
			return error;

	return NOERROR;
	}

ALERROR COverlayType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	ALERROR error;

	//	Effect

	CXMLElement *pEffect = pDesc->GetContentElementByTag(EFFECT_TAG);
	if (pEffect)
		{
		if (error = CEffectCreator::CreateFromXML(Ctx, 
				pEffect, 
				strPatternSubst(CONSTLIT("%d:e"), GetUNID()), 
				&m_pEffect))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("energy field %x: Unable to load effect"), GetUNID());
			return error;
			}
		}

	pEffect = pDesc->GetContentElementByTag(HIT_EFFECT_TAG);
	if (pEffect == NULL)
		pEffect = pDesc->GetContentElementByTag(EFFECT_WHEN_HIT_TAG);

	if (pEffect)
		{
		if (error = CEffectCreator::CreateFromXML(Ctx, 
				pEffect, 
				strPatternSubst(CONSTLIT("%d:h"), GetUNID()), 
				&m_pHitEffect))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("energy field %x: Unable to load hit effect"), GetUNID());
			return error;
			}

		//	For compatibility with previous versions, if we're using the old
		//	<ShipEnergyFieldType> then altEffect defaults to TRUE. Otherwise, for new
		//	<OverlayType> altEffect defaults to false.

		bool bAltEffect;
		if (pEffect->FindAttributeBool(ALT_EFFECT_ATTRIB, &bAltEffect))
			m_fAltHitEffect = bAltEffect;
		else
			m_fAltHitEffect = strEquals(pDesc->GetTag(), SHIP_ENERGY_FIELD_TYPE_TAG);
		}
	else
		m_fAltHitEffect = false;

	//	Rotation

	m_fRotateWithShip = !pDesc->GetAttributeBool(IGNORE_SHIP_ROTATION_ATTRIB);

	//	Damage adjustment

	int iAbsorbCount;
	LoadDamageAdj(pDesc, ABSORB_ADJ_ATTRIB, m_iAbsorbAdj, &iAbsorbCount);

	//	Bonus adjustment

	LoadDamageAdj(pDesc, BONUS_ADJ_ATTRIB, m_iBonusAdj);

	//	Load the weapon suppress

	if (error = m_WeaponSuppress.InitFromXML(pDesc->GetAttribute(WEAPON_SUPPRESS_ATTRIB)))
		{
		Ctx.sError = CONSTLIT("Unable to load weapon suppress attribute");
		return error;
		}

	//	Keep track of the events that we have

	m_fHasOnUpdateEvent = FindEventHandler(ON_UPDATE_EVENT);

	//	Are we a field/shield overlay (or part of hull)?
	//	By default, we are a shield overlay if we absorb damage.

	bool bValue;
	if (pDesc->FindAttributeBool(SHIELD_OVERLAY_ATTRIB, &bValue))
		m_fShieldOverlay = bValue;
	else
		m_fShieldOverlay = (iAbsorbCount > 0);

	//	Counter

	CXMLElement *pCounter = pDesc->GetContentElementByTag(COUNTER_TAG);
	if (pCounter)
		{
		CString sStyle = pCounter->GetAttribute(STYLE_ATTRIB);
		if (strEquals(sStyle, COUNTER_PROGRESS))
			m_iCounterType = counterProgress;
		else if (strEquals(sStyle, COUNTER_RADIUS))
			m_iCounterType = counterRadius;
		else
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unknown counter style: %s"), sStyle);
			return ERR_FAIL;
			}

		m_sCounterLabel = pCounter->GetAttribute(LABEL_ATTRIB);
		m_iCounterMax = pCounter->GetAttributeIntegerBounded(MAX_ATTRIB, 0, -1, 100);
		m_wCounterColor = ::LoadRGBColor(pCounter->GetAttribute(COLOR_ATTRIB));
		}
	else
		{
		m_iCounterType = counterNone;
		m_iCounterMax = 0;
		m_wCounterColor = 0;
		}

	//	Options

	m_fDisarmShip = pDesc->GetAttributeBool(DISARM_ATTRIB);
	m_fParalyzeShip = pDesc->GetAttributeBool(PARALYZE_ATTRIB);
	m_fDisableShipScreen = pDesc->GetAttributeBool(DISABLE_SHIP_SCREEN_ATTRIB);
	m_fSpinShip = pDesc->GetAttributeBool(SPIN_ATTRIB);

	int iDrag;
	if (pDesc->FindAttributeInteger(DRAG_ATTRIB, &iDrag))
		m_rDrag = Min(Max(0, iDrag), 100) / 100.0;
	else
		m_rDrag = 1.0;

	//	Done

	return NOERROR;
	}

CEffectCreator *COverlayType::OnFindEffectCreator (const CString &sUNID)

//	OnFindEffectCreator
//
//	Load the proper effect creator

	{
	char *pPos = sUNID.GetASCIIZPointer();
	if (*pPos != ':')
		return NULL;

	pPos++;

	switch (*pPos)
		{
		case 'e':
			return m_pEffect;

		case 'h':
			return m_pHitEffect;

		default:
			return NULL;
		}
	}
