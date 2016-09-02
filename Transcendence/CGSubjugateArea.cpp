//	CGSubjugateArea.cpp
//
//	CGSubjugateArea class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

const int COUNTERMEASURES_COUNT =			6;
const int COUNTERMEASURES_INNER_RADIUS =	94;
const int COUNTERMEASURES_ARC_ANGLE =		(360 / COUNTERMEASURES_COUNT);
const int COUNTERMEASURE_SPACING =			12;
const int COUNTERMEASURE_WIDTH =			80;

const int DAIMON_BORDER_RADIUS =			6;
const int DAIMON_COUNT =					6;
const int DAIMON_HEIGHT =					80;
const int DAIMON_WIDTH =					120;
const int DAIMON_SPACING_X =				12;
const int DAIMON_SPACING_Y =				12;

const int DMZ_WIDTH =						40;				//	Distance from outer edge of countermeasures to daimons

CGSubjugateArea::CGSubjugateArea (const CVisualPalette &VI) : 
		m_VI(VI)

//	CGSubjugateArea constructor

	{
	int i;

	//	Initialize the countermeasure loci (these form a ring around the central
	//	core).

	int iAngle = 0;
	m_CountermeasureLoci.InsertEmpty(COUNTERMEASURES_COUNT);
	for (i = 0; i < COUNTERMEASURES_COUNT; i++)
		{
		SCountermeasureLocus &Locus = m_CountermeasureLoci[i];

		Locus.iStartAngle = iAngle;
		Locus.iArc = COUNTERMEASURES_ARC_ANGLE;
		Locus.iInnerRadius = COUNTERMEASURES_INNER_RADIUS;
		Locus.iOuterRadius = COUNTERMEASURES_INNER_RADIUS + COUNTERMEASURE_WIDTH;

		iAngle += COUNTERMEASURES_ARC_ANGLE;
		}

	//	Compute the x-offset of the upper and lower daimon rows so that they 
	//	follow the curve of the central core.

	Metric rCentralRowDist = COUNTERMEASURES_INNER_RADIUS + COUNTERMEASURE_WIDTH + DMZ_WIDTH;
	Metric rRowHeight = DAIMON_HEIGHT + DAIMON_SPACING_Y;
	Metric rTopRowDist = sqrt(rCentralRowDist * rCentralRowDist - rRowHeight * rRowHeight);

	int xCentralRow = -(int)rCentralRowDist - DAIMON_WIDTH;
	int xTopRow = -(int)rTopRowDist - DAIMON_WIDTH;

	//	Initialize the daimon loci.

	m_DaimonLoci.InsertEmpty(DAIMON_COUNT);
	m_DaimonLoci[0].xPos = xCentralRow;
	m_DaimonLoci[0].yPos = -(DAIMON_HEIGHT / 2);
	m_DaimonLoci[0].cxWidth = DAIMON_WIDTH;
	m_DaimonLoci[0].cyHeight = DAIMON_HEIGHT;

	m_DaimonLoci[1].xPos = xTopRow;
	m_DaimonLoci[1].yPos = -(DAIMON_HEIGHT / 2) - (DAIMON_HEIGHT + DAIMON_SPACING_Y);
	m_DaimonLoci[1].cxWidth = DAIMON_WIDTH;
	m_DaimonLoci[1].cyHeight = DAIMON_HEIGHT;

	m_DaimonLoci[2].xPos = xTopRow;
	m_DaimonLoci[2].yPos = (DAIMON_HEIGHT / 2) + DAIMON_SPACING_Y;
	m_DaimonLoci[2].cxWidth = DAIMON_WIDTH;
	m_DaimonLoci[2].cyHeight = DAIMON_HEIGHT;

	m_DaimonLoci[3].xPos = xCentralRow - (DAIMON_WIDTH + DAIMON_SPACING_X);
	m_DaimonLoci[3].yPos = -(DAIMON_HEIGHT / 2);
	m_DaimonLoci[3].cxWidth = DAIMON_WIDTH;
	m_DaimonLoci[3].cyHeight = DAIMON_HEIGHT;

	m_DaimonLoci[4].xPos = xTopRow - (DAIMON_WIDTH + DAIMON_SPACING_X);
	m_DaimonLoci[4].yPos = -(DAIMON_HEIGHT / 2) - (DAIMON_HEIGHT + DAIMON_SPACING_Y);
	m_DaimonLoci[4].cxWidth = DAIMON_WIDTH;
	m_DaimonLoci[4].cyHeight = DAIMON_HEIGHT;

	m_DaimonLoci[5].xPos = xTopRow - (DAIMON_WIDTH + DAIMON_SPACING_X);
	m_DaimonLoci[5].yPos = (DAIMON_HEIGHT / 2) + DAIMON_SPACING_Y;
	m_DaimonLoci[5].cxWidth = DAIMON_WIDTH;
	m_DaimonLoci[5].cyHeight = DAIMON_HEIGHT;

	//	Compute some colors

	m_rgbCountermeasureBack = CG32bitPixel(255, 127, 127, 66);		//	H:0   S:50 B:100
	m_rgbDaimonBack = CG32bitPixel(229, 161, 229, 66);				//	H:300 S:30 B:90
	}

