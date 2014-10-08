//	CShieldClass.cpp
//
//	CShieldClass class

#include "PreComp.h"

#define HIT_EFFECT_TAG							CONSTLIT("HitEffect")

#define ABSORB_ADJ_ATTRIB						CONSTLIT("absorbAdj")
#define ARMOR_SHIELD_ATTRIB						CONSTLIT("armorShield")
#define DAMAGE_ADJ_ATTRIB						CONSTLIT("damageAdj")
#define DAMAGE_ADJ_LEVEL_ATTRIB					CONSTLIT("damageAdjLevel")
#define DEPLETION_DELAY_ATTRIB					CONSTLIT("depletionDelay")
#define HIT_EFFECT_ATTRIB						CONSTLIT("hitEffect")
#define HIT_POINTS_ATTRIB						CONSTLIT("hitPoints")
#define IDLE_POWER_USE_ATTRIB					CONSTLIT("idlePowerUse")
#define HP_ADJ_PER_CHARGE_ATTRIB				CONSTLIT("HPBonusPerCharge")
#define MAX_CHARGES_ATTRIB						CONSTLIT("maxCharges")
#define POWER_ADJ_PER_CHARGE_ATTRIB				CONSTLIT("powerBonusPerCharge")
#define POWER_USE_ATTRIB						CONSTLIT("powerUse")
#define REFLECT_ATTRIB							CONSTLIT("reflect")
#define REGEN_ATTRIB							CONSTLIT("regen")
#define REGEN_HP_ATTRIB							CONSTLIT("regenHP")
#define REGEN_ADJ_PER_CHARGE_ATTRIB				CONSTLIT("regenHPBonusPerCharge")
#define REGEN_TIME_ATTRIB						CONSTLIT("regenTime")
#define WEAPON_SUPPRESS_ATTRIB					CONSTLIT("weaponSuppress")

#define GET_MAX_HP_EVENT						CONSTLIT("GetMaxHP")
#define ON_SHIELD_DAMAGE_EVENT					CONSTLIT("OnShieldDamage")
#define ON_SHIELD_DOWN_EVENT					CONSTLIT("OnShieldDown")

#define FIELD_ADJUSTED_HP						CONSTLIT("adjustedHP")
#define FIELD_BALANCE							CONSTLIT("balance")
#define FIELD_DAMAGE_ADJ						CONSTLIT("damageAdj")
#define FIELD_EFFECTIVE_HP						CONSTLIT("effectiveHP")
#define FIELD_HP								CONSTLIT("hp")
#define FIELD_HP_BONUS							CONSTLIT("hpBonus")
#define FIELD_POWER								CONSTLIT("power")
#define FIELD_REGEN								CONSTLIT("regen")
#define FIELD_WEAPON_SUPPRESS					CONSTLIT("weaponSuppress")

#define STR_SHIELD_REFLECT						CONSTLIT("reflect")

struct SStdStats
	{
	int iHP;									//	HP for std shield at this level
	int iRegen;									//	HP regen each 180 ticks
	int iCost;									//	Cost for std shield
	int iPower;									//	Power (in tenths of MWs)
	};

#define MAX_REFLECTION_CHANCE		95

static SStdStats STD_STATS[MAX_ITEM_LEVEL] =
	{
		//	HP		Regen	Cost	Power
		{	35,		12,		500,	10, },
		{	45,		16,		1000,	20, },
		{	60,		20,		2000,	50, },
		{	80,		28,		4000,	100, },
		{	100,	36,		8000,	200, },

		{	135,	45,		16000,	300, },
		{	175,	60,		32000,	500, },
		{	225,	80,		65000,	1000, },
		{	300,	100,	130000,	2000, },
		{	380,	130,	250000,	3000, },

		{	500,	170,	500000,	4000, },
		{	650,	220,	-1,		6000, },
		{	850,	290,	-2,		8000, },
		{	1100,	370,	-4,		10000, },
		{	1400,	490,	-8,		12000, },

		{	1850,	630,	-16,	15000, },
		{	2400,	820,	-32,	20000, },
		{	3100,	1100,	-65,	25000, },
		{	4000,	1400,	-130,	30000, },
		{	5250,	1800,	-260,	35000, },

		{	6850,	2350,	-525,	40000, },
		{	9000,	3050,	-1000,	50000, },
		{	12000,	4000,	-2100,	60000, },
		{	15000,	5150,	-4200,	70000, },
		{	20000,	6700,	-8400,	80000, },
	};

static char *CACHED_EVENTS[CShieldClass::evtCount] =
	{
		"GetMaxHP",
		"OnShieldDamage",
		"OnShieldDown",
	};

inline SStdStats *GetStdStats (int iLevel)
	{
	if (iLevel >= 1 && iLevel <= MAX_ITEM_LEVEL)
		return &STD_STATS[iLevel - 1];
	else
		return NULL;
	}

CShieldClass::CShieldClass (void) : CDeviceClass(NULL)
	{
	}

bool CShieldClass::AbsorbDamage (CInstalledDevice *pDevice, CSpaceObject *pShip, SDamageCtx &Ctx)

