//	CArmorClass.cpp
//
//	CArmorClass class

#include "PreComp.h"

#define BLINDING_DAMAGE_ADJ_ATTRIB				CONSTLIT("blindingDamageAdj")
#define BLINDING_IMMUNE_ATTRIB					CONSTLIT("blindingImmune")
#define COMPLETE_BONUS_ATTRIB					CONSTLIT("completeBonus")
#define DAMAGE_ADJ_LEVEL_ATTRIB					CONSTLIT("damageAdjLevel")
#define DECAY_ATTRIB							CONSTLIT("decay")
#define DECAY_RATE_ATTRIB						CONSTLIT("decayRate")
#define DEVICE_DAMAGE_ADJ_ATTRIB				CONSTLIT("deviceDamageAdj")
#define DEVICE_DAMAGE_IMMUNE_ATTRIB				CONSTLIT("deviceDamageImmune")
#define DISINTEGRATION_IMMUNE_ATTRIB			CONSTLIT("disintegrationImmune")
#define EMP_DAMAGE_ADJ_ATTRIB					CONSTLIT("EMPDamageAdj")
#define EMP_IMMUNE_ATTRIB						CONSTLIT("EMPImmune")
#define INSTALL_COST_ATTRIB						CONSTLIT("installCost")
#define INSTALL_COST_ADJ_ATTRIB					CONSTLIT("installCostAdj")
#define MAX_HP_BONUS_ATTRIB						CONSTLIT("maxHPBonus")
#define PHOTO_RECHARGE_ATTRIB					CONSTLIT("photoRecharge")
#define PHOTO_REPAIR_ATTRIB						CONSTLIT("photoRepair")
#define POWER_USE_ATTRIB						CONSTLIT("powerUse")
#define RADIATION_IMMUNE_ATTRIB					CONSTLIT("radiationImmune")
#define REFLECT_ATTRIB							CONSTLIT("reflect")
#define REGEN_ATTRIB							CONSTLIT("regen")
#define REPAIR_COST_ADJ_ATTRIB					CONSTLIT("repairCostAdj")
#define REPAIR_RATE_ATTRIB						CONSTLIT("repairRate")
#define REPAIR_TECH_ATTRIB						CONSTLIT("repairTech")
#define SHATTER_IMMUNE_ATTRIB					CONSTLIT("shatterImmune")
#define SHIELD_INTERFERENCE_ATTRIB				CONSTLIT("shieldInterference")
#define STEALTH_ATTRIB							CONSTLIT("stealth")
#define UNID_ATTRIB								CONSTLIT("unid")

#define GET_MAX_HP_EVENT						CONSTLIT("GetMaxHP")
#define ON_ARMOR_DAMAGE_EVENT					CONSTLIT("OnArmorDamage")

#define FIELD_ADJUSTED_HP						CONSTLIT("adjustedHP")
#define FIELD_BALANCE							CONSTLIT("balance")
#define FIELD_DAMAGE_ADJ						CONSTLIT("damageAdj")
#define FIELD_EFFECTIVE_HP						CONSTLIT("effectiveHP")
#define FIELD_HP								CONSTLIT("hp")
#define FIELD_HP_BONUS							CONSTLIT("hpBonus")
#define FIELD_INSTALL_COST						CONSTLIT("installCost")
#define FIELD_REGEN								CONSTLIT("regen")
#define FIELD_REPAIR_COST						CONSTLIT("repairCost")
#define FIELD_SHIELD_INTERFERENCE				CONSTLIT("shieldInterference")

#define PROPERTY_BLINDING_IMMUNE				CONSTLIT("blindingImmune")
#define PROPERTY_COMPLETE_HP					CONSTLIT("completeHP")
#define PROPERTY_DEVICE_DAMAGE_IMMUNE			CONSTLIT("deviceDamageImmune")
#define PROPERTY_DEVICE_DISRUPT_IMMUNE			CONSTLIT("deviceDisruptImmune")
#define PROPERTY_DISINTEGRATION_IMMUNE			CONSTLIT("disintegrationImmune")
#define PROPERTY_EMP_IMMUNE						CONSTLIT("EMPImmune")
#define PROPERTY_HP								CONSTLIT("hp")
#define PROPERTY_RADIATION_IMMUNE				CONSTLIT("radiationImmune")
#define PROPERTY_SHATTER_IMMUNE					CONSTLIT("shatterImmune")

static CObjectClass<CArmorClass>g_Class(OBJID_CARMORCLASS, NULL);

static char g_HitPointsAttrib[] = "hitPoints";
static char g_RepairCostAttrib[] = "repairCost";
static char g_DamageAdjAttrib[] = "damageAdj";
static char g_ItemIDAttrib[] = "itemID";
#define MAX_REFLECTION_CHANCE		95

#define MAX_REFLECTION_CHANCE		95

const int BLIND_IMMUNE_LEVEL =					6;
const int RADIATION_IMMUNE_LEVEL =				7;
const int EMP_IMMUNE_LEVEL =					9;
const int DEVICE_DAMAGE_IMMUNE_LEVEL =			11;

const int TICKS_PER_UPDATE =					10;

struct SStdStats
	{
	int iHP;									//	HP for std armor at this level
	int iCost;									//	Std cost at this level
	int iRepairCost;							//	Cost to repair 1 hp
	int iInstallCost;							//	Cost to install
	int iMass;									//	Standard mass
	};

static SStdStats STD_STATS[MAX_ITEM_LEVEL] =
	{
		//						Repair	Install
		//	HP		Cost		cost	cost		Mass
		{	35,		50,			1,		10,			2500, },	
		{	45,		100,		1,		20,			2600, },
		{	60,		200,		1,		40,			2800, },
		{	80,		400,		2,		80,			2900, },
		{	100,	800,		3,		160,		3000, },

		{	135,	1600,		4,		320,		3200, },
		{	175,	3200,		6,		640,		3300, },
		{	225,	6500,		9,		1300,		3500, },
		{	300,	13000,		15,		2600,		3700, },
		{	380,	25000,		22,		5000,		3900, },

		{	500,	50000,		35,		10000,		4000, },
		{	650,	100000,		52,		20000,		4300, },
		{	850,	200000,		80,		40000,		4500, },
		{	1100,	410000,		125,	82000,		4700, },
		{	1400,	820000,		190,	164000,		5000, },

		{	1850,	1600000,	300,	320000,		5200, },
		{	2400,	3250000,	450,	650000,		5500, },
		{	3100,	6500000,	700,	1300000,	5700, },
		{	4000,	13000000,	1050,	2600000,	6000, },
		{	5250,	26000000,	1650,	5200000,	6300, },

		{	6850,	52000000,	2540,	10400000,	6600, },
		{	9000,	100000000,	3900,	20000000,	7000, },
		{	12000,	200000000,	6000,	40000000,	7300, },
		{	15000,	400000000,	9300,	80000000,	7700, },
		{	20000,	800000000,	14300,	160000000,	8000, },
	};

