//	CRTRun.cpp
//
//	CRTRun object

#include "PreComp.h"

CRTRun::CRTRun (void) : CObject(NULL),
		m_pFmt(NULL),
		m_fSplit(FALSE),
		m_fStartOfLine(FALSE),
		m_iBaseline(0)

//	CRTRun constructor

	{
	}

CString CRTRun::GetTextToPaint (void)

//	GetTextToPaint
//
//	Sometimes the text itself is changed by the format (for example,
//	when doing all-caps or ligatures. This function returns the
//	text that should actually be painted, taking the format into account

	{
	CString sText = m_sText;

	//	If this run is the start of the line, then trim out
	//	any leading spaces

	if (m_fStartOfLine)
		{
		char *pStart = sText.GetASCIIZPointer();
		strParseWhitespace(pStart, &pStart);

		if (pStart != sText.GetASCIIZPointer())
			sText = CString(pStart);
		}

	//	Convert to all-caps if necessary

	if (m_pFmt->GetAllCaps() || m_pFmt->GetSmallCaps())
		sText = strToUpper(sText);

	return sText;
	}

void CRTRun::Measure (HDC hDC,
		Twips TwipsPerPixel,
		int cxSpaceLeft,
		BOOL bStartOfLine,
		RECT *retRect,
		int *retiSplitAt,
		int *retiBaseline)

//	Measure
//
//	Measures the run
//
//	hDC: DC to use for font measurements
//	cxSpaceLeft: Pixels left on this line
//	bStartOfLine: If TRUE, we are allowed to split in the middle of the first word
//	retRect: Rect of the run (after a split)
//	retiSplitAt: -1 if the run fits; otherwise, char pos to split at
//	retiBaseline: Offset from the top of the rect to the baseline

	{
	HFONT hFont, hOldFont;
	int *pCharWidth;
	int iFit;
	int iCharsToTry;
	TEXTMETRIC tm;
	SIZE size;
	CString sText;

	//	Remember that we're the start of the line

	m_fStartOfLine = bStartOfLine;

	ASSERT(m_pFmt);
	sText = GetTextToPaint();

	//	Figure out which font to use and select it into our DC

	hFont = m_pFmt->GetFont();
	hOldFont = (HFONT)SelectObject(hDC, hFont);
	GetTextMetrics(hDC, &tm);

	//	Figure out how many characters we should try to pass. We don't want
	//	to pass too many characters in or else we will end up doing too much
	//	wasted work.

	if (tm.tmAveCharWidth > 0)
		iCharsToTry = min(sText.GetLength(), 1 + ((cxSpaceLeft + (cxSpaceLeft / 4)) / tm.tmAveCharWidth));
	else
		iCharsToTry = sText.GetLength();

	//	Allocate an int array to hold measurements for each character

	pCharWidth = (int *)MemStackAlloc(sizeof(int) * iCharsToTry);

	//	Get the widths

	GetTextExtentExPoint(hDC,
			sText.GetPointer(),
			iCharsToTry,
			cxSpaceLeft,
			&iFit,
			pCharWidth,
			&size);

	//	If, for some reason, our initial guess for iCharsToTry was too
	//	low, then do it again

	if (iFit == iCharsToTry && iCharsToTry < sText.GetLength())
		{
		//ASSERT(FALSE);

		pCharWidth = (int *)MemStackAlloc(sizeof(int) * sText.GetLength());

		GetTextExtentExPoint(hDC,
				sText.GetPointer(),
				sText.GetLength(),
				cxSpaceLeft,
				&iFit,
				pCharWidth,
				&size);
		}

	//	If the entire text did not fit, we need to figure out where we
	//	can break the line

	if (iFit < sText.GetLength())
		{
		if (iFit > 0)
			{
			int iBreak;
			char *pPos;

			//	Look for whitespace or a hyphenation mark to break
			//	the line at.

			iBreak = iFit - 1;
			pPos = sText.GetPointer();

			while (iBreak > 0
					&& !strIsWhitespace(pPos + iBreak)
					&& *pPos != '-')
				iBreak--;

			//	If we don't find any breaks, see if this is the
			//	first run of the line. If so, we have no choice
			//	but to break the line in the middle.

			if (iBreak == 0)
				{
				if (bStartOfLine)
					size.cx = pCharWidth[iFit - 1];
				else
					{
					iFit = 0;
					size.cx = 0;
					}
				}

			//	Otherwise, break the line here

			else
				{
				iFit = iBreak + 1;
				size.cx = pCharWidth[iBreak];
				}
			}
		else
			{
			iFit = 1;
			size.cx = pCharWidth[0];
			}
		}

	//	Otherwise, the whole run did fit, and so we don't need
	//	to split this run

	else
		iFit = -1;

	//	Done

	SelectObject(hDC, hOldFont);

	*retiSplitAt = iFit;
	retRect->left = 0;
	retRect->top = 0;
	retRect->right = size.cx;
	retRect->bottom = size.cy;

	if (retiBaseline)
		*retiBaseline = tm.tmAscent;
	}

void CRTRun::Paint (HDC hDC, int x, int y, RECT &rcUpdateRect)

//	Paint
//
//	Paint the run

	{
	HFONT hFont, hOldFont;
	CString sText = GetTextToPaint();

	//	Check to see if we are inside the update rect; if not, don't
	//	bother painting

	RECT rcRect = m_rcJustify;
	OffsetRect(&rcRect, x, y);
	if (!RectsIntersect(rcRect, rcUpdateRect))
		return;

	//	Setup the DC

	hFont = m_pFmt->GetFont();
	hOldFont = (HFONT)SelectObject(hDC, hFont);
	SetTextColor(hDC, m_pFmt->GetTextColor());
	SetTextAlign(hDC, TA_BASELINE);
	if (m_pFmt->GetOpaque())
		{
		SetBkMode(hDC, OPAQUE);
		SetBkColor(hDC, m_pFmt->GetBackColor());
		}
	else
		SetBkMode(hDC, TRANSPARENT);

	//	Paint

	TextOut(hDC,
			x + m_rcJustify.left,
			y + m_rcJustify.top + m_iBaseline,
			sText.GetPointer(),
			sText.GetLength());

	//	Done

	SelectObject(hDC, hOldFont);
	}

ALERROR CRTRun::Split (int iSplitAt, CRTRun **retpRun)

//	Split
//
//	Splits the run at the given character position

	{
	CRTRun *pNewRun;

	pNewRun = new CRTRun;
	if (pNewRun == NULL)
		return ERR_MEMORY;

	pNewRun->m_pFmt = m_pFmt;
	pNewRun->m_sLinkTo = m_sLinkTo;
	pNewRun->m_fSplit = TRUE;

	//	Copy the text

	pNewRun->m_sText = strSubString(m_sText, iSplitAt, m_sText.GetLength() - iSplitAt);
	m_sText = strSubString(m_sText, 0, iSplitAt);

	//	Done

	*retpRun = pNewRun;

	return NOERROR;
	}
