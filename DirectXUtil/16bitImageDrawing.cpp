//	16bitImageDrawing.cpp
//
//	Drawing routines such as lines and text

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

#define SMALL_SQUARE_SIZE					2
#define MEDIUM_SQUARE_SIZE					4

#define WU_ERROR_BITS						16
#define WU_INTENSITY_SHIFT					(WU_ERROR_BITS - ALPHA_BITS)
#define WU_WEIGHTING_COMPLEMENT_MASK		(ALPHA_LEVELS - 1)

const BYTE g_wSmallRoundMask[9] = 
	{
	100, 255, 100,
	255, 255, 255,
	100, 255, 100,
	};

void CG16bitImage::BresenhamLine (int x1, int y1, 
								  int x2, int y2,
								  WORD wColor)

//	BresenhamLine
//
//	Draws a line using Bresenham's algorithm

	{
	int dx = x2 - x1;
	int ax = Absolute(dx) * 2;
	int sx = (dx > 0 ? 1 : -1);

	int dy = y2 - y1;
	int ay = Absolute(dy) * 2;
	int sy = (dy > 0 ? 1 : -1);

	int x = x1;
	int y = y1;
	int d;

	if (ax > ay)		//	x dominant
		{
		d = ay - ax / 2;
		while (true)
			{
			DrawPixel(x, y, wColor);

			if (x == x2)
				return;
			else if (d >= 0)
				{
				y = y + sy;
				d = d - ax;
				}

			x = x + sx;
			d = d + ay;
			}
		}
	else				//	y dominant
		{
		d = ax - ay / 2;
		while (true)
			{
			DrawPixel(x, y, wColor);

			if (y == y2)
				return;
			else if (d >= 0)
				{
				x = x + sx;
				d = d - ay;
				}

			y = y + sy;
			d = d + ax;
			}
		}
	}

void CG16bitImage::BresenhamLineAA (int x1, int y1, 
									int x2, int y2,
									int iWidth,
									WORD wColor)

//	BresenhamLineAA
//
//	Anti-aliased version of Bresenham's algorithm

	{
	//	Calculate the slope

	int dx = x2 - x1;
	int ax = Absolute(dx) * 2;
	int sx = (dx > 0 ? 1 : -1);

	int dy = y2 - y1;
	int ay = Absolute(dy) * 2;
	int sy = (dy > 0 ? 1 : -1);

	//	Handle special cases

	if (dx == 0 && dy == 0)
		{
		DrawPixel(x1, y1, wColor);
		return;
		}

	//	Do it

	int i, x, y;

	if (ax > ay)		//	x-axis dominant
		{
		//	Compute the distance from the mid-point of the line to
		//	the top edge along the y-axis.

		double rHalfHeight = (double)iWidth * sqrt((double)(dx*dx + dy*dy)) / (double)(2 * abs(dx));
		int cyHalfCount = (int)(((2 * rHalfHeight) - 1) / 2);
		int cyCount = 2 * cyHalfCount + 1;

		int d = ay - ax / 2;

		//	Compute information for anti-aliasing. rDist is the distance
		//	from the top solid point to the edge of the line. We update
		//	the value as we proceed along the x-axis and use it for
		//	anti-aliasing the edge of the line. A negative value means
		//	that the top point is outside the edge.

		double rSlope = (double)dy / (double)dx;
		double rDistTop = rHalfHeight - (double)cyHalfCount - 0.25;
		double rDistBottom = rDistTop;

		double rDistTopInc = sy;
		double rDistTopDec = rSlope * sx;
		double rDistBottomInc = rSlope * sx;
		double rDistBottomDec = sy;

		//	Draw!

		for (x = x1, y = y1; x != x2; x += sx)
			{
			int yTop = y - cyHalfCount;

			//	Draw anti-aliasing above the line

			if (rDistTop > 0)
				DrawPixelTrans(x, yTop - 1, wColor, (BYTE)(255.0 * (rDistTop > 1.0 ? 1.0 : rDistTop)));

			//	Draw the solid part of the line

			if (x >= m_rcClip.left && x < m_rcClip.right)
				{
				WORD *pPos = GetPixel(GetRowStart(yTop), x);

				for (i = yTop; i < yTop + cyCount; i++)
					{
					if (i >= m_rcClip.top && i < m_rcClip.bottom)
						*pPos = wColor;

					pPos = NextRow(pPos);
					}
				}

			//	Draw anti-aliasing below the line

			if (rDistBottom > 0)
				DrawPixelTrans(x, yTop + cyCount, wColor, (BYTE)(255.0 * (rDistBottom > 1.0 ? 1.0 : rDistBottom)));

			//	Next point

			if (d >= 0)
				{
				y = y + sy;
				d = d - ax;
				rDistTop += rDistTopInc;
				rDistBottom -= rDistBottomDec;
				}

			d = d + ay;
			rDistTop -= rDistTopDec;
			rDistBottom += rDistBottomInc;
			}
		}
	else				//	y-axis dominant
		{
		//	Compute the distance from the mid-point of the line to
		//	the left-edge of the x-axis

		double rHalfWidth = (double)iWidth * sqrt((double)(dx*dx + dy*dy)) / (double)(2 * abs(dy));
		int cxHalfCount = (int)(((2 * rHalfWidth) - 1) / 2);
		int cxCount = 2 * cxHalfCount + 1;

		int d = ax - ay / 2;

		//	Compute information for anti-aliasing. rDist is the distance
		//	from the top solid point to the edge of the line. We update
		//	the value as we proceed along the x-axis and use it for
		//	anti-aliasing the edge of the line. A negative value means
		//	that the top point is outside the edge.

		double rSlope = (double)dx / (double)dy;
		double rDistLeft = rHalfWidth - (double)cxHalfCount - 0.25;
		double rDistRight = rDistLeft;

		double rDistLeftInc = sx;
		double rDistRightDec = sx;
		double rDistLeftDec = rSlope * sy;
		double rDistRightInc = rSlope * sy;

		//	Draw!

		for (y = y1, x = x1; y != y2; y += sy)
			{
			int xTop = x - cxHalfCount;

			//	Draw anti-aliasing to the left

			if (rDistLeft > 0)
				DrawPixelTrans(xTop - 1, y, wColor, (BYTE)(255.0 * (rDistLeft > 1.0 ? 1.0 : rDistLeft)));

			//	Draw the solid part of the line

			if (y >= m_rcClip.top && y < m_rcClip.bottom)
				{
				WORD *pPos = GetPixel(GetRowStart(y), xTop);

				for (i = xTop; i < xTop + cxCount; i++)
					if (i >= m_rcClip.left && i < m_rcClip.right)
						*pPos++ = wColor;
					else
						pPos++;
				}

			//	Draw anti-aliasing to the right of the line

			if (rDistRight > 0)
				DrawPixelTrans(xTop + cxCount, y, wColor, (BYTE)(255.0 * (rDistRight > 1.0 ? 1.0 : rDistRight)));

			//	Next point

			if (d >= 0)
				{
				x = x + sx;
				d = d - ay;
				rDistLeft += rDistLeftInc;
				rDistRight -= rDistRightDec;
				}

			d = d + ax;
			rDistLeft -= rDistLeftDec;
			rDistRight += rDistRightInc;
			}
		}
	}

