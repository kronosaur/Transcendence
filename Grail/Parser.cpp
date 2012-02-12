//	Parser.cpp
//
//	Grail Language Parser

#include "Alchemy.h"
#include "Labyrinth.h"

class Parser
	{
	public:
		Parser (char *pPos, char *pEndPos);

		ALERROR ParseEntry (CLScope *pScope, CLEntry **retpEntry, CString *retsError);

	private:
		ALERROR MakeError (const CString &sError, CString *retsError);
		ALERROR ParseAlgorithmValue (CLValue **retpValue, CString *retsError);
		ALERROR ParseCategories (CLEntry *pEntry, CString *retsError);
		ALERROR ParseEntryDefinition (CLEntry *pEntry, CString *retsError);
		ALERROR ParseListValue (CLValue **retpValue, CString *retsError);
		ALERROR ParseStructureValue (CLValue **retpValue, CString *retsError);
		ALERROR ParseValue (CLValue **retpValue, CString *retsError);

		inline GrailTokens CurToken (CString *retsToken) { return m_Lexer.CurToken(retsToken); }
		inline GrailTokens NextToken (CString *retsToken) { return m_Lexer.NextToken(retsToken); }

		CLexer m_Lexer;
	};

ALERROR CLabyrinth::ParseEntries (char *pPos, char *pEndPos, CString *retsError)

//	ParseEntries
//
//	Parses entries from the given stream and adds them to the
//	database (or returns an error).

	{
	ALERROR error = NOERROR;
	Parser theParser(pPos, pEndPos);
	CString sError;

	//	Keep looping until we can't find any more entries

	while (!error)
		{
		CLEntry *pEntry;

		if ((error = theParser.ParseEntry(&m_GlobalScope, &pEntry, retsError)) == NOERROR)
			{
			if (error = m_GlobalScope.AddEntry(pEntry))
				{
				*retsError = CONSTLIT("Out of memory");
				return error;
				}
			}
		}

	//	Any error except "end of file" is a real error

	if (error != ERR_NOTFOUND)
		return error;

	return NOERROR;
	}

Parser::Parser (char *pPos, char *pEndPos) : m_Lexer(pPos, pEndPos)
	{
	}

ALERROR Parser::MakeError (const CString &sError, CString *retsError)
	{
	*retsError = strPatternSubst(CONSTLIT("(%d) : error: %s"), 
			m_Lexer.GetCurLineNumber(),
			sError);

	return ERR_FAIL;
	}

