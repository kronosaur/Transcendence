//	CObjectEffectDesc.cpp
//
//	CObjectEffectDesc class
//	Copyright (c) 2014 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

#define EFFECT_ATTRIB							CONSTLIT("effect")
#define ROTATION_ATTRIB							CONSTLIT("rotation")
#define TYPE_ATTRIB								CONSTLIT("type")

#define EFFECT_ROTATE_LEFT						CONSTLIT("rotateLeft")
#define EFFECT_ROTATE_RIGHT						CONSTLIT("rotateRight")
#define EFFECT_THRUST_MAIN						CONSTLIT("thrustMain")

ALERROR CObjectEffectDesc::Bind (SDesignLoadCtx &Ctx, const CObjectImageArray &Image)

//	Bind
//
//	Bind design

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_Effects.GetCount(); i++)
		{
		//	Finish initialization

		m_Effects[i].PosCalc.InitComplete(Image.GetRotationCount(), Image.GetImageViewportSize(), m_Effects[i].iRotation);

		//	Default effect

		if (m_Effects[i].pEffect.IsEmpty())
			{
			switch (m_Effects[i].iType)
				{
				case effectThrustLeft:
				case effectThrustRight:
					m_Effects[i].pEffect.SetUNID(UNID_MANEUVERING_THRUSTER);
					break;

				case effectThrustMain:
					m_Effects[i].pEffect.SetUNID(UNID_MAIN_THRUSTER);
					break;
				}
			}

		//	Bind

		if (error = m_Effects[i].pEffect.Bind(Ctx))
			return error;
		}

	return NOERROR;
	}

CEffectCreator *CObjectEffectDesc::FindEffectCreator (const CString &sUNID) const

//	FindEffectCreator
//
//	Returns the effect creator

	{
	return NULL;
	}

int CObjectEffectDesc::GetEffectCount (DWORD dwEffects) const

//	GetEffectCount
//
//	Returns the number of effects that match the given mask

	{
	int i;
	int iCount = 0;

	for (i = 0; i < m_Effects.GetCount(); i++)
		if (dwEffects & m_Effects[i].iType)
			iCount++;

	return iCount;
	}

ALERROR CObjectEffectDesc::InitFromXML (SDesignLoadCtx &Ctx, const CString &sUNID, CXMLElement *pDesc)

//	InitFromXML
//
//	Initializes

	{
	ALERROR error;
	int i;

	//	Loop over all effects and add them to the list

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pEffectXML = pDesc->GetContentElement(i);
		SEffectDesc *pEntry = m_Effects.Insert();

		CString sType = pEffectXML->GetAttribute(TYPE_ATTRIB);
		if (strEquals(sType, EFFECT_ROTATE_LEFT))
			pEntry->iType = effectThrustLeft;
		else if (strEquals(sType, EFFECT_ROTATE_RIGHT))
			pEntry->iType = effectThrustRight;
		else if (strEquals(sType, EFFECT_THRUST_MAIN))
			pEntry->iType = effectThrustMain;
		else
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Invalid effect type: %s."), sType);
			return ERR_FAIL;
			}

		//	Load the direction

		pEntry->iRotation = pEffectXML->GetAttributeInteger(ROTATION_ATTRIB);

		//	Load the position

		if (error = pEntry->PosCalc.Init(pEffectXML))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Invalid effect position."));
			return ERR_FAIL;
			}

		//	Load the effect

		if (error = pEntry->pEffect.LoadEffect(Ctx,
				strPatternSubst(CONSTLIT("%s:%d"), sUNID, i),
				(pEffectXML->GetContentElementCount() > 0 ? pEffectXML : NULL),
				pEffectXML->GetAttribute(EFFECT_ATTRIB)))
			return error;
		}

	return NOERROR;
	}

void CObjectEffectDesc::MarkImages (void)

//	MarkImages
//
//	Mark images

	{
	int i;

	for (i = 0; i < m_Effects.GetCount(); i++)
		m_Effects[i].pEffect->MarkImages();
	}
