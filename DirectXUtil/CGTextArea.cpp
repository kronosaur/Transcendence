//	CGTextArea.cpp
//
//	Implementation of CGTextArea class

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

#define RGB_CURSOR								(CG16bitImage::RGBValue(255,255,255))

CGTextArea::CGTextArea (void) : 
		m_bEditable(false),
		m_dwStyles(alignLeft),
		m_cyLineSpacing(0),
		m_pFont(NULL),
		m_Color(CG16bitImage::RGBValue(255,255,255)),
		m_bRTFInvalid(true),
		m_pFontTable(NULL),
		m_cxJustifyWidth(0),
		m_iTick(0),
		m_iCursorLine(-1),
		m_iCursorPos(0)

//	CGTextArea constructor

	{
	}

void CGTextArea::FormatRTF (const RECT &rcRect)

//	FormatRTF
//
//	Make sure we are formatted

	{
	if (m_bRTFInvalid)
		{
		SBlockFormatDesc BlockFormat;

		BlockFormat.cxWidth = RectWidth(rcRect);
		BlockFormat.cyHeight = -1;
		BlockFormat.iHorzAlign = alignLeft;
		BlockFormat.iVertAlign = alignTop;
		BlockFormat.iExtraLineSpacing = m_cyLineSpacing;

		BlockFormat.DefaultFormat.wColor = m_Color;
		BlockFormat.DefaultFormat.pFont = m_pFont;

		m_RichText.InitFromRTF(m_sRTF, *m_pFontTable, BlockFormat);

		m_bRTFInvalid = false;
		}
	}

int CGTextArea::Justify (const RECT &rcRect)

//	Justify
//
//	Justify the text and return the height (in pixels)

	{
	if (!m_sText.IsBlank())
		{
		if (m_pFont == NULL)
			return 0;

		if (m_cxJustifyWidth != RectWidth(rcRect))
			{
			m_cxJustifyWidth = RectWidth(rcRect);
			m_Lines.DeleteAll();
			m_pFont->BreakText(m_sText, m_cxJustifyWidth, &m_Lines, CG16bitFont::SmartQuotes);
			}

		return m_Lines.GetCount() * m_pFont->GetHeight() + (m_Lines.GetCount() - 1) * m_cyLineSpacing;
		}
	else
		{
		FormatRTF(rcRect);

		RECT rcBounds;
		m_RichText.GetBounds(&rcBounds);
		return RectHeight(rcBounds);
		}
	}

void CGTextArea::Paint (CG16bitImage &Dest, const RECT &rcRect)

//	Paint
//
//	Handle paint

	{
	//	Paint the editable box

	if (m_bEditable)
		{
		WORD wBorderColor = CG16bitImage::BlendPixel(CG16bitImage::RGBValue(0, 0, 0), m_Color, 128);
		DrawRectDotted(Dest, rcRect.left, rcRect.top, RectWidth(rcRect), RectHeight(rcRect), wBorderColor);
		}

	//	Paint the content

	if (!m_sText.IsBlank())
		PaintText(Dest, rcRect);
	else
		PaintRTF(Dest, rcRect);
	}

void CGTextArea::PaintRTF (CG16bitImage &Dest, const RECT &rcRect)

//	PaintRTF
//
//	Paint RTF

	{
	int i;

	//	Must have a font table

	if (m_pFontTable == NULL)
		return;

	//	Format, if necessary

	FormatRTF(rcRect);

	//	Paint

	for (i = 0; i < m_RichText.GetFormattedSpanCount(); i++)
		{
		const SFormattedTextSpan &Span = m_RichText.GetFormattedSpan(i);
		DWORD dwOpacity = 255;

		Span.Format.pFont->DrawText(Dest,
				rcRect.left + Span.x,
				rcRect.top + Span.y,
				Span.Format.wColor,
				dwOpacity,
				Span.sText);
		}
	}

void CGTextArea::PaintText (CG16bitImage &Dest, const RECT &rcRect)

