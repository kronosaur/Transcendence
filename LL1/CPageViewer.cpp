//	CPageViewer.cpp
//
//	CPageViewer object

#include "PreComp.h"

#define PORTRAIT_PAGE_RATIO					1414
#define LANDSCAPE_PAGE_RATIO				1414
#define PORTRAIT_TEXTBLOCK_RATIO			1000
#define PORTRAIT_NOTESBLOCK_RATIO			1618
#define LANDSCAPE_TEXTBLOCK_RATIO			1000

#define RGB_PAGE_COLOR						RGB(255,255,235)
#define RGB_TEXTBLOCK_COLOR					RGB(255,255,240)

#define FIELD_SEPARATOR_HEIGHT				16

static CObjectClass<CPageViewer>g_Class(OBJID_CPAGEVIEWER, NULL);

CPageViewer::CPageViewer (void) : CObject(&g_Class)

//	CPageViewer constructor

	{
	}

CPageViewer::CPageViewer (IUController *pController) : CObject(&g_Class),
		m_pController(pController),
		m_pEntry(NULL),
		m_pFormat(NULL),
		m_pBody(NULL),
		m_pTitle(NULL),
		m_pRelatedLinks(NULL),
		m_fButtonDown(FALSE),
		m_pClickedOn(NULL),
		m_pfActionMsg(NULL)

//	CPageViewer destructor

	{
	}

CPageViewer::~CPageViewer (void)

//	CPageViewer destructor

	{
	SetPage(NULL);
	}

ALERROR CPageViewer::Activate (CUFrame *pFrame, RECT *pRect)

//	Activate
//
//	Show the frame

	{
	m_pFrame = pFrame;
	SetRect(pRect);

	return NOERROR;
	}

void CPageViewer::Animate (void)

//	Animate
//
//	Do animation

	{
	}

BOOL CPageViewer::ButtonDown (POINT ptPoint, DWORD dwFlags)

//	ButtonDown
//
//	Handle button down

	{
	//	Check to see if the user clicked on a link in the body field

	if (PtInRect(&m_rcBody, ptPoint))
		{
		CRTRun *pRun = m_pBody->HitTestRun(m_rcBody.left, m_rcBody.top - m_cyScroll, ptPoint);

		if (pRun)
			{
			CString sLink = pRun->GetLinkInfo();

			if (!sLink.IsBlank())
				{
				m_pFrame->CaptureMouse();
				m_pClickedOn = pRun;
				m_pClickedOnFrame = m_pBody;
				m_fButtonDown = TRUE;
				return TRUE;
				}
			}
		}
	else if (PtInRect(&m_rcRelatedLinks, ptPoint))
		{
		CRTRun *pRun = m_pRelatedLinks->HitTestRun(m_rcRelatedLinks.left, m_rcRelatedLinks.top, ptPoint);

		if (pRun)
			{
			CString sLink = pRun->GetLinkInfo();

			if (!sLink.IsBlank())
				{
				m_pFrame->CaptureMouse();
				m_pClickedOn = pRun;
				m_pClickedOnFrame = m_pRelatedLinks;
				m_fButtonDown = TRUE;
				return TRUE;
				}
			}
		}

	return FALSE;
	}

void CPageViewer::ButtonUp (POINT ptPoint, DWORD dwFlags)

//	ButtonUp
//
//	Handle button up

	{
	if (m_fButtonDown)
		{
		CRTRun *pRun;

		//	Release capture

		m_pFrame->ReleaseCapture();
		m_fButtonDown = FALSE;

		//	Do the action if we're still over the same link

		if (m_pClickedOnFrame == m_pBody)
			pRun = m_pClickedOnFrame->HitTestRun(m_rcBody.left, m_rcBody.top - m_cyScroll, ptPoint);
		else if (m_pClickedOnFrame == m_pRelatedLinks)
			pRun = m_pClickedOnFrame->HitTestRun(m_rcRelatedLinks.left, m_rcRelatedLinks.top, ptPoint);
		else
			pRun = NULL;

		if (pRun == m_pClickedOn)
			{
			CallNotifyProc(m_pController, m_pfActionMsg, 0, 0);
			}
		}
	}

