//	CPPLexer.cpp
//
//	Lexer for CPP syntax
//	Copyright (c) 2001 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"
#include "CPPLexer.h"

enum States
	{
	stateStart = 0,

	stateAmpersand			= 1,
	stateBang				= 2,
	stateBlockComment		= 3,
	stateBlockCommentEnd	= 4,
	stateDecimal			= 5,
	stateEquals				= 6,
	stateGreaterThan		= 7,
	stateHex				= 8,
	stateIdentifier			= 9,
	stateLessThan			= 10,
	stateLineComment		= 11,
	stateQuote				= 12,
	stateQuoteEscape		= 13,
	stateRadix				= 14,
	stateSlash				= 15,
	stateVerticalBar		= 16,
	stateWhiteSpace			= 17,

	stateEndToken			= 100,
	stateNextToken			= 101,
	};

struct StateTrans
	{
	char chChar;
	CPPTokens iToken;
	int iNewState;
	};

#define DEFINE(x)	{'\0', tkEOS, x },

#define chAlpha			'\x80'
#define chNumber		'\x81'
#define chWhite			'\x82'

static StateTrans g_StateTransitions[] =
	{
	DEFINE(stateStart)
		{ chWhite,	tkNone,				stateWhiteSpace },

		{	'_',	tkNone,				stateIdentifier },
		{ chAlpha,	tkNone,				stateIdentifier },

		{	'/',	tkNone,				stateSlash },
		{	'!',	tkNone,				stateBang },
		{	'*',	tkTimes,			stateEndToken },
		{	'(',	tkOpenParen,		stateEndToken },
		{	')',	tkCloseParen,		stateEndToken },
		{	'-',	tkMinus,			stateEndToken },
		{	'+',	tkPlus,				stateEndToken },
		{	'=',	tkNone,				stateEquals },
		{	'{',	tkOpenBrace,		stateEndToken },
		{	'}',	tkCloseBrace,		stateEndToken },
		{	'[',	tkOpenBracket,		stateEndToken },
		{	']',	tkCloseBracket,		stateEndToken },
		{	';',	tkSemiColon,		stateEndToken },
		{	'>',	tkNone,				stateGreaterThan },
		{	'<',	tkNone,				stateLessThan },
		{	',',	tkComma,			stateEndToken },
		{	'&',	tkNone,				stateAmpersand },
		{	'|',	tkNone,				stateVerticalBar },
		{	'\"',	tkNone,				stateQuote },
		{	'?',	tkTernary,			stateEndToken },
		{	':',	tkColon,			stateEndToken },

		{	'0',	tkNone,				stateRadix },
		{ chNumber,	tkNone,				stateDecimal },

		{	'\0',	tkNone,				stateEndToken },

	DEFINE(stateAmpersand)
		{	'&',	tkAnd,				stateEndToken },
		{	'\0',	tkError,			stateEndToken },

	DEFINE(stateBang)
		{	'=',	tkNotEquals,		stateEndToken },
		{	'\0',	tkNot,				stateNextToken },

	DEFINE(stateBlockComment)
		{	'*',	tkNone,				stateBlockCommentEnd },
		{	'\0',	tkNone,				stateBlockComment },

	DEFINE(stateBlockCommentEnd)
		{	'/',	tkBlockComment,		stateEndToken },
		{	'\0',	tkNone,				stateBlockComment },

	DEFINE(stateDecimal)
		{ chNumber,	tkNone,				stateDecimal },
		{	'\0',	tkDecimalLiteral,	stateNextToken },

	DEFINE(stateEquals)
		{	'=',	tkEquals,			stateEndToken },
		{	'\0',	tkAssign,			stateNextToken },

	DEFINE(stateGreaterThan)
		{	'=',	tkGreaterThanOrEqual,	stateEndToken },
		{	'\0',	tkGreaterThan,		stateNextToken },

	DEFINE(stateHex)
		{ chNumber,	tkNone,				stateHex },
		{	'a',	tkNone,				stateHex },
		{	'b',	tkNone,				stateHex },
		{	'c',	tkNone,				stateHex },
		{	'd',	tkNone,				stateHex },
		{	'e',	tkNone,				stateHex },
		{	'f',	tkNone,				stateHex },
		{	'A',	tkNone,				stateHex },
		{	'B',	tkNone,				stateHex },
		{	'C',	tkNone,				stateHex },
		{	'D',	tkNone,				stateHex },
		{	'E',	tkNone,				stateHex },
		{	'F',	tkNone,				stateHex },
		{	'\0',	tkHexLiteral,		stateNextToken },

	DEFINE(stateIdentifier)
		{	'_',	tkNone,				stateIdentifier },
		{ chAlpha,	tkNone,				stateIdentifier },
		{ chNumber,	tkNone,				stateIdentifier },
		{	'\0',	tkIdentifier,		stateNextToken },

	DEFINE(stateLessThan)
		{	'=',	tkLessThanOrEqual,	stateEndToken },
		{	'\0',	tkLessThan,			stateNextToken },

	DEFINE(stateLineComment)
		{	'\r',	tkLineComment,		stateEndToken },
		{	'\n',	tkLineComment,		stateEndToken },
		{	'\0',	tkLineComment,		stateLineComment },

	DEFINE(stateQuote)
		{	'\\',	tkNone,				stateQuoteEscape },
		{	'\"',	tkStringLiteral,	stateEndToken },
		{	'\0',	tkNone,				stateQuote },

	DEFINE(stateQuoteEscape)
		{	'\0',	tkNone,				stateQuote },

	DEFINE(stateRadix)
		{	'x',	tkNone,				stateHex },
		{	'X',	tkNone,				stateHex },
		{ chNumber,	tkError,			stateEndToken },
		{ chAlpha,	tkError,			stateEndToken },
		{	'\0',	tkDecimalLiteral,	stateNextToken },

	DEFINE(stateSlash)
		{	'/',	tkNone,				stateLineComment },
		{	'*',	tkNone,				stateBlockComment },
		{	'\0',	tkDividedBy,		stateNextToken },

	DEFINE(stateVerticalBar)
		{	'|',	tkOr,				stateEndToken },
		{	'\0',	tkError,			stateEndToken },

	DEFINE(stateWhiteSpace)
		{ chWhite,	tkWhiteSpace,		stateWhiteSpace },
		{	'\0',	tkWhiteSpace,		stateNextToken },

	DEFINE(stateEndToken)
	};

