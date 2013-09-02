//	COutputChart.cpp
//
//	COutputChart class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

static CG16bitFont NULL_FONT;

CG16bitImage &COutputChart::GetOutputImage (int *retxOrigin, int *retyOrigin)

//	GetOutputImage
//
//	Returns the output image and the origin of the content region.

	{
	*retxOrigin = 0;
	*retyOrigin = 0;
	return m_Image;
	}

WORD COutputChart::GetStyleColor (const CString &sStyle)

//	GetStyleColor
//
//	Returns the color of the given style.

	{
	SStyleDesc *pStyle = m_Styles.GetAt(sStyle);
	if (pStyle == NULL)
		return 0;

	return pStyle->wColor;
	}

const CG16bitFont &COutputChart::GetStyleFont (const CString &sStyle)

//	GetStyleFont
//
//	Returns the font of the given style.

	{
	SStyleDesc *pStyle = m_Styles.GetAt(sStyle);
	if (pStyle == NULL)
		return NULL_FONT;

	return pStyle->Font;
	}

bool COutputChart::Output (void)

//	Output
//
//	Outputs the chart to the clipboard or to a file.

	{
	if (!m_sFilespec.IsBlank())
		{
		if (m_Image.SaveAsWindowsBMP(m_sFilespec) != NOERROR)
			{
			printf("Unable to save to '%s'\n", m_sFilespec.GetASCIIZPointer());
			return false;
			}

		printf("%s\n", m_sFilespec.GetASCIIZPointer());
		}

	//	Otherwise, clipboard

	else
		{
		if (m_Image.CopyToClipboard() != NOERROR)
			{
			printf("Unable to copy to clipboard.\n");
			return false;
			}

		printf("Image copied to clipboard.\n");
		}

	//	Done

	return true;
	}

void COutputChart::SetContentSize (int cxWidth, int cyHeight)

//	SetContentSize
//
//	Sets the content size. This will reallocate the image, so all previous output
//	is lost.

	{
	m_Image.CreateBlank(cxWidth, cyHeight, false);
	}

void COutputChart::SetStyleFont (const CString &sStyle, const CString &sFontDesc)

//	SetStyleFont
//
//	Sets the font of the given style.

	{
	SStyleDesc *pStyle = m_Styles.SetAt(sStyle);

	//	Font for text

	CString sTypeface;
	int iSize;
	bool bBold;
	bool bItalic;
	if (!CG16bitFont::ParseFontDesc(sFontDesc,
			&sTypeface,
			&iSize,
			&bBold,
			&bItalic))
		{
		sTypeface = CONSTLIT("Arial");
		iSize = 10;
		bBold = false;
		bItalic = false;
		}

	pStyle->Font.Create(sTypeface, -PointsToPixels(iSize), bBold, bItalic);
	}

void COutputChart::SetStyleColor (const CString &sStyle, WORD wColor)

//	SetStyleColor
//
//	Sets the color for the given style.

	{
	SStyleDesc *pStyle = m_Styles.SetAt(sStyle);
	pStyle->wColor = wColor;
	}
