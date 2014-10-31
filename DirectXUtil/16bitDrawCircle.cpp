//	16bitDrawCircle.cpp
//
//	Drawing routines for circles

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

#include <math.h>
#include <stdio.h>
#include "NoiseImpl.h"

const DWORD RED_BLUE_COUNT =						(1 << 5);
const DWORD GREEN_COUNT =							(1 << 6);

BYTE STOCHASTIC_OPACITY[STOCHASTIC_SIZE][256];

static bool g_bStochasticInit = false;

//	Template Function ---------------------------------------------------------

template <class PAINTER> void DrawFilledCircle (PAINTER &Painter)
	{
	int iRadius = Painter.GetRadius();

	//	Deal with edge-conditions

	if (iRadius <= 0)
		return;

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	//	Draw central line

	Painter.DrawLine(iRadius, 0);

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

			Painter.DrawLine(x, y);
			iLastDraw = y;

			//	Next

			y--;
			}

		x++;

		//	Draw lines

		if (x != iLastDraw)
			Painter.DrawLine(y, x);
		}
	}

//	CompositeFilledCircleFromTable ---------------------------------------------

class CCompositeFilledCircleFromTable
	{
	public:
		CCompositeFilledCircleFromTable (CG16bitImage *pDest, int xDest, int yDest, int iRadius, WORD *pColorTable, BYTE *pOpacityTable);
		~CCompositeFilledCircleFromTable (void);

		void DrawLine (int x, int y);
		inline int GetRadius (void) const { return m_iRadius; }

	private:
		CG16bitImage *m_pDest;
		int m_xDest;
		int m_yDest;
		int m_iRadius;
		WORD *m_pColorTable;
		BYTE *m_pOpacityTable;

		DWORD *m_pRed;
		DWORD *m_pGreen;
		DWORD *m_pBlue;
	};

class CCompositeFilledCircleFromTableStochastic
	{
	public:
		CCompositeFilledCircleFromTableStochastic (CG16bitImage *pDest, int xDest, int yDest, int iRadius, WORD *pColorTable, BYTE *pOpacityTable);
		~CCompositeFilledCircleFromTableStochastic (void);

		void DrawLine (int x, int y);
		inline int GetRadius (void) const { return m_iRadius; }

	private:
		CG16bitImage *m_pDest;
		int m_xDest;
		int m_yDest;
		int m_iRadius;
		WORD *m_pColorTable;
		BYTE *m_pOpacityTable;

		DWORD *m_pRed;
		DWORD *m_pGreen;
		DWORD *m_pBlue;
	};

void CompositeFilledCircle (CG16bitImage &Dest, 
							int xDest, 
							int yDest, 
							int iRadius,
							WORD *pColorTable,
							BYTE *pOpacityTable,
							bool bStochastic)
	{
	if (bStochastic)
		{
		CCompositeFilledCircleFromTableStochastic Painter(&Dest, xDest, yDest, iRadius, pColorTable, pOpacityTable);
		InitStochasticTable();
		DrawFilledCircle(Painter);
		}
	else
		{
		CCompositeFilledCircleFromTable Painter(&Dest, xDest, yDest, iRadius, pColorTable, pOpacityTable);
		DrawFilledCircle(Painter);
		}
	}

CCompositeFilledCircleFromTable::CCompositeFilledCircleFromTable (CG16bitImage *pDest, int xDest, int yDest, int iRadius, WORD *pColorTable, BYTE *pOpacityTable) :
				m_pDest(pDest),
				m_xDest(xDest),
				m_yDest(yDest),
				m_iRadius(iRadius),
				m_pColorTable(pColorTable),
				m_pOpacityTable(pOpacityTable),
				m_pRed(NULL),
				m_pGreen(NULL),
				m_pBlue(NULL)
	{
	int i;

	if (iRadius > 0)
		{
		m_pRed = new DWORD [iRadius];
		m_pGreen = new DWORD [iRadius];
		m_pBlue = new DWORD [iRadius];
		for (i = 0; i < iRadius; i++)
			{
			WORD wColor = m_pColorTable[i];
			m_pRed[i] = (wColor >> 11) & 0x1f;
			m_pGreen[i] = (wColor >> 5) & 0x3f;
			m_pBlue[i] = wColor & 0x1f;
			}
		}
	}

CCompositeFilledCircleFromTable::~CCompositeFilledCircleFromTable (void)
	{
	if (m_pRed)
		delete [] m_pRed;

	if (m_pGreen)
		delete [] m_pGreen;

	if (m_pBlue)
		delete [] m_pBlue;
	}

void CCompositeFilledCircleFromTable::DrawLine (int x, int y)
	{
	int xStart = m_xDest - x;
	int xEnd = m_xDest + x + 1;
	const RECT &rcClip = m_pDest->GetClipRect();

	if (xEnd <= rcClip.left || xStart >= rcClip.right)
		return;

	//	See which lines we need to paint

	int yLine = m_yDest - y;
	bool bPaintTop = (yLine >= rcClip.top && yLine < rcClip.bottom);
	WORD *pCenterTop = m_pDest->GetRowStart(yLine) + m_xDest;
	BYTE *pCenterTopAlpha = m_pDest->GetAlphaRow(yLine) + m_xDest;

	yLine = m_yDest + y;
	bool bPaintBottom = ((y > 0) && (yLine >= rcClip.top && yLine < rcClip.bottom));
	WORD *pCenterBottom = m_pDest->GetRowStart(yLine) + m_xDest;
	BYTE *pCenterBottomAlpha = m_pDest->GetAlphaRow(yLine) + m_xDest;

	//	Compute radius increment

	int iRadius = y;
	int d = -y;
	int deltaE = 3;
	int deltaSE = -2 * y + 1;

	//	Loop

	int xPos = 0;

	//	Paint the center point (this is a special case)

	if (y == 0)
		{
		if (m_xDest < rcClip.right
				&& m_xDest >= rcClip.left
				&& bPaintTop
				&& m_iRadius > 0)
			{
			WORD dwOpacity = m_pOpacityTable[0];
			WORD wColor = m_pColorTable[0];

			if (dwOpacity == 255)
				{
				*pCenterTop = wColor;
				*pCenterTopAlpha = 255;
				}
			else
				{
				*pCenterTop = CG16bitImage::BlendPixel(*pCenterTop, wColor, dwOpacity);
				*pCenterTopAlpha = CG16bitImage::BlendAlpha(*pCenterTopAlpha, (BYTE)dwOpacity);
				}
			}

		//	Continue

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

		//	Compute the transparency based on the radius

		WORD dwOpacity;
		if (iRadius >= m_iRadius
				|| (dwOpacity = m_pOpacityTable[iRadius]) == 0)
			{
			//	Skip
			}
		else
			{
			if (dwOpacity == 255)
				{
				WORD wColor = m_pColorTable[iRadius];

				if (m_xDest - xPos < rcClip.right && m_xDest - xPos >= rcClip.left)
					{
					if (bPaintTop)
						{
						*(pCenterTop - xPos) = wColor;
						*(pCenterTopAlpha - xPos) = 255;
						}

					if (bPaintBottom)
						{
						*(pCenterBottom - xPos) = wColor;
						*(pCenterBottomAlpha - xPos) = 255;
						}
					}

				if (xPos > 0 && m_xDest + xPos < rcClip.right && m_xDest + xPos >= rcClip.left)
					{
					if (bPaintTop)
						{
						*(pCenterTop + xPos) = wColor;
						*(pCenterTopAlpha + xPos) = 255;
						}

					if (bPaintBottom)
						{
						*(pCenterBottom + xPos) = wColor;
						*(pCenterBottomAlpha + xPos) = 255;
						}
					}
				}
			else
				{
#define DRAW_PIXEL(pos, alphaPos)	\
					{ \
					WORD dwDest = *(pos);	\
					WORD dwRedDest = (dwDest >> 11) & 0x1f;	\
					WORD dwGreenDest = (dwDest >> 5) & 0x3f;	\
					WORD dwBlueDest = dwDest & 0x1f;	\
					\
					*(pos) = (WORD)((dwOpacity * (m_pBlue[iRadius] - dwBlueDest) >> 8) + dwBlueDest |	\
							((dwOpacity * (m_pGreen[iRadius] - dwGreenDest) >> 8) + dwGreenDest) << 5 |	\
							((dwOpacity * (m_pRed[iRadius] - dwRedDest) >> 8) + dwRedDest) << 11);	\
					\
					*(alphaPos) = CG16bitImage::BlendAlpha(*(alphaPos), (BYTE)dwOpacity);	\
					}

				if (m_xDest - xPos < rcClip.right && m_xDest - xPos >= rcClip.left)
					{
					if (bPaintTop)
						DRAW_PIXEL(pCenterTop - xPos, pCenterTopAlpha - xPos);

					if (bPaintBottom)
						DRAW_PIXEL(pCenterBottom - xPos, pCenterBottomAlpha - xPos);
					}

				if (xPos > 0 && m_xDest + xPos < rcClip.right && m_xDest + xPos >= rcClip.left)
					{
					if (bPaintTop)
						DRAW_PIXEL(pCenterTop + xPos, pCenterTopAlpha + xPos);

					if (bPaintBottom)
						DRAW_PIXEL(pCenterBottom + xPos, pCenterBottomAlpha + xPos);
					}
#undef DRAW_PIXEL
				}
			}

		xPos++;
		}
	}

