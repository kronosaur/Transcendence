//	CULineEditor.cpp
//
//	CULineEditor object

#include "Alchemy.h"
#include "ALGUI.h"

static CObjectClass<CULineEditor>g_Class(OBJID_CULINEEDITOR, NULL);
static WNDPROC g_pfEditClass = NULL;

//	Forwards

LONG APIENTRY LineEditorWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);

//	Inlines

inline CULineEditor *GetCtx (HWND hWnd) { return (CULineEditor *)GetWindowLong(hWnd, GWL_ID); }

CULineEditor::CULineEditor (void) : CObject(&g_Class)

//	CULineEditor constructor

	{
	}

CULineEditor::CULineEditor (IUController *pController) : CObject(&g_Class),
		m_pController(pController),
		m_hEditWnd(NULL),
		m_fSingleLine(FALSE),
		m_fMSWinUI(FALSE),
		m_pfActionMsg(NULL),
		m_iTabs(0)

//	CULineEditor constructor

	{
	m_Font.SetHeightInPixels(14);
	m_Font.SetTypeface(LITERAL("Lucida Console"));
	}

ALERROR CULineEditor::Activate (CUFrame *pFrame, RECT *pRect)

//	Activate
//
//	Activate content

	{
	ALERROR error;
	DWORD dwStyle;
	DWORD dwStyleEx;
	CUWindow *pWindow = pFrame->GetWindow();

	ASSERT(m_hEditWnd == NULL);

	//	Figure out the styles for the window

	dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE;
	if (m_fSingleLine)
		dwStyle |= ES_AUTOHSCROLL;
	else
		dwStyle |= ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL;

	dwStyleEx = 0;
	if (m_fMSWinUI)
		dwStyleEx |= WS_EX_CLIENTEDGE;

	//	Create the window

	if (error = pWindow->CreateChildWindow(pFrame,
			"edit",
			dwStyle,
			dwStyleEx,
			(int)this,
			&m_hEditWnd))
		return error;

	//	Subclass the window

	g_pfEditClass = (WNDPROC)GetWindowLong(m_hEditWnd, GWL_WNDPROC);
	SetWindowLong(m_hEditWnd, GWL_WNDPROC, (LONG)LineEditorWndProc);

	//	Set the font for the editor

	SendMessage(m_hEditWnd, 
			WM_SETFONT,
			(WPARAM)m_Font.GetFont(),
			MAKELPARAM(FALSE, 0));

	//	Set the tab stops

	if (m_iTabs > 0)
		SendMessage(m_hEditWnd, EM_SETTABSTOPS, (WPARAM)1, (LPARAM)&m_iTabs);

	return NOERROR;
	}

void CULineEditor::Deactivate (void)

//	Deactivate
//
//	Deactivate content

	{
	ASSERT(m_hEditWnd);

	DestroyWindow(m_hEditWnd);
	}

ALERROR CULineEditor::ExecuteCommand (void)

//	ExecuteCommand
//
//	Executes the currently selected command

	{
	CallNotifyProc(m_pController, m_pfActionMsg, 0, 0);
	return NOERROR;
	}

CString CULineEditor::GetData (void)

//	GetData
//
//	Returns the contents

	{
	CString sData;

	if (m_hEditWnd)
		{
		int iLen = GetWindowTextLength(m_hEditWnd);
		char *pPos;

		pPos = sData.GetWritePointer(iLen);
		GetWindowText(m_hEditWnd, pPos, iLen + 1);
		}

	return sData;
	}

CString CULineEditor::GetLine (int iLine)

//	GetLine
//
//	Returns the given line (0-based). If iLine is -1 it returns the
//	line at the cursor

	{
	CString sLine;
	int iLength;
	char *pPos;

	//	First we need to figure out the length of the given line. Note that
	//	EM_LINELENGTH needs a character offset, so we have to convert from
	//	a line index to a character pos.

	iLength = (int)SendMessage(m_hEditWnd, 
			EM_LINELENGTH, 
			(WPARAM)SendMessage(m_hEditWnd, EM_LINEINDEX, (WPARAM)iLine, 0),
			0);

	//	Get a buffer to store the line.

	pPos = sLine.GetWritePointer(iLength);

	//	If iLine is -1, get the correct line offset

	if (iLine == -1)
		iLine = (int)SendMessage(m_hEditWnd, EM_LINEFROMCHAR, (WPARAM)-1, 0);

	//	The first WORD of the buffer specifies the number of character
	//	that fit (what a HACK!)

	*((WORD *)pPos) = (WORD)iLength;
	SendMessage(m_hEditWnd, EM_GETLINE, (WPARAM)iLine, (LPARAM)pPos);

	//	Done

	return sLine;
	}

CString CULineEditor::GetSelection (void)

//	GetSelection
//
//	Returns the contents of the selection. If there is no selection, it
//	returns a blank string

	{
	CString sBuffer;
	char *pPos;
	int iTotalSize;
	DWORD dwStart, dwEnd;

	//	Get the selection

	SendMessage(m_hEditWnd, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);

	//	Get the entire buffer

	iTotalSize = SendMessage(m_hEditWnd, WM_GETTEXTLENGTH, 0, 0) + 1;
	pPos = sBuffer.GetWritePointer(iTotalSize);
	SendMessage(m_hEditWnd, WM_GETTEXT, (WPARAM)iTotalSize, (LPARAM)pPos);

	return strSubString(sBuffer, dwStart, dwEnd - dwStart);
	}

void CULineEditor::GotoEndOfLine (void)

