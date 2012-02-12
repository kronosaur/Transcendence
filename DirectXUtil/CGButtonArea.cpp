//	CGButtonArea.cpp
//
//	Implementation of CGButtonArea class

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

#define RGB_DISABLED_TEXT						(CG16bitImage::RGBValue(128,128,128))

CGButtonArea::CGButtonArea (void) : m_pLabelFont(NULL),
		m_bMouseOver(false),
		m_bDisabled(false),
		m_iAccelerator(-1)

//	CGButtonArea constructor

	{
	m_wLabelColor = CG16bitImage::RGBValue(255,255,255);
	}

void CGButtonArea::LButtonUp (int x, int y)

//	LButtonUp
//
//	Mouse button up

	{
	if (m_bMouseOver && !m_bDisabled && IsVisible())
		SignalAction();
	}

void CGButtonArea::MouseEnter (void)

//	MouseEnter
//
//	Mouse has entered the area

	{
	m_bMouseOver = true;
	Invalidate();
	}

void CGButtonArea::MouseLeave (void)

//	MouseLeave
//
//	Mouse has left the area

	{
	m_bMouseOver = false;
	Invalidate();
	}

void CGButtonArea::OnSetRect (void)

//	OnSetRect
//
//	Handle moves

	{
	//	If the mouse is over our area, then set state
	}

void CGButtonArea::Paint (CG16bitImage &Dest, const RECT &rcRect)

//	Paint
//
//	Handle paint

	{
	if (m_pLabelFont)
		{
		if (m_bMouseOver && !m_bDisabled)
			{
			Dest.Fill(rcRect.left, rcRect.top, RectWidth(rcRect), RectHeight(rcRect), CG16bitImage::RGBValue(128,128,128));
			}

		//	If we're disabled, paint gray

		if (m_bDisabled)
			Dest.DrawText(rcRect.left,
					rcRect.top,
					*m_pLabelFont,
					RGB_DISABLED_TEXT,
					m_sLabel);

		//	If we have an accelerator, paint in pieces

		else if (m_iAccelerator != -1)
			{
			char *pPos = m_sLabel.GetASCIIZPointer();
			int x = rcRect.left;

			if (m_iAccelerator > 0)
				Dest.DrawText(x, rcRect.top, *m_pLabelFont, m_wLabelColor, CString(pPos, m_iAccelerator, true), 0, &x);

			Dest.DrawText(x, rcRect.top, *m_pLabelFont, CG16bitImage::RGBValue(255,255,0), CString(pPos + m_iAccelerator, 1, true), 0, &x);
			Dest.DrawText(x, rcRect.top, *m_pLabelFont, m_wLabelColor, CString(pPos + m_iAccelerator + 1, m_sLabel.GetLength() - m_iAccelerator - 1, true));
			}
		else
			Dest.DrawText(rcRect.left,
					rcRect.top,
					*m_pLabelFont,
					m_wLabelColor,
					m_sLabel);
		}
	}

void CGButtonArea::SetLabelAccelerator (const CString &sKey)

//	SetLabelAccelerator
//
//	Sets the key to highlight as accelerator.

	{
	if (sKey.IsBlank())
		m_iAccelerator = -1;
	else
		{
		char *pStart = m_sLabel.GetASCIIZPointer();
		char *pPos = pStart;
		char *pKey = sKey.GetASCIIZPointer();
		bool bFirstLetter = true;

		//	First look for the accelerator as the first letter in a word

		while (*pPos != '\0')
			{
			if (bFirstLetter && CharLower((LPTSTR)(BYTE)(*pKey)) == CharLower((LPTSTR)(BYTE)(*pPos)))
				{
				m_iAccelerator = pPos - pStart;
				return;
				}

			if (bFirstLetter)
				{
				//	If we have a quote, then it still counts as the first letter
				if (*pPos != '\'' && *pPos != '\"' && *pPos != ' ')
					bFirstLetter = false;
				}
			else
				bFirstLetter = (*pPos == ' ');

			pPos++;
			}

		//	Otherwise, look for the any matching letter

		m_iAccelerator = strFind(m_sLabel, sKey);
		}
	}