static char *CACHED_EVENTS[CArmorClass::evtCount] =
	{
		"GetMaxHP",
		"OnArmorDamage",
	};

CArmorClass::CArmorClass (void) : CObject(&g_Class),
		m_pItemType(NULL)

//	CArmorClass constructor

	{
	}

EDamageResults CArmorClass::AbsorbDamage (CItemCtx &ItemCtx, SDamageCtx &Ctx)

//	AbsorbDamage
//
//	Handles getting hit by damage.
//
//	Returns damageNoDamage if all the damage was absorbed and no further processing is necessary
//	Returns damageDestroyed if the source was destroyed
//	Returns damageArmorHit if source was damage and further processing (destroy check) is needed
//
//	Sets Ctx.iDamage to the amount of hit points left after damage absorption.

	{
	DEBUG_TRY

	CSpaceObject *pSource = ItemCtx.GetSource();
	CInstalledArmor *pArmor = ItemCtx.GetArmor();
	if (pSource == NULL || pArmor == NULL)
		return damageNoDamage;

	//	Compute all the effects (this initializes elements in Ctx).

	CalcDamageEffects(ItemCtx, Ctx);

	//	First give custom weapons a chance

	bool bCustomDamage = Ctx.pDesc->FireOnDamageArmor(Ctx);
	if (pSource->IsDestroyed())
		return damageDestroyed;

	//	Damage adjustment

	CalcAdjustedDamage(ItemCtx, Ctx);

	//	If the armor has custom code to deal with damage, handle it here.

	FireOnArmorDamage(ItemCtx, Ctx);
	if (pSource->IsDestroyed())
		return damageDestroyed;

	//	If this armor section reflects this kind of damage then
	//	send the damage on

	if (Ctx.bReflect)
		{
		if (Ctx.pCause)
			Ctx.pCause->CreateReflection(Ctx.vHitPos, (Ctx.iDirection + 120 + mathRandom(0, 120)) % 360);
		return damageNoDamage;
		}

	//	If this is a disintegration attack, then disintegrate the ship

	if (Ctx.bDisintegrate)
		{
		if (!pSource->OnDestroyCheck(killedByDisintegration, Ctx.Attacker))
			return damageNoDamage;

		pSource->Destroy(killedByDisintegration, Ctx.Attacker);
		return damageDestroyed;
		}

	//	If this is a shatter attack, see if the ship is destroyed

	if (Ctx.bShatter)
		{
		if (!pSource->OnDestroyCheck(killedByShatter, Ctx.Attacker))
			return damageNoDamage;

		pSource->Destroy(killedByShatter, Ctx.Attacker);
		return damageDestroyed;
		}

	//	If this is a paralysis attack and we've gotten past the shields
	//	then freeze the ship.

	if (Ctx.bParalyze)
		pSource->MakeParalyzed(Ctx.iParalyzeTime);

	//	If this is blinding damage then our sensors are disabled

	if (Ctx.bBlind)
		pSource->MakeBlind(Ctx.iBlindTime);

	//	If this attack is radioactive, then contaminate the ship

	if (Ctx.bRadioactive)
		pSource->OnHitByRadioactiveDamage(Ctx);

	//	If this is device damage, then see if any device is damaged

	if (Ctx.bDeviceDamage)
		pSource->OnHitByDeviceDamage();

	if (Ctx.bDeviceDisrupt)
		pSource->OnHitByDeviceDisruptDamage(Ctx.iDisruptTime);

	//	Create a hit effect. (Many weapons show an effect even if no damage was
	//	done.)

	Ctx.pDesc->CreateHitEffect(pSource->GetSystem(), Ctx);

	//	If no damage has reached us, then we're done

	if (Ctx.iDamage == 0 && !bCustomDamage)
		return damageNoDamage;

	//	Give source events a chance to change the damage before we
	//	subtract from armor.

	if (pSource->HasOnDamageEvent())
		{
		pSource->FireOnDamage(Ctx);
		if (pSource->IsDestroyed())
			return damageDestroyed;
		}

	//	Take damage

	if (Ctx.iDamage <= pArmor->GetHitPoints())
		{
		pArmor->IncHitPoints(-Ctx.iDamage);
		Ctx.iDamage = 0;
		}
	else
		{
		Ctx.iDamage -= pArmor->GetHitPoints();
		pArmor->SetHitPoints(0);
		}

	return damageArmorHit;

	DEBUG_CATCH
	}

void CArmorClass::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds types used by this class

	{
	}

void CArmorClass::CalcAdjustedDamage (CItemCtx &ItemCtx, SDamageCtx &Ctx)

//	CalcAdjustedDamage
//
//	Modifies Ctx.iDamage to account for damage type adjustments, etc.

	{
	CInstalledArmor *pArmor = ItemCtx.GetArmor();

	//	Adjust for special armor damage:
	//
	//	<0	=	2.5x damage
	//	0	=	2x damage
	//	1	=	1.5x damage
	//	2	=	1.25x damage
	//	>2	=	1x damage

	int iDamageLevel = Ctx.Damage.GetArmorDamageLevel();
	if (iDamageLevel > 0)
		Ctx.iDamage = (CalcArmorDamageAdj(Ctx.Damage) * Ctx.iDamage + 50) / 100;

	//	Adjust for damage type

	int iDamageAdj = GetDamageAdj((pArmor ? pArmor->GetMods() : CItemEnhancement()), Ctx.Damage);
	Ctx.iDamage = (iDamageAdj * Ctx.iDamage + 50) / 100;
	}

int CArmorClass::CalcArmorDamageAdj (const DamageDesc &Damage) const

//	CalcArmorDamageAdj
//
//	Adjust for special armor damage:
//
//	<0	=	2.5x damage
//	0	=	2x damage
//	1	=	1.5x damage
//	2	=	1.25x damage
//	>2	=	1x damage

	{
	int iDamageLevel = Damage.GetArmorDamageLevel();
	if (iDamageLevel <= 0)
		return 100;

	int iDiff = m_pItemType->GetLevel() - iDamageLevel;

	switch (iDiff)
		{
		case 0:
			return 200;

		case 1:
			return 150;

		case 2:
			return 125;

		default:
			if (iDiff < 0)
				return 250;
			else
				return 100;
		}
	}

int CArmorClass::CalcAverageRelativeDamageAdj (void)

//	CalcAverageRelativeDamageAdj
//
//	Compares this armor against standard damage adj for the level and returns
//	the average damage adj relative to the standard.

	{
	int i;
	Metric rTotalAdj = 0.0;
	int iCount = 0;
	int iArmorLevel = m_pItemType->GetLevel();

	for (i = damageLaser; i < damageCount; i++)
		{
		//	If this damage type is within two levels of the armor level,
		//	then we include it in our calculations.

		int iDamageLevel = ::GetDamageTypeLevel((DamageTypes)i);
		if (iArmorLevel < iDamageLevel + 5
				&& iArmorLevel > iDamageLevel - 3)
			{
			int iStdAdj = GetStdDamageAdj(iArmorLevel, (DamageTypes)i);
			int iDamageAdj = GetDamageAdj((DamageTypes)i);

			rTotalAdj += (iStdAdj > 0.0 ? (Metric)iDamageAdj * 100.0 / iStdAdj : 1000.0);
			iCount++;
			}
		}

	//	Return average

	return (iCount > 0 ? (int)((rTotalAdj / iCount) + 0.5) : 100);
	}

