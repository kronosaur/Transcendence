//	CLWindow.cpp
//
//	Implements extensions to the CodeChain interpreter that
//	are specific to windowing system used by the LibraryLink

#include "Alchemy.h"
#include "TerraFirma.h"
#include "WindowsExt.h"

LONG APIENTRY CLWindowWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);

static BOOL g_bRegistered = FALSE;
static char g_szClassName[] = "TF_CLWindow";
static CObjectClass<CLWindow>g_Class(OBJID_CLWINDOW, NULL);

//	Inlines

inline CLWindow *GetThis (HWND hWnd) { return (CLWindow *)GetWindowLong(hWnd, GWL_USERDATA); }

CLWindow::CLWindow (void) : ICCAtom(&g_Class)

//	CLWindow constructor

	{
	}

CLWindow::CLWindow (CLWindowManager *pWM) : ICCAtom(&g_Class),
		m_hWnd(NULL),
		m_pController(NULL),
		m_pWM(pWM),
		m_AreaList(TRUE),
		m_hBackstore(NULL),
		m_hdcBackstore(NULL),
		m_hPalette(NULL),
		m_hdcBitmap(NULL)

//	CLWindow constructor

	{
	}

int CLWindow::AddArea (CLArea *pArea)

//	AddArea
//
//	Adds an area to the window. This method should only be
//	called by the create area calls.

	{
	ALERROR error;
	int iIndex;

	if (error = m_AreaList.AppendObject(pArea, &iIndex))
		return -1;

	//	If the window is visible, repaint

	if (IsVisible())
		{
		RECT rcRect;
		pArea->GetRect(&rcRect);
		UpdateRect(&rcRect);
		}

	return iIndex;
	}

void CLWindow::BringAreaToFront (CLArea *pArea)

//	BringAreaToFront
//
//	Brings the given area to the front of the area list

	{
	}

HRGN CLWindow::ClipRect (HDC hDC, RECT *pRect)

//	ClipRect
//
//	Clip out everything except the area. You must call UnclipRect
//	when you are done with the clip region

	{
	HRGN hOldClipRgn;

	//	Get the current clipping region

	hOldClipRgn = CreateRectRgn(0, 0, 0, 0);
	if (GetClipRgn(hDC, hOldClipRgn) == 0)
		{
		DeleteObject(hOldClipRgn);
		hOldClipRgn = NULL;
		}

	//	Clip out everything except the area

	IntersectClipRect(hDC, pRect->left, pRect->top, pRect->right, pRect->bottom);

	//	Return the old clip region

	return hOldClipRgn;
	}

ICCItem *CLWindow::Clone (CCodeChain *pCC)

//	Clone
//
//	Clone this item

	{
	ASSERT(FALSE);
	return pCC->CreateNil();
	}

ALERROR CLWindow::CreateBackstore (int cxWidth, int cyHeight)

//	CreateBackstore
//
//	Create a bitmap to do off-screen painting

	{
	HDC hDisplayDC;
	HBITMAP hBackstore;
	HDC hdcBackstore;

	//	Create the bitmap

	hDisplayDC = GetDC(NULL);
	hBackstore = CreateBitmap(cxWidth, cyHeight,
			(BYTE)GetDeviceCaps(hDisplayDC, PLANES),
			(BYTE)GetDeviceCaps(hDisplayDC, BITSPIXEL), NULL);
	ReleaseDC(NULL, hDisplayDC);
	if (hBackstore == NULL)
		return ERR_FAIL;

	//	Create the DC to use for the bitmap

	hdcBackstore = CreateCompatibleDC(NULL);
	if (hdcBackstore == NULL)
		{
		DeleteObject(hBackstore);
		return ERR_FAIL;
		}

	//	Select the palette

	if (m_hPalette)
		{
		SelectPalette(hdcBackstore, m_hPalette, FALSE);
		RealizePalette(hdcBackstore);
		}

	//	Select the bitmap into the DC

	SelectObject(hdcBackstore, hBackstore);

	//	If we're already using an off-screen bitmap, delete it now

	if (m_hdcBackstore)
		{
		DeleteDC(m_hdcBackstore);
		m_hdcBackstore = NULL;
		DeleteObject(m_hBackstore);
		m_hBackstore = NULL;
		}

	//	Switch over

	m_hBackstore = hBackstore;
	m_hdcBackstore = hdcBackstore;

	//	Paint the bitmap

	if (IsVisible())
		PaintRect(m_hdcBackstore, NULL);

	return NOERROR;
	}