void CG16bitImage::BresenhamLineAAFade (int x1, int y1, 
									int x2, int y2,
									int iWidth,
									WORD wColor1,
									WORD wColor2)

//	BresenhamLineAA
//
//	Anti-aliased version of Bresenham's algorithm

	{
	//	Calculate the slope

	int dx = x2 - x1;
	int ax = Absolute(dx) * 2;
	int sx = (dx > 0 ? 1 : -1);

	int dy = y2 - y1;
	int ay = Absolute(dy) * 2;
	int sy = (dy > 0 ? 1 : -1);

	//	Handle special cases

	if (dx == 0 && dy == 0)
		{
		DrawPixel(x1, y1, wColor1);
		return;
		}

	//	Compute color fading scale

	int iRedStart = RedValue(wColor1);
	int iRedScale = RedValue(wColor2) - iRedStart;
	int iGreenStart = GreenValue(wColor1);
	int iGreenScale = GreenValue(wColor2) - iGreenStart;
	int iBlueStart = BlueValue(wColor1);
	int iBlueScale = BlueValue(wColor2) - iBlueStart;

	//	Do it

	int i, x, y;

	if (ax > ay)		//	x-axis dominant
		{
		//	Compute the distance from the mid-point of the line to
		//	the top edge along the y-axis.

		double rHalfHeight = (double)iWidth * sqrt((double)(dx*dx + dy*dy)) / (double)(2 * abs(dx));
		int cyHalfCount = (int)(((2 * rHalfHeight) - 1) / 2);
		int cyCount = 2 * cyHalfCount + 1;

		int d = ay - ax / 2;

		//	Compute information for anti-aliasing. rDist is the distance
		//	from the top solid point to the edge of the line. We update
		//	the value as we proceed along the x-axis and use it for
		//	anti-aliasing the edge of the line. A negative value means
		//	that the top point is outside the edge.

		double rSlope = (double)dy / (double)dx;
		double rDistTop = rHalfHeight - (double)cyHalfCount - 0.25;
		double rDistBottom = rDistTop;

		//	Compute the information for fading the color from wColor1
		//	to wColor2.

		int iSteps = dx * sx;

		//	Draw!

		for (x = x1, y = y1; x != x2; x += sx)
			{
			int iStep = (x - x1) * sx;
			WORD wColor = RGBValue(iRedStart + (iStep * iRedScale / iSteps),
					iGreenStart + (iStep * iGreenScale / iSteps),
					iBlueStart + (iStep * iBlueScale / iSteps));

			int yTop = y - cyHalfCount;

			//	Draw anti-aliasing above the line

			if (rDistTop > 0)
				DrawPixelTrans(x, yTop - 1, wColor, (BYTE)(255.0 * (rDistTop > 1.0 ? 1.0 : rDistTop)));

			//	Draw the solid part of the line

			if (x >= m_rcClip.left && x < m_rcClip.right)
				{
				WORD *pPos = GetPixel(GetRowStart(yTop), x);

				for (i = yTop; i < yTop + cyCount; i++)
					{
					if (i >= m_rcClip.top && i < m_rcClip.bottom)
						*pPos = wColor;

					pPos = NextRow(pPos);
					}
				}

			//	Draw anti-aliasing below the line

			if (rDistBottom > 0)
				DrawPixelTrans(x, yTop + cyCount, wColor, (BYTE)(255.0 * (rDistBottom > 1.0 ? 1.0 : rDistBottom)));

			//	Next point

			if (d >= 0)
				{
				y = y + sy;
				d = d - ax;
				rDistTop += sy;
				rDistBottom -= sy;
				}

			d = d + ay;
			rDistTop -= rSlope * sx;
			rDistBottom += rSlope * sx;
			}
		}
	else				//	y-axis dominant
		{
		//	Compute the distance from the mid-point of the line to
		//	the left-edge of the x-axis

		double rHalfWidth = (double)iWidth * sqrt((double)(dx*dx + dy*dy)) / (double)(2 * abs(dy));
		int cxHalfCount = (int)(((2 * rHalfWidth) - 1) / 2);
		int cxCount = 2 * cxHalfCount + 1;

		int d = ax - ay / 2;

		//	Compute information for anti-aliasing. rDist is the distance
		//	from the top solid point to the edge of the line. We update
		//	the value as we proceed along the x-axis and use it for
		//	anti-aliasing the edge of the line. A negative value means
		//	that the top point is outside the edge.

		double rSlope = (double)dx / (double)dy;
		double rDistLeft = rHalfWidth - (double)cxHalfCount - 0.25;
		double rDistRight = rDistLeft;

		//	Compute the information for fading the color from wColor1
		//	to wColor2.

		int iSteps = dy * sy;

		//	Draw!

		for (y = y1, x = x1; y != y2; y += sy)
			{
			int iStep = (y - y1) * sy;
			WORD wColor = RGBValue(iRedStart + (iStep * iRedScale / iSteps),
					iGreenStart + (iStep * iGreenScale / iSteps),
					iBlueStart + (iStep * iBlueScale / iSteps));

			int xTop = x - cxHalfCount;

			//	Draw anti-aliasing to the left

			if (rDistLeft > 0)
				DrawPixelTrans(xTop - 1, y, wColor, (BYTE)(255.0 * (rDistLeft > 1.0 ? 1.0 : rDistLeft)));

			//	Draw the solid part of the line

			if (y >= m_rcClip.top && y < m_rcClip.bottom)
				{
				WORD *pPos = GetPixel(GetRowStart(y), xTop);

				for (i = xTop; i < xTop + cxCount; i++)
					if (i >= m_rcClip.left && i < m_rcClip.right)
						*pPos++ = wColor;
					else
						pPos++;
				}

			//	Draw anti-aliasing to the right of the line

			if (rDistRight > 0)
				DrawPixelTrans(xTop + cxCount, y, wColor, (BYTE)(255.0 * (rDistRight > 1.0 ? 1.0 : rDistRight)));

			//	Next point

			if (d >= 0)
				{
				x = x + sx;
				d = d - ay;
				rDistLeft += sx;
				rDistRight -= sx;
				}

			d = d + ax;
			rDistLeft -= rSlope * sy;
			rDistRight += rSlope * sy;
			}
		}
	}

