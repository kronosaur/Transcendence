//	CUFrame.cpp
//
//	CUFrame object

#include "Alchemy.h"
#include "ALGUI.h"

static CObjectClass<CUFrame>g_Class(OBJID_CUFRAME, NULL);

CUFrame::CUFrame (void) : CObject(&g_Class)

//	CUFrame constructor

	{
	}

CUFrame::CUFrame (CUWindow *pWindow, CUFrame *pParent, IUController *pController, int iID, DWORD dwFlags) : CObject(&g_Class),
		m_pWindow(pWindow),
		m_pController(pController),
		m_iID(iID),
		m_dwFlags(dwFlags),
		m_dwState(0),
		m_pContent(NULL),
		m_rgbBackground(RGB(0, 0, 0))

//	CUFrame constructor

	{
	m_pParent = pParent;
	m_pFirstChild = NULL;
	m_pNextSibling = NULL;

	//	If we have a parent, add us to the sibling list

	if (pParent)
		{
		CUFrame *pLastSibling = pParent->m_pFirstChild;

		if (pLastSibling == NULL)
			pParent->m_pFirstChild = this;
		else
			{
			while (pLastSibling->m_pNextSibling)
				pLastSibling = pLastSibling->m_pNextSibling;

			pLastSibling->m_pNextSibling = this;
			}
		}
	}

CUFrame::~CUFrame (void)

//	CUFrame destructor

	{
	//	Delete all our children

	DestroyAllChildFrames();

	//	Kill the content

	ReleaseContent();
	}

void CUFrame::Animate (void)

//	Animate
//
//	Handle animation

	{
	CUFrame *pNext = m_pFirstChild;

	while (pNext)
		{
		pNext->Animate();
		pNext = pNext->m_pNextSibling;
		}

	//	Content

	if (m_pContent)
		m_pContent->Animate();
	}

HRGN CUFrame::BeginClip (HDC hDC, RECT &rcRect)

//	BeginClip
//
//	Start a new clipping region

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

	IntersectClipRect(hDC, rcRect.left, rcRect.top, rcRect.right, rcRect.bottom);

	return hOldClipRgn;
	}

void CUFrame::EndClip (HDC hDC, HRGN hOldClip)

//	EndClip
//
//	End clip region

	{
	SelectClipRgn(hDC, hOldClip);
	if (hOldClip)
		DeleteObject(hOldClip);
	}

BOOL CUFrame::ButtonDown (POINT ptPoint, DWORD dwFlags)

//	ButtonDown
//
//	Handle mouse button down. Returns TRUE if it was handled

	{
	CUFrame *pNext = m_pFirstChild;

	//	Is this in our rect?

	if (!PtInRect(&m_rcRect, ptPoint))
		return FALSE;

	//	If the mouse is in any of our child frames, pass it to them

	while (pNext)
		{
		//	If one of our frames handled it, we return because
		//	we don't want to handle a message twice in case of
		//	overlapping frames

		if (pNext->ButtonDown(ptPoint, dwFlags))
			return TRUE;

		pNext = pNext->m_pNextSibling;
		}

	//	If no one has handled it yet, pass it on to our content

	if (m_pContent)
		return m_pContent->ButtonDown(ptPoint, dwFlags);

	//	We could not handle it

	return FALSE;
	}

void CUFrame::ButtonUp (POINT ptPoint, DWORD dwFlags)

//	ButtonUp
//
//	Handle mouse button up

	{
	CUFrame *pNext = m_pFirstChild;

	//	Is this for us?

	if (!PtInRect(&m_rcRect, ptPoint) && !HasMouseCapture())
		return;

	//	Pass it to our children

	while (pNext)
		{
		pNext->ButtonUp(ptPoint, dwFlags);
		pNext = pNext->m_pNextSibling;
		}

	//	Pass it to our content

	if (m_pContent)
		m_pContent->ButtonUp(ptPoint, dwFlags);
	}

void CUFrame::CalcChildRect (CUFrame *pChild, AutoSizeDesc *pDesc, RECT *retrcRect)

//	CalcRect
//
//	Returns the rect calculated from the parent rect and based
//	on the autosize settings

	{
	pDesc->CalcRect(&m_rcRect, retrcRect);
	}

void CUFrame::DestroyAllChildFrames (void)

//	DestroyAllChildFrames
//
//	Destroy all children of this frame

	{
	CUFrame *pNext = m_pFirstChild;
	CUFrame *pFrameToDelete;

	//	Delete all our children

	while (pNext)
		{
		pFrameToDelete = pNext;
		pNext = pNext->m_pNextSibling;
		delete pFrameToDelete;
		}

	m_pFirstChild = NULL;
	}

BOOL CUFrame::DestroyFrame (CUFrame *pFrame)

