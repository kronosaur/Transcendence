//	Damage.cpp
//
//	Damage routines and classes

#include "PreComp.h"

#define OVERLAY_TAG								CONSTLIT("Overlay")

#define DAMAGE_ATTRIB							CONSTLIT("damage")
#define ATTACK_FORM_ATTRIB						CONSTLIT("attackMode")
#define TYPE_ATTRIB								CONSTLIT("type")
#define CHANCE_ATTRIB							CONSTLIT("chance")
#define LIFETIME_ATTRIB							CONSTLIT("lifetime")

#define SPECIAL_DAMAGE_ARMOR					CONSTLIT("armor")
#define SPECIAL_DAMAGE_BLINDING					CONSTLIT("blinding")
#define SPECIAL_DAMAGE_DEVICE					CONSTLIT("device")
#define SPECIAL_DAMAGE_DEVICE_DISRUPT			CONSTLIT("deviceDisrupt")
#define SPECIAL_DAMAGE_DISINTEGRATION			CONSTLIT("disintegration")
#define SPECIAL_DAMAGE_EMP						CONSTLIT("EMP")
#define SPECIAL_DAMAGE_FUEL						CONSTLIT("fuel")
#define SPECIAL_DAMAGE_MINING					CONSTLIT("mining")
#define SPECIAL_DAMAGE_MOMENTUM					CONSTLIT("momentum")
#define SPECIAL_DAMAGE_RADIATION				CONSTLIT("radiation")
#define SPECIAL_DAMAGE_REFLECT					CONSTLIT("reflect")
#define SPECIAL_DAMAGE_SENSOR					CONSTLIT("sensor")
#define SPECIAL_DAMAGE_SHATTER					CONSTLIT("shatter")
#define SPECIAL_DAMAGE_SHIELD					CONSTLIT("shield")
#define SPECIAL_DAMAGE_WMD						CONSTLIT("WMD")
#define SPECIAL_DAMAGE_WORMHOLE					CONSTLIT("wormhole")

#define OVERLAY_TYPE_HULL						CONSTLIT("hull")

const int DEFAULT_BLINDING_TIME =				(5 * g_TicksPerSecond);
const int DEFAULT_EMP_TIME =					(7 * g_TicksPerSecond);
const int DEFAULT_DEVICE_DISRUPT_TIME =			(30 * g_TicksPerSecond);

const int MAX_INTENSITY =						7;
const int MAX_BINARY =							1;

char *g_pszDamageTypes[damageCount] =
	{
	"laser",
	"kinetic",
	"particle",
	"blast",
	"ion",
	"thermo",
	"positron",
	"plasma",
	"antimatter",
	"nano",
	"graviton",
	"singularity",
	"darkAcid",
	"darkSteel",
	"darkLightning",
	"darkFire",
	};

char *g_pszDamageName[damageCount] =
	{
	"laser",
	"kinetic",
	"particle beam",
	"blast",
	"ion",
	"thermonuclear",
	"positron beam",
	"plasma",
	"antimatter",
	"nanotech",
	"graviton",
	"singularity",
	"dark acid",
	"dark steel",
	"dark lightning",
	"dark fire",
	};

char *g_pszShortDamageName[damageCount] =
	{
	"laser",
	"kinetic",
	"particle",
	"blast",
	"ion",
	"thermo",
	"positron",
	"plasma",
	"antimatter",
	"nano",
	"graviton",
	"singularity",
	"dark acid",
	"dark steel",
	"dark lightning",
	"dark fire",
	};

//	Damage Types

CString GetDamageName (DamageTypes iType)

//	GetDamageName
//
//	Returns the name of this type of damage

	{
	if (iType == damageGeneric)
		return CONSTLIT("generic");
	else
		return CString(g_pszDamageName[iType]);
	}

CString GetDamageShortName (DamageTypes iType)

//	GetDamageShortName
//
//	Returns the short name of this type of damage

	{
	if (iType == damageGeneric)
		return CONSTLIT("generic");
	else
		return CString(g_pszShortDamageName[iType]);
	}

CString GetDamageType (DamageTypes iType)

//	GetDamageType
//
//	Returns the damage type string

	{
	if (iType == damageGeneric)
		return CONSTLIT("generic");
	else
		return CString(g_pszDamageTypes[iType]);
	}

