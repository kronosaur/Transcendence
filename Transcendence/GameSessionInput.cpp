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
					ExecuteCommandEnd(pPlayer, CGameKeys::keyPause);

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
							if (g_pTrans->m_CurrentMenu == CTranscendenceWnd::menuNone)
								DismissMenu();
							break;

                        case CTranscendenceWnd::menuSelfDestructConfirm:
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
							g_pTrans->DoSelfDestructConfirmCommand(dwData);
							DismissMenu();
							break;

                        case CTranscendenceWnd::menuComms:
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
							g_pTrans->DoCommsMenu(dwData);
							DismissMenu();
							m_pCurrentComms = NULL;
							break;

                        case CTranscendenceWnd::menuCommsTarget:
							{
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));

							m_pCurrentComms = (CSpaceObject *)dwData;
							HideMenu();
							if (m_pCurrentComms)
								ShowMenu(menuComms);
							else
								ShowMenu(menuCommsSquadron);
							break;
							}

                        case CTranscendenceWnd::menuCommsSquadron:
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
							g_pTrans->DoCommsSquadronMenu(g_pTrans->m_MenuData.GetItemLabel(iIndex), (MessageTypes)dwData, dwData2);
							DismissMenu();
							m_pCurrentComms = NULL;
							break;

                        case CTranscendenceWnd::menuInvoke:
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
							g_pTrans->DoInvocation((CPower *)dwData);
							DismissMenu();
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
							DismissMenu();
							break;

                        case CTranscendenceWnd::pickEnableDisableItem:
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
							pPlayer->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
							g_pTrans->DoEnableDisableItemCommand(dwData);
							DismissMenu();
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
	DWORD dwTVirtKey = CGameKeys::TranslateVirtKey(iVirtKey, dwKeyData);

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
					HideMenu();
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
					CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(dwTVirtKey);
					if (iCommand == CGameKeys::keyShowConsole)
						ExecuteCommand(pPlayer, iCommand);
					}
				}

			//	Handle menu, if it is up

			else if (g_pTrans->m_CurrentMenu != CTranscendenceWnd::menuNone)
				{
				if (iVirtKey == VK_ESCAPE)
					{
					g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
					HideMenu();
					}
				else
					{
					CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(dwTVirtKey);
					if ((iCommand == CGameKeys::keyInvokePower && g_pTrans->m_CurrentMenu == CTranscendenceWnd::menuInvoke)
							|| (iCommand == CGameKeys::keyCommunications && g_pTrans->m_CurrentMenu == CTranscendenceWnd::menuCommsTarget))
						{
						g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
						HideMenu();
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
							DismissMenu();
							break;

                        case CTranscendenceWnd::pickEnableDisableItem:
							g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_BUTTON_CLICK));
							pPlayer->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
							g_pTrans->DoEnableDisableItemCommand(g_pTrans->m_MenuData.GetItemData(g_pTrans->m_PickerDisplay.GetSelection()));
							DismissMenu();
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
					HideMenu();
					}

				else
					{
					CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(dwTVirtKey);
					if ((iCommand == CGameKeys::keyEnableDevice && g_pTrans->m_CurrentPicker == CTranscendenceWnd::pickEnableDisableItem)
							|| (iCommand == CGameKeys::keyUseItem && g_pTrans->m_CurrentPicker == CTranscendenceWnd::pickUsableItem))
						{
						g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
						HideMenu();
						}
					}
				}

			//	In normal game mode, the Esc key is hard-coded (it cannot be re-mapped).

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
					ShowMenu(menuGame);
					}
				}

			//	Some keys are handled by the map, if it is up.

			else if (m_bShowingSystemMap 
					&& m_SystemMap.HandleKeyDown(iVirtKey, dwKeyData))
				return;

			//	Otherwise we're in normal game mode

			else
				{
				//	See if this is a command

				CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(dwTVirtKey);
				if (iCommand == CGameKeys::keyNone)
					return;

				//	Remember that we processed this key so that we don't handle it again in
				//	OnChar.

				g_pTrans->m_chKeyDown = iVirtKey;

				//	Some commands do not honor repeat keys. In that case, ignore the command.

				if (bKeyRepeat && m_Settings.GetKeyMap().IsNonRepeatCommand(iCommand))
					return;

				//	Now execute the command.

				ExecuteCommand(pPlayer, iCommand);

				//	Handle some extra keyboard logic

				switch (iCommand)
					{
					//	If the player uses the keyboard to thrust, then turn off mouse
					//	move UI.

					case CGameKeys::keyThrustForward:
					case CGameKeys::keyRotateLeft:
					case CGameKeys::keyRotateRight:
		                pPlayer->SetMouseAimEnabled(false);
						break;

					//	We need to debounce the dock key so that it does not 
					//	select a dock screen action.

					case CGameKeys::keyDock:
						g_pTrans->m_bDockKeyDown = true;
						break;
					}
				}
			break;
			}

		case CTranscendenceWnd::gsDocked:
			{
            CPlayerShipController *pPlayer = m_Model.GetPlayer();
			if (pPlayer == NULL)
				return;

			//	Deal with console

			if (g_pTrans->m_bDebugConsole)
				{
				if (iVirtKey == VK_ESCAPE)
					HideMenu();
				else
					g_pTrans->m_DebugConsole.OnKeyDown(iVirtKey, dwKeyData);
				}

			//	Other commands

			else
				{
				CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(dwTVirtKey);
				switch (iCommand)
					{
					case CGameKeys::keyShowConsole:
						ExecuteCommand(pPlayer, iCommand);
						break;

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
	DWORD dwTVirtKey = CGameKeys::TranslateVirtKey(iVirtKey, dwKeyData);

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
				CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(dwTVirtKey);
				if (iCommand == CGameKeys::keyNone)
					return;

				//	If this is not a statefull command, skip it.

				if (!m_Settings.GetKeyMap().IsStatefulCommand(iCommand))
					return;

				//	Execute the command

				ExecuteCommandEnd(pPlayer, iCommand);

				//	Handle some extra keyboard logic

				switch (iCommand)
					{
					//	We need to debounce the dock key so that it does not 
					//	select a dock screen action.

					case CGameKeys::keyDock:
						g_pTrans->m_bDockKeyDown = false;
						break;
					}
				}

			break;
			}
		}
    }

