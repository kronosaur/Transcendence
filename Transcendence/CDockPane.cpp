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
#define ON_PANE_INIT_TAG			CONSTLIT("OnPaneInit")
#define TEXT_TAG					CONSTLIT("Text")
#define TEXT_INPUT_TAG				CONSTLIT("TextInput")

#define DESC_ATTRIB					CONSTLIT("desc")
#define ID_ATTRIB					CONSTLIT("id")
#define SHOW_COUNTER_ATTRIB			CONSTLIT("showCounter")
#define SHOW_TEXT_INPUT_ATTRIB		CONSTLIT("showTextInput")
#define STYLE_ATTRIB				CONSTLIT("style")

#define DEFAULT_DESC_ID				CONSTLIT("desc")
#define DEFAULT_COUNTER_ID			CONSTLIT("counter")
#define DEFAULT_TEXT_INPUT_ID		CONSTLIT("textInput")

#define STYLE_DEFAULT				CONSTLIT("default")
#define STYLE_WARNING				CONSTLIT("warning")

const int MAX_ACTIONS =				8;
const int FIRST_ACTION_ID =			100;
const int LAST_ACTION_ID =			199;

const int CONTROL_BORDER_RADIUS =	4;
const int CONTROL_INNER_PADDING =	8;
const int CONTROL_PADDING_BOTTOM =	24;

const int COUNTER_ID =				204;
const int COUNTER_WIDTH =			128;
const int COUNTER_HEIGHT =			40;
const int COUNTER_PADDING_BOTTOM =	24;

const int DESC_PADDING_BOTTOM =		24;
const int DESC_HEIGHT_GRANULARITY =	60;

const int TEXT_INPUT_ID =			207;
const int TEXT_INPUT_WIDTH =		380;
const int TEXT_INPUT_HEIGHT	=		40;
const int TEXT_INPUT_PADDING_BOTTOM = 24;

const int PANE_PADDING_TOP =		24;
const int PANE_PADDING_EXTRA =		0;

CDockPane::CDockPane (void) :
		m_pPaneDesc(NULL),
		m_pContainer(NULL),
		m_bInShowPane(false)

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
	}

void CDockPane::CreateControl (EControlTypes iType, const CString &sID, const CString &sStyle)

//	CreateControl
//
//	Creates a control of the given type.

	{
	const CVisualPalette &VI = g_pHI->GetVisuals();

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
			rcInput.left = m_rcPane.left + (RectWidth(m_rcPane) - COUNTER_WIDTH) / 2;
			rcInput.right = rcInput.left + COUNTER_WIDTH;

			//	Height doesn't matter for now; we recalc later
			rcInput.top = m_rcPane.top;
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
			m_pContainer->AddArea(pControl->pArea, m_rcPane, 0);
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

			pControl->pArea = pItemDisplayArea;
			m_pContainer->AddArea(pControl->pArea, m_rcPane, 0);
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
			rcInput.left = m_rcPane.left + (RectWidth(m_rcPane) - TEXT_INPUT_WIDTH) / 2;
			rcInput.right = rcInput.left + TEXT_INPUT_WIDTH;
			rcInput.top = m_rcPane.top;
			rcInput.bottom = rcInput.top + TEXT_INPUT_HEIGHT;

			pControl->pArea = pTextArea;
			m_pContainer->AddArea(pControl->pArea, rcInput, TEXT_INPUT_ID);
			break;
			}

		default:
			ASSERT(false);
		}
	}

ALERROR CDockPane::CreateControls (CString *retsError)

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
		for (i = 0; i < pControls->GetContentElementCount(); i++)
			{
			CXMLElement *pControlDef = pControls->GetContentElement(i);

			//	Figure out the type

			EControlTypes iType;
			if (strEquals(pControlDef->GetTag(), COUNTER_TAG))
				iType = controlCounter;
			else if (strEquals(pControlDef->GetTag(), ITEM_DISPLAY_TAG))
				iType = controlItemDisplay;
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

			//	Create the control

			CreateControl(iType, sID, sStyle);
			}
		}

	//	Otherwise we create default controls

	else
		{
		//	Create the text description control

		CreateControl(controlDesc, DEFAULT_DESC_ID, STYLE_DEFAULT);

		//	Create counter or input fields

		if (m_pPaneDesc->GetAttributeBool(SHOW_COUNTER_ATTRIB))
			CreateControl(controlCounter, DEFAULT_COUNTER_ID, STYLE_DEFAULT);
		else if (m_pPaneDesc->GetAttributeBool(SHOW_TEXT_INPUT_ATTRIB))
			CreateControl(controlTextInput, DEFAULT_TEXT_INPUT_ID, STYLE_DEFAULT);
		}

	return NOERROR;
	}

