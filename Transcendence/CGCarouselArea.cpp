//	CGCarouselArea.cpp
//
//	CGCarouselArea class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define FIELD_DESC							CONSTLIT("desc")
#define FIELD_DETAILS						CONSTLIT("details")
#define FIELD_ICON							CONSTLIT("icon")
#define FIELD_LARGE_ICON					CONSTLIT("largeIcon")
#define FIELD_TITLE							CONSTLIT("title")

#define STR_NO_ITEMS						CONSTLIT("There are no items here")
#define STR_NO_SHIPS						CONSTLIT("There are no ships here")

static TStaticStringTable<TStaticStringEntry<CGCarouselArea::EStyles>, 2> STYLE_TABLE = {
	"shipCompare",			CGCarouselArea::styleShipCompare,
	"shipList",				CGCarouselArea::styleShipList,
	};

CGCarouselArea::CGCarouselArea (const CVisualPalette &VI) :
		m_VI(VI)

//	CGCarouselArea constructor

	{
	}

void CGCarouselArea::CleanUp (void)

//	CleanUp
//
//	Free list data

	{
	m_pListData.Delete();
	}

int CGCarouselArea::FindSelector (int x) const

//	FindRow
//
//	Returns the selector that is at the given absolute coordinates (or -1 if not
//	found)

	{
	if (!m_pListData || x < 0 || m_cxSelector <= 0)
		return -1;

	switch (m_iStyle)
		{
		case styleShipCompare:
			{
			//	In this case, the middle entry is a static arrow.

			int iIndex = (x / m_cxSelector);
			if (iIndex == 0)
				return 0;
			else if (iIndex == 2)
				return 1;
			else
				return -1;
			}

		//	Default is a plain list of entries

		default:
			{
			if (x < 0 || x >= (m_cxSelector * m_pListData->GetCount()))
				return -1;

			return (x / m_cxSelector);
			}
		}
	}

ICCItem *CGCarouselArea::GetEntryAtCursor (void)

//	GetEntryAtCursor
//
//	Returns the current entry

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	if (!m_pListData)
		return CC.CreateNil();

	return m_pListData->GetEntryAtCursor(CC);
	}

bool CGCarouselArea::LButtonDown (int x, int y)

//	LButtonDown
//
//	Handle button down

	{
	//	Selector

	if (m_iOldCursor != -1 
			&& m_pListData->GetCount()
			&& y >= GetRect().top
			&& y < GetRect().top + m_cySelector)
		{
		//	Figure out the cursor position that the user clicked on

		int iPos = FindSelector((x - GetRect().left) - m_xFirst);
		if (iPos >= 0 && iPos < m_pListData->GetCount())
			SignalAction(iPos);

		return true;
		}

	return false;
	}

void CGCarouselArea::MouseLeave (void)

//	MouseLeave
//
//	Handle hover

	{
	}

void CGCarouselArea::MouseMove (int x, int y)

//	MouseMove
//
//	Handle hover

	{
	}

void CGCarouselArea::MouseWheel (int iDelta, int x, int y, DWORD dwFlags)

//	MouseWheel
//
//	Handles scrolling

	{
	//	Short-circuit

	if (!m_pListData)
		return;

	//	Figure out how many lines to move

	int iChange = -Sign(iDelta / MOUSE_SCROLL_SENSITIVITY);
	int iNewPos = m_pListData->GetCursor() + iChange;

	if (iNewPos != m_pListData->GetCursor())
		{
		if (iNewPos >= 0 && iNewPos < m_pListData->GetCount())
			SignalAction(iNewPos);
		}
	}

bool CGCarouselArea::MoveCursorBack (void)

//	MoveCursorBack
//
//	Move cursor back

	{
	bool bOK = (m_pListData ? m_pListData->MoveCursorBack() : false);
	if (bOK)
		Invalidate();
	return bOK;
	}

bool CGCarouselArea::MoveCursorForward (void)

//	MoveCursorForward
//
//	Move cursor forward

	{
	bool bOK = (m_pListData ? m_pListData->MoveCursorForward() : false);
	if (bOK)
		Invalidate();
	return bOK;
	}

