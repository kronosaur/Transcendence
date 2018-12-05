//	CDockPane.cpp
//
//	CDockPane class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define ACTIONS_TAG					CONSTLIT("Actions")
#define CONTROLS_TAG				CONSTLIT("Controls")
#define COUNTER_TAG					CONSTLIT("Counter")
#define INITIALIZE_TAG				CONSTLIT("Initialize")
#define ITEM_DISPLAY_TAG			CONSTLIT("ItemDisplay")
#define ITEM_LIST_DISPLAY_TAG		CONSTLIT("ItemListDisplay")
#define ON_PANE_INIT_TAG			CONSTLIT("OnPaneInit")
#define TEXT_TAG					CONSTLIT("Text")
#define TEXT_INPUT_TAG				CONSTLIT("TextInput")

#define DESC_ATTRIB					CONSTLIT("desc")
#define DESC_ID_ATTRIB				CONSTLIT("descID")
#define ID_ATTRIB					CONSTLIT("id")
#define LAYOUT_ATTRIB				CONSTLIT("layout")
#define SHOW_COUNTER_ATTRIB			CONSTLIT("showCounter")
#define SHOW_TEXT_INPUT_ATTRIB		CONSTLIT("showTextInput")
#define STYLE_ATTRIB				CONSTLIT("style")

#define DEFAULT_DESC_ID				CONSTLIT("desc")
#define DEFAULT_COUNTER_ID			CONSTLIT("counter")
#define DEFAULT_TEXT_INPUT_ID		CONSTLIT("textInput")

#define LAYOUT_BOTTOM_BAR			CONSTLIT("bottomBar")
#define LAYOUT_LEFT					CONSTLIT("left")
#define LAYOUT_RIGHT				CONSTLIT("right")

#define STYLE_DEFAULT				CONSTLIT("default")
#define STYLE_WARNING				CONSTLIT("warning")

const int MAX_ACTIONS =				8;
const int FIRST_ACTION_ID =			100;
const int LAST_ACTION_ID =			199;

const int CONTROL_BORDER_RADIUS =	4;
const int CONTROL_INNER_PADDING =	8;
const int CONTROL_PADDING_BOTTOM =	24;
const int CONTROL_PADDING_MIN =		4;
const int ACTION_MARGIN_Y =			46;

const int COUNTER_ID =				204;
const int COUNTER_WIDTH =			128;
const int COUNTER_HEIGHT =			40;
const int COUNTER_PADDING_BOTTOM =	24;

const int DESC_PADDING_BOTTOM =		24;
const int DESC_HEIGHT_GRANULARITY =	48;

const int TEXT_INPUT_ID =			207;
const int TEXT_INPUT_WIDTH =		380;
const int TEXT_INPUT_HEIGHT	=		40;
const int TEXT_INPUT_PADDING_BOTTOM = 24;

const int PANE_PADDING_TOP =		24;
const int PANE_PADDING_EXTRA =		0;

const int STD_PANE_WIDTH =			392;
const int STD_PANE_PADDING_LEFT =	8;
const int STD_PANE_PADDING_RIGHT =	8;
const int STD_PANE_PADDING_BOTTOM =	24;
const int THIN_PANE_HEIGHT =		100;

CDockPane::CDockPane (void) :
		m_pPaneDesc(NULL),
		m_iLayout(layoutNone),
		m_pContainer(NULL),
		m_bInShowPane(false),
		m_bInExecuteAction(false),
		m_bDescError(false)

//	CDockPane constructor

	{
	}

CDockPane::~CDockPane (void)

//	CDockPane destructor

	{
	CleanUp();
	}

void CDockPane::CleanUp (AGScreen *pScreen)

//	CleanUp
//
//	Clean up the pane

	{
	//	If we're still inside a screen, clean up our container

	if (pScreen && m_pContainer)
		pScreen->DestroyArea(m_pContainer);

	//	Clean Up

	m_Actions.CleanUp();
	m_Controls.DeleteAll();

	//	No need to free these fields because we don't own them

	m_pDockScreen = NULL;
	m_pPaneDesc = NULL;
	m_pContainer = NULL;
	m_bInShowPane = false;
	m_bInExecuteAction = false;
	m_sDeferredShowPane = NULL_STR;
	}

void CDockPane::CreateControl (EControlTypes iType, const CString &sID, const CString &sStyle, RECT rcPane)

