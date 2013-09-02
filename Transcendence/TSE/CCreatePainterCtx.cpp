//	CCreatePainterCtx.cpp
//
//	CCreatePainterCtx class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define FIELD_DAMAGE_HP					CONSTLIT("damageHP")
#define FIELD_SPEED						CONSTLIT("speed")

CCreatePainterCtx::~CCreatePainterCtx (void)

//	CCreatePainterCtx destructor

	{
	if (m_pData)
		m_pData->Discard(&g_pUniverse->GetCC());
	}

ICCItem *CCreatePainterCtx::GetData (void)

//	GetData
//
//	Generate data block for create painter

	{
	if (m_pData)
		return m_pData;

	//	Initialize

	CCodeChain &CC = g_pUniverse->GetCC();
	m_pData = CC.CreateSymbolTable();
	CCSymbolTable *pTable = (CCSymbolTable *)m_pData;

	//	Set values depending on what we have in context

	if (m_pWeaponFireDesc)
		SetWeaponFireDescData(CC, pTable, m_pWeaponFireDesc);

	//	Done

	return m_pData;
	}

void CCreatePainterCtx::SetWeaponFireDescData (CCodeChain &CC, CCSymbolTable *pTable, CWeaponFireDesc *pDesc)

//	SetWeaponFireDescData
//
//	Sets the data from a weapon fire desc to the data block.

	{
	pTable->SetIntegerValue(CC, FIELD_DAMAGE_HP, (int)(pDesc->GetAveDamage() + 0.5));
	pTable->SetIntegerValue(CC, FIELD_SPEED, (int)((100.0 * pDesc->GetAveInitialSpeed() / LIGHT_SPEED) + 0.5));
	}