//	CompositeFilledCircleFromTableStochastic -----------------------------------

CCompositeFilledCircleFromTableStochastic::CCompositeFilledCircleFromTableStochastic (CG16bitImage *pDest, int xDest, int yDest, int iRadius, WORD *pColorTable, BYTE *pOpacityTable) :
				m_pDest(pDest),
				m_xDest(xDest),
				m_yDest(yDest),
				m_iRadius(iRadius),
				m_pColorTable(pColorTable),
				m_pOpacityTable(pOpacityTable),
				m_pRed(NULL),
				m_pGreen(NULL),
				m_pBlue(NULL)
	{
	int i;

	if (iRadius > 0)
		{
		m_pRed = new DWORD [iRadius];
		m_pGreen = new DWORD [iRadius];
		m_pBlue = new DWORD [iRadius];
		for (i = 0; i < iRadius; i++)
			{
			WORD wColor = m_pColorTable[i];
			m_pRed[i] = (wColor >> 11) & 0x1f;
			m_pGreen[i] = (wColor >> 5) & 0x3f;
			m_pBlue[i] = wColor & 0x1f;
			}
		}
	}

CCompositeFilledCircleFromTableStochastic::~CCompositeFilledCircleFromTableStochastic (void)
	{
	if (m_pRed)
		delete [] m_pRed;

	if (m_pGreen)
		delete [] m_pGreen;

	if (m_pBlue)
		delete [] m_pBlue;
	}

void CCompositeFilledCircleFromTableStochastic::DrawLine (int x, int y)
	{
	int xStart = m_xDest - x;
	int xEnd = m_xDest + x + 1;
	const RECT &rcClip = m_pDest->GetClipRect();

	if (xEnd <= rcClip.left || xStart >= rcClip.right)
		return;

	//	See which lines we need to paint

	int yLine = m_yDest - y;
	bool bPaintTop = (yLine >= rcClip.top && yLine < rcClip.bottom);
	WORD *pCenterTop = m_pDest->GetRowStart(yLine) + m_xDest;
	BYTE *pCenterTopAlpha = m_pDest->GetAlphaRow(yLine) + m_xDest;

	yLine = m_yDest + y;
	bool bPaintBottom = ((y > 0) && (yLine >= rcClip.top && yLine < rcClip.bottom));
	WORD *pCenterBottom = m_pDest->GetRowStart(yLine) + m_xDest;
	BYTE *pCenterBottomAlpha = m_pDest->GetAlphaRow(yLine) + m_xDest;

	//	Compute radius increment

	int iRadius = y;
	int d = -y;
	int deltaE = 3;
	int deltaSE = -2 * y + 1;

	//	Loop

	int xPos = 0;

	//	Paint the center point (this is a special case)

	if (y == 0)
		{
		if (m_xDest < rcClip.right
				&& m_xDest >= rcClip.left
				&& bPaintTop
				&& m_iRadius > 0)
			{
			WORD dwOpacity = m_pOpacityTable[0];
			WORD wColor = m_pColorTable[0];

			if (dwOpacity == 255)
				{
				*pCenterTop = wColor;
				*pCenterTopAlpha = 255;
				}
			else
				{
				*pCenterTop = CG16bitImage::BlendPixel(*pCenterTop, wColor, dwOpacity);
				*pCenterTopAlpha = CG16bitImage::BlendAlpha(*pCenterTopAlpha, (BYTE)dwOpacity);
				}
			}

		//	Continue

		xPos = 1;
		d += deltaSE;
		deltaE += 2;
		iRadius++;
		}

	int iBaseIndex = (y % STOCHASTIC_DIM) * STOCHASTIC_DIM;

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

		//	Compute the transparency based on the radius

		if (iRadius >= m_iRadius)
			{
			//	Skip
			}
		else
			{
			//	Jitter opacity

			WORD dwOpacity = m_pOpacityTable[iRadius];

			//	Optimize full opacity

			if (dwOpacity == 255)
				{
				WORD wColor = m_pColorTable[iRadius];

				if (m_xDest - xPos < rcClip.right && m_xDest - xPos >= rcClip.left)
					{
					if (bPaintTop)
						{
						*(pCenterTop - xPos) = wColor;
						*(pCenterTopAlpha - xPos) = 255;
						}

					if (bPaintBottom)
						{
						*(pCenterBottom - xPos) = wColor;
						*(pCenterBottomAlpha - xPos) = 255;
						}
					}

				if (xPos > 0 && m_xDest + xPos < rcClip.right && m_xDest + xPos >= rcClip.left)
					{
					if (bPaintTop)
						{
						*(pCenterTop + xPos) = wColor;
						*(pCenterTopAlpha + xPos) = 255;
						}

					if (bPaintBottom)
						{
						*(pCenterBottom + xPos) = wColor;
						*(pCenterBottomAlpha + xPos) = 255;
						}
					}
				}
			else
				{
				//	Jitter

				dwOpacity = STOCHASTIC_OPACITY[iBaseIndex + (xPos % STOCHASTIC_DIM)][dwOpacity];

#define DRAW_PIXEL(pos, alphaPos)	\
					{ \
					WORD dwDest = *(pos);	\
					WORD dwRedDest = (dwDest >> 11) & 0x1f;	\
					WORD dwGreenDest = (dwDest >> 5) & 0x3f;	\
					WORD dwBlueDest = dwDest & 0x1f;	\
					\
					*(pos) = (WORD)((dwOpacity * (m_pBlue[iRadius] - dwBlueDest) >> 8) + dwBlueDest |	\
							((dwOpacity * (m_pGreen[iRadius] - dwGreenDest) >> 8) + dwGreenDest) << 5 |	\
							((dwOpacity * (m_pRed[iRadius] - dwRedDest) >> 8) + dwRedDest) << 11);	\
					\
					*(alphaPos) = CG16bitImage::BlendAlpha(*(alphaPos), (BYTE)dwOpacity);	\
					}

				if (m_xDest - xPos < rcClip.right && m_xDest - xPos >= rcClip.left)
					{
					if (bPaintTop)
						DRAW_PIXEL(pCenterTop - xPos, pCenterTopAlpha - xPos);

					if (bPaintBottom)
						DRAW_PIXEL(pCenterBottom - xPos, pCenterBottomAlpha - xPos);
					}

				if (xPos > 0 && m_xDest + xPos < rcClip.right && m_xDest + xPos >= rcClip.left)
					{
					if (bPaintTop)
						DRAW_PIXEL(pCenterTop + xPos, pCenterTopAlpha + xPos);

					if (bPaintBottom)
						DRAW_PIXEL(pCenterBottom + xPos, pCenterBottomAlpha + xPos);
					}
#undef DRAW_PIXEL
				}
			}

		xPos++;
		}
	}