//	CreateControl
//
//	Creates a control of the given type.

	{
	const CVisualPalette &VI = g_pHI->GetVisuals();
    const CDockScreenVisuals &DockScreenVisuals = m_pDockScreen->GetVisuals();

	switch (iType)
		{
		case controlCounter:
			{
			SControl *pControl = m_Controls.Insert();
			pControl->iType = iType;
			pControl->sID = sID;
			pControl->cyHeight = COUNTER_HEIGHT;
			pControl->cyMinHeight = pControl->cyHeight;
			pControl->cyMaxHeight = pControl->cyHeight;

			CGTextArea *pTextArea = new CGTextArea;
			pTextArea->SetEditable();
			pTextArea->SetText(CONSTLIT("0"));
			pTextArea->SetFont(&VI.GetFont(fontSubTitleHeavyBold));
			pTextArea->SetColor(CG32bitPixel(255,255,255));
			pTextArea->SetStyles(alignCenter);

			RECT rcInput;
			rcInput.left = rcPane.left + (RectWidth(rcPane) - COUNTER_WIDTH) / 2;
			rcInput.right = rcInput.left + COUNTER_WIDTH;

			//	Height doesn't matter for now; we recalc later
			rcInput.top = rcPane.top;
			rcInput.bottom = rcInput.top + COUNTER_HEIGHT;

			pControl->pArea = pTextArea;
			m_pContainer->AddArea(pControl->pArea, rcInput, COUNTER_ID);

			pControl->bReplaceInput = true;
			break;
			}

		case controlDesc:
			{
			SControl *pControl = m_Controls.Insert();
			pControl->iType = iType;
			pControl->sID = sID;
			pControl->cyHeight = 0;
			pControl->cyMinHeight = 0;
			pControl->cyMaxHeight = 0;

			//	Choose font and colors based on style

			SControlStyle Style;
			GetControlStyle(sStyle, &Style);

			CGTextArea *pTextArea = new CGTextArea;
			pTextArea->SetFont(Style.pTextFont);
			pTextArea->SetColor(Style.TextColor);
			pTextArea->SetPadding(CONTROL_INNER_PADDING);
			pTextArea->SetBorderRadius(CONTROL_BORDER_RADIUS);
			pTextArea->SetBackColor(Style.BackColor);
			pTextArea->SetLineSpacing(6);
			pTextArea->SetFontTable(&VI);

			pControl->pArea = pTextArea;
			m_pContainer->AddArea(pControl->pArea, rcPane, 0);
			break;
			}

		case controlItemDisplay:
			{
			SControl *pControl = m_Controls.Insert();
			pControl->iType = iType;
			pControl->sID = sID;
			pControl->cyHeight = 0;
			pControl->cyMinHeight = 0;
			pControl->cyMaxHeight = 0;

			CGItemDisplayArea *pItemDisplayArea = new CGItemDisplayArea;
            pItemDisplayArea->SetColor(DockScreenVisuals.GetTitleTextColor());
            pItemDisplayArea->SetBackColor(DockScreenVisuals.GetTextBackgroundColor());

			pControl->pArea = pItemDisplayArea;
			m_pContainer->AddArea(pControl->pArea, rcPane, 0);
			break;
			}

		case controlItemListDisplay:
			{
			SControl *pControl = m_Controls.Insert();
			pControl->iType = iType;
			pControl->sID = sID;
			pControl->cyHeight = 0;
			pControl->cyMinHeight = 0;
			pControl->cyMaxHeight = 0;

			CGItemListDisplayArea *pItemDisplayArea = new CGItemListDisplayArea;
            pItemDisplayArea->SetColor(DockScreenVisuals.GetTitleTextColor());
            pItemDisplayArea->SetBackColor(DockScreenVisuals.GetTextBackgroundColor());

			pControl->pArea = pItemDisplayArea;
			m_pContainer->AddArea(pControl->pArea, rcPane, 0);
			break;
			}

		case controlTextInput:
			{
			SControl *pControl = m_Controls.Insert();
			pControl->iType = iType;
			pControl->sID = sID;
			pControl->cyHeight = TEXT_INPUT_HEIGHT;
			pControl->cyMinHeight = pControl->cyHeight;
			pControl->cyMaxHeight = pControl->cyHeight;

			CGTextArea *pTextArea = new CGTextArea;
			pTextArea->SetEditable();
			pTextArea->SetFont(&VI.GetFont(fontSubTitleHeavyBold));
			pTextArea->SetColor(CG32bitPixel(255,255,255));
			pTextArea->SetCursor(0, 0);

			RECT rcInput;
			rcInput.left = rcPane.left + (RectWidth(rcPane) - TEXT_INPUT_WIDTH) / 2;
			rcInput.right = rcInput.left + TEXT_INPUT_WIDTH;
			rcInput.top = rcPane.top;
			rcInput.bottom = rcInput.top + TEXT_INPUT_HEIGHT;

			pControl->pArea = pTextArea;
			m_pContainer->AddArea(pControl->pArea, rcInput, TEXT_INPUT_ID);
			break;
			}

		default:
			ASSERT(false);
		}
	}

ALERROR CDockPane::CreateControls (RECT rcPane, CString *retsError)

