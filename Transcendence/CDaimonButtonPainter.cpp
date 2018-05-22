//	CDaimonButtonPainter.cpp
//
//	CDaimonButtonPainter class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

const int DEFAULT_RADIUS =					40;

const int STYLE_MAIN_RING1_WIDTH =			4;	//	Width of outer edge
const int STYLE_MAIN_RING2_WIDTH =			8;	//	Width of blocks around edge
const int STYLE_MAIN_DETAIL_COUNT =			6;	//	Number of blocks around edge

#define STYLECOLOR(x)						(CArtifactAwakenVisuals::GetColor(CArtifactAwakenVisuals::##x))

CDaimonButtonPainter::CDaimonButtonPainter (const CVisualPalette &VI) :
		m_VI(VI),
		m_iStyle(styleMain),
		m_xPos(0),
		m_yPos(0),
		m_iRadius(DEFAULT_RADIUS)

//	CDaimonButtonPainter constructor

	{
	}

bool CDaimonButtonPainter::HitTest (int x, int y) const

//	HitTest
//
//	Returns TRUE if the given coordinates are over the button.

	{
	//	Convert to polar coordinates

	Metric rRadius;
	VectorToPolar(CVector(x - m_xPos, m_yPos - y), &rRadius);

	//	We're inside the button if we're inside the radius

	return (mathRound(rRadius) <= m_iRadius);
	}

void CDaimonButtonPainter::Paint (CG32bitImage &Dest, EStates iState) const

//	Paint
//
//	Paint in the given state.

	{
	switch (m_iStyle)
		{
		case styleMain:
			PaintMainStyle(Dest, iState);
			break;
		}
	}

void CDaimonButtonPainter::PaintMainStyle (CG32bitImage &Dest, EStates iState) const

//	PaintMainStyle
//
//	Paints the main button style

	{
	int i;

	CG32bitPixel rgbStyleBack = STYLECOLOR(colorDeployDaimonBack);
	CG32bitPixel rgbStyleFore = STYLECOLOR(colorDeployDaimonFore);

	//	Brighten the background if we're hovering.

	CG32bitPixel rgbBack;
	if (iState == stateDisabled)
		rgbBack = CG32bitPixel(0x40, 0x40, 0x40);
	else if (iState == stateDown)
		rgbBack = CG32bitPixel::Blend(rgbStyleBack, rgbStyleFore, (BYTE)0x80);
	else if (iState == stateHover)
		rgbBack = CG32bitPixel::Blend(rgbStyleBack, rgbStyleFore, (BYTE)0x40);
	else
		rgbBack = rgbStyleBack;

	CG32bitPixel rgbFore;
	if (iState == stateDisabled)
		rgbFore = CG32bitPixel(0x80, 0x80, 0x80);
	else
		rgbFore = rgbStyleFore;

	//	Paint

	CGDraw::Circle(Dest, m_xPos, m_yPos, m_iRadius, rgbBack);

	//	Paint outer ring

	CGDraw::Arc(Dest, m_xPos, m_yPos, m_iRadius, 0, 0, STYLE_MAIN_RING1_WIDTH, rgbFore);

	int iAngleInc = 360 / STYLE_MAIN_DETAIL_COUNT;
	int iArc = iAngleInc / 2;
	int iStartAngle = AngleMod(90 - (iArc / 2));
	int iAngle = iStartAngle;
	for (i = 0; i < STYLE_MAIN_DETAIL_COUNT; i++)
		{
		CGDraw::Arc(Dest, m_xPos, m_yPos, m_iRadius, iAngle, iAngle + iArc, STYLE_MAIN_RING2_WIDTH, rgbFore);
		iAngle += iAngleInc;
		}

	//	Paint glyph

	int cxSize = (2 * m_iRadius) - (2 * STYLE_MAIN_RING2_WIDTH);
	CArtifactAwakenVisuals::PaintGlyph(Dest, CArtifactAwakenVisuals::glyphRight, m_xPos, m_yPos, cxSize, cxSize, rgbFore);
	}

void CDaimonButtonPainter::Update (void)

//	Update
//
//	Updates the animation

	{
	}
