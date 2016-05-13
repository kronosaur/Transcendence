//	CGalacticMapSession.cpp
//
//	CGalacticMapSession class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

const int SCROLL_STEP = 120;
const Metric MOUSE_WHEEL_ZOOM_IN = 1.001;
const Metric MOUSE_WHEEL_ZOOM_OUT = 1.0 / MOUSE_WHEEL_ZOOM_IN;

const int SCREEN_BORDER_X = 10;
const int SCREEN_BORDER_Y = 10;

const int PANE_PADDING_X = 10;
const int PANE_PADDING_Y = 10;

const int BORDER_RADIUS = 4;
const int HELP_PANE_WIDTH = 280;
const int HELP_PANE_HEIGHT = 180;
const BYTE HELP_PANE_OPACITY = 128;


#define STR_HELP_LINE1							CONSTLIT("[Arrows] to scroll")
#define STR_HELP_LINE2							CONSTLIT("[Esc] to exit")

static int SCALE_LEVELS[] =
	{	25, 50, 100, 200, 400 };

const int SCALE_LEVELS_COUNT = (sizeof(SCALE_LEVELS) / sizeof(SCALE_LEVELS[0]));

int CGalacticMapSession::GetScaleIndex (int iScale)

//	GetScaleIndex
//
//	Returns the scale index for the given scale

	{
	int i;

	for (i = 0; i < SCALE_LEVELS_COUNT; i++)
		if (iScale <= SCALE_LEVELS[i])
			return i;

	return SCALE_LEVELS[SCALE_LEVELS_COUNT - 1];
	}

int CGalacticMapSession::GetScale (int iScaleIndex)

//	GetScale
//
//	Returns the scale for the given index

	{
	return SCALE_LEVELS[iScaleIndex];
	}

void CGalacticMapSession::OnCleanUp (void)

//	OnCleanUp
//
//	Clean up

	{
	if (m_pPainter)
		{
		delete m_pPainter;
		m_pPainter = NULL;
		}
	}

ALERROR CGalacticMapSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	//	Get the map

	CTopologyNode *pNode = g_pUniverse->GetCurrentTopologyNode();
	if (pNode == NULL)
		return NOERROR;

	m_pMap = pNode->GetDisplayPos(&m_xCenter, &m_yCenter);
	if (m_pMap == NULL)
		return NOERROR;

	//	Get basic map information

	m_sMapName = m_pMap->GetName();

	m_pMap->GetScale(&m_iScale, &m_iMinScale, &m_iMaxScale);
	m_iScale = GetScale(GetScaleIndex(m_iScale));
	m_iMinScaleIndex = GetScaleIndex(m_iMinScale);
	m_iMaxScaleIndex = GetScaleIndex(m_iMaxScale);

	//	Create a painter

	m_pPainter = new CGalacticMapPainter(m_HI.GetVisuals(), m_pMap);

	//	Compute some rects

    GetRect(m_rcView);

    m_rcHelp.left = m_rcView.left + SCREEN_BORDER_X;
    m_rcHelp.right = m_rcHelp.left + HELP_PANE_WIDTH;
    m_rcHelp.bottom = m_rcView.bottom - SCREEN_BORDER_Y;
    m_rcHelp.top = m_rcHelp.bottom - HELP_PANE_HEIGHT;

	//	Adjust the map position

	m_pPainter->AdjustCenter(m_rcView, m_xCenter, m_yCenter, m_iScale, &m_xCenter, &m_yCenter);

	//	Initialize animation

    m_iTargetScale = m_iScale;
	m_xTargetCenter = m_xCenter;
	m_yTargetCenter = m_yCenter;

	return NOERROR;
	}

void CGalacticMapSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//	OnKeyDown
//
//	KeyDown

	{
	switch (iVirtKey)
		{
		case 'C':
			if (uiIsControlDown() && m_pPainter)
				::CopyGalacticMapToClipboard(m_HI.GetHWND(), m_pPainter);
			break;

		case VK_ADD:
		case VK_OEM_PLUS:
            {
            int iTargetScaleIndex = GetScaleIndex(m_iTargetScale);
            int iCurScale = GetScale(iTargetScaleIndex);
            if (iCurScale > m_iTargetScale)
                m_iTargetScale = Min(iCurScale, m_iMaxScale);
            else
			    m_iTargetScale = Min(GetScale(Min(iTargetScaleIndex + 1, m_iMaxScaleIndex)), m_iMaxScale);

			m_pPainter->AdjustCenter(m_rcView, m_xTargetCenter, m_yTargetCenter, m_iTargetScale, &m_xTargetCenter, &m_yTargetCenter);
			break;
            }

		case VK_CONTROL:
			break;

		case VK_DOWN:
			m_pPainter->AdjustCenter(m_rcView, m_xCenter, m_yCenter - (100 * SCROLL_STEP / m_iTargetScale), m_iTargetScale, &m_xTargetCenter, &m_yTargetCenter);
			break;

		case VK_HOME:
		case VK_END:
			{
			CTopologyNode *pNode = g_pUniverse->GetCurrentTopologyNode();
			if (pNode)
				{
				pNode->GetDisplayPos(&m_xTargetCenter, &m_yTargetCenter);
				m_pPainter->AdjustCenter(m_rcView, m_xTargetCenter, m_yTargetCenter, m_iTargetScale, &m_xTargetCenter, &m_yTargetCenter);
				}
			break;
			}

		case VK_LEFT:
			m_pPainter->AdjustCenter(m_rcView, m_xCenter - (100 * SCROLL_STEP / m_iTargetScale), m_yCenter, m_iTargetScale, &m_xTargetCenter, &m_yTargetCenter);
			break;

		case VK_NEXT:
			m_pPainter->AdjustCenter(m_rcView, m_xCenter, m_yCenter - (300 * SCROLL_STEP / m_iTargetScale), m_iTargetScale, &m_xTargetCenter, &m_yTargetCenter);
			break;

		case VK_PRIOR:
			m_pPainter->AdjustCenter(m_rcView, m_xCenter, m_yCenter + (300 * SCROLL_STEP / m_iTargetScale), m_iTargetScale, &m_xTargetCenter, &m_yTargetCenter);
			break;

		case VK_RIGHT:
			m_pPainter->AdjustCenter(m_rcView, m_xCenter + (100 * SCROLL_STEP / m_iTargetScale), m_yCenter, m_iTargetScale, &m_xTargetCenter, &m_yTargetCenter);
			break;

		case VK_SUBTRACT:
		case VK_OEM_MINUS:
            {
            int iTargetScaleIndex = GetScaleIndex(m_iTargetScale);
            int iCurScale = GetScale(iTargetScaleIndex);
            if (iCurScale < m_iTargetScale)
                m_iTargetScale = Max(iCurScale, m_iMinScale);
            else
			    m_iTargetScale = Max(m_iMinScale, GetScale(Max(m_iMinScaleIndex, iTargetScaleIndex - 1)));

			m_pPainter->AdjustCenter(m_rcView, m_xTargetCenter, m_yTargetCenter, m_iTargetScale, &m_xTargetCenter, &m_yTargetCenter);
			break;
            }

		case VK_UP:
			m_pPainter->AdjustCenter(m_rcView, m_xCenter, m_yCenter + (100 * SCROLL_STEP / m_iTargetScale), m_iTargetScale, &m_xTargetCenter, &m_yTargetCenter);
			break;

		//	Done

		default:
			m_HI.ClosePopupSession();
			break;
		}
	}

void CGalacticMapSession::OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture)

//	OnLButtonDown
//
//	LButtonDown

	{
    m_pPainter->ViewToGalactic(x, y, m_rcView, m_xCenter, m_yCenter, m_iTargetScale, &m_xAnchor, &m_yAnchor);
    m_xAnchorCenter = m_xCenter;
    m_yAnchorCenter = m_yCenter;

    m_bDragging = true;
    *retbCapture = true;
	}

void CGalacticMapSession::OnLButtonUp (int x, int y, DWORD dwFlags)

//  OnLButtonUp
//
//  LButtonUp

    {
    m_bDragging = false;
    }

void CGalacticMapSession::OnMouseMove (int x, int y, DWORD dwFlags)

//  OnMouseMOve
//
//  Mouse move

    {
    if (m_bDragging)
        {
        int xNewPos, yNewPos;
        m_pPainter->ViewToGalactic(x, y, m_rcView, m_xAnchorCenter, m_yAnchorCenter, m_iTargetScale, &xNewPos, &yNewPos);
		m_pPainter->AdjustCenter(m_rcView, m_xAnchorCenter - (xNewPos - m_xAnchor), m_yAnchorCenter - (yNewPos - m_yAnchor), m_iTargetScale, &m_xTargetCenter, &m_yTargetCenter);
        }
    }

void CGalacticMapSession::OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags)

//  OnMouseWheel
//
//  Zoom in/out

    {
    if (iDelta > 0)
        {
        m_iTargetScale = Min((int)(m_iTargetScale * Min(100.0, pow(MOUSE_WHEEL_ZOOM_IN, (Metric)iDelta))), m_iMaxScale);
		m_pPainter->AdjustCenter(m_rcView, m_xTargetCenter, m_yTargetCenter, m_iTargetScale, &m_xTargetCenter, &m_yTargetCenter);
        }
    else if (iDelta < 0)
        {
        m_iTargetScale = Max(m_iMinScale, (int)(m_iTargetScale * pow(MOUSE_WHEEL_ZOOM_OUT, (Metric)-iDelta)));
		m_pPainter->AdjustCenter(m_rcView, m_xTargetCenter, m_yTargetCenter, m_iTargetScale, &m_xTargetCenter, &m_yTargetCenter);
        }
    }