void CPageViewer::Deactivate (void)

//	Deactivate
//
//	Deactivate

	{
	//	Clean up our entry

	SetPage(NULL);
	}

ALERROR CPageViewer::GenerateFieldSection (CEntry *pEntry, CString *retsFieldSection, int *retiFieldCount)

//	GenerateFieldSection
//
//	Formats all the fields in pEntry as an XML stream

	{
	ALERROR error;
	CMemoryWriteStream Output;
	BOOL bBreakNeeded;
	int i;
	int iFieldCount = 0;

	//	If we've got no fields, blow out of here

	if (pEntry->GetFieldCount() == 0)
		{
		*retiFieldCount = 0;
		return NOERROR;
		}

	//	Begin output

	if (error = Output.Create())
		return error;

	//	Output all the fields

	bBreakNeeded = FALSE;
	for (i = 0; i < pEntry->GetFieldCount(); i++)
		{
		CField *pField = pEntry->GetField(i);
		CString sName = pField->GetName();
		int j;

		//	Skip system fields

		if (pField->IsSystemField())
			continue;

		//	Skip code fields

		if (pField->GetType() == CField::Code)
			continue;

		//	Output a break

		if (bBreakNeeded)
			{
			if (error = Output.Write("<BR>", 4, NULL))
				return error;
			}

		//	Write out the field name

		if (error = Output.Write(sName.GetPointer(), sName.GetLength(), NULL))
			return error;

		//	Output a tab

		if (error = Output.Write("<TAB/>", 6, NULL))
			return error;

		//	Output all the values (separated by commas)

		BOOL bCommaNeeded = FALSE;
		for (j = 0; j < pField->GetValueCount(); j++)
			{
			CString sValue = pField->GetValue(j);

			//	Output a comma, if necessary

			if (bCommaNeeded)
				{
				if (error = Output.Write(", ", 2, NULL))
					return error;
				}

			//	Output the value

			if (error = Output.Write(sValue.GetPointer(), sValue.GetLength(), NULL))
				return error;

			bCommaNeeded = TRUE;
			}

		bBreakNeeded = TRUE;
		iFieldCount++;
		}

	//	Done

	if (error = Output.Close())
		return error;

	*retsFieldSection = CString(Output.GetPointer(), Output.GetLength());
	*retiFieldCount = iFieldCount;

	return NOERROR;
	}

void CPageViewer::JustifyFrame (HDC hDC, CRTFrame *pFrame, Twips TwipsPerPixel, RECT &rcRect)

//	JustifyFrame
//
//	Justify this frame using the given DC and scale

	{
	HDC hTempDC = NULL;
	RECT rcFrameRect;

	//	Create a DC if necessary

	if (hDC == NULL)
		{
		hTempDC = CreateCompatibleDC(NULL);
		hDC = hTempDC;
		}

	//	Justify the frame

	rcFrameRect = rcRect;
	pFrame->Justify(hDC, &rcFrameRect, TwipsPerPixel);

	//	Done

	if (hTempDC)
		DeleteDC(hTempDC);
	}

BOOL CPageViewer::KeyDown (int iVirtKey, DWORD dwKeyFlags)

//	KeyDown
//
//	Handle key down

	{
	switch (iVirtKey)
		{
		case VK_UP:
			{
			VertScroll(4);
			return TRUE;
			}

		case VK_DOWN:
			{
			VertScroll(-4);
			return TRUE;
			}

		case VK_PRIOR:
			{
			VertScroll(RectHeight(&m_rcBody) - 64);
			return TRUE;
			}

		case VK_NEXT:
			{
			VertScroll(-(RectHeight(&m_rcBody) - 64));
			return TRUE;
			}

		default:
			return FALSE;
		}
	}

void CPageViewer::MouseMove (POINT ptPoint, DWORD dwFlags)

