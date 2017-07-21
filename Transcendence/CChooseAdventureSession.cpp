//	CChooseAdventureSession.cpp
//
//	CChooseAdventureSession class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define ALIGN_CENTER							CONSTLIT("center")
#define ALIGN_RIGHT								CONSTLIT("right")

#define CMD_CLOSE_SESSION						CONSTLIT("cmdCloseSession")
#define CMD_EXTENSIONS_CHANGED					CONSTLIT("cmdExtensionsChanged")
#define CMD_OK_SESSION							CONSTLIT("cmdOKSession")
#define CMD_NEXT_ADVENTURE						CONSTLIT("cmdNextAdventure")
#define CMD_PREV_ADVENTURE						CONSTLIT("cmdPrevAdventure")
#define CMD_SELECT_ALL							CONSTLIT("cmdSelectAll")
#define CMD_DESELECT_ALL						CONSTLIT("cmdDeselectAll")
#define CMD_GAME_ADVENTURE						CONSTLIT("gameAdventure")
#define CMD_BACK_TO_INTRO						CONSTLIT("uiBackToIntro")

#define ID_CTRL_TITLE							CONSTLIT("ctrlTitle")
#define ID_EXTENSION_LIST						CONSTLIT("idExtensionList")
#define ID_MESSAGE								CONSTLIT("idMessage")
#define ID_ROOT									CONSTLIT("idRoot")
#define ID_STATUS								CONSTLIT("idStatus")
#define ID_TITLE_AND_DESC						CONSTLIT("idTitleAndDesc")

#define EVENT_ON_CLICK							CONSTLIT("onClick")

#define PROP_CHECKED							CONSTLIT("checked")
#define PROP_COLOR								CONSTLIT("color")
#define PROP_ENABLED							CONSTLIT("enabled")
#define PROP_FADE_EDGE_HEIGHT					CONSTLIT("fadeEdgeHeight")
#define PROP_FONT								CONSTLIT("font")
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
#define PROP_VIEWPORT_HEIGHT					CONSTLIT("viewportHeight")

const int INNER_PADDING_BOTTOM =				10;
const int INNER_PADDING_LEFT =					10;
const int INNER_PADDING_RIGHT =					10;
const int INNER_PADDING_TOP =					10;
const int MAJOR_PADDING_BOTTOM =				20;
const int MAJOR_PADDING_LEFT =					20;
const int MAJOR_PADDING_TOP =					20;
const int PANE_CORNER_RADIUS =					10;
const int PANE_WIDTH =							500;
const int EXTENSION_PANE_EXTRA =				120;
const int SMALL_BUTTON_HEIGHT =					48;
const int SMALL_BUTTON_WIDTH =					48;
const int SMALL_SPACING_HORZ =					8;

CChooseAdventureSession::CChooseAdventureSession (CHumanInterface &HI, CCloudService &Service, CGameSettings &Settings) : IHISession(HI), 
		m_Service(Service),
		m_Settings(Settings),
		m_pRoot(NULL),
		m_pBackground(NULL)

//	CChooseAdventureSession constructor

	{
	}

void CChooseAdventureSession::CmdCancel (void)

//	CmdCancel
//
//	Close the session.

	{
	m_HI.ClosePopupSession();
	}

void CChooseAdventureSession::CmdNextAdventure (void)

//	CmdNextAdventure
//
//	Select the next adventure

	{
	if (m_iSelection + 1 < m_AdventureList.GetCount())
		{
		SetAdventureDesc(m_AdventureList[++m_iSelection]);

		//	See if we need to disable the next button

		if (m_iSelection + 1 == m_AdventureList.GetCount())
			{
			IAnimatron *pNext = GetElement(CMD_NEXT_ADVENTURE);
			if (pNext)
				pNext->SetPropertyBool(PROP_ENABLED, false);
			}

		//	The prev button is always enabled after this

		IAnimatron *pPrev = GetElement(CMD_PREV_ADVENTURE);
		if (pPrev)
			pPrev->SetPropertyBool(PROP_ENABLED, true);
		}
	}

void CChooseAdventureSession::CmdOK (void)

