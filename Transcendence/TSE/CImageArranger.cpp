//	CImageArranger.cpp
//
//	CImageArranger class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

CImageArranger::CImageArranger (void)

//	CImageArranger constructor

	{
	}

void CImageArranger::AddMargin (int cx, int cy)

//	AddMargin
//
//	Adds to the margin

	{
	m_cxMaxWidth += cx;
	m_cyMaxHeight += cy;
	}

void CImageArranger::ArrangeByRow (const SArrangeDesc &Desc, const TArray<SCellDesc> &Cells)

//	ArrangeByRow
//
//	Arranges the given list of cells

	{
	Init(Cells.GetCount());

	int iNext = 0;
	int y = 0;
	int cyHeader = Desc.pHeader->GetHeight();

	int cxInternalSpacing = 8;
	int cyInternalSpacing = 2 * Desc.pHeader->GetHeight();
	int cyNameSpacing = Desc.pHeader->GetHeight() / 2;

	while (iNext < Cells.GetCount())
		{
		int i;
		int cxWidthLeft = Desc.cxDesiredWidth;
		int cyRowHeight = 0;
		int iStart = iNext;

		//	First figure out how many cells will fit
		
		while (iNext < Cells.GetCount())
			{
			int cxSize = Cells[iNext].cxWidth;
			int cySize = Cells[iNext].cyHeight;
			if (cxSize > cxWidthLeft && iStart != iNext)
				break;

			int cxCell = Max(cxSize + cxInternalSpacing, Desc.cxSpacing);

			cxWidthLeft -= cxCell;
			if (cySize > cyRowHeight)
				cyRowHeight = cySize;

			iNext++;
			}

		//	Compute the total width

		int cxRowWidth = Min(Desc.cxDesiredWidth - cxWidthLeft, Desc.cxDesiredWidth);
		int xOffset = (Desc.cxDesiredWidth - cxRowWidth) / 2;

		//	See if any of the cells overlap the text from the previous cells
		//	If so, we increase y a little bit

		int x = Desc.cxSpacing + Desc.cxExtraMargin;
		for (i = iStart; i < iNext; i++)
			{
			int cxSize = Cells[i].cxWidth;
			int yOffset = (cyRowHeight - cxSize) / 2;

			int xPoint = x + xOffset + cxSize / 2;
			int yPoint = y + yOffset;

			for (int j = 0; j < iStart; j++)
				{
				int xText = GetTextX(j);
				int yText = GetTextY(j);
				int cxText = GetTextWidth(j);
				int cyText = GetTextHeight(j) + cyInternalSpacing;

				if (xPoint >= xText && xPoint < xText + cxText && yPoint < yText + cyText)
					y = yText + cyText + cyInternalSpacing;
				}

			int cxCell = Max(cxSize + cxInternalSpacing, Desc.cxSpacing);
			x += cxCell;
			}

		//	Place the cells

		x = Desc.cxSpacing + Desc.cxExtraMargin;
		int yOverlapOffset = 0;
		for (i = iStart; i < iNext; i++)
			{
			int cxSize = Cells[i].cxWidth;
			int cySize = Cells[i].cyHeight;
			int cxCell = Max(cxSize + cxInternalSpacing, Desc.cxSpacing);
			int cyCell = Max(cySize + cyInternalSpacing, Desc.cxSpacing);

			//	Center vertically

			int yOffset = (cyRowHeight - cySize) / 2;

			//	Place

			Place(i, x + xOffset, y + yOffset, cxSize, cySize);

			//	Figure out the position of the text

			int cyName;
			int cxName = Desc.pHeader->MeasureText(Cells[i].sText, &cyName);
			if (cxName <= cxSize)
				{
				int yText = y + yOffset + cySize + cyNameSpacing;
				PlaceText(i,
						x + xOffset + (cxSize - cxName) / 2,
						yText,
						cxName,
						cyName);

				yOverlapOffset = yText + cyName + cyNameSpacing;
				}
			else
				{
				//	See if the name fits above the previous name

				int yText;
				if (yOverlapOffset > 0 
						&& i > iStart 
						&& (x + xOffset + cxSize - (GetX(i - 1) + GetWidth(i - 1) / 2)) > cxName + 8)
					{
					yText = y + yOffset + cySize + cyNameSpacing;
					if (yText < GetY(i - 1) + GetHeight(i - 1))
						yText = GetY(i - 1) + GetHeight(i - 1) + cyNameSpacing;

					if (yText > GetTextY(i - 1) - (cyName + cyNameSpacing))
						yText = yOverlapOffset;
					}

				//	Otherwise, use the overlap, if it exits

				else if (yOverlapOffset == 0)
					yText = y + yOffset + cySize + cyNameSpacing;
				else
					yText = yOverlapOffset;

				PlaceText(i,
						x + xOffset + cxSize - cxName,
						yText,
						cxName,
						cyName);

				yOverlapOffset = yText + cyName + cyNameSpacing;
				}

			x += cxCell;
			}

		y += cyRowHeight;
		}

	//	Add a margin to the right and bottom

	AddMargin(Desc.cxSpacing + Desc.cxExtraMargin, cyInternalSpacing);
	}

void CImageArranger::Init (int iCells)

//	Init
//
//	Initialize

	{
	int i;

	m_Map.DeleteAll();
	m_Map.InsertEmpty(iCells);
	for (i = 0; i < iCells; i++)
		m_Map[i].x = -1;

	m_cxMaxWidth = 0;
	m_cyMaxHeight = 0;
	}

void CImageArranger::Place (int iIndex, int x, int y, int cxWidth, int cyHeight)

//	Place
//
//	Place the given image at the given position and size

	{
	SEntry *pEntry = &m_Map[iIndex];
	pEntry->x = x;
	pEntry->y = y;
	pEntry->cxWidth = cxWidth;
	pEntry->cyHeight = cyHeight;

	pEntry->xText = -1;

	m_cxMaxWidth = Max(m_cxMaxWidth, x + cxWidth);
	m_cyMaxHeight = Max(m_cyMaxHeight, y + cyHeight);
	}

void CImageArranger::PlaceText (int iIndex, int x, int y, int cxWidth, int cyHeight)

//	PlaceText
//
//	Places text at the given position and size

	{
	SEntry *pEntry = &m_Map[iIndex];
	pEntry->xText = x;
	pEntry->yText = y;
	pEntry->cxText = cxWidth;
	pEntry->cyText = cyHeight;

	m_cxMaxWidth = Max(m_cxMaxWidth, x + cxWidth);
	m_cyMaxHeight = Max(m_cyMaxHeight, y + cyHeight);
	}
