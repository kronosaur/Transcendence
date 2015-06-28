//	CHighScoreDisplay.cpp
//
//	CHighScoreDisplay class
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define ID_HIGH_SCORES_ANIMATOR					CONSTLIT("hsAnimator")
#define ID_HIGH_SCORES_SELECT_RECT				CONSTLIT("hsSelRect")

#define PROP_FADE_EDGE_HEIGHT					CONSTLIT("fadeEdgeHeight")
#define PROP_MAX_SCROLL_POS						CONSTLIT("maxScrollPos")
#define PROP_OPACITY							CONSTLIT("opacity")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_SCROLL_POS							CONSTLIT("scrollPos")
#define PROP_VIEWPORT_HEIGHT					CONSTLIT("viewportHeight")

const int SCORE_COLUMN_WIDTH =					360;
const Metric SCORE_SPACING_Y =					4.0;
const Metric SCORE_SELECTION_WIDTH =			480.0;
const Metric SCORE_SELECTION_X =				-120.0;
const DWORD SCORE_SELECTION_OPACITY =			64;

const int HIGH_SCORES_DURATION =				300;

CHighScoreDisplay::CHighScoreDisplay (void) :
		m_pHighScoreList(NULL),
		m_pReanimator(NULL),
		m_pPerformance(NULL),
		m_dwPerformance(0),
		m_iSelection(-1)

//	CHighScoreDisplay constructor

	{
	}

CHighScoreDisplay::~CHighScoreDisplay (void)

//	CHighScoreDisplay destructor

	{
	if (m_pReanimator)
		m_pReanimator->DeletePerformance(m_dwPerformance);

	if (m_pHighScoreList)
		delete m_pHighScoreList;

	DeletePerformance();
	}

void CHighScoreDisplay::CreatePerformance (CReanimator &Reanimator, const CString &sPerformanceID, const RECT &rcRect, CAdventureHighScoreList *pHighScoreList, IAnimatron **retpAnimatron)

//	CreatePerformance
//
//	Creates a performance

	{
	int i, j;

	//	Get some resources

	const CVisualPalette &VI = g_pHI->GetVisuals();
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);
	const CG16bitFont &HeaderFont = VI.GetFont(fontHeader);
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);
	CG32bitPixel rgbLightTitleColor = VI.GetColor(colorTextFade);
	CG32bitPixel rgbTitleColor = VI.GetColor(colorTextHighlight);

	//	Figure out the position

	int x = rcRect.left + 2 * RectWidth(rcRect) / 3;
	int y = rcRect.top;
	int yEnd = rcRect.bottom - HeaderFont.GetHeight();
	int cyHeight = yEnd - y;

	//	We keep track of all the score positions in an array

	m_ScrollPos.DeleteAll();
	m_ScrollPos.InsertEmpty(pHighScoreList->GetCount() + 1);

	//	Create a scroller

	CAniVScroller *pAni = new CAniVScroller;
	pAni->SetPropertyVector(PROP_POSITION, CVector((Metric)x, (Metric)y));
	pAni->SetPropertyMetric(PROP_VIEWPORT_HEIGHT, (Metric)cyHeight);
	pAni->SetPropertyMetric(PROP_FADE_EDGE_HEIGHT, (Metric)(cyHeight / 8));

	//	Add a rect that we use to highlight selection.
	//	We add it first because it paints behind everything

	IAnimatron *pRect;
	CAniRect::Create(CVector(0.0, 0.0), CVector(100.0, 10.0), rgbLightTitleColor, 0, &pRect);
	pRect->SetID(ID_HIGH_SCORES_SELECT_RECT);
	pAni->AddLine(pRect);

	//	Loop over all scores

	for (i = 0; i < pHighScoreList->GetCount(); i++)
		{
		const CGameRecord &Score = pHighScoreList->GetEntry(i);

		//	Position

		Metric yPos = pAni->GetHeight() + (i > 0 ? SCORE_SPACING_Y : 0);
		m_ScrollPos[i] = (int)(yPos - (cyHeight / 3));

		//	Score

		IAnimatron *pText;
		CAniText::Create(strFormatInteger(Score.GetScore(), -1, FORMAT_THOUSAND_SEPARATOR),
				CVector((Metric)-SubTitleFont.GetAverageWidth(), yPos),
				&SubTitleFont,
				CG16bitFont::AlignRight,
				rgbTitleColor,
				&pText);
		pAni->AddLine(pText);

		//	Name

		CAniText::Create(Score.GetUsername(),
				CVector(0.0, yPos),
				&SubTitleFont,
				0,
				rgbTitleColor,
				&pText);
		pAni->AddLine(pText);

		//	Epitaph

		CString sEpitaph = Score.GetDescription(CGameRecord::descEpitaph);

		TArray<CString> Lines;
		MediumFont.BreakText(sEpitaph, SCORE_COLUMN_WIDTH, &Lines);

		for (j = 0; j < Lines.GetCount(); j++)
			{
			CAniText::Create(Lines[j],
					CVector(0.0, pAni->GetHeight()),
					&MediumFont,
					0,
					rgbTitleColor,
					&pText);
			pAni->AddLine(pText);
			}

		//	Some stats

		CString sStats = Score.GetDescription(CGameRecord::descShip | CGameRecord::descPlayTime | CGameRecord::descResurrectCount);
		Lines.DeleteAll();
		MediumFont.BreakText(sStats, SCORE_COLUMN_WIDTH, &Lines);

		for (j = 0; j < Lines.GetCount(); j++)
			{
			CAniText::Create(Lines[j],
					CVector(0.0, pAni->GetHeight()),
					&MediumFont,
					0,
					rgbLightTitleColor,
					&pText);
			pAni->AddLine(pText);
			}
		}

	//	Set the end pos

	m_ScrollPos[i] = (int)pAni->GetHeight() - (cyHeight / 3);

	//	Done

	*retpAnimatron = pAni;
	}

