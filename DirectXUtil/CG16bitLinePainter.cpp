//	CG16bitLinePainter.cpp
//
//	CG16bitLinePainter class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

CG16bitLinePainter::ESlopeTypes CG16bitLinePainter::CalcIntermediates (const CG16bitImage &Image, int x1, int y1, int x2, int y2, int iWidth)

//	CalcIntermediates
//
//	Computes all intermediate variables in preparation for looping.
//	Returns the type of line.

	{
	int i;

	//	Calculate the slope

	m_dx = x2 - x1;
	m_ax = Absolute(m_dx) * 2;
	m_sx = (m_dx > 0 ? 1 : -1);

	m_dy = y2 - y1;
	m_ay = Absolute(m_dy) * 2;
	m_sy = (m_dy > 0 ? 1 : -1);

	m_rL = sqrt((double)(m_dx*m_dx + m_dy*m_dy));
	m_rHalfWidth = (double)iWidth / 2.0;

	//	Calculate row increment

	m_iPosRowInc = Image.GetRowStart(1) - Image.GetRowStart(0);

	//	If a zero-length line, then we're done.

	if (m_dx == 0 && m_dy == 0)
		{
		const RECT &rcDest = Image.GetClipRect();

		if (x1 >= rcDest.left && x1 < rcDest.right
				&& y1 >= rcDest.top && y1 < rcDest.bottom)
			return linePoint;
		else
			return lineNull;
		}

	//	Otherwise, check slope. In this case we are x-axis dominant.

	else if (m_ax > m_ay)
		{
		//	Compute the distance from the mid-point of the line to
		//	the top edge along the y-axis.

		double rHalfHeight = m_rHalfWidth * m_rL / (double)Absolute(m_dx);
		m_iAxisHalfWidth = (int)(((2.0 * rHalfHeight) - 1.0) / 2.0);
		m_iAxisWidth = 2 * m_iAxisHalfWidth + 1;

		m_d = m_ay - m_ax / 2;

		//	Compute information for anti-aliasing. rDist is the distance
		//	from the top solid point to the edge of the line. We update
		//	the value as we proceed along the x-axis and use it for
		//	anti-aliasing the edge of the line. A negative value means
		//	that the top point is outside the edge.

		double rSlope = (double)m_dy / (double)m_dx;
		m_rWUp = rHalfHeight - (double)m_iAxisHalfWidth - 0.25;
		m_rWDown = m_rWUp;

		m_rWUpInc = m_sy;
		m_rWUpDec = rSlope * m_sx;
		m_rWDownInc = rSlope * m_sx;
		m_rWDownDec = m_sy;

		//	Compute boundaries

		const RECT &rcDest = Image.GetClipRect();

		//	x1 <= x2

		if (m_sx == 1)
			{
			if (x1 >= rcDest.right)
				return lineNull;
			else if (x2 <= rcDest.left)
				return lineNull;

			if (x1 < rcDest.left)
				{
				int iExtra = rcDest.left - x1;
				int x = 0;

				m_yStart = y1;
				for (i = 0; i < iExtra; i++)
					NextX(x, m_yStart);

				m_xStart = rcDest.left;
				}
			else
				{
				m_xStart = x1;
				m_yStart = y1;
				}

			if (x2 > rcDest.right)
				m_xEnd = rcDest.right;
			else
				m_xEnd = x2;
			}

		//	x1 > x2

		else
			{
			if (x2 >= rcDest.right)
				return lineNull;
			else if (x1 <= rcDest.left)
				return lineNull;

			if (x1 >= rcDest.right)
				{
				int iExtra = x1 - (rcDest.right - 1);
				int x = 0;

				m_yStart = y1;
				for (i = 0; i < iExtra; i++)
					NextX(x, m_yStart);

				m_xStart = rcDest.right - 1;
				}
			else
				{
				m_xStart = x1;
				m_yStart = y1;
				}

			if (x2 < rcDest.left)
				m_xEnd = rcDest.left;
			else
				m_xEnd = x2;
			}

		m_wMin = rcDest.top;
		m_wMax = rcDest.bottom;

		return lineXDominant;
		}

	//	y-axis dominant

	else
		{
		//	Compute the distance from the mid-point of the line to
		//	the left-edge of the x-axis

		double rHalfHeight = m_rHalfWidth * m_rL / (double)Absolute(m_dy);
		m_iAxisHalfWidth = (int)(((2 * rHalfHeight) - 1) / 2);
		m_iAxisWidth = 2 * m_iAxisHalfWidth + 1;

		m_d = m_ax - m_ay / 2;

		//	Compute information for anti-aliasing. rDist is the distance
		//	from the top solid point to the edge of the line. We update
		//	the value as we proceed along the x-axis and use it for
		//	anti-aliasing the edge of the line. A negative value means
		//	that the top point is outside the edge.

		double rSlope = (double)m_dx / (double)m_dy;
		m_rWUp = rHalfHeight - (double)m_iAxisHalfWidth - 0.25;
		m_rWDown = m_rWUp;

		m_rWUpInc = m_sx;
		m_rWUpDec = rSlope * m_sy;
		m_rWDownInc = rSlope * m_sy;
		m_rWDownDec = m_sx;

		//	Compute boundaries

		const RECT &rcDest = Image.GetClipRect();

		//	y1 <= y2

		if (m_sy == 1)
			{
			if (y1 >= rcDest.bottom)
				return lineNull;
			else if (y2 <= rcDest.top)
				return lineNull;

			if (y1 < rcDest.top)
				{
				int iExtra = rcDest.top - y1;
				int y = 0;

				m_xStart = x1;
				for (i = 0; i < iExtra; i++)
					NextY(m_xStart, y);

				m_yStart = rcDest.top;
				}
			else
				{
				m_xStart = x1;
				m_yStart = y1;
				}

			if (y2 > rcDest.bottom)
				m_yEnd = rcDest.bottom;
			else
				m_yEnd = y2;
			}

		//	y1 > y2

		else
			{
			if (y2 >= rcDest.bottom)
				return lineNull;
			else if (y1 <= rcDest.top)
				return lineNull;

			if (y1 >= rcDest.bottom)
				{
				int iExtra = y1 - (rcDest.bottom - 1);
				int y = 0;

				m_xStart = x1;
				for (i = 0; i < iExtra; i++)
					NextX(m_xStart, y);

				m_yStart = rcDest.bottom - 1;
				}
			else
				{
				m_xStart = x1;
				m_yStart = y1;
				}

			if (y2 < rcDest.top)
				m_yEnd = rcDest.top;
			else
				m_yEnd = y2;
			}

		m_wMin = rcDest.left;
		m_wMax = rcDest.right;

		return lineYDominant;
		}
	}

