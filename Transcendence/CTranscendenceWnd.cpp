//	CTranscendenceWnd.cpp
//
//	CTranscendence class

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

#define MAIN_SCREEN_WIDTH					1280
#define MAIN_SCREEN_HEIGHT					1280

#define TEXT_CRAWL_X						512
#define TEXT_CRAWL_HEIGHT					320
#define TEXT_CRAWL_WIDTH					384

#define DEBUG_CONSOLE_WIDTH					512
#define DEBUG_CONSOLE_HEIGHT				600

#define STR_G_TRANS							CONSTLIT("gTrans")
#define STR_G_PLAYER						CONSTLIT("gPlayer")
#define STR_G_PLAYER_SHIP					CONSTLIT("gPlayerShip")

int g_cxScreen = 0;
int g_cyScreen = 0;

#define STR_SMALL_TYPEFACE					CONSTLIT("Tahoma")
#define STR_MEDIUM_TYPEFACE					CONSTLIT("Lucida Sans")
#define STR_LARGE_TYPEFACE					CONSTLIT("Trebuchet MS")
#define STR_FIXED_TYPEFACE					CONSTLIT("Lucida Console")

const CItem g_DummyItem;
CItemList g_DummyItemList;
CItemListManipulator g_DummyItemListManipulator(g_DummyItemList);

CTranscendenceWnd::CTranscendenceWnd (HWND hWnd, CTranscendenceController *pTC) : m_hWnd(hWnd),
		m_pTC(pTC),
		m_State(gsNone),
		m_pLargeHUD(NULL),
		m_pCurrentScreen(NULL),
		m_bShowingMap(false),
		m_bDebugConsole(false),
		m_bAutopilot(false),
		m_iDamageFlash(0),
		m_dwIntroShipClass(0),
		m_CurrentMenu(menuNone),
		m_CurrentPicker(pickNone),
		m_pIntroSystem(NULL),
		m_pMenuObj(NULL),
		m_bRedirectDisplayMessage(false),
		m_pCrawlImage(NULL),
		m_chKeyDown('\0'),
		m_bDockKeyDown(false),
		m_bNextWeaponKey(false),
		m_bNextMissileKey(false),
		m_bPrevWeaponKey(false),
		m_bPrevMissileKey(false)

//	CTranscendence constructor

	{
	ClearDebugLines();
	}

void CTranscendenceWnd::Animate (bool bTopMost)

