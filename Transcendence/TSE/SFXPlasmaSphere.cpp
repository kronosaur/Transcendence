//	SFXPlasmaSphere.cpp
//
//	Plasma Sphere SFX

#include "PreComp.h"

#define SPIKE_COUNT_ATTRIB						(CONSTLIT("spikeCount"))
#define SPIKE_LENGTH_ATTRIB						(CONSTLIT("spikeLength"))
#define PRIMARY_COLOR_ATTRIB					(CONSTLIT("primaryColor"))
#define RADIUS_ATTRIB							(CONSTLIT("radius"))
#define SECONDARY_COLOR_ATTRIB					(CONSTLIT("secondaryColor"))

void CPlasmaSphereEffectCreator::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the RECT for the effect

	{
	int iRadius = m_Radius.GetAveValue();

	retRect->left = -iRadius;
	retRect->top = -iRadius;
	retRect->right = iRadius;
	retRect->bottom = iRadius;
	}

ALERROR CPlasmaSphereEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Load from XML

	{
	ALERROR error;

	CString sValue;
	if (pDesc->FindAttribute(RADIUS_ATTRIB, &sValue))
		{
		if (error = m_Radius.LoadFromXML(sValue))
			return error;
		}
	else
		m_Radius.SetConstant(64);

	if (error = m_SpikeCount.LoadFromXML(pDesc->GetAttribute(SPIKE_COUNT_ATTRIB)))
		return error;

	if (error = m_SpikeLength.LoadFromXML(pDesc->GetAttribute(SPIKE_LENGTH_ATTRIB)))
		return error;

	m_wPrimaryColor = ::LoadRGBColor(pDesc->GetAttribute(PRIMARY_COLOR_ATTRIB));
	m_wSecondaryColor = ::LoadRGBColor(pDesc->GetAttribute(SECONDARY_COLOR_ATTRIB));

	return NOERROR;
	}

void CPlasmaSphereEffectCreator::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint effect

	{
	int i;

	int iRadius = m_Radius.Roll();
	DrawAlphaGradientCircle(Dest,
			x,
			y,
			iRadius,
			m_wPrimaryColor);

	//	Paint the spikes

	int iCount = m_SpikeCount.Roll();
	if (iCount)
		{
		int iSeparation = 360 / iCount;
		int iAngle = Ctx.iTick * 3;

		for (i = 0; i < iCount; i++)
			{
			int xDest, yDest;
			IntPolarToVector(iAngle, (Metric)m_SpikeLength.Roll(), &xDest, &yDest);

			DrawLightning(Dest,
					x, y,
					x + xDest, y + yDest,
					m_wSecondaryColor,
					16,	//	Must be a power of 2
					0.5);

			iAngle += iSeparation;
			}
		}
	}

bool CPlasmaSphereEffectCreator::PointInImage (int x, int y, int iTick, int iVariant, int iRotation) const

//	PointInImage
//
//	Return TRUE if the given point is in the effect

	{
	int iRadius = m_Radius.GetAveValue();
	return (mathSqrt(x * x + y * y) < iRadius);
	}