void CGCarouselArea::Paint (CG32bitImage &Dest, const RECT &rcRect)

//	Paint
//
//	Paint the area

	{
	DEBUG_TRY

	const CG16bitFont &LargeBold = m_VI.GetFont(fontLargeBold);

	//	Figure out where the horizonal selector list will paint

	RECT rcList = rcRect;
	rcList.bottom = rcList.top + m_cySelector;

	//	If there are no items here, then say so

	if (!m_pListData || !m_pListData->IsCursorValid())
		{
		//	Pick text based on style

		CString sText;
		switch (m_iStyle)
			{
			case styleShipCompare:
			case styleShipList:
				sText = STR_NO_SHIPS;
				break;

			default:
				sText = STR_NO_ITEMS;
				break;
			}

		//	Paint

		int x = rcList.left + (RectWidth(rcList) - LargeBold.MeasureText(sText)) / 2;
		int y = rcList.top + (RectHeight(rcList) - LargeBold.GetHeight()) / 2;

		Dest.DrawText(x, y,
				LargeBold,
				m_rgbDisabledText,
				sText);

		m_iOldCursor = -1;
		}

	//	Otherwise, paint the list of entries

	else
		{
		switch (m_iStyle)
			{
			case styleShipCompare:
				PaintListShipCompare(Dest, rcList);
				break;

			default:
				PaintList(Dest, rcList);
				break;
			}
		}

	//	Paint selected content

	RECT rcContent = rcRect;
	rcContent.top += m_cySelectorArea;

	PaintContent(Dest, rcContent);

	DEBUG_CATCH
	}

void CGCarouselArea::PaintContent (CG32bitImage &Dest, const RECT &rcRect) const

//	PaintContent
//
//	Paints the content area for the current selection.

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	//	Colors and metrics

    CG32bitPixel rgbFadeBackColor = CG32bitPixel(CG32bitPixel::Darken(m_rgbBackColor, 220), 185);

	//	Now paint the selected content

	if (m_pListData && m_pListData->IsCursorValid())
		{
		ICCItem *pEntry = m_pListData->GetEntryAtCursor(CC);

		//	Figure out some metrics about details.

		CDetailList Details(m_VI);
		Details.SetColor(m_rgbTextColor);

		ICCItem *pDetails = (pEntry ? pEntry->GetElement(FIELD_DETAILS) : NULL);
		if (pDetails)
			Details.Load(pDetails);

		int cyDetails;
		Details.Format(rcRect, &cyDetails);
		int cyPaneSplit = (cyDetails ? RectHeight(rcRect) - (cyDetails + SPACING_Y) : 0);

		//	Paint the large icon first (we paint the background on top so that 
		//	the details show up.

		ICCItem *pLargeIcon = (pEntry ? pEntry->GetElement(FIELD_LARGE_ICON) : NULL);
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
	}

void CGCarouselArea::PaintList (CG32bitImage &Dest, const RECT &rcRect)