//	CreateControls
//
//	Creates controls based on the pane descriptor. We assume that m_pContainer has
//	already been created and is empty.

	{
	int i;

	//	If there is a <Controls> element then use that to figure out what to
	//	create.

	CXMLElement *pControls = m_pPaneDesc->GetContentElementByTag(CONTROLS_TAG);
	if (pControls)
		{
        bool bDescCreated = false;

		for (i = 0; i < pControls->GetContentElementCount(); i++)
			{
			CXMLElement *pControlDef = pControls->GetContentElement(i);

			//	Figure out the type

			EControlTypes iType;
			if (strEquals(pControlDef->GetTag(), COUNTER_TAG))
				iType = controlCounter;
			else if (strEquals(pControlDef->GetTag(), ITEM_DISPLAY_TAG))
				iType = controlItemDisplay;
			else if (strEquals(pControlDef->GetTag(), ITEM_LIST_DISPLAY_TAG))
				iType = controlItemListDisplay;
			else if (strEquals(pControlDef->GetTag(), TEXT_TAG))
				iType = controlDesc;
			else if (strEquals(pControlDef->GetTag(), TEXT_INPUT_TAG))
				iType = controlTextInput;
			else
				{
				*retsError = strPatternSubst(CONSTLIT("Unknown control element: <%s>."), pControlDef->GetTag());
				return ERR_FAIL;
				}

			//	Get the ID

			CString sID;
			if (!pControlDef->FindAttribute(ID_ATTRIB, &sID))
				{
				*retsError = strPatternSubst(CONSTLIT("Missing ID attrib for control element: <%s>."), pControlDef->GetTag());
				return ERR_FAIL;
				}

			CString sStyle;
			if (!pControlDef->FindAttribute(STYLE_ATTRIB, &sStyle))
				sStyle = STYLE_DEFAULT;

            //  Keep track of default controls created

            if (strEquals(sID, DEFAULT_DESC_ID))
                bDescCreated = true;

			//	Create the control

			CreateControl(iType, sID, sStyle, rcPane);
			}

        //  If we don't have a default description control, create it. NOTE: It 
        //  is safe to create controls that we never use, since empty ones get
        //  collapsed.

        if (!bDescCreated)
            CreateControl(controlDesc, DEFAULT_DESC_ID, STYLE_DEFAULT, rcPane);
		}

	//	Otherwise we create default controls

	else
		{
		//	Create the text description control

		CreateControl(controlDesc, DEFAULT_DESC_ID, STYLE_DEFAULT, rcPane);

		//	Create counter or input fields

		if (m_pPaneDesc->GetAttributeBool(SHOW_COUNTER_ATTRIB))
			CreateControl(controlCounter, DEFAULT_COUNTER_ID, STYLE_DEFAULT, rcPane);
		else if (m_pPaneDesc->GetAttributeBool(SHOW_TEXT_INPUT_ATTRIB))
			CreateControl(controlTextInput, DEFAULT_TEXT_INPUT_ID, STYLE_DEFAULT, rcPane);
		}

	return NOERROR;
	}

void CDockPane::ExecuteAction (int iAction)

//	ExecuteAction
//
//	Executes the given action.

	{
	g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));

	//	Set up some context so we deal with re-entrancy issues.

	m_bInExecuteAction = true;
	m_sDeferredShowPane = NULL_STR;

	//	Execute

	m_Actions.Execute(iAction, m_pDockScreen);

	//	If inside the action we changed the object (e.g., deleted an item) then
	//	we might need to reload the pane. If so, we do it now.

	m_bInExecuteAction = false;
	if (!m_sDeferredShowPane.IsBlank())
		{
		m_Actions.ExecuteShowPane(m_sDeferredShowPane);
		m_sDeferredShowPane = NULL_STR;
		}
	}

void CDockPane::ExecuteCancelAction (void)

//	ExecuteCancelAction
//
//	Cancel

	{
	int iAction;
	if (m_Actions.FindSpecial(CLanguage::specialCancel, &iAction))
		ExecuteAction(iAction);
	else
		{
		m_Actions.ExecuteExitScreen();
		g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
		}
	}

void CDockPane::ExecuteShowPane (const CString &sPane, bool bDeferIfNecessary)

//	ExecuteShowPane
//
//	Shows the pane.
	
	{
	//	If we're already inside ShowPane, then we don't need to do anything.
	//	This can happen if we're inside of OnPaneInit and we do something that
	//	might normally force us to recalc the pane (such as delete an item).

	if (m_bInShowPane)
		return;

	//	If we're executing an action, and we've been asked to defer this reload,
	//	then we defer.

	if (m_bInExecuteAction && bDeferIfNecessary)
		{
		m_sDeferredShowPane = sPane;
		return;
		}

	//	Do it.

	m_Actions.ExecuteShowPane(sPane);
	}

bool CDockPane::FindControl (const CString &sID, SControl **retpControl) const

//	FindControl
//
//	Looks for the control by ID. Returns FALSE if not found.

	{
	int i;

	for (i = 0; i < m_Controls.GetCount(); i++)
		if (strEquals(sID, m_Controls[i].sID))
			{
			if (retpControl)
				*retpControl = &m_Controls[i];
			return true;
			}

	return false;
	}

