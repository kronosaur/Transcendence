//	CWeaponClass.cpp
//
//	CWeaponClass class

#include "PreComp.h"

#define MISSILES_TAG							CONSTLIT("Missiles")
#define CONFIGURATION_TAG						CONSTLIT("Configuration")

#define AIM_TOLERANCE_ATTRIB					CONSTLIT("aimTolerance")
#define ALTERNATING_ATTRIB						CONSTLIT("alternating")
#define AMMO_ID_ATTRIB							CONSTLIT("ammoID")
#define ANGLE_ATTRIB							CONSTLIT("angle")
#define CHARGES_ATTRIB							CONSTLIT("charges")
#define CONFIGURATION_ATTRIB					CONSTLIT("configuration")
#define COOLING_RATE_ATTRIB						CONSTLIT("coolingRate")
#define COUNTER_ATTRIB							CONSTLIT("counter")
#define COUNTER_ACTIVATE_ATTRIB					CONSTLIT("counterActivate")
#define COUNTER_UPDATE_ATTRIB					CONSTLIT("counterUpdate")
#define COUNTER_UPDATE_RATE_ATTRIB				CONSTLIT("counterUpdateRate")
#define FAILURE_CHANCE_ATTRIB					CONSTLIT("failureChance")
#define HEATING_ATTRIB							CONSTLIT("heating")
#define IDLE_POWER_USE_ATTRIB					CONSTLIT("idlePowerUse")
#define LAUNCHER_ATTRIB							CONSTLIT("launcher")
#define LINKED_FIRE_ATTRIB						CONSTLIT("linkedFire")
#define MAX_FIRE_ARC_ATTRIB						CONSTLIT("maxFireArc")
#define MIN_FIRE_ARC_ATTRIB						CONSTLIT("minFireArc")
#define MULTI_TARGET_ATTRIB						CONSTLIT("multiTarget")
#define POS_ANGLE_ATTRIB						CONSTLIT("posAngle")
#define POS_RADIUS_ATTRIB						CONSTLIT("posRadius")
#define POWER_USE_ATTRIB						CONSTLIT("powerUse")
#define RECOIL_ATTRIB							CONSTLIT("recoil")
#define REPORT_AMMO_ATTRIB						CONSTLIT("reportAmmo")
#define TARGET_STATIONS_ONLY_ATTRIB				CONSTLIT("targetStationsOnly")
#define TYPE_ATTRIB								CONSTLIT("type")

#define CONFIG_TYPE_DUAL						CONSTLIT("dual")
#define CONFIG_TYPE_WALL						CONSTLIT("wall")
#define CONFIG_TYPE_SPREAD2						CONSTLIT("spread2")
#define CONFIG_TYPE_SPREAD3						CONSTLIT("spread3")
#define CONFIG_TYPE_SPREAD5						CONSTLIT("spread5")
#define CONFIG_TYPE_DUAL_ALTERNATING			CONSTLIT("alternating")

#define COUNTER_TYPE_TEMPERATURE				CONSTLIT("temperature")
#define COUNTER_TYPE_CAPACITOR					CONSTLIT("capacitor")

#define ON_FIRE_WEAPON_EVENT					CONSTLIT("OnFireWeapon")

#define FIELD_AMMO_TYPE							CONSTLIT("ammoType")
#define FIELD_AVERAGE_DAMAGE					CONSTLIT("averageDamage")	//	Average damage (1000x hp)
#define FIELD_BALANCE							CONSTLIT("balance")
#define FIELD_BALANCE_DAMAGE					CONSTLIT("balanceDamage")	//	Damage that a balanced weapon of this type/level should do
#define FIELD_CONFIGURATION						CONSTLIT("configuration")
#define FIELD_DAMAGE_180						CONSTLIT("damage")			//	HP damage per 180 ticks
#define FIELD_DAMAGE_TYPE						CONSTLIT("damageType")
#define FIELD_HP								CONSTLIT("hp")
#define FIELD_IS_ALTERNATING					CONSTLIT("isAlternating")
#define FIELD_MAX_DAMAGE						CONSTLIT("maxDamage")
#define FIELD_MIN_DAMAGE						CONSTLIT("minDamage")
#define FIELD_FIRE_DELAY						CONSTLIT("fireDelay")		//	Delay (ticks)
#define FIELD_FIRE_RATE							CONSTLIT("fireRate")
#define FIELD_POWER								CONSTLIT("power")
#define FIELD_POWER_PER_SHOT					CONSTLIT("powerPerShot")	//	Power used per shot (1000x Megawatt minutes)
#define FIELD_RANGE								CONSTLIT("range")
#define FIELD_RECOIL							CONSTLIT("recoil")
#define FIELD_REPEAT_COUNT						CONSTLIT("repeatCount")
#define FIELD_SPEED								CONSTLIT("speed")
#define FIELD_VARIANT_COUNT						CONSTLIT("variantCount")

#define PROPERTY_DAMAGED						CONSTLIT("damaged")
#define PROPERTY_FIRE_ARC						CONSTLIT("fireArc")
#define PROPERTY_LINKED_FIRE_OPTIONS			CONSTLIT("linkedFireOptions")
#define PROPERTY_OMNIDIRECTIONAL				CONSTLIT("omnidirectional")
#define PROPERTY_SECONDARY						CONSTLIT("secondary")

static CObjectClass<CWeaponClass>g_Class(OBJID_CWEAPONCLASS, NULL);

const int MAX_SHOT_COUNT =				100;

static char g_FireRateAttrib[] = "fireRate";
static char g_OmnidirectionalAttrib[] = "omnidirectional";

const Metric g_DualShotSeparation =		12;					//	Radius of dual shot (pixels)
const int TEMP_DECREASE =				-1;					//	Decrease in temp per cooling rate
const int MAX_TEMP =					120;				//	Max temperature
const int OVERHEAT_TEMP =				100;				//	Weapon overheats
const Metric MAX_TARGET_RANGE =			(24.0 * LIGHT_SECOND);
const int MAX_COUNTER =					100;

const Metric STD_FIRE_RATE_SECS =		15.0;				//	Standard fire rate (secs)

struct SStdWeaponStats
	{
	int iDamage;								//	Average damage at this level
	int iPower;									//	Power (in tenths of MWs)

	int iOverTierAdj;							//	% extra damage if this number of levels above damage type level
	int iUnderTierAdj;							//	% extra damage if this number of levels below damage type level
	};

static SStdWeaponStats STD_WEAPON_STATS[MAX_ITEM_LEVEL] =
	{
		//	Damage	Power	Over	Under
		{	4,		10,		100,	100, },
		{	5,		20,		110,	95, },
		{	7,		50,		125,	80, },
		{	9,		100,	150,	65, },
		{	12,		200,	200,	53, },
			
		{	16,		300,	250,	44, },
		{	21,		500,	250,	40, },
		{	27,		1000,	250,	40, },
		{	35,		2000,	250,	40, },
		{	46,		3000,	250,	40, },
			
		{	60,		4000,	250,	40, },
		{	78,		6000,	250,	40, },
		{	101,	8000,	250,	40, },
		{	131,	10000,	250,	40, },
		{	170,	12000,	250,	40, },
			
		{	221,	15000,	250,	40, },
		{	287,	20000,	250,	40, },
		{	373,	25000,	250,	40, },
		{	485,	30000,	250,	40, },
		{	631,	35000,	250,	40, },
			
		{	820,	40000,	250,	40, },
		{	1066,	50000,	250,	40, },
		{	1386,	60000,	250,	40, },
		{	1802,	70000,	250,	40, },
		{	2343,	80000,	250,	40, },
	};

static char *CACHED_EVENTS[CWeaponClass::evtCount] =
	{
		"OnFireWeapon",
	};

inline SStdWeaponStats *GetStdWeaponStats (int iLevel)
	{
	if (iLevel >= 1 && iLevel <= MAX_ITEM_LEVEL)
		return &STD_WEAPON_STATS[iLevel - 1];
	else
		return NULL;
	}

CWeaponClass::CWeaponClass (void) : CDeviceClass(&g_Class),
		m_pConfig(NULL),
		m_bConfigAlternating(false)
	{
	}

CWeaponClass::~CWeaponClass (void)
	{
	int i;

	for (i = 0; i < m_ShotData.GetCount(); i++)
		if (m_ShotData[i].bOwned)
			delete m_ShotData[i].pDesc;

	if (m_pConfig)
		delete [] m_pConfig;
	}

bool CWeaponClass::Activate (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 CSpaceObject *pTarget,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems)

//	Activate
//
//	Activates the device (in this case, fires the weapon)

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponFireDesc *pShot = GetSelectedShotData(Ctx);

	if (retbSourceDestroyed)
		*retbSourceDestroyed = false;
	if (retbConsumedItems)
		*retbConsumedItems = false;

	//	If not enabled, no firing

	if (pShot == NULL || !pDevice->IsEnabled())
		{
		pDevice->SetLastActivateSuccessful(false);
		return false;
		}

	//	Fire

	bool bSourceDestroyed;

	//	Fire the weapon

	bool bSuccess = FireWeapon(pDevice, pShot, pSource, pTarget, 0, &bSourceDestroyed, retbConsumedItems);

	//	If firing the weapon destroyed the ship, then we bail out

	if (bSourceDestroyed)
		{
		if (retbSourceDestroyed)
			*retbSourceDestroyed = true;
		return false;
		}

	//	Keep track of whether we succeeded or not so that we know whether to consume power

	pDevice->SetLastActivateSuccessful(bSuccess);

	//	If we did not succeed, then we're done

	if (!bSuccess)
		return false;

	//	If this is a continuous fire weapon then set the device data
	//	We set to -1 because we skip the first Update after the call
	//	to Activate (since it happens on the same tick)

	if (pShot->m_iContinuous > 0)
		SetContinuousFire(pDevice, CONTINUOUS_START);

	//	Player-specific code

	if (pSource->IsPlayer())
		{
		//	Track statistics for the player

		CShip *pShip = pSource->AsShip();
		if (pShip)
			{
			CItem WeaponItem(GetItemType(), 1);
			pShip->GetController()->OnItemFired(WeaponItem);

			if ((m_bLauncher || m_bReportAmmo) && pShot->m_pAmmoType)
				{
				CItem AmmoItem(pShot->m_pAmmoType, 1);
				pShip->GetController()->OnItemFired(AmmoItem);
				}
			}

		//	Identify the weapon

		GetItemType()->SetKnown();
		}

	//	Consume power

	return true;
	}

int CWeaponClass::CalcBalance (int iVariant)

//	CalcBalance
//
//	Calculates weapon balance relative to level. +100 is 100% overpowered

	{
	int iBalance = 0;

	//	Get the standard stats for the level

	SStdWeaponStats *pStd = GetStdWeaponStats(GetLevel());
	if (pStd == NULL)
		return 0;

	//	Get the variant data

	if (iVariant < 0 || iVariant >= m_ShotData.GetCount())
		return 0;

	CWeaponFireDesc *pShot = GetVariant(iVariant);
	if (pShot == NULL)
		return 0;

	//	Compute the average damage done by the weapon per hit

	DamageDesc Damage;
	if (pShot->HasFragments())
		Damage = pShot->GetFirstFragment()->pDesc->m_Damage;
	else
		Damage = pShot->m_Damage;

	Metric rDamage = CalcConfigurationMultiplier(pShot) * Damage.GetAverageDamage();

	//	Adjust the damage by fire rate (we end up with damage per standard fire rate)

	rDamage = (STD_FIRE_RATE_SECS * rDamage / (Metric)m_iFireRateSecs);

	//	Adjust the damage for the average resistance of armor at this level

	rDamage = rDamage * (Metric)CArmorClass::GetStdDamageAdj(GetLevel(), Damage.GetDamageType()) / 100.0;

	//	Calc damage balance. +1 for each 1% above standard damage
	//	If damage is below the standard, then invert the ratio
	//	(i.e., 50% below is 1/0.5 or half strength, which
	//	counts as -100).

	Metric rDiff = (rDamage - (Metric)pStd->iDamage);
	if (rDiff > 0.0)
		iBalance += (int)(rDiff * 100.0 / (Metric)pStd->iDamage);
	else if (rDamage > 0.0)
		iBalance -= (int)(((Metric)pStd->iDamage * 100.0 / rDamage) - 100.0);
	else
		iBalance -= 200;

	return iBalance;
	}