//	CmdOK
//
//	User has accepted game settings.

	{
	int i;

	//	Prepare settings

	SAdventureSettings NewAdventure;
	NewAdventure.pAdventure = m_AdventureList[m_iSelection];

	//	Make a list of all extensions that are checked

	TArray<DWORD> Defaults;
	for (i = 0; i < m_ExtensionList.GetCount(); i++)
		{
		if (GetPropertyBool(strPatternSubst(CONSTLIT("idExtension:%d"), i), PROP_CHECKED))
			{
			NewAdventure.Extensions.Insert(m_ExtensionList[i]);
			Defaults.Insert(m_ExtensionList[i]->GetUNID());
			}
		}

	//	Remember the default extensions

	m_Settings.SetDefaultExtensions(NewAdventure.pAdventure->GetUNID(), m_ExtensionList, g_pUniverse->InDebugMode(), Defaults);

	//	Remember some variables because after we close the session this object
	//	will be gone.

	CHumanInterface &HI(m_HI);

	//	Close the session

	HI.ClosePopupSession();

	//	Send a command to the controller

	HI.HICommand(CMD_GAME_ADVENTURE, &NewAdventure);
	}

void CChooseAdventureSession::CmdPrevAdventure (void)

//	CmdPrevAdventure
//
//	Select the previous adventure

	{
	if (m_iSelection > 0)
		{
		SetAdventureDesc(m_AdventureList[--m_iSelection]);

		//	See if we need to disable the prev button

		if (m_iSelection == 0)
			{
			IAnimatron *pPrev = GetElement(CMD_PREV_ADVENTURE);
			if (pPrev)
				pPrev->SetPropertyBool(PROP_ENABLED, false);
			}

		//	The next button is always enabled after this

		IAnimatron *pNext = GetElement(CMD_NEXT_ADVENTURE);
		if (pNext)
			pNext->SetPropertyBool(PROP_ENABLED, true);
		}
	}

void CChooseAdventureSession::CmdSelectExtensions (bool bSelect)

//	CmdSelectExtension
//
//	Select or deselect all extensions

	{
	int i;

	for (i = 0; i < m_ExtensionList.GetCount(); i++)
		if (!m_ExtensionList[i]->IsDisabled())
			SetPropertyBool(strPatternSubst(CONSTLIT("idExtension:%d"), i), PROP_CHECKED, bSelect);

	SetAdventureStatus(m_AdventureList[m_iSelection], m_yBottomSection);
	}

void CChooseAdventureSession::CreateAdventureDesc (CExtension *pAdventure)

//	CreateAdventureDesc
//
//	Creates a description of the adventure

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumBoldFont = VI.GetFont(fontMediumBold);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	//	Create buttons to select the adventure at the top of the region

	int cxWidth = RectWidth(m_rcPane);
	int xCenter = cxWidth / 2;
	int xHalfSpacing = SMALL_SPACING_HORZ / 2;

	IAnimatron *pLeftButton;
	VI.CreateImageButtonSmall(NULL, 
			CMD_PREV_ADVENTURE, 
			xCenter - xHalfSpacing - SMALL_BUTTON_WIDTH, 
			MAJOR_PADDING_TOP,
			&VI.GetImage(imageSmallLeftIcon),
			0,
			&pLeftButton);

	if (m_iSelection == 0)
		pLeftButton->SetPropertyBool(PROP_ENABLED, false);

	RegisterPerformanceEvent(pLeftButton, EVENT_ON_CLICK, CMD_PREV_ADVENTURE);
	m_pRoot->AddLine(pLeftButton);

	IAnimatron *pRightButton;
	VI.CreateImageButtonSmall(NULL, 
			CMD_NEXT_ADVENTURE, 
			xCenter + xHalfSpacing, 
			MAJOR_PADDING_TOP,
			&VI.GetImage(imageSmallRightIcon),
			0,
			&pRightButton);

	if (m_iSelection + 1 == m_AdventureList.GetCount())
		pRightButton->SetPropertyBool(PROP_ENABLED, false);

	RegisterPerformanceEvent(pRightButton, EVENT_ON_CLICK, CMD_NEXT_ADVENTURE);
	m_pRoot->AddLine(pRightButton);

	//	Select

	SetAdventureDesc(pAdventure);
	}

