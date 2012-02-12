//	CCommandBar.cpp
//
//	CCommandBar object

#include "PreComp.h"

#define BACKGROUND_BITMAP_X					0
#define BACKGROUND_BITMAP_Y					0
#define BACKGROUND_BITMAP_WIDTH				80
#define BACKGROUND_BITMAP_HEIGHT			600

#define TOP_BITMAP_X						0
#define TOP_BITMAP_Y						0
#define TOP_BITMAP_WIDTH					80
#define TOP_BITMAP_HEIGHT					96

#define MIDDLE_BITMAP_X						0
#define MIDDLE_BITMAP_Y						384
#define MIDDLE_BITMAP_WIDTH					80
#define MIDDLE_BITMAP_HEIGHT				144

#define BOTTOM_BITMAP_X						0
#define BOTTOM_BITMAP_Y						528
#define BOTTOM_BITMAP_WIDTH					80
#define BOTTOM_BITMAP_HEIGHT				72

#define BUTTON_BITMAP_X						80
#define BUTTON_BITMAP_Y						0

#define BUTTON_WIDTH						80
#define BUTTON_HEIGHT						48

static int g_iRefCount = 0;
static CGBitmap g_BackgroundBmp;

static CObjectClass<CCommandBar>g_Class(OBJID_CCOMMANDBAR, NULL);

CCommandBar::CCommandBar (void) : CObject(&g_Class)

//	CCommandBar constructor

	{
	}

CCommandBar::CCommandBar (IUController *pController) : CObject(&g_Class),
		m_pController(pController),
		m_pFrame(NULL)

//	CCommandBar constructor

	{
	}

ALERROR CCommandBar::Activate (CUFrame *pFrame, RECT *pRect)

//	Activate
//
//	Handle activation

	{
	ALERROR error;

	//	Remember our rect and frame

	m_rcRect = *pRect;
	m_pFrame = pFrame;

	//	Make sure we have our resources loaded

	if (error = InitResources(pFrame->GetWindow()->GetApp()->GetResourcesDb()))
		return error;

	return NOERROR;
	}

void CCommandBar::CleanUpResources (void)

//	CleanUpResources
//
//	Cleans up global resources allocated by InitResources

	{
	g_iRefCount--;

	if (g_iRefCount == 0)
		{
		g_BackgroundBmp.Unload();
		}
	}

void CCommandBar::Deactivate (void)

//	Deactivate
//
//	Handle deactivation

	{
	CleanUpResources();

	//	We got no frame!

	m_pFrame = NULL;
	}

ALERROR CCommandBar::InitResources (IGMediaDb *pMediaDb)

//	InitResources
//
//	Make sure that our background bitmap is loaded. You must call
//	CleanUpResources to clean up.

	{
	ALERROR error;

	if (g_iRefCount == 0)
		{
		if (error = g_BackgroundBmp.SetBitmap(pMediaDb,
				IDB_COMMANDBAR,
				0,
				BACKGROUND_BITMAP_X,
				BACKGROUND_BITMAP_Y,
				BACKGROUND_BITMAP_WIDTH,
				BACKGROUND_BITMAP_HEIGHT))
			return error;
		}

	g_iRefCount++;

	return NOERROR;
	}

void CCommandBar::Paint (HDC hDC, RECT *pUpdateRect)

//	Paint
//
//	Paint

	{
	int y;
	int cyLeftToBlt;

	//	Blt the top part of the bar

	y = m_rcRect.top;
	g_BackgroundBmp.PartialBlt(hDC,
			m_rcRect.left,
			y,
			TOP_BITMAP_WIDTH,
			TOP_BITMAP_HEIGHT,
			TOP_BITMAP_X,
			TOP_BITMAP_Y,
			pUpdateRect);
	y += TOP_BITMAP_HEIGHT;

	//	Blt the middle part

	cyLeftToBlt = RectHeight(&m_rcRect) - (TOP_BITMAP_HEIGHT + BOTTOM_BITMAP_HEIGHT);
	while (cyLeftToBlt > 0)
		{
		int cyBlt = min(cyLeftToBlt, MIDDLE_BITMAP_HEIGHT);

		g_BackgroundBmp.PartialBlt(hDC,
				m_rcRect.left,
				y,
				MIDDLE_BITMAP_WIDTH,
				cyBlt,
				MIDDLE_BITMAP_X,
				MIDDLE_BITMAP_Y,
				pUpdateRect);

		y += cyBlt;
		cyLeftToBlt -= cyBlt;
		}

	//	Blt the bottom

	g_BackgroundBmp.PartialBlt(hDC,
			m_rcRect.left,
			y,
			BOTTOM_BITMAP_WIDTH,
			BOTTOM_BITMAP_HEIGHT,
			BOTTOM_BITMAP_X,
			BOTTOM_BITMAP_Y,
			pUpdateRect);
	}

void CCommandBar::Resize (RECT *pRect)

//	Resize
//
//	Worry about resize

	{
	m_rcRect = *pRect;
	}

ALERROR CCommandBar::SetMenuDesc (MenuItemDesc *pDesc, int iMenuSize)

//	SetMenuDesc
//
//	Sets the menu buttons. Note that we must call this routine after
//	the frame has been set up

	{
	ALERROR error;
	int i, y;

	ASSERT(m_pFrame);

	//	Delete any buttons that are there now

	m_pFrame->DestroyAllChildFrames();

	//	Now create some buttons, starting after the top bitmap

	y = m_rcRect.top + TOP_BITMAP_HEIGHT;

	for (i = 0; i < iMenuSize; i++)
		{
		AutoSizeDesc AutoSize;

		AutoSize.SetFixed(m_rcRect.left,
				y,
				BUTTON_WIDTH,
				BUTTON_HEIGHT);

		if (error = m_pFrame->GetWindow()->CreateBitmapButton(m_pController,
				m_pFrame,
				&AutoSize,
				m_pFrame->GetWindow()->GetApp()->GetResourcesDb(),
				IDB_COMMANDBAR,
				0,
				BUTTON_BITMAP_X,
				BUTTON_BITMAP_Y + ((int)pDesc[i].iIcon * BUTTON_HEIGHT),
				BUTTON_WIDTH,
				BUTTON_HEIGHT,
				pDesc[i].dwTag,
				pDesc[i].pfActionMsg))
			return error;

		y += BUTTON_HEIGHT;
		}

	m_pFrame->UpdateRect(&m_rcRect);

	return NOERROR;
	}