//	MouseMove
//
//	Handle mouse move

	{
	}

void CPageViewer::Paint (HDC hDC, RECT *pUpdateRect)

//	Paint
//
//	Paint

	{
	RECT rcRect;

	//	Paint the backgrounds

	gdiFillRect(hDC, &m_rcPage, RGB_PAGE_COLOR);
	gdiFillRect(hDC, &m_rcTextBlock, RGB_TEXTBLOCK_COLOR);
	gdiFillRect(hDC, &m_rcNotes, RGB_TEXTBLOCK_COLOR);

	//	Paint the title

	if (m_pTitle)
		m_pTitle->Paint(hDC, m_rcTitle.left, m_rcTitle.top, *pUpdateRect);

	//	Paint the body

	if (m_pBody)
		{
		HRGN hOldRgn;

		IntersectRect(&rcRect, &m_rcBody, pUpdateRect);

		hOldRgn = m_pFrame->BeginClip(hDC, rcRect);
		m_pBody->Paint(hDC, m_rcBody.left, m_rcBody.top - m_cyScroll, rcRect);
		m_pFrame->EndClip(hDC, hOldRgn);
		}

	//	Paint the field information at the bottom

	if (m_pFields)
		{
		RECT rcLine;

		//	Paint a line above the fields

		rcLine.left = m_rcFullBody.left;
		rcLine.right = rcLine.left + 80;
		rcLine.top = m_rcBody.bottom + FIELD_SEPARATOR_HEIGHT / 2;
		rcLine.bottom = rcLine.top + 1;
		gdiFillRect(hDC, &rcLine, RGB(96,96,96));

		//	Paint the fields

		m_pFields->Paint(hDC, 
				m_rcBody.left, 
				m_rcBody.bottom + FIELD_SEPARATOR_HEIGHT,
				*pUpdateRect);
		}

	//	Paint the related links

	if (m_pRelatedLinks)
		{
		m_pRelatedLinks->Paint(hDC, 
				m_rcRelatedLinks.left, 
				m_rcRelatedLinks.top,
				*pUpdateRect);
		}
	}

void CPageViewer::Resize (RECT *pRect)

//	Resize
//
//	Worry about resize

	{
	SetRect(pRect);
	}

ALERROR CPageViewer::SetPage (CEntry *pEntry)