CGSubjugateArea::~CGSubjugateArea (void)

//	CGSubjugateArea destructor

	{
	}

//	AGArea virtuals

bool CGSubjugateArea::LButtonDown (int x, int y)

//	LButtonDown
//
//	Handle mouse

	{
	return false;
	}

void CGSubjugateArea::OnSetRect (void)

//	OnSetRect
//
//	The rectangle has been set, so we can compute some metrics

	{
	RECT rcRect = GetParent()->GetPaintRect(GetRect());

	//	We split the rect in thirds

	int cxThird = RectWidth(rcRect) / 3;
	int xCol1 = rcRect.left;
	int xCol2 = xCol1 + cxThird;
	int xCol3 = rcRect.right - cxThird;

	//	The list of daimons in hand is in the first column

	m_rcHand.left = xCol1;
	m_rcHand.top = rcRect.top;
	m_rcHand.right = xCol1 + cxThird;
	m_rcHand.bottom = rcRect.bottom;

	//	The core is at the center of the 3rd column

	m_xCenter = xCol3 + (cxThird / 2);
	m_yCenter = rcRect.top + (RectHeight(rcRect) / 2);
	}

void CGSubjugateArea::Paint (CG32bitImage &Dest, const RECT &rcRect)

//	Paint
//
//	Paint

	{
	Dest.Fill(m_rcHand.left, m_rcHand.top, RectWidth(m_rcHand), RectHeight(m_rcHand), CG32bitPixel(128, 128, 255, 128));

	m_AICorePainter.Paint(Dest, m_xCenter, m_yCenter);
	PaintLoci(Dest, rcRect);
	}

void CGSubjugateArea::PaintLoci (CG32bitImage &Dest, const RECT &rcRect) const

//	PaintLoci
//
//	Paints the daimon and countermeasure loci.

	{
	int i;

	for (i = 0; i < m_CountermeasureLoci.GetCount(); i++)
		{
		CGDraw::Arc(Dest,
				m_xCenter,
				m_yCenter,
				m_CountermeasureLoci[i].iInnerRadius,
				m_CountermeasureLoci[i].iStartAngle,
				m_CountermeasureLoci[i].iStartAngle + m_CountermeasureLoci[i].iArc,
				COUNTERMEASURE_WIDTH,
				m_rgbCountermeasureBack,
				CGDraw::blendNormal,
				COUNTERMEASURE_SPACING / 2,
				CGDraw::ARC_INNER_RADIUS);
		}

	for (i = 0; i < m_DaimonLoci.GetCount(); i++)
		{
		CGDraw::RoundedRect(Dest,
				m_xCenter + m_DaimonLoci[i].xPos,
				m_yCenter + m_DaimonLoci[i].yPos,
				m_DaimonLoci[i].cxWidth,
				m_DaimonLoci[i].cyHeight,
				DAIMON_BORDER_RADIUS,
				m_rgbDaimonBack);
		}
	}

void CGSubjugateArea::Update (void)

//	Update
//
//	Update

	{
	}