void CGameSession::OnLButtonDblClick (int x, int y, DWORD dwFlags)

//	OnLButtonDblClick
//
//	Handle mouse

	{
	switch (g_pTrans->m_State)
		{
		case CTranscendenceWnd::gsInGame:
			//	Double-click counts as a single click
			OnLButtonDown(x, y, dwFlags, NULL);
			break;

		case CTranscendenceWnd::gsDocked:
			g_pTrans->m_pCurrentScreen->LButtonDown(x, y);
			break;
		}
	}

void CGameSession::OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture)

//	OnLButtonDown
//
//	Handle mouse

	{
	switch (g_pTrans->m_State)
		{
		case CTranscendenceWnd::gsInGame:
			{
            CPlayerShipController *pPlayer = m_Model.GetPlayer();
			if (pPlayer == NULL || !pPlayer->IsMouseAimEnabled())
				break;

			//	If paused, then we're done

			if (g_pTrans->m_bPaused)
				ExecuteCommandEnd(pPlayer, CGameKeys::keyPause);

			//	Execute the command

			else
				{
				CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(VK_LBUTTON);
				ExecuteCommand(pPlayer, iCommand);
				}
            break;
			}

		case CTranscendenceWnd::gsDocked:
			g_pTrans->m_pCurrentScreen->LButtonDown(x, y);
			break;
		}
	}

void CGameSession::OnLButtonUp (int x, int y, DWORD dwFlags) 

//	OnLButtonUp
//
//	Handle mouse

	{
	switch (g_pTrans->m_State)
		{
		case CTranscendenceWnd::gsInGame:
			{
            CPlayerShipController *pPlayer = m_Model.GetPlayer();
			if (pPlayer == NULL || !pPlayer->IsMouseAimEnabled())
				break;

			CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(VK_LBUTTON);
			ExecuteCommandEnd(pPlayer, iCommand);
            break;
			}

		case CTranscendenceWnd::gsDocked:
			g_pTrans->m_pCurrentScreen->LButtonUp(x, y);
			break;
		}
	}

