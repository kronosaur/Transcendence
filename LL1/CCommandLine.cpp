//	CCommandLine.cpp
//
//	CCommandLine object

#include "PreComp.h"

#define BACKGROUND_BITMAP_X					80
#define BACKGROUND_BITMAP_Y					0
#define BACKGROUND_BITMAP_WIDTH				520
#define BACKGROUND_BITMAP_HEIGHT			40

#define CLINE_HEIGHT						40
#define CLINE_LEFTEND_WIDTH					40
#define CLINE_LEFTREPEAT_WIDTH				96
#define CLINE_LEFTFIELDEND_WIDTH			18
#define CLINE_FIELDREPEAT_WIDTH				214
#define CLINE_RIGHTFIELDEND_WIDTH			18
#define CLINE_RIGHTREPEAT_WIDTH				96
#define CLINE_RIGHTEND_WIDTH				40

#define LEFT_MARGIN							58
#define RIGHT_MARGIN						118

#define EDITOR_HEIGHT						18
#define EDITOR_Y							11

#define EDITOR_FRAME_ID						300

static int g_iRefCount = 0;
static CGBitmap g_BackgroundBmp;

static CObjectClass<CCommandLine>g_Class(OBJID_CCOMMANDLINE, NULL);

CCommandLine::CCommandLine (void) : CObject(&g_Class)

//	CCommandLine constructor

	{
	}

CCommandLine::CCommandLine (IUController *pController) : CObject(&g_Class),
		m_pController(pController),
		m_pFrame(NULL),
		m_pfActionMsg(NULL)

//	CCommandLine constructor

	{
	}

ALERROR CCommandLine::Activate (CUFrame *pFrame, RECT *pRect)

//	Activate
//
//	Handle activation

	{
	ALERROR error;
	AutoSizeDesc AutoSize;
	CUFrame *pNewFrame;

	//	Remember our rect and frame

	m_rcRect = *pRect;
	m_pFrame = pFrame;
	m_cxFieldWidth = max(0, RectWidth(pRect) - (LEFT_MARGIN + RIGHT_MARGIN));

	//	Make sure we have our resources loaded

	if (error = InitResources(pFrame->GetWindow()->GetApp()->GetResourcesDb()))
		return error;

	//	Create an edit control

	AutoSize.SetAutoSize(0,
			EDITOR_HEIGHT, 
			LEFT_MARGIN, 
			EDITOR_Y, 
			RIGHT_MARGIN, 
			0, 
			AutoSizeDesc::DimVariable, AutoSizeDesc::MaxVariable);
	if (error = m_pFrame->GetWindow()->CreateFrame(m_pFrame, EDITOR_FRAME_ID, 0, &AutoSize, &pNewFrame))
		return error;

	m_pEditor = new CULineEditor(this);
	if (m_pEditor == NULL)
		return ERR_MEMORY;

	m_pEditor->SetActionMsg((ControllerNotifyProc)&CCommandLine::CmdAction);
	m_pEditor->SetSingleLine();

	if (error = pNewFrame->SetContent(m_pEditor, TRUE))
		return error;

	return NOERROR;
	}

void CCommandLine::CleanUpResources (void)

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

int CCommandLine::CmdAction (DWORD dwTag, DWORD dwDummy2)

//	CmdAction
//
//	Handle an action from the editor

	{
	return CallNotifyProc(m_pController, m_pfActionMsg, 0, 0);
	}

void CCommandLine::Deactivate (void)

//	Deactivate
//
//	Handle deactivation

	{
	CleanUpResources();

	//	We got no frame!

	m_pFrame = NULL;
	}

ALERROR CCommandLine::InitResources (IGMediaDb *pMediaDb)

