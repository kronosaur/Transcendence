//	LoadingScreen.cpp
//
//	Show loading screen

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

#define STARGATE_WIDTH						128
#define STARGATE_HEIGHT						128

#define RGB_BAR								CG16bitImage::RGBValue(0, 2, 10)
#define RGB_LOADING_TEXT					CG16bitImage::RGBValue(128, 128, 128)
#define RGB_COPYRIGHT_TEXT					CG16bitImage::RGBValue(56, 63, 90)
#define STR_TYPEFACE						CONSTLIT("Tahoma")
#define RGB_FRAME							CG16bitImage::RGBValue(80,80,80)

#define Y_COPYRIGHT_TEXT					392

#define TIME_BLT_LOCK						15
#define TIME_BLT_NO_LOCK					15

const int MIN_BAR_HEIGHT =					64;

void CTranscendenceWnd::AnimateLoading (void)

//	AnimateLoading
//
//	Animate loading screen

	{
	PaintLoadingScreen();

	//	Time our blts so that we can determine what method to use

	DWORD dwStartTimer;
	if (m_iCountdown <= (TIME_BLT_LOCK + TIME_BLT_NO_LOCK))
		dwStartTimer = ::GetTickCount();

	BltScreen();

	if (m_iCountdown <= (TIME_BLT_LOCK + TIME_BLT_NO_LOCK))
		m_iBltTime[0] += (::GetTickCount() - dwStartTimer);

	m_iCountdown++;

	//	After we've got enough blts, try with no lock

	if (m_iCountdown == TIME_BLT_LOCK)
		{
		m_iBltTime[1] = m_iBltTime[0] / TIME_BLT_LOCK;
		m_iBltTime[0] = 0;

#if 0
		if (m_Options.bDirectX)
			m_Screen.AssociateSurface(m_pDD);
#endif
		}

	//	After blts with no lock, see which is faster

	else if ((m_iCountdown - TIME_BLT_LOCK) == TIME_BLT_NO_LOCK)
		{
		m_iBltTime[2] = m_iBltTime[0] / TIME_BLT_NO_LOCK;

#if 0
		if (m_Options.bDebugVideo)
			{
			kernelDebugLogMessage("Blt with Lock: %d ms", m_iBltTime[1]);
			kernelDebugLogMessage("Blt without Lock: %d ms", m_iBltTime[2]);
			}

		//	If using a lock is faster, discard the surface

		if (m_Options.bDebugManualBlt || m_iBltTime[1] < m_iBltTime[2])
			{
			if (m_Options.bDebugVideo)
				kernelDebugLogMessage("Blt with Lock enabled");

			m_Screen.DiscardSurface();
			}
#endif

		//	We're done with timing; start loading the universe

		m_bLoadComplete = false;
		SetBackgroundState(bsLoadUniverse);
		}

	//	See if we're done

	if (m_iCountdown > TIME_BLT_LOCK + TIME_BLT_NO_LOCK
			&& m_bLoadComplete)
		{
		if (!m_sBackgroundError.IsBlank())
			{
			kernelDebugLogMessage(m_sBackgroundError.GetASCIIZPointer());

			m_State = gsNone;
			::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
			return;
			}

		StopLoading();

		//	Some debugging

#ifdef DEBUG_NEBULA_PAINTING
		{
		int i;
		int iCount = 1000;

		CSpaceEnvironmentType *pNebula = g_pUniverse->FindSpaceEnvironment(0x00009201);

		DWORD dwStart = ::GetTickCount();
		for (i = 0; i < iCount; i++)
			{
			pNebula->Paint(m_Screen, 320, 320);
			}

		char szBuffer[1024];
		wsprintf(szBuffer, "Paint %d in %d ms\n", iCount, ::GetTickCount() - dwStart);
		::OutputDebugString(szBuffer);
		}
#endif

		//	Start the intro screen

		StartIntro(isOpeningTitles);
		}
	}

ALERROR CTranscendenceWnd::LoadUniverseDefinition (void)

//	LoadUniverseDefinition
//
//	Load the universe definition file

	{
	ALERROR error;

	//	Make sure the universe know about our various managers

	g_pUniverse->SetDebugMode(m_Options.bDebugGame);
	g_pUniverse->SetSoundMgr(&g_pHI->GetSoundMgr());
#ifdef DEBUG
	g_pUniverse->SetDebugOutput(this);
#endif

	//	Figure out what to load. If no extension is supplied, we check for an XML
	//	file first. Otherwise, we load the .tdb

	CString sGameFile;
	if (m_Options.bUseTDB)
		sGameFile = CONSTLIT("Transcendence.tdb");
	else
		sGameFile = CONSTLIT("Transcendence");

	//	Load the Transcendence Data Definition file that describes the universe.

	if (error = g_pUniverse->Init(TransPath(sGameFile), &m_sBackgroundError))
		return error;

	//	Initialize Transcendence extensions

	CCodeChain &CC = g_pUniverse->GetCC();
	if (error = InitCodeChainExtensions(CC))
		return error;

	return NOERROR;
	}

void CTranscendenceWnd::PaintLoadingScreen (void)

