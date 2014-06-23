//	CTextBlock.cpp
//
//	CTextBlock class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.
//
//	EXAMPLES:
//
//	{\rtf This is a line of text.}
//	{\rtf This is {\b bold} text.}
//	{\rtf This is {\i italic} text.}
//	{\rtf This is {\b\i bold and italic} text.}
//	{\rtf Use a back-slash (\\) to escape.}
//	{\rtf This is a new\nline.}
//	{\rtf This is {\f:Tahoma; a typeface.}}
//	{\rtf {\f:Tahoma; Nesting {\b works}.}}
//	{\rtf {\c:0xffffff; color}}
//
//	NOTE: forward slash and backslash may both be used (so we can embbed in C code).
//
//	Reserved characters (must be escaped):
//
//	\		backslash
//	/		slash
//	{		open brace
//	}		close brace

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

#include "TextFormat.h"

#define CODE_BOLD								CONSTLIT("b")
#define CODE_COLOR								CONSTLIT("c")
#define CODE_TYPEFACE							CONSTLIT("f")
#define CODE_ITALIC								CONSTLIT("i")
#define CODE_RTF								CONSTLIT("rtf")

#define ERR_BRACE_EXPECTED						CONSTLIT("'{' expected.")
#define ERR_SEMI_COLON_EXPECTED					CONSTLIT("';' expected.")
#define ERR_INVALID_CODE_TERMINATOR				CONSTLIT("Invalid code terminator.")
#define ERR_NULL_CODE							CONSTLIT("Null code.")
#define ERR_UNEXPECTED_EOS						CONSTLIT("Unexpected end of stream.")
#define ERR_UNKNOWN_CODE						CONSTLIT("Unknown code: %s.")

class CRTFParser
	{
	public:
		CRTFParser (const CString &sInput, const IFontTable &FontTable, CTextBlock *pOutput);

		bool ParseBlock (const STextFormatDesc &InitFormat, CString *retsError);

	private:
		void AddSpan (const CString &sText, const STextFormatDesc &Desc, bool bEoP = false);
		bool ParseCode (CString *retsCode, CString *retsParam, CString *retsError);

		char *m_pInput;
		const IFontTable &m_FontTable;
		CTextBlock *m_pOutput;
	};

void CTextBlock::AddTextSpan (const CString &sText, const STextFormat &Format, bool bEoP)

//	AddTextSpan
//
//	Adds a text span

	{
	STextSpan *pSpan = m_Text.Insert();
	pSpan->sText = sText;
	pSpan->Format = Format;
	pSpan->bEoP = bEoP;
	}

CString CTextBlock::Escape (const CString &sText)

//	Escape
//
//	Escapes all reserved characters in sText

	{
	char *pPos = sText.GetASCIIZPointer();
	char *pEndPos = pPos + sText.GetLength();

	CString sResult;
	char *pDest = NULL;

	char *pStart = pPos;
	while (pPos < pEndPos)
		{
		switch (*pPos)
			{
			case '\\':
			case '/':
			case '{':
			case '}':
				{
				//	If necessary, allocate a resulting buffer (note that we can
				//	never be larger than twice the length of the original string).

				if (pDest == NULL)
					pDest = sResult.GetWritePointer(sText.GetLength() * 2);

				//	Write out the string up to now

				char *pSrc = pStart;
				while (pSrc < pPos)
					*pDest++ = *pSrc++;

				//	Write out an escaped version of the character

				*pDest++ = '\\';
				*pDest++ = *pPos;

				pStart = pPos + 1;
				break;
				}
			}

		pPos++;
		}

	//	If necessary write the remaining string

	if (pDest)
		{
		char *pSrc = pStart;
		while (pSrc < pEndPos)
			*pDest++ = *pSrc++;

		//	Truncate

		sResult.Truncate((int)(pDest - sResult.GetPointer()));

		//	Done

		return sResult;
		}

	//	If we didn't need to escape anything then we just return the original
	//	string.

	else
		return sText;
	}

void CTextBlock::Format (const SBlockFormatDesc &BlockFormat)

