//	CGalacticMapSession.cpp
//
//	CGalacticMapSession class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

const int SCROLL_STEP = 120;

const int SCREEN_BORDER_X = 10;
const int SCREEN_BORDER_Y = 10;

const int PANE_PADDING_X = 10;
const int PANE_PADDING_Y = 10;

const int BORDER_RADIUS = 4;
const int HELP_PANE_WIDTH = 280;
const BYTE HELP_PANE_OPACITY = 128;

const int DETAIL_PANE_WIDTH = 512;
const int DETAIL_PANE_HEIGHT = 512;

#define ID_DETAILS  							CONSTLIT("idDetails")

#define CMD_UI_SWITCH_TO_SYSTEM_MAP				CONSTLIT("uiSwitchToSystemMap")

#define STR_HELP_DESC							CONSTLIT("Click and drag to navigate\n[Esc] to exit")

static CMapLegendPainter::SScaleEntry LEGEND_SCALE[] =
    {
        {   100000,     "light-years",  1.0 },
        {   50000,      "light-years",  1.0 },
        {   25000,      "light-years",  1.0 },
        {   10000,      "light-years",  1.0 },
        {   5000,       "light-years",  1.0 },
        {   2500,       "light-years",  1.0 },
        {   1000,       "light-years",  1.0 },
        {   500,        "light-years",  1.0 },
        {   250,        "light-years",  1.0 },
        {   100,        "light-years",  1.0 },
        {   50,         "light-years",  1.0 },
        {   25,         "light-years",  1.0 },
        {   10,         "light-years",  1.0 },
        {   5,          "light-years",  1.0 },
        {   2,          "light-years",  1.0 },
        {   1,          "light-year",   1.0 }
    };

const int LEGEND_SCALE_COUNT = (sizeof(LEGEND_SCALE) / sizeof(LEGEND_SCALE[0]));

CGalacticMapSession::CGalacticMapSession (CHumanInterface &HI, CGameSettings &Settings, CSystemMapThumbnails &SystemMapThumbnails, SOptions &SavedState) : IHISession(HI), 
        m_Settings(Settings),
        m_SystemMapThumbnails(SystemMapThumbnails),
        m_SavedState(SavedState),
        m_pMap(NULL), 
        m_pPainter(NULL),
        m_HelpPainter(HI.GetVisuals(), LEGEND_SCALE, LEGEND_SCALE_COUNT),
        m_bDragging(false)

//  CGalacticMapSession constructor

    { 
    }

void CGalacticMapSession::OnChar (char chChar, DWORD dwKeyData)