int CWeaponClass::CalcConfiguration (CItemCtx &ItemCtx, CWeaponFireDesc *pShot, int iFireAngle, CVector *ShotPos, int *ShotDir, bool bSetAlternating)

//	CalcConfiguration
//
//	Returns the number of shots in the configuration and initializes
//	ShotPos and ShotDir appropriately.
//
//	ShotPos and ShotDir must be at least MAX_SHOT_COUNT
//
//	If ItemCtx has pSource and pDevice then ShotPos and ShotDir are absolute (source position
//	and direction has been applied). Otherwise, they are relative (caller must add to source pos and direction)
//
//	If iFireAngle is -1 and we have pSource and pDevice, we compute the current facing of the ship and device.
//
//	Returns the number of shots.

	{
	int i;
	CSpaceObject *pSource = ItemCtx.GetSource();
	CInstalledDevice *pDevice = ItemCtx.GetDevice();

	//	Compute the source position

	CVector vSource;
	if (pSource && pDevice)
		vSource = pDevice->GetPos(pSource);

	//	Compute the fire direction

	if (iFireAngle == -1)
		iFireAngle = GetDefaultFireAngle(pDevice, pSource);

	//	Fill in the arrays

	int iShotCount = 0;
	switch (m_Configuration)
		{
		case ctDual:
			{
			//	Compute a normal perpendicular to the direction of fire

			CVector Perp = PolarToVector(iFireAngle, (g_KlicksPerPixel * g_DualShotSeparation));
			Perp = Perp.Perpendicular();

			//	Create two shots

			iShotCount = 2;
			ShotPos[0] = vSource + Perp;
			ShotDir[0] = iFireAngle;

			ShotPos[1] = vSource - Perp;
			ShotDir[1] = iFireAngle;

			break;
			}

		case ctDualAlternating:
			{
			//	Compute a normal perpendicular to the direction of fire

			CVector Perp = PolarToVector(iFireAngle, (g_KlicksPerPixel * g_DualShotSeparation));
			Perp = Perp.Perpendicular();

			//	If we have a device, the alternate

			if (pDevice)
				{
				//	Figure out our polarity

				int iPolarity = GetAlternatingPos(pDevice);

				//	Create a shot either from the left or from the right depending
				//	on the continuous shot variable.

				iShotCount = 1;
				ShotPos[0] = vSource + (iPolarity ? Perp : -Perp);
				ShotDir[0] = iFireAngle;

				if (bSetAlternating)
					SetAlternatingPos(pDevice, (iPolarity + 1) % 2);
				}

			//	Otherwise, return all shots

			else
				{
				iShotCount = 2;
				ShotPos[0] = vSource + Perp;
				ShotDir[0] = iFireAngle;

				ShotPos[1] = vSource - Perp;
				ShotDir[1] = iFireAngle;
				}
			break;
			}

		case ctWall:
			{
			//	Compute a normal perpendicular to the direction of fire

			CVector Perp = PolarToVector(iFireAngle, (g_KlicksPerPixel * g_DualShotSeparation));
			Perp = Perp.Perpendicular();

			//	Create five shots

			iShotCount = 0;
			for (i = -2; i <= 2; i++)
				{
				ShotPos[iShotCount] = vSource + ((Metric)i * Perp);
				ShotDir[iShotCount] = iFireAngle;
				iShotCount++;
				}

			break;
			}

		case ctSpread5:
			{
			//	Shots at +2 and -2 degrees

			ShotPos[iShotCount] = vSource;
			ShotDir[iShotCount] = (iFireAngle + 2) % 360;
			iShotCount++;

			ShotPos[iShotCount] = vSource;
			ShotDir[iShotCount] = (iFireAngle + 358) % 360;
			iShotCount++;

			//	Fallthrough!
			}

		case ctSpread3:
			{
			//	Shot at 0 degrees

			ShotPos[iShotCount] = vSource;
			ShotDir[iShotCount] = iFireAngle;
			iShotCount++;

			//	Fallthrough!
			}

		case ctSpread2:
			{
			//	Shots at +5 and -5 degrees

			ShotPos[iShotCount] = vSource;
			ShotDir[iShotCount] = (iFireAngle + 5) % 360;
			iShotCount++;

			ShotPos[iShotCount] = vSource;
			ShotDir[iShotCount] = (iFireAngle + 355) % 360;
			iShotCount++;

			break;
			}

		case ctCustom:
			{
			if (m_bConfigAlternating && pDevice)
				{
				//	Figure out which shot we're firing

				int iShot = Max(0, Min(GetAlternatingPos(pDevice), m_iConfigCount));

				iShotCount = 1;
				ShotPos[0] = vSource + PolarToVector((iFireAngle + m_pConfig[iShot].iPosAngle) % 360, m_pConfig[iShot].rPosRadius);
				ShotDir[0] = (360 + iFireAngle + m_pConfig[iShot].Angle.Roll()) % 360;

				//	Next shot in sequence

				if (bSetAlternating)
					SetAlternatingPos(pDevice, (iShot + 1) % m_iConfigCount);
				}
			else
				{
				iShotCount = Min(MAX_SHOT_COUNT, m_iConfigCount);

				for (i = 0; i < iShotCount; i++)
					{
					ShotPos[i] = vSource + PolarToVector((iFireAngle + m_pConfig[i].iPosAngle) % 360, m_pConfig[i].rPosRadius);
					ShotDir[i] = (360 + iFireAngle + m_pConfig[i].Angle.Roll()) % 360;
					}
				}
			break;
			}

		default:
			{
			iShotCount = 1;
			ShotPos[0] = vSource;
			ShotDir[0] = iFireAngle;
			}
		}

	return iShotCount;
	}

Metric CWeaponClass::CalcConfigurationMultiplier (CWeaponFireDesc *pShot, bool bIncludeFragments) const

//	CalcConfigurationMultiplier
//
//	Calculates multiplier

	{
	if (pShot == NULL)
		{
		if (m_ShotData.GetCount() == 1)
			pShot = GetVariant(0);
		else
			return 1.0;
		}

	Metric rMult = 1.0;
	switch (m_Configuration)
		{
		case ctDual:
		case ctSpread2:
			rMult = 2.0;
			break;

		case ctSpread3:
			rMult = 3.0;
			break;

		case ctWall:
		case ctSpread5:
			rMult = 5.0;
			break;

		case ctCustom:
			if (m_bConfigAlternating)
				rMult = 1.0;
			else
				rMult = m_iConfigCount;
			break;
		}

	if (pShot->m_iContinuous > 0)
		rMult *= (pShot->m_iContinuous + 1);

	//	Include passhtrough

	if (pShot->GetPassthrough() > 0)
		rMult *= 1.0 + (4.0 * pShot->GetPassthrough() / 100.0);

	//	Compute fragment count

	if (bIncludeFragments
			&& pShot->HasFragments()
			&& pShot->GetFirstFragment()->Count.GetMaxValue() > 1)
		{
		int iMin = pShot->GetFirstFragment()->Count.GetMinValue();
		int iMax = pShot->GetFirstFragment()->Count.GetMaxValue();

		//	Compute the average, then divide by two (assume that at
		//	most one third the fragments will hit)

		rMult *= ((iMin + iMax) / 2) / 3.0;
		}

	return rMult;
	}

Metric CWeaponClass::CalcDamage (CWeaponFireDesc *pShot) const

//	CalcDamage
//
//	Computes damage for the given weapon fire desc.

	{
	//	If we have fragments we need to recurse

	if (pShot->HasFragments())
		{
		Metric rTotal = 0.0;

		CWeaponFireDesc::SFragmentDesc *pFragment = pShot->GetFirstFragment();
		while (pFragment)
			{
			//	By default, 1/8 of fragments hit, unless the fragments are radius type

			Metric rHitFraction;
			switch (pFragment->pDesc->m_iFireType)
				{
				case ftArea:
				case ftRadius:
					rHitFraction = 1.0;
					break;

				default:
					//	Assume 1/8th fragments hit on average
					rHitFraction = 0.125;
				}

			//	Add up values

			rTotal += rHitFraction * pFragment->Count.GetAveValueFloat() * CalcDamage(pFragment->pDesc);

			pFragment = pFragment->pNext;
			}

		return rTotal;
		}
	else
		{
		Metric rDamage;

		//	Average damage depends on type

		switch (pShot->m_iFireType)
			{
			case ftArea:
				//	Assume 1/8th damage points hit on average
				rDamage = 0.125 * pShot->GetAreaDamageDensityAverage() * pShot->m_Damage.GetAverageDamage();
				break;

			case ftRadius:
				//	Assume average target is far enough away to take half damage
				rDamage = 0.5 * pShot->m_Damage.GetAverageDamage();
				break;

			default:
				rDamage = pShot->m_Damage.GetAverageDamage();
			}

		//	If we have a capacitor, adjust damage

		switch (m_Counter)
			{
			case cntCapacitor:
				{
				//	Compute the number of ticks until we discharge the capacitor

				Metric rFireTime = (MAX_COUNTER / (Metric)-m_iCounterActivate) * m_iFireRate;

				//	Compute the number of ticks to recharge

				Metric rRechargeTime = (MAX_COUNTER / (Metric)m_iCounterUpdate) * m_iCounterUpdateRate;

				//	Adjust damage by the fraction of time that we spend firing.

				Metric rTotalTime = rFireTime + rRechargeTime;
				if (rTotalTime > 0.0)
					rDamage *= rFireTime / rTotalTime;
				else
					rDamage = 0.0;

				break;
				}
			}

		//	Done

		return rDamage;
		}
	}

Metric CWeaponClass::CalcDamagePerShot (CWeaponFireDesc *pShot) const

//	CalcDamagePerShot
//
//	Returns average damage per shot

	{
	return CalcConfigurationMultiplier(pShot, false) * CalcDamage(pShot);
	}

int CWeaponClass::CalcFireAngle (CItemCtx &ItemCtx, Metric rSpeed, CSpaceObject *pTarget, bool *retbOutOfArc)

//	CalcFireAngle
//
//	Calculates the default fire angle for the weapon

	{
	CSpaceObject *pSource = ItemCtx.GetSource();
	if (pSource == NULL)
		return -1;

	CInstalledDevice *pDevice = ItemCtx.GetDevice();
	if (pDevice == NULL)
		return -1;

	if (pTarget && CanRotate(ItemCtx))
		{
		int iFireAngle;

		CVector vSource = pDevice->GetPos(pSource);

		//	Get the position and velocity of the target

		CVector vTarget = pTarget->GetPos() - vSource;
		CVector vTargetVel = pTarget->GetVel() - pSource->GetVel();

		//	Figure out which direction to fire in

		Metric rTimeToIntercept = CalcInterceptTime(vTarget, vTargetVel, rSpeed);
		CVector vInterceptPoint = (rTimeToIntercept > 0.0 ? vTarget + vTargetVel * rTimeToIntercept : vTarget);
		iFireAngle = VectorToPolar(vInterceptPoint, NULL);

		//	If this is a directional weapon make sure we are in-bounds

		int iMinFireAngle, iMaxFireAngle;
		if (IsDirectional(pDevice, &iMinFireAngle, &iMaxFireAngle))
			{
			int iMin = (pSource->GetRotation() + iMinFireAngle) % 360;
			int iMax = (pSource->GetRotation() + iMaxFireAngle) % 360;

			if (iMin < iMax)
				{
				if (iFireAngle < iMin)
					iFireAngle = iMin;
				else if (iFireAngle > iMax)
					iFireAngle = iMax;
				}
			else
				{
				if (iFireAngle < iMin && iFireAngle > iMax)
					{
					int iToMax = iFireAngle - iMax;
					int iToMin = iMin - iFireAngle;

					if (iToMax > iToMin)
						iFireAngle = iMin;
					else
						iFireAngle = iMax;
					}
				}
			}

		//	Remember the fire angle (we need it later if this is a continuous
		//	fire device)

		pDevice->SetFireAngle(iFireAngle);

		return iFireAngle;
		}
	else
		return GetDefaultFireAngle(pDevice, pSource);
	}