ICCItem *CLWindow::CreateHWND (CCodeChain *pCC, CLWindowOptions *pOptions, RECT *pRect)

//	CreateHWND
//
//	Create the window

	{
	DWORD dwStyle;

	ASSERT(m_hWnd == NULL);

	//	Register the class, if we haven't already

	if (!g_bRegistered)
		{
		WNDCLASS  wc;

		//	Register the class

		utlMemSet(&wc, sizeof(wc), 0);
		wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = (WNDPROC)CLWindowWndProc;
		wc.hInstance = m_pWM->GetTF()->GetInstance();
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = GetStockObject(BLACK_BRUSH);
		wc.lpszClassName = g_szClassName;

		if (!RegisterClass(&wc))
			return pCC->CreateError(LITERAL("Unable to register CLWindow."), NULL);
		}

	//	Now create the window

	dwStyle = WS_CHILD | WS_CLIPSIBLINGS;

	m_hWnd = CreateWindow(
			g_szClassName,
			"",
			dwStyle,
			pRect->left, pRect->right, RectWidth(pRect), RectHeight(pRect),
			m_pWM->GetTF()->GetHWND(),
			(HMENU)1,
			m_pWM->GetTF()->GetInstance(),
			this);
	if (m_hWnd == NULL)
		return pCC->CreateMemoryError();

	return pCC->CreateTrue();
	}

ICCItem *CLWindow::CreateItem (CCodeChain *pCC,
							   CLWindowOptions *pOptions, 
							   CLSizeOptions *pSize, 
							   ICCItem *pController)

//	CreateItem
//
//	Initializes the item after creating it

	{
	ICCItem *pError;
	RECT rcParent;
	RECT rcRect;

	//	Initialize

	m_fBackstore = pOptions->fBackstore;
	m_SizeOptions = *pSize;
	if (pController)
		m_pController = pController->Reference();

	//	Get the size of our parent window

	m_pWM->GetRect(&rcParent);

	//	Figure out how big we should be

	m_SizeOptions.CalcSize(&rcParent, &rcRect);

	//	Create the new window. The window is hidden when we
	//	first create it.

	pError = CreateHWND(pCC, pOptions, &rcRect);
	if (pError->IsError())
		return pError;

	pError->Discard(pCC);

	//	Create a temporary DC

	ASSERT(m_hdcBitmap == NULL);
	m_hdcBitmap = CreateCompatibleDC(NULL);

	//	Done

	return pCC->CreateTrue();
	}

void CLWindow::DestroyArea (CLArea *pArea)

//	DestroyArea
//
//	Destroys the area and removes it from the list

	{
	}

void CLWindow::DestroyItem (CCodeChain *pCC)

//	DestroyItem
//
//	Destroy the item

	{
	//	Destroy the window

	if (m_hWnd)
		DestroyWindow(m_hWnd);

	//	Release reference to the controller

	if (m_pController)
		m_pController->Discard(pCC);

	//	Remove from the window manager

	m_pWM->DestroyLWindow(this);

	//	Delete resources

	if (m_hdcBitmap)
		DeleteDC(m_hdcBitmap);

	//	Done

	delete this;
	}

CLArea *CLWindow::FindArea (int iID)

//	FindArea
//
//	Returns a pointer to the area by ID. If the area is not found,
//	returns NULL

	{
	int i;

	for (i = 0; i < GetAreaCount(); i++)
		{
		CLArea *pArea = GetArea(i);
		if (pArea->GetID() == iID)
			return pArea;
		}

	return NULL;
	}

