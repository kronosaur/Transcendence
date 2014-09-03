//	CGItemListArea.cpp
//
//	CGItemListArea class

#include "PreComp.h"
#include "Transcendence.h"

const int DEFAULT_ROW_HEIGHT =				96;
const int ICON_WIDTH =						96;
const int ICON_HEIGHT =						96;

const int ITEM_TEXT_MARGIN_Y =				4;
const int ITEM_TEXT_MARGIN_X =				4;
const int ITEM_TEXT_MARGIN_BOTTOM =			10;

const int MODIFIER_SPACING_X =				4;

const WORD RGB_DISABLED_TEXT =				CG16bitImage::RGBValue(128,128,128);

const WORD RGB_MODIFIER_NORMAL_BACKGROUND =			CG16bitImage::RGBValue(101,101,101);	//	H:0   S:0   B:40
const WORD RGB_MODIFIER_NORMAL_TEXT =				CG16bitImage::RGBValue(220,220,220);	//	H:0   S:0   B:86

const int DAMAGE_ADJ_ICON_WIDTH =			16;
const int DAMAGE_ADJ_ICON_HEIGHT =			16;
const int DAMAGE_ADJ_ICON_SPACING_X =		2;

const int DAMAGE_ADJ_SPACING_X =			6;

const int ATTRIB_PADDING_X =				4;
const int ATTRIB_PADDING_Y =				1;
const int ATTRIB_SPACING_X =				2;
const int ATTRIB_SPACING_Y =				2;

#define STR_NO_ITEMS						CONSTLIT("There are no items here")

CGItemListArea::CGItemListArea (void) :
		m_pListData(NULL),
		m_iType(listNone),
		m_pUIRes(NULL),
		m_pFonts(NULL),
		m_iOldCursor(-1),
		m_yOffset(0),
		m_yFirst(0),
		m_cyRow(DEFAULT_ROW_HEIGHT)

//	CGItemListArea constructor

	{
	}

CGItemListArea::~CGItemListArea (void)

//	CGItemListArea destructor

	{
	if (m_pListData)
		delete m_pListData;
	}

int CGItemListArea::CalcRowHeight (int iRow)

//	CalcRowHeight
//
//	Returns the height of the given row

	{
	//	Set the position

	int iOldPos = m_pListData->GetCursor();
	m_pListData->SetCursor(iRow);

	//	Compute the rect where we're painting (we only care about width)

	RECT rcRect = GetRect();

	//	Compute row height based on type of list

	int cyHeight;
	switch (m_iType)
		{
		case listItem:
			{
			//	Get the item

			if (!m_pListData->IsCursorValid())
				{
				cyHeight = DEFAULT_ROW_HEIGHT;
				break;
				}

			const CItem &Item = m_pListData->GetItemAtCursor();
			CItemCtx Ctx(&Item, m_pListData->GetSource());
			CItemType *pType = Item.GetType();
			if (pType == NULL)
				{
				cyHeight = DEFAULT_ROW_HEIGHT;
				break;
				}

			//	Compute the rect where the reference text will paint

			RECT rcDrawRect = rcRect;
			rcDrawRect.left += ICON_WIDTH + ITEM_TEXT_MARGIN_X;
			rcDrawRect.right -= ITEM_TEXT_MARGIN_X;

			int iLevel = pType->GetApparentLevel();

			//	Compute the height of the row

			cyHeight = 0;

			//	Account for margin

			cyHeight += ITEM_TEXT_MARGIN_Y;

			//	Item title

			cyHeight += m_pFonts->LargeBold.GetHeight();

			//	Attributes

			TArray<SDisplayAttribute> Attribs;
			if (Item.GetDisplayAttributes(m_pListData->GetSource(), &Attribs))
				{
				int cyAttribs;
				FormatDisplayAttributes(Attribs, rcDrawRect, &cyAttribs);
				cyHeight += cyAttribs + ATTRIB_SPACING_Y;
				}

			//	Reference

			CString sReference = pType->GetReference(Ctx);

			//	If this is a weapon, then add room for the weapon damage

			if (Item.GetReferenceDamageType(m_pListData->GetSource(), -1, 0, NULL, NULL))
				cyHeight += m_pFonts->Medium.GetHeight();

			//	If this is armor or a shield, then add room for damage resistance

			else if (Item.GetReferenceDamageAdj(m_pListData->GetSource(), 0, NULL, NULL))
				cyHeight += m_pFonts->Medium.GetHeight();

			//	Measure the reference text

			int iLines;
			if (!sReference.IsBlank())
				{
				iLines = m_pFonts->Medium.BreakText(sReference, RectWidth(rcDrawRect), NULL, 0);
				cyHeight += iLines * m_pFonts->Medium.GetHeight();
				}

			//	Measure the description

			CString sDesc = Item.GetDesc();
			iLines = m_pFonts->Medium.BreakText(sDesc, RectWidth(rcDrawRect), NULL, 0);
			cyHeight += iLines * m_pFonts->Medium.GetHeight();

			//	Margin

			cyHeight += ITEM_TEXT_MARGIN_BOTTOM;

			//	Done

			cyHeight = Max(DEFAULT_ROW_HEIGHT, cyHeight);
			break;
			}

		case listCustom:
			cyHeight = m_cyRow;
			break;

		default:
			cyHeight = DEFAULT_ROW_HEIGHT;
			break;
		}

	//	Done

	m_pListData->SetCursor(iOldPos);
	return cyHeight;
	}

