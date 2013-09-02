//	CShipInteriorDesc.cpp
//
//	CShipInteriorDesc class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define COMPARTMENT_TAG							CONSTLIT("Compartment")

#define HIT_POINTS_ATTRIB						CONSTLIT("hitPoints")
#define NAME_ATTRIB								CONSTLIT("name")
#define POS_X_ATTRIB							CONSTLIT("posX")
#define POS_Y_ATTRIB							CONSTLIT("posY")
#define SIZE_X_ATTRIB							CONSTLIT("sizeX")
#define SIZE_Y_ATTRIB							CONSTLIT("sizeY")
#define TYPE_ATTRIB								CONSTLIT("type")

#define TYPE_CARGO								CONSTLIT("cargo")
#define TYPE_GENERAL							CONSTLIT("general")
#define TYPE_MAIN_DRIVE							CONSTLIT("mainDrive")

ALERROR CShipInteriorDesc::BindDesign (SDesignLoadCtx &Ctx)

//	BindDesign
//
//	Bind

	{
	return NOERROR;
	}

int CShipInteriorDesc::GetHitPoints (void) const

//	GetHitPoints
//
//	Returns the total number of hit points of all compartments.

	{
	int i;

	int iTotalHP = 0;
	for (i = 0; i < m_Compartments.GetCount(); i++)
		iTotalHP += m_Compartments[i].iMaxHP;

	return iTotalHP;
	}

ALERROR CShipInteriorDesc::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitFromXML
//
//	Initialize from XML

	{
	int i;

	//	Load all compartments

	m_Compartments.InsertEmpty(pDesc->GetContentElementCount());
	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pComp = pDesc->GetContentElement(i);
		SCompartmentDesc &Comp = m_Compartments[i];

		if (strEquals(pComp->GetTag(), COMPARTMENT_TAG))
			{
			//	Name

			if (!pComp->FindAttribute(NAME_ATTRIB, &Comp.sName))
				Comp.sName = CONSTLIT("interior compartment");

			//	Figure out the type

			CString sType = pComp->GetAttribute(TYPE_ATTRIB);
			if (sType.IsBlank()
					|| strEquals(sType, TYPE_GENERAL))
				Comp.iType = deckGeneral;
			else if (strEquals(sType, TYPE_CARGO))
				Comp.iType = deckCargo;
			else if (strEquals(sType, TYPE_MAIN_DRIVE))
				Comp.iType = deckMainDrive;
			else
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Unknown compartment type: %s"), sType);
				return ERR_FAIL;
				}

			//	Hit points

			Comp.iMaxHP = pComp->GetAttributeIntegerBounded(HIT_POINTS_ATTRIB, 0, -1, 0);

			//	Position and size

			int xPos = pComp->GetAttributeInteger(POS_X_ATTRIB);
			int yPos = pComp->GetAttributeInteger(POS_Y_ATTRIB);
			int cxWidth = pComp->GetAttributeInteger(SIZE_X_ATTRIB);
			int cyHeight = pComp->GetAttributeInteger(SIZE_Y_ATTRIB);

			//	Input is in Cartessian coordinate; we need to convert to image
			//	coordinates relative to the center.

			Comp.rcPos.left = xPos - (cxWidth / 2);
			Comp.rcPos.right = Comp.rcPos.left + cxWidth;
			Comp.rcPos.top = -yPos - (cyHeight / 2);
			Comp.rcPos.bottom = Comp.rcPos.top + cyHeight;

			//	If no coordinates then this is a default compartment

			if (cxWidth == 0 && cyHeight == 0)
				Comp.fDefault = true;
			}
		else
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Invalid interior element: %s"), pComp->GetTag());
			return ERR_FAIL;
			}
		}

	return NOERROR;
	}
