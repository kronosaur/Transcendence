//	CTurretClass.cpp
//
//	CTurretClass class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

#define MAX_FIRE_ARC_ATTRIB				CONSTLIT("maxFireArc")
#define MIN_FIRE_ARC_ATTRIB				CONSTLIT("minFireArc")
#define OMNIDIRECTIONAL_ATTRIB			CONSTLIT("omnidirectional")
#define POWER_USE_ATTRIB				CONSTLIT("powerUse")

#define MOUNTED_DEVICE_UNID_DATA		CONSTLIT("mountedDeviceUNID")

CTurretClass::CTurretClass (void) : CDeviceClass(NULL)

//	CTurretClass constructor

	{
	}

bool CTurretClass::Activate (CInstalledDevice *pDevice, 
					   CSpaceObject *pSource, 
					   CSpaceObject *pTarget,
					   bool *retbSourceDestroyed,
					   bool *retbConsumedItems)

//	Activate
//
//	Activate the device (i.e., fire weapon)

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		{
		pDevice->SetLastActivateSuccessful(false);
		return false;
		}

	return pWeapon->Activate(pDevice, pSource, pTarget, retbSourceDestroyed, retbConsumedItems);
	}

int CTurretClass::CalcFireSolution (CInstalledDevice *pDevice, CSpaceObject *pSource, CSpaceObject *pTarget)

//	CalcFireSolution
//
//	Computes the direction to fire to hit the target (or -1 if no solution)

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return -1;

	return pWeapon->CalcFireSolution(pDevice, pSource, pTarget);
	}

int CTurretClass::CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	CalcPowerUsed
//
//	Returns the amount of power consumed (in 1/10ths of MWs)

	{
	if (!pDevice->IsEnabled())
		return 0;

	//	Compute power used by the weapon itself

	CItemCtx Ctx(pSource, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	int iWeaponPower = (pWeapon ? pWeapon->CalcPowerUsed(pDevice, pSource) : 0);

	//	Add power consumed by turret

	return iWeaponPower + m_iPowerUse;
	}

ALERROR CTurretClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice)

//	CreateFromXML
//
//	Loads from XML

	{
	CTurretClass *pTurret;

	pTurret = new CTurretClass;
	if (pTurret == NULL)
		return ERR_MEMORY;

	pTurret->InitDeviceFromXML(Ctx, pDesc, pType);

	//	Turret parameters

	pTurret->m_iPowerUse = pDesc->GetAttributeIntegerBounded(POWER_USE_ATTRIB, 0, -1, 0);
	pTurret->m_bOmnidirectional = pDesc->GetAttributeBool(OMNIDIRECTIONAL_ATTRIB);
	pTurret->m_iMinFireArc = pDesc->GetAttributeIntegerBounded(MIN_FIRE_ARC_ATTRIB, 0, 360, 0);
	pTurret->m_iMaxFireArc = pDesc->GetAttributeIntegerBounded(MAX_FIRE_ARC_ATTRIB, 0, 360, 0);

	return NOERROR;
	}

bool CTurretClass::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns a data field

	{
	return false;
	}

bool CTurretClass::FindDataField (int iVariant, const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns a data field

	{
	return false;
	}

int CTurretClass::GetActivateDelay (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetActivateDelay
//
//	Returns the activation delay for the device in ticks

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return 0;

	return pWeapon->GetActivateDelay(pDevice, pSource);
	}

int CTurretClass::GetAmmoVariant (const CItemType *pItem) const

//	GetAmmoVariant
//
//	Returns the index of the variant for the given item

	{
	return -1;
	}

int CTurretClass::GetCounter (CInstalledDevice *pDevice, CSpaceObject *pSource, CounterTypes *retiType)

//	GetCounter
//
//	Returns the current counter value

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return 0;

	return pWeapon->GetCounter(pDevice, pSource, retiType);
	}

const DamageDesc *CTurretClass::GetDamageDesc (CItemCtx &Ctx)

//	GetDamageDesc
//
//	Returns the damage descriptor

	{
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return NULL;

	return pWeapon->GetDamageDesc(Ctx);
	}

