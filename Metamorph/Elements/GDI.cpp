//	GDI.cpp
//
//	Miscellaneous GDI Routines
//	Copyright (c) 2001 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"

void gdiConvertToDDB (HBITMAP hDIB, HPALETTE hPalette, HBITMAP *rethBitmap)

//	gdiConvertToDDB
//
//	Converts a DIB to a bitmap

	{
	HDC hDC;
	HPALETTE hOldPal = NULL;
	HBITMAP hBitmap;
	DIBSECTION DIBData;

	//	Get pointers

	::GetObject(hDIB, sizeof(DIBData), &DIBData);

	//	Get a DC

	hDC = ::GetDC(NULL);
	if (hPalette)
		{
		hOldPal = ::SelectPalette(hDC, hPalette, FALSE);
		::RealizePalette(hDC);
		}

	//	Create a DDB from the DIB

	hBitmap = ::CreateDIBitmap(hDC,
			(LPBITMAPINFOHEADER)&DIBData.dsBmih,
			CBM_INIT,
			DIBData.dsBm.bmBits,
			(LPBITMAPINFO)&DIBData.dsBmih,
			DIB_RGB_COLORS);

	//	Done

	if (hOldPal)
		::SelectPalette(hDC, hOldPal, FALSE);

	::ReleaseDC(NULL, hDC);

	if (hBitmap == NULL)
		throw CException(errFail);

	*rethBitmap = hBitmap;
	}

void gdiCreate16bitDIB (int cxWidth, int cyHeight, HBITMAP *rethBitmap, WORD **retpPixel)

//	dibCreate16bitDIB
//
//	Creates a DIB and returns a handle to it and a pointer to the bits.
//    
//	The format of the bits is an array of WORDs, one WORD for each pixel. 
//	The RGB values are stored as 5-6-5 bits. A single scan line is DWORD aligned.
//	The DIB is always top-down

	{
	HBITMAP hBitmap = NULL;
	BITMAPINFO *pInfo = NULL;
	DWORD *pRGBMask;
	void *pBits;
	HDC hDC = NULL;
	BYTE Buffer[sizeof(BITMAPINFO) + 2 * sizeof(DWORD)];

	//	First we allocate a bitmapinfo structure, which consists
	//	of a structure followed by an array of color values. We need to allocate
	//	three color values, but we ask for 2 because BITMAPINFO includes the first

	pInfo = (BITMAPINFO *)Buffer;
	utlMemSet(pInfo, sizeof(BITMAPINFO), 0);

	pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pInfo->bmiHeader.biWidth = cxWidth;
	pInfo->bmiHeader.biHeight = cyHeight;
	pInfo->bmiHeader.biPlanes = 1;
	pInfo->bmiHeader.biBitCount = 16;
	pInfo->bmiHeader.biCompression = BI_BITFIELDS;

	//	Set up the masks that describe the RGB arrangement

	pRGBMask = (DWORD *)&pInfo->bmiColors[0];
	pRGBMask[0] = 0xF800;				//	red component
	pRGBMask[1] = 0x07E0;				//	green component
	pRGBMask[2] = 0x001F;				//	blue component

	hDC = ::CreateCompatibleDC(NULL);
	hBitmap = ::CreateDIBSection(hDC,
			pInfo,
			DIB_RGB_COLORS,
			&pBits,
			NULL,
			0);
	if (hBitmap == NULL)
		throw CException(errFail);

	::DeleteDC(hDC);
	hDC = NULL;

	if (retpPixel)
		*retpPixel = (WORD *)pBits;

	*rethBitmap = hBitmap;
    }

void gdiCreate32bitDIB (int cxWidth, int cyHeight, HBITMAP *rethBitmap, DWORD **retpPixel)

//	dibCreate32bitDIB
//
//	Creates a DIB and returns a handle to it and a pointer to the bits.
//    
//	The format of the bits is an array of DWORDs, one DWORD for each pixel. 
//	The RGB values are stored as 8-8-8 bits. A single scan line is DWORD aligned.
//	The DIB is always bottom-up

	{
	HBITMAP hBitmap = NULL;
	BITMAPINFO *pInfo = NULL;
	void *pBits;
	HDC hDC = NULL;
	BYTE Buffer[sizeof(BITMAPINFO) + 2 * sizeof(DWORD)];

	//	First we allocate a bitmapinfo structure, which consists
	//	of a structure followed by an array of color values. We need to allocate
	//	three color values, but we ask for 2 because BITMAPINFO includes the first

	pInfo = (BITMAPINFO *)Buffer;
	utlMemSet(pInfo, sizeof(BITMAPINFO), 0);

	pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pInfo->bmiHeader.biWidth = cxWidth;
	pInfo->bmiHeader.biHeight = cyHeight;
	pInfo->bmiHeader.biPlanes = 1;
	pInfo->bmiHeader.biBitCount = 32;
	pInfo->bmiHeader.biCompression = BI_RGB;

	//	Create the DIB section

	hDC = ::CreateCompatibleDC(NULL);
	hBitmap = ::CreateDIBSection(hDC,
			pInfo,
			DIB_RGB_COLORS,
			&pBits,
			NULL,
			0);
	if (hBitmap == NULL)
		throw CException(errFail);

	::DeleteDC(hDC);
	hDC = NULL;

	if (retpPixel)
		*retpPixel = (DWORD *)pBits;
	*rethBitmap = hBitmap;
    }

void gdiFillRect (HDC hDC, const RECT &rcRect, COLORREF Color)

//	gdiFillRect
//
//	Fill the rectangle with the given color

	{
	COLORREF OldColor = SetBkColor(hDC, Color);
	ExtTextOut(hDC, rcRect.left, rcRect.top, ETO_OPAQUE, &rcRect, "", 0, NULL);
	SetBkColor(hDC, OldColor);
	}

int gdiGetFontHeight (HDC hDC, int iPointSize)

//	gdiGetFontHeight
//
//	Returns the height in pixels of a font of the given point size

	{
	HDC hScreenDC = NULL;
	if (hDC == NULL)
		{
		hScreenDC = ::GetDC(NULL);
		hDC = hScreenDC;
		}

	int iHeight = -((iPointSize * ::GetDeviceCaps(hDC, LOGPIXELSY) + 36) / 72);

	if (hScreenDC)
		::ReleaseDC(NULL, hScreenDC);

	return iHeight;
	}
