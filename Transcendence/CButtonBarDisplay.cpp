//	CButtonBarDisplay.cpp
//
//	CButtonBarDisplay class

#include "PreComp.h"
#include "Transcendence.h"

const CG32bitPixel BAR_COLOR =				CG32bitPixel(0, 2, 10);
const int BUTTON_WIDTH =					128;
const int BUTTON_HEIGHT =					80;

const int BUTTON_LABEL_Y =					64;
const int BUTTON_DESCRIPTION_Y =			90;

const int BUTTON_SPACING_X =				30;
const int EXTRA_SPACING_X =					10;

CButtonBarDisplay::CButtonBarDisplay (void)

//	CButtonBarDisplay constructor

	{
	}

CButtonBarDisplay::~CButtonBarDisplay (void)

//	CButtonBarDisplay destructor

	{
	}

void CButtonBarDisplay::CleanUp (void)

//	CleanUp
//
//	Clean up display object

	{
	}

void CButtonBarDisplay::ComputeButtonRects (void)

//	ComputeButtonRects
//
//	Compute the rects of all buttons (based on our total size)

	{
	int iCount = m_pButtons->GetCount();

	//	Same as CVisualPalette::GetWidescreenRect

	int cxCenter = Min(RectWidth(m_rcRect) - (2 * 10), 1280);
	RECT rcCenter = m_rcRect;
	rcCenter.left = m_rcRect.left + (RectWidth(m_rcRect) - cxCenter) / 2;
	rcCenter.right = rcCenter.left + cxCenter;

	//	Align centered buttons

	int cxTotalWidth = ComputeButtonWidth(CButtonBarData::alignCenter);
	int x = rcCenter.left + (RectWidth(rcCenter) - cxTotalWidth) / 2;
	int y = rcCenter.top;
	ComputeButtonRects(CButtonBarData::alignCenter, x, y);

	//	Align left buttons
	//	(We need extra space because the labels extend beyond the size of the
	//	buttons).

	x = rcCenter.left + EXTRA_SPACING_X;
	y = rcCenter.top;
	ComputeButtonRects(CButtonBarData::alignLeft, x, y);

	//	Align right buttons

	cxTotalWidth = ComputeButtonWidth(CButtonBarData::alignRight);
	x = rcCenter.right - (cxTotalWidth + EXTRA_SPACING_X);
	y = rcCenter.top;
	ComputeButtonRects(CButtonBarData::alignRight, x, y);
	}

void CButtonBarDisplay::ComputeButtonRects (CButtonBarData::AlignmentTypes iAlignment, int x, int y)

//	ComputeButtonRects
//
//	Places buttons of the given alignment at the given position

	{
	int i;
	int iCount = m_pButtons->GetCount();

	for (i = 0; i < iCount; i++)
		if (m_pButtons->GetAlignment(i) == iAlignment)
			{
			int cxWidth, cyHeight;
			ComputeButtonSize(i, &cxWidth, &cyHeight);

			RECT rcRect;
			rcRect.left = x;
			rcRect.top = y;
			rcRect.right = x + cxWidth;
			rcRect.bottom = rcRect.top + cyHeight;
			m_pButtons->SetButtonRect(i, rcRect);

			x += cxWidth + BUTTON_SPACING_X;
			}
	}

void CButtonBarDisplay::ComputeButtonSize (int iIndex, int *retcxWidth, int *retcyHeight)

//	ComputeButtonSize
//
//	Compute the size of the given button

	{
	switch (m_pButtons->GetStyle(iIndex))
		{
		case CButtonBarData::styleMedium:
			*retcxWidth = 64;
			*retcyHeight = 103;
			break;

		default:
			*retcxWidth = 128;
			*retcyHeight = 103;
		}
	}

int CButtonBarDisplay::ComputeButtonWidth (CButtonBarData::AlignmentTypes iAlignment)

//	ComputeButtonWidth
//
//	Computes the total button widths for buttons of the given
//	alignment

	{
	int i;
	int iCount = m_pButtons->GetCount();

	//	Compute the total width for all center-aligned buttons

	int cxTotalWidth = 0;
	int iSectCount = 0;
	for (i = 0; i < iCount; i++)
		if (m_pButtons->GetAlignment(i) == iAlignment)
			{
			int cxWidth, cyHeight;
			ComputeButtonSize(i, &cxWidth, &cyHeight);

			iSectCount++;
			cxTotalWidth += cxWidth;
			}

	cxTotalWidth += (iSectCount - 1) * BUTTON_SPACING_X;

	return cxTotalWidth;
	}

int CButtonBarDisplay::FindButtonAtPoint (const POINT &pt)

