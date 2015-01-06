//	CMiscellaneousClass.cpp
//
//	CMiscellaneousClass class

#include "PreComp.h"

#define POWER_USE_ATTRIB						(CONSTLIT("powerUse"))
#define POWER_TO_ACTIVATE_ATTRIB				(CONSTLIT("powerToActivate"))
#define CAPACITOR_POWER_USE_ATTRIB				(CONSTLIT("capacitorPowerUse"))
#define POWER_RATING_ATTRIB						(CONSTLIT("powerRating"))

#define TICKS_PER_UPDATE						30

CMiscellaneousClass::CMiscellaneousClass (void) : CDeviceClass(NULL)

//	CAutoDefenseClass constructor

	{
	}

int CMiscellaneousClass::CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	CalcPowerUsed
//
//	Returns the amount of power used per tick

	{
	int iPower = 0;

	//	Only if enabled

	if (!pDevice->IsEnabled())
		return 0;

	//	Add constant power use

	iPower += m_iPowerUse;

	//	If we're not ready, then we use up power to charge the capacitors

	if (!pDevice->IsReady())
		iPower += GetCapacitorPowerUse(pDevice, pSource);

	//	Done

	return iPower;
	}

ALERROR CMiscellaneousClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice)

//	CreateFromXML
//
//	Load the class

	{
	ALERROR error;
	CMiscellaneousClass *pDevice;

	pDevice = new CMiscellaneousClass;
	if (pDevice == NULL)
		return ERR_MEMORY;

	if (error = pDevice->InitDeviceFromXML(Ctx, pDesc, pType))
		return error;

	pDevice->m_iPowerRating = pDesc->GetAttributeInteger(POWER_RATING_ATTRIB);
	pDevice->m_iPowerUse = pDesc->GetAttributeInteger(POWER_USE_ATTRIB);
	pDevice->m_iPowerToActivate = pDesc->GetAttributeInteger(POWER_TO_ACTIVATE_ATTRIB);
	pDevice->m_iPowerForCapacitor = pDesc->GetAttributeInteger(CAPACITOR_POWER_USE_ATTRIB);

	//	Done

	*retpDevice = pDevice;

	return NOERROR;
	}

int CMiscellaneousClass::GetActivateDelay (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetActivateDelay
//
//	Compute activation delay based on power usage

	{
	if (m_iPowerToActivate == 0)
		return 0;

	int iPowerPerCycle = GetCapacitorPowerUse(pDevice, pSource);
	if (iPowerPerCycle == 0)
		return 0;

	return (m_iPowerToActivate / iPowerPerCycle);
	}

int CMiscellaneousClass::GetCapacitorPowerUse (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetCapacitorPowerUse
//
//	Returns the amount of power required to charge capacitors
//	for each tick

	{
	if (m_iPowerForCapacitor >= 0)
		return m_iPowerForCapacitor;
	else
		{
		//	m_iPowerForCapacitor is a % of max power

		return pSource->GetMaxPower() * (-m_iPowerForCapacitor) / 100;
		}
	}

int CMiscellaneousClass::GetCounter (CInstalledDevice *pDevice, CSpaceObject *pSource, CounterTypes *retiType)

//	GetCounter
//
//	Returns the charge level

	{
	//	If we don't need activation power, or if we're full, no counter

	if (m_iPowerToActivate == 0 || pDevice->IsReady())
		{
		if (retiType)
			*retiType = cntNone;
		return 0;
		}

	//	Figure out how long before we're ready

	int iActivateDelay = GetActivateDelay(pDevice, pSource);
	int iLevel = (iActivateDelay > 0 ? 100 - (pDevice->GetTimeUntilReady() * 100 / iActivateDelay) : 0);

	if (retiType)
		*retiType = cntCapacitor;

	return iLevel;
	}

int CMiscellaneousClass::GetPowerRating (CItemCtx &Ctx)

//	GetPowerRating
//
//	Returns the minimum reactor size needed for this device

	{
	//	If we have an explicit power rating, then use that

	if (m_iPowerRating > 0)
		return m_iPowerRating;

	//	If we can calculate max power usage, then return that

	else if (m_iPowerForCapacitor >= 0)
		return m_iPowerUse + m_iPowerForCapacitor;

	//	If power usage is relative to reactor output, then 
	//	assume 100MW reactor

	else
		return m_iPowerUse + (1000 * (-m_iPowerForCapacitor) / 100);
	}

bool CMiscellaneousClass::ShowActivationDelayCounter (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	ShowActivationDelayCounter
//
//	Returns TRUE if we show a counter while charging

	{
	return (m_iPowerToActivate != 0);
	}

void CMiscellaneousClass::Update (CInstalledDevice *pDevice, 
								  CSpaceObject *pSource, 
								  int iTick,
								  bool *retbSourceDestroyed,
								  bool *retbConsumedItems)

//	Update
//
//	Update device

	{
	if (retbConsumedItems)
		*retbConsumedItems = false;
	}