void CHighScoreDisplay::DeletePerformance (void)

//	DeletePerformance
//
//	Deletes the performance

	{
	if (IsPerformanceCreated())
		{
		m_pReanimator->StopPerformance(m_sPerformance);
		m_pReanimator->DeletePerformance(m_dwPerformance);

		m_pReanimator = NULL;
		m_dwPerformance = 0;
		m_pPerformance = NULL;
		}
	}

int CHighScoreDisplay::GetCurrentScrollPos (void)

//	GetCurrentScrollPos
//
//	Returns the score entry closest to the current scroll position.

	{
	int i;

	if (!IsPerformanceCreated())
		return -1;

	int yPos = (int)m_pPerformance->GetPropertyMetric(PROP_SCROLL_POS);

	for (i = 0; i < m_ScrollPos.GetCount() - 1; i++)
		if (m_ScrollPos[i] >= yPos)
			return Max(0, i);

	return -1;
	}

bool CHighScoreDisplay::IsPerformanceRunning (void)

//	IsPerformanceRunning
//
//	Returns TRUE if the performance is still running

	{
	if (!IsPerformanceCreated())
		return false;

	return m_pReanimator->IsPerformanceRunning(m_sPerformance);
	}

void CHighScoreDisplay::SelectNext (void)

//	SelectNext
//
//	Select the next entry in the high score list

	{
	if (!IsPerformanceCreated())
		return;

	if (m_iSelection == -1)
		{
		int iPos = GetCurrentScrollPos();
		if (iPos != -1)
			ScrollToPos(iPos);
		}
	else
		ScrollToPos(Min(m_iSelection + 1, m_pHighScoreList->GetCount() - 1));
	}

void CHighScoreDisplay::SelectPrev (void)

//	SelectPrev
//
//	Select the previous entry in the high score list

	{
	if (!IsPerformanceCreated())
		return;

	if (m_iSelection == -1)
		{
		int iPos = GetCurrentScrollPos();
		if (iPos != -1)
			ScrollToPos(iPos);
		}
	else
		ScrollToPos(Max(0, m_iSelection - 1));
	}

void CHighScoreDisplay::ScrollToPos (int iPos)

