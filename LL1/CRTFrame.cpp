//	CRTFrame.cpp
//
//	CRTFrame object

#include "PreComp.h"
#include "Tags.h"

CRTFrame::CRTFrame (void) : CObject(NULL),
		m_ParagraphList(TRUE),
		m_xScroll(0),
		m_yScroll(0),
		m_pCurFmt(NULL),
		m_pCurParagraph(NULL),
		m_pPrevParagraph(NULL),
		m_pCurTextFmt(NULL),
		m_pCurRun(NULL),
		m_fAddTab(FALSE),
		m_fCurParagraphIsHeading(FALSE),
		m_fCurParagraphIsFirstOfBlock(FALSE)

//	CRTFrame constructor

	{
	}

ALERROR CRTFrame::AddTab (void)

//	AddTab
//
//	Adds a tab to the next run

	{
	//	Done with this run

	m_pCurRun = NULL;
	m_fAddTab = TRUE;

	return NOERROR;
	}

ALERROR CRTFrame::AddText (CString sText)

//	AddText
//
//	Adds a string of text

	{
	ALERROR error;

	//	Don't bother if we've got no text

	if (sText.IsBlank())
		return NOERROR;

	//	We better have a current format already set up

	ASSERT(m_pCurFmt);
	ASSERT(m_pCurTextFmt);

	//	If we don't have a current run, create a new one

	if (m_pCurRun == NULL)
		{
		CRTRun *pRun = new CRTRun;
		if (pRun == NULL)
			return ERR_MEMORY;

		//	Check to see if we have a current paragraph; if not,
		//	create one

		if (m_pCurParagraph == NULL)
			{
			CRTParagraph *pParagraph = new CRTParagraph;
			if (pParagraph == NULL)
				{
				delete pRun;
				return ERR_MEMORY;
				}

			//	Add the paragraph to our list

			if (error = m_ParagraphList.AppendObject(pParagraph, NULL))
				{
				delete pParagraph;
				delete pRun;
				return error;
				}

			//	Set the current format

			pParagraph->SetFormat(m_pCurFmt);

			//	Set whether or not this paragraph is a heading

			if (m_fCurParagraphIsHeading)
				pParagraph->SetHeading();

			//	Set whether or not this paragraph is the first of a block

			if (m_fCurParagraphIsFirstOfBlock)
				pParagraph->SetFirstOfBlock();

			//	Set the current paragraph

			m_pCurParagraph = pParagraph;
			}

		//	Add the run to our current paragraph

		if (error = m_pCurParagraph->AppendRun(pRun))
			{
			delete pRun;
			return error;
			}

		//	Set the current format

		pRun->SetFormat(m_pCurTextFmt);
		pRun->SetLinkInfo(m_sCurLink);

		//	See if we need to add a tab

		if (m_fAddTab)
			{
			pRun->SetTab();
			m_fAddTab = FALSE;
			}

		//	Set the current run

		m_pCurRun = pRun;
		}

	//	Append the text to the run

	if (error = m_pCurRun->AppendText(sText))
		return error;

	return NOERROR;
	}

CRTRun *CRTFrame::HitTestRun (int xOrigin, int yOrigin, POINT &ptPoint)

//	HitTestRun
//
//	Returns the run that is at the given point. xOrigin and yOrigin are
//	the coordinates of the top-left of the frame relative to the client
//	area.

	{
	int i;
	POINT ptHitPoint;

	//	Offset the hit point so that it is in the frame's coordinate
	//	space (where the origin is 0,0)

	ptHitPoint.x = ptPoint.x - xOrigin;
	ptHitPoint.y = ptPoint.y - yOrigin;

	//	Ask each of the paragraphs

	for (i = 0; i < m_ParagraphList.GetCount(); i++)
		{
		CRTParagraph *pParagraph = GetParagraph(i);
		CRTRun *pRun;

		if (pRun = pParagraph->HitTestRun(ptHitPoint))
			return pRun;
		}

	//	Not found

	return NULL;
	}