//	SetPage
//
//	Set the page to view. Note that the caller owns the entry.

	{
	ALERROR error;
	CRTFormatter *pNewFormat = NULL;
	CRTFrame *pNewBody = NULL;
	CRTFrame *pNewTitle = NULL;
	CRTFrame *pNewFields = NULL;
	CRTFrame *pNewRelatedLinks = NULL;

	//	Format the entry

	if (pEntry)
		{
		CString sFieldSection;
		int iFieldCount;

		//	If we don't yet have a formatter, create a new one.
		//	For now we share the same formatter for all entries.

		if (m_pFormat)
			pNewFormat = m_pFormat;
		else
			{
			pNewFormat = new CRTFormatter(pEntry->GetCore());
			if (pNewFormat == NULL)
				return ERR_MEMORY;

			if (error = pNewFormat->Boot())
				{
				delete pNewFormat;
				return error;
				}

			pNewFormat->SetScale(m_TwipsPerPixel);
			}

		//	Format the title

		if (error = pNewFormat->FormatFrame(m_rcTitle,
				pEntry->GetTitle(), 
				LITERAL("Title"),
				LITERAL("Title"),
				&pNewTitle))
			return error;

		JustifyFrame(NULL, pNewTitle, m_TwipsPerPixel, m_rcTitle);

		//	Format the fields

		if (error = GenerateFieldSection(pEntry, &sFieldSection, &iFieldCount))
			return error;

		if (iFieldCount > 0)
			{
			if (error = pNewFormat->FormatFrame(m_rcBody,
					sFieldSection,
					LITERAL("FieldData"),
					LITERAL("FieldData"),
					&pNewFields))
				return error;

			JustifyFrame(NULL, pNewFields, m_TwipsPerPixel, m_rcBody);
			}
		else
			pNewFields = NULL;

		//	Format the body (The body gets less space if there are
		//	fields in this entry)

		m_rcBody = m_rcFullBody;
		if (pNewFields)
			m_rcBody.bottom -= pNewFields->GetHeight() + FIELD_SEPARATOR_HEIGHT;

		if (error = pNewFormat->FormatFrame(m_rcBody,
				pEntry->GetBody(),
				LITERAL("Body"),
				LITERAL("Body"),
				&pNewBody))
			return error;

		JustifyFrame(NULL, pNewBody, m_TwipsPerPixel, m_rcBody);

		//	Format the related links

		if (error = pNewFormat->FormatFrame(m_rcRelatedLinks,
				pEntry->GetRelatedLinks(), 
				LITERAL("Body"),
				LITERAL("Body"),
				&pNewRelatedLinks))
			return error;

		JustifyFrame(NULL, pNewRelatedLinks, m_TwipsPerPixel, m_rcRelatedLinks);
		}

	//	Replace the page

	if (m_pEntry)
		{
		delete m_pTitle;
		delete m_pBody;
		delete m_pFields;
		delete m_pRelatedLinks;

		//	Sometimes we reuse the same format

		if (m_pFormat != pNewFormat)
			delete m_pFormat;
		}

	m_pEntry = pEntry;
	m_pFormat = pNewFormat;
	m_pTitle = pNewTitle;
	m_pBody = pNewBody;
	m_pFields = pNewFields;
	m_pRelatedLinks = pNewRelatedLinks;
	m_cyScroll = 0;

	m_pFrame->UpdateRect(&m_rcRect);

	return NOERROR;
	}

void CPageViewer::SetRect (RECT *pRect)

