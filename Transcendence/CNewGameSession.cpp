//	CNewGameSession.cpp
//
//	CNewGameSession class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define ALIGN_CENTER							CONSTLIT("center")
#define ALIGN_RIGHT								CONSTLIT("right")

#define CMD_CHANGE_GENOME						CONSTLIT("cmdChangeGenome")
#define CMD_CLOSE_SESSION						CONSTLIT("cmdCloseSession")
#define CMD_EDIT_NAME							CONSTLIT("cmdEditName")
#define CMD_NEXT_SHIP_CLASS						CONSTLIT("cmdNextShipClass")
#define CMD_OK_SESSION							CONSTLIT("cmdOKSession")
#define CMD_PREV_SHIP_CLASS						CONSTLIT("cmdPrevShipClass")
#define CMD_GAME_CREATE							CONSTLIT("gameCreate")
#define CMD_BACK_TO_INTRO						CONSTLIT("uiBackToIntro")

#define ID_CTRL_TITLE							CONSTLIT("ctrlTitle")
#define ID_SECTION_PLAYER_GENOME				CONSTLIT("sectionPlayerGenome")
#define ID_SECTION_PLAYER_NAME					CONSTLIT("sectionPlayerName")
#define ID_PLAYER_GENOME						CONSTLIT("idPlayerGenome")
#define ID_PLAYER_NAME							CONSTLIT("idPlayerName")
#define ID_PLAYER_NAME_FIELD					CONSTLIT("idPlayerNameField")
#define ID_SETTINGS								CONSTLIT("idSettings")
#define ID_SHIP_CLASS_DESC						CONSTLIT("idShipClassDesc")
#define ID_SHIP_CLASS_IMAGE						CONSTLIT("idShipClassImage")
#define ID_SHIP_CLASS_INFO						CONSTLIT("idShipClassInfo")
#define ID_SHIP_CLASS_NAME						CONSTLIT("idShipClassName")

#define ERR_NO_ADVENTURE						CONSTLIT("No adventure defined")
#define ERR_NO_SHIP_CLASSES						CONSTLIT("Unable to find starting ship classes that match adventure criteria");

#define EVENT_ON_CLICK							CONSTLIT("onClick")

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
#define PROP_VIEWPORT_HEIGHT					CONSTLIT("viewportHeight")

#define STYLE_IMAGE								CONSTLIT("image")

#define STR_GENOME								CONSTLIT("gender")

const int ICON_HEIGHT =							48;
const int ICON_WIDTH =							48;
const int ITEM_INFO_PADDING_VERT =				16;
const int MAJOR_PADDING_BOTTOM =				20;
const int MAJOR_PADDING_HORZ =					20;
const int MAJOR_PADDING_TOP =					20;
const int MAJOR_PADDING_VERT =					20;
const int NAME_FIELD_WIDTH =					320;
const int SHIP_IMAGE_HEIGHT =					320;
const int SHIP_IMAGE_WIDTH =					320;
const int SMALL_BUTTON_HEIGHT =					48;
const int SMALL_BUTTON_WIDTH =					48;
const int SMALL_SPACING_HORZ =					8;
const int SMALL_SPACING_VERT =					8;

const int SHIP_IMAGE_RECT_HEIGHT =				SHIP_IMAGE_HEIGHT + MAJOR_PADDING_VERT;
const int SHIP_IMAGE_RECT_WIDTH =				SHIP_IMAGE_WIDTH + MAJOR_PADDING_HORZ;

const int SPECIAL_ARMOR =						100;
const int SPECIAL_REACTOR =						101;
const int SPECIAL_DRIVE =						102;
const int SPECIAL_CARGO =						103;
const int SPECIAL_DEVICE_SLOTS =				104;

CNewGameSession::CNewGameSession (CHumanInterface &HI, CCloudService &Service, const SNewGameSettings &Defaults) : IHISession(HI),
		m_Service(Service),
		m_Settings(Defaults)

//	CNewGameSession constructor

	{
	}

void CNewGameSession::AddClassInfo (CShipClass *pClass, const CDeviceDescList &Devices, const CItem &Item, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpAni)

