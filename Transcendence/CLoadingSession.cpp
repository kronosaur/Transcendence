//	CLoadingSession.cpp
//
//	CLoadingSession class

#include "PreComp.h"
#include "Transcendence.h"

const int STARGATE_WIDTH =						128;
const int STARGATE_HEIGHT =						128;

const int Y_COPYRIGHT_TEXT =					392;

const CG32bitPixel RGB_IMAGE_BACKGROUND =		CG32bitPixel(0, 0, 0);

ALERROR CLoadingSession::OnInit (CString *retsError)

//	OnInit

	{
	ALERROR error;
	const CVisualPalette &VI = m_HI.GetVisuals();

	RECT rcCenter;
	VI.GetWidescreenRect(&rcCenter);

	//	Load a JPEG of the background image

	HBITMAP hDIB;
	if (error = JPEGLoadFromResource(NULL,
			MAKEINTRESOURCE(IDR_TITLE_IMAGE),
			JPEG_LFR_DIB, 
			NULL,
			&hDIB))
		return error;

	bool bSuccess = m_TitleImage.CreateFromBitmap(hDIB);
	::DeleteObject(hDIB);
	if (!bSuccess)
		return ERR_FAIL;

	//	Load stargate image

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

	bSuccess = m_StargateImage.CreateFromBitmap(hDIB, hBitmask);
	::DeleteObject(hDIB);
	::DeleteObject(hBitmask);
	if (!bSuccess)
		return ERR_FAIL;

	//	Figure out position of copyright text.

	m_cyCopyright = Y_COPYRIGHT_TEXT - (Max(0, m_TitleImage.GetHeight() - RectHeight(rcCenter)) / 2);

	//	Figure out the position of the stargate animation

	int cxScreen = g_pHI->GetScreenWidth();
	int cyScreen = g_pHI->GetScreenHeight();

	m_rcStargate.left = (cxScreen - STARGATE_WIDTH) / 2;
	m_rcStargate.right = m_rcStargate.left + STARGATE_WIDTH;

	if (RectHeight(rcCenter) >= 512)
		{
		m_rcStargate.top = rcCenter.bottom - (STARGATE_HEIGHT / 2);
		m_rcStargate.bottom = m_rcStargate.top + STARGATE_HEIGHT;
		}
	else
		{
		m_rcStargate.bottom = cyScreen;
		m_rcStargate.top = m_rcStargate.bottom - STARGATE_HEIGHT;
		}

	return NOERROR;
	}

void CLoadingSession::OnPaint (CG32bitImage &Screen, const RECT &rcInvalid)

//	OnPaint

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumHeavyBoldFont = VI.GetFont(fontMediumHeavyBold);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	RECT rcCenter;
	VI.DrawSessionBackground(Screen, m_TitleImage, RGB_IMAGE_BACKGROUND, 0, &rcCenter);

	//	Paint copyright text

	int cxWidth = MediumHeavyBoldFont.MeasureText(m_sCopyright);
	MediumHeavyBoldFont.DrawText(Screen,
			(Screen.GetWidth() - cxWidth) / 2,
			rcCenter.top + m_cyCopyright,
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

	Screen.Blt(STARGATE_WIDTH * (m_iTick % 48),
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