int CWeaponClass::CalcFireSolution (CInstalledDevice *pDevice, CSpaceObject *pSource, CSpaceObject *pTarget)

//	CalcFireSolution
//
//	Computes the direction to fire to hit the target (or -1 if no solution)

	{
	ASSERT(pTarget);

	CItemCtx Ctx(pSource, pDevice);
	CWeaponFireDesc *pShot = GetSelectedShotData(Ctx);
	if (pShot == NULL)
		return -1;

	//	Compute source

	CVector vSource = pDevice->GetPos(pSource);

	//	We need to calculate the intercept solution.

	Metric rWeaponSpeed = pShot->GetRatedSpeed();
	CVector vTarget = pTarget->GetPos() - vSource;
	CVector vTargetVel = pTarget->GetVel() - pSource->GetVel();

	//	Figure out intercept time

	Metric rDist;
	Metric rTimeToIntercept = CalcInterceptTime(vTarget, vTargetVel, rWeaponSpeed, &rDist);
	if (rTimeToIntercept < 0.0)
		return -1;

	//	Compute direction

	CVector vInterceptPoint = vTarget + vTargetVel * rTimeToIntercept;
	return VectorToPolar(vInterceptPoint, NULL);
	}

int CWeaponClass::CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	CalcPowerUsed
//
//	Returns the power consumed

	{
	if (!pDevice->IsEnabled())
		return 0;

	//	We consume less power when we are fully charged

	int iPower = m_iPowerUse;
	if (pDevice->IsReady() && (!pDevice->IsTriggered() || !pDevice->IsLastActivateSuccessful()))
		iPower = m_iIdlePowerUse;

	//	Adjust based on power efficiency enhancement

	if (pDevice->GetMods().IsNotEmpty())
		iPower = iPower * pDevice->GetMods().GetPowerAdj() / 100;

	return iPower;
	}

bool CWeaponClass::CanRotate (CItemCtx &Ctx, int *retiMinFireArc, int *retiMaxFireArc)

//	CanRotate
//
//	Returns TRUE if the weapon is either omnidirectional or directional

	{
	//	If the weapon is natively on a turret

	if (m_bOmnidirectional)
		{
		if (retiMinFireArc)
			*retiMinFireArc = 0;
		if (retiMaxFireArc)
			*retiMaxFireArc = 0;
		return true;
		}
	else if (m_iMinFireArc != m_iMaxFireArc)
		{
		if (retiMinFireArc)
			*retiMinFireArc = m_iMinFireArc;
		if (retiMaxFireArc)
			*retiMaxFireArc = m_iMaxFireArc;
		return true;
		}

	//	If the device slot is a turret

	else if (Ctx.GetDevice())
		{
		if (Ctx.GetDevice()->IsOmniDirectional())
			{
			if (retiMinFireArc)
				*retiMinFireArc = 0;
			if (retiMaxFireArc)
				*retiMaxFireArc = 0;
			return true;
			}
		else if (Ctx.GetDevice()->IsDirectional())
			{
			if (retiMinFireArc)
				*retiMinFireArc = Ctx.GetDevice()->GetMinFireArc();
			if (retiMaxFireArc)
				*retiMaxFireArc = Ctx.GetDevice()->GetMaxFireArc();
			return true;
			}
		else
			return false;
		}

	//	Fixed weapon

	else
		return false;
	}

ALERROR CWeaponClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpWeapon)

//	CreateFromXML
//
//	Creates from an XML element

	{
	ALERROR error;
	int i;
	CWeaponClass *pWeapon;

	pWeapon = new CWeaponClass;
	if (pWeapon == NULL)
		return ERR_MEMORY;

	if (error = pWeapon->InitDeviceFromXML(Ctx, pDesc, pType))
		return error;

	pWeapon->m_iFireRateSecs = pDesc->GetAttributeInteger(CONSTLIT(g_FireRateAttrib));
	pWeapon->m_iFireRate = (int)((pWeapon->m_iFireRateSecs / STD_SECONDS_PER_UPDATE) + 0.5);
	pWeapon->m_iPowerUse = pDesc->GetAttributeIntegerBounded(POWER_USE_ATTRIB, 0, -1, 0);
	pWeapon->m_iIdlePowerUse = pDesc->GetAttributeIntegerBounded(IDLE_POWER_USE_ATTRIB, 0, -1, pWeapon->m_iPowerUse / 10);
	pWeapon->m_iRecoil = pDesc->GetAttributeInteger(RECOIL_ATTRIB);
	pWeapon->m_iFailureChance = pDesc->GetAttributeInteger(FAILURE_CHANCE_ATTRIB);
	pWeapon->m_iMinFireArc = AngleMod(pDesc->GetAttributeInteger(MIN_FIRE_ARC_ATTRIB));
	pWeapon->m_iMaxFireArc = AngleMod(pDesc->GetAttributeInteger(MAX_FIRE_ARC_ATTRIB));

	//	Configuration

	CString sConfig = pDesc->GetAttribute(CONFIGURATION_ATTRIB);
	if (strEquals(sConfig, CONFIG_TYPE_DUAL))
		pWeapon->m_Configuration = ctDual;
	else if (strEquals(sConfig, CONFIG_TYPE_WALL))
		pWeapon->m_Configuration = ctWall;
	else if (strEquals(sConfig, CONFIG_TYPE_SPREAD2))
		pWeapon->m_Configuration = ctSpread2;
	else if (strEquals(sConfig, CONFIG_TYPE_SPREAD3))
		pWeapon->m_Configuration = ctSpread3;
	else if (strEquals(sConfig, CONFIG_TYPE_SPREAD5))
		pWeapon->m_Configuration = ctSpread5;
	else if (strEquals(sConfig, CONFIG_TYPE_DUAL_ALTERNATING))
		pWeapon->m_Configuration = ctDualAlternating;
	else
		{
		CXMLElement *pConfig = pDesc->GetContentElementByTag(CONFIGURATION_TAG);
		if (pConfig && pConfig->GetContentElementCount())
			{
			pWeapon->m_Configuration = ctCustom;

			pWeapon->m_iConfigCount = pConfig->GetContentElementCount();
			pWeapon->m_pConfig = new SConfigDesc [pWeapon->m_iConfigCount];

			for (i = 0; i < pWeapon->m_iConfigCount; i++)
				{
				CXMLElement *pShotConfig = pConfig->GetContentElement(i);
				pWeapon->m_pConfig[i].Angle.LoadFromXML(pShotConfig->GetAttribute(ANGLE_ATTRIB));
				pWeapon->m_pConfig[i].iPosAngle = (pShotConfig->GetAttributeInteger(POS_ANGLE_ATTRIB) + 360) % 360;
				pWeapon->m_pConfig[i].rPosRadius = g_KlicksPerPixel * pShotConfig->GetAttributeInteger(POS_RADIUS_ATTRIB);
				}

			pWeapon->m_iConfigAimTolerance = pConfig->GetAttributeInteger(AIM_TOLERANCE_ATTRIB);
			if (pWeapon->m_iConfigAimTolerance == 0)
				pWeapon->m_iConfigAimTolerance = 5;

			pWeapon->m_bConfigAlternating = pConfig->GetAttributeBool(ALTERNATING_ATTRIB);
			}
		else
			pWeapon->m_Configuration = ctSingle;
		}

	//	Charges

	pWeapon->m_bCharges = pDesc->GetAttributeBool(CHARGES_ATTRIB);
	
	//	Counter

	sConfig = pDesc->GetAttribute(COUNTER_ATTRIB);
	if (!sConfig.IsBlank())
		{
		if (strEquals(sConfig, COUNTER_TYPE_TEMPERATURE))
			pWeapon->m_Counter = cntTemperature;
		else if (strEquals(sConfig, COUNTER_TYPE_CAPACITOR))
			pWeapon->m_Counter = cntCapacitor;
		else
			{
			Ctx.sError = CONSTLIT("Invalid weapon counter type");
			return ERR_FAIL;
			}

		pWeapon->m_iCounterActivate = pDesc->GetAttributeInteger(COUNTER_ACTIVATE_ATTRIB);
		pWeapon->m_iCounterUpdate = pDesc->GetAttributeInteger(COUNTER_UPDATE_ATTRIB);
		pWeapon->m_iCounterUpdateRate = pDesc->GetAttributeInteger(COUNTER_UPDATE_RATE_ATTRIB);
		if (pWeapon->m_iCounterUpdateRate <= 0)
			pWeapon->m_iCounterUpdateRate = 1;
		}
	else if ((pWeapon->m_iCounterActivate = pDesc->GetAttributeInteger(HEATING_ATTRIB)) > 0)
		{
		//	Backward compatibility

		pWeapon->m_Counter = cntTemperature;
		pWeapon->m_iCounterUpdate = TEMP_DECREASE;
		pWeapon->m_iCounterUpdateRate = pDesc->GetAttributeInteger(COOLING_RATE_ATTRIB);
		if (pWeapon->m_iCounterUpdateRate <= 0)
			pWeapon->m_iCounterUpdateRate = 1;
		}
	else
		{
		pWeapon->m_Counter = cntNone;
		pWeapon->m_iCounterActivate = 0;
		pWeapon->m_iCounterUpdate = 0;
		pWeapon->m_iCounterUpdateRate = 0;
		}

	//	Linked fire options

	CString sLinkedFire;
	if (pDesc->FindAttribute(LINKED_FIRE_ATTRIB, &sLinkedFire))
		{
		if (error = ParseLinkedFireOptions(Ctx, sLinkedFire, &pWeapon->m_dwLinkedFireOptions))
			return error;
		}
	else
		pWeapon->m_dwLinkedFireOptions = 0;

	//	Flags

	pWeapon->m_bOmnidirectional = pDesc->GetAttributeBool(CONSTLIT(g_OmnidirectionalAttrib));
	pWeapon->m_bMIRV = pDesc->GetAttributeBool(MULTI_TARGET_ATTRIB);

	//	If we have a Missiles tag then this weapon has ammunition; otherwise,
	//	we take the information from the root element

	CXMLElement *pMissiles = pDesc->GetContentElementByTag(MISSILES_TAG);
	if (pMissiles)
		{
		pWeapon->m_ShotData.InsertEmpty(pMissiles->GetContentElementCount());
		for (i = 0; i < pWeapon->m_ShotData.GetCount(); i++)
			{
			CXMLElement *pItem = pMissiles->GetContentElement(i);

			//	No matter what we load the ammo type.

			if (error = pWeapon->m_ShotData[i].pAmmoType.LoadUNID(Ctx, pItem->GetAttribute(AMMO_ID_ATTRIB)))
				return error;

			//	If this entry defines the missile, then we own it.

			if (pItem->FindAttribute(TYPE_ATTRIB))
				{
				pWeapon->m_ShotData[i].bOwned = true;
				pWeapon->m_ShotData[i].pDesc = new CWeaponFireDesc;

				CString sUNID = strPatternSubst(CONSTLIT("%d/%d"), pWeapon->GetUNID(), i);
				if (error = pWeapon->m_ShotData[i].pDesc->InitFromXML(Ctx, pItem, sUNID))
					return error;
				}

			//	Otherwise we expect the definition to be in the ItemType.
			//	This is best for missiles.

			else
				{
				//	AmmoID is required in this case.

				if (pWeapon->m_ShotData[i].pAmmoType.GetUNID() == 0)
					{
					Ctx.sError = CONSTLIT("Expected either type= or ammoID= definition.");
					return ERR_FAIL;
					}

				pWeapon->m_ShotData[i].bOwned = false;
				pWeapon->m_ShotData[i].pDesc = NULL;
				}
			}
		}
	else
		{
		pWeapon->m_ShotData.InsertEmpty(1);
		pWeapon->m_ShotData[0].bOwned = true;
		pWeapon->m_ShotData[0].pDesc = new CWeaponFireDesc;

		CString sUNID = strPatternSubst(CONSTLIT("%d/0"), pWeapon->GetUNID());
		if (error = pWeapon->m_ShotData[0].pDesc->InitFromXML(Ctx, pDesc, sUNID))
			return error;
		}

	//	If this weapon uses different kinds of ammo then it is a launcher

	if (pWeapon->m_ShotData.GetCount() > 1)
		pWeapon->m_bLauncher = true;
	else if (pWeapon->GetDefinedSlotCategory() == itemcatLauncher)
		pWeapon->m_bLauncher = true;
	else
		pWeapon->m_bLauncher = pDesc->GetAttributeBool(LAUNCHER_ATTRIB);

	pWeapon->m_bReportAmmo = pDesc->GetAttributeBool(REPORT_AMMO_ATTRIB);

	//	Targeting options

	pWeapon->m_bTargetStationsOnly = pDesc->GetAttributeBool(TARGET_STATIONS_ONLY_ATTRIB);

	//	Done

	*retpWeapon = pWeapon;

	return NOERROR;
	}

