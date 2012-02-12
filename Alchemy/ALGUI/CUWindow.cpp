//	CUWindow.cpp
//
//	CUWindow object

#include "Alchemy.h"
#include "ALGUI.h"

LONG APIENTRY CUWindowWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);

static BOOL g_bRegistered = FALSE;
static char g_szClassName[] = "ALGUI_CUWindow";
static CObjectClass<CUWindow>g_Class(OBJID_CUWINDOW, NULL);

//	Inlines

inline CUWindow *GetThis (HWND hWnd) { return (CUWindow *)GetWindowLong(hWnd, GWL_USERDATA); }

CUWindow::CUWindow (void) : CObject(&g_Class)

//	CUWindow constructor

	{
	}

CUWindow::CUWindow (CUApplication *pApp, IUController *pController, int iID, DWORD dwFlags) : CObject(&g_Class),
		m_pApp(pApp),
		m_hWnd(NULL),
		m_pController(pController),
		m_iID(iID),
		m_dwFlags(dwFlags),
		m_dwState(0),
		m_pFrame(NULL),
		m_pMouseCapture(NULL),
		m_pInputFocus(NULL),
		m_pfQueryCloseMsg(NULL)

//	CUWindow constructor

	{
	}

CUWindow::~CUWindow (void)

//	CUWindow destructor

	{
	//	Destroy the frame

	if (m_pFrame)
		{
		delete m_pFrame;
		m_pFrame = NULL;
		}

	//	Destroy window

	if (m_hWnd)
		{
		//	Disconnect m_hWnd so that we don't try to delete
		//	ourselves again.
		HWND hWnd = m_hWnd;
		m_hWnd = NULL;
		DestroyWindow(hWnd);
		}
	}

void CUWindow::Animate (void)

//	Animate
//
//	Called by the main loop each frame

	{
	m_pFrame->Animate();

	//	Need to update the window so that we see the results immediately
	UpdateWindow(m_hWnd);
	}

ALERROR CUWindow::Boot (void)

//	Boot
//
//	Must be called after the window object is created

	{
	ALERROR error;
	DWORD dwStyle;
	int xPos, yPos, cxWidth, cyHeight;

	ASSERT(m_hWnd == NULL);

	//	Register the class, if we haven't already

	if (!g_bRegistered)
		{
		WNDCLASS  wc;

		//	Register the class

		utlMemSet(&wc, sizeof(wc), 0);
		wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = (WNDPROC)CUWindowWndProc;
		wc.hInstance = m_pApp->GetInstance();
		wc.hIcon = LoadIcon(m_pApp->GetInstance(), m_pApp->GetIconRes());
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszClassName = g_szClassName;

		if (!RegisterClass(&wc))
			return ERR_FAIL;
		}

	//	Figure out some initial parameters

	if (IsFullScreen())
		{
		dwStyle = WS_POPUP | WS_CLIPSIBLINGS;
		xPos = 0;
		yPos = 0;
		cxWidth = GetSystemMetrics(SM_CXSCREEN);
		cyHeight = GetSystemMetrics(SM_CYSCREEN);
		}
	else
		{
		dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS;
		xPos = CW_USEDEFAULT;
		yPos = 0;
		cxWidth = CW_USEDEFAULT;
		cyHeight = 0;
		}

	//	Create the actual window

	m_hWnd = CreateWindow(
			g_szClassName,
			"",
			dwStyle,
			xPos,
			yPos,
			cxWidth,
			cyHeight,
			NULL,
			(HMENU)NULL,
			m_pApp->GetInstance(),
			this);
	if (m_hWnd == NULL)
		return ERR_MEMORY;

	//	The main frame covers the entire

	if (error = CreateFrame(NULL, 0, 0, NULL, NULL))
		return error;

	return NOERROR;
	}

void CUWindow::CaptureMouse (CUFrame *pFrame)

//	CaptureMouse
//
//	Captures the mouse

	{
	ASSERT(m_pMouseCapture == NULL);
	m_pMouseCapture = pFrame;
	::SetCapture(m_hWnd);
	}

