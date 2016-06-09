//	CGameSession.cpp
//
//	CGameSession class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

void CGameSession::OnChar (char chChar, DWORD dwKeyData)

//  OnChar
//
//  Handle characters
    
    {
	bool bKeyRepeat = uiIsKeyRepeat(dwKeyData);

	//	If we already processed the keydown, then skip it

	if (g_pTrans->m_chKeyDown 
			&& ((chChar == g_pTrans->m_chKeyDown) || (chChar == (g_pTrans->m_chKeyDown + ('a' - 'A')))))
		{
		g_pTrans->m_chKeyDown = '\0';
		return;
		}
	else
		g_pTrans->m_chKeyDown = '\0';

	//	Handle key

	switch (g_pTrans->m_State)
		{
        case CTranscendenceWnd::gsInGame:
			{
            CPlayerShipController *pPlayer = m_Model.GetPlayer();
			if (pPlayer == NULL)
				return;

			//	Handle debug console

			if (g_pTrans->m_bDebugConsole)
				{
				if (chChar >= ' ')
					{
					CString sKey = CString(&chChar, 1);
					g_pTrans->m_DebugConsole.Input(sKey);
					}
				return;
				}

			//	If we're paused, then check for unpause key

			if (g_pTrans->m_bPaused)
				{
				if (chChar == ' ')
					{
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
					g_pTrans->m_bPausedStep = true;
					}
				else
					{
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));

					g_pTrans->m_bPaused = false;
					g_pHI->HICommand(CONSTLIT("gameUnpause"));
					g_pTrans->DisplayMessage(CONSTLIT("Game continues"));
					}
				return;
				}

			//	If we're in a menu, handle it

			if (g_pTrans->m_CurrentMenu != CTranscendenceWnd::menuNone)
				{
				//	Ignore repeat keys (because otherwise we might accidentally
				//	select a menu item from keeping a key pressed too long).

				if (bKeyRepeat)
					return;

				//	Find the menu item and invoke

				CString sKey = CString(&chChar, 1);
				int iIndex;
				if ((iIndex = g_pTrans->m_MenuData.FindItemByKey(sKey)) != -1)
					{
					DWORD dwData = g_pTrans->m_MenuData.GetItemData(iIndex);
					DWORD dwData2 = g_pTrans->m_MenuData.GetItemData2(iIndex);

					switch (g_pTrans->m_CurrentMenu)
						{
                        case CTranscendenceWnd::menuGame:
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
							g_pTrans->DoGameMenuCommand(dwData);
							break;

                        case CTranscendenceWnd::menuSelfDestructConfirm:
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
							g_pTrans->DoSelfDestructConfirmCommand(dwData);
							break;

                        case CTranscendenceWnd::menuComms:
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
							g_pTrans->DoCommsMenu(dwData);
							break;

                        case CTranscendenceWnd::menuCommsTarget:
							{
							CSpaceObject *pObj = (CSpaceObject *)dwData;
							g_pTrans->HideCommsTargetMenu(pObj);
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
							if (pObj)
								g_pTrans->ShowCommsMenu((CSpaceObject *)pObj);
							else
								g_pTrans->ShowCommsSquadronMenu();
							break;
							}

                        case CTranscendenceWnd::menuCommsSquadron:
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
							g_pTrans->DoCommsSquadronMenu(g_pTrans->m_MenuData.GetItemLabel(iIndex), (MessageTypes)dwData, dwData2);
							break;

                        case CTranscendenceWnd::menuInvoke:
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
							g_pTrans->DoInvocation((CPower *)dwData);
							break;
						}
					}
				return;
				}

			//	If we're in a picker, handle it

			if (g_pTrans->m_CurrentPicker != CTranscendenceWnd::pickNone)
				{
				CString sKey = CString(&chChar, 1);
				DWORD dwData;
				bool bHotKey = g_pTrans->m_MenuData.FindItemData(sKey, &dwData);

				if (chChar == ']')
					{
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
					g_pTrans->m_PickerDisplay.SelectNext();
					}
				else if (chChar == '[')
					{
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
					g_pTrans->m_PickerDisplay.SelectPrev();
					}
				else if (bHotKey)
					{
					switch (g_pTrans->m_CurrentPicker)
						{
                        case CTranscendenceWnd::pickUsableItem:
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
							g_pTrans->DoUseItemCommand(dwData);
							break;

                        case CTranscendenceWnd::pickEnableDisableItem:
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
							pPlayer->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
							g_pTrans->DoEnableDisableItemCommand(dwData);
							break;
						}
					}

				return;
				}

			break;
			}

		case CTranscendenceWnd::gsDestroyed:
			{
			switch (chChar)
				{
				case ' ':
					g_pHI->HICommand(CONSTLIT("gameEndDestroyed"));
					break;
				}
			break;
			}

        case CTranscendenceWnd::gsDocked:
			{
			//	Handle debug console

			if (g_pTrans->m_bDebugConsole)
				{
				if (chChar >= ' ')
					{
					CString sKey = CString(&chChar, 1);
					g_pTrans->m_DebugConsole.Input(sKey);
					}
				return;
				}

			//	Ignore if this is a repeat of the dock key.

			if (bKeyRepeat && g_pTrans->m_bDockKeyDown)
				return;
			else
				g_pTrans->m_bDockKeyDown = false;

			//	Handle it.

			g_pTrans->m_CurrentDock.HandleChar(chChar);
			break;
			}
		}
    }

void CGameSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//  OnKeyDown
//
//  Key down

    {
	bool bKeyRepeat = uiIsKeyRepeat(dwKeyData);

	switch (g_pTrans->m_State)
		{
        case CTranscendenceWnd::gsInGame:
			{
			//	If no player, then nothing to do

            CPlayerShipController *pPlayer = m_Model.GetPlayer();
            if (pPlayer == NULL)
                return;

			//	Deal with console

			if (g_pTrans->m_bDebugConsole)
				{
				if (iVirtKey == VK_ESCAPE)
					{
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
					g_pTrans->m_bDebugConsole = false;
					}
				else
					g_pTrans->m_DebugConsole.OnKeyDown(iVirtKey, dwKeyData);
				}

			//	If we're paused, then check for unpause key

			else if (g_pTrans->m_bPaused)
				{
				if ((iVirtKey < 'A' || iVirtKey > 'Z') && iVirtKey != VK_SPACE && iVirtKey != VK_F9)
					{
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
					g_pTrans->m_bPaused = false;
					g_pHI->HICommand(CONSTLIT("gameUnpause"));
					g_pTrans->DisplayMessage(CONSTLIT("Game continues"));
					}

				//	We allow access to the debug console

				else
					{
					CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(iVirtKey);
					if (iCommand == CGameKeys::keyShowConsole 
							&& m_Settings.GetBoolean(CGameSettings::debugMode)
							&& !g_pUniverse->IsRegistered())
						{
						g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
						g_pTrans->m_bDebugConsole = !g_pTrans->m_bDebugConsole;
						}
					}
				}

			//	Handle menu, if it is up

			else if (g_pTrans->m_CurrentMenu != CTranscendenceWnd::menuNone)
				{
				if (iVirtKey == VK_ESCAPE)
					{
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
					g_pTrans->HideCommsTargetMenu();
					g_pTrans->m_CurrentMenu = CTranscendenceWnd::menuNone;
					}
				else
					{
					CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(iVirtKey);
					if ((iCommand == CGameKeys::keyInvokePower && g_pTrans->m_CurrentMenu == CTranscendenceWnd::menuInvoke)
							|| (iCommand == CGameKeys::keyCommunications && g_pTrans->m_CurrentMenu == CTranscendenceWnd::menuCommsTarget))
						{
						g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
						g_pTrans->HideCommsTargetMenu();
						g_pTrans->m_CurrentMenu = CTranscendenceWnd::menuNone;
						}
					}
				}

			//	Handle picker

			else if (g_pTrans->m_CurrentPicker != CTranscendenceWnd::pickNone)
				{
				if (iVirtKey == VK_RETURN)
					{
					switch (g_pTrans->m_CurrentPicker)
						{
                        case CTranscendenceWnd::pickUsableItem:
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
							g_pTrans->DoUseItemCommand(g_pTrans->m_MenuData.GetItemData(g_pTrans->m_PickerDisplay.GetSelection()));
							break;

                        case CTranscendenceWnd::pickEnableDisableItem:
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
							pPlayer->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
							g_pTrans->DoEnableDisableItemCommand(g_pTrans->m_MenuData.GetItemData(g_pTrans->m_PickerDisplay.GetSelection()));
							break;
						}
					}
				else if (iVirtKey == VK_LEFT)
					{
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
					g_pTrans->m_PickerDisplay.SelectPrev();
					}

				else if (iVirtKey == VK_RIGHT)
					{
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
					g_pTrans->m_PickerDisplay.SelectNext();
					}

				else if (iVirtKey == VK_ESCAPE)
					{
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
					g_pTrans->m_CurrentPicker = CTranscendenceWnd::pickNone;
					}

				else
					{
					CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(iVirtKey);
					if ((iCommand == CGameKeys::keyEnableDevice && g_pTrans->m_CurrentPicker == CTranscendenceWnd::pickEnableDisableItem)
							|| (iCommand == CGameKeys::keyUseItem && g_pTrans->m_CurrentPicker == CTranscendenceWnd::pickUsableItem))
						{
						g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
						g_pTrans->m_CurrentPicker = CTranscendenceWnd::pickNone;
						}
					}
				}

			//	Otherwise we're in normal game mode

			else
				{
				//	If showing the map, then we need to handle some keys

				if (m_bShowingSystemMap)
					{
                    if (m_SystemMap.HandleKeyDown(iVirtKey, dwKeyData))
                        return;
					}

				//	See if this is a command

				CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(iVirtKey);
				switch (iCommand)
					{
					case CGameKeys::keyAutopilot:
						g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
						g_pTrans->Autopilot(!g_pTrans->m_bAutopilot);
						pPlayer->SetUIMessageEnabled(uimsgAutopilotHint, false);
						g_pTrans->m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyEnableDevice:
						if (!pPlayer->DockingInProgress() 
								&& !pPlayer->GetShip()->IsParalyzed()
								&& !pPlayer->GetShip()->IsOutOfFuel()
								&& !pPlayer->GetShip()->IsTimeStopped())
							{
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
							g_pTrans->Autopilot(false);
							g_pTrans->ShowEnableDisablePicker();
							}
						g_pTrans->m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyCommunications:
						if (!pPlayer->DockingInProgress()
								&& !pPlayer->GetShip()->IsTimeStopped())
							{
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
							g_pTrans->Autopilot(false);
							pPlayer->SetUIMessageEnabled(uimsgCommsHint, false);
							g_pTrans->ShowCommsTargetMenu();
							}
						g_pTrans->m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyDock:
						if (!pPlayer->GetShip()->IsOutOfFuel()
								&& !pPlayer->GetShip()->IsParalyzed()
								&& !pPlayer->GetShip()->IsTimeStopped()
								&& !bKeyRepeat)
							{
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
							g_pTrans->Autopilot(false);
							pPlayer->Dock();
							g_pTrans->m_bDockKeyDown = true;
							}
						g_pTrans->m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyTargetNextFriendly:
						pPlayer->SelectNextFriendly(1);
						g_pTrans->m_chKeyDown = iVirtKey;
						g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
						break;

					case CGameKeys::keyTargetPrevFriendly:
						pPlayer->SelectNextFriendly(-1);
						g_pTrans->m_chKeyDown = iVirtKey;
						g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
						break;

					case CGameKeys::keyEnterGate:
						if (!pPlayer->DockingInProgress()
								&& !pPlayer->GetShip()->IsParalyzed()
								&& !pPlayer->GetShip()->IsOutOfFuel()
								&& !pPlayer->GetShip()->IsTimeStopped())
							{
							g_pTrans->Autopilot(false);
							pPlayer->Gate();
							}
						g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
						g_pTrans->m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyInvokePower:
						if (!pPlayer->DockingInProgress()
								&& !pPlayer->GetShip()->IsTimeStopped())
							{
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
							g_pTrans->Autopilot(false);
							g_pTrans->ShowInvokeMenu();
							}
						g_pTrans->m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyShowMap:
						if (m_bShowingSystemMap)
							g_pTrans->Autopilot(false);
						g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
                        ShowSystemMap(!m_bShowingSystemMap);
						pPlayer->SetUIMessageEnabled(uimsgMapHint, false);
						g_pTrans->m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyShowGalacticMap:
						g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
						g_pHI->HICommand(CONSTLIT("uiShowGalacticMap"));
						pPlayer->SetUIMessageEnabled(uimsgGalacticMapHint, false);
						g_pTrans->m_chKeyDown = iVirtKey;
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
						g_pTrans->m_chKeyDown = iVirtKey;
						break;
						}

					case CGameKeys::keySquadronCommands:
						if (!pPlayer->DockingInProgress()
								&& !pPlayer->GetShip()->IsTimeStopped())
							{
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
							g_pTrans->Autopilot(false);
							g_pTrans->ShowCommsSquadronMenu();
							}
						g_pTrans->m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyClearTarget:
						g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
						pPlayer->SetTarget(NULL);
						g_pTrans->m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyShipStatus:
						if (pPlayer->CanShowShipStatus())
							{
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
							m_Model.ShowShipScreen();
							}
						g_pTrans->m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyTargetNextEnemy:
						g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
						pPlayer->SelectNextTarget(1);
						g_pTrans->m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyTargetPrevEnemy:
						g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
						pPlayer->SelectNextTarget(-1);
						g_pTrans->m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyUseItem:
						if (!pPlayer->DockingInProgress()
								&& !pPlayer->GetShip()->IsTimeStopped())
							{
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
							g_pTrans->Autopilot(false);
							g_pTrans->ShowUsePicker();
							}
						g_pTrans->m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyNextWeapon:
						if (pPlayer && !g_pTrans->m_bNextWeaponKey)
							{
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
							g_pTrans->Autopilot(false);
							pPlayer->SetFireMain(false);
							pPlayer->ReadyNextWeapon(1);
							g_pTrans->m_chKeyDown = iVirtKey;
							g_pTrans->m_bNextWeaponKey = true;
							}
						break;

					case CGameKeys::keyPrevWeapon:
						if (pPlayer && !g_pTrans->m_bPrevWeaponKey)
							{
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
							g_pTrans->Autopilot(false);
							pPlayer->SetFireMain(false);
							pPlayer->ReadyNextWeapon(-1);
							g_pTrans->m_chKeyDown = iVirtKey;
							g_pTrans->m_bPrevWeaponKey = true;
							}
						break;

					case CGameKeys::keyThrustForward:
						if (!pPlayer->GetShip()->IsOutOfFuel()
								&& !pPlayer->GetShip()->IsTimeStopped())
							{
							g_pTrans->Autopilot(false);
							pPlayer->SetThrust(true);
                            pPlayer->SetMouseAimEnabled(false);
							}
						break;

					case CGameKeys::keyRotateLeft:
						if (!pPlayer->GetShip()->IsOutOfFuel()
								&& !pPlayer->GetShip()->IsTimeStopped())
							{
							g_pTrans->Autopilot(false);
							pPlayer->SetManeuver(RotateLeft);
                            pPlayer->SetMouseAimEnabled(false);
							}
						break;

					case CGameKeys::keyRotateRight:
						if (!pPlayer->GetShip()->IsOutOfFuel()
								&& !pPlayer->GetShip()->IsTimeStopped())
							{
							g_pTrans->Autopilot(false);
							pPlayer->SetManeuver(RotateRight);
                            pPlayer->SetMouseAimEnabled(false);
							}
						break;

					case CGameKeys::keyStop:
						if (!pPlayer->GetShip()->IsOutOfFuel()
								&& !pPlayer->GetShip()->IsTimeStopped())
							{
							g_pTrans->Autopilot(false);
							pPlayer->SetStopThrust(true);
							}
						break;

					case CGameKeys::keyFireWeapon:
						if (!pPlayer->GetShip()->IsOutOfFuel()
								&& !pPlayer->GetShip()->IsTimeStopped())
							{
							g_pTrans->Autopilot(false);
							pPlayer->SetFireMain(true);
							}
						break;

					case CGameKeys::keyFireMissile:
						if (!pPlayer->GetShip()->IsOutOfFuel()
								&& !pPlayer->GetShip()->IsTimeStopped())
							{
							g_pTrans->Autopilot(false);
							pPlayer->SetFireMissile(true);
							pPlayer->SetUIMessageEnabled(uimsgFireMissileHint, false);
							}
						break;

					case CGameKeys::keyNextMissile:
						if (!g_pTrans->m_bNextMissileKey)
							{
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
							g_pTrans->Autopilot(false);
							pPlayer->ReadyNextMissile(1);
							pPlayer->SetUIMessageEnabled(uimsgSwitchMissileHint, false);
							g_pTrans->m_bNextMissileKey = true;
							}
						break;

					case CGameKeys::keyPrevMissile:
						if (!g_pTrans->m_bPrevMissileKey)
							{
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
							g_pTrans->Autopilot(false);
							pPlayer->ReadyNextMissile(-1);
							pPlayer->SetUIMessageEnabled(uimsgSwitchMissileHint, false);
							g_pTrans->m_bPrevMissileKey = true;
							}
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
							g_pTrans->m_bDebugConsole = !g_pTrans->m_bDebugConsole;
							}
						break;
						}

					case CGameKeys::keyEnableAllDevices:
						if (!pPlayer->GetShip()->IsTimeStopped()
								&& !pPlayer->GetShip()->IsParalyzed()
								&& !pPlayer->GetShip()->IsOutOfFuel())
							{
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
							pPlayer->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
							pPlayer->EnableAllDevices(true);
							}
						break;

					case CGameKeys::keyDisableAllDevices:
						if (!pPlayer->GetShip()->IsTimeStopped()
								&& !pPlayer->GetShip()->IsParalyzed()
								&& !pPlayer->GetShip()->IsOutOfFuel())
							{
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
							pPlayer->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
							pPlayer->EnableAllDevices(false);
							}
						break;

					case CGameKeys::keyEnableAllDevicesToggle:
						if (!pPlayer->GetShip()->IsTimeStopped()
								&& !pPlayer->GetShip()->IsParalyzed()
								&& !pPlayer->GetShip()->IsOutOfFuel())
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
									&& !pPlayer->GetShip()->IsOutOfFuel())
								{
								int iDevice = (iCommand - CGameKeys::keyEnableDeviceToggle00);

								g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
								pPlayer->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
								pPlayer->ToggleEnableDevice(iDevice);
								}
							}
						else if (iVirtKey == VK_ESCAPE)
							{
							if (m_bShowingSystemMap)
								{
								g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
                                ShowSystemMap(false);
								}
							else if (g_pTrans->m_bAutopilot)
								{
								g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
								g_pTrans->Autopilot(false);
								}
							else
								{
								g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
								g_pTrans->ShowGameMenu();
								}
							}
						break;
						}
					}
				}
			break;
			}

		case CTranscendenceWnd::gsDocked:
			{
			//	Deal with console

			if (g_pTrans->m_bDebugConsole)
				{
				if (iVirtKey == VK_ESCAPE)
					g_pTrans->m_bDebugConsole = false;
				else
					g_pTrans->m_DebugConsole.OnKeyDown(iVirtKey, dwKeyData);
				}

			//	Other commands

			else
				{
				CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(iVirtKey);
				switch (iCommand)
					{
					case CGameKeys::keyShowConsole:
						{
						if (m_Settings.GetBoolean(CGameSettings::debugMode)
								&& !g_pUniverse->IsRegistered())
							g_pTrans->m_bDebugConsole = !g_pTrans->m_bDebugConsole;
						break;
						}

					default:
						switch (iVirtKey)
							{
							case VK_F1:
								g_pHI->HICommand(CONSTLIT("uiShowHelp"));
								break;

							case VK_F2:
								g_pHI->HICommand(CONSTLIT("uiShowGameStats"));
								break;

							default:
								{
								//	Let the dock screen handle it.

								g_pTrans->m_CurrentDock.HandleKeyDown(iVirtKey);
								}
							}
						break;
					}
				}

			break;
			}
		}

	return;
    }

