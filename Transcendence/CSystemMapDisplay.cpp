//	CSystemMapDisplay.cpp
//
//	CSystemMapPainter class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define STR_HELP_DESC						CONSTLIT("Scroll wheel to zoom map\n[H] to toggle HUD on/off")

const Metric SCALE_100 = g_AU / 400.0;
const int MAP_ZOOM_SPEED =					16;
const int HELP_PANE_WIDTH = 280;
const int SCREEN_BORDER_X = 10;
const int SCREEN_BORDER_Y = 10;

static CMapLegendPainter::SScaleEntry LEGEND_SCALE[] =
    {
        {   800,        "light-seconds",  LIGHT_SECOND },
        {   400,        "light-seconds",  LIGHT_SECOND },
        {   200,        "light-seconds",  LIGHT_SECOND },
        {   100,        "light-seconds",  LIGHT_SECOND },
        {   50,         "light-seconds",  LIGHT_SECOND },
        {   25,         "light-seconds",  LIGHT_SECOND },
        {   10,         "light-seconds",  LIGHT_SECOND },
    };

const int LEGEND_SCALE_COUNT = (sizeof(LEGEND_SCALE) / sizeof(LEGEND_SCALE[0]));

CSystemMapDisplay::CSystemMapDisplay (CHumanInterface &HI, CTranscendenceModel &Model, CHeadsUpDisplay &HUD) :
        m_HI(HI),
        m_Model(Model),
        m_HUD(HUD),
        m_HelpPainter(HI.GetVisuals(), LEGEND_SCALE, LEGEND_SCALE_COUNT)

//  CSystemMapDisplay constructor

    {
    }

Metric CSystemMapDisplay::GetScaleKlicksPerPixel (int iScale) const

//  GetScaleKlicksPerPixel
//
//  Converts from a relative to absolute scale

    {
    return 100.0 * SCALE_100 / iScale;
    }

bool CSystemMapDisplay::HandleKeyDown (int iVirtKey, DWORD dwKeyData)

//  HandleKeyDown
//
//  Handles a key message. Returns TRUE if we handled it.

    {
    CPlayerShipController *pPlayer = m_Model.GetPlayer();
    if (pPlayer == NULL)
        return false;

	switch (iVirtKey)
		{
		case 'H':
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			pPlayer->SetMapHUD(!pPlayer->IsMapHUDActive());
			break;

		case VK_SUBTRACT:
		case VK_OEM_MINUS:
            if (m_Scale.CanZoomOut())
                {
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
                m_Scale.ZoomOut();
                }
			break;

		case VK_ADD:
		case VK_OEM_PLUS:
            if (m_Scale.CanZoomIn())
                {
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
                m_Scale.ZoomIn();
                }
			break;

        default:
            return false;
		}

    return true;
    }

bool CSystemMapDisplay::HandleMouseWheel (int iDelta, int x, int y, DWORD dwFlags)

//  HandleMouseWheel
//
//  Zoom in/out

    {
    m_Scale.ZoomWheel(iDelta);
    return true;
    }

bool CSystemMapDisplay::Init (const RECT &rcRect)

//  Init
//
//  Initialize

    {
    m_rcScreen = rcRect;

    //  Initialize help pane

    CSystem *pSystem = m_Model.GetUniverse().GetCurrentSystem();
    if (pSystem == NULL)
        return false;

    m_Scale.Init(100, 25, 200);

    m_HelpPainter.SetWidth(HELP_PANE_WIDTH);
    m_HelpPainter.SetTitle(pSystem->GetName());
    m_HelpPainter.SetDesc(STR_HELP_DESC);

    m_HelpPainter.SetScale(GetScaleKlicksPerPixel(m_Scale.GetScale()));

    return true;
    }

void CSystemMapDisplay::OnHideMap (void)

//  OnHideMap
//
//  Clean up any map resources.

    {
    }

void CSystemMapDisplay::OnShowMap (void)

//  OnShowMap
//
//  Prepare the map for painter. We use this method to cache any values for
//  later.

    {
    CSystem *pSystem = m_Model.GetUniverse().GetCurrentSystem();
    if (pSystem == NULL)
        return;

    m_HelpPainter.SetTitle(pSystem->GetName());
    }

void CSystemMapDisplay::Paint (CG32bitImage &Screen)

//  Paint
//
//  Paint the map

    {
	SetProgramState(psPaintingMap);

    //  Update zoom effect and compute scale

    bool bScaleChanged = m_Scale.Update();
    Metric rScale = GetScaleKlicksPerPixel(m_Scale.GetScale());

	//	Paint the map

	m_Model.GetUniverse().PaintPOVMap(Screen, m_rcScreen, rScale);

    //  Paint help text

    if (bScaleChanged)
        m_HelpPainter.SetScale(rScale);

    RECT rcRect;
    m_HUD.GetClearHorzRect(&rcRect);
    int cyBottom = (m_Model.GetPlayer() && m_Model.GetPlayer()->IsMapHUDActive() ? rcRect.bottom : m_rcScreen.bottom);
    m_HelpPainter.Paint(Screen, rcRect.left + SCREEN_BORDER_X, cyBottom - (SCREEN_BORDER_Y + m_HelpPainter.GetHeight()));

    //  Done

	SetProgramState(psAnimating);
    }
