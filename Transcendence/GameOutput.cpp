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

void CTranscendenceWnd::GameOutput (const CString &sLine)

//	GameOutput
//
//	Output game report strings

	{
	}

const CG16bitFont *CTranscendenceWnd::GetFont (const CString &sFont)

//	GetFont
//
//	Returns the given font (or NULL if not found)

	{
	bool bFound;

	const CG16bitFont &TheFont = g_pHI->GetVisuals().GetFont(sFont, &bFound);
	if (!bFound)
		return NULL;

	return &TheFont;
	}
