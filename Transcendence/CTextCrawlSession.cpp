//	CTextCrawlSession.cpp
//
//	CTextCrawlSession class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define EVENT_ON_ANIMATION_DONE				CONSTLIT("onAnimationDone")

#define ID_CTRL_WAIT						CONSTLIT("ctrlWait")
#define ID_TEXT_CRAWL_PERFORMANCE			CONSTLIT("textCrawl")
#define ID_CTRL_TITLE						CONSTLIT("title")

#define CMD_OK_SESSION						CONSTLIT("cmdOKSession")
#define CMD_SHOW_OK_BUTTON					CONSTLIT("cmdShowOKButton")
#define CMD_SHOW_WAIT_ANIMATION				CONSTLIT("cmdShowWaitAnimation")

const int TEXT_CRAWL_X =					512;
const int TEXT_CRAWL_HEIGHT =				512;
const int TEXT_CRAWL_WIDTH =				384;
const int TEXT_CRAWL_RIGHT_MARGIN =			80;

const int CRAWL_DURATION =					900;
const Metric CRAWL_SPEED =					1.0;

#define PROP_OPACITY						CONSTLIT("opacity")
#define PROP_SCROLL_POS						CONSTLIT("scrollPos")
#define PROP_VIEWPORT_HEIGHT				CONSTLIT("viewportHeight")
#define PROP_SCALE							CONSTLIT("scale")
#define PROP_POSITION						CONSTLIT("position")
#define PROP_FADE_EDGE_HEIGHT				CONSTLIT("fadeEdgeHeight")

CTextCrawlSession::CTextCrawlSession (CHumanInterface &HI,
									  CCloudService &Service,
									  const CG32bitImage *pImage,
									  const CString &sText,
									  const CString &sCmdDone) : IHISession(HI),
		m_Service(Service),
		m_pImage(pImage),
		m_sText(sText),
		m_sCmdDone(sCmdDone),
		m_bWaitAnimation(false)

//	CTextCrawSession constructor

	{
	}

void CTextCrawlSession::CreateCrawlAnimation (const CString &sText, const RECT &rcRect, IAnimatron **retpAni)

//	CreateCrawlAnimation
//
//	Creates the prologue and epilogue animation

	{
	int i;

	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);
	CG32bitPixel rgbColor = VI.GetColor(colorTextAltHighlight);

	//	Adjust because scroller does not clip at the bottom

	int cyHeight = RectHeight(rcRect) - SubTitleFont.GetHeight();

	//	Create a scroller

	CAniVScroller *pAni = new CAniVScroller;
	pAni->SetPropertyVector(PROP_POSITION, CVector((Metric)rcRect.left, (Metric)rcRect.top));
	pAni->SetPropertyMetric(PROP_VIEWPORT_HEIGHT, (Metric)cyHeight);
	pAni->SetPropertyMetric(PROP_FADE_EDGE_HEIGHT, (Metric)(cyHeight / 8));

	//	Add the text

	TArray<CString> Lines;
	SubTitleFont.BreakText(sText, RectWidth(rcRect), &Lines);

	for (i = 0; i < Lines.GetCount(); i++)
		{
		IAnimatron *pText;

		CAniText::Create(Lines[i],
				CVector(0.0, pAni->GetHeight()),
				&SubTitleFont,
				0,
				rgbColor,
				&pText);
		pAni->AddLine(pText);
		}

	//	Scroll from the bottom until the text is centered.
	//	Compute the center position (remember that coordinates are cartessian; down is negative)

	int yPos = -((RectHeight(rcRect) - (SubTitleFont.GetHeight() * Lines.GetCount())) / 2);

	CLinearMetric *pScroller = new CLinearMetric;
	pScroller->SetParams(-cyHeight, yPos, CRAWL_SPEED);
	pScroller->AddListener(EVENT_ON_ANIMATION_DONE, this, CMD_SHOW_OK_BUTTON);
	pAni->AnimateProperty(PROP_SCROLL_POS, pScroller, 0);

	//	After the scrolling is done, fade-out

	CLinearFade *pFader = new CLinearFade;
	pFader->SetParams(pScroller->GetDuration() + CRAWL_DURATION, 0, 30);
	pAni->AnimateProperty(PROP_OPACITY, pFader, 0);

	//	Done

	*retpAni = pAni;
	}

