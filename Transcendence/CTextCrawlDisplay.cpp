//	CTextCrawlDisplay.cpp
//
//	CTextCrawlDisplay class

#include "PreComp.h"
#include "Transcendence.h"

CTextCrawlDisplay::CTextCrawlDisplay (void)

//	CTextCrawlDisplay constructor

	{
	}

CTextCrawlDisplay::~CTextCrawlDisplay (void)

//	CTextCrawlDisplay destructor

	{
	}

void CTextCrawlDisplay::CleanUp (void)

//	CleanUp
//
//	Clean up the display and release all memory

	{
	}

ALERROR CTextCrawlDisplay::Init (const RECT &rcRect, const CString &sText)

//	Init
//
//	Initialize the display

	{
	m_rcRect = rcRect;
	m_yPos = 0;

	//	Parse the string into multiple lines

	m_EpilogLines.DeleteAll();
	m_pFont->BreakText(sText, RectWidth(rcRect), &m_EpilogLines);

	//	Center the text in the region

	int cyHeight = m_EpilogLines.GetCount() * m_pFont->GetHeight() + 1;
	m_rcText.left = m_rcRect.left;
	m_rcText.right = m_rcRect.right;
	m_rcText.top = m_rcRect.top + (RectHeight(m_rcRect) - cyHeight) / 2;
	m_rcText.bottom = m_rcRect.bottom;

	return NOERROR;
	}

void CTextCrawlDisplay::Paint (CG32bitImage &Dest)

//	Paint
//
//	Paint the display

	{
	Dest.SetClipRect(m_rcText);

	//	Paint all the lines

	int y = m_rcText.bottom - m_yPos;
	for (int i = 0; i < m_EpilogLines.GetCount(); i++)
		{
		m_pFont->DrawText(Dest,
				m_rcText.left + 1,
				y + 1,
				CG32bitPixel(64, 64, 64),
				m_EpilogLines[i]);

		m_pFont->DrawText(Dest,
				m_rcText.left,
				y,
				CG32bitPixel(255, 255, 200),
				m_EpilogLines[i]);

		y += m_pFont->GetHeight();
		if (y > m_rcText.bottom)
			break;
		}

	Dest.ResetClipRect();
	}

void CTextCrawlDisplay::Update (void)

//	Update
//
//	Animate the display

	{
	if (m_yPos < RectHeight(m_rcText))
		m_yPos++;
	}