void CG16bitLinePainter::CalcPixelMapping (int x1, int y1, int x2, int y2, double *retrV, double *retrW)

//	CalcPixelMapping
//
//	Initializes m_rVInc, m_rWInc, etc. Must be called after CalcIntermediates.

	{
	m_rVPerX = (double)m_dx / (m_rL * m_rL);
	m_rWPerX = (double)-m_dy / (m_rHalfWidth * m_rL);
	m_rVPerY = (double)m_dy / (m_rL * m_rL);
	m_rWPerY = (double)m_dx / (m_rHalfWidth * m_rL);

	m_rVIncX = (m_sx == 1 ? m_rVPerX : -m_rVPerX);
	m_rWIncX = (m_sx == 1 ? m_rWPerX : -m_rWPerX);
	m_rVIncY = (m_sy == 1 ? m_rVPerY : -m_rVPerY);
	m_rWIncY = (m_sy == 1 ? m_rWPerY : -m_rWPerY);

	double rX = (m_xStart - x1);
	double rY = (m_yStart - y1);

	*retrV = (rX * m_rVPerX) + (rY * m_rVPerY);
	*retrW = (rX * m_rWPerX) + (rY * m_rWPerY);
	}

void CG16bitLinePainter::PaintSolid (CG16bitImage &Image, int x1, int y1, int x2, int y2, int iWidth, WORD wColor)

