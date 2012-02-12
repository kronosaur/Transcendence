//	CAniText.cpp
//
//	CAniText class

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

const int INDEX_COLOR =						4;
#define PROP_COLOR							CONSTLIT("color")

const int INDEX_OPACITY =					5;
#define PROP_OPACITY						CONSTLIT("opacity")

const int INDEX_TEXT =						6;
#define PROP_TEXT 							CONSTLIT("text")

const int INDEX_TEXT_ALIGN_HORZ =			7;
#define PROP_TEXT_ALIGN_HORZ				CONSTLIT("textAlignHorz")

const int INDEX_TEXT_ALIGN_VERT =			8;
#define PROP_TEXT_ALIGN_VERT				CONSTLIT("textAlignVert")

const int INDEX_FONT =						9;
#define PROP_FONT							CONSTLIT("font")

#define ALIGN_BOTTOM						CONSTLIT("bottom")
#define ALIGN_CENTER						CONSTLIT("center")
#define ALIGN_LEFT							CONSTLIT("left")
#define ALIGN_RIGHT							CONSTLIT("right")
#define ALIGN_TOP							CONSTLIT("top")

CAniText::CAniText (void) :
		m_dwFontFlags(0)

//	CAniText constructor

	{
	m_Properties.SetInteger(PROP_VISIBLE, 1);
	m_Properties.SetVector(PROP_POSITION, CVector());
	m_Properties.SetVector(PROP_SCALE, CVector(0.0, 0.0));
	m_Properties.SetInteger(PROP_ROTATION, 0);
	m_Properties.SetColor(PROP_COLOR, 0xffff);
	m_Properties.SetOpacity(PROP_OPACITY, 255);
	m_Properties.SetString(PROP_TEXT, NULL_STR);
	m_Properties.SetString(PROP_TEXT_ALIGN_HORZ, ALIGN_LEFT);
	m_Properties.SetString(PROP_TEXT_ALIGN_VERT, ALIGN_TOP);
	m_Properties.SetFont(PROP_FONT, NULL);
	}

void CAniText::Create (const CString &sText,
					   const CVector &vPos,
					   const CG16bitFont *pFont,
					   DWORD dwFontFlags,
					   WORD wColor,
					   IAnimatron **retpAni)

//	Create
//
//	Creates text with basic attributes

	{
	CAniText *pText = new CAniText;
	pText->SetPropertyString(PROP_TEXT, sText);
	pText->SetPropertyVector(PROP_POSITION, vPos);
	pText->SetPropertyFont(PROP_FONT, pFont);
	pText->SetFontFlags(dwFontFlags);
	pText->SetPropertyColor(PROP_COLOR, wColor);

	if (dwFontFlags & CG16bitFont::AlignCenter)
		pText->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_CENTER);
	else if (dwFontFlags & CG16bitFont::AlignRight)
		pText->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_RIGHT);

	*retpAni = pText;
	}

void CAniText::GetSpacingRect (RECT *retrcRect)

//	GetSpacingRect
//
//	Returns the spacing rect

	{
	const CG16bitFont *pFont = m_Properties[INDEX_FONT].GetFont();
	if (pFont == NULL)
		return IAnimatron::GetSpacingRect(retrcRect);

	CVector vSize = m_Properties[INDEX_SCALE].GetVector();
	int cxWidth = (int)vSize.GetX();

	//	Wrap?

	if (cxWidth > 0)
		{
		int iLines = pFont->BreakText(m_Properties[INDEX_TEXT].GetString(), cxWidth, NULL, 0);

		retrcRect->left = 0;
		retrcRect->top = 0;

		if (iLines > 1)
			{
			retrcRect->right = cxWidth;
			retrcRect->bottom = iLines * pFont->GetHeight();
			}
		else
			retrcRect->right = pFont->MeasureText(m_Properties[INDEX_TEXT].GetString(), (int *)&retrcRect->bottom);
		}
	else
		{
		retrcRect->left = 0;
		retrcRect->top = 0;
		retrcRect->right = pFont->MeasureText(m_Properties[INDEX_TEXT].GetString(), (int *)&retrcRect->bottom);
		}
	}

void CAniText::Paint (SAniPaintCtx &Ctx)

//	Paint
//
//	Paints

	{
	int i;

	const CG16bitFont *pFont = m_Properties[INDEX_FONT].GetFont();
	if (pFont == NULL)
		pFont = &CReanimator::GetDefaultFont();

	//	Get the rect size

	CVector vPos = Ctx.ToDest.Transform(m_Properties[INDEX_POSITION].GetVector());
	CVector vSize = m_Properties[INDEX_SCALE].GetVector();

	RECT rcRect;
	rcRect.left = (int)vPos.GetX();
	rcRect.top = (int)vPos.GetY();
	rcRect.right = rcRect.left + (int)vSize.GetX();
	rcRect.bottom = rcRect.top + (int)vSize.GetY();

	bool bWrap = RectWidth(rcRect) > 0;
	bool bVertAlign = RectHeight(rcRect) > 0;

	//	Get color & opacity

	WORD wColor = m_Properties[INDEX_COLOR].GetColor();
	DWORD dwOpacity = m_Properties[INDEX_OPACITY].GetOpacity() * Ctx.dwOpacityToDest / 255;

	//	Get the font flags

	DWORD dwFlags = m_dwFontFlags;
	CString sAlignHorz = m_Properties[INDEX_TEXT_ALIGN_HORZ].GetString();
	if (strEquals(sAlignHorz, ALIGN_CENTER))
		dwFlags |= CG16bitFont::AlignCenter;
	else if (strEquals(sAlignHorz, ALIGN_RIGHT))
		dwFlags |= CG16bitFont::AlignRight;

	//	See if we need to wrap the text

	int x;
	TArray<CString> Lines;
	if (bWrap)
		{
		pFont->BreakText(m_Properties[INDEX_TEXT].GetString(), RectWidth(rcRect), &Lines, dwFlags);

		if (dwFlags & CG16bitFont::AlignCenter)
			x = rcRect.left + RectWidth(rcRect) / 2;
		else if (dwFlags & CG16bitFont::AlignRight)
			x = rcRect.right;
		else
			x = rcRect.left;
		}
	else
		{
		x = rcRect.left;
		Lines.Insert(m_Properties[INDEX_TEXT].GetString());
		}

	//	Paint

	if (bVertAlign)
		{
		CString sAlignVert = m_Properties[INDEX_TEXT_ALIGN_VERT].GetString();
		int cyHeight = pFont->GetHeight() * Lines.GetCount();

		//	Compute the x and y

		int y = rcRect.top;
		if (strEquals(sAlignVert, ALIGN_CENTER))
			y += (RectHeight(rcRect) - cyHeight) / 2;
		else if (strEquals(sAlignVert, ALIGN_BOTTOM))
			y += RectHeight(rcRect) - cyHeight;

		//	Paint

		for (i = 0; i < Lines.GetCount(); i++)
			{
			pFont->DrawText(Ctx.Dest,
					x,
					y,
					wColor,
					dwOpacity,
					Lines[i],
					dwFlags);

			y += pFont->GetHeight();
			}
		}
	else
		{
		int y = rcRect.top;

		for (i = 0; i < Lines.GetCount(); i++)
			{
			pFont->DrawText(Ctx.Dest, 
					x, 
					y,
					wColor, 
					dwOpacity, 
					Lines[i], 
					dwFlags);

			y += pFont->GetHeight();
			}
		}
	}