void CGItemListArea::CleanUp (void)

//	CleanUp
//
//	Free list data

	{
	if (m_pListData)
		{
		delete m_pListData;
		m_pListData = NULL;
		}
	}

int CGItemListArea::FindRow (int y)

//	FindRow
//
//	Returns the row that is at the given absolute coordinates (or -1 if not
//	found)

	{
	int i;

	for (i = 0; i < m_Rows.GetCount(); i++)
		if (y >= m_Rows[i].yPos && y < m_Rows[i].yPos + m_Rows[i].cyHeight)
			return i;

	return -1;
	}

void CGItemListArea::FormatDisplayAttributes (TArray<SDisplayAttribute> &Attribs, const RECT &rcRect, int *retcyHeight)

//	FormatDisplayAttributes
//
//	Initializes the rcRect structure in all attribute entries.

	{
	int i;

	int cxLeft = RectWidth(rcRect);
	int x = rcRect.left;
	int y = rcRect.top;

	for (i = 0; i < Attribs.GetCount(); i++)
		{
		int cxText = (ATTRIB_PADDING_X * 2) + m_pFonts->Medium.MeasureText(Attribs[i].sText);
		int cyText = (ATTRIB_PADDING_Y * 2) + m_pFonts->Medium.GetHeight();
		if (cxText > cxLeft && cxLeft != RectWidth(rcRect))
			{
			y += cyText + ATTRIB_SPACING_Y;
			cxLeft = RectWidth(rcRect);
			}

		Attribs[i].rcRect.left = x;
		Attribs[i].rcRect.top = y;
		Attribs[i].rcRect.right = x + cxText;
		Attribs[i].rcRect.bottom = y + cyText;

		x += cxText + ATTRIB_SPACING_X;
		cxLeft -= cxText + ATTRIB_SPACING_X;
		}

	y += (ATTRIB_PADDING_Y * 2) + m_pFonts->Medium.GetHeight();

	if (retcyHeight)
		*retcyHeight = y - rcRect.top;
	}

ICCItem *CGItemListArea::GetEntryAtCursor (void)

//	GetEntryAtCursor
//
//	Returns the current entry

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	if (m_pListData == NULL)
		return CC.CreateNil();

	return m_pListData->GetEntryAtCursor(CC);
	}

void CGItemListArea::InitRowDesc (void)

