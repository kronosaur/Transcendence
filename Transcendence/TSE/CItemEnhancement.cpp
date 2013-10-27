//	CItemEnhancement.cpp
//
//	Item enhancement class

#include "PreComp.h"

int CItemEnhancement::DamageAdj2Level (int iDamageAdj)

//	DamageAdj2Level
//
//	Converts from a damage adjustment to a level (0-15)

	{
	if (iDamageAdj <= 100)
		return Min(9, 10 - (iDamageAdj + 5) / 10);
	else
		return Min(9, (100 - (10000 / (iDamageAdj + 5))) / 10);
	}

EnhanceItemStatus CItemEnhancement::Combine (const CItem &Item, CItemEnhancement Enhancement)

//	Combine
//
//	Combine the current enhancement with the given one

	{
	DWORD dwNewMods = Enhancement.m_dwMods;

	//	If we're losing the enhancement, then clear it

	if (dwNewMods == etLoseEnhancement)
		{
		if (IsEnhancement())
			{
			*this = CItemEnhancement();
			return eisEnhancementRemoved;
			}
		else
			return eisNoEffect;
		}

	//	If the item is not currently enhanced, then we take the new enhancement

	else if (m_dwMods == etNone)
		{
		//	For strength/hpBonus, use the following rules:
		//
		//	etStrengthen 0 -> Min(10%, maxHPBonus)
		//	etStrengthen {level} -> Min( {level} * 10, maxHPBonus )

		if (Enhancement.GetType() == etStrengthen
					|| Enhancement.GetType() == etHPBonus)
			{
			int iMaxBonus = Item.GetType()->GetMaxHPBonus();
			int iNewBonus = Min((Enhancement.IsStacking() ? 10 : Enhancement.GetHPBonus()), iMaxBonus);
			if (iNewBonus > 0)
				{
				SetModBonus(iNewBonus);
				return eisOK;
				}
			else
				return eisNoEffect;
			}

		//	For all others, take the enhancement

		else
			*this = Enhancement;

		return eisOK;
		}

	//	If already enhanced

	else if (m_dwMods == dwNewMods)
		{
		if (IsDisadvantage())
			return eisNoEffect;
		else
			return eisAlreadyEnhanced;
		}

	//	We currently have a disadvantage

	else if (IsDisadvantage())
		{
		//	We have a disadvantage and the enhancement is another
		//	disadvantage.

		if (Enhancement.IsDisadvantage())
			{
			switch (Enhancement.GetType())
				{
				//	If we're making the disadvantage worse, then
				//	continue; otherwise, no effect.

				case etRegenerate:
				case etResist:
				case etResistEnergy:
				case etResistMatter:
				case etPowerEfficiency:
					{
					if (Enhancement.GetType() == GetType()
							&& Enhancement.GetLevel() > GetLevel())
						{
						*this = Enhancement;
						return eisWorse;
						}
					else
						return eisNoEffect;
					}

				case etHPBonus:
				case etStrengthen:
					{
					if ((GetType() == etHPBonus || GetType() == etStrengthen)
							&& Enhancement.GetHPBonus() < GetHPBonus())
						{
						*this = Enhancement;
						return eisWorse;
						}
					else
						return eisNoEffect;
					}

				case etSpeed:
				case etSpeedOld:
					{
					if ((GetType() == etSpeed || GetType() == etSpeedOld)
							&& Enhancement.GetActivateRateAdj() > GetActivateRateAdj())
						{
						*this = Enhancement;
						return eisWorse;
						}
					else
						return eisNoEffect;
					}

				case etResistByLevel:
				case etResistByDamage:
				case etResistByDamage2:
					{
					if (Enhancement.GetType() == GetType()
							&& Enhancement.GetDamageType() == GetDamageType()
							&& Enhancement.GetLevel() > GetLevel())
						{
						*this = Enhancement;
						return eisWorse;
						}
					else
						return eisNoEffect;
					}

				//	Otherwise, a disadvantage does not affect a disadvantage

				default:
					return eisNoEffect;
				}
			}

		//	We have a disadvantage and we use an enhancement.

		else
			{
			switch (Enhancement.GetType())
				{
				//	Regeneration enhancement always repairs a disadvantage

				case etRegenerate:
					{
					*this = CItemEnhancement();
					return eisRepaired;
					}

				//	If the enhancement is the opposite of the disadvantage
				//	then the disadvantage is repaired.

				case etResist:
				case etResistEnergy:
				case etResistMatter:
				case etPowerEfficiency:
				case etResistByLevel:
				case etResistByDamage:
				case etResistByDamage2:
				case etReflect:
					{
					if (GetType() == Enhancement.GetType())
						{
						*this = CItemEnhancement();
						return eisRepaired;
						}
					else
						return eisNoEffect;
					}

				case etHPBonus:
				case etStrengthen:
					{
					if (GetType() == etHPBonus || GetType() == etStrengthen)
						{
						*this = CItemEnhancement();
						return eisRepaired;
						}
					else
						return eisNoEffect;
					}

				case etSpeed:
				case etSpeedOld:
					{
					if (GetType() == etSpeed || GetType() == etSpeedOld)
						{
						*this = CItemEnhancement();
						return eisRepaired;
						}
					else
						return eisNoEffect;
					}

				//	Otherwise, no effect

				default:
					return eisNoEffect;
				}
			}
		}

	//	We currently have an enhancement

	else
		{
		if (!Enhancement.IsDisadvantage())
			{
			switch (Enhancement.GetType())
				{
				case etHPBonus:
				case etStrengthen:
					{
					if (GetType() != etHPBonus 
							&& GetType() != etStrengthen)
						return eisNoEffect;

					//	If improving...

					int iOldBonus = GetHPBonus();
					int iMaxBonus = Item.GetType()->GetMaxHPBonus();
					int iNewBonus = Min((Enhancement.IsStacking() ? iOldBonus + 10 : Enhancement.GetHPBonus()), iMaxBonus);
					if (iNewBonus > iOldBonus)
						{
						SetModBonus(iNewBonus);
						return eisBetter;
						}
					else
						return eisNoEffect;
					}

				//	If this is the same type of enhancement and it is better,
				//	then take it (otherwise, no effect)

				case etRegenerate:
				case etResist:
				case etResistEnergy:
				case etResistMatter:
				case etPowerEfficiency:
					{
					if (Enhancement.GetType() == GetType()
							&& Enhancement.GetLevel() > GetLevel())
						{
						*this = Enhancement;
						return eisBetter;
						}
					else
						return eisNoEffect;
					}

				case etSpeed:
				case etSpeedOld:
					{
					if (Enhancement.GetType() == GetType()
							&& Enhancement.GetActivateRateAdj() < GetActivateRateAdj())
						{
						*this = Enhancement;
						return eisBetter;
						}
					else
						return eisNoEffect;
					}

				case etResistByLevel:
				case etResistByDamage:
				case etResistByDamage2:
					{
					if (Enhancement.GetType() != GetType())
						return eisNoEffect;
					else if (Enhancement.GetDamageType() != GetDamageType())
						{
						*this = Enhancement;
						return eisEnhancementReplaced;
						}
					else if (Enhancement.GetLevel() > GetLevel())
						{
						*this = Enhancement;
						return eisBetter;
						}
					else
						return eisNoEffect;
					}

				default:
					return eisNoEffect;
				}
			}
		else
			{
			//	A disadvantage always destroys any enhancement

			*this = CItemEnhancement();
			return eisEnhancementRemoved;
			}
		}
	}

