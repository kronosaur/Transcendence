//	CGameSession.cpp
//
//	CGameSession class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

void CGameSession::ExecuteCommand (CPlayerShipController *pPlayer, CGameKeys::Keys iCommand)

//	ExecuteCommand
//
//	Executes the given command.

	{
	ASSERT(pPlayer);
	if (pPlayer == NULL)
		return;

	switch (iCommand)
		{
		case CGameKeys::keyAutopilot:
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			g_pTrans->Autopilot(!g_pTrans->m_bAutopilot);
			pPlayer->SetUIMessageEnabled(uimsgAutopilotHint, false);
			break;

		case CGameKeys::keyEnableDevice:
			if (!pPlayer->DockingInProgress() 
					&& !pPlayer->GetShip()->IsParalyzed()
					&& !pPlayer->GetShip()->IsOutOfPower()
					&& !pPlayer->GetShip()->IsTimeStopped())
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				g_pTrans->Autopilot(false);
				ShowMenu(menuEnableDevice);
				}
			break;

		case CGameKeys::keyCommunications:
			if (!pPlayer->DockingInProgress()
					&& !pPlayer->GetShip()->IsTimeStopped())
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				g_pTrans->Autopilot(false);
				ShowMenu(menuCommsTarget);
				}
			break;

		case CGameKeys::keyDock:
			if (!pPlayer->GetShip()->IsOutOfPower()
					&& !pPlayer->GetShip()->IsParalyzed()
					&& !pPlayer->GetShip()->IsTimeStopped())
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
				g_pTrans->Autopilot(false);
				pPlayer->Dock();
				}
			break;

		case CGameKeys::keyTargetNextFriendly:
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			pPlayer->SelectNextFriendly(1);
			break;

		case CGameKeys::keyTargetPrevFriendly:
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			pPlayer->SelectNextFriendly(-1);
			break;

		case CGameKeys::keyEnterGate:
			if (!pPlayer->DockingInProgress()
					&& !pPlayer->GetShip()->IsParalyzed()
					&& !pPlayer->GetShip()->IsOutOfPower()
					&& !pPlayer->GetShip()->IsTimeStopped())
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
				g_pTrans->Autopilot(false);
				pPlayer->Gate();
				}
			break;

		case CGameKeys::keyInvokePower:
			if (!pPlayer->DockingInProgress()
					&& !pPlayer->GetShip()->IsTimeStopped())
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				g_pTrans->Autopilot(false);
				ShowMenu(menuInvoke);
				}
			break;

		case CGameKeys::keyShowMap:
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			if (m_bShowingSystemMap)
				g_pTrans->Autopilot(false);
            ShowSystemMap(!m_bShowingSystemMap);
			pPlayer->SetUIMessageEnabled(uimsgMapHint, false);
			break;

		case CGameKeys::keyShowGalacticMap:
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			g_pHI->HICommand(CONSTLIT("uiShowGalacticMap"));
			pPlayer->SetUIMessageEnabled(uimsgGalacticMapHint, false);
			break;

		case CGameKeys::keyPause:
			{
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
			g_pTrans->m_bPaused = true;
			if (pPlayer)
				{
				pPlayer->SetThrust(false);
				pPlayer->SetManeuver(NoRotation);
				pPlayer->SetFireMain(false);
				pPlayer->SetFireMissile(false);
				}
			g_pHI->HICommand(CONSTLIT("gamePause"));
			g_pTrans->DisplayMessage(CONSTLIT("Game paused"));
			break;
			}

		case CGameKeys::keySquadronCommands:
			if (!pPlayer->DockingInProgress()
					&& !pPlayer->GetShip()->IsTimeStopped())
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				g_pTrans->Autopilot(false);
				ShowMenu(menuCommsSquadron);
				}
			break;

		case CGameKeys::keyClearTarget:
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			pPlayer->SetTarget(NULL);
			break;

		case CGameKeys::keyShipStatus:
			if (pPlayer->CanShowShipStatus())
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				m_Model.ShowShipScreen();
				}
			break;

		case CGameKeys::keyTargetNextEnemy:
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			pPlayer->SelectNextTarget(1);
			break;

		case CGameKeys::keyTargetPrevEnemy:
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			pPlayer->SelectNextTarget(-1);
			break;

		case CGameKeys::keyUseItem:
			if (!pPlayer->DockingInProgress()
					&& !pPlayer->GetShip()->IsTimeStopped())
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				g_pTrans->Autopilot(false);
				ShowMenu(menuUseItem);
				}
			break;

		case CGameKeys::keyNextWeapon:
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			g_pTrans->Autopilot(false);
			pPlayer->SetFireMain(false);
			pPlayer->ReadyNextWeapon(1);
			break;

		case CGameKeys::keyPrevWeapon:
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			g_pTrans->Autopilot(false);
			pPlayer->SetFireMain(false);
			pPlayer->ReadyNextWeapon(-1);
			break;

		case CGameKeys::keyThrustForward:
			if (!pPlayer->DockingInProgress()
					&& !pPlayer->GetShip()->IsOutOfPower()
					&& !pPlayer->GetShip()->IsTimeStopped())
				{
				g_pTrans->Autopilot(false);
				pPlayer->SetThrust(true);
				}
			break;

		case CGameKeys::keyRotateLeft:
			if (!pPlayer->GetShip()->IsOutOfPower()
					&& !pPlayer->GetShip()->IsTimeStopped())
				{
				g_pTrans->Autopilot(false);
				pPlayer->SetManeuver(RotateLeft);
				}
			break;

		case CGameKeys::keyRotateRight:
			if (!pPlayer->GetShip()->IsOutOfPower()
					&& !pPlayer->GetShip()->IsTimeStopped())
				{
				g_pTrans->Autopilot(false);
				pPlayer->SetManeuver(RotateRight);
				}
			break;

		case CGameKeys::keyStop:
			if (!pPlayer->DockingInProgress()
					&& !pPlayer->GetShip()->IsOutOfPower()
					&& !pPlayer->GetShip()->IsTimeStopped())
				{
				g_pTrans->Autopilot(false);
				pPlayer->SetStopThrust(true);
				}
			break;

		case CGameKeys::keyFireWeapon:
			if (!pPlayer->DockingInProgress()
					&& !pPlayer->GetShip()->IsOutOfPower()
					&& !pPlayer->GetShip()->IsTimeStopped())
				{
				g_pTrans->Autopilot(false);
				pPlayer->SetFireMain(true);
				}
			break;

		case CGameKeys::keyFireMissile:
			if (!pPlayer->DockingInProgress()
					&& !pPlayer->GetShip()->IsOutOfPower()
					&& !pPlayer->GetShip()->IsTimeStopped())
				{
				g_pTrans->Autopilot(false);
				pPlayer->SetFireMissile(true);
				pPlayer->SetUIMessageEnabled(uimsgFireMissileHint, false);
				}
			break;

		case CGameKeys::keyNextMissile:
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			g_pTrans->Autopilot(false);
			pPlayer->ReadyNextMissile(1);
			pPlayer->SetUIMessageEnabled(uimsgSwitchMissileHint, false);
			break;

		case CGameKeys::keyPrevMissile:
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			g_pTrans->Autopilot(false);
			pPlayer->ReadyNextMissile(-1);
			pPlayer->SetUIMessageEnabled(uimsgSwitchMissileHint, false);
			break;

		case CGameKeys::keyShowHelp:
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			g_pHI->HICommand(CONSTLIT("uiShowHelp"));
			break;

		case CGameKeys::keyShowGameStats:
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			g_pHI->HICommand(CONSTLIT("uiShowGameStats"));
			break;

		case CGameKeys::keyVolumeDown:
			g_pHI->HICommand(CONSTLIT("uiVolumeDown"));
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			break;

		case CGameKeys::keyVolumeUp:
			g_pHI->HICommand(CONSTLIT("uiVolumeUp"));
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
			break;

		case CGameKeys::keyShowConsole:
			{
			if (m_Settings.GetBoolean(CGameSettings::debugMode)
					&& !g_pUniverse->IsRegistered())
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				if (m_CurrentMenu == menuDebugConsole)
					HideMenu();
				else
					ShowMenu(menuDebugConsole);
				}
			break;
			}

		case CGameKeys::keyEnableAllDevices:
			if (!pPlayer->GetShip()->IsTimeStopped()
					&& !pPlayer->GetShip()->IsParalyzed()
					&& !pPlayer->GetShip()->IsOutOfPower())
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
				pPlayer->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
				pPlayer->EnableAllDevices(true);
				}
			break;

		case CGameKeys::keyDisableAllDevices:
			if (!pPlayer->GetShip()->IsTimeStopped()
					&& !pPlayer->GetShip()->IsParalyzed()
					&& !pPlayer->GetShip()->IsOutOfPower())
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
				pPlayer->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
				pPlayer->EnableAllDevices(false);
				}
			break;

		case CGameKeys::keyEnableAllDevicesToggle:
			if (!pPlayer->GetShip()->IsTimeStopped()
					&& !pPlayer->GetShip()->IsParalyzed()
					&& !pPlayer->GetShip()->IsOutOfPower())
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
				pPlayer->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
				pPlayer->EnableAllDevices(!pPlayer->AreAllDevicesEnabled());
				}
			break;

		default:
			{
			if (iCommand >= CGameKeys::keyEnableDeviceToggle00 
					&& iCommand <= CGameKeys::keyEnableDeviceToggle31)
				{
				if (!pPlayer->GetShip()->IsTimeStopped()
						&& !pPlayer->GetShip()->IsParalyzed()
						&& !pPlayer->GetShip()->IsOutOfPower())
					{
					int iDevice = (iCommand - CGameKeys::keyEnableDeviceToggle00);

					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
					pPlayer->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
					pPlayer->ToggleEnableDevice(iDevice);
					}
				}
			break;
			}
		}
	}

