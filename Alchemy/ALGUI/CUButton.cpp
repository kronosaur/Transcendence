//	CUButton.cpp
//
//	CUButton object

#include "Alchemy.h"
#include "ALGUI.h"

static CObjectClass<CUButton>g_Class(OBJID_CUBUTTON, NULL);

CUButton::CUButton (void) : CObject(&g_Class)

//	contructor

	{
	}

CUButton::CUButton (IUController *pController) : CObject(&g_Class),
		m_pController(pController),
		m_pFrame(NULL),
		m_iType(Normal),
		m_dwTag(0),
		m_pfActionMsg(NULL),
		m_fButtonDown(FALSE),
		m_fSelected(FALSE),
		m_fMouseOver(FALSE),
		m_iBlinks(0)

//	CUButton constructor

	{
	}

ALERROR CUButton::Activate (CUFrame *pFrame, RECT *pRect)

//	Activate
//
//	Initialize everything

	{
	//	Remember the frame and position

	m_pFrame = pFrame;
	m_rcRect = *pRect;

	return NOERROR;
	}

void CUButton::Animate (void)

//	Animate
//
//	Animate

	{
	if (!m_fButtonDown && m_iBlinks > 0)
		{
		m_iBlinks--;
		m_pFrame->UpdateRect(&m_rcRect);
		}
	}

BOOL CUButton::ButtonDown (POINT ptPoint, DWORD dwFlags)

//	ButtonDown
//
//	Handle button down

	{
	ASSERT(m_fButtonDown == FALSE);

	m_fButtonDown = TRUE;
	m_pFrame->CaptureMouse();
	m_pFrame->UpdateRect(&m_rcRect);

	return TRUE;
	}

void CUButton::ButtonUp (POINT ptPoint, DWORD dwFlags)

//	ButtonUp
//
//	Handle button up

	{
	if (m_fButtonDown)
		{
		//	Make sure m_fMouseOver is up to date

		MouseMove(ptPoint, dwFlags);

		//	Release capture

		m_pFrame->ReleaseCapture();
		m_fButtonDown = FALSE;

		//	If we are over the button select it

		if (m_fMouseOver)
			{
			//	Paint

			m_pFrame->UpdateRect(&m_rcRect);

			//	Set blinks

			m_iBlinks = 8;

			//	Notify controller (note that we may be destroyed after
			//	we return, so don't do anything else

			CallNotifyProc(m_pController, m_pfActionMsg, m_dwTag, 0);
			}
		}
	}

void CUButton::Deactivate (void)

//	Deactivate
//
//	

	{
	}

void CUButton::MouseMove (POINT ptPoint, DWORD dwFlags)

//	MouseMove
//
//	Handle mouse move

	{
	BOOL bOldState;

	bOldState = m_fMouseOver;
	m_fMouseOver = PtInRect(&m_rcRect, ptPoint);

	//	If something changed then repaint

	if (bOldState != (BOOL)(DWORD)m_fMouseOver)
		m_pFrame->UpdateRect(&m_rcRect);
	}

void CUButton::Paint (HDC hDC, RECT *pUpdateRect)

//	Paint
//
//	Paint ourselves

	{
	//	Paint a normal button

	if (m_iType == Normal)
		{
		ASSERT(FALSE);
		}

	//	Paint a bitmap button

	else if (m_iType == Bitmap)
		{
		CGBitmap *pBitmap;

		//	Figure out which bitmap to use

		if (m_iBlinks > 0 && !m_fButtonDown)
			{
			if ((m_iBlinks % 2) == 0)
				pBitmap = &m_ButtonDownImage;
			else
				pBitmap = &m_ButtonUpImage;
			}

		//	This is the normal case

		else
			{
			if (m_fMouseOver)
				{
				if (m_fButtonDown)
					pBitmap = &m_ButtonDownImage;
				else
					pBitmap = &m_HoverImage;
				}
			else
				pBitmap = &m_ButtonUpImage;
			}

		//	Blt

		pBitmap->Blt(hDC, m_rcRect.left, m_rcRect.top, pUpdateRect);
		}
	else
		ASSERT(FALSE);
	}

void CUButton::Resize (RECT *pRect)

//	Resize
//
//	Handle a resize

	{
	m_rcRect = *pRect;
	}

ALERROR CUButton::SetImages (IGMediaDb *pMediaDb, DWORD dwUNID, DWORD dwMaskUNID, int x, int y, int cxWidth, int cyHeight)

//	SetImages
//
//	Sets the images to use for a bitmap button. This call assumes that the different
//	modes of the button are on the same bitmap in a horizontal array. The order of
//	the images is: normal, down, hover.

	{
	ALERROR error;

	if (error = SetUpImage(pMediaDb, dwUNID, dwMaskUNID, x, y, cxWidth, cyHeight))
		return error;

	if (error = SetDownImage(pMediaDb, dwUNID, dwMaskUNID, x + cxWidth, y, cxWidth, cyHeight))
		return error;

	if (error = SetHoverImage(pMediaDb, dwUNID, dwMaskUNID, x + 2 * cxWidth, y, cxWidth, cyHeight))
		return error;

	return NOERROR;
	}

void CUButton::SetType (ButtonTypes iType)

//	SetType
//
//	Sets the type of button

	{
	m_iType = iType;
	}
