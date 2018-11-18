//	CCommandLineDisplay.cpp
//
//	CCommandLineDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define BACK_COLOR					(CG32bitPixel(20, 20, 20))
#define TEXT_COLOR					(CG32bitPixel(200, 200, 200))
#define INPUT_COLOR					(CG32bitPixel(255, 255, 200))
#define HINT_COLOR					(CG32bitPixel(200, 200, 255))

const int LEFT_SPACING =			2;
const int RIGHT_SPACING =			2;
const int TOP_SPACING =				2;
const int BOTTOM_SPACING =			2;

const int CURSOR_WIDTH =			10;

CCommandLineDisplay::~CCommandLineDisplay (void)

//	CCommandLineDisplay destructor

	{
	CleanUp();
	}

void CCommandLineDisplay::AppendOutput (const CString &sLine, CG32bitPixel rgbColor)

//	AppendOutput
//
//	Append a line of output

	{
	m_iOutputStart = (m_iOutputStart + (MAX_LINES + 1) - 1) % (MAX_LINES + 1);
	if (m_iOutputStart == m_iOutputEnd)
		m_iOutputEnd = (m_iOutputEnd + (MAX_LINES + 1) - 1) % (MAX_LINES + 1);

	m_Output[m_iOutputStart] = sLine;
	m_OutputColor[m_iOutputStart] = rgbColor;

	m_bInvalid = true;
	}

void CCommandLineDisplay::CleanUp (void)

//	CleanUp
//
//	Free all resource

	{
	m_Buffer.CleanUp();
	m_bInvalid = true;
	}

const CString &CCommandLineDisplay::GetOutput (int iLine)

//	GetOutput
//
//	Returns the output line

	{
	return m_Output[(m_iOutputStart + iLine) % (MAX_LINES + 1)];
	}

CG32bitPixel CCommandLineDisplay::GetOutputColor (int iLine)

//	GetOutputColor
//
//	Returns the output line color

	{
	return m_OutputColor[(m_iOutputStart + iLine) % (MAX_LINES + 1)];
	}

int CCommandLineDisplay::GetOutputCount (void)

//	GetOutputCount
//
//	Returns the number of lines in the output buffer

	{
	if (m_iOutputStart == m_iOutputEnd)
		return 0;
	else
		return ((m_iOutputEnd + MAX_LINES + 1 - m_iOutputStart) % (MAX_LINES + 1));
	}

void CCommandLineDisplay::AppendHistory (const CString &sLine)

//	AppendHistory
//
//	Append a line of input to the history buffer

	{
	//	Use case-sensitive compare because sometimes commands differ only
	//	by case.

	if (!strEqualsCase(sLine, GetHistory(0)))
		{
		m_iHistoryStart = (m_iHistoryStart + (MAX_LINES + 1) - 1) % (MAX_LINES + 1);
		if (m_iHistoryStart == m_iHistoryEnd)
			m_iHistoryEnd = (m_iHistoryEnd + (MAX_LINES + 1) - 1) % (MAX_LINES + 1);

		m_History[m_iHistoryStart] = sLine;
		}

	m_iHistoryIndex = -1;
	}

const CString &CCommandLineDisplay::GetHistory (int iLine)

//	GetHistory
//
//	Returns the history line

	{
	return m_History[(m_iHistoryStart + iLine) % (MAX_LINES + 1)];
	}

int CCommandLineDisplay::GetHistoryCount (void)

//	GetHistoryCount
//
//	Returns the number of lines in the history buffer

	{
	if (m_iHistoryStart == m_iHistoryEnd)
		return 0;
	else
		return ((m_iHistoryEnd + MAX_LINES + 1 - m_iHistoryStart) % (MAX_LINES + 1));
	}

const CString CCommandLineDisplay::GetCurrentCmd (void)

