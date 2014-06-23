//	CListSaveFilesTask.cpp
//
//	CListSaveFilesTask class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define ALIGN_CENTER							CONSTLIT("center")
#define ALIGN_RIGHT								CONSTLIT("right")

#define ERR_DIRECTORY_FAILED					CONSTLIT("Unable to get a file list.")

#define FILL_TYPE_NONE							CONSTLIT("none")

#define LINE_TYPE_SOLID							CONSTLIT("solid")

#define PROP_COLOR								CONSTLIT("color")
#define PROP_ENABLED							CONSTLIT("enabled")
#define PROP_FADE_EDGE_HEIGHT					CONSTLIT("fadeEdgeHeight")
#define PROP_FILL_TYPE							CONSTLIT("fillType")
#define PROP_FONT								CONSTLIT("font")
#define PROP_LINE_COLOR							CONSTLIT("lineColor")
#define PROP_LINE_PADDING						CONSTLIT("linePadding")
#define PROP_LINE_TYPE							CONSTLIT("lineType")
#define PROP_LINE_WIDTH							CONSTLIT("lineWidth")
#define PROP_LL_RADIUS							CONSTLIT("llRadius")
#define PROP_LR_RADIUS							CONSTLIT("lrRadius")
#define PROP_OPACITY							CONSTLIT("opacity")
#define PROP_PADDING_BOTTOM						CONSTLIT("paddingBottom")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_TEXT								CONSTLIT("text")
#define PROP_TEXT_ALIGN_HORZ					CONSTLIT("textAlignHorz")
#define PROP_UL_RADIUS							CONSTLIT("ulRadius")
#define PROP_UR_RADIUS							CONSTLIT("urRadius")

#define STYLE_SELECTION							CONSTLIT("selection")
#define STYLE_SELECTION_FOCUS					CONSTLIT("selectionFocus")

const int ADVENTURE_ICON_HEIGHT =				96;
const int ADVENTURE_ICON_WIDTH =				96;

const int SHIP_IMAGE_HEIGHT =					96;
const int SHIP_IMAGE_WIDTH =					96;

const int ICON_SPACING_HORZ =					8;
const int INTER_LINE_SPACING =					8;
const int MAJOR_PADDING_BOTTOM =				20;
const int MAJOR_PADDING_TOP =					20;

const int SELECTION_BORDER_WIDTH =				1;
const int SELECTION_CORNER_RADIUS =				8;

CListSaveFilesTask::CListSaveFilesTask (CHumanInterface &HI, const TArray<CString> &Folders, const CString &sUsername, int cxWidth) : IHITask(HI), 
		m_Folders(Folders),
		m_sUsername(sUsername),
		m_cxWidth(cxWidth),
		m_pList(NULL)

//	CListSaveFilesTask constructor

	{
	}

CListSaveFilesTask::~CListSaveFilesTask (void)

//	CListSaveFilesTask destructor

	{
	if (m_pList)
		delete m_pList;
	}

void CListSaveFilesTask::CreateFileEntry (CGameFile &GameFile, const CTimeDate &ModifiedTime, int yStart, IAnimatron **retpEntry, int *retcyHeight)