//	DrawFilledCircleFromTable --------------------------------------------------

class CFilledCircleFromTable
	{
	public:
		CFilledCircleFromTable (CG16bitImage *pDest, int xDest, int yDest, int iRadius, WORD *pColorTable, BYTE *pOpacityTable);
		~CFilledCircleFromTable (void);

		void DrawLine (int x, int y);
		inline int GetRadius (void) const { return m_iRadius; }

	private:
		CG16bitImage *m_pDest;
		int m_xDest;
		int m_yDest;
		int m_iRadius;
		WORD *m_pColorTable;
		BYTE *m_pOpacityTable;

		DWORD *m_pRed;
		DWORD *m_pGreen;
		DWORD *m_pBlue;
	};

void DrawFilledCircle (CG16bitImage &Dest, 
					   int xDest, 
					   int yDest, 
					   int iRadius,
					   WORD *pColorTable,
					   BYTE *pOpacityTable)
	{
	CFilledCircleFromTable Painter(&Dest, xDest, yDest, iRadius, pColorTable, pOpacityTable);
	DrawFilledCircle(Painter);
	}

CFilledCircleFromTable::CFilledCircleFromTable (CG16bitImage *pDest, int xDest, int yDest, int iRadius, WORD *pColorTable, BYTE *pOpacityTable) :
				m_pDest(pDest),
				m_xDest(xDest),
				m_yDest(yDest),
				m_iRadius(iRadius),
				m_pColorTable(pColorTable),
				m_pOpacityTable(pOpacityTable),
				m_pRed(NULL),
				m_pGreen(NULL),
				m_pBlue(NULL)
	{
	int i;

	if (iRadius > 0)
		{
		m_pRed = new DWORD [iRadius];
		m_pGreen = new DWORD [iRadius];
		m_pBlue = new DWORD [iRadius];
		for (i = 0; i < iRadius; i++)
			{
			WORD wColor = m_pColorTable[i];
			m_pRed[i] = (wColor >> 11) & 0x1f;
			m_pGreen[i] = (wColor >> 5) & 0x3f;
			m_pBlue[i] = wColor & 0x1f;
			}
		}
	}

CFilledCircleFromTable::~CFilledCircleFromTable (void)
	{
	if (m_pRed)
		delete [] m_pRed;

	if (m_pGreen)
		delete [] m_pGreen;

	if (m_pBlue)
		delete [] m_pBlue;
	}

void CFilledCircleFromTable::DrawLine (int x, int y)
	{
	int xStart = m_xDest - x;
	int xEnd = m_xDest + x + 1;
	const RECT &rcClip = m_pDest->GetClipRect();

	if (xEnd <= rcClip.left || xStart >= rcClip.right)
		return;

	//	See which lines we need to paint

	int yLine = m_yDest - y;
	bool bPaintTop = (yLine >= rcClip.top && yLine < rcClip.bottom);
	WORD *pCenterTop = m_pDest->GetRowStart(yLine) + m_xDest;

	yLine = m_yDest + y;
	bool bPaintBottom = ((y > 0) && (yLine >= rcClip.top && yLine < rcClip.bottom));
	WORD *pCenterBottom = m_pDest->GetRowStart(yLine) + m_xDest;

	//	Compute radius increment

	int iRadius = y;
	int d = -y;
	int deltaE = 3;
	int deltaSE = -2 * y + 1;

	//	Loop

	int xPos = 0;

	//	Paint the center point (this is a special case)

	if (y == 0)
		{
		if (m_xDest < rcClip.right
				&& m_xDest >= rcClip.left
				&& bPaintTop
				&& m_iRadius > 0)
			{
			WORD dwOpacity = m_pOpacityTable[0];
			WORD wColor = m_pColorTable[0];

			if (dwOpacity == 255)
				*pCenterTop = wColor;
			else
				*pCenterTop = CG16bitImage::BlendPixel(*pCenterTop, wColor, dwOpacity);
			}

		//	Continue

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

		//	Compute the transparency based on the radius

		WORD dwOpacity;
		if (iRadius >= m_iRadius
				|| (dwOpacity = m_pOpacityTable[iRadius]) == 0)
			{
			//	Skip
			}
		else
			{
			if (dwOpacity == 255)
				{
				WORD wColor = m_pColorTable[iRadius];

				if (m_xDest - xPos < rcClip.right && m_xDest - xPos >= rcClip.left)
					{
					if (bPaintTop)
						*(pCenterTop - xPos) = wColor;

					if (bPaintBottom)
						*(pCenterBottom - xPos) = wColor;
					}

				if (xPos > 0 && m_xDest + xPos < rcClip.right && m_xDest + xPos >= rcClip.left)
					{
					if (bPaintTop)
						*(pCenterTop + xPos) = wColor;

					if (bPaintBottom)
						*(pCenterBottom + xPos) = wColor;
					}
				}
			else
				{
#define DRAW_PIXEL(pos)	\
					{ \
					WORD dwDest = *(pos);	\
					WORD dwRedDest = (dwDest >> 11) & 0x1f;	\
					WORD dwGreenDest = (dwDest >> 5) & 0x3f;	\
					WORD dwBlueDest = dwDest & 0x1f;	\
					\
					*(pos) = (WORD)((dwOpacity * (m_pBlue[iRadius] - dwBlueDest) >> 8) + dwBlueDest |	\
							((dwOpacity * (m_pGreen[iRadius] - dwGreenDest) >> 8) + dwGreenDest) << 5 |	\
							((dwOpacity * (m_pRed[iRadius] - dwRedDest) >> 8) + dwRedDest) << 11);	\
					}

				if (m_xDest - xPos < rcClip.right && m_xDest - xPos >= rcClip.left)
					{
					if (bPaintTop)
						DRAW_PIXEL(pCenterTop - xPos);

					if (bPaintBottom)
						DRAW_PIXEL(pCenterBottom - xPos);
					}

				if (xPos > 0 && m_xDest + xPos < rcClip.right && m_xDest + xPos >= rcClip.left)
					{
					if (bPaintTop)
						DRAW_PIXEL(pCenterTop + xPos);

					if (bPaintBottom)
						DRAW_PIXEL(pCenterBottom + xPos);
					}
#undef DRAW_PIXEL
				}
			}

		xPos++;
		}
	}

