//	SFXBeam.cpp
//
//	Paints a simple beam

#include "PreComp.h"

#define BEAM_TYPE_ATTRIB				CONSTLIT("beamType")
#define INTENSITY_ATTRIB				CONSTLIT("intensity")
#define PRIMARY_COLOR_ATTRIB			CONSTLIT("primaryColor")
#define SECONDARY_COLOR_ATTRIB			CONSTLIT("secondaryColor")

#define BEAM_TYPE_HEAVY_BLASTER			CONSTLIT("heavyblaster")
#define BEAM_TYPE_JAGGED_BOLT			CONSTLIT("jaggedBolt")
#define BEAM_TYPE_LASER					CONSTLIT("laser")
#define BEAM_TYPE_LIGHTNING				CONSTLIT("lightning")
#define BEAM_TYPE_LIGHTNING_BOLT		CONSTLIT("lightningBolt")
#define BEAM_TYPE_PARTICLE				CONSTLIT("particle")
#define BEAM_TYPE_STAR_BLASTER			CONSTLIT("starblaster")

#define BEAM_TYPE_GREEN_PARTICLE		CONSTLIT("greenparticle")
#define BEAM_TYPE_BLUE_PARTICLE			CONSTLIT("blueparticle")
#define BEAM_TYPE_BLASTER				CONSTLIT("blaster")
#define BEAM_TYPE_GREEN_LIGHTNING		CONSTLIT("greenlightning")

const int LIGHTNING_POINT_COUNT	= 16;			//	Must be a power of 2

void CBeamEffectCreator::CreateLightningGlow (SLineDesc &Line, int iPointCount, CVector *pPoints, int iSize, CG16bitBinaryRegion *retRegion)

//	CreateLightningGlow
//
//	Creates a polygon region in the shape of th lightning.

	{
	int i;

	//	Compute a perpendicular line (the line width)

	CVector vLine(Line.xTo - Line.xFrom, Line.yTo - Line.yFrom);
	CVector vHalfPerp = (iSize * vLine.Perpendicular().Normal()) / 2.0;

	//	Generate polygon points.

	SPoint *pPolygon = new SPoint [iPointCount * 2];
	SPoint *pPoint = pPolygon;
	
	//	First add all the points on one side

	for (i = 0; i < iPointCount; i++)
		{
		pPoint->x = (int)(pPoints[i].GetX() + vHalfPerp.GetX());
		pPoint->y = (int)(pPoints[i].GetY() + vHalfPerp.GetY());

		pPoint++;
		}

	//	Now add the points on the other side (backwards)

	for (i = iPointCount - 1; i >= 0; i--)
		{
		pPoint->x = (int)(pPoints[i].GetX() - vHalfPerp.GetX());
		pPoint->y = (int)(pPoints[i].GetY() - vHalfPerp.GetY());

		pPoint++;
		}

	//	Convert to region

	retRegion->CreateFromPolygon(iPointCount * 2, pPolygon);

	//	Done

	delete [] pPolygon;
	}

void CBeamEffectCreator::DrawBeam (CG16bitImage &Dest, SLineDesc &Line, SViewportPaintCtx &Ctx)

//	DrawBeam
//
//	Draws the appropriate beam

	{
	switch (m_iType)
		{
		case beamBlaster:
			DrawBeamBlaster(Dest, Line, Ctx);
			break;

		case beamHeavyBlaster:
			DrawBeamHeavyBlaster(Dest, Line, Ctx);
			break;

		case beamJaggedBolt:
			DrawBeamJaggedBolt(Dest, Line, Ctx);
			break;

		case beamLaser:
			DrawBeamLaser(Dest, Line, Ctx);
			break;

		case beamLightning:
			DrawBeamLightning(Dest, Line, Ctx);
			break;

		case beamLightningBolt:
			DrawBeamLightningBolt(Dest, Line, Ctx);
			break;

		case beamParticle:
			DrawBeamParticle(Dest, Line, Ctx);
			break;

		case beamStarBlaster:
			DrawBeamStarBlaster(Dest, Line, Ctx);
			break;
		}
	}

void CBeamEffectCreator::DrawBeamBlaster (CG16bitImage &Dest, SLineDesc &Line, SViewportPaintCtx &Ctx)