//	PaintList
//
//	Paints the list

	{
	bool bPaintCursor = false;
	int iCursor = m_pListData->GetCursor();
	int iCount = m_pListData->GetCount();

	ASSERT(iCursor >= 0 && iCursor < m_pListData->GetCount());

	//	Clip to the list rect

	RECT rcOldClip = Dest.GetClipRect();
	Dest.SetClipRect(rcRect);

	//	If the cursor has changed, update the offset so that we
	//	have a smooth scroll.

	if (m_iOldCursor != -1 
			&& m_iOldCursor != iCursor
			&& m_iOldCursor < m_pListData->GetCount())
		{
		if (m_iOldCursor < iCursor)
			m_xOffset = m_cxSelector;
		else
			m_xOffset = -m_cxSelector;
		}

	m_iOldCursor = iCursor;

	//	Figure out the ideal position of the cursor (relative to the
	//	rect).

	int xIdeal = m_xOffset + ((RectWidth(rcRect) - m_cxSelector) / 2);

	//	Figure out the actual position of the cursor cell (relative to the
	//	rect).

	int xCursor;
	int xCursorAbs = iCursor * m_cxSelector;
	int xTotalWidth = m_pListData->GetCount() * m_cxSelector;

	//	If the cursor is in the top part of the list
	if (xCursorAbs < xIdeal)
		xCursor = xCursorAbs;

	//	If the total number of lines is less than the whole rect
	else if (xTotalWidth < RectWidth(rcRect))
		xCursor = xCursorAbs;

	//	If the cursor is in the bottom part of the list
	else if ((xTotalWidth - xCursorAbs) < (RectWidth(rcRect) - xIdeal))
		xCursor = (RectWidth(rcRect) - (xTotalWidth - xCursorAbs));

	//	The cursor is in the middle of the list
	else
		xCursor = xIdeal;

	//	Figure out the item position at which we start painting

	int iStart = FindSelector(xCursorAbs - xCursor);
	ASSERT(iStart != -1);
	if (iStart == -1)
		iStart = 0;

	int xStartAbs = iStart * m_cxSelector;
	int xStart = xCursor - (xCursorAbs - xStartAbs);

	//	If all cells fit, then we center them.

	if (xTotalWidth < RectWidth(rcRect))
		xStart = (RectWidth(rcRect) - xTotalWidth) / 2;

	//	Compute x offset of first cell (so that we can handle clicks later)

	m_xFirst = xStart - xStartAbs;

	//	Paint

	int x = rcRect.left + xStart;
	int iPos = iStart;
	bool bPaintSeparator = false;
	RECT rcItem;
	RECT rcCursor;

	while (x < rcRect.right && iPos < m_pListData->GetCount())
		{
		//	Paint only if we have a valid entry. Sometimes we can
		//	start at an invalid entry because we're scrolling.

		if (iPos >= 0)
			{
			m_pListData->SetCursor(iPos);

			rcItem.top = rcRect.top;
			rcItem.left = x;
			rcItem.bottom = rcRect.top + m_cySelector;
			rcItem.right = x + m_cxSelector;

			//	See if we need to paint the cursor

			bool bIsCursor = (iPos == iCursor);

			//	Paint selection background (if selected)

			if (bIsCursor)
				{
				bPaintCursor = true;
				bPaintSeparator = false;
				rcCursor = rcItem;

				CGDraw::RoundedRect(Dest, 
						rcCursor.left,
						rcCursor.top,
						RectWidth(rcCursor),
						RectHeight(rcCursor),
						BORDER_RADIUS,
						m_rgbBackColor);
				}

			//	Paint item

			PaintSelector(Dest, rcItem, bIsCursor);
			}

		//	Next

		x += m_cxSelector;
		iPos++;
		}

	//	Done

	m_pListData->SetCursor(iCursor);
	Dest.SetClipRect(rcOldClip);

	//	Now paint the cursor

	if (bPaintCursor)
		{
		CGDraw::RoundedRectOutline(Dest,
				rcCursor.left + 1,
				rcCursor.top + 1,
				RectWidth(rcCursor) - 2,
				RectHeight(rcCursor) - 2,
				BORDER_RADIUS,
				SELECTION_WIDTH,
				m_VI.GetColor(colorAreaDialogHighlight));
		}
	}

void CGCarouselArea::PaintListShipCompare (CG32bitImage &Dest, const RECT &rcRect)

