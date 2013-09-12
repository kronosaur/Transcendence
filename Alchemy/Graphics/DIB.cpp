//	DIB.cpp
//
//	Contains routines to manipulate DIBs

#include <windows.h>
#include <MMSystem.h>

#include "Kernel.h"
#include "KernelObjID.h"
#include "Graphics.h"

#define BFT_BITMAP 0x4d42   //	'BM'
#define WIDTHBYTES(i)   ((i+31)/32*4)

 //	Forwards

DWORD dibNumColors (LPVOID pv);
ALERROR ReadDIBInfo (IReadBlock *pBlock, HANDLE *rethDIB, int *retiBitsOffset, BITMAPINFOHEADER *retbi);

ALERROR dibConvertToDDB (HBITMAP hDIB, HPALETTE hPalette, HBITMAP *rethBitmap)

//	dibConvertToDDB
//
//	Converts a DIB to a bitmap

	{
	HDC hDC;
	HPALETTE hOldPal = NULL;
	HBITMAP hBitmap;
	DIBSECTION DIBData;

	//	Get pointers

	GetObject(hDIB, sizeof(DIBData), &DIBData);

	//	Get a DC

	hDC = GetDC(NULL);
	if (hPalette)
		{
		hOldPal = SelectPalette(hDC, hPalette, FALSE);
		RealizePalette(hDC);
		}

	//	Create a DDB from the DIB

	hBitmap = CreateDIBitmap(hDC,
			(LPBITMAPINFOHEADER)&DIBData.dsBmih,
			CBM_INIT,
			DIBData.dsBm.bmBits,
			(LPBITMAPINFO)&DIBData.dsBmih,
			DIB_RGB_COLORS);

	//	Done

	if (hOldPal)
		SelectPalette(hDC, hOldPal, FALSE);

	ReleaseDC(NULL, hDC);

	if (hBitmap == NULL)
		return ERR_FAIL;

	*rethBitmap = hBitmap;

	return NOERROR;
	}

ALERROR dibCreate16bitDIB (int cxWidth, int cyHeight, HBITMAP *rethBitmap, WORD **retpPixel)

//	dibCreate16bitDIB
//
//	Creates a DIB and returns a handle to it and a pointer to the bits.
//    
//	The format of the bits is an array of WORDs, one WORD for each pixel. 
//	The RGB values are stored as 5-6-5 bits. A single scan line is DWORD aligned.
//	The DIB is always top-down

	{
	ALERROR error;
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

	hDC = CreateCompatibleDC(NULL);
	hBitmap = CreateDIBSection(hDC,
			pInfo,
			DIB_RGB_COLORS,
			&pBits,
			NULL,
			0);
	if (hBitmap == NULL)
		{
		error = GetLastError();
		goto Fail;
		}

	DeleteDC(hDC);
	hDC = NULL;

	if (retpPixel)
		*retpPixel = (WORD *)pBits;

	*rethBitmap = hBitmap;

	return NOERROR;

Fail:

	if (hBitmap)
		DeleteObject(hBitmap);

	if (hDC)
		DeleteDC(hDC);

	return error;
    }

ALERROR dibCreate24bitDIB (int cxWidth, int cyHeight, DWORD dwFlags, HBITMAP *rethBitmap, BYTE **retpPixel)

//	dibCreate24bitDIB
//
//	Creates a DIB and returns a handle to it and a pointer to the bits.
//    
//	The format of the bits is an array of BYTEs, three BYTEs for each pixel. 
//	The RGB values are stored as 8-8-8 bits. A single scan line is DWORD aligned.
//	The DIB is always bottom-up

	{
	ALERROR error;
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
	pInfo->bmiHeader.biBitCount = 24;
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
		{
		error = ::GetLastError();
		goto Fail;
		}

	::DeleteDC(hDC);
	hDC = NULL;

	if (retpPixel)
		*retpPixel = (BYTE *)pBits;
	*rethBitmap = hBitmap;

	return NOERROR;

Fail:

	if (hBitmap)
		::DeleteObject(hBitmap);

	if (hDC)
		::DeleteDC(hDC);

	return error;
	}

ALERROR dibCreate32bitDIB (int cxWidth, int cyHeight, DWORD dwFlags, HBITMAP *rethBitmap, DWORD **retpPixel)

