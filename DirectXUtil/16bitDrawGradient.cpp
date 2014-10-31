//	16bitDrawGradient.cpp
//
//	Drawing routines for gradients
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

#include <math.h>
#include <stdio.h>
#include "NoiseImpl.h"

void DrawGradientRectHorz (CG16bitImage &Dest,
						   int xDest,
						   int yDest,
						   int cxDest,
						   int cyDest,
						   WORD wStartColor,
						   WORD wEndColor,
						   DWORD dwStartOpacity,
						   DWORD dwEndOpacity)

//	DrawGradientRectHorz
//
//	Draws a horizontal gradient from left to right.

	{
	//	We use a stochastic opacity to deal with gradient artifacts.

	InitStochasticTable();

	//	Make sure we're in bounds

	int x = xDest;
	int y = yDest;
	int cxWidth = cxDest;
	int cyHeight = cyDest;
	if (!Dest.AdjustCoords(NULL, NULL, 0, 0, 
			&x, &y, 
			&cxWidth, &cyHeight))
		return;

	ASSERT(cxDest > 0);
	ASSERT(x >= xDest);

	//	Draw the gradient horizontally, column by column, from left to right.
	//	The left-most column is the start color; the right-most column is the
	//	end color.

	Metric rStep = (cxDest == 1 ? 0.0 : 1.0 / (cxDest - 1));
	Metric rGradient = rStep * (x - xDest);
	Metric rOpacityRange = ((Metric)dwEndOpacity - (Metric)dwStartOpacity);

	WORD *pPos = Dest.GetRowStart(y) + x;
	WORD *pPosEnd = pPos + cxWidth;
	WORD *pColEnd = Dest.GetRowStart(y + cyHeight) + x;
	int iRowSize = (Dest.GetRowStart(1) - Dest.GetRowStart(0));
	int iColSize = pColEnd - pPos;

	int xStochastic = x;

	while (pPos < pPosEnd)
		{
		//	Compute the color of this column

		DWORD dwColorGradient = (DWORD)(255.0 * rGradient);
		WORD wColor = CG16bitImage::BlendPixel(wStartColor, wEndColor, dwColorGradient);

		//	Compute the opacity of this column

		int iBaseIndex = (xStochastic % STOCHASTIC_DIM) * STOCHASTIC_DIM;
		DWORD dwOpacity = (DWORD)(dwStartOpacity + (rOpacityRange * rGradient));

		//	Draw the column

		WORD *pCol = pPos;
		WORD *pColEnd = pPos + iColSize;
		if (dwOpacity == 0)
			;
		else if (dwOpacity == 255)
			{
			while (pCol < pColEnd)
				{
				*pCol = wColor;
				pCol += iRowSize;
				}
			}
		else
			{
			int yStochastic = 0;

			while (pCol < pColEnd)
				{
				DWORD dwJitterOpacity = STOCHASTIC_OPACITY[iBaseIndex + (yStochastic % STOCHASTIC_DIM)][dwOpacity];

				*pCol = CG16bitImage::BlendPixel(*pCol, wColor, dwJitterOpacity);

				pCol += iRowSize;
				yStochastic++;
				}
			}

		//	Next

		xStochastic++;
		rGradient += rStep;
		pPos++;
		}
	}
