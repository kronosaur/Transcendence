//	CReactorClass.cpp
//
//	CReactorClass class

#include "PreComp.h"

#define MAX_POWER_ATTRIB						CONSTLIT("maxPower")
#define MAX_FUEL_ATTRIB							CONSTLIT("maxFuel")
#define MIN_FUEL_TECH_ATTRIB					CONSTLIT("minFuelTech")
#define MAX_FUEL_TECH_ATTRIB					CONSTLIT("maxFuelTech")
#define FUEL_EFFICIENCY_ATTRIB					CONSTLIT("fuelEfficiency")
#define FUEL_CRITERIA_ATTRIB					CONSTLIT("fuelCriteria")

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
	CReactorClass *pDevice;

	pDevice = new CReactorClass;
	if (pDevice == NULL)
		return ERR_MEMORY;

	pDevice->InitDeviceFromXML(Ctx, pDesc, pType);

	//	Load some values

	pDevice->m_Desc.iMaxPower = pDesc->GetAttributeInteger(MAX_POWER_ATTRIB);
	pDevice->m_Desc.iMaxFuel = pDesc->GetAttributeInteger(MAX_FUEL_ATTRIB);
	pDevice->m_Desc.iPowerPerFuelUnit = pDesc->GetAttributeInteger(FUEL_EFFICIENCY_ATTRIB);
	if (pDevice->m_Desc.iPowerPerFuelUnit == 0)
		pDevice->m_Desc.iPowerPerFuelUnit = g_MWPerFuelUnit;
	pDevice->m_Desc.fDamaged = false;
	pDevice->m_Desc.fEnhanced = false;

	//	Load the fuel criteria

	CString sCriteria;
	if (pDesc->FindAttribute(FUEL_CRITERIA_ATTRIB, &sCriteria))
		{
		CItem::ParseCriteria(sCriteria, &pDevice->m_FuelCriteria);

		pDevice->m_Desc.iMinFuelLevel = -1;
		pDevice->m_Desc.iMaxFuelLevel = -1;

		//	Warn if older method is used

		if (pDesc->AttributeExists(MIN_FUEL_TECH_ATTRIB) || pDesc->AttributeExists(MAX_FUEL_TECH_ATTRIB))
			kernelDebugLogMessage("Warning: minFuelTech and maxFuelTech ignored if fuelCriteria specified.");
		}

	//	If we have no fuel criteria, then use the older method

	else
		{
		int iMinFuelLevel = pDesc->GetAttributeIntegerBounded(MIN_FUEL_TECH_ATTRIB, 1, MAX_ITEM_LEVEL, 1);
		int iMaxFuelLevel = pDesc->GetAttributeIntegerBounded(MAX_FUEL_TECH_ATTRIB, iMinFuelLevel, MAX_ITEM_LEVEL, MAX_ITEM_LEVEL);

		CItem::ParseCriteria(strPatternSubst(CONSTLIT("f L:%d-%d;"), iMinFuelLevel, iMaxFuelLevel), &pDevice->m_FuelCriteria);

		pDevice->m_Desc.iMinFuelLevel = -1;
		pDevice->m_Desc.iMaxFuelLevel = -1;
		}

	//	Compute descriptor when damaged

	pDevice->m_DamagedDesc.iMaxPower = 80 * pDevice->m_Desc.iMaxPower / 100;
	pDevice->m_DamagedDesc.iMaxFuel = pDevice->m_Desc.iMaxFuel;
	pDevice->m_DamagedDesc.iMinFuelLevel = pDevice->m_Desc.iMinFuelLevel;
	pDevice->m_DamagedDesc.iMaxFuelLevel = pDevice->m_Desc.iMaxFuelLevel;
	pDevice->m_DamagedDesc.iPowerPerFuelUnit = 80 * pDevice->m_Desc.iPowerPerFuelUnit / 100;
	pDevice->m_DamagedDesc.fDamaged = true;
	pDevice->m_DamagedDesc.fEnhanced = false;

	//	Compute descriptor when enhanced

	pDevice->m_EnhancedDesc.iMaxPower = 120 * pDevice->m_Desc.iMaxPower / 100;
	pDevice->m_EnhancedDesc.iMaxFuel = pDevice->m_Desc.iMaxFuel;
	pDevice->m_EnhancedDesc.iMinFuelLevel = pDevice->m_Desc.iMinFuelLevel;
	pDevice->m_EnhancedDesc.iMaxFuelLevel = pDevice->m_Desc.iMaxFuelLevel;
	pDevice->m_EnhancedDesc.iPowerPerFuelUnit = 150 * pDevice->m_Desc.iPowerPerFuelUnit / 100;
	pDevice->m_EnhancedDesc.fDamaged = false;
	pDevice->m_EnhancedDesc.fEnhanced = true;

	//	Done

	*retpDevice = pDevice;

	return NOERROR;
	}

bool CReactorClass::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	if (strEquals(sField, FIELD_POWER))
		*retsValue = strFromInt(m_Desc.iMaxPower * 100, TRUE);
	else if (strEquals(sField, FIELD_FUEL_CRITERIA))
		*retsValue = CItem::GenerateCriteria(m_FuelCriteria);
	else if (strEquals(sField, FIELD_FUEL_EFFICIENCY))
		*retsValue = strFromInt(m_Desc.iPowerPerFuelUnit);
	else if (strEquals(sField, FIELD_FUEL_CAPACITY))
		*retsValue = strFromInt(m_Desc.iMaxFuel / FUEL_UNITS_PER_STD_ROD);
	else
		return false;

	return true;
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

bool CReactorClass::IsFuelCompatible (CItemCtx &Ctx, const CItem &FuelItem)

//	IsFuelCompatible
//
//	Returns TRUE if the given fuel item is compatible with this reactor

	{
	if (m_Desc.iMinFuelLevel == -1)
		return FuelItem.MatchesCriteria(m_FuelCriteria);
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