//	InitRowDesc
//
//	Initializes the row descriptors

	{
	int i;

	m_Rows.DeleteAll();
	if (m_pListData == NULL || m_pListData->GetCount() == 0)
		{
		m_cyTotalHeight = 0;
		return;
		}

	m_Rows.InsertEmpty(m_pListData->GetCount());
	int y = 0;
	for (i = 0; i < m_pListData->GetCount(); i++)
		{
		//	Compute the height of the row

		int cyRow = CalcRowHeight(i);

		//	Add it

		m_Rows[i].yPos = y;
		m_Rows[i].cyHeight = cyRow;

		y += cyRow;
		}

	m_cyTotalHeight = y;
	}

bool CGItemListArea::LButtonDown (int x, int y)

//	LButtonDown
//
//	Handle button down

	{
	if (m_iOldCursor != -1 && m_pListData->GetCount())
		{
		//	Figure out the cursor position that the user clicked on

		int iPos = FindRow((y - GetRect().top) - m_yFirst);
		if (iPos >= 0 && iPos < m_pListData->GetCount())
			SignalAction(iPos);

		return true;
		}

	return false;
	}

bool CGItemListArea::MoveCursorBack (void)

//	MoveCursorBack
//
//	Move cursor back

	{
	bool bOK = (m_pListData ? m_pListData->MoveCursorBack() : false);
	if (bOK)
		Invalidate();
	return bOK;
	}

bool CGItemListArea::MoveCursorForward (void)

//	MoveCursorForward
//
//	Move cursor forward

	{
	bool bOK = (m_pListData ? m_pListData->MoveCursorForward() : false);
	if (bOK)
		Invalidate();
	return bOK;
	}

void CGItemListArea::Paint (CG16bitImage &Dest, const RECT &rcRect)

