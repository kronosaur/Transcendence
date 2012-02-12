//	CNPAssetList.cpp
//
//	Implements various classes for units and resources

#include "Alchemy.h"
#include "NPEngine.h"

const int MAX_AMOUNT = 16777215;

int CNPAssetList::FindResource (DWORD dwUNID, CNPResource *retpRes) const

//	FindResource
//
//	Finds the given resource and returns the value and the index.
//	If the resource is not found we return -1 and an empty
//	resource.

	{
	int i;

	for (i = 0; i < m_iResourceCount; i++)
		{
		CNPResource Res = GetResourceByIndex(i);

		if (Res.GetClassUNID() == dwUNID)
			{
			*retpRes = Res;
			return i;
			}
		}

	//	Not found

	*retpRes = CNPResource(dwUNID, 0);
	return -1;
	}

int CNPAssetList::GetResource (DWORD dwUNID) const

//	GetResource
//
//	Returns the amount of the given resource

	{
	CNPResource Res;

	if (FindResource(dwUNID, &Res) != -1)
		return Res.GetAmount();
	else
		return 0;
	}

DWORD CNPAssetList::GetAssetUNID (int iIndex) const

//	GetAssetUNID
//
//	Returns the UNID of the given asset

	{
	if (GetAssetType(iIndex) == assetUnit)
		return GetUnitByIndex(iIndex).GetClassUNID();
	else
		return GetResourceByIndex(iIndex).GetClassUNID();
	}

ALERROR CNPAssetList::Load (const CDataPackStruct &Data)

//	Load
//
//	Loads from a data struct

	{
	int i;
	int iCount = Data.GetCount();

	int iResourceCount = Data.GetIntegerItem(0);
	CDataPackStruct *pAssets = Data.GetStructItem(1);
	for (i = 0; i < iResourceCount; i++)
		{
		CNPResource Res = CNPResource::CreateFromInt(pAssets->GetIntegerItem(i));
		ModifyResource(Res.GetClassUNID(), Res.GetAmount());
		}

	for (i = iResourceCount; i < pAssets->GetCount(); i++)
		{
		CNPUnit Unit = CNPUnit::CreateFromInt(pAssets->GetIntegerItem(i));
		AddUnit(Unit);
		}

	return NOERROR;
	}

int CNPAssetList::ModifyResource (DWORD dwUNID, int iChange)

//	ModifyResource
//
//	Adds or subtracts the given resource and returns
//	the new amount

	{
	CNPResource Res;

	int iIndex = FindResource(dwUNID, &Res);
	int iNewAmount = Res.Modify(iChange);
	if (iIndex == -1)
		{
		m_Assets.InsertElement(Res.GetAsInt(), 0, NULL);
		m_iResourceCount++;
		}
	else if (iNewAmount == 0)
		{
		m_Assets.RemoveElement(iIndex);
		m_iResourceCount--;
		}
	else
		m_Assets.ReplaceElement(iIndex, Res.GetAsInt());

	return iNewAmount;
	}

ALERROR CNPAssetList::Save (CDataPackStruct *pData) const

//	Save
//
//	Serializes to a data struct

	{
	pData->AppendInteger(m_iResourceCount);
	pData->AppendIntArray(m_Assets);
	return NOERROR;
	}

//	Resources & Units

CNPResource::CNPResource (DWORD dwUNID, int iAmount)

//	CNPResource constructor
//
//	|    8-bits   |      24-bits       |
//	------------------------------------
//	|    class    |       amount       |
//	|   ordinal   |                    |
//	|   (1-255)   |   (0-16,777,215)   |

	{
	ASSERT(GetUNIDType(dwUNID) == unidResourceClass);
	ASSERT(iAmount >= 0 && iAmount <= MAX_AMOUNT);

	m_dwData = ((GetUNIDOrdinal(dwUNID) & 0xff) << 24) | (DWORD)iAmount;
	}

CNPResource CNPResource::CreateFromInt (int iData)

//	CreateFromInt
//
//	Creates a CNPResource from an integer

	{
	CNPResource NewRes;
	NewRes.m_dwData = (DWORD)iData;
	return NewRes;
	}

int CNPResource::Modify (int iChange)

//	Modify
//
//	Modifies the amount

	{
	int iNewAmount;

	if (iChange > MAX_AMOUNT)
		iChange = MAX_AMOUNT;
	else if (iChange < -MAX_AMOUNT)
		iChange = -MAX_AMOUNT;

	iNewAmount = (int)GetAmount() + iChange;
	iNewAmount = max(0, min(MAX_AMOUNT, iNewAmount));

	m_dwData = (m_dwData & 0xff000000) + (DWORD)iNewAmount;

	return iNewAmount;
	}

CNPUnit::CNPUnit (DWORD dwUNID, int iStrength, int iExperience)

//	CNPUnit constructor
//
//	|    12-bits   | 4-bits  | 4-bits  | 4-bits  |1|  7-bits  |
//	-----------------------------------------------------------
//	|     class    | inf str | exprnce | resrvd  | |  resrvd  |
//	|    ordinal   | OR damg |         |         | |          |
//	|   (1-4095)   | (0-15)  |  (0-15) |         | |          |
//                                                ^
//                                                |
//                                                +- Disabled

	{
	ASSERT(GetUNIDType(dwUNID) == unidUnitClass);
	ASSERT(iStrength >= 0 && iStrength < 16);
	ASSERT(iExperience >= 0 && iExperience < 16);

	m_dwData = ((GetUNIDOrdinal(dwUNID) & 0xfff) << 20)
			| (((DWORD)iStrength) << 16)
			| (((DWORD)iExperience) << 12);
	}

CNPUnit CNPUnit::CreateFromInt (int iData)

//	CreateFromInt
//
//	Creates a CNPUnit from an integer

	{
	CNPUnit NewUnit;
	NewUnit.m_dwData = (DWORD)iData;
	return NewUnit;
	}