//	AddClassInfo
//
//	Creates a new class info item animation

	{
	CUIHelper Helper(m_HI);

	//	For special item info, such as reactors, drives, etc., we encode a
	//	special code in the item count.

	switch (Item.GetCount())
		{
		case SPECIAL_ARMOR:
			Helper.CreateClassInfoArmor(pClass, x, y, cxWidth, dwOptions, retcyHeight, retpAni);
			break;

		case SPECIAL_CARGO:
			Helper.CreateClassInfoCargo(pClass, Devices, x, y, cxWidth, dwOptions, retcyHeight, retpAni);
			break;

		case SPECIAL_DEVICE_SLOTS:
			Helper.CreateClassInfoDeviceSlots(pClass, Devices, x, y, cxWidth, dwOptions, retcyHeight, retpAni);
			break;

		case SPECIAL_DRIVE:
			Helper.CreateClassInfoDrive(pClass, Devices, x, y, cxWidth, dwOptions, retcyHeight, retpAni);
			break;

		case SPECIAL_REACTOR:
			Helper.CreateClassInfoReactor(pClass, Devices, x, y, cxWidth, dwOptions, retcyHeight, retpAni);
			break;

		default:
			Helper.CreateClassInfoItem(Item, x, y, cxWidth, dwOptions, NULL_STR, retcyHeight, retpAni);
		}
	}

void CNewGameSession::CmdCancel (void)

//	CmdCancel
//
//	Cancel new game

	{
	//	Remember some variables because after we close the session this object
	//	will be gone.

	CHumanInterface &HI(m_HI);

	//	New game

	HI.HICommand(CMD_BACK_TO_INTRO);
	}

void CNewGameSession::CmdChangeGenome (void)

//	CmdChangeGenom
//
//	Change genome

	{
	if (m_Settings.iPlayerGenome == genomeHumanMale)
		m_Settings.iPlayerGenome = genomeHumanFemale;
	else
		m_Settings.iPlayerGenome = genomeHumanMale;

	SetPlayerGenome(m_Settings.iPlayerGenome, m_xPlayerGenome, m_yPlayerGenome, m_cxPlayerGenome);
	}

void CNewGameSession::CmdEditName (void)

//	CmdEditName
//
//	Handles the edit name button

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	if (m_bEditingName)
		{
		IAnimatron *pEdit;
		if (m_pRoot->FindElement(ID_PLAYER_NAME_FIELD, &pEdit))
			{
			m_Settings.sPlayerName = CUniverse::ValidatePlayerName(pEdit->GetPropertyString(PROP_TEXT));
			m_Settings.bDefaultPlayerName = false;

			DeleteElement(ID_PLAYER_NAME_FIELD);
			}

		SetPlayerName(m_Settings.sPlayerName, m_xPlayerName, m_yPlayerName, m_cxPlayerName);
		m_bEditingName = false;
		}

	//	If we're not editing, then start editing

	else
		{
		DeleteElement(ID_PLAYER_NAME);

		//	Create edit control

		IAnimatron *pEdit;
		VI.CreateEditControl(NULL,
				ID_PLAYER_NAME_FIELD,
				m_xPlayerName + SMALL_BUTTON_WIDTH + MAJOR_PADDING_HORZ,
				m_yPlayerName,
				NAME_FIELD_WIDTH,
				0,
				NULL_STR,
				&pEdit,
				NULL);

		pEdit->SetPropertyString(PROP_TEXT, m_Settings.sPlayerName);

		m_pRoot->AddLine(pEdit);

		SetInputFocus(ID_PLAYER_NAME_FIELD);

		m_bEditingName = true;
		}
	}

void CNewGameSession::CmdEditNameCancel (void)

//	CmdEditNameCancel
//
//	Cancel editing name

	{
	if (m_bEditingName)
		{
		DeleteElement(ID_PLAYER_NAME_FIELD);
		SetPlayerName(m_Settings.sPlayerName, m_xPlayerName, m_yPlayerName, m_cxPlayerName);
		m_bEditingName = false;
		}
	}

void CNewGameSession::CmdNextShipClass (void)

//	CmdNextShipClass
//
//	Select the next ship class

	{
	if (m_iCurShipClass + 1 < m_ShipClasses.GetCount())
		{
		SetShipClass(m_ShipClasses[++m_iCurShipClass], m_xShipClass, m_yShipClass, m_cxShipClass);

		//	See if we need to disable the next button

		if (m_iCurShipClass + 1 == m_ShipClasses.GetCount())
			{
			IAnimatron *pNext = GetElement(CMD_NEXT_SHIP_CLASS);
			if (pNext)
				pNext->SetPropertyBool(PROP_ENABLED, false);
			}

		//	The prev button is always enabled after this

		IAnimatron *pPrev = GetElement(CMD_PREV_SHIP_CLASS);
		if (pPrev)
			pPrev->SetPropertyBool(PROP_ENABLED, true);
		}
	}