//	DrawAlphaGradientCircle ---------------------------------------------------

struct SAlphaGradientCircleLineCtx
	{
	SAlphaGradientCircleLineCtx (void) :
			pTrans(NULL),
			pRed(NULL),
			pGreen(NULL),
			pBlue(NULL),
			pRedByTrans(NULL),
			pGreenByTrans(NULL),
			pBlueByTrans(NULL),
			pColorTable(NULL),
			pOpacityTable(NULL)
		{ }

	~SAlphaGradientCircleLineCtx (void)
		{
		if (pTrans)
			delete [] pTrans;

		if (pRed)
			delete [] pRed;

		if (pGreen)
			delete [] pGreen;

		if (pBlue)
			delete [] pBlue;

		if (pRedByTrans)
			delete [] pRedByTrans;

		if (pGreenByTrans)
			delete [] pGreenByTrans;

		if (pBlueByTrans)
			delete [] pBlueByTrans;
		}

	CG16bitImage *pDest;
	int xDest;
	int yDest;
	int iRadius;
	WORD wColor;

	//	These are pre-computed if we're calculating each pixel

	DWORD dwRed;
	DWORD dwGreen;
	DWORD dwBlue;

	WORD *pTrans;
	DWORD *pRed;
	DWORD *pGreen;
	DWORD *pBlue;

	//	These are pre-computed if we are using a table

	WORD *pRedByTrans;
	WORD *pGreenByTrans;
	WORD *pBlueByTrans;

	//	These are initialized if we have a radius table

	WORD *pColorTable;
	BYTE *pOpacityTable;
	};

void DrawAlphaGradientCircleLine (const SAlphaGradientCircleLineCtx &Ctx, int x, int y)
	{
	int xStart = Ctx.xDest - x;
	int xEnd = Ctx.xDest + x + 1;
	const RECT &rcClip = Ctx.pDest->GetClipRect();

	if (xEnd <= rcClip.left || xStart >= rcClip.right)
		return;

	//	See which lines we need to paint

	int yLine = Ctx.yDest - y;
	bool bPaintTop = (yLine >= rcClip.top && yLine < rcClip.bottom);
	WORD *pCenterTop = Ctx.pDest->GetRowStart(yLine) + Ctx.xDest;

	yLine = Ctx.yDest + y;
	bool bPaintBottom = ((y > 0) && (yLine >= rcClip.top && yLine < rcClip.bottom));
	WORD *pCenterBottom = Ctx.pDest->GetRowStart(yLine) + Ctx.xDest;

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

		//	Compute the transparency based on the radius

		WORD dwTrans;
		if (iRadius >= Ctx.iRadius
				|| (dwTrans = Ctx.pTrans[iRadius]) >= 255)
			{
			xPos++;
			continue;
			}

#define DRAW_PIXEL(pos)	\
			{ \
			WORD dwDest = *(pos);	\
			WORD dwRedDest = (dwDest >> 11) & 0x1f;	\
			WORD dwGreenDest = (dwDest >> 5) & 0x3f;	\
			WORD dwBlueDest = dwDest & 0x1f;	\
\
			*(pos) = (WORD)((dwTrans * (Ctx.dwBlue - dwBlueDest) >> 8) + dwBlueDest |	\
					((dwTrans * (Ctx.dwGreen - dwGreenDest) >> 8) + dwGreenDest) << 5 |	\
					((dwTrans * (Ctx.dwRed - dwRedDest) >> 8) + dwRedDest) << 11);	\
			}

		//	Paint

		if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
			{
			if (bPaintTop)
				DRAW_PIXEL(pCenterTop - xPos);

			if (bPaintBottom)
				DRAW_PIXEL(pCenterBottom - xPos);
			}

		if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
			{
			if (bPaintTop)
				DRAW_PIXEL(pCenterTop + xPos);

			if (bPaintBottom)
				DRAW_PIXEL(pCenterBottom + xPos);
			}
#undef DRAW_PIXEL

		xPos++;
		}
	}

void DrawAlphaGradientCircleLineTable (const SAlphaGradientCircleLineCtx &Ctx, int x, int y)
	{
	int xStart = Ctx.xDest - x;
	int xEnd = Ctx.xDest + x + 1;
	const RECT &rcClip = Ctx.pDest->GetClipRect();

	if (xEnd <= rcClip.left || xStart >= rcClip.right)
		return;

	//	See which lines we need to paint

	int yLine = Ctx.yDest - y;
	bool bPaintTop = (yLine >= rcClip.top && yLine < rcClip.bottom);
	WORD *pCenterTop = Ctx.pDest->GetRowStart(yLine) + Ctx.xDest;

	yLine = Ctx.yDest + y;
	bool bPaintBottom = ((y > 0) && (yLine >= rcClip.top && yLine < rcClip.bottom));
	WORD *pCenterBottom = Ctx.pDest->GetRowStart(yLine) + Ctx.xDest;

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

		//	Compute the transparency based on the radius

		WORD dwTrans;
		if (iRadius >= Ctx.iRadius
				|| (dwTrans = Ctx.pTrans[iRadius]) >= 255)
			{
			xPos++;
			continue;
			}

#define DRAW_PIXEL(pos)	\
			{ \
			WORD dwDest = *(pos);	\
\
			*(pos) = (Ctx.pBlueByTrans[((dwDest & 0x1f) << 8) | dwTrans]	\
					| Ctx.pGreenByTrans[((dwDest & 0x7e0) << 3) | dwTrans] \
					| Ctx.pRedByTrans[((dwDest & 0xf800) >> 3) | dwTrans]); \
			}

		//	Paint

		if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
			{
			if (bPaintTop)
				DRAW_PIXEL(pCenterTop - xPos);

			if (bPaintBottom)
				DRAW_PIXEL(pCenterBottom - xPos);
			}

		if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
			{
			if (bPaintTop)
				DRAW_PIXEL(pCenterTop + xPos);

			if (bPaintBottom)
				DRAW_PIXEL(pCenterBottom + xPos);
			}
#undef DRAW_PIXEL

		xPos++;
		}
	}

void DrawAlphaGradientCircle (CG16bitImage &Dest, 
							  int xDest, 
							  int yDest, 
							  int iRadius,
							  WORD wColor)