bool CWeaponClass::FindDataField (int iVariant, const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	int i;

	if (iVariant < 0 || iVariant >= m_ShotData.GetCount())
		return false;

	CWeaponFireDesc *pShot = GetVariant(iVariant);
	if (pShot == NULL)
		return false;

	if (strEquals(sField, FIELD_AMMO_TYPE))
		{
		CItemType *pAmmoType = pShot->GetAmmoType();
		*retsValue = (pAmmoType ? strFromInt(pAmmoType->GetUNID()) : NULL_STR);
		}
	else if (strEquals(sField, FIELD_HP))
		*retsValue = strFromInt(pShot->GetHitPoints());
	else if (strEquals(sField, FIELD_MIN_DAMAGE))
		*retsValue = strFromInt((int)(CalcConfigurationMultiplier(pShot) * pShot->m_Damage.GetMinDamage()));
	else if (strEquals(sField, FIELD_MAX_DAMAGE))
		*retsValue = strFromInt((int)(CalcConfigurationMultiplier(pShot) * pShot->m_Damage.GetMaxDamage()));
	else if (strEquals(sField, FIELD_DAMAGE_TYPE))
		*retsValue = strFromInt(pShot->m_Damage.GetDamageType());
	else if (strEquals(sField, FIELD_FIRE_DELAY))
		*retsValue = strFromInt(m_iFireRate);
	else if (strEquals(sField, FIELD_FIRE_RATE))
		{
		if (m_iFireRate)
			*retsValue = strFromInt(1000 / m_iFireRate);
		else
			return false;
		}
	else if (strEquals(sField, FIELD_AVERAGE_DAMAGE))
		*retsValue = strFromInt((int)(CalcDamagePerShot(pShot) * 1000.0));
	else if (strEquals(sField, FIELD_DAMAGE_180))
		{
		Metric rDamagePerShot = CalcDamagePerShot(pShot);
		*retsValue = strFromInt((int)((rDamagePerShot * 180.0 / m_iFireRate) + 0.5));
		}
	else if (strEquals(sField, FIELD_BALANCE_DAMAGE))
		{
		//	Compute the level. For launchers we take either the missile level or the
		//	launcher level (whichever is higher).

		int iLevel;
		if (m_bLauncher)
			iLevel = Max(GetLevel(), (pShot->GetAmmoType() ? pShot->GetAmmoType()->GetLevel() : 0));
		else
			iLevel = GetLevel();

		//	DPS at this level

		Metric rDamagePerShot = STD_WEAPON_STATS[iLevel - 1].iDamage;

		//	Adjust for damage type tiers

		DamageTypes iType = (DamageTypes)GetDamageType(NULL, iVariant);
		int iTier = ::GetDamageTypeLevel(iType);
		if (iTier > iLevel)
			rDamagePerShot = rDamagePerShot * STD_WEAPON_STATS[iTier - iLevel].iUnderTierAdj / 100.0;
		else
			rDamagePerShot = rDamagePerShot * STD_WEAPON_STATS[iLevel - iTier].iOverTierAdj / 100.0;

		*retsValue = strFromInt((int)((rDamagePerShot * 180.0 / 8) + 0.5));
		}
	else if (strEquals(sField, FIELD_POWER))
		*retsValue = strFromInt(m_iPowerUse * 100);
	else if (strEquals(sField, FIELD_POWER_PER_SHOT))
		*retsValue = strFromInt((int)(((m_iFireRate * m_iPowerUse * STD_SECONDS_PER_UPDATE * 1000) / 600.0) + 0.5));
	else if (strEquals(sField, FIELD_BALANCE))
		*retsValue = strFromInt(CalcBalance(iVariant));
	else if (strEquals(sField, FIELD_RANGE))
		*retsValue = strFromInt((int)(Ticks2Seconds(pShot->GetMaxLifetime()) * pShot->GetRatedSpeed() / LIGHT_SECOND));
	else if (strEquals(sField, FIELD_RECOIL))
		*retsValue = (m_iRecoil ? strFromInt((int)(m_iRecoil * m_iRecoil * 10 * g_MomentumConstant / g_SecondsPerUpdate)) : NULL_STR);
	else if (strEquals(sField, FIELD_SPEED))
		*retsValue = strFromInt((int)((100.0 * pShot->GetRatedSpeed() / LIGHT_SECOND) + 0.5));
	else if (strEquals(sField, FIELD_VARIANT_COUNT))
		*retsValue = strFromInt(m_ShotData.GetCount());
	else if (strEquals(sField, FIELD_REPEAT_COUNT))
		*retsValue = strFromInt(pShot->m_iContinuous + 1);
	else if (strEquals(sField, FIELD_CONFIGURATION))
		{
		CCodeChain &CC = g_pUniverse->GetCC();
		CVector ShotPos[MAX_SHOT_COUNT];
		int ShotDir[MAX_SHOT_COUNT];
		int iShotCount = CalcConfiguration(CItemCtx(), pShot, 0, ShotPos, ShotDir, false);

		CMemoryWriteStream Output(5000);
		if (Output.Create() != NOERROR)
			return false;

		Output.Write("='(", 3);
		for (i = 0; i < iShotCount; i++)
			{
			ICCItem *pPos = CreateListFromVector(CC, ShotPos[i]);
			if (pPos == NULL || pPos->IsError())
				return false;

			Output.Write("(", 1);
			CString sItem = pPos->Print(&CC);
			Output.Write(sItem.GetASCIIZPointer(), sItem.GetLength());
			Output.Write(" ", 1);

			sItem = strFromInt(ShotDir[i]);
			Output.Write(sItem.GetASCIIZPointer(), sItem.GetLength());
			Output.Write(") ", 2);

			pPos->Discard(&CC);
			}
		Output.Write(")", 1);
		Output.Close();

		*retsValue = CString(Output.GetPointer(), Output.GetLength());
		return true;
		}
	else if (strEquals(sField, FIELD_IS_ALTERNATING))
		*retsValue = ((m_Configuration == ctDualAlternating || m_bConfigAlternating) ? CString("True") : NULL_STR);
	else
		return pShot->FindDataField(sField, retsValue);

	return true;
	}

bool CWeaponClass::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	CString sRootField = sField;
	int iVariant = 0;

	//	Look for a :nn suffix specifying a variant

	char *pPos = sField.GetASCIIZPointer();
	while (*pPos != '\0')
		{
		if (*pPos == ':')
			{
			sRootField.Truncate(pPos - sField.GetASCIIZPointer());
			iVariant = strParseInt(pPos + 1, 0);
			break;
			}
		pPos++;
		}

	return FindDataField(iVariant, sRootField, retsValue);
	}

CWeaponClass::EOnFireWeaponResults CWeaponClass::FireOnFireWeapon (CItemCtx &ItemCtx, 
																   CWeaponFireDesc *pShot,
																   const CVector &vSource,
																   CSpaceObject *pTarget,
																   int iFireAngle,
																   int iRepeatingCount)

//	FireOnFireWeapon
//
//	Fires OnFireWeapon event
//
//	default (or Nil) = fire weapon as normal
//	noShot = do not consume power or ammo
//	shotFired (or True) = consume power and ammo normally

	{
	SEventHandlerDesc Event;
	if (FindEventHandlerWeaponClass(evtOnFireWeapon, &Event))
		{
		CCodeChainCtx Ctx;
		EOnFireWeaponResults iResult;
		CItemEnhancementStack *pEnhancement = ItemCtx.GetDevice()->GetEnhancements();

		Ctx.SaveAndDefineSourceVar(ItemCtx.GetSource());
		Ctx.SaveAndDefineItemVar(ItemCtx);
		Ctx.DefineInteger(CONSTLIT("aFireAngle"), iFireAngle);
		Ctx.DefineVector(CONSTLIT("aFirePos"), vSource);
		Ctx.DefineInteger(CONSTLIT("aFireRepeat"), iRepeatingCount);
		Ctx.DefineSpaceObject(CONSTLIT("aTargetObj"), pTarget);
		Ctx.DefineInteger(CONSTLIT("aWeaponBonus"), (pEnhancement ? pEnhancement->GetBonus() : 0));
		Ctx.DefineItemType(CONSTLIT("aWeaponType"), pShot->GetWeaponType());

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ItemCtx.GetSource()->ReportEventError(ON_FIRE_WEAPON_EVENT, pResult);

		if (pResult->IsNil())
			iResult = resDefault;
		else
			{
			CString sValue = pResult->GetStringValue();
			if (strEquals(sValue, CONSTLIT("shotFired")))
				return resShotFired;
			else if (strEquals(sValue, CONSTLIT("noShot")))
				return resNoShot;
			else if (strEquals(sValue, CONSTLIT("default")))
				return resDefault;
			else
				return resShotFired;
			}

		Ctx.Discard(pResult);

		//	Done

		return iResult;
		}
	else
		return resDefault;
	}

bool CWeaponClass::FireWeapon (CInstalledDevice *pDevice, 
							   CWeaponFireDesc *pShot, 
							   CSpaceObject *pSource, 
							   CSpaceObject *pTarget,
							   int iRepeatingCount,
							   bool *retbSourceDestroyed,
							   bool *retbConsumedItems)

