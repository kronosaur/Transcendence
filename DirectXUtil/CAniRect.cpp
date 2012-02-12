//	CAniRect.cpp
//
//	CAniRect class

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

const int INDEX_LINE_PADDING =				4;
#define PROP_LINE_PADDING					CONSTLIT("linePadding")

const int INDEX_OPACITY =					5;
#define PROP_OPACITY						CONSTLIT("opacity")

#define PROP_COLOR							CONSTLIT("color")
#define PROP_LINE_WIDTH						CONSTLIT("lineWidth")

const int PADDING_BOTTOM =					8;
const int PADDING_LEFT =					8;
const int PADDING_RIGHT =					8;
const int PADDING_TOP =						8;


CAniRect::CAniRect (void)

//	CAniRect constructor

	{
	m_Properties.SetInteger(PROP_VISIBLE, 1);
	m_Properties.SetVector(PROP_POSITION, CVector());
	m_Properties.SetVector(PROP_SCALE, CVector(1.0, 1.0));
	m_Properties.SetInteger(PROP_ROTATION, 0);
	m_Properties.SetInteger(PROP_LINE_PADDING, 0);
	m_Properties.SetOpacity(PROP_OPACITY, 255);
	}

void CAniRect::Create (const CVector &vPos,
					   const CVector &vSize,
					   WORD wColor,
					   DWORD dwOpacity,
					   IAnimatron **retpAni)

//	Create
//
//	Creates an element

	{
	CAniRect *pRect = new CAniRect;
	pRect->SetPropertyVector(PROP_POSITION, vPos);
	pRect->SetPropertyVector(PROP_SCALE, vSize);
	pRect->SetPropertyColor(PROP_COLOR, wColor);
	pRect->SetPropertyOpacity(PROP_OPACITY, dwOpacity);

	*retpAni = pRect;
	}

void CAniRect::GetContentRect (RECT *retrcRect)

//	GetContentRect
//
//	Returns a RECT of the content area (relative to the rect itself)

	{
	CVector vScale = m_Properties[INDEX_SCALE].GetVector();
	int iPadding = m_Properties[INDEX_LINE_PADDING].GetInteger();

	retrcRect->left = iPadding + PADDING_LEFT;
	retrcRect->top = iPadding + PADDING_TOP;
	retrcRect->right = (int)vScale.GetX() - (iPadding + PADDING_RIGHT);
	retrcRect->bottom = (int)vScale.GetY() - (iPadding + PADDING_BOTTOM);
	}

void CAniRect::GetSpacingRect (RECT *retrcRect)

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

void CAniRect::Paint (SAniPaintCtx &Ctx)

//	Paint
//
//	Paints the element

	{
	//	Position and size

	CVector vPos = Ctx.ToDest.Transform(m_Properties[INDEX_POSITION].GetVector());
	CVector vPos2 = Ctx.ToDest.Transform(m_Properties[INDEX_POSITION].GetVector() + m_Properties[INDEX_SCALE].GetVector());
	CVector vSize = vPos2 - vPos;

	int x = (int)vPos.GetX();
	int y = (int)vPos.GetY();
	int cxWidth = (int)vSize.GetX();
	int cyHeight = (int)vSize.GetY();

	//	Fill method

	IAniFillMethod *pFill = GetFillMethod();
	if (pFill)
		{
		int iLinePadding = m_Properties[INDEX_LINE_PADDING].GetInteger();

		//	Init

		pFill->InitPaint(Ctx, x, y, m_Properties);

		//	Paint

		pFill->Fill(Ctx, x + iLinePadding, y + iLinePadding, cxWidth - (2 * iLinePadding), cyHeight - (2 * iLinePadding));
		}

	//	Line method

	IAniLineMethod *pLine = GetLineMethod();
	if (pLine)
		{
		pLine->InitPaint(Ctx, x, y, m_Properties);

		//	Draw the edges

		pLine->Line(Ctx, x, y, x + cxWidth, y);
		pLine->Line(Ctx, x + cxWidth - 1, y, x + cxWidth - 1, y + cyHeight);
		pLine->Line(Ctx, x, y + cyHeight - 1, x + cxWidth, y + cyHeight - 1);
		pLine->Line(Ctx, x, y, x, y + cyHeight);

		//	Draw the corners

		pLine->Corner(Ctx, x, y);
		pLine->Corner(Ctx, x + cxWidth - 1, y);
		pLine->Corner(Ctx, x, y + cyHeight - 1);
		pLine->Corner(Ctx, x + cxWidth - 1, y + cyHeight - 1);
		}
	}