void CGalacticMapSession::OnPaint (CG32bitImage &Screen, const RECT &rcInvalid)

//	OnPaint
//
//	Paint

	{
	int cxScreen = Screen.GetWidth();
	int cyScreen = Screen.GetHeight();

	const CVisualPalette &VI = m_HI.GetVisuals();
	CG32bitPixel rgbBackgroundColor = VI.GetColor(colorAreaDeep);
	CG32bitPixel rgbLineColor = VI.GetColor(colorLineFrame);
	const CG16bitFont &HeaderFont = VI.GetFont(fontHeader);
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);

	//	Paint the actual map

	if (m_pPainter)
		{
		m_pPainter->Paint(Screen, m_rcView, m_xCenter, m_yCenter, m_iScale);

		//	Paint the ship

		CSpaceObject *pPlayer = g_pUniverse->GetPlayerShip();
		if (pPlayer)
			{
			int xPos, yPos;
			g_pUniverse->GetCurrentSystem()->GetTopology()->GetDisplayPos(&xPos, &yPos);

			int xShip, yShip;
			m_pPainter->GalacticToView(xPos, yPos, m_rcView, m_xCenter, m_yCenter, m_iScale, &xShip, &yShip);

			pPlayer->PaintMap(CMapViewportCtx(), Screen, xShip, yShip);
			}
		}

	//	Paint some help text

    PaintHelpPane(Screen);
	}

void CGalacticMapSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash
//
//	Describe current state

	{
	*retsMessage = CONSTLIT("session: CGalacticMapSession\r\n");
	}

void CGalacticMapSession::OnUpdate (bool bTopMost)

//	OnUpdate
//
//	Update

	{
	if (m_iTargetScale != m_iScale)
		{
		int iDiff = m_iTargetScale - m_iScale;
		m_iScale = (Absolute(iDiff) > 1 ? m_iScale + (iDiff / 2) : m_iTargetScale);
		HIInvalidate();
		}

	if (m_xCenter != m_xTargetCenter || m_yCenter != m_yTargetCenter)
		{
		int xDiff = m_xTargetCenter - m_xCenter;
		int yDiff = m_yTargetCenter - m_yCenter;
		
		m_xCenter = (Absolute(xDiff) > 1 ? m_xCenter + (xDiff / 2) : m_xTargetCenter);
		m_yCenter = (Absolute(yDiff) > 1 ? m_yCenter + (yDiff / 2) : m_yTargetCenter);

		HIInvalidate();
		}
	}

void CGalacticMapSession::PaintHelpPane (CG32bitImage &Screen)

//  PaintHelpPane
//
//  Paints the help pane.

    {
	const CVisualPalette &VI = m_HI.GetVisuals();
	CG32bitPixel rgbBackgroundColor = VI.GetColor(colorAreaDeep);
	CG32bitPixel rgbLineColor = VI.GetColor(colorLineFrame);
	const CG16bitFont &HeaderFont = VI.GetFont(fontHeader);
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);

    //  Paint the pane background

    CGDraw::RoundedRect(Screen, m_rcHelp.left, m_rcHelp.top, RectWidth(m_rcHelp), RectHeight(m_rcHelp), BORDER_RADIUS, CG32bitPixel(VI.GetColor(colorAreaDialog), HELP_PANE_OPACITY));
    CGDraw::RoundedRectOutline(Screen, m_rcHelp.left, m_rcHelp.top, RectWidth(m_rcHelp), RectHeight(m_rcHelp), BORDER_RADIUS, 1, VI.GetColor(colorLineFrame));

    //  Paint help text

	int x = m_rcHelp.left + PANE_PADDING_X;
	int y = m_rcHelp.top + PANE_PADDING_Y;

	Screen.DrawText(x, y, HeaderFont, VI.GetColor(colorTextHighlight), m_sMapName);
	y += HeaderFont.GetHeight();
	
	Screen.DrawText(x, y, MediumFont, VI.GetColor(colorTextFade), STR_HELP_LINE1);
	y += MediumFont.GetHeight();

	Screen.DrawText(x, y, MediumFont, VI.GetColor(colorTextFade), STR_HELP_LINE2);
	y += MediumFont.GetHeight();
    }

void CopyGalacticMapToClipboard (HWND hWnd, CGalacticMapPainter *pPainter)
	{
	//	Compute the size of the map

	RECT rcView;
	rcView.left = 0;
	rcView.top = 0;
	rcView.right = pPainter->GetWidth();
	rcView.bottom = pPainter->GetHeight();

	//	Create a bitmap of the appropriate size

	CG32bitImage FullMap;
	FullMap.Create(RectWidth(rcView), RectHeight(rcView));

	//	Paint

	pPainter->Paint(FullMap, rcView, 0, 0, 100);

	//	Copy to cliboard

	FullMap.CopyToClipboard();
	}
