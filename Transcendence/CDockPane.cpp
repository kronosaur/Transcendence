//	CDockPane.cpp
//
//	CDockPane class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define ACTIONS_TAG					CONSTLIT("Actions")
#define INITIALIZE_TAG				CONSTLIT("Initialize")
#define ON_PANE_INIT_TAG			CONSTLIT("OnPaneInit")

#define DESC_ATTRIB					CONSTLIT("desc")
#define SHOW_COUNTER_ATTRIB			CONSTLIT("showCounter")
#define SHOW_TEXT_INPUT_ATTRIB		CONSTLIT("showTextInput")

const int ACTION_BUTTON_HEIGHT =	22;
const int ACTION_BUTTON_SPACING =	4;
const int MAX_ACTIONS =				8;
const int ACTION_REGION_HEIGHT =	(ACTION_BUTTON_HEIGHT * MAX_ACTIONS) + (ACTION_BUTTON_SPACING * (MAX_ACTIONS - 1));
const int FIRST_ACTION_ID =			100;
const int LAST_ACTION_ID =			199;

const int COUNTER_ID =				204;
const int COUNTER_WIDTH =			128;
const int COUNTER_HEIGHT =			40;

const int TEXT_INPUT_ID =			207;
const int TEXT_INPUT_WIDTH =		380;
const int TEXT_INPUT_HEIGHT	=		40;

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

void CDockPane::CreateControl (EControlTypes iType)

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

			pControl->pTextControl = new CGTextArea;
			pControl->pTextControl->SetEditable();
			pControl->pTextControl->SetText(CONSTLIT("0"));
			pControl->pTextControl->SetFont(&VI.GetFont(fontSubTitleHeavyBold));
			pControl->pTextControl->SetColor(CG16bitImage::RGBValue(255,255,255));
			pControl->pTextControl->SetStyles(alignCenter);

			RECT rcInput;
			rcInput.left = m_rcPane.left + (RectWidth(m_rcPane) - COUNTER_WIDTH) / 2;
			rcInput.right = rcInput.left + COUNTER_WIDTH;
			rcInput.top = m_rcPane.bottom - ACTION_REGION_HEIGHT - COUNTER_HEIGHT - 24;
			rcInput.bottom = rcInput.top + COUNTER_HEIGHT;

			m_pContainer->AddArea(pControl->pTextControl, rcInput, COUNTER_ID);

			pControl->bReplaceInput = true;
			break;
			}

		case controlDesc:
			{
			SControl *pControl = m_Controls.Insert();
			pControl->iType = iType;

			pControl->pTextControl = new CGTextArea;
			pControl->pTextControl->SetFont(&VI.GetFont(fontLarge));
			pControl->pTextControl->SetColor(VI.GetColor(colorTextDockText));
			pControl->pTextControl->SetLineSpacing(6);
			pControl->pTextControl->SetFontTable(&VI);

			RECT rcDesc;
			rcDesc.left = m_rcPane.left;
			rcDesc.top = m_rcPane.top + 16;
			rcDesc.right = m_rcPane.right;
#if 0
			if (bShowCounter || bShowTextInput)
				rcDesc.bottom = rcInput.top;
			else
#endif
				rcDesc.bottom = m_rcPane.bottom - ACTION_REGION_HEIGHT;

			m_pContainer->AddArea(pControl->pTextControl, rcDesc, 0);
			break;
			}

		case controlTextInput:
			{
			SControl *pControl = m_Controls.Insert();
			pControl->iType = iType;

			pControl->pTextControl = new CGTextArea;
			pControl->pTextControl->SetEditable();
			pControl->pTextControl->SetFont(&VI.GetFont(fontSubTitleHeavyBold));
			pControl->pTextControl->SetColor(CG16bitImage::RGBValue(255,255,255));
			pControl->pTextControl->SetCursor(0, 0);

			RECT rcInput;
			rcInput.left = m_rcPane.left + (RectWidth(m_rcPane) - TEXT_INPUT_WIDTH) / 2;
			rcInput.right = rcInput.left + TEXT_INPUT_WIDTH;
			rcInput.top = m_rcPane.bottom - ACTION_REGION_HEIGHT - TEXT_INPUT_HEIGHT - 24;
			rcInput.bottom = rcInput.top + TEXT_INPUT_HEIGHT;

			m_pContainer->AddArea(pControl->pTextControl, rcInput, TEXT_INPUT_ID);
			break;
			}

		default:
			ASSERT(false);
		}
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

