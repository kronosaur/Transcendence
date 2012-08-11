//	CReactorClass.cpp
//
//	CReactorClass class

#include "PreComp.h"

#define FUEL_CAPACITY_ATTRIB					CONSTLIT("fuelCapacity")
#define FUEL_CRITERIA_ATTRIB					CONSTLIT("fuelCriteria")
#define FUEL_EFFICIENCY_ATTRIB					CONSTLIT("fuelEfficiency")
#define MAX_FUEL_ATTRIB							CONSTLIT("maxFuel")
#define MAX_FUEL_TECH_ATTRIB					CONSTLIT("maxFuelTech")
#define MIN_FUEL_TECH_ATTRIB					CONSTLIT("minFuelTech")
#define MAX_POWER_ATTRIB						CONSTLIT("maxPower")
#define REACTOR_EFFICIENCY_ATTRIB				CONSTLIT("reactorEfficiency")
#define REACTOR_POWER_ATTRIB					CONSTLIT("reactorPower")

#define FIELD_FUEL_CAPACITY						CONSTLIT("fuelCapacity")
#define FIELD_FUEL_CRITERIA						CONSTLIT("fuelCriteria")
#define FIELD_FUEL_EFFICIENCY					CONSTLIT("fuelEfficiency")
#define FIELD_POWER								CONSTLIT("power")

CReactorClass::CReactorClass (void) : CDeviceClass(NULL)
	{
	}

ALERROR CReactorClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice)

//	CreateFromXML
//
//	Creates from an XML element

	{
	ALERROR error;
	CReactorClass *pDevice;

	pDevice = new CReactorClass;
	if (pDevice == NULL)
		return ERR_MEMORY;

	if (error = pDevice->InitDeviceFromXML(Ctx, pDesc, pType))
		return error;

	//	Load the normal descriptor

	if (error = InitReactorDesc(Ctx, pDesc, &pDevice->m_Desc))
		return error;

	//	Compute descriptor when damaged

	pDevice->m_DamagedDesc.iMaxPower = 80 * pDevice->m_Desc.iMaxPower / 100;
	pDevice->m_DamagedDesc.iMaxFuel = pDevice->m_Desc.iMaxFuel;
	pDevice->m_DamagedDesc.iMinFuelLevel = pDevice->m_Desc.iMinFuelLevel;
	pDevice->m_DamagedDesc.iMaxFuelLevel = pDevice->m_Desc.iMaxFuelLevel;
	pDevice->m_DamagedDesc.iPowerPerFuelUnit = 80 * pDevice->m_Desc.iPowerPerFuelUnit / 100;
	pDevice->m_DamagedDesc.pFuelCriteria = pDevice->m_Desc.pFuelCriteria;
	pDevice->m_DamagedDesc.fFreeFuelCriteria = false;
	pDevice->m_DamagedDesc.fDamaged = true;
	pDevice->m_DamagedDesc.fEnhanced = false;

	//	Compute descriptor when enhanced

	pDevice->m_EnhancedDesc.iMaxPower = 120 * pDevice->m_Desc.iMaxPower / 100;
	pDevice->m_EnhancedDesc.iMaxFuel = pDevice->m_Desc.iMaxFuel;
	pDevice->m_EnhancedDesc.iMinFuelLevel = pDevice->m_Desc.iMinFuelLevel;
	pDevice->m_EnhancedDesc.iMaxFuelLevel = pDevice->m_Desc.iMaxFuelLevel;
	pDevice->m_EnhancedDesc.iPowerPerFuelUnit = 150 * pDevice->m_Desc.iPowerPerFuelUnit / 100;
	pDevice->m_EnhancedDesc.pFuelCriteria = pDevice->m_Desc.pFuelCriteria;
	pDevice->m_EnhancedDesc.fFreeFuelCriteria = false;
	pDevice->m_EnhancedDesc.fDamaged = false;
	pDevice->m_EnhancedDesc.fEnhanced = true;

	//	Done

	*retpDevice = pDevice;

	return NOERROR;
	}

bool CReactorClass::FindDataField (const ReactorDesc &Desc, const CString &sField, CString *retsValue)

//	FindDataField
//
//	Finds a data field for the reactor desc.

	{
	if (strEquals(sField, FIELD_POWER))
		*retsValue = strFromInt(Desc.iMaxPower * 100);
	else if (strEquals(sField, FIELD_FUEL_CRITERIA))
		{
		if (Desc.pFuelCriteria)
			*retsValue = CItem::GenerateCriteria(*Desc.pFuelCriteria);
		else
			*retsValue = strPatternSubst(CONSTLIT("f L:%d-%d;"), Desc.iMinFuelLevel, Desc.iMaxFuelLevel);
		}
	else if (strEquals(sField, FIELD_FUEL_EFFICIENCY))
		*retsValue = strFromInt(Desc.iPowerPerFuelUnit);
	else if (strEquals(sField, FIELD_FUEL_CAPACITY))
		*retsValue = strFromInt(Desc.iMaxFuel / FUEL_UNITS_PER_STD_ROD);
	else
		return false;

	return true;
	}

