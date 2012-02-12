//	CRepairerClass.cpp
//
//	CRepairerClass class

#include "PreComp.h"


#define POWER_USE_ATTRIB			CONSTLIT("powerUse")
#define REPAIR_CYCLE_ATTRIB			CONSTLIT("repairCycle")
#define REGEN_ATTRIB				CONSTLIT("regen")

#define REPAIR_CYCLE_TIME			10

CRepairerClass::CRepairerClass (void) : CDeviceClass(NULL)
	{
	}

int CRepairerClass::CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	CalcPowerUsed
//
//	Computes the amount of power used by this device each tick

	{
	int i;

	//	Doesn't work if not enabled

	if (!pDevice->IsEnabled())
		return 0;

	int iPower = 0;

	//	Get a ship object and calculate based on armor

	CShip *pShip = pSource->AsShip();
	if (pShip)
		{
		for (i = 0; i < pShip->GetArmorSectionCount(); i++)
			{
			if (pShip->IsArmorDamaged(i))
				{
				//	Figure out the tech of this armor

				CArmorClass *pArmorClass = pShip->GetArmorSection(i)->GetClass();
				int iArmorTech = pArmorClass->GetRepairTech();

				//	If we repair armor of this tech level, then we consume power

				if (iArmorTech - 1 < m_Repair.GetCount()
						&& m_Repair[iArmorTech - 1].GetHPPerEra() > 0)
					iPower += m_iPowerUse;
				}
			}
		}

	return iPower;
	}

ALERROR CRepairerClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice)

//	CreateFromXML
//
//	Creates from an XML element

	{
	int i;
	CRepairerClass *pDevice;

	pDevice = new CRepairerClass;
	if (pDevice == NULL)
		return ERR_MEMORY;

	pDevice->InitDeviceFromXML(Ctx, pDesc, pType);

	pDevice->m_iPowerUse = pDesc->GetAttributeInteger(POWER_USE_ATTRIB);

	//	Load repair attribute

	CString sList;
	bool bRegen;
	if (pDesc->FindAttribute(REGEN_ATTRIB, &sList))
		bRegen = true;
	else if (pDesc->FindAttribute(REPAIR_CYCLE_ATTRIB, &sList))
		bRegen = false;
	else
		{
		Ctx.sError = CONSTLIT("regen attribute expected");
		return ERR_FAIL;
		}

	//	Parse the list

	TArray<CString> List;
	ParseStringList(sList, 0, &List);
	for (i = 0; i < List.GetCount(); i++)
		{
		CRegenDesc *pRegen = pDevice->m_Repair.Insert();
		if (bRegen)
			pRegen->InitFromRegenString(Ctx, List[i]);
		else
			pRegen->InitFromRepairRateString(Ctx, List[i]);
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
	if (pShip)
		{
		if ((iTick % REPAIR_CYCLE_TIME) == 0
				&& pDevice->IsEnabled() 
				&& !pDevice->IsDamaged()
				&& !pDevice->IsDisrupted())
			{
			for (i = 0; i < pShip->GetArmorSectionCount(); i++)
				{
				if (pShip->IsArmorDamaged(i))
					{
					//	Figure out the tech of this armor

					CArmorClass *pArmorClass = pShip->GetArmorSection(i)->GetClass();
					int iArmorTech = pArmorClass->GetRepairTech();

					//	Figure out the HP repaired

					if (iArmorTech - 1 < m_Repair.GetCount())
						{
						int iHP;
						if (iHP = m_Repair[iArmorTech - 1].GetRegen(iTick))
							pShip->RepairArmor(i, iHP);
						}
					}
				}
			}
		}

	if (retbConsumedItems)
		*retbConsumedItems = false;
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
