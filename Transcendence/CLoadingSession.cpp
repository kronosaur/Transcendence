//	CLoadingSession.cpp
//
//	CLoadingSession class

#include "PreComp.h"
#include "Transcendence.h"

const int STARGATE_WIDTH =						128;
const int STARGATE_HEIGHT =						128;

const int Y_COPYRIGHT_TEXT =					392;

ALERROR CLoadingSession::OnInit (CString *retsError)

//	OnInit

	{
	ALERROR error;
	const CVisualPalette &VI = m_HI.GetVisuals();

	//	Load a JPEG of the background image

	HBITMAP hDIB;
	if (error = JPEGLoadFromResource(g_hInst,
			MAKEINTRESOURCE(IDR_TITLE_IMAGE),
			JPEG_LFR_DIB, 
			NULL,
			&hDIB))
		return error;

	error = m_TitleImage.CreateFromBitmap(hDIB);
	::DeleteObject(hDIB);
	if (error)
		return error;

	//	Load stargate image

	if (error = JPEGLoadFromResource(g_hInst,
			MAKEINTRESOURCE(IDR_STARGATE_IMAGE),
			JPEG_LFR_DIB, 
			NULL, 
			&hDIB))
		return error;

	HBITMAP hBitmask;
	if (error = dibLoadFromResource(g_hInst,
			MAKEINTRESOURCE(IDR_STARGATE_MASK),
			&hBitmask))
		return error;

	error = m_StargateImage.CreateFromBitmap(hDIB, hBitmask);
	::DeleteObject(hDIB);
	::DeleteObject(hBitmask);
	if (error)
		return error;

	//	Figure out the position of the stargate animation

	int cxScreen = g_pHI->GetScreen().GetWidth();
	int cyScreen = g_pHI->GetScreen().GetHeight();

	m_rcStargate.left = (cxScreen - STARGATE_WIDTH) / 2;
	m_rcStargate.right = m_rcStargate.left + STARGATE_WIDTH;
	m_rcStargate.top = cyScreen - ((cyScreen - m_TitleImage.GetHeight()) / 2) - STARGATE_HEIGHT / 2;
	m_rcStargate.bottom = m_rcStargate.top + STARGATE_HEIGHT;

	return NOERROR;
	}

void CLoadingSession::OnPaint (CG16bitImage &Screen, const RECT &rcInvalid)

//	OnPaint

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumHeavyBoldFont = VI.GetFont(fontMediumHeavyBold);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	RECT rcCenter;
	VI.DrawSessionBackground(Screen, m_TitleImage, 0, &rcCenter);

	//	Paint copyright text

	int cxWidth = MediumHeavyBoldFont.MeasureText(m_sCopyright);
	MediumHeavyBoldFont.DrawText(Screen,
			(Screen.GetWidth() - cxWidth) / 2,
			rcCenter.top + Y_COPYRIGHT_TEXT,
			VI.GetColor(colorTextHighlight),
			m_sCopyright);

	//	Paint the loading title

	CString sLoading = CONSTLIT("Loading");
	cxWidth = SubTitleFont.MeasureText(sLoading, NULL);
	SubTitleFont.DrawText(Screen,
			(Screen.GetWidth() - cxWidth) / 2,
			m_rcStargate.bottom,
			VI.GetColor(colorTextFade),
			sLoading);

	//	Paint the stargate

	Screen.ColorTransBlt(STARGATE_WIDTH * ((m_iTick / 2) % 12),
			0,
			STARGATE_WIDTH,
			STARGATE_HEIGHT,
			255,
			m_StargateImage,
			m_rcStargate.left,
			m_rcStargate.top);
	}

void CLoadingSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash
//
//	Describe current state

	{
	*retsMessage = CONSTLIT("session: CLoadingSession\r\n");
	}

void CLoadingSession::OnUpdate (bool bTopMost)

//	OnUpdate

	{
	if (bTopMost)
		{
		m_iTick++;
		HIInvalidate(m_rcStargate);
		}
	}
