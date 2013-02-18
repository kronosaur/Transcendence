//	CStationEncounterDesc.cpp
//
//	CStationEncounterDesc class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define ENEMY_EXCLUSION_RADIUS_ATTRIB			CONSTLIT("enemyExclusionRadius")
#define LEVEL_FREQUENCY_ATTRIB					CONSTLIT("levelFrequency")
#define LOCATION_CRITERIA_ATTRIB				CONSTLIT("locationCriteria")
#define MAX_APPEARING_ATTRIB					CONSTLIT("maxAppearing")
#define UNIQUE_ATTRIB							CONSTLIT("unique")

#define UNIQUE_IN_SYSTEM						CONSTLIT("inSystem")
#define UNIQUE_IN_UNIVERSE						CONSTLIT("inUniverse")

#define VALUE_FALSE								CONSTLIT("false")
#define VALUE_TRUE								CONSTLIT("true")

int CStationEncounterDesc::CalcLevelFromFrequency (void) const

//	CalcLevelFromFrequency
//
//	Computes a level from a frequency. Returns 0 if we could not compute
//	the level.

	{
	int iLevel = 1;
	int iTotal = 0;
	int iCount = 0;

	char *pPos = m_sLevelFrequency.GetASCIIZPointer();
	while (*pPos != '\0')
		{
		int iFreq = 0;

		switch (*pPos)
			{
			case 'C':
			case 'c':
				iFreq = ftCommon;
				break;

			case 'U':
			case 'u':
				iFreq = ftUncommon;
				break;

			case 'R':
			case 'r':
				iFreq = ftRare;
				break;

			case 'V':
			case 'v':
				iFreq = ftVeryRare;
				break;
			}

		iTotal += iFreq * iLevel;
		iCount += iFreq;

		pPos++;
		if (*pPos != ' ')
			iLevel++;
		}

	//	Compute the level by averaging things out

	return (iCount > 0 ? (int)(((double)iTotal / (double)iCount) + 0.5) : 0);
	}

int CStationEncounterDesc::GetCountOfRandomEncounterLevels (void) const

//	GetCountOfRandomEncounterLevels
//
//	Returns the number of levels in which the station can be
//	encountered.

	{
	int iCount = 0;

	char *pPos = m_sLevelFrequency.GetASCIIZPointer();
	while (*pPos != '\0')
		{
		if (*pPos != ' ' && *pPos != '-')
			iCount++;

		pPos++;
		}

	return iCount;
	}

int CStationEncounterDesc::GetFrequencyByLevel (int iLevel) const

//	GetFrequencyByLevel
//
//	Returns the frequency of the encounter
	
	{
	if (!CanBeRandomlyEncountered())
		return 0;

	return ::GetFrequencyByLevel(m_sLevelFrequency, iLevel);
	}

ALERROR CStationEncounterDesc::InitFromStationTypeXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitFromStationTypeXML
//
//	Initialize from <StationType>

	{
	ALERROR error;

	//	Get unique attributes

	CString sMaxAppearing;
	if (pDesc->FindAttribute(MAX_APPEARING_ATTRIB, &sMaxAppearing))
		{
		m_bMaxCountLimit = true;
		if (error = m_MaxAppearing.LoadFromXML(sMaxAppearing))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Invalid maxAppearing parameter."));
			return error;
			}

		m_iMaxCountInSystem = 1;
		}
	else
		{
		CString sUnique = pDesc->GetAttribute(UNIQUE_ATTRIB);
		if (strEquals(sUnique, UNIQUE_IN_SYSTEM))
			{
			m_bMaxCountLimit = false;
			m_iMaxCountInSystem = 1;
			}
		else if (strEquals(sUnique, UNIQUE_IN_UNIVERSE) || strEquals(sUnique, VALUE_TRUE))
			{
			m_bMaxCountLimit = true;
			m_MaxAppearing.SetConstant(1);
			m_iMaxCountInSystem = 1;
			}
		else
			{
			m_bMaxCountLimit = false;
			m_iMaxCountInSystem = -1;
			}
		}

	//	Level frequency

	m_sLevelFrequency = pDesc->GetAttribute(LEVEL_FREQUENCY_ATTRIB);
	m_sLocationCriteria = pDesc->GetAttribute(LOCATION_CRITERIA_ATTRIB);

	//	Exclusion radius

	int iRadius;
	if (pDesc->FindAttributeInteger(ENEMY_EXCLUSION_RADIUS_ATTRIB, &iRadius)
			&& iRadius >= 0)
		m_rEnemyExclusionRadius = iRadius * LIGHT_SECOND;
	else
		m_rEnemyExclusionRadius = 30 * LIGHT_SECOND;

	//	Done

	return NOERROR;
	}