//	DrawBeamBlaster
//
//	Draws the appropriate beam

	{
	WORD wStart, wEnd;

	wStart = CG16bitImage::BlendPixel(Ctx.wSpaceColor, m_wSecondaryColor, 155);
	wEnd = Ctx.wSpaceColor;
	Dest.DrawBiColorLine(Line.xFrom, Line.yFrom,
			Line.xTo, Line.yTo,
			m_iIntensity + 4,
			wEnd,
			wStart);

	wStart = m_wSecondaryColor;
	wEnd = Ctx.wSpaceColor;
	Dest.DrawBiColorLine(Line.xFrom, Line.yFrom,
			Line.xTo, Line.yTo,
			m_iIntensity + 2,
			wEnd,
			wStart);

	wStart = m_wPrimaryColor;
	wEnd = CG16bitImage::BlendPixel(Ctx.wSpaceColor, m_wSecondaryColor, 155);
	Dest.DrawBiColorLine(Line.xFrom, Line.yFrom,
			Line.xTo, Line.yTo,
			m_iIntensity,
			wEnd,
			wStart);
	}

void CBeamEffectCreator::DrawBeamHeavyBlaster (CG16bitImage &Dest, SLineDesc &Line, SViewportPaintCtx &Ctx)

//	DrawBeamHeavyBlaster
//
//	Draws the appropriate beam

	{
	//	Convert to an angle relative to xTo, yTo

	CVector vVec(Line.xFrom - Line.xTo, Line.yFrom - Line.yTo);
	Metric rRadius;
	int iAngle = VectorToPolar(vVec, &rRadius);
	int iRadius = (int)rRadius;

	//	Can't deal with 0 sized lines

	if (iRadius == 0)
		return;

	CG16bitBinaryRegion Region;
	SPoint Poly[8];

	//	Compute some metrics

	int iLengthUnit = iRadius * (10 + m_iIntensity) / 40;
	int iWidthUnit = Max(1, iRadius * m_iIntensity / 60);

	//	Paint the outer-most glow

	WORD wColor = CG16bitImage::BlendPixel(Ctx.wSpaceColor, m_wSecondaryColor, 100);
	CreateBlasterShape(iAngle, 4 * iLengthUnit, 3 * iWidthUnit / 2, Poly);
	Region.CreateFromConvexPolygon(8, Poly);
	Region.Fill(Dest, Line.xTo, Line.yTo, wColor);

	//	Paint the inner transition

	wColor = CG16bitImage::BlendPixel(m_wSecondaryColor, m_wPrimaryColor, 128);
	wColor = CG16bitImage::BlendPixel(Ctx.wSpaceColor, wColor, 200);
	CreateBlasterShape(iAngle, 3 * iLengthUnit, iWidthUnit, Poly);
	Region.CreateFromConvexPolygon(8, Poly);
	Region.Fill(Dest, Line.xTo, Line.yTo, wColor);

	//	Paint the inner core

	CreateBlasterShape(iAngle, iLengthUnit, iWidthUnit - 1, Poly);
	Region.CreateFromConvexPolygon(8, Poly);
	Region.Fill(Dest, Line.xTo, Line.yTo, m_wPrimaryColor);
	}

void CBeamEffectCreator::DrawBeamJaggedBolt (CG16bitImage &Dest, SLineDesc &Line, SViewportPaintCtx &Ctx)

//	DrawBeamJaggedBolt
//
//	Draws a jagged bolt

	{
	int i;

	//	Rasterize the line

	CG16bitLinePainter LinePainter;
	TArray<CG16bitLinePainter::SPixelDesc> Pixels;

	LinePainter.Rasterize(Dest,
			Line.xFrom,
			Line.yFrom,
			Line.xTo,
			Line.yTo,
			m_iIntensity,
			&Pixels);

	for (i = 0; i < Pixels.GetCount(); i++)
		{
		const CG16bitLinePainter::SPixelDesc &Pixel = Pixels[i];

		DWORD dwOpacity = Min(Max(0, (int)(255.0 * (1.0 - Absolute(Pixel.rW)))), 255);
		if (Pixel.dwOpacity != 255)
			dwOpacity = dwOpacity * Pixel.dwOpacity / 255;

		if (dwOpacity == 255)
			*(Pixel.pPos) = m_wPrimaryColor;
		else
			{
			*(Pixel.pPos) = CG16bitImage::BlendPixel(*(Pixel.pPos), m_wPrimaryColor, dwOpacity);
			}
		}
	}

void CBeamEffectCreator::DrawBeamLaser (CG16bitImage &Dest, SLineDesc &Line, SViewportPaintCtx &Ctx)