int CArmorClass::CalcBalance (void)

//	CalcBalance
//
//	Determines whether the given item is balanced for its level. Negative numbers
//	mean the item is underpowered. Positive numbers mean the item is overpowered.

	{
	int i;
	int iBalance = 0;
	int iLevel = m_pItemType->GetLevel();

	//	Regeneration

	if (!m_Regen.IsEmpty())
		{
		if (m_fPhotoRepair)
			iBalance += m_Regen.GetHPPerEra();
		else
			iBalance += 5 * m_Regen.GetHPPerEra();
		}

	//	Stealth

	if (m_iStealth >= 12)
		iBalance += 4;
	else if (m_iStealth >= 10)
		iBalance += 3;
	else if (m_iStealth >= 8)
		iBalance += 2;
	else if (m_iStealth >= 6)
		iBalance += 1;

	//	Immunities

	if (m_fDisintegrationImmune)
		{
		if (iLevel <= 10)
			iBalance += 3;
		}

	if (m_iBlindingDamageAdj <= 20 && iLevel < BLIND_IMMUNE_LEVEL)
		iBalance += 1;
	else if (m_iBlindingDamageAdj > 0 && iLevel >= BLIND_IMMUNE_LEVEL)
		iBalance -= 1;

	if (m_fRadiationImmune && iLevel < RADIATION_IMMUNE_LEVEL)
		iBalance += 2;
	else if (!m_fRadiationImmune && iLevel >= RADIATION_IMMUNE_LEVEL)
		iBalance -= 2;

	if (m_iEMPDamageAdj <= 20 && iLevel < EMP_IMMUNE_LEVEL)
		iBalance += 2;
	else if (m_iEMPDamageAdj > 0 && iLevel >= EMP_IMMUNE_LEVEL)
		iBalance -= 2;

	if (m_iDeviceDamageAdj <= 20 && iLevel < DEVICE_DAMAGE_IMMUNE_LEVEL)
		iBalance += 2;
	else if (m_iDeviceDamageAdj > 0 && iLevel >= DEVICE_DAMAGE_IMMUNE_LEVEL)
		iBalance -= 2;

	if (m_iBlindingDamageAdj > 20 || m_iEMPDamageAdj > 20 || m_iDeviceDamageAdj > 20)
		{
		if (m_iBlindingDamageAdj <= 33 || m_iEMPDamageAdj <= 33 || m_iDeviceDamageAdj <= 33)
			iBalance += 2;
		else if (m_iBlindingDamageAdj <= 50 || m_iEMPDamageAdj <= 50 || m_iDeviceDamageAdj <= 50)
			iBalance += 1;
		}

	if (m_fPhotoRecharge)
		iBalance += 2;

	//	Matched sets

	if (m_iArmorCompleteBonus)
		{
		int iPercent = m_iArmorCompleteBonus * 100 / m_iHitPoints;
		iBalance += (iPercent + 5) / 10;
		}

	//	Damage Adjustment

	int iBalanceAdj = 0;
	for (i = 0; i < damageCount; i++)
		{
		int iStdAdj;
		int iDamageAdj;
		m_DamageAdj.GetAdjAndDefault((DamageTypes)i, &iDamageAdj, &iStdAdj);

		if (iStdAdj != iDamageAdj)
			{
			if (iDamageAdj > 0)
				{
				int iBonus = (int)((100.0 * (iStdAdj - iDamageAdj) / iDamageAdj) + 0.5);

				if (iBonus > 0)
					iBalanceAdj += iBonus / 4;
				else
					iBalanceAdj -= ((int)((100.0 * iDamageAdj / iStdAdj) + 0.5) - 100) / 4;
				}
			else if (iStdAdj > 0)
				{
				iBalanceAdj += iStdAdj;
				}
			}
		}

	iBalance += (Max(Min(iBalanceAdj, 100), -100)) / 5;

	//	Reflection

	for (i = 0; i < damageCount; i++)
		{
		if (m_Reflective.InSet((DamageTypes)i))
			iBalance += 8;
		}

	//	Hit Points

	if (m_iHitPoints > 0)
		{
		int iDiff = (m_iHitPoints - STD_STATS[iLevel - 1].iHP);
		if (iDiff > 0)
			iBalance += iDiff * 20 / STD_STATS[iLevel - 1].iHP;
		else if (m_iHitPoints > 0)
			iBalance -= (STD_STATS[iLevel - 1].iHP * 20 / m_iHitPoints) - 20;
		else
			iBalance -= 40;
		}

	//	Mass

	int iMass = CItem(m_pItemType, 1).GetMassKg();
	if (iMass > 0)
		{
		int iDiff = (iMass - STD_STATS[iLevel - 1].iMass);

		//	Armor twice as massive can have double the hit points

		if (iDiff > 0)
			iBalance -= iDiff * 20 / STD_STATS[iLevel - 1].iMass;

		//	Armor half as massive can have 3/4 hit points

		else if (iMass > 0)
			iBalance += (STD_STATS[iLevel - 1].iMass * 10 / iMass) - 10;
		else
			iBalance += 100;
		}

	//	Repair tech

	int iRepair = iLevel - m_iRepairTech;
	if (iRepair < 0)
		iBalance += 2 * iRepair;
	else if (iRepair > 0)
		iBalance += iRepair;

	//	Repair cost

	int iStdRepairCost = STD_STATS[m_iRepairTech - 1].iRepairCost;
	int iDiff = iStdRepairCost - m_iRepairCost;
	if (iDiff < 0)
		iBalance += Max(-8, 2 * iDiff / iStdRepairCost);
	else if (iDiff > 0)
		iBalance += 5 * iDiff / iStdRepairCost;

	//	Power consumption

	if (m_iPowerUse)
		{
		int iPercent = m_iPowerUse * 100 / CShieldClass::GetStdPower(iLevel);
		iBalance -= iPercent;
		}

	//	Meteorsteel

	if (m_fShieldInterference)
		iBalance -= 12;

	//	Decay

	if (!m_Decay.IsEmpty())
		{
		iBalance -= 4 * m_Decay.GetHPPerEra();
		}

	return 5 * iBalance;
	}

void CArmorClass::CalcDamageEffects (CItemCtx &ItemCtx, SDamageCtx &Ctx)

