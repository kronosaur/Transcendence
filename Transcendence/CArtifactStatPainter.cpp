//	CArtifactStatPainter.cpp
//
//	CArtifactStatPainter class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

const int CORNER_RADIUS =					4;

CArtifactStatPainter::CArtifactStatPainter (const CVisualPalette &VI) :
		m_VI(VI)

//	CArtifactStatPainter constructor

	{
	}

void CArtifactStatPainter::Paint (CG32bitImage &Dest) const

//	Paint
//
//	Paint the stat

	{
	const CG16bitFont &LabelFont = m_VI.GetFont(fontSmall);
	const CG16bitFont &StatFont = m_VI.GetFont(fontLargeBold);

	CG32bitPixel rgbBack = AA_STYLECOLOR(colorCountermeasureLocusBack);
	CG32bitPixel rgbLabel = CG32bitPixel(0x80, 0x80, 0x80);
	CG32bitPixel rgbStat = AA_STYLECOLOR(colorAICoreFore);

	//	Paint the background first

	CGDraw::RoundedRect(Dest,
			m_rcRect.left,
			m_rcRect.top,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			CORNER_RADIUS,
			rgbBack);

	//	Paint the label

	LabelFont.DrawText(Dest, m_rcRect, rgbLabel, m_sLabel, 0, CG16bitFont::AlignCenter);

	//	Paint the stat

	RECT rcRect = m_rcRect;
	rcRect.top += LabelFont.GetHeight();
	StatFont.DrawText(Dest, rcRect, rgbStat, strFromInt(m_iValue), 0, CG16bitFont::AlignCenter);
	}