CDockPane::SControl *CDockPane::GetControlByType (EControlTypes iType) const

//	GetControlByType
//
//	Returns the control entry

	{
	int i;

    //  If we're looking for the desc control, look by default ID also, since
    //  we might have multiple descriptor controls.

    SControl *pControl = NULL;
    switch (iType)
        {
        case controlDesc:
            {
            if (FindControl(DEFAULT_DESC_ID, &pControl))
                return pControl;
            break;
            }
        }

    //  Look for the first control that matches the type

	for (i = 0; i < m_Controls.GetCount(); i++)
		if (m_Controls[i].iType == iType)
			return &m_Controls[i];

	return NULL;
	}

int CDockPane::GetCounterValue (void) const

//	GetCounterValue
//
//	Returns the value of the counter.

	{
	CGTextArea *pCounter;
	if (pCounter = GetTextControlByType(controlCounter))
		{
		int iValue = strToInt(pCounter->GetText(), 0, NULL);

		//	Counter values are always positive.

		return Max(0, iValue);
		}
	else
		return 0;
	}

const CString &CDockPane::GetDescriptionString (void) const

//	GetDescriptionString
//
//	Returns the current description

	{
	CGTextArea *pDesc = GetTextControlByType(controlDesc);
	if (pDesc == NULL)
		return NULL_STR;

	return m_sDesc;
	}

void CDockPane::GetControlStyle (const CString &sStyle, SControlStyle *retStyle) const

//	GetControlStyle
//
//	Returns the style

	{
	const CVisualPalette &VI = g_pHI->GetVisuals();
    const CDockScreenVisuals &DockScreenVisuals = m_pDockScreen->GetVisuals();

	if (strEquals(sStyle, STYLE_WARNING))
		{
		retStyle->pTextFont = &VI.GetFont(fontLarge);
		retStyle->TextColor = VI.GetColor(colorTextWarningMsg);
		retStyle->BackColor = VI.GetColor(colorAreaWarningMsg);
		}
	else
		{
		retStyle->pTextFont = &VI.GetFont(fontLarge);
        retStyle->TextColor = DockScreenVisuals.GetTextColor();
        retStyle->BackColor = DockScreenVisuals.GetTextBackgroundColor();
		}
	}

CGTextArea *CDockPane::GetTextControlByType (EControlTypes iType) const

//	GetTextControlByType
//
//	Returns the control by type

	{
	switch (iType)
		{
		case controlCounter:
		case controlDesc:
		case controlTextInput:
            SControl *pControl = GetControlByType(iType);
            if (pControl == NULL)
                return NULL;

            return pControl->AsTextArea();
		}

	return NULL;
	}

CString CDockPane::GetTextInputValue (void) const

//	GetTextInputValue
//
//	Returns the current value of the input control

	{
	CGTextArea *pControl;
	if (pControl = GetTextControlByType(controlTextInput))
		return pControl->GetText();
	else
		return NULL_STR;
	}

bool CDockPane::HandleAction (DWORD dwTag, DWORD dwData)

//	HandleAction
//
//	Handles an action. Returns FALSE if we did not handle it.

	{
	if (dwTag >= FIRST_ACTION_ID && dwTag <= LAST_ACTION_ID)
		{
		int iAction = (dwTag - FIRST_ACTION_ID);
		ExecuteAction(iAction);
		return true;
		}
	else
		return false;
	}

bool CDockPane::HandleChar (char chChar)

//	HandleChar
//
//	Handle a character

	{
	//	Deal with input fields

	SControl *pControl;
	if (pControl = GetControlByType(controlTextInput))
		{
		CGTextArea *pTextArea = pControl->AsTextArea();
		if (chChar >= ' ' && chChar <= '~')
			{
			CString sText = pTextArea->GetText();
			sText.Append(CString(&chChar, 1));
			pTextArea->SetText(sText);
			pTextArea->SetCursor(0, sText.GetLength());
			return true;
			}
		}
	else if (pControl = GetControlByType(controlCounter))
		{
		CGTextArea *pTextArea = pControl->AsTextArea();

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
				if (pControl->bReplaceInput)
					{
					pTextArea->SetText(strFromInt(chChar - '0', false));
					pControl->bReplaceInput = false;
					}
				else
					{
					CString sCounter = pTextArea->GetText();
					sCounter.Append(strFromInt(chChar - '0', false));
					pTextArea->SetText(sCounter);
					}
				return true;
			}
		}

	//	Deal with accelerators
	//	Check to see if one of the key matches one of the accelerators

	int iAction;
	if (m_Actions.FindByKey(CString(&chChar, 1), &iAction))
		{
		ExecuteAction(iAction);
		return true;
		}

	//	Didn't handle it

	return false;
	}

bool CDockPane::HandleKeyDown (int iVirtKey)