bool CChooseAdventureSession::FindAdventurePos (DWORD dwUNID, int *retiPos)

//	FindAdventurePos
//
//	Looks for the position of the given adventure (by extension UNID). If not
//	found, we return FALSE.

	{
	int i;

	for (i = 0; i < m_AdventureList.GetCount(); i++)
		if (m_AdventureList[i]->GetUNID() == dwUNID)
			{
			if (retiPos)
				*retiPos = i;

			return true;
			}

	return false;
	}

ALERROR CChooseAdventureSession::OnCommand (const CString &sCmd, void *pData)

//	OnCommand
//
//	Handle a command

	{
	if (strEquals(sCmd, CMD_CLOSE_SESSION))
		CmdCancel();
	else if (strEquals(sCmd, CMD_NEXT_ADVENTURE))
		CmdNextAdventure();
	else if (strEquals(sCmd, CMD_PREV_ADVENTURE))
		CmdPrevAdventure();
	else if (strEquals(sCmd, CMD_OK_SESSION))
		CmdOK();
	else if (strEquals(sCmd, CMD_EXTENSIONS_CHANGED))
		SetAdventureStatus(m_AdventureList[m_iSelection], m_yBottomSection);
	else if (strEquals(sCmd, CMD_SELECT_ALL))
		CmdSelectExtensions(true);
	else if (strEquals(sCmd, CMD_DESELECT_ALL))
		CmdSelectExtensions(false);

	return NOERROR;
	}

ALERROR CChooseAdventureSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	//	Compute some metrics

	RECT rcCenter;
	VI.GetWidescreenRect(&rcCenter, &m_rcFull);

	int cxPane = Min(RectWidth(rcCenter) / 2, PANE_WIDTH);
	int xCenterLine = rcCenter.right - (RectWidth(rcCenter) / 4);
	m_rcPane = rcCenter;
	m_rcPane.left = xCenterLine - (cxPane / 2);
	m_rcPane.right = m_rcPane.left + cxPane;

	//	Make a list of all adventures

	g_pUniverse->GetAllAdventures(&m_AdventureList);

	//	Start at the first adventure

	if (!FindAdventurePos(m_Settings.GetInteger(CGameSettings::lastAdventure), &m_iSelection))
		m_iSelection = 0;

	//	Set up a menu

	TArray<CUIHelper::SMenuEntry> Menu;
	CUIHelper::SMenuEntry *pEntry = Menu.Insert();
	pEntry->sCommand = CMD_SELECT_ALL;
	pEntry->sLabel = CONSTLIT("Select all extensions");

	pEntry = Menu.Insert();
	pEntry->sCommand = CMD_DESELECT_ALL;
	pEntry->sLabel = CONSTLIT("Deselect all extensions");

	//	Create the title

	CUIHelper Helper(m_HI);
	IAnimatron *pTitle;
	Helper.CreateSessionTitle(this, m_Service, CONSTLIT("New Game"), &Menu, CUIHelper::OPTION_SESSION_OK_BUTTON, &pTitle);
	StartPerformance(pTitle, ID_CTRL_TITLE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Create a scroller to hold all the settings

	m_pRoot = new CAniVScroller;
	m_pRoot->SetPropertyVector(PROP_POSITION, CVector(m_rcPane.left, m_rcPane.top));
	m_pRoot->SetPropertyMetric(PROP_VIEWPORT_HEIGHT, (Metric)RectHeight(m_rcPane));
	m_pRoot->SetPropertyMetric(PROP_FADE_EDGE_HEIGHT, 0.0);
	m_pRoot->SetPropertyMetric(PROP_PADDING_BOTTOM, (Metric)MAJOR_PADDING_BOTTOM);

	//	Create the adventure description

	CreateAdventureDesc(m_AdventureList[m_iSelection]);

	//	Start the root pane

	StartPerformance(m_pRoot, ID_ROOT, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Set the adventure status again because we can't access the extension
	//	checkboxes until the performance starts.

	SetAdventureStatus(m_AdventureList[m_iSelection], m_yBottomSection);

	//	Done

	return NOERROR;
	}

void CChooseAdventureSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//	OnKeyDown
//
//	KeyDown

	{
	switch (iVirtKey)
		{
		case VK_ESCAPE:
			CmdCancel();
			break;

		case VK_LEFT:
			CmdPrevAdventure();
			break;

		case VK_RIGHT:
			CmdNextAdventure();
			break;

		case VK_RETURN:
			CmdOK();
			break;

		default:
			HandlePageScrollKeyDown(ID_ROOT, iVirtKey, dwKeyData);
			break;
		}
	}

void CChooseAdventureSession::OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags)

