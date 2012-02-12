//	JPEGWrapper.cpp
//
//	This is a wrapper for the IJG JPEG library

#include <windows.h>
#include "Alchemy.h"
#include "JPEGUtil.h"
#include <SetJmp.h>

//	Include IJG library definitions

extern "C" {
#define HAVE_BOOLEAN
#include "jinclude.h"
#include "JPEGLib.h"

//	Source loader

EXTERN(void) jpeg_memory_src (j_decompress_ptr cinfo, char *pData, int iDataLen);

//	Error handler

typedef struct _my_error_mgr
	{
	struct jpeg_error_mgr pub;			//	public fields

	jmp_buf setjmp_buffer;				//	longjump return
	} my_error_mgr, *my_error_ptr;

METHODDEF(void) my_error_exit (j_common_ptr cinfo)
	{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	//	Return control to the program

	longjmp(myerr->setjmp_buffer, 1);
	}
}

//	Forwards

static void CleanUpColorTable (struct jpeg_decompress_struct *pcinfo);
static ALERROR DecompressJPEG (char *pImage, int iImageSize, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap);
static void SetupColorTable (struct jpeg_decompress_struct *pcinfo, HPALETTE hPalette);

ALERROR JPEGLoadFromFile (CString sFilename, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap)

//	JPEGLoadFromFile
//
//	Loads a JPEG file into memory

	{
	ALERROR error;
	CFileReadBlock JPEGFile(sFilename);

	if (error = JPEGFile.Open())
		return error;

	error = JPEGLoadFromMemory(JPEGFile.GetPointer(0, JPEGFile.GetLength()), JPEGFile.GetLength(), dwFlags, hPalette, rethBitmap);
	JPEGFile.Close();
	if (error)
		return error;

	return NOERROR;
	}

ALERROR JPEGLoadFromResource (HINSTANCE hInst, char *pszRes, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap)

//	JPEGLoadFromResource
//
//	Loads a JPEG from a resource and returns a bitmap
//
//	If dwFlags is JPEG_LFR_8BITCOLOR, then hPalette is the palette to use for dithering

	{
	HRSRC hRes;
	HGLOBAL hGlobalRes;
	void *pImage;
	int iSize;

	hRes = FindResource(hInst, pszRes, "JPEG");
	if (hRes == NULL)
		return ERR_FAIL;

	iSize = SizeofResource(hInst, hRes);
	if (iSize == 0)
		return ERR_FAIL;

	hGlobalRes = LoadResource(hInst, hRes);
	if (hGlobalRes == NULL)
		return ERR_FAIL;

	pImage = LockResource(hGlobalRes);
	if (pImage == NULL)
		return ERR_FAIL;

	//	Done

	return JPEGLoadFromMemory((char *)pImage, iSize, dwFlags, hPalette, rethBitmap);
	}

ALERROR JPEGLoadFromMemory (char *pImage, int iSize, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap)

//	JPEGLoadFromMemory
//
//	Loads the given JPEG image from the memory block

	{
	ALERROR error;
	HBITMAP hDIB;
	HBITMAP hDDB;

	//	Decompress

	if (error = DecompressJPEG((char *)pImage, iSize, dwFlags, hPalette, &hDIB))
		return error;

	//	If we want a DIB, just return that; otherwise,
	//	convert to DDB and return that

	if (dwFlags & JPEG_LFR_DIB)
		*rethBitmap = hDIB;
	else
		{
		error = dibConvertToDDB(hDIB, hPalette, &hDDB);
		DeleteObject(hDIB);
		if (error)
			return error;

		*rethBitmap = hDDB;
		}

	return NOERROR;
	}

ALERROR DecompressJPEG (char *pImage, int iImageSize, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap)

