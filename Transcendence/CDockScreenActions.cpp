//	CDockScreenActions.cpp
//
//	CDockScreenActions class

#include "PreComp.h"
#include "Transcendence.h"

#define EXIT_TAG					CONSTLIT("Exit")
#define NAVIGATE_TAG				CONSTLIT("Navigate")
#define SHOW_PANE_TAG				CONSTLIT("ShowPane")

#define CANCEL_ATTRIB				CONSTLIT("cancel")
#define DEFAULT_ATTRIB				CONSTLIT("default")
#define ID_ATTRIB					CONSTLIT("id")
#define KEY_ATTRIB					CONSTLIT("key")
#define NAME_ATTRIB					CONSTLIT("name")
#define NEXT_KEY_ATTRIB				CONSTLIT("nextKey")
#define PANE_ATTRIB					CONSTLIT("pane")
#define PREV_KEY_ATTRIB				CONSTLIT("prevKey")
#define SCREEN_ATTRIB				CONSTLIT("screen")

#define ACTION_SPECIAL_CANCEL		CONSTLIT("cancel")
#define ACTION_SPECIAL_DEFAULT		CONSTLIT("default")
#define ACTION_SPECIAL_NEXT_KEY		CONSTLIT("nextKey")
#define ACTION_SPECIAL_PREV_KEY		CONSTLIT("prevKey")

const int ACTION_BUTTON_HEIGHT =	22;
const int ACTION_BUTTON_SPACING =	4;

ALERROR CDockScreenActions::AddAction (const CString &sID, int iPos, const CString &sLabel, ICCItem *pCode, int *retiAction)

//	AddAction
//
//	Add a new action at the given position

	{
	ASSERT(!sID.IsBlank());
	ASSERT(iPos == -1 || (iPos >= 0 && iPos <= m_Actions.GetCount()));

	if (iPos == -1)
		iPos = m_Actions.GetCount();

	//	Insert a new action at the proper position

	SActionDesc *pAction = m_Actions.InsertAt(iPos);
	pAction->sID = sID;
	pAction->sLabel = sLabel;
	pAction->pButton = NULL;
	pAction->pCmd = NULL;
	pAction->pCode = pCode->Reference();

	pAction->bVisible = true;
	pAction->bEnabled = true;

	pAction->bDefault = false;
	pAction->bCancel = false;
	pAction->bPrev = false;
	pAction->bNext = false;

	//	Done

	if (retiAction)
		*retiAction = iPos;

	return NOERROR;
	}

void CDockScreenActions::CleanUp (void)

//	CleanUp
//
//	Free any structures that we hold

	{
	int i;

	for (i = 0; i < m_Actions.GetCount(); i++)
		{
		if (m_Actions[i].pCode)
			m_Actions[i].pCode->Discard(&g_pUniverse->GetCC());
		}

	m_Actions.DeleteAll();
	}

void CDockScreenActions::CreateButtons (CGFrameArea *pFrame, DWORD dwFirstTag, const RECT &rcFrame)

//	CreateButtons
//
//	Creates the buttons on the frame

	{
	int i;

	int y = rcFrame.top;
	const CVisualPalette &VI = g_pHI->GetVisuals();
	const CG16bitFont &LabelFont = VI.GetFont(fontMediumHeavyBold);

	for (i = 0; i < GetCount(); i++)
		{
		SActionDesc *pAction = &m_Actions[i];
		if (!pAction->bVisible)
			continue;

		CGButtonArea *pButton = new CGButtonArea;
		pButton->SetLabel(pAction->sLabel);
		pButton->SetLabelFont(&LabelFont);
		pButton->SetLabelAccelerator(pAction->sKey);
		if (!pAction->bEnabled)
			pButton->SetDisabled();

		RECT rcArea;
		rcArea.left = rcFrame.left;
		rcArea.top = y;
		rcArea.right = rcFrame.right;
		rcArea.bottom = y + ACTION_BUTTON_HEIGHT;
		pFrame->AddArea(pButton, rcArea, dwFirstTag + i);

		y += ACTION_BUTTON_HEIGHT + ACTION_BUTTON_SPACING;
		}
	}

void CDockScreenActions::Execute (int iAction, CDockScreen *pScreen)