void CNewGameSession::CmdOK (void)

//	CmdOK
//
//	User has accepted game settings.

	{
	//	Prepare settings

	SNewGameSettings NewGame;
	NewGame.sPlayerName = m_Settings.sPlayerName;
	NewGame.iPlayerGenome = m_Settings.iPlayerGenome;
	NewGame.dwPlayerShip = m_ShipClasses[m_iCurShipClass]->GetUNID();
	NewGame.bDefaultPlayerName = m_Settings.bDefaultPlayerName;

	//	Remember some variables because after we close the session this object
	//	will be gone.

	CHumanInterface &HI(m_HI);

	//	New game

	HI.HICommand(CMD_GAME_CREATE, &NewGame);
	}

void CNewGameSession::CmdPrevShipClass (void)

//	CmdPrevShipClass
//
//	Select the previous ship class

	{
	if (m_iCurShipClass > 0)
		{
		SetShipClass(m_ShipClasses[--m_iCurShipClass], m_xShipClass, m_yShipClass, m_cxShipClass);

		//	See if we need to disable the prev button

		if (m_iCurShipClass == 0)
			{
			IAnimatron *pPrev = GetElement(CMD_PREV_SHIP_CLASS);
			if (pPrev)
				pPrev->SetPropertyBool(PROP_ENABLED, false);
			}

		//	The next button is always enabled after this

		IAnimatron *pNext = GetElement(CMD_NEXT_SHIP_CLASS);
		if (pNext)
			pNext->SetPropertyBool(PROP_ENABLED, true);
		}
	}

void CNewGameSession::CreatePlayerGenome (GenomeTypes iGenome, int x, int y, int cxWidth)

//	CreatePlayerGenome
//
//	Creates the player genome UI section

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumBoldFont = VI.GetFont(fontMediumBold);

	//	Label

	IAnimatron *pLabel = new CAniText;
	pLabel->SetPropertyVector(PROP_POSITION, CVector(x, y));
	pLabel->SetPropertyVector(PROP_SCALE, CVector(cxWidth - SMALL_BUTTON_WIDTH - MAJOR_PADDING_HORZ, 100.0));
	pLabel->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
	pLabel->SetPropertyFont(PROP_FONT, &MediumBoldFont);
	pLabel->SetPropertyString(PROP_TEXT, STR_GENOME);
	pLabel->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_RIGHT);

	m_pRoot->AddLine(pLabel);

	//	Create a button

	IAnimatron *pButton;
	VI.CreateImageButtonSmall(NULL, 
			CMD_CHANGE_GENOME, 
			x + cxWidth - SMALL_BUTTON_WIDTH, 
			y,
			&VI.GetImage(imageSmallHumanMale),
			0,
			&pButton);

	pButton->SetID(CMD_CHANGE_GENOME);

	RegisterPerformanceEvent(pButton, EVENT_ON_CLICK, CMD_CHANGE_GENOME);
	m_pRoot->AddLine(pButton);

	//	Player genome

	SetPlayerGenome(iGenome, x, y, cxWidth);
	}

void CNewGameSession::CreatePlayerName (const CString &sName, int x, int y, int cxWidth)

//	CreatePlayerName
//
//	Creates the player name UI section

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumBoldFont = VI.GetFont(fontMediumBold);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	//	Label

	IAnimatron *pLabel = new CAniText;
	pLabel->SetPropertyVector(PROP_POSITION, CVector(x + SMALL_BUTTON_WIDTH + MAJOR_PADDING_HORZ, y));
	pLabel->SetPropertyVector(PROP_SCALE, CVector(cxWidth, 100.0));
	pLabel->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
	pLabel->SetPropertyFont(PROP_FONT, &MediumBoldFont);
	pLabel->SetPropertyString(PROP_TEXT, CONSTLIT("name"));

	m_pRoot->AddLine(pLabel);

	//	Create a button

	IAnimatron *pButton;
	VI.CreateImageButtonSmall(NULL, 
			CMD_EDIT_NAME, 
			x, 
			y,
			&VI.GetImage(imageSmallEditIcon),
			0,
			&pButton);

	pButton->SetID(CMD_EDIT_NAME);

	RegisterPerformanceEvent(pButton, EVENT_ON_CLICK, CMD_EDIT_NAME);
	m_pRoot->AddLine(pButton);

	//	Player name

	SetPlayerName(sName, x, y, cxWidth);
	}

