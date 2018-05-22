//	CArtifactAwakenVisuals.cpp
//
//	CArtifactAwakenVisuals class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

CG32bitPixel COLOR_TABLE[CArtifactAwakenVisuals::colorCount] =
	{
	CG32bitPixel(0x4d, 0x36, 0x4d),			//	DeployDaimonBack		H:300 S:30  B:30
	CG32bitPixel(0xff, 0xb2, 0xff),			//	DeployDaimonFore		H:300 S:30  B:100
	//CG32bitPixel(0x36, 0x4d, 0x36),			//	DeployDaimonBack		H:120 S:30  B:30
	//CG32bitPixel(0xb3, 0xff, 0xb3),			//	DeployDaimonFore		H:120 S:30  B:100
	CG32bitPixel(0xff, 0xd9, 0xff),			//	DeployDaimonTitle		H:300 S:15  B:100
	CG32bitPixel(229, 161, 229, 66),		//	DaimonLocusBack			H:300 S:30  B:90
	CG32bitPixel(255, 127, 127, 66),		//	CountermeasureLocusBack	H:0   S:50  B:100

	CG32bitPixel(0xff, 0x80, 0x80, 0x80),	//	AICoreBack				H:0   S:50  B:100
	CG32bitPixel(0xff, 0x80, 0x80),			//	AICoreFore				H:0   S:50  B:100
	};

CG32bitPixel CArtifactAwakenVisuals::GetColor (EColors iColor)

//	GetColor
//
//	Returns a color

	{
	return COLOR_TABLE[iColor];
	}

void CArtifactAwakenVisuals::PaintGlyph (CG32bitImage &Dest, EGlyphs iGlyph, int x, int y, int cxWidth, int cyHeight, CG32bitPixel rgbColor)

//	PaintGlyph
//
//	Paints a glyph centered on the given coordinates.

	{
	switch (iGlyph)
		{
		case glyphRight:
			{
			static const Metric INNER_WIDTH_RATIO = 0.6;
			static const Metric WIDTH_RATIO = 0.3;
			static const Metric SMALL_ARROW_SCALE = 0.7;
			static const Metric SMALL_ARROW_OFFSET = 0.4;

			Metric rHalfHeight = 0.5 * cyHeight;
			Metric rInnerX = -rHalfHeight * INNER_WIDTH_RATIO;
			Metric rWidth = rHalfHeight * WIDTH_RATIO;

			//	Large arrow

			CGPath Path;
			Path.MoveTo(CVector(0.0, 0.0));
			Path.LineTo(CVector(rInnerX, rHalfHeight));
			Path.LineTo(CVector(rWidth, 0.0));
			Path.LineTo(CVector(rInnerX, -rHalfHeight));
			Path.Close();

			CGRegion Region;
			Path.Rasterize(&Region);

			CGDraw::Region(Dest, x, y, Region, rgbColor);

			//	Small arrow

			Path.Scale(SMALL_ARROW_SCALE);
			Path.Translate(CVector(rHalfHeight * SMALL_ARROW_OFFSET, 0.0));

			Path.Rasterize(&Region);

			CGDraw::Region(Dest, x, y, Region, rgbColor);
			break;
			}
		}
	}