inline bool IsAlpha (char ch)
	{
	return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
	}

inline bool IsNumber (char ch)
	{
	return (ch >= '0' && ch <='9');
	}

inline bool IsWhite (char ch)
	{
	return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\v' || ch == '\f');
	}

CCPPLexer::CCPPLexer (char *pPos, int iLength)

//	CPPLexer constructor

	{
	m_pPos = pPos;
	m_pEndPos = pPos + iLength;
	m_iToken = tkEOS;
	m_iLineNumber = 1;

	//	Loop over all state transitions looking for the beginning
	//	of each state table

	int iOffset = 0;
	StateTrans *pTrans = g_StateTransitions;
	while (pTrans->iToken != tkEOS 
			|| pTrans->iNewState != stateEndToken)
		{
		if (pTrans->iToken == tkEOS)
			m_iStateOffset[pTrans->iNewState] = iOffset + 1;

		pTrans++;
		iOffset++;
		}
	}

int CCPPLexer::GetTokenInteger (void)

//	GetTokenInteger
//
//	Returns the integer value of the token

	{
	switch (m_iToken)
		{
		case tkDecimalLiteral:
		case tkHexLiteral:
			return strToInt(m_sToken, 0, NULL);
		}

	return -1;
	}

CPPTokens CCPPLexer::NextToken (void)

//	NextToken
//
//	Returns the next token

	{
	char *pPos;
	char *pEndPos = m_pPos;
	CPPTokens iToken;

	do
		{
		pPos = pEndPos;
		iToken = Recognize(pPos, &pEndPos);
		}
	while (iToken == tkWhiteSpace || iToken == tkLineComment || iToken == tkBlockComment);

	m_iToken = iToken;
	m_sToken = CString(pPos, pEndPos - pPos);
	m_pPos = pEndPos;

	return iToken;
	}

CPPTokens CCPPLexer::Recognize (char *pPos, char **retpEndPos)

//	Recognize
//
//	Recognize a token

	{
	char *pEndPos = pPos;
	int iState = stateStart;
	CPPTokens iToken = tkNone;

	while (iToken == tkNone)
		{
		//	Look for transition
		StateTrans *pTrans = g_StateTransitions + m_iStateOffset[iState];
		while (pTrans->chChar != '\0'
				&& pTrans->chChar != *pEndPos
				&& (pTrans->chChar != chAlpha || !IsAlpha(*pEndPos))
				&& (pTrans->chChar != chNumber || !IsNumber(*pEndPos))
				&& (pTrans->chChar != chWhite || !IsWhite(*pEndPos)))
			pTrans++;

		//	If we're done, then return
		if (pTrans->iNewState == stateEndToken)
			{
			if (pTrans->iToken == tkNone)
				iToken = tkEOS;
			else
				{
				if (*pEndPos == '\n')
					m_iLineNumber++;
				pEndPos++;
				iToken = pTrans->iToken;
				}
			ASSERT(pEndPos <= m_pEndPos);
			}
		else if (pTrans->iNewState == stateNextToken)
			iToken = pTrans->iToken;
		else
			{
			if (pEndPos == m_pEndPos)
				iToken = tkError;
			else
				{
				if (*pEndPos == '\n')
					m_iLineNumber++;
				pEndPos++;
				iState = pTrans->iNewState;
				}
			}
		}

	*retpEndPos = pEndPos;
	return iToken;
	}
