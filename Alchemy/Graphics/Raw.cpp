//	Raw.cpp
//
//	Contains routines to manipulate the raw image format (DWORD per pixel,
//	8bits each for RGB, plus 8bits for transparency information

#include <windows.h>
#include <MMSystem.h>

#include "Kernel.h"
#include "KernelObjID.h"
#include "Graphics.h"

ALERROR rawConvertToDDB (int cxWidth, int cyHeight, RAWPIXEL *pRaw, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap)

//	rawConvertToDDB
//
//	Converts a raw image to a screen compatible bitmap
//
//	cxWidth: The width of the raw image array in pixels
//	cyHeight: The height of the raw image array in pixels
//	pRaw: The actual array of raw image data. The image is a 2-dimensional
//		array cxWidth by cyHeight pixels. Each pixel is a 32bit value.
//	dwFlags: Unused
//	hPalette: Current palette being used by the main window (may be NULL)
//
//	rethBitmap: Returns a DDB compatible with the screen

	{
	ALERROR error;
	HBITMAP hDIB;

	//	Start by converting to a DIB

	if (error = rawConvertTo16bitDIB(cxWidth, cyHeight, pRaw, dwFlags, &hDIB))
		return error;

	//	Now convert to a DDB

	error = dibConvertToDDB(hDIB, hPalette, rethBitmap);
	DeleteObject(hDIB);
	if (error)
		return error;

	return NOERROR;
	}

ALERROR rawConvertTo16bitDIB (int cxWidth, int cyHeight, RAWPIXEL *pRaw, DWORD dwFlags, HBITMAP *rethBitmap)

//	rawConvertTo16bitDIB
//
//	Converts a raw image to a 16bit DIB
//
//	cxWidth: The width of the raw image array in pixels
//	cyHeight: The height of the raw image array in pixels
//	pRaw: The actual array of raw image data. The image is a 2-dimensional
//		array cxWidth by cyHeight pixels. Each pixel is a 32bit value.
//	dwFlags: Unused
//
//	rethBitmap: Returns a DIB. Note that depending on the graphics card, the
//		speed of BitBlt with this DIB will vary. For best results, convert to
//		a DDB before Blitting.

	{
	ALERROR error;
	HBITMAP hBitmap;
	WORD *p16BitPixel;
	RAWPIXEL *pRawPixel;
	int iScanline;

	//	Create a destination DIB

	if (error = dibCreate16bitDIB(cxWidth,
			cyHeight,
			&hBitmap,
			&p16BitPixel))
		return error;

	//	Process each pixel in the raw bitmap by scanlines

	iScanline = 0;
	pRawPixel = pRaw;
	while (iScanline < cyHeight)
		{
		int i;

		for (i = 0; i < cxWidth; i++)
			{
			int iRed, iGreen, iBlue;

			//	Down-sample to 5 or 6 bits

			iRed = (32 * rawRed(*pRawPixel)) / 256;			//	5 bits of red
			iGreen = (64 * rawGreen(*pRawPixel)) / 256;		//	6 bits of green
			iBlue = (32 * rawBlue(*pRawPixel)) / 256;		//	5 bits of blue

			//	Create pixel value

			*p16BitPixel = (((WORD)iRed) << 11) | (((WORD)iGreen) << 5) | (WORD)iBlue;

			//	Next

			p16BitPixel++;
			pRawPixel++;
			}

		//	We need to make sure that each output scan line is aligned on
		//	a DWORD boundary. We assume here that pPixel is a pointer to
		//	a WORD.

		p16BitPixel += (cxWidth % 2);
		iScanline++;
		}

	//	Done

	*rethBitmap = hBitmap;

	return NOERROR;
	}