//	OnMouseWheel
//
//	Handle mouse wheel

	{
	HandlePageScrollMouseWheel(ID_ROOT, iDelta);
	}

void CChooseAdventureSession::OnPaint (CG32bitImage &Screen, const RECT &rcInvalid)

//	OnPaint
//
//	Paint

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	RECT rcCenter;
	VI.DrawSessionBackground(Screen, CG32bitImage(), CG32bitPixel(), CVisualPalette::OPTION_SESSION_DLG_BACKGROUND, &rcCenter);

	//	Paint the background image

	if (m_pBackground)
		Screen.Blt(0,
				0,
				RectWidth(m_rcBackground),
				RectHeight(m_rcBackground),
				*m_pBackground,
				m_rcBackground.left,
				m_rcBackground.top);
	}

void CChooseAdventureSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash
//
//	Describe current state

	{
	*retsMessage = CONSTLIT("session: CChooseAdventureSession\r\n");
	}

void CChooseAdventureSession::SetAdventureDesc (CExtension *pAdventure)

//	SetAdventureDesc
//
//	Sets the adventure

	{
	//	Create the background image. This initializes m_pBackground and 
	//	m_rcBackground.

	SetBackgroundImage(pAdventure);

	//	Create the title and description

	SetAdventureTitle(pAdventure, &m_yBottomSection);
	m_yBottomSection += MAJOR_PADDING_BOTTOM;

	//	Set all extensions

	SetExtensions(pAdventure, m_yBottomSection);

	//	Set registered game text

	SetAdventureStatus(pAdventure, m_yBottomSection);

	//	If we have no extensions then disable the menu items

	bool bDisable = (m_ExtensionList.GetCount() == 0);
	SetPropertyBool(CMD_SELECT_ALL, PROP_ENABLED, !bDisable);
	SetPropertyBool(CMD_DESELECT_ALL, PROP_ENABLED, !bDisable);

	//	If this adventure is disabled, then disable the OK button

	SetPropertyBool(CMD_OK_SESSION, PROP_ENABLED, !pAdventure->IsDisabled());
	}

void CChooseAdventureSession::SetAdventureStatus (CExtension *pAdventure, int yPos)

