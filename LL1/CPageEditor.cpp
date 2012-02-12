//	CPageEditor.cpp
//
//	CPageEditor object

#include "PreComp.h"

#define LEFT_MARGIN								4
#define RIGHT_MARGIN							4
#define TOP_MARGIN								4
#define BOTTOM_MARGIN							4
#define VERT_SEPARATOR							4

#define TAB_STOPS								12

static CObjectClass<CPageEditor>g_Class(OBJID_CPAGEEDITOR, NULL);

CPageEditor::CPageEditor (void) : CObject(&g_Class)

//	CPageEditor constructor

	{
	}

CPageEditor::CPageEditor (IUController *pController) : CObject(&g_Class),
		m_pController(pController),
		m_pFrame(NULL),
		m_pBody(NULL),
		m_pMeta(NULL),
		m_pCore(NULL)

//	CPageEditor constructor

	{
	}

ALERROR CPageEditor::Activate (CUFrame *pFrame, RECT *pRect)

//	Activate
//
//	Handle activation

	{
	ALERROR error;
	AutoSizeDesc AutoSize;
	RECT rcBody, rcMeta;
	CUFrame *pNewFrame;

	ASSERT(m_pBody == NULL);
	ASSERT(m_pMeta == NULL);

	m_rcRect = *pRect;
	m_pFrame = pFrame;

	//	Calc some metrics

	CalcMetrics(m_rcRect, &rcBody, &rcMeta);

	//	Create the body

	AutoSize.SetFixed(rcBody.left, rcBody.top, RectWidth(&rcBody), RectHeight(&rcBody));
	if (error = m_pFrame->GetWindow()->CreateFrame(m_pFrame, -1, 0, &AutoSize, &pNewFrame))
		return error;

	m_pBody = new CULineEditor(this);
	if (m_pBody == NULL)
		return ERR_MEMORY;

	m_pBody->SetActionMsg((ControllerNotifyProc)&CPageEditor::CmdAction);
	m_pBody->SetTabStops(TAB_STOPS);

	if (error = pNewFrame->SetContent(m_pBody, TRUE))
		return error;

	//	Create the meta data

	AutoSize.SetFixed(rcMeta.left, rcMeta.top, RectWidth(&rcMeta), RectHeight(&rcMeta));
	if (error = m_pFrame->GetWindow()->CreateFrame(m_pFrame, -1, 0, &AutoSize, &pNewFrame))
		return error;

	m_pMeta = new CULineEditor(this);
	if (m_pMeta == NULL)
		return ERR_MEMORY;

	m_pMeta->SetTabStops(TAB_STOPS);

	if (error = pNewFrame->SetContent(m_pMeta, TRUE))
		return error;

	return NOERROR;
	}

void CPageEditor::CalcMetrics (RECT &rcRect, RECT *retrcBody, RECT *retrcMeta)

//	CalcMetrics
//
//	Calculate the sizes of the body and the meta editors

	{
	int iBodyWidthNumerator;
	BOOL bWideMeta = FALSE;

	//	HACK: If the shift-key is down, make the meta editor wider. We do this
	//	for when we need to edit a lot of field code.

	if (IsShiftDown())
		iBodyWidthNumerator = 1;
	else
		iBodyWidthNumerator = 2;

	//	Place the body and the meta data side by side and make the body 2/3
	//	of the horizontal size.

	retrcBody->left = LEFT_MARGIN;
	retrcBody->right = retrcBody->left + iBodyWidthNumerator * (RectWidth(&rcRect) - (LEFT_MARGIN + RIGHT_MARGIN + VERT_SEPARATOR)) / 3;
	retrcBody->top = TOP_MARGIN;
	retrcBody->bottom = RectHeight(&rcRect) - BOTTOM_MARGIN;

	retrcMeta->left = retrcBody->right + VERT_SEPARATOR;
	retrcMeta->right = RectWidth(&rcRect) - RIGHT_MARGIN;
	retrcMeta->top = TOP_MARGIN;
	retrcMeta->bottom = RectHeight(&rcRect) - BOTTOM_MARGIN;
	}

int CPageEditor::CmdAction (DWORD dwTag, DWORD dwDummy2)

//	CmdAction
//
//	Execute a command from the body field

	{
	if (m_pCore)
		{
		CString sCommand;
		CString sResult;

		//	If we have a selection, ask for that; otherwise, get the
		//	current line

		if (m_pBody->HasSelection())
			sCommand = m_pBody->GetSelection();
		else
			sCommand = m_pBody->GetLine(-1);

		//	Execute the command

		sResult = m_pCore->ExecuteCommand(sCommand);

		//	Insert the result

		m_pBody->GotoEndOfLine();
		m_pBody->InsertText(strPatternSubst(LITERAL("\r\n%s\r\n"), sResult));
		}

	return 0;
	}

void CPageEditor::Deactivate (void)

//	Deactivate
//
//	Handle deactivation

	{
	//	We got no frame!

	m_pFrame = NULL;
	}

ALERROR CPageEditor::GetPage (CCore *pCore, CEntry **retpEntry)

//	GetPage
//
//	Returns the entry

	{
	ALERROR error;
	CEntry *pEntry;
	CString sError;
	int iErrorPos;

	//	First load the meta data

	if (error = CEntry::LoadFromXML(pCore, m_pMeta->GetData(), &pEntry, &sError))
		{
		MessageBox(m_pFrame->GetWindow()->GetHWND(), sError.GetASCIIZPointer(), "Parse Error", MB_OK | MB_ICONERROR);
		return ERR_FAIL;
		}

	//	Make sure the body is properly formatted

	if (error = XMLValidate(m_pBody->GetData(), &sError, &iErrorPos))
		{
		MessageBox(m_pFrame->GetWindow()->GetHWND(), sError.GetASCIIZPointer(), "Parse Error", MB_OK | MB_ICONERROR);
		return ERR_FAIL;
		}

	//	Now load the body

	pEntry->SetBody(m_pBody->GetData());

	//	Done

	*retpEntry = pEntry;

	return NOERROR;
	}

void CPageEditor::Resize (RECT *pRect)

//	Resize
//
//	Handle a resize

	{
	}

ALERROR CPageEditor::SetPage (CEntry *pEntry)

//	SetPage
//
//	Sets the page to be editted

	{
	ALERROR error;

	ASSERT(m_pBody);
	ASSERT(m_pMeta);

	//	Get the body and place it in the appropriate editor

	m_pBody->SetData(pEntry->GetBody());

	//	Temporarily remove the body and output to XML to generate the
	//	meta data

	CString sBody = pEntry->GetBody();
	pEntry->SetBody(LITERAL(""));

	CString sMetaData;
	error = CEntry::SaveToXML(pEntry, &sMetaData);
	pEntry->SetBody(sBody);

	if (error)
		return error;

	m_pMeta->SetData(sMetaData);

	return NOERROR;
	}