void CG16bitImage::BresenhamLineAATrans (int x1, int y1, 
										 int x2, int y2,
										 int iWidth,
										 WORD wColor,
										 DWORD dwOpacity)

//	BresenhamLineAATrans
//
//	Anti-aliased version of Bresenham's algorithm

	{
	double rOpacity = dwOpacity;

	//	Calculate the slope

	int dx = x2 - x1;
	int ax = Absolute(dx) * 2;
	int sx = (dx > 0 ? 1 : -1);

	int dy = y2 - y1;
	int ay = Absolute(dy) * 2;
	int sy = (dy > 0 ? 1 : -1);

	//	Handle special cases

	if (dx == 0 && dy == 0)
		{
		DrawPixelTrans(x1, y1, wColor, (BYTE)dwOpacity);
		return;
		}

	//	Do it

	int i, x, y;

	if (ax > ay)		//	x-axis dominant
		{
		//	Compute the distance from the mid-point of the line to
		//	the top edge along the y-axis.

		double rHalfHeight = (double)iWidth * sqrt((double)(dx*dx + dy*dy)) / (double)(2 * abs(dx));
		int cyHalfCount = (int)(((2 * rHalfHeight) - 1) / 2);
		int cyCount = 2 * cyHalfCount + 1;

		int d = ay - ax / 2;

		//	Compute information for anti-aliasing. rDist is the distance
		//	from the top solid point to the edge of the line. We update
		//	the value as we proceed along the x-axis and use it for
		//	anti-aliasing the edge of the line. A negative value means
		//	that the top point is outside the edge.

		double rSlope = (double)dy / (double)dx;
		double rDistTop = rHalfHeight - (double)cyHalfCount - 0.25;
		double rDistBottom = rDistTop;

		double rDistTopInc = sy;
		double rDistTopDec = rSlope * sx;
		double rDistBottomInc = rSlope * sx;
		double rDistBottomDec = sy;

		//	Draw!

		for (x = x1, y = y1; x != x2; x += sx)
			{
			int yTop = y - cyHalfCount;

			//	Draw anti-aliasing above the line

			if (rDistTop > 0)
				DrawPixelTrans(x, yTop - 1, wColor, (BYTE)(rOpacity * (rDistTop > 1.0 ? 1.0 : rDistTop)));

			//	Draw the solid part of the line

			if (x >= m_rcClip.left && x < m_rcClip.right)
				{
				WORD *pPos = GetPixel(GetRowStart(yTop), x);

				for (i = yTop; i < yTop + cyCount; i++)
					{
					if (i >= m_rcClip.top && i < m_rcClip.bottom)
						*pPos = CG16bitImage::BlendPixel(*pPos, wColor, dwOpacity);

					pPos = NextRow(pPos);
					}
				}

			//	Draw anti-aliasing below the line

			if (rDistBottom > 0)
				DrawPixelTrans(x, yTop + cyCount, wColor, (BYTE)(rOpacity * (rDistBottom > 1.0 ? 1.0 : rDistBottom)));

			//	Next point

			if (d >= 0)
				{
				y = y + sy;
				d = d - ax;
				rDistTop += rDistTopInc;
				rDistBottom -= rDistBottomDec;
				}

			d = d + ay;
			rDistTop -= rDistTopDec;
			rDistBottom += rDistBottomInc;
			}
		}
	else				//	y-axis dominant
		{
		//	Compute the distance from the mid-point of the line to
		//	the left-edge of the x-axis

		double rHalfWidth = (double)iWidth * sqrt((double)(dx*dx + dy*dy)) / (double)(2 * abs(dy));
		int cxHalfCount = (int)(((2 * rHalfWidth) - 1) / 2);
		int cxCount = 2 * cxHalfCount + 1;

		int d = ax - ay / 2;

		//	Compute information for anti-aliasing. rDist is the distance
		//	from the top solid point to the edge of the line. We update
		//	the value as we proceed along the x-axis and use it for
		//	anti-aliasing the edge of the line. A negative value means
		//	that the top point is outside the edge.

		double rSlope = (double)dx / (double)dy;
		double rDistLeft = rHalfWidth - (double)cxHalfCount - 0.25;
		double rDistRight = rDistLeft;

		double rDistLeftInc = sx;
		double rDistRightDec = sx;
		double rDistLeftDec = rSlope * sy;
		double rDistRightInc = rSlope * sy;

		//	Draw!

		for (y = y1, x = x1; y != y2; y += sy)
			{
			int xTop = x - cxHalfCount;

			//	Draw anti-aliasing to the left

			if (rDistLeft > 0)
				DrawPixelTrans(xTop - 1, y, wColor, (BYTE)(rOpacity * (rDistLeft > 1.0 ? 1.0 : rDistLeft)));

			//	Draw the solid part of the line

			if (y >= m_rcClip.top && y < m_rcClip.bottom)
				{
				WORD *pPos = GetPixel(GetRowStart(y), xTop);

				for (i = xTop; i < xTop + cxCount; i++)
					if (i >= m_rcClip.left && i < m_rcClip.right)
						*pPos = CG16bitImage::BlendPixel(*pPos, wColor, dwOpacity);
					else
						pPos++;
				}

			//	Draw anti-aliasing to the right of the line

			if (rDistRight > 0)
				DrawPixelTrans(xTop + cxCount, y, wColor, (BYTE)(rOpacity * (rDistRight > 1.0 ? 1.0 : rDistRight)));

			//	Next point

			if (d >= 0)
				{
				x = x + sx;
				d = d - ay;
				rDistLeft += rDistLeftInc;
				rDistRight -= rDistRightDec;
				}

			d = d + ax;
			rDistLeft -= rDistLeftDec;
			rDistRight += rDistRightInc;
			}
		}
	}

