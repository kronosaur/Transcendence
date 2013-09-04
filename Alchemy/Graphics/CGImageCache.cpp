//	CGImageCache.cpp
//
//	Implements CGImageCache class

#include <windows.h>
#include <MMSystem.h>

#include "Kernel.h"
#include "KernelObjID.h"
#include "Graphics.h"

#ifndef NO_DIRECT_DRAW

CGImageCache::CGImageCache (CGScreen *pScreen, IGMediaDb *pMediaDb) : 
		m_pScreen(pScreen),
		m_pMediaDb(pMediaDb),
		m_Images(sizeof(IMAGEDATASTRUCT), 10)

//	CGImageCache constructor

	{
	}

CGImageCache::~CGImageCache (void)

//	CGImageCache destructor

	{
	FlushCache();
	}

ALERROR CGImageCache::AddImage (DWORD dwUNID, LPDIRECTDRAWSURFACE7 pSurface, CGChannelStruct *pTrans, HBITMAP hDepth, int *retiIndex)

//	AddImage
//
//	Add an image to the cache

	{
	ALERROR error;
	int iIndex;
	IMAGEDATASTRUCT data;

	data.pSurface = pSurface;
	data.pTrans = pTrans;
	data.hDepth = hDepth;

	//	Add it to the list of surfaces

	if (error = m_Images.AppendStruct(&data, &iIndex))
		return error;

	//	Add it to our UNID table

	if (error = m_UNIDMap.AddEntry(dwUNID, iIndex))
		return error;

	//	Done

	if (retiIndex)
		*retiIndex = iIndex;

	return NOERROR;
	}

ALERROR CGImageCache::ConvertToChannel (HBITMAP hTransparency, CGChannelStruct **retpTrans)

//	ConvertToChannel
//
//	Converts an 8-bit grayscale bitmap to a channel format

	{
	BITMAP bm;
	CGChannelStruct *pTrans;
	HDC hDC;
	BYTE bmibuffer[sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD)];
	BITMAPINFO *bmi = (BITMAPINFO *)bmibuffer;
	RGBQUAD *pColors = (RGBQUAD *)&bmi[1];

	//	Get some basic info about the bitmap

	GetObject(hTransparency, sizeof(BITMAP), &bm);

	//	Allocate a buffer

	pTrans = (CGChannelStruct *)MemAlloc(sizeof(CGChannelStruct) + bm.bmWidthBytes * bm.bmHeight);
	if (pTrans == NULL)
		return ERR_MEMORY;

	//	Initialize some stuff

	pTrans->cxWidthBytes = bm.bmWidthBytes;
	pTrans->cxWidth = bm.bmWidth;
	pTrans->cyHeight = bm.bmHeight;
	pTrans->pMap = (BYTE *)&pTrans[1];

	//	Create a BITMAPINFO structure describing how we want the bits
	//	to be returned to us

	utlMemSet(bmi, sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD), 0);
	bmi->bmiHeader.biSize = sizeof(BITMAPINFO);
	bmi->bmiHeader.biWidth = bm.bmWidth;
	bmi->bmiHeader.biHeight = -bm.bmHeight;	//	negative means top-down
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biBitCount = 8;
	bmi->bmiHeader.biCompression = BI_RGB;

	hDC = CreateCompatibleDC(NULL);
	GetDIBits(hDC, hTransparency, 0, bm.bmHeight, pTrans->pMap, bmi, DIB_RGB_COLORS);
	DeleteDC(hDC);

	//	Color table

#if 0
	int i;
	for (i = 0; i < 256; i++)
		kernelDebugLogMessage("TEST: Color %d = %d,%d,%d",
				i,
				pColors[i].rgbRed,
				pColors[i].rgbGreen,
				pColors[i].rgbBlue);

	//	Convert from palette indeces to grayscale values

	for (i = 0; i < pTrans->cxWidthBytes * pTrans->cyHeight; i++)
		{
		if ((i % pTrans->cxWidthBytes) == 32)
			kernelDebugLogMessage("TEST: Color index %d", pTrans->pMap[i]);

		pTrans->pMap[i] = pColors[pTrans->pMap[i]].rgbBlue;
		}
#endif

	//	Done

	*retpTrans = pTrans;

	return NOERROR;
	}

int CGImageCache::FindImage (DWORD dwUNID)

//	FindImage
//
//	Finds the given image by UNID

	{
	int iIndex;

	if (m_UNIDMap.Find((int)dwUNID, &iIndex) != NOERROR)
		return -1;

	return iIndex;
	}

