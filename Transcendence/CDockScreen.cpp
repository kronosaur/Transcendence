//	CDockScreen.cpp
//
//	CDockScreen class

#include "PreComp.h"
#include "Transcendence.h"

const int PICKER_ROW_HEIGHT	=	96;
const int PICKER_ROW_COUNT =	4;

const int g_cxDockScreen = 800;
const int g_cyDockScreen = 400;
const int g_cxBackground = 1024;
const int g_cyBackground = 400;
const int DESC_PANE_X =			600;
const int BACKGROUND_FOCUS_X =	(DESC_PANE_X / 2);
const int BACKGROUND_FOCUS_Y =	(g_cyBackground / 2);

const int STATUS_BAR_HEIGHT	=	20;
const int g_cyTitle =			72;
const int g_cxActionsRegion =	400;
const int g_cyAction =			22;
const int g_cyActionSpacing =	4;
const int g_iMaxActions =		8;
const int g_cyActionsRegion =	(g_cyAction * g_iMaxActions) + (g_cyActionSpacing * (g_iMaxActions - 1));

const int g_cyItemTitle =		32;
const int g_cxItemMargin =		132;
const int g_cxItemImage =		96;
const int g_cyItemImage =		96;

const int g_cxStats =			400;
const int g_cyStats =			30;
const int g_cxCargoStats =		200;
const int g_cxCargoStatsLabel =	100;

const int g_cxCounter =			128;
const int g_cyCounter =			40;

const int TEXT_INPUT_WIDTH =	380;
const int TEXT_INPUT_HEIGHT	=	40;

const int g_FirstActionID =		100;
const int g_LastActionID =		199;

const int g_PrevActionID =		200;
const int g_NextActionID =		201;
const int g_ItemTitleID =		202;
const int g_ItemDescID =		203;
const int g_CounterID =			204;
const int g_ItemImageID =		205;
const int PICKER_ID	=			206;
const int TEXT_INPUT_ID =		207;
const int IMAGE_AREA_ID =		208;

const int ACTION_CUSTOM_NEXT_ID =	300;
const int ACTION_CUSTOM_PREV_ID =	301;

#define ACTIONS_TAG					CONSTLIT("Actions")
#define CANVAS_TAG					CONSTLIT("Canvas")
#define DISPLAY_TAG					CONSTLIT("Display")
#define GROUP_TAG					CONSTLIT("Group")
#define IMAGE_TAG					CONSTLIT("Image")
#define INITIAL_PANE_TAG			CONSTLIT("InitialPane")
#define INITIALIZE_TAG				CONSTLIT("Initialize")
#define LIST_TAG					CONSTLIT("List")
#define NEUROHACK_TAG				CONSTLIT("Neurohack")
#define ON_DISPLAY_INIT_TAG			CONSTLIT("OnDisplayInit")
#define ON_INIT_TAG					CONSTLIT("OnInit")
#define ON_PANE_INIT_TAG			CONSTLIT("OnPaneInit")
#define ON_SCREEN_INIT_TAG			CONSTLIT("OnScreenInit")
#define TEXT_TAG					CONSTLIT("Text")

#define ALIGN_ATTRIB				CONSTLIT("align")
#define ANIMATE_ATTRIB				CONSTLIT("animate")
#define BACKGROUND_ID_ATTRIB		CONSTLIT("backgroundID")
#define BOTTOM_ATTRIB				CONSTLIT("bottom")
#define CENTER_ATTRIB				CONSTLIT("center")
#define COLOR_ATTRIB				CONSTLIT("color")
#define FONT_ATTRIB					CONSTLIT("font")
#define HEIGHT_ATTRIB				CONSTLIT("height")
#define ID_ATTRIB					CONSTLIT("id")
#define INITIAL_ITEM_ATTRIB			CONSTLIT("initialItem")
#define LEFT_ATTRIB					CONSTLIT("left")
#define LIST_ATTRIB					CONSTLIT("list")
#define NAME_ATTRIB					CONSTLIT("name")
#define NO_LIST_NAVIGATION_ATTRIB	CONSTLIT("noListNavigation")
#define PANE_ATTRIB					CONSTLIT("pane")
#define RIGHT_ATTRIB				CONSTLIT("right")
#define ROW_HEIGHT_ATTRIB			CONSTLIT("rowHeight")
#define SHOW_TEXT_INPUT_ATTRIB		CONSTLIT("showTextInput")
#define TOP_ATTRIB					CONSTLIT("top")
#define TRANSPARENT_ATTRIB			CONSTLIT("transparent")
#define VALIGN_ATTRIB				CONSTLIT("valign")
#define VCENTER_ATTRIB				CONSTLIT("vcenter")
#define WIDTH_ATTRIB				CONSTLIT("width")

#define SCREEN_TYPE_ITEM_PICKER		CONSTLIT("itemPicker")
#define SCREEN_TYPE_CUSTOM_PICKER	CONSTLIT("customPicker")

#define ALIGN_CENTER				CONSTLIT("center")
#define ALIGN_RIGHT					CONSTLIT("right")
#define ALIGN_LEFT					CONSTLIT("left")
#define ALIGN_BOTTOM				CONSTLIT("bottom")
#define ALIGN_TOP					CONSTLIT("top")
#define ALIGN_MIDDLE				CONSTLIT("middle")

#define DATA_FROM_PLAYER			CONSTLIT("player")
#define DATA_FROM_SOURCE			CONSTLIT("source")
#define DATA_FROM_STATION			CONSTLIT("station")

static char g_PanesTag[] = "Panes";
static char g_ListOptionsTag[] = "ListOptions";

static char g_ScreenTypeAttrib[] = "type";
static char g_DescAttrib[] = "desc";
static char g_DataFromAttrib[] = "dataFrom";
static char g_ShowCounterAttrib[] = "showCounter";

CDockScreen::CDockScreen (void) : CObject(NULL),
		m_pTrans(NULL),
		m_pFonts(NULL),
		m_pUniv(NULL),
		m_pPlayer(NULL),
		m_pLocation(NULL),
		m_pLocalScreens(NULL),
		m_pDesc(NULL),
		m_pScreen(NULL),
		m_bFirstOnInit(true),
		m_bInOnInit(false),

		m_pBackgroundImage(NULL),
		m_pCredits(NULL),
		m_pCargoSpace(NULL),
		m_bFreeBackgroundImage(false),

		m_pItemListControl(NULL),
		m_pDisplayInitialize(NULL),
		m_bDisplayAnimate(false),

		m_pCounter(NULL),
		m_bReplaceCounter(false),

		m_pTextInput(NULL),

		m_pPanes(NULL),
		m_pCurrentPane(NULL),
		m_pCurrentFrame(NULL),
		m_pFrameDesc(NULL),
		m_bInShowPane(false),
		m_bNoListNavigation(false)

//	CDockScreen constructor

	{
	}

CDockScreen::~CDockScreen (void)

//	CDockScreen destructor

	{
	CleanUpScreen();
	}

void CDockScreen::Action (DWORD dwTag, DWORD dwData)

//	Action
//
//	Button pressed

	{
	//	Get the data for the particular button

	switch (dwTag)
		{
		case g_PrevActionID:
			if (!m_bNoListNavigation)
				{
				SelectPrevItem();
				ShowItem();
				m_CurrentActions.ExecuteShowPane(EvalInitialPane());
				}
			break;

		case g_NextActionID:
			if (!m_bNoListNavigation)
				{
				SelectNextItem();
				ShowItem();
				m_CurrentActions.ExecuteShowPane(EvalInitialPane());
				}
			break;

		case PICKER_ID:
			{
			if (!m_bNoListNavigation)
				{
				if (dwData == ITEM_LIST_AREA_PAGE_UP_ACTION)
					{
					SelectPrevItem();
					SelectPrevItem();
					SelectPrevItem();
					m_pItemListControl->Invalidate();
					m_CurrentActions.ExecuteShowPane(EvalInitialPane());
					}
				else if (dwData == ITEM_LIST_AREA_PAGE_DOWN_ACTION)
					{
					SelectNextItem();
					SelectNextItem();
					SelectNextItem();
					m_pItemListControl->Invalidate();
					m_CurrentActions.ExecuteShowPane(EvalInitialPane());
					}
				else
					{
					m_pItemListControl->SetCursor(dwData);
					m_CurrentActions.ExecuteShowPane(EvalInitialPane());
					}
				}
			break;
			}

		default:
			{
			if (dwTag >= g_FirstActionID && dwTag <= g_LastActionID)
				{
				m_CurrentActions.Execute(dwTag - g_FirstActionID, this);
				break;
				}
			}
		}
	}