CLWindowManager *CLWindow::GetWM (void)

//	GetWM
//
//	Returns Window Manager

	{
	return m_pWM;
	}

void CLWindow::HideWindow (void)

//	HideWindow
//
//	Hide the window

	{
	}

void CLWindow::PaintRect (HDC hDC, RECT *pUpdateRect)

//	PaintRect
//
//	Paint the given rect on the DC

	{
	int i;
	RECT rcUpdate;
	int iStartArea;

	//	Compute the update rect

	if (pUpdateRect == NULL)
		GetClientRect(m_hWnd, &rcUpdate);
	else
		rcUpdate = *pUpdateRect;

	//	Look for the top-most opaque area

	iStartArea = -1;
	for (i = 0; i < GetAreaCount(); i++)
		{
		CLArea *pArea = GetArea(i);
		RECT rcRect;

		//	If this area is transparent, keep going

		if (pArea->IsTransparent() || pArea->IsHidden())
			continue;

		//	If this area doesn't cover the entire rect, then we keep going

		pArea->GetRect(&rcRect);
		if (!RectEncloses(&rcRect, &rcUpdate))
			continue;

		//	Since this area is opaque and covert the entire update region,
		//	we don't need to paint below this area

		iStartArea = i;
		}

	//	If necessary, paint the background

	if (iStartArea == -1)
		{
		gdiFillRect(hDC, &rcUpdate, RGB(0, 0, 0));
		iStartArea = 0;
		}

	//	We iterate over all the areas and ask them to paint

	for (i = iStartArea; i < GetAreaCount(); i++)
		{
		CLArea *pArea = GetArea(i);
		RECT rcRect;

		pArea->GetRect(&rcRect);
		if (!pArea->IsHidden() && IntersectRect(&rcRect, &rcRect, &rcUpdate))
			{
			HRGN hOldClipRgn = ClipRect(hDC, &rcRect);
			pArea->Paint(hDC, &rcRect);
			UnclipRect(hDC, hOldClipRgn);
			}
		}
	}

CString CLWindow::Print (CCodeChain *pCC)

//	Print
//
//	Render as text

	{
	return LITERAL("[Library Link Window]");
	}

void CLWindow::SendAreaToBack (CLArea *pArea)

//	SendAreaToBack
//
//	Sends the area to the back of the list

	{
	}

void CLWindow::SetSize (CLSizeOptions *pOptions)

//	SetSize
//
//	Sets the size of the window and how it reacts to
//	changes in its parent's size

	{
	RECT rcRect;
	RECT rcNewRect;

	//	Remember our new size settings

	if (pOptions)
		m_SizeOptions = *pOptions;

	//	Get the size of our parent window

	m_pWM->GetRect(&rcRect);

	//	Figure out how big we should be

	m_SizeOptions.CalcSize(&rcRect, &rcNewRect);

	//	Change our size

	MoveWindow(m_hWnd,
			rcNewRect.left,
			rcNewRect.top,
			RectWidth(&rcNewRect),
			RectHeight(&rcNewRect),
			TRUE);
	}

void CLWindow::ShowWindow (void)

//	ShowWindow
//
//	Shows the window

	{
	//	Make sure we're up to date

	if (HasBackstore())
		{
		RECT rcClient;

		GetClientRect(m_hWnd, &rcClient);
		if (CreateBackstore(RectWidth(&rcClient), RectHeight(&rcClient)) != NOERROR)
			m_fBackstore = FALSE;
		else
			PaintRect(m_hdcBackstore, NULL);
		}

	//	Show the window

	::ShowWindow(m_hWnd, SW_SHOW);
	::UpdateWindow(m_hWnd);
	}

ICCItem *CLWindow::StreamItem (CCodeChain *pCC, IWriteStream *pStream)

