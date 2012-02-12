//	CommandLineWnd.cpp
//
//	This window implements a raw CodeChain command line

#include "Alchemy.h"
#include "TerraFirma.h"

//#define KEYPAD_ENTER_TO_EXECUTE	TRUE

static WNDPROC g_pfEditClass = NULL;

//	Forwards

LONG APIENTRY CommandLineWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);

//	Inlines

inline CCommandLineWnd *GetCtx (HWND hWnd) { return (CCommandLineWnd *)GetWindowLong(hWnd, GWL_ID); }

CCommandLineWnd::CCommandLineWnd (void) :
	m_pTF(NULL),
	m_hWnd(NULL)

//	CCommandLineWnd constructor

	{
	}

CCommandLineWnd::~CCommandLineWnd (void)

//	CCommandLineWnd destructor

	{
	if (m_hWnd)
		Destroy();
	}

ALERROR CCommandLineWnd::Create (CTerraFirma *pTF, HWND hParent, RECT rcRect)

//	Create
//
//	Create a new window

	{
	DWORD dwStyle;

	ASSERT(m_hWnd == NULL);

	m_pTF = pTF;

	//	First, create the window

	dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_VSCROLL;
	dwStyle |= ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN;

	m_hWnd = CreateWindow(
			"edit",
			"",
			dwStyle,
			rcRect.left, rcRect.top, RectWidth(&rcRect), RectHeight(&rcRect),
			hParent,
			(HMENU)this,
			m_pTF->GetInstance(),
			NULL);
	if (m_hWnd == NULL)
		return ERR_FAIL;

	//	Subclass the window

	g_pfEditClass = (WNDPROC)GetWindowLong(m_hWnd, GWL_WNDPROC);
	SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)CommandLineWndProc);

	return NOERROR;
	}

void CCommandLineWnd::Destroy (void)

//	Destroy
//
//	Destroy the window

	{
	ASSERT(m_hWnd);
	DestroyWindow(m_hWnd);
	m_hWnd = NULL;
	}

void CCommandLineWnd::ExecuteCommand (void)

//	ExecuteCommand
//
//	Executes the currently selected command

	{
	CString sCommand;
	CString sResult;

	//	Get the command

	sCommand = GetCommand();

	//	Process the command

	sResult = m_pTF->ExecuteCommand(sCommand);

	//	Output the result

	OutputResult(sResult);
	}

CString CCommandLineWnd::GetCommand (void)

//	GetCommand
//
//	Gets the command currently being executed by the user

	{
	DWORD dwStart, dwEnd;

	//	Get the selection

	SendMessage(m_hWnd, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);

	//	If we don't have a selection, then take the entire
	//	line as a command

	if (dwStart == dwEnd)
		{
		CString sLine;
		char *pPos;
		int iLine, iLength;

		iLine = (int)SendMessage(m_hWnd, EM_LINEFROMCHAR, (WPARAM)-1, 0);
		iLength = (int)SendMessage(m_hWnd, EM_LINELENGTH, (WPARAM)dwStart, 0);

		pPos = sLine.GetWritePointer(iLength + sizeof(WORD));

		//	The first WORD of the buffer specifies the number of character
		//	that fit (what a HACK!)

		*((WORD *)pPos) = (WORD)iLength;
		SendMessage(m_hWnd, EM_GETLINE, (WPARAM)iLine, (LPARAM)pPos);

		//	Now remove the WORD

		return strSubString(sLine, 0, iLength);
		}

	//	Otherwise, take the selection

	else
		{
		CString sBuffer;
		char *pPos;
		int iTotalSize;

		//	Get the entire buffer

		iTotalSize = SendMessage(m_hWnd, WM_GETTEXTLENGTH, 0, 0) + 1;
		pPos = sBuffer.GetWritePointer(iTotalSize);
		SendMessage(m_hWnd, WM_GETTEXT, (WPARAM)iTotalSize, (LPARAM)pPos);

		return strSubString(sBuffer, dwStart, dwEnd - dwStart);
		}
	}

CString CCommandLineWnd::GetData (void)