void CDockPane::ExecuteCancelAction (void)

//	ExecuteCancelAction
//
//	Cancel

	{
	int iAction;
	if (m_Actions.FindSpecial(CDockScreenActions::specialCancel, &iAction))
		m_Actions.Execute(iAction, m_pDockScreen);
	else
		m_Actions.ExecuteExitScreen();

	g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
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

	if (strEquals(sStyle, STYLE_WARNING))
		{
		retStyle->pTextFont = &VI.GetFont(fontLarge);
		retStyle->TextColor = VI.GetColor(colorTextWarningMsg);
		retStyle->BackColor = VI.GetColor(colorAreaWarningMsg);
		}
	else
		{
		retStyle->pTextFont = &VI.GetFont(fontLarge);
		retStyle->TextColor = VI.GetColor(colorTextDockText);
		retStyle->BackColor = VI.GetColor(colorAreaDialogInputFocus);
		}
	}

CGTextArea *CDockPane::GetTextControlByType (EControlTypes iType) const

//	GetTextControlByType
//
//	Returns the control by type

	{
	int i;

	switch (iType)
		{
		case controlCounter:
		case controlDesc:
		case controlTextInput:
			for (i = 0; i < m_Controls.GetCount(); i++)
				if (m_Controls[i].iType == iType)
					return m_Controls[i].AsTextArea();
			break;
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

		g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
		m_Actions.Execute(iAction, m_pDockScreen);

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
		g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
		m_Actions.Execute(iAction, m_pDockScreen);
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
			if (m_Actions.FindSpecial(CDockScreenActions::specialPrevKey, &iAction))
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				m_Actions.Execute(iAction, m_pDockScreen);
				return true;
				}
			break;
			}

		case VK_DOWN:
		case VK_RIGHT:
			{
			int iAction;
			if (m_Actions.FindSpecial(CDockScreenActions::specialNextKey, &iAction))
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				m_Actions.Execute(iAction, m_pDockScreen);
				return true;
				}
			break;
			}

		case VK_NEXT:
			{
			int iAction;
			if (m_Actions.FindSpecial(CDockScreenActions::specialPgDnKey, &iAction))
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				m_Actions.Execute(iAction, m_pDockScreen);
				return true;
				}
			break;
			}

		case VK_PRIOR:
			{
			int iAction;
			if (m_Actions.FindSpecial(CDockScreenActions::specialPgUpKey, &iAction))
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				m_Actions.Execute(iAction, m_pDockScreen);
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
			if (m_Actions.FindSpecial(CDockScreenActions::specialDefault, &iAction))
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				m_Actions.Execute(iAction, m_pDockScreen);

				return true;
				}
			else if (m_Actions.GetCount() == 1)
				{
				g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
				m_Actions.Execute(0, m_pDockScreen);

				return true;
				}
			break;
			}
		}

	return false;
	}

ALERROR CDockPane::InitPane (CDockScreen *pDockScreen, CXMLElement *pPaneDesc, const RECT &rcPaneRect)

