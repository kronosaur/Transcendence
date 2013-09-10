//	CGBitmap.cpp
//
//	Implements CGBitmap class

#include <windows.h>
#include <MMSystem.h>

#include "Kernel.h"
#include "KernelObjID.h"
#include "Graphics.h"

static CObjectClass<CGBitmap>g_Class(OBJID_CGBITMAP, NULL);

CGBitmap::CGBitmap (void) : CObject(&g_Class),
		m_pMediaDb(NULL),
		m_dwUNID(0),
		m_dwMaskUNID(0),
		m_hBitmap(NULL),
		m_hBitmask(NULL)

//	CGBitmap constructor

	{
	}

CGBitmap::CGBitmap (IGMediaDb *pMediaDb, 
					  DWORD dwUNID,
					  DWORD dwMaskUNID,
					  int x, int y, int cxWidth, int cyHeight) : CObject(&g_Class),
		m_pMediaDb(pMediaDb),
		m_dwUNID(dwUNID),
		m_dwMaskUNID(dwMaskUNID),
		m_hBitmap(NULL),
		m_hBitmask(NULL)

//	CGBitmap constructor

	{
	m_rcRect.left = x;
	m_rcRect.top = y;
	m_rcRect.right = x + cxWidth;
	m_rcRect.bottom = y + cyHeight;
	}

CGBitmap::~CGBitmap (void)

//	CGBitmap destructor

	{
	Unload();
	}

void CGBitmap::Blt (HDC hDC, int x, int y, RECT *pUpdateRect)

//	Blt
//
//	Blt the bitmap to the DC

	{
	PartialBlt(hDC,
			x,
			y,
			RectWidth(&m_rcRect),
			RectHeight(&m_rcRect),
			0,
			0,
			pUpdateRect);
	}

void CGBitmap::CleanUpMask (HBITMAP hBitmap, HBITMAP hMask)

//	CleanUpMask
//
//	Cleans up the bitmap so that everything outside its mask
//	is black

	{
	HDC hDC;
	HDC hMaskDC;
	BITMAP bitmap;

	//	Get bitmap size

	GetObject(hBitmap, sizeof(BITMAP), &bitmap);

	hDC = CreateCompatibleDC(NULL);
	hMaskDC = CreateCompatibleDC(NULL);

	SelectObject(hDC, hBitmap);
	SelectObject(hMaskDC, hMask);

	//	Blt DSna

    BitBlt(hDC,
            0,
            0,
            bitmap.bmWidth,
            bitmap.bmHeight,
            hMaskDC,
           	0,
            0,
            0x220326);

	DeleteDC(hDC);
	DeleteDC(hMaskDC);
	}

ALERROR CGBitmap::Load (void)

//	Load
//
//	Make sure the bitmap is loaded

	{
	ALERROR error;

	//	If it is already loaded we have nothing to do

	if (m_hBitmap)
		return NOERROR;

	//	Load the bitmap

	if (error = m_pMediaDb->GetBitmap(m_dwUNID, &m_hBitmap))
		return error;

	//	Load the mask if necessary

	if (m_dwMaskUNID)
		{
		if (error = m_pMediaDb->GetBitmap(m_dwMaskUNID, &m_hBitmask))
			{
			m_pMediaDb->ReleaseBitmap(m_dwUNID);
			m_hBitmap = NULL;
			return error;
			}

		CleanUpMask(m_hBitmap, m_hBitmask);
		}

	return NOERROR;
	}

void CGBitmap::MaskedBlt (HDC hDestDC,
				  int x,
				  int y,
				  int cxWidth,
				  int cyHeight,
				  HDC hBitmapDC,
				  HBITMAP hBitmask,
				  int xOffset,
				  int yOffset)

