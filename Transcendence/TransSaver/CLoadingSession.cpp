//	CLoadingSessions.cpp
//
//	CLoadingSessions class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

const int STARGATE_WIDTH =						128;
const int STARGATE_HEIGHT =						128;

const int STARGATE_FRAME_COUNT =				12;

CLoadingSession::CLoadingSession (CHumanInterface &HI, CTransSaverModel &Model) : IHISession(HI),
		m_Model(Model)

//	CLoadingSession constructor

	{
	}

void CLoadingSession::OnCleanUp (void)

//	OnCleanUp
//
//	Clean up

	{
	}

ALERROR CLoadingSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	ALERROR error;

	//	Load stargate image

	HBITMAP hDIB;
	if (error = JPEGLoadFromResource(NULL,
			MAKEINTRESOURCE(IDR_STARGATE_IMAGE),
			JPEG_LFR_DIB, 
			NULL, 
			&hDIB))
		return error;

	HBITMAP hBitmask;
	if (error = dibLoadFromResource(NULL,
			MAKEINTRESOURCE(IDR_STARGATE_MASK),
			&hBitmask))
		return error;

	error = m_StargateImage.CreateFromBitmap(hDIB, hBitmask);
	::DeleteObject(hDIB);
	::DeleteObject(hBitmask);
	if (error)
		return error;

	//	Start

	m_iTick = 0;

	return NOERROR;
	}

void CLoadingSession::OnPaint (CG16bitImage &Screen, const RECT &rcInvalid)

//	OnPaint
//
//	Paint the screen

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &LogoFont = VI.GetFont(fontLogoTitle);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);
	WORD wHighlightColor = VI.GetColor(colorTextHighlight);
	WORD wNormalColor = VI.GetColor(colorTextNormal);

	//	Paint the background

	Screen.Fill(rcInvalid.left, rcInvalid.top, RectWidth(rcInvalid), RectHeight(rcInvalid), CG16bitImage::RGBValue(0, 0, 0));

	//	Paint the stargate

	Screen.ColorTransBlt(STARGATE_WIDTH * ((m_iTick / 2) % STARGATE_FRAME_COUNT),
			0,
			STARGATE_WIDTH,
			STARGATE_HEIGHT,
			255,
			m_StargateImage,
			m_rcStargate.left,
			m_rcStargate.top);

	//	Paint the text

	int x = m_HI.GetScreen().GetWidth() / 2;
	int y = m_rcStargate.bottom + LogoFont.GetHeight() / 2;

	LogoFont.DrawText(Screen, x, y, wHighlightColor, 255, CONSTLIT("TRANSCENDENCE"), CG16bitFont::AlignCenter);
	MediumFont.DrawText(Screen, x + LogoFont.MeasureText(CONSTLIT("TRANSCENDENCE")) / 2, y + MediumFont.GetAscent() / 2, wHighlightColor, 255, CONSTLIT("®"), 0);
	y += LogoFont.GetHeight();

	CString sText = strPatternSubst(CONSTLIT("%s %s"), m_Model.GetProductName(), m_Model.GetVersion());
	MediumFont.DrawText(Screen, x, y, wNormalColor, 255, sText, CG16bitFont::AlignCenter);
	y += MediumFont.GetHeight();

	MediumFont.DrawText(Screen, x, y, wNormalColor, 255, m_Model.GetCopyright(), CG16bitFont::AlignCenter);
	y += MediumFont.GetHeight();

	y += SubTitleFont.GetHeight();
	SubTitleFont.DrawText(Screen, x, y, wHighlightColor, 255, CONSTLIT("http://transcendence-game.com"), CG16bitFont::AlignCenter);
	}

void CLoadingSession::OnSize (int cxWidth, int cyHeight)

//	OnSize
//
//	Sets the size of the screen

	{
	m_rcScreen.left = 0;
	m_rcScreen.right = cxWidth;
	m_rcScreen.top = 0;
	m_rcScreen.bottom = cyHeight;

	//	Compute the position of the stargate

	m_rcStargate.left = (cxWidth - STARGATE_WIDTH) / 2;
	m_rcStargate.right = m_rcStargate.left + STARGATE_WIDTH;
	m_rcStargate.top = (cyHeight - STARGATE_HEIGHT) / 2;
	m_rcStargate.bottom = m_rcStargate.top + STARGATE_HEIGHT;
	}

void CLoadingSession::OnUpdate (bool bTopMost)

//	OnUpdate
//
//	Update

	{
	m_iTick++;
	HIInvalidate(m_rcStargate);
	}