//	CreateFileEntry
//
//	Creates a display entry for the save file

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	int x = 0;
	int y = 0;
	int xText = x + ADVENTURE_ICON_WIDTH + ICON_SPACING_HORZ;
	int cxText = m_cxWidth - (ADVENTURE_ICON_WIDTH + 2 * ICON_SPACING_HORZ + SHIP_IMAGE_WIDTH);

	//	Start with a sequencer

	CAniSequencer *pRoot = new CAniSequencer;
	pRoot->SetPropertyVector(PROP_POSITION, CVector(0, yStart));

	//	Add the character name and current star system

	CString sHeading = strPatternSubst(CONSTLIT("%s — %s"), GameFile.GetPlayerName(), GameFile.GetSystemName());

	IAnimatron *pName = new CAniText;
	pName->SetPropertyVector(PROP_POSITION, CVector(xText, y));
	pName->SetPropertyVector(PROP_SCALE, CVector(10000, 1000));
	pName->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
	pName->SetPropertyFont(PROP_FONT, &SubTitleFont);
	pName->SetPropertyString(PROP_TEXT, sHeading);

	pRoot->AddTrack(pName, 0);
	y += SubTitleFont.GetHeight();

	//	Now add some additional information

	CShipClass *pClass = g_pUniverse->FindShipClass(GameFile.GetPlayerShip());
	CString sShipClass = (pClass ? pClass->GetName() : NULL_STR);
	CString sGenome = strCapitalize(GetGenomeName(GameFile.GetPlayerGenome()));

	CString sState;
	if (GameFile.IsGameResurrect())
		sState = strPatternSubst(CONSTLIT("Resurrect in the %s System"), GameFile.GetSystemName());
	else
		sState = strPatternSubst(CONSTLIT("Continue in the %s System"), GameFile.GetSystemName());

	CString sDesc;
	if (!sGenome.IsBlank() && !sShipClass.IsBlank())
		sDesc = strPatternSubst(CONSTLIT("%s — %s — %s"), sGenome, sShipClass, sState);
	else
		sDesc = sState;

	IAnimatron *pDesc = new CAniText;
	pDesc->SetPropertyVector(PROP_POSITION, CVector(xText, y));
	pDesc->SetPropertyVector(PROP_SCALE, CVector(cxText, 1000));
	pDesc->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
	pDesc->SetPropertyFont(PROP_FONT, &MediumFont);
	pDesc->SetPropertyString(PROP_TEXT, sDesc);

	RECT rcLine;
	pDesc->GetSpacingRect(&rcLine);

	pRoot->AddTrack(pDesc, 0);
	y += RectHeight(rcLine);

	//	Adventure info

	CExtension *pAdventure = NULL;
	bool bHasAdventureIcon = false;

	if (g_pUniverse->FindExtension(GameFile.GetAdventure(), 0, &pAdventure))
		{
		//	Adventure icon

		CG16bitImage *pIcon;
		pAdventure->CreateIcon(ADVENTURE_ICON_WIDTH, ADVENTURE_ICON_HEIGHT, &pIcon);

		if (pIcon)
			{
			int xOffset = (ADVENTURE_ICON_WIDTH - pIcon->GetWidth()) / 2;
			IAnimatron *pIconAni = new CAniRect;
			pIconAni->SetPropertyVector(PROP_POSITION, CVector(x + xOffset, 0));
			pIconAni->SetPropertyVector(PROP_SCALE, CVector(pIcon->GetWidth(), pIcon->GetHeight()));
			pIconAni->SetFillMethod(new CAniImageFill(pIcon, true));

			pRoot->AddTrack(pIconAni, 0);

			bHasAdventureIcon = true;
			}

		//	Adventure name

		pName = new CAniText;
		pName->SetPropertyVector(PROP_POSITION, CVector(xText, y));
		pName->SetPropertyVector(PROP_SCALE, CVector(10000, 1000));
		pName->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogLabel));
		pName->SetPropertyFont(PROP_FONT, &MediumFont);
		pName->SetPropertyString(PROP_TEXT, pAdventure->GetName());

		pRoot->AddTrack(pName, 0);
		y += MediumFont.GetHeight();
		}

	//	Create an image of the ship class

	if (pClass)
		{
		const CObjectImageArray &ObjImage = pClass->GetImage();
		if (ObjImage.IsLoaded())
			{
			RECT rcRect = ObjImage.GetImageRect();
			CG16bitImage &Image = ObjImage.GetImage(NULL_STR);
			int cxImage = RectWidth(rcRect);
			int cyImage = RectHeight(rcRect);

			int cxNewWidth = Min(SHIP_IMAGE_WIDTH, cxImage);
			int cyNewHeight = cxNewWidth;

			CG16bitImage *pNewImage = new CG16bitImage;
			pNewImage->CreateFromImageTransformed(Image, 
					rcRect.left, 
					rcRect.top, 
					cxImage,
					cyImage,
					(Metric)cxNewWidth / cxImage,
					(Metric)cyNewHeight / cyImage,
					0.0);

			//	Position

			int xImage = x + m_cxWidth - SHIP_IMAGE_WIDTH + (SHIP_IMAGE_WIDTH - cxNewWidth) / 2;
			int yImage = (SHIP_IMAGE_HEIGHT - cyNewHeight) / 2;

			//	New image frame

			IAnimatron *pImageFrame = new CAniRect;
			pImageFrame->SetPropertyVector(PROP_POSITION, CVector(xImage, yImage));
			pImageFrame->SetPropertyVector(PROP_SCALE, CVector(cxNewWidth, cyNewHeight));
			pImageFrame->SetFillMethod(new CAniImageFill(pNewImage, true));

			pRoot->AddTrack(pImageFrame, 0);
			}
		}

	//	Extra information

	CString sEpitaph = GameFile.GetEpitaph();
	int iScore = GameFile.GetScore();
	CTimeDate LocalTime = ModifiedTime.ToLocalTime();
	CString sModifiedTime = LocalTime.Format("%d %B %Y %I:%M %p");
	CString sFilename = pathGetFilename(GameFile.GetFilespec());

	CString sGameType;
	if (GameFile.IsRegistered())
		sGameType = CONSTLIT("Registered");
	else if (GameFile.IsDebug())
		sGameType = CONSTLIT("Debug");
	else
		sGameType = CONSTLIT("Unregistered");

	CString sExtra;
	if (!sEpitaph.IsBlank())
		sExtra = strPatternSubst(CONSTLIT("Score %d — %s\n%s — %s — %s"), iScore, sEpitaph, sGameType, sModifiedTime, sFilename);
	else if (iScore > 0)
		sExtra = strPatternSubst(CONSTLIT("Score %d\n%s — %s — %s"), iScore, sGameType, sModifiedTime, sFilename);
	else
		sExtra = strPatternSubst(CONSTLIT("%s — %s — %s"), sGameType, sModifiedTime, sFilename);

	pDesc = new CAniText;
	pDesc->SetPropertyVector(PROP_POSITION, CVector(xText, y));
	pDesc->SetPropertyVector(PROP_SCALE, CVector(cxText, 1000));
	pDesc->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogLabel));
	pDesc->SetPropertyFont(PROP_FONT, &MediumFont);
	pDesc->SetPropertyString(PROP_TEXT, sExtra);

	pDesc->GetSpacingRect(&rcLine);

	pRoot->AddTrack(pDesc, 0);
	y += RectHeight(rcLine);

	//	Done

	*retpEntry = pRoot;

	if (retcyHeight)
		*retcyHeight = (bHasAdventureIcon ? Max(ADVENTURE_ICON_HEIGHT, y) : y);
	}

