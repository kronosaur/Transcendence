//	CDetailList.cpp
//
//	CDetailList class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define FIELD_DESC							CONSTLIT("desc")
#define FIELD_DETAILS						CONSTLIT("details")
#define FIELD_ICON							CONSTLIT("icon")
#define FIELD_LARGE_ICON					CONSTLIT("largeIcon")
#define FIELD_TITLE							CONSTLIT("title")

void CDetailList::Format (const RECT &rcRect, int *retcyHeight)

//	Format
//
//	Initializes rcRect for each detail entry based on the overall rect for the
//	content pane.

	{
	int i;
	const CG16bitFont &Medium = m_VI.GetFont(fontMedium);
	const CG16bitFont &MediumBold = m_VI.GetFont(fontMediumBold);

	//	Short-circuit

	if (m_List.GetCount() == 0)
		{
		if (retcyHeight) *retcyHeight = 0;
		return;
		}

	//	We arrange the details along two columns, aligned to the bottom.

	int cxPane = RectWidth(rcRect);
	int cxColumn = (cxPane - (3 * SPACING_X)) / 2;

	RECT rcColumn;
	rcColumn.left = 0;
	rcColumn.right = cxColumn;
	rcColumn.top = 0;
	rcColumn.bottom = RectHeight(rcRect);

	//	The text area is smaller because of the icon.

	RECT rcText = rcColumn;
	rcText.right -= SPACING_X + DETAIL_ICON_WIDTH;

	//	First we measure the height of each detail entry.

	for (i = 0; i < m_List.GetCount(); i++)
		{
		int cyDetails = (!m_List[i].sTitle.IsBlank() ? MediumBold.GetHeight() : 0);

		//	Compute height of description

		RECT rcBounds;
		m_List[i].Desc.GetBounds(rcText, &rcBounds);
		cyDetails += RectHeight(rcBounds);

		//	Remember the text height separately.

		m_List[i].cyText = cyDetails;

		//	If the icon is larger, then we use that height.

		m_List[i].cyRect = Max(cyDetails, DETAIL_ICON_HEIGHT);
		}

	//	Figure out where we break the columns. iBreakAt is the index of the 
	//	first detail entry in column 2.

	int iBreakAt = (m_List.GetCount() + 1) / 2;

	//	Add up the total height of each column. NOTE: For this calculationg we
	//	ignore any spacing between entries.

	int cyCol1 = 0;
	for (i = 0; i < iBreakAt; i++)
		cyCol1 += m_List[i].cyRect;

	int cyCol2 = 0;
	for (i = iBreakAt; i < m_List.GetCount(); i++)
		cyCol2 += m_List[i].cyRect;

	int cyMax = Max(cyCol1, cyCol2);

	//	See if we can fit better if we move the first entry from column 2 to
	//	column 1.

	if (m_List.GetCount() > 2)
		{
		while (iBreakAt < m_List.GetCount() - 1
				&& Max(cyCol1 + m_List[iBreakAt].cyRect, cyCol2 - m_List[iBreakAt].cyRect) < cyMax)
			{
			cyCol1 += m_List[iBreakAt].cyRect;
			cyCol2 -= m_List[iBreakAt].cyRect;
			cyMax = Max(cyCol1, cyCol2);
			iBreakAt++;
			}

		//	See if we can fit better if we move the last entry from column 1 to
		//	column 2.

		while (iBreakAt > 0
				&& Max(cyCol1 - m_List[iBreakAt - 1].cyRect, cyCol2 + m_List[iBreakAt - 1].cyRect) < cyMax)
			{
			cyCol1 -= m_List[iBreakAt - 1].cyRect;
			cyCol2 += m_List[iBreakAt - 1].cyRect;
			cyMax = Max(cyCol1, cyCol2);
			iBreakAt--;
			}
		}

	//	Now compute column metrics including spacing.

	cyCol1 += (iBreakAt * SPACING_Y) + SPACING_Y;
	cyCol2 += ((m_List.GetCount() - iBreakAt) * SPACING_Y) + SPACING_Y;
	cyMax = Max(cyCol1, cyCol2);

	//	Now format all entries. Start with the first column.

	int yCol1 = rcRect.bottom - cyCol1;
	for (i = 0; i < iBreakAt; i++)
		{
		m_List[i].bAlignRight = false;
		m_List[i].Desc.SetAlignment(alignLeft);

		m_List[i].rcRect.left = rcRect.left + SPACING_X;
		m_List[i].rcRect.right = rcRect.left + SPACING_X + cxColumn;
		m_List[i].rcRect.top = yCol1;
		m_List[i].rcRect.bottom = yCol1 + m_List[i].cyRect;

		yCol1 += m_List[i].cyRect + SPACING_Y;
		}

	//	Now format the entries in the second column.

	int yCol2 = rcRect.bottom - cyCol2;
	for (i = iBreakAt; i < m_List.GetCount(); i++)
		{
		m_List[i].bAlignRight = true;
		m_List[i].Desc.SetAlignment(alignRight);

		m_List[i].rcRect.left = rcRect.right - SPACING_X - cxColumn;
		m_List[i].rcRect.right = rcRect.right - SPACING_X;
		m_List[i].rcRect.top = yCol2;
		m_List[i].rcRect.bottom = yCol2 + m_List[i].cyRect;

		yCol2 += m_List[i].cyRect + SPACING_Y;
		}

	//	Return the max column height.

	if (retcyHeight)
		*retcyHeight = cyMax;
	}