//	GetData
//
//	Returns the contents of the buffer

	{
	int iTotalSize;
	CString sBuffer;
	char *pPos;

	ASSERT(m_hWnd);

	iTotalSize = SendMessage(m_hWnd, WM_GETTEXTLENGTH, 0, 0) + 1;
	pPos = sBuffer.GetWritePointer(iTotalSize);
	SendMessage(m_hWnd, WM_GETTEXT, (WPARAM)iTotalSize, (LPARAM)pPos);

	return sBuffer;
	}

void CCommandLineWnd::LoadBuffer (CString sMapEntry)

//	LoadBuffer
//
//	Loads the command line buffer from the current library

	{
	ALERROR error;
	ICCItem *pBufferUNID;
	ICCItem *pBuffer;
	CLibraryFile *pLibrary = m_pTF->GetLibrary();
	CCodeChain *pCC = m_pTF->GetCC();
	CString sBuffer;

	ASSERT(pLibrary);

	//	Look for the map entry

	pBufferUNID = pLibrary->LoadMapEntry(sMapEntry);
	if (pBufferUNID->IsError())
		{
		CString sError = pBufferUNID->Print(pCC);
		OutputResult(sError);
		return;
		}

	//	If we found it, load the entry

	error = pLibrary->ReadEntry(pBufferUNID->GetIntegerValue(), &pBuffer);
	pBufferUNID->Discard(pCC);
	if (error)
		{
		OutputResult(LITERAL("Unable to load command line buffer"));
		return;
		}

	//	Convert to string

	sBuffer = pBuffer->GetStringValue();
	pBuffer->Discard(pCC);

	//	Set the text

	SetData(sBuffer);
	}

void CCommandLineWnd::OutputResult (CString sResult)

//	OutputResult
//
//	Output the result of a command execution

	{
	int iStart, iEnd;
	CString sInsertion;

	//	Get the selection

	SendMessage(m_hWnd, EM_GETSEL, (WPARAM)&iStart, (LPARAM)&iEnd);

	//	If we don't have a selection, then insert the result
	//	in the next line

	if (iStart == iEnd)
		{
		int iLine, iLength;

		//	Set the selection to the end of the line

		iLine = (int)SendMessage(m_hWnd, EM_LINEFROMCHAR, (WPARAM)-1, 0);
		iStart = (int)SendMessage(m_hWnd, EM_LINEINDEX, (WPARAM)iLine, 0);
		iLength = (int)SendMessage(m_hWnd, EM_LINELENGTH, (WPARAM)iStart, 0);
		SendMessage(m_hWnd, EM_SETSEL, (WPARAM)(iStart + iLength), (LPARAM)(iStart + iLength));
		}
	else
		{
		int iLine, iLength;

		//	Get the line that contains the end of the selection

		iLine = (int)SendMessage(m_hWnd, EM_LINEFROMCHAR, (WPARAM)iEnd, 0);
		iStart = (int)SendMessage(m_hWnd, EM_LINEINDEX, (WPARAM)iLine, 0);
		iLength = (int)SendMessage(m_hWnd, EM_LINELENGTH, (WPARAM)iStart, 0);
		SendMessage(m_hWnd, EM_SETSEL, (WPARAM)(iStart + iLength), (LPARAM)(iStart + iLength));
		}

	//	Add a return at the beginning

	sInsertion = LITERAL("\r\n");
	sInsertion.Append(sResult);
	sInsertion.Append(LITERAL("\r\n"));

	//	Insert the result

	SendMessage(m_hWnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)sInsertion.GetASCIIZPointer());
	}

void CCommandLineWnd::SaveBuffer (CString sMapEntry)