//	SetAdventureStatus
//
//	Sets the adventure status

	{
	int i;

	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);
	const CG16bitFont &MediumBoldFont = VI.GetFont(fontMediumBold);

	bool bRegistered = !g_pUniverse->InDebugMode();
	if (bRegistered)
		{
		//	Make a list of all extensions

		TArray<CExtension *> Extensions;
		for (i = 0; i < m_ExtensionList.GetCount(); i++)
			{
			if (GetPropertyBool(strPatternSubst(CONSTLIT("idExtension:%d"), i), PROP_CHECKED))
				Extensions.Insert(m_ExtensionList[i]);
			}

		//	See if the game can be registered

		if (!g_pUniverse->GetExtensionCollection().IsRegisteredGame(pAdventure, Extensions, 0))
			bRegistered = false;
		}

	//	Generate status

	CString sStatus;
	if (g_pUniverse->InDebugMode())
		sStatus = CONSTLIT("debug game");
	else if (bRegistered)
		sStatus = CONSTLIT("registered game");
	else
		sStatus = CONSTLIT("unregistered game");

	//	Create a sequencer to hold all the animations

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(0, yPos), &pRoot);
	pRoot->SetID(ID_STATUS);

	//	Label

	int x = 0;
	int y = INNER_PADDING_BOTTOM;
	int cxMaxWidth = (RectWidth(m_rcPane) / 2) - MAJOR_PADDING_LEFT;

	IAnimatron *pLabel = new CAniText;
	pLabel->SetPropertyVector(PROP_POSITION, CVector(x, y));
	pLabel->SetPropertyVector(PROP_SCALE, CVector(cxMaxWidth, 100.0));
	pLabel->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
	pLabel->SetPropertyFont(PROP_FONT, &MediumBoldFont);
	pLabel->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_RIGHT);
	pLabel->SetPropertyString(PROP_TEXT, sStatus);

	pRoot->AddTrack(pLabel, 0);
	y += MediumBoldFont.GetHeight() + INNER_PADDING_TOP;

	//	Adventure filespec

	IAnimatron *pFilespec = new CAniText;
	pFilespec->SetPropertyVector(PROP_POSITION, CVector(x, y));
	pFilespec->SetPropertyVector(PROP_SCALE, CVector(cxMaxWidth, 100.0));
	pFilespec->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogLabel));
	pFilespec->SetPropertyFont(PROP_FONT, &MediumFont);
	pFilespec->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_RIGHT);
	pFilespec->SetPropertyString(PROP_TEXT, pAdventure->GetFilespec());

	pRoot->AddTrack(pFilespec, 0);

	RECT rcRect;
	pFilespec->GetSpacingRect(&rcRect);
	y += RectHeight(rcRect);

	//	Release number

	if (pAdventure->GetRelease() > 0)
		{
		IAnimatron *pRelease = new CAniText;
		pRelease->SetPropertyVector(PROP_POSITION, CVector(x, y));
		pRelease->SetPropertyVector(PROP_SCALE, CVector(cxMaxWidth, 100.0));
		pRelease->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogLabel));
		pRelease->SetPropertyFont(PROP_FONT, &MediumFont);
		pRelease->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_RIGHT);
		pRelease->SetPropertyString(PROP_TEXT, strPatternSubst(CONSTLIT("Release %d"), pAdventure->GetRelease()));

		pRoot->AddTrack(pRelease, 0);
		y += MediumFont.GetHeight();
		}
	
	//	Delete the original frame

	DeleteElement(ID_STATUS);

	//	Add to pane

	m_pRoot->AddLine(pRoot);
	}

void CChooseAdventureSession::SetAdventureTitle (CExtension *pAdventure, int *rety)

//	SetAdventureTitle
//
//	Sets the adventure

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	//	Create a sequencer to hold all the animations

	int yFrame = MAJOR_PADDING_TOP + SMALL_BUTTON_HEIGHT + INNER_PADDING_BOTTOM;

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(0, yFrame), &pRoot);
	pRoot->SetID(ID_TITLE_AND_DESC);

	//	Create the background rect

	CAniRoundedRect *pFrame = new CAniRoundedRect;
	pFrame->SetPropertyVector(PROP_POSITION, CVector(0, 0));
	pFrame->SetPropertyColor(PROP_COLOR, VI.GetColor(colorAreaDeep));
	pFrame->SetPropertyOpacity(PROP_OPACITY, 128);
	pFrame->SetPropertyInteger(PROP_UL_RADIUS, PANE_CORNER_RADIUS);
	pFrame->SetPropertyInteger(PROP_UR_RADIUS, PANE_CORNER_RADIUS);
	pFrame->SetPropertyInteger(PROP_LL_RADIUS, PANE_CORNER_RADIUS);
	pFrame->SetPropertyInteger(PROP_LR_RADIUS, PANE_CORNER_RADIUS);

	pRoot->AddTrack(pFrame, 0);

	//	Add the title of the adventure

	int y = INNER_PADDING_TOP;
	int cxInner = RectWidth(m_rcPane) - (INNER_PADDING_LEFT + INNER_PADDING_RIGHT);

	IAnimatron *pTitle = new CAniText;
	pTitle->SetPropertyVector(PROP_POSITION, CVector(INNER_PADDING_LEFT, y));
	pTitle->SetPropertyVector(PROP_SCALE, CVector(cxInner, 100));
	pTitle->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogTitle));
	pTitle->SetPropertyFont(PROP_FONT, &SubTitleFont);
	pTitle->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_CENTER);
	pTitle->SetPropertyString(PROP_TEXT, pAdventure->GetName());

	pRoot->AddTrack(pTitle, 0);

	RECT rcRect;
	pTitle->GetSpacingRect(&rcRect);
	y += RectHeight(rcRect) + INNER_PADDING_TOP;

	//	Add desc

	IAnimatron *pText = new CAniRichText(VI);
	pText->SetPropertyVector(PROP_POSITION, CVector(INNER_PADDING_LEFT, y));
	pText->SetPropertyVector(PROP_SCALE, CVector(cxInner, RectHeight(m_rcPane)));
	pText->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
	pText->SetPropertyFont(PROP_FONT, &MediumFont);
	pText->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_CENTER);

	//	If the adventure is disabled, for some reason, we show that reason.

	if (pAdventure->IsDisabled())
		pText->SetPropertyString(PROP_TEXT, strPatternSubst(CONSTLIT("{/rtf %s }"), pAdventure->GetDisabledReason()));
	else
		pText->SetPropertyString(PROP_TEXT, pAdventure->GetDesc());

	pRoot->AddTrack(pText, 0);

	pText->GetSpacingRect(&rcRect);
	y += RectHeight(rcRect);

	//	Now that we know the size of the text, size the frame

	pFrame->SetPropertyVector(PROP_SCALE, CVector(RectWidth(m_rcPane), y + INNER_PADDING_BOTTOM));

	//	Delete the original frame

	DeleteElement(ID_TITLE_AND_DESC);

	//	Add to global root

	m_pRoot->AddLine(pRoot);

	//	Return the y position

	*rety = yFrame + y + INNER_PADDING_BOTTOM;
	}