//	DecompressJPEG
//
//	Decompresses the given block of data and returns a bitmap

	{
	ALERROR error;
	struct jpeg_decompress_struct cinfo;
	my_error_mgr jerr;
	WORD *p16BitPixel = NULL;
	BYTE *pIndexedPixel = NULL;
	HBITMAP hBitmap = NULL;
	JSAMPROW pScanline = NULL;
	BOOL bDecompressCreated = FALSE;

	//	Initialize error handler

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer))
		{
		error = ERR_FAIL;
		goto Fail;
		}

	//	Initialize the decompression object

	jpeg_create_decompress(&cinfo);
	bDecompressCreated = TRUE;

	//	Setup the source data object

	jpeg_memory_src(&cinfo, pImage, iImageSize);

	//	Read the header

	jpeg_read_header(&cinfo, TRUE);

	//	If we're outputing 8-bit indexed colors, we need to set up some stuff

	if (dwFlags & JPEG_LFR_8BITCOLOR)
		SetupColorTable(&cinfo, hPalette);

	//	Check to see if we're scaling

	if (dwFlags & JPEG_LFR_HALFSCALE)
		{
		cinfo.scale_num = 1;
		cinfo.scale_denom = 2;
		}
	else if (dwFlags & JPEG_LFR_THIRDSCALE)
		{
		cinfo.scale_num = 1;
		cinfo.scale_denom = 3;
		}
	else if (dwFlags & JPEG_LFR_QUARTERSCALE)
		{
		cinfo.scale_num = 1;
		cinfo.scale_denom = 4;
		}

	//	Begin. This will fill-in the cinfo.output_width and _height fields

	jpeg_start_decompress(&cinfo);

	//	Create an appropriate DIB

	if (dwFlags & JPEG_LFR_8BITCOLOR)
		{
#ifdef LATER
		if (error = GdiCreatePaletteDIB(cinfo.output_width,
				cinfo.output_height,
				hPalette,
				&hBitmap,
				&pIndexedPixel))
			goto Fail;
#else
		ASSERT(FALSE);
#endif
		}
	else
		{
		if (error = dibCreate16bitDIB(cinfo.output_width,
				cinfo.output_height,
				&hBitmap,
				&p16BitPixel))
			goto Fail;

		//	We start at the bottom because DIBs are stored bottom-up

		p16BitPixel += (cinfo.output_height - 1) * AlignUp(cinfo.output_width, 2);
		}

	//	Allocate a temporary scanline buffer

	pScanline = (JSAMPROW)MemAlloc(cinfo.output_width * 3 * sizeof(JSAMPLE));
	if (pScanline == NULL)
		{
		error = ERR_MEMORY;
		goto Fail;
		}

	//	Now process each scan line

	while (cinfo.output_scanline < cinfo.output_height)
		{
		int i, iScanlines;
		JSAMPLE *pSample;

		//	Load the next scanline into our buffer

		iScanlines = jpeg_read_scanlines(&cinfo, &pScanline, 1);
		if (iScanlines < 1)
			{
			error = ERR_FAIL;
			goto Fail;
			}

		//	Process differently if it is 8-bit color

		if (dwFlags & JPEG_LFR_8BITCOLOR)
			{
			//	Convert the scanline to the DIB format. The scanline will be
			//	in indexed color. We convert this to DIB
			//	8-bit indexed color.

			pSample = pScanline;
			for (i = 0; i < (int)cinfo.output_width; i++)
				{
				*pIndexedPixel = *pSample;

				//	Next

				pIndexedPixel++;
				pSample++;
				}

			//	We need to make sure that each output scan line is aligned on
			//	a DWORD boundary.

			if (cinfo.output_width % 4)
				pIndexedPixel += 4 - (cinfo.output_width % 4);
			}
		else
			{
			WORD *pPos = p16BitPixel;

			//	Convert the scanline to the DIB format. The scanline will be
			//	in RGB format, with one byte per color. We convert this to DIB
			//	16-bit format which uses a WORD for RGB using 5-6-5 bits.

			pSample = pScanline;
			for (i = 0; i < (int)cinfo.output_width; i++)
				{
				int iRed, iGreen, iBlue;

				//	Split up RGB */

				iRed = *pSample;
				iGreen = *(pSample+1);
				iBlue = *(pSample+2);

				//	Down-sample to 5 or 6 bits

				iRed = (32 * iRed) / 256;				//	5 bits of red
				iGreen = (64 * iGreen) / 256;			//	6 bits of green
				iBlue = (32 * iBlue) / 256;				//	5 bits of blue

				//	Create pixel value

				*pPos = (((WORD)iRed) << 11) + (((WORD)iGreen) << 5) + (WORD)iBlue;

				//	Next

				pPos += 1;
				pSample += 3;
				}

			//	We need to make sure that each output scan line is aligned on
			//	a DWORD boundary. We assume here that pPixel is a pointer to
			//	a WORD.

			p16BitPixel -= AlignUp(cinfo.output_width, 2);
			}
		}

	//	Clean up

	jpeg_finish_decompress(&cinfo);

	if (dwFlags & JPEG_LFR_8BITCOLOR)
		CleanUpColorTable(&cinfo);

	jpeg_destroy_decompress(&cinfo);
	bDecompressCreated = FALSE;

	MemFree(pScanline);

	//	Done with the bitmap

	*rethBitmap = hBitmap;
	
	return NOERROR;

