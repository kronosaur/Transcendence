//	CRegenDesc.cpp
//
//	CRegenDesc class

#include "PreComp.h"

#define DECAY_RATE_ATTRIB						CONSTLIT("decayRate")
#define REPAIR_RATE_ATTRIB						CONSTLIT("repairRate")

const int TICKS_PER_CYCLE =						10;
const int CYCLES_PER_ERA =						360;
const int STANDARD_REGEN_PERIOD =				180;

CRegenDesc::CRegenDesc (int iHPPerEra)

//	CRegenDesc constructor

	{
	Init(iHPPerEra);
	}

void CRegenDesc::Add (const CRegenDesc &Desc)

//	Add
//
//	Adds the given regen desc

	{
	m_iHPPerCycle += Desc.m_iHPPerCycle;

	int iRemainder = m_iHPPerEraRemainder + Desc.m_iHPPerEraRemainder;
	m_iHPPerCycle += iRemainder / CYCLES_PER_ERA;
	m_iHPPerEraRemainder = iRemainder % CYCLES_PER_ERA;

	m_bEmpty = (m_iHPPerCycle == 0 && m_iHPPerEraRemainder == 0);
	}

int CRegenDesc::GetHPPerEra (void)

//	GetHPPerEra
//
//	Returns the number of HP regened per era

	{
	return (m_iHPPerCycle * CYCLES_PER_ERA) + m_iHPPerEraRemainder;
	}

int CRegenDesc::GetRegen (int iTick)

//	GetRegen
//
//	Returns the number of HP to regen on the given tick. This must be
//	once per cycle (and no more).

	{
	if (m_bEmpty)
		return 0;

	int iCycleNo = (iTick / TICKS_PER_CYCLE) % CYCLES_PER_ERA;

	if (m_iHPPerEraRemainder)
		{
		//	Remaining HP are spread out across the entire era.

		int iRemainderPeriod = CYCLES_PER_ERA / m_iHPPerEraRemainder;
		if ((iCycleNo % iRemainderPeriod) != 0)
			return m_iHPPerCycle;

		int iLastCycle = iRemainderPeriod * (m_iHPPerEraRemainder - 1);

		if (iCycleNo > iLastCycle)
			return m_iHPPerCycle;
		
		return m_iHPPerCycle + 1;
		}

	return m_iHPPerCycle;
	}

void CRegenDesc::Init (int iHPPerEra)

//	Init
//
//	Initializes from HPPerEra

	{
	m_iHPPerCycle = iHPPerEra / CYCLES_PER_ERA;
	m_iHPPerEraRemainder = iHPPerEra % CYCLES_PER_ERA;

	m_bEmpty = (m_iHPPerCycle == 0 && m_iHPPerEraRemainder == 0);
	}

ALERROR CRegenDesc::InitFromRegenString (SDesignLoadCtx &Ctx, const CString &sRegen)

//	InitFromRegenString
//
//	Initializes from a regen value

	{
	Metric rRegen = strToDouble(sRegen, 0.0);
	int iHPPerEra = (int)((rRegen * (CYCLES_PER_ERA * TICKS_PER_CYCLE) / STANDARD_REGEN_PERIOD) + 0.5);
	Init(iHPPerEra);

	return NOERROR;
	}

ALERROR CRegenDesc::InitFromRepairRateString (SDesignLoadCtx &Ctx, const CString &sRepairRate)

//	InitFromRepairRateString
//
//	Initializes from a repair rate value

	{
	int iRegenRate = strToInt(sRepairRate, 0);
	if (iRegenRate <= 0)
		{
		ASSERT(m_bEmpty);
		return NOERROR;
		}

	int iRegenHP = 1;
	int iPeriodsPerEra = (CYCLES_PER_ERA * TICKS_PER_CYCLE) / iRegenRate;
	int iPeriodsRemainder = (CYCLES_PER_ERA * TICKS_PER_CYCLE) % iRegenRate;

	int iHPPerEra = iPeriodsPerEra * iRegenHP + (iPeriodsRemainder * iRegenHP / iRegenRate);
	Init(iHPPerEra);

	return NOERROR;
	}

ALERROR CRegenDesc::InitFromXML (SDesignLoadCtx &Ctx, 
								 CXMLElement *pDesc, 
								 const CString &sRegenAttrib, 
								 const CString &sRegenRate,
								 const CString &sRegenHP)

//	InitFromXML
//
//	Initializes from XML

	{
	//	Compute the total number of HP regened in an era

	int iHPPerEra;

	CString sRegen;
	if (pDesc->FindAttribute(sRegenAttrib, &sRegen))
		{
		Metric rRegen = strToDouble(sRegen, 0.0);

		iHPPerEra = (int)((rRegen * (CYCLES_PER_ERA * TICKS_PER_CYCLE) / STANDARD_REGEN_PERIOD) + 0.5);
		}
	else if (!sRegenRate.IsBlank())
		{
		int iRegenRate = pDesc->GetAttributeIntegerBounded(sRegenRate, 0, -1, 0);
		if (iRegenRate == 0)
			{
			ASSERT(m_bEmpty);
			return NOERROR;
			}

		int iRegenHP = (sRegenHP.IsBlank() ? 1 : pDesc->GetAttributeIntegerBounded(sRegenHP, 0, -1, 0));

		int iPeriodsPerEra = (CYCLES_PER_ERA * TICKS_PER_CYCLE) / iRegenRate;
		int iPeriodsRemainder = (CYCLES_PER_ERA * TICKS_PER_CYCLE) % iRegenRate;

		iHPPerEra = iPeriodsPerEra * iRegenHP + (iPeriodsRemainder * iRegenHP / iRegenRate);
		}

	Init(iHPPerEra);

	return NOERROR;
	}
