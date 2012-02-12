//	CGenericWnd.cpp
//
//	This window is a generic window that is actually controlled by
//	a CNodeEditor object

#include "Alchemy.h"
#include "TerraFirma.h"

static BOOL g_bRegistered = FALSE;
static char g_szClassName[] = "TF_Generic";

//	Forwards

LONG APIENTRY GenericWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);
static LONG WMCreate (HWND hWnd, LPCREATESTRUCT pCreate);

//	Inlines

inline CGenericWnd *GetObj (HWND hWnd) { return (CGenericWnd *)GetWindowLong(hWnd, GWL_USERDATA); }

CGenericWnd::CGenericWnd (void) :
		m_hWnd(NULL),
		m_pEditor(NULL)

//	CGenericWnd constructor

	{
	}

CGenericWnd::~CGenericWnd (void)

//	CGenericWnd destructor

	{
	if (m_hWnd)
		Destroy();
	}

ALERROR CGenericWnd::Create (CNodeEditor *pEditor, HWND hParent, RECT *pRect)

//	Create
//
//	Create the new window

	{
	DWORD dwStyle;

	//	Register the class, if we haven't already

	if (!g_bRegistered)
		{
		WNDCLASS  wc;

		//	Register the class

		utlMemSet(&wc, sizeof(wc), 0);
		wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = (WNDPROC)GenericWndProc;
		wc.hInstance = pEditor->GetTF()->GetInstance();
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = GetStockObject(BLACK_BRUSH);
		wc.lpszClassName = g_szClassName;

		if (!RegisterClass(&wc))
			return ERR_FAIL;
		}

	//	Initialize this right away

	m_pEditor = pEditor;

	//	Now create the window

	dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE;

	m_hWnd = CreateWindow(
			g_szClassName,
			"",
			dwStyle,
			pRect->left, pRect->top, RectWidth(pRect), RectHeight(pRect),
			hParent,
			(HMENU)1,
			pEditor->GetTF()->GetInstance(),
			this);
	if (m_hWnd == NULL)
		return ERR_FAIL;

	return NOERROR;
	}

void CGenericWnd::Destroy (void)

//	Destroy
//
//	Destroy the window

	{
	ASSERT(m_hWnd);
	DestroyWindow(m_hWnd);
	m_hWnd = NULL;
	}

LONG APIENTRY GenericWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)

//	GenericWndProc
//
//	WndProc for generic window

	{
	switch (message)
		{
		case WM_CREATE:
			return WMCreate(hWnd, (LPCREATESTRUCT)lParam);

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

LONG WMCreate (HWND hWnd, LPCREATESTRUCT pCreate)

//	WMCreate
//
//	Handle WM_CREATE

	{
	//	Store the object pointer in the user struct

	SetWindowLong(hWnd, GWL_USERDATA, (LONG)pCreate->lpCreateParams);

	return 0;
	}