DamageTypes LoadDamageTypeFromXML (const CString &sAttrib)

//	LoadDamageTypeFromXML
//
//	Converts from string to DamageType

	{
	int iType;

	for (iType = 0; iType < damageCount; iType++)
		if (strEquals(sAttrib, CString(g_pszDamageTypes[iType])))
			return (DamageTypes)iType;

	//	Generic

	if (strEquals(sAttrib, CONSTLIT("generic")))
		return damageGeneric;

	//	Backwards compatibility

	if (strEquals(sAttrib, CONSTLIT("dark acid")))
		return damageDarkAcid;
	else if (strEquals(sAttrib, CONSTLIT("dark steel")))
		return damageDarkSteel;
	else if (strEquals(sAttrib, CONSTLIT("dark lightning")))
		return damageDarkLightning;
	else if (strEquals(sAttrib, CONSTLIT("dark fire")))
		return damageDarkFire;

	return damageError;
	}

//	DamageDesc -----------------------------------------------------------------

void DamageDesc::AddEnhancements (CItemEnhancementStack *pEnhancements)

//	AddEnhancements
//
//	Applies any enhancements in the given stack to the damage descriptor
//	(including bonus and special damage).

	{
	if (pEnhancements == NULL)
		return;

	//	Add bonus

	AddBonus(pEnhancements->GetBonus());

	//	Add special damage

	pEnhancements->ApplySpecialDamage(this);
	}

SpecialDamageTypes DamageDesc::ConvertToSpecialDamageTypes (const CString &sValue)

//	ConvertToSpecialDamageTypes
//
//	Converts

	{
	if (strEquals(sValue, SPECIAL_DAMAGE_ARMOR))
		return specialArmor;
	else if (strEquals(sValue, SPECIAL_DAMAGE_BLINDING))
		return specialBlinding;
	else if (strEquals(sValue, SPECIAL_DAMAGE_DEVICE))
		return specialDeviceDamage;
	else if (strEquals(sValue, SPECIAL_DAMAGE_DEVICE_DISRUPT))
		return specialDeviceDisrupt;
	else if (strEquals(sValue, SPECIAL_DAMAGE_DISINTEGRATION))
		return specialDisintegration;
	else if (strEquals(sValue, SPECIAL_DAMAGE_EMP))
		return specialEMP;
	else if (strEquals(sValue, SPECIAL_DAMAGE_FUEL))
		return specialFuel;
	else if (strEquals(sValue, SPECIAL_DAMAGE_MINING))
		return specialMining;
	else if (strEquals(sValue, SPECIAL_DAMAGE_MOMENTUM))
		return specialMomentum;
	else if (strEquals(sValue, SPECIAL_DAMAGE_RADIATION))
		return specialRadiation;
	else if (strEquals(sValue, SPECIAL_DAMAGE_SHATTER))
		return specialShatter;
	else if (strEquals(sValue, SPECIAL_DAMAGE_SHIELD))
		return specialShieldDisrupt;
	else if (strEquals(sValue, SPECIAL_DAMAGE_WMD))
		return specialWMD;
	else if (strEquals(sValue, SPECIAL_DAMAGE_WORMHOLE))
		return specialWormhole;
	else
		return specialNone;
	}

int DamageDesc::GetSpecialDamage (SpecialDamageTypes iSpecial) const

//	GetSpecialDamage
//
//	Returns special damage level

	{
	switch (iSpecial)
		{
		case specialArmor:
			return m_ArmorDamage;

		case specialBlinding:
			return m_BlindingDamage;

		case specialDeviceDamage:
			return m_DeviceDamage;

		case specialDeviceDisrupt:
			return m_DeviceDisruptDamage;

		case specialDisintegration:
			return m_DisintegrationDamage;

		case specialEMP:
			return m_EMPDamage;

		case specialFuel:
			return m_FuelDamage;

		case specialMining:
			return m_MiningAdj;

		case specialMomentum:
			return m_MomentumDamage;

		case specialRadiation:
			return m_RadiationDamage;

		case specialShatter:
			return m_ShatterDamage;

		case specialShieldDisrupt:
			return m_ShieldDamage;

		case specialWMD:
			return m_MassDestructionAdj;

		default:
			return 0;
		}
	}