void CTextCrawlSession::OnCleanUp (void)

//	OnCleanUp
//
//	Clean up

	{
	}

ALERROR CTextCrawlSession::OnCommand (const CString &sCmd, void *pData)

//	OnCommand
//
//	Handle a command

	{
	if (strEquals(sCmd, CMD_OK_SESSION))
		{
		OnCommand(CMD_SHOW_WAIT_ANIMATION);
		m_HI.HICommand(m_sCmdDone);
		}
	else if (strEquals(sCmd, CMD_SHOW_OK_BUTTON))
		{
		CUIHelper Helper(m_HI);
		IAnimatron *pTitle;
		Helper.CreateSessionTitle(this, 
				m_Service, 
				NULL_STR, 
				NULL, 
				CUIHelper::OPTION_SESSION_NO_HEADER 
					| CUIHelper::OPTION_SESSION_NO_CANCEL_BUTTON
					| CUIHelper::OPTION_SESSION_OK_BUTTON, 
				&pTitle);
		StartPerformance(pTitle, ID_CTRL_TITLE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
		}
	else if (strEquals(sCmd, CMD_SHOW_WAIT_ANIMATION))
		{
		if (!m_bWaitAnimation)
			{
			CUIHelper Helper(m_HI);

			IAnimatron *pAni;
			Helper.CreateSessionWaitAnimation(ID_CTRL_WAIT, CONSTLIT("Creating Game"), &pAni);
			StartPerformance(pAni, ID_CTRL_WAIT, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

			m_bWaitAnimation = true;
			}
		}

	return NOERROR;
	}

ALERROR CTextCrawlSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	RECT rcCenter;
	const CVisualPalette &VI = m_HI.GetVisuals();
	VI.GetWidescreenRect(&rcCenter);

	//	Compute the RECT where the text will go

	RECT rcRect;
	rcRect.top = rcCenter.top + (RectHeight(rcCenter) - TEXT_CRAWL_HEIGHT) / 2;
	rcRect.left = rcCenter.left + (RectWidth(rcCenter) / 2);
	rcRect.bottom = rcRect.top + TEXT_CRAWL_HEIGHT;
	rcRect.right = rcRect.left + TEXT_CRAWL_WIDTH;

	//	Create the text animation

	IAnimatron *pAni;
	CreateCrawlAnimation(m_sText, rcRect, &pAni);
	StartPerformance(pAni, ID_TEXT_CRAWL_PERFORMANCE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	return NOERROR;
	}

void CTextCrawlSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//	OnKeyDown
//
//	KeyDown

	{
	switch (iVirtKey)
		{
		case VK_UP:
		case VK_DOWN:
			break;

		default:
			OnCommand(CMD_OK_SESSION);
			break;
		}
	}

void CTextCrawlSession::OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture)

//	OnLButtonDown
//
//	LButtonDown

	{
	//	Done

	OnCommand(CMD_OK_SESSION);
	}

void CTextCrawlSession::OnPaint (CG32bitImage &Screen, const RECT &rcInvalid)

//	OnPaint
//
//	Paint

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	CG32bitImage Blank;
	const CG32bitImage *pImage = (m_pImage ? m_pImage : &Blank);

	RECT rcCenter;
	VI.DrawSessionBackground(Screen, *pImage, VI.GetColor(colorAreaDeep), 0, &rcCenter);
	}

void CTextCrawlSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash
//
//	Describe current state

	{
	*retsMessage = CONSTLIT("session: CTextCrawlSession\r\n");
	}

void CTextCrawlSession::OnUpdate (bool bTopMost)

//	OnUpdate
//
//	Update

	{
	}
