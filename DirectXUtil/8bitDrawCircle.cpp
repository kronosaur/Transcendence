//	8bitDrawCircle.cpp
//
//	Drawing routines for circles

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

struct SGradientCircleLineCtx
	{
	CG16bitImage *pDest;
	int xDest;
	int yDest;
	int iRadius;
	BYTE byCenter;
	BYTE byEdge;
	BYTE byDiff;
	bool bReverse;
	};

//	DrawFilledCircle -----------------------------------------------------------

void DrawFilledCircle8bit (CG16bitImage &Dest, int xDest, int yDest, int iRadius, BYTE byValue)
	{
	//	Deal with edge-conditions

	if (!Dest.HasAlpha() || iRadius <= 0)
		return;

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	DrawHorzLine8bit(Dest, xDest - iRadius, yDest, 1 + 2 * iRadius, byValue);

	//	Loop

	while (y > x)
		{
		if (d < 0)
			{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
			}
		else
			{
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;

			DrawHorzLine8bit(Dest, xDest - x, yDest - y, 1 + 2 * x, byValue);
			DrawHorzLine8bit(Dest, xDest - x, yDest + y, 1 + 2 * x, byValue);

			y--;
			}

		x++;

		if (y >= x)
			{
			DrawHorzLine8bit(Dest, xDest - y, yDest - x, 1 + 2 * y, byValue);
			DrawHorzLine8bit(Dest, xDest - y, yDest + x, 1 + 2 * y, byValue);
			}
		}
	}

//	DrawGradientCircle ---------------------------------------------------------

void DrawGradientCircleLine (const SGradientCircleLineCtx &Ctx, int x, int y)
	{
	int xStart = Ctx.xDest - x;
	int xEnd = Ctx.xDest + x + 1;
	const RECT &rcClip = Ctx.pDest->GetClipRect();

	if (xEnd <= rcClip.left || xStart >= rcClip.right)
		return;

	//	See which lines we need to paint

	int yLine = Ctx.yDest - y;
	bool bPaintTop = (yLine >= rcClip.top && yLine < rcClip.bottom);
	BYTE *pCenterTop = Ctx.pDest->GetAlphaRow(yLine) + Ctx.xDest;

	yLine = Ctx.yDest + y;
	bool bPaintBottom = ((y > 0) && (yLine >= rcClip.top && yLine < rcClip.bottom));
	BYTE *pCenterBottom = Ctx.pDest->GetAlphaRow(yLine) + Ctx.xDest;

	//	Compute radius increment

	int iRadius = y;
	int d = -y;
	int deltaE = 3;
	int deltaSE = -2 * y + 1;

	//	Loop

	int xPos = 0;

	//	This will skip the center pixel in the circle (avoids a divide by
	//	zero in the inner loop).

	if (y == 0)
		{
		if (bPaintTop && Ctx.xDest < rcClip.right && Ctx.xDest >= rcClip.left)
			*pCenterTop = Ctx.byCenter;

		xPos = 1;
		d += deltaSE;
		deltaE += 2;
		iRadius++;
		}

	//	Blt the line 

	while (xPos <= x)
		{
		//	Figure out the radius of the pixel at this location

		if (d < 0)
			{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
			}
		else
			{
			d += deltaSE;
			deltaE += 2;
			//  deltaSE += 0;
			iRadius++;
			}

		//	Compute the value based on the radius

		BYTE byValue;
		if (Ctx.bReverse)
			byValue = (BYTE)((DWORD)Ctx.byCenter + ((DWORD)Ctx.byDiff * (DWORD)iRadius / (DWORD)Ctx.iRadius));
		else
			byValue = (BYTE)((DWORD)Ctx.byCenter - ((DWORD)Ctx.byDiff * (DWORD)iRadius / (DWORD)Ctx.iRadius));

		//	Paint

		if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
			{
			if (bPaintTop)
				*(pCenterTop - xPos) = byValue;

			if (bPaintBottom)
				*(pCenterBottom - xPos) = byValue;
			}

		if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
			{
			if (bPaintTop)
				*(pCenterTop + xPos) = byValue;

			if (bPaintBottom)
				*(pCenterBottom + xPos) = byValue;
			}

		xPos++;
		}
	}

void DrawGradientCircle8bit (CG16bitImage &Dest, 
							 int xDest, 
							 int yDest, 
							 int iRadius,
							 BYTE byCenter,
							 BYTE byEdge)

//	DrawGradientCircle8bit
//
//	Draws a filled circle gradient

	{
	if (!Dest.HasAlpha() || iRadius <= 0)
		return;

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	//	Prepare struct

	SGradientCircleLineCtx Ctx;
	Ctx.pDest = &Dest;
	Ctx.xDest = xDest;
	Ctx.yDest = yDest;
	Ctx.iRadius = iRadius;
	Ctx.byCenter = byCenter;
	Ctx.byEdge = byEdge;
	Ctx.bReverse = (byEdge > byCenter);
	Ctx.byDiff = (!Ctx.bReverse ? (byCenter - byEdge) : (byEdge - byCenter));

	//	Draw central line

	DrawGradientCircleLine(Ctx, iRadius, 0);

	//	Draw lines above and below the center

	int iLastDraw = -1;
	while (y > x)
		{
		if (d < 0)
			{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
			}
		else
			{
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;

			//	Draw lines

			DrawGradientCircleLine(Ctx, x, y);
			iLastDraw = y;

			//	Next

			y--;
			}

		x++;

		//	Draw lines

		if (x != iLastDraw)
			DrawGradientCircleLine(Ctx, y, x);
		}
	}

void RasterizeQuarterCircle8bit (int iRadius, int *retSolid, BYTE *retEdge, DWORD byOpacity)

//	RasterizeQuarterCircle8bit
//
//	Returns two arrays:
//
//	The first is an array (of size iRadius) is the length of the solid part of
//	each raster line. We guarantee that the solid part is always 1 less than 
//	the radius.
//
//	The second array (also of size iRadius) is the opacity value of each
//	pixel at the edge of the raster line.

	{
	int i;

	ASSERT(iRadius > 0);

	Metric rRow = iRadius - 0.5;
	Metric rRadius2 = (iRadius * iRadius);

	for (i = 0; i < iRadius; i++)
		{
		Metric rLen = sqrt(rRadius2 - (rRow * rRow));
		retSolid[i] = (int)rLen;
		retEdge[i] = (BYTE)((rLen - retSolid[i]) * 255);

		//	We make sure that we fit inside the radius, so the solid part always
		//	has to be 1 less than the radius

		if (retSolid[i] == iRadius)
			{
			retSolid[i]--;
			retEdge[i] = 255;
			}

		rRow -= 1.0;
		}

	//	Adjust for opacity

	if (byOpacity != 255)
		{
		for (i = 0; i < iRadius; i++)
			retEdge[i] = (BYTE)(retEdge[i] * byOpacity / 255);
		}
	}