void DrawArc (CG16bitImage &Dest, int xCenter, int yCenter, int iRadius, int iStartAngle, int iEndAngle, int iLineWidth, WORD wColor)

//	DrawArc
//
//	Draws axis-aligned arcs

	{
	//	Temporaries

	int iHalfWidth = iLineWidth / 2;
	Metric rOuterRadius = iRadius + iHalfWidth;
	Metric rInnerRadius = rOuterRadius - iLineWidth;
	Metric rOuterRadius2 = rOuterRadius * rOuterRadius;
	Metric rInnerRadius2 = rInnerRadius * rInnerRadius;

	//	Figure out which quadrants we paint

	bool bUpperRight;
	bool bUpperLeft;
	bool bLowerLeft;
	bool bLowerRight;
	if (iStartAngle <= iEndAngle)
		{
		bUpperRight = (iStartAngle < 90);
		bUpperLeft = (iStartAngle < 180) && (iEndAngle > 90);
		bLowerLeft = (iStartAngle < 270) && (iEndAngle > 180);
		bLowerRight = (iStartAngle < 360) && (iEndAngle > 270);
		}
	else
		{
		bUpperRight = (iEndAngle > 0);
		bUpperLeft = (iEndAngle > 90) || (iStartAngle < 180);
		bLowerLeft = (iEndAngle > 180) || (iStartAngle < 270);
		bLowerRight = (iEndAngle > 270) || (iStartAngle < 360);
		}

	//	Iterate from the center up (and use symmetry for the four quadrants)

	int iRow = 0;
	Metric rRow = 0.5;
	Metric rRowEnd = rOuterRadius;
	while (rRow < rRowEnd)
		{
		Metric rRow2 = rRow * rRow;

		Metric rOuterLen = sqrt(rOuterRadius2 - rRow2);
		Metric rInnerLen = (rRow < rInnerRadius ? sqrt(rInnerRadius2 - rRow2) : 0.0);
		Metric rWidth = rOuterLen - rInnerLen;

		int iSolidOuter = (int)rOuterLen;
		Metric rOuterFraction = rOuterLen - (Metric)iSolidOuter;
		int iSolidWidth = (int)(rWidth - rOuterFraction);
		Metric rInnerFraction = (rWidth - rOuterFraction) - (Metric)iSolidWidth;

		//	If we have a solid width, then paint the solid part of the edge

		if (iSolidWidth > 0)
			{
			if (bUpperRight)
				Dest.FillLine(xCenter + iSolidOuter - iSolidWidth, yCenter - 1 - iRow, iSolidWidth, wColor);
			if (bLowerRight)
				Dest.FillLine(xCenter + iSolidOuter - iSolidWidth, yCenter + iRow, iSolidWidth, wColor);
			if (bUpperLeft)
				Dest.FillLine(xCenter - iSolidOuter, yCenter - 1 - iRow, iSolidWidth, wColor);
			if (bLowerLeft)
				Dest.FillLine(xCenter - iSolidOuter, yCenter + iRow, iSolidWidth, wColor);
			}

		//	Paint the edges

		DWORD byOuterEdge = (DWORD)(255 * rOuterFraction);
		if (byOuterEdge)
			{
			if (bUpperRight)
				Dest.SetPixelTrans(xCenter + iSolidOuter, yCenter - 1 - iRow, wColor, byOuterEdge);
			if (bLowerRight)
				Dest.SetPixelTrans(xCenter + iSolidOuter, yCenter + iRow, wColor, byOuterEdge);
			if (bUpperLeft)
				Dest.SetPixelTrans(xCenter - iSolidOuter - 1, yCenter - 1 - iRow, wColor, byOuterEdge);
			if (bLowerLeft)
				Dest.SetPixelTrans(xCenter - iSolidOuter - 1, yCenter + iRow, wColor, byOuterEdge);
			}

		DWORD byInnerEdge = (DWORD)(255 * rInnerFraction);
		if (byInnerEdge)
			{
			if (bUpperRight)
				Dest.SetPixelTrans(xCenter + iSolidOuter - iSolidWidth - 1, yCenter - 1 - iRow, wColor, byInnerEdge);
			if (bLowerRight)
				Dest.SetPixelTrans(xCenter + iSolidOuter - iSolidWidth - 1, yCenter + iRow, wColor, byInnerEdge);
			if (bUpperLeft)
				Dest.SetPixelTrans(xCenter - iSolidOuter + iSolidWidth, yCenter - 1 - iRow, wColor, byInnerEdge);
			if (bLowerLeft)
				Dest.SetPixelTrans(xCenter - iSolidOuter + iSolidWidth, yCenter + iRow, wColor, byInnerEdge);
			}

		//	Next

		iRow++;
		rRow = (Metric)iRow + 0.5;
		}
	}

void DrawBrokenLine (CG16bitImage &Dest, int xSrc, int ySrc, int xDest, int yDest, int xyBreak, WORD wColor)

