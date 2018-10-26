//	CHeadsUpDisplay.cpp
//
//	CHeadsUpDisplay class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

void CHeadsUpDisplay::CleanUp (void)

//  CleanUp
//
//  Release resources

    {
	if (m_pArmorPainter)
		{
		delete m_pArmorPainter;
		m_pArmorPainter = NULL;
		}

	if (m_pShieldsPainter)
		{
		delete m_pShieldsPainter;
		m_pShieldsPainter = NULL;
		}

    if (m_pReactorPainter)
        {
        delete m_pReactorPainter;
        m_pReactorPainter = NULL;
        }

    if (m_pWeaponsPainter)
        {
        delete m_pWeaponsPainter;
        m_pWeaponsPainter = NULL;
        }
    }

void CHeadsUpDisplay::GetClearHorzRect (RECT *retrcRect) const

//  GetClearHorzRect
//
//  Returns a RECT that excludes all corner HUD elements.

    {
    RECT rcReactor;
    if (m_pReactorPainter)
        m_pReactorPainter->GetRect(&rcReactor);

    RECT rcArmor;
    if (m_pArmorPainter)
        m_pArmorPainter->GetRect(&rcArmor);

    RECT rcWeapons;
    if (m_pWeaponsPainter)
        m_pWeaponsPainter->GetRect(&rcWeapons);

    retrcRect->left = m_rcScreen.left;
    retrcRect->right = m_rcScreen.right;
    retrcRect->top = (m_pReactorPainter ? rcReactor.bottom : m_rcScreen.top);
    retrcRect->bottom = Min((m_pArmorPainter ? rcArmor.top : m_rcScreen.bottom), (m_pWeaponsPainter ? rcWeapons.top : m_rcScreen.bottom));
    }

bool CHeadsUpDisplay::Init (const RECT &rcRect)

//  Init
//
//  Initialize

    {
    CPlayerShipController *pPlayer = m_Model.GetPlayer();
    if (pPlayer == NULL)
        return false;

    CShip *pShip = pPlayer->GetShip();
    if (pShip == NULL)
        return false;

    CleanUp();
    m_rcScreen = rcRect;

    //  Initialize armor

	SDesignLoadCtx Ctx;
	m_pArmorPainter = IHUDPainter::Create(Ctx, pShip->GetClass(), hudArmor);
	m_pShieldsPainter = IHUDPainter::Create(Ctx, pShip->GetClass(), hudShields);

	if (m_pArmorPainter)
		m_pArmorPainter->SetLocation(rcRect, IHUDPainter::locAlignBottom | IHUDPainter::locAlignRight);

    //  Initialize reactor

	m_pReactorPainter = IHUDPainter::Create(Ctx, pShip->GetClass(), hudReactor);
	if (m_pReactorPainter)
		m_pReactorPainter->SetLocation(rcRect, IHUDPainter::locAlignTop | IHUDPainter::locAlignLeft);

    //  Initialize weapons

	m_pWeaponsPainter = IHUDPainter::Create(Ctx, pShip->GetClass(), hudTargeting);
	if (m_pWeaponsPainter)
		m_pWeaponsPainter->SetLocation(rcRect, IHUDPainter::locAlignBottom | IHUDPainter::locAlignLeft);

    //  Success!

    return true;
    }

void CHeadsUpDisplay::Invalidate (EHUDTypes iHUD)

//  Invalidate
//
//  Invalidates a specific HUD. If iHUD == hudNone, then we invalidate all HUDs.

    {
    switch (iHUD)
        {
        case hudNone:
            if (m_pArmorPainter)
                m_pArmorPainter->Invalidate();

            if (m_pShieldsPainter)
                m_pShieldsPainter->Invalidate();

            if (m_pReactorPainter)
                m_pReactorPainter->Invalidate();

            if (m_pWeaponsPainter)
                m_pWeaponsPainter->Invalidate();
            break;

        case hudArmor:
        case hudShields:
            if (m_pArmorPainter)
                m_pArmorPainter->Invalidate();

            if (m_pShieldsPainter)
                m_pShieldsPainter->Invalidate();
            break;

        case hudReactor:
            if (m_pReactorPainter)
                m_pReactorPainter->Invalidate();
            break;

        case hudTargeting:
            if (m_pWeaponsPainter)
                m_pWeaponsPainter->Invalidate();
            break;

        default:
            ASSERT(false);
            break;
        }
    }

void CHeadsUpDisplay::Paint (CG32bitImage &Screen, bool bInDockScreen)

//  Paint
//
//  Paint

    {
	DEBUG_TRY

    CPlayerShipController *pPlayer = m_Model.GetPlayer();
    if (pPlayer == NULL)
        return;

	SHUDPaintCtx PaintCtx;
	PaintCtx.pSource = pPlayer->GetShip();
	PaintCtx.byOpacity = g_pUniverse->GetSFXOptions().GetHUDOpacity();

    //  If we're in a dock screen, we don't always paint the HUD if we don't 
    //  have enough room.

    bool bPaintTop = true;
    bool bPaintBottom = true;
    if (bInDockScreen)
        {
        if (Screen.GetHeight() < 768)
            bPaintBottom = false;

        if (Screen.GetHeight() < 960)
            bPaintTop = false;
        }

    //  Paint armor

    if (m_pArmorPainter && bPaintBottom)
        {
	    PaintCtx.iSegmentSelected = m_iSelection;
	    PaintCtx.pShieldsHUD = m_pShieldsPainter;

		SetProgramState(psPaintingArmorDisplay);
    	m_pArmorPainter->Paint(Screen, PaintCtx);

        PaintCtx.iSegmentSelected = -1;
        PaintCtx.pShieldsHUD = NULL;
        }

    //  Paint reactor

    if (m_pReactorPainter && bPaintTop)
        {
        SetProgramState(psPaintingReactorDisplay);
        m_pReactorPainter->Paint(Screen, PaintCtx);
        }

    //  Paint weapons

    if (m_pWeaponsPainter && bPaintBottom)
        {
        SetProgramState(psPaintingTargetDisplay);
        m_pWeaponsPainter->Paint(Screen, PaintCtx);
        }

    SetProgramState(psAnimating);

	DEBUG_CATCH
    }

void CHeadsUpDisplay::SetArmorSelection (int iSelection)

//  SetArmorSelection
//
//  Selects a given armor section.

    {
    if (m_pArmorPainter
            && m_iSelection != iSelection)
        {
        m_iSelection = iSelection;
        Invalidate(hudArmor);
        }
    }

void CHeadsUpDisplay::Update (int iTick)

//  Update
//
//  Update displays

    {
	DEBUG_TRY

    CPlayerShipController *pPlayer = m_Model.GetPlayer();
    if (pPlayer == NULL)
        return;

	SHUDUpdateCtx UpdateCtx;
	UpdateCtx.pSource = pPlayer->GetShip();
	UpdateCtx.iTick = iTick;

    if (m_pArmorPainter)
        m_pArmorPainter->Update(UpdateCtx);

    if (m_pShieldsPainter)
        m_pShieldsPainter->Update(UpdateCtx);

    if (m_pReactorPainter)
        m_pReactorPainter->Update(UpdateCtx);

    if (m_pWeaponsPainter)
        m_pWeaponsPainter->Update(UpdateCtx);

	DEBUG_CATCH
    }