int CTurretClass::GetDefaultFireAngle (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetDefaultFireAngle
//
//	Returns the default angle at which the device fires

	{
	if (pDevice && pSource)
		return (pSource->GetRotation() + pDevice->GetRotation() + AngleMiddle(m_iMinFireArc, m_iMaxFireArc)) % 360;
	else
		return AngleMiddle(m_iMinFireArc, m_iMaxFireArc);
	}

int CTurretClass::GetDamageType (CInstalledDevice *pDevice, int iVariant)

//	GetDamageType
//
//	Returns the damage type caused by the weapons

	{
	CItemCtx Ctx(NULL, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return damageGeneric;

	return pWeapon->GetDamageType(pDevice, iVariant);
	}

Metric CTurretClass::GetMaxEffectiveRange (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget)

//	GetMaxEffectiveRange
//
//	Returns the max effective range of the weapons

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return 0.0;

	return pWeapon->GetMaxEffectiveRange(pSource, pDevice, pTarget);
	}

int CTurretClass::GetPowerRating (CItemCtx &Ctx)

//	GetPowerRating
//
//	Returns the power rating of the device

	{
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	return m_iPowerUse + (pWeapon ? pWeapon->GetPowerRating(Ctx) : 0);
	}

CString CTurretClass::GetReference (CItemCtx &Ctx, int iVariant, DWORD dwFlags)

//	GetReference
//
//	Returns the reference description for the device

	{
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return GetReferencePower(Ctx);

	return pWeapon->GetReference(Ctx, iVariant, dwFlags);
	}

bool CTurretClass::GetReferenceDamageType (CItemCtx &Ctx, int iVariant, DamageTypes *retiDamage, CString *retsReference) const

//	GetReferenceDamageType
//
//	Returns the damage type caused by the weapons

	{
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return false;

	return pWeapon->GetReferenceDamageType(Ctx, iVariant, retiDamage, retsReference);
	}

void CTurretClass::GetSelectedVariantInfo (CSpaceObject *pSource, 
									 CInstalledDevice *pDevice,
									 CString *retsLabel,
									 int *retiAmmoLeft,
									 CItemType **retpType)

//	GetSelectedVariantInfo
//
//	Returns the info for the current variant

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return;

	return pWeapon->GetSelectedVariantInfo(pSource, pDevice, retsLabel, retiAmmoLeft, retpType);
	}

int CTurretClass::GetValidVariantCount (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	GetValidVariantCount
//
//	Returns the number of variants that are valid

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return 0;

	return pWeapon->GetValidVariantCount(pSource, pDevice);
	}

CWeaponClass *CTurretClass::GetWeapon (CItemCtx &Ctx) const

//	GetWeapon
//
//	Returns the weapons class that is installed on the turret.
//	May return NULL if no weapon is installed.

	{
	//	The device class mounted on this turret is stored in item data

	const CItem &Item = Ctx.GetItem();
	CString sUNID = Item.GetData(MOUNTED_DEVICE_UNID_DATA);
	CDeviceClass *pDevice = g_pUniverse->FindDeviceClass(strToInt(sUNID, 0));
	if (pDevice == NULL)
		return NULL;

	return pDevice->AsWeaponClass();
	}

int CTurretClass::GetWeaponEffectiveness (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget)

//	GetWeaponEffectiveness
//
//	Returns the weapon effectiveness against the given target

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return 0;

	return pWeapon->GetWeaponEffectiveness(pSource, pDevice, pTarget);
	}

bool CTurretClass::IsAreaWeapon (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	IsAreaWeapon
//
//	Returns TRUE if this is an area weapon

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return false;

	return pWeapon->IsAreaWeapon(pSource, pDevice);
	}

bool CTurretClass::IsVariantSelected (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	IsVariantSelected
//
//	Returns TRUE if a valid variant has been selected

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return false;

	return pWeapon->IsVariantSelected(pSource, pDevice);
	}

bool CTurretClass::IsWeaponAligned (CSpaceObject *pShip, CInstalledDevice *pDevice, CSpaceObject *pTarget, int *retiAimAngle, int *retiFireAngle)