void DamageDesc::SetSpecialDamage (SpecialDamageTypes iSpecial, int iLevel)

//	SetSpecialDamage
//
//	Sets special damage

	{
	switch (iSpecial)
		{
		case specialArmor:
			m_ArmorDamage = Max(1, Min(iLevel, MAX_ITEM_LEVEL));
			break;

		case specialBlinding:
			m_BlindingDamage = Max(0, Min(iLevel, 7));
			break;

		case specialDeviceDamage:
			m_DeviceDamage = Max(0, Min(iLevel, 7));
			break;

		case specialDeviceDisrupt:
			m_DeviceDisruptDamage = Max(0, Min(iLevel, 7));
			break;

		case specialDisintegration:
			m_DisintegrationDamage = Max(0, Min(iLevel, 7));
			break;

		case specialEMP:
			m_EMPDamage = Max(0, Min(iLevel, 7));
			break;

		case specialFuel:
			m_FuelDamage = Max(0, Min(iLevel, 7));
			break;

		case specialMining:
			m_MiningAdj = Max(0, Min(iLevel, 7));
			break;

		case specialMomentum:
			m_MomentumDamage = Max(0, Min(iLevel, 7));
			break;

		case specialRadiation:
			m_RadiationDamage = Max(0, Min(iLevel, 7));
			break;

		case specialShatter:
			m_ShatterDamage = Max(0, Min(iLevel, 7));
			break;

		case specialShieldDisrupt:
			m_ShieldDamage = Max(1, Min(iLevel, MAX_ITEM_LEVEL));
			break;

		case specialWMD:
			m_MassDestructionAdj = Max(0, Min(iLevel, 7));
			break;
		}
	}

void DamageDesc::SetDamage (int iDamage)

//	SetDamage
//
//	Sets a constant damage

	{
	m_Damage = DiceRange(0, 0, iDamage);
	}

CString DamageDesc::GetDesc (DWORD dwFlags)

//	GetDesc
//
//	Returns a description of the damage:
//
//	laser 1-4 (+50%)

	{
	CString sDamageType = GetDamageShortName(m_iType);

	if (dwFlags & flagAverageDamage)
		{
		Metric rDamage = GetAverageDamage();
		rDamage += rDamage * m_iBonus / 100.0;

		int iDamage10 = (int)((rDamage * 10.0) + 0.5);
		int iDamage = iDamage10 / 10;
		int iDamageTenth = iDamage10 % 10;


		if (iDamageTenth == 0)
			return strPatternSubst(CONSTLIT("%s %d hp"), sDamageType, iDamage);
		else
			return strPatternSubst(CONSTLIT("%s %d.%d hp"), sDamageType, iDamage, iDamageTenth);
		}
	else
		{
		int iMin = m_Damage.GetCount() + m_Damage.GetBonus();
		int iMax = m_Damage.GetCount() * m_Damage.GetFaces() + m_Damage.GetBonus();

		char szBuffer[1024];
		if (m_iBonus == 0)
			{
			int iLen;
			if (iMin == iMax)
				iLen = wsprintf(szBuffer, "%s %d", sDamageType, iMax);
			else
				iLen = wsprintf(szBuffer, "%s %d-%d", sDamageType, iMin, iMax);

			return CString(szBuffer, iLen);
			}
		else if (m_iBonus > 0)
			{
			int iLen;
			if (iMin == iMax)
				iLen = wsprintf(szBuffer, "%s %d (+%d%%)", sDamageType, iMax, m_iBonus);
			else
				iLen = wsprintf(szBuffer, "%s %d-%d (+%d%%)", sDamageType, iMin, iMax, m_iBonus);

			return CString(szBuffer, iLen);
			}
		else
			{
			int iLen;
			if (iMin == iMax)
				iLen = wsprintf(szBuffer, "%s %d (-%d%%)", sDamageType, iMax, -m_iBonus);
			else
				iLen = wsprintf(szBuffer, "%s %d-%d (-%d%%)", sDamageType, iMin, iMax, -m_iBonus);

			return CString(szBuffer, iLen);
			}
		}
	}