void CDockScreen::AddDisplayControl (CXMLElement *pDesc, 
									 AGScreen *pScreen, 
									 SDisplayControl *pParent,
									 const RECT &rcFrame, 
									 SDisplayControl **retpDControl)

//	AddDisplayControl
//
//	Adds a control from XML element

	{
	int i;
	CCodeChain &CC = m_pUniv->GetCC();
	SDisplayControl *pDControl = m_Controls.Insert();

	//	Set basic attribs

	pDControl->sID = pDesc->GetAttribute(ID_ATTRIB);

	//	Propage animation from parent, unless overridden.
	//	Note: we default to always animate

	if (!pDesc->FindAttributeBool(ANIMATE_ATTRIB, &pDControl->bAnimate))
		pDControl->bAnimate = (pParent ? pParent->bAnimate : true);

	//	Control rect (relative to pScreen)

	RECT rcRect;
	InitDisplayControlRect(pDesc, rcFrame, &rcRect);

	//	Get the font

	const CG16bitFont *pControlFont = &m_pFonts->Large;
	CString sFontName;
	if (pDesc->FindAttribute(FONT_ATTRIB, &sFontName))
		pControlFont = &GetFontByName(*m_pFonts, sFontName);

	WORD wControlColor;
	CString sColorName;
	if (pDesc->FindAttribute(COLOR_ATTRIB, &sColorName))
		wControlColor = ::LoadRGBColor(sColorName);
	else
		wControlColor = CG16bitImage::RGBValue(255, 255, 255);

	//	Create the control based on the type

	if (strEquals(pDesc->GetTag(), TEXT_TAG))
		{
		pDControl->iType = ctrlText;

		CGTextArea *pControl = new CGTextArea;
		pControl->SetFont(pControlFont);
		pControl->SetColor(wControlColor);

		CString sAlign = pDesc->GetAttribute(ALIGN_ATTRIB);
		if (strEquals(sAlign, ALIGN_CENTER))
			pControl->SetStyles(alignCenter);
		else if (strEquals(sAlign, ALIGN_RIGHT))
			pControl->SetStyles(alignRight);

		pScreen->AddArea(pControl, rcRect, 0);
		pDControl->pArea = pControl;

		//	Load the text code

		pDControl->pCode = CC.Link(pDesc->GetContentText(0), 0, NULL);
		}
	else if (strEquals(pDesc->GetTag(), IMAGE_TAG))
		{
		pDControl->iType = ctrlImage;

		CGImageArea *pControl = new CGImageArea;
		pControl->SetTransBackground(pDesc->GetAttributeBool(TRANSPARENT_ATTRIB));

		DWORD dwStyles = 0;
		CString sAlign = pDesc->GetAttribute(ALIGN_ATTRIB);
		if (strEquals(sAlign, ALIGN_CENTER))
			dwStyles |= alignCenter;
		else if (strEquals(sAlign, ALIGN_RIGHT))
			dwStyles |= alignRight;
		else
			dwStyles |= alignLeft;

		sAlign = pDesc->GetAttribute(VALIGN_ATTRIB);
		if (strEquals(sAlign, ALIGN_CENTER))
			dwStyles |= alignMiddle;
		else if (strEquals(sAlign, ALIGN_BOTTOM))
			dwStyles |= alignBottom;
		else
			dwStyles |= alignTop;

		pControl->SetStyles(dwStyles);

		pScreen->AddArea(pControl, rcRect, 0);
		pDControl->pArea = pControl;

		//	Load the code that returns the image

		pDControl->pCode = CC.Link(pDesc->GetContentText(0), 0, NULL);
		}
	else if (strEquals(pDesc->GetTag(), CANVAS_TAG))
		{
		pDControl->iType = ctrlCanvas;

		CGDrawArea *pControl = new CGDrawArea;
		pScreen->AddArea(pControl, rcRect, 0);
		pDControl->pArea = pControl;

		//	Load the draw code

		pDControl->pCode = CC.Link(pDesc->GetContentText(0), 0, NULL);
		}
	else if (strEquals(pDesc->GetTag(), GROUP_TAG))
		{
		for (i = 0; i < pDesc->GetContentElementCount(); i++)
			{
			CXMLElement *pSubDesc = pDesc->GetContentElement(i);
			AddDisplayControl(pSubDesc, pScreen, pDControl, rcRect);
			}
		}
	else if (strEquals(pDesc->GetTag(), NEUROHACK_TAG))
		{
		pDControl->iType = ctrlNeurohack;

		CGNeurohackArea *pControl = new CGNeurohackArea;
		pScreen->AddArea(pControl, rcRect, 0);
		pDControl->pArea = pControl;

		//	Load the text code

		pDControl->pCode = CC.Link(pDesc->GetContentText(0), 0, NULL);
		}

	//	Done

	if (retpDControl)
		*retpDControl = pDControl;
	}

void CDockScreen::CleanUpScreen (void)

//	CleanUpScreen
//
//	Called to bring the screen down after InitScreen

	{
	if (m_pScreen)
		{
		delete m_pScreen;
		m_pScreen = NULL;

		//	Note: No need to free any of the controls because deleting
		//	the screen will free them.
		m_pCurrentFrame = NULL;
		m_pFrameDesc = NULL;
		m_pCounter = NULL;
		m_pTextInput = NULL;
		m_pItemListControl = NULL;
		}

	if (m_pBackgroundImage)
		{
		if (m_bFreeBackgroundImage)
			delete m_pBackgroundImage;

		m_pBackgroundImage = NULL;
		}

	m_Controls.DeleteAll();
	m_CurrentActions.CleanUp();
	m_pDisplayInitialize = NULL;

	m_bInShowPane = false;
	}

ALERROR CDockScreen::CreateBackgroundImage (CXMLElement *pDesc)