//	dibCreate32bitDIB
//
//	Creates a DIB and returns a handle to it and a pointer to the bits.
//    
//	The format of the bits is an array of DWORDs, one DWORD for each pixel. 
//	The RGB values are stored as 8-8-8 bits. A single scan line is DWORD aligned.
//	The DIB is always bottom-up

	{
	ALERROR error;
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

	hDC = CreateCompatibleDC(NULL);
	hBitmap = CreateDIBSection(hDC,
			pInfo,
			DIB_RGB_COLORS,
			&pBits,
			NULL,
			0);
	if (hBitmap == NULL)
		{
		error = GetLastError();
		goto Fail;
		}

	DeleteDC(hDC);
	hDC = NULL;

	*retpPixel = (DWORD *)pBits;
	*rethBitmap = hBitmap;

	return NOERROR;

Fail:

	if (hBitmap)
		DeleteObject(hBitmap);

	if (hDC)
		DeleteDC(hDC);

	return error;
    }

ALERROR dibGetInfo (HBITMAP hDIB, int *retcxWidth, int *retcyHeight, void **retpBase, int *retiStride)

//	dibGetInfo
//
//	Returns a pointer to the bits of a DIBSECTION.
//	Any of the return parameters may be NULL.

	{
	DIBSECTION ds;

	if (GetObject(hDIB, sizeof(ds), &ds) == sizeof(ds)
			&& ds.dsBm.bmBits)
		{
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
			if (retpBase)
				*retpBase = (void *) (((int) ds.dsBm.bmBits) + (ds.dsBm.bmWidthBytes * (ds.dsBm.bmHeight - 1))); 
			if (retiStride)
				*retiStride = (int)(-ds.dsBm.bmWidthBytes);
			}         
		else 
			{
			if (retpBase)
				*retpBase  = ds.dsBm.bmBits; 
			if (retiStride)
				*retiStride = ds.dsBm.bmWidthBytes;
			}

		//	Return size

		if (retcxWidth)
			*retcxWidth = ds.dsBm.bmWidth;
		if (retcyHeight)
			*retcyHeight = ds.dsBm.bmHeight;
		}
	else
		return ERR_FAIL;

	return NOERROR;
	}

bool dibIs16bit (HBITMAP hDIB)

//	dibIs16bit
//
//	TRUE if this is a 16-bit DIB

	{
	DIBSECTION ds;

	return (::GetObject(hDIB, sizeof(ds), &ds) == sizeof(ds)
			&& ds.dsBmih.biBitCount == 16
			&& ds.dsBitfields[0] == 0xf800	//	5 bits of Red
			&& ds.dsBitfields[1] == 0x7e0	//	6 bits of Green
			&& ds.dsBitfields[2] == 0x1f);	//	5 bits of Blue
	}

bool dibIs24bit (HBITMAP hDIB)

//	dibIs24bit
//
//	TRUE if this is a 24-bit DIB

	{
	DIBSECTION ds;

	return (::GetObject(hDIB, sizeof(ds), &ds) == sizeof(ds)
			&& ds.dsBmih.biBitCount == 24);
	}

ALERROR dibLoadFromBlock (IReadBlock &Data, HBITMAP *rethDIB, EBitmapTypes *retiType)

//	dibLoadFromBlock
//
//	Loads a DIB into memory

	{
	ALERROR error;
	HBITMAP hDIB;
	HANDLE hFileData;
	int iBitsOffset;
	BITMAPINFOHEADER bi;

	if (error = Data.Open())
		return error;

	//	Get information

	if (error = ReadDIBInfo(&Data, &hFileData, &iBitsOffset, &bi))
		return error;

	//	Return the bit-depth of the original bitmap

	if (retiType)
		{
		switch (bi.biBitCount)
			{
			case 1:
				*retiType = bitmapMonochrome;
				break;

			case 8:
				*retiType = bitmapAlpha;
				break;

			default:
				*retiType = bitmapRGB;
				break;
			}
		}

	//	Calculate how much space we need for the bits

	DWORD dwBits = bi.biSizeImage;
	DWORD dwPaletteSize = dibPaletteSize(&bi);
	DWORD dwLen = bi.biSize + dwPaletteSize + dwBits;

	//	Create a DIBSection.

	if (error = dibCreate16bitDIB(bi.biWidth, bi.biHeight, &hDIB, NULL))
		{
		GlobalFree(hFileData);
		return error;
		}

	//	Set the bits

	HDC hDC = CreateCompatibleDC(NULL);
	SetDIBits(hDC,
			hDIB,
			0,
			bi.biHeight,
			Data.GetPointer(iBitsOffset, -1),
			(BITMAPINFO *)GlobalLock(hFileData),
			DIB_RGB_COLORS);

	::DeleteDC(hDC);

	GlobalUnlock(hFileData);
	GlobalFree(hFileData);

	//	Done

	*rethDIB = hDIB;

	return NOERROR;
	}

