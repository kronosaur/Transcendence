//	Load.cpp
//
//	Implements loading JPEGs with Intel JPEG library

#include <windows.h>

#include "Kernel.h"
#include "KernelObjID.h"
#include "Graphics.h"
#include "IntelJPEGUtil.h"

#include "ijl.h"

ALERROR JPEGLoadFromFile (CString sFilename, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap)

//	JPEGLoadFromFile
//
//	Load from a JPEG file

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

ALERROR JPEGLoadFromMemory (char *pImage, int iSize, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap)

//	JPEGLoadFromMemory
//
//	Load from a memory stream

	{
	ALERROR error;
	IJLERR jerr;

	//	Initialize library

	JPEG_CORE_PROPERTIES jcprops;
	jerr = ijlInit(&jcprops);
	if (jerr != IJL_OK)
		return ERR_FAIL;

	//	Point to the buffer

	jcprops.JPGFile = NULL;
	jcprops.JPGBytes = (BYTE *)pImage;
	jcprops.JPGSizeBytes = iSize;

	//	Read parameters

	jerr = ijlRead(&jcprops, IJL_JBUFF_READPARAMS);
	if (jerr != IJL_OK)
		{
		ijlFree(&jcprops);
		return ERR_FAIL;
		}

	DWORD width = jcprops.JPGWidth;
	DWORD height = jcprops.JPGHeight;
	DWORD nchannels = 3;	//	24-bits
	DWORD dib_line_width = width * nchannels;
	DWORD dib_pad_bytes = IJL_DIB_PAD_BYTES(width,nchannels);

	//	Allocate a dib

	HBITMAP hBitmap;
	BYTE *p24BitPixel;
	if (error = dibCreate24bitDIB(width,
			height,
			0,
			&hBitmap,
			&p24BitPixel))
		{
		ijlFree(&jcprops);
		return error;
		}

	//	Setup the library to load into the dib format

	jcprops.DIBWidth = width;
	jcprops.DIBHeight = -(int)height;
	jcprops.DIBChannels = nchannels;
	jcprops.DIBColor = IJL_BGR;
	jcprops.DIBPadBytes = dib_pad_bytes;
	jcprops.DIBBytes = p24BitPixel;

	//	Setup the color space

	switch (jcprops.JPGChannels)
		{
		case 1:
			jcprops.JPGColor = IJL_G;
			break;

		case 3:
			jcprops.JPGColor = IJL_YCBCR;
			break;

		default:
			{
			jcprops.DIBColor = (IJL_COLOR)IJL_OTHER;
			jcprops.JPGColor = (IJL_COLOR)IJL_OTHER;
			}
		}

	//	Load the data in the buffer

	jerr = ijlRead(&jcprops, IJL_JBUFF_READWHOLEIMAGE);
	if (jerr != IJL_OK)
		{
		::DeleteObject(hBitmap);
		ijlFree(&jcprops);
		return ERR_FAIL;
		}

	//	Done

	ijlFree(&jcprops);

	//	If we want a DIB, just return that; otherwise,
	//	convert to DDB and return that

	if (dwFlags & JPEG_LFR_DIB)
		*rethBitmap = hBitmap;
	else
		{
		HBITMAP hDDB;
		error = dibConvertToDDB(hBitmap, hPalette, &hDDB);
		::DeleteObject(hBitmap);
		if (error)
			return error;

		*rethBitmap = hDDB;
		}

	return NOERROR;
	}

ALERROR JPEGLoadFromResource (HINSTANCE hInst, char *pszRes, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap)

//	JPEGLoadFromResource
//
//	Load from a resource

	{
	HRSRC hRes;
	HGLOBAL hGlobalRes;
	void *pImage;
	int iSize;

	hRes = FindResource(hInst, pszRes, "JPEG");
	if (hRes == NULL)
		return ERR_NOTFOUND;

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
