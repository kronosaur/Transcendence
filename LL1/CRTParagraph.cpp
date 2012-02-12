//	CRTParagraph.cpp
//
//	CRTParagraph object

#include "PreComp.h"

CRTParagraph::CRTParagraph (void) : CObject(NULL),
		m_RunList(TRUE), 
		m_iNumber(-1),
		m_fHeading(FALSE),
		m_fFirstOfSection(FALSE),
		m_fFirstOfBlock(FALSE),
		m_fLastOfBlock(FALSE)
	
//	CRTParagraph constructor

	{
	}

CRTRun *CRTParagraph::HitTestRun (POINT &ptPoint)

//	HitTestRun
//
//	Hit test to see if the point is in one of our runs

	{
	//	If we're not in this rect, then bail

	if (PtInRect(&m_rcJustify, ptPoint))
		{
		int i;

		for (i = 0; i < m_RunList.GetCount(); i++)
			{
			CRTRun *pRun = GetRun(i);

			if (pRun->HitTest(ptPoint))
				return pRun;
			}
		}

	//	Not found

	return NULL;
	}

void CRTParagraph::Justify (HDC hDC, RECT *ioRect, Twips TwipsPerPixel)

//	Justify
//
//	Measure and justify all the runs

	{
	int cxWidth = RectWidth(ioRect);
	int i, x, y;
	int iNextRunToProcess;
	BOOL bFirstLineIndent;
	BOOL bSpaceBefore;

	//	Start the origin at the top-left

	y = ioRect->top;
	iNextRunToProcess = 0;

	//	Figure out if we need to skip space before

	bSpaceBefore = TRUE;

	//	We don't need space before if the previous paragraph
	//	is a heading

	if (m_fFirstOfSection)
		bSpaceBefore = FALSE;

	//	We don't need space before if this is in the middle of
	//	a block.

	if (m_pFmt->SpacingBeforeFirstOfBlock() && !IsFirstOfBlock())
		bSpaceBefore = FALSE;

	//	Spacing

	if (bSpaceBefore)
		y += Twips2Pixels(m_pFmt->m_SpaceBefore, TwipsPerPixel);

	//	Figure out where to draw bullets

	if (m_pFmt->HasBullet())
		{
		m_xBullet = ioRect->left + Twips2Pixels(m_pFmt->m_LeftMargin + m_pFmt->m_FirstLineLeftMargin, TwipsPerPixel);
		m_yBullet = y;
		}

	//	Figure out if we need to do a first line indent

	bFirstLineIndent = TRUE;

	//	If we have a bullet, then we don't do a first line indent
	//	because the bullet is outdented at the first line indent mark

	if (m_pFmt->HasBullet())
		bFirstLineIndent = FALSE;

	//	If this paragraph is the first paragraph after a section and
	//	we have the appropriate flag, then also ignore the first line
	//	indent.

	if (m_pFmt->NoIndentAfterHeading() && m_fFirstOfSection)
		bFirstLineIndent = FALSE;

	//	Loop over all lines until we've processed all runs

	while (iNextRunToProcess < m_RunList.GetCount())
		{
		int cxSpaceLeft;
		int iRun;
		int cyLineHeight;
		int cyLineBaseline;
		Twips LeftMargin;
		int cxEntireLine;

		//	Adjust for the margins

		LeftMargin = m_pFmt->m_LeftMargin;
		if (bFirstLineIndent)
			{
			LeftMargin += m_pFmt->m_FirstLineLeftMargin;
			bFirstLineIndent = FALSE;
			}

		//	First thing we need to do is to figure out how many runs
		//	will fit on this line

		cxSpaceLeft = cxWidth - Twips2Pixels(LeftMargin + m_pFmt->m_RightMargin, TwipsPerPixel);
		cyLineHeight = 0;
		cyLineBaseline = 0;
		iRun = iNextRunToProcess;
		x = ioRect->left + Twips2Pixels(LeftMargin, TwipsPerPixel);
		cxEntireLine = cxSpaceLeft;

		//	Loop until all the runs for this line have been processed

		while (cxSpaceLeft > 0 && iRun < m_RunList.GetCount())
			{
			CRTRun *pRun = GetRun(iRun);
			RECT rcRun;
			int iSplitAt;
			int cyBaseline;

			//	If this run has a tab at the beginning, figure out where
			//	we need to advance to.

			if (pRun->HasTab())
				{
				Twips NextTab;

				NextTab = m_pFmt->GetNextTab(x * TwipsPerPixel);
				if (NextTab > 0.0)
					{
					int Skip = (int)((NextTab - x * TwipsPerPixel) / TwipsPerPixel);

					x += Skip;
					cxSpaceLeft -= Skip;
					}
				}

			//	Measure this run. If the run is bigger than the space we have
			//	left on the line, then see where we can split it.

			pRun->Measure(hDC,
					TwipsPerPixel, 
					cxSpaceLeft, 
					(cxSpaceLeft == cxEntireLine),	//	If we've got the whole line, allow splitting a very long word
					&rcRun, 
					&iSplitAt,
					&cyBaseline);

			//	If none of the line fits, then we're done

			if (iSplitAt != -1 && rcRun.right == 0)
				cxSpaceLeft = 0;

			//	Otherwise, handle the run

			else
				{
				//	If only part of the run fits, split the run

				if (iSplitAt != -1)
					{
					CRTRun *pNewRun;

					//	Split the run

					pRun->Split(iSplitAt, &pNewRun);
					m_RunList.InsertObject(pNewRun, iRun + 1, NULL);

					//	We're done

					cxSpaceLeft = 0;
					}
				else
					cxSpaceLeft -= rcRun.right;

				//	Set the justification for this run

				RECT rcJustify;
				rcJustify.left = x;
				rcJustify.top = y;
				rcJustify.right = rcJustify.left + rcRun.right;
				rcJustify.bottom = rcJustify.top + rcRun.bottom;
				pRun->SetJustification(rcJustify);

				//	Line height

				if (rcRun.bottom > cyLineHeight)
					cyLineHeight = rcRun.bottom;

				//	Base line

				if (cyBaseline > cyLineBaseline)
					cyLineBaseline = cyBaseline;

				//	Next run

				iRun++;
				x += rcRun.right;
				}
			}

		//	This line is composed of runs from iNextRunToProcess to iRun-1.

		ASSERT(iRun > iNextRunToProcess);

		//	Now loop over all the runs of this line and adjust their
		//	justification so that their baselines match (this can only be done
		//	afterwards, since we don't know cyLineHeight until we've got
		//	all runs).

		for (i = iNextRunToProcess; i < iRun; i++)
			{
			CRTRun *pRun = GetRun(i);
			RECT rcJustify;

			//	Adjust justification

			rcJustify = pRun->GetJustification();
			rcJustify.bottom = rcJustify.top + cyLineHeight;
			pRun->SetJustification(rcJustify);

			//	Adjust baseline

			pRun->SetBaseline(cyLineBaseline);
			}

		//	Next line

		iNextRunToProcess = iRun;
		y += cyLineHeight;
		}

	//	Skip some space after

	if (!m_pFmt->SpacingAfterLastOfBlock() || IsLastOfBlock())
		y += Twips2Pixels(m_pFmt->m_SpaceAfter, TwipsPerPixel);

	//	Return our rect

	ioRect->bottom = y;

	//	Remember our justification

	m_rcJustify = *ioRect;
	}