//	Execute
//
//	Executes the given action

	{
	SActionDesc *pAction = &m_Actions[iAction];

	//	Ignore the action if disabled or invisible

	if (!pAction->bEnabled || !pAction->bVisible)
		return;

	//	If we have a special action, execute that

	if (pAction->pCmd)
		{
		if (strEquals(pAction->pCmd->GetTag(), NAVIGATE_TAG))
			{
			CString sScreen = pAction->pCmd->GetAttribute(SCREEN_ATTRIB);
			CString sPane = pAction->pCmd->GetAttribute(PANE_ATTRIB);

			if (g_pTrans->GetModel().ShowScreen(NULL, sScreen, sPane) != NOERROR)
				{
				CString sError = strPatternSubst(CONSTLIT("Unable to show screen: %s"), sScreen);
				pScreen->SetDescription(sError);
				::kernelDebugLogMessage(sError.GetASCIIZPointer());
				return;
				}
			}

		else if (strEquals(pAction->pCmd->GetTag(), EXIT_TAG))
			ExecuteExitScreen();

		else if (strEquals(pAction->pCmd->GetTag(), SHOW_PANE_TAG))
			ExecuteShowPane(pAction->pCmd->GetAttribute(PANE_ATTRIB));
		}

	//	Otherwise, if we have a code block, execute that

	else if (pAction->pCode)
		{
		ExecuteCode(pScreen, pAction->sID, pAction->pCode);
		}

	//	Otherwise, if we have unlinked code, execute that

	else if (!pAction->sCode.IsBlank())
		{
		//	Link the code

		ICCItem *pExp = g_pUniverse->GetCC().Link(pAction->sCode, 0);

		//	Execute

		ExecuteCode(pScreen, pAction->sID, pExp);

		//	Done

		pExp->Discard(&g_pUniverse->GetCC());
		}
	}

void CDockScreenActions::ExecuteCode (CDockScreen *pScreen, const CString &sID, ICCItem *pCode)

//	ExecuteCode
//
//	Executes this block of code

	{
	CCodeChainCtx Ctx;
	Ctx.DefineString(CONSTLIT("aActionID"), sID);
	Ctx.SetScreen(pScreen);

	ICCItem *pResult = Ctx.RunLambda(pCode);

	if (pResult->IsError())
		{
		CString sError = pResult->GetStringValue();

		pScreen->SetDescription(sError);
		::kernelDebugLogMessage(sError.GetASCIIZPointer());
		}

	Ctx.Discard(pResult);
	}

void CDockScreenActions::ExecuteExitScreen (bool bForceUndock)

//	ExecuteExitScreen
//
//	Exits the current screen

	{
	g_pTrans->GetModel().ExitScreenSession(bForceUndock);
	}

void CDockScreenActions::ExecuteShowPane (const CString &sPane)

//	ExecuteShowPane
//
//	Show the given pane

	{
	g_pTrans->GetModel().ShowPane(sPane);
	}

bool CDockScreenActions::FindByID (const CString &sID, int *retiAction)

//	FindByID
//
//	Looks for the action by ID

	{
	int i;

	for (i = 0; i < m_Actions.GetCount(); i++)
		{
		SActionDesc *pAction = &m_Actions[i];

		if (strEquals(sID, pAction->sID))
			{
			if (retiAction)
				*retiAction = i;
			return true;
			}
		}

	return false;
	}

bool CDockScreenActions::FindByID (ICCItem *pItem, int *retiAction)

//	FindByID
//
//	Looks for the action by ID

	{
	if (pItem->IsIdentifier())
		return FindByID(pItem->GetStringValue(), retiAction);
	else if (pItem->IsInteger())
		return FindByID(strFromInt(pItem->GetIntegerValue()), retiAction);
	else
		return false;
	}

bool CDockScreenActions::FindByKey (const CString &sKey, int *retiAction)

//	FindByKey
//
//	Looks for the action by accelerator key

	{
	int i;

	for (i = 0; i < m_Actions.GetCount(); i++)
		{
		SActionDesc *pAction = &m_Actions[i];

		if (pAction->bEnabled 
				&& pAction->bVisible
				&& strEquals(sKey, pAction->sKey))
			{
			if (retiAction)
				*retiAction = i;
			return true;
			}
		}

	return false;
	}

bool CDockScreenActions::FindSpecial (SpecialAttribs iSpecial, int *retiAction)

//	FindSpecial
//
//	Finds the special action

	{
	int i;

	for (i = 0; i < m_Actions.GetCount(); i++)
		{
		SActionDesc *pAction = &m_Actions[i];

		if (pAction->bEnabled 
				&& pAction->bVisible
				&& IsSpecial(i, iSpecial))
			{
			if (retiAction)
				*retiAction = i;
			return true;
			}
		}

	return false;
	}

int CDockScreenActions::GetVisibleCount (void) const

//	GetVisibleCount
//
//	Get the number of visible actions

	{
	int i;

	int iCount = 0;
	for (i = 0; i < m_Actions.GetCount(); i++)
		if (m_Actions[i].bVisible)
			iCount++;

	return iCount;
	}

ALERROR CDockScreenActions::InitFromXML (CXMLElement *pActions, CString *retsError)