//	PaintLoadingScreen
//
//	Paints the loading screen

	{
	CG16bitImage &TheScreen = g_pHI->GetScreen();

	int cyBarHeight = Max(MIN_BAR_HEIGHT, (g_cyScreen - m_TitleImage.GetHeight()) / 2);
	int xImage = (g_cxScreen - m_TitleImage.GetWidth()) / 2;
	int yImage = (g_cyScreen - m_TitleImage.GetHeight()) / 2;

	//	Paint the parts that don't change

	if (m_bTitleInvalid)
		{
		//	Paint bars across top and bottom

		TheScreen.Fill(0, 
				0, 
				g_cxScreen, 
				cyBarHeight,
				RGB_BAR);

		TheScreen.Fill(0, 
				g_cyScreen - cyBarHeight, 
				g_cxScreen, 
				cyBarHeight,
				RGB_BAR);

		//	Paint image

		TheScreen.Blt(0,
				0,
				m_TitleImage.GetWidth(),
				m_TitleImage.GetHeight(),
				m_TitleImage,
				xImage,
				yImage);

	//	Paint a frame around viewscreen

		TheScreen.FillLine(0, yImage, g_cxScreen, RGB_FRAME);
		TheScreen.FillLine(0, yImage + m_TitleImage.GetHeight(), g_cxScreen, RGB_FRAME);

		//	Paint the copyright text

		int cxWidth = m_Fonts.MediumHeavyBold.MeasureText(m_sCopyright);
		m_Fonts.MediumHeavyBold.DrawText(TheScreen,
				(g_cxScreen - cxWidth) / 2,
				yImage + Y_COPYRIGHT_TEXT,
				m_Fonts.wTitleColor,
				m_sCopyright);

		//	Paint the loading title

		CString sLoading = CONSTLIT("Loading");
		cxWidth = m_Fonts.SubTitle.MeasureText(sLoading, NULL);
		m_Fonts.SubTitle.DrawText(TheScreen,
				(g_cxScreen - cxWidth) / 2,
				m_rcMainScreen.bottom - (m_Fonts.SubTitle.GetHeight() + 32),
				RGB_LOADING_TEXT,
				sLoading);

#ifdef PERSISTENT_BUFFER
		m_bTitleInvalid = false;
#endif
		}

	//	Animate the stargate

	RECT rcSG;
	rcSG.left = (g_cxScreen - STARGATE_WIDTH) / 2;
	rcSG.right = rcSG.left + STARGATE_WIDTH;
	rcSG.top = m_rcScreen.bottom - ((RectHeight(m_rcScreen) - m_TitleImage.GetHeight()) / 2) - STARGATE_HEIGHT / 2;
	rcSG.bottom = rcSG.top + STARGATE_HEIGHT;

#ifdef PERSISTENT_BUFFER
	if ((m_iCountdown % 2) == 0)
		{
		//	Erase the stargate

		TheScreen.Blt(rcSG.left - xImage,
				m_TitleImage.GetHeight() - STARGATE_HEIGHT / 2,
				STARGATE_WIDTH,
				STARGATE_HEIGHT / 2,
				m_TitleImage,
				rcSG.left,
				rcSG.top);

		TheScreen.Fill(rcSG.left, 
				rcSG.top + STARGATE_HEIGHT / 2, 
				STARGATE_WIDTH, 
				STARGATE_HEIGHT / 2,
				RGB_BAR);

		//	Paint stargate

		TheScreen.ColorTransBlt(STARGATE_WIDTH * ((m_iCountdown / 2) % 12),
				0,
				STARGATE_WIDTH,
				STARGATE_HEIGHT,
				255,
				m_StargateImage,
				rcSG.left,
				rcSG.top);
		}
#else
	TheScreen.ColorTransBlt(STARGATE_WIDTH * ((m_iCountdown / 2) % 12),
			0,
			STARGATE_WIDTH,
			STARGATE_HEIGHT,
			255,
			m_StargateImage,
			rcSG.left,
			rcSG.top);
#endif
	}

ALERROR CTranscendenceWnd::StartLoading (void)

//	StartLoading
//
//	Start loading universe (on a different thread)

	{
	ALERROR error;

	m_sBackgroundError = NULL_STR;
	m_State = gsLoading;
	m_iCountdown = 0;
	m_iBltTime[0] = 0;

	//	Start a background thread to load the universe

	CreateBackgroundThread();

	//	Load a JPEG of the loading screen

	HBITMAP hDIB;
	if (error = JPEGLoadFromResource(g_hInst, 
			MAKEINTRESOURCE(IDR_TITLE_IMAGE),
			JPEG_LFR_DIB, 
			NULL, 
			&hDIB))
		return error;

	error = m_TitleImage.CreateFromBitmap(hDIB);
	::DeleteObject(hDIB);
	if (error)
		return error;

	//	Load stargate image

	if (error = JPEGLoadFromResource(g_hInst,
			MAKEINTRESOURCE(IDR_STARGATE_IMAGE),
			JPEG_LFR_DIB, 
			NULL, 
			&hDIB))
		return error;

	HBITMAP hBitmask;
	if (error = dibLoadFromResource(g_hInst,
			MAKEINTRESOURCE(IDR_STARGATE_MASK),
			&hBitmask))
		return error;

	error = m_StargateImage.CreateFromBitmap(hDIB, hBitmask);
	::DeleteObject(hDIB);
	::DeleteObject(hBitmask);
	if (error)
		return error;

	//	Fonts

	m_bTitleInvalid = true;

	return NOERROR;
	}

void CTranscendenceWnd::StopLoading (void)

//	StopLoading
//
//	Clean up loading screen

	{
	ASSERT(m_State == gsLoading);

	m_TitleImage.Destroy();
	m_StargateImage.Destroy();
	}

