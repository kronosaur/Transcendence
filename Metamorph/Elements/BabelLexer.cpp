//	BabelLexer.cpp
//
//	CLexer object
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"
#include "Babel.h"
#include "BabelParser.h"

inline bool IsIdentifierChar (char chChar)
	{
	return (chChar >= '0' && chChar <= '9')
			|| (chChar >= 'A' && chChar <= 'Z')
			|| (chChar >= 'a' && chChar <= 'z')
			|| (chChar == '_');
	}

inline bool IsDigit (char chChar)
	{
	return (chChar >= '0' && chChar <= '9');
	}

CLexer::CLexer (char *pPos, char *pEndPos) :
		m_pPos(pPos),
		m_pEndPos(pEndPos),
		m_iLineNumber(1)

//	CLexer Constructor

	{
	}

BabelTokens CLexer::NextToken (CString *retsToken, Options iMode)

//	NextToken
//
//	Returns the next token in the stream

	{
	enum States
		{
		stStart,
		stStartCode,
		stDone,
		stDoneKeepNextChar,

		stCheckForDoubleColon,
		stCheckForComment,
		stIdentifier,
		stLineComment,
		stBlockComment,
		stCheckForEndBlockComment,
		stQuotedString,
		stEscapeQuote,
		stLineBlock,
		stCheckForEndLineBlock,
		stParagraph,
		stCheckForEndParagraph,
		stCheckForNegativeNumber,
		stInteger,
		stHexInteger,

		stCode,
		stCodeCheckForComment,
		stCodeLineComment,
		stCodeBlockComment,
		stCodeBlockCommentEnd,
		};

	//	Start in a different state depending on the mode

	States iState;
	if (iMode == modeCode)
		iState = stStartCode;
	else
		iState = stStart;

	//	Initialize

	char *pStart;
	m_sToken = NULL;
	m_iToken = tkEOS;

	States iNextState;					//	Used for stEscapeQuote
	bool bBlankLine;					//	Used for stParagraph
	int iNesting;						//	Used for stStartCode

	while (iState != stDone && iState != stDoneKeepNextChar)
		{
		if (m_pPos == m_pEndPos)
			break;

		switch (iState)
			{
			case stStart:
				{
				switch (*m_pPos)
					{
					//	Swallow whitespace

					case ' ':
					case '\t':
					case '\r':
					case '\n':
						break;

					//	Comment

					case '/':
						iState = stCheckForComment;
						break;

					//	Quoted string

					case '\"':
						pStart = m_pPos + 1;
						iState = stQuotedString;
						break;

					//	Line block

					case '|':
						pStart = m_pPos + 1;
						iState = stLineBlock;
						break;

					//	Paragraph

					case '¶':
						pStart = m_pPos + 1;
						bBlankLine = true;
						iState = stParagraph;
						break;

					//	Symbols

					case '*':
						m_sToken = CONSTLIT("*");
						m_iToken = tkStar;
						iState = stDone;
						break;

					case ':':
						iState = stCheckForDoubleColon;
						break;

					case ';':
						m_sToken = CONSTLIT(";");
						m_iToken = tkSemiColon;
						iState = stDone;
						break;

					case '(':
						m_sToken = CONSTLIT("(");
						m_iToken = tkLeftParen;
						iState = stDone;
						break;

					case ')':
						m_sToken = CONSTLIT(")");
						m_iToken = tkRightParen;
						iState = stDone;
						break;

					case '[':
						m_sToken = CONSTLIT("[");
						m_iToken = tkLeftBracket;
						iState = stDone;
						break;

					case ']':
						m_sToken = CONSTLIT("]");
						m_iToken = tkRightBracket;
						iState = stDone;
						break;

					case '{':
						m_sToken = CONSTLIT("{");
						m_iToken = tkLeftBrace;
						iState = stDone;
						break;

					case '}':
						m_sToken = CONSTLIT("}");
						m_iToken = tkRightBrace;
						iState = stDone;
						break;

					case '=':
						m_sToken = CONSTLIT("=");
						m_iToken = tkEquals;
						iState = stDone;
						break;

					case '>':
						m_sToken = CONSTLIT(">");
						m_iToken = tkGreaterThan;
						iState = stDone;
						break;

					case '<':
						m_sToken = CONSTLIT("<");
						m_iToken = tkLessThan;
						iState = stDone;
						break;

					case ',':
						m_sToken = CONSTLIT(",");
						m_iToken = tkComma;
						iState = stDone;
						break;

					case '!':
						m_sToken = CONSTLIT("!");
						m_iToken = tkBang;
						iState = stDone;
						break;

					case '-':
						pStart = m_pPos;
						iState = stCheckForNegativeNumber;
						break;

					default:
						if (IsDigit(*m_pPos))
							{
							pStart = m_pPos;
							iState = stInteger;
							}
						else if (IsIdentifierChar(*m_pPos))
							{
							pStart = m_pPos;
							iState = stIdentifier;
							}
						else
							{
							m_sToken = CString(m_pPos, 1);
							m_iToken = tkOtherSymbol;
							iState = stDone;
							break;
							}
						break;
					}
				break;
				}

			case stCheckForNegativeNumber:
				{
				if (IsDigit(*m_pPos))
					iState = stInteger;
				else
					{
					m_sToken = CONSTLIT("-");
					m_iToken = tkOtherSymbol;
					iState = stDoneKeepNextChar;
					}
				}

			case stCheckForDoubleColon:
				{
				if (*m_pPos == ':')
					{
					m_sToken = CONSTLIT("::");
					m_iToken = tkDoubleColon;
					iState = stDone;
					}
				else
					{
					m_sToken = CONSTLIT(":");
					m_iToken = tkColon;
					iState = stDoneKeepNextChar;
					}
				break;
				}

			case stCheckForComment:
				{
				switch (*m_pPos)
					{
					case '/':
						iState = stLineComment;
						break;

					case '*':
						iState = stBlockComment;
						break;

					default:
						{
						m_sToken = CONSTLIT("/");
						m_iToken = tkSlash;
						iState = stDoneKeepNextChar;
						}
					}
				break;
				}

			case stInteger:
				{
				if (IsDigit(*m_pPos))
					;
				else if (*m_pPos == 'x' || *m_pPos == 'X')
					iState = stHexInteger;
				else
					{
					m_sToken.Append(CString(pStart, m_pPos - pStart));
					m_iToken = tkInteger;
					iState = stDoneKeepNextChar;
					}
				break;
				}

			case stHexInteger:
				{
				if (IsDigit(*m_pPos)
						|| (*m_pPos >= 'A' && *m_pPos <= 'F')
						|| (*m_pPos >= 'a' && *m_pPos <= 'f'))
					;
				else
					{
					m_sToken.Append(CString(pStart, m_pPos - pStart));
					m_iToken = tkInteger;
					iState = stDoneKeepNextChar;
					}
				break;
				}

			case stLineComment:
				{
				switch (*m_pPos)
					{
					case '\n':
						iState = stStart;
						break;
					}
				break;
				}

			case stBlockComment:
				{
				switch (*m_pPos)
					{
					case '*':
						iState = stCheckForEndBlockComment;
						break;
					}
				break;
				}

			case stCheckForEndBlockComment:
				{
				switch (*m_pPos)
					{
					case '/':
						iState = stStart;
						break;

					case '*':
						break;

					default:
						iState = stBlockComment;
						break;
					}
				break;
				}

			case stIdentifier:
				{
				//	If we're at the end, return it

				if (!IsIdentifierChar(*m_pPos))
					{
					m_sToken = CString(pStart, m_pPos - pStart);
					m_iToken = tkIdentifier;
					iState = stDoneKeepNextChar;
					}
				break;
				}

			case stQuotedString:
				{
				switch (*m_pPos)
					{
					case '\"':
						{
						m_sToken.Append(CString(pStart, m_pPos - pStart));
						m_iToken = tkString;
						iState = stDone;
						break;
						}

					case '\\':
						m_sToken.Append(CString(pStart, m_pPos - pStart));
						iState = stEscapeQuote;
						iNextState = stQuotedString;
						break;
					}
				break;
				}

			case stEscapeQuote:
				{
				m_sToken.Append(CString(m_pPos, 1));
				pStart = m_pPos+1;
				iState = iNextState;
				break;
				}

			case stLineBlock:
				{
				switch (*m_pPos)
					{
					case '\n':
					case '\r':
						m_sToken.Append(CString(pStart, m_pPos - pStart));
						iState = stCheckForEndLineBlock;
						break;

					case '\\':
						m_sToken.Append(CString(pStart, m_pPos - pStart));
						iState = stEscapeQuote;
						iNextState = stLineBlock;
						break;
					}
				break;
				}

			case stCheckForEndLineBlock:
				{
				switch (*m_pPos)
					{
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						break;

					case '|':
						m_sToken.Append(CONSTLIT("\n"));
						pStart = m_pPos + 1;
						iState = stLineBlock;
						break;

					default:
						{
						m_iToken = tkLineBlock;
						iState = stDoneKeepNextChar;
						}
					}
				break;
				}

			case stParagraph:
				{
				switch (*m_pPos)
					{
					case '\n':
					case '\r':
						m_sToken.Append(CString(pStart, m_pPos - pStart));
						iState = stCheckForEndParagraph;
						break;

					case '\\':
						m_sToken.Append(CString(pStart, m_pPos - pStart));
						iState = stEscapeQuote;
						iNextState = stParagraph;
						break;

					default:
						bBlankLine = false;
					}
				break;
				}

			case stCheckForEndParagraph:
				{
				switch (*m_pPos)
					{
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						break;

					case '|':
						if (bBlankLine)
							m_sToken.Append(CONSTLIT("\n"));
						else
							m_sToken.Append(CONSTLIT(" "));

						bBlankLine = true;
						pStart = m_pPos + 1;
						iState = stParagraph;
						break;

					default:
						{
						m_iToken = tkParagraph;
						iState = stDoneKeepNextChar;
						}
					}
				break;
				}

			case stStartCode:
				{
				switch (*m_pPos)
					{
					//	If this is a close brace then we are done

					case '}':
						{
						iState = stDone;
						m_iToken = tkCode;
						m_sToken = CONSTLIT("");
						break;
						}

					//	Otherwise, we keep parsing until our nesting
					//	level is down to 0.

					default:
						{
						pStart = m_pPos;
						iNesting = 1;
						iState = stCode;
						}
					}
				break;
				}

			case stCode:
				{
				switch (*m_pPos)
					{
					case '/':
						{
						iState = stCodeCheckForComment;
						break;
						}

					case '{':
						{
						iNesting++;
						break;
						}

					case '}':
						{
						iNesting--;
						if (iNesting == 0)
							{
							iState = stDone;
							m_iToken = tkCode;
							m_sToken = CString(pStart, m_pPos - pStart);
							}
						break;
						}
					}
				break;
				}

			case stCodeCheckForComment:
				{
				switch (*m_pPos)
					{
					case '/':
						iState = stCodeLineComment;
						break;

					case '*':
						iState = stCodeBlockComment;
						break;

					default:
						{
						iState = stCode;
						}
					}
				break;
				}

			case stCodeLineComment:
				{
				switch (*m_pPos)
					{
					case '\r':
					case '\n':
						iState = stCode;
						break;
					}
				break;
				}

			case stCodeBlockComment:
				{
				switch (*m_pPos)
					{
					case '*':
						iState = stCodeBlockCommentEnd;
						break;
					}
				break;
				}

			case stCodeBlockCommentEnd:
				{
				switch (*m_pPos)
					{
					case '/':
						iState = stCode;
						break;

					default:
						iState = stCodeBlockComment;
					}
				break;
				}

			default:
				ASSERT(false);
			}

		if (iState != stDoneKeepNextChar)
			{
			if (*m_pPos == '\n')
				m_iLineNumber++;

			m_pPos++;
			}
		}

	//	Done

	if (retsToken)
		*retsToken = m_sToken;

	return m_iToken;
	}

CString BabelQuotedString (const CString &sString)
	{
	char *pPos = sString.GetASCIIZ();
	char *pStart = (char *)_alloca(2 * sString.GetLength() + 2);
	char *pDest = pStart;
	while (*pPos)
		{
		if (*pPos == '\"')
			{
			*pDest++ = '\\';
			*pDest++ = '\"';
			}
		else if (*pPos == '\\')
			{
			*pDest++ = '\\';
			*pDest++ = '\\';
			}
		else
			*pDest++ = *pPos;

		pPos++;
		}

	*pDest = '\0';

	return CString(pStart);
	}