//	InitResources
//
//	Make sure that our background bitmap is loaded. You must call
//	CleanUpResources to clean up.

	{
	ALERROR error;

	if (g_iRefCount == 0)
		{
		if (error = g_BackgroundBmp.SetBitmap(pMediaDb,
				IDB_COMMANDLINE,
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

void CCommandLine::Paint (HDC hDC, RECT *pUpdateRect)

//	Paint
//
//	Paint

	{
	int x, xSrc;
	int cxLeftToBlt;

	//	Blt the left part of the bar

	x = m_rcRect.left;
	xSrc = 0;

	g_BackgroundBmp.PartialBlt(hDC,
			x,
			m_rcRect.top,
			CLINE_LEFTEND_WIDTH,
			CLINE_HEIGHT,
			xSrc,
			0,
			pUpdateRect);
	x += CLINE_LEFTEND_WIDTH;
	xSrc += CLINE_LEFTEND_WIDTH;

	//	Blt the left repeating part

	cxLeftToBlt = LEFT_MARGIN - CLINE_LEFTEND_WIDTH - CLINE_LEFTFIELDEND_WIDTH;
	while (cxLeftToBlt > 0)
		{
		int cxBlt = min(cxLeftToBlt, CLINE_LEFTREPEAT_WIDTH);

		g_BackgroundBmp.PartialBlt(hDC,
				x,
				m_rcRect.top,
				cxBlt,
				CLINE_HEIGHT,
				xSrc,
				0,
				pUpdateRect);

		x += cxBlt;
		cxLeftToBlt -= cxBlt;
		}
	xSrc += CLINE_LEFTREPEAT_WIDTH;

	//	Blt the left edge of the field

	g_BackgroundBmp.PartialBlt(hDC,
			x,
			m_rcRect.top,
			CLINE_LEFTFIELDEND_WIDTH,
			CLINE_HEIGHT,
			xSrc,
			0,
			pUpdateRect);
	x += CLINE_LEFTFIELDEND_WIDTH;
	xSrc += CLINE_LEFTFIELDEND_WIDTH;

	//	Blt the field

	cxLeftToBlt = m_cxFieldWidth;
	while (cxLeftToBlt > 0)
		{
		int cxBlt = min(cxLeftToBlt, CLINE_FIELDREPEAT_WIDTH);

		g_BackgroundBmp.PartialBlt(hDC,
				x,
				m_rcRect.top,
				cxBlt,
				CLINE_HEIGHT,
				xSrc,
				0,
				pUpdateRect);

		x += cxBlt;
		cxLeftToBlt -= cxBlt;
		}
	xSrc += CLINE_FIELDREPEAT_WIDTH;

	//	Blt the right edge of the field

	g_BackgroundBmp.PartialBlt(hDC,
			x,
			m_rcRect.top,
			CLINE_RIGHTFIELDEND_WIDTH,
			CLINE_HEIGHT,
			xSrc,
			0,
			pUpdateRect);
	x += CLINE_RIGHTFIELDEND_WIDTH;
	xSrc += CLINE_RIGHTFIELDEND_WIDTH;

	//	Blt the repeating end

	cxLeftToBlt = RIGHT_MARGIN - CLINE_RIGHTEND_WIDTH - CLINE_RIGHTFIELDEND_WIDTH;
	while (cxLeftToBlt > 0)
		{
		int cxBlt = min(cxLeftToBlt, CLINE_RIGHTREPEAT_WIDTH);

		g_BackgroundBmp.PartialBlt(hDC,
				x,
				m_rcRect.top,
				cxBlt,
				CLINE_HEIGHT,
				xSrc,
				0,
				pUpdateRect);

		x += cxBlt;
		cxLeftToBlt -= cxBlt;
		}
	xSrc += CLINE_RIGHTREPEAT_WIDTH;

	//	Blt the right end

	g_BackgroundBmp.PartialBlt(hDC,
			x,
			m_rcRect.top,
			CLINE_RIGHTEND_WIDTH,
			CLINE_HEIGHT,
			xSrc,
			0,
			pUpdateRect);
	x += CLINE_RIGHTEND_WIDTH;
	xSrc += CLINE_RIGHTEND_WIDTH;
	}

void CCommandLine::Resize (RECT *pRect)

//	Resize
//
//	Worry about resize

	{
	m_rcRect = *pRect;

	//	Figure out the width of the field

	m_cxFieldWidth = max(0, RectWidth(pRect) - (LEFT_MARGIN + RIGHT_MARGIN));
	}

