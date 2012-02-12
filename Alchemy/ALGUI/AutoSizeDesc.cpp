//	AutoSizeDesc.cpp
//
//	AutoSizeDesc object

#include "Alchemy.h"
#include "ALGUI.h"

AutoSizeDesc::AutoSizeDesc (void)

//	AutoSizeDesc constructor

	{
	}

void AutoSizeDesc::CalcAxis (int iParentMin, 
							 int iParentMax, 
							 int iMinBoundary,
							 int iMaxBoundary,
							 int iDim,
							 AlignmentTypes Alignment,
							 LONG *retiMin, 
							 LONG *retiMax)

//	CalcAxis
//
//	Calculate the dimensions of the axis

	{
	switch (Alignment)
		{
		//	The lower part is variable; the dimension is fixed

		case MinVariable:
			{
			*retiMax = iParentMax - iMaxBoundary;
			*retiMin = *retiMax - iDim;
			break;
			}

		//	The upper part is variable; the dimension is fixed

		case MaxVariable:
			{
			*retiMin = iParentMin + iMinBoundary;
			*retiMax = *retiMin + iDim;
			break;
			}

		//	The dimension is variable; the limits are fixed

		case DimVariable:
			{
			*retiMin = iParentMin + iMinBoundary;
			*retiMax = iParentMax - iMaxBoundary;
			break;
			}

		//	Both limits are variable; the dimension is fixed

		case Centered:
			{
			*retiMin = iParentMin + ((iParentMax - iParentMin) - iDim) / 2;
			*retiMax = iParentMax = *retiMin + iDim;
			break;
			}

		default:
			ASSERT(FALSE);
		}
	}

void AutoSizeDesc::CalcRect (RECT *pParentRect, RECT *retrcRect)

//	CalcRect
//
//	Returns the rect calculated from the parent rect and based
//	on the autosize settings

	{
	//	Calc each axis

	CalcAxis(pParentRect->left,
			pParentRect->right, 
			m_rcBoundaries.left,
			m_rcBoundaries.right,
			m_cxWidth,
			m_HorzAlignment,
			&retrcRect->left, 
			&retrcRect->right);
	CalcAxis(pParentRect->top,
			pParentRect->bottom,
			m_rcBoundaries.top,
			m_rcBoundaries.bottom,
			m_cyHeight,
			m_VertAlignment,
			&retrcRect->top,
			&retrcRect->bottom);
	}

void AutoSizeDesc::SetAutoSize (int cxWidth, int cyHeight,
								int iLeftMargin, int iTopMargin, int iRightMargin, int iBottomMargin,
								AlignmentTypes iHorzAlignment, AlignmentTypes iVertAlignment)

//	SetAutoSize
//
//	Sets the autosize parameters

	{
	m_rcBoundaries.left = iLeftMargin;
	m_rcBoundaries.top = iTopMargin;
	m_rcBoundaries.right = iRightMargin;
	m_rcBoundaries.bottom = iBottomMargin;
	m_cxWidth = cxWidth;
	m_cyHeight = cyHeight;
	m_HorzAlignment = iHorzAlignment;
	m_VertAlignment = iVertAlignment;
	}

void AutoSizeDesc::SetCentered (int cxWidth, int cyHeight)

//	SetCentered
//
//	Set centered on screen

	{
	m_rcBoundaries.left = 0;
	m_rcBoundaries.top = 0;
	m_rcBoundaries.right = 0;
	m_rcBoundaries.bottom = 0;
	m_cxWidth = cxWidth;
	m_cyHeight = cyHeight;
	m_HorzAlignment = Centered;
	m_VertAlignment = Centered;
	}

void AutoSizeDesc::SetFixed (int x, int y, int cxWidth, int cyHeight)

//	SetFixed
//
//	Sets the autosize paraments to a fixed frame at a given location

	{
	m_rcBoundaries.left = x;
	m_rcBoundaries.top = y;
	m_rcBoundaries.right = 0;
	m_rcBoundaries.bottom = 0;
	m_cxWidth = cxWidth;
	m_cyHeight = cyHeight;
	m_HorzAlignment = MaxVariable;
	m_VertAlignment = MaxVariable;
	}

void AutoSizeDesc::SetFull (void)

//	SetFull
//
//	Sets the autosize paraments encompass the parent frame

	{
	m_rcBoundaries.left = 0;
	m_rcBoundaries.top = 0;
	m_rcBoundaries.right = 0;
	m_rcBoundaries.bottom = 0;
	m_cxWidth = 0;
	m_cyHeight = 0;
	m_HorzAlignment = DimVariable;
	m_VertAlignment = DimVariable;
	}

void AutoSizeDesc::SetBottomPanel (int cyHeight, int xLeftMargin, int xRightMargin)

//	SetBottomPanel
//
//	Sets autosize parameters to be a bottom panel with a variable width

	{
	m_rcBoundaries.left = xLeftMargin;
	m_rcBoundaries.top = 0;
	m_rcBoundaries.right = xRightMargin;
	m_rcBoundaries.bottom = 0;
	m_cxWidth = 0;
	m_cyHeight = cyHeight;
	m_HorzAlignment = DimVariable;
	m_VertAlignment = MinVariable;
	}

void AutoSizeDesc::SetLeftPanel (int cxWidth, int yTopMargin, int yBottomMargin)

//	SetLeftPanel
//
//	Sets autosize parameters to be a left panel with a variable height

	{
	m_rcBoundaries.left = 0;
	m_rcBoundaries.top = yTopMargin;
	m_rcBoundaries.right = 0;
	m_rcBoundaries.bottom = yBottomMargin;
	m_cxWidth = cxWidth;
	m_cyHeight = 0;
	m_HorzAlignment = MaxVariable;
	m_VertAlignment = DimVariable;
	}

void AutoSizeDesc::SetTopPanel (int cyHeight, int xLeftMargin, int xRightMargin)

//	SetTopPanel
//
//	Sets autosize parameters to be a bottom panel with a variable width

	{
	m_rcBoundaries.left = xLeftMargin;
	m_rcBoundaries.top = 0;
	m_rcBoundaries.right = xRightMargin;
	m_rcBoundaries.bottom = 0;
	m_cxWidth = 0;
	m_cyHeight = cyHeight;
	m_HorzAlignment = DimVariable;
	m_VertAlignment = MaxVariable;
	}

