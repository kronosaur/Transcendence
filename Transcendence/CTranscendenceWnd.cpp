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

#define DEBUG_CONSOLE_WIDTH					0.5
#define DEBUG_CONSOLE_HEIGHT				0.8

int g_cxScreen = 0;
int g_cyScreen = 0;

#define CONTROLLER_PLAYER					CONSTLIT("player")

#define STR_SMALL_TYPEFACE					CONSTLIT("Tahoma")
#define STR_MEDIUM_TYPEFACE					CONSTLIT("Tahoma")
#define STR_LARGE_TYPEFACE					CONSTLIT("Trebuchet MS")
#define STR_FIXED_TYPEFACE					CONSTLIT("Lucida Console")

#define BAR_COLOR							CG32bitPixel(0, 2, 10)

CTranscendenceWnd::CTranscendenceWnd (HWND hWnd, CTranscendenceController *pTC) : m_hWnd(hWnd),
		m_pTC(pTC),
		m_State(gsNone),
		m_pCurrentScreen(NULL),
		m_bAutopilot(false),
		m_dwIntroShipClass(0),
		m_CurrentMenu(menuNone),
		m_CurrentPicker(pickNone),
		m_pIntroSystem(NULL),
		m_pStargateEffect(NULL),
		m_pMenuObj(NULL),
		m_bRedirectDisplayMessage(false),
		m_chKeyDown('\0'),
		m_bDockKeyDown(false),
		m_bPaused(false)

//	CTranscendence constructor

	{
	ClearDebugLines();
	}

void CTranscendenceWnd::CleanUpPlayerShip (void)

//	CleanUpPlayerShip
//
//	Clean up player ship structures when ship is destroyed

	{
	DEBUG_TRY

	m_LRSDisplay.CleanUp();
	m_DeviceDisplay.CleanUp();
	m_MenuDisplay.CleanUp();
	m_PickerDisplay.CleanUp();

	m_bPaused = false;

	DEBUG_CATCH
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
	g_cxScreen = g_pHI->GetScreenMgr().GetWidth();
	g_cyScreen = g_pHI->GetScreenMgr().GetHeight();
	}

IPlayerController *CTranscendenceWnd::CreatePlayerController (void)

//	CreatePlayerController
	
	{
	return new CTranscendencePlayer; 
	}

IShipController *CTranscendenceWnd::CreateShipController (const CString &sController)

//	CreateShipController
//
//	Creates custom ship controllers.

	{
	if (strEquals(sController, CONTROLLER_PLAYER))
		return new CPlayerShipController;
	else
		return NULL;
	}

void CTranscendenceWnd::DebugConsoleOutput (const CString &sOutput)

