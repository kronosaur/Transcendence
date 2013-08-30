//	SFXBolt.cpp
//
//	Paints a simple bolt

#include "PreComp.h"

#define LENGTH_ATTRIB							(CONSTLIT("length"))
#define WIDTH_ATTRIB							(CONSTLIT("width"))
#define PRIMARY_COLOR_ATTRIB					(CONSTLIT("primaryColor"))
#define SECONDARY_COLOR_ATTRIB					(CONSTLIT("secondaryColor"))

ALERROR CBoltEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Initialize type-specific data

	{
	m_iLength = pDesc->GetAttributeInteger(LENGTH_ATTRIB);
	m_iWidth = pDesc->GetAttributeInteger(WIDTH_ATTRIB);
	m_wPrimaryColor = ::LoadRGBColor(pDesc->GetAttribute(PRIMARY_COLOR_ATTRIB));
	m_wSecondaryColor = ::LoadRGBColor(pDesc->GetAttribute(SECONDARY_COLOR_ATTRIB));

	return NOERROR;
	}

ALERROR CBoltEffectCreator::OnEffectBindDesign (SDesignLoadCtx &Ctx)

//	OnEffectBindDesign
//
//	Resolve loading

	{
	return NOERROR;
	}

void CBoltEffectCreator::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the rect of the image

	{
	retRect->left = -m_iLength;
	retRect->right = m_iLength;
	retRect->top = -m_iLength;
	retRect->bottom = m_iLength;
	}

void CBoltEffectCreator::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint the effect

	{
	//	Paint the tail

	WORD wStart, wEnd;
	int xStart, yStart;
	CVector vTail = PolarToVector(Ctx.iRotation, m_iLength);

	if (m_iWidth >= 3)
		{
		xStart = x - (int)(vTail.GetX() + 0.5);
		yStart = y + (int)(vTail.GetY() + 0.5);

		wStart = CG16bitImage::BlendPixel(Ctx.wSpaceColor, m_wSecondaryColor, 155);
		wEnd = Ctx.wSpaceColor;
		Dest.DrawBiColorLine(xStart, yStart,
				x, y,
				m_iWidth,
				wEnd,
				wStart);
		}

	vTail = vTail / 2.0;
	xStart = x - (int)(vTail.GetX() + 0.5);
	yStart = y + (int)(vTail.GetY() + 0.5);

	wStart = m_wPrimaryColor;
	wEnd = CG16bitImage::BlendPixel(Ctx.wSpaceColor, m_wSecondaryColor, 200);
	Dest.DrawBiColorLine(xStart, yStart,
			x, y,
			Max(1, m_iWidth / 2),
			wEnd,
			wStart);
	}

bool CBoltEffectCreator::PointInImage (int x, int y, int iTick, int iVariant, int iRotation) const

//	PointInImage
//
//	Returns TRUE if the given point is in the image

	{
	return (Absolute(x) < m_iWidth && Absolute(y) < m_iWidth);
	}