//	DrawAlphaGradientCircle
//
//	Draws a filled circle of the given color. The circle has an alpha gradient
//	that ranges from fully opaque in the center to fully transparent at the edges.

	{
	int i;

	//	Deal with edge-conditions

	if (iRadius <= 0)
		{
		Dest.DrawPixel(xDest, yDest, wColor);
		return;
		}

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	//	Prepare struct

	SAlphaGradientCircleLineCtx Ctx;
	Ctx.pDest = &Dest;
	Ctx.xDest = xDest;
	Ctx.yDest = yDest;
	Ctx.iRadius = iRadius;
	Ctx.wColor = wColor;

	//	If the radius is less than 256 then it's faster just to compute the 
	//	values for each pixel inside the circle.

	if (iRadius <= 320)
		{
		//	Pre-compute some color info

		DWORD dwColor = wColor;
		Ctx.dwRed = (dwColor >> 11) & 0x1f;
		Ctx.dwGreen = (dwColor >> 5) & 0x3f;
		Ctx.dwBlue = dwColor & 0x1f;

		//	Compute the transparency for each radius value

		Ctx.pTrans = new WORD [iRadius];
		for (i = 0; i < iRadius; i++)
			Ctx.pTrans[i] = (WORD)(255 - (255 * i / iRadius));

		//	Draw central line

		DrawAlphaGradientCircleLine(Ctx, iRadius, 0);

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

				DrawAlphaGradientCircleLine(Ctx, x, y);
				iLastDraw = y;

				//	Next

				y--;
				}

			x++;

			//	Draw lines

			if (x != iLastDraw)
				DrawAlphaGradientCircleLine(Ctx, y, x);
			}
		}

	//	Otherwise it's faster to compute a table of red/blue and green values 
	//	for each possible transparency

	else
		{
		DWORD dwTrans;
		DWORD dwValue;

		//	Compute the transparency for each radius value

		Ctx.pTrans = new WORD [iRadius];
		for (i = 0; i < iRadius; i++)
			Ctx.pTrans[i] = 255 - (255 * i / iRadius);

		//	Decompose the color

		DWORD dwColor = wColor;
		Ctx.dwRed = (dwColor >> 11) & 0x1f;
		Ctx.dwGreen = (dwColor >> 5) & 0x3f;
		Ctx.dwBlue = dwColor & 0x1f;

		//	Build the red/blue table

		Ctx.pRedByTrans = new WORD [RED_BLUE_COUNT * 256];
		Ctx.pBlueByTrans = new WORD [RED_BLUE_COUNT * 256];
		for (dwTrans = 0; dwTrans < 256; dwTrans++)
			for (dwValue = 0; dwValue < RED_BLUE_COUNT; dwValue++)
				{
				Ctx.pBlueByTrans[(dwValue << 8) | dwTrans] = (WORD)((dwTrans * (Ctx.dwBlue - dwValue) >> 8) + dwValue);
				Ctx.pRedByTrans[(dwValue << 8) | dwTrans] = (WORD)((dwTrans * (Ctx.dwRed - dwValue) >> 8) + dwValue) << 11;
				}

		//	Build the green table

		Ctx.pGreenByTrans = new WORD [GREEN_COUNT * 256];
		for (dwTrans = 0; dwTrans < 256; dwTrans++)
			for (dwValue = 0; dwValue < GREEN_COUNT; dwValue++)
				Ctx.pGreenByTrans[(dwValue << 8) | dwTrans] = (WORD)((dwTrans * (Ctx.dwGreen - dwValue) >> 8) + dwValue) << 5;

		//	Draw central line

		DrawAlphaGradientCircleLineTable(Ctx, iRadius, 0);

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

				DrawAlphaGradientCircleLineTable(Ctx, x, y);
				iLastDraw = y;

				//	Next

				y--;
				}

			x++;

			//	Draw lines

			if (x != iLastDraw)
				DrawAlphaGradientCircleLineTable(Ctx, y, x);
			}
		}
	}

//	DrawBltCircle -------------------------------------------------------------

struct SBltCircleLineCtx
	{
	//	Dest
	CG16bitImage *pDest;
	int xDest;
	int yDest;
	int iRadius;

	//	Source
	const CG16bitImage *pSrc;
	int xSrc;
	int ySrc;
	int cxSrc;
	int cySrc;
	DWORD byOpacity;
	};

void DrawBltCircleLine (const SBltCircleLineCtx &Ctx, int x, int y)

//	DrawBltCircleLine
//
//	Draws a single horizontal line across the circle. For each point on the line
//	we compute the radius and angle so that we can map a bitmap around the circle
//	(effectively, a polar coordinates map).
//
//	The calculation of the radius uses a modified version of the algorithm
//	described in page 84 of Foley and van Dam. But because we are computing the
//	radius as we advance x, the function that we are computing is:
//
//	F(x,r) = x^2 - r^2 + Y^2
//
//	In which Y^2 is constant (the y coordinate of this line).
//
//	The solution leads to the two decision functions:
//
//	deltaE = 2xp + 3
//	deltaSE = 2xp - 2r + 1
//
//	The calculation of the angle relies on a fast arctangent approximation
//	on page 389 of Graphic Gems II.

	{
	int xStart = Ctx.xDest - x;
	int xEnd = Ctx.xDest + x + 1;
	const RECT &rcClip = Ctx.pDest->GetClipRect();

	if (xEnd <= rcClip.left || xStart >= rcClip.right)
		return;

	//	See which lines we need to paint

	int yLine = Ctx.yDest - y;
	bool bPaintTop = (yLine >= rcClip.top && yLine < rcClip.bottom);
	WORD *pCenterTop = Ctx.pDest->GetRowStart(yLine) + Ctx.xDest;

	yLine = Ctx.yDest + y;
	bool bPaintBottom = ((y > 0) && (yLine >= rcClip.top && yLine < rcClip.bottom));
	WORD *pCenterBottom = Ctx.pDest->GetRowStart(yLine) + Ctx.xDest;

	//	Compute radius increment

	int iRadius = y;
	int d = -y;
	int deltaE = 3;
	int deltaSE = -2 * y + 1;

	//	Compute angle increment

	const int iFixedPoint = 8192;
	int iAngle = 2 * iFixedPoint;
	int angle1 = (y == 0 ? (2 * iFixedPoint) : (iFixedPoint / y));
	int num1 = iFixedPoint * y;

	int cxSrcQuadrant = Ctx.cxSrc / 4;
	int cxSrcHalf = Ctx.cxSrc / 2;

	//	Loop

	int xPos = 0;

	//	This will skip the center pixel in the circle (avoids a divide by
	//	zero in the inner loop).

	if (y == 0)
		{
		xPos = 1;
		d += deltaSE;
		deltaE += 2;
		iRadius++;
		}

	//	Blt the line based on the type of source image

	if (Ctx.pSrc->HasAlpha() && Ctx.byOpacity != 255)
		{
		BYTE *pAlpha5 = g_Alpha5[Ctx.byOpacity];
		BYTE *pAlpha6 = g_Alpha6[Ctx.byOpacity];

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

			//	If we're beyond the size of the source image then continue

			int yOffset = Ctx.iRadius - iRadius;
			if (yOffset >= Ctx.cySrc || yOffset < 0)
				{
				xPos++;
				continue;
				}

			yOffset += Ctx.ySrc;

			//	Figure out the angle of the pixel at this location

			if (xPos < y)
				iAngle -= angle1;
			else
				iAngle = num1 / xPos;

			int xOffset = Ctx.xSrc + (iAngle * cxSrcQuadrant / (2 * iFixedPoint));

			//	Figure out the row of the source image

			WORD *pSrcRow = Ctx.pSrc->GetRowStart(yOffset);
			BYTE *pSrcAlphaRow = Ctx.pSrc->GetAlphaRow(yOffset);

			//	Paint

#define DRAW_PIXEL									\
				{									\
				DWORD dwDestAlpha = 255 - ((*pAlpha) * Ctx.byOpacity / 255); \
				BYTE *pAlpha5Inv = g_Alpha5[dwDestAlpha]; \
				BYTE *pAlpha6Inv = g_Alpha6[dwDestAlpha]; \
\
				DWORD dTemp = *pDest; \
				DWORD sTemp = *(pSrcRow + xSrc); \
\
				DWORD dwRedResult = pAlpha5Inv[(dTemp & 0xf800) >> 11] + pAlpha5[(sTemp & 0xf800) >> 11]; \
				DWORD dwGreenResult = pAlpha6Inv[(dTemp & 0x7e0) >> 5] + pAlpha6[(sTemp & 0x7e0) >> 5]; \
				DWORD dwBlueResult = pAlpha5Inv[(dTemp & 0x1f)] + pAlpha5[(sTemp & 0x1f)]; \
\
				*pDest = (WORD)((dwRedResult << 11) | (dwGreenResult << 5) | (dwBlueResult)); \
				}

			if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
				{
				if (bPaintTop)
					{
					int xSrc = cxSrcHalf - xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterTop - xPos;
						DRAW_PIXEL;
						}
					}

				if (bPaintBottom)
					{
					int xSrc = cxSrcHalf + xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterBottom - xPos;
						DRAW_PIXEL;
						}
					}
				}

			if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
				{
				if (bPaintTop)
					{
					int xSrc = xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterTop + xPos;
						DRAW_PIXEL;
						}
					}

				if (bPaintBottom)
					{
					int xSrc = (Ctx.cxSrc - 1) - xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterBottom + xPos;
						DRAW_PIXEL;
						}
					}
				}