void CDetailList::Load (ICCItem *pDetails)

//	Load
//
//	Load details from an TLisp item to an array.

	{
	int i;

	m_List.DeleteAll();

	if (pDetails == NULL || !pDetails->IsList())
		return;

	for (i = 0; i < pDetails->GetCount(); i++)
		{
		ICCItem *pEntry = pDetails->GetElement(i);
		if (!pEntry->IsSymbolTable()
				|| pEntry->IsNil())
			continue;

		SDetailEntry *pNewEntry = m_List.Insert();
		pNewEntry->sTitle = pEntry->GetStringAt(FIELD_TITLE);

		//	Load description, which could be RTF

		pNewEntry->Desc.SetFontTable(&m_VI);
		pNewEntry->Desc.SetDefaultFont(&m_VI.GetFont(fontMedium));
		pNewEntry->Desc.SetText(pEntry->GetStringAt(FIELD_DESC));

		//	Load image

		ICCItem *pIcon = pEntry->GetElement(FIELD_ICON);
		if (pIcon)
			{
			DWORD dwIcon = CTLispConvert::AsImageDesc(pIcon, &pNewEntry->rcIcon);
			if (dwIcon)
				pNewEntry->pIcon = g_pUniverse->GetLibraryBitmap(dwIcon); 
			}
		}
	}

void CDetailList::Paint (CG32bitImage &Dest) const

//	PaintDetails
//
//	Paint all detail entries. We must have already called FormatDetails.

	{
	int i;
	const CG16bitFont &Medium = m_VI.GetFont(fontMedium);
	const CG16bitFont &MediumBold = m_VI.GetFont(fontMediumBold);

	for (i = 0; i < m_List.GetCount(); i++)
		{
		const SDetailEntry &Entry = m_List[i];

		//	Paint the icon

		if (Entry.pIcon)
			{
			if (Entry.bAlignRight)
				CPaintHelper::PaintScaledImage(Dest, Entry.rcRect.right - DETAIL_ICON_WIDTH, Entry.rcRect.top, DETAIL_ICON_WIDTH, DETAIL_ICON_HEIGHT, *Entry.pIcon, Entry.rcIcon);
			else
				CPaintHelper::PaintScaledImage(Dest, Entry.rcRect.left, Entry.rcRect.top, DETAIL_ICON_WIDTH, DETAIL_ICON_HEIGHT, *Entry.pIcon, Entry.rcIcon);
			}

		//	Paint text

		int yText = Entry.rcRect.top + (Entry.cyRect - Entry.cyText) / 2;
		DWORD dwTextFlags = (Entry.bAlignRight ? CG16bitFont::AlignRight : 0);

		//	Paint the title

		if (!Entry.sTitle.IsBlank())
			{
			int xText = (Entry.bAlignRight ? Entry.rcRect.right - DETAIL_ICON_WIDTH - SPACING_X : Entry.rcRect.left + DETAIL_ICON_WIDTH + SPACING_X);
			MediumBold.DrawText(Dest, xText, yText, m_rgbTextColor, Entry.sTitle, dwTextFlags);
			yText += MediumBold.GetHeight();
			}

		//	Paint the description

		RECT rcDesc;
		if (Entry.bAlignRight)
			{
			rcDesc.left = Entry.rcRect.left;
			rcDesc.right = Entry.rcRect.right - DETAIL_ICON_WIDTH - SPACING_X;
			}
		else
			{
			rcDesc.left = Entry.rcRect.left + DETAIL_ICON_WIDTH + SPACING_X;
			rcDesc.right = Entry.rcRect.right;
			}
		rcDesc.top = yText;
		rcDesc.bottom = Entry.rcRect.bottom;

		Entry.Desc.Paint(Dest, rcDesc);
		}
	}
