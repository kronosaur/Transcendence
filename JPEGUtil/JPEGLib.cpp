//	JPEGWrapper.cpp
//
//	This is a wrapper for the IJG JPEG library

#include <Windows.h>
#include "Basics.h"
#include "JPEG.hpp"

#include "JPEGLib.h"

STATUS JPEGLoadFromResource (HINSTANCE hInst, char *pszRes, DWORD dwFlags, HBITMAP *rethBitmap)

//	JPEGLoadFromResource
//
//	Loads a JPEG from a resource and returns a bitmap

	{
	return NOERROR;
	}

STATUS DecompressJPEG (void *pImage, DWORD dwFlags, HBITMAP *rethBitmap)

//	DecompressJPEG
//
//	Decompresses the given block of data and returns a bitmap

	{
	STATUS error;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	//	Initialize the decompression object

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	return NOERROR;
	}