void CNewGameSession::CreateShipClass (CShipClass *pClass, int x, int y, int cxWidth)

//	CreateShipClass
//
//	Creates the ship class selection elements

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumBoldFont = VI.GetFont(fontMediumBold);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	//	Create buttons to select the class

	int xCenter = (x + cxWidth / 2);
	int xHalfSpacing = SMALL_SPACING_HORZ / 2;

	IAnimatron *pLeftButton;
	VI.CreateImageButtonSmall(NULL, 
			CMD_PREV_SHIP_CLASS, 
			xCenter - xHalfSpacing - SMALL_BUTTON_WIDTH, 
			y,
			&VI.GetImage(imageSmallLeftIcon),
			0,
			&pLeftButton);

	if (m_iCurShipClass == 0)
		pLeftButton->SetPropertyBool(PROP_ENABLED, false);

	RegisterPerformanceEvent(pLeftButton, EVENT_ON_CLICK, CMD_PREV_SHIP_CLASS);
	m_pRoot->AddLine(pLeftButton);

	IAnimatron *pRightButton;
	VI.CreateImageButtonSmall(NULL, 
			CMD_NEXT_SHIP_CLASS, 
			xCenter + xHalfSpacing, 
			y,
			&VI.GetImage(imageSmallRightIcon),
			0,
			&pRightButton);

	if (m_iCurShipClass + 1 == m_ShipClasses.GetCount())
		pRightButton->SetPropertyBool(PROP_ENABLED, false);

	RegisterPerformanceEvent(pRightButton, EVENT_ON_CLICK, CMD_NEXT_SHIP_CLASS);
	m_pRoot->AddLine(pRightButton);

	//	Label

	IAnimatron *pLabel = new CAniText;
	pLabel->SetPropertyVector(PROP_POSITION, CVector(x, y + SMALL_BUTTON_HEIGHT + SMALL_SPACING_VERT));
	pLabel->SetPropertyVector(PROP_SCALE, CVector(cxWidth, 100.0));
	pLabel->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
	pLabel->SetPropertyFont(PROP_FONT, &MediumBoldFont);
	pLabel->SetPropertyString(PROP_TEXT, CONSTLIT("ship class"));
	pLabel->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_CENTER);

	m_pRoot->AddLine(pLabel);

	//	Ship class

	SetShipClass(pClass, x, y, cxWidth);
	}

void CNewGameSession::OnCleanUp (void)

//	OnCleanUp
//
//	Clean up

	{
	}

ALERROR CNewGameSession::OnCommand (const CString &sCmd, void *pData)

//	OnCommand
//
//	Handle a command

	{
	if (strEquals(sCmd, CMD_CLOSE_SESSION))
		CmdCancel();
	else if (strEquals(sCmd, CMD_OK_SESSION))
		CmdOK();
	else if (strEquals(sCmd, CMD_NEXT_SHIP_CLASS))
		CmdNextShipClass();
	else if (strEquals(sCmd, CMD_PREV_SHIP_CLASS))
		CmdPrevShipClass();
	else if (strEquals(sCmd, CMD_CHANGE_GENOME))
		CmdChangeGenome();
	else if (strEquals(sCmd, CMD_EDIT_NAME))
		CmdEditName();

	return NOERROR;
	}