void CRTFrame::Justify (HDC hDC, RECT *ioRect, Twips TwipsPerPixel)

//	Justify
//
//	Measure and justify all the runs

	{
	int cxWidth = RectWidth(ioRect);
	int i;
	int x, y;
	int iParagraphNumber;
	CRTParagraph *pPrevParagraph = NULL;

	//	Start the origin at the top-left

	x = 0;
	y = 0;

	//	Loop over all paragraphs

	iParagraphNumber = -1;
	for (i = 0; i < m_ParagraphList.GetCount(); i++)
		{
		CRTParagraph *pParagraph = GetParagraph(i);
		RECT rcParagraph;

		//	Handle numbered paragraphs

		if (pParagraph->IsNumbered())
			{
			if (iParagraphNumber == -1)
				iParagraphNumber = 1;
			else
				iParagraphNumber++;

			pParagraph->SetNumber(iParagraphNumber);
			}
		else
			iParagraphNumber = -1;

		//	If the previous paragraph is a heading (or if this is the
		//	first paragraph), tell this paragraph that it is the first
		//	paragraph of a section

		if (pPrevParagraph == NULL || pPrevParagraph->IsHeading())
			pParagraph->SetFirstOfSection();

		//	Create a rect for the paragraph. The justification will return
		//	a rect that encloses the paragraph. Note that the bottom is
		//	never set on input since we allow scrolling.

		rcParagraph.left = x;
		rcParagraph.top = y;
		rcParagraph.right = rcParagraph.left + cxWidth;
		rcParagraph.bottom = 0;

		//	Justify each paragraph

		pParagraph->Justify(hDC, &rcParagraph, TwipsPerPixel);

		//	Increment the y position

		y += RectHeight(&rcParagraph);

		//	Remember this paragraph as the previous one

		pPrevParagraph = pParagraph;
		}

	//	Return our rect

	ioRect->bottom = ioRect->top + y;

	//	Remember our justification

	m_rcJustify = *ioRect;
	m_rcJustify.top = 0;
	m_rcJustify.bottom = y;
	}

ALERROR CRTFrame::NewParagraph (void)

//	NewParagraph
//
//	starts a new paragraph

	{
	//	Remember the previous paragraph

	if (m_pCurParagraph)
		m_pPrevParagraph = m_pCurParagraph;

	//	End the run and the paragraph

	m_pCurRun = NULL;
	m_pCurParagraph = NULL;

	//	Clear flags

	m_fCurParagraphIsHeading = FALSE;
	m_fCurParagraphIsFirstOfBlock = FALSE;

	return NOERROR;
	}

void CRTFrame::Paint (HDC hDC, int x, int y, RECT &rcUpdateRect)

//	Paint
//
//	Paints the rect at the given screen locations.

	{
	int i;
	RECT rcFrame;

	//	Check to see if we are inside the update rect; if not, don't
	//	bother painting

	rcFrame = m_rcJustify;

	//	HACK: We need to offset the RECT so that we can get to
	//	scrolled coordinates. Unfortunately, the y coordinates of
	//	m_rcJustify are always 0 based while the x coordinates are
	//	already adjusted to the output view. Thus we only offset
	//	the y coordinates.

	OffsetRect(&rcFrame, 0, y);

	if (!RectsIntersect(rcFrame, rcUpdateRect))
		return;

	//	Loop over all paragraphs

	for (i = 0; i < m_ParagraphList.GetCount(); i++)
		{
		CRTParagraph *pParagraph = (CRTParagraph *)m_ParagraphList.GetObject(i);

		//	Paint the paragraph

		pParagraph->Paint(hDC, x, y, rcUpdateRect);
		}
	}

ALERROR CRTFrame::SetCurrentLink (CString sLink)

//	SetCurrentLink
//
//	Sets the current link information

	{
	//	End the run

	m_pCurRun = NULL;

	//	Set the link info

	m_sCurLink = sLink;

	return NOERROR;
	}