ALERROR CUWindow::CreateBitmapButton (IUController *pController,
									CUFrame *pParent, 
									AutoSizeDesc *pDesc,
									IGMediaDb *pMediaDb,
									DWORD dwUNID,
									DWORD dwMaskUNID,
									int x,
									int y,
									int cxWidth,
									int cyHeight,
									DWORD dwTag,
									ControllerNotifyProc pfMsg)

//	CreateBitmapButton
//
//	Wrapper to create a frame and a bitmap button content
//
//	pController: controller
//	pParent: parent frame (may be NULL)
//	pDesc: AutoSize parameters for button frame
//	pMediaDb: Media db to use to load images from
//	dwUNID: UNID of button image
//	dwMaskUNID: UNID of button mask image (0 if none)
//	x, y: coordinates of button image
//	cxWidth, cyHeight: size of button image
//	dwTag: Tag for button
//	pfMsg: Notification to call when button is pushed

	{
	ALERROR error;
	CUFrame *pFrame;
	CUButton *pButton;

	//	First create a frame for the button

	if (error = CreateFrame(pParent, 
			(int)dwTag,
			0,
			pDesc,
			&pFrame))
		return error;

	//	Now create a button object

	pButton = new CUButton(pController);
	if (pButton == NULL)
		return ERR_MEMORY;

	//	Set some button properties

	pButton->SetTag(dwTag);
	pButton->SetType(CUButton::Bitmap);
	pButton->SetActionMsg(pfMsg);
	if (error = pButton->SetImages(pMediaDb,
			dwUNID,
			dwMaskUNID,
			x, y, cxWidth, cyHeight))
		{
		delete pButton;
		return error;
		}

	//	Add it to the frame

	if (error = pFrame->SetContent(pButton, TRUE))
		{
		delete pButton;
		return error;
		}

	return NOERROR;
	}

ALERROR CUWindow::CreateChildWindow (CUFrame *pFrame,
			char *pszClass,
			DWORD dwStyle,
			DWORD dwStyleEx,
			int iID,
			HWND *rethWnd)

//	CreateChildWindow
//
//	Create a child window and associate it with the given frame

	{
	HWND hWnd;
	RECT rcRect;

	//	Get the rect of the frame

	pFrame->GetRect(&rcRect);

	//	Create the window

	hWnd = CreateWindowEx(
			dwStyleEx,
			pszClass,
			"",
			dwStyle,
			rcRect.left, rcRect.top, RectWidth(&rcRect), RectHeight(&rcRect),
			m_hWnd,
			(HMENU)iID,
			m_pApp->GetInstance(),
			NULL);
	if (hWnd == NULL)
		return ERR_MEMORY;

	//	Done

	if (rethWnd)
		*rethWnd = hWnd;

	return NOERROR;
	}

ALERROR CUWindow::CreateFrame (CUFrame *pParent, int iID, DWORD dwFlags, AutoSizeDesc *pDesc, CUFrame **retpFrame)

//	CreateFrame
//
//	Creates a frame within the window
//
//	pParent: Parent frame. If NULL, the parent is the window's main frame
//	iID: ID of the frame
//	dwFlags: Flags for the frame
//	pDesc: AutoSize descriptor
//
//	retpFrame: Resulting new frame

	{
	CUFrame *pFrame;

	//	If NULL, this is the main frame

	if (pParent == NULL)
		pParent = m_pFrame;

	//	Create the new frame

	pFrame = new CUFrame(this, pParent, m_pController, iID, dwFlags);
	if (pFrame == NULL)
		return ERR_MEMORY;

	//	Set the autosize parameters

	pFrame->SetAutoSize(pDesc);

	//	If we don't have a main frame yet, this is the main frame

	if (m_pFrame == NULL)
		m_pFrame = pFrame;

	//	Return it

	if (retpFrame)
		*retpFrame = pFrame;

	return NOERROR;
	}

void CUWindow::DestroyFrame (CUFrame *pFrame)