//	Format
//
//	Format the text for the given width

	{
	int i;

	m_Formatted.DeleteAll();

	//	Keep looping until we've formatted all spans.

	STextSpan *pSpan = (m_Text.GetCount() > 0 ? &m_Text[0] : NULL);
	STextSpan *pSpanEnd = (pSpan ? pSpan + m_Text.GetCount() : NULL);

	TArray<STextSpan> Line;
	STextSpan LeftOver;

	int y = 0;
	int iSpanPos = 0;
	int cxLeft = BlockFormat.cxWidth;
	bool bLineEnd = false;
	bool bLineStart = true;

	while (pSpan)
		{
		//	Build up a string with as much of the span as we can fit.

		CString sOutput;
		char *pOutStart = sOutput.GetWritePointer(pSpan->sText.GetLength());
		char *pOut = pOutStart;

		char *pPos = pSpan->sText.GetASCIIZPointer() + iSpanPos;
		char *pPosEnd = pSpan->sText.GetASCIIZPointer() + pSpan->sText.GetLength();

		while (pPos < pPosEnd && !bLineEnd)
			{
			//	Find the next word

			char *pStart = pPos;
			while (pPos < pPosEnd && *pPos != ' ' && *pPos != '-')
				pPos++;

			//	Measure the length of the word

			int cxWord = pSpan->Format.pFont->MeasureText(CString(pStart, (int)(pPos - pStart), TRUE));

			//	If the word fits, add it to the output

			if (cxWord <= cxLeft || bLineStart)
				{
				//	Add the separator

				if (pPos < pPosEnd)
					{
					cxWord += pSpan->Format.pFont->MeasureText(CString(pPos, 1, TRUE));
					pPos++;
					}

				char *pSrc = pStart;
				while (pSrc < pPos)
					*pOut++ = *pSrc++;

				cxLeft -= cxWord;
				bLineStart = false;

				//	Are we at the end of the span?

				if (pPos == pPosEnd)
					{
					sOutput.Truncate((int)(pOut - pOutStart));
					bLineEnd = false;
					}
				}

			//	Otherwise, we're done with the line

			else
				{
				bLineEnd = true;
				sOutput.Truncate((int)(pOut - pOutStart));
				iSpanPos += sOutput.GetLength();
				}
			}

		//	At this point:
		//
		//	sOutput is the part of the span that we should add to the line.
		//	bLineEnd is TRUE if we should end the line in the middle of the span.
		//		(If FALSE then sOutput is the entire span).
		//	iSpanPos is the offset into the span where we should continue adding
		//		(Only valid if bLineEnd is TRUE).

		if (!sOutput.IsBlank())
			{
			STextSpan *pNewSpan = Line.Insert();
			pNewSpan->bEoP = (!bLineEnd && pSpan->bEoP);
			pNewSpan->Format = pSpan->Format;
			pNewSpan->sText = sOutput;
			}

		//	If we've hit the end of the line, then we output everything we've
		//	got in the line to the formatted buffer.

		if (bLineEnd || pSpan->bEoP || (pSpan + 1) == pSpanEnd)
			{
			//	Compute metrics for each span and the line as a whole.

			int cyLineAscent = 0;
			int cxLine = 0;
			int cyLine = 0;

			if (Line.GetCount() == 0)
				cyLine = pSpan->Format.pFont->GetHeight();
			else
				{
				for (i = 0; i < Line.GetCount(); i++)
					{
					//	Compute the max ascent of the line.

					int cyAscent = Line[i].Format.pFont->GetAscent();
					if (cyAscent > cyLineAscent)
						cyLineAscent = cyAscent;

					//	Compute the width of each span, the total width of the line
					//	and the max height of the line.

					int cyHeight;
					Line[i].cx = Line[i].Format.pFont->MeasureText(Line[i].sText, &cyHeight);
					cxLine += Line[i].cx;
					if (cyHeight > cyLine)
						cyLine = cyHeight;
					}
				}

			//	Compute to horz position of the line (based on block alignment)

			int x;
			if (BlockFormat.iHorzAlign == alignRight && BlockFormat.cxWidth != -1)
				x = BlockFormat.cxWidth - cxLine;
			else if (BlockFormat.iHorzAlign == alignCenter && BlockFormat.cxWidth != -1)
				x = (BlockFormat.cxWidth - cxLine) / 2;
			else
				x = 0;

			//	Format all the spans for this line

			for (i = 0; i < Line.GetCount(); i++)
				{
				SFormattedTextSpan *pFormatted = m_Formatted.Insert();

				pFormatted->sText = Line[i].sText;
				pFormatted->Format = Line[i].Format;

				pFormatted->x = x;
				pFormatted->y = y + (cyLineAscent - Line[i].Format.pFont->GetAscent());

				pFormatted->cx = Line[i].cx;
				pFormatted->cy = Line[i].Format.pFont->GetHeight();

				//	Advance the line width

				x += Line[i].cx;
				}

			//	Advance the line

			y += cyLine + BlockFormat.iExtraLineSpacing;
			Line.DeleteAll();

			cxLeft = BlockFormat.cxWidth;
			bLineStart = true;
			}

		//	Next span

		if (!bLineEnd)
			{
			pSpan++;
			if (pSpan >= pSpanEnd)
				pSpan = NULL;

			iSpanPos = 0;
			}

		bLineEnd = false;
		}
	}

