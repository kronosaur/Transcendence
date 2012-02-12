//	CLexer.cpp
//
//	CLexer object

#include "Alchemy.h"
#include "Labyrinth.h"

inline bool IsIdentifierChar (char chChar)
	{
	return (chChar >= '0' && chChar <= '9')
			|| (chChar >= 'A' && chChar <= 'Z')
			|| (chChar >= 'a' && chChar <= 'z')
			|| (chChar == '_');
	}

CLexer::CLexer (char *pPos, char *pEndPos) :
		m_pPos(pPos),
		m_pEndPos(pEndPos),
		m_iLineNumber(1)

//	CLexer Constructor

	{
	}

GrailTokens CLexer::NextToken (CString *retsToken)

//	NextToken
//
//	Returns the next token in the stream

	{
	enum States
		{
		stStart,
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
		};

	States iState = stStart;
	char *pStart;
	m_sToken.Truncate(0);
	m_iToken = tkEOS;

	States iNextState;					//	Used for stEscapeQuote
	bool bBlankLine;					//	Used for stParagraph

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

					default:
						if (IsIdentifierChar(*m_pPos))
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