//	HandleKeyDown
//
//	Handle key down

	{
	switch (iVirtKey)
		{
		case VK_UP:
		case VK_LEFT:
			{
			int iAction;
			if (m_Actions.FindSpecial(CLanguage::specialPrevKey, &iAction))
				{
				ExecuteAction(iAction);
				return true;
				}
			break;
			}

		case VK_DOWN:
		case VK_RIGHT:
			{
			int iAction;
			if (m_Actions.FindSpecial(CLanguage::specialNextKey, &iAction))
				{
				ExecuteAction(iAction);
				return true;
				}
			break;
			}

		case VK_NEXT:
			{
			int iAction;
			if (m_Actions.FindSpecial(CLanguage::specialPgDnKey, &iAction))
				{
				ExecuteAction(iAction);
				return true;
				}
			break;
			}

		case VK_PRIOR:
			{
			int iAction;
			if (m_Actions.FindSpecial(CLanguage::specialPgUpKey, &iAction))
				{
				ExecuteAction(iAction);
				return true;
				}
			break;
			}

		case VK_BACK:
			{
			SControl *pControl;
			if (pControl = GetControlByType(controlTextInput))
				{
				CGTextArea *pTextArea = pControl->AsTextArea();
				CString sText = pTextArea->GetText();
				if (sText.GetLength() > 1)
					{
					sText = strSubString(sText, 0, sText.GetLength() - 1);
					pTextArea->SetText(sText);
					pTextArea->SetCursor(0, sText.GetLength());
					}
				else
					{
					pTextArea->SetText(NULL_STR);
					pTextArea->SetCursor(0, 0);
					}

				return true;
				}
			else if (pControl = GetControlByType(controlCounter))
				{
				CGTextArea *pTextArea = pControl->AsTextArea();
				CString sCounter = pTextArea->GetText();
				if (sCounter.GetLength() > 1)
					{
					pTextArea->SetText(strSubString(sCounter, 0, sCounter.GetLength() - 1));
					pControl->bReplaceInput = false;
					}
				else
					{
					pTextArea->SetText(CONSTLIT("0"));
					pControl->bReplaceInput = true;
					}

				return true;
				}
			break;
			}

		case VK_ESCAPE:
			ExecuteCancelAction();
			return true;

		case VK_RETURN:
			{
			int iAction;
			if (m_Actions.FindSpecial(CLanguage::specialDefault, &iAction))
				{
				ExecuteAction(iAction);
				return true;
				}
			else if (m_Actions.GetCount() == 1)
				{
				ExecuteAction(0);
				return true;
				}
			break;
			}
		}

	return false;
	}

bool CDockPane::InitLayout (const CString &sLayout, const RECT &rcFullRect, CString *retsError)

//	InitLayout
//
//	Initializes the layout metrics.

	{
	if (sLayout.IsBlank() || strEquals(sLayout, LAYOUT_RIGHT))
		{
		m_iLayout = layoutRight;

		m_rcControls.right = rcFullRect.right - STD_PANE_PADDING_RIGHT;
		m_rcControls.left = m_rcControls.right - STD_PANE_WIDTH;
		m_rcControls.top = rcFullRect.top;
		m_rcControls.bottom = rcFullRect.bottom;

		m_rcActions = m_rcControls;
		}
	else if (strEquals(sLayout, LAYOUT_LEFT))
		{
		m_iLayout = layoutLeft;

		m_rcControls.left = STD_PANE_PADDING_LEFT;
		m_rcControls.right = m_rcControls.left + STD_PANE_WIDTH;
		m_rcControls.top = rcFullRect.top;
		m_rcControls.bottom = rcFullRect.bottom;

		m_rcActions = m_rcControls;
		}
	else if (strEquals(sLayout, LAYOUT_BOTTOM_BAR))
		{
		m_iLayout = layoutBottomBar;

		//	Controls are centered

		m_rcControls.left = rcFullRect.left + (RectWidth(rcFullRect) - STD_PANE_WIDTH) / 2;
		m_rcControls.right = m_rcControls.left + STD_PANE_WIDTH;
		m_rcControls.top = rcFullRect.top;
		m_rcControls.bottom = rcFullRect.bottom - STD_PANE_PADDING_BOTTOM;

		//	Actions the the full width

		m_rcActions = rcFullRect;
		m_rcActions.bottom -= STD_PANE_PADDING_BOTTOM;
		}
	else
		{
		if (retsError) *retsError = strPatternSubst(CONSTLIT("Invalid layout: %s"), sLayout);
		return false;
		}

	//	Done

	return true;
	}

ALERROR CDockPane::InitPane (CDockScreen *pDockScreen, CXMLElement *pPaneDesc, const RECT &rcFullRect)