//	CreateBackgroundImage
//
//	Creates the background image to use for the dock screen. Initializes
//	m_pBackgroundImage and m_bFreeBackgroundImage

	{
	enum BackgroundTypes { backgroundNone, backgroundImage, backgroundObj };

	ASSERT(m_pLocation);
	ASSERT(m_pBackgroundImage == NULL);

	BackgroundTypes iType = backgroundNone;
	DWORD dwBackgroundID = 0;

	//	Figure out which background to use

	CString sBackgroundID;
	if (pDesc->FindAttribute(BACKGROUND_ID_ATTRIB, &sBackgroundID))
		{
		//	If the attribute exists, but is empty (or equals "none") then
		//	we don't have a background

		if (sBackgroundID.IsBlank() || strEquals(sBackgroundID, CONSTLIT("none")))
			iType = backgroundNone;

		//	If the ID is "object" then we should the object

		else if (strEquals(sBackgroundID, CONSTLIT("object")))
			iType = backgroundObj;

		//	Otherwise, we expect an integer

		else
			{
			iType = backgroundImage;
			dwBackgroundID = strToInt(sBackgroundID, 0);
			}
		}

	//	If no attribute specified, ask the location

	else
		{
		dwBackgroundID = m_pLocation->GetDefaultBkgnd();
		iType = (dwBackgroundID ? backgroundImage : backgroundObj);
		}

	//	Load the image

	CG16bitImage *pImage = NULL;
	if (dwBackgroundID)
		pImage = m_pUniv->GetLibraryBitmap(dwBackgroundID);

	//	Sometimes (like in the case of item lists) the image is larger than normal

	int cyExtra = 0;
	if (pImage)
		cyExtra = Max(pImage->GetHeight() - g_cyBackground, 0);

	//	Create a new image for the background

	m_pBackgroundImage = new CG16bitImage;
	m_bFreeBackgroundImage = true;
	m_pBackgroundImage->CreateBlank(g_cxBackground, g_cyBackground + cyExtra, false);

	if (cyExtra)
		m_pBackgroundImage->Fill(0, g_cyBackground, g_cxBackground, cyExtra, 0);

	//	Load the dock screen background based on the ship class

	DWORD dwScreenUNID = DEFAULT_DOCK_SCREEN_IMAGE_UNID;
	DWORD dwScreenMaskUNID = DEFAULT_DOCK_SCREEN_MASK_UNID;
	CG16bitImage *pScreenImage = m_pUniv->GetLibraryBitmap(dwScreenUNID);

	//	Blt to background

	if (pScreenImage)
		m_pBackgroundImage->Blt(0, 0, g_cxBackground, g_cyBackground, *pScreenImage, 0, 0);

	//	If not image, then we're done

	if (iType == backgroundNone)
		;

	//	Paint the object as the background

	else if (iType == backgroundObj)
		{
		SViewportPaintCtx Ctx;
		Ctx.fNoSelection = true;
		Ctx.pObj = m_pLocation;
		m_pLocation->Paint(*m_pBackgroundImage,
				BACKGROUND_FOCUS_X,
				BACKGROUND_FOCUS_Y,
				Ctx);
		}

	//	If we have an image with a mask, just blt the masked image

	else if (pImage && pImage->HasMask())
		m_pBackgroundImage->ColorTransBlt(0, 0, pImage->GetWidth(), pImage->GetHeight(), 255, *pImage, 0, 0);

	//	If we have an image with no mask, then we need to create our own mask

	else if (pImage)
		{
		CG16bitImage *pScreenMask = m_pUniv->GetLibraryBitmap(dwScreenMaskUNID);
		if (pScreenMask)
			m_pBackgroundImage->BltWithMask(0, 0, pImage->GetWidth(), pImage->GetHeight(), *pScreenMask, *pImage, 0, 0);
		else
			m_pBackgroundImage->Blt(0, 0, pImage->GetWidth(), pImage->GetHeight(), *pImage, 0, 0);
		}

	return NOERROR;
	}

ALERROR CDockScreen::CreateItemPickerControl (CXMLElement *pDesc, AGScreen *pScreen, const RECT &rcRect)

//	CreateItemPickerControl
//
//	Creates control for item picker screen

	{
	//	Calculate some basic metrics

	int xScreen = rcRect.left;
	int yScreen = (RectHeight(rcRect) - g_cyDockScreen) / 2;

	//	Create the picker control

	m_pItemListControl = new CGItemListArea;
	if (m_pItemListControl == NULL)
		return ERR_MEMORY;

	m_pItemListControl->SetUIRes(&g_pTrans->GetUIRes());
	m_pItemListControl->SetFontTable(m_pFonts);

	RECT rcPicker;
	rcPicker.left = xScreen + 12;
	rcPicker.top = yScreen + 12;
	rcPicker.right = xScreen + DESC_PANE_X - 44;
	rcPicker.bottom = rcPicker.top + (PICKER_ROW_COUNT * PICKER_ROW_HEIGHT);
	pScreen->AddArea(m_pItemListControl, rcPicker, PICKER_ID);

	return NOERROR;
	}

ALERROR CDockScreen::CreateTitleAndBackground (CXMLElement *pDesc, AGScreen *pScreen, const RECT &rcRect)

//	CreateTitleAndBackground
//
//	Creates a standard screen based on the screen descriptor element

	{
	//	Generate a background image

	CreateBackgroundImage(pDesc);
	int cyBackgroundImage = (m_pBackgroundImage ? m_pBackgroundImage->GetHeight() : 512);

	//	Add the background

	RECT rcBackArea;
	CGImageArea *pImage = NULL;

	if (m_pBackgroundImage)
		{
		pImage = new CGImageArea;
		RECT rcImage;
		rcImage.left = 0;
		rcImage.top = 0;
		rcImage.right = g_cxBackground;
		rcImage.bottom = cyBackgroundImage;
		pImage->SetImage(m_pBackgroundImage, rcImage);
		}

	rcBackArea.left = 0;
	rcBackArea.top = (RectHeight(rcRect) - g_cyDockScreen) / 2;
	rcBackArea.right = rcBackArea.left + g_cxBackground;
	rcBackArea.bottom = rcBackArea.top + cyBackgroundImage;

	if (pImage)
		pScreen->AddArea(pImage, rcBackArea, IMAGE_AREA_ID);

	//	Add a background bar to the title part

	pImage = new CGImageArea;
	pImage->SetBackColor(m_pFonts->wAltBlueBackground);
	RECT rcArea;
	rcArea.left = 0;
	rcArea.top = rcBackArea.top - g_cyTitle;
	rcArea.right = rcRect.right;
	rcArea.bottom = rcBackArea.top - STATUS_BAR_HEIGHT;
	pScreen->AddArea(pImage, rcArea, 0);

	pImage = new CGImageArea;
	pImage->SetBackColor(CG16bitImage::DarkenPixel(m_pFonts->wAltBlueBackground, 200));
	rcArea.left = 0;
	rcArea.top = rcBackArea.top - STATUS_BAR_HEIGHT;
	rcArea.right = rcRect.right;
	rcArea.bottom = rcBackArea.top;
	pScreen->AddArea(pImage, rcArea, 0);

	//	Get the name of this location

	CString sName = EvalString(pDesc->GetAttribute(NAME_ATTRIB));
	if (sName.IsBlank())
		sName = m_pLocation->GetNounPhrase(nounTitleCapitalize);

	//	Add the name as a title to the screen

	CGTextArea *pText = new CGTextArea;
	pText->SetText(sName);
	pText->SetFont(&m_pFonts->Title);
	pText->SetColor(m_pFonts->wTitleColor);
	pText->AddShadowEffect();
	rcArea.left = 8;
	rcArea.top = rcBackArea.top - g_cyTitle;
	rcArea.right = rcRect.right;
	rcArea.bottom = rcBackArea.top;
	pScreen->AddArea(pText, rcArea, 0);

	//	Add the money area

	int cyOffset = (STATUS_BAR_HEIGHT - m_pFonts->MediumHeavyBold.GetHeight()) / 2;

	m_pCredits = new CGTextArea;
	m_pCredits->SetFont(&m_pFonts->MediumHeavyBold);
	m_pCredits->SetColor(m_pFonts->wTitleColor);

	rcArea.left = rcRect.right - g_cxStats;
	rcArea.top = rcBackArea.top - STATUS_BAR_HEIGHT + cyOffset;
	rcArea.right = rcRect.right;
	rcArea.bottom = rcArea.top + g_cyStats;
	pScreen->AddArea(m_pCredits, rcArea, 0);

	//	Add the cargo space label

	pText = new CGTextArea;
	pText->SetText(CONSTLIT("Cargo Space:"));
	pText->SetFont(&m_pFonts->MediumHeavyBold);
	pText->SetColor(m_pFonts->wTitleColor);

	rcArea.left = rcRect.right - g_cxCargoStats;
	rcArea.top = rcBackArea.top - STATUS_BAR_HEIGHT + cyOffset;
	rcArea.right = rcRect.right;
	rcArea.bottom = rcArea.top + g_cyStats;
	pScreen->AddArea(pText, rcArea, 0);

	//	Add the cargo space area

	m_pCargoSpace = new CGTextArea;
	m_pCargoSpace->SetFont(&m_pFonts->MediumHeavyBold);
	m_pCargoSpace->SetColor(m_pFonts->wTitleColor);

	rcArea.left = rcRect.right - g_cxCargoStats + g_cxCargoStatsLabel;
	rcArea.top = rcBackArea.top - STATUS_BAR_HEIGHT + cyOffset;
	rcArea.right = rcRect.right;
	rcArea.bottom = rcArea.top + g_cyStats;
	pScreen->AddArea(m_pCargoSpace, rcArea, 0);

	UpdateCredits();

	return NOERROR;
	}

void CDockScreen::DeleteCurrentItem (int iCount)

//	DeleteCurrentItem
//
//	Deletes the given number of items at the cursor

	{
	m_pItemListControl->DeleteAtCursor(iCount);
	ShowItem();
	}

