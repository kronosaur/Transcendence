//	CGItemListDisplayArea.cpp
//
//	CGItemListDisplayArea class
//	Copyright (c) 2015 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define FIELD_ENABLED				CONSTLIT("enabled")
#define FIELD_ITEM					CONSTLIT("item")
#define FIELD_ITEMS					CONSTLIT("items")
#define FIELD_SOURCE				CONSTLIT("source")

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

bool CGItemListDisplayArea::InitFromDesc (CCodeChain &CC, ICCItem *pDesc)

//	InitFromDesc
//
//	Initialize from a descriptor

	{
	if (pDesc->IsNil())
		SetItemList(NULL, CItemList());

	//	If this is a list then we expect a list of item values

	else if (pDesc->IsList())
		{
		if (!InitFromItemList(CC, pDesc))
			return false;
		}
	else
		return false;

	return true;
	}

bool CGItemListDisplayArea::InitFromItemList (CCodeChain &CC, ICCItem *pItemList)

//	InitFromItemList
//
//	Expects a list of items or a list of structs.

	{
	int i;

	m_ItemList.DeleteAll();

	for (i = 0; i < pItemList->GetCount(); i++)
		{
		SEntry *pEntry = m_ItemList.Insert();

		ICCItem *pItemDesc = pItemList->GetElement(i);
		if (pItemDesc->IsSymbolTable())
			{
			ICCItem *pItem = pItemDesc->GetElement(FIELD_ITEM);
			if (pItem)
				pEntry->Item = ::CreateItemFromList(CC, pItem);

			ICCItem *pEnabled = pItemDesc->GetElement(FIELD_ENABLED);
			pEntry->bGrayed = (pEnabled && pEnabled->IsNil());
			}
		else
			pEntry->Item = ::CreateItemFromList(CC, pItemDesc);

		//	Make sure this is a valid item

		if (pEntry->Item.GetType() == NULL)
			{
			m_ItemList.Delete(m_ItemList.GetCount() - 1);
			continue;
			}
		}

	SortItemList();

	return true;
	}

int CGItemListDisplayArea::Justify (const RECT &rcRect)

//	Justify
//
//	Figure out how big we are.

	{
	int i;
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

		//	See how many lines we need for the items

		int iMaxLines = 0;
		for (i = 0; i < m_ItemList.GetCount(); i++)
			{
			const CItem &Item = m_ItemList[i].Item;
			int iLinesNeeded = Medium.BreakText(Item.GetNounPhrase(CItemCtx(), nounShort), m_cxBox);
			if (iLinesNeeded > iMaxLines)
				iMaxLines = iLinesNeeded;
			}

		m_cyBox = ICON_HEIGHT + Max(1, iMaxLines) * Medium.GetHeight();

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
	DEBUG_TRY

	int i;
	const CVisualPalette &VI = g_pHI->GetVisuals();

	//	Draw background

	CGDraw::RoundedRect(Dest,
			rcRect.left,
			rcRect.top,
			RectWidth(rcRect),
			RectHeight(rcRect),
			BORDER_RADIUS,
			m_rgbBackColor);

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
		CG32bitPixel rgbColorTitle = m_rgbTextColor;
		CG32bitPixel rgbCountBox = CG32bitPixel::Darken(m_rgbBackColor, 0x80);
		CG32bitPixel rgbCount = m_rgbTextColor;
		DWORD dwNounPhraseFlags = nounShort;

		//	Some metrics

		int xIcon = (m_cxBox - ICON_WIDTH) / 2;
		int iCol = 0;
		int iRow = 0;

		//	Paint each item

		for (i = 0; i < m_ItemList.GetCount(); i++)
			{
			const CItem &Item = m_ItemList[i].Item;
			CItemType *pItemType = Item.GetType();
			if (pItemType == NULL)
				continue;

			//	Compute position

			int xBox = rcInner.left + (iRow == m_iRows - 1 ? m_xLastRowOffset : m_xOffset) + (iCol * m_cxBox);
			int yBox = rcInner.top + (iRow * m_cyBox);

			//	Paint the icon

			DrawItemTypeIcon(Dest, xBox + xIcon, yBox, pItemType, ICON_WIDTH, ICON_HEIGHT, m_ItemList[i].bGrayed);

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
			Medium.DrawText(Dest, rcText, rgbColorTitle, Item.GetNounPhrase(CItemCtx(), dwNounPhraseFlags), 0, CG16bitFont::AlignCenter);
				
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
		CG32bitPixel rgbColorTitle = m_rgbTextColor;
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

	DEBUG_CATCH
	}

void CGItemListDisplayArea::SetItemList (CSpaceObject *pSource, const CItemList &ItemList) 

//	SetItemList
//
//	Sets from a normal item list.
	
	{
	int i;

	m_pSource = pSource;
	
	m_ItemList.DeleteAll();
	m_ItemList.InsertEmpty(ItemList.GetCount());
	for (i = 0; i < ItemList.GetCount(); i++)
		m_ItemList[i].Item = ItemList.GetItem(i);

	SortItemList();
	
	Invalidate();
	}

void CGItemListDisplayArea::SortItemList (void)

//	SortItemList
//
//	Sorts the item list

	{
	int i;

	TSortMap<CString, int> Sort;
	for (i = 0; i < m_ItemList.GetCount(); i++)
		Sort.Insert(m_ItemList[i].Item.CalcSortKey(), i);

	TArray<SEntry> NewList;
	NewList.InsertEmpty(m_ItemList.GetCount());
	for (i = 0; i < m_ItemList.GetCount(); i++)
		NewList[i] = m_ItemList[Sort[i]];

	m_ItemList.TakeHandoff(NewList);
	}