//	CalcDamageEffects
//
//	Initialize the damage effects based on the damage and on this armor type.

	{
	CSpaceObject *pSource = ItemCtx.GetSource();
	CInstalledArmor *pArmor = ItemCtx.GetArmor();

	//	Compute all the effects (if we don't have installed armor, then the 
	//	caller is responsible for setting this).

	if (pArmor)
		Ctx.iHPLeft = pArmor->GetHitPoints();

	//	Reflect

	Ctx.bReflect = (IsReflective(ItemCtx, Ctx.Damage) && Ctx.iDamage > 0);

	//	Disintegration

	int iDisintegration = Ctx.Damage.GetDisintegrationDamage();
	Ctx.bDisintegrate = (iDisintegration > 0 && !IsDisintegrationImmune(ItemCtx));

	//	Shatter

	int iShatter = Ctx.Damage.GetShatterDamage();
	if (iShatter)
		{
		//	Compute the threshold mass. Below this size, we shatter the object

		int iMassLimit = 10 * mathPower(5, iShatter);
		Ctx.bShatter = (pSource && pSource->GetMass() < iMassLimit);
		}
	else
		Ctx.bShatter = false;

	//	Blinding

	int iBlinding = Ctx.Damage.GetBlindingDamage();
	if (iBlinding && !IsBlindingDamageImmune(ItemCtx))
		{
		//	The chance of being blinded is dependent
		//	on the rating.

		int iChance = 4 * iBlinding * iBlinding * GetBlindingDamageAdj() / 100;
		Ctx.bBlind = (mathRandom(1, 100) <= iChance);
		Ctx.iBlindTime = Ctx.iDamage * g_TicksPerSecond / 2;
		}
	else
		Ctx.bBlind = false;

	//	EMP

	int iEMP = Ctx.Damage.GetEMPDamage();
	if (iEMP && !IsEMPDamageImmune(ItemCtx))
		{
		//	The chance of being paralyzed is dependent
		//	on the EMP rating.

		int iChance = 4 * iEMP * iEMP * GetEMPDamageAdj() / 100;
		Ctx.bParalyze = (mathRandom(1, 100) <= iChance);
		Ctx.iParalyzeTime = Ctx.iDamage * g_TicksPerSecond / 2;
		}
	else
		Ctx.bParalyze = false;

	//	Device disrupt

	int iDeviceDisrupt = Ctx.Damage.GetDeviceDisruptDamage();
	if (iDeviceDisrupt && !IsDeviceDamageImmune(ItemCtx))
		{
		//	The chance of damaging a device depends on the rating.

		int iChance = 4 * iDeviceDisrupt * iDeviceDisrupt * GetDeviceDamageAdj() / 100;
		Ctx.bDeviceDisrupt = (mathRandom(1, 100) <= iChance);
		Ctx.iDisruptTime = 2 * Ctx.iDamage * g_TicksPerSecond;
		}
	else
		Ctx.bDeviceDisrupt = false;

	//	Device damage

	int iDeviceDamage = Ctx.Damage.GetDeviceDamage();
	if (iDeviceDamage && !IsDeviceDamageImmune(ItemCtx))
		{
		//	The chance of damaging a device depends on the rating.

		int iChance = 4 * iDeviceDamage * iDeviceDamage * GetDeviceDamageAdj() / 100;
		Ctx.bDeviceDamage = (mathRandom(1, 100) <= iChance);
		}
	else
		Ctx.bDeviceDamage = false;

	//	Radiation

	int iRadioactive = Ctx.Damage.GetRadiationDamage();
	Ctx.bRadioactive = (iRadioactive > 0 && !IsRadiationImmune(ItemCtx));

	//	Some effects decrease damage

	if (iBlinding || iEMP)
		Ctx.iDamage = 0;
	else if (iDeviceDamage)
		Ctx.iDamage = Ctx.iDamage / 2;
	}

int CArmorClass::CalcPowerUsed (CInstalledArmor *pArmor)

//	CalcPowerUsed
//
//	Computes the amount of power used by this armor segment (this
//	only applies to powered armor)

	{
	return m_iPowerUse;
	}

ALERROR CArmorClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CArmorClass **retpArmor)

//	CreateFromXML
//
//	Create armor class desc

	{
	ALERROR error;
	CArmorClass *pArmor;
	CString sValue;

	ASSERT(pType);

	pArmor = new CArmorClass;
	if (pArmor == NULL)
		return ERR_MEMORY;

	int iLevel = (pType ? pType->GetLevel() : 1);

	pArmor->m_pItemType = pType;
	pArmor->m_iHitPoints = pDesc->GetAttributeIntegerBounded(CONSTLIT(g_HitPointsAttrib), 0);
	pArmor->m_iArmorCompleteBonus = pDesc->GetAttributeIntegerBounded(COMPLETE_BONUS_ATTRIB, 0);

	//	Regen & Decay

	if (error = pArmor->m_Regen.InitFromXML(Ctx, pDesc, REGEN_ATTRIB, REPAIR_RATE_ATTRIB, NULL_STR, TICKS_PER_UPDATE))
		return error;

	if (error = pArmor->m_Decay.InitFromXML(Ctx, pDesc, DECAY_ATTRIB, DECAY_RATE_ATTRIB, NULL_STR, TICKS_PER_UPDATE))
		return error;

	//	Install cost based on level

	int iInstallCostAdj = pDesc->GetAttributeIntegerBounded(INSTALL_COST_ADJ_ATTRIB, 0, -1, 100);
	pArmor->m_iInstallCost = iInstallCostAdj * pDesc->GetAttributeIntegerBounded(INSTALL_COST_ATTRIB, 0, -1, STD_STATS[iLevel - 1].iInstallCost) / 100;

	//	Repair tech defaults to level

	pArmor->m_iRepairTech = pDesc->GetAttributeIntegerBounded(REPAIR_TECH_ATTRIB, 1, MAX_TECH_LEVEL, iLevel);

	//	Repair cost is based on repair tech

	int iRepairCostAdj = pDesc->GetAttributeIntegerBounded(REPAIR_COST_ADJ_ATTRIB, 0, -1, 100);
	pArmor->m_iRepairCost = iRepairCostAdj * pDesc->GetAttributeIntegerBounded(CONSTLIT(g_RepairCostAttrib), 0, -1, STD_STATS[pArmor->m_iRepairTech - 1].iRepairCost) / 100;

	//	Load the new damage adjustment structure

	pArmor->m_iDamageAdjLevel = pDesc->GetAttributeIntegerBounded(DAMAGE_ADJ_LEVEL_ATTRIB, 1, MAX_ITEM_LEVEL, iLevel);
	if (error = pArmor->m_DamageAdj.InitFromXML(Ctx, pDesc))
		return error;

	//	Blind-immune

	bool bValue;
	if (pDesc->FindAttributeBool(BLINDING_IMMUNE_ATTRIB, &bValue))
		pArmor->m_iBlindingDamageAdj = (bValue ? 0 : 100);
	else
		pArmor->m_iBlindingDamageAdj = pDesc->GetAttributeIntegerBounded(BLINDING_DAMAGE_ADJ_ATTRIB, 
				0, 
				-1, 
				iLevel >= BLIND_IMMUNE_LEVEL ? 0 : 100);

	//	Radiation-immune

	if (pDesc->FindAttributeBool(RADIATION_IMMUNE_ATTRIB, &bValue))
		pArmor->m_fRadiationImmune = bValue;
	else
		pArmor->m_fRadiationImmune = (iLevel >= RADIATION_IMMUNE_LEVEL ? true : false);

	//	EMP-immune

	if (pDesc->FindAttributeBool(EMP_IMMUNE_ATTRIB, &bValue))
		pArmor->m_iEMPDamageAdj = (bValue ? 0 : 100);
	else
		pArmor->m_iEMPDamageAdj = pDesc->GetAttributeIntegerBounded(EMP_DAMAGE_ADJ_ATTRIB, 
				0, 
				-1, 
				iLevel >= EMP_IMMUNE_LEVEL ? 0 : 100);

	//	Device damage immune

	if (pDesc->FindAttributeBool(DEVICE_DAMAGE_IMMUNE_ATTRIB, &bValue))
		pArmor->m_iDeviceDamageAdj = (bValue ? 0 : 100);
	else
		pArmor->m_iDeviceDamageAdj = pDesc->GetAttributeIntegerBounded(DEVICE_DAMAGE_ADJ_ATTRIB, 
				0,
				-1, 
				iLevel >= DEVICE_DAMAGE_IMMUNE_LEVEL ? 0 : 100);

	//	Shatter immune

	pArmor->m_fShatterImmune = pDesc->GetAttributeBool(SHATTER_IMMUNE_ATTRIB);

	//	Disintegration immune

	pArmor->m_fDisintegrationImmune = pDesc->GetAttributeBool(DISINTEGRATION_IMMUNE_ATTRIB);

	pArmor->m_fPhotoRepair = pDesc->GetAttributeBool(PHOTO_REPAIR_ATTRIB);
	pArmor->m_fPhotoRecharge = pDesc->GetAttributeBool(PHOTO_RECHARGE_ATTRIB);
	pArmor->m_fShieldInterference = pDesc->GetAttributeBool(SHIELD_INTERFERENCE_ATTRIB);

	pArmor->m_iStealth = pDesc->GetAttributeInteger(STEALTH_ATTRIB);
	if (pArmor->m_iStealth == 0)
		pArmor->m_iStealth = CSpaceObject::stealthNormal;

	pArmor->m_iPowerUse = pDesc->GetAttributeIntegerBounded(POWER_USE_ATTRIB, 0);
	pArmor->m_iMaxHPBonus = pDesc->GetAttributeIntegerBounded(MAX_HP_BONUS_ATTRIB, 0, -1, 150);

	//	Load reflection

	if (error = pArmor->m_Reflective.InitFromXML(pDesc->GetAttribute(REFLECT_ATTRIB)))
		return error;

	//	Done

	*retpArmor = pArmor;

	return NOERROR;
	}

