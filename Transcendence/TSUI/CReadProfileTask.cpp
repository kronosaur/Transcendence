//	CReadProfileTask.cpp
//
//	CReadProfileTask class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define PROP_COLOR								CONSTLIT("color")
#define PROP_ENABLED							CONSTLIT("enabled")
#define PROP_FADE_EDGE_HEIGHT					CONSTLIT("fadeEdgeHeight")
#define PROP_FONT								CONSTLIT("font")
#define PROP_LL_RADIUS							CONSTLIT("llRadius")
#define PROP_LR_RADIUS							CONSTLIT("lrRadius")
#define PROP_PADDING_BOTTOM						CONSTLIT("paddingBottom")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_TEXT								CONSTLIT("text")
#define PROP_TEXT_ALIGN_HORZ					CONSTLIT("textAlignHorz")
#define PROP_UL_RADIUS							CONSTLIT("ulRadius")
#define PROP_UR_RADIUS							CONSTLIT("urRadius")

#define ALIGN_CENTER							CONSTLIT("center")
#define ALIGN_RIGHT								CONSTLIT("right")

const int FRAME_RADIUS =						10;
const int ICON_AREA_HEIGHT =					240;
const int ICON_AREA_WIDTH =						154;
const int PADDING_LEFT =						8;
const int PADDING_RIGHT =						8;
const int PADDING_TOP =							8;
const int PADDING_BOTTOM =						16;
const int MAJOR_PADDING_BOTTOM =				20;
const int MAJOR_PADDING_TOP =					20;
const int TITLE_BAR_HEIGHT =					24;
const int SCORE_AREA_WIDTH =					320;

const int MAX_HIGH_SCORES =						10;

const WORD RGB_MAIN_DARK =						CG16bitImage::RGBValue(27, 27, 27);
const WORD RGB_MAIN_LIGHT =						CG16bitImage::RGBValue(38, 38, 38);
const WORD RGB_TITLE_BAR_BACK =					CG16bitImage::RGBValue(77, 77, 77);

#define STR_NO_GAMES_ON_RECORD					CONSTLIT("No games on record")
#define STR_NO_GAMES_DESC						CONSTLIT("You have not yet completed any games.")

struct SEpitaphDesc
	{
	CString sEpitaph;
	int cyHeight;
	};

CReadProfileTask::CReadProfileTask (CHumanInterface &HI, CCloudService &Service, int cxWidth) : IHITask(HI), 
		m_Service(Service),
		m_cxWidth(cxWidth),
		m_pList(NULL)

//	CReadProfileTask constructor

	{
	}

CReadProfileTask::~CReadProfileTask (void)

//	CReadProfileTask destructor

	{
	if (m_pList)
		delete m_pList;
	}

void CReadProfileTask::CreateAdventureRecordDisplay (CAdventureRecord &Record, int yStart, IAnimatron **retpAni)

