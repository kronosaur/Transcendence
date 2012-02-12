//	PrologEpilog.cpp
//
//	Code to show prolog and epilog screens

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

#define TEXT_CRAWL_X						512
#define TEXT_CRAWL_HEIGHT					512
#define TEXT_CRAWL_WIDTH					384
#define TEXT_CRAWL_RIGHT_MARGIN				80

const int CRAWL_DURATION =					900;
const Metric CRAWL_SPEED =					1.0;

#define BAR_COLOR							CG16bitImage::RGBValue(0, 2, 10)
#define RGB_FRAME							CG16bitImage::RGBValue(80,80,80)
const WORD RGB_CRAWL_TEXT =					CG16bitImage::RGBValue(255,255,200);

#define PROP_OPACITY						CONSTLIT("opacity")
#define PROP_SCROLL_POS						CONSTLIT("scrollPos")
#define PROP_VIEWPORT_HEIGHT				CONSTLIT("viewportHeight")
#define PROP_SCALE							CONSTLIT("scale")
#define PROP_POSITION						CONSTLIT("position")

void CTranscendenceWnd::AnimateCrawlScreen (void)

//	AnimateCrawlScreen
//
//	Paints the crawl screen

	{
	CG16bitImage &TheScreen = g_pHI->GetScreen();

	PaintCrawlBackground();

	//	Paint the crawl

	m_CrawlText.Paint(TheScreen);
	m_CrawlText.Update();
	}

void CTranscendenceWnd::AnimateProlog (bool bTopMost)

//	AnimateProlog
//
//	Paint prologue screen

	{
	AnimateCrawlScreen();

	if (bTopMost)
		g_pHI->GetScreenMgr().Blt();

	//	If we're done creating the screen and the user wants to continue, then
	//	we go forward

	if (m_bContinue && m_bGameCreated)
		{
		//	Check for error

		if (!m_sBackgroundError.IsBlank())
			{
			CString sError = strPatternSubst(CONSTLIT("Unable to begin new game: %s"), m_sBackgroundError);
			kernelDebugLogMessage(sError.GetASCIIZPointer());

			StartIntro(isBlank);

			::MessageBox(m_hWnd, 
					sError.GetASCIIZPointer(), 
					"Transcendence", 
					MB_OK | MB_ICONSTOP);
			return;
			}

		StopProlog();
		StartGame(true);
		}
	}

void CTranscendenceWnd::GetCrawlAnimationRect (RECT *retrcRect)

//	GetCrawlAnimationRect
//
//	Returns the RECT that needs to be repainted when animating

	{
	retrcRect->top = m_rcMainScreen.top + (RectHeight(m_rcMainScreen) - TEXT_CRAWL_HEIGHT) / 2;
	retrcRect->left = m_rcMainScreen.left + TEXT_CRAWL_X;
	retrcRect->bottom = retrcRect->top + TEXT_CRAWL_HEIGHT;
	retrcRect->right = m_rcMainScreen.right - TEXT_CRAWL_RIGHT_MARGIN;
	}

void CTranscendenceWnd::InitCrawlBackground (DWORD dwImage)

//	InitCrawlBackground
//
//	Loads the appropriate image

	{
	if (dwImage)
		m_pCrawlImage = g_pUniverse->GetLibraryBitmap(dwImage);
	else
		m_pCrawlImage = NULL;
	}

ALERROR CTranscendenceWnd::InitCrawlScreen (void)

//	InitCrawlScreen
//
//	Initializes the crawl screen. Assumes that m_dwCrawlImage and
//	m_sCrawlText have been initialized.

	{
	ALERROR error;

	//	Set the image

	InitCrawlBackground(m_pTC->GetModel().GetCrawlImage());

	//	Set the text

	RECT rcRect;
	rcRect.top = (g_cyScreen - TEXT_CRAWL_HEIGHT) / 2;
	rcRect.left = m_rcMainScreen.left + TEXT_CRAWL_X;
	rcRect.bottom = rcRect.top + TEXT_CRAWL_HEIGHT;
	rcRect.right = rcRect.left + TEXT_CRAWL_WIDTH;
	m_CrawlText.SetFont(&m_Fonts.SubTitle);
	if (error = m_CrawlText.Init(rcRect, m_pTC->GetModel().GetCrawlText()))
		return error;

	return NOERROR;
	}

void CTranscendenceWnd::PaintCrawlBackground (void)

//	PaintCrawlBackground
//
//	Paints the crawl image and background

	{
	CG16bitImage &TheScreen = g_pHI->GetScreen();
	RECT rcInvalid;
	bool bInvalid = g_pHI->GetScreenMgr().GetInvalidRect(&rcInvalid);

	if (m_pCrawlImage)
		{
		RECT rcImage;
		rcImage.left = (g_cxScreen - m_pCrawlImage->GetWidth()) / 2;
		rcImage.top = (g_cyScreen - m_pCrawlImage->GetHeight()) / 2;
		rcImage.right = rcImage.left + m_pCrawlImage->GetWidth();
		rcImage.bottom = rcImage.top + m_pCrawlImage->GetHeight();

		//	Paint the parts that don't change

		if (bInvalid)
			{
			//	Paint bars across top and bottom

			TheScreen.Fill(0, 
					0, 
					g_cxScreen, 
					g_cyScreen,
					BAR_COLOR);

			//	Paint image

			TheScreen.Blt(0,
					0,
					m_pCrawlImage->GetWidth(),
					m_pCrawlImage->GetHeight(),
					*m_pCrawlImage,
					rcImage.left,
					rcImage.top);
			}

		//	Paint the background that the crawl is over

		RECT rcRect;
		GetCrawlAnimationRect(&rcRect);

		TheScreen.Blt(rcRect.left - rcImage.left,
				rcRect.top - rcImage.top,
				RectWidth(rcRect),
				RectHeight(rcRect),
				*m_pCrawlImage,
				rcRect.left,
				rcRect.top);

		//	Frame

		TheScreen.FillLine(0, rcImage.top, g_cxScreen, RGB_FRAME);
		TheScreen.FillLine(0, rcImage.bottom, g_cxScreen, RGB_FRAME);
		}
	else
		{
		if (bInvalid)
			{
			//	Erase the whole background

			TheScreen.Fill(0, 
					0, 
					g_cxScreen, 
					g_cyScreen,
					BAR_COLOR);
			}

		//	Paint the background that the crawl is over

		RECT rcRect;
		GetCrawlAnimationRect(&rcRect);

		TheScreen.Fill(rcRect.left,
				rcRect.top,
				RectWidth(rcRect),
				RectHeight(rcRect),
				BAR_COLOR);
		}
	}

ALERROR CTranscendenceWnd::StartProlog (void)

//	StartProlog
//
//	Starts the prolog screen

	{
	ALERROR error;

	//	Init the crawl screen

	if (error = InitCrawlScreen())
		return error;

	//	If we've got a prolog screen then we wait for the player to
	//	click before we continue. Otherwise, we continue automatically

	m_bContinue = (m_pTC->GetModel().GetCrawlImage() == 0);

	//	Done

	m_State = gsProlog;
	ShowCursor(true);

	return NOERROR;
	}

void CTranscendenceWnd::StopProlog (void)

//	StopProlog
//
//	Clean up the prolog screen

	{
	ASSERT(m_State == gsProlog);

	m_CrawlText.CleanUp();
	ShowCursor(false);
	}