//	GetCurrentCmd
//
//	Returns the command fragment under the cursor from the input line

	{
	int iPos;
	CString sWord;

	// Want to extract the word to the left of cursor
	for (iPos = m_iCursorPos - 1; iPos >= 0; iPos--)
		{
		char *pPos = m_sInput.GetASCIIZPointer() + iPos;
		if (*pPos == ' ' || *pPos == '(')
			{
			iPos++;
			break;
			}
		if (*pPos == ')' || *pPos == '\'' || *pPos == '"')
			return NULL_STR;
		}

	if (iPos < 0) iPos = 0;
	return strSubString(m_sInput, iPos, m_iCursorPos - iPos);
	}

void CCommandLineDisplay::AutoCompleteSearch (void)

//	AutocompleteSearch
//
//	Searches the global symbol table for matches to the current command.

	{
	const CString sCurCmd = GetCurrentCmd();
	CString sCommon;
	CString sHelp;

	ClearHint();
	if (sCurCmd.IsBlank())
		return;

	//	Get the list of global symbols

	ICCItem *pGlobals = g_pUniverse->GetCC().GetGlobals();

	int iMatches = 0;

	for (int i = 0; i < pGlobals->GetCount(); i++)
		{
		CString sGlobal = pGlobals->GetKey(i);

		//	Partial match
		if (strStartsWith(sGlobal, sCurCmd))
			{
			if (iMatches == 0)
				sCommon = sGlobal;
			//	If we have multiple matching commands then find the longest common stem
			else
				{
				int iLen = min(sCommon.GetLength(), sGlobal.GetLength());
				char *pPos1 = sCommon.GetPointer();
				char *pPos2 = sGlobal.GetPointer();
				int i;
				for (i = 0; i < iLen; i++)
					{
					if (CharLower((LPTSTR)(BYTE)(*pPos1)) != CharLower((LPTSTR)(BYTE)(*pPos2)))
						break;
					pPos1++;
					pPos2++;
					}
				sCommon.Truncate(i);
				m_sHint.Append(CONSTLIT(" "));
				}
			//	Append the command to the auto complete hint
			m_sHint.Append(sGlobal);
			iMatches++;
			}

		if (strEquals(sGlobal, sCurCmd))
			{
			//	Exact match - get help text
			ICCItem *pItem = pGlobals->GetElement(i);
			if (pItem->IsPrimitive())
				sHelp = pItem->GetHelp();
			}
		}

	//	If the common stem is longer than the current command, then auto complete
	if (sCommon.GetLength() > sCurCmd.GetLength())
		Input(strSubString(sCommon, sCurCmd.GetLength(), -1));

	//	If we only have one match then no need to show hint as we have either
	//	auto completed or will show help text insead
	if (iMatches == 1)
		m_sHint = NULL_STR;

	if (!sHelp.IsBlank())
		{
		if (!m_sHint.IsBlank())
			m_sHint.Append(CONSTLIT("\n"));
		m_sHint.Append(sHelp);
		}
	}

ALERROR CCommandLineDisplay::Init (const RECT &rcRect)

//	Init
//
//	Initialize

	{
	m_pFont = &m_VI.GetFont(fontConsoleMediumHeavy);
	if (m_pFont == NULL)
		return ERR_FAIL;

	m_rcRect = rcRect;
	m_iOutputStart = 0;
	m_iOutputEnd = 0;
	m_iHistoryStart = 0;
	m_iHistoryEnd = 0;
	m_iHistoryIndex = -1;
	m_iCursorPos = 0;
	m_iScrollPos = 0;
	m_sInput = NULL_STR;
	m_bInvalid = true;

	return NOERROR;
	}

void CCommandLineDisplay::Input (const CString &sInput)

//	Input
//
//	Add characters to input buffer

	{
	if (m_iCursorPos < m_sInput.GetLength())
		{
		CString sCat;
		sCat = strSubString(m_sInput, 0, m_iCursorPos);
		sCat.Append(sInput);
		sCat.Append(strSubString(m_sInput, m_iCursorPos, -1));
		m_sInput = sCat;
		}
	else
		{
		m_sInput.Append(sInput);
		}
	m_iCursorPos += sInput.GetLength();
	m_bInvalid = true;
	}