#undef DRAW_PIXEL

			xPos++;
			}
		}
	else if (Ctx.byOpacity != 255)
		{
		BYTE *pAlpha5 = g_Alpha5[Ctx.byOpacity];
		BYTE *pAlpha6 = g_Alpha6[Ctx.byOpacity];
		BYTE *pAlpha5Inv = g_Alpha5[255 - Ctx.byOpacity];
		BYTE *pAlpha6Inv = g_Alpha6[255 - Ctx.byOpacity];

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

			//	If we're beyond the size of the source image then continue

			int yOffset = Ctx.iRadius - iRadius;
			if (yOffset >= Ctx.cySrc || yOffset < 0)
				{
				xPos++;
				continue;
				}

			yOffset += Ctx.ySrc;

			//	Figure out the angle of the pixel at this location

			if (xPos < y)
				iAngle -= angle1;
			else
				iAngle = num1 / xPos;

			int xOffset = Ctx.xSrc + (iAngle * cxSrcQuadrant / (2 * iFixedPoint));

			//	Figure out the row of the source image

			WORD *pSrcRow = Ctx.pSrc->GetRowStart(yOffset);
			BYTE *pSrcAlphaRow = Ctx.pSrc->GetAlphaRow(yOffset);

			//	Paint

#define DRAW_PIXEL									\
				{									\
				DWORD dTemp = *pDest; \
				DWORD sTemp = *(pSrcRow + xSrc); \
\
				DWORD dwRedResult = pAlpha5Inv[(dTemp & 0xf800) >> 11] + pAlpha5[(sTemp & 0xf800) >> 11]; \
				DWORD dwGreenResult = pAlpha6Inv[(dTemp & 0x7e0) >> 5] + pAlpha6[(sTemp & 0x7e0) >> 5]; \
				DWORD dwBlueResult = pAlpha5Inv[(dTemp & 0x1f)] + pAlpha5[(sTemp & 0x1f)]; \
\
				*pDest = (WORD)((dwRedResult << 11) | (dwGreenResult << 5) | (dwBlueResult)); \
				}

			if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
				{
				if (bPaintTop)
					{
					int xSrc = cxSrcHalf - xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterTop - xPos;
						DRAW_PIXEL;
						}
					}

				if (bPaintBottom)
					{
					int xSrc = cxSrcHalf + xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterBottom - xPos;
						DRAW_PIXEL;
						}
					}
				}

			if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
				{
				if (bPaintTop)
					{
					int xSrc = xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterTop + xPos;
						DRAW_PIXEL;
						}
					}

				if (bPaintBottom)
					{
					int xSrc = (Ctx.cxSrc - 1) - xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterBottom + xPos;
						DRAW_PIXEL;
						}
					}
				}
#undef DRAW_PIXEL

			xPos++;
			}
		}
	else if (Ctx.pSrc->HasAlpha())
		{
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

			//	If we're beyond the size of the source image then continue

			int yOffset = Ctx.iRadius - iRadius;
			if (yOffset >= Ctx.cySrc || yOffset < 0)
				{
				xPos++;
				continue;
				}

			yOffset += Ctx.ySrc;

			//	Figure out the angle of the pixel at this location

			if (xPos < y)
				iAngle -= angle1;
			else
				iAngle = num1 / xPos;

			int xOffset = Ctx.xSrc + (iAngle * cxSrcQuadrant / (2 * iFixedPoint));

			//	Figure out the row of the source image

			WORD *pSrcRow = Ctx.pSrc->GetRowStart(yOffset);
			BYTE *pSrcAlphaRow = Ctx.pSrc->GetAlphaRow(yOffset);

			//	Paint

#define DRAW_PIXEL									\
				{									\
				if (*pAlpha == 255)					\
					*pDest = *(pSrcRow + xSrc);		\
				else								\
					{								\
					DWORD dwInvTrans = ((*pAlpha) ^ 0xff);	\
					DWORD dwSource = *(pSrcRow + xSrc);	\
					DWORD dwDest = *pDest;	\
\
					DWORD dwRedGreenS = ((dwSource << 8) & 0x00f80000) | (dwSource & 0x000007e0);	\
					DWORD dwRedGreen = (((((dwDest << 8) & 0x00f80000) | (dwDest & 0x000007e0)) * dwInvTrans) >> 8) + dwRedGreenS;	\
					DWORD dwBlue = (((dwDest & 0x1f) * dwInvTrans) >> 8) + (dwSource & 0x1f);	\
\
					*pDest = (WORD)(((dwRedGreen & 0x00f80000) >> 8) | (dwRedGreen & 0x000007e0) | dwBlue);	\
					}	\
				}

			if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
				{
				if (bPaintTop)
					{
					int xSrc = cxSrcHalf - xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterTop - xPos;
						DRAW_PIXEL;
						}
					}

				if (bPaintBottom)
					{
					int xSrc = cxSrcHalf + xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterBottom - xPos;
						DRAW_PIXEL;
						}
					}
				}

			if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
				{
				if (bPaintTop)
					{
					int xSrc = xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterTop + xPos;
						DRAW_PIXEL;
						}
					}

				if (bPaintBottom)
					{
					int xSrc = (Ctx.cxSrc - 1) - xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterBottom + xPos;
						DRAW_PIXEL;
						}
					}
				}
