//	GDI.cpp
//
//	Contains routines to manipulate basic GDI calls

#include <windows.h>
#include <MMSystem.h>

#include "Kernel.h"
#include "KernelObjID.h"
#include "Graphics.h"

void gdiFillRect (HDC hDC, const RECT *pRect, COLORREF Color)

//	gdiFillRect
//
//	Fill the rectangle with the given color

	{
	COLORREF OldColor = SetBkColor(hDC, Color);
	ExtTextOut(hDC, pRect->left, pRect->top, ETO_OPAQUE, pRect, "", 0, NULL);
	SetBkColor(hDC, OldColor);
	}
