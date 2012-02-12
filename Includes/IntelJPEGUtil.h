//	IntelJPEGUtil.h
//
//	Header for using Intel JPEGLib library
//	Note: Requires Kernel.h

#ifndef INCL_INTEL_JPEG
#define INCL_INTEL_JPEG

#define JPEG_LFR_DIB					0x00000010

ALERROR JPEGLoadFromFile (CString sFilename, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap);
ALERROR JPEGLoadFromMemory (char *pImage, int iSize, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap);
ALERROR JPEGLoadFromResource (HINSTANCE hInst, char *pszRes, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap);

#endif