//	CreateAdventureRecordDisplay
//
//	Creates an animation displaying the adventure record

	{
	int i;

	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);
	const CG16bitFont &MediumBoldFont = VI.GetFont(fontMediumBold);
	const CG16bitFont &LargeFont = VI.GetFont(fontLarge);
	const CG16bitFont &HeaderFont = VI.GetFont(fontHeader);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	//	Compute the width of the center area

	int cxCenterArea = m_cxWidth - (ICON_AREA_WIDTH + SCORE_AREA_WIDTH);
	int cxCenterContent = cxCenterArea - (PADDING_LEFT + PADDING_RIGHT);

	//	Get a list of all score epitaphs (these are the only variable height
	//	components, so we need to get them to figure out out size).

	int cyHeight = TITLE_BAR_HEIGHT + PADDING_TOP;
	TArray<SEpitaphDesc> Epitaphs;
	for (i = 0; i < CAdventureRecord::specialIDCount; i++)
		{
		CGameRecord &GameRecord = Record.GetRecordAt(CAdventureRecord::specialIDFirst + i);
		if (GameRecord.GetScore())
			{
			SEpitaphDesc *pEntry = Epitaphs.Insert();

			pEntry->sEpitaph = GameRecord.GetDescription(CGameRecord::descAll);
			pEntry->cyHeight = MediumFont.GetHeight() * MediumFont.BreakText(pEntry->sEpitaph, cxCenterContent, NULL);

			//	Measure

			cyHeight += MediumBoldFont.GetHeight();
			cyHeight += SubTitleFont.GetHeight();
			cyHeight += pEntry->cyHeight;

			cyHeight += PADDING_TOP;
			}
		}

	cyHeight = Max(cyHeight, TITLE_BAR_HEIGHT + ICON_AREA_HEIGHT);

	int cyHighScoresHeight = TITLE_BAR_HEIGHT + PADDING_TOP + MediumBoldFont.GetHeight() + MAX_HIGH_SCORES * HeaderFont.GetHeight() + PADDING_BOTTOM;
	cyHeight = Max(cyHeight, cyHighScoresHeight);

	int cyBody = cyHeight - TITLE_BAR_HEIGHT;

	//	Get the adventure desc.
	//
	//	NOTE: The Universe structures are not thread-safe, but we can guarantee
	//	that this (background) task only runs while	the foreground is showing 
	//	the CProfileSession. Thus there should be no other thread accessing the 
	//	Universe.
	//
	//	We need to make sure the CProfileSession does not exit until all
	//	background tasks are completed (or cancelled).

	CExtension *pAdventure;
	CString sAdventureTitle;
	CG16bitImage *pIcon;
	if (g_pUniverse->FindExtension(Record.GetAdventureUNID(), 0, &pAdventure))
		{
		sAdventureTitle = pAdventure->GetName();
		pAdventure->CreateIcon(ICON_AREA_WIDTH, cyBody, &pIcon);
		}
	else
		{
		pAdventure = NULL;
		sAdventureTitle = strPatternSubst(CONSTLIT("Adventure %x"), Record.GetAdventureUNID());
		pIcon = NULL;
		}

	//	Start with a sequencer

	CAniSequencer *pRoot = new CAniSequencer;
	pRoot->SetPropertyVector(PROP_POSITION, CVector(0, yStart));

	//	Start with a title bar

	IAnimatron *pTitleBar = new CAniRoundedRect;
	pTitleBar->SetPropertyVector(PROP_POSITION, CVector());
	pTitleBar->SetPropertyVector(PROP_SCALE, CVector(m_cxWidth, TITLE_BAR_HEIGHT));
	pTitleBar->SetPropertyColor(PROP_COLOR, RGB_TITLE_BAR_BACK);
	pTitleBar->SetPropertyInteger(PROP_UL_RADIUS, 0);
	pTitleBar->SetPropertyInteger(PROP_UR_RADIUS, FRAME_RADIUS);
	pTitleBar->SetPropertyInteger(PROP_LL_RADIUS, 0);
	pTitleBar->SetPropertyInteger(PROP_LR_RADIUS, 0);
	pRoot->AddTrack(pTitleBar, 0);

	//	Adventure name

	IAnimatron *pTitle = new CAniText;
	pTitle->SetPropertyVector(PROP_POSITION, CVector(PADDING_LEFT, (TITLE_BAR_HEIGHT - LargeFont.GetHeight()) / 2));
	pTitle->SetPropertyVector(PROP_SCALE, CVector(m_cxWidth, TITLE_BAR_HEIGHT));
	pTitle->SetPropertyFont(PROP_FONT, &LargeFont);
	pTitle->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
	pTitle->SetPropertyString(PROP_TEXT, sAdventureTitle);
	pRoot->AddTrack(pTitle, 0);

	//	Create the background body

	IAnimatron *pIconArea = new CAniRect;
	pIconArea->SetPropertyVector(PROP_POSITION, CVector(0, TITLE_BAR_HEIGHT));
	pIconArea->SetPropertyVector(PROP_SCALE, CVector(ICON_AREA_WIDTH, cyBody));
	pIconArea->SetPropertyColor(PROP_COLOR, RGB_MAIN_DARK);
	pRoot->AddTrack(pIconArea, 0);

	IAnimatron *pCenterArea = new CAniRect;
	pCenterArea->SetPropertyVector(PROP_POSITION, CVector(ICON_AREA_WIDTH, TITLE_BAR_HEIGHT));
	pCenterArea->SetPropertyVector(PROP_SCALE, CVector(cxCenterArea, cyBody));
	pCenterArea->SetPropertyColor(PROP_COLOR, RGB_MAIN_LIGHT);
	pRoot->AddTrack(pCenterArea, 0);

	IAnimatron *pScoreArea = new CAniRoundedRect;
	pScoreArea->SetPropertyVector(PROP_POSITION, CVector(m_cxWidth - SCORE_AREA_WIDTH, TITLE_BAR_HEIGHT));
	pScoreArea->SetPropertyVector(PROP_SCALE, CVector(SCORE_AREA_WIDTH, cyBody));
	pScoreArea->SetPropertyColor(PROP_COLOR, RGB_MAIN_DARK);
	pScoreArea->SetPropertyInteger(PROP_UL_RADIUS, 0);
	pScoreArea->SetPropertyInteger(PROP_UR_RADIUS, 0);
	pScoreArea->SetPropertyInteger(PROP_LL_RADIUS, 0);
	pScoreArea->SetPropertyInteger(PROP_LR_RADIUS, FRAME_RADIUS);
	pRoot->AddTrack(pScoreArea, 0);

	//	Adventure icon

	if (pIcon)
		{
		IAnimatron *pIconAni = new CAniRect;
		pIconAni->SetPropertyVector(PROP_POSITION, CVector(0, TITLE_BAR_HEIGHT));
		pIconAni->SetPropertyVector(PROP_SCALE, CVector(pIcon->GetWidth(), pIcon->GetHeight()));
		pIconAni->SetFillMethod(new CAniImageFill(pIcon, true));

		pRoot->AddTrack(pIconAni, 0);
		}

	//	Create the appropriate records

	int x = ICON_AREA_WIDTH + PADDING_LEFT;
	int y = TITLE_BAR_HEIGHT + PADDING_TOP;
	int iEpitaph = 0;
	for (i = 0; i < CAdventureRecord::specialIDCount; i++)
		{
		CGameRecord &GameRecord = Record.GetRecordAt(CAdventureRecord::specialIDFirst + i);
		if (GameRecord.GetScore())
			{
			//	Title of the record

			IAnimatron *pRecordTitle = new CAniText;
			pRecordTitle->SetPropertyVector(PROP_POSITION, CVector(x, y));
			pRecordTitle->SetPropertyVector(PROP_SCALE, CVector(cxCenterContent, cyBody));
			pRecordTitle->SetPropertyColor(PROP_COLOR, VI.GetColor(colorAreaDialogHighlight));
			pRecordTitle->SetPropertyFont(PROP_FONT, &MediumBoldFont);
			switch (CAdventureRecord::specialIDFirst + i)
				{
				case CAdventureRecord::personalBest:
					pRecordTitle->SetPropertyString(PROP_TEXT, CONSTLIT("Personal Best"));
					break;

				case CAdventureRecord::mostRecent:
					pRecordTitle->SetPropertyString(PROP_TEXT, CONSTLIT("Most Recent"));
					break;

				default:
					pRecordTitle->SetPropertyString(PROP_TEXT, CONSTLIT("Other"));
					break;
				}

			pRoot->AddTrack(pRecordTitle, 0);
			y += MediumBoldFont.GetHeight();

			//	Username who holds the record + score

			CString sScoreAndUser = strPatternSubst(CONSTLIT("%s %s"), strFormatInteger(GameRecord.GetScore(), -1, FORMAT_THOUSAND_SEPARATOR), GameRecord.GetUsername());

			IAnimatron *pName = new CAniText;
			pName->SetPropertyVector(PROP_POSITION, CVector(x, y));
			pName->SetPropertyVector(PROP_SCALE, CVector(cxCenterContent, cyBody));
			pName->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
			pName->SetPropertyFont(PROP_FONT, &SubTitleFont);
			pName->SetPropertyString(PROP_TEXT, sScoreAndUser);

			pRoot->AddTrack(pName, 0);
			y += SubTitleFont.GetHeight();

			//	Epitaph

			IAnimatron *pEpitaph = new CAniText;
			pEpitaph->SetPropertyVector(PROP_POSITION, CVector(x, y));
			pEpitaph->SetPropertyVector(PROP_SCALE, CVector(cxCenterContent, cyBody));
			pEpitaph->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogLabel));
			pEpitaph->SetPropertyFont(PROP_FONT, &MediumFont);
			pEpitaph->SetPropertyString(PROP_TEXT, Epitaphs[iEpitaph].sEpitaph);

			pRoot->AddTrack(pEpitaph, 0);
			y += Epitaphs[iEpitaph].cyHeight;

			iEpitaph++;

			//	Next

			y += PADDING_TOP;
			}
		}

	//	Create the high scores

	x = m_cxWidth - SCORE_AREA_WIDTH + PADDING_LEFT;
	y = TITLE_BAR_HEIGHT + PADDING_TOP;

	//	Area title

	pTitle = new CAniText;
	pTitle->SetPropertyVector(PROP_POSITION, CVector(x, y));
	pTitle->SetPropertyVector(PROP_SCALE, CVector(SCORE_AREA_WIDTH - PADDING_LEFT, cyBody));
	pTitle->SetPropertyColor(PROP_COLOR, VI.GetColor(colorAreaDialogHighlight));
	pTitle->SetPropertyFont(PROP_FONT, &MediumBoldFont);
	pTitle->SetPropertyString(PROP_TEXT, CONSTLIT("High Scores"));

	pRoot->AddTrack(pTitle, 0);
	y += MediumBoldFont.GetHeight();

	int cxScore = PADDING_LEFT + HeaderFont.GetAverageWidth() * 10;
	int xName = x + cxScore + PADDING_RIGHT;

	//	Add all the high scores

	int iCount = Min(MAX_HIGH_SCORES, Record.GetHighScoreCount());
	for (i = 0; i < iCount; i++)
		{
		CGameRecord &GameRecord = Record.GetRecordAt(i);

		//	Score

		IAnimatron *pScore = new CAniText;
		pScore->SetPropertyVector(PROP_POSITION, CVector(x, y));
		pScore->SetPropertyVector(PROP_SCALE, CVector(cxScore, cyBody));
		pScore->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
		pScore->SetPropertyFont(PROP_FONT, &HeaderFont);
		pScore->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_RIGHT);
		pScore->SetPropertyString(PROP_TEXT, strFormatInteger(GameRecord.GetScore(), -1, FORMAT_THOUSAND_SEPARATOR));

		pRoot->AddTrack(pScore, 0);

		//	Name

		IAnimatron *pName = new CAniText;
		pName->SetPropertyVector(PROP_POSITION, CVector(xName, y));
		pName->SetPropertyVector(PROP_SCALE, CVector(SCORE_AREA_WIDTH - (cxScore + 2 * PADDING_LEFT), cyBody));
		pName->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
		pName->SetPropertyFont(PROP_FONT, &HeaderFont);
		pName->SetPropertyString(PROP_TEXT, GameRecord.GetUsername());

		pRoot->AddTrack(pName, 0);

		//	Next

		y += HeaderFont.GetHeight();
		}

	//	Done

	*retpAni = pRoot;
	}

