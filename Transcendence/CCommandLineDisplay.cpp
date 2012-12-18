//	CCommandLineDisplay.cpp
//
//	CCommandLineDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define BACK_COLOR					(CG16bitImage::RGBValue(20, 20, 20))
#define TEXT_COLOR					(CG16bitImage::RGBValue(200, 200, 200))
#define INPUT_COLOR					(CG16bitImage::RGBValue(255, 255, 200))

const int LEFT_SPACING =			2;
const int RIGHT_SPACING =			2;
const int TOP_SPACING =				2;
const int BOTTOM_SPACING =			2;

const int CURSOR_WIDTH =			10;

CCommandLineDisplay::CCommandLineDisplay (void) : m_pTrans(NULL),
		m_bInvalid(true),
		m_iCounter(0)

//	CCommandLineDisplay constructor

	{
	}

CCommandLineDisplay::~CCommandLineDisplay (void)

//	CCommandLineDisplay destructor

	{
	CleanUp();
	}

void CCommandLineDisplay::AppendOutput (const CString &sLine, WORD wColor)

//	AppendOutput
//
//	Append a line of output

	{
	m_iOutputStart = (m_iOutputStart + (MAX_LINES + 1) - 1) % (MAX_LINES + 1);
	if (m_iOutputStart == m_iOutputEnd)
		m_iOutputEnd = (m_iOutputEnd + (MAX_LINES + 1) - 1) % (MAX_LINES + 1);

	m_Output[m_iOutputStart] = sLine;
	m_OutputColor[m_iOutputStart] = wColor;

	m_bInvalid = true;
	}

void CCommandLineDisplay::CleanUp (void)

//	CleanUp
//
//	Free all resource

	{
	m_Buffer.Destroy();
	m_bInvalid = true;
	}

const CString &CCommandLineDisplay::GetOutput (int iLine)

//	GetOutput
//
//	Returns the output line

	{
	return m_Output[(m_iOutputStart + iLine) % (MAX_LINES + 1)];
	}

WORD CCommandLineDisplay::GetOutputColor (int iLine)

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

ALERROR CCommandLineDisplay::Init (CTranscendenceWnd *pTrans, const RECT &rcRect)

//	Init
//
//	Initialize

	{
	m_pTrans = pTrans;
	m_rcRect = rcRect;
	m_iOutputStart = 0;
	m_iOutputEnd = 0;
	m_sInput = NULL_STR;
	m_bInvalid = true;

	return NOERROR;
	}

void CCommandLineDisplay::Input (const CString &sInput)

//	Input
//
//	Add characters to input buffer

	{
	m_sInput.Append(sInput);
	m_bInvalid = true;
	}

void CCommandLineDisplay::InputBackspace (void)

//	InputBackspace
//
//	Delete characters from input buffer

	{
	if (m_sInput.GetLength() > 0)
		{
		m_sInput = strSubString(m_sInput, 0, m_sInput.GetLength() - 1);
		m_bInvalid = true;
		}
	}

void CCommandLineDisplay::InputEnter (void)

//	InputEnter
//
//	Invoke input

	{
	m_sLastLine = m_sInput;
	Output(m_sInput, INPUT_COLOR);
	ClearInput();
	}

void CCommandLineDisplay::InputLastLine (void)

//	InputLastLine
//
//	Recalls the last line

	{
	if (!m_sLastLine.IsBlank())
		{
		m_sInput = m_sLastLine;
		m_bInvalid = true;
		}
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

		case VK_RETURN:
			{
			CString sInput = GetInput();
			if (!sInput.IsBlank())
				{
				CCodeChain &CC = g_pUniverse->GetCC();

				InputEnter();

				CCodeChainCtx Ctx;
				ICCItem *pCode = Ctx.Link(sInput, 0, NULL);
				ICCItem *pResult = Ctx.Run(pCode);

				CString sOutput;
				if (pResult->IsIdentifier())
					sOutput = pResult->Print(&CC, PRFLAG_NO_QUOTES | PRFLAG_ENCODE_FOR_DISPLAY);
				else
					sOutput = CC.Unlink(pResult);

				Ctx.Discard(pResult);
				Ctx.Discard(pCode);

				Output(sOutput);
				}
			break;
			}

		case VK_UP:
			InputLastLine();
			break;
		}
	}

void CCommandLineDisplay::Output (const CString &sOutput, WORD wColor)

