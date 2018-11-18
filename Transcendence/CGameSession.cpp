//	CGameSession.cpp
//
//	CGameSession class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

CGameSession::CGameSession (STranscendenceSessionCtx &CreateCtx) : IHISession(*CreateCtx.pHI),
		m_Settings(*CreateCtx.pSettings),
        m_Model(*CreateCtx.pModel),
		m_DebugConsole(*CreateCtx.pDebugConsole),
		m_Soundtrack(*CreateCtx.pSoundtrack),
		m_iUI(uiNone),
		m_bMouseAim(true),
        m_HUD(*CreateCtx.pHI, *CreateCtx.pModel),
        m_bShowingSystemMap(false),
        m_SystemMap(*CreateCtx.pHI, *CreateCtx.pModel, m_HUD),
		m_CurrentMenu(menuNone),
		m_pCurrentComms(NULL),
        m_iDamageFlash(0),
		m_bIgnoreButtonUp(false),
		m_bIgnoreMouseMove(false),
		m_CurrentDock(*this)

//	CGameSession constructor

	{
	}

void CGameSession::DismissMenu (void)

//	DismissMenu
//
//	The menu has already been hidden (in g_pTrans) but we need to dismiss it
//	for our own context.

	{
	if (m_CurrentMenu != menuNone)
		{
		m_CurrentMenu = menuNone;

		//	Mouse controls the ship again

		ShowCursor(false);
		SyncMouseToPlayerShip();

		//	Ignore the next mouse move message, for purpose of enabling mouse
		//	control.

		m_bIgnoreMouseMove = true;
		}
	}

IPlayerController::EUIMode CGameSession::GetUIMode (void) const

//	GetUIMode
//
//	Returns the current UI mode

	{
	switch (g_pTrans->m_State)
		{
		case CTranscendenceWnd::gsInGame:
			return IPlayerController::uimodeSpace;

		case CTranscendenceWnd::gsDocked:
			return IPlayerController::uimodeDockScreen;

		case CTranscendenceWnd::gsEnteringStargate:
		case CTranscendenceWnd::gsWaitingForSystem:
		case CTranscendenceWnd::gsLeavingStargate:
			return IPlayerController::uimodeInStargate;

		default:
			return IPlayerController::uimodeUnknown;
		}
	}

void CGameSession::HideMenu (void)

//	HideMenu
//
//	Hides the current menu.

	{
	switch (m_CurrentMenu)
		{
		case menuNone:
			return;

		case menuComms:
			g_pTrans->HideCommsTargetMenu(NULL);
			g_pTrans->m_CurrentMenu = CTranscendenceWnd::menuNone;
			break;

		case menuCommsSquadron:
			g_pTrans->m_CurrentMenu = CTranscendenceWnd::menuNone;
			break;

		case menuCommsTarget:
			g_pTrans->HideCommsTargetMenu(m_pCurrentComms);
			g_pTrans->m_CurrentMenu = CTranscendenceWnd::menuNone;
			break;

		case menuDebugConsole:
			m_DebugConsole.SetEnabled(false);
			break;

		case menuEnableDevice:
			g_pTrans->m_CurrentPicker = CTranscendenceWnd::pickNone;
			break;

		case menuGame:
			g_pTrans->m_CurrentMenu = CTranscendenceWnd::menuNone;
			break;

		case menuInvoke:
			g_pTrans->m_CurrentMenu = CTranscendenceWnd::menuNone;
			break;

		case menuSelfDestructConfirm:
			g_pTrans->m_CurrentMenu = CTranscendenceWnd::menuNone;
			break;

		case menuUseItem:
			g_pTrans->m_CurrentPicker = CTranscendenceWnd::pickNone;
			break;
		}

	DismissMenu();
	}

void CGameSession::InitUI (void)

//	InitUI
//
//	Initializes the primary UI based on the player ship

	{
	//	Default to none

	m_iUI = uiNone;

	//	Get the player ship settings.
	//
	//	NOTE: For now we get the UI settings from the class, but in the future we
	//	could get it from the ship, if we want to allow switching UIs without
	//	switching ship classes.

	CPlayerShipController *pPlayer = m_Model.GetPlayer();
	if (pPlayer == NULL)
		return;

	CShip *pPlayerShip = pPlayer->GetShip();
	if (pPlayerShip == NULL)
		return;

	const CPlayerSettings *pPlayerSettings = pPlayerShip->GetClass()->GetPlayerSettings();
	if (pPlayerSettings == NULL)
		return;

	m_iUI = pPlayerSettings->GetDefaultUI();

	//	Initialize some variables based on UI

	switch (m_iUI)
		{
		case uiPilot:
			m_bMouseAim = !m_Settings.GetBoolean(CGameSettings::noMouseAim);
			break;

		case uiCommand:
			m_bMouseAim = false;
			break;

		default:
			ASSERT(false);
			break;
		}

	//	Mouse aim setting might have changed since the last time we loaded the game,
	//	but since the player controller keeps its own state, we need to tell it
	//	about our current state.

	pPlayer->OnMouseAimSetting(m_bMouseAim);
	}

void CGameSession::OnCleanUp (void)