bool CArmorClass::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	int i;

	if (strEquals(sField, FIELD_HP))
		*retsValue = strFromInt(m_iHitPoints);
	else if (strEquals(sField, FIELD_BALANCE))
		*retsValue = strFromInt(CalcBalance());
	else if (strEquals(sField, FIELD_EFFECTIVE_HP))
		{
		int iHP;
		int iHPbyDamageType[damageCount];
		GetReferenceDamageAdj(NULL, NULL, &iHP, iHPbyDamageType);
		*retsValue = strFromInt(::CalcEffectiveHP(m_pItemType->GetLevel(), iHP, iHPbyDamageType));
		}
	else if (strEquals(sField, FIELD_ADJUSTED_HP))
		{
		int iHP;
		int iHPbyDamageType[damageCount];
		GetReferenceDamageAdj(NULL, NULL, &iHP, iHPbyDamageType);

		CString sResult;
		for (i = 0; i < damageCount; i++)
			{
			if (i > 0)
				sResult.Append(CONSTLIT("\t"));
			sResult.Append(strFromInt(iHPbyDamageType[i]));
			}
		*retsValue = sResult;
		}
	else if (strEquals(sField, FIELD_DAMAGE_ADJ))
		{
		retsValue->Truncate(0);

		for (i = 0; i < damageCount; i++)
			{
			if (i > 0)
				retsValue->Append(CONSTLIT("\t"));

			retsValue->Append(strFromInt(m_DamageAdj.GetAdj((DamageTypes)i)));
			}
		}
	else if (strEquals(sField, FIELD_HP_BONUS))
		{
		CString sResult;

		for (i = 0; i < damageCount; i++)
			{
			if (i > 0)
				sResult.Append(CONSTLIT(", "));

			int iBonus = m_DamageAdj.GetHPBonus((DamageTypes)i);
			if (iBonus == -100)
				sResult.Append(CONSTLIT("***"));
			else
				sResult.Append(strPatternSubst(CONSTLIT("%3d"), iBonus));
			}

		*retsValue = sResult;
		}
	else if (strEquals(sField, FIELD_REPAIR_COST))
		*retsValue = strFromInt(m_iRepairCost);
	else if (strEquals(sField, FIELD_REGEN))
		*retsValue = strFromInt((int)m_Regen.GetHPPer180());
	else if (strEquals(sField, FIELD_INSTALL_COST))
		*retsValue = strFromInt(m_iInstallCost);
	else if (strEquals(sField, FIELD_SHIELD_INTERFERENCE))
		{
		if (m_fShieldInterference)
			*retsValue = CONSTLIT("True");
		else
			*retsValue = NULL_STR;
		}
	else
		return false;

	return true;
	}

int CArmorClass::FireGetMaxHP (CItemCtx &ItemCtx, int iMaxHP) const

//	FireGetMaxHP
//
//	Fire GetMaxHP event

	{
	SEventHandlerDesc Event;
	if (FindEventHandlerArmorClass(evtGetMaxHP, &Event))
		{
		//	Setup arguments

		CCodeChainCtx Ctx;
		Ctx.SaveAndDefineSourceVar(ItemCtx.GetSource());
		Ctx.SaveAndDefineItemVar(ItemCtx);

		Ctx.DefineInteger(CONSTLIT("aMaxHP"), iMaxHP);

		ICCItem *pResult = Ctx.Run(Event);

		if (pResult->IsError())
			ItemCtx.GetSource()->ReportEventError(GET_MAX_HP_EVENT, pResult);
		else if (!pResult->IsNil())
			iMaxHP = Max(0, pResult->GetIntegerValue());

		Ctx.Discard(pResult);
		}

	return iMaxHP;
	}

void CArmorClass::FireOnArmorDamage (CItemCtx &ItemCtx, SDamageCtx &Ctx)