//	DrawBrokenLine
//
//	Draws a connection between two points using only horizontal and vertical lines

	{
	if (xyBreak < 0)
		xyBreak = -xyBreak;

	//	Different algorithm for horizontal vs. vertical orientation

	if (abs(xDest - xSrc) > abs(yDest - ySrc))
		{
		int xDelta = xDest - xSrc;
		if (xDelta < 0)
			{
			Swap(xSrc, xDest);
			xDelta = -xDelta;
			}

		int yDelta = yDest - ySrc;

		//	First segment

		int iDist;
		if (ySrc != yDest)
			iDist = min(xyBreak, xDelta);
		else
			iDist = xDelta;

		Dest.FillLine(xSrc, ySrc, iDist, wColor);

		//	Connector

		if (ySrc != yDest)
			{
			if (yDelta > 0)
				Dest.FillColumn(xSrc + iDist, ySrc, yDelta, wColor);
			else
				Dest.FillColumn(xSrc + iDist, yDest, -yDelta, wColor);

			//	Last segment

			if (iDist < xDelta)
				Dest.FillLine(xSrc + iDist, yDest, xDelta - iDist, wColor);
			}
		}
	else
		{
		int yDelta = yDest - ySrc;
		if (yDelta < 0)
			{
			Swap(ySrc, yDest);
			yDelta = -yDelta;
			}

		int xDelta = xDest - xSrc;

		//	First segment

		int iDist;
		if (xSrc != xDest)
			iDist = min(xyBreak, yDelta);
		else
			iDist = yDelta;

		Dest.FillColumn(xSrc, ySrc, iDist, wColor);

		//	Connector

		if (xSrc != xDest)
			{
			if (xDelta > 0)
				Dest.FillLine(xSrc, ySrc + iDist, xDelta, wColor);
			else
				Dest.FillLine(xDest, ySrc + iDist, -xDelta, wColor);

			//	Last segment

			if (iDist < yDelta)
				Dest.FillColumn(xDest, ySrc + iDist, yDelta - iDist, wColor);
			}
		}
	}

void CG16bitImage::DrawLineProc (SDrawLineCtx *pCtx, DRAWLINEPROC pfProc)

//	DrawLineProc
//
//	Calls proc at each pixel of a Bresenham line draw algorithm. Must
//	call DrawLineProcInit first

	{
	if (pCtx->IsXDominant())		//	x dominant
		{
		pCtx->d = pCtx->ay - pCtx->ax / 2;
		while (true)
			{
			pfProc(pCtx);

			if (pCtx->x == pCtx->x2)
				return;
			else if (pCtx->d >= 0)
				{
				pCtx->y = pCtx->y + pCtx->sy;
				pCtx->d = pCtx->d - pCtx->ax;
				}

			pCtx->x = pCtx->x + pCtx->sx;
			pCtx->d = pCtx->d + pCtx->ay;
			}
		}
	else				//	y dominant
		{
		pCtx->d = pCtx->ax - pCtx->ay / 2;
		while (true)
			{
			pfProc(pCtx);

			if (pCtx->y == pCtx->y2)
				return;
			else if (pCtx->d >= 0)
				{
				pCtx->x = pCtx->x + pCtx->sx;
				pCtx->d = pCtx->d - pCtx->ay;
				}

			pCtx->y = pCtx->y + pCtx->sy;
			pCtx->d = pCtx->d + pCtx->ax;
			}
		}
	}

void CG16bitImage::DrawLineProcInit (int x1, int y1, int x2, int y2, SDrawLineCtx *pCtx)

//	DrawLineProcInit
//
//	Initializes DrawLineProc structure

	{
	pCtx->pDest = this;

	pCtx->x1 = x1;
	pCtx->y1 = y1;
	pCtx->x2 = x2;
	pCtx->y2 = y2;

	pCtx->dx = x2 - x1;
	pCtx->ax = Absolute(pCtx->dx) * 2;
	pCtx->sx = (pCtx->dx > 0 ? 1 : -1);

	pCtx->dy = y2 - y1;
	pCtx->ay = Absolute(pCtx->dy) * 2;
	pCtx->sy = (pCtx->dy > 0 ? 1 : -1);

	pCtx->x = x1;
	pCtx->y = y1;
	}

void CG16bitImage::WuLine (int x0, int y0, 
						   int x1, int y1,
						   int iWidth,
						   WORD wColor)