//  OnCleanUp
//
//  We're going away

    {
	HideMenu();
    }

ALERROR CGameSession::OnInit (CString *retsError)

//  OnInit
//
//  Initialize session

    {
    m_rcScreen = g_pTrans->m_rcScreen;
    SetNoCursor(true);
	InitUI();
    m_HUD.Init(m_rcScreen);
    m_SystemMap.Init(m_rcScreen);

	//	Move the mouse cursor so that it points to where the ship is points.
	//	Otherwise the ship will try to turn to point to the mouse.

	SyncMouseToPlayerShip();

    return NOERROR;
    }

void CGameSession::OnObjDestroyed (const SDestroyCtx &Ctx)

//	OnObjDestroyed
//
//	An object has been destroyed

	{
	//	If we're showing the comms menu, reload the list (in case
	//	any ships got destroyed)

	if (m_CurrentMenu == menuCommsTarget)
		{
		HideMenu();
		ShowMenu(menuCommsTarget);
		}
	else if (m_CurrentMenu == menuComms && m_pCurrentComms == Ctx.pObj)
		{
		HideMenu();
		m_pCurrentComms = NULL;
		}
	}

void CGameSession::OnPlayerDestroyed (SDestroyCtx &Ctx, const CString &sEpitaph)

//  OnPlayerDestroyed
//
//  Player was destroyed

    {
	DEBUG_TRY

	//	Clean up

	HideMenu();
	g_pTrans->m_bAutopilot = false;
	m_bShowingSystemMap = false;
	if (g_pTrans->m_State == CTranscendenceWnd::gsDocked)
		m_Model.GetPlayer()->Undock();

	//	Update display

	CString sMsg = sEpitaph;
	if (strEquals(strWord(sMsg, 0), CONSTLIT("was")))
		sMsg = strSubString(sMsg, 4, -1);
	sMsg.Capitalize(CString::capFirstLetter);
	g_pTrans->DisplayMessage(sMsg);
    m_HUD.Invalidate(hudArmor);

	//	If we are insured, then set our state so that we come back to life
	if (Ctx.bResurrectPending)
		{
		//	Prepare resurrect

		g_pTrans->m_State = CTranscendenceWnd::gsDestroyed;
		g_pTrans->m_iCountdown = TICKS_AFTER_DESTROYED;
		}

	//	Otherwise, proceed with destruction

	else
		{
		//	Done with ship screens

		g_pTrans->CleanUpPlayerShip();

		//	Player destroyed

		g_pTrans->m_State = CTranscendenceWnd::gsDestroyed;
		g_pTrans->m_iCountdown = TICKS_AFTER_DESTROYED;
		}

	DEBUG_CATCH
    }
    
void CGameSession::OnPlayerEnteredStargate (CTopologyNode *pNode)

//  OnPlayerEnteredStargate
//
//  Player entered the stargate

    {
    ShowSystemMap(false);
    }

void CGameSession::OnShowDockScreen (bool bShow)

//  OnShowDockScreen
//
//  Show or hide the dock screen.

    {
	if (bShow && g_pTrans->m_State != CTranscendenceWnd::gsDocked)
		{
		//	Show the cursor, if it was previously hidden

		if (g_pTrans->m_State == CTranscendenceWnd::gsInGame)
			ShowCursor(true);

		//	New state

		g_pUniverse->SetLogImageLoad(false);
		g_pTrans->m_State = CTranscendenceWnd::gsDocked;
		}
	else if (g_pTrans->m_State == CTranscendenceWnd::gsDocked)
		{
		//	Deselect armor

		m_HUD.SetArmorSelection(-1);

		//	Hide the cursor

		ShowCursor(false);
		SyncMouseToPlayerShip();
		m_bIgnoreMouseMove = true;

		//	New state

		g_pUniverse->SetLogImageLoad(true);
		g_pTrans->m_State = CTranscendenceWnd::gsInGame;

		//	Clean up

		g_pTrans->m_pCurrentScreen = NULL;
		}
    }

void CGameSession::PaintInfoText (CG32bitImage &Dest, const CString &sTitle, const TArray<CString> &Body, bool bAboveTargeting)

//	PaintInfoText
//
//	Paints info text on the screen

	{
	int i;

	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &TitleFont = VI.GetFont(fontHeader);
	const CG16bitFont &BodyFont = VI.GetFont(fontMedium);

	int x = m_rcScreen.left + 2 * TitleFont.GetAverageWidth();
	int cy = TitleFont.GetHeight() + Body.GetCount() * BodyFont.GetHeight();
	int cySpacing = BodyFont.GetHeight() / 2;

    RECT rcCenter;
    m_HUD.GetClearHorzRect(&rcCenter);
	int y = (bAboveTargeting ? rcCenter.bottom : m_rcScreen.bottom) - (cy + cySpacing);

	Dest.DrawText(x,
			y,
			TitleFont,
			VI.GetColor(colorTextHighlight),
			sTitle);
	y += TitleFont.GetHeight();

	//	Paint the body

	for (i = 0; i < Body.GetCount(); i++)
		{
		Dest.DrawText(x,
				y,
				BodyFont,
				VI.GetColor(colorTextDialogLabel),
				Body[i]);
		y += BodyFont.GetHeight();
		}
	}

