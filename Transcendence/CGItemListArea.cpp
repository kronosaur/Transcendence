//	CGItemListArea.cpp
//
//	CGItemListArea class

#include "PreComp.h"
#include "Transcendence.h"

const int ITEM_TEXT_MARGIN_Y =				4;
const int ITEM_TEXT_MARGIN_X =				4;
const int ITEM_TEXT_MARGIN_BOTTOM =			10;
const int BORDER_RADIUS =					4;
const int SELECTION_WIDTH =					2;

const int MODIFIER_SPACING_X =				4;

const CG32bitPixel RGB_DISABLED_TEXT =					CG32bitPixel(128,128,128);

const CG32bitPixel RGB_MODIFIER_NORMAL_BACKGROUND =		CG32bitPixel(101,101,101);	//	H:0   S:0   B:40
const CG32bitPixel RGB_MODIFIER_NORMAL_TEXT =			CG32bitPixel(220,220,220);	//	H:0   S:0   B:86

const int DAMAGE_ADJ_ICON_WIDTH =			16;
const int DAMAGE_ADJ_ICON_HEIGHT =			16;
const int DAMAGE_ADJ_ICON_SPACING_X =		2;

const int DAMAGE_ADJ_SPACING_X =			6;

const int ATTRIB_PADDING_X =				4;
const int ATTRIB_PADDING_Y =				1;
const int ATTRIB_SPACING_X =				2;
const int ATTRIB_SPACING_Y =				2;

const int MOUSE_SCROLL_SENSITIVITY =		30;

const int TAB_PADDING_X =					16;
const int TAB_HEIGHT =						24;

#define STR_NO_ITEMS						CONSTLIT("There are no items here")

CGItemListArea::CGItemListArea (const CVisualPalette &VI) :
		m_VI(VI)

//	CGItemListArea constructor

	{
	}

CGItemListArea::~CGItemListArea (void)

//	CGItemListArea destructor

	{
	if (m_pListData)
		delete m_pListData;
	}

void CGItemListArea::AddTab (DWORD dwID, const CString &sLabel)

//	AddTab
//
//	Adds a new tab

	{
	const CG16bitFont &MediumFont = m_VI.GetFont(fontMedium);

	STabDesc *pNewTab = m_Tabs.Insert();
	pNewTab->dwID = dwID;
	pNewTab->sLabel = sLabel;
	pNewTab->cxWidth = MediumFont.MeasureText(sLabel) + 2 * TAB_PADDING_X;

	if (m_iCurTab == -1)
		m_iCurTab = 0;

	m_cyTabHeight = TAB_HEIGHT;
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
				}
			else
				{
				CUIHelper UIHelper(*g_pHI);

				DWORD dwOptions = 0;
				if (m_bNoArmorSpeedDisplay)
					dwOptions |= CUIHelper::OPTION_NO_ARMOR_SPEED_DISPLAY;
				if (m_bActualItems)
					dwOptions |= CUIHelper::OPTION_KNOWN;

				cyHeight = UIHelper.CalcItemEntryHeight(m_pListData->GetSource(), m_pListData->GetItemAtCursor(), rcRect, dwOptions);
				}

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

void CGItemListArea::EnableTab (DWORD dwID, bool bEnabled)