void CDockScreen::ExecuteCancelAction (void)

//	ExecuteCancelAction
//
//	Executes the cancel action for the screen to exit the screen

	{
	int iAction;
	if (m_CurrentActions.FindSpecial(CDockScreenActions::specialCancel, &iAction))
		m_CurrentActions.Execute(iAction, this);
	else
		m_CurrentActions.ExecuteExitScreen();
	}

int CDockScreen::GetCounter (void)

//	GetCounter
//
//	Returns the value of the counter field

	{
	if (m_pCounter)
		return strToInt(m_pCounter->GetText(), 0, NULL);
	else
		return 0;
	}

const CItem &CDockScreen::GetCurrentItem (void)

//	GetCurrentItem
//
//	Returns the current item at the cursor

	{
	return m_pItemListControl->GetItemAtCursor();
	}

ICCItem *CDockScreen::GetCurrentListEntry (void)

//	GetCurrentListEntry
//
//	Returns the current list entry

	{
	return m_pItemListControl->GetEntryAtCursor();
	}

CG16bitImage *CDockScreen::GetDisplayCanvas (const CString &sID)

//	GetDisplayCanvas
//
//	Returns the given canvas (NULL if none found)

	{
	SDisplayControl *pControl = FindDisplayControl(sID);
	if (pControl == NULL || pControl->pArea == NULL)
		return NULL;

	if (pControl->iType != ctrlCanvas)
		return NULL;

	CGDrawArea *pCanvasControl = (CGDrawArea *)pControl->pArea;
	return &pCanvasControl->GetCanvas();
	}

CString CDockScreen::GetTextInput (void)

//	GetTextInput
//
//	Returns the value of the input field

	{
	if (m_pTextInput)
		return m_pTextInput->GetText();
	else
		return NULL_STR;
	}

bool CDockScreen::IsCurrentItemValid (void)

//	IsCurrentItemValid
//
//	Returns TRUE if current item is valid

	{
	return m_pItemListControl->IsCursorValid();
	}

bool CDockScreen::EvalBool (const CString &sCode)

//	EvalBool
//
//	Evaluates the given CodeChain code.

	{
	CCodeChainCtx Ctx;
	Ctx.SetScreen(this);

	char *pPos = sCode.GetPointer();
	ICCItem *pExp = Ctx.Link(sCode, 1, NULL);

	ICCItem *pResult = Ctx.Run(pExp);
	Ctx.Discard(pExp);

	if (pResult->IsError())
		{
		SetDescription(pResult->GetStringValue());
		kernelDebugLogMessage(pResult->GetStringValue().GetASCIIZPointer());
		}

	bool bResult = !pResult->IsNil();
	Ctx.Discard(pResult);

	return bResult;
	}

CString CDockScreen::EvalInitialPane (void)

//	EvalInitialPane
//
//	Invokes <InitialPane> code and returns the result (or "Default")

	{
	CXMLElement *pInitialPane = m_pDesc->GetContentElementByTag(INITIAL_PANE_TAG);
	if (pInitialPane)
		{
		CString sCode = pInitialPane->GetContentText(0);
		bool bError;
		CString sPane = EvalString(sCode, true, &bError);
		if (bError)
			return NULL_STR;
		else
			return sPane;
		}
	else
		return CONSTLIT("Default");
	}

CSpaceObject *CDockScreen::EvalListSource (const CString &sString)

//	EvalListSource
//
//	Returns the object from which we should display items

	{
	char *pPos = sString.GetPointer();

	//	See if we need to evaluate

	if (*pPos == '=')
		{
		CCodeChainCtx Ctx;
		Ctx.SetScreen(this);

		ICCItem *pExp = Ctx.Link(sString, 1, NULL);

		ICCItem *pResult = Ctx.Run(pExp);
		Ctx.Discard(pExp);

		if (pResult->IsError())
			{
			CString sError = pResult->GetStringValue();

			SetDescription(sError);
			kernelDebugLogMessage(sError.GetASCIIZPointer());

			Ctx.Discard(pResult);
			return NULL;
			}

		//	Convert to an object pointer

		CSpaceObject *pSource;
		if (strEquals(pResult->GetStringValue(), DATA_FROM_PLAYER))
			pSource = m_pPlayer->GetShip();
		else if (strEquals(pResult->GetStringValue(), DATA_FROM_STATION)
				|| strEquals(pResult->GetStringValue(), DATA_FROM_SOURCE))
			pSource = m_pLocation;
		else
			pSource = Ctx.AsSpaceObject(pResult);

		Ctx.Discard(pResult);
		return pSource;
		}

	//	Otherwise, compare to constants

	else if (strEquals(sString, DATA_FROM_PLAYER))
		return m_pPlayer->GetShip();
	else
		return m_pLocation;
	}

CString CDockScreen::EvalString (const CString &sString, bool bPlain, bool *retbError)

//	EvalString
//
//	Evaluates a string using CodeChain. A string that begins
//	with an equals sign indicates a CodeChain expression.

	{
	if (retbError)
		*retbError = false;

	CCodeChain &CC = m_pUniv->GetCC();
	char *pPos = sString.GetPointer();

	if (bPlain || *pPos == '=')
		{
		CCodeChainCtx Ctx;
		Ctx.SetScreen(this);

		ICCItem *pExp = Ctx.Link(sString, (bPlain ? 0 : 1), NULL);

		ICCItem *pResult = Ctx.Run(pExp);
		Ctx.Discard(pExp);

		if (pResult->IsError())
			{
			CString sError = pResult->GetStringValue();

			SetDescription(sError);
			kernelDebugLogMessage(sError.GetASCIIZPointer());
			if (retbError)
				*retbError = true;
			}

		//	Note: We use GetStringValue instead of Unlink because we don't
		//	want to preserve CC semantics (e.g., we don't need strings to
		//	be quoted).

		CString sResult = pResult->GetStringValue();
		Ctx.Discard(pResult);

		return sResult;
		}
	else
		return strCEscapeCodes(sString);
	}

CDockScreen::SDisplayControl *CDockScreen::FindDisplayControl (const CString &sID)

//	FindDisplayControl
//
//	Returns the display control with the given ID (or NULL if not found)

	{
	int i;

	for (i = 0; i < m_Controls.GetCount(); i++)
		{
		if (strEquals(sID, m_Controls[i].sID))
			return &m_Controls[i];
		}

	return NULL;
	}

void CDockScreen::HandleChar (char chChar)

//	HandleChar
//
//	Handle char events

	{
	//	Deal with input fields

	if (m_pTextInput)
		{
		if (chChar >= ' ' && chChar <= '~')
			{
			CString sText = m_pTextInput->GetText();
			sText.Append(CString(&chChar, 1));
			m_pTextInput->SetText(sText);
			m_pTextInput->SetCursor(0, sText.GetLength());
			return;
			}
		}
	else if (m_pCounter)
		{
		switch (chChar)
			{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (m_bReplaceCounter)
					{
					m_pCounter->SetText(strFromInt(chChar - '0', false));
					m_bReplaceCounter = false;
					}
				else
					{
					CString sCounter = m_pCounter->GetText();
					sCounter.Append(strFromInt(chChar - '0', false));
					m_pCounter->SetText(sCounter);
					}
				return;
			}
		}

	//	Deal with accelerators
	//	Check to see if one of the key matches one of the accelerators

	int iAction;
	if (m_CurrentActions.FindByKey(CString(&chChar, 1), &iAction))
		m_CurrentActions.Execute(iAction, this);
	}

void CDockScreen::HandleKeyDown (int iVirtKey)