DWORD CItemEnhancement::EncodeABC (DWORD dwTypeCode, int A, int B, int C)

//	EncodeABC
//
//	Encodes a mod with data A, B, and C.

	{
	DWORD dwDataA = Max(0, Min(A, (int)etDataAMax));
	DWORD dwDataB = Max(0, Min(B, (int)etDataBMax));
	DWORD dwDataC = Max(0, Min(B, (int)etDataCMax));
	return dwTypeCode
			| dwDataA
			| (dwDataB << 16)
			| (dwDataC << 24);
	}

DWORD CItemEnhancement::EncodeAX (DWORD dwTypeCode, int A, int X)

//	EncodeAX
//
//	Encodes a mod with data A and X.

	{
	DWORD dwDataA = Max(0, Min(A, (int)etDataAMax));
	DWORD dwDataX = Max(0, Min(X, (int)etDataXMax));
	return dwTypeCode
			| dwDataA
			| (dwDataX << 16);
	}

DWORD CItemEnhancement::Encode12 (DWORD dwTypeCode, int Data1, int Data2)

//	Encode12
//
//	Encodes a mod with data 1 and 2.

	{
	DWORD dwData1 = Max(0, Min(Data1, (int)0xF));
	DWORD dwData2 = Max(0, Min(Data2, (int)0xF));
	return dwTypeCode
			| dwData1
			| (dwData2 << 4);
	}