void CCommandLineDisplay::InputBackspace (void)

//	InputBackspace
//
//	Delete characters from input buffer

	{
	if (m_iCursorPos > 0)
		{
		if (m_iCursorPos == m_sInput.GetLength())
			m_sInput = strSubString(m_sInput, 0, m_sInput.GetLength() - 1);
		else
			m_sInput = strCat(strSubString(m_sInput, 0, m_iCursorPos - 1), strSubString(m_sInput, m_iCursorPos, -1));
		m_iCursorPos--;
		m_bInvalid = true;
		}
	}

void CCommandLineDisplay::InputDelete (void)

//	InputDelete
//
//	Delete characters from input buffer

	{
	if (m_sInput.GetLength() > 0 && m_iCursorPos < m_sInput.GetLength())
		{
		if (m_iCursorPos == 0)
			m_sInput = strSubString(m_sInput, 1, -1);
		else
			m_sInput = strCat(strSubString(m_sInput, 0, m_iCursorPos), strSubString(m_sInput, m_iCursorPos+1, -1));
		m_bInvalid = true;
		}
	}

void CCommandLineDisplay::InputEnter (void)

//	InputEnter
//
//	Invoke input

	{
	AppendHistory(m_sInput);
	Output(m_sInput, INPUT_COLOR);
	ClearInput();
	ClearHint();
	}

void CCommandLineDisplay::InputHistoryUp (void)

//	InputHistoryUp
//
//	Recalls a line from the history buffer

	{
	if (m_iHistoryIndex < (GetHistoryCount()-1))
		{
		m_iHistoryIndex++;
		m_sInput = GetHistory(m_iHistoryIndex);
		m_iCursorPos = m_sInput.GetLength();
		m_bInvalid = true;
		}
	}

void CCommandLineDisplay::InputHistoryDown (void)

//	InputHistoryDown
//
//	Recalls a line from the history buffer

	{
	if (m_iHistoryIndex > 0)
		{
		m_iHistoryIndex--;
		m_sInput = GetHistory(m_iHistoryIndex);
		m_iCursorPos = m_sInput.GetLength();
		m_bInvalid = true;
		}
	else
		{
		m_iHistoryIndex = -1;
		ClearInput();
		}
	}

bool CCommandLineDisplay::OnChar (char chChar, DWORD dwKeyData)

//	OnChar
//
//	Deals with WM_CHAR if enabled. Returns FALSE if it does not want the 
//	character.

	{
	if (!m_bEnabled)
		return false;

	if (chChar >= ' ')
		{
		CString sKey = CString(&chChar, 1);
		Input(sKey);
		}

	//	Either way, we take the character

	return true;
	}

void CCommandLineDisplay::OnKeyDown (int iVirtKey, DWORD dwKeyState)

