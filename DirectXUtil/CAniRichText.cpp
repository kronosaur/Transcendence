//	CAniRichText.cpp
//
//	CAniRichText class

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

const int INDEX_TEXT =						4;
#define PROP_TEXT 							CONSTLIT("text")

const int INDEX_TEXT_ALIGN_HORZ =			5;
#define PROP_TEXT_ALIGN_HORZ				CONSTLIT("textAlignHorz")

const int INDEX_TEXT_ALIGN_VERT =			6;
#define PROP_TEXT_ALIGN_VERT				CONSTLIT("textAlignVert")

const int INDEX_COLOR =						7;
#define PROP_COLOR							CONSTLIT("color")

const int INDEX_FONT =						8;
#define PROP_FONT							CONSTLIT("font")

#define ALIGN_BOTTOM						CONSTLIT("bottom")
#define ALIGN_CENTER						CONSTLIT("center")
#define ALIGN_LEFT							CONSTLIT("left")
#define ALIGN_RIGHT							CONSTLIT("right")
#define ALIGN_TOP							CONSTLIT("top")

CAniRichText::CAniRichText (const IFontTable &FontTable) :
		m_FontTable(FontTable),
		m_bInvalid(true)

//	CAniRichText constructor

	{
	m_Properties.SetInteger(PROP_VISIBLE, 1);
	m_Properties.SetVector(PROP_POSITION, CVector());
	m_Properties.SetVector(PROP_SCALE, CVector(0.0, 0.0));
	m_Properties.SetInteger(PROP_ROTATION, 0);
	m_Properties.SetString(PROP_TEXT, NULL_STR);
	m_Properties.SetString(PROP_TEXT_ALIGN_HORZ, ALIGN_LEFT);
	m_Properties.SetString(PROP_TEXT_ALIGN_VERT, ALIGN_TOP);
	m_Properties.SetColor(PROP_COLOR, 0xffff);
	m_Properties.SetFont(PROP_FONT, NULL);
	}

void CAniRichText::Format (int cxWidth, int cyHeight)

//	Format
//
//	Format the text

	{
	if (m_bInvalid)
		{
		SBlockFormatDesc BlockFormat;

		BlockFormat.cxWidth = cxWidth;
		BlockFormat.cyHeight = cyHeight;

		CString sAlignHorz = m_Properties[INDEX_TEXT_ALIGN_HORZ].GetString();
		if (strEquals(sAlignHorz, ALIGN_CENTER))
			BlockFormat.iHorzAlign = alignCenter;
		else if (strEquals(sAlignHorz, ALIGN_RIGHT))
			BlockFormat.iHorzAlign = alignRight;
		else
			BlockFormat.iHorzAlign = alignLeft;

		CString sAlignVert = m_Properties[INDEX_TEXT_ALIGN_VERT].GetString();
		if (strEquals(sAlignVert, ALIGN_CENTER))
			BlockFormat.iVertAlign = alignMiddle;
		else if (strEquals(sAlignVert, ALIGN_BOTTOM))
			BlockFormat.iVertAlign = alignBottom;
		else
			BlockFormat.iVertAlign = alignTop;

		BlockFormat.DefaultFormat.wColor = m_Properties[INDEX_COLOR].GetColor();
		BlockFormat.DefaultFormat.pFont = m_Properties[INDEX_FONT].GetFont();

		m_Text.InitFromRTF(m_Properties[INDEX_TEXT].GetString(), m_FontTable, BlockFormat);

		m_bInvalid = false;
		}
	}

void CAniRichText::GetSpacingRect (RECT *retrcRect)

//	GetSpacingRect
//
//	Returns the spacing rect

	{
	CVector vSize = m_Properties[INDEX_SCALE].GetVector();

	Format((int)vSize.GetX(), (int)vSize.GetY());
	m_Text.GetBounds(retrcRect);
	}

void CAniRichText::OnPropertyChanged (const CString &sName)

//	OnPropertyChanged
//
//	Property has changed

	{
	if (strEquals(sName, PROP_TEXT) 
			|| strEquals(sName, PROP_SCALE) 
			|| strEquals(sName, PROP_TEXT_ALIGN_HORZ)
			|| strEquals(sName, PROP_TEXT_ALIGN_VERT))
		m_bInvalid = true;
	}

void CAniRichText::Paint (SAniPaintCtx &Ctx)

//	Paint
//
//	Paints

	{
	int i;

	//	Get some metrics

	CVector vPos = Ctx.ToDest.Transform(m_Properties[INDEX_POSITION].GetVector());
	int x = (int)vPos.GetX();
	int y = (int)vPos.GetY();

	CVector vSize = m_Properties[INDEX_SCALE].GetVector();

	//	Make sure we are formatted

	Format((int)vSize.GetX(), (int)vSize.GetY());

	//	Paint

	for (i = 0; i < m_Text.GetFormattedSpanCount(); i++)
		{
		const SFormattedTextSpan &Span = m_Text.GetFormattedSpan(i);
		DWORD dwOpacity = Span.Format.dwOpacity * Ctx.dwOpacityToDest / 255;

		Span.Format.pFont->DrawText(Ctx.Dest,
				x + Span.x,
				y + Span.y,
				Span.Format.wColor,
				dwOpacity,
				Span.sText);
		}
	}
