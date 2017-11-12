//	GameOutput.cpp
//
//	Handles CTranscendenceWnd methods related to outputing game stats

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

void CTranscendenceWnd::ConsoleOutput (const CString &sLine)

//	ConsoleOutput
//
//	Output to the console

	{
	DebugConsoleOutput(sLine);
	}

void CTranscendenceWnd::DebugOutput (const CString &sLine)

//	DebugOutput
//
//	Output some debug strings

	{
#ifdef DEBUG
	m_DebugLines[m_iDebugLinesEnd] = sLine;
	m_iDebugLinesEnd = (m_iDebugLinesEnd + 1) % DEBUG_LINES_COUNT;

	if (m_iDebugLinesEnd == m_iDebugLinesStart)
		m_iDebugLinesStart = (m_iDebugLinesStart + 1) % DEBUG_LINES_COUNT;
#endif
	}

bool CTranscendenceWnd::FindFont (const CString &sFont, const CG16bitFont **retpFont) const

//	FindFont
//
//	Returns the given font (or FALSE if not found)

	{
	bool bFound;
	const CG16bitFont &TheFont = g_pHI->GetVisuals().GetFont(sFont, &bFound);
	if (!bFound)
		return false;

	if (retpFont)
		*retpFont = &TheFont;

	return true;
	}

void CTranscendenceWnd::GameOutput (const CString &sLine)

//	GameOutput
//
//	Output game report strings

	{
	}

CG32bitPixel CTranscendenceWnd::GetColor (const CString &sColor) const

//	GetColor
//
//	Returns the given named color. Returns a default color if not found.

	{
	return g_pHI->GetVisuals().GetColor(sColor);
	}

const CG16bitFont &CTranscendenceWnd::GetFont (const CString &sFont) const

//	GetFont
//
//	Returns the given named font (returns a default font if not found).

	{
	return g_pHI->GetVisuals().GetFont(sFont);
	}