//	Paint
//
//	Paint the area

	{
	//	Can't paint if we are not properly initialized

	if (m_pFonts == NULL)
		return;

	//	If there are no items here, then say so

	if (m_pListData == NULL || !m_pListData->IsCursorValid())
		{
		int x = rcRect.left + (RectWidth(rcRect) - m_pFonts->LargeBold.MeasureText(STR_NO_ITEMS)) / 2;
		int y = rcRect.top + (RectHeight(rcRect) - m_pFonts->LargeBold.GetHeight()) / 2;

		Dest.DrawText(x, y,
				m_pFonts->LargeBold,
				RGB_DISABLED_TEXT,
				STR_NO_ITEMS);

		m_iOldCursor = -1;
		}

	//	Otherwise, paint the list of items

	else
		{
		int iCursor = m_pListData->GetCursor();
		int iCount = m_pListData->GetCount();

		ASSERT(iCursor >= 0 && iCursor < m_Rows.GetCount());

		//	If the cursor has changed, update the offset so that we
		//	have a smooth scroll.

		if (m_iOldCursor != -1 
				&& m_iOldCursor != iCursor
				&& m_iOldCursor < m_Rows.GetCount())
			{
			int cyDiff = (m_Rows[iCursor].cyHeight - m_Rows[m_iOldCursor].cyHeight) / 2;

			if (m_iOldCursor < iCursor)
				m_yOffset = m_Rows[m_iOldCursor].cyHeight + cyDiff;
			else
				m_yOffset = -(m_Rows[iCursor].cyHeight - cyDiff);
			}

		m_iOldCursor = iCursor;

		//	Figure out the ideal position of the cursor (relative to the
		//	rect).

		int yIdeal = m_yOffset + ((RectHeight(rcRect) - m_Rows[iCursor].cyHeight) / 2);

		//	Figure out the actual position of the cursor row (relative to the
		//	rect).

		int yCursor;

		//	If the cursor is in the top part of the list
		if (m_Rows[iCursor].yPos < yIdeal)
			yCursor = m_Rows[iCursor].yPos;

		//	If the total number of lines is less than the whole rect
		else if (m_cyTotalHeight < RectHeight(rcRect))
			yCursor = m_Rows[iCursor].yPos;

		//	If the cursor is in the bottom part of the list
		else if ((m_cyTotalHeight - m_Rows[iCursor].yPos) < (RectHeight(rcRect) - yIdeal))
			yCursor = (RectHeight(rcRect) - (m_cyTotalHeight - m_Rows[iCursor].yPos));

		//	The cursor is in the middle of the list
		else
			yCursor = yIdeal;

		//	Figure out the item position at which we start painting

		int iStart = FindRow(m_Rows[iCursor].yPos - yCursor);
		ASSERT(iStart != -1);
		if (iStart == -1)
			iStart = 0;

		int yStart = yCursor - (m_Rows[iCursor].yPos - m_Rows[iStart].yPos);

		//	Compute y offset of first row (so that we can handle clicks later)

		m_yFirst = yStart - m_Rows[iStart].yPos;

		//	Paint

		int y = rcRect.top + yStart;
		int iPos = iStart;
		bool bPaintSeparator = false;
		RECT rcItem;

		while (y < rcRect.bottom && iPos < m_Rows.GetCount())
			{
			//	Paint previous separator

			if (bPaintSeparator)
				{
				Dest.Fill(rcItem.left,
						rcItem.bottom - 1,
						RectWidth(rcItem),
						1,
						CG16bitImage::RGBValue(80,80,80));
				}
			else
				bPaintSeparator = true;

			//	Paint only if we have a valid entry. Sometimes we can
			//	start at an invalid entry because we're scrolling.

			if (iPos >= 0)
				{
				m_pListData->SetCursor(iPos);

				rcItem.top = y;
				rcItem.left = rcRect.left;
				rcItem.bottom = y + m_Rows[iPos].cyHeight;
				rcItem.right = rcRect.right;

				//	See if we need to paint the cursor

				bool bPaintCursor = (iPos == iCursor);

				//	Paint selection background (if selected)

				if (bPaintCursor)
					{
					Dest.FillTrans(rcItem.left,
							rcItem.top,
							RectWidth(rcItem),
							RectHeight(rcItem),
							m_pFonts->wSelectBackground,
							128);

					bPaintSeparator = false;
					}

				//	Paint item

				switch (m_iType)
					{
					case listCustom:
						PaintCustom(Dest, rcItem, bPaintCursor);
						break;

					case listItem:
						PaintItem(Dest, m_pListData->GetItemAtCursor(), rcItem, bPaintCursor);
						break;
					}
				}

			//	Next

			y += m_Rows[iPos].cyHeight;
			iPos++;
			}

		//	Done

		m_pListData->SetCursor(iCursor);
		}
	}

void CGItemListArea::PaintCustom (CG16bitImage &Dest, const RECT &rcRect, bool bSelected)

//	PaintCustom
//
//	Paints a custom element

	{
	//	Paint the image

	m_pListData->PaintImageAtCursor(Dest, rcRect.left, rcRect.top);

	RECT rcDrawRect = rcRect;
	rcDrawRect.left += ICON_WIDTH + ITEM_TEXT_MARGIN_X;
	rcDrawRect.right -= ITEM_TEXT_MARGIN_X;
	rcDrawRect.top += ITEM_TEXT_MARGIN_Y;

	//	Paint the title

	int cyHeight;
	RECT rcTitle = rcDrawRect;
	m_pFonts->LargeBold.DrawText(Dest,
			rcTitle,
			m_pFonts->wItemTitle,
			m_pListData->GetTitleAtCursor(),
			0,
			CG16bitFont::SmartQuotes | CG16bitFont::TruncateLine,
			&cyHeight);

	rcDrawRect.top += cyHeight;

	//	Paint the description

	m_pFonts->Medium.DrawText(Dest, 
			rcDrawRect,
			(bSelected ? m_pFonts->wItemDescSelected : m_pFonts->wItemDesc),
			m_pListData->GetDescAtCursor(),
			0,
			CG16bitFont::SmartQuotes,
			&cyHeight);

	rcDrawRect.top += cyHeight;
	}