//	PaintListShipCompare
//
//	Paints the list in ship compare style.

	{
	//	This only works for two entries

	ASSERT(m_pListData);
	if (m_pListData->GetCount() < 2)
		return;

	bool bPaintCursor = false;
	int iCursor = m_pListData->GetCursor();

	//	Clip to the list rect

	RECT rcOldClip = Dest.GetClipRect();
	Dest.SetClipRect(rcRect);
	m_iOldCursor = iCursor;

	//	We center the list on the area

	int xOffset = (RectWidth(rcRect) - (3 * m_cxSelector)) / 2;
	m_xFirst = xOffset;

	//	Paint

	int x = rcRect.left + xOffset;
	int iPos = 0;
	bool bPaintSeparator = false;
	RECT rcItem;
	RECT rcCursor;

	bool bPaintArrow = true;
	while (x < rcRect.right && iPos < m_pListData->GetCount())
		{
		//	Paint only if we have a valid entry. Sometimes we can
		//	start at an invalid entry because we're scrolling.

		if (iPos >= 0)
			{
			m_pListData->SetCursor(iPos);

			rcItem.top = rcRect.top;
			rcItem.left = x;
			rcItem.bottom = rcRect.top + m_cySelector;
			rcItem.right = x + m_cxSelector;

			//	See if we need to paint the cursor

			bool bIsCursor = (iPos == iCursor);

			//	Paint selection background (if selected)

			if (bIsCursor)
				{
				bPaintCursor = true;
				bPaintSeparator = false;
				rcCursor = rcItem;

				CGDraw::RoundedRect(Dest, 
						rcCursor.left,
						rcCursor.top,
						RectWidth(rcCursor),
						RectHeight(rcCursor),
						BORDER_RADIUS,
						m_rgbBackColor);
				}

			//	Paint item

			PaintSelector(Dest, rcItem, bIsCursor);
			}

		//	Next

		x += m_cxSelector;
		iPos++;

		//	Paint the arrow

		if (bPaintArrow)
			{
			const CG32bitImage &Icon = m_VI.GetImage(imageRightIcon);
			Dest.Blt(0,
					0,
					Icon.GetWidth(),
					Icon.GetHeight(),
					Icon,
					x + (m_cxSelector - Icon.GetWidth()) / 2,
					rcRect.top + (m_cySelector - Icon.GetHeight()) / 2);

			//	Next

			x += m_cxSelector;
			bPaintArrow = false;
			}
		}

	//	Done

	m_pListData->SetCursor(iCursor);
	Dest.SetClipRect(rcOldClip);

	//	Now paint the cursor

	if (bPaintCursor)
		{
		CGDraw::RoundedRectOutline(Dest,
				rcCursor.left + 1,
				rcCursor.top + 1,
				RectWidth(rcCursor) - 2,
				RectHeight(rcCursor) - 2,
				BORDER_RADIUS,
				SELECTION_WIDTH,
				m_VI.GetColor(colorAreaDialogHighlight));
		}
	}

void CGCarouselArea::PaintSelector (CG32bitImage &Dest, const RECT &rcRect, bool bSelected) const

//	PaintSelector
//
//	Paints the selector element

	{
	const CG16bitFont &Medium = m_VI.GetFont(fontMedium);

	//	Paint the icon

	m_pListData->PaintImageAtCursor(Dest, rcRect.left + (RectWidth(rcRect) - m_cxIcon) / 2, rcRect.top, m_cxIcon, m_cyIcon, m_rIconScale);

	//	Paint the text below that

	RECT rcText;
	rcText.left = rcRect.left + SELECTOR_PADDING_LEFT;
	rcText.right = rcRect.right - SELECTOR_PADDING_RIGHT;
	rcText.top = rcRect.top + m_cyIcon;
	rcText.bottom = rcRect.bottom;

	Medium.DrawText(Dest, 
			rcText,
			m_rgbTextColor,
			m_pListData->GetTitleAtCursor(),
			0,
			CG16bitFont::AlignCenter | CG16bitFont::TruncateBlock);
	}

void CGCarouselArea::SetList (CCodeChain &CC, ICCItem *pList)

//	SetList
//
//	Sets the list from a CC list

	{
	CleanUp();

	//	Create the new data source

	m_pListData.Set(new CListWrapper(&CC, pList));

	//	Done

	Invalidate();
	}

bool CGCarouselArea::SetStyle (const CString &sStyle)

//	SetStyle
//
//	Sets the style. Returns FALSE if the given style is invalid.

	{
	const TStaticStringEntry<CGCarouselArea::EStyles> *pEntry = STYLE_TABLE.GetAt(sStyle);
	if (pEntry == NULL)
		return false;

	m_iStyle = pEntry->Value;
	return true;
	}

void CGCarouselArea::Update (void)

//	Update
//
//	Update state

	{
	//	Handle smooth scroll

	if (m_xOffset)
		{
		m_xOffset = CUIHelper::ScrollAnimationDecay(m_xOffset);
		Invalidate();
		}
	}
