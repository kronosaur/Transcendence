//	CImageGrid.cpp
//
//	CImageGrid class

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

void CImageGrid::Create (int iCount, int cxCellWidth, int cyCellHeight)

//	Create
//
//	Create the grid

	{
	ASSERT(iCount > 0);

	m_iCount = iCount;
	m_cxCellWidth = cxCellWidth;
	m_cyCellHeight = cyCellHeight;

	//	Make a square out of all the images.

	m_iCellColumns = mathSqrt(iCount);
	if (m_iCellColumns * m_iCellColumns < iCount)
		m_iCellColumns++;

	m_iCellRows = AlignUp(iCount, m_iCellColumns) / m_iCellColumns;

	//	Compute the size of the image

	int cxWidth = m_iCellColumns * cxCellWidth;
	int cyHeight = m_iCellRows * cyCellHeight;

	m_Image.Create(cxWidth, cyHeight);
	}

void CImageGrid::GetCellCenter (int iIndex, int *retx, int *rety)

//	GetCellCenter
//
//	Return the coordinates of the center of the given cell.

	{
	int iRow = iIndex / m_iCellColumns;
	int iCol = iIndex % m_iCellColumns;

	*retx = (iCol * m_cxCellWidth) + m_cxCellWidth / 2;
	*rety = (iRow * m_cyCellHeight) + m_cyCellHeight / 2;
	}