//	OnKeyDown
//
//	Handle WM_KEYDOWN

	{
	switch (iVirtKey)
		{
		case VK_BACK:
			InputBackspace();
			break;

		case VK_DELETE:
			InputDelete();
			break;

		case VK_RETURN:
			{
			CString sInput = GetInput();
			if (!sInput.IsBlank())
				{
				CCodeChain &CC = g_pUniverse->GetCC();

				InputEnter();

				CCodeChainCtx Ctx;
				ICCItemPtr pCode = Ctx.LinkCode(sInput);
				ICCItemPtr pResult = Ctx.RunCode(pCode);

				CString sOutput;
				if (pResult->IsIdentifier())
					sOutput = pResult->Print(&CC, PRFLAG_NO_QUOTES | PRFLAG_ENCODE_FOR_DISPLAY);
				else
					sOutput = CC.Unlink(pResult);

				Output(sOutput);
				}
			break;
			}

		case VK_TAB:
			{
			AutoCompleteSearch();
			break;
			}

		case VK_UP:
			InputHistoryUp();
			break;

		case VK_DOWN:
			InputHistoryDown();
			break;

		case VK_LEFT:
			if (m_iCursorPos > 0) m_iCursorPos--;
			m_bInvalid = true;
			break;

		case VK_RIGHT:
			if (m_iCursorPos < m_sInput.GetLength()) m_iCursorPos++;
			m_bInvalid = true;
			break;

		case VK_HOME:
			m_iCursorPos = 0;
			m_bInvalid = true;
			break;

		case VK_END:
			m_iCursorPos = m_sInput.GetLength();
			m_bInvalid = true;
			break;

		case VK_PRIOR:
			if (m_iScrollPos < 2*MAX_LINES) m_iScrollPos++;
			m_bInvalid = true;
			break;

		case VK_NEXT:
			if (m_iScrollPos > 0) m_iScrollPos--;
			m_bInvalid = true;
			break;
		}
	}

void CCommandLineDisplay::Output (const CString &sOutput, CG32bitPixel rgbColor)

//	Output
//
//	Output the given string

	{
	int i;

	if (m_pFont->GetAverageWidth() == 0)
		return;

	if (rgbColor.IsNull())
		rgbColor = TEXT_COLOR;

	if (sOutput.IsBlank())
		{
		AppendOutput(NULL_STR, rgbColor);
		return;
		}

	int cxCol = m_pFont->GetAverageWidth();
	int iCols = Max(1, (RectWidth(m_rcRect) - (LEFT_SPACING + RIGHT_SPACING)) / cxCol);

	//	Split into lines

	TArray<CString> Lines;
	m_pFont->BreakText(sOutput, 
			(RectWidth(m_rcRect) - (LEFT_SPACING + RIGHT_SPACING)),
			&Lines);

	//	Split the string into parts

	for (i = 0; i < Lines.GetCount(); i++)
		AppendOutput(Lines[i], rgbColor);
	}

void CCommandLineDisplay::Paint (CG32bitImage &Dest)

//	Paint
//
//	Paint display

	{
	if (!m_bEnabled)
		return;

	Update();

	//	Paint the cursor

	m_Buffer.Fill(m_rcCursor.left + 1, 
			m_rcCursor.top, 
			RectWidth(m_rcCursor) - 1, 
			RectHeight(m_rcCursor),
			(((m_iCounter % 30) < 20) ? INPUT_COLOR : BACK_COLOR));

	//	Redraw character under cursor
	if ((m_iCounter % 30) >= 20 && m_iCursorPos < m_sInput.GetLength())
		{
		CString sLine(m_sInput.GetASCIIZPointer() + m_iCursorPos, 1);
		m_Buffer.DrawText(m_rcCursor.left, m_rcCursor.top, *m_pFont, INPUT_COLOR, sLine);
		}

	//	Blt

	Dest.Blt(0,
			0,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			200,
			m_Buffer,
			m_rcRect.left,
			m_rcRect.top);

	m_iCounter++;
	}

void CCommandLineDisplay::Update (void)