int CItemEnhancement::GetAbsorbAdj (const DamageDesc &Damage) const

//	GetAbsorbAdj
//
//	Returns damage absorbed

	{
	switch (GetType())
		{
		case etReflect:
			{
			if (IsDisadvantage() && Damage.GetDamageType() == GetDamageType())
				return Level2DamageAdj(GetLevel());
			else
				return 100;
			}

		default:
			return 100;
		}
	}

int CItemEnhancement::GetDamageAdj (const DamageDesc &Damage) const

//	GetDamageAdj
//
//	Returns the damage adjustment confered by this mod

	{
	switch (GetType())
		{
		case etResist:
			return Level2DamageAdj(GetLevel(), IsDisadvantage());

		case etResistEnergy:
			return (Damage.IsEnergyDamage() ? Level2DamageAdj(GetLevel(), IsDisadvantage()) : 100);

		case etResistMatter:
			return (Damage.IsMatterDamage() ? Level2DamageAdj(GetLevel(), IsDisadvantage()) : 100);

		case etResistByLevel:
			{
			if (Damage.GetDamageType() == GetDamageType()
					|| Damage.GetDamageType() == GetDamageType() + 1)
				return Level2DamageAdj(GetLevel(), IsDisadvantage());
			else
				return 100;
			}

		case etResistByDamage:
			return (Damage.GetDamageType() == GetDamageType() ? Level2DamageAdj(GetLevel(), IsDisadvantage()) : 100);

		case etResistByDamage2:
			{
			if (Damage.GetDamageType() == GetDamageType())
				//	0 = 100			100
				//	1 = 90			111
				//	2 = 80			125
				//	3 = 70			143
				return Level2DamageAdj(GetLevel(), IsDisadvantage());
			else if (Damage.GetDamageType() == GetDamageType() + 2)
				//	0 = 100			100
				//	1 = 95			105
				//	2 = 90			112
				//	3 = 85			121
				return 100 + ((Level2DamageAdj(GetLevel(), IsDisadvantage()) - 100) / 2);
			else
				return 100;
			}

		default:
			return 100;
		}
	}

int CItemEnhancement::GetHPBonus (void) const

//	GetHPBonus
//
//	Returns the damage bonus for the weapon, in percentage points.
//	(e.g., 10 = +10%).

	{
	switch (GetType())
		{
		case etHPBonus:
			return (IsDisadvantage() ? -GetDataX() : GetDataX());

		case etStrengthen:
			return Level2Bonus(GetLevel(), IsDisadvantage());

		default:
			return 0;
		}
	}

CString CItemEnhancement::GetEnhancedDesc (const CItem &Item, CSpaceObject *pInstalled, CInstalledDevice *pDevice) const

