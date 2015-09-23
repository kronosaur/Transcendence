//	CGItemListDisplayArea.cpp
//
//	CGItemListDisplayArea class
//	Copyright (c) 2015 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

const int BORDER_RADIUS =						4;
const int PADDING =								4;

const int ITEM_TEXT_MARGIN_Y =					4;
const int ITEM_TEXT_MARGIN_X =					4;
const int ITEM_TITLE_EXTRA_MARGIN =				4;
const int ITEM_TEXT_MARGIN_BOTTOM =				10;

const int ICON_WIDTH =							48;
const int ICON_HEIGHT =							48;
const int BOX_WIDTH =							ICON_WIDTH * 2;
const int BOX_SPACING_X =						4;
const int BOX_SPACING_Y =						2;
const int COUNT_PADDING_X =						2;
const int COUNT_PADDING_Y =						1;

CGItemListDisplayArea::CGItemListDisplayArea (void) :
		m_pSource(NULL),
		m_iCols(0),
		m_iRows(0),
		m_xOffset(0)

//	CGItemListDisplayArea constructor

	{
	}

int CGItemListDisplayArea::Justify (const RECT &rcRect)

//	Justify
//
//	Figure out how big we are.

	{
	const CVisualPalette &VI = g_pHI->GetVisuals();

	RECT rcInner = rcRect;
	rcInner.left += PADDING;
	rcInner.right -= PADDING;
	rcInner.top += PADDING;
	rcInner.bottom -= PADDING;

	//	If we have an item, format it

	if (m_ItemList.GetCount() > 0)
		{
		const CG16bitFont &Medium = VI.GetFont(fontMedium);

		//	Compute the size of each box (where each item will paint).

		m_cxBox = BOX_WIDTH;
		m_cyBox = ICON_HEIGHT + 2 * Medium.GetHeight();

		//	Figure out how many boxes fit in our space

		m_iCols = Max(1, RectWidth(rcInner) / m_cxBox);
		m_iRows = AlignUp(m_ItemList.GetCount(), m_iCols) / m_iCols;
		m_xOffset = (RectWidth(rcInner) - (m_cxBox * m_iCols)) / 2;

		int iLastRowCols = m_ItemList.GetCount() % m_iCols;
		m_xLastRowOffset = (RectWidth(rcInner) - (m_cxBox * iLastRowCols)) / 2;

		//	Return the height

		return (2 * PADDING) + (m_iRows * m_cyBox);
		}

	//	Otherwise, we expect a title and description

	else if (!m_sTitle.IsBlank() || !m_sDesc.IsBlank())
		{
		const CG16bitFont &LargeBold = VI.GetFont(fontLargeBold);
		const CG16bitFont &Medium = VI.GetFont(fontMedium);

		RECT rcDrawRect = rcInner;
		rcDrawRect.left += ITEM_TEXT_MARGIN_X;
		rcDrawRect.right -= ITEM_TEXT_MARGIN_X;

		int cyHeight = ITEM_TEXT_MARGIN_Y + PADDING;
		cyHeight += LargeBold.GetHeight();
		cyHeight += ITEM_TITLE_EXTRA_MARGIN;	//	A little extra spacing

		int iLines = Medium.BreakText(m_sDesc, RectWidth(rcDrawRect), NULL, 0);
		cyHeight += iLines * Medium.GetHeight();

		//	Margin

		cyHeight += ITEM_TEXT_MARGIN_BOTTOM + PADDING;

		return cyHeight;
		}

	//	Otherwise, if we have nothing, then we're collapsed

	else
		return 0;
	}

void CGItemListDisplayArea::Paint (CG32bitImage &Dest, const RECT &rcRect)