//	PaintSolid
//
//	Paints a solid line

	{
	int x, y;

	//	Calculate the line type and paint accordingly

	switch (CalcIntermediates(Image, x1, y1, x2, y2, iWidth))
		{
		case lineNull:
			break;

		case linePoint:
			Image.DrawPixel(x1, y1, wColor);
			break;

		case lineXDominant:
			for (x = m_xStart, y = m_yStart - m_iAxisHalfWidth - 1; x != m_xEnd; x += m_sx)
				{
				int w = y;
				int wLast = w + m_iAxisWidth + 1;
				WORD *pPos = Image.GetPixel(Image.GetRowStart(w), x);

				//	Draw anti-aliasing above the line

				if (m_rWUp > 0.0 && w >= m_wMin && w < m_wMax)
					*pPos = Image.BlendPixel(*pPos, wColor, (BYTE)(255.0 * (m_rWUp > 1.0 ? 1.0 : m_rWUp)));

				w++;
				pPos += m_iPosRowInc;

				//	Draw the solid part of the line

				for (; w < wLast; w++, pPos += m_iPosRowInc)
					{
					if (w >= m_wMin && w < m_wMax)
						*pPos = wColor;
					}

				//	Draw anti-aliasing below the line

				if (m_rWDown > 0 && w >= m_wMin && w < m_wMax)
					*pPos = Image.BlendPixel(*pPos, wColor, (BYTE)(255.0 * (m_rWDown > 1.0 ? 1.0 : m_rWDown)));

				//	Next point

				NextX(x, y);
				}
			break;

		case lineYDominant:
			for (y = m_yStart, x = m_xStart - m_iAxisHalfWidth - 1; y != m_yEnd; y += m_sy)
				{
				int w = x;
				int wLast = w + m_iAxisWidth + 1;
				WORD *pPos = Image.GetPixel(Image.GetRowStart(y), w);

				//	Draw anti-aliasing to the left

				if (m_rWUp > 0 && w >= m_wMin && w < m_wMax)
					*pPos = Image.BlendPixel(*pPos, wColor, (BYTE)(255.0 * (m_rWUp > 1.0 ? 1.0 : m_rWUp)));

				w++;
				pPos++;

				//	Draw the solid part of the line

				for (; w < wLast; w++, pPos++)
					{
					if (w >= m_wMin && w < m_wMax)
						*pPos = wColor;
					}

				//	Draw anti-aliasing to the right of the line

				if (m_rWDown > 0 && w >= m_wMin && w < m_wMax)
					*pPos = Image.BlendPixel(*pPos, wColor, (BYTE)(255.0 * (m_rWDown > 1.0 ? 1.0 : m_rWDown)));

				//	Next point

				NextY(x, y);
				}
			break;
		}
	}

void CG16bitLinePainter::Rasterize (CG16bitImage &Image, int x1, int y1, int x2, int y2, int iWidth, TArray<SPixelDesc> *retPixels)

