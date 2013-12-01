//	8bitImageDrawing.cpp
//
//	Drawing routines for 8-bit masks

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

void CreateRoundedRectAlpha (int cxWidth, int cyHeight, int iRadius, CG16bitImage *retImage)

//	CreateRoundedRectAlpha
//
//	Creates an image with a round-rect mask of the given parameters.

	{
	int i;

	if (cxWidth <= 0 || cyHeight <= 0)
		return;

	//	Start with a full rectangle

	retImage->CreateBlankAlpha(cxWidth, cyHeight, 255);

	//	Radius can't be larger than half the dimensions

	iRadius = Min(Min(iRadius, cxWidth / 2), cyHeight / 2);
	if (iRadius <= 0)
		return;

	//	Generate a set of raster lines for the corner
	//	NOTE: We guarantee that the solid part is always 1 less than the radius.

	int *pSolid = new int [iRadius];
	BYTE *pEdge = new BYTE [iRadius];
	RasterizeQuarterCircle8bit(iRadius, pSolid, pEdge);

	//	Mask out the corners

	for (i = 0; i < iRadius; i++)
		{
		BYTE *pTopRow = retImage->GetAlphaRow(i);
		BYTE *pBottomRow = retImage->GetAlphaRow(cyHeight - (i + 1));

		int cxEdge = (iRadius - (pSolid[i] + 1));

		//	Left corners

		if (cxEdge > 0)
			{
			utlMemSet(pTopRow, cxEdge, 0);
			utlMemSet(pBottomRow, cxEdge, 0);
			}

		pTopRow[cxEdge] = pEdge[i];
		pBottomRow[cxEdge] = pEdge[i];

		//	Right corners

		if (cxEdge > 0)
			{
			utlMemSet(pTopRow + cxWidth - cxEdge, cxEdge, 0);
			utlMemSet(pBottomRow + cxWidth - cxEdge, cxEdge, 0);
			}

		pTopRow[cxWidth - cxEdge - 1] = pEdge[i];
		pBottomRow[cxWidth - cxEdge - 1] = pEdge[i];
		}

	//	Clean up

	delete [] pSolid;
	delete [] pEdge;
	}

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