//  OnChar
//
//  Handle character

    {
    //  We handle certain commands

	CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommandFromChar(chChar);
    switch (iCommand)
        {
        //  Switch back to system map

        case CGameKeys::keyShowMap:
            SaveState();
            m_HI.HICommand(CMD_UI_SWITCH_TO_SYSTEM_MAP);
            break;

        //  If we hit the galactic map key again, we close this window

        case CGameKeys::keyShowGalacticMap:
            SaveState();
            m_HI.ClosePopupSession();
            break;
        }
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
	const CG16bitFont &HeaderFont = VI.GetFont(fontHeader);
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);

    //  Refresh global objects

    if (g_pUniverse->GetCurrentSystem())
        g_pUniverse->GetGlobalObjects().Refresh(g_pUniverse->GetCurrentSystem());

	//	Compute some rects

    GetRect(m_rcView);

	//	Get the map

	CTopologyNode *pNode = g_pUniverse->GetCurrentTopologyNode();
	if (pNode == NULL)
		return NOERROR;

	m_pMap = pNode->GetDisplayPos(&m_xCenter, &m_yCenter);
	if (m_pMap == NULL)
		return NOERROR;

	//	Get basic map information

    int iScale;
    int iMinScale;
    int iMaxScale;
	m_pMap->GetScale(&iScale, &iMinScale, &iMaxScale);

    //  If we've got saved state, use that

    if (m_SavedState.iScale != 0)
        {
        iScale = m_SavedState.iScale;
        m_xCenter = m_SavedState.xCenter;
        m_yCenter = m_SavedState.yCenter;
        }

    //  Initialize

    m_Scale.Init(iScale, iMinScale, iMaxScale);

	//	Create a painter

	m_pPainter = new CGalacticMapPainter(*g_pUniverse, m_pMap, m_SystemMapThumbnails);
    m_pPainter->SetViewport(m_rcView);
    m_pPainter->SetScale(m_Scale.GetScale());

	//	Adjust the map position so we fit

	m_pPainter->AdjustCenter(m_xCenter, m_yCenter, m_Scale.GetTargetScale(), &m_xCenter, &m_yCenter);
    m_pPainter->SetPos(m_xCenter, m_yCenter);

    //  Initialize help pane

    m_HelpPainter.SetWidth(HELP_PANE_WIDTH);
    m_HelpPainter.SetTitle(m_pMap->GetName());
    m_HelpPainter.SetDesc(STR_HELP_DESC);

	//	Initialize animation

	m_xTargetCenter = m_xCenter;
	m_yTargetCenter = m_yCenter;
    SetTargetScale();

    //  Select something (but only if we're in the same node)

    if (m_SavedState.pSelected
            && pNode == m_SavedState.pCurNode)
        Select(m_SavedState.pSelected, true);
    else
        m_SavedState.pSelected = NULL;

    //  Remember our current node

    m_SavedState.pCurNode = pNode;

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
            if (m_Scale.CanZoomIn())
                {
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
                m_Scale.ZoomIn();
                SetTargetScale();
                }
			break;

		case VK_CONTROL:
			break;

		case VK_DOWN:
			m_pPainter->AdjustCenter(m_xCenter, m_yCenter - (100 * SCROLL_STEP / m_Scale.GetTargetScale()), m_Scale.GetTargetScale(), &m_xTargetCenter, &m_yTargetCenter);
			break;

        case VK_ESCAPE:
            SaveState();
			m_HI.ClosePopupSession();
            break;

		case VK_HOME:
		case VK_END:
			{
			CTopologyNode *pNode = g_pUniverse->GetCurrentTopologyNode();
			if (pNode)
				{
				pNode->GetDisplayPos(&m_xTargetCenter, &m_yTargetCenter);
				m_pPainter->AdjustCenter(m_xTargetCenter, m_yTargetCenter, m_Scale.GetTargetScale(), &m_xTargetCenter, &m_yTargetCenter);
				}
			break;
			}

		case VK_LEFT:
			m_pPainter->AdjustCenter(m_xCenter - (100 * SCROLL_STEP / m_Scale.GetTargetScale()), m_yCenter, m_Scale.GetTargetScale(), &m_xTargetCenter, &m_yTargetCenter);
			break;

		case VK_NEXT:
			m_pPainter->AdjustCenter(m_xCenter, m_yCenter - (300 * SCROLL_STEP / m_Scale.GetTargetScale()), m_Scale.GetTargetScale(), &m_xTargetCenter, &m_yTargetCenter);
			break;

		case VK_PRIOR:
			m_pPainter->AdjustCenter(m_xCenter, m_yCenter + (300 * SCROLL_STEP / m_Scale.GetTargetScale()), m_Scale.GetTargetScale(), &m_xTargetCenter, &m_yTargetCenter);
			break;

		case VK_RIGHT:
			m_pPainter->AdjustCenter(m_xCenter + (100 * SCROLL_STEP / m_Scale.GetTargetScale()), m_yCenter, m_Scale.GetTargetScale(), &m_xTargetCenter, &m_yTargetCenter);
			break;

		case VK_SUBTRACT:
		case VK_OEM_MINUS:
            if (m_Scale.CanZoomOut())
                {
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
                m_Scale.ZoomOut();
                SetTargetScale();
                }
            break;

		case VK_UP:
			m_pPainter->AdjustCenter(m_xCenter, m_yCenter + (100 * SCROLL_STEP / m_Scale.GetTargetScale()), m_Scale.GetTargetScale(), &m_xTargetCenter, &m_yTargetCenter);
			break;
		}
	}

void CGalacticMapSession::Select (CTopologyNode *pNode, bool bNoSound)

//  Select
//
//  Selects the given node

    {
    //  If this node is already selected, skip

    if (pNode == m_pPainter->GetSelection())
        return;

    //  Select

	if (!bNoSound)
		g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
    m_pPainter->SetSelection(pNode);

    //  We always stop the performance, if there is one

    StopPerformance(ID_DETAILS);

    //  If we have a valid node, then show data for it.

    if (pNode)
        {
        RECT rcPane = m_rcView;
        rcPane.right = m_rcView.right - SCREEN_BORDER_X;
        rcPane.left = rcPane.right - DETAIL_PANE_WIDTH;
        rcPane.top = m_rcView.top + (RectHeight(m_rcView) - DETAIL_PANE_HEIGHT) / 2;
        rcPane.bottom = rcPane.top + DETAIL_PANE_HEIGHT;

        CGalacticMapSystemDetails Details(m_HI.GetVisuals(), GetReanimator(), rcPane);

        IAnimatron *pAni;
        Details.CreateDetailsPane(pNode, &pAni);

    	StartPerformance(pAni, ID_DETAILS, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
        }
    }

void CGalacticMapSession::OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture)

//	OnLButtonDown
//
//	LButtonDown

	{
    //  If we clicked on a node, then select

    CGalacticMapPainter::SSelectResult Selection;
    if (m_pPainter->HitTest(x, y, Selection)
            && Selection.pNode)
        {
        Select(Selection.pNode);
        }

    //  Otherwise, click and drag

    else
        {
        m_pPainter->ViewToGalactic(x, y, m_xCenter, m_yCenter, m_Scale.GetTargetScale(), &m_xAnchor, &m_yAnchor);
        m_xAnchorCenter = m_xCenter;
        m_yAnchorCenter = m_yCenter;

        m_bDragging = true;
        *retbCapture = true;
        }
	}

