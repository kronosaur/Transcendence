//	CEnergyFieldType.cpp
//
//	CEnergyFieldType class

#include "PreComp.h"

#define EFFECT_WHEN_HIT_TAG						CONSTLIT("EffectWhenHit")
#define EFFECT_TAG								CONSTLIT("Effect")
#define HIT_EFFECT_TAG							CONSTLIT("HitEffect")
#define SHIP_ENERGY_FIELD_TYPE_TAG				CONSTLIT("ShipEnergyFieldType")

#define ABSORB_ADJ_ATTRIB						CONSTLIT("absorbAdj")
#define ALT_EFFECT_ATTRIB						CONSTLIT("altEffect")
#define IGNORE_SHIP_ROTATION_ATTRIB				CONSTLIT("ignoreSourceRotation")
#define UNID_ATTRIB								CONSTLIT("UNID")
#define BONUS_ADJ_ATTRIB						CONSTLIT("weaponBonusAdj")
#define WEAPON_SUPPRESS_ATTRIB					CONSTLIT("weaponSuppress")

#define ON_UPDATE_EVENT							CONSTLIT("OnUpdate")

#define SUPPRESS_ALL							CONSTLIT("*")

CEnergyFieldType::CEnergyFieldType (void) : 
		m_pEffect(NULL),
		m_pHitEffect(NULL)

//	CEnergyFieldType constructor

	{
	}

CEnergyFieldType::~CEnergyFieldType (void)

//	CEnergyFieldType destructor

	{
	if (m_pEffect)
		delete m_pEffect;

	if (m_pHitEffect)
		delete m_pHitEffect;
	}

bool CEnergyFieldType::AbsorbsWeaponFire (CInstalledDevice *pWeapon)

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

int CEnergyFieldType::GetDamageAbsorbed (CSpaceObject *pSource, SDamageCtx &Ctx)

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

int CEnergyFieldType::GetWeaponBonus (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetWeaponBonus
//
//	Returns the bonus for this weapon

	{
	int iType = pDevice->GetDamageType();
	if (iType == damageGeneric)
		return 0;

	return m_iBonusAdj[iType];
	}

void CEnergyFieldType::OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	OnAddTypesUsed
//
//	Add types used to the list.

	{
	//	LATER: m_pEffect and m_pHitEffect are always local; we should allow
	//	them to be an design type.
	}

ALERROR CEnergyFieldType::OnBindDesign (SDesignLoadCtx &Ctx)

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

ALERROR CEnergyFieldType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	ALERROR error;
	int i;

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
			m_bAltHitEffect = bAltEffect;
		else
			m_bAltHitEffect = strEquals(pDesc->GetTag(), SHIP_ENERGY_FIELD_TYPE_TAG);
		}
	else
		m_bAltHitEffect = false;

	//	Rotation

	m_bRotateWithShip = !pDesc->GetAttributeBool(IGNORE_SHIP_ROTATION_ATTRIB);

	//	Damage adjustment

	LoadDamageAdj(pDesc, ABSORB_ADJ_ATTRIB, m_iAbsorbAdj);

	//	Bonus adjustment

	LoadDamageAdj(pDesc, BONUS_ADJ_ATTRIB, m_iBonusAdj);

	//	Load the weapon suppress

	CString sSuppress = pDesc->GetAttribute(WEAPON_SUPPRESS_ATTRIB);
	if (strEquals(sSuppress, SUPPRESS_ALL))
		{
		for (i = 0; i < damageCount; i++)
			m_WeaponSuppress.Add(i);
		}
	else
		{
		TArray<CString> WeaponSuppress;
		if (error = strDelimit(sSuppress,
				';',
				0,
				&WeaponSuppress))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("energy field %x: Unable to load weapon suppress attribute"), GetUNID());
			return error;
			}

		for (i = 0; i < WeaponSuppress.GetCount(); i++)
			{
			CString sType = strTrimWhitespace(WeaponSuppress[i]);
			int iType = LoadDamageTypeFromXML(sType);
			if (iType == damageError)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("energy field %x: Unable to load weapon suppress attribute"), GetUNID());
				return ERR_FAIL;
				}

			m_WeaponSuppress.Add(iType);
			}
		}

	//	Keep track of the events that we have

	m_bHasOnUpdateEvent = FindEventHandler(ON_UPDATE_EVENT);

	//	Done

	return NOERROR;
	}

CEffectCreator *CEnergyFieldType::OnFindEffectCreator (const CString &sUNID)

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
