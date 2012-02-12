//	CLSizeOptions.cpp
//
//	Implements the CLSizeOptions object

#include "Alchemy.h"
#include "TerraFirma.h"
#include "WindowsExt.h"

CLSizeOptions::CLSizeOptions (void)

//	CLSizeOptions constructor

	{
	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = 0;
	rcRect.bottom = 0;

	iHorzOptions = BothFixed;
	iVertOptions = BothFixed;
	}

void CLSizeOptions::CalcAxis (AxisOptions iOption, int iPL, int iPU, int iL, int iU, LONG *retiL, LONG *retiU)

//	CalcAxis
//
//	Calculate a single axis

	{
	switch (iOption)
		{
		//	Lower and Upper are both margins

		case BothFixed:
			*retiL = iPL + iL;
			*retiU = iPU - iU;
			break;

		//	Lower is margin, upper is dimension

		case FixedLower:
			*retiL = iPL + iL;
			*retiU = *retiL + iU;
			break;

		//	Upper is margin, lower is dimension

		case FixedUpper:
			*retiU = iPU - iU;
			*retiL = *retiU - iL;
			break;

		default:
			ASSERT(FALSE);
		}
	}

void CLSizeOptions::CalcSize (RECT *pParent, RECT *retrcRect)

//	CalcSize
//
//	Calculates the size of the child based on the parent's
//	rect size

	{
	CalcAxis(iHorzOptions, pParent->left, pParent->right, rcRect.left, rcRect.right, &retrcRect->left, &retrcRect->right);
	CalcAxis(iVertOptions, pParent->top, pParent->bottom, rcRect.top, rcRect.bottom, &retrcRect->top, &retrcRect->bottom);
	}

BOOL CLSizeOptions::ConvertItemToRect (ICCItem *pRect, RECT *retrcRect)

//	Reads the rect from the item

	{
	ICCItem *pItem;

	//	Get left

	pItem = pRect->GetElement(0);
	if (pItem == NULL)
		return FALSE;

	retrcRect->left = pItem->GetIntegerValue();

	//	Get top

	pItem = pRect->GetElement(1);
	if (pItem == NULL)
		return FALSE;

	retrcRect->top = pItem->GetIntegerValue();

	//	Get right

	pItem = pRect->GetElement(2);
	if (pItem == NULL)
		return FALSE;

	retrcRect->right = pItem->GetIntegerValue();

	//	Get bottom

	pItem = pRect->GetElement(3);
	if (pItem == NULL)
		return FALSE;

	retrcRect->bottom = pItem->GetIntegerValue();

	return TRUE;
	}

BOOL CLSizeOptions::ReadFromItem (ICCItem *pRect, ICCItem *pOptions)

//	ReadFromItem
//
//	Reads the settings from two items. The first is a list of four elements,
//	each representing a coord of a rect. The second is a list of
//	options

	{
	ICCItem *pItem;

	//	Get rect

	if (!ConvertItemToRect(pRect, &rcRect))
		return FALSE;

	//	Get options

	pItem = pOptions->GetElement(0);
	if (pItem == NULL)
		return FALSE;

	iHorzOptions = (AxisOptions)pItem->GetIntegerValue();

	pItem = pOptions->GetElement(1);
	if (pItem == NULL)
		return FALSE;

	iVertOptions = (AxisOptions)pItem->GetIntegerValue();

	return TRUE;
	}