//	AbsorbDamage
//
//	Absorbs damage.
//	NOTE: We always set Ctx.iAbsorb properly, regardless of the return value.

	{
	DEBUG_TRY

	//	If we're depleted then we cannot absorb anything

	Ctx.iHPLeft = GetHPLeft(pDevice, pShip);
	if (Ctx.iHPLeft == 0)
		{
		Ctx.iAbsorb = 0;
		return false;
		}

	//	Calculate how much we will absorb

	int iAbsorbAdj = (Ctx.Damage.GetDamageType() == damageGeneric ? 100 : m_iAbsorbAdj[Ctx.Damage.GetDamageType()]);
	Ctx.iAbsorb = (Ctx.iDamage * iAbsorbAdj) / 100;
	if (pDevice->GetMods().IsNotEmpty())
		Ctx.iAbsorb = Ctx.iAbsorb * pDevice->GetMods().GetAbsorbAdj(Ctx.Damage) / 100;

	//	Compute how much damage we take (based on the type of damage)

	int iAdj = GetDamageAdj(pDevice->GetMods(), Ctx.Damage);
	Ctx.iShieldDamage = (Ctx.iAbsorb * iAdj) / 100;

	//	If shield generator is damaged then sometimes we take extra damage

	if (pDevice->IsDamaged() || pDevice->IsDisrupted())
		{
		int iRoll = mathRandom(1, 100);

		if (iRoll <= 10)
			Ctx.iAbsorb = 75 * Ctx.iAbsorb / 100;
		else if (iRoll <= 25)
			Ctx.iShieldDamage *= 2;
		}

	//	If the amount of shield damage is greater than HP left, then scale
	//	the amount of HP that we aborb

	if (Ctx.iShieldDamage > Ctx.iHPLeft)
		{
		//	ASSERT: We know that iShieldDamage is > 0 because iHPLeft is > 0.

		Ctx.iAbsorb = Ctx.iHPLeft * Ctx.iAbsorb / Ctx.iShieldDamage;
		Ctx.iShieldDamage = Ctx.iHPLeft;
		}

	//	See if we're reflective

	int iReflectChance;
	if (!pDevice->GetMods().IsReflective(Ctx.Damage, &iReflectChance))
		iReflectChance = 0;
	if (m_Reflective.InSet(Ctx.Damage.GetDamageType()))
		iReflectChance = Max(iReflectChance, MAX_REFLECTION_CHANCE);
	if (iReflectChance 
			&& Ctx.pCause 
			&& Ctx.Damage.GetShieldDamageLevel() == 0)
		{
		//	Compute the chance that we will reflect (based on the strength of
		//	our shields)

		int iMaxHP = GetMaxHP(pDevice, pShip);
		int iEfficiency = (iMaxHP == 0 ? 100 : 50 + (Ctx.iHPLeft * 50 / iMaxHP));
		int iChance = iEfficiency * iReflectChance / 100;

		//	See if we reflect

		if (Ctx.bReflect = (mathRandom(1, 100) <= iChance))
			{
			Ctx.iOriginalAbsorb = Ctx.iAbsorb;
			Ctx.iOriginalShieldDamage = Ctx.iShieldDamage;
			Ctx.iAbsorb = Ctx.iDamage;
			Ctx.iShieldDamage = 0;
			}
		}
	else
		Ctx.bReflect = false;

	//	Give custom damage a chance to react. These events can modify the
	//	following variables:
	//
	//	Ctx.bReflect
	//	Ctx.iAbsorb
	//	Ctx.iShieldDamage
	//
	//	OR
	//
	//	Ctx.iDamage (if we skip further processing)

	if (Ctx.pDesc->FireOnDamageShields(Ctx, pDevice->GetDeviceSlot()))
		return (Ctx.iDamage == 0);

	FireOnShieldDamage(CItemCtx(pShip, pDevice), Ctx);

	//	If we reflect, then create the reflection

	if (Ctx.bReflect)
		Ctx.pCause->CreateReflection(Ctx.vHitPos, (Ctx.iDirection + 120 + mathRandom(0, 120)) % 360);

	//	Create shield effect

	if (Ctx.iAbsorb || Ctx.bReflect)
		{
		if (m_pHitEffect)
			m_pHitEffect->CreateEffect(pShip->GetSystem(),
					NULL,
					Ctx.vHitPos,
					pShip->GetVel(),
					Ctx.iDirection);
		}

	//	Shield takes damage

	if (Ctx.iShieldDamage > 0)
		{
		if (Ctx.iShieldDamage >= Ctx.iHPLeft)
			SetDepleted(pDevice, pShip);
		else
			SetHPLeft(pDevice, Ctx.iHPLeft - Ctx.iShieldDamage);

		pShip->OnComponentChanged(comShields);
		}

	//	Set the remaining damage

	Ctx.iDamage -= Ctx.iAbsorb;
	return (Ctx.iDamage == 0);

	DEBUG_CATCH
	}

bool CShieldClass::AbsorbsWeaponFire (CInstalledDevice *pDevice, CSpaceObject *pSource, CInstalledDevice *pWeapon)

//	AbsorbsWeaponFire
//
//	Returns TRUE if the shield absorbs fire from the given weapon
//	when installed on the same ship

	{
	int iType = pWeapon->GetClass()->GetDamageType(pWeapon);
	if (iType != -1 
			&& m_WeaponSuppress.InSet(iType)
			&& pDevice->IsEnabled()
			&& !IsDepleted(pDevice))
		return true;
	else
		return false;
	}

