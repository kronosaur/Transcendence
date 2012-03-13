//	SFXStarburst.cpp
//
//	Starburst SFX

#include "PreComp.h"


#define SPIKE_COUNT_ATTRIB						(CONSTLIT("spikeCount"))
#define SPIKE_LENGTH_ATTRIB						(CONSTLIT("spikeLength"))
#define PRIMARY_COLOR_ATTRIB					(CONSTLIT("primaryColor"))
#define SECONDARY_COLOR_ATTRIB					(CONSTLIT("secondaryColor"))
#define STYLE_ATTRIB							(CONSTLIT("style"))
#define LIFETIME_ATTRIB							CONSTLIT("lifetime")

#define STYLE_PLAIN								(CONSTLIT("plain"))
#define STYLE_MORNING_STAR						(CONSTLIT("morningStar"))
#define STYLE_LIGHTNING_STAR					(CONSTLIT("lightningStar"))
#define STYLE_FLARE								(CONSTLIT("flare"))

void CStarburstEffectCreator::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the RECT for the effect

	{
	retRect->left = -2;
	retRect->top = -2;
	retRect->right = 2;
	retRect->bottom = 2;
	}

void CStarburstEffectCreator::CreateDiamondSpike (int iAngle, int iLength, int iWidthAngle, SPoint *Poly)

//	CreateDiamondSpike
//
//	Creates a diamond spike

	{
	CVector vPos;

	//	Start at the origin

	Poly[0].x = 0;
	Poly[0].y = 0;

	//	The left side

	vPos = PolarToVector(iAngle + 360 - iWidthAngle, iLength / 2);
	Poly[1].x = (int)vPos.GetX();
	Poly[1].y = -(int)vPos.GetY();

	//	The tip

	vPos = PolarToVector(iAngle, iLength);
	Poly[2].x = (int)vPos.GetX();
	Poly[2].y = -(int)vPos.GetY();

	//	The right side

	vPos = PolarToVector(iAngle + iWidthAngle, iLength / 2);
	Poly[3].x = (int)vPos.GetX();
	Poly[3].y = -(int)vPos.GetY();
	}

ALERROR CStarburstEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Load from XML

	{
	ALERROR error;

	CString sStyle = pDesc->GetAttribute(STYLE_ATTRIB);
	if (strEquals(sStyle, STYLE_MORNING_STAR))
		m_iStyle = styleMorningStar;
	else if (strEquals(sStyle, STYLE_LIGHTNING_STAR))
		m_iStyle = styleLightningStar;
	else if (strEquals(sStyle, STYLE_FLARE))
		m_iStyle = styleFlare;
	else
		m_iStyle = stylePlain;

	if (error = m_SpikeCount.LoadFromXML(pDesc->GetAttribute(SPIKE_COUNT_ATTRIB)))
		return error;

	if (error = m_SpikeLength.LoadFromXML(pDesc->GetAttribute(SPIKE_LENGTH_ATTRIB)))
		return error;

	m_wPrimaryColor = ::LoadRGBColor(pDesc->GetAttribute(PRIMARY_COLOR_ATTRIB));
	m_wSecondaryColor = ::LoadRGBColor(pDesc->GetAttribute(SECONDARY_COLOR_ATTRIB));

	m_iLifetime = pDesc->GetAttributeInteger(LIFETIME_ATTRIB);

	return NOERROR;
	}

void CStarburstEffectCreator::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint effect

	{
	int i;

	//	Figure out how many spikes

	int iCount = m_SpikeCount.Roll();
	if (iCount == 0)
		return;

	//	Paint based on style

	switch (m_iStyle)
		{
		case stylePlain:
			{
			int iSeparation = 360 / iCount;
			int iAngle = Ctx.iTick * 3;

			for (i = 0; i < iCount; i++)
				{
				int xDest, yDest;
				IntPolarToVector(iAngle, (Metric)m_SpikeLength.Roll(), &xDest, &yDest);

				Dest.DrawBiColorLine(x, y,
						x + xDest,
						y + yDest,
						1,
						m_wPrimaryColor,
						Ctx.wSpaceColor);

				iAngle += iSeparation;
				}

			break;
			}

		case styleFlare:
			{
			int iSeparation = 360 / iCount;
			int iAngle = Ctx.iTick * 5;
			int iExtra = (iCount > 4 ? iCount / 4 : 1);

			for (i = 0; i < iCount; i++)
				{
				Metric rLength = (Metric)m_SpikeLength.Roll();
				if ((i % iExtra) == 0)
					rLength = rLength * 3.0;

				int xDest, yDest;
				IntPolarToVector(iAngle, rLength, &xDest, &yDest);

				Dest.DrawBiColorLine(x, y,
						x + xDest,
						y + yDest,
						1,
						m_wPrimaryColor,
						Ctx.wSpaceColor);

				iAngle += iSeparation;
				}

			//	Paint the glowing orb

			DrawAlphaGradientCircle(Dest,
					x,
					y,
					m_SpikeLength.GetMinValue(),
					m_wSecondaryColor);

			break;
			}

		case styleMorningStar:
			{
			//	Paint the spikes

			for (i = 0; i < iCount; i++)
				{
				SPoint Spike[4];
				int iSpikeLength = m_SpikeLength.Roll();
				CreateDiamondSpike(mathRandom(1, 360), iSpikeLength, 5, Spike);

				CG16bitRegion Region;
				Region.CreateFromConvexPolygon(4, Spike);
				Region.Fill(Dest, x, y, m_wSecondaryColor);
				}

			//	Paint the glowing orb

			DrawAlphaGradientCircle(Dest,
					x,
					y,
					m_SpikeLength.GetMaxValue(),
					m_wPrimaryColor);
			break;
			}

		case styleLightningStar:
			{
			//	Paint the tail

			for (i = 0; i < 3; i++)
				{
				CVector vTail = PolarToVector(Ctx.iRotation + mathRandom(-10, 10) + 360, 2 * m_SpikeLength.GetMaxValue());
				int xStart = x - (int)(vTail.GetX() + 0.5);
				int yStart = y + (int)(vTail.GetY() + 0.5);

				DrawLightning(Dest,
						xStart,
						yStart,
						x,
						y,
						m_wPrimaryColor,
						16,
						0.4);
				}

			//	Paint the spikes

			for (i = 0; i < iCount; i++)
				{
				SPoint Spike[4];
				int iSpikeLength = m_SpikeLength.Roll();
				CreateDiamondSpike(mathRandom(1, 360), iSpikeLength, 5, Spike);

				CG16bitRegion Region;
				Region.CreateFromConvexPolygon(4, Spike);
				Region.Fill(Dest, x, y, m_wSecondaryColor);
				}

			//	Paint the glowing orb

			DrawAlphaGradientCircle(Dest,
					x,
					y,
					m_SpikeLength.GetMaxValue() / 2,
					m_wPrimaryColor);
			break;
			}
		}
	}

bool CStarburstEffectCreator::PointInImage (int x, int y, int iTick, int iVariant) const

//	PointInImage
//
//	Return TRUE if the given point is in the effect

	{
	RECT rcRect;
	GetRect(&rcRect);
	return (x >= rcRect.left && x < rcRect.right && y >= rcRect.top && y < rcRect.bottom);
	}
