//	CDaimonListPainter.cpp
//
//	CDaimonListPainter class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

const int ICON_WIDTH =						64;
const int ICON_HEIGHT =						64;

const int CORNER_RADIUS =					4;
const int LINE_WIDTH =						1;
const int SELECTION_EXTRA_WIDTH =			40;

CDaimonListPainter::CDaimonListPainter (const CVisualPalette &VI) :
		m_VI(VI),
		m_pList(NULL),
		m_cxWidth(100),
		m_yAnimation(0)

	//	CDaimonListPainter constructor

	{
	}

int CDaimonListPainter::CalcListPos (void) const

//	CalcListPos
//
//	Returns the y coordinate of the top of the list in destination (m_rcRect)
//	coordinates. We assume that Justify has been called.

	{
	if (m_pList == NULL)
		return 0;

	int iSelection = m_pList->GetSelection();
	if (iSelection == -1)
		return 0;

	//	Compute the offset from the top of the RECT to the top of the selected
	//	entry. The selected entry is centered.

	int yOffset = (RectHeight(m_rcRect) - m_pList->GetDaimonHeight(iSelection)) / 2;

	//	Compute the origin of the full list in RECT coordinates.

	return m_rcRect.top + (yOffset - CalcTop(iSelection)) + m_yAnimation;
	}

int CDaimonListPainter::CalcTop (int iIndex) const

//	CalcTop
//
//	Computes the distance between the absolute top of the list (at y = 0) and
//	the top of the given entry (in pixels). Must call Justify before this.

	{
	int i;

	if (m_pList == NULL)
		return 0;

	//	Add up the heights until we get to the index

	int y = 0;
	for (i = 0; i < iIndex; i++)
		y += m_pList->GetDaimonHeight(i);

	//	Done

	return y;
	}

bool CDaimonListPainter::HitTest (int xPos, int yPos, int *retiIndex) const

//	HitTest
//
//	If we've clicked on an entry, we return TRUE and the index of the entry.

	{
	int i;

	if (m_pList == NULL)
		return 0;

	Justify(m_cxWidth);

	//	Compute some metrics

	int x = m_rcRect.left;
	int y = CalcListPos();

	//	Loop over all daimons

	for (i = 0; i < m_pList->GetCount(); i++)
		{
		int cyDaimon = m_pList->GetDaimonHeight(i);

		//	If we're above our RECT, then skip

		if (y + cyDaimon <= m_rcRect.top)
			{
			y += cyDaimon;
			continue;
			}

		//	If we're below our RECT, then we're done

		if (y >= m_rcRect.bottom)
			break;

		//	See if we're in the rect

		if (xPos >= x && xPos < x + m_cxWidth && yPos >= y && yPos < y + cyDaimon)
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}

		//	Next

		y += cyDaimon;
		}

	//	Not found, but if we're in our rect, then return TRUE because we want
	//	to handle scroll wheel.

	if (retiIndex)
		*retiIndex = -1;

	return (xPos >= m_rcRect.left && xPos < m_rcRect.right && yPos >= m_rcRect.top && yPos < m_rcRect.bottom);
	}

int CDaimonListPainter::Justify (int cxWidth) const

//	Justify
//
//	Justify all entries and set the height parameters. OK to call if we're 
//	already justified.
//
//	Returns the total height of all daimons.

	{
	int i;

	if (m_pList == NULL)
		return 0;

	CUIHelper Helper(*g_pHI);
	RECT rcRect;
	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = cxWidth;
	rcRect.bottom = 1000;

	int yTotal = 0;
	for (i = 0; i < m_pList->GetCount(); i++)
		{
		//	If already justified, skip.

		if (m_pList->GetDaimonHeight(i) != -1)
			{
			yTotal += m_pList->GetDaimonHeight(i);
			continue;
			}

		//	Format appropriately.

		CItemType *pDaimon = m_pList->GetDaimon(i);

		int yDaimon = Helper.CalcItemEntryHeight(NULL, CItem(pDaimon, 1), rcRect, CUIHelper::OPTION_SMALL_ICON | CUIHelper::OPTION_TITLE);

		m_pList->SetDaimonHeight(i, yDaimon);
		yTotal += yDaimon;
		}

	return yTotal;
	}