//	InitPane
//
//	Initializes the pane.

	{
	CString sError;

	//	Initialize

	AGScreen *pScreen = pDockScreen->GetScreen();
	CleanUp(pScreen);

	m_pDockScreen = pDockScreen;
	m_pPaneDesc = pPaneDesc;
	ICCItem *pData = m_pDockScreen->GetData();

	//	Make sure we don't recurse

	m_bInShowPane = true;

	//	Pane layout

	if (!InitLayout(m_pPaneDesc->GetAttribute(LAYOUT_ATTRIB), rcFullRect, &sError))
		{
		ReportError(strPatternSubst(CONSTLIT("Pane %s: %s"), pPaneDesc->GetTag(), sError));
		return NOERROR;
		}

	//	Initialize list of actions.

	if (m_Actions.InitFromXML(m_pDockScreen->GetExtension(), m_pPaneDesc->GetContentElementByTag(ACTIONS_TAG), pData, &sError) != NOERROR)
		{
		ReportError(strPatternSubst(CONSTLIT("Pane %s: %s"), pPaneDesc->GetTag(), sError));
		return NOERROR;
		}

	//	Create a new pane
	//
	//	NOTE: Children of the container have RECTs relative to the screen,
	//	not the container, so these coordinates don't really matter. EXCEPT
	//	that we don't want the container to overlap the dock screen display.
	//	At this point, we don't know the size/location of the pane (because
	//	we haven't yet measured all the controls). So we just specify a 
	//	temporary size and change it later inside of RenderXXX.

	RECT rcContainer;
	rcContainer.left = 0;
	rcContainer.top = 0;
	rcContainer.right = RectWidth(pScreen->GetRect());
	rcContainer.bottom = RectHeight(pScreen->GetRect());

	m_pContainer = new CGFrameArea;
	pScreen->AddArea(m_pContainer, rcContainer, 0);

	//	Create the appropriate set of controls

	if (CreateControls(m_rcControls, &sError) != NOERROR)
		{
		ReportError(strPatternSubst(CONSTLIT("Pane %s: %s"), pPaneDesc->GetTag(), sError));
		return NOERROR;
		}

	//	Set the description text

	CString sValue;
	if (m_pPaneDesc->FindAttribute(DESC_ATTRIB, &sValue))
		{
		CString sDesc;
		if (!m_pDockScreen->EvalString(sValue, pData, false, eventNone, &sDesc))
			ReportError(strPatternSubst(CONSTLIT("Error evaluating desc param: %s"), sValue));
		else
			SetDescription(sDesc);
		}
	else if (m_pPaneDesc->FindAttribute(DESC_ID_ATTRIB, &sValue))
		{
		ICCItemPtr pResult;
		if (!m_pDockScreen->Translate(sValue, pData, pResult))
			ReportError(strPatternSubst(CONSTLIT("Unknown language ID: %s"), sValue));
		else
			SetDescription(pResult->GetStringValue());
		}

	//	Evaluate the initialize element
	//	
	//	This gives the frame a chance to initialize any dynamic
	//	action buttons before we actually create the buttons.

	CXMLElement *pInit = m_pPaneDesc->GetContentElementByTag(ON_PANE_INIT_TAG);
	if (pInit == NULL)
		pInit = m_pPaneDesc->GetContentElementByTag(INITIALIZE_TAG);

	if (pInit)
		{
		CString sCode = pInit->GetContentText(0);
		CString sError;
		if (!m_pDockScreen->EvalString(sCode, pData, true, eventNone, &sError))
			ReportError(strPatternSubst(CONSTLIT("Error evaluating <OnPaneInit>: %s"), sError));
		}

	//	We might have called exit inside OnPaneInit. If so, we exit

	if (m_pDockScreen->GetScreen() == NULL)
		return NOERROR;

	//	Allow other design types to override the pane

	CString sResolvedScreen;
	CDesignType *pResolvedRoot = m_pDockScreen->GetResolvedRoot(&sResolvedScreen);
	g_pUniverse->FireOnGlobalPaneInit(m_pDockScreen, pResolvedRoot, sResolvedScreen, m_pPaneDesc->GetTag(), pData);
	if (m_pDockScreen->GetScreen() == NULL)
		return NOERROR;

	//	Now that all the controls (and actions) have been initialized, resize them
	//	so that they fit

	switch (m_iLayout)
		{
		case layoutBottomBar:
			RenderControlsBottomBar();
			break;

		default:
			RenderControlsColumn();
			break;
		}

	//	Done

	m_bInShowPane = false;

	return NOERROR;
	}

void CDockPane::JustifyControls (int *retcyTotalHeight)

//	JustifyControls
//
//	Initializes cyHeight for all controls (by justifying all controls).

	{
	int i;

	int cyControls = 0;
	for (i = 0; i < m_Controls.GetCount(); i++)
		{
		SControl &Control = m_Controls[i];

		//	Compute the desired height of all variable-height controls

		switch (Control.iType)
			{
			case controlDesc:
			case controlItemDisplay:
			case controlItemListDisplay:
				Control.cyHeight = Control.pArea->Justify(Control.pArea->GetRect());
				break;
			}

		//	Add up the total

		if (Control.cyHeight > 0)
			{
			if (i != 0)
				cyControls += CONTROL_PADDING_BOTTOM;

			cyControls += Control.cyHeight;
			}
		}

	//	Done

	if (retcyTotalHeight)
		*retcyTotalHeight = cyControls;
	}