//	GetEnhancedDesc
//
//	Get short description of enhancement.
//
//	NOTE: This currently include bonuses confered by other ship systems (stored in
//	the device structure. In the future we need a better mechanism)

	{
	CItemEnhancementStack *pAllEnhancements = (pDevice ? pDevice->GetEnhancements() : NULL);

	switch (GetType())
		{
		case etHPBonus:
		case etStrengthen:
			{
			switch (Item.GetType()->GetCategory())
				{
				case itemcatWeapon:
				case itemcatLauncher:
				case itemcatShields:
					{
					int iDamageBonus;

					//	See if this device is installed; if so, then the bonus is
					//	calculated and cached in the device; we do this so that we can
					//	include bonuses from all sources.

					if (pAllEnhancements)
						iDamageBonus = pAllEnhancements->GetBonus();
					else
						iDamageBonus = GetHPBonus();

					//	Bonus

					if (iDamageBonus < 0)
						return strPatternSubst(CONSTLIT("%d%%"), iDamageBonus);
					else
						return strPatternSubst(CONSTLIT("+%d%%"), iDamageBonus);
					}

				default:
					if (IsDisadvantage())
						return strPatternSubst(CONSTLIT("-%d%%"), 100 - GetHPAdj());
					else
						return strPatternSubst(CONSTLIT("+%d%%"), GetHPAdj() - 100);
				}
			}

		case etRegenerate:
			return (IsDisadvantage() ? CONSTLIT("-Decay") : CONSTLIT("+Regen"));

		case etReflect:
			return strPatternSubst((IsDisadvantage() ? CONSTLIT("-%s Trans") : CONSTLIT("+%s Reflect")),
					strCapitalizeWords(::GetDamageShortName(GetDamageType())));

		case etRepairOnHit:
			return strPatternSubst(CONSTLIT("+%s Repair"), strCapitalizeWords(::GetDamageShortName(GetDamageType())));

		case etResist:
			return (IsDisadvantage() ? CONSTLIT("-Vulnerable") : CONSTLIT("+Resistant"));

		case etResistEnergy:
			return (IsDisadvantage() ? CONSTLIT("-Energy Vulnerable") : CONSTLIT("+Energy Resistant"));

		case etResistMatter:
			return (IsDisadvantage() ? CONSTLIT("-Matter Vulnerable") : CONSTLIT("+Matter Resistant"));

		case etResistByLevel:
			{
			if (IsDisadvantage())
				return strPatternSubst(CONSTLIT("-%s -%s"), strCapitalizeWords(::GetDamageShortName(GetDamageType())), strCapitalizeWords(::GetDamageShortName((DamageTypes)(GetDamageType() + 1))));
			else
				return strPatternSubst(CONSTLIT("+%s +%s"), strCapitalizeWords(::GetDamageShortName(GetDamageType())), strCapitalizeWords(::GetDamageShortName((DamageTypes)(GetDamageType() + 1))));
			}

		case etResistByDamage:
			return strPatternSubst((IsDisadvantage() ? CONSTLIT("-%s") : CONSTLIT("+%s")),
					strCapitalizeWords(::GetDamageShortName(GetDamageType())));

		case etResistByDamage2:
			{
			if (IsDisadvantage())
				return strPatternSubst(CONSTLIT("-%s -%s"), strCapitalizeWords(::GetDamageShortName(GetDamageType())), strCapitalizeWords(::GetDamageShortName((DamageTypes)(GetDamageType() + 2))));
			else
				{
				switch (GetDamageType())
					{
					case damageLaser:
						return CONSTLIT("+Ablative");

					case damageKinetic:
						return CONSTLIT("+Reactive");

					default:
						return strPatternSubst(CONSTLIT("+%s +%s"), 
								strCapitalizeWords(::GetDamageShortName(GetDamageType())), 
								strCapitalizeWords(::GetDamageShortName((DamageTypes)(GetDamageType() + 2))));
					}
				}
			}

		case etSpecialDamage:
			{
			switch (GetLevel2())
				{
				case specialRadiation:
					return CONSTLIT("+Rad Immune");

				case specialBlinding:
					return CONSTLIT("+Blind Immune");

				case specialEMP:
					return CONSTLIT("+EMP Immune");

				case specialDeviceDamage:
					return CONSTLIT("+Dev Protect");

				case specialDisintegration:
					return CONSTLIT("+Dis Immune");

				default:
					return CONSTLIT("+Unk Immune");
				}
			}

		case etImmunityIonEffects:
			return (IsDisadvantage() ? CONSTLIT("-No Shields") : CONSTLIT("+Ionization Immune"));

		case etPhotoRegenerate:
			return CONSTLIT("+SolRegen");

		case etPhotoRecharge:
			return CONSTLIT("+SolPower");

		case etPowerEfficiency:
			return (IsDisadvantage() ? CONSTLIT("-Drain") : CONSTLIT("+Efficient"));

		case etSpeed:
		case etSpeedOld:
			return (IsDisadvantage() ? CONSTLIT("-Slow") : CONSTLIT("+Fast"));

		default:
			return CONSTLIT("+Unknown");
		}
	}

int CItemEnhancement::GetActivateRateAdj (int *retiMinDelay, int *retiMaxDelay) const

//	GetActivateRateAdj
//
//	Returns the adj to activate/fire delay

	{
	int iAdj = 100;
	int iMinDelay = 0;
	int iMaxDelay = 0;

	switch (GetType())
		{
		case etSpeed:
			if (IsDisadvantage())
				iAdj = 100 + GetDataA() * 5;
			else
				iAdj = GetDataA();

			iMinDelay = GetDataB();
			iMaxDelay = GetDataC();
			break;

		case etSpeedOld:
			iAdj = Level2DamageAdj(GetLevel(), IsDisadvantage());
			break;

		default:
			iAdj = 100;
		}

	if (retiMinDelay)
		*retiMinDelay = iMinDelay;

	if (retiMaxDelay)
		*retiMaxDelay = iMaxDelay;

	return iAdj;
	}