//	ScrollToPos
//
//	Scroll to the given entry

	{
	if (!IsPerformanceCreated() || iPos < 0 || iPos >= m_pHighScoreList->GetCount())
		return;

	const CVisualPalette &VI = g_pHI->GetVisuals();
	const CG16bitFont &HeaderFont = VI.GetFont(fontHeader);

	//	Get the current scroll position

	int yCurrent = (int)m_pPerformance->GetPropertyMetric(PROP_SCROLL_POS);
	int yDest = m_ScrollPos[iPos];

	//	Delete any current animation

	m_pPerformance->RemoveAnimation(ID_HIGH_SCORES_ANIMATOR);

	//	Create animation that goes from the current position
	//	to the desired position

	CLinearMetric *pScroller = new CLinearMetric;
	pScroller->SetParams(yCurrent, yDest, (yDest - yCurrent) / 16.0f);
	m_pPerformance->AnimateProperty(PROP_SCROLL_POS, pScroller, 0, ID_HIGH_SCORES_ANIMATOR);

	//	Create animation that fades out the whole list after a while

	CLinearFade *pFader = new CLinearFade;
	pFader->SetParams(pScroller->GetDuration() + HIGH_SCORES_DURATION, 0, 30);
	m_pPerformance->AnimateProperty(PROP_OPACITY, pFader, 0, ID_HIGH_SCORES_ANIMATOR);

	//	Set the selection to the proper spot

	IAnimatron *pSelRect;
	if (m_pPerformance->FindElement(ID_HIGH_SCORES_SELECT_RECT, &pSelRect))
		{
		int cyOffset = (m_rcScreen.bottom - HeaderFont.GetHeight() - m_rcScreen.top) / 3;

		pSelRect->SetPropertyVector(PROP_POSITION, CVector(SCORE_SELECTION_X, yDest + cyOffset));
		pSelRect->SetPropertyVector(PROP_SCALE, CVector(SCORE_SELECTION_WIDTH, m_ScrollPos[iPos + 1] - yDest));
		pSelRect->SetPropertyOpacity(PROP_OPACITY, SCORE_SELECTION_OPACITY);
		}

	//	Restart animation

	m_pReanimator->StartPerformance(m_dwPerformance);

	//	Remember the position

	m_iSelection = iPos;
	}

void CHighScoreDisplay::SetHighScoreList (CAdventureHighScoreList *pList)

//	SetHighScoreList
//
//	Set the list

	{
	if (m_pHighScoreList)
		delete m_pHighScoreList;

	m_pHighScoreList = pList;

	//	If we've got a new high score list, then we need to delete the 
	//	performance.

	DeletePerformance();
	}

bool CHighScoreDisplay::StartPerformance (CReanimator &Reanimator, const CString &sPerformanceID, const RECT &rcScreen)

//	StartPerformance
//
//	Starts the high score display performance. We return TRUE if we started it,
//	or false if we failed (for some reason).

	{
	//	If we don't have a high score list, then there's nothing we can do.

	if (m_pHighScoreList == NULL)
		return false;

	//	Make sure we have a performance

	if (!IsPerformanceCreated())
		{
		m_pReanimator = &Reanimator;
		m_sPerformance = sPerformanceID;
		m_rcScreen = rcScreen;

		CreatePerformance(Reanimator, sPerformanceID, rcScreen, m_pHighScoreList, &m_pPerformance);
		m_dwPerformance = Reanimator.AddPerformance(m_pPerformance, m_sPerformance);
		}

	//	If we've got a selection, then scroll to the position

	int iSelection = m_pHighScoreList->GetSelection();
	if (iSelection != -1)
		ScrollToPos(iSelection);

	//	Otherwise, we just scroll

	else
		{
		m_pPerformance->RemoveAnimation(ID_HIGH_SCORES_ANIMATOR);

		//	Scroller

		CLinearMetric *pScroller = new CLinearMetric;
		Metric cyViewport = m_pPerformance->GetPropertyMetric(PROP_VIEWPORT_HEIGHT);
		Metric cyHeight = m_pPerformance->GetPropertyMetric(PROP_MAX_SCROLL_POS);
		pScroller->SetParams(-cyViewport, cyHeight + cyViewport, 2.0);
		m_pPerformance->AnimateProperty(PROP_SCROLL_POS, pScroller, 0, ID_HIGH_SCORES_ANIMATOR);

		//	Clear the selection

		IAnimatron *pSelRect;
		if (m_pPerformance->FindElement(ID_HIGH_SCORES_SELECT_RECT, &pSelRect))
			pSelRect->SetPropertyOpacity(PROP_OPACITY, 0);

		//	Restart

		m_pReanimator->StartPerformance(m_dwPerformance);

		//	No selection

		m_iSelection = -1;
		}

	return true;
	}

void CHighScoreDisplay::StopPerformance (void)

//	StopPerformance
//
//	Stops the performance

	{
	if (IsPerformanceCreated())
		m_pReanimator->StopPerformance(m_sPerformance);
	}