void CGItemListArea::PaintDisplayAttributes (CG16bitImage &Dest, TArray<SDisplayAttribute> &Attribs)

//	PaintDisplayAttributes
//
//	Paints all display attributes. We assume that FormatDisplayAttributes
//	has already been called.

	{
	int i;
	const CVisualPalette &VI = g_pHI->GetVisuals();

	for (i = 0; i < Attribs.GetCount(); i++)
		{
		WORD wBackColor;
		WORD wTextColor;

		//	Figure out the colors

		switch (Attribs[i].iType)
			{
			case attribPositive:
				wBackColor = VI.GetColor(colorAreaAdvantage);
				wTextColor = VI.GetColor(colorTextAdvantage);
				break;

			case attribNegative:
				wBackColor = VI.GetColor(colorAreaDisadvantage);
				wTextColor = VI.GetColor(colorTextDisadvantage);
				break;

			default:
				wBackColor = RGB_MODIFIER_NORMAL_BACKGROUND;
				wTextColor = RGB_MODIFIER_NORMAL_TEXT;
				break;
			}

		//	Draw the background

		::DrawRoundedRect(Dest, 
				Attribs[i].rcRect.left, 
				Attribs[i].rcRect.top, 
				RectWidth(Attribs[i].rcRect), 
				RectHeight(Attribs[i].rcRect), 
				4, 
				wBackColor);

		//	Draw the text

		m_pFonts->Medium.DrawText(Dest, 
				Attribs[i].rcRect.left + ATTRIB_PADDING_X, 
				Attribs[i].rcRect.top + ATTRIB_PADDING_Y, 
				wTextColor, 
				255, 
				Attribs[i].sText);
		}
	}

void CGItemListArea::PaintItem (CG16bitImage &Dest, const CItem &Item, const RECT &rcRect, bool bSelected)

