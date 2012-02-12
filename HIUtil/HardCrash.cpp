//	HardCrash.cpp
//
//	Shows hard crash session in CHumanInterface object
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"
#include "HIUtil.h"
#include "Utilities.h"

class CHardCrashSession : public IHISession
	{
	public:
		CHardCrashSession (const CString &sTitle, const CString &sDescription);

		//	IHISession virtuals
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags) { g_pHI->Shutdown(); }
		virtual void OnPaint (CG16bitImage &Screen, const RECT &rcInvalid);

	private:
		CString m_sTitle;
		CString m_sDescription;
	};

CHardCrashSession::CHardCrashSession (const CString &sTitle, const CString &sDescription) :
		m_sTitle(sTitle),
		m_sDescription(sDescription)

//	CHardCrashSession constructor

	{
	}

void CHardCrashSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//	OnKeyDown

	{
	switch (iVirtKey)
		{
		case VK_RETURN:
		case VK_ESCAPE:
			g_pHI->Shutdown();
			break;
		}
	}

void CHardCrashSession::OnPaint (CG16bitImage &Screen, const RECT &rcInvalid)

//	OnPaint
//
//	Paint

	{
	const CVisualPalette &VI = g_pHI->GetVisualPalette();
	const CG16bitFont &TitleFont = VI.GetFont(CONSTLIT("DialogTitle"));
	const CG16bitFont &DescFont = VI.GetFont(CONSTLIT("DialogText"));
	WORD wDescColor = VI.GetColor(CONSTLIT("DialogText"));

	Screen.Fill(0, 0, Screen.GetWidth(), Screen.GetHeight(), 0);

	const int FRAME_WIDTH = 512;
	const int FRAME_HEIGHT = 256;
	const int FRAME_INNER_MARGIN = 20;
	const WORD RGB_FRAME_HEADER = CG16bitImage::RGBValue(72, 72, 72);
	const WORD RGB_FRAME_BACKGROUND = CG16bitImage::RGBValue(64, 64, 64);
	const WORD RGB_TITLE = CG16bitImage::RGBValue(0, 0, 0);

	//	Measure out the description

	RECT rcRect;
	rcRect.left = 0;
	rcRect.right = FRAME_WIDTH - (2 * FRAME_INNER_MARGIN);
	rcRect.top = 0;
	int cyDesc;
	DescFont.DrawText(Screen, rcRect, wDescColor, m_sDescription, 0, CG16bitFont::AlignCenter | CG16bitFont::MeasureOnly, &cyDesc);

	//	Compute the height

	int cyHeader = (FRAME_HEIGHT / 3);
	int cyFrameHeight = Max(FRAME_HEIGHT, cyHeader + (4 * FRAME_INNER_MARGIN) + cyDesc);

	//	Compute some metrics

	RECT rcFrame;
	rcFrame.left = (Screen.GetWidth() - FRAME_WIDTH) / 2;
	rcFrame.right = rcFrame.left + FRAME_WIDTH;
	rcFrame.top = (Screen.GetHeight() - cyFrameHeight) / 2;
	rcFrame.bottom = rcFrame.top + cyFrameHeight;

	//	Paint the background frame

	Screen.Fill(rcFrame.left, rcFrame.top, FRAME_WIDTH, cyHeader, RGB_FRAME_HEADER);
	Screen.Fill(rcFrame.left, rcFrame.top + cyHeader, FRAME_WIDTH, cyFrameHeight - cyHeader, RGB_FRAME_BACKGROUND);

	//	Paint the error title

	rcRect = rcFrame;
	rcRect.top += cyHeader - TitleFont.GetAscent() - FRAME_INNER_MARGIN;
	TitleFont.DrawText(Screen, rcRect, RGB_TITLE, m_sTitle, 0, CG16bitFont::AlignCenter | CG16bitFont::TruncateLine);

	//	Paint the description

	rcRect = rcFrame;
	rcRect.top += cyHeader + FRAME_INNER_MARGIN;
	rcRect.left += FRAME_INNER_MARGIN;
	rcRect.right -= FRAME_INNER_MARGIN;
	DescFont.DrawText(Screen, rcRect, wDescColor, m_sDescription, 0, CG16bitFont::AlignCenter);
	}

void CHumanInterface::ShowHardCrashSession (const CString &sTitle, const CString &sDescription)

//	ShowHardCrashSession
//
//	Show the session

	{
	IHISession *pSession = new CHardCrashSession(sTitle, sDescription);
	ShowSession(pSession);
	}
