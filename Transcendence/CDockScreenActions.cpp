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
#define DESC_ID_ATTRIB				CONSTLIT("descID")
#define ID_ATTRIB					CONSTLIT("id")
#define KEY_ATTRIB					CONSTLIT("key")
#define MINOR_ATTRIB				CONSTLIT("minor")
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

const int BOTTOM_MARGIN_Y =			46;

const int CONTROL_BORDER_RADIUS =	4;
const int CONTROL_INNER_PADDING =	8;

const int AREA_PADDING =			64;

CDockScreenActions::~CDockScreenActions (void)

//	CDockScreenActions destructor

	{
	CleanUp();
	}

ALERROR CDockScreenActions::AddAction (const CString &sID, int iPos, const CString &sLabel, CExtension *pExtension, ICCItem *pCode, int *retiAction)

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
	pAction->pExtension = pExtension;
	pAction->pCmd = NULL;
	pAction->pCode = pCode->Reference();

	pAction->bVisible = true;
	pAction->bEnabled = true;
	pAction->bMinor = false;

	//	Set the label, etc.

	SetLabelDesc(pAction, sLabel);

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

	if (m_pData)
		{
		m_pData->Discard(&g_pUniverse->GetCC());
		m_pData = NULL;
		}
	}

void CDockScreenActions::CreateButtons (CGFrameArea *pFrame, CDesignType *pRoot, DWORD dwFirstTag, const RECT &rcFrame)