int DamageDesc::GetMinDamage (void)
	{
	return m_Damage.GetCount() + m_Damage.GetBonus();
	}

int DamageDesc::GetMaxDamage (void)
	{
	return m_Damage.GetCount() * m_Damage.GetFaces() + m_Damage.GetBonus();
	}

ALERROR DamageDesc::LoadFromXML (SDesignLoadCtx &Ctx, const CString &sAttrib)

//	LoadFromXML
//
//	Loads damage of the form:
//
//	damagetype:1d1+1; ion6; momentum5; radiation4; deviceDisrupt6

	{
	ALERROR error;

	//	Initialize

	m_iType = damageGeneric;
	m_iBonus = 0;
	m_iCause = killedByDamage;
	m_EMPDamage = 0;
	m_MomentumDamage = 0;
	m_RadiationDamage = 0;
	m_DisintegrationDamage = 0;
	m_DeviceDisruptDamage = 0;
	m_BlindingDamage = 0;
	m_SensorDamage = 0;
	m_ShieldDamage = 0;
	m_ArmorDamage = 0;
	m_WormholeDamage = 0;
	m_FuelDamage = 0;
	m_fNoSRSFlash = false;
	m_fAutomatedWeapon = false;
	m_DeviceDamage = 0;
	m_MassDestructionAdj = 0;
	m_MiningAdj = 0;
	m_ShatterDamage = 0;

	//	Loop over all segments separated by semi-colons

	char *pPos = sAttrib.GetASCIIZPointer();
	while (true)
		{
		CString sKeyword;
		CString sValue;

		if (error = ParseTerm(Ctx, pPos, &sKeyword, &sValue, &pPos))
			{
			if (error == ERR_NOTFOUND)
				break;
			else
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Unable to parse damage: %s"), sAttrib);
				return error;
				}
			}

		if (error = LoadTermFromXML(Ctx, sKeyword, sValue))
			return error;
		}

	return NOERROR;
	}

ALERROR DamageDesc::LoadTermFromXML (SDesignLoadCtx &Ctx, const CString &sType, const CString &sArg)

//	LoadTermFromXML
//
//	Loads a damage desc term of one of the following forms:
//
//	laser:1d4
//	radiation1
//	shield:7

	{
	ALERROR error;

	//	Now figure out what to do based on the word. If this is a damage type
	//	then we load it as such.

	DamageTypes iType = LoadDamageTypeFromXML(sType);
	if (iType != damageError)
		{
		m_iType = iType;
		if (error = m_Damage.LoadFromXML(sArg))
			return error;
		}

	//	Otherwise, we expect the arg to be an integer

	else
		{
		int iCount;
		if (sArg.IsBlank())
			//	If there is no arg, then we assume 1. This handles cases where
			//	the modifier is binary, such as "disintegration".
			iCount = 1;
		else
			{
			iCount = strToInt(sArg, -1);
			if (iCount < 0)
				return ERR_FAIL;
			}

		if (strEquals(sType, SPECIAL_DAMAGE_EMP))
			m_EMPDamage = (DWORD)Min(iCount, MAX_INTENSITY);
		else if (strEquals(sType, SPECIAL_DAMAGE_RADIATION))
			m_RadiationDamage = (DWORD)Min(iCount, MAX_BINARY);
		else if (strEquals(sType, SPECIAL_DAMAGE_MOMENTUM))
			m_MomentumDamage = (DWORD)Min(iCount, MAX_INTENSITY);
		else if (strEquals(sType, SPECIAL_DAMAGE_DISINTEGRATION))
			m_DisintegrationDamage = (DWORD)Min(iCount, MAX_BINARY);
		else if (strEquals(sType, SPECIAL_DAMAGE_DEVICE_DISRUPT))
			m_DeviceDisruptDamage = (DWORD)Min(iCount, MAX_INTENSITY);
		else if (strEquals(sType, SPECIAL_DAMAGE_BLINDING))
			m_BlindingDamage = (DWORD)Min(iCount, MAX_INTENSITY);
		else if (strEquals(sType, SPECIAL_DAMAGE_SENSOR))
			m_SensorDamage = (DWORD)Min(iCount, MAX_INTENSITY);
		else if (strEquals(sType, SPECIAL_DAMAGE_SHIELD))
			{
			m_ShieldDamage = (BYTE)Min(iCount, MAX_ITEM_LEVEL);

			//	For versions prior to 1.1 we used a different calculation for
			//	shield level (because we only allocated 3 bits for this value

			if (Ctx.GetAPIVersion() < 3)
				m_ShieldDamage = 1 + ((m_ShieldDamage * m_ShieldDamage) / 2);
			}
		else if (strEquals(sType, SPECIAL_DAMAGE_ARMOR))
			m_ArmorDamage = (BYTE)Max(1, Min(iCount, MAX_ITEM_LEVEL));
		else if (strEquals(sType, SPECIAL_DAMAGE_WORMHOLE))
			m_WormholeDamage = (DWORD)Min(iCount, MAX_INTENSITY);
		else if (strEquals(sType, SPECIAL_DAMAGE_FUEL))
			m_FuelDamage = (DWORD)Min(iCount, MAX_INTENSITY);
		else if (strEquals(sType, SPECIAL_DAMAGE_WMD))
			m_MassDestructionAdj = (DWORD)Min(iCount, MAX_INTENSITY);
		else if (strEquals(sType, SPECIAL_DAMAGE_DEVICE))
			m_DeviceDamage = (DWORD)Min(iCount, MAX_INTENSITY);
		else if (strEquals(sType, SPECIAL_DAMAGE_MINING))
			m_MiningAdj = (DWORD)Min(iCount, MAX_INTENSITY);
		else if (strEquals(sType, SPECIAL_DAMAGE_SHATTER))
			m_ShatterDamage = (DWORD)Min(iCount, MAX_INTENSITY);
		else
			return ERR_FAIL;
		}

	return NOERROR;
	}