void CGameSession::PaintSoundtrackTitles (CG32bitImage &Dest)

//	PaintSoundtrackTitles
//
//	Paints the info about the currently playing soundtrack.

	{
	int iPos;
	CMusicResource *pTrack = m_Soundtrack.GetCurrentTrack(&iPos);
	if (pTrack == NULL)
		return;

	int iSegment = pTrack->FindSegment(iPos);

	//	Time

	int iMilliseconds = iPos % 1000;
	int iSeconds = (iPos / 1000) % 60;
	int iMinutes = (iPos / 60000);

	//	Add all the components

	TArray<CString> Desc;
	Desc.InsertEmpty(3);
	Desc[0] = pTrack->GetPerformedBy();
	Desc[1] = pTrack->GetFilename();
	Desc[2] = strPatternSubst(CONSTLIT("Segment %d of %d [%02d:%02d.%03d]"), iSegment + 1, pTrack->GetSegmentCount(), iMinutes, iSeconds, iMilliseconds);

	//	Paint

	PaintInfoText(Dest, pTrack->GetTitle(), Desc, true);
	}

bool CGameSession::ShowMenu (EMenuTypes iMenu)

//	ShowMenu
//
//	Shows the given menu.

	{
	CPlayerShipController *pPlayer = m_Model.GetPlayer();
	if (pPlayer == NULL)
		return false;

	//	Can't do anything if we're currently showing a menu. Need to dismiss
	//	first.

	if (m_CurrentMenu != menuNone)
		return false;

	//	Bring up the menu

	switch (iMenu)
		{
		case menuNone:
			return false;

		case menuComms:
			if (m_pCurrentComms == NULL)
				return false;

			g_pTrans->ShowCommsMenu(m_pCurrentComms);
			break;

		case menuCommsSquadron:
			g_pTrans->ShowCommsSquadronMenu();
			if (g_pTrans->m_CurrentMenu == CTranscendenceWnd::menuNone)
				return false;
			break;

		case menuCommsTarget:
			pPlayer->SetUIMessageEnabled(uimsgCommsHint, false);
			g_pTrans->ShowCommsTargetMenu();
			if (g_pTrans->m_CurrentMenu == CTranscendenceWnd::menuNone)
				return false;
			break;

		case menuDebugConsole:
			m_DebugConsole.SetEnabled(true);
			break;

		case menuEnableDevice:
			g_pTrans->ShowEnableDisablePicker();
			if (g_pTrans->m_CurrentPicker == CTranscendenceWnd::pickNone)
				return false;
			break;

		case menuGame:
			g_pTrans->ShowGameMenu();
			break;

		case menuInvoke:
			g_pTrans->ShowInvokeMenu();
			if (g_pTrans->m_CurrentMenu == CTranscendenceWnd::menuNone)
				return false;
			break;

		case menuUseItem:
			g_pTrans->ShowUsePicker();
			if (g_pTrans->m_CurrentPicker == CTranscendenceWnd::pickNone)
				return false;
			break;

		default:
			return false;
		}

	//	Show our cursor, in case the menus have mouse UI

	ShowCursor(true);

	//	Set state

	m_CurrentMenu = iMenu;

	return true;
	}

void CGameSession::ShowSystemMap (bool bShow)

//  ShowSystemMap
//
//  Shows or hides the system map.

    {
    if (m_bShowingSystemMap != bShow)
        {
        m_bShowingSystemMap = bShow;

        if (m_bShowingSystemMap)
            m_SystemMap.OnShowMap();
        else
            m_SystemMap.OnHideMap();
        }
    }

void CGameSession::SyncMouseToPlayerShip (void)

//	SyncMouseToPlayerShip
//
//	Move the mouse so it points where the player ship is pointing.

	{
	CPlayerShipController *pPlayer = m_Model.GetPlayer();
	if (pPlayer == NULL)
		return;

	CShip *pPlayerShip = pPlayer->GetShip();
	if (pPlayerShip == NULL)
		return;

	//	If we're not using the mouse to move the ship, then we don't need to
	//	do this.

	if (!IsMouseAimEnabled())
		return;

	//	Create a vector that points to where the mouse should be relative to the
	//	center of the viewport. We pick 300 pixels because that's the largest value
	//	that would fit on a 1024x600 screen (our minimum req).

	CVector vMouse = PolarToVector(pPlayerShip->GetRotation(), 300.0);

	//	Compute the center of the viewport in screen coordinates

	int xCenter = m_HI.GetScreenWidth() / 2;
	int yCenter = m_HI.GetScreenHeight() / 2;

	//	Compute the mouse position in screen coordinates.

	int xMouse = xCenter + (int)vMouse.GetX();
	int yMouse = yCenter - (int)vMouse.GetY();

	//	Convert to global coordinates

	int xGlobal, yGlobal;
	m_HI.GetScreenMgr().LocalToGlobal(xMouse, yMouse, &xGlobal, &yGlobal);

	::SetCursorPos(xGlobal, yGlobal);
	}