//	DestroyFrame
//
//	Destroys the frame
	
	{ 
	RECT rcRect;

	//	Make sure we're not holding on to this pointer

	if (pFrame == m_pMouseCapture)
		ReleaseCapture();

	if (pFrame == m_pInputFocus)
		m_pInputFocus = NULL;

	//	Remember the old rect

	pFrame->GetRect(&rcRect);

	//	Destroy

	m_pFrame->DestroyFrame(pFrame);

	//	Repaint

	UpdateRect(&rcRect);
	}

void CUWindow::Hide (void)

//	Hide
//
//	Hide the window from the screen

	{
	ShowWindow(m_hWnd, SW_HIDE);
	}

void CUWindow::ReleaseCapture (void)

//	ReleaseCapture
//
//	Release mouse capture

	{
	::ReleaseCapture();
	m_pMouseCapture = NULL;
	}

void CUWindow::Resize (int x, int y, int cxWidth, int cyHeight)

//	Resize
//
//	Resize the window

	{
	MoveWindow(m_hWnd, x, y, cxWidth, cyHeight, TRUE);
	}

void CUWindow::SetInputFocus (CUFrame *pFrame)

//	SetInputFocus
//
//	Sets the focus to the given frame

	{
	m_pInputFocus = pFrame;
	}

void CUWindow::Show (void)

//	Show
//
//	Show the window

	{
	ShowWindow(m_hWnd, SW_SHOW);
	}

void CUWindow::ShowMainWindow (void)

//	ShowMainWindow
//
//	Show the main window for the first time

	{
	if (!IsWindowVisible(m_hWnd))
		ShowWindow(m_hWnd, m_pApp->GetInitialShow());
	}

void CUWindow::SetTitle (CString sString)

//	SetTitle
//
//	Set the window title

	{
	SetWindowText(m_hWnd, sString.GetASCIIZPointer());
	}

void CUWindow::UpdateRect (RECT *pRect)

//	UpdateRect
//
//	Repaint the given rect (if pRect is NULL, entire window is repainted)

	{
	InvalidateRect(m_hWnd, pRect, FALSE);
	}

LONG CUWindow::WMButtonDown (int x, int y, DWORD dwFlags)

//	WMButtonDown
//
//	Handle WM_LBUTTONDOWN

	{
	POINT ptPoint;

	ptPoint.x = x;
	ptPoint.y = y;
	m_pFrame->ButtonDown(ptPoint, dwFlags);
	return 0;
	}

LONG CUWindow::WMButtonUp (int x, int y, DWORD dwFlags)

//	WMButtonUp
//
//	Handle WM_LBUTTONUP

	{
	POINT ptPoint;

	ptPoint.x = x;
	ptPoint.y = y;

	//	If captured, give it to the frame; otherwise
	//	give it to whoever the mouse is over

	if (m_pMouseCapture)
		m_pMouseCapture->ButtonUp(ptPoint, dwFlags);
	else
		m_pFrame->ButtonUp(ptPoint, dwFlags);

	return 0;
	}

LONG CUWindow::WMChar (TCHAR chChar, DWORD dwKeyData)

//	WMChar
//
//	Handle WM_CHAR

	{
	return 0;
	}

LONG CUWindow::WMClose (void)

//	WMClose
//
//	Handle WM_CLOSE

	{
	BOOL bOkToClose;

	//	Ask our controller if it is ok to close

	if (m_pfQueryCloseMsg)
		bOkToClose = (BOOL)CallNotifyProc(m_pController, m_pfQueryCloseMsg, (DWORD)m_iID, 0);
	else
		bOkToClose = TRUE;

	//	Destroy the window

	if (bOkToClose)
		DestroyWindow(m_hWnd);

	return 0;
	}

LONG CUWindow::WMDestroy (void)

//	WMDestroy
//
//	Destroy the window

	{
	//	If we're destroying the window from inside our own destructor,
	//	we clear out m_hWnd so that we don't delete ourselves twice

	if (m_hWnd)
		{
		//	Clear out m_hWnd so we don't try to destroy it

		m_hWnd = NULL;

		//	Now remove ourselves from the application list

		m_pApp->DestroyWindow(this);

		//	Note that the last call deleted the object, so don't try
		//	to do anything else
		}

	return 0;
	}