void CTextBlock::GetBounds (RECT *retrcRect)

//	GetBounds
//
//	Returns the bounding rect

	{
	int i;

	if (m_Formatted.GetCount() == 0)
		{
		retrcRect->left = 0;
		retrcRect->right = 0;
		retrcRect->top = 0;
		retrcRect->bottom = 0;
		return;
		}

	retrcRect->left = 100000;
	retrcRect->right = -100000;
	retrcRect->top = 100000;
	retrcRect->bottom = -100000;

	for (i = 0; i < m_Formatted.GetCount(); i++)
		{
		SFormattedTextSpan *pSpan = &m_Formatted[i];

		if (pSpan->x < retrcRect->left)
			retrcRect->left = pSpan->x;
		if (pSpan->x + pSpan->cx > retrcRect->right)
			retrcRect->right = pSpan->x + pSpan->cx;

		if (pSpan->y < retrcRect->top)
			retrcRect->top = pSpan->y;
		if (pSpan->y + pSpan->cy > retrcRect->bottom)
			retrcRect->bottom = pSpan->y + pSpan->cy;
		}
	}

bool CTextBlock::InitFromRTF (const CString &RTF, const IFontTable &FontTable, const SBlockFormatDesc &BlockFormat, CString *retsError)

//	InitFromRTF
//
//	Initializes the text block from an RTF stream.

	{
	m_Text.DeleteAll();

	CRTFParser Parser(RTF, FontTable, this);

	STextFormatDesc Desc;
	Desc.pFont = BlockFormat.DefaultFormat.pFont;
	Desc.wColor = BlockFormat.DefaultFormat.wColor;
	Desc.dwOpacity = BlockFormat.DefaultFormat.dwOpacity;

	CString sError;
	bool bSuccess = Parser.ParseBlock(Desc, &sError);

	//	If we failed, then use the error as the text

	if (!bSuccess)
		{
		m_Text.DeleteAll();

		STextSpan *pText = m_Text.Insert();
		pText->sText = sError;
		pText->Format.dwOpacity = 255;
		pText->Format.wColor = CG16bitImage::RGBValue(255, 255, 0);
		pText->Format.pFont = FontTable.GetFont(Desc);
		pText->bEoP = false;
		}

	Format(BlockFormat);

	//	Done

	if (retsError)
		*retsError = sError;

	return bSuccess;
	}

CString CTextBlock::LoadAsRichText (const CString &sText)

//	LoadAsRichText
//
//	Takes either a RTF string (with "{\rtf...") or a plain string and returns
//	a valid RTF string.

	{
	//	If this is already an RTF string, just return it.

	if (strStartsWith(sText, CONSTLIT("{\\rtf")) || strStartsWith(sText, CONSTLIT("{/rtf")))
		return sText;

	//	Otherwise, escape the string

	return strPatternSubst(CONSTLIT("{\\rtf %s}"), Escape(sText));
	}

//	CRTFParser -----------------------------------------------------------------

CRTFParser::CRTFParser (const CString &sInput, const IFontTable &FontTable, CTextBlock *pOutput) :
		m_pInput(sInput.GetASCIIZPointer()),
		m_FontTable(FontTable),
		m_pOutput(pOutput)

//	CRTFParser constructor

	{
	}

void CRTFParser::AddSpan (const CString &sText, const STextFormatDesc &Desc, bool bEoP)

//	AddSpan
//
//	Adds a span of text

	{
	STextFormat Format;

	Format.pFont = (Desc.pFont ? Desc.pFont : m_FontTable.GetFont(Desc));
	Format.wColor = Desc.wColor;
	Format.dwOpacity = Desc.dwOpacity;

	m_pOutput->AddTextSpan(sText, Format, bEoP);
	}

bool CRTFParser::ParseBlock (const STextFormatDesc &InitFormat, CString *retsError)