//	Update
//
//	Update the buffer

	{
	if (!m_bInvalid 
			|| m_pFont == NULL 
			|| m_pFont->GetHeight() == 0 
			|| m_pFont->GetAverageWidth() == 0)
		return;

	//	If we don't yet have a buffer, create one

	if (m_Buffer.IsEmpty())
		{
		m_Buffer.Create(RectWidth(m_rcRect), RectHeight(m_rcRect));
		//m_Buffer.SetBlending(220);
		}

	//	Clear

	int cxWidth = RectWidth(m_rcRect);
	int cyHeight = RectHeight(m_rcRect);
	m_Buffer.Set(BACK_COLOR);

	CG32bitPixel rgbColor = CG32bitPixel(0, 160, 221);
	CG32bitPixel rgbFadeColor = CG32bitPixel(0, 80, 110);

	//	Outline the box

	m_Buffer.DrawLine(0, 0, cxWidth - 1, 0, 1, rgbColor);
	CGDraw::LineGradient(m_Buffer, 0, 0, 0, cyHeight - 1, 1, rgbColor, rgbFadeColor);
	CGDraw::LineGradient(m_Buffer, cxWidth - 1, 0, cxWidth - 1, cyHeight - 1, 1, rgbColor, rgbFadeColor);
	m_Buffer.DrawLine(0, cyHeight - 1, cxWidth, cyHeight - 1, 1, rgbFadeColor);

	//	Figure out how many lines and columns we can display

	int cyLine = m_pFont->GetHeight();
	int iLines = (cyHeight - (TOP_SPACING + BOTTOM_SPACING)) / cyLine;
	iLines = Max(1, iLines);

	int cxCol = m_pFont->GetAverageWidth();
	int iCols = (cxWidth - (LEFT_SPACING + RIGHT_SPACING)) / cxCol;
	iCols = Max(1, iCols);

	//	Figure out how many lines we need for input

	int iInputCols = m_sInput.GetLength() + 1;
	int iInputLines = (iInputCols / iCols) + ((iInputCols % iCols) ? 1 : 0);

	//	Figure out how many lines we need for the hint

	TArray<CString> HintLines;
	m_pFont->BreakText(m_sHint,
			(RectWidth(m_rcRect) - (LEFT_SPACING + RIGHT_SPACING)),
			&HintLines);

	int iHintLines = HintLines.GetCount();

	//	Figure out how many lines in the output

	int iOutputLines = GetOutputCount();

	//	Paint from the bottom up

	int iTotalLines = Min(iLines, iInputLines + iHintLines + iOutputLines);
	int x = LEFT_SPACING;
	int yMin = TOP_SPACING;
	int y = yMin + (iTotalLines - 1) * cyLine;

	//	Paint the input line

	int iRemainder = iInputCols % iCols;
	int iRemainderText = (iRemainder == 0 ? iCols : iRemainder) - 1;

	int iStart = m_sInput.GetLength() - iRemainderText;
	while (y >= yMin && iStart >= 0)
		{
		CString sLine(m_sInput.GetASCIIZPointer() + iStart, iRemainderText);
		m_Buffer.DrawText(x, y, *m_pFont, INPUT_COLOR, sLine);

		// Work out where the cursor should be
		if (m_iCursorPos >= iStart && (m_iCursorPos - iStart) < iCols)
			{
			m_rcCursor.left = x + (m_iCursorPos-iStart) * cxCol;
			m_rcCursor.top = y;
			m_rcCursor.right = m_rcCursor.left + cxCol;
			m_rcCursor.bottom = m_rcCursor.top + cyLine;
			}
		y -= cyLine;
		iStart -= iCols;
		iRemainderText = iCols;
		}

	//	Work out how much we can scroll the display
	
	m_iScrollPos = Min(m_iScrollPos, iInputLines+iHintLines+iOutputLines - iTotalLines);
	int iScroll = m_iScrollPos;

	//	Paint the hint line

	while (y >= yMin && iHintLines > 0)
		{
		if (iScroll <= 0)
			{
			m_Buffer.DrawText(x, y, *m_pFont, HINT_COLOR, HintLines[iHintLines - 1]);
			y -= cyLine;
			}
		else
			iScroll--;

		iHintLines--;
		}

	//	Paint each line of output

	int iLine = 0;
	while (y >= yMin && iLine < GetOutputCount())
		{
		if (iScroll <= 0)
			{
			m_Buffer.DrawText(x, y, *m_pFont, GetOutputColor(iLine), GetOutput(iLine));
			y -= cyLine;
			}
		else
			iScroll--;

		iLine++;
		}

	m_bInvalid = false;
	}