ALERROR DamageDesc::ParseTerm (SDesignLoadCtx &Ctx, char *pPos, CString *retsKeyword, CString *retsValue, char **retpPos)

//	ParseTerm
//
//	Parses a damage term of the form:
//
//	{keyword} : {value} [;,]
//	{keyword} {numberValue} [;,]
//	{keyword} [;,]

	{
	enum States
		{
		stateStart,
		stateKeyword,
		stateLookingForSeparator,
		stateFoundSeparator,
		stateValue,
		stateDone,
		};

	CString sKeyword;
	CString sValue;
	int iState = stateStart;
	char *pStart;

	while (iState != stateDone)
		{
		switch (iState)
			{
			case stateStart:
				if (*pPos == '\0')
					return ERR_NOTFOUND;
				else if (!strIsWhitespace(pPos) && *pPos != ',' && *pPos != ';')
					{
					pStart = pPos;
					iState = stateKeyword;
					}
				break;

			case stateKeyword:
				if (*pPos == '\0' || *pPos == ';' || *pPos == ',')
					{
					sKeyword = CString(pStart, (int)(pPos - pStart));
					iState = stateDone;
					}
				else if (strIsWhitespace(pPos))
					{
					sKeyword = CString(pStart, (int)(pPos - pStart));

					//	For backwards compatibility we accept a space in 
					//	"dark acid", etc.

					if (!strEquals(sKeyword, CONSTLIT("dark")))
						iState = stateLookingForSeparator;
					}
				else if (*pPos == ':')
					{
					sKeyword = CString(pStart, (int)(pPos - pStart));
					iState = stateFoundSeparator;
					}
				else if (*pPos >= '0' && *pPos <= '9')
					{
					sKeyword = CString(pStart, (int)(pPos - pStart));
					pStart = pPos;
					iState = stateValue;
					}
				break;

			case stateLookingForSeparator:
				if (*pPos == '\0' || *pPos == ';' || *pPos == ',')
					iState = stateDone;
				else if (*pPos == ':')
					iState = stateFoundSeparator;
				else if (!strIsWhitespace(pPos))
					{
					pStart = pPos;
					iState = stateValue;
					}

				break;

			case stateFoundSeparator:
				if (*pPos == '\0')
					return ERR_FAIL;
				else if (!strIsWhitespace(pPos))
					{
					pStart = pPos;
					iState = stateValue;
					}
				break;

			case stateValue:
				if (strIsWhitespace(pPos) || *pPos == ';' || *pPos == ',' || *pPos == '\0')
					{
					sValue = CString(pStart, (int)(pPos - pStart));
					iState = stateDone;
					}
				break;
			}

		if (*pPos != '\0')
			pPos++;
		}

	//	Done

	if (retsKeyword)
		*retsKeyword = sKeyword;

	if (retsValue)
		*retsValue = sValue;

	if (retpPos)
		*retpPos = pPos;

	return NOERROR;
	}