ALERROR CNewGameSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	ALERROR error;
	int i;

	const CVisualPalette &VI = m_HI.GetVisuals();

	//	The main pane is divided into three columns. Compute the size and
	//	position here.

	RECT rcCenter;
	VI.GetWidescreenRect(&rcCenter);

	m_cxLeftCol = RectWidth(rcCenter) / 3;
	m_cxRightCol = m_cxLeftCol;
	m_cxCenterCol = RectWidth(rcCenter) - (m_cxLeftCol + m_cxRightCol);

	m_xLeftCol = 0;
	m_xCenterCol = m_cxLeftCol;
	m_xRightCol = m_cxLeftCol + m_cxCenterCol;

	//	Compute the location of various elements (relative to the upper-left of
	//	the root vscroller).

	m_xPlayerName = m_xLeftCol;
	m_yPlayerName = MAJOR_PADDING_TOP;
	m_cxPlayerName = m_cxLeftCol;
	m_xPlayerGenome = m_xRightCol;
	m_yPlayerGenome = MAJOR_PADDING_TOP;
	m_cxPlayerGenome = m_cxRightCol;
	m_xShipClass = m_xLeftCol;
	m_yShipClass = MAJOR_PADDING_TOP;
	m_cxShipClass = RectWidth(rcCenter);

	//	Generate a list of ship classes

	CAdventureDesc *pAdventure = g_pUniverse->GetCurrentAdventureDesc();
	if (pAdventure == NULL)
		{
		*retsError = ERR_NO_ADVENTURE;
		return ERR_FAIL;
		}

	if (error = pAdventure->GetStartingShipClasses(&m_ShipClasses, retsError))
		return error;

	if (m_ShipClasses.GetCount() == 0)
		{
		*retsError = ERR_NO_SHIP_CLASSES;
		return ERR_FAIL;
		}

	//	Find the default ship class in the list (it's OK if we don't find it).

	m_iCurShipClass = 0;
	for (i = 0; i < m_ShipClasses.GetCount(); i++)
		if (m_ShipClasses[i]->GetUNID() == m_Settings.dwPlayerShip)
			{
			m_iCurShipClass = i;
			break;
			}

	//	Create the title

	CUIHelper Helper(m_HI);
	IAnimatron *pTitle;
	Helper.CreateSessionTitle(this, m_Service, pAdventure->GetName(), NULL, CUIHelper::OPTION_SESSION_OK_BUTTON, &pTitle);
	StartPerformance(pTitle, ID_CTRL_TITLE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Create a scroller to hold all the settings

	m_pRoot = new CAniVScroller;
	m_pRoot->SetPropertyVector(PROP_POSITION, CVector(rcCenter.left, rcCenter.top));
	m_pRoot->SetPropertyMetric(PROP_VIEWPORT_HEIGHT, (Metric)RectHeight(rcCenter));
	m_pRoot->SetPropertyMetric(PROP_FADE_EDGE_HEIGHT, 0.0);
	m_pRoot->SetPropertyMetric(PROP_PADDING_BOTTOM, (Metric)MAJOR_PADDING_BOTTOM);

	//	Create the player name

	CreatePlayerName(m_Settings.sPlayerName, m_xPlayerName, m_yPlayerName, m_cxPlayerName);
	m_bEditingName = false;

	//	Create the player genome

	CreatePlayerGenome(m_Settings.iPlayerGenome, m_xPlayerGenome, m_yPlayerGenome, m_cxPlayerGenome);

	//	Create the ship class

	CreateShipClass(m_ShipClasses[m_iCurShipClass], m_xShipClass, m_yShipClass, m_cxShipClass);

	//	Start the settings pane

	StartPerformance(m_pRoot, ID_SETTINGS, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Done

	return NOERROR;
	}

void CNewGameSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//	OnKeyDown
//
//	KeyDown

	{
	switch (iVirtKey)
		{
		case VK_RETURN:
			if (m_bEditingName)
				CmdEditName();
			else
				CmdOK();
			break;

		case VK_ESCAPE:
			if (m_bEditingName)
				CmdEditNameCancel();
			else
				CmdCancel();
			break;

		case VK_LEFT:
			CmdPrevShipClass();
			break;

		case VK_RIGHT:
			CmdNextShipClass();
			break;

		default:
			HandlePageScrollKeyDown(ID_SETTINGS, iVirtKey, dwKeyData);
			break;
		}
	}

void CNewGameSession::OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture)

//	OnLButtonDown
//
//	LButtonDown

	{
	}

void CNewGameSession::OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags)

//	OnMouseWheel
//
//	Handle mouse wheel

	{
	HandlePageScrollMouseWheel(ID_SETTINGS, iDelta);
	}

void CNewGameSession::OnPaint (CG32bitImage &Screen, const RECT &rcInvalid)

//	OnPaint
//
//	Paint

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	RECT rcCenter;
	VI.DrawSessionBackground(Screen, CG32bitImage(), CG32bitPixel(), CVisualPalette::OPTION_SESSION_DLG_BACKGROUND, &rcCenter);
	}

void CNewGameSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash
//
//	Describe current state

	{
	*retsMessage = CONSTLIT("session: CNewGameSession\r\n");
	}

void CNewGameSession::OnUpdate (bool bTopMost)

//	OnUpdate
//
//	Update

	{
	}

void CNewGameSession::SetPlayerGenome (GenomeTypes iGenome, int x, int y, int cxWidth)

