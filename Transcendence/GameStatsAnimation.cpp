//	GameStatsAnimation.cpp
//
//	GameStats animation functions

#include "PreComp.h"
#include "Transcendence.h"

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
const DWORD GAME_STAT_SELECTION_OPACITY =	64;
const int GAME_STAT_DURATION =				300;

void CopyGameStatsToClipboard (HWND hWnd, const CGameStats &GameStats)

//	CopyGameStatsToClipboard
//
//	Copy the game stats to the clipboard

	{
	CMemoryWriteStream Output;

	if (Output.Create() != NOERROR)
		return;

	GameStats.WriteAsText(&Output);

	//	Terminate

	Output.Write("\0", 1);

	//	Copy

	uiCopyTextToClipboard(hWnd, CString(Output.GetPointer(), Output.GetLength(), TRUE));
	Output.Close();
	}

void CreateGameStatsAnimation (const CGameStats &GameStats, 
							   const RECT rcRect, 
							   int xCenterLine,
							   IAnimatron **retpAni)

//	CreateGameStatsAnimation
//
//	Create animation showing all the stats

	{
	int i;

	const CString &sDefaultSectionName = GameStats.GetDefaultSectionName();

	//	Get some resources

	const CVisualPalette &VI = g_pHI->GetVisuals();
	CG32bitPixel rgbTextHighlightColor = VI.GetColor(colorTextHighlight);
	CG32bitPixel rgbTextFadeColor = VI.GetColor(colorTextFade);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);
	const CG16bitFont &HeaderFont = VI.GetFont(fontHeader);
	const CG16bitFont &LargeFont = VI.GetFont(fontLarge);

	//	Figure out the position

	int x = xCenterLine;
	int y = rcRect.top;
	int yEnd = rcRect.bottom - HeaderFont.GetHeight();
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
	CAniRect::Create(CVector(0.0, 0.0), CVector(0.0, 0.0), rgbTextFadeColor, 0, &pRect);
	pRect->SetID(ID_GAME_STAT_SELECT_RECT);
	pAni->AddLine(pRect);

	//	Some offsets

	int yNameOffset = HeaderFont.GetAscent() - LargeFont.GetAscent();

	//	Add the text

	CString sPrevSection = CONSTLIT("(start)");
	for (i = 0; i < GameStats.GetCount(); i++)
		{
		IAnimatron *pText;

		//	Get the stats

		CString sName;
		CString sValue;
		CString sSection;
		GameStats.GetEntry(i, &sName, &sValue, &sSection);

		//	Compute the vertical position of this line

		Metric yLine = pAni->GetHeight();

		//	If necessary, add a new section header

		if (!strEquals(sSection, sPrevSection))
			{
			if (i != 0)
				yLine += SubTitleFont.GetHeight();

			CString sSectionTitle;
			if (sSection.IsBlank())
				sSectionTitle = strToUpper(sDefaultSectionName);
			else
				sSectionTitle = strToLower(sSection);

			CAniText::Create(sSectionTitle,
					CVector(0.0, yLine),
					&SubTitleFont,
					CG16bitFont::AlignCenter,
					(sSection.IsBlank() ? rgbTextHighlightColor : rgbTextFadeColor),
					&pText);
			pAni->AddLine(pText);

			yLine += 2 * SubTitleFont.GetHeight();
			sPrevSection = sSection;
			}

		//	Each scroll position places the line 1/3 of the way down

		pScroller->AddScrollPos(yLine - (cyHeight / 3), yLine);

		//	Right-justify the name

		Metric xSpacing = 10.0;
		Metric xName = (sValue.IsBlank() ? -(LargeFont.MeasureText(sName) / 2) : -(LargeFont.MeasureText(sName) + xSpacing));

		//	Add two text lines

		CAniText::Create(sName,
				CVector(xName, yLine + yNameOffset),
				&LargeFont,
				0,
				rgbTextHighlightColor,
				&pText);
		pAni->AddLine(pText);

		if (!sValue.IsBlank())
			{
			//	Wrap, if necessary

			TArray<CString> Lines;
			HeaderFont.BreakText(sValue, (rcRect.right - xCenterLine) - 2 * (int)xSpacing, &Lines);

			for (int j = 0; j < Lines.GetCount(); j++)
				{
				CAniText::Create(Lines[j],
						CVector(xSpacing, yLine + j * HeaderFont.GetHeight()),
						&HeaderFont,
						0,
						rgbTextFadeColor,
						&pText);

				pAni->AddLine(pText);
				}
			}
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

void SelectGameStat (IAnimatron *pAni, int iStatPos, int cxWidth, int iDuration)

//	SelectGameStat
//
//	Selects the next game stat

	{
	const CVisualPalette &VI = g_pHI->GetVisuals();
	int cySelection = VI.GetFont(fontHeader).GetHeight();

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

	int iNewPos = Max(1, Min(iStatPos, pScroller->GetCount() - 2));

	//	Set the scroll position

	pScroller->SetScrollToPos(iNewPos, -6.0);

	//	Fader

	CLinearFade *pFader;
	if (!pAni->FindAnimation(ID_GAME_STAT_FADER, (IPropertyAnimator **)&pFader))
		return;

	if (iDuration == durationInfinite)
		pFader->SetParams(durationInfinite, 0, 0);
	else
		pFader->SetParams(pScroller->GetDuration() + iDuration, 0, 30);

	//	Set the selection to the proper spot

	IAnimatron *pSelRect;
	if (pAni->FindElement(ID_GAME_STAT_SELECT_RECT, &pSelRect))
		{
		Metric yPos;
		pScroller->GetScrollPos(iNewPos, &yPos);

		pSelRect->SetPropertyVector(PROP_POSITION, CVector(-cxWidth / 2, yPos));
		pSelRect->SetPropertyVector(PROP_SCALE, CVector(cxWidth, cySelection));
		pSelRect->SetPropertyOpacity(PROP_OPACITY, GAME_STAT_SELECTION_OPACITY);
		}
	}