//	SetRect
//
//	Sets a new rect for the viewer and recalculates all metrics

	{
	int iFullWidth, iFullHeight, iPageWidth, iPageHeight;

	m_rcRect = *pRect;

	//	Obtain a RECT that is centered on the full RECT, is
	//	a large as possible, and has a ratio of 1:sqrt(2)

	iFullWidth = RectWidth(pRect);
	iFullHeight = RectHeight(pRect);

	//	If the height is greater than the width then
	//	we have a portrait orientation. Otherwise we have
	//	landscape orientation

	if (iFullHeight > iFullWidth)
		{
		m_fLandscape = FALSE;

		if (iFullHeight > MulDiv(iFullWidth, PORTRAIT_PAGE_RATIO, 1000))
			{
			iPageWidth = iFullWidth;
			iPageHeight = MulDiv(iPageWidth, PORTRAIT_PAGE_RATIO, 1000);
			}
		else
			{
			iPageHeight = iFullHeight;
			iPageWidth = MulDiv(iPageHeight, 1000, PORTRAIT_PAGE_RATIO);
			}

		}
	else
		{
		m_fLandscape = TRUE;

		if (iFullWidth > MulDiv(iFullHeight, LANDSCAPE_PAGE_RATIO, 1000))
			{
			iPageHeight = iFullHeight;
			iPageWidth = MulDiv(iPageHeight, LANDSCAPE_PAGE_RATIO, 1000);
			}
		else
			{
			iPageWidth = iFullWidth;
			iPageHeight = MulDiv(iPageWidth, 1000, LANDSCAPE_PAGE_RATIO);
			}
		}

	m_rcPage.left = m_rcRect.left + (iFullWidth - iPageWidth) / 2;
	m_rcPage.top = m_rcRect.top + (iFullHeight - iPageHeight) / 2;
	m_rcPage.right = m_rcPage.left + iPageWidth;
	m_rcPage.bottom = m_rcPage.top + iPageHeight;

	//	Depending on the orientation we do different layouts

	if (m_fLandscape)
		{
		int iTopMargin, iLeftMargin, iRightMargin, iBottomMargin;
		int cxSpacing;

		//	Figure out the margins

		iLeftMargin = iPageHeight / 12;
		iRightMargin = iLeftMargin;
		iTopMargin = iLeftMargin;
		iBottomMargin = MulDiv(iTopMargin, PORTRAIT_PAGE_RATIO, 1000);

		//	Figure out the size of the text block

		m_rcTextBlock.top = m_rcPage.top + iTopMargin;
		m_rcTextBlock.bottom = m_rcPage.bottom - iBottomMargin;
		m_rcTextBlock.left = m_rcPage.left + iLeftMargin;
		m_rcTextBlock.right = m_rcTextBlock.left + MulDiv(RectHeight(&m_rcTextBlock), LANDSCAPE_TEXTBLOCK_RATIO, 1000);

		//	Figure out the dimensions of the notes block

		cxSpacing = iTopMargin / 2;

		m_rcNotes.top = m_rcPage.top + iTopMargin;
		m_rcNotes.bottom = m_rcPage.bottom - iBottomMargin;
		m_rcNotes.left = m_rcTextBlock.right + cxSpacing;
		m_rcNotes.right = m_rcPage.right - iRightMargin;
		}
	else
		{
		int iTopMargin, iLeftMargin, iRightMargin, iBottomMargin;
		int cySpacing;

		//	Figure out the margins

		iLeftMargin = iPageWidth / 12;
		iRightMargin = iLeftMargin;
		iTopMargin = iLeftMargin;
		iBottomMargin = MulDiv(iTopMargin, PORTRAIT_PAGE_RATIO, 1000);

		//	Figure out the size of the text block

		m_rcTextBlock.left = m_rcPage.left + iLeftMargin;
		m_rcTextBlock.right = m_rcPage.right - iRightMargin;
		m_rcTextBlock.top = m_rcPage.top + iTopMargin;
		m_rcTextBlock.bottom = MulDiv(RectWidth(&m_rcTextBlock), PORTRAIT_TEXTBLOCK_RATIO, 1000);

		//	Figure out the dimensions of the notes block

		cySpacing = iTopMargin / 2;

		m_rcNotes.top = m_rcTextBlock.bottom + cySpacing;
		m_rcNotes.bottom = m_rcPage.bottom - iBottomMargin;
		m_rcNotes.left = m_rcTextBlock.left;
		m_rcNotes.right = m_rcTextBlock.right;
		}

	//	Figure out the position of the title and the body

	m_rcTitle = m_rcTextBlock;
	m_rcTitle.bottom = m_rcTitle.top + 64;

	m_rcFullBody = m_rcTextBlock;
	m_rcFullBody.top = m_rcTitle.bottom + 8;

	//	For now the body is the whole rect

	m_rcBody = m_rcFullBody;

	//	The related links is the entire notes section

	m_rcRelatedLinks = m_rcNotes;

	//	Calculate Twips per pixel

	m_TwipsPerPixel = CRTFormatter::CalcTwipsPerScreenPixel();

	//	Repaint

	SetPage(m_pEntry);
	}

void CPageViewer::VertScroll (int iAmount)

//	VertScroll
//
//	Vertical scroll

	{
	//	Bail out if we have no body

	if (m_pBody == NULL)
		return;

	//	Calculate how much to scroll

	if (iAmount > 0)
		iAmount = min(iAmount, m_cyScroll);
	else
		{
		int iMaxScroll = m_pBody->GetHeight() - RectHeight(&m_rcBody);
		if (iMaxScroll > m_cyScroll)
			iAmount = -min(-iAmount, iMaxScroll - m_cyScroll);
		else
			iAmount = 0;
		}

	//	Scroll

	if (iAmount != 0)
		{
		RECT rcUpdate;

		ScrollWindowEx(m_pFrame->GetWindow()->GetHWND(),
				0,
				iAmount,
				&m_rcBody,
				&m_rcBody,
				NULL,
				&rcUpdate,
				SW_INVALIDATE);

		m_cyScroll -= iAmount;
		}
	}