//	SetPlayerGenome
//
//	Sets the current player genome

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumBoldFont = VI.GetFont(fontMediumBold);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	//	Delete the previous one

	DeleteElement(ID_PLAYER_GENOME);

	//	Player genome

	IAnimatron *pGenome = new CAniText;
	pGenome->SetID(ID_PLAYER_GENOME);
	pGenome->SetPropertyVector(PROP_POSITION, CVector(x, y + MediumBoldFont.GetHeight()));
	pGenome->SetPropertyVector(PROP_SCALE, CVector(cxWidth - SMALL_BUTTON_WIDTH - MAJOR_PADDING_HORZ, 100.0));
	pGenome->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogLabel));
	pGenome->SetPropertyFont(PROP_FONT, &SubTitleFont);
	pGenome->SetPropertyString(PROP_TEXT, strTitleCapitalize(GetGenomeName(iGenome)));
	pGenome->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_RIGHT);

	m_pRoot->AddLine(pGenome);

	//	Change the button image

	IAnimatron *pButton;
	if (m_pRoot->FindElement(CMD_CHANGE_GENOME, &pButton))
		{
		const CG32bitImage *pImage;

		if (iGenome == genomeHumanMale)
			pImage = &VI.GetImage(imageSmallHumanMale);
		else
			pImage = &VI.GetImage(imageSmallHumanFemale);

		IAnimatron *pStyle = pButton->GetStyle(STYLE_IMAGE);
		pStyle->SetFillMethod(new CAniImageFill(pImage, false));
		}
	}

void CNewGameSession::SetPlayerName (const CString &sName, int x, int y, int cxWidth)

//	SetPlayerName
//
//	Sets the current player name

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumBoldFont = VI.GetFont(fontMediumBold);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	//	Delete the previous one

	DeleteElement(ID_PLAYER_NAME);

	//	Player name

	IAnimatron *pName = new CAniText;
	pName->SetID(ID_PLAYER_NAME);
	pName->SetPropertyVector(PROP_POSITION, CVector(x + SMALL_BUTTON_WIDTH + MAJOR_PADDING_HORZ, y + MediumBoldFont.GetHeight()));
	pName->SetPropertyVector(PROP_SCALE, CVector(cxWidth, 100.0));
	pName->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogLabel));
	pName->SetPropertyFont(PROP_FONT, &SubTitleFont);
	pName->SetPropertyString(PROP_TEXT, sName);

	m_pRoot->AddLine(pName);
	}

void CNewGameSession::SetShipClass (CShipClass *pClass, int x, int y, int cxWidth)

