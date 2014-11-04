//	CAISettings.cpp
//
//	CAISettings class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

#define AGGRESSOR_ATTRIB						CONSTLIT("aggressor")
#define ASCEND_ON_GATE_ATTRIB					CONSTLIT("ascendOnGate")
#define COMBAT_SEPARATION_ATTRIB				CONSTLIT("combatSeparation")
#define COMBAT_STYLE_ATTRIB						CONSTLIT("combatStyle")
#define FIRE_ACCURACY_ATTRIB					CONSTLIT("fireAccuracy")
#define FIRE_RANGE_ADJ_ATTRIB					CONSTLIT("fireRangeAdj")
#define FIRE_RATE_ADJ_ATTRIB					CONSTLIT("fireRateAdj")
#define FLOCK_FORMATION_ATTRIB					CONSTLIT("flockFormation")
#define FLYBY_COMBAT_ATTRIB						CONSTLIT("flybyCombat")
#define NO_SHIELD_RETREAT_ATTRIB				CONSTLIT("ignoreShieldsDown")
#define NO_ATTACK_ON_THREAT_ATTRIB				CONSTLIT("noAttackOnThreat")
#define NO_DOGFIGHTS_ATTRIB						CONSTLIT("noDogfights")
#define NO_FRIENDLY_FIRE_ATTRIB					CONSTLIT("noFriendlyFire")
#define NO_FRIENDLY_FIRE_CHECK_ATTRIB			CONSTLIT("noFriendlyFireCheck")
#define NO_NAV_PATHS_ATTRIB						CONSTLIT("noNavPaths")
#define NO_ORDER_GIVER_ATTRIB					CONSTLIT("noOrderGiver")
#define NO_TARGETS_OF_OPPORTUNITY_ATTRIB		CONSTLIT("noTargetsOfOpportunity")
#define NON_COMBATANT_ATTRIB					CONSTLIT("nonCombatant")
#define PERCEPTION_ATTRIB						CONSTLIT("perception")
#define STAND_OFF_COMBAT_ATTRIB					CONSTLIT("standOffCombat")

#define COMBAT_STYLE_ADVANCED					CONSTLIT("advanced")
#define COMBAT_STYLE_CHASE						CONSTLIT("chase")
#define COMBAT_STYLE_FLYBY						CONSTLIT("flyby")
#define COMBAT_STYLE_NO_RETREAT					CONSTLIT("noRetreat")
#define COMBAT_STYLE_STANDARD					CONSTLIT("standard")
#define COMBAT_STYLE_STAND_OFF					CONSTLIT("standOff")

#define STR_TRUE								CONSTLIT("True")

CAISettings::CAISettings (void)

//	CAISettings constructor

	{
	}

AICombatStyles CAISettings::ConvertToAICombatStyle (const CString &sValue)

//	ConvertToAICombatStyle
//
//	Converts a string to combat style

	{
	if (strEquals(sValue, COMBAT_STYLE_ADVANCED))
		return aicombatAdvanced;
	else if (strEquals(sValue, COMBAT_STYLE_CHASE))
		return aicombatChase;
	else if (strEquals(sValue, COMBAT_STYLE_FLYBY))
		return aicombatFlyby;
	else if (strEquals(sValue, COMBAT_STYLE_NO_RETREAT))
		return aicombatNoRetreat;
	else if (strEquals(sValue, COMBAT_STYLE_STANDARD))
		return aicombatStandOff;
	else if (strEquals(sValue, COMBAT_STYLE_STAND_OFF))
		return aicombatStandOff;
	else
		return aicombatStandard;
	}

CString CAISettings::ConvertToID (AICombatStyles iStyle)

//	ConvertToID
//
//	Converts a combat style to a string ID

	{
	switch (iStyle)
		{
		case aicombatStandard:
			return COMBAT_STYLE_STANDARD;

		case aicombatStandOff:
			return COMBAT_STYLE_STAND_OFF;

		case aicombatFlyby:
			return COMBAT_STYLE_FLYBY;

		case aicombatNoRetreat:
			return COMBAT_STYLE_NO_RETREAT;

		case aicombatChase:
			return COMBAT_STYLE_CHASE;

		case aicombatAdvanced:
			return COMBAT_STYLE_ADVANCED;

		default:
			return NULL_STR;
		}
	}

CString CAISettings::GetValue (const CString &sSetting)