//	FindButtonAtPoint
//
//	Returns the button at the given point

	{
	int i;

	for (i = 0; i < m_pButtons->GetCount(); i++)
		{
		RECT rcRect = m_pButtons->GetButtonRect(i);
		if (::PtInRect(&rcRect, pt))
			return i;
		}

	return -1;
	}

void CButtonBarDisplay::GetImageRect (int iIndex, bool bSelected, RECT *retrcRect)

//	GetImageRect
//
//	Returns the RECT for the given image

	{
	int iImageIndex = m_pButtons->GetImageIndex(iIndex);

	switch (m_pButtons->GetStyle(iIndex))
		{
		case CButtonBarData::styleMedium:
			m_pButtons->GetImageSize(iIndex, retrcRect);
			::OffsetRect(retrcRect, 
					(iImageIndex % 2) * 64 + (bSelected ? 128 : 0), 
					(iImageIndex / 2) * 64);
			break;

		default:
			m_pButtons->GetImageSize(iIndex, retrcRect);
			::OffsetRect(retrcRect, 
					(bSelected ? 128 : 0), 
					iImageIndex * 64);
		}
	}

ALERROR CButtonBarDisplay::Init (CTranscendenceWnd *pTrans, 
								 CButtonBarData *pData, 
								 const RECT &rcRect)

//	Init
//
//	Initialize

	{
	m_pTrans = pTrans;
	m_pButtons = pData;
	m_rcRect = rcRect;

	//	Compute metrics

	ComputeButtonRects();

	//	Initialize

	POINT pt;
	pTrans->GetMousePos(&pt);
	m_iSelected = FindButtonAtPoint(pt);

	return NOERROR;
	}

bool CButtonBarDisplay::OnChar (char chChar)

//	OnChar
//
//	Handle character

	{
	return false;
	}

bool CButtonBarDisplay::OnKeyDown (int iVirtKey)

//	OnKeyDown
//
//	Handle key down

	{
	return false;
	}

bool CButtonBarDisplay::OnLButtonDoubleClick (int x, int y)

//	OnLButtonDoubleClick
//
//	Handle double-click

	{
	return OnLButtonDown(x, y);
	}

bool CButtonBarDisplay::OnLButtonDown (int x, int y)

//	OnLButtonDown
//
//	Handle mouse

	{
	if (m_iSelected != -1 && m_pButtons->GetVisible(m_iSelected))
		{
		DWORD dwCmd = m_pButtons->GetCmdID(m_iSelected);
		m_pTrans->DoCommand(dwCmd);
		return true;
		}

	return false;
	}

void CButtonBarDisplay::OnMouseMove (int x, int y)

//	OnMouseMove
//
//	Handle moving the mouse

	{
	POINT pt;
	pt.x = x;
	pt.y = y;

	//	Loop over all buttons to see if we are over any of them

	m_iSelected = FindButtonAtPoint(pt);
	}

void CButtonBarDisplay::Paint (CG32bitImage &Dest)

//	Paint
//
//	Paint the bar

	{
	int i;

	//	Fill background

	Dest.Fill(m_rcRect.left,
			m_rcRect.top,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			BAR_COLOR);

	//	Get the images

	const CG32bitImage &Images = m_pButtons->GetImage();

	//	Paint each button

	for (i = 0; i < m_pButtons->GetCount(); i++)
		{
		RECT rcRect = m_pButtons->GetButtonRect(i);

		//	Skip invisible buttons

		if (!m_pButtons->GetVisible(i))
			continue;

		//	Paint the image

		RECT rcSrc;
		GetImageRect(i, (i == m_iSelected), &rcSrc);

		Dest.Blt(rcSrc.left,
				rcSrc.top,
				RectWidth(rcSrc),
				RectHeight(rcSrc),
				Images,
				rcRect.left,
				rcRect.top);

		//	Paint the button label

		int cxWidth = m_pFonts->SubTitle.MeasureText(m_pButtons->GetLabel(i), NULL);
		m_pFonts->SubTitle.DrawText(Dest,
				rcRect.left + (RectWidth(rcRect) - cxWidth) / 2,
				rcRect.top + BUTTON_LABEL_Y,
				CG32bitPixel(128,128,128),
				m_pButtons->GetLabel(i));

		//	Paint the description

		cxWidth = m_pFonts->Medium.MeasureText(m_pButtons->GetDescription(i), NULL);
		m_pFonts->Medium.DrawText(Dest,
				rcRect.left + (RectWidth(rcRect) - cxWidth) / 2,
				rcRect.top + BUTTON_DESCRIPTION_Y,
				CG32bitPixel(255,255,255),
				m_pButtons->GetDescription(i));
		}
	}

void CButtonBarDisplay::Update (void)

//	Update
//
//	Update the bar

	{
	}
