//	CGLockedSurface.cpp
//
//	Implements CGLockedSurface class

#include <windows.h>
#include <MMSystem.h>

#include "Kernel.h"
#include "KernelObjID.h"
#include "Graphics.h"

#ifndef NO_DIRECT_DRAW

//	Forwards

void BlankChannel (CGChannelStruct *pChannel, RECT *pRect);

inline WORD Make16BitPixel (WORD wRed, WORD wGreen, WORD wBlue)
	{
	return ((wRed & 0x1f) << 10) | ((wGreen & 0x1f) << 5) | (wBlue & 0x1F);
	}

CGLockedSurface::CGLockedSurface (void) :
		m_pSurface(NULL),
		m_pDepth(NULL),
		m_pTrans(NULL)

//	CGSurface constructor

	{
	utlMemSet(&m_desc, sizeof(m_desc), 0);
	m_desc.dwSize = sizeof(m_desc);
	}

CGLockedSurface::~CGLockedSurface (void)

//	CGSurface destructor

	{
	Destroy();
	}

#ifdef DEBUG
void CGLockedSurface::Assert16BitOnly (void)

//	Assert16BitOnly
//
//	Function works only on 16-bit colors

	{
	if (m_desc.ddpfPixelFormat.dwRGBBitCount != 16)
		{
		kernelDebugLogMessage("Unable to handle %d bit color depth", m_desc.ddpfPixelFormat.dwRGBBitCount);
		DebugBreak();
		}
	}
#endif

void CGLockedSurface::Blank (RECT *pRect)

//	Blank
//
//	Fill the surface with black

	{
	DDBLTFX ddbltfx;

	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = 0;

	m_pSurface->Unlock(m_desc.lpSurface);
	m_pSurface->Blt(pRect,
			NULL,
			NULL,
			DDBLT_COLORFILL | DDBLT_WAIT,
			&ddbltfx);
	m_pSurface->Lock(NULL, &m_desc, DDLOCK_WAIT, NULL);

	//	Blank transparency

	if (m_pTrans)
		BlankChannel(m_pTrans, pRect);

	//	Blank depth

	if (m_pDepth)
		BlankChannel(m_pDepth, pRect);
	}

WORD CGLockedSurface::BlendPixel (WORD pxDest, WORD pxSource, BYTE byTrans)

//	BlendPixel
//
//	Blends the dest and the source according to byTrans

	{
	DWORD dwBlue = (128 + (255 - byTrans) * (int)((pxDest & 0x1F)) + byTrans * (int)((pxSource & 0x1F))) / 256;
	DWORD dwGreen = (128 + (255 - byTrans) * (int)((pxDest & 0x3E0) >> 5) + byTrans * (int)((pxSource & 0x3E0) >> 5)) / 256;
	DWORD dwRed = (128 + (255 - byTrans) * (int)((pxDest & 0x7c00) >> 11) + byTrans * (int)((pxSource & 0x7c00) >> 10)) / 256;

	return Make16BitPixel((WORD)dwRed, (WORD)dwGreen, (WORD)dwBlue);
	}

void CGLockedSurface::Blt (CGLockedSurface *pDest, int xDest, int yDest, int cxWidth, int cyHeight, int xSource, int ySource)

//	Blt
//
//	Blts this surface to the destination. Assumes that both are the same
//	format.

	{
	WORD *pSourcePix;
	WORD *pDestPix;
	int x, y;

	Assert16BitOnly();

	for (y = 0; y < cyHeight; y++)
		{
		pSourcePix = GetPixelPtr(xSource, ySource + y);
		pDestPix = pDest->GetPixelPtr(xDest, yDest + y);

		for (x = 0; x < cxWidth; x++)
			*pDestPix++ = *pSourcePix++;
		}
	}

ALERROR CGLockedSurface::Create (LPDIRECTDRAWSURFACE7 pSurface, RECT *pRect, CGChannelStruct *pTrans, CGChannelStruct *pDepth)

//	Create
//
//	Creates the object. This function must be called before any others

	{
	HRESULT result;

	ASSERT(m_pSurface == NULL);

	//	Lock the surface

	if (result = pSurface->Lock(pRect, &m_desc, DDLOCK_WAIT, NULL))
	//if (result = pSurface->Lock(pRect, &m_desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL))
		{
#ifdef DEBUG
		kernelDebugLogMessage("CGLockedSurface::Create: Unable to lock surface (%x)", result);
#endif
		return ERR_FAIL;
		}

	//	Remember the rect

	if (pRect)
		m_rcRect = *pRect;
	else
		{
		m_rcRect.left = 0;
		m_rcRect.top = 0;
		m_rcRect.right = m_desc.dwWidth;
		m_rcRect.bottom = m_desc.dwHeight;
		}

	//	Done

	m_pSurface = pSurface;
	m_pTrans = pTrans;
	m_pDepth = pDepth;

	return NOERROR;
	}

void CGLockedSurface::Destroy (void)

//	Destroy
//
//	Destroy the surface

	{
	if (m_pSurface)
		{
		m_pSurface->Unlock(m_desc.lpSurface);
		m_pSurface = NULL;
		}
	}

BYTE *CGLockedSurface::GetDepthPtr (int x, int y)

//	GetDepthPtr
//
//	Return a pointer to the depth map at this location

	{
	//	If we don't have a depth map, we always return NULL

	if (m_pDepth == NULL)
		return NULL;

	//	Now get the proper size

	return m_pDepth->pMap + (y * m_pDepth->cxWidthBytes) + x;
	}

BYTE *CGLockedSurface::GetTransPtr (int x, int y)

//	GetTransPtr
//
//	Return a pointer to the transparency map at this location

	{
	//	If we don't have a transparency map, we always return NULL

	if (m_pTrans == NULL)
		return NULL;

	//	Now get the proper size

	return m_pTrans->pMap + (y * m_pTrans->cxWidthBytes) + x;
	}

//	Channel functions ----------------------------------------------------

void BlankChannel (CGChannelStruct *pChannel, RECT *pRect)

//	BlankChannel
//
//	Sets channel to 0

	{
	int y;

	for (y = pRect->top; y < pRect->bottom; y++)
		{
		BYTE *pPos = pChannel->pMap + (y * pChannel->cxWidthBytes) + pRect->left;
		utlMemSet(pPos, RectWidth(pRect), 0);
		}
	}

#endif