//	FireOnArmorDamage
//
//	Fires OnArmorDamage event

	{
	SEventHandlerDesc Event;
	if (FindEventHandlerArmorClass(evtOnArmorDamage, &Event))
		{
		//	Setup arguments

		CCodeChainCtx CCCtx;
		CCCtx.SaveAndDefineSourceVar(ItemCtx.GetSource());
		CCCtx.SaveAndDefineItemVar(ItemCtx);

		CCCtx.DefineInteger(CONSTLIT("aArmorHP"), Ctx.iHPLeft);
		CCCtx.DefineInteger(CONSTLIT("aArmorSeg"), Ctx.iSectHit);
		CCCtx.DefineSpaceObject(CONSTLIT("aAttacker"), Ctx.Attacker.GetObj());
		CCCtx.DefineSpaceObject(CONSTLIT("aCause"), Ctx.pCause);
		CCCtx.DefineDamageEffects(CONSTLIT("aDamageEffects"), Ctx);
		CCCtx.DefineInteger(CONSTLIT("aDamageHP"), Ctx.iDamage);
		CCCtx.DefineString(CONSTLIT("aDamageType"), GetDamageShortName(Ctx.Damage.GetDamageType()));
		CCCtx.DefineInteger(CONSTLIT("aHitDir"), Ctx.iDirection);
		CCCtx.DefineVector(CONSTLIT("aHitPos"), Ctx.vHitPos);
		CCCtx.DefineSpaceObject(CONSTLIT("aOrderGiver"), (Ctx.Attacker.GetObj() ? Ctx.Attacker.GetObj()->GetOrderGiver(Ctx.Attacker.GetCause()) : NULL));
		CCCtx.DefineItemType(CONSTLIT("aWeaponType"), Ctx.pDesc->GetWeaponType());

		ICCItem *pResult = CCCtx.Run(Event);

		//	If we return Nil, then nothing

		if (pResult->IsNil())
			NULL;

		//	If we return an integer, then this is the damage that armor should take

		else if (pResult->IsInteger())
			Ctx.iDamage = pResult->GetIntegerValue();

		//	If we return a list, then we it to be a DamageEffects list (modifications to
		//	aDamageEffects)

		else if (pResult->IsList())
			LoadDamageEffectsFromItem(pResult, Ctx);

		CCCtx.Discard(pResult);
		}
	}

int CArmorClass::GetDamageAdj (CItemEnhancement Mods, const DamageDesc &Damage)

//	GetDamageAdj
//
//	Returns the damage adjustment for the given damage type

	{
	int iDamageAdj = GetDamageAdj(Damage.GetDamageType());

	if (Mods.IsNotEmpty())
		return iDamageAdj * Mods.GetDamageAdj(Damage) / 100;
	else
		return iDamageAdj;
	}

int CArmorClass::GetDamageAdjForWeaponLevel (int iLevel)

//	GetDamageAdjForWeaponLevel
//
//	Returns the least effective damage adjustment of all damage types appropriate
//	to the given level.

	{
	int i;

	int iBestAdj = 0;
	for (i = 0; i < damageCount; i++)
		{
		if (CWeaponClass::IsStdDamageType((DamageTypes)i, iLevel))
			{
			int iDamageAdj = GetDamageAdj((DamageTypes)i);
			if (iDamageAdj > iBestAdj)
				iBestAdj = iDamageAdj;
			}
		}

	return iBestAdj;
	}

int CArmorClass::GetDamageEffectiveness (CSpaceObject *pAttacker, CInstalledDevice *pWeapon)

//	GetDamageEffectiveness
//
//	Returns the effectiveness of the given weapon against this shield.
//
//	< 0		The weapon is ineffective against us.
//	0-99	The weapon is less effective than average.
//	100		The weapon has average effectiveness
//	> 100	The weapon is more effective than average.

	{
	const DamageDesc *pDamage = pWeapon->GetDamageDesc(CItemCtx(pAttacker, pWeapon));
	if (pDamage == NULL)
		return 100;

	int iBonus = m_DamageAdj.GetHPBonus(pDamage->GetDamageType());
	if (iBonus <= -100)
		return -1;

	//	Compute score based on bonus

	int iScore;
	if (iBonus <= 0)
		iScore = 100 - iBonus;
	else
		iScore = 100 - Min(100, (iBonus / 2));

	//	See if we do extra damage

	if (pDamage->GetArmorDamageLevel())
		iScore += (CalcArmorDamageAdj(*pDamage) / 2);

	//	Done

	return iScore;
	}

ICCItem *CArmorClass::GetItemProperty (CItemCtx &Ctx, const CString &sName)

//	GetItemProperty
//
//	Returns armor property

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	if (strEquals(sName, PROPERTY_BLINDING_IMMUNE))
		return CC.CreateBool(IsBlindingDamageImmune(Ctx));

	else if (strEquals(sName, PROPERTY_COMPLETE_HP))
		return CC.CreateInteger(GetMaxHP(Ctx, true));

	else if (strEquals(sName, PROPERTY_DEVICE_DAMAGE_IMMUNE))
		return CC.CreateBool(IsDeviceDamageImmune(Ctx));

	else if (strEquals(sName, PROPERTY_DEVICE_DISRUPT_IMMUNE))
		return CC.CreateBool(IsDeviceDamageImmune(Ctx));

	else if (strEquals(sName, PROPERTY_DISINTEGRATION_IMMUNE))
		return CC.CreateBool(IsDisintegrationImmune(Ctx));

	else if (strEquals(sName, PROPERTY_EMP_IMMUNE))
		return CC.CreateBool(IsEMPDamageImmune(Ctx));

	else if (strEquals(sName, PROPERTY_HP))
		return CC.CreateInteger(GetMaxHP(Ctx));

	else if (strEquals(sName, PROPERTY_RADIATION_IMMUNE))
		return CC.CreateBool(IsRadiationImmune(Ctx));

	else if (strEquals(sName, PROPERTY_SHATTER_IMMUNE))
		return CC.CreateBool(IsShatterImmune(Ctx));

	else if (m_pItemType)
		return CreateResultFromDataField(CC, m_pItemType->GetDataField(sName));

	else
		return CC.CreateNil();
	}

int CArmorClass::GetMaxHP (CItemCtx &ItemCtx, bool bForceComplete)

//	GetMaxHP
//
//	Returns the max HP for this kind of armor

	{
	//	Start with hit points defined by the class

	int iHP = m_iHitPoints;

	//	Fire event to compute HP, if necessary

	iHP = FireGetMaxHP(ItemCtx, iHP);

	//	Add mods

	const CItemEnhancement &Mods = ItemCtx.GetMods();
	if (Mods.IsNotEmpty())
		iHP = iHP * Mods.GetHPAdj() / 100;

	//	Add complete bonus

	CInstalledArmor *pSect = ItemCtx.GetArmor();
	if (bForceComplete || (pSect && pSect->IsComplete()))
		iHP += m_iArmorCompleteBonus;

	//	Done

	return iHP;
	}

CString CArmorClass::GetReference (CItemCtx &Ctx, int iVariant)
	