bool CShieldClass::Activate (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 CSpaceObject *pTarget,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems)

//	Activate
//
//	Activates device

	{
	return false;
	}

int CShieldClass::CalcBalance (void)

//	CalcBalance
//
//	Computes the relative balance of this shield relative to it level

	{
	int i;
	int iBalance = 0;

	SStdStats *pStd = GetStdStats(GetLevel());
	if (pStd == NULL)
		return 0;

	//	Calc HPs. +1 for each 1% above standard HP
	//	If hp is below the standard, then invert the ratio
	//	(i.e., 50% below is 1/0.5 or half strength, which
	//	counts as -100).

	int iMaxHP;
	CalcMinMaxHP(CItemCtx(), m_iMaxCharges, 0, 0, NULL, &iMaxHP);

	int iDiff = (iMaxHP - pStd->iHP);
	if (iDiff > 0)
		iBalance += iDiff * 100 / pStd->iHP;
	else if (iMaxHP > 0)
		iBalance -= (pStd->iHP * 100 / iMaxHP) - 100;
	else
		iBalance -= 200;

	//	Compute the regen HP for each 180 ticks
	//	+1 for each 1% above standard rate

	int iRegen = (int)m_Regen.GetHPPer180();
	iDiff = (iRegen - pStd->iRegen);
	if (iDiff > 0)
		iBalance += iDiff * 100 / pStd->iHP;
	else if (iRegen > 0)
		iBalance -= (pStd->iRegen * 100 / iRegen) - 100;
	else
		iBalance -= 200;

	//	Account for damage adjustments

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

	iBalance += Max(Min(iBalanceAdj, 100), -100);

	//	Done

	return iBalance;
	}

void CShieldClass::CalcMinMaxHP (CItemCtx &Ctx, int iCharges, int iArmorSegs, int iTotalHP, int *retiMin, int *retiMax) const

//	CalcMinMaxHP
//
//	Returns the min and max HP of this shield
//
//	iCharges = m_iMaxCharges or the current charges on item
//	iArmorSegs = count of armor segments on ship (or 0)
//	iTotalHP = current total HP of all armor segments (or 0)

	{
	int iMax = m_iHitPoints;
	int iMin = iMax;

	if (m_iExtraHPPerCharge)
		iMax = Max(0, iMax + (m_iExtraHPPerCharge * iCharges));

	if (m_iArmorShield)
		{
		iMin = m_iArmorShield;

		if (iArmorSegs > 0)
			iMax = Min(iMax, ((m_iArmorShield * iTotalHP / iArmorSegs) + 5) / 10);
		}

	//	If we're installed, fire the custom event to get max HPs

	if (Ctx.GetSource() && Ctx.GetDevice())
		{
		CItemEnhancementStack *pEnhancements = Ctx.GetDevice()->GetEnhancements();

		iMax = FireGetMaxHP(Ctx.GetDevice(), Ctx.GetSource(), iMax);

		//	Apply bonus from device (which includes mods)

		if (pEnhancements)
			iMax += (iMax * pEnhancements->GetBonus() / 100);
		}

	//	Otherwise, we just apply mods

	else
		{
		const CItemEnhancement &Mods = Ctx.GetMods();
		if (Mods.IsNotEmpty())
			iMax = iMax * Mods.GetHPAdj() / 100;
		}

	//	Done

	if (iMin > iMax)
		iMin = iMax;

	if (retiMin)
		*retiMin = iMin;

	if (retiMax)
		*retiMax = iMax;
	}

int CShieldClass::CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	CalcPowerUsed
//
//	Returns the amount of power used by the shields

	{
	int iPower = 0;

	//	Only if enabled

	if (!pDevice->IsEnabled())
		return 0;

	//	If we're regenerating shields, then we consume more power
	//	otherwise, we only consume half power

	if ((!m_Regen.IsEmpty() || m_iExtraRegenPerCharge > 0) && pDevice->IsRegenerating())
		iPower += m_iPowerUse;
	else
		iPower += m_iIdlePowerUse;

	//	Adjust based on charges

	if (m_iExtraPowerPerCharge)
		iPower += m_iExtraPowerPerCharge * pDevice->GetCharges(pSource);

	//	Adjust based on power efficiency enhancement

	if (pDevice->GetMods().IsNotEmpty())
		iPower = iPower * pDevice->GetMods().GetPowerAdj() / 100;

	return iPower;
	}

ALERROR CShieldClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpShield)

