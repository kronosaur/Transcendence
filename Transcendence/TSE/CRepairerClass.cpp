//	CRepairerClass.cpp
//
//	CRepairerClass class

#include "PreComp.h"


#define POWER_USE_ATTRIB			CONSTLIT("powerUse")
#define REPAIR_CYCLE_ATTRIB			CONSTLIT("repairCycle")
#define REGEN_ATTRIB				CONSTLIT("regen")

#define REPAIR_CYCLE_TIME			10

static char *CACHED_EVENTS[CRepairerClass::evtCount] =
	{
		"GetArmorRegen",
	};

CRepairerClass::CRepairerClass (void) : CDeviceClass(NULL)
	{
	}

int CRepairerClass::CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	CalcPowerUsed
//
//	Computes the amount of power used by this device each tick

	{
	//	Doesn't work if not enabled

	if (!pDevice->IsEnabled())
		return 0;

	//	We've stored power consumption on the device itself

	return (int)pDevice->GetData();
	}

void CRepairerClass::CalcRegen (CInstalledDevice *pDevice, CShip *pShip, int iSegment, int iTick, int *retiHP, int *retiPower)

//	CalcRegen
//
//	Returns the HP repaired and power consumed to repair the given segment

	{
	int iRegenHP = 0;
	int iPower = 0;

	//	Get the armor class for this segment

	CArmorClass *pArmorClass = pShip->GetArmorSection(iSegment)->GetClass();
	if (pArmorClass == NULL)
		{
		*retiHP = iRegenHP;
		*retiPower = iPower;
		return;
		}

	//	Compute the default regen and power consumption based on descriptor

	int iArmorTech = pArmorClass->GetRepairTech();
	if (iArmorTech - 1 < m_Repair.GetCount())
		{
		iRegenHP = m_Repair[iArmorTech - 1].GetRegen(iTick, REPAIR_CYCLE_TIME);
		iPower = (m_Repair[iArmorTech - 1].GetHPPerEra() > 0 ? m_iPowerUse : 0);
		}

	//	If we have an event to compute regen, then use it.

	SEventHandlerDesc Event;
	if (FindEventHandlerRepairerClass(evtGetArmorRegen, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(pShip);
		Ctx.SaveAndDefineItemVar(pShip->GetItemForDevice(pDevice));
		Ctx.DefineInteger(CONSTLIT("aArmorSeg"), iSegment);
		Ctx.DefineItemType(CONSTLIT("aArmorType"), pArmorClass->GetItemType());

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			{
			iRegenHP = 0;
			iPower = 0;
			pShip->ReportEventError(CONSTLIT("GetArmorRegen"), pResult);
			}
		else if (pResult->IsInteger())
			iRegenHP = pResult->GetIntegerValue();
		else if (!pResult->IsNil())
			{
			//	We expect a struct

			ICCItem *pRegen = pResult->GetElement(CONSTLIT("regen"));
			if (pRegen)
				iRegenHP = pRegen->GetIntegerValue();

			ICCItem *pPowerUse = pResult->GetElement(CONSTLIT("powerUse"));
			if (pPowerUse)
				iPower = pPowerUse->GetIntegerValue();
			}

		Ctx.Discard(pResult);

		//	Convert to regen HP using a CRegenDesc structure

		CRegenDesc Regen;
		Regen.InitFromRegen((double)iRegenHP, REPAIR_CYCLE_TIME);
		iRegenHP = Regen.GetRegen(iTick, REPAIR_CYCLE_TIME);
		}

	//	Done

	*retiHP = iRegenHP;
	*retiPower = iPower;
	}

ALERROR CRepairerClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice)

//	CreateFromXML
//
//	Creates from an XML element

	{
	ALERROR error;
	int i;
	CRepairerClass *pDevice;

	pDevice = new CRepairerClass;
	if (pDevice == NULL)
		return ERR_MEMORY;

	if (error = pDevice->InitDeviceFromXML(Ctx, pDesc, pType))
		return error;

	pDevice->m_iPowerUse = pDesc->GetAttributeInteger(POWER_USE_ATTRIB);

	//	Load repair attribute
	//
	//	NOTE: It's OK if we don't specify this; it probably means that we have
	//	an event to calculate it.

	CString sList;
	bool bRegen;
	if (pDesc->FindAttribute(REGEN_ATTRIB, &sList))
		bRegen = true;
	else if (pDesc->FindAttribute(REPAIR_CYCLE_ATTRIB, &sList))
		bRegen = false;
	else
		bRegen = false;

	//	Parse the list

	TArray<CString> List;
	ParseStringList(sList, 0, &List);
	for (i = 0; i < List.GetCount(); i++)
		{
		CRegenDesc *pRegen = pDevice->m_Repair.Insert();
		if (bRegen)
			pRegen->InitFromRegenString(Ctx, List[i], REPAIR_CYCLE_TIME);
		else
			pRegen->InitFromRepairRateString(Ctx, List[i], REPAIR_CYCLE_TIME);
		}

	//	Done

	*retpDevice = pDevice;

	return NOERROR;
	}

void CRepairerClass::Update (CInstalledDevice *pDevice, CSpaceObject *pSource, int iTick, bool *retbSourceDestroyed, bool *retbConsumedItems)

//	Update
//
//	Updates the device

	{
	int i;

	CShip *pShip = pSource->AsShip();
	if ((iTick % REPAIR_CYCLE_TIME) == 0 && pShip)
		{
		int iTotalPower = 0;

		//	If the repairer is enabled, attempt to repair armor. We compute the 
		//	total power consumed (per tick).

		if (pDevice->IsEnabled() 
				&& !pDevice->IsDamaged()
				&& !pDevice->IsDisrupted())
			{
			for (i = 0; i < pShip->GetArmorSectionCount(); i++)
				{
				if (pShip->IsArmorDamaged(i))
					{
					int iHP;
					int iPowerPerSegment;

					CalcRegen(pDevice, pShip, i, iTick, &iHP, &iPowerPerSegment);

					//	Repair armor

					if (iHP)
						pShip->RepairArmor(i, iHP);

					//	Add to consumption

					iTotalPower += iPowerPerSegment;
					}
				}
			}

		//	Store the power consumption on the device itself. We will keep on
		//	consuming this amount of power per tick until we update again.

		pDevice->SetData(iTotalPower);
		}

	if (retbConsumedItems)
		*retbConsumedItems = false;
	}

ALERROR CRepairerClass::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Bind design

	{
	//	Load events

	CItemType *pType = GetItemType();
	pType->InitCachedEvents(evtCount, CACHED_EVENTS, m_CachedEvents);

	return NOERROR;
	}

void CRepairerClass::OnInstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList)

//	OnInstall
//
//	Device installed

	{
	//	Identified when installed

	if (pSource->IsPlayer())
		GetItemType()->SetKnown();
	}
