//	CSpaceEnvironmentType.cpp
//
//	CSpaceEnvironmentType class

#include "PreComp.h"

#define IMAGE_TAG								CONSTLIT("Image")

#define UNID_ATTRIB								CONSTLIT("UNID")
#define LRS_JAMMER_ATTRIB						CONSTLIT("lrsJammer")
#define SHIELD_JAMMER_ATTRIB					CONSTLIT("shieldJammer")
#define SRS_JAMMER_ATTRIB						CONSTLIT("srsJammer")
#define DRAG_FACTOR_ATTRIB						CONSTLIT("dragFactor")
#define ATTRIBUTES_ATTRIB						CONSTLIT("attributes")

#define ON_OBJ_UPDATE_EVENT						CONSTLIT("OnObjUpdate")

ALERROR CSpaceEnvironmentType::FireOnUpdate (CSpaceObject *pObj, CString *retsError)

//	FireOnUpdate
//
//	Fire OnUpdate event (once per 15 ticks)

	{
	ICCItem *pCode;
	if (FindEventHandler(ON_OBJ_UPDATE_EVENT, &pCode))
		{
		CCodeChainCtx Ctx;

		Ctx.DefineSpaceObject(CONSTLIT("aObj"), pObj);

		ICCItem *pResult = Ctx.Run(pCode);
		if (pResult->IsError())
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("SpaceEnv OnUpdate: %s"), pResult->GetStringValue());
			return ERR_FAIL;
			}

		Ctx.Discard(pResult);
		}

	return NOERROR;
	}

ALERROR CSpaceEnvironmentType::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind the design

	{
	ALERROR error;

	if (error = m_Image.OnDesignLoadComplete(Ctx))
		return error;

	return NOERROR;
	}

ALERROR CSpaceEnvironmentType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	ALERROR error;

	//	Load basic stuff

	m_bLRSJammer = pDesc->GetAttributeBool(LRS_JAMMER_ATTRIB);
	m_bShieldJammer = pDesc->GetAttributeBool(SHIELD_JAMMER_ATTRIB);
	m_bSRSJammer = pDesc->GetAttributeBool(SRS_JAMMER_ATTRIB);

	//	Drag

	int iDrag = pDesc->GetAttributeInteger(DRAG_FACTOR_ATTRIB);
	if (iDrag)
		m_rDragFactor = (iDrag / 100.0);
	else
		m_rDragFactor = 1.0;

	//	Load image

	CXMLElement *pImage = pDesc->GetContentElementByTag(IMAGE_TAG);
	if (pImage)
		{
		if (error = m_Image.InitFromXML(Ctx, pImage))
			return error;
		}

	//	Keep track of the events that we have

	m_bHasOnUpdateEvent = FindEventHandler(ON_OBJ_UPDATE_EVENT);

	//	Done

	return NOERROR;
	}

void CSpaceEnvironmentType::Paint (CG16bitImage &Dest, int x, int y)

//	Paint
//
//	Paint the space environment

	{
	m_Image.PaintImage(Dest, x, y, 0, 0);
	}

void CSpaceEnvironmentType::PaintLRS (CG16bitImage &Dest, int x, int y)

//	PaintLRS
//
//	Paint environment in LRS

	{
	int i;

	for (i = 0; i < 20; i++)
		{
		int x1 = x + mathRandom(-16, 16);
		int y1 = y + mathRandom(-16, 16);

		int r = 85 + mathRandom(-17, 17);
		int g = 100 + mathRandom(-20, 20);
		int b = 90 + mathRandom(-18, 18);

		Dest.DrawPixel(x1, y1, CG16bitImage::RGBValue(r,g,b));
		}
	}
