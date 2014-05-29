//	CPaintHelper.cpp
//
//	CPaintHelper class
//	Copyright (c) 2014 by Kronosaur Productions, LLC.

#include "PreComp.h"

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