//	FireWeapon
//
//	Fires the weapon. Returns TRUE if we should consume power, etc.

	{
	int i;

	//	Figure out the source of the shot

	CVector vSource = pDevice->GetPos(pSource);

	//	Figure out the speed of the shot

	Metric rSpeed = pShot->GetInitialSpeed();

	//	Slight HACK: If we have no target and this is a tracking weapon
	//	then we get the target from the device. We do this here because
	//	it is somewhat expensive to get the target from the device so
	//	we only do it if we really need it.

	if (pTarget == NULL && pShot->IsTracking())
		pTarget = pDevice->GetTarget(pSource);

	//	Get the fire angle from the device (the AI sets it when it has pre-
	//	calculated the target and fire solution).

	int iFireAngle = pDevice->GetFireAngle();

	//	If the fire angle is -1 then we need to calc it ourselves

	if (iFireAngle == -1)
		{
		bool bOutOfArc;
		iFireAngle = CalcFireAngle(CItemCtx(pSource, pDevice), rSpeed, pTarget, &bOutOfArc);
		}

	//	Pre-init

	if (retbConsumedItems)
		*retbConsumedItems = false;
	if (retbSourceDestroyed)
		*retbSourceDestroyed = false;

	//	Figure out if fire is suppressed by some other object
	//	on the source

	bool bFireSuppressed = pSource->AbsorbWeaponFire(pDevice);

	//	If we're prone to failure, then bad things happen

	bool bFailure = false;
	if (m_iFailureChance > 0)
		{
		if (mathRandom(1, 100) <= m_iFailureChance)
			bFailure = true;
		}

	//	If we're damaged, figure out what bad things happen

	if (pDevice->IsDamaged() || pDevice->IsDisrupted())
		bFailure = true;

	bool bMisfire = false;
	bool bExplosion = false;
	if (bFailure)
		{
		int iRoll = mathRandom(1, 100);

		//	10% of the time, everything works OK

		if (iRoll <= 10)
			;

		//	60% of the time, nothing happens

		else if (iRoll <= 70)
			{
			pSource->OnDeviceStatus(pDevice, failWeaponJammed);

			//	Still counts--we should consume power

			return true;
			}

		//	20% of the time, we fire in a random direction

		else if (iRoll <= 90)
			{
			bMisfire = true;
			pSource->OnDeviceStatus(pDevice, failWeaponMisfire);
			}

		//	10% of the time, the shot explodes

		else
			{
			bExplosion = true;
			bFireSuppressed = true;
			pSource->OnDeviceStatus(pDevice, failWeaponExplosion);
			}
		}

	//	Deal with counter

	switch (m_Counter)
		{
		case cntTemperature:
			{
			if (pDevice->GetTemperature() >= OVERHEAT_TEMP)
				{
				if (pDevice->GetTemperature() >= MAX_TEMP)
					{
					pSource->OnDeviceStatus(pDevice, failWeaponJammed);

					//	Still counts--consume power

					return true;
					}
				else
					{
					int iRoll = mathRandom(1, 100);

					//	25% of the time, everything works OK

					if (iRoll <= 25)
						;

					//	25% of the time, the weapon jams

					else if (iRoll <= 50)
						{
						pSource->OnDeviceStatus(pDevice, failWeaponJammed);
						return true;
						}

					//	25% of the time, the weapon is disabled

					else if (iRoll <= 75)
						{
						pSource->DisableDevice(pDevice);
						return true;
						}

					//	25% of the time, the weapon is damaged

					else
						{
						pSource->DamageItem(pDevice);
						pSource->OnDeviceStatus(pDevice, failDeviceOverheat);
						}
					}
				}

			//	Update temperature

			pDevice->IncTemperature(m_iCounterActivate);
			pSource->OnComponentChanged(comDeviceCounter);
			break;
			}

		case cntCapacitor:
			{
			//	If we don't have enough capacitor power, then we can't fire

			if (pDevice->GetTemperature() < m_iCounterActivate)
				return false;

			//	Consume capacitor

			pDevice->IncTemperature(m_iCounterActivate);
			pSource->OnComponentChanged(comDeviceCounter);
			break;
			}
		}

	//	If this is a misfire, adjust the angle

	if (bMisfire)
		iFireAngle = (iFireAngle + mathRandom(-60, 60) + 360) % 360;

	//	Figure out how many shots to create

	CVector ShotPos[MAX_SHOT_COUNT];
	int ShotDir[MAX_SHOT_COUNT];
	int iShotCount = CalcConfiguration(CItemCtx(pSource, pDevice), pShot, iFireAngle, ShotPos, ShotDir, (iRepeatingCount == pShot->m_iContinuous));

	//	If we're independently targeted, then compute targets for the remaining shots

	if (m_bMIRV && iShotCount > 1)
		{
		TArray<CSpaceObject *> TargetList;
		int iFound = pSource->GetNearestVisibleEnemies(iShotCount, 
				MAX_TARGET_RANGE, 
				&TargetList, 
				NULL, 
				FLAG_INCLUDE_NON_AGGRESSORS);

		int j = 1;
		for (i = 0; i < iFound && j < iShotCount; i++)
			{
			CSpaceObject *pNewTarget = TargetList[i];
			if (pNewTarget != pTarget)
				{
				//	Calculate direction to fire in

				CVector vTarget = pNewTarget->GetPos() - ShotPos[j];
				CVector vTargetVel = pNewTarget->GetVel() - pSource->GetVel();
				Metric rTimeToIntercept = CalcInterceptTime(vTarget, vTargetVel, rSpeed);
				CVector vInterceptPoint = (rTimeToIntercept > 0.0 ? vTarget + pNewTarget->GetVel() * rTimeToIntercept : vTarget);

				ShotDir[j] = VectorToPolar(vInterceptPoint, NULL);
				//ShotVel[j] = pSource->GetVel() + PolarToVector(ShotDir[j], rSpeed);

				j++;
				}
			}
		}

	//	If the shot requires ammo, check to see that the source has
	//	enough ammo for all shots

	if (iRepeatingCount == 0)
		{
		if (pShot->m_pAmmoType)
			{
			//	Select the ammo

			CItemListManipulator ItemList(pSource->GetItemList());
			CItem Item(pShot->m_pAmmoType, iShotCount);
			bool bAmmoSelected = ItemList.SetCursorAtItem(Item);

			//	If we could not select ammo (because we don't have any)
			//	and this is the first shot, then we can't fire at all

			if (!bAmmoSelected)
				{
				return false;
				}
			}
		else if (m_bCharges)
			{
			if (pDevice->GetCharges(pSource) <= 0)
				{
				return false;
				}
			}
		}

	//	Create barrel flash effect

	CEffectCreator *pFireEffect;
	if (!bFireSuppressed && (pFireEffect = pShot->GetFireEffect()))
		{
		CCreatePainterCtx PainterCtx;
		PainterCtx.SetWeaponFireDesc(pShot);

		for (i = 0; i < iShotCount; i++)
			{
			IEffectPainter *pPainter = pFireEffect->CreatePainter(PainterCtx);

			pSource->AddEffect(pPainter, ShotPos[i], 0, ShotDir[i]);
			}
		}

	//	Create all the shots

	CVector vRecoil;
	if (!bFireSuppressed)
		{
		bool bNoShotsFired = true;

		for (i = 0; i < iShotCount; i++)
			{
			//	Fire out to event, if the weapon has one.
			//	Otherwise, we create weapon fire

			EOnFireWeaponResults iResult;
			iResult = FireOnFireWeapon(CItemCtx(pSource, pDevice), 
					pShot, 
					ShotPos[i], 
					pTarget,
					ShotDir[i], 
					iRepeatingCount);

			//	Did we destroy the source?

			if (pSource->IsDestroyed())
				*retbSourceDestroyed = true;

			//	If we didn't fire a shot in the event handler, do it now

			if (iResult == resDefault)
				{
				CSpaceObject *pNewObj;

				pSource->GetSystem()->CreateWeaponFire(pShot,
						pDevice->GetEnhancements(),
						killedByDamage,
						CDamageSource(pSource, killedByDamage),
						ShotPos[i],
						pSource->GetVel() + PolarToVector(ShotDir[i], rSpeed),
						ShotDir[i],
						pTarget,
						((iRepeatingCount == 0 && i == 0) ? CSystem::CWF_WEAPON_FIRE : CSystem::CWF_FRAGMENT),
						&pNewObj);

				//	If this shot was created by automated weapon fire, then set flag

				if (pDevice->IsAutomatedWeapon())
					pNewObj->SetAutomatedWeapon();

				bNoShotsFired = false;
				}

			//	This result means that OnFireWeapon fired a shot

			else if (iResult == resShotFired)
				bNoShotsFired = false;

			//	Add up all the shot directions to end up with a recoil dir

			if (m_iRecoil && iResult != resNoShot)
				vRecoil = vRecoil + PolarToVector(ShotDir[i], 1.0);
			}

		//	If no shots were fired, then we're done

		if (bNoShotsFired)
			return false;
		}

	//	Consume ammo

	bool bNextVariant = false;
	if (iRepeatingCount == pShot->m_iContinuous)
		{
		if (pShot->m_pAmmoType)
			{
			//	Select the ammo

			CItemListManipulator ItemList(pSource->GetItemList());
			CItem Item(pShot->m_pAmmoType, iShotCount);
			bool bAmmoSelected = ItemList.SetCursorAtItem(Item);

			//	If we selected some ammo and this is the last
			//	shot in the series, then consume ammo

			if (bAmmoSelected)
				{
				//	If we've exhausted our ammunition, remember to
				//	select the next variant

				if (ItemList.GetItemAtCursor().GetCount() == 1)
					bNextVariant = true;

				ItemList.DeleteAtCursor(1);

				//	Remember to tell the ship that we've consumed items

				if (retbConsumedItems)
					*retbConsumedItems = true;
				}
			}
		else if (m_bCharges)
			{
			if (pDevice->GetCharges(pSource) > 0)
				{
				pDevice->IncCharges(pSource, -1);

				if (retbConsumedItems)
					*retbConsumedItems = true;
				}
			}
		}

	//	Sound effect

	if (!bFireSuppressed)
		pShot->PlayFireSound(pSource);

	//	Recoil

	if (!bFireSuppressed && m_iRecoil)
		{
		CVector vAccel = vRecoil.Normal() * (Metric)(-10 * m_iRecoil * m_iRecoil);
		pSource->Accelerate(vAccel, g_MomentumConstant);
		pSource->ClipSpeed(pSource->GetMaxSpeed());
		}

	//	Switch to the next variant if necessary

	if (bNextVariant)
		{
		if (!SelectNextVariant(pSource, pDevice))
			SelectFirstVariant(pSource, pDevice);
		}

	//	Create an explosion if weapon damage

	if (bExplosion && !pSource->IsDestroyed())
		{
		SDamageCtx Ctx;
		Ctx.pObj = pSource;
		Ctx.pDesc = pShot;
		Ctx.Damage = pShot->m_Damage;
		Ctx.Damage.SetCause(killedByWeaponMalfunction);
		Ctx.iDirection = (pDevice->GetPosAngle() + 360 + mathRandom(0, 30) - 15) % 360;
		Ctx.vHitPos = vSource;
		Ctx.pCause = pSource;
		Ctx.Attacker = CDamageSource(pSource, killedByWeaponMalfunction);

		EDamageResults iResult = pSource->Damage(Ctx);

		if (iResult == damageDestroyed 
				|| iResult == damagePassthroughDestroyed)
			*retbSourceDestroyed = true;
		}

	//	Done!

	return true;
	}