int CItemEnhancement::GetEnhancedRate (int iRate) const

//	GetEnhancedRate
//
//	Returns the enhanced rate

	{
	switch (GetType())
		{
		case etSpeed:
			{
			int iMin;
			int iMax;
			int iAdj = GetActivateRateAdj(&iMin, &iMax);
			int iNewRate = Max(iMin, (iRate * iAdj + 50) / 100);
			if (iMax != 0 && iNewRate > iMax)
				return iMax;
			else
				return iNewRate;
			}

		case etSpeedOld:
			return Max(1, (iRate * Level2DamageAdj(GetLevel(), IsDisadvantage()) + 50) / 100);

		default:
			return iRate;
		}
	}

int CItemEnhancement::GetHPAdj (void) const

//	GetHPAdj
//
//	Get increase/decrease in HP

	{
	switch (GetType())
		{
		case etHPBonus:
			{
			int iData = GetDataX();

			if (IsDisadvantage())
				{
				if (iData >= 0 && iData <= 90)
					return 100 - iData;
				else
					return 10;
				}
			else
				return 100 + iData;
			}

		case etStrengthen:
			{
			int iLevel = GetLevel();

			if (IsDisadvantage())
				{
				if (iLevel >= 0 && iLevel <= 9)
					return 100 - (10 * iLevel);
				else
					return 10;
				}
			else
				return 100 + (10 * iLevel);
			}

		default:
			return 100;
		}
	}

int CItemEnhancement::GetPowerAdj (void) const

//	GetPowerAdj
//
//	Get the increase/decrease in power usage

	{
	switch (GetType())
		{
		case etPowerEfficiency:
			{
			int iLevel = GetLevel();

			if (IsDisadvantage())
				return 100 + (10 * iLevel);
			else
				{
				if (iLevel >= 0 && iLevel <= 9)
					return 100 - (10 * iLevel);
				else
					return 10;
				}
			}

		default:
			return 100;
		}
	}

SpecialDamageTypes CItemEnhancement::GetSpecialDamage (int *retiLevel) const

//	GetSpecialDamage
//
//	Returns special damage enhancement type (or specialNone)

	{
	SpecialDamageTypes iSpecial;
	int iLevel;

	switch (GetType())
		{
		case etSpecialDamage:
			iSpecial = (SpecialDamageTypes)GetLevel2();
			iLevel = 0;
			break;

		case etConferSpecialDamage:
			iSpecial = (SpecialDamageTypes)GetDataA();
			iLevel = GetDataB();
			break;

		default:
			iSpecial = specialNone;
			iLevel = 0;
		}

	if (retiLevel)
		*retiLevel = iLevel;

	return iSpecial;
	}

int CItemEnhancement::GetValueAdj (const CItem &Item) const

//	GetValueAdj
//
//	Returns adjustment to item value based on modification

	{
	if (IsDisadvantage())
		{
		switch (GetType())
			{
			case etHPBonus:
				return Max(-80, -GetDataX());

			case etStrengthen:
			case etResistByLevel:
			case etResistByDamage:
			case etResistByDamage2:
			case etSpeedOld:
				return Max(-80, -10 * GetLevel());

			case etSpeed:
				return Max(-80, -3 * GetDataA());

			case etResist:
			case etResistMatter:
			case etResistEnergy:
				return Max(-80, -20 * GetLevel());

			case etPhotoRegenerate:
			case etPhotoRecharge:
			case etRepairOnHit:
			case etPowerEfficiency:
				return -50;

			case etRegenerate:
			case etReflect:
			case etSpecialDamage:
			case etImmunityIonEffects:
				return -80;

			default:
				return 0;
			}
		}
	else
		{
		switch (GetType())
			{
			case etHPBonus:
				return 2 * GetDataX();

			case etStrengthen:
				return 20 * GetLevel();

			case etPowerEfficiency:
				return 15;

			case etResistByLevel:
			case etResistByDamage:
			case etResistByDamage2:
			case etSpeedOld:
				return 10 * GetLevel();

			case etSpeed:
				return 2 * (100 - GetDataA());

			case etResist:
			case etResistMatter:
			case etResistEnergy:
				return 50 * GetLevel();

			case etPhotoRegenerate:
			case etPhotoRecharge:
			case etRepairOnHit:
				return 30;

			case etRegenerate:
			case etReflect:
			case etSpecialDamage:
			case etImmunityIonEffects:
				return 100;

			default:
				return 0;
			}
		}
	}