//	Output
//
//	Output the given string

	{
	int i;

	if (m_pFonts == NULL || m_pFonts->Console.GetAverageWidth() == 0)
		return;

	if (wColor == 0)
		wColor = TEXT_COLOR;

	if (sOutput.IsBlank())
		{
		AppendOutput(NULL_STR, wColor);
		return;
		}

	int cxCol = m_pFonts->Console.GetAverageWidth();
	int iCols = Max(1, (RectWidth(m_rcRect) - (LEFT_SPACING + RIGHT_SPACING)) / cxCol);

	//	Split into lines

	TArray<CString> Lines;
	m_pFonts->Console.BreakText(sOutput, 
			(RectWidth(m_rcRect) - (LEFT_SPACING + RIGHT_SPACING)),
			&Lines);

	//	Split the string into parts

	for (i = 0; i < Lines.GetCount(); i++)
		AppendOutput(Lines[i], wColor);
	}

void CCommandLineDisplay::Paint (CG16bitImage &Dest)

//	Paint
//
//	Paint display

	{
	Update();

	//	Paint the cursor

	m_Buffer.Fill(m_rcCursor.left + 1, 
			m_rcCursor.top, 
			RectWidth(m_rcCursor) - 1, 
			RectHeight(m_rcCursor),
			(((m_iCounter % 30) < 20) ? INPUT_COLOR : BACK_COLOR));

	//	Blt

	Dest.ColorTransBlt(0,
			0,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			255,
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
			|| m_pFonts == NULL 
			|| m_pFonts->Console.GetHeight() == 0 
			|| m_pFonts->Console.GetAverageWidth() == 0)
		return;

	//	If we don't yet have a buffer, create one

	if (m_Buffer.IsEmpty())
		{
		m_Buffer.CreateBlank(RectWidth(m_rcRect), RectHeight(m_rcRect), false);
		m_Buffer.SetBlending(220);
		}

	//	Clear

	int cxWidth = RectWidth(m_rcRect);
	int cyHeight = RectHeight(m_rcRect);
	m_Buffer.Fill(0, 0, cxWidth, cyHeight, BACK_COLOR);

	WORD wColor = CG16bitImage::RGBValue(0, 160, 221);
	WORD wFadeColor = CG16bitImage::RGBValue(0, 80, 110);

	//	Outline the box

	m_Buffer.DrawLine(0, 0, cxWidth - 1, 0, 1, wColor);
	m_Buffer.DrawBiColorLine(0, 0, 0, cyHeight - 1, 1, wColor, wFadeColor);
	m_Buffer.DrawBiColorLine(cxWidth - 1, 0, cxWidth - 1, cyHeight - 1, 1, wColor, wFadeColor);
	m_Buffer.DrawLine(0, cyHeight - 1, cxWidth, cyHeight - 1, 1, wFadeColor);

	//	Figure out how many lines and columns we can display

	int cyLine = m_pFonts->Console.GetHeight();
	int iLines = (cyHeight - (TOP_SPACING + BOTTOM_SPACING)) / cyLine;
	iLines = Max(1, iLines);

	int cxCol = m_pFonts->Console.GetAverageWidth();
	int iCols = (cxWidth - (LEFT_SPACING + RIGHT_SPACING)) / cxCol;
	iCols = Max(1, iCols);

	//	Figure out how many lines we need for input

	int iInputCols = m_sInput.GetLength() + 1;
	int iInputLines = (iInputCols / iCols) + ((iInputCols % iCols) ? 1 : 0);

	//	Figure out how many lines in the output

	int iOutputLines = GetOutputCount();

	//	Paint from the bottom up

	int iTotalLines = Min(iLines, iInputLines + iOutputLines);
	int x = LEFT_SPACING;
	int yMin = TOP_SPACING;
	int y = yMin + (iTotalLines - 1) * cyLine;

	//	Paint the input line

	int iRemainder = iInputCols % iCols;
	int iRemainderText = (iRemainder == 0 ? iCols : iRemainder) - 1;

	m_rcCursor.left = x + iRemainderText * cxCol;
	m_rcCursor.top = y;
	m_rcCursor.right = m_rcCursor.left + cxCol;
	m_rcCursor.bottom = m_rcCursor.top + cyLine;

	int iStart = m_sInput.GetLength() - iRemainderText;
	while (y >= yMin && iStart >= 0)
		{
		CString sLine(m_sInput.GetASCIIZPointer() + iStart, iRemainderText);
		m_Buffer.DrawText(x, y, m_pFonts->Console, INPUT_COLOR, sLine);

		y -= cyLine;
		iStart -= iCols;
		iRemainderText = iCols;
		}

	//	Paint each line of output

	int iLine = 0;
	while (y >= yMin && iLine < GetOutputCount())
		{
		m_Buffer.DrawText(x, y, m_pFonts->Console, GetOutputColor(iLine), GetOutput(iLine));

		y -= cyLine;
		iLine++;
		}

	m_bInvalid = false;
	}
