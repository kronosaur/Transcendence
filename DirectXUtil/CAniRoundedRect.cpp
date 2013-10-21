//	CAniRoundedRect.cpp
//
//	CAniRoundedRect class

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

#include "Reanimator.h"

const int INDEX_VISIBLE =					0;
#define PROP_VISIBLE						CONSTLIT("visible")

const int INDEX_POSITION =					1;
#define PROP_POSITION						CONSTLIT("position")

const int INDEX_SCALE =						2;
#define PROP_SCALE							CONSTLIT("scale")

const int INDEX_ROTATION =					3;
#define PROP_ROTATION						CONSTLIT("rotation")

const int INDEX_UL_RADIUS =					4;
#define PROP_UL_RADIUS						CONSTLIT("ulRadius")

const int INDEX_UR_RADIUS =					5;
#define PROP_UR_RADIUS						CONSTLIT("urRadius")

const int INDEX_LL_RADIUS =					6;
#define PROP_LL_RADIUS						CONSTLIT("llRadius")

const int INDEX_LR_RADIUS =					7;
#define PROP_LR_RADIUS						CONSTLIT("lrRadius")

const int INDEX_OPACITY =					8;
#define PROP_OPACITY						CONSTLIT("opacity")

#define PROP_COLOR							CONSTLIT("color")

const int PADDING_BOTTOM =					8;
const int PADDING_LEFT =					8;
const int PADDING_RIGHT =					8;
const int PADDING_TOP =						8;

CAniRoundedRect::CAniRoundedRect (void)

//	CAniRoundedRect constructor

	{
	int i;

	m_Properties.SetInteger(PROP_VISIBLE, 1);
	m_Properties.SetVector(PROP_POSITION, CVector());
	m_Properties.SetVector(PROP_SCALE, CVector(1.0, 1.0));
	m_Properties.SetInteger(PROP_ROTATION, 0);
	m_Properties.SetInteger(PROP_UL_RADIUS, 10);
	m_Properties.SetInteger(PROP_UR_RADIUS, 10);
	m_Properties.SetInteger(PROP_LL_RADIUS, 10);
	m_Properties.SetInteger(PROP_LR_RADIUS, 10);
	m_Properties.SetInteger(PROP_OPACITY, 255);

	//	Initialize corners

	for (i = 0; i < cornerCount; i++)
		m_Corner[i].iRadius = 0;
	}

CAniRoundedRect::~CAniRoundedRect (void)

//	CAniRoundedRect destructor

	{
	}

void CAniRoundedRect::Create (const CVector &vPos,
					   const CVector &vSize,
					   WORD wColor,
					   DWORD dwOpacity,
					   IAnimatron **retpAni)

//	Create
//
//	Creates an element

	{
	CAniRoundedRect *pRect = new CAniRoundedRect;
	pRect->SetPropertyVector(PROP_POSITION, vPos);
	pRect->SetPropertyVector(PROP_SCALE, vSize);
	pRect->SetPropertyColor(PROP_COLOR, wColor);
	pRect->SetPropertyOpacity(PROP_OPACITY, dwOpacity);

	*retpAni = pRect;
	}

void CAniRoundedRect::GetContentRect (RECT *retrcRect)

//	GetContentRect
//
//	Returns a RECT of the content area (relative to the rect itself)

	{
	CVector vScale = m_Properties[INDEX_SCALE].GetVector();

	retrcRect->left = PADDING_LEFT;
	retrcRect->top = PADDING_TOP;
	retrcRect->right = (int)vScale.GetX() - PADDING_RIGHT;
	retrcRect->bottom = (int)vScale.GetY() - PADDING_BOTTOM;
	}

void CAniRoundedRect::GetSpacingRect (RECT *retrcRect)

//	GetSpacingRect
//
//	Returns the size

	{
	CVector vScale = m_Properties[INDEX_SCALE].GetVector();

	retrcRect->left = 0;
	retrcRect->top = 0;
	retrcRect->right = (int)vScale.GetX();
	retrcRect->bottom = (int)vScale.GetY();
	}

void CAniRoundedRect::InitCornerRaster (Corners iCorner, int iRadius)

