//	CWeaponBenchmarkCtx.cpp
//
//	CWeaponBenchmarkCtx class
//  Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define PROPERTY_DAMAGE		        		CONSTLIT("damage")
#define PROPERTY_DAMAGE_TYPE        		CONSTLIT("damageType")

bool CWeaponBenchmarkCtx::GetStats (CItemType *pWeaponItem, SStats &retStats)

//  GetStats
//
//  Returns stats for the given weapon item (this can be either a weapon or a
//  missile). We return TRUE if successful.

    {
    int i;
    CCodeChain &CC = g_pUniverse->GetCC();

    retStats.TimeTable.DeleteAll();

    if (!InitArmorTable())
        return false;

    //  Compute some stats about the weapon

    CItem WeaponItem(pWeaponItem, 1);
    CItemCtx Ctx(WeaponItem);
    CCodeChainCtx TLispCtx;

    Metric rDamage180 = WeaponItem.GetItemPropertyDouble(TLispCtx, Ctx, PROPERTY_DAMAGE);
    DamageTypes iDamageType = (DamageTypes)WeaponItem.GetItemPropertyInteger(TLispCtx, Ctx, PROPERTY_DAMAGE_TYPE);
    if (iDamageType == -1)
        return false;

    //  Track some averages and totals

    int iTotalTime = 0;
    int iArmorCount = 0;
    retStats.pBestArmor = NULL;
    retStats.iBestTime = 0;
    retStats.pWorstArmor = NULL;
    retStats.iWorstTime = 0;
    retStats.iInvulnerableCount = 0;

    //  For each armor of the same level as this weapon, figure out how many 
    //  ticks it would take to destroy.

    TArray<CItemType *> &Armors = *m_ArmorTable.SetAt(pWeaponItem->GetLevel());
    for (i = 0; i < Armors.GetCount(); i++)
        {
        CItem ArmorItem(Armors[i], 1);
        CItemCtx ArmorCtx(ArmorItem);
        CArmorClass *pArmorClass = ArmorCtx.GetArmorClass();
        if (pArmorClass == NULL)
            continue;

        Metric rHP = pArmorClass->GetMaxHP(ArmorCtx, true);
        Metric rDamageAdj = pArmorClass->GetDamageAdj(ArmorCtx, iDamageType);

        //  Compute number of ticks to destroy the armor

        Metric rAdjDamage180 = rDamage180 * rDamageAdj / 100.0;
        int iTicks = (rAdjDamage180 > 0.0 ? (int)((180.0 * rHP / rAdjDamage180) + 0.5) : -1);

        //  Add to our table

        retStats.TimeTable.Insert(Armors[i], iTicks);

        //  Compute totals and averages

        if (iTicks > 0)
            {
            iTotalTime += iTicks;
            iArmorCount++;

            if (retStats.pBestArmor == NULL || iTicks < retStats.iBestTime)
                {
                retStats.pBestArmor = Armors[i];
                retStats.iBestTime = iTicks;
                }

            if (retStats.pWorstArmor == NULL 
                    || (retStats.iWorstTime > 0 && iTicks > retStats.iWorstTime))
                {
                retStats.pWorstArmor = Armors[i];
                retStats.iWorstTime = iTicks;
                }
            }
        else
            {
            retStats.iInvulnerableCount++;
            retStats.pWorstArmor = Armors[i];
            retStats.iWorstTime = -1;
            }
        }

    //  Compute average

    if (iArmorCount > 0)
        retStats.iAverageTime = (int)(((Metric)iTotalTime / (Metric)iArmorCount) + 0.5);
    else
        retStats.iAverageTime = 0;

    return true;
    }

bool CWeaponBenchmarkCtx::InitArmorTable (void)

//  InitArmorTable
//
//  Initializes the armor table, which lists armor by level.

    {
    int i;

    //  If we've already initialized the table, then nothing to do.

    if (m_ArmorTable.GetCount() != 0)
        return true;

    //  Create table

    for (i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
        {
        CItemType *pType = g_pUniverse->GetItemType(i);

        //  Skip non-armor

        if (!pType->IsArmor()
                || pType->IsVirtual())
            continue;

        //  Add to our list, indexed by level.

        TArray<CItemType *> *pTable = m_ArmorTable.SetAt(pType->GetLevel());
        pTable->Insert(pType);
        }

    //  Done

    return true;
    }