//	CreateFromXML
//
//	Creates from an XML element

	{
	ALERROR error;
	CShieldClass *pShield;
	int i;

	pShield = new CShieldClass;
	if (pShield == NULL)
		return ERR_MEMORY;

	if (error = pShield->InitDeviceFromXML(Ctx, pDesc, pType))
		return error;

	pShield->m_iHitPoints = pDesc->GetAttributeInteger(HIT_POINTS_ATTRIB);
	pShield->m_iArmorShield = pDesc->GetAttributeInteger(ARMOR_SHIELD_ATTRIB);
	pShield->m_iPowerUse = pDesc->GetAttributeIntegerBounded(POWER_USE_ATTRIB, 0, -1, 0);
	pShield->m_iIdlePowerUse = pDesc->GetAttributeIntegerBounded(IDLE_POWER_USE_ATTRIB, 0, -1, pShield->m_iPowerUse / 2);
	pShield->m_iExtraHPPerCharge = pDesc->GetAttributeInteger(HP_ADJ_PER_CHARGE_ATTRIB);
	pShield->m_iExtraPowerPerCharge = pDesc->GetAttributeInteger(POWER_ADJ_PER_CHARGE_ATTRIB);
	pShield->m_iExtraRegenPerCharge = pDesc->GetAttributeInteger(REGEN_ADJ_PER_CHARGE_ATTRIB);
	pShield->m_iMaxCharges = pDesc->GetAttributeInteger(MAX_CHARGES_ATTRIB);

	//	Load regen value

	CString sRegen;
	if (pDesc->FindAttribute(REGEN_ATTRIB, &sRegen))
		{
		if (error = pShield->m_Regen.InitFromRegenString(Ctx, sRegen))
			return error;

		int iDepletion;
		if (pDesc->FindAttributeInteger(DEPLETION_DELAY_ATTRIB, &iDepletion))
			pShield->m_iDepletionTicks = Max(1, iDepletion);
		else
			pShield->m_iDepletionTicks = 360;
		}
	else
		{
		int iRegenTime = pDesc->GetAttributeInteger(REGEN_TIME_ATTRIB);

		if (error = pShield->m_Regen.InitFromRegenTimeAndHP(Ctx, iRegenTime, pDesc->GetAttributeInteger(REGEN_HP_ATTRIB)))
			return error;

		int iDepletion;
		if (pDesc->FindAttributeInteger(DEPLETION_DELAY_ATTRIB, &iDepletion))
			pShield->m_iDepletionTicks = Max(1, iDepletion * iRegenTime);
		else
			pShield->m_iDepletionTicks = 360;
		}

	//	Load damage adjustment

	pShield->m_iDamageAdjLevel = pDesc->GetAttributeIntegerBounded(DAMAGE_ADJ_LEVEL_ATTRIB, 1, MAX_ITEM_LEVEL, pType->GetLevel());
	if (error = pShield->m_DamageAdj.InitFromXML(Ctx, pDesc))
		return error;

	//	Load absorb adjustment; if attribute not found, assume 100% for everything

	CString sAbsorbAdj;
	if (pDesc->FindAttribute(ABSORB_ADJ_ATTRIB, &sAbsorbAdj))
		{
		CIntArray AbsorbAdj;
		if (error = ::ParseAttributeIntegerList(sAbsorbAdj, &AbsorbAdj))
			return error;

		for (i = 0; i < damageCount; i++)
			pShield->m_iAbsorbAdj[i] = (i < AbsorbAdj.GetCount() ? AbsorbAdj.GetElement(i) : 0);
		}
	else
		{
		for (i = 0; i < damageCount; i++)
			pShield->m_iAbsorbAdj[i] = 100;
		}

	//	Load the weapon suppress

	if (error = pShield->m_WeaponSuppress.InitFromXML(pDesc->GetAttribute(WEAPON_SUPPRESS_ATTRIB)))
		{
		Ctx.sError = CONSTLIT("Unable to load weapon suppress attribute");
		return error;
		}

	//	Load reflection

	if (error = pShield->m_Reflective.InitFromXML(pDesc->GetAttribute(REFLECT_ATTRIB)))
		{
		Ctx.sError = CONSTLIT("Unable to load reflective attribute");
		return error;
		}

	//	Effects

	if (error = pShield->m_pHitEffect.LoadEffect(Ctx,
			strPatternSubst(CONSTLIT("%d:h"), pType->GetUNID()),
			pDesc->GetContentElementByTag(HIT_EFFECT_TAG),
			pDesc->GetAttribute(HIT_EFFECT_ATTRIB)))
		return error;

	//	Done

	*retpShield = pShield;

	return NOERROR;
	}

void CShieldClass::Deplete (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	Disable
//
//	Lower shields

	{
	SetDepleted(pDevice, pSource);
	pSource->OnComponentChanged(comShields);
	}

bool CShieldClass::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	int i;

	if (strEquals(sField, FIELD_HP))
		*retsValue = strFromInt(m_iHitPoints);
	else if (strEquals(sField, FIELD_EFFECTIVE_HP))
		{
		int iHP;
		int iHPbyDamageType[damageCount];
		GetReferenceDamageAdj(NULL, NULL, &iHP, iHPbyDamageType);
		*retsValue = strFromInt(::CalcEffectiveHP(GetLevel(), iHP, iHPbyDamageType));
		}
	else if (strEquals(sField, FIELD_REGEN))
		*retsValue = strFromInt((int)m_Regen.GetHPPer180());
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
	else if (strEquals(sField, FIELD_POWER))
		*retsValue = strFromInt(m_iPowerUse * 100);
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
	else if (strEquals(sField, FIELD_BALANCE))
		*retsValue = strFromInt(CalcBalance());
	else if (strEquals(sField, FIELD_WEAPON_SUPPRESS))
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
		return false;

	return true;
	}

int CShieldClass::FireGetMaxHP (CInstalledDevice *pDevice, CSpaceObject *pSource, int iMaxHP) const