void CGImageCache::FlushCache (void)

//	FlushCache
//
//	Remove all images from the cache

	{
	int i;

	//	Release all surfaces

	for (i = 0; i < GetImageCount(); i++)
		{
		IMAGEDATASTRUCT *pData = GetImage(i);

		pData->pSurface->Release();
		if (pData->hDepth)
			DeleteObject(pData->hDepth);
		if (pData->pTrans)
			MemFree(pData->pTrans);
		}

	//	Empty the lists

	m_UNIDMap.RemoveAll();
	m_Images.RemoveAll();
	}

ALERROR CGImageCache::LoadBitmapImage (DWORD dwImageUNID, DWORD dwTransparencyUNID, DWORD dwDepthUNID, int *retiIndex)

//	LoadBitmapImage
//
//	Loads an image into the cache. This should only be called once
//	when the cache is initialized or when an image is first requested.

	{
	ALERROR error;
	HRESULT result;
	HBITMAP hBitmap;
	DDSURFACEDESC ddsd;
	BITMAP bm;
	LPDIRECTDRAWSURFACE7 pSurface;
	HDC hDC, hBitmapDC;
	HBITMAP hOldBitmap;
	HBITMAP hDepth;
	CGChannelStruct *pTrans = NULL;

	//	Load the bitmap from the image db

	if (error = m_pMediaDb->LoadBitmap(dwImageUNID, &hBitmap))
		return error;

	//	Get some info from bitmap

	GetObject(hBitmap, sizeof(bm), &bm);

	//	Create a surface of the proper size

	utlMemSet(&ddsd, sizeof(ddsd), 0);
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = bm.bmWidth;
	ddsd.dwHeight = bm.bmHeight;

	if (m_pScreen->GetDD()->CreateSurface(&ddsd, &pSurface, NULL) != DD_OK)
		{
		DeleteObject(hBitmap);
		return ERR_FAIL;
		}

	if (result = pSurface->GetDC(&hDC) != DD_OK)
		{
		pSurface->Release();
		DeleteObject(hBitmap);
		return ERR_FAIL;
		}

	//	Blt the bitmap onto the surface

	hBitmapDC = CreateCompatibleDC(NULL);
	hOldBitmap = (HBITMAP)SelectObject(hBitmapDC, hBitmap);

	BitBlt(hDC,
			0,
			0,
			bm.bmWidth,
			bm.bmHeight,
			hBitmapDC,
			0,
			0,
			SRCCOPY);

	SelectObject(hBitmapDC, hOldBitmap);
	DeleteDC(hBitmapDC);
	DeleteObject(hBitmap);

	pSurface->ReleaseDC(hDC);

	//	Load the transparency bitmap

	if (dwTransparencyUNID)
		{
		HBITMAP hTransparency;

		if (error = m_pMediaDb->LoadBitmap(dwTransparencyUNID, &hTransparency))
			{
			pSurface->Release();
			return error;
			}

		//	Convert this bitmap into an 8-bit map

		error = ConvertToChannel(hTransparency, &pTrans);
		DeleteObject(hTransparency);
		if (error)
			{
			pSurface->Release();
			return error;
			}
		}

	//	Load the depth bitmap

	if (dwDepthUNID)
		{
		if (error = m_pMediaDb->LoadBitmap(dwDepthUNID, &hDepth))
			{
			if (pTrans)
				MemFree(pTrans);
			pSurface->Release();
			return error;
			}
		}
	else
		hDepth = NULL;

	//	Add the surface to our cache

	if (error = AddImage(dwImageUNID, pSurface, pTrans, hDepth, retiIndex))
		{
		if (hDepth)
			DeleteObject(hDepth);
		if (pTrans)
			MemFree(pTrans);
		pSurface->Release();
		return ERR_FAIL;
		}

	return NOERROR;
	}

ALERROR CGImageCache::LockImageSurface (int iIndex, CGLockedSurface *pLockedSurface)

//	LockImageSurface
//
//	Locks the image and fill-in the locked surface object

	{
	IMAGEDATASTRUCT *pData = GetImage(iIndex);
	return pLockedSurface->Create(pData->pSurface, NULL, pData->pTrans, NULL);
	}

void CGImageCache::UnlockImageSurface (CGLockedSurface *pLockedSurface)

//	UnlockImageSurface
//
//	Unlocks a surface locked by LockImageSurface

	{
	pLockedSurface->Destroy();
	}

#endif