//	GotoEndOfLine
//
//	Moves the caret to the end of the current line
//	If there is a selection, it moves the caret to the end of the line
//	containing the end of the selection.

	{
	int iStart, iEnd;

	//	Get the selection

	SendMessage(m_hEditWnd, EM_GETSEL, (WPARAM)&iStart, (LPARAM)&iEnd);

	//	If we don't have a selection, then go to the end of the line with
	//	the caret.

	if (iStart == iEnd)
		{
		int iLength;

		//	Set the selection to the end of the line

		iStart = (int)SendMessage(m_hEditWnd, EM_LINEINDEX, (WPARAM)-1, 0);
		iLength = (int)SendMessage(m_hEditWnd, EM_LINELENGTH, (WPARAM)iStart, 0);
		SendMessage(m_hEditWnd, EM_SETSEL, (WPARAM)(iStart + iLength), (LPARAM)(iStart + iLength));
		}
	else
		{
		int iLine, iLength;

		//	Get the line that contains the end of the selection

		iLine = (int)SendMessage(m_hEditWnd, EM_LINEFROMCHAR, (WPARAM)iEnd, 0);
		iStart = (int)SendMessage(m_hEditWnd, EM_LINEINDEX, (WPARAM)iLine, 0);
		iLength = (int)SendMessage(m_hEditWnd, EM_LINELENGTH, (WPARAM)iStart, 0);
		SendMessage(m_hEditWnd, EM_SETSEL, (WPARAM)(iStart + iLength), (LPARAM)(iStart + iLength));
		}
	}

BOOL CULineEditor::HasSelection (void)

//	HasSelection
//
//	Returns TRUE if the editor has a selection

	{
	DWORD dwStart, dwEnd;
	SendMessage(m_hEditWnd, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	return (dwStart != dwEnd);
	}

void CULineEditor::InsertText (CString sText)

//	InsertText
//
//	Inserts text at the selection

	{
	SendMessage(m_hEditWnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)sText.GetASCIIZPointer());
	}

void CULineEditor::Resize (RECT *pRect)

//	Resize
//
//	Handle a resize

	{
	if (m_hEditWnd)
		MoveWindow(m_hEditWnd, pRect->left, pRect->top, RectWidth(pRect), RectHeight(pRect), TRUE);
	}

void CULineEditor::SetData (CString sData)

//	SetData
//
//	Sets the contents

	{
	if (m_hEditWnd)
		SetWindowText(m_hEditWnd, sData.GetASCIIZPointer());
	}

void CULineEditor::SetFocus (void)

//	SetFocus
//
//	Sets the focus here

	{
	if (m_hEditWnd)
		::SetFocus(m_hEditWnd);
	}

LONG CULineEditor::WMChar (WPARAM wParam, LPARAM lParam)

//	WMChar
//
//	Handle WM_CHAR

	{
	char chChar = (char)wParam;

	switch (chChar)
		{
		case '\010':
			{
			int iStart, iEnd;

			//	If we've got a caret

			SendMessage(m_hEditWnd, EM_GETSEL, (WPARAM)&iStart, (LPARAM)&iEnd);
			if (iStart == iEnd)
				{
				//	If we're at the beginning of a line, don't delete
				//	back to the previous line

				if (iStart == (int)SendMessage(m_hEditWnd, EM_LINEINDEX, (WPARAM)-1, 0))
					return 0;
				}

			return CallWindowProc((WNDPROC)g_pfEditClass, m_hEditWnd, WM_CHAR, wParam, lParam);
			}

		case '\012':
			{
			//	if we've pressed Ctrl-Enter swallow the message because
			//	we handled it separately

			return 0;
			}

		case '\015':
			{
			//	If we've pressed the Enter on the numeric pad
			//	the swallow this message because we want to handle it
			//	separately.

#ifdef KEYPAD_ENTER_TO_EXECUTE
			if (lParam & 0x1000000L)
				return 0;
#endif
			//	If this is a single line, swallow the message because
			//	we don't want the control to beep.

			if (m_fSingleLine)
				return 0;

			return CallWindowProc((WNDPROC)g_pfEditClass, m_hEditWnd, WM_CHAR, wParam, lParam);
			}

		default:
			return CallWindowProc((WNDPROC)g_pfEditClass, m_hEditWnd, WM_CHAR, wParam, lParam);
		}
	}

LONG CULineEditor::WMKeyDown (WPARAM wParam, LPARAM lParam)

//	WMKeyDown
//
//	Handle WM_KEYDOWN

	{
	ALERROR error;
	int iVirtKey = (int)wParam;

	switch (iVirtKey)
		{
		case VK_RETURN:
			{
			//	If we've pressed the Enter on the numeric pad
			//	or if the control key is down, then execute
			//	the command.
			//
			//	Bit 24 of lParam differentiates between the two
			//	enters.

#ifdef KEYPAD_ENTER_TO_EXECUTE
			if ((lParam & 0x1000000L) || (IsControlDown()) || m_fSingleLine)
#else
			if (IsControlDown() || m_fSingleLine)
#endif
				{
				if (error = ExecuteCommand())
					m_pController->DisplayError(error);
				return 0;
				}
			else
				return CallWindowProc((WNDPROC)g_pfEditClass, m_hEditWnd, WM_KEYDOWN, wParam, lParam);
			}

		default:
			return CallWindowProc((WNDPROC)g_pfEditClass, m_hEditWnd, WM_KEYDOWN, wParam, lParam);
		}
	}

LONG APIENTRY LineEditorWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)

//	LineEditorWndProc
//
//	Subclass of an edit control

	{
	switch (message)
		{
		case WM_CHAR:
			return GetCtx(hWnd)->WMChar(wParam, lParam);

		case WM_KEYDOWN:
			return GetCtx(hWnd)->WMKeyDown(wParam, lParam);

		default:
			return CallWindowProc((WNDPROC)g_pfEditClass, hWnd, message, wParam, lParam);
		}
	}