ALERROR CReadProfileTask::OnExecute (ITaskProcessor *pProcessor, CString *retsResult)

//	OnExecute
//
//	Execute the task
	
	{
	ALERROR error;
	int i;
	const CVisualPalette &VI = m_HI.GetVisuals();

	//	Get the profile data

	if (error = m_Service.ReadProfile(pProcessor, &m_Profile, retsResult))
		return error;

	//	Sort the adventure records by name

	TSortMap<CString, int> SortedList;
	for (i = 0; i < m_Profile.GetAdventureRecordCount(); i++)
		SortedList.Insert(m_Profile.GetAdventureRecord(i).GetAdventureName(), i);

	//	Generate a Reanimator list of the profile. The root will be a VScroller

	m_pList = new CAniVScroller;
	m_pList->SetPropertyMetric(PROP_FADE_EDGE_HEIGHT, 0.0);
	m_pList->SetPropertyMetric(PROP_PADDING_BOTTOM, (Metric)MAJOR_PADDING_BOTTOM);

	//	If no adventures, then we just add a notice

	if (SortedList.GetCount() == 0)
		{
		RECT rcRect;
		rcRect.left = 0;
		rcRect.right = m_cxWidth;
		rcRect.top = 0;
		rcRect.bottom = 512;

		IAnimatron *pMsg;
		VI.CreateMessagePane(NULL, NULL_STR, STR_NO_GAMES_ON_RECORD, STR_NO_GAMES_DESC, rcRect, 0, &pMsg);

		m_pList->AddLine(pMsg);
		}

	//	Otherwise, add all the adventure records

	else
		{
		//	Loop over all adventure records and add them to the scroller

		int y = MAJOR_PADDING_TOP;
		for (i = 0; i < SortedList.GetCount(); i++)
			{
			//	Add a record for this adventure

			IAnimatron *pEntry;
			CreateAdventureRecordDisplay(m_Profile.GetAdventureRecord(SortedList[i]), y, &pEntry);

			m_pList->AddLine(pEntry);

			//	Figure out the size

			RECT rcSize;
			pEntry->GetSpacingRect(&rcSize);

			y += RectHeight(rcSize) + 2 * PADDING_TOP;
			}
		}

	//	Done

	return NOERROR;
	}