ALERROR CItemEnhancement::InitFromDesc (const CString &sDesc, CString *retsError)

//	InitFromDesc
//
//	Initializes from a string of the following forms:
//
//	{number}					Interpret as a mod code
//	+armor:{n}					Add	armor special damage, where n is an item level
//	+hpBonus:{n}				Add hp bonus.
//	+immunity:{s}				Immunity to special damage s.
//	+reflect:{s}				Reflects damage type s.
//	+regen						Regenerate
//	+resist:{s}:{n}				DamageAdj for type s set to n
//	+resistDamageClass:{s}:{n}	DamageAdj for type s (and its next-tier mate) set to n
//	+resistDamageTier:{s}:{n}	DamageAdj for type s (and its tier mate) set to n
//	+resistEnergy:{n}			DamageAdj for energy damage set to n
//	+resistMatter:{n}			DamageAdj for matter damage set to n
//	+shield:{n}					Add shield disrupt special damage, where n is an item level
//	+speed:{n}					Faster. n is new delay value as a percent of normal

	{
	//	If the string is a number then we interpret it as a mod code.

	bool bFailed;
	DWORD dwCode = strToInt(sDesc, 0, &bFailed);
	if (!bFailed)
		{
		m_dwMods = dwCode;
		return NOERROR;
		}

	//	Parse the string

	char *pPos = sDesc.GetASCIIZPointer();

	//	Expect either "+" or "-" (for disadvantage)

	bool bDisadvantage;
	if (*pPos == '+')
		bDisadvantage = false;
	else if (*pPos == '-')
		bDisadvantage = true;
	else
		{
		if (retsError)
			*retsError = CONSTLIT("Invalid enhancement description: expected '+' or '-'.");
		return ERR_FAIL;
		}

	pPos++;

	//	Parse the enhancement name

	char *pStart = pPos;
	while (*pPos != ':' && *pPos != '\0')
		pPos++;

	CString sID(pStart, (int)(pPos - pStart));

	//	See if we have a value

	int iValue = 0;
	CString sValue;
	if (*pPos == ':')
		{
		pPos++;

		if (*pPos == '-' || *pPos == '+' || (*pPos >= '0' && *pPos <= '9'))
			iValue = strParseInt(pPos, 0, &pPos);
		else
			{
			char *pStart = pPos;
			while (*pPos != '\0' && *pPos != ':')
				pPos++;

			sValue = CString(pStart, (int)(pPos - pStart));
			}
		}

	//	See if we have a second value

	int iValue2 = 0;
	if (*pPos == ':')
		{
		pPos++;
		iValue2 = strParseInt(pPos, 0, &pPos);
		}

	//	See if this is an hpBonus

	if (strEquals(sID, CONSTLIT("hpBonus")))
		{
		if (bDisadvantage && iValue > 0)
			iValue = -iValue;

		if (iValue < -100)
			{
			if (retsError)
				*retsError = CONSTLIT("hpBonus penalty cannot exceed 100%.");
			return ERR_FAIL;
			}
		else if (iValue < 0)
			SetModCode(EncodeAX(etHPBonus | etDisadvantage, 0, -iValue));
		else if (iValue == 0)
			SetModCode(Encode12(etStrengthen));
		else if (iValue <= 1000)
			SetModCode(EncodeAX(etHPBonus, 0, iValue));
		else
			{
			if (retsError)
				*retsError = CONSTLIT("hpBonus cannot exceed 1000%.");
			return ERR_FAIL;
			}
		}

	//	Immunity

	else if (strEquals(sID, CONSTLIT("immunity")))
		{
		if (strEquals(sValue, CONSTLIT("ionEffects")))
			m_dwMods = Encode12(etImmunityIonEffects);
		else
			{
			SpecialDamageTypes iSpecial = DamageDesc::ConvertToSpecialDamageTypes(sValue);
			switch (iSpecial)
				{
				case specialRadiation:
				case specialBlinding:
				case specialEMP:
				case specialDeviceDamage:
				case specialDisintegration:
				case specialMomentum:
				case specialShieldDisrupt:
				case specialDeviceDisrupt:
				case specialShatter:
					{
					if (bDisadvantage)
						{
						if (retsError)
							*retsError = CONSTLIT("Disadvantage not supported.");
						return ERR_FAIL;
						}

					SetModImmunity(iSpecial);
					break;
					}

				default:
					{
					if (retsError)
						*retsError = strPatternSubst(CONSTLIT("Invalid immunity: %s"), sID);
					return ERR_FAIL;
					}
				}
			}
		}

	//	Reflect bonus

	else if (strEquals(sID, CONSTLIT("reflect")))
		{
		DamageTypes iDamageType = LoadDamageTypeFromXML(sValue);
		if (iDamageType == damageError)
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Invalid damage type: %s"), sValue);
			return ERR_FAIL;
			}

		SetModReflect(iDamageType);
		}

	//	Regen

	else if (strEquals(sID, CONSTLIT("regen")))
		{
		m_dwMods = Encode12(etRegenerate | (bDisadvantage ? etDisadvantage : 0));
		}

	//	Resist damage

	else if (strEquals(sID, CONSTLIT("resist")))
		{
		DamageTypes iDamageType = LoadDamageTypeFromXML(sValue);
		if (iDamageType == damageError)
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Invalid damage type: %s"), sValue);
			return ERR_FAIL;
			}

		SetModResistDamage(iDamageType, iValue2);
		}

	//	Resist damage

	else if (strEquals(sID, CONSTLIT("resistDamageClass")))
		{
		DamageTypes iDamageType = LoadDamageTypeFromXML(sValue);
		if (iDamageType == damageError)
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Invalid damage type: %s"), sValue);
			return ERR_FAIL;
			}

		SetModResistDamageClass(iDamageType, iValue2);
		}

	//	Resist damage tier

	else if (strEquals(sID, CONSTLIT("resistDamageTier")))
		{
		DamageTypes iDamageType = LoadDamageTypeFromXML(sValue);
		if (iDamageType == damageError)
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Invalid damage type: %s"), sValue);
			return ERR_FAIL;
			}

		SetModResistDamageTier(iDamageType, iValue2);
		}

	//	Resist energy/matter

	else if (strEquals(sID, CONSTLIT("resistEnergy")))
		SetModResistEnergy(iValue);
	else if (strEquals(sID, CONSTLIT("resistMatter")))
		SetModResistMatter(iValue);

	//	Speed bonus

	else if (strEquals(sID, CONSTLIT("speed")))
		{
		if (iValue <= 0)
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Invalid speed value: %s."), iValue);
			return ERR_FAIL;
			}
		else
			//	LATER: Support min and max delay limits
			SetModSpeed(iValue);
		}

	//	Otherwise, see if this is a special damage 

	else
		{
		SpecialDamageTypes iSpecial = DamageDesc::ConvertToSpecialDamageTypes(sID);
		switch (iSpecial)
			{
			case specialArmor:
			case specialShieldDisrupt:
				{
				if (bDisadvantage)
					{
					if (retsError)
						*retsError = CONSTLIT("Disadvantage not supported.");
					return ERR_FAIL;
					}

				if (iValue < 1 || iValue > MAX_ITEM_LEVEL)
					{
					if (retsError)
						*retsError = strPatternSubst(CONSTLIT("Invalid %s damage level: %d"), sID, iValue);
					return ERR_FAIL;
					}

				SetModSpecialDamage(iSpecial, iValue);
				break;
				}

			default:
				{
				if (retsError)
					*retsError = strPatternSubst(CONSTLIT("Invalid enhancement name: %s"), sID);
				return ERR_FAIL;
				}
			}
		}

	//	Done

	return NOERROR;
	}