//	CreateButtons
//
//	Creates the buttons on the frame

	{
	int i;

	const CVisualPalette &VI = g_pHI->GetVisuals();
	const CG16bitFont &MajorLabelFont = VI.GetFont(fontMediumHeavyBold);
	const CG16bitFont &MinorLabelFont = VI.GetFont(fontMedium);

	//	Loop over all buttons and get some stats about them. We also generate
	//	all labels because we might need to grow the (horizontal) size of buttons
	//	depending on it.

	bool bLongButtons = false;
	int iMinorButtonCount = 0;
	for (i = 0; i < GetCount(); i++)
		{
		SActionDesc *pAction = &m_Actions[i];
		if (!pAction->bVisible)
			continue;

		if (pAction->bMinor)
			iMinorButtonCount++;

		if (!pAction->sDesc.IsBlank() || !pAction->sDescID.IsBlank())
			bLongButtons = true;

		//	If the label is currently blank, then look up the ID in the language
		//	table and see if we have something.

		CString sLabelDesc;
		if (pAction->sLabel.IsBlank() 
				&& pRoot
				&& !pAction->sID.IsBlank()
				&& pRoot->TranslateText(NULL, pAction->sID, NULL, &sLabelDesc))
			{
			ParseLabelDesc(sLabelDesc, &pAction->sLabelTmp, &pAction->sKeyTmp);

			//	We need to set the action key because we have to check for it
			//	during input.

			if (!pAction->sKeyTmp.IsBlank())
				pAction->sKey = pAction->sKeyTmp;
			}
		else
			{
			pAction->sLabelTmp = pAction->sLabel;
			pAction->sKeyTmp = pAction->sKey;
			}

		//	If we've got a quoted label, then make it longer

		char *pPos = pAction->sLabelTmp.GetASCIIZPointer();
		if (*pPos == '\"' || *pPos == '“')
			bLongButtons = true;
		}

	//	We create buttons in one of two areas. Major buttons are at the top of
	//	the frame. Minor buttons are at the bottom. We start by counting the 
	//	number of minor buttons.

	int yMajor = rcFrame.top;
	int yMinor = rcFrame.bottom - BOTTOM_MARGIN_Y - (iMinorButtonCount * ACTION_BUTTON_HEIGHT) - ((iMinorButtonCount - 1) * ACTION_BUTTON_SPACING);

	//	Padding

	int xLeft = rcFrame.left + (bLongButtons ? 0 : AREA_PADDING);
	int xRight = rcFrame.right - (bLongButtons ? 0 : AREA_PADDING);

	//	Create all buttons

	for (i = 0; i < GetCount(); i++)
		{
		SActionDesc *pAction = &m_Actions[i];
		if (!pAction->bVisible)
			continue;

		//	Add the button

		CGButtonArea *pButton = new CGButtonArea;

		//	Set the label and key (which we already computed)

		pButton->SetLabel(pAction->sLabelTmp);
		pButton->SetLabelAccelerator(pAction->sKeyTmp);

		//	If we have a description, set that

		if (!pAction->sDesc.IsBlank())
			pButton->SetDesc(pAction->sDesc);

		//	Otherwise, if we have a description ID, then translate

		else if (!pAction->sDescID.IsBlank() && pRoot)
			{
			CString sText;
			if (pRoot->TranslateText(NULL, pAction->sDescID, NULL, &sText))
				pButton->SetDesc(sText);
			}

		//	Enabled/disabled

		if (!pAction->bEnabled)
			pButton->SetDisabled();

		//	These properties of the button depend on the type (major or minor)

		if (pAction->bMinor)
			{
			pButton->SetLabelFont(&MinorLabelFont);

			RECT rcArea;
			rcArea.left = xLeft;
			rcArea.top = yMinor;
			rcArea.right = xRight;
			rcArea.bottom = yMinor + ACTION_BUTTON_HEIGHT;
			pFrame->AddArea(pButton, rcArea, dwFirstTag + i);

			yMinor += ACTION_BUTTON_HEIGHT + ACTION_BUTTON_SPACING;
			}
		else
			{
			pButton->SetLabelFont(&MajorLabelFont);
			pButton->SetPadding(CONTROL_INNER_PADDING);
			pButton->SetBorderRadius(CONTROL_BORDER_RADIUS);
			pButton->SetBackColor(VI.GetColor(colorAreaDialogInput));
			pButton->SetBackColorHover(VI.GetColor(colorAreaDialogTitle));
			pButton->SetDescColor(CG32bitPixel(128, 128, 128));	//	Same as CGItemDisplayArea
			pButton->SetDescFont(&VI.GetFont(fontMedium));

			RECT rcArea;
			rcArea.left = xLeft;
			rcArea.top = yMajor;
			rcArea.right = xRight;
			rcArea.bottom = yMajor + ACTION_BUTTON_HEIGHT;

			int cyHeight = pButton->Justify(rcArea);
			rcArea.bottom = rcArea.top + cyHeight;

			pFrame->AddArea(pButton, rcArea, dwFirstTag + i);

			yMajor += cyHeight + ACTION_BUTTON_SPACING;
			}
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

			CString sError;
			if (g_pTrans->GetModel().ShowScreen(NULL, sScreen, sPane, NULL, &sError) != NOERROR)
				{
				pScreen->SetDescription(sError);
				::kernelDebugLogMessage(sError);
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
		ExecuteCode(pScreen, pAction->sID, pAction->pExtension, pAction->pCode);
		}

	//	Otherwise, if we have unlinked code, execute that

	else if (!pAction->sCode.IsBlank())
		{
		//	Link the code

		ICCItem *pExp = g_pUniverse->GetCC().Link(pAction->sCode, 0);

		//	Execute

		ExecuteCode(pScreen, pAction->sID, pAction->pExtension, pExp);

		//	Done

		pExp->Discard(&g_pUniverse->GetCC());
		}
	}

void CDockScreenActions::ExecuteCode (CDockScreen *pScreen, const CString &sID, CExtension *pExtension, ICCItem *pCode)

//	ExecuteCode
//
//	Executes this block of code

	{
	CCodeChainCtx Ctx;
	Ctx.SetExtension(pExtension);
	Ctx.SaveAndDefineDataVar(m_pData);
	Ctx.DefineString(CONSTLIT("aActionID"), sID);
	Ctx.SetScreen(pScreen);
	Ctx.SetDockScreenList(pScreen->GetListData());

	ICCItem *pResult = Ctx.RunLambda(pCode);

	if (pResult->IsError())
		{
		CString sError = pResult->GetStringValue();

		pScreen->SetDescription(sError);
		::kernelDebugLogMessage(sError);
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

ALERROR CDockScreenActions::InitFromXML (CExtension *pExtension, CXMLElement *pActions, ICCItem *pData, CString *retsError)

//	InitFromXML
//
//	Initialize from XML

	{
	int i;

	//	Clean up current actions

	CleanUp();

	//	Remember the data pointer

	m_pData = (pData ? pData->Reference() : NULL);

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

		SetLabelDesc(pAction, pActionDesc->GetAttribute(NAME_ATTRIB));

		//	ID

		pAction->sID = pActionDesc->GetAttribute(ID_ATTRIB);

		//	For backwards compatibility we generate an ID if one is
		//	not specified.

		if (pAction->sID.IsBlank())
			pAction->sID = strPatternSubst(CONSTLIT("%d"), i);

		//	Description

		pAction->sDescID = pActionDesc->GetAttribute(DESC_ID_ATTRIB);

		//	Action

		pAction->pExtension = pExtension;
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

		//	Handle older settings. We only set these if they are specified.
		//	Otherwise, we take the values set by SetLabelDesc.

		pActionDesc->FindAttribute(KEY_ATTRIB, &pAction->sKey);
		pActionDesc->FindAttributeBool(DEFAULT_ATTRIB, &pAction->bDefault);
		pActionDesc->FindAttributeBool(CANCEL_ATTRIB, &pAction->bCancel);
		pActionDesc->FindAttributeBool(NEXT_KEY_ATTRIB, &pAction->bNext);
		pActionDesc->FindAttributeBool(PREV_KEY_ATTRIB, &pAction->bPrev);

		//	Defaults

		pAction->bVisible = true;
		pAction->bEnabled = true;
		pAction->bMinor = pActionDesc->GetAttributeBool(MINOR_ATTRIB);
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

void CDockScreenActions::ParseLabelDesc (const CString &sLabelDesc, CString *retsLabel, CString *retsKey, TArray<SpecialAttribs> *retSpecial)

//	ParseLabelDesc
//
//	Parses a label descriptor of the following forms:
//
//	Action:		This is a normal label
//	[A]ction:	A is the special key
//	[Enter]:	Treated as a normal label because key is > 1 character
//	*Action:	This is the default action
//	^Action:	This is the cancel action
//	>Action:	This is the next key
//	<Action:	This is the prev key

	{
	char *pPos = sLabelDesc.GetASCIIZPointer();

	//	Parse any special attribute prefixes

	while (*pPos == '*' || *pPos == '^' || *pPos == '>' || *pPos == '<')
		{
		if (retSpecial)
			{
			switch (*pPos)
				{
				case '*':
					retSpecial->Insert(specialDefault);
					break;

				case '^':
					retSpecial->Insert(specialCancel);
					break;

				case '>':
					retSpecial->Insert(specialNextKey);
					break;

				case '<':
					retSpecial->Insert(specialPrevKey);
					break;
				}
			}

		pPos++;
		}

	//	Now parse the label looking for the bracket syntax.

	CString sLabel;
	CString sKey;
	char *pStart = pPos;

	while (*pPos != '\0')
		{
		if (pPos[0] == '[' && pPos[1] != '\0' && pPos[2] == ']')
			{
			if (pStart)
				sLabel.Append(CString(pStart, (int)(pPos - pStart)));

			pPos++;
			if (*pPos == '\0')
				break;

			if (*pPos != ']')
				{
				sKey = CString(pPos, 1);
				sLabel.Append(sKey);
				pPos++;
				}

			if (*pPos == ']')
				pPos++;

			pStart = pPos;
			continue;
			}
		else
			pPos++;
		}

	if (pStart != pPos)
		sLabel.Append(CString(pStart, (int)(pPos - pStart)));

	//	Done

	if (retsLabel)
		*retsLabel = sLabel;

	if (retsKey)
		*retsKey = sKey;
	}

ALERROR CDockScreenActions::RemoveAction (int iAction)

//	RemoveAction
//
//	Removes the given action

	{
	SActionDesc *pAction = &m_Actions[iAction];
	if (pAction->pCode)
		{
		pAction->pCode->Discard(&g_pUniverse->GetCC());
		pAction->pCode = NULL;
		}

	m_Actions.Delete(iAction);

	return NOERROR;
	}

void CDockScreenActions::SetDesc (int iAction, const CString &sDesc)

//	SetDesc
//
//	Sets a description for an action

	{
	SActionDesc *pAction = &m_Actions[iAction];
	pAction->sDesc = sDesc;
	}

void CDockScreenActions::SetEnabled (int iAction, bool bEnabled)

//	SetEnabled
//
//	Enables the action

	{
	SActionDesc *pAction = &m_Actions[iAction];
	pAction->bEnabled = bEnabled;
	}

void CDockScreenActions::SetLabel (int iAction, const CString &sLabelDesc, const CString &sKey)

//	SetLabel
//
//	Sets the label and key for the actions

	{
	SActionDesc *pAction = &m_Actions[iAction];

	if (!sLabelDesc.IsBlank())
		SetLabelDesc(pAction, sLabelDesc, false);

	if (!sKey.IsBlank())
		pAction->sKey = sKey;
	}

void CDockScreenActions::SetLabelDesc (SActionDesc *pAction, const CString &sLabelDesc, bool bOverrideSpecial)

//	SetLabelDesc
//
//	Sets the label and key based on a label descriptor.
//
//	NOTE: We overwrite label, key, and special, so all other ways of specifying
//	those should happen after this call.

	{
	int i;

	CString sLabel;
	CString sKey;
	TArray<SpecialAttribs> Special;

	ParseLabelDesc(sLabelDesc, &sLabel, &sKey, &Special);

	pAction->sLabel = sLabel;
	pAction->sKey = sKey;

	if (bOverrideSpecial || Special.GetCount() > 0)
		{
		pAction->bCancel = false;
		pAction->bDefault = false;
		pAction->bNext = false;
		pAction->bPrev = false;

		for (i = 0; i < Special.GetCount(); i++)
			SetSpecial(pAction, Special[i], true);
		}
	}

void CDockScreenActions::SetSpecial (SActionDesc *pAction, SpecialAttribs iSpecial, bool bEnabled)

//	SetSpecial
//
//	Sets the special attribute for an action

	{
	switch (iSpecial)
		{
		case specialAll:
			pAction->bCancel = bEnabled;
			pAction->bDefault = bEnabled;
			pAction->bNext = bEnabled;
			pAction->bPrev = bEnabled;
			break;

		case specialDefault:
			pAction->bDefault = bEnabled;
			break;

		case specialCancel:
			pAction->bCancel = bEnabled;
			break;

		case specialNextKey:
			pAction->bNext = bEnabled;
			break;

		case specialPrevKey:
			pAction->bPrev = bEnabled;
			break;
		}
	}

void CDockScreenActions::SetSpecial (int iAction, SpecialAttribs iSpecial, bool bEnabled)

//	SetSpecial
//
//	Sets the special attribute for an action

	{
	SetSpecial(&m_Actions[iAction], iSpecial, bEnabled);
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
	}
