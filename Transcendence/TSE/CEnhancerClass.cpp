//	CEnhancerClass.cpp
//
//	CEnhancerClass class

#include "PreComp.h"

#define DAMAGE_ADJ_ATTRIB						CONSTLIT("damageAdj")
#define ENHANCEMENT_TYPE_ATTRIB					CONSTLIT("enhancementType")
#define HP_BONUS_ATTRIB							CONSTLIT("hpBonus")
#define POWER_USE_ATTRIB						CONSTLIT("powerUse")
#define ACTIVATE_ADJ_ATTRIB						CONSTLIT("activateAdj")
#define MIN_ACTIVATE_DELAY_ATTRIB				CONSTLIT("minActivateDelay")
#define MAX_ACTIVATE_DELAY_ATTRIB				CONSTLIT("maxActivateDelay")
#define CRITERIA_ATTRIB							CONSTLIT("criteria")

CEnhancerClass::CEnhancerClass (void) : CDeviceClass(NULL)

//	CEnhancerClass constructor

	{
	}

int CEnhancerClass::CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	CalcPowerUsed
//
//	Returns the amount of power used per tick

	{
	if (!pDevice->IsEnabled())
		return 0;

	return m_iPowerUse;
	}

ALERROR CEnhancerClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice)

//	CreateFromXML
//
//	Load device data from XML

	{
	ALERROR error;
	CEnhancerClass *pDevice;

	pDevice = new CEnhancerClass;
	if (pDevice == NULL)
		return ERR_MEMORY;

	if (error = pDevice->InitDeviceFromXML(Ctx, pDesc, pType))
		return error;

	//	The old style is to have an array of damage adj; the new way is to just
	//	have a single damage adj and a criteria

	int iDamageAdjCount;
	LoadDamageAdj(pDesc, DAMAGE_ADJ_ATTRIB, pDevice->m_iDamageAdjArray, &iDamageAdjCount);
	if (iDamageAdjCount)
		pDevice->m_bUseArray = true;
	else
		{
		pDevice->m_iDamageAdj = pDesc->GetAttributeInteger(HP_BONUS_ATTRIB);
		pDevice->m_bUseArray = false;
		}

	pDevice->m_iPowerUse = pDesc->GetAttributeInteger(POWER_USE_ATTRIB);
	pDevice->m_sEnhancementType = pDesc->GetAttribute(ENHANCEMENT_TYPE_ATTRIB);
	pDevice->m_iActivateAdj = pDesc->GetAttributeIntegerBounded(ACTIVATE_ADJ_ATTRIB, 1, -1, 100);
	pDevice->m_iMinActivateDelay = pDesc->GetAttributeIntegerBounded(MIN_ACTIVATE_DELAY_ATTRIB, 0, -1, 0);
	pDevice->m_iMaxActivateDelay = pDesc->GetAttributeIntegerBounded(MAX_ACTIVATE_DELAY_ATTRIB, 0, -1, 0);

	//	Load the item criteria

	CString sCriteria;
	if (!pDesc->FindAttribute(CRITERIA_ATTRIB, &sCriteria))
		sCriteria = CONSTLIT("w");

	CItem::ParseCriteria(sCriteria, &pDevice->m_Criteria);

	//	Done

	*retpDevice = pDevice;

	return NOERROR;
	}

bool CEnhancerClass::GetDeviceEnhancementDesc (CInstalledDevice *pDevice, CSpaceObject *pSource, CInstalledDevice *pWeapon, SDeviceEnhancementDesc *retDesc)

//	GetDeviceEnhancementDesc
//
//	Returns SDeviceEnhancementDesc structure

	{
	if (!pDevice->IsEnabled() 
			|| pDevice->IsDamaged())
		return false;

	//	If the target item does not match criteria, then no enhancement

	if (!pSource->GetItemForDevice(pWeapon).MatchesCriteria(m_Criteria))
		return false;

	//	Type

	retDesc->sID = m_sEnhancementType;

	//	Get the damage bonus

	if (m_bUseArray)
		{
		//	NOTE: We enter 0 for a variant because we want to hard-code to 
		//	the first variant (otherwise we would default to the selected
		//	variant)

		int iType = pWeapon->GetDamageType(0);
		retDesc->iBonus = (iType != -1 ? m_iDamageAdjArray[iType] : 0);
		}
	else
		retDesc->iBonus = m_iDamageAdj;

	//	Get the activate adj bonus

	retDesc->iActivateAdj = m_iActivateAdj;
	retDesc->iMinActivateDelay = m_iMinActivateDelay;
	retDesc->iMaxActivateDelay = m_iMaxActivateDelay;

	return true;
	}

bool CEnhancerClass::OnAccumulateEnhancements (CItemCtx &Device, CInstalledDevice *pTarget, TArray<CString> &EnhancementIDs, CItemEnhancementStack *pEnhancements)

//	OnAccumulateEnhancements
//
//	Enhances pTarget

	{
	CInstalledDevice *pDevice = Device.GetDevice();
	CSpaceObject *pSource = Device.GetSource();

	//	If disabled or damaged then we do not enhance anything

	if (pDevice 
			&& (!pDevice->IsEnabled() || pDevice->IsDamaged()))
		return false;

	//	If the target item does not match our criteria, then no enhancement

	if (pSource 
			&& pTarget
			&& !pSource->GetItemForDevice(pTarget).MatchesCriteria(m_Criteria))
		return false;

	//	If this enhancement type has already been applied, then nothing

	if (!m_sEnhancementType.IsBlank()
			&& EnhancementIDs.Find(m_sEnhancementType))
		return false;

	//	Keep track of enhancements.

	bool bEnhanced = false;

	//	Add HP bonus enhancements

	int iBonus;
	if (m_bUseArray)
		{
		//	NOTE: We enter 0 for a variant because we want to hard-code to 
		//	the first variant (otherwise we would default to the selected
		//	variant)

		int iType = pTarget->GetDamageType(0);
		iBonus = (iType != -1 ? m_iDamageAdjArray[iType] : 0);
		}
	else
		iBonus = m_iDamageAdj;

	if (iBonus != 0)
		{
		pEnhancements->InsertHPBonus(iBonus);
		bEnhanced = true;
		}

	//	Add speed enhancements

	if (m_iActivateAdj != 100)
		{
		pEnhancements->InsertActivateAdj(m_iActivateAdj, m_iMinActivateDelay, m_iMaxActivateDelay);
		bEnhanced = true;
		}

	//	Remember that we added this enhancement type

	if (!m_sEnhancementType.IsBlank())
		EnhancementIDs.Insert(m_sEnhancementType);

	//	Done

	return bEnhanced;
	}