//	WuLine
//
//	Draws an antialiased line using Xiaolin Wu's algorithsm.
//	From Michael Abrash's Graphics Programming Black Book
//	p775 (Fast Antialiased Lines Using Wu's Algorithm)

	{
	//	Make sure the line runs top to bottom

	if (y0 > y1)
		{
		int temp;
		temp = y0; y0 = y1; y1 = temp;
		temp = x0; x0 = x1; x1 = temp;
		}

	WORD *pPos = NULL;

	//	Figure out whether the line is going to the left or
	//	to the right.

	int xDir, xDelta, yDelta;
	if ((xDelta = x1 - x0) >= 0)
		xDir = 1;
	else
		{
		xDir = -1;
		xDelta = -xDelta;
		}

	//	See if this is a horizontal line

	if ((yDelta = y1 - y0) == 0)
		{
		//	If offscreen, nothing to do

		if (!InClipY(y0))
			return;

		//	Increment until we are in range

		while (xDelta > 0 && !InClipX(x0))
			{
			xDelta--;
			x0 += xDir;
			}

		while (xDelta > 0 && !InClipX(x0 + xDir * xDelta))
			xDelta--;

		//	Draw

		pPos = GetPixel(GetRowStart(y0), x0);
		while (xDelta-- > 0)
			{
			*pPos = wColor;
			pPos += xDir;
			}

		return;
		}

	//	Draw vertical line

	if (xDelta == 0)
		{
		//	If offscreen, nothing to do

		if (!InClipX(x0))
			return;

		//	Increment until we are in range

		while (yDelta > 0 && !InClipY(y0))
			{
			yDelta--;
			y0++;
			}

		while (yDelta > 0 && !InClipY(y0 + yDelta))
			yDelta--;

		pPos = GetPixel(GetRowStart(y0), x0);
		while (yDelta-- > 0)
			{
			*pPos = wColor;
			pPos = NextRow(pPos);
			}

		return;
		}

	//	Draw diagonal line

	if (xDelta == yDelta)
		{
		//	Increment until we are in range

		while (xDelta > 0 && !InClipX(x0))
			{
			xDelta--;
			yDelta--;
			x0 += xDir;
			}

		while (xDelta > 0 && !InClipX(x0 + xDir * xDelta))
			{
			xDelta--;
			yDelta--;
			}

		//	Increment until we are in range

		while (yDelta > 0 && !InClipY(y0))
			{
			xDelta--;
			yDelta--;
			y0++;
			}

		while (yDelta > 0 && !InClipY(y0 + yDelta))
			{
			xDelta--;
			yDelta--;
			}

		pPos = GetPixel(GetRowStart(y0), x0);
		while (yDelta-- > 0)
			{
			*pPos = wColor;
			pPos = NextRow(pPos) + xDir;
			}

		return;
		}

	//	Draw the initial pixel, which is always exactly intersected
	//	by the line and so needs no weighing

	if (InClipX(x0) && InClipY(y0))
		{
		pPos = GetPixel(GetRowStart(y0), x0);
		*pPos = wColor;
		}

	//	Initialize the line error accumulator

	int iErrorAcc = 0;

	//	Is this an X-major or Y-major line?

	if (yDelta > xDelta)
		{
		//	Y-major line: Calculate 16-bit fixed-point fractional part of a
		//	pixel that x advances each time y advances 1 pixel, truncating
		//	the result so that we won't overrun the endpoint along
		//	the x axis.

		int iErrorAdj = (xDelta << WU_ERROR_BITS) / yDelta;

		//	Draw all pixels except the first and last

		while (--yDelta)
			{
			//	Add the error accumulator and see if we've run over

			iErrorAcc += iErrorAdj;
			if (iErrorAcc > 0xFFFF)
				{
				x0 += xDir;
				iErrorAcc &= 0xFFFF;

				if (pPos)
					{
					if (!InClipX(x0))
						return;

					pPos = pPos + xDir;
					}
				}

			y0++;	//	Y-major, so always advance
			if (pPos)
				{
				if (!InClipY(y0))
					return;

				pPos = NextRow(pPos);
				}

			//	Make sure we're in range

			if (pPos == NULL)
				{
				if (InClipX(x0) && InClipY(y0))
					pPos = GetPixel(GetRowStart(y0), x0);
				else
					continue;
				}

			//	The most significant bits of ErrorAcc give us the
			//	intensity weighting for this pixel, and the complement
			//	of the weighting for the paired pixel

			int iWeighting = iErrorAcc >> WU_INTENSITY_SHIFT;
			*pPos = BlendPixel(*pPos, wColor, (iWeighting ^ WU_WEIGHTING_COMPLEMENT_MASK));

			WORD *pComplementPos = pPos + xDir;
			*pComplementPos = BlendPixel(*pComplementPos, wColor, iWeighting);
			}
		}
	else
		{
		//	X-major line

		int iErrorAdj = (yDelta << WU_ERROR_BITS) / xDelta;

		//	Draw all pixels except the first and last

		while (--xDelta)
			{
			//	Add the error accumulator and see if we've run over

			iErrorAcc += iErrorAdj;
			if (iErrorAcc > 0xFFFF)
				{
				y0++;
				iErrorAcc &= 0xFFFF;

				if (pPos)
					{
					if (!InClipY(y0))
						return;
					pPos = NextRow(pPos);
					}
				}

			x0 += xDir;	//	X-major, so always advance
			if (pPos)
				{
				if (!InClipX(x0))
					return;
				pPos = pPos + xDir;
				}

			//	Make sure we're in range

			if (pPos == NULL)
				{
				if (InClipX(x0) && InClipY(y0))
					pPos = GetPixel(GetRowStart(y0), x0);
				else
					continue;
				}

			//	The most significant bits of ErrorAcc give us the
			//	intensity weighting for this pixel, and the complement
			//	of the weighting for the paired pixel

			int iWeighting = iErrorAcc >> WU_INTENSITY_SHIFT;
			*pPos = BlendPixel(*pPos, wColor, (iWeighting ^ WU_WEIGHTING_COMPLEMENT_MASK));

			WORD *pComplementPos = NextRow(pPos);
			*pComplementPos = BlendPixel(*pComplementPos, wColor, iWeighting);
			}
		}

	//	Now draw the final pixel

	if (InClipX(x1) && InClipY(y1))
		*GetPixel(GetRowStart(y1), x1) = wColor;
	}

void CG16bitImage::DrawDot (int x, int y, WORD wColor, MarkerTypes iMarker)