void CGameSession::ExecuteCommandEnd (CPlayerShipController *pPlayer, CGameKeys::Keys iCommand)

//	ExecuteCommandEnd
//
//	For statefull commands (e.g., thrust) this stops the command.

	{
	switch (iCommand)
		{
		case CGameKeys::keyThrustForward:
			pPlayer->SetThrust(false);
			break;

		case CGameKeys::keyRotateLeft:
			if (pPlayer->GetManeuver() == RotateLeft)
				pPlayer->SetManeuver(NoRotation);
			break;

		case CGameKeys::keyRotateRight:
			if (pPlayer->GetManeuver() == RotateRight)
				pPlayer->SetManeuver(NoRotation);
			break;

		case CGameKeys::keyStop:
			pPlayer->SetStopThrust(false);
			break;

		case CGameKeys::keyFireWeapon:
			pPlayer->SetFireMain(false);
			break;

		case CGameKeys::keyFireMissile:
			pPlayer->SetFireMissile(false);
			break;

		case CGameKeys::keyPause:
			if (g_pTrans->m_bPaused)
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));

				g_pTrans->m_bPaused = false;
				g_pHI->HICommand(CONSTLIT("gameUnpause"));
				g_pTrans->DisplayMessage(CONSTLIT("Game continues"));
				}
			break;
		}
	}
