//	CVisualPalette.cpp
//
//	CVisualPalette class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"
#include "HIUtil.h"
#include "Utilities.h"

#define DEFAULT_STYLE						CONSTLIT("Default")

CVisualPalette::CVisualPalette (void)

//	CVisualPalette constructor

	{
	//	Create some default entries

	AddColor(DEFAULT_STYLE, CG16bitImage::RGBValue(128, 128, 128));
	AddFont(DEFAULT_STYLE, CONSTLIT("Arial"), -15);
	}

void CVisualPalette::AddColor (const CString &sStyle, WORD wColor)

//	AddColor
//
//	Adds a color to the palette

	{
	WORD *pColor = m_ColorTable.Set(sStyle);
	*pColor = wColor;
	}

void CVisualPalette::AddFont (const CString &sStyle, const CString &sTypeface, int iSize, bool bBold, bool bItalic, bool bUnderline)

//	AddFont
//
//	Adds a font to the palette

	{
	CG16bitFont *pFont = m_FontTable.Set(sStyle);
	pFont->Create(sTypeface, iSize, bBold, bItalic, bUnderline);
	}

WORD CVisualPalette::GetColor (const CString &sStyle) const

//	GetColor
//
//	Returns a color

	{
	WORD *pColor = m_ColorTable.Find(sStyle);
	if (pColor == NULL)
		return GetColor(DEFAULT_STYLE);

	return *pColor;
	}

const CG16bitFont &CVisualPalette::GetFont (const CString &sStyle) const

//	GetFont
//
//	Returns a font

	{
	CG16bitFont *pFont = m_FontTable.Find(sStyle);
	if (pFont == NULL)
		return GetFont(DEFAULT_STYLE);

	return *pFont;
	}