//	HandleKeyDown
//
//	Handle key down events

	{
	switch (iVirtKey)
		{
		case VK_UP:
		case VK_LEFT:
			if (m_pItemListControl)
				Action(g_PrevActionID);
			else
				{
				int iAction;
				if (m_CurrentActions.FindSpecial(CDockScreenActions::specialPrevKey, &iAction))
					m_CurrentActions.Execute(iAction, this);
				}
			break;

		case VK_DOWN:
		case VK_RIGHT:
			if (m_pItemListControl)
				Action(g_NextActionID);
			else
				{
				int iAction;
				if (m_CurrentActions.FindSpecial(CDockScreenActions::specialNextKey, &iAction))
					m_CurrentActions.Execute(iAction, this);
				}
			break;

		case VK_PRIOR:
			if (m_pItemListControl)
				Action(PICKER_ID, ITEM_LIST_AREA_PAGE_UP_ACTION);
			break;

		case VK_NEXT:
			if (m_pItemListControl)
				Action(PICKER_ID, ITEM_LIST_AREA_PAGE_DOWN_ACTION);
			break;

		case VK_BACK:
			{
			if (m_pTextInput)
				{
				CString sText = m_pTextInput->GetText();
				if (sText.GetLength() > 1)
					{
					sText = strSubString(sText, 0, sText.GetLength() - 1);
					m_pTextInput->SetText(sText);
					m_pTextInput->SetCursor(0, sText.GetLength());
					}
				else
					{
					m_pTextInput->SetText(NULL_STR);
					m_pTextInput->SetCursor(0, 0);
					}
				}
			else if (m_pCounter)
				{
				CString sCounter = m_pCounter->GetText();
				if (sCounter.GetLength() > 1)
					{
					m_pCounter->SetText(strSubString(sCounter, 0, sCounter.GetLength() - 1));
					m_bReplaceCounter = false;
					}
				else
					{
					m_pCounter->SetText(CONSTLIT("0"));
					m_bReplaceCounter = true;
					}
				}
			break;
			}

		case VK_ESCAPE:
			ExecuteCancelAction();
			break;

		case VK_RETURN:
			{
			int iAction;
			if (m_CurrentActions.FindSpecial(CDockScreenActions::specialDefault, &iAction))
				m_CurrentActions.Execute(iAction, this);
			else if (m_CurrentActions.GetCount() == 1)
				m_CurrentActions.Execute(0, this);
			break;
			}
		}
	}

ALERROR CDockScreen::InitCodeChain (CTranscendenceWnd *pTrans, CSpaceObject *pStation)

//	InitCodeChain
//
//	Initializes CodeChain language

	{
	CCodeChain &CC = m_pUniv->GetCC();

	//	Define some globals

	CC.DefineGlobalInteger(CONSTLIT("gSource"), (int)pStation);
	CC.DefineGlobalInteger(CONSTLIT("gScreen"), (int)this);

	return NOERROR;
	}

ALERROR CDockScreen::InitCustomList (void)

//	InitCustomList
//
//	Initializes the custom list for this screen

	{
	//	Get the list element

	CXMLElement *pListData = m_pDesc->GetContentElementByTag(LIST_TAG);
	if (pListData == NULL)
		return ERR_FAIL;

	//	See if we define a custom row height

	CString sRowHeight;
	if (pListData->FindAttribute(ROW_HEIGHT_ATTRIB, &sRowHeight))
		{
		bool bError;
		int cyRow = strToInt(EvalString(sRowHeight, false, &bError), -1);
		if (!bError && cyRow > 0)
			m_pItemListControl->SetRowHeight(cyRow);
		}

	//	Get the list to show

	CCodeChain &CC = m_pUniv->GetCC();
	ICCItem *pExp = CC.Link(pListData->GetContentText(0), 0, NULL);

	//	Evaluate the function

	CCodeChainCtx Ctx;
	Ctx.SetScreen(this);

	ICCItem *pResult = Ctx.Run(pExp);
	Ctx.Discard(pExp);

	if (pResult->IsError())
		{
		CString sError = pResult->GetStringValue();

		SetDescription(sError);
		kernelDebugLogMessage(sError.GetASCIIZPointer());

		return NOERROR;
		}

	//	Set this expression as the list

	m_pItemListControl->SetList(CC, pResult);
	Ctx.Discard(pResult);

	//	Position the cursor on the next relevant item

	SelectNextItem();

	//	Give the screen a chance to start at a different item (other
	//	than the first)

	CString sInitialItemFunc = pListData->GetAttribute(INITIAL_ITEM_ATTRIB);
	if (!sInitialItemFunc.IsBlank())
		{
		bool bMore = IsCurrentItemValid();
		while (bMore && !EvalBool(sInitialItemFunc))
			SelectNextItem(&bMore);
		}

	return NOERROR;
	}

ALERROR CDockScreen::InitDisplay (CXMLElement *pDisplayDesc, AGScreen *pScreen, const RECT &rcScreen)

//	InitDisplay
//
//	Initializes display controls

	{
	int i;
	CCodeChain &CC = m_pUniv->GetCC();

	ASSERT(m_Controls.GetCount() == 0);

	//	Init

	m_pDisplayInitialize = NULL;
	if (!pDisplayDesc->FindAttributeBool(ANIMATE_ATTRIB, &m_bDisplayAnimate))
		m_bDisplayAnimate = true;

	//	Allocate the controls

	int iControlCount = pDisplayDesc->GetContentElementCount();
	if (iControlCount == 0)
		return NOERROR;

	//	Compute the canvas rect for the controls (relative to pScreen)

	RECT rcCanvas;
	rcCanvas.left = 0;
	rcCanvas.top = (RectHeight(rcScreen) - g_cyBackground) / 2;
	rcCanvas.right = rcCanvas.left + DESC_PANE_X;
	rcCanvas.bottom = rcCanvas.top + g_cyBackground;

	//	Create each control

	for (i = 0; i < iControlCount; i++)
		{
		CXMLElement *pControlDesc = pDisplayDesc->GetContentElement(i);

		//	If this is the initialize tag, remember it

		if (strEquals(pControlDesc->GetTag(), INITIALIZE_TAG)
				|| strEquals(pControlDesc->GetTag(), ON_DISPLAY_INIT_TAG))
			m_pDisplayInitialize = pControlDesc;

		//	Otherwise, Add the control

		else
			AddDisplayControl(pControlDesc, pScreen, NULL, rcCanvas);
		}

	return NOERROR;
	}

void CDockScreen::InitDisplayControlRect (CXMLElement *pDesc, const RECT &rcFrame, RECT *retrcRect)

//	InitDisplayControlRect
//
//	Initializes the RECT for a display control

	{
	RECT rcRect;

	rcRect.left = pDesc->GetAttributeInteger(LEFT_ATTRIB);
	rcRect.top = pDesc->GetAttributeInteger(TOP_ATTRIB);
	rcRect.right = pDesc->GetAttributeInteger(RIGHT_ATTRIB);
	rcRect.bottom = pDesc->GetAttributeInteger(BOTTOM_ATTRIB);

	int xCenter;
	bool bXCenter = pDesc->FindAttributeInteger(CENTER_ATTRIB, &xCenter);

	int yCenter;
	bool bYCenter = pDesc->FindAttributeInteger(VCENTER_ATTRIB, &yCenter);

	if (rcRect.right < 0)
		rcRect.right = RectWidth(rcFrame) + rcRect.right;

	if (rcRect.bottom < 0)
		rcRect.bottom = RectHeight(rcFrame) + rcRect.bottom;

	int cxWidth = pDesc->GetAttributeInteger(WIDTH_ATTRIB);
	if (cxWidth)
		{
		if (bXCenter)
			{
			rcRect.left = xCenter + ((RectWidth(rcFrame) - cxWidth) / 2);
			rcRect.right = rcRect.left + cxWidth;
			}
		else if (rcRect.right == 0)
			rcRect.right = rcRect.left + cxWidth;
		else if (rcRect.left == 0)
			rcRect.left = rcRect.right - cxWidth;
		}
	else
		{
		if (rcRect.right < rcRect.left)
			rcRect.right = RectWidth(rcFrame);
		}

	int cyHeight = pDesc->GetAttributeInteger(HEIGHT_ATTRIB);
	if (cyHeight)
		{
		if (bYCenter)
			{
			rcRect.top = yCenter + ((RectHeight(rcFrame) - cyHeight) / 2);
			rcRect.bottom = rcRect.top + cyHeight;
			}
		else if (rcRect.bottom == 0)
			rcRect.bottom = rcRect.top + cyHeight;
		else if (rcRect.top == 0)
			rcRect.top = rcRect.bottom - cyHeight;
		}
	else
		{
		if (rcRect.bottom < rcRect.top)
			rcRect.bottom = RectHeight(rcFrame);
		}

	//	If we're still 0-size, then we take up the whole frame

	if (rcRect.left == 0 && rcRect.top == 0
			&& rcRect.top == 0 && rcRect.bottom == 0)
		{
		rcRect.right = RectWidth(rcFrame);
		rcRect.bottom = RectHeight(rcFrame);
		}

	//	Otherwise, make sure we don't exceed the canvas

	else
		{
		rcRect.left = Min(RectWidth(rcFrame), Max(0, (int)rcRect.left));
		rcRect.right = Min(RectWidth(rcFrame), (int)Max(rcRect.left, rcRect.right));
		rcRect.top = Min(RectHeight(rcFrame), Max(0, (int)rcRect.top));
		rcRect.bottom = Min(RectHeight(rcFrame), (int)Max(rcRect.top, rcRect.bottom));
		}

	//	RECT is in absolute coordinates

	rcRect.left += rcFrame.left;
	rcRect.right += rcFrame.left;
	rcRect.top += rcFrame.top;
	rcRect.bottom += rcFrame.top;

	//	Done

	*retrcRect = rcRect;
	}

