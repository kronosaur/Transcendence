//	ShieldStats.cpp
//
//	Generate statistics about shields

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define FIELD_HP							CONSTLIT("hp")
#define FIELD_REGEN							CONSTLIT("regen")

void GenerateShieldStats (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i;

	CString sUNID = pCmdLine->GetAttribute(CONSTLIT("unid"));
	DWORD dwUNID = strToInt(sUNID, 0, NULL);
	CItemType *pItem = Universe.FindItemType(dwUNID);
	if (pItem == NULL)
		{
		CItemCriteria Crit;
		CItem::InitCriteriaAll(&Crit);
		CItem Item = CItem::CreateItemByName(sUNID, Crit);
		pItem = Item.GetType();

		if (pItem == NULL)
			{
			printf("ERROR: Unknown item '%s'\n", sUNID.GetASCIIZPointer());
			return;
			}
		}

	if (pItem->GetCategory() != itemcatShields)
		{
		printf("ERROR: Item '%s' is not a shield generator\n", pItem->GetNounPhrase().GetASCIIZPointer());
		return;
		}

	bool bVerbose = pCmdLine->GetAttributeBool(CONSTLIT("verbose"));
	bool bEval = pCmdLine->GetAttributeBool(CONSTLIT("eval"));

	//	Get the stats for the shield

	Metric rHP = (Metric)pItem->GetDataFieldInteger(FIELD_HP);
	Metric rHPRegenPerTick = pItem->GetDataFieldInteger(FIELD_REGEN) / 1000.0;

	int iDamageAdj[damageCount];
	CString sDamageAdj = pItem->GetDataField(CONSTLIT("damageAdj"));
	char *pPos = sDamageAdj.GetASCIIZPointer();
	int iCount = 0;
	while (iCount < damageCount)
		{
		iDamageAdj[iCount] = strParseInt(pPos, 0, &pPos);
		if (*pPos != '\0')
			pPos++;
		iCount++;
		}

	//	Print header

	printf("%s\n\n", pItem->GetNounPhrase().GetASCIIZPointer());

	//	Loop over all weapons and sort them by level and then name

	CSymbolTable List(FALSE, TRUE);
	for (i = 0; i < Universe.GetItemTypeCount(); i++)
		{
		CItemType *pWeapon = Universe.GetItemType(i);
		if (pWeapon->GetCategory() != itemcatWeapon)
			continue;

		CString sLevel = (pWeapon->GetLevel() < 10 ? strPatternSubst(CONSTLIT("0%d"), pWeapon->GetLevel()) : strFromInt(pWeapon->GetLevel()));
		CString sSortName = strPatternSubst(CONSTLIT("%s%s"), sLevel, pWeapon->GetNounPhrase());
		List.AddEntry(sSortName, (CObject *)pWeapon);
		}

	//	Loop over sorted list and output data

	for (i = 0; i < List.GetCount(); i++)
		{
		CItemType *pWeapon = (CItemType *)List.GetValue(i);

		//	Get the data for the weapon

		int iFireDelay = pWeapon->GetDataFieldInteger(CONSTLIT("fireDelay"));
		Metric rAverageDamage = pWeapon->GetDataFieldInteger(CONSTLIT("averageDamage")) / 1000.0;
		int iDamageType = pWeapon->GetDataFieldInteger(CONSTLIT("damageType"));
		if (iDamageType < 0 || iDamageType >= damageCount)
			iDamageType = 0;

		//	Adjust damage for type

		rAverageDamage = rAverageDamage * (iDamageAdj[iDamageType] / 100.0);
		if (rAverageDamage < 1.0)
			rAverageDamage = 0.0;

		//	Calculate how many shots it would take to pierce through the shields

		char szBuffer[256];
		Metric rShotsToDeplete;
		Metric rRegenPerShot = rHPRegenPerTick * (Metric)iFireDelay;
		if (rRegenPerShot >= rAverageDamage)
			{
			rShotsToDeplete = 1000000.0;
			lstrcpy(szBuffer, "ineffective");
			}
		else
			{
			Metric rDrainPerShot = rAverageDamage - rRegenPerShot;
			rShotsToDeplete = rHP / rDrainPerShot;
			sprintf(szBuffer, "%.2f", rShotsToDeplete);
			}

		//	See if this weapon is overpowered or underpowered

		char szEval[256];
		if (bEval)
			{
			lstrcpy(szEval, "\t");
			if (pWeapon->GetLevel() < pItem->GetLevel())
				{
				if (rShotsToDeplete <= 10.0)
					lstrcpy(szEval, "\tOVERpowered");
				}
			else
				{
				if (rShotsToDeplete > 20.0)
					lstrcpy(szEval, "\tUNDERpowered");
				}
			}
		else
			lstrcpy(szEval, "");

		//	Print table

		if (bVerbose)
			{
			printf("%s\t%s\t%s\t(%d ticks; %.2f damage; %.2f regen/shot)%s\n",
					(LPSTR)strLevel(pWeapon->GetLevel()),
					pWeapon->GetNounPhrase().GetASCIIZPointer(),
					szBuffer,
					iFireDelay,
					rAverageDamage,
					rRegenPerShot,
					szEval);
			}
		else
			{
			printf("%s\t%s\t%s%s\n",
					(LPSTR)strLevel(pWeapon->GetLevel()),
					pWeapon->GetNounPhrase().GetASCIIZPointer(),
					szBuffer,
					szEval);
			}
		}
	}
