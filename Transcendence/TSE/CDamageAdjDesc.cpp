//	CDamageAdjDesc.cpp
//
//	CDamageAdjDesc class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define DAMAGE_ADJ_ATTRIB					CONSTLIT("damageAdj")
#define HP_BONUS_ATTRIB						CONSTLIT("hpBonus")

ALERROR CDamageAdjDesc::Bind (SDesignLoadCtx &Ctx, const CDamageAdjDesc *pDefault)

//	Bind
//
//	Bind the design

	{
	Compute(pDefault);
	return NOERROR;
	}

void CDamageAdjDesc::Compute (const CDamageAdjDesc *pDefault)

//	Compute
//
//	Compute the m_iDamageAdj table based on the descriptors

	{
	int i;

	m_pDefault = pDefault;

	for (i = 0; i < damageCount; i++)
		{
		switch (m_Desc[i].dwAdjType)
			{
			case adjDefault:
				if (pDefault)
					m_iDamageAdj[i] = pDefault->GetAdj((DamageTypes)i);
				else
					m_iDamageAdj[i] = 100;
				break;

			case adjAbsolute:
				m_iDamageAdj[i] = (int)(DWORD)m_Desc[i].dwAdjValue;
				break;

			case adjRelative:
				if (pDefault)
					{
					int iValue = (int)(short)m_Desc[i].dwAdjValue;

					int iInc = iValue + (10000 / pDefault->GetAdj((DamageTypes)i)) - 100;
					if (iInc > -100)
						m_iDamageAdj[i] = 10000 / (100 + iInc);
					else
						m_iDamageAdj[i] = 10000000;
					}
				else
					m_iDamageAdj[i] = 100;
				break;

			default:
				ASSERT(false);
			}
		}
	}

void CDamageAdjDesc::GetAdjAndDefault (DamageTypes iDamageType, int *retiAdj, int *retiDefault) const

//	GetAdjAndDefault
//
//	Returns the adjustment and the default

	{
	if (retiAdj)
		*retiAdj = GetAdj(iDamageType);

	if (retiDefault)
		*retiDefault = (m_pDefault ? m_pDefault->GetAdj(iDamageType) : 100);
	}

int CDamageAdjDesc::GetHPBonus (DamageTypes iDamageType) const

//	GetHPBonus
//
//	Returns the HP bonus

	{
	if (m_Desc[iDamageType].dwAdjType == adjRelative)
		return (int)(short)(m_Desc[iDamageType].dwAdjValue);

	else if (m_Desc[iDamageType].dwAdjType == adjDefault)
		return 0;

	else
		{
		int iDefault;
		int iDamageAdj;

		GetAdjAndDefault(iDamageType, &iDamageAdj, &iDefault);
		if (iDamageAdj == 0)
			return -100;

		int iBonus = (int)((100.0 * (iDefault - iDamageAdj) / iDamageAdj) + 0.5);

		//	Prettify. Because of rounding-error, sometimes a bonus of +25 or -25 comes out as
		//	+24 or -24. This is because we store a damage adjustment not the bonus.

		if (((iBonus + 1) % 25) == 0)
			iBonus++;
		else if (((iBonus - 1) % 25) == 0)
			iBonus--;
		else if (iBonus == 48)
			iBonus = 50;

		return iBonus;
		}
	}

ALERROR CDamageAdjDesc::InitFromArray (int *pTable)

//	InitFromArray
//
//	Initializes from an array. The array must be at least damageCount entries
//	long.
//
//	In this path there is no need for Bind.

	{
	int i;

	for (i = 0; i < damageCount; i++)
		{
		m_Desc[i].dwAdjType = adjAbsolute;
		m_Desc[i].dwAdjValue = (DWORD)pTable[i];

		m_iDamageAdj[i] = pTable[i];
		}

	return NOERROR;
	}

ALERROR CDamageAdjDesc::InitFromDamageAdj (SDesignLoadCtx &Ctx, const CString &sAttrib, bool bNoDefault)