ALERROR Parser::ParseAlgorithmValue (CLValue **retpValue, CString *retsError)
	{
#if 0
	//	Make sure we start with a bang

	if (CurToken(NULL) != tkBang)
		return MakeError(CONSTLIT("! expected"), retsError);

	//	Next token should be an open brace

	if (NextToken(NULL) != tkLeftBrace)
		return MakeError(CONSTLIT("{ expected"), retsError);

	//	Create the algorithm value

	CLAlgorithm *pAlgorithm = new CLAlgorithm;
	if (pAlgorithm == NULL)
		return MakeError(CONSTLIT("Out of memory"), retsError);

	//	Now parse a code block

	if (error = ParseCodeBlock(
#endif

	return NOERROR;
	}

ALERROR Parser::ParseCategories (CLEntry *pEntry, CString *retsError)
	{
	//	Make sure we start with open bracket

	if (CurToken(NULL) != tkLeftBracket)
		return MakeError(CONSTLIT("[ expected"), retsError);

	//	Now loop until we're done

	CString sCategory;
	GrailTokens iToken = NextToken(&sCategory);
	while (iToken == tkIdentifier)
		{
		pEntry->AddCategory(sCategory);

		iToken = NextToken(&sCategory);
		if (iToken == tkComma)
			iToken = NextToken(&sCategory);
		}

	if (iToken != tkRightBracket)
		return MakeError(CONSTLIT("] or , expected"), retsError);

	//	Next token...

	NextToken(NULL);

	return NOERROR;
	}

ALERROR Parser::ParseEntry (CLScope *pScope, CLEntry **retpEntry, CString *retsError)
	{
	ALERROR error;

	//	Start

	CString sEntryName;
	GrailTokens iToken = NextToken(&sEntryName);

	//	Is this the end?

	if (iToken == tkEOS)
		return ERR_NOTFOUND;

	//	Better be an identifier

	if (iToken != tkIdentifier)
		return MakeError(CONSTLIT("Identifier expected"), retsError);

	//	Create a new entry

	CLEntry *pEntry = new CLEntry(sEntryName, pScope);
	if (pEntry == NULL)
		return MakeError(CONSTLIT("Out of memory"), retsError);

	//	Parse the categories

	if (NextToken(NULL) != tkLeftBracket)
		return MakeError(CONSTLIT("[ expected"), retsError);

	if (error = ParseCategories(pEntry, retsError))
		return error;

	//	Expect double-colon

	if (CurToken(NULL) != tkDoubleColon)
		return MakeError(CONSTLIT(":: expected"), retsError);

	//	Parse an entry definition

	NextToken(NULL);
	if (error = ParseEntryDefinition(pEntry, retsError))
		return error;

	//	Done

	*retpEntry = pEntry;

	return NOERROR;
	}

ALERROR Parser::ParseEntryDefinition (CLEntry *pEntry, CString *retsError)
	{
	ALERROR error;

	//	Make sure we start with open brace

	if (CurToken(NULL) != tkLeftBrace)
		return MakeError(CONSTLIT("{ expected"), retsError);

	//	Keep looping through all the fields and subentries

	CString sIdentifier;
	GrailTokens iToken = NextToken(&sIdentifier);
	while (iToken == tkIdentifier
			|| iToken == tkEquals
			|| iToken == tkOtherSymbol)
		{
		iToken = NextToken(NULL);

		//	If we've got a bracket then we must be doing a nested
		//	definition.

		if (iToken == tkLeftBracket)
			{
			CLEntry *pSubEntry = new CLEntry(sIdentifier, pEntry->GetScope());
			if (pSubEntry == NULL)
				return MakeError(CONSTLIT("Out of memory"), retsError);

			if (error = ParseCategories(pSubEntry, retsError))
				return error;

			//	Expect the double-colon definition

			if (CurToken(NULL) != tkDoubleColon)
				return MakeError(CONSTLIT(":: expected"), retsError);

			//	Now recurse

			NextToken(NULL);
			if (error = ParseEntryDefinition(pSubEntry, retsError))
				return error;

			//	Add subentry to our entry

			pEntry->AddSubEntry(pSubEntry);

			//	Next token...

			iToken = CurToken(&sIdentifier);
			}

		//	Otherwise we've got a field

		else if (iToken == tkColon)
			{
			NextToken(NULL);

			CLValue *pValue;
			if (error = ParseValue(&pValue, retsError))
				return error;

			//	Add a field definition

			if (error = pEntry->AddField(sIdentifier, pValue))
				return error;

			//	Next token...

			iToken = CurToken(&sIdentifier);
			}

		else
			return MakeError(CONSTLIT("Field or definition expected"), retsError);
		}

	//	This better be the right brace

	if (CurToken(NULL) != tkRightBrace)
		return MakeError(CONSTLIT("} expected"), retsError);

	//	Next token..

	NextToken(NULL);

	return NOERROR;
	}

ALERROR Parser::ParseListValue (CLValue **retpValue, CString *retsError)
	{
	ALERROR error;

	//	Make sure we start with open bracket

	if (CurToken(NULL) != tkLeftBracket)
		return MakeError(CONSTLIT("[ expected"), retsError);

	//	Create the list

	CLList *pList = new CLList;
	if (pList == NULL)
		return MakeError(CONSTLIT("Out of memory"), retsError);

	//	Keep looping until we're done

	NextToken(NULL);
	while (CurToken(NULL) != tkRightBracket)
		{
		CLValue *pValue;

		if (error = ParseValue(&pValue, retsError))
			return error;

		pList->AddValue(pValue);

		//	Skip commas

		if (CurToken(NULL) == tkComma)
			NextToken(NULL);
		}

	//	Skip bracket

	NextToken(NULL);

	//	Done

	*retpValue = pList;

	return NOERROR;
	}

ALERROR Parser::ParseStructureValue (CLValue **retpValue, CString *retsError)
	{
	ALERROR error;

	//	Make sure we start with open brace

	if (CurToken(NULL) != tkLeftBrace)
		return MakeError(CONSTLIT("{ expected"), retsError);

	//	Create a structure

	CLStructure *pStruct = new CLStructure;
	if (pStruct == NULL)
		return MakeError(CONSTLIT("Out of memory"), retsError);

	//	Keep looping through all fields

	CString sIdentifier;
	GrailTokens iToken = NextToken(&sIdentifier);
	while (iToken == tkIdentifier 
			|| iToken == tkEquals
			|| iToken == tkOtherSymbol)
		{
		NextToken(NULL);

		CLValue *pValue;
		if (error = ParseValue(&pValue, retsError))
			return error;

		pStruct->AddField(sIdentifier, pValue);
		}

	//	Better end on a right-brace

	if (CurToken(NULL) != tkRightBrace)
		return MakeError(CONSTLIT("} expected"), retsError);

	//	Next token...

	NextToken(NULL);

	return NOERROR;
	}

ALERROR Parser::ParseValue (CLValue **retpValue, CString *retsError)
	{
	ALERROR error;
	CLValue *pValue;

	//	Parse the value based on what we've got

	CString sToken;
	switch (CurToken(&sToken))
		{
		case tkString:
		case tkLineBlock:
		case tkParagraph:
			{
			pValue = new CLString(sToken);
			if (pValue == NULL)
				return MakeError(CONSTLIT("Out of memory"), retsError);

			NextToken(NULL);
			break;
			}

		case tkIdentifier:
			{
			pValue = new CLIdentifier(sToken);
			if (pValue == NULL)
				return MakeError(CONSTLIT("Out of memory"), retsError);

			NextToken(NULL);
			break;
			}

		case tkLeftBracket:
			{
			if (error = ParseListValue(&pValue, retsError))
				return error;

			break;
			}

		case tkLeftBrace:
			{
			if (error = ParseStructureValue(&pValue, retsError))
				return error;

			break;
			}

		case tkBang:
			{
			if (error = ParseAlgorithmValue(&pValue, retsError))
				return error;
			break;
			}

		default:
			return MakeError(strPatternSubst(CONSTLIT("Unexpected token: %s"), sToken), retsError);
		}

	//	Done

	*retpValue = pValue;

	return NOERROR;
	}