ALERROR dibLoadFromFile (CString sFilename, HBITMAP *rethDIB, EBitmapTypes *retiType)

//	dibLoadFromFile
//
//	Loads a DIB into memory

	{
	CFileReadBlock DIBFile(sFilename);
	return dibLoadFromBlock(DIBFile, rethDIB, retiType);
	}

ALERROR dibLoadFromResource (HINSTANCE hInst, char *szResource, HBITMAP *rethDIB, EBitmapTypes *retiType)

//	dibLoadFromResource
//
//	Loads a DIB from a resource

	{
	HRSRC hRes = ::FindResource(hInst, szResource, RT_BITMAP);
	if (hRes == NULL)
		return ERR_FAIL;

	int iLength = ::SizeofResource(hInst, hRes);

	HGLOBAL hData = ::LoadResource(hInst, hRes);
	if (hData == NULL)
		return ERR_FAIL;

	LPVOID pData = ::LockResource(hData);
	CBufferReadBlock Data(CString((char *)pData, iLength, TRUE));

	return dibLoadFromBlock(Data, rethDIB, retiType);
	}

DWORD dibNumColors (LPVOID pv)

//	dibNumColors
//
//	Returns the size of the color table
//
//	pv: pointer either to a BITMAPCOREHEADER or BITMAPINFOHEADER

	{
    LPBITMAPINFOHEADER pbi = (LPBITMAPINFOHEADER)pv;
    LPBITMAPCOREHEADER pbc = (LPBITMAPCOREHEADER)pv;
    BOOL bInfoHeader = pbi->biSize == sizeof(BITMAPINFOHEADER);
    DWORD dwNumColors;
    
	//	Calculate the number of colors in the palette. With the BITMAPINFO
    //	format headers, the size of the palette is in biClrUsed, whereas
    //	in the BITMAPCORE - style headers, it is dependent on the bits 
    //	per pixel ( = 2 raised to the power of bits/pixel) */
    
	if (bInfoHeader && pbi->biClrUsed != 0)
        dwNumColors = pbi->biClrUsed;
    else
        {
    	int nBits;
        
        if (bInfoHeader)
            nBits = pbi->biBitCount;
        else
            nBits = pbc->bcBitCount;
        
		//	Anything with more than 8-bits per pixel does not have a color
        //	table.
        
        if (nBits > 8)
            dwNumColors = 0;
        else
        	dwNumColors = 1L << nBits;
        }

	return dwNumColors;
	}

DWORD dibPaletteSize (LPVOID pv)

//	dibPaletteSize
//
//	Returns the size (in bytes) of the DIB color table
//
//	pv: pointer either to a BITMAPCOREHEADER or BITMAPINFOHEADER

	{
    LPBITMAPINFOHEADER pbi = (LPBITMAPINFOHEADER)pv;
    BOOL bInfoHeader = pbi->biSize == sizeof(BITMAPINFOHEADER);
    
	//	Return the palette size
    
    if (bInfoHeader)
        return (DWORD)(dibNumColors(pv) * sizeof(RGBQUAD));
    else
        return (DWORD)(dibNumColors(pv) * sizeof(RGBTRIPLE));
	}

ALERROR ReadDIBInfo (IReadBlock *pBlock, HANDLE *rethDIB, int *retiBitsOffset, BITMAPINFOHEADER *retbi)

