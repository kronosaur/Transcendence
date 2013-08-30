//	CAniImageFill.cpp
//
//	CAniImageFill class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

#include "Reanimator.h"

#define PROP_OPACITY						CONSTLIT("opacity")

CAniImageFill::CAniImageFill (const CG16bitImage *pImage, bool bFreeImage) :
		m_pImage(pImage),
		m_bFreeImage(bFreeImage),
		m_dwOpacity(255),
		m_xOrigin(0),
		m_yOrigin(0)

//	CAniImageFill constructor

	{
	if (m_pImage)
		{
		if (m_pImage->GetWidth() == 0 || m_pImage->GetHeight() == 0)
			{
			if (bFreeImage)
				delete m_pImage;

			m_pImage = NULL;
			}
		}
	}

CAniImageFill::~CAniImageFill (void)

//	CAniImageFill destructor

	{
	if (m_pImage && m_bFreeImage)
		delete m_pImage;
	}

void CAniImageFill::CalcTile (int x, int y, int *retxTile, int *retyTile)

//	CalcTile
//
//	Calculates the upper-left coords of the source image.

	{
	int xTile = x - m_xOrigin;
	if (xTile >= 0)
		xTile = xTile % m_pImage->GetWidth();
	else
		xTile = m_pImage->GetWidth() - ((-xTile) % m_pImage->GetWidth());

	int yTile = y - m_yOrigin;
	if (yTile >= 0)
		yTile = yTile % m_pImage->GetHeight();
	else
		yTile = m_pImage->GetHeight() - ((-yTile) % m_pImage->GetHeight());

	//	Done

	*retxTile = xTile;
	*retyTile = yTile;
	}

void CAniImageFill::Fill (SAniPaintCtx &Ctx, int x, int y, int cxWidth, int cyHeight)

//	Fill
//
//	Fill

	{
	if (m_pImage == NULL)
		return;

	int xTile;
	int yTile;
	CalcTile(x, y, &xTile, &yTile);

	int yDest = y;
	int yLeft = cyHeight;
	while (yLeft)
		{
		int cyTile = Min(yLeft, m_pImage->GetHeight() - yTile);

		int xDest = x;
		int xLeft = cxWidth;
		while (xLeft)
			{
			int cxTile = Min(xLeft, m_pImage->GetWidth() - xTile);

			Ctx.Dest.ColorTransBlt(xTile, yTile, cxTile, cyTile, m_dwOpacity, *m_pImage, xDest, yDest);

			xDest += cxTile;
			xLeft -= cxTile;
			xTile = 0;
			}

		yDest += cyTile;
		yLeft -= cyTile;
		yTile = 0;
		}
	}

void CAniImageFill::Fill (SAniPaintCtx &Ctx, int x, int y, const TArray<SSimpleRasterLine> &Lines)

//	Fill
//
//	Files the set of lines

	{
	if (m_pImage == NULL)
		return;

	SSimpleRasterLine *pLine = &Lines[0];
	SSimpleRasterLine *pLineEnd = pLine + Lines.GetCount();

	while (pLine < pLineEnd)
		{
		int xDest = x + pLine->x;
		int yDest = y + pLine->y;

		int xTile;
		int yTile;
		CalcTile(xDest, yDest, &xTile, &yTile);

		int xSolidTile = xTile;
		int xLeft = pLine->cxLength;
		while (xLeft)
			{
			int cxTile = Min(xLeft, m_pImage->GetWidth() - xTile);

			Ctx.Dest.ColorTransBlt(xSolidTile, yTile, cxTile, 1, m_dwOpacity, *m_pImage, xDest, yDest);

			xDest += cxTile;
			xLeft -= cxTile;
			xSolidTile = 0;
			}

		//	Edges

		if (pLine->byLeftEdge)
			{
			int xSrc = (xTile > 0 ? xTile - 1 : m_pImage->GetWidth() - 1);

			DWORD dwOpacity = m_dwOpacity * pLine->byLeftEdge / 255;
			WORD wColor = *m_pImage->GetPixel(m_pImage->GetRowStart(yTile), xSrc);
			Ctx.Dest.SetPixelTrans(x + pLine->x - 1, yDest, wColor, dwOpacity);
			}

		if (pLine->byRightEdge)
			{
			int xSrc = (x + pLine->cxLength) % m_pImage->GetWidth();

			DWORD dwOpacity = m_dwOpacity * pLine->byRightEdge / 255;
			WORD wColor = *m_pImage->GetPixel(m_pImage->GetRowStart(yTile), xSrc);
			Ctx.Dest.SetPixelTrans(x + pLine->x + pLine->cxLength, yDest, wColor, dwOpacity);
			}

		pLine++;
		}
	}

void CAniImageFill::Fill (SAniPaintCtx &Ctx, int x, int y, const CG16bitBinaryRegion &Region)

//	Fill
//
//	Fills the region

	{
	if (m_pImage == NULL)
		return;

	//	LATER
	}

void CAniImageFill::InitDefaults (CAniPropertySet &Properties)

//	InitDefaults
//
//	Initializes animatron properties to defaults

	{
	}

void CAniImageFill::InitPaint (SAniPaintCtx &Ctx, int xOrigin, int yOrigin, CAniPropertySet &Properties)

//	InitPaint
//
//	Initializes internal cache from properties

	{
	m_dwOpacity = Properties.GetOpacity(PROP_OPACITY) * Ctx.dwOpacityToDest / 255;
	m_xOrigin = xOrigin;
	m_yOrigin = yOrigin;
	}