void CRTParagraph::Paint (HDC hDC, int x, int y, RECT &rcUpdateRect)

//	Paint
//
//	Paints the rect at the given screen locations.

	{
	int i;

	//	Check to see if we are inside the update rect; if not, don't
	//	bother painting

	RECT rcRect = m_rcJustify;
	OffsetRect(&rcRect, x, y);
	if (!RectsIntersect(rcRect, rcUpdateRect))
		return;

	//	Paint any numbering, if necessary

	if (m_pFmt->HasBullet() && m_RunList.GetCount() > 0)
		{
		HFONT hFont, hOldFont;
		CRTTextFormat *pFmt = GetRun(0)->GetFormat();
		int iBaselineOffset = GetRun(0)->GetBaseline();
		CString sBullet;

		//	Figure out the number to paint

		sBullet = m_pFmt->GetBullet(m_iNumber);

		//	Setup the DC

		hFont = pFmt->GetFont();
		hOldFont = (HFONT)SelectObject(hDC, hFont);
		SetTextColor(hDC, pFmt->GetTextColor());
		SetTextAlign(hDC, TA_BASELINE);
		if (pFmt->GetOpaque())
			{
			SetBkMode(hDC, OPAQUE);
			SetBkColor(hDC, pFmt->GetBackColor());
			}
		else
			SetBkMode(hDC, TRANSPARENT);

		//	Paint

		TextOut(hDC,
				x + m_xBullet,
				y + m_yBullet + iBaselineOffset,
				sBullet.GetPointer(),
				sBullet.GetLength());

		//	Done

		SelectObject(hDC, hOldFont);
		}

	//	Loop over all runs

	for (i = 0; i < m_RunList.GetCount(); i++)
		{
		CRTRun *pRun = GetRun(i);

		//	Paint the run

		pRun->Paint(hDC, x, y, rcUpdateRect);
		}
	}