//	EnableTab
//
//	Enable/disable a tab

	{
	int iTab;
	if (FindTab(dwID, &iTab))
		{
		m_Tabs[iTab].bDisabled = !bEnabled;
		Invalidate();
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

bool CGItemListArea::FindTab (DWORD dwID, int *retiIndex) const

//	FindTab
//
//	Finds the given tab

	{
	int i;

	for (i = 0; i < m_Tabs.GetCount(); i++)
		if (m_Tabs[i].dwID == dwID)
			{
			if (retiIndex)
				*retiIndex = i;

			return true;
			}

	return false;
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

bool CGItemListArea::GetNextTab (DWORD *retdwID) const

//	GetNextTab
//
//	Returns the next tab

	{
	if (m_iCurTab == -1 || m_Tabs.GetCount() < 2)
		return false;

	int iTab = (m_iCurTab + 1) % m_Tabs.GetCount();
	while (m_Tabs[iTab].bDisabled && iTab != m_iCurTab)
		iTab = (iTab + 1) % m_Tabs.GetCount();

	if (iTab == m_iCurTab)
		return false;
	
	if (retdwID)
		*retdwID = m_Tabs[iTab].dwID;

	return true;
	}

bool CGItemListArea::GetPrevTab (DWORD *retdwID) const

//	GetPrevTab
//
//	Returns the previous tab

	{
	if (m_iCurTab == -1 || m_Tabs.GetCount() < 2)
		return false;

	int iTab = (m_iCurTab + m_Tabs.GetCount() - 1) % m_Tabs.GetCount();
	while (m_Tabs[iTab].bDisabled && iTab != m_iCurTab)
		iTab = (iTab + m_Tabs.GetCount() - 1) % m_Tabs.GetCount();

	if (iTab == m_iCurTab)
		return false;

	if (retdwID)
		*retdwID = m_Tabs[iTab].dwID;

	return true;
	}

bool CGItemListArea::HitTestTabs (int x, int y, int *retiTab)

//	HitTestTabs
//
//	See if we hit a tab.

	{
	int i;
	RECT rcRect = GetRect();

	if (y >= rcRect.top && y < rcRect.top + m_cyTabHeight)
		{
		int xTab = rcRect.left;

		for (i = 0; i < m_Tabs.GetCount(); i++)
			{
			if (x >= xTab && x < xTab + m_Tabs[i].cxWidth)
				{
				if (retiTab)
					*retiTab = i;

				return true;
				}

			xTab += m_Tabs[i].cxWidth;
			}
		}

	//	If we get this far, not found.

	return false;
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
	//	Hit test the tab area

	int iTab;
	if (HitTestTabs(x, y, &iTab))
		{
		if (!m_Tabs[iTab].bDisabled)
			SignalAction(m_Tabs[iTab].dwID);

		return true;
		}

	//	List

	else if (m_iOldCursor != -1 && m_pListData->GetCount())
		{
		int yListTop = GetRect().top + m_cyTabHeight;

		//	Figure out the cursor position that the user clicked on

		int iPos = FindRow((y - yListTop) - m_yFirst);
		if (iPos >= 0 && iPos < m_pListData->GetCount())
			SignalAction(iPos);

		return true;
		}

	return false;
	}

void CGItemListArea::MouseLeave (void)

//	MouseLeave
//
//	Handle hover

	{
	if (m_iHoverTab != -1)
		{
		m_iHoverTab = -1;
		Invalidate();
		}
	}

void CGItemListArea::MouseMove (int x, int y)

//	MouseMove
//
//	Handle hover

	{
	int iTab;
	if (!HitTestTabs(x, y, &iTab))
		iTab = -1;

	if (iTab != m_iHoverTab)
		{
		m_iHoverTab = iTab;
		Invalidate();
		}
	}

void CGItemListArea::MouseWheel (int iDelta, int x, int y, DWORD dwFlags)

//	MouseWheel
//
//	Handles scrolling

	{
	//	Short-circuit

	if (m_pListData == NULL)
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

void CGItemListArea::MoveTabToFront (DWORD dwID)

//	MoveTabToFront
//
//	Moves the tab to the front.

	{
	int iTab;

	if (!FindTab(dwID, &iTab))
		return;

	//	If we're already at the front, then nothing to do

	if (iTab == 0)
		return;

	//	Remember the selected tab so we can select it after everything moves.

	DWORD dwCurTab = 0;
	if (m_iCurTab != -1)
		dwCurTab = m_Tabs[m_iCurTab].dwID;

	//	Re-order the tabs

	STabDesc Tab = m_Tabs[iTab];
	m_Tabs.Delete(iTab);
	m_Tabs.Insert(Tab, 0);

	//	Set the new current tab

	if (m_iCurTab != -1)
		if (!FindTab(dwCurTab, &m_iCurTab))
			m_iCurTab = -1;

	Invalidate();
	}

void CGItemListArea::Paint (CG32bitImage &Dest, const RECT &rcRect)

//	Paint
//
//	Paint the area

	{
	DEBUG_TRY

	int i;

	//	Can't paint if we are not properly initialized

	if (m_pFonts == NULL)
		return;

	bool bPaintCursor = false;
	RECT rcCursor;

	//	Paint Background

    CG32bitPixel rgbFadeBackColor = CG32bitPixel(CG32bitPixel::Darken(m_rgbBackColor, 220), 220);
	CGDraw::RoundedRect(Dest,
			rcRect.left,
			rcRect.top,
			RectWidth(rcRect),
			RectHeight(rcRect),
			BORDER_RADIUS + 1,
			rgbFadeBackColor);

	//	Paint the tab area, if we have one

	if (m_Tabs.GetCount() > 0)
		{
		int x = rcRect.left + 1;
		int y = rcRect.top + 1;

		//	Paint the background of the tab area

		CG32bitPixel rgbBack = m_rgbBackColor;
		CGDraw::RoundedRect(Dest, rcRect.left + 1, rcRect.top + 1, RectWidth(rcRect) - 2, m_cyTabHeight - 2, BORDER_RADIUS - 1, rgbBack);

		Dest.FillLine(rcRect.left, rcRect.top + m_cyTabHeight - 1, RectWidth(rcRect), CG32bitPixel(80, 80, 80));

		//	Paint the tabs

		for (i = 0; i < m_Tabs.GetCount(); i++)
			{
			RECT rcTab;
			rcTab.left = x;
			rcTab.right = rcTab.left + m_Tabs[i].cxWidth;
			rcTab.top = y;
			rcTab.bottom = y + m_cyTabHeight - 2;

			PaintTab(Dest, m_Tabs[i], rcTab, (i == m_iCurTab), (i == m_iHoverTab));

			x += m_Tabs[i].cxWidth;
			}
		}

	//	Figure out where the list will paint

	RECT rcList = rcRect;
	rcList.top += m_cyTabHeight;

	//	If there are no items here, then say so

	if (m_pListData == NULL || !m_pListData->IsCursorValid())
		{
		int x = rcList.left + (RectWidth(rcList) - m_pFonts->LargeBold.MeasureText(STR_NO_ITEMS)) / 2;
		int y = rcList.top + (RectHeight(rcList) - m_pFonts->LargeBold.GetHeight()) / 2;

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

		//	Clip to the list rect

		RECT rcOldClip = Dest.GetClipRect();
		Dest.SetClipRect(rcList);

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

		int yIdeal = m_yOffset + ((RectHeight(rcList) - m_Rows[iCursor].cyHeight) / 2);

		//	Figure out the actual position of the cursor row (relative to the
		//	rect).

		int yCursor;

		//	If the cursor is in the top part of the list
		if (m_Rows[iCursor].yPos < yIdeal)
			yCursor = m_Rows[iCursor].yPos;

		//	If the total number of lines is less than the whole rect
		else if (m_cyTotalHeight < RectHeight(rcList))
			yCursor = m_Rows[iCursor].yPos;

		//	If the cursor is in the bottom part of the list
		else if ((m_cyTotalHeight - m_Rows[iCursor].yPos) < (RectHeight(rcList) - yIdeal))
			yCursor = (RectHeight(rcList) - (m_cyTotalHeight - m_Rows[iCursor].yPos));

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

		int y = rcList.top + yStart;
		int iPos = iStart;
		bool bPaintSeparator = false;
		RECT rcItem;

		while (y < rcList.bottom && iPos < m_Rows.GetCount())
			{
			//	Paint previous separator

			if (bPaintSeparator)
				{
				Dest.Fill(rcItem.left + BORDER_RADIUS,
						rcItem.bottom - 1,
						RectWidth(rcItem) - (BORDER_RADIUS * 2),
						1,
						CG32bitPixel(50,50,50));
				}
			else
				bPaintSeparator = true;

			//	Paint only if we have a valid entry. Sometimes we can
			//	start at an invalid entry because we're scrolling.

			if (iPos >= 0)
				{
				m_pListData->SetCursor(iPos);

				rcItem.top = y;
				rcItem.left = rcList.left;
				rcItem.bottom = y + m_Rows[iPos].cyHeight;
				rcItem.right = rcList.right;

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

				switch (m_iType)
					{
					case listCustom:
						PaintCustom(Dest, rcItem, bIsCursor);
						break;

					case listItem:
						PaintItem(Dest, m_pListData->GetItemAtCursor(), rcItem, bIsCursor);
						break;
					}
				}

			//	Next

			y += m_Rows[iPos].cyHeight;
			iPos++;
			}

		//	Done

		m_pListData->SetCursor(iCursor);
		Dest.SetClipRect(rcOldClip);
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

	DEBUG_CATCH
	}

void CGItemListArea::PaintCustom (CG32bitImage &Dest, const RECT &rcRect, bool bSelected)

//	PaintCustom
//
//	Paints a custom element

	{
	//	Paint the image

	m_pListData->PaintImageAtCursor(Dest, rcRect.left, rcRect.top, m_cxIcon, m_cyIcon, m_rIconScale);

	RECT rcDrawRect = rcRect;
	rcDrawRect.left += m_cxIcon + ITEM_TEXT_MARGIN_X;
	rcDrawRect.right -= ITEM_TEXT_MARGIN_X;
	rcDrawRect.top += ITEM_TEXT_MARGIN_Y;

	//	Measure the title and description

	CString sTitle = m_pListData->GetTitleAtCursor();
	int cyText = m_pFonts->LargeBold.GetHeight();

	CString sDesc = m_pListData->GetDescAtCursor();
	int iLines = m_pFonts->Medium.BreakText(sDesc, RectWidth(rcDrawRect), NULL, 0);
	cyText += iLines * m_pFonts->Medium.GetHeight();

	//	Text is vertically centered.

	int yOffset = Max(0, (RectHeight(rcDrawRect) - cyText) / 2);

	//	Paint the title

	int cyHeight;
	rcDrawRect.top += yOffset;
	m_pFonts->LargeBold.DrawText(Dest,
			rcDrawRect,
			m_pFonts->rgbItemTitle,
			m_pListData->GetTitleAtCursor(),
			0,
			CG16bitFont::SmartQuotes | CG16bitFont::TruncateLine,
			&cyHeight);

	rcDrawRect.top += cyHeight;

	//	Paint the description

	m_pFonts->Medium.DrawText(Dest, 
			rcDrawRect,
			(bSelected ? m_pFonts->rgbItemDescSelected : m_pFonts->rgbItemDesc),
			m_pListData->GetDescAtCursor(),
			0,
			CG16bitFont::SmartQuotes,
			&cyHeight);

	rcDrawRect.top += cyHeight;
	}

void CGItemListArea::PaintItem (CG32bitImage &Dest, const CItem &Item, const RECT &rcRect, bool bSelected)

//	PaintItem
//
//	Paints the item

	{
	CUIHelper UIHelper(*g_pHI);

	DWORD dwOptions = 0;
	if (bSelected)
		dwOptions |= CUIHelper::OPTION_SELECTED;
	if (m_bNoArmorSpeedDisplay)
		dwOptions |= CUIHelper::OPTION_NO_ARMOR_SPEED_DISPLAY;
	if (m_bActualItems)
		dwOptions |= CUIHelper::OPTION_KNOWN;

	UIHelper.PaintItemEntry(Dest, m_pListData->GetSource(), Item, rcRect, m_rgbTextColor, dwOptions);
	}

void CGItemListArea::PaintTab (CG32bitImage &Dest, const STabDesc &Tab, const RECT &rcRect, bool bSelected, bool bHover)

//	PaintTab
//
//	Paints a tab

	{
	const CG16bitFont &MediumFont = m_VI.GetFont(fontMedium);

	CG32bitPixel rgbBackColor;
	CG32bitPixel rgbTextColor;
	if (Tab.bDisabled)
		rgbTextColor = CG32bitPixel(m_rgbTextColor, 64);

	else if (bSelected)
		{
		rgbBackColor = CG32bitPixel(m_rgbTextColor, 128);
		rgbTextColor = m_rgbBackColor;
		}
	else if (bHover)
		{
		rgbBackColor = CG32bitPixel(m_rgbTextColor, 30);
		rgbTextColor = CG32bitPixel(m_rgbTextColor, 190);
		}
	else
		rgbTextColor = CG32bitPixel(m_rgbTextColor, 190);

	if (!Tab.bDisabled && (bSelected || bHover))
		CGDraw::RoundedRect(Dest,
				rcRect.left,
				rcRect.top,
				RectWidth(rcRect),
				RectHeight(rcRect),
				BORDER_RADIUS - 1,
				rgbBackColor);

	MediumFont.DrawText(Dest, rcRect, rgbTextColor, Tab.sLabel, 0, CG16bitFont::AlignCenter | CG16bitFont::AlignMiddle);
	}

void CGItemListArea::SelectTab (DWORD dwID)

//	SelectTab
//
//	Selects the given tab

	{
	int iTab;
	if (FindTab(dwID, &iTab))
		{
		m_iCurTab = iTab;
		Invalidate();
		}
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
		m_yOffset = CUIHelper::ScrollAnimationDecay(m_yOffset);
		Invalidate();
		}
	}

