//	SFXEllipse.cpp
//
//	Ellipse SFX

#include "PreComp.h"

#define ECCENTRICITY_ATTRIB						CONSTLIT("eccentricity")
#define LINE_COLOR_ATTRIB						CONSTLIT("lineColor")
#define LINE_STYLE_ATTRIB						CONSTLIT("lineStyle")
#define LINE_WIDTH_ATTRIB						CONSTLIT("lineWidth")
#define RADIUS_ATTRIB							CONSTLIT("radius")
#define ROTATION_ATTRIB							CONSTLIT("rotation")

#define STYLE_DASHED							CONSTLIT("dashed")
#define STYLE_SOLID								CONSTLIT("solid")

CVector CEllipseEffectCreator::GetPoint (Metric rAngle) const

//	GetPoint
//
//	Returns the point on the ellipse at the given angle (assuming the focus
//	is at 0,0).

	{
	Metric rRadius = m_rSemiMajorAxis * (1.0 - (m_rEccentricity * m_rEccentricity)) 
			/ (1.0 - m_rEccentricity * cos(rAngle));

	return CVector(cos(rAngle + m_rRotation) * rRadius, sin(rAngle + m_rRotation) * rRadius);
	}

void CEllipseEffectCreator::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the rect of the effect

	{
	}

ALERROR CEllipseEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Loads from XML

	{
	m_rSemiMajorAxis = pDesc->GetAttributeIntegerBounded(RADIUS_ATTRIB, 0, -1, 100);
	m_rEccentricity = (pDesc->GetAttributeIntegerBounded(ECCENTRICITY_ATTRIB, 0, 99, 0)) / 100.0;
	m_rRotation = AngleToRadians(pDesc->GetAttributeIntegerBounded(ROTATION_ATTRIB, 0, -1, 0) % 360);

	m_wLineColor = ::LoadRGBColor(pDesc->GetAttribute(LINE_COLOR_ATTRIB));
	m_iLineWidth = pDesc->GetAttributeIntegerBounded(LINE_WIDTH_ATTRIB, 1, -1, 1);
	m_sLineStyle = pDesc->GetAttribute(LINE_STYLE_ATTRIB);
	if (m_sLineStyle.IsBlank())
		m_sLineStyle = STYLE_SOLID;

	return NOERROR;
	}

void CEllipseEffectCreator::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint the effect

	{
	int iRadius = (int)m_rSemiMajorAxis;
	if (iRadius == 0)
		{
		Dest.DrawPixel(x, y, m_wLineColor);
		return;
		}

	//	Different algorithms for solid vs. dashed lines

	if (strEquals(m_sLineStyle, STYLE_SOLID))
		{
		//	Figure out how long each segment should be. Heuristically we choose
		//	twice the width, but no less than 3 pixels.

		int iSegLen = Max(3, 2 * m_iLineWidth);

		//	Compute the angle to advance (roughly) the segment length

		Metric rInc = (Metric)iSegLen / m_rSemiMajorAxis;

		//	We draw this as a series of line segments. We start at angle 0 and paint
		//	segments until we get back to the beginning.

		Metric rAngle = 0.0;
		CVector vStart = GetPoint(rAngle);
		int xStart = (int)vStart.GetX();
		int yStart = (int)vStart.GetY();

		while (rAngle < 2 * g_Pi)
			{
			rAngle += rInc;
			
			CVector vEnd = GetPoint(rAngle);
			int xEnd = (int)vEnd.GetX();
			int yEnd = (int)vEnd.GetY();

			Dest.DrawLine(x + xStart, y + yStart, x + xEnd, y + yEnd, m_iLineWidth, m_wLineColor);

			xStart = xEnd;
			yStart = yEnd;
			}
		}

	//	Dashed lines

	else
		{
		}
	}

bool CEllipseEffectCreator::PointInImage (int x, int y, int iTick, int iVariant, int iRotation) const

//	PointInImage
//
//	Returns TRUE if point in in the image

	{
	return false;
	}