Fail:

	if (bDecompressCreated)
		jpeg_destroy_decompress(&cinfo);

	if (hBitmap)
		DeleteObject(hBitmap);

	if (pScanline)
		MemFree(pScanline);

	return error;
	}

void SetupColorTable (struct jpeg_decompress_struct *pcinfo, HPALETTE hPalette)

//	SetupColorTable
//
//	Sets up the decompression structure to use a color table
//	specified by the given palette. Note that this routine allocates a new color
//	table and stuffs it into cinfo.colormap. The caller is responsible for
//	freeing it.

	{
    int i, iPalEntryCount;
    PALETTEENTRY PalEntry[256];
	WORD wDummy;
	JSAMPARRAY pColorTable = NULL;

	//	Get palette info
	
	GetObject(hPalette, sizeof(WORD), &wDummy);
    iPalEntryCount = (int)wDummy;
    if (iPalEntryCount > 256)
        iPalEntryCount = 256;

	GetPaletteEntries(hPalette, 0, iPalEntryCount, PalEntry);

	//	Allocate a color table. The format is an array of three pointers to arrays
	//	(one for red, green, and blue). The RGB arrays follow, each is PalEntryCount
	//	in length

	pColorTable = (JSAMPARRAY)MemAlloc(3 * sizeof(JSAMPROW) + sizeof(JSAMPLE) * 3 * iPalEntryCount);
	if (pColorTable == NULL)
		return;

	//	Set up some variables

	pcinfo->quantize_colors = TRUE;
	pcinfo->desired_number_of_colors = iPalEntryCount;
	pcinfo->dither_mode = JDITHER_FS;

	//	Set up the color table

	pColorTable[0] = (JSAMPROW)&pColorTable[3];
	pColorTable[1] = (JSAMPROW)(((char *)&pColorTable[3]) + iPalEntryCount * sizeof(JSAMPLE));
	pColorTable[2] = (JSAMPROW)(((char *)&pColorTable[3]) + 2 * iPalEntryCount * sizeof(JSAMPLE));

	for (i = 0; i < iPalEntryCount; i++)
		{
		pColorTable[0][i] = PalEntry[i].peRed;
		pColorTable[1][i] = PalEntry[i].peGreen;
		pColorTable[2][i] = PalEntry[i].peBlue;
		}

	pcinfo->actual_number_of_colors = iPalEntryCount;
	pcinfo->colormap = pColorTable;
	}

void CleanUpColorTable (struct jpeg_decompress_struct *pcinfo)

//	CleanUpColorTable
//
//	Must be called after SetupColorTable

	{
	MemFree(pcinfo->colormap);
	pcinfo->colormap = NULL;
	}