//	FireGetMaxHP
//
//	Fire GetMaxHP event

	{
	SEventHandlerDesc Event;
	if (FindEventHandlerShieldClass(evtGetMaxHP, &Event))
		{
		ASSERT(pSource);
		ASSERT(pDevice);

		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.SaveAndDefineItemVar(pSource->GetItemForDevice(pDevice));
		Ctx.DefineInteger(CONSTLIT("aMaxHP"), iMaxHP);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			pSource->ReportEventError(GET_MAX_HP_EVENT, pResult);
		else if (!pResult->IsNil())
			iMaxHP = Max(0, pResult->GetIntegerValue());

		Ctx.Discard(pResult);
		}

	return iMaxHP;
	}

void CShieldClass::FireOnShieldDamage (CItemCtx &ItemCtx, SDamageCtx &Ctx)

//	FireOnShieldDamage
//
//	Fire OnShieldDamage

	{
	SEventHandlerDesc Event;
	if (FindEventHandlerShieldClass(evtOnShieldDamage, &Event))
		{
		//	Setup arguments

		CCodeChainCtx CCCtx;
		CCCtx.SaveAndDefineSourceVar(ItemCtx.GetSource());
		CCCtx.SaveAndDefineItemVar(ItemCtx);

		CCCtx.DefineInteger(CONSTLIT("aArmorSeg"), Ctx.iSectHit);
		CCCtx.DefineSpaceObject(CONSTLIT("aCause"), Ctx.pCause);
		CCCtx.DefineSpaceObject(CONSTLIT("aAttacker"), Ctx.Attacker.GetObj());
		CCCtx.DefineSpaceObject(CONSTLIT("aOrderGiver"), (Ctx.Attacker.GetObj() ? Ctx.Attacker.GetObj()->GetOrderGiver(Ctx.Attacker.GetCause()) : NULL));
		CCCtx.DefineVector(CONSTLIT("aHitPos"), Ctx.vHitPos);
		CCCtx.DefineInteger(CONSTLIT("aHitDir"), Ctx.iDirection);
		CCCtx.DefineInteger(CONSTLIT("aDamageHP"), Ctx.iDamage);
		CCCtx.DefineString(CONSTLIT("aDamageType"), GetDamageShortName(Ctx.Damage.GetDamageType()));
		CCCtx.DefineItemType(CONSTLIT("aWeaponType"), Ctx.pDesc->GetWeaponType());

		CCCtx.DefineInteger(CONSTLIT("aShieldHP"), Ctx.iHPLeft);
		CCCtx.DefineInteger(CONSTLIT("aShieldDamageHP"), Ctx.iShieldDamage);
		CCCtx.DefineInteger(CONSTLIT("aArmorDamageHP"), Ctx.iDamage - Ctx.iAbsorb);
		if (Ctx.bReflect)
			{
			CCCtx.DefineString(CONSTLIT("aShieldReflect"), STR_SHIELD_REFLECT);
			CCCtx.DefineInteger(CONSTLIT("aOriginalShieldDamageHP"), Ctx.iOriginalShieldDamage);
			CCCtx.DefineInteger(CONSTLIT("aOriginalArmorDamageHP"), Ctx.iDamage - Ctx.iOriginalAbsorb);
			}
		else
			{
			CCCtx.DefineNil(CONSTLIT("aShieldReflect"));
			CCCtx.DefineInteger(CONSTLIT("aOriginalShieldDamageHP"), Ctx.iShieldDamage);
			CCCtx.DefineInteger(CONSTLIT("aOriginalArmorDamageHP"), Ctx.iDamage - Ctx.iAbsorb);
			}

		ICCItem *pResult = CCCtx.Run(Event);

		//	If we return Nil, then nothing

		if (pResult->IsNil())
			NULL;

		//	If an error, report it

		else if (pResult->IsError())
			ItemCtx.GetSource()->ReportEventError(ON_SHIELD_DAMAGE_EVENT, pResult);

		//	If we return a list, then modify variables

		else if (pResult->IsList())
			{
			//	A single value means we modified the damage to armor

			if (pResult->GetCount() == 1)
				{
				if (strEquals(pResult->GetElement(0)->GetStringValue(), STR_SHIELD_REFLECT))
					{
					Ctx.bReflect = true;
					Ctx.iAbsorb = Ctx.iDamage;
					Ctx.iShieldDamage = 0;
					}
				else
					{
					Ctx.iShieldDamage = Max(0, Min(pResult->GetElement(0)->GetIntegerValue(), Ctx.iHPLeft));
					if (Ctx.bReflect)
						{
						Ctx.bReflect = false;
						Ctx.iAbsorb = Ctx.iOriginalAbsorb;
						}
					}
				}

			//	Two values mean we modified both damage to armor and shield damage

			else if (pResult->GetCount() == 2)
				{
				Ctx.bReflect = false;
				Ctx.iShieldDamage = Max(0, Min(pResult->GetElement(0)->GetIntegerValue(), Ctx.iHPLeft));
				Ctx.iAbsorb = Max(0, Ctx.iDamage - Max(0, pResult->GetElement(1)->GetIntegerValue()));
				}

			//	Otherwise, we deal with reflection

			else
				{
				Ctx.bReflect = strEquals(pResult->GetElement(0)->GetStringValue(), STR_SHIELD_REFLECT);
				Ctx.iShieldDamage = Max(0, Min(pResult->GetElement(1)->GetIntegerValue(), Ctx.iHPLeft));
				Ctx.iAbsorb = Max(0, Ctx.iDamage - Max(0, pResult->GetElement(2)->GetIntegerValue()));
				}
			}

		CCCtx.Discard(pResult);
		}
	}