//	GetReference
//
//	Returns a string that describes the basic attributes
//	of this armor.
//
//	Example:
//
//		30 hp; laser-resistant; impact-resistant

	{
	int i;
	CString sReference;

	//	Get modifications

	int iLevel = m_pItemType->GetLevel();
	const CItemEnhancement &Mods = Ctx.GetMods();

	//	Radiation 

	if (m_fRadiationImmune || Mods.IsRadiationImmune())
		{
		if (iLevel < RADIATION_IMMUNE_LEVEL)
			AppendReferenceString(&sReference, CONSTLIT("radiation-immune"));
		}
	else if (iLevel >= RADIATION_IMMUNE_LEVEL)
		AppendReferenceString(&sReference, CONSTLIT("radiation-vulnerable"));

	//	If we're immune to blinding/EMP/device damage, then collapse
	//	it all under a single entry

	bool bCheckedBlind = false;
	bool bCheckedEMP = false;
	bool bCheckedDevice = false;

	if ((m_iBlindingDamageAdj == 0 || Mods.IsBlindingImmune())
			&& (m_iEMPDamageAdj == 0 || Mods.IsEMPImmune())
			&& (m_iDeviceDamageAdj < 100 || Mods.IsDeviceDamageImmune()))
		{
		if (iLevel < DEVICE_DAMAGE_IMMUNE_LEVEL)
			AppendReferenceString(&sReference, CONSTLIT("ion effect-immune"));

		bCheckedBlind = true;
		bCheckedEMP = true;
		bCheckedDevice = true;
		}

	//	Collapse blind and EMP resistance

	else if ((m_iBlindingDamageAdj == 0 || Mods.IsBlindingImmune())
			&& (m_iEMPDamageAdj == 0 || Mods.IsEMPImmune()))
		{
		if (iLevel < EMP_IMMUNE_LEVEL)
			AppendReferenceString(&sReference, CONSTLIT("blind-, EMP-immune"));

		bCheckedBlind = true;
		bCheckedEMP = true;
		}
	else if ((m_iBlindingDamageAdj < 100) && (iLevel < BLIND_IMMUNE_LEVEL)
			&& (m_iEMPDamageAdj < 100))
		{
		AppendReferenceString(&sReference, CONSTLIT("blind-, EMP-resistant"));

		bCheckedBlind = true;
		bCheckedEMP = true;
		}

	//	Otherwise, treat each separate
	//
	//	Blindness

	if (!bCheckedBlind)
		{
		if (m_iBlindingDamageAdj == 0 || Mods.IsBlindingImmune())
			{
			if (iLevel < BLIND_IMMUNE_LEVEL)
				AppendReferenceString(&sReference, CONSTLIT("blind-immune"));
			}
		else if (m_iBlindingDamageAdj < 100)
			{
			if (iLevel < BLIND_IMMUNE_LEVEL)
				AppendReferenceString(&sReference, CONSTLIT("blind-resistant"));
			else
				AppendReferenceString(&sReference, CONSTLIT("blind-vulnerable"));
			}
		else if (iLevel >= BLIND_IMMUNE_LEVEL)
			AppendReferenceString(&sReference, CONSTLIT("blind-vulnerable"));
		}

	//	EMP

	if (!bCheckedEMP)
		{
		if (m_iEMPDamageAdj == 0 || Mods.IsEMPImmune())
			{
			if (iLevel < EMP_IMMUNE_LEVEL)
				AppendReferenceString(&sReference, CONSTLIT("EMP-immune"));
			}
		else if (m_iEMPDamageAdj < 100)
			{
			if (iLevel < EMP_IMMUNE_LEVEL)
				AppendReferenceString(&sReference, CONSTLIT("EMP-resistant"));
			else
				AppendReferenceString(&sReference, CONSTLIT("EMP-vulnerable"));
			}
		else if (iLevel >= EMP_IMMUNE_LEVEL)
			AppendReferenceString(&sReference, CONSTLIT("EMP-vulnerable"));
		}

	//	Device damage

	if (!bCheckedDevice)
		{
		if (m_iDeviceDamageAdj < 100 || Mods.IsDeviceDamageImmune())
			{
			if (iLevel < DEVICE_DAMAGE_IMMUNE_LEVEL)
				AppendReferenceString(&sReference, CONSTLIT("device-protect"));
			}
		else if (iLevel >= DEVICE_DAMAGE_IMMUNE_LEVEL)
			AppendReferenceString(&sReference, CONSTLIT("device-vulnerable"));
		}

	//	Disintegration

	if (m_fDisintegrationImmune || Mods.IsDisintegrationImmune())
		AppendReferenceString(&sReference, CONSTLIT("disintegrate-immune"));

	//	Shatter

	if (IsShatterImmune(Ctx))
		AppendReferenceString(&sReference, CONSTLIT("shatter-immune"));

	//	Shield interference

	if (m_fShieldInterference || Mods.IsShieldInterfering())
		AppendReferenceString(&sReference, CONSTLIT("no-shields"));

	//	Photo repair

	if (m_fPhotoRepair || Mods.IsPhotoRegenerating())
		AppendReferenceString(&sReference, CONSTLIT("photo-repair"));

	//	Solar power

	if (m_fPhotoRecharge || Mods.IsPhotoRecharge())
		AppendReferenceString(&sReference, CONSTLIT("solar"));

	//	Regeneration

	if ((!m_Regen.IsEmpty() && !m_fPhotoRepair) || Mods.IsRegenerating())
		AppendReferenceString(&sReference, CONSTLIT("regenerate"));

	//	Decay

	if (!m_Decay.IsEmpty() || Mods.IsDecaying())
		AppendReferenceString(&sReference, CONSTLIT("decay"));

	//	Reflection

	for (i = 0; i < damageCount; i++)
		{
		if (m_Reflective.InSet((DamageTypes)i)
				|| (Mods.IsReflective() && Mods.GetDamageType() == i))
			AppendReferenceString(&sReference, strPatternSubst(CONSTLIT("%s-reflecting"), GetDamageShortName((DamageTypes)i)));
		}

	//	Done

	return sReference;
	}

bool CArmorClass::GetReferenceDamageAdj (const CItem *pItem, CSpaceObject *pInstalled, int *retiHP, int *retArray)

//	GetReferenceDamageAdj
//
//	Returns armor HP after adjustment for damage type

	{
	int i;

	CItemCtx ItemCtx(pItem, pInstalled);
	int iHP = GetMaxHP(ItemCtx);

	if (retiHP)
		*retiHP = iHP;

	for (i = 0; i < damageCount; i++)
		{
		DamageDesc Damage((DamageTypes)i, DiceRange(0, 0, 0));
		int iAdj = GetDamageAdj(ItemCtx.GetMods(), Damage);

		if (retArray)
			retArray[i] = CalcHPDamageAdj(iHP, iAdj);
		}

	return true;
	}

CString CArmorClass::GetShortName (void)

//	GetShortName
//
//	Returns the short name. This will basically omit the leading "segment of"
//	from the item name.

	{
	return m_pItemType->GetNounPhrase(nounShort);
	}

int CArmorClass::GetStdCost (int iLevel)

