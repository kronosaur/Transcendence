//	DIB.cpp
//
//	Miscellaneous DIB Routines
//	Copyright (c) 2001 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"

void dibGetInfo (HBITMAP hDIB, dibInfoStruct *dibInfo)
	{
	DIBSECTION ds;

#ifndef DEBUG
	::GetObject(hDIB, sizeof(ds), &ds);
#else
	if (::GetObject(hDIB, sizeof(ds), &ds) != sizeof(ds)
			|| ds.dsBm.bmBits == NULL)
		//	Must be a DIB
		ASSERT(false);
#endif

    // For backwards compatibility with Get/SetBitmapBits, GDI does 
    // not accurately report the bitmap pitch in bmWidthBytes.  It 
    // always computes bmWidthBytes assuming WORD-aligned scanlines 
    // regardless of the platform.
	// 
    // Therefore, if the platform is WinNT, which uses DWORD-aligned 
    // scanlines, adjust the bmWidthBytes value. 

	if (sysGetAPIFlags() & API_FLAG_WINNT)
		ds.dsBm.bmWidthBytes = AlignUp(ds.dsBm.bmWidthBytes, sizeof(DWORD));

    // If biHeight is positive, then the bitmap is a bottom-up DIB. 
    // If biHeight is negative, then the bitmap is a top-down DIB. 
	
	if (ds.dsBmih.biHeight > 0)
		{
		dibInfo->pBase = (BYTE *) (((int) ds.dsBm.bmBits) + (ds.dsBm.bmWidthBytes * (ds.dsBm.bmHeight - 1)));
		dibInfo->iStride = (int)(-ds.dsBm.bmWidthBytes);
		}         
	else 
		{
		dibInfo->pBase = (BYTE *)ds.dsBm.bmBits;
		dibInfo->iStride = ds.dsBm.bmWidthBytes;
		}

	//	Return other data

	dibInfo->cxWidth = ds.dsBm.bmWidth;
	dibInfo->cyHeight = ds.dsBm.bmHeight;

	//	Bit count. This switch statement must
	//	set up iBitPattern and iColorTableSize.

	switch (ds.dsBmih.biBitCount)
		{
		case 32:
			dibInfo->iBitPattern = dib32bits;
			dibInfo->iColorTableSize = 0;
			break;

		case 24:
			dibInfo->iBitPattern = dib24bits;
			dibInfo->iColorTableSize = 0;
			break;

		case 16:
			{
			if (ds.dsBitfields[0] == 0xf800			//	5 bits of Red
					&& ds.dsBitfields[1] == 0x7e0	//	6 bits of Green
					&& ds.dsBitfields[2] == 0x1f)	//	5 bits of Blue
				dibInfo->iBitPattern = dib16bits565;
			else
				dibInfo->iBitPattern = dib16bits555;

			dibInfo->iColorTableSize = 0;
			break;
			}

		case 8:
			dibInfo->iBitPattern = dib8bits;
			if (ds.dsBmih.biClrUsed == 0)
				dibInfo->iColorTableSize = 256;
			else
				dibInfo->iColorTableSize = ds.dsBmih.biClrUsed;
			break;

		case 4:
			dibInfo->iBitPattern = dib4bits;
			if (ds.dsBmih.biClrUsed == 0)
				dibInfo->iColorTableSize = 16;
			else
				dibInfo->iColorTableSize = ds.dsBmih.biClrUsed;
			break;

		case 1:
			dibInfo->iBitPattern = dibMono;
			if (ds.dsBmih.biClrUsed == 0)
				dibInfo->iColorTableSize = 2;
			else
				dibInfo->iColorTableSize = ds.dsBmih.biClrUsed;
			break;

		default:
			//	Don't know how to handle this image
			ASSERT(false);
		}

	//	Setup the color table, if we've got one

	if (dibInfo->iColorTableSize > 0)
		{
		ASSERT(dibInfo->iColorTableSize <= 256);

		//	Get the color table from the DIB

		HDC hDC = ::CreateCompatibleDC(NULL);
		HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hDC, hDIB);
		RGBQUAD ColorTable[256];
		::GetDIBColorTable(hDC, 0, dibInfo->iColorTableSize, ColorTable);
		::SelectObject(hDC, hOldBitmap);
		::DeleteDC(hDC);

		//	Store in the dib info structure

		for (int i = 0; i < dibInfo->iColorTableSize; i++)
			{
			dibInfo->ColorTable[i] = RGB(ColorTable[i].rgbRed,
					ColorTable[i].rgbGreen,
					ColorTable[i].rgbBlue);
			}
		}
	}

