//  ItemBenchmarks.h
//
//  Classes to evaluate weapons and armor.
//  Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CWeaponBenchmarkCtx
    {
    public:
        struct SStats
            {
            int iAverageTime;               //  Average ticks to destroy armor at level
            CItemType *pBestArmor;          //  Armor that we are best against
            int iBestTime;                  //  Time to destroy best armor
            CItemType *pWorstArmor;         //  Armor that we are worst against
            int iWorstTime;                 //  Time to destroy worst armor
            int iInvulnerableCount;         //  Number of invulnerable armors

            TSortMap<CItemType *, int> TimeTable;
            };

        bool GetStats (CItemType *pWeaponItem, SStats &retStats);

    private:
        bool InitArmorTable (void);

        TSortMap<int, TArray<CItemType *>> m_ArmorTable;
    };