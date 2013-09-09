//	CItemEnhancementStack.cpp
//
//	CItemEnhancementStack class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

void CItemEnhancementStack::ApplySpecialDamage (DamageDesc *pDamage) const

//	ApplySpecialDamage
//
//	Adds special damage types to the given damage descriptor.

	{
	int i;

	for (i = 0; i < m_Stack.GetCount(); i++)
		{
		int iLevel;
		SpecialDamageTypes iSpecial;
		if ((iSpecial = m_Stack[i].GetSpecialDamage(&iLevel)) != specialNone)
			{
			if (iLevel > pDamage->GetSpecialDamage(iSpecial))
				pDamage->SetSpecialDamage(iSpecial, iLevel);
			}
		}
	}

int CItemEnhancementStack::CalcActivateDelay (CItemCtx &DeviceCtx) const

//	CalcActivateDelay
//
//	Calculates the activation delay (in ticks) for the given device if we apply
//	this enhancement stack.

	{
	int i;

	CInstalledDevice *pDevice = DeviceCtx.GetDevice();
	if (pDevice == NULL)
		return 0;

	//	Get the raw activation delay. NOTE: This DOES NOT include
	//	any enhancements on the item.

	Metric rDelay = pDevice->GetClass()->GetActivateDelay(pDevice, DeviceCtx.GetSource());

	//	Apply enhancements (including on the item itself)

	for (i = 0; i < m_Stack.GetCount(); i++)
		{
		int iMin, iMax;
		int iAdj = m_Stack[i].GetActivateRateAdj(&iMin, &iMax);
		if (iAdj != 100)
			{
			rDelay = iAdj * rDelay / 100.0;
			if (rDelay < (Metric)iMin)
				rDelay = (Metric)iMin;
			else if (iMax > 0 && rDelay > (Metric)iMax)
				rDelay = (Metric)iMax;
			}
		}

	return (int)(rDelay + 0.5);
	}

void CItemEnhancementStack::CalcCache (void) const

//	CalcCache
//
//	Computes accumulated values

	{
	int i;

	m_iBonus = 0;
	m_Damage = DamageDesc(damageGeneric, DiceRange());

	for (i = 0; i < m_Stack.GetCount(); i++)
		{
		m_iBonus += m_Stack[i].GetHPBonus();

		int iLevel;
		SpecialDamageTypes iSpecial;
		if ((iSpecial = m_Stack[i].GetSpecialDamage(&iLevel)) != specialNone)
			m_Damage.SetSpecialDamage(iSpecial, iLevel);
		}

	//	Done

	m_bCacheValid = true;
	}

void CItemEnhancementStack::Delete (void)

//	Delete
//
//	Delete reference

	{
	if (--m_dwRefCount <= 0)
		delete this;
	}

int CItemEnhancementStack::GetBonus (void) const

//	GetBonus
//
//	Returns the accumulated bonus for all enhancements on the stack.

	{
	if (!m_bCacheValid)
		CalcCache();

	return m_iBonus;
	}

const DamageDesc &CItemEnhancementStack::GetDamage (void) const

//	GetDamage
//
//	Returns special damage.

	{
	if (!m_bCacheValid)
		CalcCache();

	return m_Damage;
	}

void CItemEnhancementStack::Insert (const CItemEnhancement &Mods)

//	Insert
//
//	Appends the given enhancement to the stack.

	{
	m_Stack.Insert(Mods);
	}

void CItemEnhancementStack::InsertActivateAdj (int iAdj, int iMin, int iMax)

//	InsertActivateAdj
//
//	Appends an activate adjustment enhancement.

	{
	m_Stack.InsertEmpty();
	m_Stack[m_Stack.GetCount() - 1].SetModSpeed(iAdj, iMin, iMax);
	}

void CItemEnhancementStack::InsertHPBonus (int iBonus)

//	InsertHPBonus
//
//	Appends an HP bonus enhancement.

	{
	m_Stack.InsertEmpty();
	m_Stack[m_Stack.GetCount() - 1].SetModBonus(iBonus);
	}

void CItemEnhancementStack::ReadFromStream (SLoadCtx &Ctx, CItemEnhancementStack **retpStack)

//	ReadFromStream
//
//	Reads from stream

	{
	int i;

	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad == 0)
		{
		*retpStack = NULL;
		return;
		}

	CItemEnhancementStack *pStack = new CItemEnhancementStack;
	pStack->m_Stack.InsertEmpty(dwLoad);

	for (i = 0; i < pStack->m_Stack.GetCount(); i++)
		pStack->m_Stack[i].ReadFromStream(Ctx);

	*retpStack = pStack;
	}

void CItemEnhancementStack::WriteToStream (CItemEnhancementStack *pStack, IWriteStream *pStream)

//	WriteToStream
//
//	Writes to stream
//
//	DWORD			Number of items in stack
//		CItemEnhancement

	{
	int i;

	if (pStack == NULL)
		{
		DWORD dwSave = 0;
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}
	else
		{
		DWORD dwSave = pStack->m_Stack.GetCount();
		pStream->Write((char *)&dwSave, sizeof(DWORD));
	
		for (i = 0; i < pStack->m_Stack.GetCount(); i++)
			pStack->m_Stack[i].WriteToStream(pStream);
		}
	}