void CShieldClass::FireOnShieldDown (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	FireOnShieldDown
//
//	Fire OnShieldDown event

	{
	SEventHandlerDesc Event;
	if (FindEventHandlerShieldClass(evtOnShieldDown, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.SaveAndDefineItemVar(pSource->GetItemForDevice(pDevice));

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			pSource->ReportEventError(ON_SHIELD_DOWN_EVENT, pResult);
		Ctx.Discard(pResult);
		}
	}

int CShieldClass::GetDamageAdj (CItemEnhancement Mods, const DamageDesc &Damage) const

//	GetDamageAdj
//
//	Damage adjustment

	{
	//	The adjustment varies by shield class

	int iAdj = m_DamageAdj.GetAdj(Damage.GetDamageType());

	//	Adjust based on difference in level (negative numbers means the shield
	//	is lower than the damage level):
	//
	//	...
	//	-3	=	4.5x damage
	//	-2	=	4x damage
	//	-1	=	3.5x damage
	//	0	=	3x damage
	//	1	=	2.5x damage
	//	2	=	2x damage
	//	3	=	1.5x damage
	//	>3	=	1x damage

	if (Damage.GetShieldDamageLevel())
		iAdj = iAdj * Max(100, 300 + (50 * (Damage.GetShieldDamageLevel() - GetLevel()))) / 100;

	//	Adjust based on enhancements

	if (Mods.IsNotEmpty())
		iAdj = iAdj * Mods.GetDamageAdj(Damage) / 100;

	return iAdj;
	}

int CShieldClass::GetDamageEffectiveness (CSpaceObject *pAttacker, CInstalledDevice *pWeapon)

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

	//	See if the weapon does extra damage to shields

	if (pDamage->GetShieldDamageLevel())
		{
		int iAdj = 100 * Max(100, 300 + (50 * (pDamage->GetShieldDamageLevel() - GetLevel()))) / 100;
		iScore += (iAdj / 2);
		}

	//	Done

	return iScore;
	}

int CShieldClass::GetHPLeft (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetHPLeft
//
//	Returns the number of HP left of shields

	{
	int iHPLeft = (int)pDevice->GetData();

	if (iHPLeft < 0)
		return 0;
	else
		return iHPLeft;
	}

int CShieldClass::GetMaxHP (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetMaxHP
//
//	Max HP of shields
	
	{
	int iMax = m_iHitPoints;

	//	Adjust based on charges

	if (m_iExtraHPPerCharge)
		iMax = Max(0, iMax + m_iExtraHPPerCharge * pDevice->GetCharges(pSource));

	//	Adjust if shield is based on armor strength

	CShip *pShip;
	if (m_iArmorShield && (pShip = pSource->AsShip()))
		{
		//	Compute the average HP of all the armor

		int iArmorHP = 0;
		int iArmorCount = pShip->GetArmorSectionCount();
		for (int i = 0; i < iArmorCount; i++)
			iArmorHP += pShip->GetArmorSection(i)->GetHitPoints();

		if (iArmorCount > 0)
			iArmorHP = ((m_iArmorShield * iArmorHP / iArmorCount) + 5) / 10;

		//	Return HP left

		iMax = Min(iMax, iArmorHP);
		}

	//	Fire event

	iMax = FireGetMaxHP(pDevice, pSource, iMax);

	//	Adjust based on enhancements

	CItemEnhancementStack *pEnhancements = pDevice->GetEnhancements();
	if (pEnhancements)
		iMax = iMax + ((iMax * pEnhancements->GetBonus()) / 100);

	//	Done

	return iMax;
	}

int CShieldClass::GetPowerRating (CItemCtx &Ctx)

//	GetPowerRating
//
//	Returns the power rating of the item

	{
	int iPower = m_iPowerUse;

	const CItemEnhancement &Mods = Ctx.GetMods();
	if (Mods.IsNotEmpty())
		iPower = iPower * Mods.GetPowerAdj() / 100;

	return iPower;
	}

CString CShieldClass::GetReference (CItemCtx &Ctx, int iVariant, DWORD dwFlags)

//	GetReference
//
//	Returns a string that describes the basic attributes
//	of this shield
//
//	Example:
//
//		20 hp (average regen); 100MW

	{
	int i;

	CString sReference;
	const CItemEnhancement &Mods = Ctx.GetMods();

	//	Compute the strength string

	int iMin, iMax;
	CalcMinMaxHP(Ctx, m_iMaxCharges, 0, 0, &iMin, &iMax);

	//	Compute the regeneration

	sReference = strPatternSubst("%s — regen @ %s", 
			GetReferencePower(Ctx),
			m_Regen.GetReferenceRate(CONSTLIT("hp/sec")));

	//	Reflection

	for (i = 0; i < damageCount; i++)
		{
		if (m_Reflective.InSet((DamageTypes)i)
				|| (Mods.IsReflective() && Mods.GetDamageType() == i))
			sReference.Append(strPatternSubst(CONSTLIT(" — %s-reflecting"), GetDamageShortName((DamageTypes)i)));
		}

	return sReference;
	}

bool CShieldClass::GetReferenceDamageAdj (const CItem *pItem, CSpaceObject *pInstalled, int *retiHP, int *retArray) const

//	GetReferenceDamageAdj
//
//	Returns an array of damage adj values

	{
	CItemCtx Ctx(pItem, pInstalled);
	int i;

	const CItemEnhancement &Mods = Ctx.GetMods();

	int iMinHP, iMaxHP;
	CalcMinMaxHP(Ctx, m_iMaxCharges, 0, 0, &iMinHP, &iMaxHP);

	if (retiHP)
		*retiHP = iMaxHP;

	if (retArray)
		{
		for (i = 0; i < damageCount; i++)
			{
			DamageDesc Damage((DamageTypes)i, DiceRange(0, 0, 0));
			int iAdj = GetDamageAdj(Mods, Damage);
			if (iAdj == 0)
				retArray[i] = -1;
			else
				retArray[i] = (int)((iMaxHP * 100.0 / iAdj) + 0.5);
			}
		}

	return true;
	}

void CShieldClass::GetStatus (CInstalledDevice *pDevice, CSpaceObject *pSource, int *retiStatus, int *retiMaxStatus)

//	GetStatus
//
//	Returns the status of the shields

	{
	*retiStatus = GetHPLeft(pDevice, pSource);
	*retiMaxStatus = GetMaxHP(pDevice, pSource);
	}

int CShieldClass::GetStdCost (int iLevel)

//	GetStdCost
//
//	Returns the standard price of a shield at this level

	{
	if (iLevel >= 1 && iLevel <= MAX_ITEM_LEVEL)
		return STD_STATS[iLevel - 1].iCost;
	else
		return -1;
	}

int CShieldClass::GetStdEffectiveHP (int iLevel)

//	GetStdEffectiveHP
//
//	Returns the effective HP of a shield at this level

	{
	int i;

	if (iLevel >= 1 && iLevel <= MAX_ITEM_LEVEL)
		{
		int iHPbyDamageType[damageCount];
		for (i = 0; i < damageCount; i++)
			iHPbyDamageType[i] = CalcHPDamageAdj(STD_STATS[iLevel - 1].iHP, g_pUniverse->GetShieldDamageAdj(iLevel)->GetAdj((DamageTypes)i));

		return ::CalcEffectiveHP(iLevel, STD_STATS[iLevel - 1].iHP, iHPbyDamageType);
		}
	else
		return -1;
	}

int CShieldClass::GetStdHP (int iLevel)

//	GetStdHP
//
//	Returns the standard HP for a shield at the given level

	{
	if (iLevel >= 1 && iLevel <= MAX_ITEM_LEVEL)
		return STD_STATS[iLevel - 1].iHP;
	else
		return -1;
	}

int CShieldClass::GetStdPower (int iLevel)

//	GetStdPower
//
//	Returns the standard power consumed by a shield at the given level (in 1/10th MW)

	{
	if (iLevel >= 1 && iLevel <= MAX_ITEM_LEVEL)
		return STD_STATS[iLevel - 1].iPower;
	else
		return -1;
	}

int CShieldClass::GetStdRegen (int iLevel)

//	GetStdRegen
//
//	Returns the standard regen for a shield at the given level (in hp/180 ticks)

	{
	if (iLevel >= 1 && iLevel <= MAX_ITEM_LEVEL)
		return STD_STATS[iLevel - 1].iRegen;
	else
		return -1;
	}

bool CShieldClass::IsDepleted (CInstalledDevice *pDevice)

//	IsDepleted
//
//	Returns TRUE if shields depleted

	{
	return ((int)pDevice->GetData() < 0); 
	}

ALERROR CShieldClass::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Bind design

	{
	ALERROR error;

	//	Compute shield damage adjustments

	if (error = m_DamageAdj.Bind(Ctx, g_pUniverse->GetShieldDamageAdj(m_iDamageAdjLevel)))
		return error;

	//	Load events

	CItemType *pType = GetItemType();
	pType->InitCachedEvents(evtCount, CACHED_EVENTS, m_CachedEvents);

	//	Hit effect

	if (error = m_pHitEffect.Bind(Ctx))
		return error;

	//	If the hit effect is NULL, then use default

	if (m_pHitEffect == NULL)
		m_pHitEffect.Set(g_pUniverse->FindEffectType(g_ShieldEffectUNID));

	return NOERROR;
	}

CEffectCreator *CShieldClass::OnFindEffectCreator (const CString &sUNID)

//	OnFindEffectCreator
//
//	Find the effect. We start after the shield class UNID.
//
//	{unid}:h		Hit effect

	{
	//	We start after the shield class UNID

	char *pPos = sUNID.GetASCIIZPointer();
	if (*pPos != ':')
		return NULL;

	pPos++;

	switch (*pPos)
		{
		case 'h':
			return m_pHitEffect;

		default:
			return NULL;
		}
	}

void CShieldClass::OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	OnAddTypesUsed
//
//	Adds types used by this class

	{
	retTypesUsed->SetAt(m_pHitEffect.GetUNID(), true);
	}

void CShieldClass::OnInstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList)

//	OnInstall
//
//	Called when the device is installed

	{
	//	Set shields to max HP

	SetHPLeft(pDevice, GetMaxHP(pDevice, pSource));

	//	Identified

	if (pSource->IsPlayer())
		GetItemType()->SetKnown();
	}

void CShieldClass::OnMarkImages (void)

//	OnMarkImages
//
//	Marks images used by shield

	{
	if (m_pHitEffect)
		m_pHitEffect->MarkImages();
	}

void CShieldClass::Recharge (CInstalledDevice *pDevice, CShip *pShip, int iStatus)

//	Recharge
//
//	Recharges hitpoints (this is a manual recharge, so we ignore whether we are
//	depleted or not).

	{
	int iMaxHP = GetMaxHP(pDevice, pShip);
	int iHPLeft = GetHPLeft(pDevice, pShip);
	SetHPLeft(pDevice, min(iMaxHP, iHPLeft + iStatus));
	pShip->OnComponentChanged(comShields);
	}

bool CShieldClass::RequiresItems (void)

//	RequiresItems
//
//	Shield requires some other item to function

	{
#ifdef LATER
	//	Need to explicitly list superconducting coils as a required
	//	item for these shields to function
#else
	return m_Regen.IsEmpty();
#endif
	}

void CShieldClass::Reset (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	Reset
//
//	Lower shields

	{
	//	Note: We do not call Deplete because we don't want to invoke the
	//	OnShieldDown event

	SetHPLeft(pDevice, 0);
	pSource->OnComponentChanged(comShields);
	}

void CShieldClass::SetDepleted (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	SetDepleted
//
//	Makes shields depleted

	{
	pDevice->SetData((DWORD)(-m_iDepletionTicks));

	//	Fire event (We don't fire the event if we're disabled because we
	//	don't want something like the Invincible deflector to disable the ship
	//	if the shield is not enabled)

	if (pDevice->IsEnabled())
		FireOnShieldDown(pDevice, pSource);
	}

void CShieldClass::SetHPLeft (CInstalledDevice *pDevice, int iHP)

//	SetHPLeft
//
//	Sets HP left on shields

	{
	pDevice->SetData((DWORD)iHP);
	}

void CShieldClass::Update (CInstalledDevice *pDevice, CSpaceObject *pSource, int iTick, bool *retbSourceDestroyed, bool *retbConsumedItems)

//	Update
//
//	Updates the shield

	{
	//	Initialize to not regenerating

	pDevice->SetRegenerating(false);

	//	If we're not enabled then we drop shields

	if (!pDevice->IsEnabled())
		{
		if (GetHPLeft(pDevice, 0))
			{
			//	Note: We don't call SetDepleted because we don't want to fire the OnShieldDown
			//	event. If necessary, we should add an OnDeviceDisabled event.

			SetHPLeft(pDevice, 0);
			pSource->OnComponentChanged(comShields);
			}

		return;
		}

	//	If we're damaged, then there is a chance that we might be depleted

	if (pDevice->IsDamaged() || pDevice->IsDisrupted())
		{
		if ((iTick % 10) == 0 && mathRandom(1, 100) <= 5)
			{
			Deplete(pDevice, pSource);
			pSource->OnDeviceStatus(pDevice, failShieldFailure);
			}
		}

	//	If we regenerate then do so now.

	if (!m_Regen.IsEmpty())
		{
		//	If we're depleted, then we don't increase
		//	HP (but we still set the regenerating flag
		//	because we will consume power)

		if (UpdateDepleted(pDevice))
			pDevice->SetRegenerating(true);

		//	Otherwise, see if need to regenerate

		else
			{
			int iMaxHP = GetMaxHP(pDevice, pSource);
			int iHPLeft = GetHPLeft(pDevice, pSource);

			if (iHPLeft != iMaxHP)
				{
				int iRegenHP = m_Regen.GetRegen(iTick);

				//	Figure out how much to regen

				if (m_iExtraRegenPerCharge && ((iTick % 30) == 0))
					{
					int iCharges = pDevice->GetCharges(pSource);
					int iExtra180 = (m_iExtraRegenPerCharge * iCharges);
					int iExtra30 = iExtra180 / 6;
					int iExtraRemainder = iExtra180 % 6;

					iRegenHP += iExtra30;
					if (mathRandom(1, 6) <= iExtraRemainder)
						iRegenHP += 1;
					}

				//	Regen

				SetHPLeft(pDevice, min(iMaxHP, iHPLeft + iRegenHP));
				pSource->OnComponentChanged(comShields);

				//	Remember that we regenerated this turn (so that we can
				//	consume power)

				pDevice->SetRegenerating(true);
				}
			}
		}

	if (retbConsumedItems)
		*retbConsumedItems = false;
	}

bool CShieldClass::UpdateDepleted (CInstalledDevice *pDevice)

//	UpdateDepleted
//
//	Updates depleted state. Returns FALSE if we are not
//	depleted.

	{
	int iLeft = (int)pDevice->GetData();
	if (iLeft < 0)
		{
		iLeft++;
		pDevice->SetData((DWORD)iLeft);
		return true;
		}
	else
		return false;
	}