//	ReadDIBInfo
//
//	Reads the DIB info from a block of memory

	{
	int iLen = pBlock->GetLength();
	char *pPos = pBlock->GetPointer(0, iLen);
	char *pEnd = pPos + iLen;
	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;
	int iBitmapInfoOffset;

	//	Read in the type WORD

	if (pPos + sizeof(WORD) + sizeof(DWORD) > pEnd)	return ERR_FAIL;
	utlMemCopy(pPos, (char *)&bf.bfType, sizeof(WORD));
	pPos += sizeof(WORD);

	//	Read in the next three DWORDs

	utlMemCopy(pPos, (char *)&bf.bfSize, sizeof(DWORD) * 3);
	pPos += sizeof(DWORD) * 3;

	//	Do we have an RC HEADER?

	if (bf.bfType != BFT_BITMAP)
		{
		bf.bfOffBits = 0;
		pPos = pBlock->GetPointer(0, iLen);
		iBitmapInfoOffset = 0;
		}
	else
		iBitmapInfoOffset = sizeof(WORD) + 3 * sizeof(DWORD);

	//	Read the info header

	if (pPos + sizeof(bi) > pEnd) return ERR_FAIL;
	utlMemCopy(pPos, (char *)&bi, sizeof(bi));
	pPos += sizeof(bi);

	DWORD dwNumColors = dibNumColors(&bi);

	//	Check the nature of the info block and extract the field
	//	information accordingly. Convert to a BITMAPINFOHEADER,
	//	if necessary.

	int iSize = (int)bi.biSize;
	switch (iSize)
		{
		case sizeof(BITMAPINFOHEADER):
			break;

		case sizeof(BITMAPCOREHEADER):
			{
			BITMAPCOREHEADER bc = *(BITMAPCOREHEADER *)&bi;
			DWORD dwWidth = (DWORD)bc.bcWidth;
			DWORD dwHeight = (DWORD)bc.bcHeight;
			WORD wPlanes = bc.bcPlanes;
			WORD wBitCount = bc.bcBitCount;

			bi.biSize = sizeof(BITMAPINFOHEADER);
			bi.biWidth = dwWidth;
			bi.biHeight = dwHeight;
			bi.biPlanes = wPlanes;
			bi.biBitCount = wBitCount;
			bi.biCompression = BI_RGB;
			bi.biSizeImage = 0;
			bi.biXPelsPerMeter = 0;
			bi.biYPelsPerMeter = 0;
			bi.biClrUsed = dwNumColors;
			bi.biClrImportant = dwNumColors;

			pPos += (int)(sizeof BITMAPCOREHEADER) - (int)(sizeof BITMAPINFOHEADER);
			break;
			}

		//	Not a DIB!

		default:
			return ERR_FAIL;
		}

	//	Fill in some default values if they are zero

	if (bi.biSizeImage == 0)
		bi.biSizeImage = WIDTHBYTES((DWORD)bi.biWidth * bi.biBitCount) * bi.biHeight;

	if (bi.biClrUsed == 0)
        bi.biClrUsed = dwNumColors;

	//	Allocate memory for the BITMAPINFO struct and the color table

	HANDLE hbi = GlobalAlloc(GHND, (LONG)bi.biSize + dwNumColors * sizeof(RGBQUAD));
	if (!hbi)
		return ERR_MEMORY;

	LPBITMAPINFOHEADER lpbi = (BITMAPINFOHEADER *)GlobalLock(hbi);
	*lpbi = bi;

	//	Get the pointer to the color table

	RGBQUAD *pRGB = (RGBQUAD *)((char *)lpbi + bi.biSize);
	if (dwNumColors)
		{
		if (iSize == sizeof(BITMAPCOREHEADER))
			{
			//	Convert an old color table (3 byte RGBTRIPLEs) to a 
			//	new table (4 byte RGBQUADs)

			if (pPos + dwNumColors * sizeof(RGBTRIPLE) > pEnd) return ERR_FAIL;
			utlMemCopy(pPos, (char *)pRGB, dwNumColors * sizeof(RGBTRIPLE));
			pPos += dwNumColors * sizeof(RGBTRIPLE);

			for (int i = dwNumColors - 1; i >= 0; i--)
				{
				RGBQUAD rgb;

				rgb.rgbRed = ((RGBTRIPLE *)pRGB)[i].rgbtRed;
				rgb.rgbGreen = ((RGBTRIPLE *)pRGB)[i].rgbtGreen;
				rgb.rgbBlue = ((RGBTRIPLE *)pRGB)[i].rgbtBlue;
				rgb.rgbReserved = (BYTE)0;

				pRGB[i] = rgb;
				}
			}
		else
			{
			if (pPos + dwNumColors * sizeof(RGBQUAD) > pEnd) return ERR_FAIL;
			utlMemCopy(pPos, (char *)pRGB, dwNumColors * sizeof(RGBQUAD));
			pPos += dwNumColors * sizeof(RGBQUAD);
			}
		}

	//	Done

	if (bf.bfOffBits)
		*retiBitsOffset = bf.bfOffBits;
	else
		*retiBitsOffset = pPos - pBlock->GetPointer(0, 1);

	if (retbi)
		*retbi = bi;

	GlobalUnlock(hbi);
	*rethDIB = (HBITMAP)hbi;

	return NOERROR;
	}