//	DrawDot
//
//	Paints a dot in various styles

	{
	switch (iMarker)
		{
		case markerPixel:
			{
			DrawPixel(x, y, wColor);
			break;
			}

		case markerSmallRound:
			{
			int xStart, xEnd, yStart, yEnd;

			x = x - 1;
			y = y - 1;

			xStart = max(m_rcClip.left, x);
			xEnd = min(m_rcClip.right, x + 3);
			yStart = max(m_rcClip.top, y);
			yEnd = min(m_rcClip.bottom, y + 3);

			for (int yPos = yStart; yPos < yEnd; yPos++)
				{
				WORD *pPos = GetPixel(GetRowStart(yPos), xStart);

				for (int xPos = xStart; xPos < xEnd; xPos++)
					{
					BYTE byTrans = g_wSmallRoundMask[(yPos - y) * 3 + xPos - x];

					if (byTrans == 255)
						*pPos = wColor;
					else if (byTrans == 0)
						NULL;
					else
						*pPos = BlendPixel(*pPos, wColor, byTrans);

					pPos++;
					}

				pPos = NextRow(pPos);
				}

			break;
			}

		case markerSmallCross:
			{
			int yTop = y - SMALL_SQUARE_SIZE;
			int yBottom = y + SMALL_SQUARE_SIZE;
			int xLeft = x - SMALL_SQUARE_SIZE;
			int xRight = x + SMALL_SQUARE_SIZE;

			//	Draw the horizontal line

			if (y >= m_rcClip.top && y < m_rcClip.bottom)
				{
				WORD *pRowStart = GetRowStart(y);
				WORD *pPos = pRowStart + max(m_rcClip.left, xLeft);
				WORD *pPosEnd = pRowStart + min(xRight+1, m_rcClip.right);

				while (pPos < pPosEnd)
					*pPos++ = wColor;
				}

			//	Draw the vertical line

			if (x >= m_rcClip.left && x < m_rcClip.right)
				{
				WORD *pPos = GetRowStart(max(yTop+1, m_rcClip.top)) + x;
				WORD *pPosEnd = GetRowStart(min(yBottom, m_rcClip.bottom)) + x;

				while (pPos < pPosEnd)
					{
					*pPos = wColor;
					pPos = NextRow(pPos);
					}
				}

			break;
			}

		case markerMediumCross:
			{
			int yTop = y - MEDIUM_SQUARE_SIZE;
			int yBottom = y + MEDIUM_SQUARE_SIZE;
			int xLeft = x - MEDIUM_SQUARE_SIZE;
			int xRight = x + MEDIUM_SQUARE_SIZE;

			//	Draw the horizontal line

			if (y >= m_rcClip.top && y < m_rcClip.bottom)
				{
				WORD *pRowStart = GetRowStart(y);
				WORD *pPos = pRowStart + max(m_rcClip.left, xLeft);
				WORD *pPosEnd = pRowStart + min(xRight+1, m_rcClip.right);

				while (pPos < pPosEnd)
					*pPos++ = wColor;
				}

			//	Draw the vertical line

			if (x >= m_rcClip.left && x < m_rcClip.right)
				{
				WORD *pPos = GetRowStart(max(yTop, m_rcClip.top)) + x;
				WORD *pPosEnd = GetRowStart(min(yBottom+1, m_rcClip.bottom)) + x;

				while (pPos < pPosEnd)
					{
					*pPos = wColor;
					pPos = NextRow(pPos);
					}
				}

			break;
			}

		case markerSmallFilledSquare:
			{
			int xLeft = x - SMALL_SQUARE_SIZE;
			int yTop = y - SMALL_SQUARE_SIZE;
			WORD wInsideColor = BlendPixel(0, wColor, 192);

			DrawDot(x, y, wColor, markerSmallSquare);
			Fill(xLeft + 1, yTop + 1, 2 * SMALL_SQUARE_SIZE - 1, 2 * SMALL_SQUARE_SIZE - 1, wInsideColor);
			break;
			}

		case markerSmallCircle:
			{
			WORD wFade1 = BlendPixel(0, wColor, 192);
			WORD wFade2 = BlendPixel(0, wColor, 128);

			DrawPixel(x - 2, y, wColor);
			DrawPixel(x + 2, y, wColor);
			DrawPixel(x, y - 2, wColor);
			DrawPixel(x, y + 2, wColor);

			DrawPixel(x - 1, y - 1, wFade2);
			DrawPixel(x - 1, y + 1, wFade2);
			DrawPixel(x + 1, y - 1, wFade2);
			DrawPixel(x + 1, y + 1, wFade2);

			DrawPixel(x - 1, y - 2, wFade1);
			DrawPixel(x + 1, y - 2, wFade1);
			DrawPixel(x - 1, y + 2, wFade1);
			DrawPixel(x + 1, y + 2, wFade1);
			DrawPixel(x + 2, y - 1, wFade1);
			DrawPixel(x + 2, y + 1, wFade1);
			DrawPixel(x - 2, y - 1, wFade1);
			DrawPixel(x - 2, y + 1, wFade1);
			break;
			}

		case markerTinyCircle:
			{
			DrawPixel(x - 1, y, wColor);
			DrawPixel(x - 1, y + 1, wColor);
			DrawPixel(x + 2, y, wColor);
			DrawPixel(x + 2, y + 1, wColor);

			DrawPixel(x, y - 1, wColor);
			DrawPixel(x, y + 2, wColor);
			DrawPixel(x + 1, y - 1, wColor);
			DrawPixel(x + 1, y + 2, wColor);
			break;
			}

		case markerSmallSquare:
			{
			int yTop = y - SMALL_SQUARE_SIZE;
			int yBottom = y + SMALL_SQUARE_SIZE;
			int xLeft = x - SMALL_SQUARE_SIZE;
			int xRight = x + SMALL_SQUARE_SIZE;

			//	Draw the top line

			if (yTop >= m_rcClip.top && yTop < m_rcClip.bottom)
				{
				WORD *pRowStart = GetRowStart(yTop);
				WORD *pPos = pRowStart + max(m_rcClip.left, xLeft);
				WORD *pPosEnd = pRowStart + min(xRight+1, m_rcClip.right);

				while (pPos < pPosEnd)
					*pPos++ = wColor;
				}

			//	Draw the bottom line

			if (yBottom >= m_rcClip.top && yBottom < m_rcClip.bottom)
				{
				WORD *pRowStart = GetRowStart(yBottom);
				WORD *pPos = pRowStart + max(m_rcClip.left, xLeft);
				WORD *pPosEnd = pRowStart + min(xRight+1, m_rcClip.right);

				while (pPos < pPosEnd)
					*pPos++ = wColor;
				}

			//	Draw the left line

			if (xLeft >= m_rcClip.left && xLeft < m_rcClip.right)
				{
				WORD *pPos = GetRowStart(max(yTop+1, m_rcClip.top)) + xLeft;
				WORD *pPosEnd = GetRowStart(min(yBottom, m_rcClip.bottom)) + xLeft;

				while (pPos < pPosEnd)
					{
					*pPos = wColor;
					pPos = NextRow(pPos);
					}
				}

			//	Draw the right line

			if (xRight >= m_rcClip.left && xRight < m_rcClip.right)
				{
				WORD *pPos = GetRowStart(max(yTop+1, m_rcClip.top)) + xRight;
				WORD *pPosEnd = GetRowStart(min(yBottom, m_rcClip.bottom)) + xRight;

				while (pPos < pPosEnd)
					{
					*pPos = wColor;
					pPos = NextRow(pPos);
					}
				}

			break;
			}
		}
	}

void DrawDottedLine (CG16bitImage &Dest, int x1, int y1, int x2, int y2, WORD wColor)

//	DrawDottedLine
//
//	Draw a dotted line

	{
	int dx = x2 - x1;
	int ax = Absolute(dx) * 2;
	int sx = (dx > 0 ? 1 : -1);

	int dy = y2 - y1;
	int ay = Absolute(dy) * 2;
	int sy = (dy > 0 ? 1 : -1);

	int x = x1;
	int y = y1;
	int d;

	if (ax > ay)		//	x dominant
		{
		d = ay - ax / 2;
		while (true)
			{
			if ((x % 2) == 0)
				Dest.DrawPixel(x, y, wColor);

			if (x == x2)
				return;
			else if (d >= 0)
				{
				y = y + sy;
				d = d - ax;
				}

			x = x + sx;
			d = d + ay;
			}
		}
	else				//	y dominant
		{
		d = ax - ay / 2;
		while (true)
			{
			if ((y % 2) == 0)
				Dest.DrawPixel(x, y, wColor);

			if (y == y2)
				return;
			else if (d >= 0)
				{
				x = x + sx;
				d = d - ay;
				}

			y = y + sy;
			d = d + ax;
			}
		}
	}

