//	JPEGUtil.h
//
//	Header for using JPEGLib library
//	Note: Requires Basics.h

#ifndef INCL_JPEG
#define INCL_JPEG

#define JPEG_LFR_8BITCOLOR				0x00000001
#define JPEG_LFR_HALFSCALE				0x00000002
#define JPEG_LFR_THIRDSCALE				0x00000004
#define JPEG_LFR_QUARTERSCALE			0x00000008
#define JPEG_LFR_DIB					0x00000010

ALERROR JPEGLoadFromFile (CString sFilename, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap);
ALERROR JPEGLoadFromMemory (char *pImage, int iSize, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap);
ALERROR JPEGLoadFromResource (HINSTANCE hInst, char *pszRes, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap);

#endif