//	InitFromXML
//
//	Initialize from XML

	{
	int i;

	//	Clean up current actions

	CleanUp();

	//	If no actions, that's OK

	if (pActions == NULL)
		return NOERROR;

	//	Load all actions

	m_Actions.InsertEmpty(pActions->GetContentElementCount());
	for (i = 0; i < pActions->GetContentElementCount(); i++)
		{
		CXMLElement *pActionDesc = pActions->GetContentElement(i);
		SActionDesc *pAction = &m_Actions[i];

		//	Label and key

		pAction->sID = pActionDesc->GetAttribute(ID_ATTRIB);
		pAction->sLabel = pActionDesc->GetAttribute(NAME_ATTRIB);
		pAction->sKey = pActionDesc->GetAttribute(KEY_ATTRIB);

		//	For backwards compatibility we generate an ID if one is
		//	not specified.

		if (pAction->sID.IsBlank())
			pAction->sID = strPatternSubst(CONSTLIT("%d"), i);

		//	Action

		if (pActionDesc->GetContentElementCount() > 0)
			{
			pAction->pCmd = pActionDesc->GetContentElement(0);
			pAction->pCode = NULL;
			}
		else
			{
			pAction->sCode = pActionDesc->GetContentText(0);
			pAction->pCmd = NULL;
			pAction->pCode = NULL;
			}

		//	Some flags

		pAction->bDefault = pActionDesc->GetAttributeBool(DEFAULT_ATTRIB);
		pAction->bCancel = pActionDesc->GetAttributeBool(CANCEL_ATTRIB);
		pAction->bNext = pActionDesc->GetAttributeBool(NEXT_KEY_ATTRIB);
		pAction->bPrev = pActionDesc->GetAttributeBool(PREV_KEY_ATTRIB);

		//	Defaults

		pAction->pButton = NULL;
		pAction->bVisible = true;
		pAction->bEnabled = true;
		}

	return NOERROR;
	}

bool CDockScreenActions::IsSpecial (int iAction, SpecialAttribs iSpecial)

//	IsSpecial
//
//	Returns TRUE if the given action has the special attrib

	{
	switch (iSpecial)
		{
		case specialDefault:
			return m_Actions[iAction].bDefault;

		case specialCancel:
			return m_Actions[iAction].bCancel;

		case specialNextKey:
			return m_Actions[iAction].bNext;

		case specialPrevKey:
			return m_Actions[iAction].bPrev;

		default:
			return false;
		}
	}

void CDockScreenActions::SetEnabled (int iAction, bool bEnabled)

//	SetEnabled
//
//	Enables the action

	{
	SActionDesc *pAction = &m_Actions[iAction];
	pAction->bEnabled = bEnabled;

	if (pAction->pButton)
		pAction->pButton->SetDisabled(!bEnabled);
	}

void CDockScreenActions::SetLabel (int iAction, const CString &sLabel, const CString &sKey)

//	SetLabel
//
//	Sets the label and key for the actions

	{
	SActionDesc *pAction = &m_Actions[iAction];

	if (!sLabel.IsBlank())
		pAction->sLabel = sLabel;

	if (!sKey.IsBlank())
		pAction->sKey = sKey;

	if (pAction->pButton)
		{
		pAction->pButton->SetLabel(pAction->sLabel);
		pAction->pButton->SetLabelAccelerator(pAction->sKey);
		}
	}

void CDockScreenActions::SetSpecial (int iAction, SpecialAttribs iSpecial, bool bEnabled)

//	SetSpecial
//
//	Sets the special attribute for an action

	{
	switch (iSpecial)
		{
		case specialAll:
			m_Actions[iAction].bCancel = bEnabled;
			m_Actions[iAction].bDefault = bEnabled;
			m_Actions[iAction].bNext = bEnabled;
			m_Actions[iAction].bPrev = bEnabled;
			break;

		case specialDefault:
			m_Actions[iAction].bDefault = bEnabled;
			break;

		case specialCancel:
			m_Actions[iAction].bCancel = bEnabled;
			break;

		case specialNextKey:
			m_Actions[iAction].bNext = bEnabled;
			break;

		case specialPrevKey:
			m_Actions[iAction].bPrev = bEnabled;
			break;
		}
	}

bool CDockScreenActions::SetSpecial (CCodeChain &CC, int iAction, ICCItem *pSpecial, ICCItem **retpError)

//	SetSpecial
//
//	Sets the attributes based on a list of strings

	{
	int i;

	//	Clear all the flags

	SetSpecial(iAction, CDockScreenActions::specialAll, false);

	//	Set them based on list

	for (i = 0; i < pSpecial->GetCount(); i++)
		{
		CString sSpecial = pSpecial->GetElement(i)->GetStringValue();

		if (strEquals(sSpecial, ACTION_SPECIAL_CANCEL))
			SetSpecial(iAction, CDockScreenActions::specialCancel);
		else if (strEquals(sSpecial, ACTION_SPECIAL_DEFAULT))
			SetSpecial(iAction, CDockScreenActions::specialDefault);
		else if (strEquals(sSpecial, ACTION_SPECIAL_NEXT_KEY))
			SetSpecial(iAction, CDockScreenActions::specialNextKey);
		else if (strEquals(sSpecial, ACTION_SPECIAL_PREV_KEY))
			SetSpecial(iAction, CDockScreenActions::specialPrevKey);
		else
			{
			if (retpError)
				*retpError = CC.CreateError(CONSTLIT("Unknown action special key"), pSpecial->GetElement(i));
			return false;
			}
		}

	return true;
	}

void CDockScreenActions::SetVisible (int iAction, bool bVisible)

//	SetVisible
//
//	Makes the button visible/invisible

	{
	SActionDesc *pAction = &m_Actions[iAction];
	pAction->bVisible = bVisible;

	if (pAction->pButton)
		pAction->pButton->ShowHide(bVisible);
	}