//	DrawBeamLaser
//
//	Draws the appropriate beam

	{
	WORD wGlow = CG16bitImage::BlendPixel(Ctx.wSpaceColor, m_wSecondaryColor, 100);

	Dest.DrawLine(Line.xFrom, Line.yFrom,
			Line.xTo, Line.yTo,
			m_iIntensity + 2,
			wGlow);

	Dest.DrawLine(Line.xFrom, Line.yFrom,
			Line.xTo, Line.yTo,
			m_iIntensity,
			m_wPrimaryColor);
	}

void CBeamEffectCreator::DrawBeamLightning (CG16bitImage &Dest, SLineDesc &Line, SViewportPaintCtx &Ctx)

//	DrawBeamLightning
//
//	Draws the appropriate beam

	{
	//	The central part of the beam is different depending on the
	//	intensity.

	if (m_iIntensity < 4)
		{
		WORD wStart = CG16bitImage::BlendPixel(Ctx.wSpaceColor, m_wPrimaryColor, 128);
		Dest.DrawBiColorLine(Line.xFrom, Line.yFrom,
				Line.xTo, Line.yTo,
				3,
				Ctx.wSpaceColor,
				wStart);

		WORD wEnd = CG16bitImage::BlendPixel(Ctx.wSpaceColor, m_wPrimaryColor, 155);
		Dest.DrawBiColorLine(Line.xFrom, Line.yFrom,
				Line.xTo, Line.yTo,
				1,
				wEnd,
				m_wPrimaryColor);
		}
	else if (m_iIntensity < 10)
		{
		WORD wStart = CG16bitImage::BlendPixel(Ctx.wSpaceColor, m_wSecondaryColor, 155);
		Dest.DrawBiColorLine(Line.xFrom, Line.yFrom,
				Line.xTo, Line.yTo,
				5,
				Ctx.wSpaceColor,
				wStart);

		Dest.DrawBiColorLine(Line.xFrom, Line.yFrom,
				Line.xTo, Line.yTo,
				3,
				Ctx.wSpaceColor,
				m_wSecondaryColor);

		WORD wEnd = CG16bitImage::BlendPixel(Ctx.wSpaceColor, m_wPrimaryColor, 155);
		Dest.DrawBiColorLine(Line.xFrom, Line.yFrom,
				Line.xTo, Line.yTo,
				1,
				wEnd,
				m_wPrimaryColor);
		}
	else
		{
		//	Convert to an angle relative to xTo, yTo

		CVector vVec(Line.xFrom - Line.xTo, Line.yFrom - Line.yTo);
		Metric rRadius;
		int iAngle = VectorToPolar(vVec, &rRadius);
		int iRadius = (int)rRadius;

		//	Can't deal with 0 sized lines

		if (iRadius == 0)
			return;

		CG16bitBinaryRegion Region;
		SPoint Poly[8];

		//	Paint the outer-most glow

		WORD wColor = CG16bitImage::BlendPixel(Ctx.wSpaceColor, m_wSecondaryColor, 100);
		CreateBlasterShape(iAngle, iRadius, iRadius / 6, Poly);
		Region.CreateFromConvexPolygon(8, Poly);
		Region.Fill(Dest, Line.xTo, Line.yTo, wColor);

		//	Paint the inner transition

		wColor = CG16bitImage::BlendPixel(m_wSecondaryColor, m_wPrimaryColor, 128);
		wColor = CG16bitImage::BlendPixel(Ctx.wSpaceColor, wColor, 200);
		CreateBlasterShape(iAngle, iRadius * 2 / 3, iRadius / 7, Poly);
		Region.CreateFromConvexPolygon(8, Poly);
		Region.Fill(Dest, Line.xTo, Line.yTo, wColor);

		//	Paint the inner core

		CreateBlasterShape(iAngle, iRadius / 2, iRadius / 8, Poly);
		Region.CreateFromConvexPolygon(8, Poly);
		Region.Fill(Dest, Line.xTo, Line.yTo, m_wPrimaryColor);
		}

	//	Compute the half-way point

	int xHalf = (Line.xFrom + Line.xTo) / 2;
	int yHalf = (Line.yFrom + Line.yTo) / 2;

	//	Draw lightning

	int iCount = m_iIntensity + mathRandom(0, 2);
	for (int j = 0; j < iCount; j++)
		if (mathRandom(1, 2) == 1)
			DrawLightning(Dest, 
					xHalf, 
					yHalf, 
					Line.xTo, 
					Line.yTo, 
					m_wPrimaryColor, 
					LIGHTNING_POINT_COUNT, 
					0.5);
		else
			DrawLightning(Dest, 
					Line.xFrom, 
					Line.yFrom, 
					Line.xTo, 
					Line.yTo, 
					m_wSecondaryColor, 
					LIGHTNING_POINT_COUNT, 
					0.3);
	}

