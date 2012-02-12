//	BabelParser.cpp
//
//	CParser object
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"
#include "Babel.h"
#include "BabelParser.h"

CParser::CParser (char *pPos, char *pEndPos) : m_Lexer(pPos, pEndPos)
	{
	}

CBabelData CParser::Parse (void)
	{
	//	Start

	CString sEntryName;
	BabelTokens iToken = NextToken(&sEntryName);

	//	Is this the end?

	if (iToken == tkEOS)
		return CBabelData(dtNil);

	//	Parse a value

	return ParseValue();
	}

CBabelData CParser::ParseCodeValue (void)

//	ParseCodeValue
//
//	Parses a code value:
//
//	Code
//		{
//		... code with C++ style comments
//		}

	{
	//	Skip the code identifier

	NextToken(NULL);

	//	The next token better be an open brace

	if (CurToken(NULL) != tkLeftBrace)
		ThrowError(CONSTLIT("{ expected"));

	//	Parse the code

	CString sCode;
	if (m_Lexer.NextToken(&sCode, CLexer::modeCode) != tkCode)
		ThrowError(CONSTLIT("invalid code"));

	//	We've got the code

	NextToken();
	return CBabelData(dtCode, sCode);
	}

CBabelData CParser::ParseListValue (void)

//	ParseListValue
//
//	Parses a list

	{
	//	Make sure we start with open bracket

	if (CurToken(NULL) != tkLeftBracket)
		ThrowError(CONSTLIT("[ expected"));

	CBabelData List(dtList);

	//	Keep looping until we're done

	NextToken(NULL);
	while (CurToken(NULL) != tkRightBracket)
		{
		List.AddElement(ParseValue());

		//	Skip commas

		if (CurToken(NULL) == tkComma)
			NextToken(NULL);
		}

	//	Skip bracket

	NextToken(NULL);

	//	Done

	return List;
	}

CBabelData CParser::ParseStructureValue (void)

//	ParseStructureValue
//
//	Parses a structure

	{
	//	Make sure we start with open brace

	if (CurToken(NULL) != tkLeftBrace)
		ThrowError(CONSTLIT("{ expected"));

	CBabelData Struct(dtStructure);

	//	Keep looping through all fields

	BabelTokens iToken = NextToken(NULL);
	while (iToken == tkIdentifier 
			|| iToken == tkEquals
			|| iToken == tkOtherSymbol)
		{
		CString sIdentifier;
		CurToken(&sIdentifier);

		//	Must have a colon separator
		if (NextToken(NULL) != tkColon)
			ThrowError(CONSTLIT(": expected"));

		//	Get the value
		NextToken(NULL);
		Struct.AddField(sIdentifier, ParseValue());
		iToken = CurToken(NULL);
		}

	//	Better end on a right-brace

	if (CurToken(NULL) != tkRightBrace)
		ThrowError(CONSTLIT("} expected"));

	//	Next token...

	NextToken(NULL);

	//	Done

	return Struct;
	}

CBabelData CParser::ParseValue (void)

//	ParseValue
//
//	Parses a value

	{
	//	Parse the value based on what we've got

	CString sToken;
	switch (CurToken(&sToken))
		{
		case tkIdentifier:
			{
			if (strEquals(sToken, CONSTLIT("nil")))
				return CBabelData(dtNil);
			else if (strEquals(sToken, CONSTLIT("code")))
				return ParseCodeValue();
			else
				ThrowError(strCat(CONSTLIT("Invalid identifier: "), sToken));
			}

		case tkInteger:
			{
			NextToken(NULL);
			bool bFailed;
			int iInt = strToInt(sToken, 0, &bFailed);
			if (bFailed)
				ThrowError(strCat(CONSTLIT("Invalid integer: "), sToken));

			return CBabelData(iInt);
			}

		case tkString:
		case tkLineBlock:
		case tkParagraph:
			{
			NextToken(NULL);
			return CBabelData(sToken);
			}

		case tkLeftBracket:
			return ParseListValue();

		case tkLeftBrace:
			return ParseStructureValue();

		default:
			ThrowError(strCat(CONSTLIT("Unexpected token: "), sToken));
		}

	//	Should never get here...
	ASSERT(false);
	return CBabelData(dtNil);
	}

void CParser::ThrowError (const CString &sError)

//	ThrowError
//
//	Throws a parsing error

	{
	throw CException(errFail, strPatternSubst(CONSTLIT("(%d) : error: %s"), 
			m_Lexer.GetCurLineNumber(),
			sError));
	}
