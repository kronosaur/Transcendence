//	CNPUnitClass.cpp
//
//	Implements CNPUnitClass class

#include "Alchemy.h"
#include "NPEngine.h"

CNPUnitClass::CNPUnitClass (void) : CObject(NULL),
		m_dwUNID(NullUNID),
		m_sName(CONSTLIT("Unknown")),
		m_sClassName(CONSTLIT("Unknown")),
		m_iHull(-1),
		m_iFirepower(-1),
		m_iArmor(-1),
		m_iStealth(-1),
		m_iScanners(-1),
		m_iHyperdrive(-1),
		m_iBuildTech(techNone),
		m_iBuildTime(-1)

//	CNPUnitClass constructor

	{
	}

ALERROR CNPUnitClass::Load (const CDataPackStruct &Data)

//	Load
//
//	Loads from data

	{
	m_dwUNID = Data.GetIntegerItem(0);
	m_sName = Data.GetStringItem(1);
	m_sClassName = Data.GetStringItem(2);

	CDataPackStruct *pStats = Data.GetStructItem(3);
	m_iHull = pStats->GetIntegerItem(0);
	m_iFirepower = pStats->GetIntegerItem(1);
	m_iFireRate = pStats->GetIntegerItem(2);
	m_iArmor = pStats->GetIntegerItem(3);
	m_iStealth = pStats->GetIntegerItem(4);
	m_iScanners = pStats->GetIntegerItem(5);
	m_iHyperdrive = pStats->GetIntegerItem(6);

	CDataPackStruct *pBuild = Data.GetStructItem(4);
	m_iBuildTech = (NPTechLevels)pBuild->GetIntegerItem(0);
	m_iBuildTime = pBuild->GetIntegerItem(1);
	m_BuildRes.Load(*pBuild->GetStructItem(2));

	CDataPackStruct *pTraits = Data.GetStructItem(5);
	for (int i = 0; i < pTraits->GetCount(); i++)
		m_Traits.AddElement(pTraits->GetIntegerItem(i));

	return NOERROR;
	}

ALERROR CNPUnitClass::LoadFromXML (CXMLElement *pElement, CSymbolTable &Symbols)

//	LoadFromXML
//
//	Loads from an XML element

	{
	m_dwUNID = MakeUNID(unidUnitClass, pElement->GetAttributeInteger(CONSTLIT("UNID")));
	m_sName = pElement->GetAttribute(CONSTLIT("Name"));

	for (int i = 0; i < pElement->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pElement->GetContentElement(i);

		if (strCompare(pItem->GetTag(), CONSTLIT("General")) == 0)
			{
			m_sClassName = pItem->GetAttribute(CONSTLIT("ClassName"));
			m_iHull = pItem->GetAttributeInteger(CONSTLIT("Hull"));
			m_iFirepower = pItem->GetAttributeInteger(CONSTLIT("Firepower"));
			m_iFireRate = pItem->GetAttributeInteger(CONSTLIT("FireRate"));
			m_iArmor = pItem->GetAttributeInteger(CONSTLIT("Armor"));
			m_iStealth = pItem->GetAttributeInteger(CONSTLIT("Stealth"));
			m_iScanners = pItem->GetAttributeInteger(CONSTLIT("Scanners"));
			m_iHyperdrive = pItem->GetAttributeInteger(CONSTLIT("Hyperdrive"));
			m_iBuildTech = (NPTechLevels)GetSymbolicAttribute(Symbols, pItem, CONSTLIT("BuildTech"));
			m_iBuildTime = pItem->GetAttributeInteger(CONSTLIT("BuildTime"));
			}
		else if (strCompare(pItem->GetTag(), CONSTLIT("Trait")) == 0)
			{
			int iTrait = GetSymbolicAttribute(Symbols, pItem, CONSTLIT("Trait"));
			m_Traits.AddElement(iTrait);
			}
		else if (strCompare(pItem->GetTag(), CONSTLIT("BuildConsume")) == 0)
			{
			m_BuildRes.ModifyResource(GetSymbolicAttribute(Symbols, pItem, CONSTLIT("Res")),
					pItem->GetAttributeInteger(CONSTLIT("Amount")));
			}
		}

	return NOERROR;
	}

ALERROR CNPUnitClass::Save (CDataPackStruct *pData) const

//	Save
//
//	Saves to data

	{
	CDataPackStruct *pStruct;

	pData->AppendInteger(m_dwUNID);
	pData->AppendString(m_sName);
	pData->AppendString(m_sClassName);

	//	stats struct
	pData->AppendNewStruct(&pStruct);
	pStruct->AppendInteger(m_iHull);
	pStruct->AppendInteger(m_iFirepower);
	pStruct->AppendInteger(m_iFireRate);
	pStruct->AppendInteger(m_iArmor);
	pStruct->AppendInteger(m_iStealth);
	pStruct->AppendInteger(m_iScanners);
	pStruct->AppendInteger(m_iHyperdrive);

	//	build struct
	pData->AppendNewStruct(&pStruct);
	pStruct->AppendInteger(m_iBuildTech);
	pStruct->AppendInteger(m_iBuildTime);

	CDataPackStruct *pRes;
	pStruct->AppendNewStruct(&pRes);
	m_BuildRes.Save(pRes);

	//	traits
	CIntArray Traits;
	EnumTraits(&Traits);
	pData->AppendIntArray(Traits);

	return NOERROR;
	}