ALERROR CItemEnhancement::InitFromDesc (ICCItem *pItem, CString *retsError)

//	InitFromDesc
//
//	Initializes from a CodeChain item

	{
	if (pItem->IsInteger())
		{
		m_dwMods = (DWORD)pItem->GetIntegerValue();
		return NOERROR;
		}
	else
		return InitFromDesc(pItem->GetStringValue(), retsError);
	}

ALERROR CItemEnhancement::InitFromDesc (SDesignLoadCtx &Ctx, const CString &sDesc)

//	InitFromDesc
//
//	Initializes from a descriptor
	
	{
	return InitFromDesc(sDesc, &Ctx.sError); 
	}

bool CItemEnhancement::IsEqual (const CItemEnhancement &Comp) const

//	IsEqual
//
//	Returns TRUE if enhancements are equal

	{
	return (Comp.m_dwMods == m_dwMods
			&& Comp.m_pEnhancer == m_pEnhancer
			&& Comp.m_iExpireTime == m_iExpireTime); 
	}

bool CItemEnhancement::IsReflective (const DamageDesc &Damage, int *retiReflectChance) const

//	IsReflective
//
//	Returns TRUE if we reflect the given damage

	{
	switch (GetType())
		{
		case etReflect:
			{
			if (!IsDisadvantage() && Damage.GetDamageType() == GetDamageType())
				{
				if (retiReflectChance)
					*retiReflectChance = 50 + (GetLevel() * 5);

				return true;
				}
			else
				return false;
			}

		default:
			return false;
		}
	}