//	InitCornerRaster
//
//	Initializes the given corner

	{
	int i;

	SCornerRaster *pCorner = &m_Corner[iCorner];
	if (pCorner->iRadius != iRadius)
		{
		pCorner->iRadius = Max(0, iRadius);
		pCorner->Lines.DeleteAll();
		if (iRadius)
			pCorner->Lines.InsertEmpty(iRadius);

		if (iRadius <= 0)
			return;

		//	Generate a set of raster lines for the corner

		int *pSolid = new int [iRadius];
		BYTE *pEdge = new BYTE [iRadius];
		RasterizeQuarterCircle8bit(iRadius, pSolid, pEdge);

		//	Different lines depending on the direction

		switch (iCorner)
			{
			case upperLeftCorner:
				for (i = 0; i < iRadius; i++)
					{
					SSimpleRasterLine *pLine = &pCorner->Lines[i];

					pLine->y = i;
					pLine->x = iRadius - pSolid[i];
					pLine->cxLength = pSolid[i];
					pLine->byLeftEdge = pEdge[i];
					pLine->byRightEdge = 0;
					}
				break;

			case upperRightCorner:
				for (i = 0; i < iRadius; i++)
					{
					SSimpleRasterLine *pLine = &pCorner->Lines[i];

					pLine->y = i;
					pLine->x = 0;
					pLine->cxLength = pSolid[i];
					pLine->byLeftEdge = 0;
					pLine->byRightEdge = pEdge[i];
					}
				break;

			case lowerLeftCorner:
				for (i = 0; i < iRadius; i++)
					{
					SSimpleRasterLine *pLine = &pCorner->Lines[i];

					pLine->y = (iRadius - 1) - i;
					pLine->x = iRadius - pSolid[i];
					pLine->cxLength = pSolid[i];
					pLine->byLeftEdge = pEdge[i];
					pLine->byRightEdge = 0;
					}
				break;

			case lowerRightCorner:
				for (i = 0; i < iRadius; i++)
					{
					SSimpleRasterLine *pLine = &pCorner->Lines[i];

					pLine->y = (iRadius - 1) - i;
					pLine->x = 0;
					pLine->cxLength = pSolid[i];
					pLine->byLeftEdge = 0;
					pLine->byRightEdge = pEdge[i];
					}
				break;

			default:
				ASSERT(false);
			}

		//	Done

		delete [] pSolid;
		delete [] pEdge;
		}
	}

void CAniRoundedRect::Paint (SAniPaintCtx &Ctx)