void CGalacticMapSession::OnLButtonUp (int x, int y, DWORD dwFlags)

//  OnLButtonUp
//
//  LButtonUp

    {
    if (m_bDragging)
        {
        int xNewPos, yNewPos;
        m_pPainter->ViewToGalactic(x, y, m_xAnchorCenter, m_yAnchorCenter, m_Scale.GetTargetScale(), &xNewPos, &yNewPos);

        //  If we didn't drag very much, then clear the selection.

        if (Absolute(xNewPos - m_xAnchor) <= 2 && Absolute(yNewPos - m_yAnchor) <= 2)
            Select(NULL);

        m_bDragging = false;
        }
    }

void CGalacticMapSession::OnMouseMove (int x, int y, DWORD dwFlags)

//  OnMouseMOve
//
//  Mouse move

    {
    if (m_bDragging)
        {
        int xNewPos, yNewPos;
        m_pPainter->ViewToGalactic(x, y, m_xAnchorCenter, m_yAnchorCenter, m_Scale.GetTargetScale(), &xNewPos, &yNewPos);
		m_pPainter->AdjustCenter(m_xAnchorCenter - (xNewPos - m_xAnchor), m_yAnchorCenter - (yNewPos - m_yAnchor), m_Scale.GetTargetScale(), &m_xTargetCenter, &m_yTargetCenter);
        }
    }

void CGalacticMapSession::OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags)

//  OnMouseWheel
//
//  Zoom in/out

    {
    m_Scale.ZoomWheel(iDelta);
    SetTargetScale();
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
		m_pPainter->Paint(Screen);

		//	Paint the ship

		CSpaceObject *pPlayer = g_pUniverse->GetPlayerShip();
		if (pPlayer)
			{
			int xPos, yPos;
			g_pUniverse->GetCurrentSystem()->GetTopology()->GetDisplayPos(&xPos, &yPos);

			int xShip, yShip;
			m_pPainter->GalacticToView(xPos, yPos, m_xCenter, m_yCenter, m_Scale.GetScale(), &xShip, &yShip);

			pPlayer->PaintMap(CMapViewportCtx(), Screen, xShip, yShip);
			}
		}

	//	Paint some help text

    m_HelpPainter.Paint(Screen, 
            m_rcView.left + SCREEN_BORDER_X, 
            m_rcView.bottom - (SCREEN_BORDER_Y + m_HelpPainter.GetHeight()));
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
    if (m_pPainter == NULL)
        return;

    if (m_Scale.Update())
        {
        m_pPainter->SetScale(m_Scale.GetScale());
		HIInvalidate();
        }

	if (m_xCenter != m_xTargetCenter || m_yCenter != m_yTargetCenter)
		{
		int xDiff = m_xTargetCenter - m_xCenter;
		int yDiff = m_yTargetCenter - m_yCenter;
		
		m_xCenter = (Absolute(xDiff) > 1 ? m_xCenter + (xDiff / 2) : m_xTargetCenter);
		m_yCenter = (Absolute(yDiff) > 1 ? m_yCenter + (yDiff / 2) : m_yTargetCenter);
        m_pPainter->SetPos(m_xCenter, m_yCenter);

		HIInvalidate();
		}
	}

void CGalacticMapSession::SaveState (void)

//  SaveState
//
//  Save state so we can come back to the same scale/position

    {
    m_SavedState.iScale = m_Scale.GetTargetScale();
    m_SavedState.xCenter = m_xTargetCenter;
    m_SavedState.yCenter = m_yTargetCenter;
    m_SavedState.pSelected = m_pPainter->GetSelection();
    }

void CGalacticMapSession::SetTargetScale (void)

//  SetTargetScale
//
//  Sets the target scale

    {
	m_pPainter->AdjustCenter(m_xTargetCenter, m_yTargetCenter, m_Scale.GetTargetScale(), &m_xTargetCenter, &m_yTargetCenter);

    //  Initialize scale legend

    Metric rLYPerPixel = (m_pMap == NULL ? 0.0 : (100.0 * m_pMap->GetLightYearsPerPixel() / (Metric)m_Scale.GetTargetScale()));
    m_HelpPainter.SetScale(rLYPerPixel);
    }

void CopyGalacticMapToClipboard (HWND hWnd, CGalacticMapPainter *pPainter)
	{
    //  Save some parameters

    RECT rcOldView = pPainter->GetViewport();
    int xOldCenter, yOldCenter;
    pPainter->GetPos(&xOldCenter, &yOldCenter);
    int iOldScale = pPainter->GetScale();

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

    pPainter->SetViewport(rcView);
    pPainter->SetScale(100);
    pPainter->SetPos(0, 0);
	pPainter->Paint(FullMap);

	//	Copy to cliboard

	FullMap.CopyToClipboard();

    //  Restore

    pPainter->SetScale(iOldScale);
    pPainter->SetViewport(rcOldView);
    pPainter->SetPos(xOldCenter, yOldCenter);
	}