ALERROR CListSaveFilesTask::OnExecute (ITaskProcessor *pProcessor, CString *retsResult)

//	OnExecute
//
//	Execute the task
	
	{
	int i;

	const CVisualPalette &VI = m_HI.GetVisuals();

	//	Make a list of all files in the directory

	TArray<CString> SaveFiles;
	bool bAtLeastOneFolder = false;
	for (i = 0; i < m_Folders.GetCount(); i++)
		{
		if (!fileGetFileList(m_Folders[i], NULL_STR, CONSTLIT("*.sav"), 0, &SaveFiles))
			::kernelDebugLogMessage("Unable to read from save file folder: %s", m_Folders[i]);
		else
			bAtLeastOneFolder = true;
		}

	//	If we couldn't read from any folder, return an error

	if (!bAtLeastOneFolder)
		{
		*retsResult = ERR_DIRECTORY_FAILED;
		return ERR_FAIL;
		}

	//	Sort by modified time (most recent first)

	TSortMap<CTimeDate, CString> SortedList(DescendingSort);
	for (i = 0; i < SaveFiles.GetCount(); i++)
		SortedList.Insert(fileGetModifiedTime(SaveFiles[i]), SaveFiles[i]);

	//	Generate a Reanimator list of the profile. The root will be a CAniListBox

	m_pList = new CAniListBox;
	m_pList->SetPropertyMetric(PROP_FADE_EDGE_HEIGHT, 0.0);
	m_pList->SetPropertyMetric(PROP_PADDING_BOTTOM, (Metric)MAJOR_PADDING_BOTTOM);

	//	Set the selection style for the list

	IAnimatron *pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, VI.GetColor(colorAreaDialogInputFocus));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, VI.GetColor(colorAreaDialogHighlight));
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, SELECTION_BORDER_WIDTH);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, SELECTION_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, SELECTION_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, SELECTION_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, SELECTION_CORNER_RADIUS);
	m_pList->SetStyle(STYLE_SELECTION_FOCUS, pStyle);

	pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, VI.GetColor(colorAreaDialogInputFocus));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, SELECTION_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, SELECTION_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, SELECTION_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, SELECTION_CORNER_RADIUS);
	m_pList->SetStyle(STYLE_SELECTION, pStyle);

	//	No need to log image load

	g_pUniverse->SetLogImageLoad(false);

	//	Loop over all files and add them to the scroller

	int y = MAJOR_PADDING_TOP;
	for (i = 0; i < SortedList.GetCount(); i++)
		{
		CString sFilename = SortedList[i];
		CGameFile GameFile;

		//	Ignore files that we can't open

		if (GameFile.Open(sFilename) != NOERROR)
			continue;

		//	If the universe is not valid, then this is not a proper save file
		//	(this can happen in the first system).

		if (!GameFile.IsUniverseValid())
			continue;

		//	If we're signed in, then we only show games for the given user
		//	(or unregistered games).

		if (GameFile.IsRegistered() && !strEquals(GameFile.GetUsername(), m_sUsername))
			continue;

		//	Generate a record for the file

		IAnimatron *pEntry;
		int cyHeight;
		CreateFileEntry(GameFile, SortedList.GetKey(i), y, &pEntry, &cyHeight);

		m_pList->AddEntry(sFilename, pEntry);
		y += cyHeight + INTER_LINE_SPACING;

		//	Done

		GameFile.Close();
		}

	g_pUniverse->SetLogImageLoad(true);

	//	Done

	return NOERROR;
	}