//	SaveBuffer
//
//	Saves the command line buffer to the current library

	{
	ALERROR error;
	ICCItem *pBufferUNID;
	ICCItem *pBuffer;
	ICCItem *pResult;
	CLibraryFile *pLibrary = m_pTF->GetLibrary();
	CCodeChain *pCC = m_pTF->GetCC();
	CString sBuffer;

	ASSERT(pLibrary);

	//	Get the entire buffer

	sBuffer = GetData();

	//	Create an item

	pBuffer = pCC->CreateString(sBuffer);
	if (pBuffer->IsError())
		{
		CString sError = pBuffer->Print(pCC);
		pBuffer->Discard(pCC);
		OutputResult(sError);
		return;
		}

	//	Look for the map entry

	pBufferUNID = pLibrary->LoadMapEntry(sMapEntry);

	//	If we could not find it, create a new one

	if (pBufferUNID->IsError())
		{
		int iUNID;

		if (pBufferUNID->GetIntegerValue() != CCRESULT_NOTFOUND)
			{
			CString sError = pBufferUNID->Print(pCC);
			pBuffer->Discard(pCC);
			pBufferUNID->Discard(pCC);
			OutputResult(sError);
			return;
			}

		pBufferUNID->Discard(pCC);

		//	Add the entry to the library and get an UNID back

		if (error = pLibrary->CreateEntry(pBuffer, &iUNID))
			{
			pBuffer->Discard(pCC);
			OutputResult(LITERAL("Unable to create new buffer entry"));
			return;
			}

		pBuffer->Discard(pCC);

		//	Create an UNID item

		pBufferUNID = pCC->CreateInteger(iUNID);
		if (pBufferUNID->IsError())
			{
			CString sError = pBufferUNID->Print(pCC);
			pBufferUNID->Discard(pCC);
			OutputResult(sError);
			return;
			}

		//	Store the unid in the map

		pResult = pLibrary->SaveMapEntry(sMapEntry, pBufferUNID);
		pBufferUNID->Discard(pCC);
		if (pResult->IsError())
			{
			CString sError = pResult->Print(pCC);
			OutputResult(sError);
			pResult->Discard(pCC);
			return;
			}

		pResult->Discard(pCC);
		return;
		}

	//	If we found it, save the entry

	error = pLibrary->WriteEntry(pBufferUNID->GetIntegerValue(), pBuffer);
	pBufferUNID->Discard(pCC);
	pBuffer->Discard(pCC);
	if (error)
		{
		OutputResult(LITERAL("Unable to load command line buffer"));
		return;
		}
	}

void CCommandLineWnd::SetData (CString sData)

//	SetData
//
//	Fills the buffer

	{
	ASSERT(m_hWnd);
	SendMessage(m_hWnd, WM_SETTEXT, 0, (LPARAM)sData.GetASCIIZPointer());
	}

LONG CCommandLineWnd::WMChar (WPARAM wParam, LPARAM lParam)

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

			SendMessage(m_hWnd, EM_GETSEL, (WPARAM)&iStart, (LPARAM)&iEnd);
			if (iStart == iEnd)
				{
				//	If we're at the beginning of a line, don't delete
				//	back to the previous line

				if (iStart == (int)SendMessage(m_hWnd, EM_LINEINDEX, (WPARAM)-1, 0))
					return 0;
				}

			return CallWindowProc((FARPROC)g_pfEditClass, m_hWnd, WM_CHAR, wParam, lParam);
			}

		case '\012':
			{
			//	if we've pressed Ctrl-Enter swallow the message because
			//	we handled it separately

			return 0;
			}

#ifdef KEYPAD_ENTER_TO_EXECUTE
		case '\015':
			{
			//	If we've pressed the Enter on the numeric pad
			//	the swallow this message because we want to handle it
			//	separately.

			if (lParam & 0x1000000L)
				return 0;
			else
				return CallWindowProc((FARPROC)g_pfEditClass, m_hWnd, WM_CHAR, wParam, lParam);
			}
#endif

		default:
			return CallWindowProc((FARPROC)g_pfEditClass, m_hWnd, WM_CHAR, wParam, lParam);
		}
	}

LONG CCommandLineWnd::WMKeyDown (WPARAM wParam, LPARAM lParam)

//	WMKeyDown
//
//	Handle WM_KEYDOWN

	{
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
			if ((lParam & 0x1000000L) || (IsControlDown()))
#else
			if (IsControlDown())
#endif
				{
				ExecuteCommand();
				return 0;
				}
			else
				return CallWindowProc((FARPROC)g_pfEditClass, m_hWnd, WM_KEYDOWN, wParam, lParam);
			}

		default:
			return CallWindowProc((FARPROC)g_pfEditClass, m_hWnd, WM_KEYDOWN, wParam, lParam);
		}
	}

LONG APIENTRY CommandLineWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)

//	CommandLineWndProc
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
			return CallWindowProc((FARPROC)g_pfEditClass, hWnd, message, wParam, lParam);
		}
	}