void DamageDesc::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads from a stream
//
//	DWORD		m_iType
//	DWORD		DiceRange
//	DWORD		m_iBonus
//	DWORD		m_iCause
//	DWORD		Extra damage
//	DWORD		Extra damage 2

	{
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&m_iType, sizeof(DWORD));
	m_Damage.ReadFromStream(Ctx);
	Ctx.pStream->Read((char *)&m_iBonus, sizeof(DWORD));

	if (Ctx.dwVersion >= 18)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_iCause = (DestructionTypes)dwLoad;
		}
	else
		m_iCause = killedByDamage;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_EMPDamage = dwLoad & 0x07;
	m_MomentumDamage = (dwLoad >> 3) & 0x07;
	m_RadiationDamage = (dwLoad >> 6) & 0x07;
	m_DeviceDisruptDamage = (dwLoad >> 9) & 0x07;
	m_BlindingDamage = (dwLoad >> 12) & 0x07;
	m_SensorDamage = (dwLoad >> 15) & 0x07;
	if (Ctx.dwVersion < 73)
		m_ShieldDamage = (BYTE)((dwLoad >> 18) & 0x07);
	m_WormholeDamage = (dwLoad >> 21) & 0x07;
	m_FuelDamage = (dwLoad >> 24) & 0x07;
	m_DisintegrationDamage = (dwLoad >> 27) & 0x07;
	m_fNoSRSFlash = (dwLoad >> 30) & 0x01;
	m_fAutomatedWeapon = (dwLoad >> 31) & 0x01;

	//	In previous versions we had a bit that determined whether this damage
	//	came from weapons malfunction

	if (Ctx.dwVersion < 18 && ((dwLoad >> 30) & 0x01))
		m_iCause = killedByWeaponMalfunction;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_DeviceDamage = dwLoad & 0x07;
	m_MassDestructionAdj = (dwLoad >> 3) & 0x07;
	m_MiningAdj = (dwLoad >> 6) & 0x07;
	m_ShatterDamage = (dwLoad >> 9) & 0x07;
	m_dwSpare2 = 0;

	if (Ctx.dwVersion >= 73)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_ShieldDamage = (BYTE)(dwLoad & 0xff);
		m_ArmorDamage = (BYTE)((dwLoad & 0xff00) >> 8);
		}
	}

int DamageDesc::RollDamage (void) const

//	RollDamage
//
//	Computes hp damage

	{
	int iDamage = m_Damage.Roll();
	if (m_iBonus > 0)
		{
		int iNum = iDamage * m_iBonus;
		int iWhole = iNum / 100;
		int iPartial = iNum % 100;
		if (mathRandom(1, 100) <= iPartial)
			iWhole++;

		return Max(0, iDamage + iWhole);
		}
	else
		return Max(0, iDamage);
	}

void DamageDesc::WriteToStream (IWriteStream *pStream) const