bool CReactorClass::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	return FindDataField(m_Desc, sField, retsValue);
	}

const ReactorDesc *CReactorClass::GetReactorDesc (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetReactorDesc
//
//	Returns the reactor descriptor

	{
	if (pDevice == NULL)
		return &m_Desc;
	else if (pDevice->IsDamaged() || pDevice->IsDisrupted())
		return &m_DamagedDesc;
	else if (pDevice->IsEnhanced())
		return &m_EnhancedDesc;
	else
		return &m_Desc;
	}

ALERROR CReactorClass::InitReactorDesc (SDesignLoadCtx &Ctx, CXMLElement *pDesc, ReactorDesc *retDesc, bool bShipClass)

//	InitReactorDesc
//
//	Initialize reactor desc structure from XML.

	{
	//	Initialize

	if (retDesc->pFuelCriteria)
		{
		delete retDesc->pFuelCriteria;
		retDesc->pFuelCriteria = NULL;
		retDesc->fFreeFuelCriteria = false;
		}

	//	Load some values

	if (bShipClass)
		{
		retDesc->iMaxPower = pDesc->GetAttributeIntegerBounded(REACTOR_POWER_ATTRIB, 0, -1, 100);
		retDesc->iMaxFuel = pDesc->GetAttributeIntegerBounded(FUEL_CAPACITY_ATTRIB, 0, -1, retDesc->iMaxPower * 250);
		retDesc->iPowerPerFuelUnit = pDesc->GetAttributeIntegerBounded(REACTOR_EFFICIENCY_ATTRIB, 0, -1, g_MWPerFuelUnit);
		}
	else
		{
		retDesc->iMaxPower = pDesc->GetAttributeIntegerBounded(MAX_POWER_ATTRIB, 0, -1, 100);
		retDesc->iMaxFuel = pDesc->GetAttributeIntegerBounded(MAX_FUEL_ATTRIB, 0, -1, retDesc->iMaxFuel * 250);
		retDesc->iPowerPerFuelUnit = pDesc->GetAttributeIntegerBounded(FUEL_EFFICIENCY_ATTRIB, 0, -1, g_MWPerFuelUnit);
		}


	retDesc->fDamaged = false;
	retDesc->fEnhanced = false;

	//	Load the fuel criteria

	CString sCriteria;
	if (pDesc->FindAttribute(FUEL_CRITERIA_ATTRIB, &sCriteria))
		{
		retDesc->pFuelCriteria = new CItemCriteria;
		retDesc->fFreeFuelCriteria = true;

		CItem::ParseCriteria(sCriteria, retDesc->pFuelCriteria);

		retDesc->iMinFuelLevel = -1;
		retDesc->iMaxFuelLevel = -1;

		//	Warn if older method is used

		if (pDesc->AttributeExists(MIN_FUEL_TECH_ATTRIB) || pDesc->AttributeExists(MAX_FUEL_TECH_ATTRIB))
			kernelDebugLogMessage("Warning: minFuelTech and maxFuelTech ignored if fuelCriteria specified.");
		}

	//	If we have no fuel criteria, then use the older method

	else
		{
		retDesc->iMinFuelLevel = pDesc->GetAttributeIntegerBounded(MIN_FUEL_TECH_ATTRIB, 1, MAX_ITEM_LEVEL, 1);
		retDesc->iMaxFuelLevel = pDesc->GetAttributeIntegerBounded(MAX_FUEL_TECH_ATTRIB, retDesc->iMinFuelLevel, MAX_ITEM_LEVEL, (bShipClass ? 3 : MAX_ITEM_LEVEL));
		}

	return NOERROR;
	}

bool CReactorClass::IsFuelCompatible (CItemCtx &Ctx, const CItem &FuelItem)

//	IsFuelCompatible
//
//	Returns TRUE if the given fuel item is compatible with this reactor

	{
	if (m_Desc.pFuelCriteria)
		return FuelItem.MatchesCriteria(*m_Desc.pFuelCriteria);
	else
		{
		int iLevel = FuelItem.GetType()->GetLevel();
		return (iLevel >= m_Desc.iMinFuelLevel 
				&& iLevel <= m_Desc.iMaxFuelLevel);
		}
	}

void CReactorClass::OnInstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList)

//	OnInstall
//
//	Device installed

	{
	//	Identified when installed

	if (pSource->IsPlayer())
		GetItemType()->SetKnown();
	}
