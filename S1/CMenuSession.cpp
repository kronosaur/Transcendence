//	CMenuSession.cpp
//
//	CMenuSession class

#include "stdafx.h"
#include "S1.h"

#define ID_GAME_STAT_PERFORMANCE			CONSTLIT("gameStatPerformance")

#define ID_GAME_STAT_FADER					CONSTLIT("gsFader")
#define ID_GAME_STAT_SCROLLER				CONSTLIT("gsScroller")
#define ID_GAME_STAT_SELECT_RECT			CONSTLIT("gsSelRect")

#define PROP_OPACITY						CONSTLIT("opacity")
#define PROP_SCROLL_POS						CONSTLIT("scrollPos")
#define PROP_VIEWPORT_HEIGHT				CONSTLIT("viewportHeight")
#define PROP_SCALE							CONSTLIT("scale")
#define PROP_POSITION						CONSTLIT("position")

const Metric GAME_STAT_SELECTION_WIDTH =	480.0;
const Metric GAME_STAT_SELECTION_X =		-240.0;
const DWORD GAME_STAT_SELECTION_OPACITY =	128;
const int GAME_STAT_DURATION =				300;

const WORD RGB_MENU_TITLE =					CG16bitImage::RGBValue(0x80, 0x80, 0x80);

void CMenuSession::CreateMenuAnimation (const RECT &rcRect, IAnimatron **retpAni)

//	CreateMenuAnimation
//
//	Creates a menu animation by calling g_pCtrl

	{
	int i;

	m_iCount = g_pCtrl->GetMenuCount();
	m_iPos = -1;

	//	Get some resources

	const CVisualPalette &VI = g_pHI->GetVisuals();
	WORD wTextHighlightColor = VI.GetColor(colorTextHighlight);
	WORD wTextFadeColor = VI.GetColor(colorTextFade);
	const CG16bitFont &TitleFont = VI.GetFont(fontTitle);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);
	const CG16bitFont &HeaderFont = VI.GetFont(fontHeader);
	const CG16bitFont &LargeFont = VI.GetFont(fontLarge);

	//	Figure out the position

	int x = rcRect.left;
	int y = rcRect.top;
	int yEnd = rcRect.bottom - SubTitleFont.GetHeight();
	int cxWidth = rcRect.right - x;

	//	Adjust because scroller does not clip at the bottom

	int cyHeight = yEnd - y;

	//	Create a scrolling area and a list scroller

	CAniVScroller *pAni = new CAniVScroller;
	pAni->SetPropertyVector(CONSTLIT("position"), CVector((Metric)x, (Metric)y));
	pAni->SetPropertyMetric(CONSTLIT("viewportHeight"), (Metric)cyHeight);
	pAni->SetPropertyMetric(CONSTLIT("fadeEdgeHeight"), (Metric)(cyHeight / 8));

	CListScroller *pScroller = new CListScroller;
	pAni->AnimateProperty(PROP_SCROLL_POS, pScroller, 0, ID_GAME_STAT_SCROLLER);

	//	The first scroll position is the list at the bottom, offscreen

	pScroller->AddScrollPos(-cyHeight);

	//	Add a rect that we use to highlight selection.
	//	We add it first because it paints behind everything

	IAnimatron *pRect;
	CAniRect::Create(CVector(0.0, 0.0), CVector(0.0, 0.0), wTextFadeColor, 0, &pRect);
	pRect->SetID(ID_GAME_STAT_SELECT_RECT);
	pAni->AddLine(pRect);

	int xMargin = 2 * SubTitleFont.GetAverageWidth();

	//	Add the text

	for (i = 0; i < m_iCount; i++)
		{
		IAnimatron *pText;

		//	Get the stats

		CString sName = g_pCtrl->GetMenuItemLabel(i);

		//	Compute the vertical position of this line

		Metric yLine = pAni->GetHeight();

		//	Each scroll position places the line 1/2 of the way down

		pScroller->AddScrollPos(yLine - (cyHeight / 2), yLine);

		//	Add menu line

		CAniText::Create(sName,
				CVector(x + xMargin, yLine),
				&TitleFont,
				0,
				wTextHighlightColor,
				&pText);
		pAni->AddLine(pText);
		}

	//	The last position is the list scrolled off top

	pScroller->AddScrollPos(pAni->GetHeight());

	//	Initiate scroll from start to end

	pScroller->SetScrollToPos(-1, 1.0);

	//	Create animation that fades out the whole list after a while

	CLinearFade *pFader = new CLinearFade;
	pFader->SetParams(pScroller->GetDuration() + GAME_STAT_DURATION, 0, 30);
	pAni->AnimateProperty(PROP_OPACITY, pFader, 0, ID_GAME_STAT_FADER);

	//	Done

	*retpAni = pAni;
	}

void CMenuSession::OnCleanUp (void)

//	OnCleanUp

	{
	}

ALERROR CMenuSession::OnInit (void)

//	OnInit

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	//	Initialize some resources

	m_SuperTitleFont.Create(CONSTLIT("Lucida Sans"), -100);

	//	Get the widescreen dimensions

	int cxWidth = m_HI.GetScreen().GetWidth();
	int cyHeight = Min(m_HI.GetScreen().GetHeight(), cxWidth * 9 / 16);
	m_rcWidescreen.left = 0;
	m_rcWidescreen.right = cxWidth;
	m_rcWidescreen.top = (m_HI.GetScreen().GetHeight() - cyHeight) / 2;
	m_rcWidescreen.bottom = m_rcWidescreen.top + cyHeight;

	//	Start

	SetMenu();

	return NOERROR;
	}

void CMenuSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//	OnKeyDown

	{
	switch (iVirtKey)
		{
		case VK_DOWN:
		case VK_RIGHT:
			SetSelection(GAME_STAT_POSITION_NEXT);
			break;

		case VK_END:
			SetSelection(GAME_STAT_POSITION_END);
			break;

		case VK_HOME:
			SetSelection(GAME_STAT_POSITION_HOME);
			break;

		case VK_LEFT:
		case VK_UP:
			SetSelection(GAME_STAT_POSITION_PREV);
			break;

		case VK_NEXT:
			SetSelection(GAME_STAT_POSITION_NEXT_PAGE);
			break;

		case VK_PRIOR:
			SetSelection(GAME_STAT_POSITION_PREV_PAGE);
			break;

		case VK_RETURN:
			if (m_iPos != -1)
				{
				g_pCtrl->CmdSelectMenuItem(m_iPos - 1);

				StopAllPerformances();
				SetMenu();
				}
			break;

		case VK_ESCAPE:
			if (g_pCtrl->CmdEscape())
				{
				StopAllPerformances();
				SetMenu();
				}
			break;
		}
	}

void CMenuSession::OnLButtonDown (int x, int y, DWORD dwFlags)

//	OnLButtonDown

	{
	}

void CMenuSession::OnPaint (CG16bitImage &Screen, const RECT &rcInvalid)

//	OnPaint

	{
	Screen.Fill(m_rcWidescreen.left, m_rcWidescreen.top, RectWidth(m_rcWidescreen), RectHeight(m_rcWidescreen), 0);
	Screen.FillLine(m_rcWidescreen.left, m_rcWidescreen.top, RectWidth(m_rcWidescreen), CG16bitImage::RGBValue(0xC0, 0xC0, 0xC0));
	Screen.FillLine(m_rcWidescreen.left, m_rcWidescreen.bottom, RectWidth(m_rcWidescreen), CG16bitImage::RGBValue(0xC0, 0xC0, 0xC0));

	m_SuperTitleFont.DrawText(Screen,
			m_rcWidescreen.left,
			m_rcWidescreen.top,
			RGB_MENU_TITLE,
			g_pCtrl->GetMenuTitle());
	}

void CMenuSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash

	{
	}

void CMenuSession::OnUpdate (void)

//	OnUpdate

	{
	}

void CMenuSession::SelectMenuItem (IAnimatron *pAni, int iStatPos, int cxWidth, int iDuration)

//	SelectGameStat
//
//	Selects the next game stat

	{
	const CVisualPalette &VI = g_pHI->GetVisuals();
	int cySelection = VI.GetFont(fontTitle).GetHeight();

	//	Get the scroller

	CListScroller *pScroller;
	if (!pAni->FindAnimation(ID_GAME_STAT_SCROLLER, (IPropertyAnimator **)&pScroller))
		return;

	int iPos = pScroller->GetCurScrollPos();

	//	Handle special positions

	switch (iStatPos)
		{
		case GAME_STAT_POSITION_NEXT:
			iStatPos = iPos + 1;
			break;

		case GAME_STAT_POSITION_PREV:
			iStatPos = iPos - 1;
			break;

		case GAME_STAT_POSITION_NEXT_PAGE:
			iStatPos = iPos + 8;
			break;

		case GAME_STAT_POSITION_PREV_PAGE:
			iStatPos = iPos - 8;
			break;

		case GAME_STAT_POSITION_HOME:
			iStatPos = 1;
			break;

		case GAME_STAT_POSITION_END:
			iStatPos = pScroller->GetCount() - 2;
			break;
		}

	//	Make sure we're in bounds

	m_iPos = Max(1, Min(iStatPos, pScroller->GetCount() - 2));

	//	Set the scroll position

	pScroller->SetScrollToPos(m_iPos, -6.0);

	//	Fader

	CLinearFade *pFader;
	if (!pAni->FindAnimation(ID_GAME_STAT_FADER, (IPropertyAnimator **)&pFader))
		return;

	if (iDuration == -1)
		pFader->SetParams(-1, 0, 0);
	else
		pFader->SetParams(pScroller->GetDuration() + iDuration, 0, 30);

	//	Set the selection to the proper spot

	IAnimatron *pSelRect;
	if (pAni->FindElement(ID_GAME_STAT_SELECT_RECT, &pSelRect))
		{
		Metric yPos;
		pScroller->GetScrollPos(m_iPos, &yPos);

		pSelRect->SetPropertyVector(PROP_POSITION, CVector(0, yPos));
		pSelRect->SetPropertyVector(PROP_SCALE, CVector(cxWidth, cySelection));
		pSelRect->SetPropertyOpacity(PROP_OPACITY, GAME_STAT_SELECTION_OPACITY);
		}
	}

void CMenuSession::SetMenu (void)
	{
	RECT rcMenu = m_rcWidescreen;
	rcMenu.top += 100;
	rcMenu.bottom -= 100;
	rcMenu.right = RectWidth(m_rcWidescreen) / 3;

	IAnimatron *pAni;
	CreateMenuAnimation(rcMenu, &pAni);
	SelectMenuItem(pAni, 1, RectWidth(rcMenu), -1);
	StartPerformance(pAni, ID_GAME_STAT_PERFORMANCE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
	}

void CMenuSession::SetSelection (int iPos)
	{
	IAnimatron *pAni = GetPerformance(ID_GAME_STAT_PERFORMANCE);
	if (pAni == NULL)
		return;

	//	Set

	SelectMenuItem(pAni, iPos, RectWidth(m_rcWidescreen) / 3, -1);

	//	Restart animation

	StartPerformance(ID_GAME_STAT_PERFORMANCE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
	}