//	ParseBlock
//
//	Parses a block and leaves the input position at the first character after
//	the end of the block.

	{
	//	Better be the beginning of a block

	if (*m_pInput++ != '{')
		{
		*retsError = ERR_BRACE_EXPECTED;
		return false;
		}

	//	Keep track of the current format and text

	STextFormatDesc Format = InitFormat;

	//	Keep looping until we hit the end of the block

	bool bBlockStart = true;
	while (*m_pInput != '}')
		{
		//	End of stream

		if (*m_pInput == '\0')
			{
			*retsError = ERR_UNEXPECTED_EOS;
			return false;
			}

		//	If an escape character then parse an op code

		else if (*m_pInput == '\\' || *m_pInput == '/')
			{
			m_pInput++;

			//	If this is the beginning of the block then parse some codes

			if (bBlockStart)
				{
				CString sCode;
				CString sParam;
				if (!ParseCode(&sCode, &sParam, retsError))
					return false;

				//	Interpret code

				if (strEquals(sCode, CODE_BOLD))
					Format.bBold = true;
				else if (strEquals(sCode, CODE_COLOR))
					{
					DWORD dwRGB = (DWORD)strToInt(sParam, 0);
					Format.wColor = CG16bitImage::RGBValue(GetRValue(dwRGB), GetGValue(dwRGB), GetBValue(dwRGB));
					}
				else if (strEquals(sCode, CODE_TYPEFACE))
					{
					Format.sTypeface = sParam;
					Format.pFont = NULL;
					}
				else if (strEquals(sCode, CODE_ITALIC))
					Format.bItalic = true;
				else if (strEquals(sCode, CODE_RTF))
					;
				else
					{
					*retsError = strPatternSubst(ERR_UNKNOWN_CODE, sCode);
					return false;
					}
				}

			//	Parse some escape characters

			else
				{
				switch (*m_pInput)
					{
					case '{':
					case '}':
					case '\\':
					case '/':
						AddSpan(CString(m_pInput, 1), Format);
						break;

					case 'n':
						AddSpan(NULL_STR, Format, true);
						break;

					default:
						{
						*retsError = strPatternSubst(ERR_UNKNOWN_CODE, CString(m_pInput, 1));
						return false;
						}
					}

				m_pInput++;
				}
			}

		//	Special characters

		else if (*m_pInput == '\n')
			{
			AddSpan(NULL_STR, Format, true);
			m_pInput++;
			}

		//	If we have an open brace, then we recurse

		else if (*m_pInput == '{')
			{
			if (!ParseBlock(Format, retsError))
				return false;

			bBlockStart = false;
			}

		//	Otherwise, this is some text

		else
			{
			char *pStart = m_pInput;

			//	Find the end of the span

			while (*m_pInput != '\\' && *m_pInput != '/' && *m_pInput != '{' && *m_pInput != '}' && *m_pInput != '\n' && *m_pInput != '\0')
				m_pInput++;

			//	Add the text as a span

			if (pStart != m_pInput)
				AddSpan(CString(pStart, (int)(m_pInput - pStart)), Format);

			bBlockStart = false;
			}
		}

	m_pInput++;

	//	Done

	return true;
	}

bool CRTFParser::ParseCode (CString *retsCode, CString *retsParam, CString *retsError)

//	ParseCode
//
//	Parses an escape code

	{
	//	Parse the code

	char *pStart = m_pInput;
	while (*m_pInput != ' ' && *m_pInput != '\\' && *m_pInput != '/' && *m_pInput != '}' && *m_pInput != '}' && *m_pInput != ';' && *m_pInput != ':' && *m_pInput != '\0')
		m_pInput++;

	*retsCode = CString(pStart, (int)(m_pInput - pStart));
	if (retsCode->GetLength() == 0)
		{
		*retsError = ERR_NULL_CODE;
		return false;
		}

	//	Do we have a parameter?

	if (*m_pInput == ':')
		{
		m_pInput++;

		pStart = m_pInput;
		while (*m_pInput != ';' && *m_pInput != '}' && *m_pInput != '}' && *m_pInput != '\\' && *m_pInput != '/' && *m_pInput != ':' && *m_pInput != '\0')
			m_pInput++;

		if (*m_pInput++ != ';')
			{
			*retsError = ERR_SEMI_COLON_EXPECTED;
			return false;
			}

		*retsParam = CString(pStart, (int)((m_pInput - 1) - pStart));
		}
	else
		*retsParam = NULL_STR;

	//	Parse code terminator

	if (*m_pInput == ' ')
		m_pInput++;
	else if (*m_pInput == '\\' || *m_pInput == '/')
		;
	else
		{
		*retsError = ERR_INVALID_CODE_TERMINATOR;
		return false;
		}

	//	Done

	return true;
	}