void CDaimonListPainter::OnSelectionChanged (int iOldSelection, int iNewSelection)

//	OnSelectionChanged
//
//	The selection has changed.

	{
	if (m_pList == NULL)
		return;

	Justify(m_cxWidth);

	//	Compute the difference between old and new. We calculate the distance
	//	(in pixels) to ADD to the vertical paint position in order to get
	//	the old selection to be painted in the select position.

	m_yAnimation = (CalcTop(iNewSelection) - CalcTop(iOldSelection)) + (m_pList->GetDaimonHeight(iNewSelection) - m_pList->GetDaimonHeight(iOldSelection)) / 2;
	}

void CDaimonListPainter::OnSelectionDeleted (int iOldSelection)

//	OnSelectionDeleted
//
//	The selection has been deleted.

	{
	if (m_pList == NULL)
		return;

	Justify(m_cxWidth);
	m_yAnimation = 0;
	}

void CDaimonListPainter::Paint (CG32bitImage &Dest)

//	Paint
//
//	Paint the list

	{
	int i;

	//	If no list, no need to paint

	if (m_pList == NULL)
		return;

	int iSelection = m_pList->GetSelection();
	if (iSelection == -1)
		return;

	//	Make sure we are justified (so we know the heights of all daimons)

	Justify(m_cxWidth);

	//	Compute some metrics

	int x = m_rcRect.left;
	int y = CalcListPos();

	//	Make sure we don't paint outside our bounds

	RECT rcOldClip = Dest.GetClipRect();
	Dest.SetClipRect(m_rcRect);

	//	Paint all daimons

	for (i = 0; i < m_pList->GetCount(); i++)
		{
		int cyDaimon = m_pList->GetDaimonHeight(i);

		//	If we're above our RECT, then skip

		if (y + cyDaimon <= m_rcRect.top)
			{
			y += cyDaimon;
			continue;
			}

		//	If we're below our RECT, then we're done

		if (y >= m_rcRect.bottom)
			break;

		//	If this is the selection, draw a background

		if (i == iSelection)
			{
			Dest.ResetClipRect();
			CGDraw::RoundedRect(Dest, x, y, m_cxWidth + SELECTION_EXTRA_WIDTH, cyDaimon, CORNER_RADIUS, AA_STYLECOLOR(colorDeployDaimonBack));
			CGDraw::RoundedRectOutline(Dest, x, y, m_cxWidth + SELECTION_EXTRA_WIDTH, cyDaimon, CORNER_RADIUS, LINE_WIDTH, AA_STYLECOLOR(colorDeployDaimonFore));
			Dest.SetClipRect(m_rcRect);
			}

		//	Paint

		PaintDaimon(Dest, m_pList->GetDaimon(i), x, y, m_cxWidth, cyDaimon);

		//	Next

		y += cyDaimon;
		}

	//	Done

	Dest.SetClipRect(rcOldClip);
	}

void CDaimonListPainter::PaintDaimon (CG32bitImage &Dest, CItemType *pDaimon, int x, int y, int cxWidth, int cyHeight)

//	PaintDaimon
//
//	Paints a daimon entry at the given position.

	{
	CUIHelper Helper(*g_pHI);

	RECT rcItem;
	rcItem.left = x;
	rcItem.top = y;
	rcItem.right = x + cxWidth;
	rcItem.bottom = y + cyHeight;
	Helper.PaintItemEntry(Dest, NULL, CItem(pDaimon, 1), rcItem, AA_STYLECOLOR(colorDeployDaimonTitle), CUIHelper::OPTION_SMALL_ICON | CUIHelper::OPTION_TITLE);
	}

bool CDaimonListPainter::Update (void)

//	Update
//
//	Update animation. Returns TRUE if we need to repaint.

	{
	if (m_yAnimation)
		{
		m_yAnimation = CUIHelper::ScrollAnimationDecay(m_yAnimation);
		return true;
		}

	return false;
	}
