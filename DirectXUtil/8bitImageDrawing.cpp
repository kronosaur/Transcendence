//	8bitImageDrawing.cpp
//
//	Drawing routines for 8-bit masks

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

void DrawHorzLine8bit (CG16bitImage &Dest, int x, int y, int cxWidth, BYTE byValue)

//	DrawHorzLine8bit
//
//	Draws a single pixel horizontal line

	{
	const RECT &rcClip = Dest.GetClipRect();
	int xEnd = x + cxWidth;

	//	Make sure this row is in range

	if (!Dest.HasAlpha() || y < rcClip.top || y >= rcClip.bottom
			|| xEnd <= rcClip.left || x >= rcClip.right)
		return;

	int xStart = Max(x, (int)rcClip.left);
	xEnd = Min(xEnd, (int)rcClip.right);

	//	Get the pointers

	BYTE *pPos = Dest.GetAlphaRow(y) + xStart;
	BYTE *pPosEnd = Dest.GetAlphaRow(y) + xEnd;
	while (pPos < pPosEnd)
		*pPos++ = byValue;
	}

void DrawFilledRect8bit (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, BYTE byValue)

//	DrawFilledRect8bit
//
//	Fills a rectangle in an 8-bit mask

	{
	//	Make sure we're in bounds

	if (!Dest.HasAlpha() || !Dest.AdjustCoords(NULL, NULL, 0, 0, 
			&x, &y, 
			&cxWidth, &cyHeight))
		return;

	//	Fill

	BYTE *pRow = Dest.GetAlphaRow(y);
	BYTE *pRowEnd = Dest.GetAlphaRow(y + cyHeight);
	while (pRow < pRowEnd)
		{
		BYTE *pPos = pRow;
		BYTE *pPosEnd = pRow + cxWidth;
		while (pPos < pPosEnd)
			*pPos++ = byValue;

		pRow = Dest.NextAlphaRow(pRow);
		}
	}