void CBeamEffectCreator::DrawBeamLightningBolt (CG16bitImage &Dest, SLineDesc &Line, SViewportPaintCtx &Ctx)

//	DrawBeamLightningBolt
//
//	Draws the appropriate beam

	{
	int i;

	//	Compute a set of points for the lightning

	int iPointCount = LIGHTNING_POINT_COUNT;
	CVector *pPoints = new CVector [iPointCount];
	pPoints[0] = CVector(Line.xFrom, Line.yFrom);
	pPoints[iPointCount - 1] = CVector(Line.xTo, Line.yTo);

	ComputeLightningPoints(iPointCount, pPoints, 0.3);

	//	Draw based on intensity

	switch (m_iIntensity)
		{
		//	Draw nothing

		case 0:
			break;

		//	At intensity 1 or 2 just draw a lightning line with the primary color

		case 1:
		case 2:
			{
			for (i = 0; i < iPointCount-1; i++)
				{
				const CVector &vFrom = pPoints[i];
				const CVector &vTo = pPoints[i + 1];

				Dest.DrawLine((int)vFrom.GetX(), (int)vFrom.GetY(),
						(int)vTo.GetX(), (int)vTo.GetY(),
						m_iIntensity,
						m_wPrimaryColor);
				}
			break;
			}

		//	At intensity 3 or 4 draw secondary color around primary.

		case 3:
		case 4:
			{
			for (i = 0; i < iPointCount-1; i++)
				{
				const CVector &vFrom = pPoints[i];
				const CVector &vTo = pPoints[i + 1];

				Dest.DrawLine((int)vFrom.GetX(), (int)vFrom.GetY(),
						(int)vTo.GetX(), (int)vTo.GetY(),
						m_iIntensity,
						m_wSecondaryColor);
				}

			for (i = 0; i < iPointCount-1; i++)
				{
				const CVector &vFrom = pPoints[i];
				const CVector &vTo = pPoints[i + 1];

				Dest.DrawLine((int)vFrom.GetX(), (int)vFrom.GetY(),
						(int)vTo.GetX(), (int)vTo.GetY(),
						m_iIntensity - 2,
						m_wPrimaryColor);
				}
			break;
			}

		//	At intensity 5-10 we draw a glow around the beam

		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			{
			//	Create a polygon for the glow shaped like a wide version of the
			//	lightning bolt.

			CG16bitBinaryRegion GlowRegion;
			CreateLightningGlow(Line, iPointCount, pPoints, m_iIntensity, &GlowRegion);

			//	Paint the glow

			GlowRegion.FillTrans(Dest, 0, 0, m_wSecondaryColor, 128);

			//	Paint the main bolt

			for (i = 0; i < iPointCount-1; i++)
				{
				const CVector &vFrom = pPoints[i];
				const CVector &vTo = pPoints[i + 1];

				Dest.DrawLine((int)vFrom.GetX(), (int)vFrom.GetY(),
						(int)vTo.GetX(), (int)vTo.GetY(),
						4,
						m_wSecondaryColor);
				}

			for (i = 0; i < iPointCount-1; i++)
				{
				const CVector &vFrom = pPoints[i];
				const CVector &vTo = pPoints[i + 1];

				Dest.DrawLine((int)vFrom.GetX(), (int)vFrom.GetY(),
						(int)vTo.GetX(), (int)vTo.GetY(),
						2,
						m_wPrimaryColor);
				}

			break;
			}

		//	For even larger beams we do a double glow

		default:
			{
			//	Create a polygon for the glow shaped like a wide version of the
			//	lightning bolt.

			CG16bitBinaryRegion GlowRegion1;
			CreateLightningGlow(Line, iPointCount, pPoints, m_iIntensity, &GlowRegion1);

			CG16bitBinaryRegion GlowRegion2;
			CreateLightningGlow(Line, iPointCount, pPoints, 10, &GlowRegion2);

			//	Paint the glow

			GlowRegion1.FillTrans(Dest, 0, 0, m_wSecondaryColor, 48);
			GlowRegion2.FillTrans(Dest, 0, 0, m_wSecondaryColor, 96);

			//	Paint the main bolt

			for (i = 0; i < iPointCount-1; i++)
				{
				const CVector &vFrom = pPoints[i];
				const CVector &vTo = pPoints[i + 1];

				Dest.DrawLine((int)vFrom.GetX(), (int)vFrom.GetY(),
						(int)vTo.GetX(), (int)vTo.GetY(),
						4,
						m_wSecondaryColor);
				}

			for (i = 0; i < iPointCount-1; i++)
				{
				const CVector &vFrom = pPoints[i];
				const CVector &vTo = pPoints[i + 1];

				Dest.DrawLine((int)vFrom.GetX(), (int)vFrom.GetY(),
						(int)vTo.GetX(), (int)vTo.GetY(),
						2,
						m_wPrimaryColor);
				}

			break;
			}
		}

	//	Done

	delete [] pPoints;
	}