void CChooseAdventureSession::SetBackgroundImage (CExtension *pAdventure)

//	SetBackgroundImage
//
//	Creates a background image.

	{
	//	Free the previous image

	if (m_pBackground)
		{
		delete m_pBackground;
		m_pBackground = NULL;
		}

	//	Get the cover image from the adventure. If no cover image then we have
	//	nothing to do.

	CG32bitImage *pCover = pAdventure->GetCoverImage();
	if (pCover == NULL)
		return;

	//	Get some metrics of the screen

	const CVisualPalette &VI = m_HI.GetVisuals();

	RECT rcCenter;
	RECT rcFull;
	VI.GetWidescreenRect(&rcCenter, &rcFull);

	int xRightEdge = rcCenter.right - (3 * RectWidth(rcCenter) / 8);

	//	Scale the cover image so that it fits vertically in the pane

	Metric rScale;
	int cxFinalWidth;
	int yCrop;
	int cyCrop;
	if (pCover->GetHeight() < RectHeight(rcCenter))
		{
		rScale = (Metric)RectHeight(rcCenter) / pCover->GetHeight();
		cxFinalWidth = (int)(pCover->GetWidth() * rScale);
		yCrop = 0;
		cyCrop = pCover->GetHeight();
		}
	else
		{
		rScale = 1.0;
		cxFinalWidth = pCover->GetWidth();
		cyCrop = RectHeight(rcCenter);
		yCrop = (pCover->GetHeight() - cyCrop) / 4;
		}

	//	Compute the size and position of the resulting image.

	int xCenter = (rcCenter.left + RectWidth(rcCenter) / 2);

	m_rcBackground = rcCenter;
	if (cxFinalWidth > (xRightEdge - rcCenter.left))
		{
		m_rcBackground.right = xRightEdge;
		m_rcBackground.left = m_rcBackground.right - cxFinalWidth;
		}
	else
		{
		m_rcBackground.left = Min(xCenter - (2 * cxFinalWidth) / 3, xRightEdge - cxFinalWidth);
		m_rcBackground.right = m_rcBackground.left + cxFinalWidth;
		}

	//	Create a new image

	m_pBackground = new CG32bitImage;
	m_pBackground->CreateFromImageTransformed(*pCover,
			0,
			yCrop,
			pCover->GetWidth(),
			cyCrop,
			rScale,
			rScale,
			0.0);

	//	Draw a gradient over the image to fade into the background

	if (m_rcBackground.right > xCenter)
		{
		int xCenterAdj = xCenter + 120;
		int xStart = m_pBackground->GetWidth() - (m_rcBackground.right - xCenterAdj);
		CGDraw::RectGradient(*m_pBackground,
				xStart,
				0,
				(xRightEdge - xCenterAdj),
				m_pBackground->GetHeight(),
				CG32bitPixel(VI.GetColor(colorAreaDialog), 0),
				VI.GetColor(colorAreaDialog),
				gradientHorizontal);
		}
	}

