//	CDisplayAttributeDefinitions.cpp
//
//	CDisplayAttributeDefinitions class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define ITEM_ATTRIBUTE_TAG						CONSTLIT("ItemAttribute")

#define CRITERIA_ATTRIB							CONSTLIT("criteria")
#define LABEL_ATTRIB							CONSTLIT("label")
#define LABEL_TYPE_ATTRIB						CONSTLIT("labelType")

#define TYPE_POSITIVE							CONSTLIT("advantage")
#define TYPE_NEGATIVE							CONSTLIT("disadvantage")
#define TYPE_NEUTRAL							CONSTLIT("neutral")

void CDisplayAttributeDefinitions::AccumulateAttributes (const CItem &Item, TArray<SDisplayAttribute> *retList) const

//	AccumulateAttributes
//
//	Adds display attributes possessed by Item to retList.

	{
	int i;

	for (i = 0; i < m_Definitions.GetCount(); i++)
		{
		if (Item.MatchesCriteria(m_Definitions[i].Criteria))
			retList->Insert(SDisplayAttribute(m_Definitions[i].iType, m_Definitions[i].sText));
		}
	}

void CDisplayAttributeDefinitions::Append (const CDisplayAttributeDefinitions &Attribs)

//	Append
//
//	Appends definitions.

	{
	int i;

	if (Attribs.m_Definitions.GetCount() == 0)
		return;

	int iDest = m_Definitions.GetCount();
	m_Definitions.InsertEmpty(Attribs.m_Definitions.GetCount());

	for (i = 0; i < Attribs.m_Definitions.GetCount(); i++)
		m_Definitions[iDest++] = Attribs.m_Definitions[i];
	}

ALERROR CDisplayAttributeDefinitions::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitFromXML
//
//	Initialize

	{
	int i;

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pDef = pDesc->GetContentElement(i);
		if (strEquals(pDef->GetTag(), ITEM_ATTRIBUTE_TAG))
			{
			SEntry *pEntry = m_Definitions.Insert();
			pEntry->sText = pDef->GetAttribute(LABEL_ATTRIB);

			//	Criteria

			CString sCriteria;
			if (pDef->FindAttribute(CRITERIA_ATTRIB, &sCriteria))
				CItem::ParseCriteria(sCriteria, &pEntry->Criteria);
			else
				CItem::InitCriteriaAll(&pEntry->Criteria);

			//	Type

			CString sType;
			if (pDef->FindAttribute(LABEL_TYPE_ATTRIB, &sType))
				{
				if (strEquals(sType, TYPE_POSITIVE))
					pEntry->iType = attribPositive;
				else if (strEquals(sType, TYPE_NEGATIVE))
					pEntry->iType = attribNegative;
				else if (strEquals(sType, TYPE_NEUTRAL))
					pEntry->iType = attribNeutral;
				else
					{
					Ctx.sError = strPatternSubst(CONSTLIT("Invalid label type: %s."), sType);
					return ERR_FAIL;
					}
				}
			else
				pEntry->iType = attribNeutral;
			}
		else
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unknown display attribute definition: %s."), pDef->GetTag());
			return ERR_FAIL;
			}
		}

	return NOERROR;
	}