void CGameSession::OnKeyUp (int iVirtKey, DWORD dwKeyData)

//  OnKeyUp
//
//  Key up

    {
	switch (g_pTrans->m_State)
		{
        case CTranscendenceWnd::gsInGame:
        case CTranscendenceWnd::gsDocked:
        case CTranscendenceWnd::gsEnteringStargate:
			{
			//	If no player, then nothing to do

            CPlayerShipController *pPlayer = m_Model.GetPlayer();
            if (pPlayer == NULL)
                return;

            //  Handle key

			if (g_pTrans->m_CurrentMenu != CTranscendenceWnd::menuNone
					&& iVirtKey >= 'A' && iVirtKey < 'Z')
				NULL;

			else if (g_pTrans->m_bDebugConsole)
				NULL;

			else
				{
				CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(iVirtKey);
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

					case CGameKeys::keyNextWeapon:
						g_pTrans->m_bNextWeaponKey = false;
						break;

					case CGameKeys::keyNextMissile:
						g_pTrans->m_bNextMissileKey = false;
						break;

					case CGameKeys::keyPrevWeapon:
						g_pTrans->m_bPrevWeaponKey = false;
						break;

					case CGameKeys::keyPrevMissile:
						g_pTrans->m_bPrevMissileKey = false;
						break;

					case CGameKeys::keyDock:
						g_pTrans->m_bDockKeyDown = false;
						break;
					}
				}

			break;
			}
		}
    }