//	Animate
//
//	Called on each frame

	{
	bool bFailed = false;
	CG16bitImage &TheScreen = g_pHI->GetScreen();

	try
		{
		SetProgramState(psAnimating);

		//	Do the appropriate thing

		switch (m_State)
			{
			case gsIntro:
				AnimateIntro(bTopMost);
				break;

			case gsProlog:
				AnimateProlog(bTopMost);
				break;

			case gsInGame:
			case gsDestroyed:
				{
				DWORD dwStartTimer;
				if (m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
					dwStartTimer = ::GetTickCount();

				//	Figure out some stats

				bool bBlind = false;
				bool bSRSEnhanced = false;
				bool bShowMapHUD = false;
				CShip *pShip = NULL;
				if (GetPlayer())
					{
					pShip = GetPlayer()->GetShip();
					bBlind = pShip->IsBlind();
					bSRSEnhanced = pShip->IsSRSEnhanced();
					bShowMapHUD = GetPlayer()->IsMapHUDActive();
					}

				//	Update some displays

				if ((m_iTick % 7) == 0)
					{
					SetProgramState(psUpdatingReactorDisplay);
					m_ReactorDisplay.Update();
					SetProgramState(psAnimating);
					}

				//	If we're showing damage flash, fill the screen

				if (m_iDamageFlash > 0 && (m_iDamageFlash % 2) == 0)
					{
					TheScreen.Fill(0, 0, g_cxScreen, g_cyScreen, CG16bitImage::RGBValue(128,0,0));
					if (pShip && pShip->GetSystem())
						{
						if (m_bShowingMap)
							g_pUniverse->PaintObjectMap(TheScreen, m_rcMainScreen, pShip);
						else
							g_pUniverse->PaintObject(TheScreen, m_rcMainScreen, pShip);
						}
					}

				//	Otherwise, if we're in map mode, paint the map

				else if (m_bShowingMap)
					{
					SetProgramState(psPaintingMap);
					PaintMap();
					SetProgramState(psAnimating);
					}

				//	Otherwise, if we're blind, paint scramble

				else if (bBlind 
						&& (m_iTick % (20 + (((m_iTick / 100) * pShip->GetDestiny()) % 100))) > 15)
					PaintSRSSnow();

				//	Otherwise, paint the normal SRS screen

				else
					{
					SetProgramState(psPaintingSRS);
					g_pUniverse->PaintPOV(TheScreen, m_rcMainScreen, bSRSEnhanced);
					SetProgramState(psAnimating);

					PaintMainScreenBorder();
					}

				if (m_iDamageFlash > 0)
					m_iDamageFlash--;

				//	Paint various displays

				SetProgramState(psPaintingLRS);
				PaintLRS();

				if (!m_bShowingMap || bShowMapHUD)
					{
					SetProgramState(psPaintingArmorDisplay);
					m_ArmorDisplay.Paint(TheScreen);

					SetProgramState(psPaintingReactorDisplay);
					m_ReactorDisplay.Paint(TheScreen);

					SetProgramState(psPaintingTargetDisplay);
					m_TargetDisplay.Paint(TheScreen);

					SetProgramState(psPaintingDeviceDisplay);
					m_DeviceDisplay.Paint(TheScreen);
					}

				if (m_CurrentPicker == pickNone)
					{
					SetProgramState(psPaintingMessageDisplay);
					m_MessageDisplay.Paint(TheScreen);
					}

				SetProgramState(psAnimating);

				if (m_CurrentMenu != menuNone)
					m_MenuDisplay.Paint(TheScreen);
				if (m_CurrentPicker != pickNone)
					m_PickerDisplay.Paint(TheScreen);
				if (m_bDebugConsole)
					m_DebugConsole.Paint(TheScreen);

#ifdef DEBUG_LINE_OF_FIRE
				if (GetPlayer())
					{
					if (!GetPlayer()->GetShip()->IsLineOfFireClear(GetPlayer()->GetShip()->GetPos(),
							NULL,
							GetPlayer()->GetShip()->GetRotation()))
						g_pUniverse->DebugOutput("line of fire blocked");
					}
#endif
#ifdef DEBUG
				PaintDebugLines();
#endif

				//	Figure out how long it took to paint

				if (m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
					{
					DWORD dwNow = ::GetTickCount();
					m_iPaintTime[m_iFrameCount % FRAME_RATE_COUNT] = dwNow - dwStartTimer;
					dwStartTimer = dwNow;
					}

				//	Some debug information

				if (m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
					PaintFrameRate();

				//	Update the screen

				if (bTopMost)
					g_pHI->GetScreenMgr().Blt();

				//	Figure out how long it took to blt

				if (m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
					{
					DWORD dwNow = ::GetTickCount();
					m_iBltTime[m_iFrameCount % FRAME_RATE_COUNT] = dwNow - dwStartTimer;
					dwStartTimer = dwNow;
					}

				//	Update the universe

				if (!m_bPaused || m_bPausedStep)
					{
					SetProgramState(psUpdating);
					g_pUniverse->Update(g_SecondsPerUpdate);
					if (m_bAutopilot)
						{
						g_pUniverse->Update(g_SecondsPerUpdate);
						g_pUniverse->Update(g_SecondsPerUpdate);
						g_pUniverse->Update(g_SecondsPerUpdate);
						g_pUniverse->Update(g_SecondsPerUpdate);
						}
					SetProgramState(psAnimating);

					if (GetPlayer())
						GetPlayer()->Update(m_iTick);
					if (GetPlayer() && GetPlayer()->GetSelectedTarget())
						m_TargetDisplay.Invalidate();
					m_iTick++;

					m_bPausedStep = false;
					}

				m_MessageDisplay.Update();

				//	Figure out how long it took to update

				if (m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
					{
					DWORD dwNow = ::GetTickCount();
					m_iUpdateTime[m_iFrameCount % FRAME_RATE_COUNT] = dwNow - dwStartTimer;
					dwStartTimer = dwNow;
					}

				//	Destroyed?

				if (m_State == gsDestroyed)
					{
					if (!m_bPaused || m_bPausedStep)
						{
						if (--m_iCountdown == 0)
							g_pHI->HICommand(CONSTLIT("gameEndDestroyed"));
						m_bPausedStep = false;
						}
					}

				break;
				}

			case gsDocked:
				{
				//	Paint the screen

				m_pCurrentScreen->Paint(TheScreen);
				m_pCurrentScreen->Update();
				PaintMainScreenBorder();
				m_ArmorDisplay.Paint(TheScreen);
				m_TargetDisplay.Paint(TheScreen);

				//	Debug console

				if (m_bDebugConsole)
					m_DebugConsole.Paint(TheScreen);

				//	We don't paint the LRS because the player doesn't need it and
				//	because it overwrites the credits/cargo space display
				//PaintLRS();

				//	Update the screen

				if (bTopMost)
					g_pHI->GetScreenMgr().Blt();

				//	Update the universe (at 1/4 rate)

				if ((m_iTick % 4) == 0)
					g_pUniverse->Update(g_SecondsPerUpdate);
				m_MessageDisplay.Update();
				m_CurrentDock.Update(m_iTick);
				m_iTick++;

				//	Invalidate areas of the screen that are overlapped by
				//	the displays. Note that we need to convert to main screen
				//	coordinates.

				if (m_pCurrentScreen)
					{
					RECT rcRect = m_ArmorDisplay.GetRect();
					::OffsetRect(&rcRect, -m_rcMainScreen.left, -m_rcMainScreen.top);
					m_pCurrentScreen->Invalidate(rcRect);

					rcRect = m_TargetDisplay.GetRect();
					::OffsetRect(&rcRect, -m_rcMainScreen.left, -m_rcMainScreen.top);
					m_pCurrentScreen->Invalidate(rcRect);

					rcRect = m_rcLRS;
					::OffsetRect(&rcRect, -m_rcMainScreen.left, -m_rcMainScreen.top);
					m_pCurrentScreen->Invalidate(rcRect);

					if (m_bDebugConsole)
						{
						rcRect = m_DebugConsole.GetRect();
						::OffsetRect(&rcRect, -m_rcMainScreen.left, -m_rcMainScreen.top);
						m_pCurrentScreen->Invalidate(rcRect);
						}
					}

				break;
				}

			case gsEnteringStargate:
				{
				//	Update some displays

				if ((m_iTick % 10) == 0)
					m_ReactorDisplay.Update();

				//	Tell the universe to paint

				g_pUniverse->PaintPOV(TheScreen, m_rcMainScreen, false);
				PaintMainScreenBorder();
				PaintLRS();
				m_ArmorDisplay.Paint(TheScreen);
				m_MessageDisplay.Paint(TheScreen);
				m_ReactorDisplay.Paint(TheScreen);
				m_TargetDisplay.Paint(TheScreen);
				m_DeviceDisplay.Paint(TheScreen);

				//	Debug information

				if (m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
					PaintFrameRate();

#ifdef DEBUG
				PaintDebugLines();
#endif

				//	Update the screen

				if (bTopMost)
					g_pHI->GetScreenMgr().Blt();

				//	Update the universe

				g_pUniverse->Update(g_SecondsPerUpdate);
				m_MessageDisplay.Update();
				m_iTick++;

				if (--m_iCountdown == 0)
					EnterStargate();
				break;
				}

			case gsLeavingStargate:
				{
				//	Update some displays

				if ((m_iTick % 10) == 0)
					m_ReactorDisplay.Update();

				//	Tell the universe to paint

				g_pUniverse->PaintPOV(TheScreen, m_rcMainScreen, false);
				PaintMainScreenBorder();
				PaintLRS();
				m_ArmorDisplay.Paint(TheScreen);
				m_MessageDisplay.Paint(TheScreen);
				m_ReactorDisplay.Paint(TheScreen);
				m_TargetDisplay.Paint(TheScreen);
				m_DeviceDisplay.Paint(TheScreen);

				//	Debug information

				if (m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
					PaintFrameRate();

#ifdef DEBUG
				PaintDebugLines();
#endif

				//	Update the screen

				if (bTopMost)
					g_pHI->GetScreenMgr().Blt();

				//	Update the universe

				g_pUniverse->Update(g_SecondsPerUpdate);
				m_MessageDisplay.Update();
				m_iTick++;

				if (--m_iCountdown == 0)
					LeaveStargate();
				break;
				}
			}

		//	Flip

		if (bTopMost)
			g_pHI->GetScreenMgr().Flip();

		SetProgramState(psUnknown);
		}
	catch (...)
		{
		bFailed = true;
		}

	//	Deal with errors/crashes

	if (bFailed)
		{
		g_pHI->GetScreenMgr().StopDX();
		ReportCrash();
		}
	}

void CTranscendenceWnd::CleanUpPlayerShip (void)

//	CleanUpPlayerShip
//
//	Clean up player ship structures when ship is destroyed

	{
	m_ReactorDisplay.CleanUp();
	m_DeviceDisplay.CleanUp();
	m_TargetDisplay.CleanUp();
	m_MenuDisplay.CleanUp();
	m_PickerDisplay.CleanUp();

	m_bDebugConsole = false;
	}

void CTranscendenceWnd::ClearDebugLines (void)

//	ClearDebugLines
//
//	Clear debug lines

	{
#ifdef DEBUG
	m_iDebugLinesStart = 0;
	m_iDebugLinesEnd = 0;
#endif
	}

void CTranscendenceWnd::ComputeScreenSize (void)

//	ComputeScreenSize
//
//	Computes g_cxScreen and g_cyScreen

	{
	//	By default we use the current resolution (unless in windowed mode)

	if (m_pTC->GetOptionBoolean(CGameSettings::windowedMode))
		{
		RECT rcClient;
		::GetClientRect(m_hWnd, &rcClient);

		g_cxScreen = RectWidth(rcClient);
		g_cyScreen = RectHeight(rcClient);
		}
	else
		{
		g_cxScreen = ::GetSystemMetrics(SM_CXSCREEN);
		g_cyScreen = ::GetSystemMetrics(SM_CYSCREEN);
		}

	//	Switch to 1024 resolution, if requested

	if (m_pTC->GetOptionBoolean(CGameSettings::force1024Res))
		{
		g_cxScreen = 1024;
		g_cyScreen = 768;
		}
	}

void CTranscendenceWnd::DebugConsoleOutput (const CString &sOutput)

//	DebugConsoleOutput
//
//	Output to debug console

	{
	m_DebugConsole.Output(sOutput);
	}

void CTranscendenceWnd::DoCommand (DWORD dwCmd)

//	DoCommand
//
//	Do command

	{
	switch (dwCmd)
		{
		case CMD_CONTINUE_OLD_GAME:
			g_pHI->HICommand(CONSTLIT("gameSelectSaveFile"));
			break;

		case CMD_START_NEW_GAME:
			g_pHI->HICommand(CONSTLIT("gameSelectAdventure"));
			break;

		case CMD_QUIT_GAME:
			::SendMessage(m_hWnd, WM_CLOSE, 0, 0);
			break;
		}
	}

void CTranscendenceWnd::EnterStargate (void)

//	EnterStargate
//
//	Place the player in the new system

	{
	//	State

	m_State = gsLeavingStargate;
	m_iCountdown = TICKS_AFTER_GATE;

	//	Let the model handle everything

	m_pTC->GetModel().OnPlayerTraveledThroughGate();
	}

void CTranscendenceWnd::GetMousePos (POINT *retpt)

//	GetMousePos
//
//	Returns the mouse position relative to the Transcendence window

	{
	::GetCursorPos(retpt);
	::ScreenToClient(m_hWnd, retpt);
	}

void CTranscendenceWnd::LeaveStargate (void)

//	LeaveStargate
//
//	Place the player in the new system

	{
	//	State

	m_State = gsInGame;

	//	Let the model handle everything

	m_pTC->GetModel().OnPlayerExitedGate();
	}

void CTranscendenceWnd::OnAniCommand (const CString &sID, const CString &sEvent, const CString &sCmd, DWORD dwData)

//	OnAniCommand
//
//	Handle a Reanimator command

	{
	switch (m_State)
		{
		case gsIntro:
			OnCommandIntro(sCmd, NULL);
			break;
		}
	}

void CTranscendenceWnd::PaintDebugLines (void)

//	PaintDebugLines
//
//	Paint debug output

	{
	CG16bitImage &TheScreen = g_pHI->GetScreen();

#ifdef DEBUG
	int iLine = m_iDebugLinesStart;
	int iPos = 0;
	while (iLine != m_iDebugLinesEnd)
		{
		TheScreen.DrawText(0,
				iPos++ * 12,
				m_Fonts.Medium,
				CG16bitImage::RGBValue(128,0,0),
				m_DebugLines[iLine]);

		iLine = (iLine + 1) % DEBUG_LINES_COUNT;
		}
#endif
	}

void CTranscendenceWnd::PaintFrameRate (void)

//	PaintFrameRate
//
//	Paints the frame rate

	{
	int i;
	CG16bitImage &TheScreen = g_pHI->GetScreen();

	if (m_iStartAnimation == 0)
		m_iStartAnimation = GetTickCount();
	else
		{
		//	Figure out how much time has elapsed since the last frame

		int iNow = GetTickCount();
		m_iFrameTime[m_iFrameCount % FRAME_RATE_COUNT] = iNow - m_iStartAnimation;

		//	Add up all the times

		int iTotalFrameTime = 0;
		int iTotalPaintTime = 0;
		int iTotalBltTime = 0;
		int iTotalUpdateTime = 0;
		for (i = 0; i < FRAME_RATE_COUNT; i++)
			{
			iTotalFrameTime += m_iFrameTime[i];
			iTotalPaintTime += m_iPaintTime[i];
			iTotalBltTime += m_iBltTime[i];
			iTotalUpdateTime += m_iUpdateTime[i];
			}

		//	Compute the frame rate (in 10ths of frames)

		int rFrameRate = 0;
		if (iTotalFrameTime > 0)
			rFrameRate = 10000 * FRAME_RATE_COUNT / iTotalFrameTime;

		char szBuffer[256];
		int iLen = wsprintf(szBuffer, "Frames: %d   Paint: %d  Blt: %d  Update: %d",
				rFrameRate / 10,
				iTotalPaintTime / FRAME_RATE_COUNT,
				iTotalBltTime / FRAME_RATE_COUNT,
				iTotalUpdateTime / FRAME_RATE_COUNT);

		TheScreen.DrawText(300, 0, m_Fonts.Header, CG16bitImage::RGBValue(80,80,80), CString(szBuffer, iLen));

		//	Every once in a while, output to log file

		if (m_iFrameCount > FRAME_RATE_COUNT
				&& ((m_iFrameCount % 300) == 0))
			kernelDebugLogMessage(szBuffer);

		//	Next

		m_iFrameCount++;
		m_iStartAnimation = iNow;
		}
	}

void CTranscendenceWnd::PlayerDestroyed (const CString &sText, bool bResurrectionPending)

//	PlayerDestroyed
//
//	This method gets called when the player is destroyed

	{
	//	Clean up

	m_CurrentPicker = pickNone;
	m_CurrentMenu = menuNone;
	m_bAutopilot = false;
	m_bShowingMap = false;
	if (m_State == gsDocked)
		GetPlayer()->Undock();

	//	Update display

	CString sMsg = sText;
	if (strEquals(strWord(sMsg, 0), CONSTLIT("was")))
		sMsg = strSubString(sMsg, 4, -1);
	sMsg.Capitalize(CString::capFirstLetter);
	DisplayMessage(sMsg);
	UpdateArmorDisplay();

	//	If we are insured, then set our state so that we come back to life
	if (bResurrectionPending)
		{
		//	Prepare resurrect

		m_State = gsDestroyed;
		m_iCountdown = TICKS_AFTER_DESTROYED;
		}

	//	Otherwise, proceed with destruction

	else
		{
		//	Done with ship screens

		CleanUpPlayerShip();

		//	Player destroyed

		m_State = gsDestroyed;
		m_iCountdown = TICKS_AFTER_DESTROYED;
		}
	}

void CTranscendenceWnd::PlayerEnteredGate (CSystem *pSystem, 
										   CTopologyNode *pDestNode,
										   const CString &sDestEntryPoint)

//	PlayerEnteredGate
//
//	This method gets called when the player enters a stargate

	{
	//	Remember the destination gate and set our state

	m_iCountdown = TICKS_BEFORE_GATE;
	m_State = gsEnteringStargate;
	m_bShowingMap = false;
	m_bAutopilot = false;
	}

void CTranscendenceWnd::RedirectDisplayMessage (bool bRedirect)

//	RedirectDisplayMessage
//
//	Start/stop displaying messages

	{
	if (bRedirect)
		{
		m_bRedirectDisplayMessage = true;
		m_sRedirectMessage = NULL_STR;
		}
	else
		{
		m_bRedirectDisplayMessage = false;
		}
	}

void CTranscendenceWnd::ReportCrash (void)

//	ReportCrash
//
//	Report a crash

	{
#ifdef DEBUG_PROGRAMSTATE

	m_sCrashInfo = NULL_STR;

	//	Program state

	switch (g_iProgramState)
		{
		case psAnimating:
			m_sCrashInfo.Append(CONSTLIT("program state: miscellaneous animation\r\n"));
			break;

		case psPaintingSRS:
			m_sCrashInfo.Append(CONSTLIT("program state: painting SRS\r\n"));
			ReportCrashObj(&m_sCrashInfo);
			break;

		case psUpdating:
			m_sCrashInfo.Append(CONSTLIT("program state: updating universe\r\n"));
			break;

		case psUpdatingEvents:
			m_sCrashInfo.Append(CONSTLIT("program state: updating events\r\n"));
			ReportCrashEvent(&m_sCrashInfo);
			break;

		case psUpdatingBehavior:
			m_sCrashInfo.Append(CONSTLIT("program state: updating object behavior\r\n"));
			ReportCrashObj(&m_sCrashInfo);
			break;

		case psUpdatingObj:
			m_sCrashInfo.Append(CONSTLIT("program state: updating object\r\n"));
			ReportCrashObj(&m_sCrashInfo);
			break;

		case psUpdatingHitTest:
			m_sCrashInfo.Append(CONSTLIT("program state: updating object hit test\r\n"));
			ReportCrashObj(&m_sCrashInfo);
			break;

		case psUpdatingMove:
			m_sCrashInfo.Append(CONSTLIT("program state: updating object motion\r\n"));
			ReportCrashObj(&m_sCrashInfo);
			break;

		case psUpdatingEncounters:
			m_sCrashInfo.Append(CONSTLIT("program state: updating random encounters\r\n"));
			break;

		case psDestroyNotify:
			m_sCrashInfo.Append(CONSTLIT("program state: handling OnObjDestroy\r\n"));
			ReportCrashObj(&m_sCrashInfo);
			break;

		case psUpdatingReactorDisplay:
			m_sCrashInfo.Append(CONSTLIT("program state: updating reactor display\r\n"));
			break;

		case psPaintingMap:
			m_sCrashInfo.Append(CONSTLIT("program state: painting map\r\n"));
			break;

		case psPaintingLRS:
			m_sCrashInfo.Append(CONSTLIT("program state: painting LRS\r\n"));
			break;

		case psPaintingArmorDisplay:
			m_sCrashInfo.Append(CONSTLIT("program state: painting armor display\r\n"));
			break;

		case psPaintingMessageDisplay:
			m_sCrashInfo.Append(CONSTLIT("program state: painting message display\r\n"));
			break;

		case psPaintingReactorDisplay:
			m_sCrashInfo.Append(CONSTLIT("program state: painting reactor display\r\n"));
			break;

		case psPaintingTargetDisplay:
			m_sCrashInfo.Append(CONSTLIT("program state: painting target display\r\n"));
			ReportCrashObj(&m_sCrashInfo, (GetPlayer() ? GetPlayer()->GetShip() : NULL));
			break;

		case psPaintingDeviceDisplay:
			m_sCrashInfo.Append(CONSTLIT("program state: painting device display\r\n"));
			break;

		case psStargateEnter:
			m_sCrashInfo.Append(CONSTLIT("program state: entering stargate\r\n"));
			break;

		case psStargateEndGame:
			m_sCrashInfo.Append(CONSTLIT("program state: entering end-game stargate\r\n"));
			break;

		case psStargateLoadingSystem:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate, loading destination system\r\n"));
			break;

		case psStargateCreatingSystem:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate, creating destination system\r\n"));
			break;

		case psStargateUpdateExtended:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate, updating extended\r\n"));
			break;

		case psStargateTransferringGateFollowers:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate, transferring gate followers\r\n"));
			break;

		case psStargateSavingSystem:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate, saving current system\r\n"));
			break;

		case psStargateFlushingSystem:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate, flushing current system\r\n"));
			break;

		case psStargateGarbageCollecting:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate, garbage-collecting images\r\n"));
			break;

		case psStargateLoadingBitmaps:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate, loading new images\r\n"));
			break;

		case psLoadingImages:
			m_sCrashInfo.Append(CONSTLIT("program state: in LoadImages\r\n"));
			ReportCrashObj(&m_sCrashInfo);
			break;

		case psStargateEnterDone:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate done\r\n"));
			break;

		case psOnObjDestroyed:
			m_sCrashInfo.Append(CONSTLIT("program state: in OnObjDestroyed\r\n"));
			ReportCrashObj(&m_sCrashInfo);
			break;

		case psCustomError:
			if (g_sProgramError)
				m_sCrashInfo.Append(*g_sProgramError);
			m_sCrashInfo.Append(CONSTLIT("\r\n"));
			break;

		default:
			m_sCrashInfo.Append(CONSTLIT("program state: unknown\r\n"));
		}

	//	Game state

	switch (m_State)
		{
		case gsInGame:
			m_sCrashInfo.Append(CONSTLIT("game state: in game\r\n"));
			break;

		case gsDocked:
			m_sCrashInfo.Append(CONSTLIT("game state: docked\r\n"));
			break;

		case gsDestroyed:
			m_sCrashInfo.Append(CONSTLIT("game state: destroyed\r\n"));
			break;

		case gsEnteringStargate:
			m_sCrashInfo.Append(CONSTLIT("game state: entering stargate\r\n"));
			break;

		case gsLeavingStargate:
			m_sCrashInfo.Append(CONSTLIT("game state: leaving stargate\r\n"));
			break;

		case gsIntro:
			m_sCrashInfo.Append(CONSTLIT("game state: intro\r\n"));
			break;

		case gsProlog:
			m_sCrashInfo.Append(CONSTLIT("game state: prolog\r\n"));
			break;

		default:
			m_sCrashInfo.Append(CONSTLIT("game state: unknown\r\n"));
		}
#endif

	throw CException(ERR_FAIL);
	}

void CTranscendenceWnd::ReportCrashEvent (CString *retsMessage)

//	ReportCrashEvent
//
//	Include information about the given event

	{
	try
		{
		retsMessage->Append(g_pProgramEvent->DebugCrashInfo());
		}
	catch (...)
		{
		retsMessage->Append(strPatternSubst(CONSTLIT("error obtaining crash info from event: %x\r\n"), g_pProgramEvent));
		}
	}

void CTranscendenceWnd::ShowErrorMessage (const CString &sError)

//	ShowErrorMessage
//
//	Shows an error message box

	{
	::kernelDebugLogMessage(sError);

	::MessageBox(m_hWnd, 
			sError.GetASCIIZPointer(), 
			"Transcendence", 
			MB_OK | MB_ICONSTOP);
	}

ALERROR CTranscendenceWnd::StartGame (bool bNewGame)

//	StartGame
//
//	Start the game (either a new game or a previously saved game)

	{
	ALERROR error;
	int i;

	//	Initialize sub-displays

	if (error = InitDisplays())
		return error;

	//	Welcome

	m_MessageDisplay.ClearAll();
	const CString &sWelcome = g_pUniverse->GetCurrentAdventureDesc()->GetWelcomeMessage();
	if (!sWelcome.IsBlank())
		m_MessageDisplay.DisplayMessage(sWelcome, m_Fonts.wTitleColor);

	//	Set map state

	m_bShowingMap = false;
	m_iMapScale = 1;
	m_iMapZoomEffect = 0;
	for (i = 0; i < MAP_SCALE_COUNT; i++)
		if (i == 0)
			m_rMapScale[i] = g_AU / 800.0;
		else
			m_rMapScale[i] = m_rMapScale[i - 1] * 2.0;

	//	Set the state appropriately

	m_State = gsInGame;
	m_bPaused = false;
	m_bPausedStep = false;
	m_bDebugConsole = false;
	m_bAutopilot = false;
	m_CurrentPicker = pickNone;
	m_CurrentMenu = menuNone;
	m_iTick = 0;

	//	Start the game. This ends up calling CTranscendenceModel::StartGame

	if (bNewGame)
		g_pHI->HICommand(CONSTLIT("gameStartNew"));
	else
		g_pHI->HICommand(CONSTLIT("gameStartExisting"));

	return NOERROR;
	}

LONG CTranscendenceWnd::WMChar (char chChar, DWORD dwKeyData)

//	WMChar
//
//	Handle WM_CHAR

	{
	bool bKeyRepeat = uiIsKeyRepeat(dwKeyData);

	//	If we already processed the keydown, then skip it

	if (m_chKeyDown 
			&& ((chChar == m_chKeyDown) || (chChar == (m_chKeyDown + ('a' - 'A')))))
		{
		m_chKeyDown = '\0';
		return 0;
		}
	else
		m_chKeyDown = '\0';

	//	Handle key

	switch (m_State)
		{
		case gsInGame:
			{
			//	Handle debug console

			if (m_bDebugConsole)
				{
				if (chChar >= ' ')
					{
					CString sKey = CString(&chChar, 1);
					m_DebugConsole.Input(sKey);
					}
				return 0;
				}

			//	If we're paused, then check for unpause key

			if (m_bPaused)
				{
				if (chChar == ' ')
					m_bPausedStep = true;
				else
					{
					m_bPaused = false;
					DisplayMessage(CONSTLIT("Game continues"));
					}
				return 0;
				}

			//	If we're in a menu, handle it

			if (m_CurrentMenu != menuNone)
				{
				//	Ignore repeat keys (because otherwise we might accidentally
				//	select a menu item from keeping a key pressed too long).

				if (bKeyRepeat)
					return 0;

				//	Find the menu item and invoke

				CString sKey = CString(&chChar, 1);
				int iIndex;
				if ((iIndex = m_MenuData.FindItemByKey(sKey)) != -1)
					{
					DWORD dwData = m_MenuData.GetItemData(iIndex);
					DWORD dwData2 = m_MenuData.GetItemData2(iIndex);

					switch (m_CurrentMenu)
						{
						case menuGame:
							DoGameMenuCommand(dwData);
							break;

						case menuSelfDestructConfirm:
							DoSelfDestructConfirmCommand(dwData);
							break;

						case menuComms:
							DoCommsMenu(dwData);
							break;

						case menuCommsTarget:
							if (dwData)
								ShowCommsMenu((CSpaceObject *)dwData);
							else
								ShowCommsSquadronMenu();
							break;

						case menuCommsSquadron:
							DoCommsSquadronMenu(m_MenuData.GetItemLabel(iIndex), (MessageTypes)dwData, dwData2);
							break;

						case menuInvoke:
							DoInvocation((CPower *)dwData);
							break;
						}
					}
				return 0;
				}

			//	If we're in a picker, handle it

			if (m_CurrentPicker != pickNone)
				{
				CString sKey = CString(&chChar, 1);
				DWORD dwData;
				bool bHotKey = m_MenuData.FindItemData(sKey, &dwData);

				if (chChar == ']')
					m_PickerDisplay.SelectNext();
				else if (chChar == '[')
					m_PickerDisplay.SelectPrev();
				else if (bHotKey)
					{
					switch (m_CurrentPicker)
						{
						case pickUsableItem:
							DoUseItemCommand(dwData);
							break;

						case pickEnableDisableItem:
							GetPlayer()->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
							DoEnableDisableItemCommand(dwData);
							break;
						}
					}

				return 0;
				}

			break;
			}

		case gsIntro:
			OnCharIntro(chChar, dwKeyData);
			break;

		case gsDestroyed:
			{
			switch (chChar)
				{
				case ' ':
					g_pHI->HICommand(CONSTLIT("gameEndDestroyed"));
					break;
				}
			break;
			}

		case gsDocked:
			{
			//	Handle debug console

			if (m_bDebugConsole)
				{
				if (chChar >= ' ')
					{
					CString sKey = CString(&chChar, 1);
					m_DebugConsole.Input(sKey);
					}
				return 0;
				}

			//	Ignore if this is a repeat of the dock key.

			if (bKeyRepeat && m_bDockKeyDown)
				return 0;
			else
				m_bDockKeyDown = false;

			//	Handle it.

			m_CurrentDock.HandleChar(chChar);
			break;
			}
		}

	return 0;
	}

LONG CTranscendenceWnd::WMClose (void)

//	WMClose
//
//	Handle WM_CLOSE

	{
	//	Do not allow the user to close if we're entering or leaving
	//	a stargate (because we are too lazy to handle saving a game in the
	//	middle of a gate).

	if (m_State == gsEnteringStargate || m_State == gsLeavingStargate)
		return 0;

	//	If we have a loading error, display it now

	if (!m_sBackgroundError.IsBlank())
		{
		g_pHI->GetScreenMgr().StopDX();

		::MessageBox(m_hWnd, 
				m_sBackgroundError.GetASCIIZPointer(), 
				"Transcendence", 
				MB_OK | MB_ICONSTOP);
		}

	//	Destroy the window

	g_pHI->Shutdown(HIShutdownByUser);
	return 0;
	}

LONG CTranscendenceWnd::WMCreate (CString *retsError)

//	WMCreate
//
//	Handle WM_CREATE

	{
	ALERROR error;
	int i;

	//	Get the version information

	{
	SFileVersionInfo VerInfo;
	fileGetVersionInfo(NULL_STR, &VerInfo);
	m_sVersion = strPatternSubst(CONSTLIT("%s %s"), VerInfo.sProductName, VerInfo.sProductVersion);
	m_sCopyright = VerInfo.sCopyright;
	kernelDebugLogMessage(m_sVersion);
	}

	//	Load preferences

	//LoadPreferences();
	//g_pHI->GetSoundMgr().SetWaveVolume(m_Prefs.iSoundVolume);

	//	Compute screen size

	ComputeScreenSize();

	//	Initialize frame rate information

	m_iStartAnimation = 0;
	m_iFrameCount = 0;
	for (i = 0; i < FRAME_RATE_COUNT; i++)
		{
		m_iFrameTime[i] = 0;
		m_iPaintTime[i] = 0;
		m_iUpdateTime[i] = 0;
		m_iBltTime[i] = 0;
		}

	//	Initialize

	m_rcScreen.left = 0;
	m_rcScreen.top = 0;
	m_rcScreen.right = g_cxScreen;
	m_rcScreen.bottom = g_cyScreen;

	int cxMainScreen = Min(MAIN_SCREEN_WIDTH, g_cxScreen);
	int cyMainScreen = Min(MAIN_SCREEN_HEIGHT, g_cyScreen);

	m_rcMainScreen.left = (g_cxScreen - cxMainScreen) / 2;
	m_rcMainScreen.top = (g_cyScreen - cyMainScreen) / 2;
	m_rcMainScreen.right = m_rcMainScreen.left + cxMainScreen;
	m_rcMainScreen.bottom = m_rcMainScreen.top + cyMainScreen;

	//	Initialize the font table

	m_Fonts.Small.Create(STR_SMALL_TYPEFACE, -10);
	m_Fonts.Medium.Create(STR_SMALL_TYPEFACE, -13);
	m_Fonts.MediumBold.Create(STR_SMALL_TYPEFACE, -13, true);
	m_Fonts.MediumHeavyBold.Create(STR_MEDIUM_TYPEFACE, -14, true);
	m_Fonts.Large.Create(STR_MEDIUM_TYPEFACE, -16);
	m_Fonts.LargeBold.Create(STR_MEDIUM_TYPEFACE, -16, true);

	m_Fonts.Header.CreateFromResource(g_hInst, "DXFN_HEADER");
	m_Fonts.HeaderBold.CreateFromResource(g_hInst, "DXFN_HEADER_BOLD");
	m_Fonts.SubTitle.CreateFromResource(g_hInst, "DXFN_SUBTITLE");
	m_Fonts.SubTitleBold.CreateFromResource(g_hInst, "DXFN_SUBTITLE_BOLD");
	m_Fonts.SubTitleHeavyBold.CreateFromResource(g_hInst, "DXFN_SUBTITLE_HEAVY_BOLD");
	m_Fonts.Title.CreateFromResource(g_hInst, "DXFN_TITLE");
	m_Fonts.LogoTitle.CreateFromResource(g_hInst, "DXFN_LOGO_TITLE");
	m_Fonts.Console.Create(STR_FIXED_TYPEFACE, -14);

	//	Output the typeface that we got

	if (m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
		{
		kernelDebugLogMessage("Small typeface: %s", m_Fonts.Small.GetTypeface());
		kernelDebugLogMessage("Medium typeface: %s", m_Fonts.Large.GetTypeface());
		kernelDebugLogMessage("Large typeface: %s", m_Fonts.Header.GetTypeface());
		kernelDebugLogMessage("Console typeface: %s", m_Fonts.Console.GetTypeface());
		}

	//	Set colors

	m_Fonts.wTextColor = CG16bitImage::RGBValue(191,196,201);
	m_Fonts.wTitleColor = CG16bitImage::RGBValue(218,235,255);
	m_Fonts.wLightTitleColor = CG16bitImage::RGBValue(120,129,140);
	m_Fonts.wHelpColor = CG16bitImage::RGBValue(103,114,128);
	m_Fonts.wBackground = CG16bitImage::RGBValue(15,17,18);
	m_Fonts.wSectionBackground = CG16bitImage::RGBValue(86,82,73);
	m_Fonts.wSelectBackground = CG16bitImage::RGBValue(115,230,115);
	//m_Fonts.wSelectBackground = CG16bitImage::RGBValue(255,225,103);

	m_Fonts.wAltGreenColor = CG16bitImage::RGBValue(5,211,5);
	m_Fonts.wAltGreenBackground = CG16bitImage::RGBValue(23,77,23);
	m_Fonts.wAltYellowColor = CG16bitImage::RGBValue(255,225,103);
	m_Fonts.wAltYellowBackground = CG16bitImage::RGBValue(65,57,24);
	m_Fonts.wAltRedColor = CG16bitImage::RGBValue(4,179,4);
	m_Fonts.wAltRedBackground = CG16bitImage::RGBValue(76,0,0);
	m_Fonts.wAltBlueColor = CG16bitImage::RGBValue(87,111,205);
	m_Fonts.wAltBlueBackground = CG16bitImage::RGBValue(52,57,64);

	m_Fonts.wItemTitle = CG16bitImage::RGBValue(255,255,255);
	m_Fonts.wItemRef = CG16bitImage::RGBValue(255,255,255);
	m_Fonts.wItemDesc = CG16bitImage::RGBValue(128,128,128);
	m_Fonts.wItemDescSelected = CG16bitImage::RGBValue(200,200,200);

	//	Initialize UI resources

	if (error = m_UIRes.Init(&m_Fonts))
		{
		*retsError = CONSTLIT("Unable to initialize UI resources.");
		goto Fail;
		}

	//	Play Intro Music

	if (!m_pTC->GetOptionBoolean(CGameSettings::noMusic))
		g_pHI->GetSoundMgr().PlayMusic(CONSTLIT("TranscendenceMarch.mp3"));

	//	Initialize debug console

	RECT rcRect;
	rcRect.left = m_rcScreen.right - (DEBUG_CONSOLE_WIDTH + 4);
	rcRect.top = (RectHeight(m_rcScreen) - DEBUG_CONSOLE_HEIGHT) / 2;
	rcRect.right = rcRect.left + DEBUG_CONSOLE_WIDTH;
	rcRect.bottom = rcRect.top + DEBUG_CONSOLE_HEIGHT;
	m_DebugConsole.SetFontTable(&m_Fonts);
	m_DebugConsole.Init(this, rcRect);

	m_DebugConsole.Output(CONSTLIT("Transcendence Debug Console"));
	m_DebugConsole.Output(m_sVersion);
	m_DebugConsole.Output(m_sCopyright);
	m_DebugConsole.Output(NULL_STR);

	return 0;

Fail:

	return -1;
	}

LONG CTranscendenceWnd::WMDestroy (void)

//	WMDestroy
//
//	Handle WM_DESTROY

	{
	//	If we're docked, undock first

	if (m_State == gsDocked)
		GetPlayer()->Undock();

	//	Clean up debug console

	m_DebugConsole.CleanUp();

	return 0;
	}

LONG CTranscendenceWnd::WMKeyDown (int iVirtKey, DWORD dwKeyData)

//	WMKeyDown
//
//	Handle WM_KEYDOWN

	{
	bool bKeyRepeat = uiIsKeyRepeat(dwKeyData);

	switch (m_State)
		{
		case gsInGame:
			{
			//	If no player, then nothing to do

			if (GetPlayer() == NULL)
				NULL;

			//	Deal with console

			else if (m_bDebugConsole)
				{
				if (iVirtKey == VK_ESCAPE)
					m_bDebugConsole = false;
				else
					m_DebugConsole.OnKeyDown(iVirtKey, dwKeyData);
				}

			//	If we're paused, then check for unpause key

			else if (m_bPaused)
				{
				if ((iVirtKey < 'A' || iVirtKey > 'Z') && iVirtKey != VK_SPACE && iVirtKey != VK_F9)
					{
					m_bPaused = false;
					DisplayMessage(CONSTLIT("Game continues"));
					}

				//	We allow access to the debug console

				else
					{
					CGameKeys::Keys iCommand = m_pTC->GetKeyMap().GetGameCommand(iVirtKey);
					if (iCommand == CGameKeys::keyShowConsole 
							&& m_pTC->GetOptionBoolean(CGameSettings::debugMode)
							&& !g_pUniverse->IsRegistered())
						m_bDebugConsole = !m_bDebugConsole;
					}
				}

			//	Handle menu, if it is up

			else if (m_CurrentMenu != menuNone)
				{
				if (iVirtKey == VK_ESCAPE)
					m_CurrentMenu = menuNone;
				else
					{
					CGameKeys::Keys iCommand = m_pTC->GetKeyMap().GetGameCommand(iVirtKey);
					if ((iCommand == CGameKeys::keyInvokePower && m_CurrentMenu == menuInvoke)
							|| (iCommand == CGameKeys::keyCommunications && m_CurrentMenu == menuCommsTarget))
						m_CurrentMenu = menuNone;
					}
				}

			//	Handle picker

			else if (m_CurrentPicker != pickNone)
				{
				if (iVirtKey == VK_RETURN)
					{
					switch (m_CurrentPicker)
						{
						case pickUsableItem:
							DoUseItemCommand(m_MenuData.GetItemData(m_PickerDisplay.GetSelection()));
							break;

						case pickEnableDisableItem:
							GetPlayer()->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
							DoEnableDisableItemCommand(m_MenuData.GetItemData(m_PickerDisplay.GetSelection()));
							break;
						}
					}
				else if (iVirtKey == VK_LEFT)
					m_PickerDisplay.SelectPrev();

				else if (iVirtKey == VK_RIGHT)
					m_PickerDisplay.SelectNext();

				else if (iVirtKey == VK_ESCAPE)
					m_CurrentPicker = pickNone;

				else
					{
					CGameKeys::Keys iCommand = m_pTC->GetKeyMap().GetGameCommand(iVirtKey);
					if ((iCommand == CGameKeys::keyEnableDevice && m_CurrentPicker == pickEnableDisableItem)
							|| (iCommand == CGameKeys::keyUseItem && m_CurrentPicker == pickUsableItem))
						m_CurrentPicker = pickNone;
					}
				}

			//	Otherwise we're in normal game mode

			else
				{
				//	If showing the map, then we need to handle some keys

				if (m_bShowingMap)
					{
					switch (iVirtKey)
						{
						case 'H':
							GetPlayer()->SetMapHUD(!GetPlayer()->IsMapHUDActive());
							break;

						case VK_SUBTRACT:
						case VK_OEM_MINUS:
							if (m_iMapScale < (MAP_SCALE_COUNT - 1))
								{
								m_iMapScale++;
								m_iMapZoomEffect = 100;
								}
							break;

						case VK_ADD:
						case VK_OEM_PLUS:
							if (m_iMapScale > 0)
								{
								m_iMapScale--;
								m_iMapZoomEffect = -100;
								}
							break;
						}

					//	Fall through because normal commands are available with the map
					}

				//	See if this is a command

				CGameKeys::Keys iCommand = m_pTC->GetKeyMap().GetGameCommand(iVirtKey);
				switch (iCommand)
					{
					case CGameKeys::keyAutopilot:
						Autopilot(!m_bAutopilot);
						GetPlayer()->SetUIMessageEnabled(uimsgAutopilotHint, false);
						m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyEnableDevice:
						if (!GetPlayer()->DockingInProgress() 
								&& !GetPlayer()->GetShip()->IsOutOfFuel()
								&& !GetPlayer()->GetShip()->IsTimeStopped())
							{
							Autopilot(false);
							ShowEnableDisablePicker();
							}
						m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyCommunications:
						if (!GetPlayer()->DockingInProgress()
								&& !GetPlayer()->GetShip()->IsTimeStopped())
							{
							Autopilot(false);
							GetPlayer()->SetUIMessageEnabled(uimsgCommsHint, false);
							ShowCommsTargetMenu();
							}
						m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyDock:
						if (!GetPlayer()->GetShip()->IsOutOfFuel()
								&& !GetPlayer()->GetShip()->IsTimeStopped()
								&& !bKeyRepeat)
							{
							Autopilot(false);
							GetPlayer()->Dock();
							m_bDockKeyDown = true;
							}
						m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyTargetNextFriendly:
						GetPlayer()->SelectNextFriendly(1);
						m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyTargetPrevFriendly:
						GetPlayer()->SelectNextFriendly(-1);
						m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyEnterGate:
						if (!GetPlayer()->DockingInProgress()
								&& !GetPlayer()->GetShip()->IsOutOfFuel()
								&& !GetPlayer()->GetShip()->IsTimeStopped())
							{
							Autopilot(false);
							GetPlayer()->Gate();
							}
						m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyInvokePower:
						if (!GetPlayer()->DockingInProgress()
								&& !GetPlayer()->GetShip()->IsTimeStopped())
							{
							Autopilot(false);
							ShowInvokeMenu();
							}
						m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyShowMap:
						if (m_bShowingMap)
							Autopilot(false);
						m_bShowingMap = !m_bShowingMap;
						GetPlayer()->SetUIMessageEnabled(uimsgMapHint, false);
						m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyShowGalacticMap:
						g_pHI->HICommand(CONSTLIT("uiShowGalacticMap"));
						GetPlayer()->SetUIMessageEnabled(uimsgGalacticMapHint, false);
						m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyPause:
						{
						m_bPaused = true;
						if (GetPlayer())
							{
							GetPlayer()->SetThrust(false);
							GetPlayer()->SetManeuver(IShipController::NoRotation);
							GetPlayer()->SetFireMain(false);
							GetPlayer()->SetFireMissile(false);
							}
						DisplayMessage(CONSTLIT("Game paused"));
						m_chKeyDown = iVirtKey;
						break;
						}

					case CGameKeys::keySquadronCommands:
						if (!GetPlayer()->DockingInProgress()
								&& !GetPlayer()->GetShip()->IsTimeStopped())
							{
							Autopilot(false);
							ShowCommsSquadronMenu();
							}
						m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyClearTarget:
						GetPlayer()->SetTarget(NULL);
						m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyShipStatus:
						if (!GetPlayer()->DockingInProgress())
							GetModel().ShowShipScreen();
						m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyTargetNextEnemy:
						GetPlayer()->SelectNextTarget(1);
						m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyTargetPrevEnemy:
						GetPlayer()->SelectNextTarget(-1);
						m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyUseItem:
						if (!GetPlayer()->DockingInProgress()
								&& !GetPlayer()->GetShip()->IsTimeStopped())
							{
							Autopilot(false);
							ShowUsePicker();
							}
						m_chKeyDown = iVirtKey;
						break;

					case CGameKeys::keyNextWeapon:
						if (GetPlayer() && !m_bNextWeaponKey)
							{
							Autopilot(false);
							GetPlayer()->SetFireMain(false);
							GetPlayer()->ReadyNextWeapon(1);
							UpdateWeaponStatus();
							m_chKeyDown = iVirtKey;
							m_bNextWeaponKey = true;
							}
						break;

					case CGameKeys::keyPrevWeapon:
						if (GetPlayer() && !m_bPrevWeaponKey)
							{
							Autopilot(false);
							GetPlayer()->SetFireMain(false);
							GetPlayer()->ReadyNextWeapon(-1);
							UpdateWeaponStatus();
							m_chKeyDown = iVirtKey;
							m_bPrevWeaponKey = true;
							}
						break;

					case CGameKeys::keyThrustForward:
						if (!GetPlayer()->GetShip()->IsOutOfFuel()
								&& !GetPlayer()->GetShip()->IsTimeStopped())
							{
							Autopilot(false);
							GetPlayer()->SetThrust(true);
							}
						break;

					case CGameKeys::keyRotateLeft:
						if (!GetPlayer()->GetShip()->IsOutOfFuel()
								&& !GetPlayer()->GetShip()->IsTimeStopped())
							{
							Autopilot(false);
							GetPlayer()->SetManeuver(IShipController::RotateLeft);
							}
						break;

					case CGameKeys::keyRotateRight:
						if (!GetPlayer()->GetShip()->IsOutOfFuel()
								&& !GetPlayer()->GetShip()->IsTimeStopped())
							{
							Autopilot(false);
							GetPlayer()->SetManeuver(IShipController::RotateRight);
							}
						break;

					case CGameKeys::keyStop:
						if (!GetPlayer()->GetShip()->IsOutOfFuel()
								&& !GetPlayer()->GetShip()->IsTimeStopped())
							{
							Autopilot(false);
							GetPlayer()->SetStopThrust(true);
							}
						break;

					case CGameKeys::keyFireWeapon:
						if (!GetPlayer()->GetShip()->IsOutOfFuel()
								&& !GetPlayer()->GetShip()->IsTimeStopped())
							{
							Autopilot(false);
							GetPlayer()->SetFireMain(true);
							}
						break;

					case CGameKeys::keyFireMissile:
						if (!GetPlayer()->GetShip()->IsOutOfFuel()
								&& !GetPlayer()->GetShip()->IsTimeStopped())
							{
							Autopilot(false);
							GetPlayer()->SetFireMissile(true);
							GetPlayer()->SetUIMessageEnabled(uimsgFireMissileHint, false);
							}
						break;

					case CGameKeys::keyNextMissile:
						if (!m_bNextMissileKey)
							{
							Autopilot(false);
							GetPlayer()->ReadyNextMissile(1);
							UpdateWeaponStatus();
							GetPlayer()->SetUIMessageEnabled(uimsgSwitchMissileHint, false);
							m_bNextMissileKey = true;
							}
						break;

					case CGameKeys::keyPrevMissile:
						if (!m_bPrevMissileKey)
							{
							Autopilot(false);
							GetPlayer()->ReadyNextMissile(-1);
							UpdateWeaponStatus();
							GetPlayer()->SetUIMessageEnabled(uimsgSwitchMissileHint, false);
							m_bPrevMissileKey = true;
							}
						break;

					case CGameKeys::keyShowHelp:
						g_pHI->HICommand(CONSTLIT("uiShowHelp"));
						break;

					case CGameKeys::keyShowGameStats:
						g_pHI->HICommand(CONSTLIT("uiShowGameStats"));
						break;

					case CGameKeys::keyVolumeDown:
						{
						int iVolume = GetSoundVolumeOption();
						if (--iVolume >= 0)
							{
							SetSoundVolumeOption(iVolume);
							DisplayMessage(strPatternSubst(CONSTLIT("Volume %d"), iVolume));
							}
						break;
						}

					case CGameKeys::keyVolumeUp:
						{
						int iVolume = GetSoundVolumeOption();
						if (++iVolume <= 10)
							{
							SetSoundVolumeOption(iVolume);
							DisplayMessage(strPatternSubst(CONSTLIT("Volume %d"), iVolume));
							}
						break;
						}

					case CGameKeys::keyShowConsole:
						{
						if (m_pTC->GetOptionBoolean(CGameSettings::debugMode)
								&& !g_pUniverse->IsRegistered())
							m_bDebugConsole = !m_bDebugConsole;
						break;
						}

					case CGameKeys::keyEnableAllDevices:
						if (!GetPlayer()->GetShip()->IsTimeStopped())
							{
							GetPlayer()->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
							GetPlayer()->EnableAllDevices(true);
							}
						break;

					case CGameKeys::keyDisableAllDevices:
						if (!GetPlayer()->GetShip()->IsTimeStopped())
							{
							GetPlayer()->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
							GetPlayer()->EnableAllDevices(false);
							}
						break;

					case CGameKeys::keyEnableAllDevicesToggle:
						if (!GetPlayer()->GetShip()->IsTimeStopped())
							{
							GetPlayer()->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
							GetPlayer()->EnableAllDevices(!GetPlayer()->AreAllDevicesEnabled());
							}
						break;

					default:
						{
						if (iCommand >= CGameKeys::keyEnableDeviceToggle00 
								&& iCommand <= CGameKeys::keyEnableDeviceToggle31)
							{
							if (!GetPlayer()->GetShip()->IsTimeStopped())
								{
								int iDevice = (iCommand - CGameKeys::keyEnableDeviceToggle00);

								GetPlayer()->SetUIMessageEnabled(uimsgEnableDeviceHint, false);
								GetPlayer()->ToggleEnableDevice(iDevice);
								}
							}
						else if (iVirtKey == VK_ESCAPE)
							{
							if (m_bShowingMap)
								m_bShowingMap = false;
							else if (m_bAutopilot)
								Autopilot(false);
							else
								ShowGameMenu();
							}
						break;
						}
					}
				}
			break;
			}

		case gsIntro:
			OnKeyDownIntro(iVirtKey, dwKeyData);
			break;

		case gsProlog:
			m_bContinue = true;
			break;

		case gsDocked:
			{
			//	Deal with console

			if (m_bDebugConsole)
				{
				if (iVirtKey == VK_ESCAPE)
					m_bDebugConsole = false;
				else
					m_DebugConsole.OnKeyDown(iVirtKey, dwKeyData);
				}

			//	Other commands

			else
				{
				CGameKeys::Keys iCommand = m_pTC->GetKeyMap().GetGameCommand(iVirtKey);
				switch (iCommand)
					{
					case CGameKeys::keyShowConsole:
						{
						if (m_pTC->GetOptionBoolean(CGameSettings::debugMode)
								&& !g_pUniverse->IsRegistered())
							m_bDebugConsole = !m_bDebugConsole;
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

								m_CurrentDock.HandleKeyDown(iVirtKey);
								}
							}
						break;
					}
				}

			break;
			}
		}

	return 0;
	}

LONG CTranscendenceWnd::WMKeyUp (int iVirtKey, DWORD dwKeyData)

//	WMKeyUp
//
//	Handle WM_KEYUP

	{
	switch (m_State)
		{
		case gsInGame:
		case gsDocked:
		case gsEnteringStargate:
			{
			if (m_CurrentMenu != menuNone
					&& iVirtKey >= 'A' && iVirtKey < 'Z')
				NULL;

			else if (m_bDebugConsole)
				NULL;

			else
				{
				CGameKeys::Keys iCommand = m_pTC->GetKeyMap().GetGameCommand(iVirtKey);
				switch (iCommand)
					{
					case CGameKeys::keyThrustForward:
						if (GetPlayer())
							GetPlayer()->SetThrust(false);
						break;

					case CGameKeys::keyRotateLeft:
						if (GetPlayer() && GetPlayer()->GetManeuver() == IShipController::RotateLeft)
							GetPlayer()->SetManeuver(IShipController::NoRotation);
						break;

					case CGameKeys::keyRotateRight:
						if (GetPlayer() && GetPlayer()->GetManeuver() == IShipController::RotateRight)
							GetPlayer()->SetManeuver(IShipController::NoRotation);
						break;

					case CGameKeys::keyStop:
						if (GetPlayer())
							GetPlayer()->SetStopThrust(false);
						break;

					case CGameKeys::keyFireWeapon:
						if (GetPlayer())
							GetPlayer()->SetFireMain(false);
						break;

					case CGameKeys::keyFireMissile:
						if (GetPlayer())
							GetPlayer()->SetFireMissile(false);
						break;

					case CGameKeys::keyNextWeapon:
						m_bNextWeaponKey = false;
						break;

					case CGameKeys::keyNextMissile:
						m_bNextMissileKey = false;
						break;

					case CGameKeys::keyPrevWeapon:
						m_bPrevWeaponKey = false;
						break;

					case CGameKeys::keyPrevMissile:
						m_bPrevMissileKey = false;
						break;

					case CGameKeys::keyDock:
						m_bDockKeyDown = false;
						break;
					}
				}

			break;
			}
		}

	return 0;
	}

LONG CTranscendenceWnd::WMLButtonDblClick (int x, int y, DWORD dwFlags)

//	WMLButtonDblClick
//
//	Handle WM_LBUTTONDBLCLICK

	{
	switch (m_State)
		{
		case gsIntro:
			OnDblClickIntro(x, y, dwFlags);
			break;

		case gsDocked:
			m_pCurrentScreen->LButtonDown(x, y);
			break;
		}

	return 0;
	}

LONG CTranscendenceWnd::WMLButtonDown (int x, int y, DWORD dwFlags)

//	WMLButtonDown
//
//	Handle WM_LBUTTONDOWN

	{
	switch (m_State)
		{
		case gsIntro:
			OnLButtonDownIntro(x, y, dwFlags);
			break;

		case gsProlog:
			m_bContinue = true;
			break;

		case gsDocked:
			m_pCurrentScreen->LButtonDown(x, y);
			break;
		}

	return 0;
	}

LONG CTranscendenceWnd::WMLButtonUp (int x, int y, DWORD dwFlags)

//	WMLButtonUp
//
//	Handle WM_LBUTTONUP

	{
	switch (m_State)
		{
		case gsIntro:
			OnLButtonUpIntro(x, y, dwFlags);
			break;

		case gsDocked:
			m_pCurrentScreen->LButtonUp(x, y);
			break;
		}

	return 0;
	}

LONG CTranscendenceWnd::WMMouseMove (int x, int y, DWORD dwFlags)

//	WMMouseMove
//
//	Handle WM_MOUSEMOVE

	{
	switch (m_State)
		{
		case gsIntro:
			OnMouseMoveIntro(x, y, dwFlags);
			break;

		case gsDocked:
			m_pCurrentScreen->MouseMove(x, y);
			break;
		}

	return 0;
	}

LONG CTranscendenceWnd::WMMove (int x, int y)

//	WMMove
//
//	Handle WM_MOVE

	{
	RECT rcClient;

	::GetClientRect(m_hWnd, &rcClient);
	m_rcWindow.left = (RectWidth(rcClient) - g_cxScreen) / 2;
	m_rcWindow.right = m_rcWindow.left + g_cxScreen;
	m_rcWindow.top = (RectHeight(rcClient) - g_cyScreen) / 2;
	m_rcWindow.bottom = m_rcWindow.top + g_cyScreen;

	::IntersectRect(&m_rcWindow, &m_rcWindow, &rcClient);

	::ClientToScreen(m_hWnd, (LPPOINT)&m_rcWindow);
	::ClientToScreen(m_hWnd, (LPPOINT)&m_rcWindow + 1);

	return 0;
	}

LONG CTranscendenceWnd::WMSize (int cxWidth, int cyHeight, int iSize)

//	WMSize
//
//	Handle WM_SIZE

	{
	RECT rcClient;

	//	Compute the RECT where we draw the screen (in client coordinate)

	::GetClientRect(m_hWnd, &rcClient);
	m_rcWindowScreen.left = (RectWidth(rcClient) - g_cxScreen) / 2;
	m_rcWindowScreen.right = m_rcWindow.left + g_cxScreen;
	m_rcWindowScreen.top = (RectHeight(rcClient) - g_cyScreen) / 2;
	m_rcWindowScreen.bottom = m_rcWindow.top + g_cyScreen;
	::IntersectRect(&m_rcWindowScreen, &m_rcWindowScreen, &rcClient);

	//	Convert to screen coordinate

	m_rcWindow = m_rcWindowScreen;
	::ClientToScreen(m_hWnd, (LPPOINT)&m_rcWindow);
	::ClientToScreen(m_hWnd, (LPPOINT)&m_rcWindow + 1);

	//	Repaint

	::InvalidateRect(m_hWnd, NULL, TRUE);

	return 0;
	}