//	WriteToStream
//
//	Writes out to a stream
//
//	DWORD		m_iType
//	DWORD		DiceRange
//	DWORD		m_iBonus
//	DWORD		m_iCause
//	DWORD		Extra damage
//	DWORD		Extra damage 2
//	DWORD		Extra damage 3

	{
	DWORD dwSave;

	pStream->Write((char *)&m_iType, sizeof(DWORD));
	m_Damage.WriteToStream(pStream);
	pStream->Write((char *)&m_iBonus, sizeof(DWORD));

	dwSave = m_iCause;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = m_EMPDamage;
	dwSave |= m_MomentumDamage << 3;
	dwSave |= m_RadiationDamage << 6;
	dwSave |= m_DeviceDisruptDamage << 9;
	dwSave |= m_BlindingDamage << 12;
	dwSave |= m_SensorDamage << 15;
//	UNUSED: dwSave |= m_ShieldDamage << 18;
	dwSave |= m_WormholeDamage << 21;
	dwSave |= m_FuelDamage << 24;
	dwSave |= m_DisintegrationDamage << 27;
	dwSave |= m_fNoSRSFlash << 30;
	dwSave |= m_fAutomatedWeapon << 31;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = m_DeviceDamage;
	dwSave |= m_MassDestructionAdj << 3;
	dwSave |= m_MiningAdj << 6;
	dwSave |= m_ShatterDamage << 9;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = (m_ArmorDamage << 8) | m_ShieldDamage;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

//	DamageTypeSet --------------------------------------------------------------

ALERROR DamageTypeSet::InitFromXML (const CString &sAttrib)

//	InitFromXML
//
//	Initialize set from semi-comma separated list

	{
	int i;

	//	Blank means empty

	if (sAttrib.IsBlank())
		return NOERROR;

	//	"*" means all damage

	if (strEquals(sAttrib, CONSTLIT("*")))
		{
		for (i = 0; i < damageCount; i++)
			Add(i);
		return NOERROR;
		}

	//	Load each damage type

	TArray<CString> ArraySet;
	ParseStringList(sAttrib, 0, &ArraySet);

	for (i = 0; i < ArraySet.GetCount(); i++)
		{
		int iType = LoadDamageTypeFromXML(ArraySet[i]);
		if (iType == damageError || iType == damageGeneric)
			return ERR_FAIL;

		Add(iType);
		}

	return NOERROR;
	}

//	Damage Utilities -----------------------------------------------------------

static void AddEffectItem (CCodeChain &CC, CCLinkedList *pList, const CString &sEffect, int iTime = -1)
	{
	CCLinkedList *pNewItem = (CCLinkedList *)CC.CreateLinkedList();
	if (pNewItem->IsError())
		{
		pNewItem->Discard(&CC);
		return;
		}

	ICCItem *pField = CC.CreateString(sEffect);
	pNewItem->Append(&CC, pField);
	pField->Discard(&CC);

	if (iTime != -1)
		{
		pField = CC.CreateInteger(iTime);
		pNewItem->Append(&CC, pField);
		pField->Discard(&CC);
		}

	pList->Append(&CC, pNewItem);
	pNewItem->Discard(&CC);
	}

ICCItem *CreateItemFromDamageEffects (CCodeChain &CC, SDamageCtx &Ctx)

//	CreateItemFromDamageEffects
//
//	Packages up the damage effects from Ctx into a single item.
//
//	The item is a list of damage effects, each damage effects 
//	is one of the following:
//
//	([damage type] [damage])
//	('blinding [time in ticks])
//	('device)
//	('deviceDisrupt)
//	('disintegrate)
//	('EMP [time in ticks])
//	('radiation)
//	('reflect)
//	('shatter)

	{
	//	Short-circuit if we have no effects

	if (!Ctx.bBlind && !Ctx.bDeviceDamage && !Ctx.bDeviceDisrupt && !Ctx.bDisintegrate
			&& !Ctx.bParalyze && !Ctx.bRadioactive && !Ctx.bReflect
			&& !Ctx.bShatter)
		return CC.CreateNil();

	//	Create a list to hold the result

	CCLinkedList *pList = (CCLinkedList *)CC.CreateLinkedList();

	//	Add the damage

	if (Ctx.iDamage)
		AddEffectItem(CC, pList, CString(g_pszDamageTypes[Ctx.Damage.GetDamageType()]), Ctx.iDamage);

	//	Add effects

	if (Ctx.bBlind)
		AddEffectItem(CC, pList, SPECIAL_DAMAGE_BLINDING, Ctx.iBlindTime);

	if (Ctx.bDeviceDamage)
		AddEffectItem(CC, pList, SPECIAL_DAMAGE_DEVICE);

	if (Ctx.bDeviceDisrupt)
		AddEffectItem(CC, pList, SPECIAL_DAMAGE_DEVICE_DISRUPT, Ctx.iDisruptTime);

	if (Ctx.bDisintegrate)
		AddEffectItem(CC, pList, SPECIAL_DAMAGE_DISINTEGRATION);

	if (Ctx.bParalyze)
		AddEffectItem(CC, pList, SPECIAL_DAMAGE_EMP, Ctx.iParalyzeTime);

	if (Ctx.bRadioactive)
		AddEffectItem(CC, pList, SPECIAL_DAMAGE_RADIATION);

	if (Ctx.bReflect)
		AddEffectItem(CC, pList, SPECIAL_DAMAGE_REFLECT);

	if (Ctx.bShatter)
		AddEffectItem(CC, pList, SPECIAL_DAMAGE_SHATTER);

	return pList;
	}

void LoadDamageEffectsFromItem (ICCItem *pItem, SDamageCtx &Ctx)

//	LoadDamageEffectsFromItem
//
//	Modifies Ctx to load damage effects from the given item.
//	The item is a list of damage effects, each damage effects 
//	is one of the following:
//
//	([damage type] [damage])
//	('blinding [time in ticks])
//	('device)
//	('disintegrate)
//	('EMP [time in ticks])
//	('radiation)
//	('reflect)
//	('shatter)

	{
	int i;

	//	Initialize Ctx to defaults

	Ctx.iDamage = 0;
	Ctx.bBlind = false;
	Ctx.bDeviceDamage = false;
	Ctx.bDeviceDisrupt = false;
	Ctx.bDisintegrate = false;
	Ctx.bParalyze = false;
	Ctx.bRadioactive = false;
	Ctx.bReflect = false;
	Ctx.bShatter = false;

	//	Keep track of whether we found normal damage or not
	//	so that we don't keep looking (to save time)

	bool bNormalDamageFound = false;

	//	Now load all the effects from the item

	for (i = 0; i < pItem->GetCount(); i++)
		{
		ICCItem *pEffect = pItem->GetElement(i);
		if (pEffect->GetCount() == 0)
			continue;

		CString sDamage = pEffect->GetElement(0)->GetStringValue();
		DamageTypes iDamageType;
		if (!bNormalDamageFound && (iDamageType = LoadDamageTypeFromXML(sDamage)) != damageError)
			{
			Ctx.iDamage = Max(0, pEffect->GetElement(1)->GetIntegerValue());
			if (iDamageType != Ctx.Damage.GetDamageType())
				{
				Ctx.Damage.SetDamageType(iDamageType);
				Ctx.Damage.SetDamage(Ctx.iDamage);
				}

			bNormalDamageFound = true;
			}
		else if (strEquals(sDamage, SPECIAL_DAMAGE_BLINDING))
			{
			Ctx.bBlind = true;
			if (pEffect->GetCount() >= 2)
				Ctx.iBlindTime = Max(0, pEffect->GetElement(1)->GetIntegerValue());
			else
				Ctx.iBlindTime = DEFAULT_BLINDING_TIME;
			}
		else if (strEquals(sDamage, SPECIAL_DAMAGE_DEVICE))
			Ctx.bDeviceDamage = true;
		else if (strEquals(sDamage, SPECIAL_DAMAGE_DEVICE_DISRUPT))
			{
			Ctx.bDeviceDisrupt = true;
			if (pEffect->GetCount() >= 2)
				Ctx.iDisruptTime = Max(0, pEffect->GetElement(1)->GetIntegerValue());
			else
				Ctx.iDisruptTime = DEFAULT_DEVICE_DISRUPT_TIME;
			}
		else if (strEquals(sDamage, SPECIAL_DAMAGE_DISINTEGRATION))
			Ctx.bDisintegrate = true;
		else if (strEquals(sDamage, SPECIAL_DAMAGE_EMP))
			{
			Ctx.bParalyze = true;
			if (pEffect->GetCount() >= 2)
				Ctx.iParalyzeTime = Max(0, pEffect->GetElement(1)->GetIntegerValue());
			else
				Ctx.iParalyzeTime = DEFAULT_EMP_TIME;
			}
		else if (strEquals(sDamage, SPECIAL_DAMAGE_RADIATION))
			Ctx.bRadioactive = true;
		else if (strEquals(sDamage, SPECIAL_DAMAGE_REFLECT))
			Ctx.bReflect = true;
		else if (strEquals(sDamage, SPECIAL_DAMAGE_SHATTER))
			Ctx.bShatter = true;
		}
	}