int CItemEnhancement::Level2Bonus (int iLevel, bool bDisadvantage)

//	Level2Bonus
//
//	Convert from a level (0-15) to a bonus adj (+%)

	{
	if (bDisadvantage)
		return -10 * Min(10, iLevel);
	else
		return 10 * iLevel;
	}

int CItemEnhancement::Level2DamageAdj (int iLevel, bool bDisadvantage)

//	Level2DamageAdj
//
//	Convert from a level (0-15) to a damage adj (%)

	{
	if (bDisadvantage)
		return (10000 / Level2DamageAdj(iLevel, false));
	else
		{
		if (iLevel >= 0 && iLevel <= 9)
			return (100 - (10 * iLevel));
		else
			return 10;
		}
	}

void CItemEnhancement::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads an enhancement

	{
	ReadFromStream(Ctx.dwVersion, Ctx.pStream);
	}

void CItemEnhancement::ReadFromStream (DWORD dwVersion, IReadStream *pStream)

//	ReadFromStream
//
//	Sometimes we don't have a full Ctx, such as when loading from an ICCItem

	{
	DWORD dwLoad;

	pStream->Read((char *)&m_dwMods, sizeof(DWORD));

	if (dwVersion >= 46)
		{
		pStream->Read((char *)&m_dwID, sizeof(DWORD));

		pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_pEnhancer = (dwLoad ? g_pUniverse->FindItemType(dwLoad) : NULL);

		pStream->Read((char *)&m_iExpireTime, sizeof(DWORD));
		}

	//	Previous to v46, we did not set m_dwID

	if (dwVersion < 46 && m_dwMods != 0)
		m_dwID = g_pUniverse->CreateGlobalID();
	}

void CItemEnhancement::SetModBonus (int iBonus)

//	SetModBonus
//
//	Sets bonus enhancement

	{
	if (iBonus >= 0)
		m_dwMods = EncodeAX(etHPBonus, 0, iBonus);
	else
		m_dwMods = EncodeAX(etHPBonus | etDisadvantage, 0, -iBonus);
	}

void CItemEnhancement::SetModSpecialDamage (SpecialDamageTypes iSpecial, int iLevel)

//	SetModSpecialDamage
//
//	Sets special damage

	{
	if (iLevel == 0)
		m_dwMods = Encode12(etSpecialDamage, 0, iSpecial);
	else
		m_dwMods = EncodeABC(etConferSpecialDamage, iSpecial, iLevel);
	}

void CItemEnhancement::SetModSpeed (int iAdj, int iMinDelay, int iMaxDelay)

//	SetModSpeed
//
//	Sets a speed adjustment mod.

	{
	if (iAdj <= 100)
		m_dwMods = EncodeABC(etSpeed, iAdj, iMinDelay, iMaxDelay);
	else
		m_dwMods = EncodeABC(etSpeed | etDisadvantage, (iAdj - 100) / 5, iMinDelay, iMaxDelay);
	}

void CItemEnhancement::WriteToStream (IWriteStream *pStream) const

//	WriteToStream
//
//	Writes an enhancement

	{
	DWORD dwSave;

	pStream->Write((char *)&m_dwMods, sizeof(DWORD));
	pStream->Write((char *)&m_dwID, sizeof(DWORD));

	dwSave = (m_pEnhancer ? m_pEnhancer->GetUNID() : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_iExpireTime, sizeof(DWORD));
	}