void CBeamEffectCreator::DrawBeamParticle (CG16bitImage &Dest, SLineDesc &Line, SViewportPaintCtx &Ctx)

//	DrawBeamParticle
//
//	Draws the appropriate beam

	{
	const int iSteps = 20;
	double xStep = (double)(Line.xTo - Line.xFrom) / (double)iSteps;
	double yStep = (double)(Line.yTo - Line.yFrom) / (double)iSteps;
	double xPaint = Line.xFrom;
	double yPaint = Line.yFrom;
	int i, j;

	for (i = 0; i < iSteps; i++)
		{
		int x = ((int)xPaint);
		int y = ((int)yPaint);

		if (mathRandom(1,3) == 1)
			Dest.DrawDot(x, y, m_wPrimaryColor, CG16bitImage::markerSmallRound);
		else
			{
			for (j = 0; j < m_iIntensity; j++)
				Dest.DrawDot(x + mathRandom(-m_iIntensity, m_iIntensity),
						y + mathRandom(-m_iIntensity, m_iIntensity),
						m_wSecondaryColor,
						CG16bitImage::markerPixel);
			}

		xPaint += xStep;
		yPaint += yStep;
		}
	}

void CBeamEffectCreator::DrawBeamStarBlaster (CG16bitImage &Dest, SLineDesc &Line, SViewportPaintCtx &Ctx)

//	DrawBeamStarBlaster
//
//	Draws the appropriate beam

	{
	WORD wStart, wEnd;

	wStart = CG16bitImage::BlendPixel(Ctx.wSpaceColor, m_wPrimaryColor, 155);
	wEnd = Ctx.wSpaceColor;
	Dest.DrawBiColorLine(Line.xFrom, Line.yFrom,
			Line.xTo, Line.yTo,
			3,
			wEnd,
			wStart);

	wStart = m_wSecondaryColor;
	wEnd = CG16bitImage::BlendPixel(Ctx.wSpaceColor, m_wPrimaryColor, 155);
	Dest.DrawBiColorLine(Line.xFrom, Line.yFrom,
			Line.xTo, Line.yTo,
			1,
			wEnd,
			wStart);

	//	Draw starburst

	int iCount = (m_iIntensity / 2) + mathRandom(4, 9);
	for (int i = 0; i < iCount; i++)
		{
		CVector vLine(Line.xTo, Line.yTo);
		vLine = vLine + PolarToVector(mathRandom(0,359), 4 * m_iIntensity + mathRandom(1, 11));
		Dest.DrawBiColorLine(Line.xTo, Line.yTo,
				(int)vLine.GetX(), (int)vLine.GetY(),
				1,
				wStart,
				Ctx.wSpaceColor);
		}
	}

void CBeamEffectCreator::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the rect bounds of the image

	{
	int iSize = (int)((LIGHT_SECOND * g_SecondsPerUpdate) / g_KlicksPerPixel);

	retRect->left = -iSize;
	retRect->right = iSize;
	retRect->top = -iSize;
	retRect->bottom = iSize;
	}