//	InitFromDamageAdj
//
//	Loads a damage adjustment descriptor as follows:
//
//	100,100,...
//	laser:100; kinetic:100; ...

	{
	ALERROR error;
	int i;

	//	We expect a list of damageAdj percent values, either with a damageType
	//	label or ordered by damageType.

	TArray<CString> DamageAdj;
	if (error = ParseDamageTypeList(sAttrib, &DamageAdj))
		{
		Ctx.sError = CONSTLIT("Invalid damageAdj definition.");
		return error;
		}

	//	Apply damage adj

	for (i = 0; i < damageCount; i++)
		{
		if (DamageAdj[i].IsBlank())
			{
			if (bNoDefault)
				{
				m_Desc[i].dwAdjType = adjAbsolute;
				m_Desc[i].dwAdjValue = 100;
				}
			else
				{
				m_Desc[i].dwAdjType = adjDefault;
				m_Desc[i].dwAdjValue = 0;
				}
			}
		else
			{
			int iValue = strToInt(DamageAdj[i], 0);
			if (iValue < 0)
				{
				Ctx.sError = CONSTLIT("damageAdj values cannot be negative.");
				return ERR_FAIL;
				}
			else if (iValue > 32767)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("damageAdj value is out of range: %d."), iValue);
				return ERR_FAIL;
				}

			m_Desc[i].dwAdjType = adjAbsolute;
			m_Desc[i].dwAdjValue = (WORD)iValue;

			//	If this table doesn't have a default, then it is the default table.
			//	Initialize m_iDamageAdj so we don't have to bind.

			if (bNoDefault)
				m_iDamageAdj[i] = iValue;
			}
		}

	//	Done

	return NOERROR;
	}

ALERROR CDamageAdjDesc::InitFromHPBonus (SDesignLoadCtx &Ctx, const CString &sAttrib)

//	InitFromHPBonus
//
//	Loads an HP bonus descriptor as follows:
//
//	+25, -25, ...
//	laser:+25; kinetic:-25; ...

	{
	ALERROR error;
	int i;

	//	We expect a list of percent adjustments

	TArray<CString> DamageAdj;
	if (error = ParseDamageTypeList(sAttrib, &DamageAdj))
		{
		Ctx.sError = CONSTLIT("Invalid hpBonus definition.");
		return error;
		}

	//	Apply damage adj

	for (i = 0; i < damageCount; i++)
		{
		//	An omitted value means default

		if (DamageAdj[i].IsBlank())
			{
			m_Desc[i].dwAdjType = adjDefault;
			m_Desc[i].dwAdjValue = 0;
			}

		//	A star means no damage

		else if (*DamageAdj[i].GetASCIIZPointer() == '*')
			{
			m_Desc[i].dwAdjType = adjAbsolute;
			m_Desc[i].dwAdjValue = 0;
			}

		//	Otherwise, this is a relative value

		else
			{
			bool bNull;
			int iValue;
			iValue = strToInt(DamageAdj[i], 0, &bNull);
			if (bNull)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Invalid hpBonus value: %s."), DamageAdj[i]);
				return ERR_FAIL;
				}

			if (iValue == 0)
				{
				m_Desc[i].dwAdjType = adjDefault;
				m_Desc[i].dwAdjValue = 0;
				}
			else
				{
				if (iValue > 32767 || iValue < -32768)
					{
					Ctx.sError = strPatternSubst(CONSTLIT("hpBonus is out of range: %d."), iValue);
					return ERR_FAIL;
					}

				m_Desc[i].dwAdjType = adjRelative;
				m_Desc[i].dwAdjValue = (WORD)(DWORD)iValue;
				}
			}
		}

	return NOERROR;
	}

ALERROR CDamageAdjDesc::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, bool bIsDefault)

//	InitFromXML
//
//	Initialize from XML.
//
//	If bIsDefault is TRUE then we don't need to bind.

	{
	ALERROR error;
	int i;
	CString sValue;

	if (pDesc->FindAttribute(HP_BONUS_ATTRIB, &sValue))
		{
		if (bIsDefault)
			{
			Ctx.sError = CONSTLIT("Default damageAdj tables must have absolute values.");
			return ERR_FAIL;
			}

		if (error = InitFromHPBonus(Ctx, sValue))
			return error;
		}
	else if (pDesc->FindAttribute(DAMAGE_ADJ_ATTRIB, &sValue))
		{
		if (error = InitFromDamageAdj(Ctx, sValue, bIsDefault))
			return error;
		}
	else
		{
		if (bIsDefault)
			{
			Ctx.sError = CONSTLIT("Default damageAdj tables must have absolute values.");
			return ERR_FAIL;
			}

		for (i = 0; i < damageCount; i++)
			{
			m_Desc[i].dwAdjType = adjDefault;
			m_Desc[i].dwAdjValue = 0;
			}
		}

	//	Done

	return NOERROR;
	}