//	DestroyFrame
//
//	Destroy a child frame. Returns TRUE if successful

	{
	CUFrame *pNext = m_pFirstChild;
	CUFrame *pPrev = NULL;

	//	See if it is one of the children

	while (pNext)
		{
		//	Destroy this one?

		if (pNext == pFrame)
			{
			//	Remove from linked-list

			if (pPrev == NULL)
				m_pFirstChild = pNext->m_pNextSibling;
			else
				pPrev->m_pNextSibling = pNext->m_pNextSibling;

			//	delete

			delete pFrame;
			return TRUE;
			}
		else
			{
			//	See if it is a child frame

			if (pNext->DestroyFrame(pFrame))
				return TRUE;

			pPrev = pNext;
			pNext = pNext->m_pNextSibling;
			}
		}

	return FALSE;
	}

CUFrame *CUFrame::FindFrame (int iID)

//	FindFrame
//
//	Find the frame with the given ID

	{
	CUFrame *pNext = m_pFirstChild;
	CUFrame *pFrame;

	//	Is it us?

	if (iID == m_iID)
		return this;

	//	Is it any of our children

	while (pNext)
		{
		pFrame = pNext->FindFrame(iID);
		if (pFrame)
			return pFrame;

		pNext = pNext->m_pNextSibling;
		}

	//	Didn't find it

	return NULL;
	}

void CUFrame::KeyDown (int iVirtKey, DWORD dwKeyData)

//	KeyDown
//
//	Handle key down

	{
	BOOL bHandled = FALSE;

	//	Pass it to our content

	if (m_pContent)
		bHandled = m_pContent->KeyDown(iVirtKey, dwKeyData);

	//	If our content did not handle it, pass it up to our
	//	parent

	if (m_pParent && !bHandled)
		m_pParent->KeyDown(iVirtKey, dwKeyData);
	}

void CUFrame::KeyUp (int iVirtKey, DWORD dwKeyData)

//	KeyUp
//
//	Handle key up

	{
	//	Pass it to our content

	if (m_pContent)
		m_pContent->KeyUp(iVirtKey, dwKeyData);
	}

void CUFrame::MouseMove (POINT ptPoint, DWORD dwFlags)

//	MouseMove
//
//	Handle mouse moving

	{
	CUFrame *pNext = m_pFirstChild;

	//	Pass it to our children

	while (pNext)
		{
		pNext->MouseMove(ptPoint, dwFlags);
		pNext = pNext->m_pNextSibling;
		}

	//	Pass it to our content

	if (m_pContent)
		m_pContent->MouseMove(ptPoint, dwFlags);
	}

void CUFrame::Paint (HDC hDC, RECT *pUpdateRect)

//	Paint
//
//	Paint to a DC

	{
	RECT rcRect;

	//	Paint if we intersect the update rect

	if (IntersectRect(&rcRect, pUpdateRect, &m_rcRect))
		{
		CUFrame *pNext = m_pFirstChild;

		//	Fill with our background color

		gdiFillRect(hDC, &rcRect, m_rgbBackground);

		//	Now paint our content

		if (m_pContent)
			m_pContent->Paint(hDC, &rcRect);

		//	Let our children paint

		while (pNext)
			{
			pNext->Paint(hDC, &rcRect);
			pNext = pNext->m_pNextSibling;
			}
		}
	}

void CUFrame::ReleaseContent (void)

//	ReleaseContent
//
//	Release the current content

	{
	if (m_pContent)
		{
		m_pContent->Deactivate();

		if (m_dwState & FRAME_STATE_OWNEDCONTENT)
			delete m_pContent;

		m_pContent = NULL;
		}
	}

void CUFrame::Resize (void)

//	Resize
//
//	Notification that the frame's rect needs to be re-computed

	{
	CUFrame *pChild = m_pFirstChild;

	//	If we've go no parent, then we conform to the size of the
	//	window.

	if (m_pParent == NULL)
		m_pWindow->GetRect(&m_rcRect);

	//	Otherwise we calculate our rect based on our parent's rect
	//	and our AutoSize settings

	else
		m_pParent->CalcChildRect(this, &m_AutoSize, &m_rcRect);

	//	Tell our content

	if (m_pContent)
		m_pContent->Resize(&m_rcRect);

	//	Resize our children

	while (pChild)
		{
		pChild->Resize();
		pChild = pChild->m_pNextSibling;
		}
	}

void CUFrame::SetAutoSize (AutoSizeDesc *pDesc)

//	SetAutoSize
//
//	Set new autosize paramenters

	{
	//	Set the autosize. Note that when we're creating the main frame
	//	we don't have an autosize, so we check here.

	if (pDesc)
		m_AutoSize = *pDesc;
	else
		m_AutoSize.SetFull();

	//	Resize

	Resize();
	}

ALERROR CUFrame::SetContent (IUFrameContent *pContent, BOOL bOwned)

//	SetContent
//
//	Sets the content for this frame

	{
	ALERROR error;

	//	Tell the content that it's active now

	if (error = pContent->Activate(this, &m_rcRect))
		return error;

	//	If we've already got content and we own it, free it now

	ReleaseContent();

	//	Done

	m_pContent = pContent;

	return NOERROR;
	}

void CUFrame::SetFocus (void)

//	SetFocus
//
//	Sets the focus on the given frame

	{
	//	Tell our window that we want the focus

	m_pWindow->SetInputFocus(this);

	//	Tell our content that we've got the focus
	
	if (m_pContent)
		m_pContent->SetFocus();
	}