ALERROR CBeamEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Initialize type-specific data

	{
	m_iType = CBeamEffectCreator::ParseBeamType(pDesc->GetAttribute(BEAM_TYPE_ATTRIB));
	if (m_iType == beamUnknown)
		{
		Ctx.sError = CONSTLIT("Invalid weapon beam type");
		return ERR_FAIL;
		}

	//	Load colors and intensity

	m_wPrimaryColor = LoadRGBColor(pDesc->GetAttribute(PRIMARY_COLOR_ATTRIB));
	m_wSecondaryColor = LoadRGBColor(pDesc->GetAttribute(SECONDARY_COLOR_ATTRIB));
	m_iIntensity = pDesc->GetAttributeIntegerBounded(INTENSITY_ATTRIB, 0, -1, 1);

	//	For backward compatibility, some old types are converted

	switch (m_iType)
		{
		case beamGreenParticle:
			m_iType = beamParticle;
			m_wPrimaryColor = CG16bitImage::RGBValue(95,241,42);
			m_wSecondaryColor = CG16bitImage::RGBValue(95,241,42);
			break;

		case beamBlueParticle:
			m_iType = beamParticle;
			m_wPrimaryColor = CG16bitImage::RGBValue(255, 255, 255);
			m_wSecondaryColor = CG16bitImage::RGBValue(64, 83, 255);
			break;

		case beamBlaster:
			if (m_wPrimaryColor == 0) m_wPrimaryColor = CG16bitImage::RGBValue(255, 255, 0);
			if (m_wSecondaryColor == 0) m_wSecondaryColor = CG16bitImage::RGBValue(255, 0, 0);
			break;
		}

	return NOERROR;
	}

ALERROR CBeamEffectCreator::OnEffectBindDesign (SDesignLoadCtx &Ctx)

//	OnEffectBindDesign
//
//	Resolve loading

	{
	return NOERROR;
	}

void CBeamEffectCreator::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint the effect

	{
	SLineDesc Line;

	//	NOTE: We add 1.0 to make the ends of multiple beam objects overlap.
	//	(Because round-off tends to lose a pixel.)

	Metric rLength = 1.0 + (LIGHT_SPEED * g_SecondsPerUpdate / g_KlicksPerPixel);
	CVector vFrom = PolarToVector(Ctx.iRotation, -rLength);

	Line.xFrom = x + (int)(vFrom.GetX() + 0.5);
	Line.yFrom = y - (int)(vFrom.GetY() + 0.5);
	Line.xTo = x;
	Line.yTo = y;

	DrawBeam(Dest, Line, Ctx);
	}

void CBeamEffectCreator::PaintHit (CG16bitImage &Dest, int x, int y, const CVector &vHitPos, SViewportPaintCtx &Ctx)

//	PaintHit
//
//	Paint the effect when hit

	{
	SLineDesc Line;
	Metric rLength = LIGHT_SPEED * g_SecondsPerUpdate / g_KlicksPerPixel;
	CVector vFrom = PolarToVector(Ctx.iRotation, -rLength);

	Line.xFrom = x + (int)(vFrom.GetX() + 0.5);
	Line.yFrom = y - (int)(vFrom.GetY() + 0.5);
	Ctx.XFormRel.Transform(vHitPos, &Line.xTo, &Line.yTo);

	DrawBeam(Dest, Line, Ctx);
	}

CBeamEffectCreator::BeamTypes CBeamEffectCreator::ParseBeamType (const CString &sValue)

//	ParseBeamType
//
//	Parses the beam type parameter

	{
	if (strEquals(sValue, BEAM_TYPE_LASER))
		return beamLaser;
	else if (strEquals(sValue, BEAM_TYPE_LIGHTNING))
		return beamLightning;
	else if (strEquals(sValue, BEAM_TYPE_LIGHTNING_BOLT))
		return beamLightningBolt;
	else if (strEquals(sValue, BEAM_TYPE_HEAVY_BLASTER))
		return beamHeavyBlaster;
	else if (strEquals(sValue, BEAM_TYPE_GREEN_PARTICLE))
		return beamGreenParticle;
	else if (strEquals(sValue, BEAM_TYPE_BLUE_PARTICLE))
		return beamBlueParticle;
	else if (strEquals(sValue, BEAM_TYPE_BLASTER))
		return beamBlaster;
	else if (strEquals(sValue, BEAM_TYPE_STAR_BLASTER))
		return beamStarBlaster;
	else if (strEquals(sValue, BEAM_TYPE_GREEN_LIGHTNING))
		return beamGreenLightning;
	else if (strEquals(sValue, BEAM_TYPE_PARTICLE))
		return beamParticle;
	else if (strEquals(sValue, BEAM_TYPE_JAGGED_BOLT))
		return beamJaggedBolt;
	else
		return beamUnknown;
	}

bool CBeamEffectCreator::PointInImage (int x, int y, int iTick, int iVariant, int iRotation) const

//	PointInImage
//
//	Returns TRUE if the given point is in the image

	{
	return (Absolute(x) < m_iIntensity && Absolute(y) < m_iIntensity);
	}

