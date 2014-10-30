//	CStationEncounterDesc.cpp
//
//	CStationEncounterDesc class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define CRITERIA_TAG							CONSTLIT("Criteria")
#define ENCOUNTER_TAG							CONSTLIT("Encounter")

#define ENEMY_EXCLUSION_RADIUS_ATTRIB			CONSTLIT("enemyExclusionRadius")
#define LEVEL_FREQUENCY_ATTRIB					CONSTLIT("levelFrequency")
#define LOCATION_CRITERIA_ATTRIB				CONSTLIT("locationCriteria")
#define MAX_APPEARING_ATTRIB					CONSTLIT("maxAppearing")
#define MIN_APPEARING_ATTRIB					CONSTLIT("minAppearing")
#define NUMBER_APPEARING_ATTRIB					CONSTLIT("numberAppearing")
#define SYSTEM_CRITERIA_ATTRIB					CONSTLIT("systemCriteria")
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
//	Returns the frequency of the encounter by level. Note this is used for stats 
//	and random encounters. To determine the frequency of a station in a given
//	system, use GetFrequencyBySystem instead.

	{
	if (!CanBeRandomlyEncountered())
		return 0;

	if (m_sLevelFrequency.IsBlank())
		return ftCommon;
	else
		return ::GetFrequencyByLevel(m_sLevelFrequency, iLevel);
	}

int CStationEncounterDesc::GetFrequencyByNode (CTopologyNode *pNode, CStationType *pType) const

//	GetFrequencyByNode
//
//	Returns the frequency of the encounter in this system.

	{
	if (!CanBeRandomlyEncountered())
		return 0;

	//	If we have system criteria, then check it here.

	if (m_bSystemCriteria)
		{
		CTopologyNode::SCriteriaCtx Ctx;
		Ctx.pTopology = &g_pUniverse->GetTopology();
		if (!pNode->MatchesCriteria(Ctx, m_SystemCriteria))
			return 0;
		}

	//	Check by level frequency

	if (m_sLevelFrequency.IsBlank())
		return ftCommon;
	else
		return ::GetFrequencyByLevel(m_sLevelFrequency, pNode->GetLevel());
	}

ALERROR CStationEncounterDesc::InitFromStationTypeXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitFromStationTypeXML
//
//	Initialize from <StationType>

	{
	//	If we have an encounter element, then use that.

	CXMLElement *pEncounter = pDesc->GetContentElementByTag(ENCOUNTER_TAG);
	if (pEncounter)
		return InitFromXML(Ctx, pEncounter);

	//	Otherwise, we use information at the root

	return InitFromXML(Ctx, pDesc);
	}

ALERROR CStationEncounterDesc::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitFromXML
//
//	Initialize from an <Encounter> element (or a root <StationType> element).

	{
	ALERROR error;
	bool bNumberSet = false;

	//	Number appearing (at least this number, unique in system)

	CString sAttrib;
	if (pDesc->FindAttribute(MIN_APPEARING_ATTRIB, &sAttrib)
			|| pDesc->FindAttribute(NUMBER_APPEARING_ATTRIB, &sAttrib))
		{
		m_bNumberAppearing = true;
		if (error = m_NumberAppearing.LoadFromXML(sAttrib))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Invalid numberAppearing parameter."));
			return error;
			}

		m_iMaxCountInSystem = 1;
		bNumberSet = true;
		}

	//	Get maximum limit (at most this number, unique in system)

	if (pDesc->FindAttribute(MAX_APPEARING_ATTRIB, &sAttrib))
		{
		m_bMaxCountLimit = true;
		if (error = m_MaxAppearing.LoadFromXML(sAttrib))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Invalid maxAppearing parameter."));
			return error;
			}

		m_iMaxCountInSystem = 1;
		bNumberSet = true;
		}

	//	Otherwise, we check uniqueness values

	if (!bNumberSet)
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

	//	System criteria

	CXMLElement *pCriteria = pDesc->GetContentElementByTag(CRITERIA_TAG);
	if (pCriteria)
		{
		if (error = CTopologyNode::ParseCriteria(pCriteria, &m_SystemCriteria, &Ctx.sError))
			return error;

		m_bSystemCriteria = true;
		}
	else if (pDesc->FindAttribute(SYSTEM_CRITERIA_ATTRIB, &sAttrib))
		{
		if (error = CTopologyNode::ParseCriteria(sAttrib, &m_SystemCriteria, &Ctx.sError))
			return error;

		m_bSystemCriteria = true;
		}

	//	Level frequency and criteria

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

void CStationEncounterDesc::InitLevelFrequency (void)

//	InitLevelFrequency
//
//	Initializes m_sLevelFrequency if it is not set and if we have a system
//	criteria. This must be called only after the topology has been initialized.

	{
	int i;

	if ((m_sLevelFrequency.IsBlank() || m_bAutoLevelFrequency)
			&& m_bSystemCriteria)
		{
		//	Start with no encounters

		m_sLevelFrequency = CString("----- ----- ----- ----- -----");

		//	Loop over all nodes and check to see if we appear at the node.
		//	If we do, we mark that level 'common.'

		CTopologyNode::SCriteriaCtx Ctx;
		CTopology &Topology = g_pUniverse->GetTopology();
		for (i = 0; i < Topology.GetTopologyNodeCount(); i++)
			{
			CTopologyNode *pNode = Topology.GetTopologyNode(i);

			if (pNode->MatchesCriteria(Ctx, m_SystemCriteria))
				::SetFrequencyByLevel(m_sLevelFrequency, pNode->GetLevel(), ftCommon);
			}

		//	Remember that we initialized this so that we can save it out.

		m_bAutoLevelFrequency = true;
		}
	}

void CStationEncounterDesc::ReadFromStream (SUniverseLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads from a stream

	{
	CString sLevelFrequency;

	sLevelFrequency.ReadFromStream(Ctx.pStream);
	if (!sLevelFrequency.IsBlank())
		{
		m_sLevelFrequency = sLevelFrequency;
		m_bAutoLevelFrequency = true;
		}
	}

void CStationEncounterDesc::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Writes to a stream

	{
	//	We only need to write the level frequency if we generated it from
	//	topology.

	if (m_bAutoLevelFrequency)
		m_sLevelFrequency.WriteToStream(pStream);
	else
		NULL_STR.WriteToStream(pStream);
	}