void CChooseAdventureSession::SetExtensions (CExtension *pAdventure, int yPos)

//	SetExtensions
//
//	Set the extensions objects

	{
	int i;

	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumBoldFont = VI.GetFont(fontMediumBold);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);
	CString sError;

	//	Make a list of extensions valid for the given adventure

	if (g_pUniverse->GetExtensionCollection().ComputeAvailableExtensions(pAdventure, 
			(g_pUniverse->InDebugMode() ? (CExtensionCollection::FLAG_DEBUG_MODE | CExtensionCollection::FLAG_INCLUDE_AUTO) : 0),
			TArray<DWORD>(),
			&m_ExtensionList,
			&sError) != NOERROR)
		//	LATER: Show error
		return;

	//	If we don't have any extensions, then delete the section

	if (m_ExtensionList.GetCount() == 0)
		{
		DeleteElement(ID_EXTENSION_LIST);
		return;
		}

	//	Create a sequencer to hold all the animations

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(0, yPos), &pRoot);
	pRoot->SetID(ID_EXTENSION_LIST);

	//	Label

	int x = (RectWidth(m_rcPane) / 2) + INNER_PADDING_LEFT;
	int y = INNER_PADDING_BOTTOM;
	int cxMaxWidth = (m_rcFull.right - INNER_PADDING_LEFT - (m_rcPane.left + x));

	IAnimatron *pLabel = new CAniText;
	pLabel->SetPropertyVector(PROP_POSITION, CVector(x, y));
	pLabel->SetPropertyVector(PROP_SCALE, CVector(cxMaxWidth, 100.0));
	pLabel->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
	pLabel->SetPropertyFont(PROP_FONT, &MediumBoldFont);
	pLabel->SetPropertyString(PROP_TEXT, CONSTLIT("extensions"));

	pRoot->AddTrack(pLabel, 0);
	y += MediumBoldFont.GetHeight() + INNER_PADDING_TOP;

	//	Get the default list of options

	TArray<DWORD> Defaults;
	m_Settings.GetDefaultExtensions(pAdventure->GetUNID(), m_ExtensionList, g_pUniverse->InDebugMode(), &Defaults);

	//	Add all extension options

	int cySpacing = INNER_PADDING_BOTTOM / 2;

	for (i = 0; i < m_ExtensionList.GetCount(); i++)
		{
		int cyLine;
		bool bDisabled = m_ExtensionList[i]->IsDisabled();

		//	Label

		CString sLabel;
		if (bDisabled)
			sLabel = strPatternSubst(CONSTLIT("%s [%s]"), m_ExtensionList[i]->GetName(), m_ExtensionList[i]->GetDisabledReason());
		else if (g_pUniverse->InDebugMode() && !m_ExtensionList[i]->GetVersion().IsBlank())
			sLabel = strPatternSubst(CONSTLIT("%s [%s]"), m_ExtensionList[i]->GetName(), m_ExtensionList[i]->GetVersion());
		else
			sLabel = m_ExtensionList[i]->GetName();

		//	Create a button

		IAnimatron *pButton;
		VI.CreateCheckbox(pRoot,
				strPatternSubst(CONSTLIT("idExtension:%d"), i),
				x,
				y,
				cxMaxWidth,
				CVisualPalette::OPTION_CHECKBOX_LARGE_FONT,
				sLabel,
				&pButton,
				&cyLine);

		if (bDisabled)
			pButton->SetPropertyBool(PROP_ENABLED, false);
		else if (Defaults.Find(m_ExtensionList[i]->GetUNID()))
			pButton->SetPropertyBool(PROP_CHECKED, true);

		//	Add an event when clicked so that we can recompute game 
		//	registration.

		RegisterPerformanceEvent(pButton, EVENT_ON_CLICK, CMD_EXTENSIONS_CHANGED);

		//	Next

		y += cyLine + cySpacing;
		}

	//	Delete the original frame

	DeleteElement(ID_EXTENSION_LIST);

	//	Add to pane

	m_pRoot->AddLine(pRoot);
	}
