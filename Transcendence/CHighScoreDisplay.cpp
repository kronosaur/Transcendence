//	CHighScoreDisplay.cpp
//
//	CHighScoreDisplay class

#include "PreComp.h"
#include "Transcendence.h"

const int SCORE_COLUMN_WIDTH =				116;
const int FADE_TIME =						30;

CHighScoreDisplay::CHighScoreDisplay (void) : m_pList(NULL),
		m_iMode(modeNormal)

//	CHighScoreDisplay constructor

	{
	}

CHighScoreDisplay::~CHighScoreDisplay (void)

//	CHighScoreDisplay destructor

	{
	CleanUp();
	}

void CHighScoreDisplay::CleanUp (void)

//	CleanUp
//
//	Must be called to release resources

	{
	m_pList = NULL;

	//	NOTE: We only do this to save memory. The destructor will
	//	automatically free the memory.

	m_Buffer.Destroy();
	}

int CHighScoreDisplay::ComputeFirstEntry (int iHighlight)

//	ComputeFirstEntry
//
//	Computes the first entry. iHighlight is the entry that we
//	should highlight (-1 if no highlight)

	{
	ASSERT(m_pList);

	if (iHighlight == -1)
		return 0;
	else
		{
		if (iHighlight >= 2)
			return iHighlight - 2;
		else
			return 0;
		}
	}

void CHighScoreDisplay::FadeIn (void)

//	FadeIn
//
//	Set fade in

	{
	m_iMode = modeFadeIn;
	m_iCounter = FADE_TIME;
	}

void CHighScoreDisplay::FadeOut (void)

//	FadeIn
//
//	Set fade in

	{
	m_iMode = modeFadeOut;
	m_iCounter = FADE_TIME;
	}

ALERROR CHighScoreDisplay::Init (const RECT &rcRect, CHighScoreList *pList, int iHighlight)

//	Init
//
//	Must be called to initialize

	{
	ALERROR error;

	ASSERT(m_pList == NULL);
	m_pList = pList;
	m_rcRect = rcRect;
	m_iSelectedEntry = iHighlight;
	m_iFirstEntry = ComputeFirstEntry(m_iSelectedEntry);

	//	Create the off-screen buffer

	if (error = m_Buffer.CreateBlank(RectWidth(rcRect), RectHeight(rcRect), false))
		return error;

	m_Buffer.SetTransparentColor(CG16bitImage::RGBValue(0,0,0));

	//	Fade in

	FadeIn();

	return NOERROR;
	}

void CHighScoreDisplay::Paint (CG16bitImage &Dest)

//	Paint
//
//	Paints the display

	{
	ASSERT(m_pList);

	if (m_iMode == modeHidden)
		return;

	//	Opacity

	DWORD dwOpacity;
	if (m_iMode == modeFadeIn)
		dwOpacity = 255 * (FADE_TIME - m_iCounter) / FADE_TIME;
	else if (m_iMode == modeFadeOut)
		dwOpacity = 255 * m_iCounter / FADE_TIME;
	else
		dwOpacity = 255;

	//	If we have a selected entry, highlight the background

	if (m_iSelectedEntry != -1)
		{
		RECT rcHighlight = m_rcHighlight;
		::OffsetRect(&rcHighlight, m_rcRect.left, m_rcRect.top);

		Dest.FillTransRGB(rcHighlight.left,
				rcHighlight.top,
				RectWidth(rcHighlight),
				RectHeight(rcHighlight),
				RGB(255, 255, 255),
				32 * dwOpacity / 255);
		}

	//	Blt the scores

	Dest.ColorTransBlt(0,
			0,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			dwOpacity,
			m_Buffer,
			m_rcRect.left,
			m_rcRect.top);
	}

void CHighScoreDisplay::SelectNext (void)

//	SelectNext
//
//	Select the next score

	{
	if (m_iSelectedEntry + 1 < m_pList->GetCount())
		{
		m_iSelectedEntry++;
		m_iFirstEntry = ComputeFirstEntry(m_iSelectedEntry);
		}
	}

void CHighScoreDisplay::SelectPrevious (void)

