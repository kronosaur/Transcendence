//	CGDetailsArea.cpp
//
//	CGDetailsArea class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define FIELD_DETAILS						CONSTLIT("details")
#define FIELD_LARGE_ICON					CONSTLIT("largeIcon")

CGDetailsArea::CGDetailsArea (const CVisualPalette &VI) : 
		m_VI(VI)

//	CGDetailsArea constructor

	{
	}

void CGDetailsArea::CleanUp (void)

//	CleanUp
//
//	Clean up area

	{
	m_pData.Delete();
	}

void CGDetailsArea::Paint (CG32bitImage &Dest, const RECT &rcRect)

//	Paint
//
//	Paint the detail.

	{
	DEBUG_TRY

	CCodeChain &CC = g_pUniverse->GetCC();

	//	Colors and metrics

    CG32bitPixel rgbFadeBackColor = CG32bitPixel(CG32bitPixel::Darken(m_rgbBackColor, 220), 185);

	//	Now paint the selected content

	if (m_pData && !m_pData->IsNil() && m_pData->IsSymbolTable())
		{
		//	Figure out some metrics about details.

		CDetailList Details(m_VI);
		Details.SetColor(m_rgbTextColor);

		ICCItem *pDetails = m_pData->GetElement(FIELD_DETAILS);
		if (pDetails)
			Details.Load(pDetails);

		int cyDetails;
		Details.Format(rcRect, &cyDetails);
		int cyPaneSplit = (cyDetails ? RectHeight(rcRect) - (cyDetails + SPACING_Y) : 0);

		//	Paint the large icon first (we paint the background on top so that 
		//	the details show up.

		ICCItem *pLargeIcon = m_pData->GetElement(FIELD_LARGE_ICON);
		if (pLargeIcon)
			{
			RECT rcImage;
			DWORD dwImage = CTLispConvert::AsImageDesc(pLargeIcon, &rcImage);
			CG32bitImage *pImage = g_pUniverse->GetLibraryBitmap(dwImage);

			int cxImage = RectWidth(rcImage);
			int cyImage = RectHeight(rcImage);

			if (pImage)
				{
				int x = rcRect.left + (RectWidth(rcRect) - m_cxLargeIcon) / 2;
				int y = rcRect.top;

				CPaintHelper::PaintScaledImage(Dest, x, y, m_cxLargeIcon, m_cyLargeIcon, *pImage, rcImage);
				}
			}

		//	Paint background

		CGDraw::RoundedRect(Dest,
				rcRect.left,
				rcRect.top,
				RectWidth(rcRect),
				RectHeight(rcRect),
				BORDER_RADIUS + 1,
				rgbFadeBackColor);

		//	Paint the details

		Details.Paint(Dest);
		}

	//	Otherwise, just paint a blank area

	else
		{
		CGDraw::RoundedRect(Dest,
				rcRect.left,
				rcRect.top,
				RectWidth(rcRect),
				RectHeight(rcRect),
				BORDER_RADIUS + 1,
				rgbFadeBackColor);
		}

	//	Paint a frame

	CGDraw::RoundedRectOutline(Dest,
			rcRect.left,
			rcRect.top,
			RectWidth(rcRect),
			RectHeight(rcRect),
			BORDER_RADIUS,
			1,
			CG32bitPixel(80,80,80));

	DEBUG_CATCH
	}