//	MaskedBlt
//
//	Blts the given bitmap through a mask to the destination DC.

	{
    if (sysGetAPIFlags() & API_FLAG_MASKBLT)
        {
    	MaskBlt(hDestDC,
        		x,
        		y,
        		cxWidth,
        		cyHeight,
        		hBitmapDC,
        		xOffset,
        		yOffset,
        		hBitmask,
        		xOffset,
        		yOffset,
        		0xCCAA0000L);
    	}
    else
        {
    	HDC hMaskDC;
   		HBITMAP hOldBitmap;
        COLORREF rgbOldText;
        COLORREF rgbOldBk;
        
		//	Make sure the destination DC has the proper colors
        //	set or else the colors will be mapped */
            
		rgbOldText = SetTextColor(hDestDC, RGB(0, 0, 0));
        rgbOldBk = SetBkColor(hDestDC, RGB(255, 255, 255));
            
       	//	Blt the mask down so that it erases */
            
        hMaskDC = CreateCompatibleDC(NULL);
        hOldBitmap = (HBITMAP)SelectObject(hMaskDC, hBitmask);
        BitBlt(hDestDC,
        		x,
           		y,
            	cxWidth,
            	cyHeight,
            	hMaskDC,
            	xOffset,
            	yOffset,
            	SRCAND);
		SelectObject(hMaskDC, hOldBitmap);
        DeleteDC(hMaskDC);
            
        BitBlt(hDestDC,
            	x,
            	y,
            	cxWidth,
            	cyHeight,
            	hBitmapDC,
           		xOffset,
            	yOffset,
            	SRCPAINT);
        
        SetTextColor(hDestDC, rgbOldText);
        SetBkColor(hDestDC, rgbOldBk);
    	}
    }

void CGBitmap::PartialBlt (HDC hDC, int x, int y, int cxWidth, int cyHeight, int xOffset, int yOffset, RECT *pUpdateRect)

//	PartialBlt
//
//	Blts part of the bitmap

	{
	RECT rcFullBitmap, rcDest;

	rcFullBitmap.left = x;
	rcFullBitmap.top = y;
	rcFullBitmap.right = x + cxWidth;
	rcFullBitmap.bottom = y + cyHeight;
	if (IntersectRect(&rcDest, &rcFullBitmap, pUpdateRect))
		{
		HDC hBitmapDC = m_pMediaDb->SelectBitmap(m_hBitmap);

		if (m_hBitmask)
			{
			MaskedBlt(hDC,
					rcDest.left,
					rcDest.top,
					cxWidth,
					cyHeight,
					hBitmapDC,
					m_hBitmask,
					m_rcRect.left + xOffset + rcDest.left - x,
					m_rcRect.top + yOffset + rcDest.top - y);
			}
		else
			{
			BitBlt(hDC,
					rcDest.left,
					rcDest.top,
					cxWidth,
					cyHeight,
					hBitmapDC,
					m_rcRect.left + xOffset + rcDest.left - x,
					m_rcRect.top + yOffset + rcDest.top - y,
					SRCCOPY);
			}

		m_pMediaDb->DeselectBitmap();
		}
	}

ALERROR CGBitmap::SetBitmap (IGMediaDb *pMediaDb,
						  DWORD dwUNID,
						  DWORD dwMaskUNID,
						  int x, int y, int cxWidth, int cyHeight)

//	SetBitmap
//
//	Set bitmap info

	{
	//	Unload any previous bitmap

	Unload();

	//	Set the new variables

	m_pMediaDb = pMediaDb;
	m_dwUNID = dwUNID;
	m_dwMaskUNID = dwMaskUNID;
	m_rcRect.left = x;
	m_rcRect.top = y;
	m_rcRect.right = x + cxWidth;
	m_rcRect.bottom = y + cyHeight;

	//	Load

	return Load();
	}

void CGBitmap::Unload (void)

//	Unload
//
//	Unloads the bitmaps

	{
	if (m_hBitmap)
		{
		m_pMediaDb->ReleaseBitmap(m_dwUNID);
		m_hBitmap = NULL;
		}

	if (m_hBitmask)
		{
		m_pMediaDb->ReleaseBitmap(m_dwMaskUNID);
		m_hBitmap = NULL;
		}
	}