//	SelectPrevious
//
//	Select the previous score

	{
	if (m_iSelectedEntry - 1 >= 0)
		{
		m_iSelectedEntry--;
		m_iFirstEntry = ComputeFirstEntry(m_iSelectedEntry);
		}
	}

void CHighScoreDisplay::Show (bool bShow)

//	Show
//
//	Shows/hides display

	{
	m_iMode = (bShow ? modeNormal : modeHidden);
	}

void CHighScoreDisplay::Update (void)

//	Update
//
//	Updates the display

	{
	ASSERT(m_pList);

	m_Buffer.Fill(0, 0, RectWidth(m_rcRect), RectHeight(m_rcRect), CG16bitImage::RGBValue(0, 0, 0));

	//	Don't bother drawing anything if we've got no scores

	if (m_pList->GetCount() == 0)
		return;

	//	Draw Title

	m_pFonts->SubTitle.DrawText(m_Buffer, 
			0, 
			0, 
			CG16bitImage::RGBValue(128,128,128), 
			CONSTLIT("High Score List"));

	int y = m_pFonts->SubTitle.GetHeight();

	//	Draw each score

	TArray<CString> Lines;
	for (int i = m_iFirstEntry; 
			(i < m_pList->GetCount()) && (y + m_pFonts->SubTitle.GetHeight() < RectHeight(m_rcRect)); 
			i++)
		{
		const CGameRecord &Entry = m_pList->GetEntry(i);

		if (i == m_iSelectedEntry)
			{
			m_rcHighlight.left = 0;
			m_rcHighlight.top = y;
			m_rcHighlight.right = RectWidth(m_rcRect);
			}

		CString sScore = strFromInt(Entry.GetScore(), false);
		int cxWidth = m_pFonts->SubTitle.MeasureText(sScore, NULL);
		m_pFonts->SubTitle.DrawText(m_Buffer,
				(SCORE_COLUMN_WIDTH - 10) - cxWidth,
				y,
				CG16bitImage::RGBValue(128,128,128),
				sScore);

		CString sName;
		if (Entry.IsDebug())
			sName = strPatternSubst(CONSTLIT("%d. %s [debug]"), i+1, Entry.GetPlayerName());
		else
			sName = strPatternSubst(CONSTLIT("%d. %s [%s]"), i+1, Entry.GetPlayerName(), Entry.GetShipClass());
		m_pFonts->Medium.DrawText(m_Buffer,
				SCORE_COLUMN_WIDTH,
				y,
				CG16bitImage::RGBValue(255,255,255),
				sName);

		CString sText = Entry.GetEndGameEpitaph();
		sText.Capitalize(CString::capFirstLetter);
		if (!Entry.GetPlayTimeString().IsBlank())
			sText.Append(strPatternSubst(CONSTLIT("\nPlayed for %s"), Entry.GetPlayTimeString()));

		if (Entry.GetResurrectCount() == 1)
			sText.Append(CONSTLIT(" (resurrected once)"));
		else if (Entry.GetResurrectCount() > 1)
			sText.Append(strPatternSubst(CONSTLIT(" (resurrected %d times)"), Entry.GetResurrectCount()));

		y += m_pFonts->Medium.GetHeight();
		Lines.DeleteAll();
		m_pFonts->Medium.BreakText(sText, RectWidth(m_rcRect) - SCORE_COLUMN_WIDTH, &Lines);
		for (int j = 0; j < Lines.GetCount(); j++)
			{
			m_pFonts->Medium.DrawText(m_Buffer,
					SCORE_COLUMN_WIDTH,
					y,
					CG16bitImage::RGBValue(128,128,128),
					Lines[j]);

			y += m_pFonts->Medium.GetHeight();
			}

		y += 2;

		if (i == m_iSelectedEntry)
			m_rcHighlight.bottom = y;
		}

	//	Update

	switch (m_iMode)
		{
		case modeFadeIn:
			if (--m_iCounter == 0)
				m_iMode = modeNormal;
			break;

		case modeFadeOut:
			if (--m_iCounter == 0)
				m_iMode = modeHidden;
			break;
		}
	}