//	StreamItem
//
//	Stream the sub-class specific data

	{
	ASSERT(FALSE);
	return pCC->CreateError(LITERAL("CLWindow cannot be streamed"), NULL);
	}

void CLWindow::UnclipRect (HDC hDC, HRGN hOldClipRgn)

//	EndAreaClip
//
//	Undo area clip. After this routine is called, hOldClipRgn is freed

	{
	SelectClipRgn(hDC, hOldClipRgn);
	if (hOldClipRgn)
		DeleteObject(hOldClipRgn);
	}

ICCItem *CLWindow::UnstreamItem (CCodeChain *pCC, IReadStream *pStream)

//	UnstreamItem
//
//	Unstream the sub-class specific data

	{
	ASSERT(FALSE);
	return pCC->CreateError(LITERAL("Unable to load CLWindow class"), NULL);
	}

void CLWindow::UpdateRect (RECT *pUpdateRect)

//	UpdateRect
//
//	Invalidates the given rect and repaints it

	{
	//	If we're not visible, blow out of here

	if (!IsVisible())
		return;

	//	Update the backstore, if necessary

	if (HasBackstore())
		PaintRect(m_hdcBackstore, pUpdateRect);

	InvalidateRect(m_hWnd, pUpdateRect, FALSE);
	}

LONG CLWindow::WMPaint (void)

//	WMPaint
//
//	Handle WM_PAINT message

	{
	PAINTSTRUCT ps;
	HDC hDC;
	HPALETTE hOldPalette = NULL;

	hDC = BeginPaint(m_hWnd, &ps);

	if (m_hPalette)
		{
		hOldPalette = SelectPalette(hDC, m_hPalette, FALSE);
		RealizePalette(hDC);
		}

	//	If we're using an off-screen bitmap, paint from the off-
	//	screen bitmap. Otherwise, paint directly on the DC.

	if (HasBackstore())
		{
		BitBlt(hDC,
				ps.rcPaint.left,
				ps.rcPaint.top,
				RectWidth(&ps.rcPaint),
				RectHeight(&ps.rcPaint),
				m_hdcBackstore,
				ps.rcPaint.left,
				ps.rcPaint.top,
				SRCCOPY);
		}
	else
		PaintRect(hDC, &ps.rcPaint);

	//	Clean up

	if (hOldPalette)
		SelectPalette(hDC, hOldPalette, FALSE);

	EndPaint(m_hWnd, &ps);
	return 0;
	}

LONG CLWindow::WMSize (int cxWidth, int cyHeight, int iSizeType)

//	WMSize
//
//	Handle WM_SIZE

	{
	int i;
	RECT rcRect;

	//	If we've got an off-screen bmp, we need to resize it

	if (HasBackstore())
		{
		if (CreateBackstore(cxWidth, cyHeight) != NOERROR)
			m_fBackstore = FALSE;
		}

	//	Resize any areas

	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = cxWidth;
	rcRect.bottom = cyHeight;

	//	We iterate over all the areas and ask them to resize

	for (i = 0; i < GetAreaCount(); i++)
		{
		CLArea *pArea = GetArea(i);
		pArea->WindowSize(&rcRect);
		}

	return 0;
	}

LONG APIENTRY CLWindowWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)

//	GenericWndProc
//
//	WndProc for generic window

	{
	switch (message)
		{
		case WM_CREATE:
			{
			LPCREATESTRUCT pCreate = (LPCREATESTRUCT)lParam;
			CLWindow *pWindow = (CLWindow *)pCreate->lpCreateParams;

			//	Store the object pointer in the user struct

			SetWindowLong(hWnd, GWL_USERDATA, (LONG)pWindow);

			//	Set up the m_hWnd variable as soon as possible

			pWindow->m_hWnd = hWnd;

			//	Done

			return 0;
			}

		case WM_PAINT:
			return GetThis(hWnd)->WMPaint();

		case WM_SIZE:
			return GetThis(hWnd)->WMSize((int)LOWORD(lParam), (int)HIWORD(lParam), (int)wParam);

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