//	PaintText
//
//	Paint plain text

	{
	//	Paint the text

	if (m_pFont)
		{
		//	If we haven't justified the text for this size, do it now

		if (m_cxJustifyWidth != RectWidth(rcRect))
			{
			m_cxJustifyWidth = RectWidth(rcRect);
			m_Lines.DeleteAll();
			m_pFont->BreakText(m_sText, m_cxJustifyWidth, &m_Lines, CG16bitFont::SmartQuotes);
			}

		//	Compute the rect within which we draw the text

		RECT rcText = rcRect;
		if (m_bEditable)
			{
			int iVSpacing = (RectHeight(rcRect) - m_pFont->GetHeight()) / 2;
			rcText.left += iVSpacing;
			rcText.right -= iVSpacing;
			rcText.top += iVSpacing;
			rcText.bottom -= iVSpacing;
			}

		//	Clip to text rect

		RECT rcOldClip = Dest.GetClipRect();
		Dest.SetClipRect(rcText);

		//	Figure out how many lines fit in the rect

		int iMaxLineCount = RectHeight(rcText) / m_pFont->GetHeight();

		//	If there are too many lines, and we're editable, start at the end

		int iStart = 0;
		if (m_bEditable && iMaxLineCount < m_Lines.GetCount())
			iStart = m_Lines.GetCount() - iMaxLineCount;

		//	Paint each line

		int x = rcText.left;
		int y = rcText.top;
		for (int i = iStart; i < m_Lines.GetCount(); i++)
			{
			CString sLine = m_Lines[i];

			//	Trim the last space in the line, if necessary

			char *pPos = sLine.GetASCIIZPointer();
			if (sLine.GetLength() > 0 && pPos[sLine.GetLength() - 1] == ' ')
				sLine = strTrimWhitespace(sLine);

			//	Alignment

			int xLine;
			if (m_dwStyles & alignCenter)
				{
				int cxWidth = m_pFont->MeasureText(sLine);
				xLine = x + (RectWidth(rcText) - cxWidth) / 2;
				}
			else if (m_dwStyles & alignRight)
				{
				int cxWidth = m_pFont->MeasureText(sLine);
				xLine = x + (RectWidth(rcRect) - cxWidth);
				}
			else
				xLine = x;

			//	Paint

			if (HasEffects())
				m_pFont->DrawTextEffect(Dest, xLine, y, m_Color, sLine, GetEffectCount(), GetEffects());
			else
				Dest.DrawText(xLine, y, *m_pFont, m_Color, sLine);

			//	Next

			y += m_pFont->GetHeight() + m_cyLineSpacing;
			if (y >= rcText.bottom)
				break;
			}

		//	Paint the cursor

		if (m_bEditable && m_iCursorLine >= iStart)
			{
			int cxPos = (m_iCursorLine < m_Lines.GetCount() ? m_pFont->MeasureText(CString(m_Lines[m_iCursorLine].GetASCIIZPointer(), m_iCursorPos, true)) : 0);
			int y = rcText.top + (m_iCursorLine - iStart) * (m_pFont->GetHeight() + m_cyLineSpacing);
			int x = rcText.left;
			if (m_dwStyles & alignCenter)
				{
				int cxWidth = (m_iCursorLine < m_Lines.GetCount() ? m_pFont->MeasureText(m_Lines[m_iCursorLine]) : 0);
				x += ((RectWidth(rcText) - cxWidth) / 2) + cxPos;
				}
			else if (m_dwStyles & alignRight)
				{
				int cxWidth = (m_iCursorLine < m_Lines.GetCount() ? m_pFont->MeasureText(m_Lines[m_iCursorLine]) : 0);
				x += (RectWidth(rcText) - cxWidth) + cxPos;
				}
			else
				x += cxPos;

			if (((m_iTick / 30) % 2) > 0)
				{
				Dest.Fill(x, y, 2, m_pFont->GetHeight(), RGB_CURSOR);
				}
			}

		//	Restore clip

		Dest.SetClipRect(rcOldClip);
		}
	}