//	SetShipClass
//
//	Sets the ship class

	{
	int i;

	g_pUniverse->SetLogImageLoad(false);

	const CPlayerSettings *pPlayerSettings = pClass->GetPlayerSettings();

	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumBoldFont = VI.GetFont(fontMediumBold);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	//	Ship class name

	SetShipClassName(pClass->GetNounPhrase(nounGeneric), x, y, cxWidth);
	SetShipClassDesc(pPlayerSettings->GetDesc(), x, y, cxWidth);

	//	Offset

	int yOffset = SMALL_BUTTON_HEIGHT + SMALL_SPACING_VERT + MediumBoldFont.GetHeight() + 2 * SubTitleFont.GetHeight();

	//	Ship class image

	SetShipClassImage(pClass, x, y + yOffset, cxWidth);

	//	Delete previous info

	DeleteElement(ID_SHIP_CLASS_INFO);

	//	Create a sequencer for all class info components

	CAniSequencer *pClassInfo;
	CAniSequencer::Create(CVector(x, y + yOffset + SubTitleFont.GetHeight()), &pClassInfo);
	pClassInfo->SetID(ID_SHIP_CLASS_INFO);

	//	Generate default devices for the ship class

	CDeviceDescList Devices;
	pClass->GenerateDevices(1, Devices);

	//	Generate list of all weapons, sorted by level and name

	TSortMap<CString, CItem> RightSide;
	for (i = 0; i < Devices.GetCount(); i++)
		{
        const CItem &DevItem = Devices.GetDeviceDesc(i).Item;
		CDeviceClass *pDevice = Devices.GetDeviceClass(i);
		if (pDevice->GetCategory() == itemcatWeapon ||
				pDevice->GetCategory() == itemcatLauncher)
			RightSide.Insert(strPatternSubst(CONSTLIT("%02d_%02d_%s"), 1, DevItem.GetLevel(), DevItem.GetNounPhrase(CItemCtx())), DevItem);
		}

	//	Add shields

	TSortMap<CString, CItem> LeftSide;
    const SDeviceDesc *pShields = Devices.GetDeviceDescByName(devShields);
	if (pShields)
		RightSide.Insert(strPatternSubst(CONSTLIT("%02d_%02d_%s"), 2, pShields->Item.GetLevel(), pShields->Item.GetNounPhrase(CItemCtx())), pShields->Item);

	//	Add armor

	RightSide.Insert(CONSTLIT("03"), CItem(g_pUniverse->GetItemType(0), SPECIAL_ARMOR));

	//	Add reactor

	LeftSide.Insert(CONSTLIT("01"), CItem(g_pUniverse->GetItemType(0), SPECIAL_REACTOR));

	//	Add engines

	LeftSide.Insert(CONSTLIT("02"), CItem(g_pUniverse->GetItemType(0), SPECIAL_DRIVE));

	//	Add cargo

	LeftSide.Insert(CONSTLIT("03"), CItem(g_pUniverse->GetItemType(0), SPECIAL_CARGO));

	//	Add misc devices

	for (i = 0; i < Devices.GetCount(); i++)
		{
        const CItem &DevItem = Devices.GetDeviceDesc(i).Item;
		CDeviceClass *pDevice = Devices.GetDeviceClass(i);
		if (pDevice->GetCategory() == itemcatMiscDevice)
			LeftSide.Insert(strPatternSubst(CONSTLIT("%02d_%02d_%s"), 4, DevItem.GetLevel(), DevItem.GetNounPhrase(CItemCtx())), DevItem);
		}

	//	Add device slots

	LeftSide.Insert(CONSTLIT("05"), CItem(g_pUniverse->GetItemType(0), SPECIAL_DEVICE_SLOTS));

	//	Set the ship class info. All weapons go to the right of the ship image

	int xPos = (cxWidth / 2) + (SHIP_IMAGE_RECT_WIDTH / 2);
	int yPos = 0;
	int cxInfo = (cxWidth - xPos);

	for (i = 0; i < RightSide.GetCount(); i++)
		{
		int cyInfo;
		IAnimatron *pInfo;
		AddClassInfo(pClass, Devices, RightSide[i], xPos, yPos, cxInfo, 0, &cyInfo, &pInfo);

		pClassInfo->AddTrack(pInfo, 0);
		yPos += cyInfo + ITEM_INFO_PADDING_VERT;
		}

	//	Misc devices go on the left

	xPos = (cxWidth / 2) - (SHIP_IMAGE_RECT_WIDTH / 2);
	yPos = 0;
	cxInfo = xPos;

	for (i = 0; i < LeftSide.GetCount(); i++)
		{
		int cyInfo;
		IAnimatron *pInfo;
		AddClassInfo(pClass, Devices, LeftSide[i], xPos, yPos, cxInfo, CUIHelper::OPTION_ITEM_RIGHT_ALIGN, &cyInfo, &pInfo);

		pClassInfo->AddTrack(pInfo, 0);
		yPos += cyInfo + ITEM_INFO_PADDING_VERT;
		}

	m_pRoot->AddLine(pClassInfo);

	g_pUniverse->SetLogImageLoad(true);
	}

void CNewGameSession::SetShipClassDesc (const CString &sDesc, int x, int y, int cxWidth)

//	SetShipClassDesc
//
//	Sets the current ship class description

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);
	const CG16bitFont &MediumBoldFont = VI.GetFont(fontMediumBold);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	//	Delete the previous one

	DeleteElement(ID_SHIP_CLASS_DESC);

	int xCenter = x + (cxWidth / 2);
	int cxDesc = 2 * SHIP_IMAGE_WIDTH;

	//	Ship class

	IAnimatron *pDesc = new CAniText;
	pDesc->SetID(ID_SHIP_CLASS_DESC);
	pDesc->SetPropertyVector(PROP_POSITION, CVector(xCenter - cxDesc / 2, y + SMALL_BUTTON_HEIGHT + SMALL_SPACING_VERT + MediumBoldFont.GetHeight() + SubTitleFont.GetHeight()));
	pDesc->SetPropertyVector(PROP_SCALE, CVector(cxDesc, 100.0));
	pDesc->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogLabel));
	pDesc->SetPropertyFont(PROP_FONT, &MediumFont);
	pDesc->SetPropertyString(PROP_TEXT, sDesc);
	pDesc->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_CENTER);

	m_pRoot->AddLine(pDesc);
	}