//	GetValue
//
//	Get the current value of the given setting

	{
	if (strEquals(sSetting, AGGRESSOR_ATTRIB))
		return (m_fAggressor ? STR_TRUE : NULL_STR);
	else if (strEquals(sSetting, ASCEND_ON_GATE_ATTRIB))
		return (m_fAscendOnGate ? STR_TRUE : NULL_STR);
	else if (strEquals(sSetting, COMBAT_SEPARATION_ATTRIB))
		return (m_rMinCombatSeparation > 0.0 ? strFromInt((int)(m_rMinCombatSeparation / g_KlicksPerPixel)) : NULL_STR);
	else if (strEquals(sSetting, COMBAT_STYLE_ATTRIB))
		return ConvertToID(m_iCombatStyle);
	else if (strEquals(sSetting, FIRE_ACCURACY_ATTRIB))
		return strFromInt(m_iFireAccuracy);
	else if (strEquals(sSetting, FIRE_RANGE_ADJ_ATTRIB))
		return strFromInt(m_iFireRangeAdj);
	else if (strEquals(sSetting, FIRE_RATE_ADJ_ATTRIB))
		return strFromInt(m_iFireRateAdj);
	else if (strEquals(sSetting, FLOCK_FORMATION_ATTRIB))
		return (m_fFlockFormation ? STR_TRUE : NULL_STR);
	else if (strEquals(sSetting, NO_ATTACK_ON_THREAT_ATTRIB))
		return (m_fNoAttackOnThreat ? STR_TRUE : NULL_STR);
	else if (strEquals(sSetting, NO_TARGETS_OF_OPPORTUNITY_ATTRIB))
		return (m_fNoTargetsOfOpportunity ? STR_TRUE : NULL_STR);
	else if (strEquals(sSetting, NO_DOGFIGHTS_ATTRIB))
		return (m_fNoDogfights ? STR_TRUE : NULL_STR);
	else if (strEquals(sSetting, NO_SHIELD_RETREAT_ATTRIB))
		return (m_fNoShieldRetreat ? STR_TRUE : NULL_STR);
	else if (strEquals(sSetting, NO_FRIENDLY_FIRE_ATTRIB))
		return (m_fNoFriendlyFire ? STR_TRUE : NULL_STR);
	else if (strEquals(sSetting, NO_FRIENDLY_FIRE_CHECK_ATTRIB))
		return (m_fNoFriendlyFireCheck ? STR_TRUE : NULL_STR);
	else if (strEquals(sSetting, NO_NAV_PATHS_ATTRIB))
		return (m_fNoNavPaths ? STR_TRUE : NULL_STR);
	else if (strEquals(sSetting, NO_ORDER_GIVER_ATTRIB))
		return (m_fNoOrderGiver ? STR_TRUE : NULL_STR);
	else if (strEquals(sSetting, NON_COMBATANT_ATTRIB))
		return (m_fNonCombatant ? STR_TRUE : NULL_STR);
	else if (strEquals(sSetting, PERCEPTION_ATTRIB))
		return strFromInt(m_iPerception);
	else
		return NULL_STR;
	}

ALERROR CAISettings::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitFromXML
//
//	Initialize from XML element

	{
	//	Combat style

	CString sCombatStyle;
	if (pDesc->FindAttribute(COMBAT_STYLE_ATTRIB, &sCombatStyle))
		m_iCombatStyle = ConvertToAICombatStyle(sCombatStyle);
	else
		{
		//	Compatibility with version < 0.97

		if (pDesc->GetAttributeBool(FLYBY_COMBAT_ATTRIB))
			m_iCombatStyle = aicombatFlyby;
		else if (pDesc->GetAttributeBool(STAND_OFF_COMBAT_ATTRIB))
			m_iCombatStyle = aicombatStandOff;
		else
			m_iCombatStyle = aicombatStandard;
		}

	//	Parameters

	m_iFireRateAdj = pDesc->GetAttributeIntegerBounded(FIRE_RATE_ADJ_ATTRIB, 1, -1, 10);
	m_iFireRangeAdj = pDesc->GetAttributeIntegerBounded(FIRE_RANGE_ADJ_ATTRIB, 1, -1, 100);
	m_iFireAccuracy = pDesc->GetAttributeIntegerBounded(FIRE_ACCURACY_ATTRIB, 0, 100, 100);
	m_iPerception = pDesc->GetAttributeIntegerBounded(PERCEPTION_ATTRIB, CSpaceObject::perceptMin, CSpaceObject::perceptMax, CSpaceObject::perceptNormal);

	//	Combat separation

	int iSep;
	if (pDesc->FindAttributeInteger(COMBAT_SEPARATION_ATTRIB, &iSep))
		m_rMinCombatSeparation = iSep * g_KlicksPerPixel;
	else
		//	A negative value means that we compute it based on the image size
		m_rMinCombatSeparation = -1.0;

	//	Flags

	m_fAscendOnGate = pDesc->GetAttributeBool(ASCEND_ON_GATE_ATTRIB);
	m_fNoShieldRetreat = pDesc->GetAttributeBool(NO_SHIELD_RETREAT_ATTRIB);
	m_fNoDogfights = pDesc->GetAttributeBool(NO_DOGFIGHTS_ATTRIB);
	m_fNonCombatant = pDesc->GetAttributeBool(NON_COMBATANT_ATTRIB);
	m_fNoFriendlyFire = pDesc->GetAttributeBool(NO_FRIENDLY_FIRE_ATTRIB);
	m_fAggressor = pDesc->GetAttributeBool(AGGRESSOR_ATTRIB);
	m_fNoAttackOnThreat = pDesc->GetAttributeBool(NO_ATTACK_ON_THREAT_ATTRIB);
	m_fNoTargetsOfOpportunity = pDesc->GetAttributeBool(NO_TARGETS_OF_OPPORTUNITY_ATTRIB);
	m_fNoFriendlyFireCheck = pDesc->GetAttributeBool(NO_FRIENDLY_FIRE_CHECK_ATTRIB);
	m_fNoNavPaths = pDesc->GetAttributeBool(NO_NAV_PATHS_ATTRIB);
	m_fNoOrderGiver = pDesc->GetAttributeBool(NO_ORDER_GIVER_ATTRIB);
	m_fFlockFormation = pDesc->GetAttributeBool(FLOCK_FORMATION_ATTRIB);

	return NOERROR;
	}