//	GetStdCost
//
//	Returns standard cost by level

	{
	ASSERT(iLevel >= 1 && iLevel <= MAX_ITEM_LEVEL);
	return STD_STATS[iLevel - 1].iCost;
	}

int CArmorClass::GetStdDamageAdj (int iLevel, DamageTypes iDamage)

//	GetStdDamageAdj
//
//	Returns the standard damage adjustment for the given damage at
//	the given level

	{
	ASSERT(iLevel >= 1 && iLevel <= MAX_ITEM_LEVEL);
	return g_pUniverse->GetArmorDamageAdj(iLevel)->GetAdj(iDamage);
	}

int CArmorClass::GetStdEffectiveHP (int iLevel)

//	GetStdEffectiveHP
//
//	Returns effective HP by level

	{
	int i;
	ASSERT(iLevel >= 1 && iLevel <= MAX_ITEM_LEVEL);

	int iHPbyDamageType[damageCount];
	for (i = 0; i < damageCount; i++)
		iHPbyDamageType[i] = CalcHPDamageAdj(STD_STATS[iLevel - 1].iHP, g_pUniverse->GetArmorDamageAdj(iLevel)->GetAdj((DamageTypes)i));

	return ::CalcEffectiveHP(iLevel, STD_STATS[iLevel - 1].iHP, iHPbyDamageType);
	}

int CArmorClass::GetStdHP (int iLevel)

//	GetStdHP
//
//	Returns standard hp by level

	{
	ASSERT(iLevel >= 1 && iLevel <= MAX_ITEM_LEVEL);
	return STD_STATS[iLevel - 1].iHP;
	}

int CArmorClass::GetStdMass (int iLevel)

//	GetStdMass
//
//	Returns standard mass by level (in kg)

	{
	ASSERT(iLevel >= 1 && iLevel <= MAX_ITEM_LEVEL);
	return STD_STATS[iLevel - 1].iMass;
	}

bool CArmorClass::IsReflective (CItemCtx &ItemCtx, const DamageDesc &Damage)

//	IsReflective
//
//	Returns TRUE if the armor reflects this damage

	{
	const CItemEnhancement &Mods = ItemCtx.GetMods();

	int iReflectChance = 0;

	//	Base armor chance

	if (m_Reflective.InSet(Damage.GetDamageType()))
		iReflectChance = MAX_REFLECTION_CHANCE;

	//	Mods

	int iModReflect;
	if (Mods.IsNotEmpty() && Mods.IsReflective(Damage, &iModReflect))
		iReflectChance = Max(iReflectChance, iModReflect);

	//	Done

	if (iReflectChance)
		{
		CInstalledArmor *pSect = ItemCtx.GetArmor();

		int iMaxHP = GetMaxHP(ItemCtx);
		int iHP = (pSect ? pSect->GetHitPoints() : iMaxHP);

		//	Adjust based on how damaged the armor is

		iReflectChance = (iMaxHP > 0 ? iHP * iReflectChance / iMaxHP : iReflectChance);

		return (mathRandom(1, 100) <= iReflectChance);
		}
	else
		return false;
	}

ALERROR CArmorClass::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Called on Bind

	{
	ALERROR error;

	//	Compute armor damage adjustments

	if (error = m_DamageAdj.Bind(Ctx, g_pUniverse->GetArmorDamageAdj(m_iDamageAdjLevel)))
		return error;

	//	Cache some events

	CItemType *pType = GetItemType();
	pType->InitCachedEvents(evtCount, CACHED_EVENTS, m_CachedEvents);

	return NOERROR;
	}

void CArmorClass::Update (CInstalledArmor *pArmor, CSpaceObject *pObj, int iTick, bool *retbModified)

//	Update
//
//	Updates the armor. This should be called once every 10 ticks

	{
	bool bModified = false;

	//	Compute total regeneration by adding mods to intrinsic

	if (pArmor->GetMods().IsRegenerating()	
			|| pArmor->GetMods().IsPhotoRegenerating()
			|| !m_Regen.IsEmpty())
		{
		CItemCtx ItemCtx(pObj, pArmor);

		int iHPNeeded = GetMaxHP(ItemCtx) - pArmor->GetHitPoints();

		if (iHPNeeded > 0)
			{
			//	Combine all regeneration

			CRegenDesc *pRegen;
			CRegenDesc RegenWithMod;
			if (pArmor->GetMods().IsRegenerating()	|| pArmor->GetMods().IsPhotoRegenerating())
				{
				RegenWithMod.Init(4);
				RegenWithMod.Add(m_Regen);
				pRegen = &RegenWithMod;
				}
			else
				pRegen = &m_Regen;

			//	Compute the HP that we regenerate this cycle

			int iHP = Min(iHPNeeded, pRegen->GetRegen(iTick, TICKS_PER_UPDATE));

			//	If this is photo-repair armor then adjust the cycle
			//	based on how far away we are from the sun.

			if (iHP > 0)
				if (m_fPhotoRepair || pArmor->GetMods().IsPhotoRegenerating())
					{
					int iIntensity = pObj->GetSystem()->CalculateLightIntensity(pObj->GetPos());
					if (mathRandom(1, 100) > iIntensity)
						iHP = 0;
					}

			//	Repair

			if (iHP > 0)
				{
				pArmor->IncHitPoints(iHP);
				bModified = true;
				}
			}
		}

	//	See if we're decaying

	if (pArmor->GetHitPoints() > 0
			&& (pArmor->GetMods().IsDecaying() || !m_Decay.IsEmpty()))
		{
		//	Combine decay with mod

		CRegenDesc *pDecay;
		CRegenDesc DecayWithMod;
		if (pArmor->GetMods().IsDecaying())
			{
			DecayWithMod.Init(4);
			DecayWithMod.Add(m_Decay);
			pDecay = &DecayWithMod;
			}
		else
			pDecay = &m_Decay;

		//	Compute the HP that we decay this cycle

		int iHP = Min(pArmor->GetHitPoints(), pDecay->GetRegen(iTick, TICKS_PER_UPDATE));

		//	Decrement

		if (iHP > 0)
			{
			pArmor->IncHitPoints(-iHP);
			bModified = true;
			}
		}

	//	If this is solar armor then recharge the object

	if (pArmor->GetMods().IsPhotoRecharge() || m_fPhotoRecharge)
		{
		int iIntensity = pObj->GetSystem()->CalculateLightIntensity(pObj->GetPos());

		//	Intensity varies from 0 to 100 so this will recharge up to
		//	100 units of fuel every 10 ticks or 10 units per tick. At 1.5MW per fuel
		//	unit, this means that a single armor plate can support up to 15MW when
		//	right next to the sun.

		pObj->Refuel(iIntensity);
		}

	//	If this armor interferes with shields, then lower shields now

	if (pArmor->GetMods().IsShieldInterfering() || m_fShieldInterference)
		pObj->DeactivateShields();

	//	Done

	*retbModified = bModified;
	}
