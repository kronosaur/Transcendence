// calcimage.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Alchemy.h"

inline DWORD GetRed (DWORD dwPixel) { return (dwPixel & 0x00FF0000) >> 16; }
inline DWORD GetGreen (DWORD dwPixel) { return (dwPixel & 0x0000FF00) >> 8; }
inline DWORD GetBlue (DWORD dwPixel) { return (dwPixel & 0x000000FF); }
inline DWORD MakeRed (DWORD dwRed) { return (dwRed << 16); }
inline DWORD MakeGreen (DWORD dwGreen) { return (dwGreen << 8); }
inline DWORD MakeBlue (DWORD dwBlue) { return dwBlue; }

DWORD Compose (DWORD dwImage, DWORD dwMask)
	{
	if (dwMask == 0x00)
		return 0x00;
	else
		{
		DWORD dwResult = dwImage * 0xFF / dwMask;
		if (dwResult > 0xFF)
			dwResult = 0xFF;
		return dwResult;
		}
	}

void DoIt (char *pImageFile, char *pMaskFile)
	{
	HBITMAP hImage = (HBITMAP)::LoadImage(NULL, pImageFile, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if (hImage == NULL)
		{
		printf("calcimage: Unable to load '%s'\n", pImageFile);
		return;
		}

	DIBSECTION ImageInfo;
	::GetObject(hImage, sizeof(ImageInfo), &ImageInfo);
	DWORD *pImageBits = (DWORD *)ImageInfo.dsBm.bmBits;

	HBITMAP hMask = (HBITMAP)::LoadImage(NULL, pMaskFile, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if (hMask == NULL)
		{
		printf("calcimage: Unable to load '%s'\n", pMaskFile);
		return;
		}

	DIBSECTION MaskInfo;
	::GetObject(hMask, sizeof(MaskInfo), &MaskInfo);
	DWORD *pMaskBits = (DWORD *)MaskInfo.dsBm.bmBits;

	//	Create a new DIB of the same size

	BITMAPINFOHEADER bmi;
	::ZeroMemory(&bmi, sizeof(bmi));
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biWidth = ImageInfo.dsBmih.biWidth;
	bmi.biHeight = ImageInfo.dsBmih.biHeight;
	bmi.biPlanes = 1;
	bmi.biBitCount = 32;
	bmi.biCompression = BI_RGB;

	//	Create the DIB section

	DWORD *pResultBits;
	HBITMAP hResult = ::CreateDIBSection(NULL,
			(BITMAPINFO *)&bmi,
			DIB_RGB_COLORS,
			(void **)&pResultBits,
			NULL,
			0);

	//	Iterate over the result and compose it based on image and mask data

	DWORD *pResultPos = pResultBits;
	DWORD *pResultEnd = pResultPos + (bmi.biWidth * bmi.biHeight);
	BYTE *pImagePos = (BYTE *)pImageBits;
	BYTE *pMaskPos = (BYTE *)pMaskBits;

	while (pResultPos < pResultEnd)
		{
		DWORD dwRed = MakeBlue(Compose(*pImagePos++, *pMaskPos++));
		DWORD dwGreen = MakeGreen(Compose(*pImagePos++, *pMaskPos++));
		DWORD dwBlue = MakeRed(Compose(*pImagePos++, *pMaskPos++));

		*pResultPos = dwRed | dwGreen | dwBlue;

		pResultPos++;
		}

	//	Save result out to file Result.bmp

	CFileWriteStream Output(CString("Result.bmp"), FALSE);
	BITMAPFILEHEADER bmfh;

	::ZeroMemory(&bmfh, sizeof(bmfh));
	bmfh.bfType = 0x4d42;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) 
			+ (sizeof(DWORD) * (bmi.biWidth * bmi.biHeight));
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	Output.Create();
	Output.Write((char *)&bmfh, sizeof(bmfh), NULL);
	Output.Write((char *)&bmi, sizeof(bmi), NULL);
	Output.Write((char *)pResultBits, (sizeof(DWORD) * (bmi.biWidth * bmi.biHeight)), NULL);

	Output.Close();
	}

int main (int argc, char* argv[])
	{
	if (!kernelInit())
		{
		printf("calcimage: Unable to initialize Alchemy.\n");
		return -1;
		}

	if (argc < 2)
		{
		printf("calcimage image.bmp mask.bmp\n");
		return 0;
		}

	DoIt(argv[1], argv[2]);

	kernelCleanUp();
	return 0;
	}