#undef DRAW_PIXEL

			xPos++;
			}
		}
	else
		{
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

			//	If we're beyond the size of the source image then continue

			int yOffset = Ctx.iRadius - iRadius;
			if (yOffset >= Ctx.cySrc || yOffset < 0)
				{
				xPos++;
				continue;
				}

			yOffset += Ctx.ySrc;

			//	Figure out the angle of the pixel at this location

			if (xPos < y)
				iAngle -= angle1;
			else
				iAngle = num1 / xPos;

			int xOffset = iAngle * cxSrcQuadrant / (2 * iFixedPoint);

			//	Figure out the row of the source image

			WORD *pSrcRow = Ctx.pSrc->GetRowStart(yOffset);

			//	Paint

			if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
				{
				if (bPaintTop)
					*(pCenterTop - xPos) = *(pSrcRow + cxSrcHalf - xOffset);

				if (bPaintBottom)
					*(pCenterBottom - xPos) = *(pSrcRow + cxSrcHalf + xOffset);
				}

			if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
				{
				if (bPaintTop)
					*(pCenterTop + xPos) = *(pSrcRow + xOffset);

				if (bPaintBottom)
					*(pCenterBottom + xPos) = *(pSrcRow + (Ctx.cxSrc - 1) - xOffset);
				}

			xPos++;
			}
		}
	}

void DrawBltCircle (CG16bitImage &Dest, 
					int xDest, 
					int yDest, 
					int iRadius,
					const CG16bitImage &Src,
					int xSrc,
					int ySrc,
					int cxSrc,
					int cySrc,
					DWORD byOpacity)

//	DrawBltCircle
//
//	Takes the source image and blts it as a circle

	{
	//	Deal with edge-conditions

	if (iRadius <= 0)
		return;

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	//	Prepare struct

	SBltCircleLineCtx Ctx;
	Ctx.pDest = &Dest;
	Ctx.xDest = xDest;
	Ctx.yDest = yDest;
	Ctx.iRadius = iRadius;
	Ctx.pSrc = &Src;
	Ctx.xSrc = xSrc;
	Ctx.ySrc = ySrc;
	Ctx.cxSrc = cxSrc;
	Ctx.cySrc = cySrc;
	Ctx.byOpacity = byOpacity;

	//	Draw central line

	DrawBltCircleLine(Ctx, iRadius, 0);

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

			DrawBltCircleLine(Ctx, x, y);
			iLastDraw = y;

			//	Next

			y--;
			}

		x++;

		//	Draw lines

		if (x != iLastDraw)
			DrawBltCircleLine(Ctx, y, x);
		}
	}

//	DrawFilledCircle ----------------------------------------------------------

void DrawFilledCircle (CG16bitImage &Dest, int xDest, int yDest, int iRadius, WORD wColor)

//	DrawFilledCircle
//
//	Draws a filled circle centered on the given coordinates.
//
//	Foley and vanDam. Computer Graphics: Principles and Practice.
//	p.87

	{
	//	Deal with edge-conditions

	if (iRadius <= 0)
		{
		Dest.DrawPixel(xDest, yDest, wColor);
		return;
		}

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	Dest.FillLine(xDest - iRadius, yDest, 1 + 2 * iRadius, wColor);

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

			Dest.FillLine(xDest - x, yDest - y, 1 + 2 * x, wColor);
			Dest.FillLine(xDest - x, yDest + y, 1 + 2 * x, wColor);

			y--;
			}

		x++;

		if (y >= x)
			{
			Dest.FillLine(xDest - y, yDest - x, 1 + 2 * y, wColor);
			Dest.FillLine(xDest - y, yDest + x, 1 + 2 * y, wColor);
			}
		}
	}

void DrawFilledCircleGray (CG16bitImage &Dest, int xDest, int yDest, int iRadius, WORD wColor, DWORD byOpacity)

//	DrawFilledCircleGray
//
//	Draws an transparent filled circle

	{
	//	Deal with edge-conditions

	if (iRadius <= 0)
		{
		Dest.DrawPixelTrans(xDest, yDest, wColor, (BYTE)byOpacity);
		return;
		}
	else if (byOpacity == 255)
		{
		DrawFilledCircle(Dest, xDest, yDest, iRadius, wColor);
		return;
		}

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	Dest.FillLineGray(xDest - iRadius, yDest, 1 + 2 * iRadius, wColor, byOpacity);

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

			Dest.FillLineGray(xDest - x, yDest - y, 1 + 2 * x, wColor, byOpacity);
			Dest.FillLineGray(xDest - x, yDest + y, 1 + 2 * x, wColor, byOpacity);

			y--;
			}

		x++;

		if (y >= x)
			{
			Dest.FillLineGray(xDest - y, yDest - x, 1 + 2 * y, wColor, byOpacity);
			Dest.FillLineGray(xDest - y, yDest + x, 1 + 2 * y, wColor, byOpacity);
			}
		}
	}

void DrawFilledCircleTrans (CG16bitImage &Dest, int xDest, int yDest, int iRadius, WORD wColor, DWORD byOpacity)

//	DrawFilledCircleTrans
//
//	Draws an transparent filled circle

	{
	//	Deal with edge-conditions

	if (iRadius <= 0)
		{
		Dest.DrawPixelTrans(xDest, yDest, wColor, (BYTE)byOpacity);
		return;
		}
	else if (byOpacity == 255)
		{
		DrawFilledCircle(Dest, xDest, yDest, iRadius, wColor);
		return;
		}

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	Dest.FillLineTrans(xDest - iRadius, yDest, 1 + 2 * iRadius, wColor, byOpacity);

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

			Dest.FillLineTrans(xDest - x, yDest - y, 1 + 2 * x, wColor, byOpacity);
			Dest.FillLineTrans(xDest - x, yDest + y, 1 + 2 * x, wColor, byOpacity);

			y--;
			}

		x++;

		if (y >= x)
			{
			Dest.FillLineTrans(xDest - y, yDest - x, 1 + 2 * y, wColor, byOpacity);
			Dest.FillLineTrans(xDest - y, yDest + x, 1 + 2 * y, wColor, byOpacity);
			}
		}
	}

//	DrawGlowRing --------------------------------------------------------------

struct SGlowRingLineCtx
	{
	CG16bitImage *pDest;
	int xDest;
	int yDest;
	int iRadius;
	int iRingThickness;
	WORD *wColor;
	DWORD *byOpacity;

	DWORD *dwRed;
	DWORD *dwGreen;
	DWORD *dwBlue;
	};

