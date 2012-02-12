//	UI.cpp
//
//	Miscellaneous UI functions

#include "Kernel.h"

ALERROR uiCopyTextToClipboard (HWND hWnd, const CString &sText)

//	uiCopyTextToClipboard
//
//	Copy the given text to the system clipboard

	{
	//	Open the clipboard

	if (!::OpenClipboard(hWnd))
		return ERR_FAIL;

	if (!::EmptyClipboard())
		{
		::CloseClipboard();
		return ERR_FAIL;
		}

	//	Put the text in an allocated global handle

	HGLOBAL hHandle = ::GlobalAlloc(GMEM_MOVEABLE, sText.GetLength() + 1);
	if (hHandle == NULL)
		{
		::CloseClipboard();
		return ERR_MEMORY;
		}

	char *pHandle = (char *)::GlobalLock(hHandle);
	utlMemCopy(sText.GetASCIIZPointer(), pHandle, sText.GetLength() + 1);
	::GlobalUnlock(hHandle);

	//	Set the clipboard data (system owns the handle)

	if (!::SetClipboardData(CF_TEXT, hHandle))
		{
		DWORD dwError = ::GetLastError();
		::GlobalFree(hHandle);
		::CloseClipboard();
		return ERR_FAIL;
		}

	//	Done

	::CloseClipboard();

	return NOERROR;
	}

void uiGetCenteredWindowRect (int cxWidth, 
							  int cyHeight, 
							  RECT *retrcRect,
							  bool bClip)

//	uiGetCenteredWindowRect
//
//	Returns the RECT of a window of the given dimensions centered on the
//	workarea of the primary screen (i.e., area not hidden by taskbar, etc.)

	{
	RECT rcWorkArea;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);

	if (bClip)
		{
		int xOffset = max(0, (RectWidth(rcWorkArea) - cxWidth) / 2);
		int yOffset = max(0, (RectHeight(rcWorkArea) - cyHeight) / 2);
		retrcRect->left = rcWorkArea.left + xOffset;
		retrcRect->top = rcWorkArea.top + yOffset;
		retrcRect->right = Min((int)(retrcRect->left + cxWidth), (int)rcWorkArea.right);
		retrcRect->bottom = Min((int)(retrcRect->top + cyHeight), (int)rcWorkArea.bottom);
		}
	else
		{
		int xOffset = (RectWidth(rcWorkArea) - cxWidth) / 2;
		int yOffset = (RectHeight(rcWorkArea) - cyHeight) / 2;
		retrcRect->left = rcWorkArea.left + xOffset;
		retrcRect->top = rcWorkArea.top + yOffset;
		retrcRect->right = retrcRect->left + cxWidth;
		retrcRect->bottom = retrcRect->top + cyHeight;
		}
	}

