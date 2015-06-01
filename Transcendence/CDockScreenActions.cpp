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

const int CONTROL_BORDER_RADIUS =		4;
const int CONTROL_INNER_PADDING_HORZ =	8;
const int CONTROL_INNER_PADDING_VERT =	8;

const int AREA_PADDING =			64;

struct SSpecialDesc
	{
	char *pszAttrib;
	CDockScreenActions::SpecialAttribs dwSpecial;
	};

SSpecialDesc SPECIAL_DESC[] =
	{
		{	"cancel",	CDockScreenActions::specialCancel },
		{	"default",	CDockScreenActions::specialDefault },
		{	"nextKey",	CDockScreenActions::specialNextKey },
		{	"pgDnKey",	CDockScreenActions::specialPgDnKey },
		{	"pgUpKey",	CDockScreenActions::specialPgUpKey },
		{	"prevKey",	CDockScreenActions::specialPrevKey },
	};

const int SPECIAL_DESC_COUNT =	(sizeof(SPECIAL_DESC) / sizeof(SPECIAL_DESC[0]));

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

int CDockScreenActions::CalcAreaHeight (CDesignType *pRoot, const RECT &rcFrame)

//	CalcAreaHeight
//
//	Compute the height of all buttons.

	{
	//	Force a justification

	m_cxJustify = -1;
	return Justify(pRoot, RectWidth(rcFrame));
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

	//	Make sure we're justified

	Justify(pRoot, RectWidth(rcFrame));

	//	We create buttons in one of two areas. Major buttons are at the top of
	//	the frame. Minor buttons are at the bottom. We start by counting the 
	//	number of minor buttons.

	int yMajor = rcFrame.top;
	int yMinor = Max(rcFrame.top + m_cyMajorButtons,
			rcFrame.bottom - BOTTOM_MARGIN_Y - (m_iMinorButtonCount * ACTION_BUTTON_HEIGHT) - ((m_iMinorButtonCount - 1) * ACTION_BUTTON_SPACING));

	//	Padding

	int xLeft = rcFrame.left + (m_bLongButtons ? 0 : AREA_PADDING);
	int xRight = rcFrame.right - (m_bLongButtons ? 0 : AREA_PADDING);

	//	Create all buttons

	for (i = 0; i < GetCount(); i++)
		{
		SActionDesc *pAction = &m_Actions[i];
		if (!pAction->bVisible)
			continue;

		//	Add the button

		CGButtonArea *pButton = new CGButtonArea;

		//	Set the label and key (which we already computed in the justify
		//	pass).

		pButton->SetLabel(pAction->sLabelTmp);
		pButton->SetLabelAccelerator(pAction->sKeyTmp);
		pButton->SetDesc(pAction->sDescTmp);

		//	Set common properties

		pButton->SetDisabled(!pAction->bEnabled);
		pButton->SetLabelColor(VI.GetColor(colorTextHighlight));

		//	These properties of the button depend on the type (major or minor)

		if (pAction->bMinor)
			{
			pButton->SetLabelFont(&MinorLabelFont);
			pButton->SetAcceleratorColor(VI.GetColor(colorAreaAccelerator));

			RECT rcArea;
			rcArea.left = xLeft + CONTROL_INNER_PADDING_HORZ;
			rcArea.top = yMinor;
			rcArea.right = xRight;
			rcArea.bottom = yMinor + ACTION_BUTTON_HEIGHT;
			pFrame->AddArea(pButton, rcArea, dwFirstTag + i);

			yMinor += ACTION_BUTTON_HEIGHT + ACTION_BUTTON_SPACING;
			}
		else
			{
			pButton->SetLabelFont(&MajorLabelFont);
			pButton->SetAcceleratorColor(VI.GetColor(colorTextAccelerator));
			pButton->SetPadding(CONTROL_INNER_PADDING_VERT);
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

CDockScreenActions::SpecialAttribs CDockScreenActions::GetSpecialFromName (const CString &sSpecialName)

//	GetSpecialFromName
//
//	Converts from a name to special attribute

	{
	int i;

	for (i = 0; i < SPECIAL_DESC_COUNT; i++)
		if (strEquals(sSpecialName, CString(SPECIAL_DESC[i].pszAttrib, -1, true)))
			return SPECIAL_DESC[i].dwSpecial;

	return specialNone;
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
	int i, j;

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
		for (j = 0; j < SPECIAL_DESC_COUNT; j++)
			{
			bool bSpecial;
			if (pActionDesc->FindAttributeBool(CString(SPECIAL_DESC[j].pszAttrib, -1, true), &bSpecial))
				pAction->dwSpecial |= SPECIAL_DESC[j].dwSpecial;
			}

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
	return ((m_Actions[iAction].dwSpecial & iSpecial) ? true : false);
	}

int CDockScreenActions::Justify (CDesignType *pRoot, int cxJustify)

//	Justify
//
//	Justifies all buttons and generates heights. We return the total height of 
//	all buttons.

	{
	const CVisualPalette &VI = g_pHI->GetVisuals();
	int i;

	//	If we're already justified, nothing to do

	if (m_cxJustify == cxJustify)
		return m_cyTotalHeight;

	//	Get some fonts

	const CG16bitFont &MajorLabelFont = VI.GetFont(fontMediumHeavyBold);
	const CG16bitFont &MinorLabelFont = VI.GetFont(fontMedium);

	//	Loop over all buttons and get some stats about them. We also generate
	//	all labels because we might need to grow the (horizontal) size of buttons
	//	depending on it.

	m_bLongButtons = false;
	m_iMinorButtonCount = 0;
	for (i = 0; i < GetCount(); i++)
		{
		SActionDesc *pAction = &m_Actions[i];
		if (!pAction->bVisible)
			continue;

		if (pAction->bMinor)
			m_iMinorButtonCount++;

		if (!pAction->sDesc.IsBlank() || !pAction->sDescID.IsBlank())
			m_bLongButtons = true;

		//	If the label is currently blank, then look up the ID in the language
		//	table and see if we have something.

		CString sLabelDesc;
		if (pAction->sLabel.IsBlank() 
				&& pRoot
				&& !pAction->sID.IsBlank()
				&& pRoot->TranslateText(NULL, pAction->sID, NULL, &sLabelDesc))
			{
			TArray<CDockScreenActions::SpecialAttribs> Special;
			ParseLabelDesc(sLabelDesc, &pAction->sLabelTmp, &pAction->sKeyTmp, &Special);

			//	We need to set the action key because we have to check for it
			//	during input.

			if (!pAction->sKeyTmp.IsBlank())
				pAction->sKey = pAction->sKeyTmp;

			//	Set any special accelerators too (but only if we defined them).

			if (Special.GetCount() > 0)
				SetSpecial(pAction, Special);
			}
		else
			{
			pAction->sLabelTmp = pAction->sLabel;
			pAction->sKeyTmp = pAction->sKey;
			}

		//	If we've got a quoted label, then make it longer

		char *pPos = pAction->sLabelTmp.GetASCIIZPointer();
		if (*pPos == '\"' || *pPos == '“')
			m_bLongButtons = true;

		//	If we have a description, set that

		if (!pAction->sDesc.IsBlank())
			pAction->sDescTmp = pAction->sDesc;

		//	Otherwise, if we have a description ID, then translate

		else if (!pAction->sDescID.IsBlank() && pRoot)
			{
			if (!pRoot->TranslateText(NULL, pAction->sDescID, NULL, &pAction->sDescTmp))
				pAction->sDescTmp = NULL_STR;
			}
		else
			pAction->sDescTmp = NULL_STR;
		}

	//	Now add up all the heights

	int cxWidth = cxJustify;
	if (!m_bLongButtons)
		cxWidth -= (2 * AREA_PADDING);

	m_cyTotalHeight = 0;
	m_cyMajorButtons = 0;
	for (i = 0; i < GetCount(); i++)
		{
		SActionDesc *pAction = &m_Actions[i];
		if (!pAction->bVisible)
			continue;

		//	Minor buttons are fixed height

		if (pAction->bMinor)
			m_cyTotalHeight += ACTION_BUTTON_HEIGHT + ACTION_BUTTON_SPACING;

		//	Major buttons need to be justified

		else
			{
			int cyHeight = 0;

			//	Top padding and label

			cyHeight += CONTROL_INNER_PADDING_VERT;
			cyHeight += MajorLabelFont.GetHeight();

			//	Description

			if (!pAction->sDescTmp.IsBlank())
				{
				int cxTextWidth = cxWidth - (2 * CONTROL_INNER_PADDING_HORZ);
				int iLines = VI.GetFont(fontMedium).BreakText(pAction->sDescTmp, cxTextWidth, NULL);
				cyHeight += iLines * VI.GetFont(fontMedium).GetHeight();
				}

			//	Bottom

			cyHeight += CONTROL_INNER_PADDING_VERT + ACTION_BUTTON_SPACING;

			m_cyTotalHeight += cyHeight;
			m_cyMajorButtons += cyHeight;
			}
		}

	//	Bottom margin

	m_cyTotalHeight += BOTTOM_MARGIN_Y;

	//	Done

	m_cxJustify = cxJustify;
	return m_cyTotalHeight;
	}

void CDockScreenActions::ParseLabelDesc (const CString &sLabelDesc, CString *retsLabel, CString *retsKey, TArray<SpecialAttribs> *retSpecial)

//	ParseLabelDesc
//
//	Parses a label descriptor of the following forms:
//
//	Action:				This is a normal label
//	[PageDown] Action:	This is a multi-key accelerator
//	[A]ction:			A is the special key
//	[Enter]:			Treated as a normal label because key is > 1 character
//	*Action:			This is the default action
//	^Action:			This is the cancel action
//	>Action:			This is the next key
//	<Action:			This is the prev key

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

	//	Parse out the label and any accelerator key

	CString sLabel;
	CString sKey;

	//	See if we have a multi-key accelerator label

	char *pStart = NULL;
	char *pAccelStart = NULL;
	char *pAccelEnd = NULL;
	if (*pPos == '[')
		{
		pStart = pPos;
		while (*pStart != '\0' && *pStart != ']')
			pStart++;

		if (*pStart == ']' && pStart[1] != '\0')
			{
			pAccelEnd = pStart;

			pStart++;
			while (*pStart == ' ')
				pStart++;

			//	Must be more than 1 character long

			pAccelStart = pPos + 1;
			if ((int)(pAccelEnd - pAccelStart) <= 1)
				pStart = NULL;
			}
		else
			pStart = NULL;
		}

	//	If we found a multi-key accelerator label, use that

	if (pStart)
		{
		sLabel = CString(pStart);

		//	Look up this key by name

		CString sKeyName = CString(pAccelStart, (int)(pAccelEnd - pAccelStart));
		DWORD dwKey = CGameKeys::GetKey(sKeyName);
		
		//	If this is a valid key, then we use it as an accelerator 
		//	and special key.

		if (dwKey != INVALID_VIRT_KEY)
			{
			sKey = CGameKeys::GetKeyLabel(dwKey);

			//	We only support a limited number of keys

			switch (dwKey)
				{
				case VK_DOWN:
				case VK_RIGHT:
					retSpecial->Insert(specialNextKey);
					break;

				case VK_ESCAPE:
					retSpecial->Insert(specialCancel);
					break;

				case VK_LEFT:
				case VK_UP:
					retSpecial->Insert(specialPrevKey);
					break;

				case VK_NEXT:
					retSpecial->Insert(specialPgDnKey);
					break;

				case VK_PRIOR:
					retSpecial->Insert(specialPgUpKey);
					break;

				case VK_RETURN:
					retSpecial->Insert(specialDefault);
					break;
				}
			}

		//	Otherwise, just an accelerator

		else
			sKey = sKeyName;
		}

	//	Otherwise, parse for an embedded label using the bracket syntax.

	else
		{
		pStart = pPos;

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
		}

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
	CString sLabel;
	CString sKey;
	TArray<SpecialAttribs> Special;

	ParseLabelDesc(sLabelDesc, &sLabel, &sKey, &Special);

	pAction->sLabel = sLabel;
	pAction->sKey = sKey;

	if (bOverrideSpecial || Special.GetCount() > 0)
		SetSpecial(pAction, Special);
	}

void CDockScreenActions::SetSpecial (SActionDesc *pAction, const TArray<SpecialAttribs> &Special)

//	SetSpecial
//
//	Sets all special attributes

	{
	int i;

	pAction->dwSpecial = specialNone;

	for (i = 0; i < Special.GetCount(); i++)
		pAction->dwSpecial |= Special[i];
	}

void CDockScreenActions::SetSpecial (SActionDesc *pAction, SpecialAttribs iSpecial, bool bEnabled)

//	SetSpecial
//
//	Sets the special attribute for an action

	{
	if (bEnabled)
		pAction->dwSpecial |= iSpecial;
	else
		pAction->dwSpecial &= ~iSpecial;
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
		SpecialAttribs iSpecial = GetSpecialFromName(sSpecial);
		if (iSpecial == specialNone)
			{
			if (retpError)
				*retpError = CC.CreateError(CONSTLIT("Unknown action special key"), pSpecial->GetElement(i));
			return false;
			}

		SetSpecial(iAction, iSpecial);
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