void DrawGlowRingLine (const SGlowRingLineCtx &Ctx, int x, int y)
	{
	int xStart = Ctx.xDest - x;
	int xEnd = Ctx.xDest + x + 1;
	const RECT &rcClip = Ctx.pDest->GetClipRect();

	if (xEnd <= rcClip.left || xStart >= rcClip.right)
		return;

	//	See which lines we need to paint

	int yLine = Ctx.yDest - y;
	bool bPaintTop = (yLine >= rcClip.top && yLine < rcClip.bottom);
	WORD *pCenterTop = Ctx.pDest->GetRowStart(yLine) + Ctx.xDest;

	yLine = Ctx.yDest + y;
	bool bPaintBottom = ((y > 0) && (yLine >= rcClip.top && yLine < rcClip.bottom));
	WORD *pCenterBottom = Ctx.pDest->GetRowStart(yLine) + Ctx.xDest;

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

		//	Compute the index into the ramp based on radius and ring thickness
		//	(If we're outside the ramp, then continue)

		int iIndex = Ctx.iRadius - iRadius;
		if (iIndex >= Ctx.iRingThickness || iIndex < 0)
			{
			xPos++;
			continue;
			}

		//	Compute the transparency

		DWORD dwOpacity = Ctx.byOpacity[iIndex];

		//	Optimize opaque painting

		if (dwOpacity >= 255)
			{
			WORD wColor = Ctx.wColor[iIndex];

			if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
				{
				if (bPaintTop)
					*(pCenterTop - xPos) = wColor;

				if (bPaintBottom)
					*(pCenterBottom - xPos) = wColor;
				}

			if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
				{
				if (bPaintTop)
					*(pCenterTop + xPos) = wColor;

				if (bPaintBottom)
					*(pCenterBottom + xPos) = wColor;
				}
			}
		else if (dwOpacity == 0)
			;
		else
			{
			DWORD dwRed = Ctx.dwRed[iIndex];
			DWORD dwGreen = Ctx.dwGreen[iIndex];
			DWORD dwBlue = Ctx.dwBlue[iIndex];

			//	Draw transparent

#define DRAW_PIXEL(pos)	\
				{ \
				DWORD dwDest = (DWORD)*(pos);	\
				DWORD dwRedDest = (dwDest >> 11) & 0x1f;	\
				DWORD dwGreenDest = (dwDest >> 5) & 0x3f;	\
				DWORD dwBlueDest = dwDest & 0x1f;	\
				\
				*(pos) = (WORD)((dwOpacity * (dwBlue - dwBlueDest) >> 8) + dwBlueDest |	\
						((dwOpacity * (dwGreen - dwGreenDest) >> 8) + dwGreenDest) << 5 |	\
						((dwOpacity * (dwRed - dwRedDest) >> 8) + dwRedDest) << 11);	\
				}

			//	Paint

			if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
				{
				if (bPaintTop)
					DRAW_PIXEL(pCenterTop - xPos);

				if (bPaintBottom)
					DRAW_PIXEL(pCenterBottom - xPos);
				}

			if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
				{
				if (bPaintTop)
					DRAW_PIXEL(pCenterTop + xPos);

				if (bPaintBottom)
					DRAW_PIXEL(pCenterBottom + xPos);
				}
#undef DRAW_PIXEL
			}

		xPos++;
		}
	}

void DrawGlowRing (CG16bitImage &Dest,
				   int xDest,
				   int yDest,
				   int iRadius,
				   int iRingThickness,
				   WORD wColor,
				   DWORD dwOpacity)

//	DrawGlowRing
//
//	Draws a glowing ring of the given color

	{
	int i;

	if (iRingThickness < 1)
		return;

	//	Generate the color ramps

	WORD *wColorRamp = new WORD [iRingThickness];
	DWORD *byOpacityRamp = new DWORD [iRingThickness];

	int iCenter = iRingThickness / 2;
	int iExtra = iRingThickness % 2;
	int iOuter = iCenter - 1;
	int iInner = iCenter + iExtra;

	//	Init the center

	DWORD dwPosOpacity = dwOpacity;
	if (iExtra)
		{
		wColorRamp[iCenter] = wColor;
		byOpacityRamp[iCenter] = dwPosOpacity;
		}

	//	Edges

	for (i = 0; i < iCenter; i++)
		{
		ASSERT(iOuter >= 0 && iOuter < iRingThickness);
		ASSERT(iInner >=0 && iInner < iRingThickness);

		dwPosOpacity = dwOpacity * (iCenter - i) / (iCenter + 1);

		wColorRamp[iOuter] = wColor;
		byOpacityRamp[iOuter] = dwPosOpacity;

		wColorRamp[iInner] = wColor;
		byOpacityRamp[iInner] = dwPosOpacity;

		iOuter--;
		iInner++;
		}

	//	Done

	DrawGlowRing(Dest, xDest, yDest, iRadius, iRingThickness, wColorRamp, byOpacityRamp);
	}

void DrawGlowRing (CG16bitImage &Dest,
				   int xDest,
				   int yDest,
				   int iRadius,
				   int iRingThickness,
				   WORD *wColorRamp,
				   DWORD *byOpacityRamp)

//	DrawGlowRing
//
//	Draws a glowing ring

	{
	int i;

	//	Deal with edge-conditions

	if (iRingThickness <= 0 || wColorRamp == NULL || byOpacityRamp == NULL || iRadius <= 0)
		return;

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	//	Prepare struct

	SGlowRingLineCtx Ctx;
	Ctx.pDest = &Dest;
	Ctx.xDest = xDest;
	Ctx.yDest = yDest;
	Ctx.iRadius = iRadius;
	Ctx.iRingThickness = iRingThickness;
	Ctx.wColor = wColorRamp;
	Ctx.byOpacity = byOpacityRamp;

	//	Decompose the color ramp into RGB

	Ctx.dwRed = new DWORD [iRingThickness];
	Ctx.dwGreen = new DWORD [iRingThickness];
	Ctx.dwBlue = new DWORD [iRingThickness];
	for (i = 0; i < iRingThickness; i++)
		{
		Ctx.dwRed[i] = (wColorRamp[i] >> 11) & 0x1f;
		Ctx.dwGreen[i] = (wColorRamp[i] >> 5) & 0x3f;
		Ctx.dwBlue[i] = (wColorRamp[i]) & 0x1f;
		}

	//	Draw central line

	DrawGlowRingLine(Ctx, iRadius, 0);

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

			DrawGlowRingLine(Ctx, x, y);
			iLastDraw = y;

			//	Next

			y--;
			}

		x++;

		//	Draw lines

		if (x != iLastDraw)
			DrawGlowRingLine(Ctx, y, x);
		}

	//	Done

	delete [] Ctx.dwRed;
	delete [] Ctx.dwGreen;
	delete [] Ctx.dwBlue;
	}

void InitStochasticTable (void)
	{
	int i;

	if (!g_bStochasticInit)
		{
		for (i = 0; i < STOCHASTIC_SIZE; i++)
			{
			int iOpacity;
			for (iOpacity = 0; iOpacity < 256; iOpacity++)
				{
				int iJitter = mathRandom(-7, 7);
				BYTE byNewOpacity = Max(0, Min(iOpacity + iJitter, 255));
				STOCHASTIC_OPACITY[i][iOpacity] = byNewOpacity;
				}
			}

		g_bStochasticInit = true;
		}
	}