//	Rasterize
//
//	Initializes the result array with info about each pixel to draw to form the
//	given line.

	{
	int x, y;

	retPixels->DeleteAll();

	//	Calculate the line type and paint accordingly

	switch (CalcIntermediates(Image, x1, y1, x2, y2, iWidth))
		{
		case lineNull:
			break;

		case linePoint:
			{
			SPixelDesc *pPixel = retPixels->Insert();
			pPixel->pPos = Image.GetRowStart(y1) + x1;
			pPixel->x = x1;
			pPixel->y = y1;
			pPixel->rV = 0.0;
			pPixel->rW = 0.0;
			break;
			}

		case lineXDominant:
			{
			double rV, rW;
			CalcPixelMapping(x1, y1, x2, y2, &rV, &rW);

			rV += (-m_iAxisHalfWidth - 1) * m_rVPerY;
			rW += (-m_iAxisHalfWidth - 1) * m_rWPerY;

			int iMaxPixelCount = Absolute(m_dx) * (m_iAxisWidth + 2);
			int iPixel = 0;
			retPixels->InsertEmpty(iMaxPixelCount);

			for (x = m_xStart, y = m_yStart - m_iAxisHalfWidth - 1; x != m_xEnd; x += m_sx)
				{
				int w = y;
				int wLast = w + m_iAxisWidth + 1;
				WORD *pPos = Image.GetPixel(Image.GetRowStart(w), x);

				double rInnerV = rV;
				double rInnerW = rW;

				//	Draw anti-aliasing above the line

				if (m_rWUp > 0.0 && w >= m_wMin && w < m_wMax)
					{
					SPixelDesc &Pixel = retPixels->GetAt(iPixel++);
					Pixel.pPos = pPos;
					Pixel.dwOpacity = (DWORD)(255.0 * (m_rWUp > 1.0 ? 1.0 : m_rWUp));
					Pixel.x = x;
					Pixel.y = w;
					Pixel.rV = rInnerV;
					Pixel.rW = rInnerW;
					}

				w++;
				pPos += m_iPosRowInc;
				rInnerV += m_rVPerY;
				rInnerW += m_rWPerY;

				//	Draw the solid part of the line

				for (; w < wLast; w++, pPos += m_iPosRowInc)
					{
					if (w >= m_wMin && w < m_wMax)
						{
						SPixelDesc &Pixel = retPixels->GetAt(iPixel++);
						Pixel.pPos = pPos;
						Pixel.dwOpacity = 255;
						Pixel.x = x;
						Pixel.y = w;
						Pixel.rV = rInnerV;
						Pixel.rW = rInnerW;
						}

					rInnerV += m_rVPerY;
					rInnerW += m_rWPerY;
					}

				//	Draw anti-aliasing below the line

				if (m_rWDown > 0 && w >= m_wMin && w < m_wMax)
					{
					SPixelDesc &Pixel = retPixels->GetAt(iPixel++);
					Pixel.pPos = pPos;
					Pixel.dwOpacity = (DWORD)(255.0 * (m_rWDown > 1.0 ? 1.0 : m_rWDown));
					Pixel.x = x;
					Pixel.y = w;

					//	Decrement because we are aliasing

					Pixel.rV = rInnerV;
					Pixel.rW = rInnerW;
					}

				//	Next point

				NextX(x, y, rV, rW);
				}

			retPixels->SetCount(iPixel);
			break;
			}

		case lineYDominant:
			{
			double rV, rW;
			CalcPixelMapping(x1, y1, x2, y2, &rV, &rW);

			rV += (-m_iAxisHalfWidth - 1) * m_rVPerX;
			rW += (-m_iAxisHalfWidth - 1) * m_rWPerX;

			int iMaxPixelCount = Absolute(m_dy) * (m_iAxisWidth + 2);
			int iPixel = 0;
			retPixels->InsertEmpty(iMaxPixelCount);

			for (y = m_yStart, x = m_xStart - m_iAxisHalfWidth - 1; y != m_yEnd; y += m_sy)
				{
				int w = x;
				int wLast = w + m_iAxisWidth + 1;
				WORD *pPos = Image.GetPixel(Image.GetRowStart(y), w);

				double rInnerV = rV;
				double rInnerW = rW;


				//	Draw anti-aliasing to the left

				if (m_rWUp > 0 && w >= m_wMin && w < m_wMax)
					{
					SPixelDesc &Pixel = retPixels->GetAt(iPixel++);
					Pixel.pPos = pPos;
					Pixel.dwOpacity = (DWORD)(255.0 * (m_rWUp > 1.0 ? 1.0 : m_rWUp));
					Pixel.x = w;
					Pixel.y = y;
					Pixel.rV = rInnerV;
					Pixel.rW = rInnerW;
					}

				w++;
				pPos++;
				rInnerV += m_rVPerX;
				rInnerW += m_rWPerX;

				//	Draw the solid part of the line

				for (; w < wLast; w++, pPos++)
					{
					if (w >= m_wMin && w < m_wMax)
						{
						SPixelDesc &Pixel = retPixels->GetAt(iPixel++);
						Pixel.pPos = pPos;
						Pixel.dwOpacity = 255;
						Pixel.x = w;
						Pixel.y = y;
						Pixel.rV = rInnerV;
						Pixel.rW = rInnerW;
						}

					rInnerV += m_rVPerX;
					rInnerW += m_rWPerX;
					}

				//	Draw anti-aliasing to the right of the line

				if (m_rWDown > 0 && w >= m_wMin && w < m_wMax)
					{
					SPixelDesc &Pixel = retPixels->GetAt(iPixel++);
					Pixel.pPos = pPos;
					Pixel.dwOpacity = (DWORD)(255.0 * (m_rWDown > 1.0 ? 1.0 : m_rWDown));
					Pixel.x = w;
					Pixel.y = y;
					Pixel.rV = rInnerV;
					Pixel.rW = rInnerW;
					}

				//	Next point

				NextY(x, y, rV, rW);
				}

			retPixels->SetCount(iPixel);
			break;
			}
		}
	}