//	IsWeaponAligned
//
//	Returns TRUE if the weapon is aligned

	{
	//	See what weapon is mounted on the turret

	CItemCtx Ctx(pShip, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);

	//	Get the shot data

	CWeaponFireDesc *pShot = pWeapon->GetSelectedShotData(Ctx);
	if (pWeapon == NULL || pShot == NULL)
		{
		if (retiAimAngle)
			*retiAimAngle = -1;
		if (retiFireAngle)
			*retiFireAngle = -1;
		return false;
		}

	ASSERT(pTarget);

	//	Compute source

	CVector vSource = pDevice->GetPos(pShip);

	//	We need to calculate the intercept solution.

	Metric rWeaponSpeed = pShot->GetRatedSpeed();
	CVector vTarget = pTarget->GetPos() - vSource;
	CVector vTargetVel = pTarget->GetVel() - pShip->GetVel();

	//	Figure out which direction to fire in

	Metric rDist;
	Metric rTimeToIntercept = CalcInterceptTime(vTarget, vTargetVel, rWeaponSpeed, &rDist);
	CVector vInterceptPoint = (rTimeToIntercept > 0.0 ? vTarget + vTargetVel * rTimeToIntercept : vTarget);
	int iAim = VectorToPolar(vInterceptPoint, NULL);
	if (retiAimAngle)
		*retiAimAngle = iAim;

	//	Omnidirectional weapons are always aligned

	if (m_bOmnidirectional)
		{
		if (retiFireAngle)
			*retiFireAngle = iAim;
		return true;
		}

	int iFacingAngle = (pShip->GetRotation() + pDevice->GetRotation() + AngleMiddle(m_iMinFireArc, m_iMaxFireArc)) % 360;

	//	Area weapons are always aligned

	if (pShot->m_iFireType == ftArea)
		{
		if (retiFireAngle)
			*retiFireAngle = iFacingAngle;
		return true;
		}

	//	If this is a directional weapon, figure out whether the target
	//	is in the fire arc

	int iMinFireAngle = m_iMinFireArc;
	int iMaxFireAngle = m_iMaxFireArc;
	bool bDirectional = (m_iMinFireArc != m_iMaxFireArc);

	//	Tracking weapons behave like directional weapons with 120 degree field

	if (!bDirectional && pShot->IsTracking())
		{
		iMinFireAngle = 300;
		iMaxFireAngle = 60;
		bDirectional = true;
		}

	if (bDirectional)
		{
		int iMin = (pShip->GetRotation() + iMinFireAngle) % 360;
		int iMax = (pShip->GetRotation() + iMaxFireAngle) % 360;

		//	Are we in the fire arc?

		bool bInArc;
		if (iMin < iMax)
			{
			//	e.g., iMin == 0; iMax == 180

			bInArc = (iAim >= iMin && iAim <= iMax);
			}
		else
			{
			//	e.g., iMin == 315; iMax == 45

			bInArc = (iAim >= iMin || iAim <= iMax);
			}

		//	Compute the fire direction

		if (retiFireAngle)
			{
			if (bInArc)
				*retiFireAngle = iAim;
			else if (AngleOffset(iAim, iMin) < AngleOffset(iAim, iMax))
				*retiFireAngle = iMin;
			else
				*retiFireAngle = iMax;
			}

		return bInArc;
		}

	//	Figure out our aim tolerance
	//	LATER: Get this from the weapon

	int iAimTolerance = 5;

	//	Fire angle

	if (retiFireAngle)
		*retiFireAngle = iFacingAngle;

	//	Compute the angular size of the target

	CVector vUR;
	CVector vLL;
	pTarget->GetBoundingRect(&vUR, &vLL);
	Metric rSize = (vUR.GetX() - vLL.GetX());
	int iHalfAngularSize = (int)(30 * rSize / rDist);

	//	Figure out how far off we are from the direction that we
	//	want to fire in.

	int iAimOffset = AngleOffset(iFacingAngle, iAim);

	//	If we're facing in the direction that we want to fire, 
	//	then we're aligned...

	if (iAimOffset <= Max(iAimTolerance, iHalfAngularSize))
		return true;

	//	Otherwise, we're not and we need to return the aim direction

	else
		return false;
	}

ALERROR CTurretClass::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Binds design

	{
	return NOERROR;
	}

void CTurretClass::OnInstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList)

//	OnInstall
//
//	Called when the device is installed

	{
	}

void CTurretClass::OnLoadImages (void)

//	OnLoadImages
//
//	Called when we need to load images

	{
	}

void CTurretClass::OnMarkImages (void)

//	OnMarkImages
//
//	Mark images that we need

	{
	}

void CTurretClass::OnUninstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList)

//	OnUninstall
//
//	Called when device is uninstalled

	{
	}

bool CTurretClass::RequiresItems (void)

//	RequiresItems
//
//	TRUE if device requires items

	{
	return false;
	}

bool CTurretClass::SelectFirstVariant (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	SelectFirstVariant
//
//	Selects the first variant

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return false;

	return pWeapon->SelectFirstVariant(pSource, pDevice);
	}

bool CTurretClass::SelectNextVariant (CSpaceObject *pSource, CInstalledDevice *pDevice, int iDir)

//	SelectNextVariant
//
//	Selects the next variant

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return false;

	return pWeapon->SelectNextVariant(pSource, pDevice, iDir);
	}

bool CTurretClass::ShowActivationDelayCounter (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	ShowActivationDelayCounter
//
//	Returns TRUE if we should how the activation delay counter

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return false;

	return pWeapon->ShowActivationDelayCounter(pSource, pDevice);
	}

void CTurretClass::Update (CInstalledDevice *pDevice, 
					 CSpaceObject *pSource, 
					 int iTick,
					 bool *retbSourceDestroyed,
					 bool *retbConsumedItems)

//	Update
//
//	Updates the device

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return;

	pWeapon->Update(pDevice, pSource, iTick, retbSourceDestroyed, retbConsumedItems);
	}

bool CTurretClass::ValidateSelectedVariant (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	ValidateSelectedVariant
//
//	Returns TRUE if the selected variant is valid

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponClass *pWeapon = GetWeapon(Ctx);
	if (pWeapon == NULL)
		return false;

	return pWeapon->ValidateSelectedVariant(pSource, pDevice);
	}