void CAISettings::InitToDefault (void)

//	InitToDefault
//
//	Initialize to default settings

	{
	m_iCombatStyle = aicombatStandard;
	m_iFireRateAdj = 10;					//	Normal fire rate
	m_iFireRangeAdj = 100;					//	100% of fire range
	m_iFireAccuracy = 100;					//	100% accuracy
	m_iPerception = CSpaceObject::perceptNormal;
	m_rMinCombatSeparation = -1.0;			//	Compute based on image size

	m_fAscendOnGate = false;
	m_fNoShieldRetreat = false;
	m_fNoDogfights = false;
	m_fNonCombatant = false;
	m_fNoFriendlyFire = false;
	m_fAggressor = false;
	m_fNoAttackOnThreat = false;
	m_fNoFriendlyFireCheck = false;
	m_fNoNavPaths = false;
	m_fNoOrderGiver = false;
	}

void CAISettings::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read from stream
//
//	DWORD		m_iCombatStyle
//	DWORD		LO = m_iFireRateAdj; HI = m_iFireRangeAdj
//	DWORD		LO = m_iFireAccuracy; HI = m_iPerception
//	Metric		m_rMinCombatSeparation
//	DWORD		flags

	{
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iCombatStyle = (AICombatStyles)dwLoad;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iFireRateAdj = (int)LOWORD(dwLoad);
	m_iFireRangeAdj = (int)HIWORD(dwLoad);

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iFireAccuracy = (int)LOWORD(dwLoad);
	m_iPerception = (int)HIWORD(dwLoad);

	Ctx.pStream->Read((char *)&m_rMinCombatSeparation, sizeof(Metric));

	//	Flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_fNoShieldRetreat =		((dwLoad & 0x00000001) ? true : false);
	m_fNoDogfights =			((dwLoad & 0x00000002) ? true : false);
	m_fNonCombatant =			((dwLoad & 0x00000004) ? true : false);
	m_fNoFriendlyFire =			((dwLoad & 0x00000008) ? true : false);
	m_fAggressor =				((dwLoad & 0x00000010) ? true : false);
	m_fNoFriendlyFireCheck =	((dwLoad & 0x00000020) ? true : false);
	m_fNoOrderGiver =			((dwLoad & 0x00000040) ? true : false);
	m_fAscendOnGate =			((dwLoad & 0x00000080) ? true : false);
	m_fNoNavPaths =				((dwLoad & 0x00000100) ? true : false);
	m_fNoAttackOnThreat =		((dwLoad & 0x00000200) ? true : false);
	m_fNoTargetsOfOpportunity =	((dwLoad & 0x00000400) ? true : false);
	m_fFlockFormation =			((dwLoad & 0x00000800) ? true : false);
	}

CString CAISettings::SetValue (const CString &sSetting, const CString &sValue)

