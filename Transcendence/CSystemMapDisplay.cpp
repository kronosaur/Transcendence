//	CSystemMapDisplay.cpp
//
//	CSystemMapPainter class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define STR_HELP_DESC						CONSTLIT("[+] and [-] to zoom map\n[H] to toggle HUD on/off")

const int MAP_ZOOM_SPEED =					16;
const int HELP_PANE_WIDTH = 280;
const int SCREEN_BORDER_X = 10;
const int SCREEN_BORDER_Y = 10;

CSystemMapDisplay::CSystemMapDisplay (CHumanInterface &HI, CTranscendenceModel &Model, CHeadsUpDisplay &HUD) :
        m_HI(HI),
        m_Model(Model),
        m_HUD(HUD),
        m_iMapScale(1),
        m_iMapZoomEffect(0),
        m_HelpPainter(HI.GetVisuals())

//  CSystemMapDisplay constructor

    {
    int i;

    m_rMapScale[0] = g_AU / 800.0;
	for (i = 1; i < MAP_SCALE_COUNT; i++)
		m_rMapScale[i] = m_rMapScale[i - 1] * 2.0;
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
			if (m_iMapScale < (MAP_SCALE_COUNT - 1))
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				m_iMapScale++;
				m_iMapZoomEffect = 100;
				}
			break;

		case VK_ADD:
		case VK_OEM_PLUS:
			if (m_iMapScale > 0)
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				m_iMapScale--;
				m_iMapZoomEffect = -100;
				}
			break;

        default:
            return false;
		}

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

    m_HelpPainter.SetWidth(HELP_PANE_WIDTH);
    m_HelpPainter.SetTitle(pSystem->GetName());
    m_HelpPainter.SetDesc(STR_HELP_DESC);

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
    }

void CSystemMapDisplay::Paint (CG32bitImage &Screen)

//  Paint
//
//  Paint the map

    {
	SetProgramState(psPaintingMap);

	//	Paint the map

	Metric rScale = m_rMapScale[m_iMapScale];
	if (m_iMapZoomEffect != 0)
		{
		if (m_iMapZoomEffect > 0)
			{
			rScale = rScale * (1.0 - (m_iMapZoomEffect / 200.0));
			m_iMapZoomEffect = Max(0, m_iMapZoomEffect - MAP_ZOOM_SPEED);
			}
		else
			{
			rScale = rScale * (1.0 - (m_iMapZoomEffect / 100.0));
			m_iMapZoomEffect = Min(0, m_iMapZoomEffect + MAP_ZOOM_SPEED);
			}
		}

	m_Model.GetUniverse().PaintPOVMap(Screen, m_rcScreen, rScale);

    //  Paint help text

    RECT rcRect;
    m_HUD.GetClearHorzRect(&rcRect);
    int cyBottom = (m_Model.GetPlayer() && m_Model.GetPlayer()->IsMapHUDActive() ? rcRect.bottom : m_rcScreen.bottom);
    m_HelpPainter.Paint(Screen, rcRect.left + SCREEN_BORDER_X, cyBottom - (SCREEN_BORDER_Y + m_HelpPainter.GetHeight()));

    //  Done

	SetProgramState(psAnimating);
    }
