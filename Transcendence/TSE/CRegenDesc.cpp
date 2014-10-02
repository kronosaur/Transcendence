//	CRegenDesc.cpp
//
//	CRegenDesc class

#include "PreComp.h"

#define DECAY_RATE_ATTRIB						CONSTLIT("decayRate")
#define REPAIR_RATE_ATTRIB						CONSTLIT("repairRate")

const int CYCLES_PER_ERA =						360;
const int STANDARD_REGEN_PERIOD =				180;	//	In ticks

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

double CRegenDesc::GetHPPer180 (int iTicksPerCycle) const

//	GetHPPer180
//
//	Returns the number of HP per 180 ticks

	{
	return 180.0 * GetHPPerEra() / (CYCLES_PER_ERA * iTicksPerCycle);
	}

int CRegenDesc::GetHPPerEra (void) const

//	GetHPPerEra
//
//	Returns the number of HP regened per era

	{
	return (m_iHPPerCycle * CYCLES_PER_ERA) + m_iHPPerEraRemainder;
	}

CString CRegenDesc::GetReferenceRate (const CString &sUnits, int iTicksPerCycle) const

//	GetReferenceRate
//
//	Returns a rate in the following forms:
//
//	<0.1 hp/sec
//	1.0 hp/sec
//	none
//
//	(Where sUnits = "hp/sec")

	{
	if (IsEmpty())
		return CONSTLIT("none");
	else
		{
		double rSecondsPerEra = CYCLES_PER_ERA * iTicksPerCycle / g_TicksPerSecond;
		int iRate10 = (int)(10.0 * GetHPPerEra() / rSecondsPerEra);

		if (iRate10 == 0)
			return strPatternSubst(CONSTLIT("<0.1 %s"), sUnits);
		else if ((iRate10 % 10) == 0)
			return strPatternSubst(CONSTLIT("%d %s"), iRate10 / 10, sUnits);
		else
			return strPatternSubst(CONSTLIT("%d.%d %s"), iRate10 / 10, iRate10 % 10, sUnits);
		}
	}

int CRegenDesc::GetRegen (int iTick, int iTicksPerCycle) const

//	GetRegen
//
//	Returns the number of HP to regen on the given tick. This must be
//	once per cycle (and no more).

	{
	if (m_bEmpty)
		return 0;

	//	In normal mode we trickle out HPs evenly over time.

	if (m_iCyclesPerBurst == 1)
		{
		int iCycleNo = (iTick / iTicksPerCycle) % CYCLES_PER_ERA;

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

	//	In burst mode, we regen HPs once per burst (but it all evens out to the
	//	same amount in the end).

	else
		{
		//	Compute the time between bursts as a percent of an era (e.g., 0.5
		//	means the time between bursts is 1/2 an era).

		Metric rBurstTime = (Metric)m_iCyclesPerBurst / CYCLES_PER_ERA;

		//	Compute the number of HP that we gain on each burst.

		int iHPPerBurst = (int)((GetHPPerEra() * rBurstTime) + 0.5);

		//	See if it is time to burst

		if (((iTick / iTicksPerCycle) % m_iCyclesPerBurst) == 0)
			return iHPPerBurst;
		else
			return 0;
		}
	}

void CRegenDesc::Init (int iHPPerEra, int iCyclesPerBurst)

//	Init
//
//	Initializes from HPPerEra

	{
	m_iHPPerCycle = iHPPerEra / CYCLES_PER_ERA;
	m_iHPPerEraRemainder = iHPPerEra % CYCLES_PER_ERA;
	m_iCyclesPerBurst = iCyclesPerBurst;

	m_bEmpty = (m_iHPPerCycle == 0 && m_iHPPerEraRemainder == 0);
	}

void CRegenDesc::InitFromRegen (double rRegen, int iTicksPerCycle)

//	InitFromRegen
//
//	Initializes from a regen value

	{
	int iHPPerEra = (int)((rRegen * (CYCLES_PER_ERA * iTicksPerCycle) / STANDARD_REGEN_PERIOD) + 0.5);
	Init(iHPPerEra);
	}

ALERROR CRegenDesc::InitFromRegenString (SDesignLoadCtx &Ctx, const CString &sRegen, int iTicksPerCycle)

//	InitFromRegenString
//
//	Initializes from a regen value

	{
	Metric rRegen = strToDouble(sRegen, 0.0);
	int iHPPerEra = (int)((rRegen * (CYCLES_PER_ERA * iTicksPerCycle) / STANDARD_REGEN_PERIOD) + 0.5);
	Init(iHPPerEra);

	return NOERROR;
	}

ALERROR CRegenDesc::InitFromRepairRateString (SDesignLoadCtx &Ctx, const CString &sRepairRate, int iTicksPerCycle)

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
	int iPeriodsPerEra = (CYCLES_PER_ERA * iTicksPerCycle) / iRegenRate;
	int iPeriodsRemainder = (CYCLES_PER_ERA * iTicksPerCycle) % iRegenRate;

	int iHPPerEra = iPeriodsPerEra * iRegenHP + (iPeriodsRemainder * iRegenHP / iRegenRate);
	Init(iHPPerEra);

	return NOERROR;
	}

ALERROR CRegenDesc::InitFromRegenTimeAndHP (SDesignLoadCtx &Ctx, int iRegenTime, int iRegenHP, int iTicksPerCycle)

//	InitFromRegenTimeAndHP
//
//	iRegenTime is the time between regen events (in game seconds)
//	iRegenHP is the number of HP regenerated per event.

	{
	if (iRegenTime < 0 || iRegenHP < 0)
		{
		Ctx.sError = CONSTLIT("RegenTime and RegenHP cannot be less than 0.");
		return ERR_FAIL;
		}
	else if (iRegenTime == 0 || iRegenHP == 0)
		{
		Init(0);
		return NOERROR;
		}

	int iTicksPerEra = CYCLES_PER_ERA * iTicksPerCycle;
	int iTicksPerEvent10 = (int)(10 * iRegenTime / STD_SECONDS_PER_UPDATE);

	double rEventsPerEra = 10.0 * iTicksPerEra / iTicksPerEvent10;
	int iHPPerEra = (int)(rEventsPerEra * iRegenHP);

	Init(iHPPerEra, Max(1, (int)(iRegenTime / STD_SECONDS_PER_UPDATE) / iTicksPerCycle));

	return NOERROR;
	}

ALERROR CRegenDesc::InitFromXML (SDesignLoadCtx &Ctx, 
								 CXMLElement *pDesc, 
								 const CString &sRegenAttrib, 
								 const CString &sRegenRate,
								 const CString &sRegenHP,
								 int iTicksPerCycle)

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

		iHPPerEra = (int)((rRegen * (CYCLES_PER_ERA * iTicksPerCycle) / STANDARD_REGEN_PERIOD) + 0.5);
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

		int iPeriodsPerEra = (CYCLES_PER_ERA * iTicksPerCycle) / iRegenRate;
		int iPeriodsRemainder = (CYCLES_PER_ERA * iTicksPerCycle) % iRegenRate;

		iHPPerEra = iPeriodsPerEra * iRegenHP + (iPeriodsRemainder * iRegenHP / iRegenRate);
		}

	Init(iHPPerEra);

	return NOERROR;
	}