ALERROR CDockScreen::InitItemList (void)

//	InitItemList
//
//	Initializes the item list for this screen

	{
	CSpaceObject *pListSource;

	//	Get the list options element

	CXMLElement *pOptions = m_pDesc->GetContentElementByTag(CONSTLIT(g_ListOptionsTag));
	if (pOptions == NULL)
		return ERR_FAIL;

	//	Figure out where to get the data from: either the station
	//	or the player's ship.

	pListSource = EvalListSource(pOptions->GetAttribute(CONSTLIT(g_DataFromAttrib)));
	if (pListSource == NULL)
		return ERR_FAIL;

	//	Set the list control

	m_pItemListControl->SetList(pListSource);

	//	Initialize flags that control what items we will show

	CItem::ParseCriteria(EvalString(pOptions->GetAttribute(LIST_ATTRIB)), &m_ItemCriteria);
	m_pItemListControl->SetFilter(m_ItemCriteria);

	//	If we have content, then eval the function (note that this might
	//	re-enter and set the filter)

	CString sCode = pOptions->GetContentText(0);
	if (!sCode.IsBlank())
		EvalString(sCode, true);

	//	Position the cursor on the next relevant item

	SelectNextItem();

	//	Give the screen a chance to start at a different item (other
	//	than the first)

	CString sInitialItemFunc = pOptions->GetAttribute(INITIAL_ITEM_ATTRIB);
	if (!sInitialItemFunc.IsBlank())
		{
		bool bMore = IsCurrentItemValid();
		while (bMore && !EvalBool(sInitialItemFunc))
			SelectNextItem(&bMore);
		}

	return NOERROR;
	}

ALERROR CDockScreen::InitScreen (HWND hWnd, 
								 RECT &rcRect, 
								 CSpaceObject *pLocation, 
								 CXMLElement *pLocalScreens,
								 CXMLElement *pDesc, 
								 const CString &sPane,
								 CString *retsPane,
								 AGScreen **retpScreen)

//	InitScreen
//
//	Initializes the docking screen. Returns an AGScreen object
//	that has been initialized appropriately.

	{
	ALERROR error;

	//	Make sure we clean up first

	CleanUpScreen();
	m_pFonts = &g_pTrans->GetFonts();

	//	Init some variables

	m_pLocation = pLocation;
	m_pLocalScreens = pLocalScreens;
	m_pPlayer = g_pTrans->GetPlayer();
	m_pDesc = pDesc;
	m_pUniv = g_pUniverse;

	//	Initialize CodeChain processor

	if (error = InitCodeChain(g_pTrans, m_pLocation))
		return error;

	//	Give the screen a chance to re-direct

	CXMLElement *pOnInit = m_pDesc->GetContentElementByTag(ON_SCREEN_INIT_TAG);
	if (pOnInit == NULL)
		pOnInit = m_pDesc->GetContentElementByTag(ON_INIT_TAG);

	if (pOnInit)
		{
		m_bInOnInit = true;

		CString sCode = pOnInit->GetContentText(0);
		EvalString(sCode, true);

		m_bInOnInit = false;
		m_bFirstOnInit = false;

		//	If we've already got a screen set up then we don't need to
		//	continue (OnInit has navigated to a different screen).

		if (m_pScreen)
			{
			if (retsPane)
				*retsPane = NULL_STR;

			return NOERROR;
			}
		}

	m_bFirstOnInit = false;

	//	Create a new screen

	m_pScreen = new AGScreen(hWnd, rcRect);
	m_pScreen->SetController(this);

	RECT rcScreen;
	rcScreen.left = 0;
	rcScreen.top = 0;
	rcScreen.right = RectWidth(rcRect);
	rcScreen.bottom = RectHeight(rcRect);

	//	Creates the title and background controls

	if (error = CreateTitleAndBackground(m_pDesc, m_pScreen, rcScreen))
		return error;

	//	Get the list of panes for this screen

	m_pPanes = m_pDesc->GetContentElementByTag(CONSTLIT(g_PanesTag));

	//	If this is an item list screen then add the appropriate
	//	controls.

	CString sType = m_pDesc->GetAttribute(CONSTLIT(g_ScreenTypeAttrib));
	if (strEquals(sType, SCREEN_TYPE_ITEM_PICKER))
		{
		if (error = CreateItemPickerControl(m_pDesc, m_pScreen, rcScreen))
			return error;

		//	Create an item list manipulator for the items of this object

		if (error = InitItemList())
			return error;
		}
	else if (strEquals(sType, SCREEN_TYPE_CUSTOM_PICKER))
		{
		if (error = CreateItemPickerControl(m_pDesc, m_pScreen, rcScreen))
			return error;

		//	Create an item list manipulator for the items of this object

		if (error = InitCustomList())
			return error;
		}

	//	If we have a display element, then load the display controls

	CXMLElement *pDisplay = m_pDesc->GetContentElementByTag(DISPLAY_TAG);
	if (pDisplay)
		{
		if (error = InitDisplay(pDisplay, m_pScreen, rcScreen))
			return error;
		}

	//	Show the pane

	m_rcPane.left = rcScreen.right - g_cxActionsRegion;
	m_rcPane.top = (RectHeight(rcScreen) - g_cyDockScreen) / 2;
	m_rcPane.right = rcScreen.right - 8;
	m_rcPane.bottom = m_rcPane.top + g_cyBackground;

	if (!sPane.IsBlank())
		{
		ShowPane(sPane);
		if (retsPane)
			*retsPane = sPane;
		}
	else
		{
		CString sPane = EvalInitialPane();

		//	If evaluation fails due to an error, then show default

		if (sPane.IsBlank())
			sPane = CONSTLIT("Default");

		ShowPane(sPane);
		if (retsPane)
			*retsPane = sPane;
		}

	//	Done

	*retpScreen = m_pScreen;

	return NOERROR;
	}

void CDockScreen::ResetList (CSpaceObject *pLocation)

//	ResetList
//
//	Resets the display list

	{
	if (m_pItemListControl && m_pItemListControl->GetSource() == pLocation)
		{
		m_pItemListControl->ResetCursor();
		m_pItemListControl->MoveCursorForward();
		ShowItem();
		m_CurrentActions.ExecuteShowPane(EvalInitialPane());
		}
	}

void CDockScreen::ShowDisplay (bool bAnimateOnly)