//	Paint
//
//	Paints the element
//
//	LATER: If the rect has a line and fill and opacity then we need to draw
//	to an off-screen bitmap first (otherwise the line and fill will not
//	combine properly).

	{
	//	Position and size

	CVector vPos = Ctx.ToDest.Transform(m_Properties[INDEX_POSITION].GetVector());
	CVector vPos2 = Ctx.ToDest.Transform(m_Properties[INDEX_POSITION].GetVector() + m_Properties[INDEX_SCALE].GetVector());
	CVector vSize = vPos2 - vPos;

	//	Get the size in integer values

	int x = (int)vPos.GetX();
	int y = (int)vPos.GetY();
	int cxWidth = (int)vSize.GetX();
	int cyHeight = (int)vSize.GetY();

	//	Figure out the size of each rounded-corner

	int ulRadius = m_Properties[INDEX_UL_RADIUS].GetInteger();
	int urRadius = m_Properties[INDEX_UR_RADIUS].GetInteger();
	int llRadius = m_Properties[INDEX_LL_RADIUS].GetInteger();
	int lrRadius = m_Properties[INDEX_LR_RADIUS].GetInteger();

	//	Keep track of the size of each edge

	int cyTopEdge = Max(ulRadius, urRadius);
	int cyBottomEdge = Max(llRadius, lrRadius);
	int cxLeftEdge = Max(ulRadius, llRadius);
	int cxRightEdge = Max(urRadius, lrRadius);

	//	Some useful intermediates

	int cxInner = cxWidth - (cxLeftEdge + cxRightEdge);
	int cyInner = cyHeight - (cyTopEdge + cyBottomEdge);

	//	Fill the rounded rect

	IAniFillMethod *pFill = GetFillMethod();
	if (pFill)
		{
		pFill->InitPaint(Ctx, x, y, m_Properties);

		//	Paint the upper-left corner

		if (ulRadius)
			{
			InitCornerRaster(upperLeftCorner, ulRadius);
			pFill->Fill(Ctx, x, y, m_Corner[upperLeftCorner].Lines);

			if (cyTopEdge > ulRadius)
				pFill->Fill(Ctx, x, y + ulRadius, cxLeftEdge, cyTopEdge - ulRadius);

			if (cxLeftEdge > ulRadius)
				pFill->Fill(Ctx, x + ulRadius, y, cxLeftEdge - ulRadius, ulRadius);
			}
		else
			pFill->Fill(Ctx, x, y, cxLeftEdge, cyTopEdge);

		//	Paint the upper-right corner

		if (urRadius)
			{
			InitCornerRaster(upperRightCorner, urRadius);
			pFill->Fill(Ctx, x + cxWidth - urRadius, y, m_Corner[upperRightCorner].Lines);

			if (cyTopEdge > urRadius)
				pFill->Fill(Ctx, x + cxWidth - cxRightEdge, y + urRadius, cxRightEdge, cyTopEdge - urRadius);

			if (cxRightEdge > urRadius)
				pFill->Fill(Ctx, x + cxWidth - cxRightEdge, y, cxRightEdge - urRadius, urRadius);
			}
		else
			pFill->Fill(Ctx, x + cxWidth - cxRightEdge, y, cxRightEdge, cyTopEdge);

		//	Paint the lower-left corner

		if (llRadius)
			{
			InitCornerRaster(lowerLeftCorner, llRadius);
			pFill->Fill(Ctx, x, y + cyHeight - llRadius, m_Corner[lowerLeftCorner].Lines);

			if (cyBottomEdge > llRadius)
				pFill->Fill(Ctx, x, y + cyHeight - cyBottomEdge, cxLeftEdge, cyBottomEdge - llRadius);

			if (cxLeftEdge > llRadius)
				pFill->Fill(Ctx, x + llRadius, y + cyHeight - llRadius, cxLeftEdge - llRadius, llRadius);
			}
		else
			pFill->Fill(Ctx, x, y + cyHeight - cyBottomEdge, cxLeftEdge, cyBottomEdge);

		//	Paint the lower-right corner

		if (lrRadius)
			{
			InitCornerRaster(lowerRightCorner, lrRadius);
			pFill->Fill(Ctx, x + cxWidth - lrRadius, y + cyHeight - lrRadius, m_Corner[lowerRightCorner].Lines);

			if (cyBottomEdge > lrRadius)
				pFill->Fill(Ctx, x + cxWidth - cxRightEdge, y + cyHeight - cyBottomEdge, cxRightEdge, cyBottomEdge - lrRadius);

			if (cxRightEdge > lrRadius)
				pFill->Fill(Ctx, x + cxWidth - cxRightEdge, y + cyHeight - lrRadius, cxRightEdge - lrRadius, lrRadius);
			}
		else
			pFill->Fill(Ctx, x + cxWidth - cxRightEdge, y + cyHeight - cyBottomEdge, cxRightEdge, cyBottomEdge);

		//	Paint the top edge

		pFill->Fill(Ctx, x + cxLeftEdge, y, cxInner, cyTopEdge);

		//	Paint the left edge

		pFill->Fill(Ctx, x, y + cyTopEdge, cxLeftEdge, cyInner);

		//	Paint the right edge

		pFill->Fill(Ctx, x + cxWidth - cxRightEdge, y + cyTopEdge, cxRightEdge, cyInner);

		//	Paint the bottom edge

		pFill->Fill(Ctx, x + cxLeftEdge, y + cyHeight - cyBottomEdge, cxInner, cyBottomEdge);

		//	Paint the inside

		pFill->Fill(Ctx, x + cxLeftEdge, y + cyTopEdge, cxInner, cyInner);
		}

	//	Line method

	IAniLineMethod *pLine = GetLineMethod();
	if (pLine)
		{
		pLine->InitPaint(Ctx, x, y, m_Properties);

		//	Draw the edges

		pLine->Line(Ctx, x + ulRadius, y, x + cxWidth - urRadius, y);
		pLine->Line(Ctx, x + cxWidth - 1, y + urRadius, x + cxWidth - 1, y + cyHeight - lrRadius);
		pLine->Line(Ctx, x + llRadius, y + cyHeight - 1, x + cxWidth - lrRadius, y + cyHeight - 1);
		pLine->Line(Ctx, x, y + ulRadius, x, y + cyHeight - llRadius);

		//	Upper-left corner

		if (ulRadius)
			pLine->Arc(Ctx, x + ulRadius, y + ulRadius, ulRadius, 90, 180);
		else
			pLine->Corner(Ctx, x, y);

		//	Upper-right corner

		if (urRadius)
			pLine->Arc(Ctx, x + cxWidth - urRadius, y + urRadius, urRadius, 0, 90);
		else
			pLine->Corner(Ctx, x + cxWidth - 1, y);

		//	Lower-left corner

		if (llRadius)
			pLine->Arc(Ctx, x + llRadius, y + cyHeight - llRadius, llRadius, 180, 270);
		else
			pLine->Corner(Ctx, x, y + cyHeight - 1);

		//	Lower-right corner

		if (lrRadius)
			pLine->Arc(Ctx, x + cxWidth - lrRadius, y + cyHeight - lrRadius, lrRadius, 270, 360);
		else
			pLine->Corner(Ctx, x + cxWidth - 1, y + cyHeight - 1);
		}
	}