void CDockPane::PositionControls (int x, int y)

//	PositionControls
//
//	Positions the set of controls at the given coordinates.

	{
	int i;

	for (i = 0; i < m_Controls.GetCount(); i++)
		{
		SControl &Control = m_Controls[i];

		//	If this control is 0 height, then hide it

		if (Control.cyHeight == 0)
			Control.pArea->Hide();

		//	Otherwise, resize it appropriately

		else
			{
			RECT rcControl = Control.pArea->GetRect();

			rcControl.top = y;
			rcControl.bottom = rcControl.top + Control.cyHeight;
			Control.pArea->Show();
			Control.pArea->SetRect(rcControl);

			y += Control.cyHeight + CONTROL_PADDING_BOTTOM;
			}
		}
	}

void CDockPane::RenderControlsBottomBar (void)

//	RenderControlsBottomBar
//
//	Position all controls and actions so they fit.

	{
	//	Compute the vertical height of the actions

	int cyActions = m_Actions.CalcAreaHeight(m_pDockScreen->GetResolvedRoot(), CDockScreenActions::arrangeHorizontal, m_rcActions);

	//	Compute the desired height of all variable-height controls

	int cyControls;
	JustifyControls(&cyControls);

	//	We position the controls just above the actions

	int x = m_rcControls.left;
	int y = m_rcControls.bottom - cyActions - STD_PANE_PADDING_BOTTOM - cyControls;
	PositionControls(x, y);

	//	Create the action buttons at the bottom

	m_Actions.CreateButtons(m_pDockScreen->GetVisuals(), m_pContainer, m_pDockScreen->GetResolvedRoot(), FIRST_ACTION_ID, CDockScreenActions::arrangeHorizontal, m_rcActions);

	//	Now that we know the size of the pane, we set the container size so that we
	//	don't overlap the screen display.

	RECT rcContainer;
	rcContainer.left = m_rcActions.left;
	rcContainer.right = m_rcActions.right;
	rcContainer.top = y;
	rcContainer.bottom = m_rcActions.bottom;
	m_pContainer->SetRect(rcContainer);
	}

void CDockPane::RenderControlsColumn (void)

//	RenderControlsColumn
//
//	Position all controls and actions so they fit.

	{
	//	Figure out how much room we need for actions and how much we have left
	//	for controls.

	int cyActions = m_Actions.CalcAreaHeight(m_pDockScreen->GetResolvedRoot(), CDockScreenActions::arrangeVertical, m_rcActions);
	int cyAvailable = RectHeight(m_rcControls) - cyActions;

	//	Compute the desired height of all variable-height controls

	int cyControls = 0;
	JustifyControls(&cyControls);

	//	Account for padding, top and bottom

	cyControls += PANE_PADDING_TOP + CONTROL_PADDING_BOTTOM;

	//	Actions take priority, in case that we don't have enough room for both.

	int cyControlsFull;
	if (cyControls > cyAvailable)
		cyControlsFull = cyControls - CONTROL_PADDING_BOTTOM + CONTROL_PADDING_MIN;
	else if (cyControls > cyAvailable - ACTION_MARGIN_Y)
		cyControlsFull = cyControls;
	else
		cyControlsFull = Min(cyAvailable, AlignUp(cyControls, DESC_HEIGHT_GRANULARITY));

	//int cyControlsAdj = Min(cyControls, cyAvailable);

	//	Figure out where to start.

	int x = m_rcControls.left;
	int y = m_rcControls.top + PANE_PADDING_TOP;
	PositionControls(x, y);

	//	Create the action buttons (deals with extra space above and show/hide)

	RECT rcActions;
	rcActions.left = m_rcControls.left;
	rcActions.top = m_rcControls.top + cyControlsFull;
	rcActions.right = m_rcControls.right;
	rcActions.bottom = m_rcControls.bottom;

	m_Actions.CreateButtons(m_pDockScreen->GetVisuals(), m_pContainer, m_pDockScreen->GetResolvedRoot(), FIRST_ACTION_ID, CDockScreenActions::arrangeVertical, rcActions);

	//	Now that we know the size of the pane, we set the container size so that we
	//	don't overlap the screen display.

	m_pContainer->SetRect(m_rcControls);
	}

ALERROR CDockPane::ReportError (const CString &sError)

//	ReportError
//
//	Report an error while evaluating pane descriptor

	{
	//	Make sure we have a description control

	if (GetTextControlByType(controlDesc) == NULL)
		CreateControl(controlDesc, DEFAULT_DESC_ID, STYLE_WARNING, m_rcControls);

	//	Report the error through the screen. This will add screen information and
	//	eventually call us back at SetDescription.

	return m_pDockScreen->ReportError(sError);
	}

bool CDockPane::SetControlValue (const CString &sID, ICCItem *pValue)