//	ShowDisplay
//
//	Updates the controls on the display

	{
	int i;
	CCodeChain &CC = m_pUniv->GetCC();

	//	Run initialize

	if (m_pDisplayInitialize)
		{
		CString sCode = m_pDisplayInitialize->GetContentText(0);
		EvalString(sCode, true);
		}

	//	Set controls

	for (i = 0; i < m_Controls.GetCount(); i++)
		{
		//	If we're animating, skip controls that don't need animation

		if (bAnimateOnly && !m_Controls[i].bAnimate)
			continue;

		//	Update control based on type

		switch (m_Controls[i].iType)
			{
			case ctrlImage:
				{
				CGImageArea *pControl = (CGImageArea *)m_Controls[i].pArea;

				if (m_Controls[i].pCode)
					{
					CCodeChainCtx Ctx;
					Ctx.SetScreen(this);

					ICCItem *pResult = Ctx.Run(m_Controls[i].pCode);

					//	If we have an error, report it

					if (pResult->IsError())
						{
						if (!bAnimateOnly)
							{
							kernelDebugLogMessage(pResult->GetStringValue().GetASCIIZPointer());
							}
						Ctx.Discard(pResult);
						break;
						}

					//	The result is the image descriptor

					CG16bitImage *pImage;
					RECT rcImage;
					GetImageDescFromList(CC, pResult, &pImage, &rcImage);
					if (pImage)
						pControl->SetImage(pImage, rcImage);

					//	Done

					Ctx.Discard(pResult);
					}

				break;
				}

			case ctrlCanvas:
				{
				CGDrawArea *pControl = (CGDrawArea *)m_Controls[i].pArea;

				if (m_Controls[i].pCode)
					{
					CCodeChainCtx Ctx;
					Ctx.SetScreen(this);
					CG16bitImage *pCanvas = &pControl->GetCanvas();
					Ctx.SetCanvas(pCanvas);

					pCanvas->Fill(0, 0, pCanvas->GetWidth(), pCanvas->GetHeight(), 0);

					ICCItem *pResult = Ctx.Run(m_Controls[i].pCode);

					//	If we have an error, report it

					if (pResult->IsError())
						{
						if (!bAnimateOnly)
							{
							kernelDebugLogMessage(pResult->GetStringValue().GetASCIIZPointer());
							}

						Ctx.Discard(pResult);
						break;
						}

					//	Done

					Ctx.Discard(pResult);
					}

				break;
				}

			case ctrlNeurohack:
				{
				CGNeurohackArea *pControl = (CGNeurohackArea *)m_Controls[i].pArea;

				if (m_Controls[i].pCode)
					{
					CCodeChainCtx Ctx;
					Ctx.SetScreen(this);

					ICCItem *pResult = Ctx.Run(m_Controls[i].pCode);

					//	If we have an error, report it

					if (pResult->IsError())
						{
						if (!bAnimateOnly)
							{
							kernelDebugLogMessage(pResult->GetStringValue().GetASCIIZPointer());
							}

						Ctx.Discard(pResult);
						break;
						}

					//	The result is the data descriptor in this order:
					//
					//	Willpower
					//	Damage

					int iWillpower = (pResult->GetCount() > 0 ? pResult->GetElement(0)->GetIntegerValue() : 0);
					int iDamage = (pResult->GetCount() > 1 ? pResult->GetElement(1)->GetIntegerValue() : 0);
					pControl->SetData(iWillpower, iDamage);

					//	Done

					Ctx.Discard(pResult);
					}

				break;
				}

			case ctrlText:
				{
				CGTextArea *pControl = (CGTextArea *)m_Controls[i].pArea;

				if (m_Controls[i].pCode)
					{
					CCodeChainCtx Ctx;
					Ctx.SetScreen(this);

					ICCItem *pResult = Ctx.Run(m_Controls[i].pCode);

					//	The result is the text for the control

					pControl->SetText(pResult->GetStringValue());

					//	If we have an error, report it as well

					if (pResult->IsError())
						{
						if (!bAnimateOnly)
							{
							kernelDebugLogMessage(pResult->GetStringValue().GetASCIIZPointer());
							}
						}

					//	Done

					Ctx.Discard(pResult);
					}

				break;
				}
			}
		}
	}

void CDockScreen::ShowItem (void)

//	ShowItem
//
//	Sets the title and description for the currently selected item

	{
	m_pItemListControl->Invalidate();

	//	If we've got an installed armor segment selected, then highlight
	//	it on the armor display

	if (m_pItemListControl->IsCursorValid())
		{
		const CItem &Item = m_pItemListControl->GetItemAtCursor();
		if (Item.IsInstalled() && Item.GetType()->IsArmor())
			{
			int iSeg = Item.GetInstalled();
			g_pTrans->SelectArmor(iSeg);
			}
		else
			g_pTrans->SelectArmor(-1);
		}
	else
		g_pTrans->SelectArmor(-1);
	}

void CDockScreen::SetCounter (int iCount)

//	SetCounter
//
//	Sets the value of the counter field

	{
	if (m_pCounter)
		{
		CString sText = strFromInt(iCount, false);
		m_pCounter->SetText(sText);
		m_bReplaceCounter = true;
		}
	}

ALERROR CDockScreen::SetDisplayText (const CString &sID, const CString &sText)

//	SetDisplayText
//
//	Sets the text for a display control

	{
	SDisplayControl *pControl = FindDisplayControl(sID);
	if (pControl == NULL || pControl->pArea == NULL)
		return ERR_FAIL;

	if (pControl->iType != ctrlText)
		return ERR_FAIL;

	CGTextArea *pTextControl = (CGTextArea *)pControl->pArea;
	pTextControl->SetText(g_pTrans->ComposePlayerNameString(sText));

	//	If we're explicitly setting the text, then we cannot animate

	pControl->bAnimate = false;

	return NOERROR;
	}

void CDockScreen::SetListFilter (const CItemCriteria &Filter)

//	SetListFilter
//
//	Filters the list given the criteria

	{
	if (m_pItemListControl)
		{
		m_pItemListControl->SetFilter(Filter);
		ShowItem();

		if (!m_bInShowPane)
			m_CurrentActions.ExecuteShowPane(EvalInitialPane());
		}
	}

void CDockScreen::SetTextInput (const CString &sText)

//	SetTextInput
//
//	Sets the value of the text input field

	{
	if (m_pTextInput)
		{
		m_pTextInput->SetText(sText);
		m_pTextInput->SetCursor(0, sText.GetLength());
		}
	}

void CDockScreen::ShowPane (const CString &sName)