//	Paint
//
//	Paint the area

	{
	int i;
	const CVisualPalette &VI = g_pHI->GetVisuals();

	//	Draw background

	CGDraw::RoundedRect(Dest,
			rcRect.left,
			rcRect.top,
			RectWidth(rcRect),
			RectHeight(rcRect),
			BORDER_RADIUS,
			VI.GetColor(colorAreaDialogInputFocus));

	RECT rcInner = rcRect;
	rcInner.left += PADDING;
	rcInner.right -= PADDING;
	rcInner.top += PADDING;
	rcInner.bottom -= PADDING;

	//	Paint the item

	if (m_ItemList.GetCount() > 0)
		{
		const CG16bitFont &Large = VI.GetFont(fontLarge);
		const CG16bitFont &Medium = VI.GetFont(fontMedium);
		CG32bitPixel rgbColorTitle = VI.GetColor(colorTextHighlight);
		CG32bitPixel rgbCountBox = VI.GetColor(colorAreaDeep);
		CG32bitPixel rgbCount = VI.GetColor(colorTextHighlight);
		DWORD dwNounPhraseFlags = nounShort;

		//	Some metrics

		int xIcon = (m_cxBox - ICON_WIDTH) / 2;
		int iCol = 0;
		int iRow = 0;

		//	Paint each item

		for (i = 0; i < m_ItemList.GetCount(); i++)
			{
			const CItem &Item = m_ItemList.GetItem(i);
			CItemType *pItemType = Item.GetType();
			if (pItemType == NULL)
				continue;

			//	Compute position

			int xBox = rcInner.left + (iRow == m_iRows - 1 ? m_xLastRowOffset : m_xOffset) + (iCol * m_cxBox);
			int yBox = rcInner.top + (iRow * m_cyBox);

			//	Paint the icon

			DrawItemTypeIcon(Dest, xBox + xIcon, yBox, pItemType, ICON_WIDTH, ICON_HEIGHT);

			//	Paint the count

			if (Item.GetCount() > 1)
				{
				CString sCount = strPatternSubst(CONSTLIT("×%d"), Item.GetCount());
				int cyCount;
				int cxCount = Large.MeasureText(sCount, &cyCount);
				int cxCountBox = cxCount + 2 * COUNT_PADDING_X;
				int cyCountBox = cyCount + 2 * COUNT_PADDING_Y;
				int xCountBox = xBox + xIcon + ICON_WIDTH - cxCountBox;
				int yCountBox = yBox + ICON_HEIGHT - cyCountBox;

				CGDraw::RoundedRect(Dest, xCountBox, yCountBox, cxCountBox, cyCountBox, 2, rgbCountBox);
				Large.DrawText(Dest, xCountBox + COUNT_PADDING_X, yCountBox + COUNT_PADDING_Y, rgbCount, sCount);
				}

			//	Paint the text underneath

			RECT rcText;
			rcText.left = xBox;
			rcText.top = yBox + ICON_HEIGHT;
			rcText.right = rcText.left + m_cxBox;
			rcText.bottom = rcText.top + Medium.GetHeight() * 2;
			Medium.DrawText(Dest, rcText, rgbColorTitle, Item.GetNounPhrase(dwNounPhraseFlags), 0, CG16bitFont::AlignCenter);
				
			//	Next

			iCol++;
			if (iCol >= m_iCols)
				{
				iCol = 0;
				iRow++;
				}
			}
		}

	//	Paint title and text

	else
		{
		const CG16bitFont &LargeBold = VI.GetFont(fontLargeBold);
		const CG16bitFont &Medium = VI.GetFont(fontMedium);
		CG32bitPixel rgbColorTitle = VI.GetColor(colorTextHighlight);
		CG32bitPixel rgbColorDesc = CG32bitPixel(128,128,128);

		RECT rcDrawRect = rcInner;
		rcDrawRect.left += ITEM_TEXT_MARGIN_X;
		rcDrawRect.right -= ITEM_TEXT_MARGIN_X;
		rcDrawRect.top += ITEM_TEXT_MARGIN_Y;

		int cyHeight = 0;
		LargeBold.DrawText(Dest,
				rcDrawRect,
				rgbColorTitle,
				m_sTitle,
				0,
				CG16bitFont::SmartQuotes | CG16bitFont::TruncateLine,
				&cyHeight);

		rcDrawRect.top += cyHeight + ITEM_TITLE_EXTRA_MARGIN;

		Medium.DrawText(Dest,
				rcDrawRect,
				rgbColorDesc,
				m_sDesc,
				0,
				CG16bitFont::SmartQuotes,
				&cyHeight);
		}
	}