//	SetValue
//
//	Set AISettings value

	{
	if (strEquals(sSetting, AGGRESSOR_ATTRIB))
		m_fAggressor = !sValue.IsBlank();
	else if (strEquals(sSetting, ASCEND_ON_GATE_ATTRIB))
		m_fAscendOnGate = !sValue.IsBlank();
	else if (strEquals(sSetting, COMBAT_SEPARATION_ATTRIB))
		m_rMinCombatSeparation = Max(1, strToInt(sValue, 1)) * g_KlicksPerPixel;
	else if (strEquals(sSetting, COMBAT_STYLE_ATTRIB))
		m_iCombatStyle = ConvertToAICombatStyle(sValue);
	else if (strEquals(sSetting, FIRE_ACCURACY_ATTRIB))
		m_iFireAccuracy = Max(0, Min(strToInt(sValue, 100), 100));
	else if (strEquals(sSetting, FIRE_RANGE_ADJ_ATTRIB))
		m_iFireRangeAdj = Max(1, strToInt(sValue, 100));
	else if (strEquals(sSetting, FIRE_RATE_ADJ_ATTRIB))
		m_iFireRateAdj = Max(1, strToInt(sValue, 10));
	else if (strEquals(sSetting, FLOCK_FORMATION_ATTRIB))
		m_fFlockFormation = !sValue.IsBlank();
	else if (strEquals(sSetting, NO_ATTACK_ON_THREAT_ATTRIB))
		m_fNoAttackOnThreat = !sValue.IsBlank();
	else if (strEquals(sSetting, NO_TARGETS_OF_OPPORTUNITY_ATTRIB))
		m_fNoTargetsOfOpportunity = !sValue.IsBlank();
	else if (strEquals(sSetting, NO_DOGFIGHTS_ATTRIB))
		m_fNoDogfights = !sValue.IsBlank();
	else if (strEquals(sSetting, NO_SHIELD_RETREAT_ATTRIB))
		m_fNoShieldRetreat = !sValue.IsBlank();
	else if (strEquals(sSetting, NO_FRIENDLY_FIRE_ATTRIB))
		m_fNoFriendlyFire = !sValue.IsBlank();
	else if (strEquals(sSetting, NO_FRIENDLY_FIRE_CHECK_ATTRIB))
		m_fNoFriendlyFireCheck = !sValue.IsBlank();
	else if (strEquals(sSetting, NO_NAV_PATHS_ATTRIB))
		m_fNoNavPaths = !sValue.IsBlank();
	else if (strEquals(sSetting, NO_ORDER_GIVER_ATTRIB))
		m_fNoOrderGiver = !sValue.IsBlank();
	else if (strEquals(sSetting, NON_COMBATANT_ATTRIB))
		m_fNonCombatant = !sValue.IsBlank();
	else if (strEquals(sSetting, PERCEPTION_ATTRIB))
		m_iPerception = Max((int)CSpaceObject::perceptMin, Min(strToInt(sValue, CSpaceObject::perceptNormal), (int)CSpaceObject::perceptMax));
	else
		return NULL_STR;

	return GetValue(sSetting);
	}

void CAISettings::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write to a stream
//
//	DWORD		m_iCombatStyle
//	DWORD		LO = m_iFireRateAdj; HI = m_iFireRangeAdj
//	DWORD		LO = m_iFireAccuracy; HI = m_iPerception
//	Metric		m_rMinCombatSeparation
//	DWORD		flags

	{
	DWORD dwSave;

	dwSave = (DWORD)m_iCombatStyle;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = MAKELONG(m_iFireRateAdj, m_iFireRangeAdj);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = MAKELONG(m_iFireAccuracy, m_iPerception);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_rMinCombatSeparation, sizeof(Metric));

	//	Flags

	dwSave = 0;
	dwSave |= (m_fNoShieldRetreat ?			0x00000001 : 0);
	dwSave |= (m_fNoDogfights ?				0x00000002 : 0);
	dwSave |= (m_fNonCombatant ?			0x00000004 : 0);
	dwSave |= (m_fNoFriendlyFire ?			0x00000008 : 0);
	dwSave |= (m_fAggressor ?				0x00000010 : 0);
	dwSave |= (m_fNoFriendlyFireCheck ?		0x00000020 : 0);
	dwSave |= (m_fNoOrderGiver ?			0x00000040 : 0);
	dwSave |= (m_fAscendOnGate ?			0x00000080 : 0);
	dwSave |= (m_fNoNavPaths ?				0x00000100 : 0);
	dwSave |= (m_fNoAttackOnThreat ?		0x00000200 : 0);
	dwSave |= (m_fNoTargetsOfOpportunity ?	0x00000400 : 0);
	dwSave |= (m_fFlockFormation ?			0x00000800 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}