void CNewGameSession::SetShipClassImage (CShipClass *pClass, int x, int y, int cxWidth)

//	SetShipClassImage
//
//	Sets the current ship class image

	{
	const CPlayerSettings *pPlayerSettings = pClass->GetPlayerSettings();

    //  Ask the class for a hero image

    const CG32bitImage *pImage = (!pClass->GetHeroImage().IsEmpty() ? &pClass->GetHeroImage().GetImage(CONSTLIT("New Game")) : NULL);
    
	//	Delete the previous one

	DeleteElement(ID_SHIP_CLASS_IMAGE);

	//	Add the new one, if we've got one.

	const CG32bitImage *pImageToUse = NULL;
	bool bFree = false;

	if (pImage && !pImage->IsEmpty())
		{
		//	If this image is not the right size, then create a resized version
		//	that is.

		if (pImage->GetWidth() != SHIP_IMAGE_WIDTH || pImage->GetHeight() != SHIP_IMAGE_HEIGHT)
			{
			int cxNewWidth = SHIP_IMAGE_WIDTH;
			int cyNewHeight = cxNewWidth * pImage->GetHeight() / pImage->GetWidth();
			if (cyNewHeight > SHIP_IMAGE_HEIGHT)
				{
				cyNewHeight = SHIP_IMAGE_HEIGHT;
				cxNewWidth = cyNewHeight * pImage->GetWidth() / pImage->GetHeight();
				}

			CG32bitImage *pNewImage = new CG32bitImage;
			pNewImage->CreateFromImageTransformed(*pImage, 0, 0, pImage->GetWidth(), pImage->GetHeight(), (Metric)cxNewWidth / pImage->GetWidth(), (Metric)cyNewHeight / pImage->GetHeight(), 0.0);

			pImageToUse = pNewImage;
			bFree = true;
			}
		else
			{
			pImageToUse = pImage;
			bFree = false;
			}
		}

	//	If we don't have an image then ask the class to paint it

	else
		{
		CG32bitImage *pNewImage = new CG32bitImage;
		pNewImage->Create(SHIP_IMAGE_WIDTH, SHIP_IMAGE_HEIGHT);

		ViewportTransform Trans;
		pClass->Paint(*pNewImage, 
				SHIP_IMAGE_WIDTH / 2, 
				SHIP_IMAGE_HEIGHT / 2, 
				Trans, 
				0, 
				0
				);

		pImageToUse = pNewImage;
		bFree = true;
		}

	//	Position

	int xImage = x + (cxWidth - pImageToUse->GetWidth()) / 2;
	int yImage = y + (SHIP_IMAGE_HEIGHT - pImageToUse->GetHeight()) / 2;

	//	New image frame

	bool bAutoMask = (pClass->GetAPIVersion() < 26);

	IAnimatron *pImageFrame = new CAniRect;
	pImageFrame->SetID(ID_SHIP_CLASS_IMAGE);
	pImageFrame->SetPropertyVector(PROP_POSITION, CVector(xImage, yImage));
	pImageFrame->SetPropertyVector(PROP_SCALE, CVector(pImageToUse->GetWidth(), pImageToUse->GetHeight()));
	pImageFrame->SetFillMethod(new CAniImageFill(pImageToUse, bFree, bAutoMask));

	m_pRoot->AddLine(pImageFrame);
	}

void CNewGameSession::SetShipClassName (const CString &sName, int x, int y, int cxWidth)

//	SetShipClassName
//
//	Sets the current ship class name

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumBoldFont = VI.GetFont(fontMediumBold);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	//	Delete the previous one

	DeleteElement(ID_SHIP_CLASS_NAME);

	//	Ship class

	IAnimatron *pName = new CAniText;
	pName->SetID(ID_SHIP_CLASS_NAME);
	pName->SetPropertyVector(PROP_POSITION, CVector(x, y + SMALL_BUTTON_HEIGHT + SMALL_SPACING_VERT + MediumBoldFont.GetHeight()));
	pName->SetPropertyVector(PROP_SCALE, CVector(cxWidth, 100.0));
	pName->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogLabel));
	pName->SetPropertyFont(PROP_FONT, &SubTitleFont);
	pName->SetPropertyString(PROP_TEXT, sName);
	pName->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_CENTER);

	m_pRoot->AddLine(pName);
	}