//	SetControlValue
//
//	Sets the value of the given control. Return FALSE if we could not find a 
//	control of the given ID.

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	SControl *pControl;
	if (!FindControl(sID, &pControl))
		return false;

	switch (pControl->iType)
		{
		case controlCounter:
			{
			CGTextArea *pTextArea = pControl->AsTextArea();
			CString sText = strFromInt(pValue->GetIntegerValue());
			pTextArea->SetText(sText);
			pControl->bReplaceInput = true;
			return true;
			}

		case controlDesc:
			{
			CGTextArea *pTextArea = pControl->AsTextArea();

			//	Setting the descriptor to Nil or blank collapses it.

			if (pValue->IsNil() || pValue->GetStringValue().IsBlank())
				pTextArea->SetRichText(NULL_STR);

			//	Otherwise, format for RTF

			else
				{
				CUIHelper UIHelper(*g_pHI);
				CString sRTF;
				UIHelper.GenerateDockScreenRTF(pValue->GetStringValue(), &sRTF);

				pTextArea->SetRichText(sRTF);
				}

			return true;
			}

		case controlItemDisplay:
			{
			CGItemDisplayArea *pItemDisplayArea = pControl->AsItemDisplayArea();

			//	Nil means nil

			if (pValue->IsNil())
				{
				pItemDisplayArea->SetItem(NULL, CItem::NullItem());
				return true;
				}

			//	If a structure, we expect two fields:

			else if (pValue->IsSymbolTable())
				{
				ICCItem *pItemCC = pValue->GetElement(CONSTLIT("item"));
				if (pItemCC == NULL)
					{
					CString sTitle;
					CString sDesc;

					//	If no item, then we expect title and desc

					ICCItem *pTitleCC = pValue->GetElement(CONSTLIT("title"));
					if (pTitleCC)
						sTitle = pTitleCC->GetStringValue();

					ICCItem *pDescCC = pValue->GetElement(CONSTLIT("desc"));
					if (pDescCC)
						sDesc = pDescCC->GetStringValue();

					pItemDisplayArea->SetItem(NULL, CItem::NullItem());
					pItemDisplayArea->SetText(sTitle, sDesc);

					return true;
					}

				CItem Item = ::CreateItemFromList(CC, pItemCC);

				CSpaceObject *pSource = NULL;
				ICCItem *pSourceCC = pValue->GetElement(CONSTLIT("source"));
				if (pSourceCC)
					pSource = ::CreateObjFromItem(CC, pSourceCC);

				pItemDisplayArea->SetItem(pSource, Item);
				return true;
				}

			//	If this is a list then we expect an item value

			else if (pValue->IsList())
				{
				CItem Item = ::CreateItemFromList(CC, pValue);
				pItemDisplayArea->SetItem(NULL, Item);
				return true;
				}

			return false;
			}

		case controlItemListDisplay:
			{
			CGItemListDisplayArea *pDisplayArea = pControl->AsItemListDisplayArea();
			return pDisplayArea->InitFromDesc(CC, pValue);
			}

		case controlTextInput:
			{
			CGTextArea *pTextArea = pControl->AsTextArea();
			CString sValue = pValue->GetStringValue();
			pTextArea->SetText(sValue);
			pTextArea->SetCursor(0, sValue.GetLength());
			return true;
			}
		}

	return false;
	}

void CDockPane::SetCounterValue (int iValue)

//	SetCounterValue
//
//	Sets the value of the counter

	{
	SControl *pControl;
	if (pControl = GetControlByType(controlCounter))
		{
		CGTextArea *pTextArea = pControl->AsTextArea();
		CString sText = strFromInt(iValue);
		pTextArea->SetText(sText);
		pControl->bReplaceInput = true;
		}
	}

void CDockPane::SetDescription (const CString &sDesc)

//	SetDescription
//
//	Sets the description

	{
	//	If we've already set an error, then we ignore subsequent changes.

	if (!m_bDescError)
		m_sDesc = sDesc;

	SControl *pControl;
	if (pControl = GetControlByType(controlDesc))
		{
		CGTextArea *pTextArea = pControl->AsTextArea();
		CUIHelper UIHelper(*g_pHI);
		CString sRTF;
		UIHelper.GenerateDockScreenRTF(m_sDesc, &sRTF);

		pTextArea->SetRichText(sRTF);
		}
	}

void CDockPane::SetDescriptionError (const CString &sDesc)

//	SetDescriptionError
//
//	Sets an error message

	{
	m_bDescError = false;
	SetDescription(sDesc);
	m_bDescError = true;
	}

void CDockPane::SetTextInputValue (const CString &sValue)

//	SetTextInputValue
//
//	Sets the value of text input

	{
	SControl *pControl;
	if (pControl = GetControlByType(controlTextInput))
		{
		CGTextArea *pTextArea = pControl->AsTextArea();
		pTextArea->SetText(sValue);
		pTextArea->SetCursor(0, sValue.GetLength());
		}
	}