CGTextArea *CDockPane::GetControlByType (EControlTypes iType) const

//	GetControlByType
//
//	Returns the control by type

	{
	int i;

	for (i = 0; i < m_Controls.GetCount(); i++)
		if (m_Controls[i].iType == iType)
			return m_Controls[i].pTextControl;

	return NULL;
	}

CDockPane::SControl *CDockPane::GetControlEntryByType (EControlTypes iType) const

//	GetControlEntryByType
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
	if (pCounter = GetControlByType(controlCounter))
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
	CGTextArea *pDesc = GetControlByType(controlDesc);
	if (pDesc == NULL)
		return NULL_STR;

	return m_sDesc;
	}

CString CDockPane::GetTextInputValue (void) const

//	GetTextInputValue
//
//	Returns the current value of the input control

	{
	CGTextArea *pControl;
	if (pControl = GetControlByType(controlTextInput))
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
	if (pControl = GetControlEntryByType(controlTextInput))
		{
		if (chChar >= ' ' && chChar <= '~')
			{
			CString sText = pControl->pTextControl->GetText();
			sText.Append(CString(&chChar, 1));
			pControl->pTextControl->SetText(sText);
			pControl->pTextControl->SetCursor(0, sText.GetLength());
			return true;
			}
		}
	else if (pControl = GetControlEntryByType(controlCounter))
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
				if (pControl->bReplaceInput)
					{
					pControl->pTextControl->SetText(strFromInt(chChar - '0', false));
					pControl->bReplaceInput = false;
					}
				else
					{
					CString sCounter = pControl->pTextControl->GetText();
					sCounter.Append(strFromInt(chChar - '0', false));
					pControl->pTextControl->SetText(sCounter);
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

		case VK_BACK:
			{
			SControl *pControl;
			if (pControl = GetControlEntryByType(controlTextInput))
				{
				CString sText = pControl->pTextControl->GetText();
				if (sText.GetLength() > 1)
					{
					sText = strSubString(sText, 0, sText.GetLength() - 1);
					pControl->pTextControl->SetText(sText);
					pControl->pTextControl->SetCursor(0, sText.GetLength());
					}
				else
					{
					pControl->pTextControl->SetText(NULL_STR);
					pControl->pTextControl->SetCursor(0, 0);
					}

				return true;
				}
			else if (pControl = GetControlEntryByType(controlCounter))
				{
				CString sCounter = pControl->pTextControl->GetText();
				if (sCounter.GetLength() > 1)
					{
					pControl->pTextControl->SetText(strSubString(sCounter, 0, sCounter.GetLength() - 1));
					pControl->bReplaceInput = false;
					}
				else
					{
					pControl->pTextControl->SetText(CONSTLIT("0"));
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

	//	Create a new pane

	m_pContainer = new CGFrameArea;
	pScreen->AddArea(m_pContainer, m_rcPane, 0);

	//	Initialize list of actions

	CString sError;
	if (m_Actions.InitFromXML(m_pDockScreen->GetExtension(), m_pPaneDesc->GetContentElementByTag(ACTIONS_TAG), pData, &sError) != NOERROR)
		{
		sError = strPatternSubst(CONSTLIT("Pane %s: %s"), pPaneDesc->GetTag(), sError);
		SetDescription(sError);
		kernelDebugLogMessage(sError);
		return NOERROR;
		}

	//	Create the text description control

	CreateControl(controlDesc);

	//	Create counter or input fields

	if (m_pPaneDesc->GetAttributeBool(SHOW_COUNTER_ATTRIB))
		CreateControl(controlCounter);
	else if (m_pPaneDesc->GetAttributeBool(SHOW_TEXT_INPUT_ATTRIB))
		CreateControl(controlTextInput);

	//	Set the description text

	CString sDesc;
	if (!m_pDockScreen->EvalString(m_pPaneDesc->GetAttribute(DESC_ATTRIB), pData, false, eventNone, &sDesc))
		m_pDockScreen->ReportError(strPatternSubst(CONSTLIT("Error evaluating desc param: %s"), sDesc));
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
			m_pDockScreen->ReportError(strPatternSubst(CONSTLIT("Error evaluating <OnPaneInit>: %s"), sError));
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
	//	Check to see if the description is too large for the area. If so, then
	//	we shift everything down.

	CGTextArea *pDesc = GetControlByType(controlDesc);
	RECT rcDesc = pDesc->GetRect();

	int cyDesc = pDesc->Justify(rcDesc);
	int cyRow = (ACTION_BUTTON_HEIGHT + ACTION_BUTTON_SPACING);
	int cyExtraNeeded = (cyDesc + cyRow) - RectHeight(rcDesc);
	int cyExtraSpace = 0;
	if (cyExtraNeeded > 0)
		{
		int iExtraRows = Min(AlignUp(cyExtraNeeded, cyRow) / cyRow, MAX_ACTIONS - m_Actions.GetVisibleCount());
		if (iExtraRows > 0)
			{
			cyExtraSpace = iExtraRows * cyRow;

			rcDesc.bottom += cyExtraSpace;
			pDesc->SetRect(rcDesc);
			}
		}

	//	Compute some metrics for the rest of the controls

	int yDescBottom = rcDesc.top + cyDesc;
	int yActionsTop = m_rcPane.bottom - ACTION_REGION_HEIGHT + cyExtraSpace;
	int cyInput = yActionsTop - yDescBottom;

	//	Move the input field, if necessary

	CGTextArea *pInput;
	if (pInput = GetControlByType(controlTextInput))
		{
		RECT rcRect;
		rcRect = pInput->GetRect();
		rcRect.top = yDescBottom + (cyInput - TEXT_INPUT_HEIGHT) / 2;
		rcRect.bottom = rcRect.top + TEXT_INPUT_HEIGHT;
		pInput->SetRect(rcRect);
		}

	//	Move the counter, if necessary

	if (pInput = GetControlByType(controlCounter))
		{
		RECT rcRect;
		rcRect = pInput->GetRect();
		rcRect.top = yDescBottom + (cyInput - COUNTER_HEIGHT) / 2;
		rcRect.bottom = rcRect.top + COUNTER_HEIGHT;
		pInput->SetRect(rcRect);
		}

	//	Create the action buttons (deals with extra space above and show/hide)

	RECT rcActions;
	rcActions.left = m_rcPane.left;
	rcActions.top = yActionsTop;
	rcActions.right = m_rcPane.right;
	rcActions.bottom = m_rcPane.bottom;

	m_Actions.CreateButtons(m_pContainer, m_pDockScreen->GetResolvedRoot(), FIRST_ACTION_ID, rcActions);
	}

void CDockPane::SetCounterValue (int iValue)

//	SetCounterValue
//
//	Sets the value of the counter

	{
	SControl *pControl;
	if (pControl = GetControlEntryByType(controlCounter))
		{
		CString sText = strFromInt(iValue);
		pControl->pTextControl->SetText(sText);
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
	if (pControl = GetControlEntryByType(controlDesc))
		{
		CUIHelper UIHelper(*g_pHI);
		CString sRTF;
		UIHelper.GenerateDockScreenRTF(sDesc, &sRTF);

		pControl->pTextControl->SetRichText(sRTF);
		}
	}

void CDockPane::SetTextInputValue (const CString &sValue)

//	SetTextInputValue
//
//	Sets the value of text input

	{
	SControl *pControl;
	if (pControl = GetControlEntryByType(controlTextInput))
		{
		pControl->pTextControl->SetText(sValue);
		pControl->pTextControl->SetCursor(0, sValue.GetLength());
		}
	}
