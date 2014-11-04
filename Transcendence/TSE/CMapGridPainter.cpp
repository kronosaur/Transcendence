//	CMapGridPainter.cpp
//
//	CMapGridPainter class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

const Metric GRID_SIZE =								100.0 * LIGHT_SECOND;
const WORD RGB_GRID_LINE =								CG16bitImage::RGBValue(43, 45, 51);

void CMapGridPainter::AddLines (const TArray<SLine> &NewLines, TArray<SLine> *retLines)

//	AddLines
//
//	Adds lines to the array, handling overlap.

	{
	int i;

	//	Optimize edge condition

	if (retLines->GetCount() == 0)
		{
		*retLines = NewLines;
		return;
		}

	//	Loop over all new lines and see if any overlap with the existing ones.

	for (i = 0; i < NewLines.GetCount(); i++)
		{
		int iPos;
		if (FindKey(*retLines, NewLines[i].xyKey, &iPos))
			{
			SLine &Original = retLines->GetAt(iPos);

			//	NewLine:		o--------
			//	Original:				   o--------o

			if (NewLines[i].xyFrom < Original.xyFrom)
				{
				//	NewLine:		o-------o
				//	Original:				   o--------o
				//
				//	Disjoint, so we need to add the new line.

				if (NewLines[i].xyTo < Original.xyFrom)
					{
					retLines->Insert(NewLines[i]);
					}

				//	NewLine:		o-----------------o
				//	Original:			o--------o
				//
				//	New line is a superset, so we take its parameters

				else if (NewLines[i].xyTo > Original.xyTo)
					{
					Original.xyFrom = NewLines[i].xyFrom;
					Original.xyTo = NewLines[i].xyTo;
					}

				//	NewLine:		o-------o
				//	Original:			o--------o
				//
				//	Overlap, so take the new line's xyFrom

				else
					{
					Original.xyFrom = NewLines[i].xyFrom;
					}
				}

			//	NewLine:					o--------
			//	Original:		o--------o
			//
			//	Always disjoint, so add the new line

			else if (NewLines[i].xyFrom > Original.xyTo)
				{
				retLines->Insert(NewLines[i]);
				}

			//	NewLine:			o--------
			//	Original:		o--------o

			else
				{
				//	NewLine:		o-------o
				//	Original:	o--------o
				//
				//	Overlap, so take the new line's xyTo

				if (NewLines[i].xyTo > Original.xyTo)
					{
					Original.xyTo = NewLines[i].xyTo;
					}

				//	NewLine:		o---o
				//	Original:	o--------o
				//
				//	New line is a subset, so we can ignore it

				else
					{
					NULL;
					}
				}
			}

		//	If not found, just add the new line

		else
			retLines->Insert(NewLines[i]);
		}
	}

void CMapGridPainter::AddRegion (const CVector &vCenter, Metric rWidth, Metric rHeight)

//	AddRegion
//
//	Adds a rectangular region to paint. We combine this appropriately with any
//	previously added regions.

	{
	int x, y;

	Metric rHalfWidth = 0.5 * rWidth;
	Metric rHalfHeight = 0.5 * rHeight;

	int xFrom = (int)floor((vCenter.GetX() - rHalfWidth) / GRID_SIZE);
	int xTo = (int)floor((vCenter.GetX() + rHalfWidth) / GRID_SIZE);
	int yFrom = (int)floor((vCenter.GetY() - rHalfHeight) / GRID_SIZE);
	int yTo = (int)floor((vCenter.GetY() + rHalfHeight) / GRID_SIZE);

	//	Null case

	if (xFrom == xTo || yFrom == yTo)
		return;

	//	Start with vertical lines

	TArray<SLine> NewLines;
	for (x = xFrom; x <= xTo; x++)
		{
		SLine *pNewLine = NewLines.Insert();
		pNewLine->xyKey = x;
		pNewLine->xyFrom = yFrom;
		pNewLine->xyTo = yTo;
		}

	AddLines(NewLines, &m_VertLines);

	//	Add horizontal lines

	NewLines.DeleteAll();
	for (y = yFrom; y <= yTo; y++)
		{
		SLine *pNewLine = NewLines.Insert();
		pNewLine->xyKey = y;
		pNewLine->xyFrom = xFrom;
		pNewLine->xyTo = xTo;
		}

	AddLines(NewLines, &m_HorzLines);

	m_bRecalcNeeded = true;
	}

bool CMapGridPainter::FindKey (const TArray<SLine> &Lines, int xyKey, int *retiIndex)

//	FindKey
//
//	Finds the given key in the array of lines.

	{
	int i;

	for (i = 0; i < Lines.GetCount(); i++)
		if (Lines[i].xyKey == xyKey)
			{
			if (retiIndex)
				*retiIndex =i;

			return true;
			}

	return false;
	}

void CMapGridPainter::Paint (CG16bitImage &Dest, CMapViewportCtx &PaintCtx)

//	Paint
//
//	Paints the grid

	{
	if (IsEmpty())
		return;

	if (m_bRecalcNeeded)
		{
		RecalcGrid();
		m_bRecalcNeeded = false;
		}

	Paint(Dest, PaintCtx, m_HorzLines);
	Paint(Dest, PaintCtx, m_VertLines);
	}

void CMapGridPainter::Paint (CG16bitImage &Dest, CMapViewportCtx &PaintCtx, const TArray<SLine> &Lines)

//	Paint
//
//	Paint the array of lines

	{
	int i;

	for (i = 0; i < Lines.GetCount(); i++)
		{
		int xFrom, yFrom;
		PaintCtx.Transform(Lines[i].vFrom, &xFrom, &yFrom);

		int xTo, yTo;
		PaintCtx.Transform(Lines[i].vTo, &xTo, &yTo);

		Dest.DrawLine(xFrom, yFrom, xTo, yTo, 1, RGB_GRID_LINE);
		}
	}

void CMapGridPainter::RecalcGrid (void)

//	RecalcGrid
//
//	Compute the grid points

	{
	int i;

	//	Vertical lines

	for (i = 0; i < m_VertLines.GetCount(); i++)
		{
		Metric rX = m_VertLines[i].xyKey * GRID_SIZE;
		m_VertLines[i].vFrom = CVector(rX, m_VertLines[i].xyFrom * GRID_SIZE);
		m_VertLines[i].vTo = CVector(rX, m_VertLines[i].xyTo * GRID_SIZE);
		}

	//	Horizontal lines

	for (i = 0; i < m_HorzLines.GetCount(); i++)
		{
		Metric rY = m_HorzLines[i].xyKey * GRID_SIZE;
		m_HorzLines[i].vFrom = CVector(m_HorzLines[i].xyFrom * GRID_SIZE, rY);
		m_HorzLines[i].vTo = CVector(m_HorzLines[i].xyTo * GRID_SIZE, rY);
		}
	}
