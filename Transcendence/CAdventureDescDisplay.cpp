//	CAdventureDescDisplay.cpp
//
//	CAdventureDescDisplay class

#include "PreComp.h"
#include "Transcendence.h"

const WORD RGB_BACKGROUND =					CG16bitImage::RGBValue(0, 0, 0);
const WORD RGB_CLASS_NAME =					CG16bitImage::RGBValue(128, 128, 128);
const WORD RGB_DESCRIPTION =				CG16bitImage::RGBValue(128, 128, 128);
const WORD RGB_ITEM_TITLE =					CG16bitImage::RGBValue(255, 255, 255);
const WORD RGB_ITEM_DESC =					CG16bitImage::RGBValue(128, 128, 128);
const WORD RGB_ITEM_NUMBER =				CG16bitImage::RGBValue(200, 200, 200);

const int TEXT_AREA_HEIGHT =				160;
const int TEXT_AREA_WIDTH =					640;

const int MARGIN_Y =						8;

CAdventureDescDisplay::CAdventureDescDisplay (void) : 
		m_bInvalid(true), 
		m_iCurrentIndex(-1)

//	CAdventureDescDisplay constructor

	{
	}

CAdventureDescDisplay::~CAdventureDescDisplay (void)

//	CAdventureDescDisplay destructor

	{
	CleanUp();
	}

void CAdventureDescDisplay::CleanUp (void)

//	CleanUp
//
//	Must be called to release resources

	{
	//	NOTE: We only do this to save memory. The destructor will
	//	automatically free the memory.

	m_Buffer.Destroy();
	}

ALERROR CAdventureDescDisplay::Init (CTranscendenceWnd *pTrans, const RECT &rcRect)

//	Init
//
//	Must be called to initialize

	{
	ALERROR error;

	m_rcRect = rcRect;
	m_pTrans = pTrans;
	m_pFonts = &pTrans->GetFonts();

	//	Create the off-screen buffer

	if (error = m_Buffer.CreateBlank(RectWidth(rcRect), RectHeight(rcRect), false))
		return error;

	//	Get the list of adventures

	g_pUniverse->GetAllAdventures(&m_AdventureList);

	//	Start at the first adventure available

	m_iCurrentIndex = 0;

	//	Init

	m_bInvalid = true;

	return NOERROR;
	}

bool CAdventureDescDisplay::OnKeyDown (int iVirtKey)

//	OnKeyDown
//
//	Handle keydown

	{
	switch (iVirtKey)
		{
		case VK_RIGHT:
		case VK_DOWN:
			SelectNext();
			break;

		case VK_LEFT:
		case VK_UP:
			SelectPrev();
			break;
		}

	return false;
	}

void CAdventureDescDisplay::Paint (CG16bitImage &Dest)

//	Paint
//
//	Paints the display

	{
	if (m_bInvalid)
		{
		PaintBuffer();
		m_bInvalid = false;
		}

	Dest.Blt(0,
			0,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			m_Buffer,
			m_rcRect.left,
			m_rcRect.top);
	}

void CAdventureDescDisplay::PaintBuffer (void)

//	PaintBuffer
//
//	Paint off-screen buffer

	{
	CExtension *pAdventure = m_AdventureList[m_iCurrentIndex];
	RECT rcFull;
	rcFull.left = 0;
	rcFull.top = 0;
	rcFull.right = RectWidth(m_rcRect);
	rcFull.bottom = RectHeight(m_rcRect);

	//	Erase

	m_Buffer.Fill(0, 0, RectWidth(m_rcRect), RectHeight(m_rcRect), 0);

	RECT rcTextArea = rcFull;
	rcTextArea.top = rcTextArea.bottom - TEXT_AREA_HEIGHT;
	rcTextArea.left = rcFull.left + (RectWidth(rcFull) - TEXT_AREA_WIDTH) / 2;
	rcTextArea.right = rcTextArea.left + TEXT_AREA_WIDTH;

	//	Paint the background image

	CG16bitImage *pBackground = pAdventure->GetCoverImage();
	if (pBackground)
		{
		RECT rcImage;
		rcImage.left = (RectWidth(rcFull) - pBackground->GetWidth()) / 2;
		rcImage.right = rcImage.left + pBackground->GetWidth();

		if (pBackground->GetHeight() > RectHeight(rcFull))
			{
			rcImage.top = rcFull.top;
			rcImage.bottom = rcFull.bottom;
			}
		else
			{
			rcImage.top = rcFull.bottom - pBackground->GetHeight();
			rcImage.bottom = rcFull.bottom;
			}

		//	Blt the background in the center

		m_Buffer.Blt(0,
				0,
				RectWidth(rcImage),
				RectHeight(rcImage),
				*pBackground,
				rcImage.left,
				rcImage.top);

		//	Paint a black transluscent area over the image

		RECT rcDest;
		if (::IntersectRect(&rcDest, &rcImage, &rcTextArea))
			m_Buffer.FillTrans(rcDest.left, rcDest.top, RectWidth(rcDest), RectHeight(rcDest), 0, 128);
		}

	//	Paint the name of the adventure

	int cyHeight;
	int cxWidth = m_pFonts->SubTitle.MeasureText(pAdventure->GetName(), &cyHeight);
	m_pFonts->SubTitle.DrawText(m_Buffer,
			rcTextArea.left + (RectWidth(rcTextArea) - cxWidth) / 2,
			rcTextArea.top + MARGIN_Y,
			m_pFonts->wTitleColor,
			pAdventure->GetName());

	//	Paint the description

	RECT rcRect = rcTextArea;
	rcRect.top += cyHeight;
	m_pFonts->Medium.DrawText(m_Buffer,
			rcRect,
			m_pFonts->wTextColor,
			pAdventure->GetDesc(),
			0,
			CG16bitFont::AlignCenter);

	//	Paint the filespec

	rcRect.top = rcRect.bottom - m_pFonts->Medium.GetHeight();
	m_pFonts->Medium.DrawText(m_Buffer,
			rcRect,
			m_pFonts->wHelpColor,
			pAdventure->GetFilespec(),
			0,
			CG16bitFont::AlignCenter);
	}

void CAdventureDescDisplay::SelectNext (void)

//	SelectNext
//
//	Select the next ship

	{
	m_iCurrentIndex = (m_iCurrentIndex + 1) % m_AdventureList.GetCount();
	m_bInvalid = true;
	}

void CAdventureDescDisplay::SelectPrev (void)

//	SelectPrev
//
//	Select the previous ship

	{
	m_iCurrentIndex = (m_iCurrentIndex + m_AdventureList.GetCount() - 1) % m_AdventureList.GetCount();
	m_bInvalid = true;
	}

void CAdventureDescDisplay::Update (void)

//	Update
//
//	Updates the display

	{
	}
