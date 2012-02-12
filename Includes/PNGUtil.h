//	PNGUtil.h
//
//	PNG Utilities

#ifndef INCL_PNGUTIL
#define INCL_PNGUTIL

#define PNG_LFR_DIB					0x00000001

ALERROR PNGLoadFromFile (const CString &sFilename, 
						 DWORD dwFlags, 
						 HPALETTE hPalette, 
						 HBITMAP *rethBitmap,
						 HBITMAP *rethBitmask);

#endif