void DrawRectDotted (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, WORD wColor)

//	DrawRectDotted
//
//	Draws a dotted rect outline

	{
	DrawDottedLine(Dest, x, y, x + cxWidth, y, wColor);
	DrawDottedLine(Dest, x, y + cyHeight, x + cxWidth, y + cyHeight, wColor);
	DrawDottedLine(Dest, x, y, x, y + cyHeight, wColor);
	DrawDottedLine(Dest, x + cxWidth, y, x + cxWidth, y + cyHeight, wColor);
	}

void DrawRoundedRect (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, int iRadius, WORD wColor)

//	DrawRoundedRect
//
//	Draws a filled rect with rounded corners.

	{
	int i;

	if (iRadius <= 0)
		{
		Dest.Fill(x, y, cxWidth, cyHeight, wColor);
		return;
		}

	//	Generate a set of raster lines for the corner

	int *pSolid = new int [iRadius];
	BYTE *pEdge = new BYTE [iRadius];
	RasterizeQuarterCircle8bit(iRadius, pSolid, pEdge);

	//	Fill in each corner

	for (i = 0; i < iRadius; i++)
		{
		int xOffset = iRadius - pSolid[i];
		int cxLine = cxWidth - (iRadius * 2) + (pSolid[i] * 2);

		//	Top edge

		Dest.FillLine(x + xOffset, y + i, cxLine, wColor);
		Dest.SetPixelTrans(x + xOffset - 1, y + i, wColor, pEdge[i]);
		Dest.SetPixelTrans(x + cxWidth - xOffset, y + i, wColor, pEdge[i]);

		//	Bottom edge

		Dest.FillLine(x + xOffset, y + cyHeight - i - 1, cxLine, wColor);
		Dest.SetPixelTrans(x + xOffset - 1, y + cyHeight - i - 1, wColor, pEdge[i]);
		Dest.SetPixelTrans(x + cxWidth - xOffset, y + cyHeight - i - 1, wColor, pEdge[i]);
		}

	//	Fill the center

	Dest.Fill(x, y + iRadius, cxWidth, (cyHeight - 2 * iRadius), wColor);

	//	Done

	delete [] pSolid;
	delete [] pEdge;
	}

void DrawRoundedRectTrans (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, int iRadius, WORD wColor, DWORD byOpacity)

//	DrawRoundedRectTrans
//
//	Draws a filled rect with rounded corners.

	{
	int i;

	if (byOpacity == 255)
		{
		DrawRoundedRect(Dest, x, y, cxWidth, cyHeight, iRadius, wColor);
		return;
		}

	if (iRadius <= 0)
		{
		Dest.FillTrans(x, y, cxWidth, cyHeight, wColor, byOpacity);
		return;
		}

	//	Generate a set of raster lines for the corner

	int *pSolid = new int [iRadius];
	BYTE *pEdge = new BYTE [iRadius];
	RasterizeQuarterCircle8bit(iRadius, pSolid, pEdge, byOpacity);

	//	Fill in each corner

	for (i = 0; i < iRadius; i++)
		{
		int xOffset = iRadius - pSolid[i];
		int cxLine = cxWidth - (iRadius * 2) + (pSolid[i] * 2);

		//	Top edge

		Dest.FillTrans(x + xOffset, y + i, cxLine, 1, wColor, byOpacity);
		Dest.SetPixelTrans(x + xOffset - 1, y + i, wColor, pEdge[i]);
		Dest.SetPixelTrans(x + cxWidth - xOffset, y + i, wColor, pEdge[i]);

		//	Bottom edge

		Dest.FillTrans(x + xOffset, y + cyHeight - i - 1, cxLine, 1, wColor, byOpacity);
		Dest.SetPixelTrans(x + xOffset - 1, y + cyHeight - i - 1, wColor, pEdge[i]);
		Dest.SetPixelTrans(x + cxWidth - xOffset, y + cyHeight - i - 1, wColor, pEdge[i]);
		}

	//	Fill the center

	Dest.FillTrans(x, y + iRadius, cxWidth, (cyHeight - 2 * iRadius), wColor, byOpacity);

	//	Done

	delete [] pSolid;
	delete [] pEdge;
	}

void DrawRoundedRectOutline (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, int iRadius, int iLineWidth, WORD wColor)

//	DrawRoundedRectOutline
//
//	Draws the outline of a rounded rectangle.

	{
	//	Range checking

	if (iLineWidth <= 0)
		return;

	iRadius = Min(Min(Max(0, iRadius), cxWidth / 2), cyHeight / 2);

	//	Intermediates

	int iHalfWidth = iLineWidth / 2;

	//	Paint the straight edges first.

	Dest.Fill(x + iRadius, y - iHalfWidth, cxWidth - (2 * iRadius), iLineWidth, wColor);
	Dest.Fill(x + iRadius, y + cyHeight - iHalfWidth, cxWidth - (2 * iRadius), iLineWidth, wColor);
	Dest.Fill(x - iHalfWidth, y + iRadius, iLineWidth, cyHeight - (2 * iRadius), wColor);
	Dest.Fill(x + cxWidth - iHalfWidth, y + iRadius, iLineWidth, cyHeight - (2 * iRadius), wColor);

	//	If no radius, then we're done

	if (iRadius == 0)
		return;

	//	Paint the corners

	DrawArc(Dest, x + iRadius, y + iRadius, iRadius, 90, 180, iLineWidth, wColor);
	DrawArc(Dest, x + cxWidth - iRadius, y + iRadius, iRadius, 0, 90, iLineWidth, wColor);
	DrawArc(Dest, x + iRadius, y + cyHeight - iRadius, iRadius, 180, 270, iLineWidth, wColor);
	DrawArc(Dest, x + cxWidth - iRadius, y + cyHeight - iRadius, iRadius, 270, 360, iLineWidth, wColor);
	}