//	PaintItem
//
//	Paints the item

	{
	//	Item context

	CItemCtx Ctx(&Item, m_pListData->GetSource());
	CItemType *pItemType = Item.GetType();

	//	Paint the image

	DrawItemTypeIcon(Dest, rcRect.left, rcRect.top, pItemType);

	RECT rcDrawRect = rcRect;
	rcDrawRect.left += ICON_WIDTH + ITEM_TEXT_MARGIN_X;
	rcDrawRect.right -= ITEM_TEXT_MARGIN_X;
	rcDrawRect.top += ITEM_TEXT_MARGIN_Y;

	//	Paint the item name

	int cyHeight;
	RECT rcTitle = rcDrawRect;
	m_pFonts->LargeBold.DrawText(Dest,
			rcTitle,
			m_pFonts->wItemTitle,
			Item.GetNounPhrase(nounCount | nounNoModifiers),
			0,
			CG16bitFont::SmartQuotes | CG16bitFont::TruncateLine,
			&cyHeight);

	rcDrawRect.top += cyHeight;

	//	Paint the display attributes

	TArray<SDisplayAttribute> Attribs;
	if (Item.GetDisplayAttributes(m_pListData->GetSource(), &Attribs))
		{
		FormatDisplayAttributes(Attribs, rcDrawRect, &cyHeight);
		PaintDisplayAttributes(Dest, Attribs);
		rcDrawRect.top += cyHeight + ATTRIB_SPACING_Y;
		}

	//	Stats

	CString sStat;

	int iLevel = pItemType->GetApparentLevel();
	CString sReference = pItemType->GetReference(Ctx);
	DamageTypes iDamageType;
	CString sDamageRef;
	int iDamageAdj[damageCount];
	int iHP;

	if (Item.GetReferenceDamageType(m_pListData->GetSource(), -1, 0, &iDamageType, &sDamageRef))
		{
		//	Paint the damage type reference

		m_pUIRes->DrawReferenceDamageType(Dest,
				rcDrawRect.left + DAMAGE_ADJ_SPACING_X,
				rcDrawRect.top,
				iDamageType,
				sDamageRef);

		rcDrawRect.top += m_pFonts->Medium.GetHeight();

		//	Paint additional reference in the line below

		if (!sReference.IsBlank())
			{
			m_pFonts->Medium.DrawText(Dest, 
					rcDrawRect,
					m_pFonts->wItemRef,
					sReference,
					0,
					0,
					&cyHeight);

			rcDrawRect.top += cyHeight;
			}
		}
	else if (Item.GetReferenceDamageAdj(m_pListData->GetSource(), 0, &iHP, iDamageAdj))
		{
		//	Paint the initial text

		sStat = strPatternSubst("hp: %d ", iHP);
		int cxWidth = m_pFonts->Medium.MeasureText(sStat, &cyHeight);
		m_pFonts->Medium.DrawText(Dest, 
				rcDrawRect,
				m_pFonts->wItemRef,
				sStat,
				0,
				0,
				&cyHeight);

		//	Paint the damage type array

		m_pUIRes->DrawReferenceDamageAdj(Dest,
				rcDrawRect.left + cxWidth + DAMAGE_ADJ_SPACING_X,
				rcDrawRect.top,
				iLevel,
				iHP,
				iDamageAdj);

		rcDrawRect.top += cyHeight;

		//	Paint additional reference in the line below

		if (!sReference.IsBlank())
			{
			m_pFonts->Medium.DrawText(Dest, 
					rcDrawRect,
					m_pFonts->wItemRef,
					sReference,
					0,
					0,
					&cyHeight);

			rcDrawRect.top += cyHeight;
			}
		}
	else
		{
		m_pFonts->Medium.DrawText(Dest, 
				rcDrawRect,
				m_pFonts->wItemRef,
				sReference,
				0,
				0,
				&cyHeight);

		rcDrawRect.top += cyHeight;
		}

	//	Description

	CString sDesc = Item.GetDesc();
	m_pFonts->Medium.DrawText(Dest,
			rcDrawRect,
			(bSelected ? m_pFonts->wItemDescSelected : m_pFonts->wItemDesc),
			sDesc,
			0,
			CG16bitFont::SmartQuotes,
			&cyHeight);
	rcDrawRect.top += cyHeight;
	}

void CGItemListArea::SetList (CSpaceObject *pSource)

//	SetList
//
//	Sets the item list that this control will display

	{
	CleanUp();

	//	Make sure the items in the source are sorted

	pSource->GetItemList().SortItems();

	//	Create a new data source

	m_pListData = new CItemListWrapper(pSource);
	m_iType = listItem;

	//	Done

	InitRowDesc();
	Invalidate();
	}

void CGItemListArea::SetList (CItemList &ItemList)

//	SetList
//
//	Set list of items

	{
	CleanUp();

	//	Create a new data source

	m_pListData = new CItemListWrapper(ItemList);
	m_iType = listItem;

	//	Done

	InitRowDesc();
	Invalidate();
	}

void CGItemListArea::SetList (CCodeChain &CC, ICCItem *pList)

//	SetList
//
//	Sets the list from a CC list

	{
	CleanUp();

	//	Create the new data source

	m_pListData = new CListWrapper(&CC, pList);
	m_iType = listCustom;

	//	Done

	InitRowDesc();
	Invalidate();
	}

void CGItemListArea::Update (void)

//	Update
//
//	Update state

	{
	//	Handle smooth scroll

	if (m_yOffset)
		{
		int iDelta;
		if (m_yOffset > 0)
			{
			iDelta = Max(12, m_yOffset / 4);
			m_yOffset = Max(0, m_yOffset - iDelta);
			}
		else
			{
			iDelta = Min(-12, m_yOffset / 4);
			m_yOffset = Min(0, m_yOffset - iDelta);
			}

		Invalidate();
		}
	}