LONG CUWindow::WMKeyDown (int iVirtKey, DWORD dwKeyData)

//	WMKeyDown
//
//	Handle WM_KEYDOWN

	{
	//	Give it to the frame

	if (m_pInputFocus)
		m_pInputFocus->KeyDown(iVirtKey, dwKeyData);

	return 0;
	}

LONG CUWindow::WMKeyUp (int iVirtKey, DWORD dwKeyData)

//	WMKeyUp
//
//	Handle WM_KEYUP

	{
	//	Give it to the frame

	if (m_pInputFocus)
		m_pInputFocus->KeyUp(iVirtKey, dwKeyData);

	return 0;
	}

LONG CUWindow::WMMouseMove (int x, int y, DWORD dwFlags)

//	WMMouseMove
//
//	Handle WM_MOUSEMOVE

	{
	POINT ptPoint;

	ptPoint.x = x;
	ptPoint.y = y;

	//	If captured, give it to the frame; otherwise
	//	give it to whoever the mouse is over

	if (m_pMouseCapture)
		m_pMouseCapture->MouseMove(ptPoint, dwFlags);
	else
		m_pFrame->MouseMove(ptPoint, dwFlags);

	return 0;
	}

LONG CUWindow::WMPaint (void)

//	WMPaint
//
//	Handle WM_PAINT

	{
	HDC hDC;
	PAINTSTRUCT ps;

	hDC = BeginPaint(m_hWnd, &ps);
	m_pFrame->Paint(hDC, &ps.rcPaint);
	EndPaint(m_hWnd, &ps);

	return 0;
	}

LONG CUWindow::WMSize (DWORD dwSizeType, int cxWidth, int cyHeight)

//	WMSize
//
//	Handle WM_SIZE

	{
	if (dwSizeType != SIZE_MINIMIZED)
		{
		if (m_pFrame)
			m_pFrame->Resize();
		}

	return 0;
	}

LONG APIENTRY CUWindowWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)

//	CUWindowWndProc
//
//	WndProc for generic window

	{
	switch (message)
		{
		case WM_CLOSE:
			return GetThis(hWnd)->WMClose();

		case WM_CHAR:
			return GetThis(hWnd)->WMChar((TCHAR)wParam, (DWORD)lParam);

		case WM_CREATE:
			{
			LPCREATESTRUCT pCreate = (LPCREATESTRUCT)lParam;
			CUWindow *pWindow = (CUWindow *)pCreate->lpCreateParams;

			//	Store the object pointer in the user struct

			SetWindowLong(hWnd, GWL_USERDATA, (LONG)pWindow);

			//	Set up the m_hWnd variable as soon as possible

			pWindow->m_hWnd = hWnd;

			//	Done

			return 0;
			}

		case WM_DESTROY:
			return GetThis(hWnd)->WMDestroy();

		case WM_KEYDOWN:
			return GetThis(hWnd)->WMKeyDown((int)wParam, (DWORD)lParam);

		case WM_KEYUP:
			return GetThis(hWnd)->WMKeyUp((int)wParam, (DWORD)lParam);

		case WM_LBUTTONDOWN:
			return GetThis(hWnd)->WMButtonDown(LOWORD(lParam), HIWORD(lParam), wParam);

		case WM_LBUTTONUP:
			return GetThis(hWnd)->WMButtonUp(LOWORD(lParam), HIWORD(lParam), wParam);

		case WM_MOUSEMOVE:
			return GetThis(hWnd)->WMMouseMove(LOWORD(lParam), HIWORD(lParam), wParam);

		case WM_PAINT:
			return GetThis(hWnd)->WMPaint();

		case WM_SIZE:
			return GetThis(hWnd)->WMSize(wParam, (int)LOWORD(lParam), (int)HIWORD(lParam));

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