void dibGetInfo (BITMAPINFO *pbmi, dibInfoStruct *dibInfo)
	{
	int iOffsetToBits = pbmi->bmiHeader.biSize;

	//	Some basic data

	dibInfo->cxWidth = pbmi->bmiHeader.biWidth;
	dibInfo->cyHeight = abs(pbmi->bmiHeader.biHeight);

	//	Bit count. This switch statement must
	//	set up iBitPattern and iColorTableSize.

	switch (pbmi->bmiHeader.biBitCount)
		{
		case 32:
			dibInfo->iBitPattern = dib32bits;
			dibInfo->iColorTableSize = 0;

			//	Color table at the end of bmi?
			if (pbmi->bmiHeader.biCompression == BI_BITFIELDS)
				iOffsetToBits += 3 * sizeof(DWORD);
			break;

		case 24:
			dibInfo->iBitPattern = dib24bits;
			dibInfo->iColorTableSize = 0;

			//	Color table at the end of bmi?
			iOffsetToBits += pbmi->bmiHeader.biClrUsed * sizeof(DWORD);
			break;

		case 16:
			{
			if (pbmi->bmiHeader.biCompression == BI_RGB)
				{
				dibInfo->iBitPattern = dib16bits555;

				//	Color table at the end of bmi?
				iOffsetToBits += pbmi->bmiHeader.biClrUsed * sizeof(DWORD);
				}
			else
				{
				ASSERT(pbmi->bmiHeader.biCompression == BI_BITFIELDS);

				if (*(DWORD *)&pbmi->bmiColors[0] == 0xf800			//	5 bits of Red
						&& *(DWORD *)&pbmi->bmiColors[1] == 0x7e0	//	6 bits of Green
						&& *(DWORD *)&pbmi->bmiColors[2] == 0x1f)	//	5 bits of Blue
					dibInfo->iBitPattern = dib16bits565;
				else
					dibInfo->iBitPattern = dib16bits555;

				//	Color table at the end of bmi?
				iOffsetToBits += 3 * sizeof(DWORD);
				}

			dibInfo->iColorTableSize = 0;
			break;
			}

		case 8:
			dibInfo->iBitPattern = dib8bits;
			if (pbmi->bmiHeader.biClrUsed == 0)
				dibInfo->iColorTableSize = 256;
			else
				dibInfo->iColorTableSize = pbmi->bmiHeader.biClrUsed;

			//	Color table at the end of bmi?
			iOffsetToBits += dibInfo->iColorTableSize * sizeof(DWORD);
			break;

		case 4:
			dibInfo->iBitPattern = dib4bits;
			if (pbmi->bmiHeader.biClrUsed == 0)
				dibInfo->iColorTableSize = 16;
			else
				dibInfo->iColorTableSize = pbmi->bmiHeader.biClrUsed;

			//	Color table at the end of bmi?
			iOffsetToBits += dibInfo->iColorTableSize * sizeof(DWORD);
			break;

		case 1:
			dibInfo->iBitPattern = dibMono;
			if (pbmi->bmiHeader.biClrUsed == 0)
				dibInfo->iColorTableSize = 2;
			else
				dibInfo->iColorTableSize = pbmi->bmiHeader.biClrUsed;

			//	Color table at the end of bmi?
			iOffsetToBits += dibInfo->iColorTableSize * sizeof(DWORD);
			break;

		default:
			//	Don't know how to handle this image
			ASSERT(false);
		}

	//	Setup the color table

	if (dibInfo->iColorTableSize > 0)
		{
		ASSERT(dibInfo->iColorTableSize <= 256);

		//	Store in the dib info structure

		for (int i = 0; i < dibInfo->iColorTableSize; i++)
			{
			dibInfo->ColorTable[i] = RGB(pbmi->bmiColors[i].rgbRed,
					pbmi->bmiColors[i].rgbGreen,
					pbmi->bmiColors[i].rgbBlue);
			}
		}

    // If biHeight is positive, then the bitmap is a bottom-up DIB. 
    // If biHeight is negative, then the bitmap is a top-down DIB. 
	
	BYTE *pBits = ((BYTE *)pbmi) + iOffsetToBits;
	if (pbmi->bmiHeader.biHeight > 0)
		{
		dibInfo->pBase = (BYTE *) (((int) pBits) + (AlignUp(pbmi->bmiHeader.biWidth, sizeof(DWORD)) * (pbmi->bmiHeader.biHeight - 1)));
		dibInfo->iStride = (int)(-AlignUp(pbmi->bmiHeader.biWidth, sizeof(DWORD)));
		}         
	else 
		{
		dibInfo->pBase = pBits;
		dibInfo->iStride = AlignUp(pbmi->bmiHeader.biWidth, sizeof(DWORD));
		}
	}