int CWeaponClass::GetActivateDelay (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetActivateDelay
//
//	Returns the number of ticks between shots
//	NOTE: We do not adjust for enhancements.

	{
	return m_iFireRate;
	}

int CWeaponClass::GetAmmoVariant (const CItemType *pItem) const

//	GetAmmoVariant
//
//	Returns the variant that fires the given ammo (or -1 if the weapons
//	does not fire the ammo)

	{
	int i;

	DWORD dwItemUNID = (pItem ? pItem->GetUNID() : 0);
	if (dwItemUNID == 0)
		return -1;

	for (i = 0; i < m_ShotData.GetCount(); i++)
		{
		if (m_ShotData[i].pAmmoType.GetUNID() == dwItemUNID)
			return i;
		}

	return -1;
	}

ItemCategories CWeaponClass::GetCategory (void) const

//	GetCategory
//
//	Returns the weapon class category

	{
	if (m_bLauncher)
		return itemcatLauncher;
	else
		return itemcatWeapon;
	}

int CWeaponClass::GetCounter (CInstalledDevice *pDevice, CSpaceObject *pSource, CounterTypes *retiType)

//	GetCounter
//
//	Return counter

	{
	//	Return the type

	if (retiType)
		*retiType = m_Counter;

	if (m_Counter == cntNone)
		return 0;

	//	If we're a capacitor, then don't show the counter if we are full

	if (m_Counter == cntCapacitor && pDevice->GetTemperature() >= MAX_COUNTER)
		return 0;

	//	Otherwise, return the current value

	return pDevice->GetTemperature();
	}

int CWeaponClass::GetAlternatingPos (CInstalledDevice *pDevice)
	{
	return (int)(DWORD)HIBYTE(LOWORD(pDevice->GetData()));
	}

DWORD CWeaponClass::GetContinuousFire (CInstalledDevice *pDevice) 
	{
	return (int)(DWORD)LOBYTE(LOWORD(pDevice->GetData()));
	}

int CWeaponClass::GetCurrentVariant (CInstalledDevice *pDevice)
	{
	return (int)(short)HIWORD(pDevice->GetData()); 
	}

const DamageDesc *CWeaponClass::GetDamageDesc (CItemCtx &Ctx)

//	GetDamageDesc
//
//	Returns the kind of damage caused by this weapon

	{
	CWeaponFireDesc *pShot = GetSelectedShotData(Ctx);
	if (pShot == NULL)
		return NULL;
	else
		return &pShot->m_Damage;
	}

int CWeaponClass::GetDamageType (CInstalledDevice *pDevice, int iVariant)

//	GetDamageType
//
//	Returns the kind of damage caused by this weapon

	{
	CWeaponFireDesc *pShot;

	if (iVariant == -1)
		{
		if (pDevice)
			pShot = GetSelectedShotData(CItemCtx(NULL, pDevice));
		else if (m_ShotData.GetCount() > 0)
			pShot = GetVariant(0);
		else
			return damageGeneric;
		}
	else
		pShot = GetVariant(iVariant);

	//	OK if we don't find shot--could be a launcher with no ammo

	if (pShot == NULL)
		return damageGeneric;

	return pShot->m_Damage.GetDamageType();
	}

int CWeaponClass::GetDefaultFireAngle (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetDefaultFireAngle
//
//	Gets the natural fire direction (not counting omni or swivel mounts)

	{
	if (pDevice && pSource)
		return (pSource->GetRotation() + pDevice->GetRotation() + AngleMiddle(m_iMinFireArc, m_iMaxFireArc)) % 360;
	else
		return AngleMiddle(m_iMinFireArc, m_iMaxFireArc);
	}

ICCItem *CWeaponClass::GetItemProperty (CItemCtx &Ctx, const CString &sName)

//	GetItemProperty
//
//	Returns the item property. Subclasses should call this if they do not
//	understand the property.

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	if (strEquals(sName, PROPERTY_FIRE_ARC))
		{
		CInstalledDevice *pDevice = Ctx.GetDevice();	//	May be NULL
		int iMinFireArc;
		int iMaxFireArc;

		//	Omnidirectional

		if (IsOmniDirectional(pDevice))
			return CC.CreateString(PROPERTY_OMNIDIRECTIONAL);

		//	Fire arc

		else if (IsDirectional(pDevice, &iMinFireArc, &iMaxFireArc))
			{
			//	Create a list

			ICCItem *pResult = CC.CreateLinkedList();
			if (pResult->IsError())
				return pResult;

			CCLinkedList *pList = (CCLinkedList *)pResult;

			pList->AppendIntegerValue(&CC, iMinFireArc);
			pList->AppendIntegerValue(&CC, iMaxFireArc);

			return pResult;
			}

		//	Otherwise, see if we are pointing in a particular direction

		else
			{
			int iFacingAngle = AngleMod((pDevice ? pDevice->GetRotation() : 0) + AngleMiddle(m_iMinFireArc, m_iMaxFireArc));
			if (iFacingAngle == 0)
				return CC.CreateNil();
			else
				return CC.CreateInteger(iFacingAngle);
			}
		}

	else if (strEquals(sName, PROPERTY_LINKED_FIRE_OPTIONS))
		{
		//	Get the options from the device

		DWORD dwOptions = GetLinkedFireOptions(Ctx);
		if (dwOptions == 0)
			return CC.CreateNil();

		//	Create a list

		ICCItem *pResult = CC.CreateLinkedList();
		if (pResult->IsError())
			return pResult;

		CCLinkedList *pList = (CCLinkedList *)pResult;

		//	Add options

		if (dwOptions & CDeviceClass::lkfAlways)
			pList->AppendStringValue(&CC, CDeviceClass::GetLinkedFireOptionString(CDeviceClass::lkfAlways));
		else if (dwOptions & CDeviceClass::lkfTargetInRange)
			pList->AppendStringValue(&CC, CDeviceClass::GetLinkedFireOptionString(CDeviceClass::lkfTargetInRange));
		else if (dwOptions & CDeviceClass::lkfEnemyInRange)
			pList->AppendStringValue(&CC, CDeviceClass::GetLinkedFireOptionString(CDeviceClass::lkfEnemyInRange));

		//	Done

		return pResult;
		}

	else if (strEquals(sName, PROPERTY_OMNIDIRECTIONAL))
		{
		CInstalledDevice *pDevice = Ctx.GetDevice();	//	May be NULL
		return CC.CreateBool(IsOmniDirectional(pDevice));
		}

	//	Otherwise, just get the property from the base class

	else
		return CDeviceClass::GetItemProperty(Ctx, sName);
	}

Metric CWeaponClass::GetMaxEffectiveRange (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget)

//	GetMaxEffectiveRange
//
//	Returns the greatest range at which the weapon is still
//	effective.

	{
	if (pSource)
		{
		CItemCtx Ctx(pSource, pDevice);
		CWeaponFireDesc *pShot = GetSelectedShotData(Ctx);
		if (pShot == NULL)
			return 0.0;
		else
			{
			if (pTarget && !pTarget->CanMove())
				{
				Metric rSpeed = (pShot->GetRatedSpeed() + pShot->m_rMaxMissileSpeed) / 2;
				return (rSpeed * (pShot->GetMaxLifetime() * 90 / 100)) + (128.0 * g_KlicksPerPixel);
				}
			else
				return pShot->m_rMaxEffectiveRange;
			}
		}
	else
		{
		if (m_ShotData.GetCount() > 0)
			return GetVariant(0)->m_rMaxEffectiveRange;
		else
			return 0.0;
		}
	}

DWORD CWeaponClass::GetLinkedFireOptions (CItemCtx &Ctx)

//	GetLinkedFireOptions
//
//	Returns linked fire options for the weapon
	
	{
	//	If the device has linked fire, then honor that.

	DWORD dwOptions;
	if (Ctx.GetDevice() && (dwOptions = Ctx.GetDevice()->GetLinkedFireOptions()))
		return dwOptions;

	//	Otherwise, take native linked fire setting from weapon.

	return m_dwLinkedFireOptions; 
	}

int CWeaponClass::GetPowerRating (CItemCtx &Ctx)

//	GetPowerRating
//
//	Returns the rated power

	{
	int iPower = m_iPowerUse;

	const CItemEnhancement &Mods = Ctx.GetMods();
	if (Mods.IsNotEmpty())
		iPower = iPower * Mods.GetPowerAdj() / 100;

	return iPower;
	}

CString GetReferenceFireRate (int iFireRate)
	{
	if (iFireRate <= 0)
		return NULL_STR;

	int iRate = (int)((10.0 * g_TicksPerSecond / iFireRate) + 0.5);
	if (iRate == 0)
		return CONSTLIT(" @ <0.1 shots/sec");
	else if ((iRate % 10) == 0)
		{
		if ((iRate / 10) == 1)
			return strPatternSubst(CONSTLIT(" @ %d shot/sec"), iRate / 10);
		else
			return strPatternSubst(CONSTLIT(" @ %d shots/sec"), iRate / 10);
		}
	else
		return strPatternSubst(CONSTLIT(" @ %d.%d shots/sec"), iRate / 10, iRate % 10);
	}

CString CWeaponClass::GetReference (CItemCtx &Ctx, int iVariant, DWORD dwFlags)

//	GetReference
//
//	Returns a string that describes the basic attributes
//	of this weapon.
//
//	Example:
//
//		laser 1-4 (x2); tracking; 100MW

	{
	CString sReference;
	const CItemEnhancement &Mods = Ctx.GetMods();

	//	Power consumption

	if (iVariant == -1 && !(dwFlags & flagNoPowerReference))
		sReference = GetReferencePower(Ctx);

	//	Additional properties

	if (m_ShotData.GetCount() == 1 || iVariant != -1)
		{
		CWeaponFireDesc *pShot = GetReferenceShotData(GetVariant(iVariant != -1 ? iVariant : 0));
		DamageDesc Damage = pShot->m_Damage;

		//	Modify the damage based on any enhancements that the ship may have

		CInstalledDevice *pDevice = Ctx.GetDevice();
		if (pDevice)
			Damage.AddEnhancements(pDevice->GetEnhancements());
		else
			Damage.AddBonus(Mods.GetHPBonus());

		//	Area of effect

		if (pShot->m_iFireType == ftArea)
			AppendReferenceString(&sReference, CONSTLIT("shockwave"));
		else if (pShot->m_iFireType == ftRadius)
			AppendReferenceString(&sReference, CONSTLIT("area effect"));

		//	Compute special abilities. Start with tracking

		if (pShot->IsTracking())
			AppendReferenceString(&sReference, CONSTLIT("tracking"));

		//	Blinding

		if (Damage.GetBlindingDamage() > 0)
			AppendReferenceString(&sReference, CONSTLIT("blinding"));

		//	Radiation

		if (Damage.GetRadiationDamage() > 0)
			AppendReferenceString(&sReference, CONSTLIT("radiation"));

		//	EMP

		if (Damage.GetEMPDamage() > 0)
			AppendReferenceString(&sReference, CONSTLIT("EMP"));

		//	Device damage

		if (Damage.GetDeviceDamage() > 0)
			AppendReferenceString(&sReference, CONSTLIT("device damage"));

		if (Damage.GetDeviceDisruptDamage() > 0)
			AppendReferenceString(&sReference, CONSTLIT("device disrupt"));

		//	Disintegration

		if (Damage.GetDisintegrationDamage() > 0)
			AppendReferenceString(&sReference, CONSTLIT("disintegration"));

		//	WMD

		if (Damage.GetMassDestructionAdj() >= 95)
			AppendReferenceString(&sReference, CONSTLIT("mass destruction"));
		else if (Damage.GetMassDestructionAdj() >= 70)
			AppendReferenceString(&sReference, CONSTLIT("heavy station damage"));
		else if (Damage.GetMassDestructionAdj() >= 30)
			AppendReferenceString(&sReference, CONSTLIT("medium station damage"));
		else if (Damage.GetMassDestructionAdj() >= 10)
			AppendReferenceString(&sReference, CONSTLIT("light station damage"));

		//	Shields

		if (Damage.GetShieldDamageLevel() > 0)
			AppendReferenceString(&sReference, CONSTLIT("shield buster"));

		//	Armor

		if (Damage.GetArmorDamageLevel() > 0)
			AppendReferenceString(&sReference, CONSTLIT("armor penetrator"));
		}

	return sReference;
	}

bool CWeaponClass::GetReferenceDamageType (CItemCtx &Ctx, int iVariant, DamageTypes *retiDamage, CString *retsReference) const

//	GetReferenceDamageType
//
//	Returns the damage type done by the weapon

	{
	const CItem &Item = Ctx.GetItem();
	CSpaceObject *pSource = Ctx.GetSource();
	CInstalledDevice *pDevice = Ctx.GetDevice();

	DamageTypes iDamageType;
	CString sReference;
	CItemEnhancement Mods = Item.GetMods();

	//	Get the root class. This is different from this class in the case of
	//	autodefense devices, which embed a weapon.

	CDeviceClass *pRootItem = Item.GetType()->GetDeviceClass();

	//	Fire rate

	int iFireRate = (pRootItem ? pRootItem->GetActivateDelay(pDevice, pSource) : m_iFireRate);
	CString sFireRate = GetReferenceFireRate(pDevice ? pDevice->GetActivateDelay(pSource) : Mods.GetEnhancedRate(iFireRate));

	//	Compute the damage string and special string

	if (m_ShotData.GetCount() != 1 && iVariant == -1)
		{
		sReference = CONSTLIT("missile weapon");

		sReference.Append(sFireRate);
		iDamageType = damageGeneric;
		}
	else
		{
		//	Get the damage

		int iFragments;
		CWeaponFireDesc *pShot = GetReferenceShotData(GetVariant(iVariant != -1 ? iVariant : 0), &iFragments);
		DamageDesc Damage = pShot->m_Damage;
		iDamageType = Damage.GetDamageType();

		//	Modify the damage based on any enhancements that the ship may have

		if (pSource && pDevice)
			Damage.AddEnhancements(pDevice->GetEnhancements());
		else
			Damage.AddBonus(Mods.GetHPBonus());

		//	Get description

		CString sDamage = Damage.GetDesc(DamageDesc::flagAverageDamage);

		//	Add the multiplier

		int iMult = (int)CalcConfigurationMultiplier(pShot);
		if (iMult > 1)
			sDamage.Append(strPatternSubst(CONSTLIT(" (x%d)"), iMult));

		if (pShot->m_iFireType == ftParticles)
			sDamage.Append(CONSTLIT(" (max)"));

		sReference.Append(sDamage);

		//	Compute fire rate

		sReference.Append(sFireRate);
		}

	//	Done

	if (retiDamage)
		*retiDamage = iDamageType;

	if (retsReference)
		*retsReference = sReference;

	return true;
	}

CWeaponFireDesc *CWeaponClass::GetReferenceShotData (CWeaponFireDesc *pShot, int *retiFragments) const

//	GetReferenceShotData
//
//	If a fragment of pShot does more damage than the shot itself, then
//	return the fragment (since it is more representative)

	{
	CWeaponFireDesc *pBestShot = pShot;
	Metric rBestDamage = pShot->m_Damage.GetAverageDamage();
	int iBestFragments = 1;

	CWeaponFireDesc::SFragmentDesc *pFragDesc = pShot->GetFirstFragment();
	while (pFragDesc)
		{
		int iFragments = pFragDesc->Count.GetAveValue();
		switch (pFragDesc->pDesc->m_iFireType)
			{
			//	Area damage counts as multiple hits

			case ftArea:
				iFragments *= 3;
				break;

			//	Radius damage always hits (if in range)

			case ftRadius:
				break;

			//	For others, only some of the fragments will hit

			default:
				if (!pFragDesc->pDesc->IsTracking())
					iFragments /= 8;
				break;
			}

		Metric rDamage = pFragDesc->pDesc->m_Damage.GetAverageDamage() * iFragments;
		if (rDamage > rBestDamage)
			{
			pBestShot = pFragDesc->pDesc;
			rBestDamage = rDamage;
			iBestFragments = iFragments;
			}

		pFragDesc = pFragDesc->pNext;
		}

	if (retiFragments)
		*retiFragments = iBestFragments;

	return pBestShot;
	}

CWeaponFireDesc *CWeaponClass::GetSelectedShotData (CItemCtx &Ctx)

//	GetSelectedShotData
//
//	Returns the currently selected shot data (or NULL if none)

	{
	int iSelection = GetCurrentVariant(Ctx.GetDevice());
	if (iSelection != -1 && iSelection < m_ShotData.GetCount())
		return GetVariant(iSelection);
	else
		return NULL;
	}

void CWeaponClass::GetSelectedVariantInfo (CSpaceObject *pSource, 
										   CInstalledDevice *pDevice,
										   CString *retsLabel,
										   int *retiAmmoLeft,
										   CItemType **retpType)

//	GetSelectedVariantInfo
//
//	Returns information about the selected variant

	{
	CItemListManipulator ItemList(pSource->GetItemList());
	CItemCtx Ctx(pSource, pDevice);
	CWeaponFireDesc *pShot = GetSelectedShotData(Ctx);
	if (pShot)
		{
		if (pShot->m_pAmmoType == NULL)
			{
			if (retsLabel)
				*retsLabel = CString();
			if (retiAmmoLeft)
				{
				if (m_bCharges)
					*retiAmmoLeft = pDevice->GetCharges(pSource);
				else
					*retiAmmoLeft = -1;
				}
			if (retpType)
				*retpType = GetItemType();
			}
		else
			{
			CItem Item(pShot->m_pAmmoType, 1);

			if (retiAmmoLeft)
				{
				if (ItemList.SetCursorAtItem(Item))
					*retiAmmoLeft = ItemList.GetItemAtCursor().GetCount();
				else
					*retiAmmoLeft = 0;
				}

			if (retsLabel)
				{
				if (GetCategory() == itemcatLauncher)
					*retsLabel = Item.GetNounPhrase(nounCapitalize);
				else
					*retsLabel = CString();
				}

			if (retpType)
				*retpType = pShot->m_pAmmoType;
			}
		}
	else
		{
		if (retsLabel)
			*retsLabel = CString();
		if (retiAmmoLeft)
			*retiAmmoLeft = 0;
		if (retpType)
			*retpType = NULL;
		}
	}

int CWeaponClass::GetStdDamage (int iLevel)

//	GetStdDamage
//
//	Returns standard damage at this level.

	{
	SStdWeaponStats *pStd = GetStdWeaponStats(iLevel);
	if (pStd == NULL)
		return 0;

	return pStd->iDamage;
	}

int CWeaponClass::GetValidVariantCount (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	GetValidVariantCount
//
//	Returns the number of valid variants for this weapons

	{
	int iCount = 0;

	for (int i = 0; i < m_ShotData.GetCount(); i++)
		{
		if (VariantIsValid(pSource, pDevice, *GetVariant(i)))
			iCount++;
		}

	return iCount;
	}

int CWeaponClass::GetWeaponEffectiveness (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget)

//	GetWeaponEffectiveness
//
//	Returns:
//
//	< 0		If the weapon is ineffective against the target
//	0		If the weapon has normal effect on target
//	1-100	If the weapon is particularly effective against the target
//
//	This call is used to figure out whether we should use an EMP or blinder
//	cannon against the target.

	{
	int iScore = 0;

	CItemCtx Ctx(pSource, pDevice);
	CWeaponFireDesc *pShot = GetSelectedShotData(Ctx);
	if (pShot == NULL)
		return -100;

	//	If we don't enough ammo, clearly we will not be effective

	if (pShot->m_pAmmoType)
		{
		CItemListManipulator ItemList(pSource->GetItemList());
		CItem Item(pShot->m_pAmmoType, 1);
		if (!ItemList.SetCursorAtItem(Item))
			return -100;
		}

	//	Check some targeting options

	if (m_bTargetStationsOnly 
			&& (pTarget == NULL || pTarget->GetCategory() != CSpaceObject::catStation))
		return -100;

	//	Check our state

	switch (m_Counter)
		{
		//	If we're overheating, we will not be effective

		case cntTemperature:
			if (pDevice->IsWaiting() && pDevice->GetTemperature() > 0)
				return -100;

			if (pDevice->GetTemperature() + m_iCounterActivate >= MAX_COUNTER)
				{
				pDevice->SetWaiting(true);
				return -100;
				}

			pDevice->SetWaiting(false);
			break;

		//	If our capacitor is discharged, we will not be effective

		case cntCapacitor:
			if (pDevice->IsWaiting() && pDevice->GetTemperature() < MAX_COUNTER)
				return -100;

			if (pDevice->GetTemperature() < m_iCounterActivate)
				{
				pDevice->SetWaiting(true);
				return -100;
				}

			pDevice->SetWaiting(false);
			break;
		}

	//	If the weapon has EMP damage and the target is not paralysed then
	//	this is very effective.

	if (pTarget && pShot->m_Damage.GetEMPDamage() > 0)
		{
		if (pTarget->IsParalyzed())
			iScore -= 50;
		else
			iScore += 100;
		}

	//	If the weapon has blinding damage and the target is not blind then
	//	this is very effective

	if (pTarget && pShot->m_Damage.GetBlindingDamage() > 0)
		{
		if (pTarget->IsBlind())
			iScore -= 50;
		else
			iScore += 100;
		}

	//	Score

	return iScore;
	}

bool CWeaponClass::IsAmmoWeapon (void)

//	IsAmmoWeapon
//
//	Returns TRUE if weapon uses ammo

	{
	return (m_bLauncher
			|| (m_ShotData.GetCount() > 0 && m_ShotData[0].pAmmoType));
	}

bool CWeaponClass::IsAreaWeapon (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	IsAreaWeapon
//
//	Is this a weapon with an area of effect

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponFireDesc *pShot = GetSelectedShotData(Ctx);
	if (pShot == NULL)
		return false;

	if (pShot->m_iFireType == ftArea)
		return true;

	if (pShot->HasFragments() && pShot->GetFirstFragment()->pDesc->m_iFireType == ftArea)
		return true;

	return false;
	}

bool CWeaponClass::IsDirectional (CInstalledDevice *pDevice, int *retiMinFireArc, int *retiMaxFireArc)

//	IsDirectional
//
//	Returns TRUE if the weapon can turn but is not omni

	{
	//	If the weapon is omnidirectional then we don't need directional 
	//	calculations.

	if (m_bOmnidirectional || (pDevice && pDevice->IsOmniDirectional()))
		return false;

	//	If we have a device, combine the fire arcs of device slot and weapon

	if (pDevice)
		{
		//	If the device is directional then we always take the fire arc from
		//	the device slot.

		if (pDevice->IsDirectional())
			{
			if (retiMinFireArc)
				*retiMinFireArc = pDevice->GetMinFireArc();
			if (retiMaxFireArc)
				*retiMaxFireArc = pDevice->GetMaxFireArc();

			return true;
			}

		//	Otherwise, see if the weapon is directional.

		else if (m_iMinFireArc != m_iMaxFireArc)
			{
			//	If the device points in a specific direction then we offset the
			//	weapon's fire arc.

			int iDeviceSlotOffset = pDevice->GetMinFireArc();

			if (retiMinFireArc)
				*retiMinFireArc = (m_iMinFireArc + iDeviceSlotOffset) % 360;
			if (retiMaxFireArc)
				*retiMaxFireArc = (m_iMaxFireArc + iDeviceSlotOffset) % 360;

			return true;
			}

		//	Otherwise, we are not directional

		else
			return false;
		}
	else
		{
		//	Otherwise, just check the weapon

		if (retiMinFireArc)
			*retiMinFireArc = m_iMinFireArc;
		if (retiMaxFireArc)
			*retiMaxFireArc = m_iMaxFireArc;

		return (m_iMinFireArc != m_iMaxFireArc);
		}
	}

bool CWeaponClass::IsOmniDirectional (CInstalledDevice *pDevice)

//	IsOmniDirectional
//
//	Returns TRUE if the weapon is omnidirectional (not limited)

	{
	//	The device slot improves the weapon. If the device slot is directional, then
	//	the weapon is directional. If the device slot is omni directional, then the
	//	weapon is omnidirectional.

	if (pDevice && pDevice->IsOmniDirectional())
		return true;

	return m_bOmnidirectional;
	}

bool CWeaponClass::IsStdDamageType (DamageTypes iDamageType, int iLevel)

//	IsStdDamageType
//
//	Returns TRUE if the given damage type is standard at the given level.

	{
	int iTierLevel = ::GetDamageTypeLevel(iDamageType);

	return (iLevel >= iTierLevel && iLevel < iTierLevel + 3);
	}

bool CWeaponClass::IsTrackingWeapon (CItemCtx &Ctx)

//	IsTrackingWeapon
//
//	Returns TRUE if we're a tracking weapon
	
	{
	CWeaponFireDesc *pShot = GetSelectedShotData(Ctx);
	if (pShot == NULL)
		return false;

	return pShot->IsTracking();
	}

bool CWeaponClass::IsVariantSelected (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	IsVariantSelected
//
//	Returns TRUE if we've selected some variant (i.e., we haven't selected 0xffff)

	{
	return (GetCurrentVariant(pDevice) != -1);
	}

bool CWeaponClass::IsWeaponAligned (CSpaceObject *pShip, 
									CInstalledDevice *pDevice, 
									CSpaceObject *pTarget, 
									int *retiAimAngle, 
									int *retiFireAngle)

//	IsWeaponAligned
//
//	Return TRUE if weapon is aligned on target.
//
//	Note: If the weapon is invalid, we return an aim angle of -1

	{
	CItemCtx Ctx(pShip, pDevice);
	CWeaponFireDesc *pShot = GetSelectedShotData(Ctx);
	if (pShot == NULL)
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

	if (IsOmniDirectional(pDevice))
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

	//	Figure out our aim tolerance

	int iAimTolerance;
	switch (m_Configuration)
		{
		case ctDual:
		case ctDualAlternating:
			iAimTolerance = 10;
			break;

		case ctSpread2:
			iAimTolerance = 20;
			break;

		case ctSpread3:
		case ctSpread5:
			iAimTolerance = 30;
			break;

		case ctWall:
			iAimTolerance = 45;
			break;

		case ctCustom:
			iAimTolerance = 2 * m_iConfigAimTolerance;
			break;

		default:
			{
			if (m_iFireRate > 10)
				iAimTolerance = 2;
			else if (m_iFireRate > 4)
				iAimTolerance = 4;
			else
				iAimTolerance = 6;
			}
		}

	//	If this is a directional weapon, figure out whether the target
	//	is in the fire arc

	int iMinFireAngle;
	int iMaxFireAngle;
	bool bDirectional = IsDirectional(pDevice, &iMinFireAngle, &iMaxFireAngle);

	//	Tracking weapons behave like directional weapons with 120 degree field

	if (!bDirectional && pShot->IsTracking())
		{
		iMinFireAngle = 300;
		iMaxFireAngle = 60;
		bDirectional = true;
		}

	if (bDirectional)
		{
		int iMin = (pShip->GetRotation() + iMinFireAngle - iAimTolerance + 360) % 360;
		int iMax = (pShip->GetRotation() + iMaxFireAngle + iAimTolerance) % 360;

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

	//	Fire angle

	if (retiFireAngle)
		*retiFireAngle = iFacingAngle;

	//	Compute the angular size of the target

	int iHalfAngularSize = (int)(30.0 * pTarget->GetHitSize() / rDist);

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

bool CWeaponClass::NeedsAutoTarget (CItemCtx &Ctx, int *retiMinFireArc, int *retiMaxFireArc)

//	NeedsAutoTarget
//
//	Returns TRUE if this weapon is either a swivel weapon, an omniweapon, or a
//	tracking weapon.
//
//	If necessary we return the (absolute) fire arc where we should look for 
//	targets (if we're a swivel weapon).

	{
	//	If we're a tracking weapon then we have no swivel restrictions

	if (IsTrackingWeapon(Ctx))
		{
		if (retiMinFireArc) *retiMinFireArc = 0;
		if (retiMaxFireArc) *retiMaxFireArc = 0;
		return true;
		}

	//	If we're an omni or swivel weapon, adjust our fire arc

	int iMinFireArc = 0;
	int iMaxFireArc = 0;
	if (CanRotate(Ctx, &iMinFireArc, &iMaxFireArc))
		{
		//	Adjust arc based on player rotation

		if (iMinFireArc != iMaxFireArc
				&& Ctx.GetSource())
			{
			iMinFireArc = AngleMod(iMinFireArc + Ctx.GetSource()->GetRotation());
			iMaxFireArc = AngleMod(iMaxFireArc + Ctx.GetSource()->GetRotation());
			}

		if (retiMinFireArc) *retiMinFireArc = iMinFireArc;
		if (retiMaxFireArc) *retiMaxFireArc = iMaxFireArc;

		return true;
		}

	//	Otherwise, we don't need a target

	return false;
	}

void CWeaponClass::OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	OnAddTypesUsed
//
//	Adds types used by this class

	{
	int i;

	for (i = 0; i < m_ShotData.GetCount(); i++)
		if (m_ShotData[i].bOwned)
			m_ShotData[i].pDesc->AddTypesUsed(retTypesUsed);
	}

ALERROR CWeaponClass::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Done loading all design elements

	{
	DEBUG_TRY

	ALERROR error;

	//	Events

	GetItemType()->InitCachedEvents(evtCount, CACHED_EVENTS, m_CachedEvents);

	//	Shots

	for (int i = 0; i < m_ShotData.GetCount(); i++)
		{
		//	Bind the ammoID

		if (error = m_ShotData[i].pAmmoType.Bind(Ctx))
			return error;

		//	If we own this definition, then we need to bind it.

		if (m_ShotData[i].bOwned)
			{
			if (error = m_ShotData[i].pDesc->OnDesignLoadComplete(Ctx))
				return error;
			}

		//	Otherwise, all we have to do is point to the descriptor from
		//	the item.

		else
			{
			//	Must be valid

			if (m_ShotData[i].pAmmoType == NULL)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Unable to find ItemType for ammoID: %x"), m_ShotData[i].pAmmoType.GetUNID());
				return ERR_FAIL;
				}

			//	Get the descriptor

			m_ShotData[i].pDesc = m_ShotData[i].pAmmoType->GetMissileDesc();
			if (m_ShotData[i].pDesc == NULL)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("ItemType must have <Missile> definition: %x"), m_ShotData[i].pAmmoType.GetUNID());
				return ERR_FAIL;
				}
			}
		}

	return NOERROR;

	DEBUG_CATCH
	}

CEffectCreator *CWeaponClass::OnFindEffectCreator (const CString &sUNID)

//	OnFindEffectCreator
//
//	Find the effect. We start after the weapon class UNID.

	{
	//	We start after the weapon class UNID

	char *pPos = sUNID.GetASCIIZPointer();
	if (*pPos != '/')
		return NULL;

	pPos++;

	//	Parse the variant

	int iOrdinal = strParseInt(pPos, 0, &pPos);
	if (iOrdinal < 0 || iOrdinal >= GetVariantCount())
		return NULL;

	//	Now ask the weapon fire desc to parse the remaining UNID

	CWeaponFireDesc *pDesc = GetVariant(iOrdinal);
	return pDesc->FindEffectCreator(CString(pPos));
	}

void CWeaponClass::OnMarkImages (void)

//	OnMarkImages
//
//	Load all images that we need

	{
	int i;

	for (i = 0; i < m_ShotData.GetCount(); i++)
		m_ShotData[i].pDesc->MarkImages();
	}

bool CWeaponClass::RequiresItems (void)

//	RequiresItems
//
//	Returns TRUE if this weapon requires ammo

	{
	if (m_ShotData.GetCount() == 0)
		return false;
	else if (m_ShotData.GetCount() == 1)
		return (m_ShotData[0].pAmmoType != NULL);
	else
		return true;
	}

bool CWeaponClass::SelectFirstVariant (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	SelectFirstVariant
//
//	Selects the first valid variant for this weapon

	{
	SetCurrentVariant(pDevice, -1);
	return SelectNextVariant(pSource, pDevice);
	}

bool CWeaponClass::SelectNextVariant (CSpaceObject *pSource, CInstalledDevice *pDevice, int iDir)

//	SelectNextVariant
//
//	Selects the next valid variant for this weapon

	{
	int iStart = GetCurrentVariant(pDevice);
	if (iStart == -1)
		iStart = m_ShotData.GetCount() + (iDir == 1 ? 0 : -1);
	else
		iStart = m_ShotData.GetCount() + iStart + iDir;

	//	Loop over all variants

	int iSelection = -1;
	if (m_ShotData.GetCount() > 0)
		{
		for (int i = 0; i < m_ShotData.GetCount(); i++)
			{
			int iTry = (iStart + (i * iDir)) % m_ShotData.GetCount();

			//	If this variant is valid, then we're done

			if (VariantIsValid(pSource, pDevice, *GetVariant(iTry)))
				{
				iSelection = iTry;
				break;
				}
			}
		}

	//	If we found a selection, then select the variant

	SetCurrentVariant(pDevice, iSelection);
	return (iSelection != -1);
	}

void CWeaponClass::SetAlternatingPos (CInstalledDevice *pDevice, int iAlternatingPos)

//	SetAlternatingPos
//
//	Sets the alternating position

	{
	pDevice->SetData((pDevice->GetData() & 0xFFFF00FF) | (((DWORD)iAlternatingPos & 0xFF) << 8));
	}

void CWeaponClass::SetContinuousFire (CInstalledDevice *pDevice, DWORD dwContinuous)

//	SetContinuousFire
//
//	Sets the continuous fire counter for the device

	{
	pDevice->SetData((pDevice->GetData() & 0xFFFFFF00) | (dwContinuous & 0xFF));
	}

void CWeaponClass::SetCurrentVariant (CInstalledDevice *pDevice, int iVariant)

//	SetCurrentVariant
//
//	Sets the current variant for the device

	{
	pDevice->SetData((((DWORD)(WORD)(short)iVariant) << 16) | LOWORD(pDevice->GetData()));
	}

void CWeaponClass::Update (CInstalledDevice *pDevice, CSpaceObject *pSource, int iTick, bool *retbSourceDestroyed, bool *retbConsumedItems)

//	Update
//
//	Update

	{
	CItemCtx Ctx(pSource, pDevice);

	if (retbConsumedItems)
		*retbConsumedItems = false;
	if (retbSourceDestroyed)
		*retbSourceDestroyed = false;

	//	Change counter on update

	if (IsCounterEnabled() && (iTick % m_iCounterUpdateRate) == 0)
		{
		if (m_iCounterUpdate > 0)
			{
			if (pDevice->GetTemperature() < MAX_COUNTER)
				{
				pDevice->IncTemperature(Min(m_iCounterUpdate, MAX_COUNTER - pDevice->GetTemperature()));
				pSource->OnComponentChanged(comDeviceCounter);
				}
			}
		else
			{
			if (pDevice->GetTemperature() > 0)
				{
				pDevice->IncTemperature(Max(m_iCounterUpdate, -pDevice->GetTemperature()));
				pSource->OnComponentChanged(comDeviceCounter);
				}
			}
		}

	//	Done if we're disabled

	if (!pDevice->IsEnabled())
		return;

	//	See if we continue to fire

	DWORD dwContinuous = GetContinuousFire(pDevice);
	if (dwContinuous == CONTINUOUS_START)
		{
		//	-1 is used to skip the first update cycle
		//	(which happens on the same tick as Activate)

		CWeaponFireDesc *pShot = GetSelectedShotData(Ctx);
		SetContinuousFire(pDevice, (pShot ? pShot->m_iContinuous : 0));
		}
	else if (dwContinuous > 0)
		{
		CWeaponFireDesc *pShot = GetSelectedShotData(Ctx);
		if (pShot)
			{
			bool bSourceDestroyed;

			FireWeapon(pDevice, 
					pShot, 
					pSource, 
					NULL, 
					1 + pShot->m_iContinuous - dwContinuous,
					&bSourceDestroyed,
					retbConsumedItems);

			if (bSourceDestroyed)
				{
				if (retbSourceDestroyed)
					*retbSourceDestroyed = true;
				return;
				}
			}

		dwContinuous--;
		SetContinuousFire(pDevice, dwContinuous);
		}
	}

bool CWeaponClass::ValidateSelectedVariant (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	ValidateSelectedVariant
//
//	If the selected variant is valid, then it returns TRUE. Otherwise,
//	it selects a different valid variant. If not valid variants are found,
//	it returns FALSE

	{
	CItemCtx Ctx(pSource, pDevice);
	CWeaponFireDesc *pShot = GetSelectedShotData(Ctx);
	if (pShot && VariantIsValid(pSource, pDevice, *pShot))
		return true;

	if (SelectNextVariant(pSource, pDevice))
		return true;

	if (SelectFirstVariant(pSource, pDevice))
		return true;

	return false;
	}

bool CWeaponClass::VariantIsValid (CSpaceObject *pSource, CInstalledDevice *pDevice, CWeaponFireDesc &ShotData)

//	VariantIsValid
//
//	Returns TRUE if the variant is valid

	{
	//	If we have charges, then we're only valid if we have charges left.

	if (m_bCharges)
		{
		if (pDevice->GetCharges(pSource) == 0)
			return false;
		}

	//	If we do not need ammo, then we're always valid

	if (ShotData.GetAmmoType() == NULL)
		return true;

	//	Otherwise, check to see if we have enough ammo

	CItemListManipulator ItemList(pSource->GetItemList());
	CItem Item(ShotData.GetAmmoType(), 1);
	if (ItemList.SetCursorAtItem(Item))
		return true;

	//	Not valid

	return false;
	}