void CGameSession::OnMButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture)

//	OnMButtonDown
//
//	Handle mouse

	{
	switch (g_pTrans->m_State)
		{
		case CTranscendenceWnd::gsInGame:
			{
            CPlayerShipController *pPlayer = m_Model.GetPlayer();
			if (pPlayer == NULL || !pPlayer->IsMouseAimEnabled())
				break;

			//	If paused, then we're done

			if (g_pTrans->m_bPaused)
				ExecuteCommandEnd(pPlayer, CGameKeys::keyPause);

			//	Execute the command

			else
				{
				CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(VK_MBUTTON);
				ExecuteCommand(pPlayer, iCommand);
				}
            break;
			}
		}
	}

void CGameSession::OnMButtonUp (int x, int y, DWORD dwFlags) 

//	OnMButtonUp
//
//	Handle mouse

	{
	switch (g_pTrans->m_State)
		{
		case CTranscendenceWnd::gsInGame:
			{
            CPlayerShipController *pPlayer = m_Model.GetPlayer();
			if (pPlayer == NULL || !pPlayer->IsMouseAimEnabled())
				break;

			CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(VK_MBUTTON);
			ExecuteCommandEnd(pPlayer, iCommand);
            break;
			}
		}
	}

void CGameSession::OnMouseMove (int x, int y, DWORD dwFlags) 
	
//	OnLButtonDblClick
//
//	Handle mouse

	{
	switch (g_pTrans->m_State)
		{
		case CTranscendenceWnd::gsInGame:
			{
            CPlayerShipController *pPlayer = m_Model.GetPlayer();
			if (pPlayer == NULL)
				break;

            if (g_pHI->HasMouseMoved(x, y))
                pPlayer->SetMouseAimEnabled(true);
            break;
			}

		case CTranscendenceWnd::gsDocked:
			g_pTrans->m_pCurrentScreen->MouseMove(x, y);
			break;
		}
	}

void CGameSession::OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags)

//  OnMouseWheel
//
//  Handle mouse wheel

    {
    switch (g_pTrans->m_State)
        {
        case CTranscendenceWnd::gsInGame:
            if (m_bShowingSystemMap)
                m_SystemMap.HandleMouseWheel(iDelta, x, y, dwFlags);
            break;

		case CTranscendenceWnd::gsDocked:
			g_pTrans->m_pCurrentScreen->MouseWheel(iDelta, x, y, dwFlags);
			break;
        }
    }

void CGameSession::OnRButtonDblClick (int x, int y, DWORD dwFlags)

//	OnRButtonDblClick
//
//	Handle mouse

	{
	switch (g_pTrans->m_State)
		{
		case CTranscendenceWnd::gsInGame:
			//	Counts as single-click

			OnRButtonDown(x, y, dwFlags);
			break;
		}
	}

void CGameSession::OnRButtonDown (int x, int y, DWORD dwFlags)

//	OnRButtonDown
//
//	Handle mouse
	
	{
	switch (g_pTrans->m_State)
		{
		case CTranscendenceWnd::gsInGame:
			{
            CPlayerShipController *pPlayer = m_Model.GetPlayer();
			if (pPlayer == NULL || !pPlayer->IsMouseAimEnabled())
				break;

			//	If paused, then we're done

			if (g_pTrans->m_bPaused)
				ExecuteCommandEnd(pPlayer, CGameKeys::keyPause);

			//	Execute the command

			else
				{
				CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(VK_RBUTTON);
				ExecuteCommand(pPlayer, iCommand);
				}
            break;
			}
		}
	}

void CGameSession::OnRButtonUp (int x, int y, DWORD dwFlags)

//	OnRButtonUp
//
//	Handle mouse

	{
	switch (g_pTrans->m_State)
		{
		case CTranscendenceWnd::gsInGame:
			{
            CPlayerShipController *pPlayer = m_Model.GetPlayer();
			if (pPlayer == NULL || !pPlayer->IsMouseAimEnabled())
				break;

			CGameKeys::Keys iCommand = m_Settings.GetKeyMap().GetGameCommand(VK_RBUTTON);
			ExecuteCommandEnd(pPlayer, iCommand);
            break;
			}
		}
	}