//	DebugConsoleOutput
//
//	Output to debug console

	{
	m_pTC->GetDebugConsole().Output(sOutput);
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

void CTranscendenceWnd::GetMousePos (POINT *retpt)

//	GetMousePos
//
//	Returns the mouse position relative to the Transcendence window

	{
	::GetCursorPos(retpt);
	::ScreenToClient(m_hWnd, retpt);
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

void CTranscendenceWnd::OnStargateSystemReady (void)

//	OnStargateSystemReady
//
//	The new system is ready

	{
	ASSERT(m_State == gsWaitingForSystem);
	m_State = gsLeavingStargate;
	m_iCountdown = TICKS_AFTER_GATE;

	if (m_pStargateEffect)
		{
		delete m_pStargateEffect;
		m_pStargateEffect = NULL;
		}
	}

void CTranscendenceWnd::PaintDebugLines (void)

//	PaintDebugLines
//
//	Paint debug output

	{
	CG32bitImage &TheScreen = g_pHI->GetScreen();

#ifdef DEBUG
	int iLine = m_iDebugLinesStart;
	int iPos = 0;
	while (iLine != m_iDebugLinesEnd)
		{
		TheScreen.DrawText(0,
				iPos++ * 12,
				m_Fonts.Medium,
				CG32bitPixel(128,0,0),
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
	CG32bitImage &TheScreen = g_pHI->GetScreen();

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

		TheScreen.DrawText(300, 0, m_Fonts.Header, CG32bitPixel(80,80,80), CString(szBuffer, iLen));

		//	Every once in a while, output to log file

		if (m_iFrameCount > FRAME_RATE_COUNT
				&& ((m_iFrameCount % 300) == 0))
			kernelDebugLogString(CString(szBuffer));

		//	Next

		m_iFrameCount++;
		m_iStartAnimation = iNow;
		}
	}

void CTranscendenceWnd::PlayerEndGame (void)

//	PlayerEndGame
//
//	End the game (we assume we've called the model already).

	{
	m_State = gsEndGame;
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
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psPaintingSRS:
			m_sCrashInfo.Append(CONSTLIT("program state: painting SRS\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			ReportCrashObj(&m_sCrashInfo);
			break;

		case psUpdating:
			m_sCrashInfo.Append(CONSTLIT("program state: updating universe\r\n"));
			break;

		case psUpdatingEvents:
			m_sCrashInfo.Append(CONSTLIT("program state: updating events\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			ReportCrashEvent(&m_sCrashInfo);
			break;

		case psUpdatingBehavior:
			m_sCrashInfo.Append(CONSTLIT("program state: updating object behavior\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			ReportCrashObj(&m_sCrashInfo);
			break;

		case psUpdatingObj:
			m_sCrashInfo.Append(CONSTLIT("program state: updating object\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			ReportCrashObj(&m_sCrashInfo);
			break;

		case psUpdatingHitTest:
			m_sCrashInfo.Append(CONSTLIT("program state: updating object hit test\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			ReportCrashObj(&m_sCrashInfo);
			break;

		case psUpdatingMove:
			m_sCrashInfo.Append(CONSTLIT("program state: updating object motion\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			ReportCrashObj(&m_sCrashInfo);
			break;

		case psUpdatingEncounters:
			m_sCrashInfo.Append(CONSTLIT("program state: updating random encounters\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psDestroyNotify:
			m_sCrashInfo.Append(CONSTLIT("program state: handling OnObjDestroy\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			ReportCrashObj(&m_sCrashInfo);
			break;

		case psUpdatingReactorDisplay:
			m_sCrashInfo.Append(CONSTLIT("program state: updating reactor display\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psPaintingMap:
			m_sCrashInfo.Append(CONSTLIT("program state: painting map\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psPaintingLRS:
			m_sCrashInfo.Append(CONSTLIT("program state: painting LRS\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psPaintingArmorDisplay:
			m_sCrashInfo.Append(CONSTLIT("program state: painting armor display\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psPaintingMessageDisplay:
			m_sCrashInfo.Append(CONSTLIT("program state: painting message display\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psPaintingReactorDisplay:
			m_sCrashInfo.Append(CONSTLIT("program state: painting reactor display\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psPaintingTargetDisplay:
			m_sCrashInfo.Append(CONSTLIT("program state: painting target display\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			ReportCrashObj(&m_sCrashInfo, (GetPlayer() ? GetPlayer()->GetShip() : NULL));
			break;

		case psPaintingDeviceDisplay:
			m_sCrashInfo.Append(CONSTLIT("program state: painting device display\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psStargateEnter:
			m_sCrashInfo.Append(CONSTLIT("program state: entering stargate\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psStargateEndGame:
			m_sCrashInfo.Append(CONSTLIT("program state: entering end-game stargate\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psStargateLoadingSystem:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate, loading destination system\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psStargateCreatingSystem:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate, creating destination system\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psStargateUpdateExtended:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate, updating extended\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psStargateTransferringGateFollowers:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate, transferring gate followers\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psStargateSavingSystem:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate, saving current system\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psStargateFlushingSystem:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate, flushing current system\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psStargateGarbageCollecting:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate, garbage-collecting images\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psStargateLoadingBitmaps:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate, loading new images\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psLoadingImages:
			m_sCrashInfo.Append(CONSTLIT("program state: in LoadImages\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			ReportCrashObj(&m_sCrashInfo);
			break;

		case psStargateEnterDone:
			m_sCrashInfo.Append(CONSTLIT("program state: enter stargate done\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;

		case psOnObjDestroyed:
			m_sCrashInfo.Append(CONSTLIT("program state: in OnObjDestroyed\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			ReportCrashObj(&m_sCrashInfo);
			break;

		case psCustomError:
			if (g_sProgramError)
				m_sCrashInfo.Append(*g_sProgramError);
			ReportCrashSystem(&m_sCrashInfo);
			m_sCrashInfo.Append(CONSTLIT("\r\n"));
			break;

		default:
			m_sCrashInfo.Append(CONSTLIT("program state: unknown\r\n"));
			ReportCrashSystem(&m_sCrashInfo);
			break;
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

void CTranscendenceWnd::ReportCrashSystem (CString *retsMessage) const

//	ReportCrashSystem
//
//	Reports information about the current system.

	{
	CSystem *pSystem = (g_pUniverse ? g_pUniverse->GetCurrentSystem() : NULL);
	if (pSystem == NULL)
		return;

	CString sSystemNode;
	CString sSystemName;
	DWORD dwSystemType = 0;
	CSystem::SDebugInfo DebugInfo;

	try
		{
		//	Get some basic data

		CTopologyNode *pNode = pSystem->GetTopology();
		if (pNode == NULL)
			sSystemNode = CONSTLIT("none");
		else
			sSystemNode = pNode->GetID();

		CSystemType *pType = pSystem->GetType();
		dwSystemType = (pType ? pType->GetUNID() : 0);

		sSystemName = pSystem->GetName();

		//	Debug info

		pSystem->GetDebugInfo(DebugInfo);
		}
	catch (...)
		{
		retsMessage->Append(CONSTLIT("error obtaining system data.\r\n"));
		}

	retsMessage->Append(strPatternSubst(CONSTLIT("system: %s\r\nsystem type: %08x\r\nsystem name: %s\r\n"), sSystemNode, dwSystemType, sSystemName));
	retsMessage->Append(strPatternSubst(CONSTLIT("total objs: %d\r\ndestroyed: %d\r\ndeleted: %d\r\nbad: %d\r\nstars: %d\r\n"), 
			DebugInfo.iTotalObjs, 
			DebugInfo.iDestroyedObjs,
			DebugInfo.iDeletedObj,
			DebugInfo.iBadObjs,
			DebugInfo.iStarObjs));

	if (DebugInfo.bBadStarCache)
		retsMessage->Append(CONSTLIT("bad star cache\r\n"));
	}

void CTranscendenceWnd::ShowErrorMessage (const CString &sError)

//	ShowErrorMessage
//
//	Shows an error message box

	{
	::kernelDebugLogString(sError);

	::MessageBox(m_hWnd, 
			sError.GetASCIIZPointer(), 
			"Transcendence", 
			MB_OK | MB_ICONSTOP);
	}

ALERROR CTranscendenceWnd::StartGame (void)

//	StartGame
//
//	Start the game (either a new game or a previously saved game)

	{
	ALERROR error;

	//	Initialize sub-displays

	if (error = InitDisplays())
		return error;

	//	Welcome

	m_MessageDisplay.ClearAll();
	const CString &sWelcome = g_pUniverse->GetCurrentAdventureDesc()->GetWelcomeMessage();
	if (!sWelcome.IsBlank())
		m_MessageDisplay.DisplayMessage(sWelcome, m_Fonts.rgbTitleColor);

	//	Set the state appropriately

	m_State = gsInGame;
	m_bPaused = false;
	m_bPausedStep = false;
	m_bAutopilot = false;
	m_CurrentPicker = pickNone;
	m_CurrentMenu = menuNone;
	m_iTick = 0;

	return NOERROR;
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
	kernelDebugLogString(m_sVersion);
	}

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

	m_Fonts.Header.CreateFromResource(NULL, "DXFN_HEADER");
	m_Fonts.HeaderBold.CreateFromResource(NULL, "DXFN_HEADER_BOLD");
	m_Fonts.SubTitle.CreateFromResource(NULL, "DXFN_SUBTITLE");
	m_Fonts.SubTitleBold.CreateFromResource(NULL, "DXFN_SUBTITLE_BOLD");
	m_Fonts.SubTitleHeavyBold.CreateFromResource(NULL, "DXFN_SUBTITLE_HEAVY_BOLD");
	m_Fonts.Title.CreateFromResource(NULL, "DXFN_TITLE");
	m_Fonts.LogoTitle.CreateFromResource(NULL, "DXFN_LOGO_TITLE");
	m_Fonts.Console.Create(STR_FIXED_TYPEFACE, -14);

	//	Output the typeface that we got

	if (m_pTC->GetOptionBoolean(CGameSettings::debugVideo))
		{
		kernelDebugLogPattern("Small typeface: %s", m_Fonts.Small.GetTypeface());
		kernelDebugLogPattern("Medium typeface: %s", m_Fonts.Large.GetTypeface());
		kernelDebugLogPattern("Large typeface: %s", m_Fonts.Header.GetTypeface());
		kernelDebugLogPattern("Console typeface: %s", m_Fonts.Console.GetTypeface());
		}

	//	Set colors

	m_Fonts.rgbTextColor = CG32bitPixel(191,196,201);
	m_Fonts.rgbTitleColor = CG32bitPixel(218,235,255);
	m_Fonts.rgbLightTitleColor = CG32bitPixel(120,129,140);
	m_Fonts.rgbHelpColor = CG32bitPixel(103,114,128);
	m_Fonts.rgbBackground = CG32bitPixel(15,17,18);
	m_Fonts.rgbSectionBackground = CG32bitPixel(86,82,73);
	m_Fonts.rgbSelectBackground = CG32bitPixel(237, 137, 36);

	m_Fonts.rgbAltGreenColor = CG32bitPixel(5,211,5);
	m_Fonts.rgbAltGreenBackground = CG32bitPixel(23,77,23);
	m_Fonts.rgbAltYellowColor = CG32bitPixel(255,225,103);
	m_Fonts.rgbAltYellowBackground = CG32bitPixel(65,57,24);
	m_Fonts.rgbAltRedColor = CG32bitPixel(4,179,4);
	m_Fonts.rgbAltRedBackground = CG32bitPixel(76,0,0);
	m_Fonts.rgbAltBlueColor = CG32bitPixel(87,111,205);
	m_Fonts.rgbAltBlueBackground = CG32bitPixel(52,57,64);

	m_Fonts.rgbItemTitle = CG32bitPixel(255,255,255);
	m_Fonts.rgbItemRef = CG32bitPixel(255,255,255);
	m_Fonts.rgbItemDesc = CG32bitPixel(128,128,128);
	m_Fonts.rgbItemDescSelected = CG32bitPixel(200,200,200);

	//	Initialize UI resources

	if (error = m_UIRes.Init(&m_Fonts))
		{
		*retsError = CONSTLIT("Unable to initialize UI resources.");
		goto Fail;
		}

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

LONG CTranscendenceWnd::WMRButtonDown (int x, int y, DWORD dwFlags)

//	WMRButtonDown
//
//	Handle WM_RBUTTONDOWN

	{
	switch (m_State)
		{
        case gsInGame:
#if 0

            if (GetPlayer() && GetPlayer()->IsMouseAimEnabled())
    			Autopilot(false);
#endif
            break;
		}

	return 0;
	}

LONG CTranscendenceWnd::WMRButtonUp (int x, int y, DWORD dwFlags)

//	WMRButtonUp
//
//	Handle WM_RBUTTONUP

	{
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
