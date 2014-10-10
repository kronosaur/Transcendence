//	CPaintHelper.cpp
//
//	CPaintHelper class
//	Copyright (c) 2014 by Kronosaur Productions, LLC.

#include "PreComp.h"

const int STATUS_BAR_WIDTH =					100;
const int STATUS_BAR_HEIGHT =					15;

void CPaintHelper::PaintStatusBar (CG16bitImage &Dest, int x, int y, int iTick, WORD wColor, const CString &sLabel, int iPos, int iMaxPos, int *retcyHeight)

//	PaintStatusBar
//
//	Paints a status bar

	{
	if (iMaxPos == 0)
		return;

	DWORD dwOpacity = 220;
	DWORD dwDarkOpacity = 128;

	int xStart = x - (STATUS_BAR_WIDTH / 2);
	int yStart = y;

	int iFill = Max(1, iPos * STATUS_BAR_WIDTH / iMaxPos);

	//	Draw

	Dest.FillTrans(xStart, yStart + 1, iFill, STATUS_BAR_HEIGHT - 2, wColor, dwOpacity);
	Dest.FillTrans(xStart + iFill, yStart + 1, STATUS_BAR_WIDTH - iFill, STATUS_BAR_HEIGHT - 2, 0, dwDarkOpacity);

	Dest.DrawLineTrans(xStart, yStart, xStart + STATUS_BAR_WIDTH, yStart, 1, wColor, dwDarkOpacity);
	Dest.DrawLineTrans(xStart, yStart + STATUS_BAR_HEIGHT - 1, xStart + STATUS_BAR_WIDTH, yStart + STATUS_BAR_HEIGHT - 1, 1, wColor, dwDarkOpacity);
	Dest.DrawLineTrans(xStart - 1, yStart, xStart - 1, yStart + STATUS_BAR_HEIGHT, 1, wColor, dwDarkOpacity);
	Dest.DrawLineTrans(xStart + STATUS_BAR_WIDTH, yStart, xStart + STATUS_BAR_WIDTH, yStart + STATUS_BAR_HEIGHT, 1, wColor, dwDarkOpacity);

	//	Draw the label

	if (!sLabel.IsBlank())
		{
		const CG16bitFont &Font = g_pUniverse->GetNamedFont(CUniverse::fontSRSObjCounter);
		WORD wLabelColor = CG16bitImage::RGBValue(255, 255, 255);
		Font.DrawText(Dest, x, y, wLabelColor, sLabel, CG16bitFont::AlignCenter);
		}

	//	Return height

	if (retcyHeight)
		*retcyHeight = STATUS_BAR_HEIGHT;
	}

void CPaintHelper::PaintTargetHighlight (CG16bitImage &Dest, int x, int y, int iTick, int iRadius, int iRingSpacing, int iDelay, WORD wColor)

//	PaintTargetHighlight
//
//	Paints an animating target highlight.

	{
	ASSERT(iDelay >= 1);

	int iExpand = ((iTick / iDelay) % iRingSpacing);
	int iOpacityStep = iExpand * (80 / iRingSpacing);
	DrawGlowRing(Dest, x, y, iRadius, 6, wColor);
	DrawGlowRing(Dest, x, y, iRadius + iExpand, 3, wColor, 240 - iOpacityStep);
	DrawGlowRing(Dest, x, y, iRadius + iRingSpacing + iExpand, 2, wColor, 160 - iOpacityStep);
	DrawGlowRing(Dest, x, y, iRadius + 2 * iRingSpacing + iExpand, 1, wColor, 80 - iOpacityStep);
	}
