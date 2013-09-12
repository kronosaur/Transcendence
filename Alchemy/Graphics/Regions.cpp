//	Regions.cpp
//
//	Contains routines to manipulate regions

#include <windows.h>
#include <MMSystem.h>

#include "Kernel.h"
#include "KernelObjID.h"
#include "Graphics.h"

inline bool PixelAt (DWORD *pLine, int x)
	{
	return (((0x80 >> (x % 8)) & ((BYTE *)pLine)[x / 8]) ? true : false);
	}

ALERROR rgnCreateFromBitmap (HBITMAP hBitmap, HRGN *rethRgn)

//	rgnCreateFromBitmap
//
//	Creates a region from a bitmap

	{
	ALERROR error;
	HDC hDC = NULL;
	DWORD *pLine = NULL;
	RGNDATAHEADER rdh;
	BITMAP bm;
	int x, y;
	int iLineSize;
	char bitmapinfo[sizeof(BITMAPINFOHEADER) + 2 * sizeof(DWORD)];
	BITMAPINFOHEADER *pbmi = (BITMAPINFOHEADER *)bitmapinfo;

	//	Figure out the size of the bitmap

	GetObject(hBitmap, sizeof(bm), &bm);

	//	Prepare an output stream

	CMemoryWriteStream Output(bm.bmWidth * bm.bmHeight * sizeof(RECT));
	if (error = Output.Create())
		goto Fail;

	rdh.dwSize = sizeof(rdh);
	rdh.iType = RDH_RECTANGLES;
	rdh.nCount = 0;
	rdh.nRgnSize = 0;

	//	We assume that the bitmap already represents the bounding rect

	rdh.rcBound.top = 0;
	rdh.rcBound.left = 0;
	rdh.rcBound.bottom = bm.bmHeight;
	rdh.rcBound.right = bm.bmWidth;

	//	Write out the unitialized header; we will fill it in later

	if (error = Output.Write((char *)&rdh, sizeof(rdh), NULL))
		goto Fail;

	//	Prepare

	hDC = CreateCompatibleDC(NULL);
	iLineSize = AlignUp(bm.bmWidth, 32) / 32;
	pLine = (DWORD *)MemAlloc(iLineSize * sizeof(DWORD));

	ZeroMemory(pbmi, sizeof(BITMAPINFOHEADER));
	pbmi->biSize = sizeof(BITMAPINFOHEADER);
	pbmi->biWidth = bm.bmWidth;
	pbmi->biHeight = bm.bmHeight;
	pbmi->biPlanes = 1;
	pbmi->biBitCount = 1;
	pbmi->biCompression = BI_RGB;

	//	Loop over each scan line

	for (y = 0; y < bm.bmHeight; y++)
		{
		int xRunStart;

		GetDIBits(hDC,
				hBitmap,
				y,
				1,
				pLine,
				(BITMAPINFO *)pbmi,
				DIB_RGB_COLORS);

		//	Look for the beginning of a run

		x = 0;
		xRunStart = -1;
		while (x < bm.bmWidth)
			{
			if (xRunStart == -1)
				{
				//	If we're not in a run and we suddenly find a white pixel
				//	then we know that we begin a run

				if (PixelAt(pLine, x))
					xRunStart = x;
				}
			else
				{
				//	If we're in a run and we suddenly find a black pixel
				//	then we know that we're done

				if (!PixelAt(pLine, x))
					{
					RECT rcRect;

					rcRect.left = xRunStart;
					rcRect.top = y;
					rcRect.right = x;
					rcRect.bottom = y + 1;

					//	Add the rect to the region

					if (error = Output.Write((char *)&rcRect, sizeof(rcRect), NULL))
						goto Fail;

					rdh.nCount++;
					rdh.nRgnSize += sizeof(RECT);

					xRunStart = -1;
					}
				}

			x++;
			}

		//	If we were in a run, then end it

		if (xRunStart != -1)
			{
			RECT rcRect;

			rcRect.left = xRunStart;
			rcRect.top = y;
			rcRect.right = x;
			rcRect.bottom = y + 1;

			//	Add the rect to the region

			if (error = Output.Write((char *)&rcRect, sizeof(rcRect), NULL))
				goto Fail;

			rdh.nCount++;
			rdh.nRgnSize += sizeof(RECT);
			}
		}

	//	Close it out

	MemFree(pLine);
	pLine = NULL;

	DeleteDC(hDC);
	hDC = NULL;

	if (error = Output.Close())
		goto Fail;

	//	Update the header

	*(RGNDATAHEADER *)Output.GetPointer() = rdh;

	//	Create the region

	*rethRgn = ExtCreateRegion(NULL,
			Output.GetLength(),
			(RGNDATA *)Output.GetPointer());

	//	Done

	return NOERROR;

Fail:

	if (pLine)
		MemFree(pLine);

	if (hDC)
		DeleteDC(hDC);

	return error;
	}