//	ShowPane
//
//	Shows the pane of the given name

	{
#ifdef DEBUG_STRING_LEAKS
	CString::DebugMark();
#endif

	//	If sName is blank, then it is likely due to EvalInitialPane failing

	if (sName.IsBlank())
		return;

	//	Find the pane named

	CXMLElement *pNewPane = m_pPanes->GetContentElementByTag(sName);
	if (pNewPane == NULL)
		{
		CString sError = strPatternSubst(CONSTLIT("Unable to find pane: %s"), sName);
		if (m_pCurrentPane)
			SetDescription(sError);
		kernelDebugLogMessage(sError.GetASCIIZPointer());
		return;
		}

	//	Make sure we don't recurse

	m_bInShowPane = true;

	//	Destroy the previous pane

	if (m_pCurrentFrame)
		m_pScreen->DestroyArea(m_pCurrentFrame);

	//	Create a new pane

	CGFrameArea *pFrame = new CGFrameArea;
	m_pScreen->AddArea(pFrame, m_rcPane, 0);

	//	Find the pane element

	m_pCurrentPane = pNewPane;

	//	Initialize list of actions

	CString sError;
	if (m_CurrentActions.InitFromXML(m_pCurrentPane->GetContentElementByTag(ACTIONS_TAG), &sError) != NOERROR)
		{
		sError = strPatternSubst(CONSTLIT("Pane %s: %s"), sName, sError);
		if (m_pCurrentPane)
			SetDescription(sError);
		kernelDebugLogMessage(sError.GetASCIIZPointer());
		return;
		}

	//	Are we showing the counter?

	RECT rcInput;
	bool bShowCounter = m_pCurrentPane->GetAttributeBool(CONSTLIT(g_ShowCounterAttrib));
	if (bShowCounter)
		{
		m_pCounter = new CGTextArea;
		m_pCounter->SetEditable();
		m_pCounter->SetText(CONSTLIT("0"));
		m_pCounter->SetFont(&m_pFonts->SubTitleHeavyBold);
		m_pCounter->SetColor(CG16bitImage::RGBValue(255,255,255));
		m_pCounter->SetStyles(alignCenter);

		rcInput.left = m_rcPane.left + (RectWidth(m_rcPane) - g_cxCounter) / 2;
		rcInput.right = rcInput.left + g_cxCounter;
		rcInput.top = m_rcPane.bottom - g_cyActionsRegion - g_cyCounter - 24;
		rcInput.bottom = rcInput.top + g_cyCounter;

		pFrame->AddArea(m_pCounter, rcInput, g_CounterID);

		m_bReplaceCounter = true;
		}
	else
		m_pCounter = NULL;

	//	Are we showing an input field

	bool bShowTextInput = m_pCurrentPane->GetAttributeBool(SHOW_TEXT_INPUT_ATTRIB);
	if (bShowTextInput && !bShowCounter)
		{
		m_pTextInput = new CGTextArea;
		m_pTextInput->SetEditable();
		m_pTextInput->SetFont(&m_pFonts->SubTitleHeavyBold);
		m_pTextInput->SetColor(CG16bitImage::RGBValue(255,255,255));
		m_pTextInput->SetCursor(0, 0);

		rcInput.left = m_rcPane.left + (RectWidth(m_rcPane) - TEXT_INPUT_WIDTH) / 2;
		rcInput.right = rcInput.left + TEXT_INPUT_WIDTH;
		rcInput.top = m_rcPane.bottom - g_cyActionsRegion - TEXT_INPUT_HEIGHT - 24;
		rcInput.bottom = rcInput.top + TEXT_INPUT_HEIGHT;

		pFrame->AddArea(m_pTextInput, rcInput, TEXT_INPUT_ID);
		}
	else
		m_pTextInput = NULL;

	//	Create the description

	CString sDesc = EvalString(m_pCurrentPane->GetAttribute(CONSTLIT(g_DescAttrib)));
	m_pFrameDesc = new CGTextArea;
	m_pFrameDesc->SetText(sDesc);
	m_pFrameDesc->SetFont(&m_pFonts->Large);
	m_pFrameDesc->SetColor(m_pFonts->wTextColor);
	m_pFrameDesc->SetLineSpacing(3);

	//	Justify the text

	RECT rcDesc;
	rcDesc.left = m_rcPane.left;
	rcDesc.top = m_rcPane.top + 16;
	rcDesc.right = m_rcPane.right;
	if (bShowCounter || bShowTextInput)
		rcDesc.bottom = rcInput.top;
	else
		rcDesc.bottom = m_rcPane.bottom - g_cyActionsRegion;

	pFrame->AddArea(m_pFrameDesc, rcDesc, 0);

	//	Show the currently selected item

	if (m_pItemListControl)
		{
		//	Update armor items to match the current state (the damaged flag)

		CSpaceObject *pLocation = m_pItemListControl->GetSource();
		if (pLocation)
			pLocation->UpdateArmorItems();

		//	Update the item list

		ShowItem();

		//	If this is set, don't allow the list selection to change

		m_bNoListNavigation = m_pCurrentPane->GetAttributeBool(NO_LIST_NAVIGATION_ATTRIB);
		}
	else
		{
		g_pTrans->SelectArmor(-1);
		m_bNoListNavigation = false;
		}

	//	Update the display

	if (m_Controls.GetCount() > 0)
		ShowDisplay();

	//	Done

	m_pCurrentFrame = pFrame;

	//	Evaluate the initialize element
	//	
	//	This gives the frame a chance to initialize any dynamic
	//	action buttons before we actually create the buttons.

	CXMLElement *pInit = m_pCurrentPane->GetContentElementByTag(ON_PANE_INIT_TAG);
	if (pInit == NULL)
		pInit = m_pCurrentPane->GetContentElementByTag(INITIALIZE_TAG);

	if (pInit)
		{
		CString sCode = pInit->GetContentText(0);
		EvalString(sCode, true);
		}

	//	We might have called exit inside OnPaneInit. If so, we exit

	if (m_pScreen == NULL)
		return;

	//	Allow other design types to override the pane

	CDesignType *pRoot;
	CString sScreen;
	g_pTrans->GetModel().GetScreenSession(NULL, &pRoot, &sScreen);
	g_pUniverse->FireOnGlobalPaneInit(this, pRoot, sScreen, sName);

	//	Check to see if the description is too large for the area. If so, then
	//	we shift everything down.

	int cyDesc = m_pFrameDesc->Justify(rcDesc);
	int cyRow = (g_cyAction + g_cyActionSpacing);
	int cyExtraNeeded = (cyDesc + cyRow) - RectHeight(rcDesc);
	int cyExtraSpace = 0;
	if (cyExtraNeeded > 0)
		{
		int iExtraRows = Min(AlignUp(cyExtraNeeded, cyRow) / cyRow, g_iMaxActions - m_CurrentActions.GetVisibleCount());
		if (iExtraRows > 0)
			{
			cyExtraSpace = iExtraRows * cyRow;

			rcDesc.bottom += cyExtraSpace;
			m_pFrameDesc->SetRect(rcDesc);
			}
		}

	//	Compute some metrics for the rest of the controls

	int yDescBottom = rcDesc.top + cyDesc;
	int yActionsTop = m_rcPane.bottom - g_cyActionsRegion + cyExtraSpace;
	int cyInput = yActionsTop - yDescBottom;

	//	Move the input field, if necessary

	if (m_pTextInput)
		{
		RECT rcRect;
		rcRect = m_pTextInput->GetRect();
		rcRect.top = yDescBottom + (cyInput - TEXT_INPUT_HEIGHT) / 2;
		rcRect.bottom = rcRect.top + TEXT_INPUT_HEIGHT;
		m_pTextInput->SetRect(rcRect);
		}

	//	Move the counter, if necessary

	if (m_pCounter)
		{
		RECT rcRect;
		rcRect = m_pCounter->GetRect();
		rcRect.top = yDescBottom + (cyInput - g_cyCounter) / 2;
		rcRect.bottom = rcRect.top + g_cyCounter;
		m_pCounter->SetRect(rcRect);
		}

	//	Create the action buttons (deals with extra space above and show/hide)

	RECT rcActions;
	rcActions.left = m_rcPane.left;
	rcActions.top = yActionsTop;
	rcActions.right = m_rcPane.right;
	rcActions.bottom = m_rcPane.bottom;

	m_CurrentActions.CreateButtons(pFrame, g_FirstActionID, rcActions);

	//	Update screen

	UpdateCredits();
	m_bInShowPane = false;

#ifdef DEBUG_STRING_LEAKS
	CString::DebugOutputLeakedStrings();
#endif
	}

void CDockScreen::SelectNextItem (bool *retbMore)

//	SelectNextItem
//
//	Selects the next item in the list

	{
	bool bMore = m_pItemListControl->MoveCursorForward();

	if (retbMore)
		*retbMore = bMore;
	}

void CDockScreen::SelectPrevItem (void)

//	SelectPrevItem
//
//	Selects the previous item in the list

	{
	m_pItemListControl->MoveCursorBack();
	}

void CDockScreen::SetDescription (const CString &sDesc)

//	SetDescription
//
//	Sets the description of the current pane

	{
	if (m_pFrameDesc)
		m_pFrameDesc->SetText(g_pTrans->ComposePlayerNameString(sDesc));
	}

void CDockScreen::Update (int iTick)

//	Update
//
//	Updates the display

	{
	if (m_pScreen)
		m_pScreen->Update();

	if (m_bDisplayAnimate && (iTick % 10) == 0)
		{
		ShowDisplay(true);
		}
	}

void CDockScreen::UpdateCredits (void)

//	UpdateCredits
//
//	Updates the display of credits

	{
	//	Money

	CEconomyType *pEconomy = m_pLocation->GetDefaultEconomy();
	m_pCredits->SetText(strPatternSubst(CONSTLIT("%s: %d"), 
			strCapitalize(pEconomy->GetCurrencyNamePlural()),
			(int)m_pPlayer->GetCredits(pEconomy->GetUNID())
			));

	//	Cargo space

	m_pCargoSpace->SetText(strPatternSubst("%d ton%p", m_pPlayer->GetCargoSpace()));
	}
