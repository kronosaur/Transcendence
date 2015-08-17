//	CItemTypeTable.cpp
//
//	CItemTypeTable class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

static char *g_szTypeCode[] =
	{
	"",
	"Armor",
	"Weapon",
	"Missile",
	"Shield",
	"Device",
	"Useful",
	"Fuel",
	"Misc",
	};

static char *g_szFreqCode[] =
	{
	"",	"C", "UC", "R", "VR", "NR",
	};

bool CItemTypeTable::Filter (const CString &sCriteria)

//	Filter
//
//	Filters the current table to include only the given criteria. Returns TRUE
//	if the table is non-empty.

	{
	int i;

	//	Short circuit

	if (sCriteria.IsBlank())
		return (m_bAll || (m_Table.GetCount() > 0));

	//	Compute the criteria

	CItemCriteria Crit;
	CItem::ParseCriteria(sCriteria, &Crit);

	//	Loop over all the current items in the table and generate a new table
	//	with the selection.

	TArray<CItemType *> NewTable;
	for (i = 0; i < GetCount(); i++)
		{
		CItemType *pType = GetItemType(i);
		CItem Item(pType, 1);

		if (!Item.MatchesCriteria(Crit))
			continue;

		NewTable.Insert(pType);
		}

	//	Replace the table

	m_Table = NewTable;
	m_bAll = false;

	//	Done

	return (m_Table.GetCount() > 0);
	}

int CItemTypeTable::GetCount (void) const

//	GetCount
//
//	Returns the number of items in the table.

	{
	if (m_bAll)
		return g_pUniverse->GetItemTypeCount();
	else
		return m_Table.GetCount();
	}

int CItemTypeTable::GetItemFreqIndex (CItemType *pType) const
	{
	int iFreq = pType->GetFrequency();
	if (iFreq == ftCommon)
		return 1;
	else if (iFreq == ftUncommon)
		return 2;
	else if (iFreq == ftRare)
		return 3;
	else if (iFreq == ftVeryRare)
		return 4;
	else
		return 5;
	}

CItemType *CItemTypeTable::GetItemType (int iIndex) const

//	GetItemType
//
//	Returns the nth item type.

	{
	if (m_bAll)
		return g_pUniverse->GetItemType(iIndex);
	else
		return m_Table[iIndex];
	}

int CItemTypeTable::GetItemTypeIndex (CItemType *pType) const
	{
	switch (pType->GetCategory())
		{
		case itemcatArmor:
			return 1;

		case itemcatLauncher:
		case itemcatWeapon:
			return 2;

		case itemcatMissile:
			return 3;

		case itemcatShields:
			return 4;

		case itemcatMiscDevice:
		case itemcatCargoHold:
		case itemcatReactor:
		case itemcatDrive:
			return 5;

		case itemcatUseful:
			return 6;

		case itemcatFuel:
			return 7;

		default:
			return 8;
		}
	}

void CItemTypeTable::Sort (void)

//	Sort
//
//	Sort the list.

	{
	int i;

	//	Loop over all items that match and add them to
	//	a sorted table.

	TSortMap<CString, CItemType *> Sorted;
	for (i = 0; i < GetCount(); i++)
		{
		CItemType *pType = GetItemType(i);

		//	Add with sort key

		CString sSort = strPatternSubst(CONSTLIT("%02d%s%02d%s"),
				pType->GetLevel(),
				CString(g_szTypeCode[GetItemTypeIndex(pType)]), 
				GetItemFreqIndex(pType), 
				pType->GetNounPhrase());

		Sorted.Insert(sSort, pType);
		}

	//	Add back in sorted order.

	m_Table.DeleteAll();
	m_Table.InsertEmpty(Sorted.GetCount());
	for (i = 0; i < Sorted.GetCount(); i++)
		m_Table[i] = Sorted[i];

	m_bAll = false;
	}