//	InitPane
//
//	Initializes the pane.

	{
	//	Initialize

	AGScreen *pScreen = pDockScreen->GetScreen();
	CleanUp(pScreen);

	m_pDockScreen = pDockScreen;
	m_pPaneDesc = pPaneDesc;
	m_rcPane = rcPaneRect;
	ICCItem *pData = m_pDockScreen->GetData();

	//	Make sure we don't recurse

	m_bInShowPane = true;

	//	Initialize list of actions.

	CString sError;
	if (m_Actions.InitFromXML(m_pDockScreen->GetExtension(), m_pPaneDesc->GetContentElementByTag(ACTIONS_TAG), pData, &sError) != NOERROR)
		{
		ReportError(strPatternSubst(CONSTLIT("Pane %s: %s"), pPaneDesc->GetTag(), sError));
		return NOERROR;
		}

	//	Create a new pane

	m_pContainer = new CGFrameArea;
	pScreen->AddArea(m_pContainer, m_rcPane, 0);

	//	Create the appropriate set of controls

	if (CreateControls(&sError) != NOERROR)
		{
		ReportError(strPatternSubst(CONSTLIT("Pane %s: %s"), pPaneDesc->GetTag(), sError));
		return NOERROR;
		}

	//	Set the description text

	CString sDesc;
	if (!m_pDockScreen->EvalString(m_pPaneDesc->GetAttribute(DESC_ATTRIB), pData, false, eventNone, &sDesc))
		ReportError(strPatternSubst(CONSTLIT("Error evaluating desc param: %s"), sDesc));
	else
		SetDescription(sDesc);

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
	g_pUniverse->FireOnGlobalPaneInit(m_pDockScreen, pResolvedRoot, sResolvedScreen, m_pPaneDesc->GetTag());
	if (m_pDockScreen->GetScreen() == NULL)
		return NOERROR;

	//	Now that all the controls (and actions) have been initialized, resize them
	//	so that they fit

	RenderControls();

	//	Done

	m_bInShowPane = false;

	return NOERROR;
	}

void CDockPane::RenderControls (void)

//	RenderControls
//
//	Position all controls and actions so they fit.

	{
	int i;

	//	Figure out how much room we need for actions and how much we have left
	//	for controls.

	int cyActions = m_Actions.CalcAreaHeight(m_pDockScreen->GetResolvedRoot(), m_rcPane);
	int cyAvailable = RectHeight(m_rcPane) - PANE_PADDING_TOP - cyActions;

	//	Compute the desired height of all variable-height controls

	int cyControls = 0;
	for (i = 0; i < m_Controls.GetCount(); i++)
		{
		SControl &Control = m_Controls[i];

		//	Compute the desired height of all variable-height controls

		switch (Control.iType)
			{
			case controlDesc:
			case controlItemDisplay:
				Control.cyHeight = Control.pArea->Justify(Control.pArea->GetRect());
				break;
			}

		//	Add up the total

		cyControls += Control.cyHeight + CONTROL_PADDING_BOTTOM;
		}

	//	Compute the buffer between the controls and the actions

	int cyControlsFull = Min(Max(PANE_PADDING_TOP + cyControls, cyAvailable), AlignUp(PANE_PADDING_TOP + cyControls, DESC_HEIGHT_GRANULARITY));

	//	Figure out where to start.

	int y = m_rcPane.top + PANE_PADDING_TOP;

	//	Now resize all the control to the appropriate height.

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

	//	Create the action buttons (deals with extra space above and show/hide)

	RECT rcActions;
	rcActions.left = m_rcPane.left;
	rcActions.top = m_rcPane.top + cyControlsFull;
	rcActions.right = m_rcPane.right;
	rcActions.bottom = m_rcPane.bottom;

	m_Actions.CreateButtons(m_pContainer, m_pDockScreen->GetResolvedRoot(), FIRST_ACTION_ID, rcActions);
	}

ALERROR CDockPane::ReportError (const CString &sError)

//	ReportError
//
//	Report an error while evaluating pane descriptor

	{
	//	Make sure we have a description control

	if (GetTextControlByType(controlDesc) == NULL)
		CreateControl(controlDesc, DEFAULT_DESC_ID, STYLE_WARNING);

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
	m_sDesc = sDesc;

	SControl *pControl;
	if (pControl = GetControlByType(controlDesc))
		{
		CGTextArea *pTextArea = pControl->AsTextArea();
		CUIHelper UIHelper(*g_pHI);
		CString sRTF;
		UIHelper.GenerateDockScreenRTF(sDesc, &sRTF);

		pTextArea->SetRichText(sRTF);
		}
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
