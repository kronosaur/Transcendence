//	Parser.cpp
//
//	XML Parser

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"

enum TokenTypes
	{
	tkEOF,						//	end of file
	tkPIOpen,					//	<?
	tkPIClose,					//	?>
	tkTagOpen,					//	<
	tkTagClose,					//	>
	tkEndTagOpen,				//	</
	tkSimpleTagClose,			//	/>
	tkEquals,					//	=
	tkQuote,					//	"
	tkSingleQuote,				//	'
	tkText,						//	plain text
	tkDeclOpen,					//	<!
	tkBracketOpen,				//	[
	tkBracketClose,				//	]
	tkError,					//	error
	};

enum StateTypes
	{
	StartState,
	StartDeclState,
	OpenTagState,
	ContentState,
	SlashState,
	QuestionState,
	IdentifierState,
	TextState,
	AttributeTextState,
	CommentState,
	CDATAState,
	EntityState,
	AttributeState,
	ParseEntityState,
	EntityDeclarationState,
	EntityDeclarationFindValueState,
	EntityDeclarationValueState,
	EntityDeclarationEndState,
	};

#define STR_DOCTYPE								CONSTLIT("DOCTYPE")

struct ParserCtx
	{
	public:
		ParserCtx (IReadBlock *pStream, IXMLParserController *pController);
		ParserCtx (ParserCtx *pParentCtx, const CString &sString);

		void DefineEntity (const CString &sName, const CString &sValue);
		CString LookupEntity (const CString &sName, bool *retbFound = NULL);

	public:
		IXMLParserController *m_pController;
		ParserCtx *m_pParentCtx;

		char *pPos;
		char *pEndPos;

		CSymbolTable EntityTable;

		CXMLElement *pElement;

		TokenTypes iToken;
		CString sToken;
		int iLine;

		bool m_bParseRootElement;
		bool m_bParseRootTag;
		CString m_sRootTag;

		TokenTypes iAttribQuote;

		CString sError;
	};

ParserCtx::ParserCtx (IReadBlock *pStream, IXMLParserController *pController) : 
		EntityTable(TRUE, FALSE),
		m_pController(pController)
	{
	pPos = pStream->GetPointer(0, pStream->GetLength());
	pEndPos = pPos + pStream->GetLength();
	pElement = NULL;
	iToken = tkEOF;
	iLine = 1;
	m_bParseRootElement = false;
	m_bParseRootTag = false;

	m_pParentCtx = NULL;
	}

ParserCtx::ParserCtx (ParserCtx *pParentCtx, const CString &sString) : 
		EntityTable(TRUE, FALSE),
		m_pController(pParentCtx->m_pController)
	{
	pPos = sString.GetPointer();
	pEndPos = pPos + sString.GetLength();
	pElement = NULL;
	iToken = tkEOF;
	iLine = 1;
	m_bParseRootElement = false;
	m_bParseRootTag = false;

	m_pParentCtx = pParentCtx;
	}

void ParserCtx::DefineEntity (const CString &sName, const CString &sValue)
	{
	CString *pValue = new CString(sValue);
	EntityTable.AddEntry(sName, pValue);
	}

CString ParserCtx::LookupEntity (const CString &sName, bool *retbFound)
	{
	CString *pValue;

	//	Lookup in table

	if (EntityTable.Lookup(sName, (CObject **)&pValue) != NOERROR)
		{
		if (m_pParentCtx)
			return m_pParentCtx->LookupEntity(sName, retbFound);
		else if (m_pController)
			return m_pController->ResolveExternalEntity(sName, retbFound);
		else
			{
			if (retbFound) *retbFound = false;
			return sName;
			}
		}

	if (retbFound) *retbFound = true;
	return *pValue;
	}

//	Forwards

ALERROR ParseElement (ParserCtx *pCtx, CXMLElement **retpElement);
ALERROR ParsePrologue (ParserCtx *pCtx);
TokenTypes ParseToken (ParserCtx *pCtx, StateTypes iInitialState = StartState);
CString ResolveEntity (ParserCtx *pCtx, const CString &sName, bool *retbFound);

ALERROR CXMLElement::ParseXML (IReadBlock *pStream, 
							   CXMLElement **retpElement, 
							   CString *retsError,
							   CExternalEntityTable *retEntityTable)
	{
	return ParseXML(pStream, NULL, retpElement, retsError, retEntityTable);
	}

ALERROR CXMLElement::ParseXML (IReadBlock *pStream, 
							   IXMLParserController *pController,
							   CXMLElement **retpElement, 
							   CString *retsError,
							   CExternalEntityTable *retEntityTable)

//	ParseXML
//
//	Parses the block and returns an XML element

	{
	ALERROR error;

	//	Open the stream

	if (error = pStream->Open())
		{
		*retsError = CONSTLIT("unable to open XML stream");
		return error;
		}

	//	Initialize context

	ParserCtx Ctx(pStream, pController);

	//	Parse the prologue

	if (error = ParsePrologue(&Ctx))
		goto Fail;

	//	Next token must be an element open tag

	if (Ctx.iToken != tkTagOpen)
		{
		error = ERR_FAIL;
		Ctx.sError = LITERAL("root element expected");
		goto Fail;
		}

	//	Parse the root element

	if (error = ParseElement(&Ctx, retpElement))
		goto Fail;

	//	Done

	pStream->Close();
	if (retEntityTable)
		retEntityTable->AddTable(Ctx.EntityTable);

	return NOERROR;

Fail:

	pStream->Close();
	*retsError = strPatternSubst(LITERAL("Line(%d): %s"), Ctx.iLine, Ctx.sError);
	return error;
	}

ALERROR CXMLElement::ParseSingleElement (IReadBlock *pStream, 
										 IXMLParserController *pController,
										 CXMLElement **retpElement, 
										 CString *retsError)

//	ParseElement
//
//	Parses a single element

	{
	ALERROR error;

	//	Open the stream

	if (error = pStream->Open())
		{
		*retsError = CONSTLIT("unable to open XML stream");
		return error;
		}

	//	Initialize context

	ParserCtx Ctx(pStream, pController);

	//	Get the first token

	if (ParseToken(&Ctx) != tkTagOpen)
		{
		error = ERR_FAIL;
		Ctx.sError = LITERAL("Element expected");
		goto Fail;
		}

	//	Parse the root element

	if (error = ParseElement(&Ctx, retpElement))
		goto Fail;

	//	Done

	pStream->Close();

	return NOERROR;

Fail:

	pStream->Close();
	*retsError = strPatternSubst(LITERAL("Line(%d): %s"), Ctx.iLine, Ctx.sError);
	return error;
	}

ALERROR ParseDTD (ParserCtx *pCtx)

//	ParseDTD
//
//	Parse DTD declarations

	{
	ParseToken(pCtx, StartDeclState);
	while (pCtx->iToken != tkBracketClose
			&& pCtx->iToken != tkEOF)
		{
		ParseToken(pCtx, StartDeclState);
		}

	return NOERROR;
	}

ALERROR ParseElement (ParserCtx *pCtx, CXMLElement **retpElement)

//	ParseElement
//
//	Parses an element and returns it. We assume that we've already
//	parsed an open tag

	{
	ALERROR error;
	CXMLElement *pElement;

	ASSERT(pCtx->iToken == tkTagOpen);

	//	Parse the tag name

	if (ParseToken(pCtx) != tkText)
		{
		pCtx->sError = LITERAL("element tag expected");
		return ERR_FAIL;
		}

	//	Create a new element with the tag

	pElement = new CXMLElement(pCtx->sToken, pCtx->pElement);
	if (pElement == NULL)
		{
		pCtx->sError = LITERAL("out of memory");
		return ERR_FAIL;
		}

	//	Keep parsing until the tag is done

	ParseToken(pCtx);
	while (pCtx->iToken != tkTagClose && pCtx->iToken != tkSimpleTagClose)
		{
		//	If we've got an identifier then this must be an attribute

		if (pCtx->iToken == tkText)
			{
			CString sAttribute = pCtx->sToken;
			CString sValue;

			//	Expect an equals sign

			if (ParseToken(pCtx) != tkEquals)
				{
				pCtx->sError = LITERAL("= expected");
				delete pElement;
				return ERR_FAIL;
				}

			//	Expect a quote

			ParseToken(pCtx);
			if (pCtx->iToken != tkQuote && pCtx->iToken != tkSingleQuote)
				{
				pCtx->sError = LITERAL("attribute value must be quoted");
				delete pElement;
				return ERR_FAIL;
				}

			//	Remember what kind of qoute we used so that we can match it
			//	(and so we ignore the other kind inside it).

			pCtx->iAttribQuote = pCtx->iToken;

			//	Expect the value 

			ParseToken(pCtx, AttributeState);
			if (pCtx->iToken == tkText)
				{
				sValue = pCtx->sToken;
				ParseToken(pCtx);
				}
			else
				sValue = LITERAL("");

			//	Now expect an end-quote

			if (pCtx->iToken != pCtx->iAttribQuote)
				{
				if (pCtx->iToken != tkError || pCtx->sError.IsBlank())
					pCtx->sError = LITERAL("mismatched attribute quote");
				delete pElement;
				return ERR_FAIL;
				}

			//	Add the attribute to the element

			if (error = pElement->AddAttribute(sAttribute, sValue))
				{
				delete pElement;
				return ERR_FAIL;
				}

			//	Parse the next token

			ParseToken(pCtx);
			}

		//	Otherwise this is an error

		else
			{
			if (pCtx->iToken != tkError || pCtx->sError.IsBlank())
				pCtx->sError = LITERAL("attribute expected");
			delete pElement;
			return ERR_FAIL;
			}
		}

	//	Give our controller a chance to deal with an element
	//	(We use this in Transcendence to parse the <Library> element, which
	//	contains external entities).
	//
	//	NOTE: We only worry about top-level elements (i.e., elements immediately
	//	under the root).

	if (pCtx->m_pController && pCtx->pElement && pCtx->pElement->GetParentElement() == NULL)
		{
		if (error = pCtx->m_pController->OnOpenTag(pElement, &pCtx->sError))
			{
			delete pElement;
			return error;
			}
		}

	//	If we don't have an empty element then keep parsing until
	//	we find a close tag

	if (!pCtx->m_bParseRootElement && pCtx->iToken == tkTagClose)
		{
		CXMLElement *pParentElement;

		//	We are recursing

		pParentElement = pCtx->pElement;
		pCtx->pElement = pElement;

		//	Parse until we've got the begin close tag

		while (ParseToken(pCtx, ContentState) != tkEndTagOpen)
			{
			//	If this is text then append it as content

			if (pCtx->iToken == tkText)
				{
				if (error = pElement->AppendContent(pCtx->sToken))
					{
					pCtx->pElement = pParentElement;
					delete pElement;
					return error;
					}
				}

			//	Otherwise, append an element

			else if (pCtx->iToken == tkTagOpen)
				{
				CXMLElement *pSubElement;

				if (error = ParseElement(pCtx, &pSubElement))
					{
					pCtx->pElement = pParentElement;
					delete pElement;
					return error;
					}

				if (error = pElement->AppendSubElement(pSubElement))
					{
					pCtx->pElement = pParentElement;
					delete pElement;
					return error;
					}
				}

			//	Otherwise we're in trouble

			else
				{
				pCtx->pElement = pParentElement;
				if (pCtx->iToken != tkError || pCtx->sError.IsBlank())
					pCtx->sError = LITERAL("content expected");
				delete pElement;
				return ERR_FAIL;
				}
			}

		//	Done

		pCtx->pElement = pParentElement;

		//	The element tag should match ours

		if (ParseToken(pCtx) != tkText
				|| strCompareAbsolute(pCtx->sToken, pElement->GetTag()) != 0)
			{
			pCtx->sError = LITERAL("close tag does not match open");
			delete pElement;
			return ERR_FAIL;
			}

		//	Parse the end tag

		if (ParseToken(pCtx) != tkTagClose)
			{
			pCtx->sError = LITERAL("close tag expected");
			delete pElement;
			return ERR_FAIL;
			}
		}

	//	Done

	*retpElement = pElement;

	return NOERROR;
	}

ALERROR ParsePrologue (ParserCtx *pCtx)

//	ParsePrologue
//
//	Parses <?XML prologue

	{
	ALERROR error;

	//	We don't allow any whitespace at the beginning

	if (*pCtx->pPos != '<')
		{
		pCtx->sError = LITERAL("<?XML prologue expected");
		return ERR_FAIL;
		}

	//	Expect open processor instruction. If we don't find it,
	//	then we assume that there is no prologue and proceed
	//	to parse the root element.

	if (ParseToken(pCtx) != tkPIOpen)
		return NOERROR;

	//	Expect XML tag

	if (ParseToken(pCtx) != tkText || strCompareAbsolute(pCtx->sToken, LITERAL("xml")) != 0)
		{
		pCtx->sError = LITERAL("<?XML prologue expected");
		return ERR_FAIL;
		}

	//	Parse contents

	while (ParseToken(pCtx) == tkText)
		{
		if (strEquals(pCtx->sToken, CONSTLIT("version")))
			{
			if (ParseToken(pCtx) != tkEquals)
				{
				pCtx->sError = LITERAL("version attribute expected");
				return ERR_FAIL;
				}

			if (ParseToken(pCtx) != tkQuote)
				{
				pCtx->sError = LITERAL("version attribute expected");
				return ERR_FAIL;
				}

			if (ParseToken(pCtx) != tkText || strCompareAbsolute(pCtx->sToken, LITERAL("1.0")) != 0)
				{
				pCtx->sError = LITERAL("version 1.0 expected");
				return ERR_FAIL;
				}

			if (ParseToken(pCtx) != tkQuote)
				{
				pCtx->sError = LITERAL("invalid version attribute");
				return ERR_FAIL;
				}
			}
		else if (strEquals(pCtx->sToken, CONSTLIT("encoding")))
			{
			if (ParseToken(pCtx) != tkEquals)
				{
				pCtx->sError = LITERAL("encoding attribute expected");
				return ERR_FAIL;
				}

			if (ParseToken(pCtx) != tkQuote)
				{
				pCtx->sError = LITERAL("encoding attribute expected");
				return ERR_FAIL;
				}

			if (ParseToken(pCtx) != tkText)
				{
				pCtx->sError = LITERAL("invalid encoding attribute");
				return ERR_FAIL;
				}

			if (ParseToken(pCtx) != tkQuote)
				{
				pCtx->sError = LITERAL("invalid encoding attribute");
				return ERR_FAIL;
				}
			}
		else
			{
			//	Assume it is an unknown attribute

			if (ParseToken(pCtx) != tkEquals)
				{
				pCtx->sError = LITERAL("invalid prologue attribute");
				return ERR_FAIL;
				}

			if (ParseToken(pCtx) != tkQuote)
				{
				pCtx->sError = LITERAL("invalid prologue attribute");
				return ERR_FAIL;
				}

			if (ParseToken(pCtx) != tkText)
				{
				pCtx->sError = LITERAL("invalid prologue attribute");
				return ERR_FAIL;
				}

			if (ParseToken(pCtx) != tkQuote)
				{
				pCtx->sError = LITERAL("invalid prologue attribute");
				return ERR_FAIL;
				}
			}

#ifdef LATER
	//	Handle EncodingDecl
	//	Handle RMDecl
#endif
		}

	//	Expect close

	if (pCtx->iToken != tkPIClose)
		{
		pCtx->sError = LITERAL("invalid XML prologue");
		return ERR_FAIL;
		}

	//	See if we've got a DOCTYPE declaration

	if (ParseToken(pCtx) == tkDeclOpen)
		{
		if (ParseToken(pCtx) != tkText
				|| !strEquals(pCtx->sToken, STR_DOCTYPE))
			{
			pCtx->sError = LITERAL("DOCTYPE declaration expected");
			return ERR_FAIL;
			}

		//	Get the name

		if (ParseToken(pCtx) != tkText)
			{
			pCtx->sError = LITERAL("DOCTYPE: name expected");
			return ERR_FAIL;
			}

		//	If we're just looking for the root tag, then we can quit after this

		if (pCtx->m_bParseRootTag)
			{
			pCtx->m_sRootTag = pCtx->sToken;
			return NOERROR;
			}

		//	External ID?

		if (ParseToken(pCtx, StartDeclState) == tkText)
			{
			//	Either SYSTEM or PUBLIC

			//	Expect a quote

			if (ParseToken(pCtx) != tkQuote)
				{
				pCtx->sError = LITERAL("DOCTYPE: external ID expected");
				return ERR_FAIL;
				}

			pCtx->iAttribQuote = tkQuote;

			//	Get the path or URL

			if (ParseToken(pCtx, AttributeState) != tkText)
				{
				pCtx->sError = LITERAL("DOCTYPE: external ID expected");
				return ERR_FAIL;
				}

			CString sDTDPath = pCtx->sToken;

			//	End quote

			if (ParseToken(pCtx) != pCtx->iAttribQuote)
				{
				pCtx->sError = LITERAL("DOCTYPE: \" expected");
				return ERR_FAIL;
				}

			ParseToken(pCtx);
			}

		//	Internal DTD?

		if (pCtx->iToken == tkBracketOpen)
			{
			if (error = ParseDTD(pCtx))
				return error;

			if (pCtx->iToken != tkBracketClose)
				{
				pCtx->sError = LITERAL("DOCTYPE: ] expected");
				return ERR_FAIL;
				}

			ParseToken(pCtx);
			}

		//	Close

		if (pCtx->iToken != tkTagClose)
			{
			pCtx->sError = LITERAL("DOCTYPE: > expected");
			return ERR_FAIL;
			}

		ParseToken(pCtx);
		}

	//	Consume tokens until we get a tag open

	while (pCtx->iToken == tkText)
		ParseToken(pCtx);

	return NOERROR;
	}

TokenTypes ParseToken (ParserCtx *pCtx, StateTypes iInitialState)

//	ParseToken
//
//	Parses the next token and updatex pCtx->iToken and pCtx->sToken.
//	If bContent is TRUE, then we treat whitespace as text.

	{
	bool bDone = false;
	bool bNoEOF = false;
	StateTypes iState;
	char *pStartRun;
	CString sName;

	//	If we're parsing an entity then the rules change slightly

	if (iInitialState == ParseEntityState)
		{
		iInitialState = TextState;
		pStartRun = pCtx->pPos;
		pCtx->sToken = CString("");
		bNoEOF = true;
		}

	//	If we're parsing content then start in the content state

	iState = iInitialState;

	//	Keep parsing until we're done

	while (pCtx->pPos < pCtx->pEndPos && !bDone)
		{
		char chChar = *pCtx->pPos;
		StateTypes iSavedState;

		switch (iState)
			{
			case StartState:
				{
				switch (chChar)
					{
					//	Swallow whitespace
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						break;

					case '<':
						iState = OpenTagState;
						break;

					case '>':
						pCtx->iToken = tkTagClose;
						bDone = true;
						break;

					case '/':
						iState = SlashState;
						break;

					case '?':
						iState = QuestionState;
						break;

					case '=':
						pCtx->iToken = tkEquals;
						bDone = true;
						break;

					case '"':
						pCtx->iToken = tkQuote;
						bDone = true;
						break;

					case '\'':
						pCtx->iToken = tkSingleQuote;
						bDone = true;
						break;

					default:
						iState = IdentifierState;
						pStartRun = pCtx->pPos;
						break;
					}

				break;
				}

			case StartDeclState:
				{
				switch (chChar)
					{
					//	Swallow whitespace
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						break;

					case '<':
						iState = OpenTagState;
						break;

					case '>':
						pCtx->iToken = tkTagClose;
						bDone = true;
						break;

					case '/':
						iState = SlashState;
						break;

					case '?':
						iState = QuestionState;
						break;

					case '=':
						pCtx->iToken = tkEquals;
						bDone = true;
						break;

					case '"':
						pCtx->iToken = tkQuote;
						bDone = true;
						break;

					case '[':
						pCtx->iToken = tkBracketOpen;
						bDone = true;
						break;

					case ']':
						pCtx->iToken = tkBracketClose;
						bDone = true;
						break;

					default:
						iState = IdentifierState;
						pStartRun = pCtx->pPos;
						break;
					}

				break;
				}

			case ContentState:
				{
				switch (chChar)
					{
					case '<':
						iState = OpenTagState;
						break;

					case '>':
						pCtx->iToken = tkTagClose;
						bDone = true;
						break;

					case '&':
						iState = EntityState;
						iSavedState = TextState;
						pStartRun = pCtx->pPos + 1;
						pCtx->sToken = CString("");
						break;

					default:
						iState = TextState;
						pStartRun = pCtx->pPos;
						pCtx->sToken = CString("");
						break;
					}

				break;
				}

			case AttributeState:
				{
				switch (chChar)
					{
					case '&':
						iState = EntityState;
						iSavedState = AttributeTextState;
						pStartRun = pCtx->pPos + 1;
						pCtx->sToken = CString("");
						break;

					default:
						if ((chChar == '"' && pCtx->iAttribQuote == tkQuote)
								|| (chChar == '\'' && pCtx->iAttribQuote == tkSingleQuote))
							{
							pCtx->iToken = pCtx->iAttribQuote;
							bDone = true;
							break;
							}
						else
							{
							iState = AttributeTextState;
							pStartRun = pCtx->pPos;
							pCtx->sToken = CString("");
							break;
							}
					}

				break;
				}

			case OpenTagState:
				{
				switch (chChar)
					{
					case '?':
						pCtx->iToken = tkPIOpen;
						bDone = true;
						break;

					case '/':
						pCtx->iToken = tkEndTagOpen;
						bDone = true;
						break;

					case '!':
						{
						//	Is this a comment?

						if ((pCtx->pPos + 2 < pCtx->pEndPos)
								&& pCtx->pPos[1] == '-' 
								&& pCtx->pPos[2] == '-')
							{
							iState = CommentState;
							pCtx->pPos += 2;
							}

						//	Is this CDATA?

						else if ((pCtx->pPos + 7 < pCtx->pEndPos)
								&& pCtx->pPos[1] == '['
								&& pCtx->pPos[2] == 'C'
								&& pCtx->pPos[3] == 'D'
								&& pCtx->pPos[4] == 'A'
								&& pCtx->pPos[5] == 'T'
								&& pCtx->pPos[6] == 'A'
								&& pCtx->pPos[7] == '[')
							{
							iState = CDATAState;
							pCtx->sToken = CONSTLIT("");
							pCtx->pPos += 7;
							pStartRun = pCtx->pPos + 1;
							}

						//	Is this and entity declaration?

						else if ((pCtx->pPos + 7 < pCtx->pEndPos)
								&& pCtx->pPos[1] == 'E'
								&& pCtx->pPos[2] == 'N'
								&& pCtx->pPos[3] == 'T'
								&& pCtx->pPos[4] == 'I'
								&& pCtx->pPos[5] == 'T'
								&& pCtx->pPos[6] == 'Y')
							{
							pCtx->pPos += 7;

							//	Skip any whitespace

							while (*pCtx->pPos == ' '
									|| *pCtx->pPos == '\t'
									|| *pCtx->pPos == '\r'
									|| *pCtx->pPos == '\n')
								pCtx->pPos++;

							iState = EntityDeclarationState;
							pStartRun = pCtx->pPos;
							}

						//	Else it is a declaration of some sort

						else
							{
							pCtx->iToken = tkDeclOpen;
							bDone = true;
							}

						break;
						}

					default:
						pCtx->iToken = tkTagOpen;
						pCtx->pPos--;
						bDone = true;
						break;
					}

				break;
				}

			case EntityDeclarationState:
				{
				switch (chChar)
					{
					//	Whitespace means the end of the name
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						sName = CString(pStartRun, pCtx->pPos - pStartRun);
						iState = EntityDeclarationFindValueState;
						break;

					case '\"':
					case '>':
					case '<':
						pCtx->iToken = tkError;
						bDone = true;
						break;
					}
				break;
				}

			case EntityDeclarationFindValueState:
				{
				switch (chChar)
					{
					case '\"':
						pStartRun = pCtx->pPos + 1;
						iState = EntityDeclarationValueState;
						break;

					case '>':
					case '<':
						pCtx->iToken = tkError;
						bDone = true;
						break;
					}
				break;
				}

			case EntityDeclarationValueState:
				{
				switch (chChar)
					{
					case '\"':
						{
						CString sValue = CString(pStartRun, pCtx->pPos - pStartRun);
						pCtx->DefineEntity(sName, sValue);
						iState = EntityDeclarationEndState;
						break;
						}

					case '>':
					case '<':
						pCtx->iToken = tkError;
						bDone = true;
						break;
					}
				break;
				}

			case EntityDeclarationEndState:
				{
				if (chChar == '>')
					iState = StartDeclState;
				break;
				}

			case SlashState:
				{
				if (chChar == '>')
					pCtx->iToken = tkSimpleTagClose;
				else
					pCtx->iToken = tkError;

				bDone = true;
				break;
				}

			case QuestionState:
				{
				if (chChar == '>')
					pCtx->iToken = tkPIClose;
				else
					pCtx->iToken = tkError;

				bDone = true;
				break;
				}

			case IdentifierState:
				{
				switch (chChar)
					{
					case ' ':
					case '\t':
					case '\n':
					case '\r':
					case '=':
					case '>':
					case '?':
					case '/':
					case '"':
					case '<':
						pCtx->iToken = tkText;
						pCtx->sToken = CString(pStartRun, pCtx->pPos - pStartRun);
						pCtx->pPos--;
						bDone = true;
						break;
					}

				break;
				}

			case TextState:
				{
				switch (chChar)
					{
					case '<':
					case '>':
						pCtx->iToken = tkText;
						pCtx->sToken.Append(CString(pStartRun, pCtx->pPos - pStartRun));
						pCtx->pPos--;
						bDone = true;
						break;

					//	Handle embeded entities

					case '&':
						pCtx->sToken.Append(CString(pStartRun, pCtx->pPos - pStartRun));
						pStartRun = pCtx->pPos + 1;
						iSavedState = TextState;
						iState = EntityState;
						break;
					}

				break;
				}

			case AttributeTextState:
				{
				switch (chChar)
					{
					//	Handle embeded entities

					case '&':
						pCtx->sToken.Append(CString(pStartRun, pCtx->pPos - pStartRun));
						pStartRun = pCtx->pPos + 1;
						iSavedState = AttributeTextState;
						iState = EntityState;
						break;

					default:
						{
						if ((chChar == '"' && pCtx->iAttribQuote == tkQuote)
								|| (chChar == '\'' && pCtx->iAttribQuote == tkSingleQuote))
							{
							pCtx->iToken = tkText;
							pCtx->sToken.Append(CString(pStartRun, pCtx->pPos - pStartRun));
							pCtx->pPos--;
							bDone = true;
							break;
							}
						}
					}

				break;
				}

			case EntityState:
				{
				switch (chChar)
					{
					case ';':
						{
						CString sEntity(pStartRun, pCtx->pPos - pStartRun);

						bool bFound;
						pCtx->sToken.Append(ResolveEntity(pCtx, sEntity, &bFound));
						if (!bFound)
							{
							pCtx->iToken = tkError;
							pCtx->sError = strPatternSubst(CONSTLIT("Invalid entity: %s"), sEntity);
							bDone = true;
							break;
							}

						pStartRun = pCtx->pPos + 1;
						iState = iSavedState;
						break;
						}

					case ' ':
					case '>':
					case '<':
					case '\"':
					case '\'':
					case '\\':
					case '&':
						{
						CString sEntity(pStartRun, (pCtx->pPos + 1) - pStartRun);

						pCtx->iToken = tkError;
						pCtx->sError = strPatternSubst(CONSTLIT("Illegal character in entity: '%s' (or missing semi-colon)."), sEntity);
						bDone = true;
						break;
						}
					}

				break;
				}

			case CommentState:
				{
				if (chChar == '-' 
						&& (pCtx->pPos + 2 < pCtx->pEndPos)
						&& pCtx->pPos[1] == '-'
						&& pCtx->pPos[2] == '>')
					{
					if (iInitialState == StartDeclState)
						iState = iInitialState;
					else
						iState = ContentState;
					pCtx->pPos += 2;
					}

				break;
				}

			case CDATAState:
				{
				if (chChar == ']'
						&& (pCtx->pPos + 2 < pCtx->pEndPos)
						&& pCtx->pPos[1] == ']'
						&& pCtx->pPos[2] == '>')
					{
					pCtx->iToken = tkText;
					pCtx->sToken.Append(CString(pStartRun, pCtx->pPos - pStartRun));
					pCtx->pPos += 2;
					bDone = true;
					}

				break;
				}

			default:
				ASSERT(FALSE);
			}

		//	Count lines

		if (chChar == '\n')
			pCtx->iLine++;

		//	Next character

		pCtx->pPos++;
		}

	//	If we're not done, then we hit the end of the file

	if (!bDone)
		{
		if (bNoEOF)
			{
			pCtx->iToken = tkText;
			pCtx->sToken.Append(CString(pStartRun, pCtx->pPos - pStartRun));
			}
		else
			pCtx->iToken = tkEOF;
		}

	return pCtx->iToken;
	}

CString ResolveEntity (ParserCtx *pCtx, const CString &sName, bool *retbFound)

//	ResolveEntity
//
//	Resolves the entity from the parser table

	{
	*retbFound = true;
	CString sResult;

	//	Check to see if the name is one of the standard entities

	if (strEquals(sName, CONSTLIT("amp")))
		return CONSTLIT("&");
	else if (strEquals(sName, CONSTLIT("lt")))
		return CONSTLIT("<");
	else if (strEquals(sName, CONSTLIT("gt")))
		return CONSTLIT(">");
	else if (strEquals(sName, CONSTLIT("quot")))
		return CONSTLIT("\"");
	else if (strEquals(sName, CONSTLIT("apos")))
		return CONSTLIT("'");

	//	If the entity is a hex number, then this is a character

	char *pPos = sName.GetASCIIZPointer();
	if (*pPos == '#')
		{
		pPos++;
		if (*pPos == 'x' || *pPos == 'X')
			{
			*pPos++;
			char chChar = (char)strParseIntOfBase(pPos, 16, 0x20, NULL, NULL);
			return CString(&chChar, 1);
			}
		else
			{
			char chChar = (char)strParseInt(pPos, 0x20);
			return CString(&chChar, 1);
			}
		}

	//	Otherwise, it is a general attribute

	bool bFound;
	CString sValue = pCtx->LookupEntity(sName, &bFound);
	if (bFound)
		{
		//	Parse the value to resolve embedded entities

		ParserCtx SubCtx(pCtx, sValue);

		ParseToken(&SubCtx, ParseEntityState);
		if (SubCtx.iToken == tkText)
			sResult = SubCtx.sToken;
		else
			{
			bFound = false;
			sResult = sName;
			}
		}

	if (retbFound)
		*retbFound = bFound;

	return sResult;
	}

ALERROR CXMLElement::ParseEntityTable (IReadBlock *pStream, CExternalEntityTable *retEntityTable, CString *retsError)

//	ParseEntityTable
//
//	This function parses only the entity table

	{
	ALERROR error;

	//	Open the stream

	if (error = pStream->Open())
		{
		*retsError = CONSTLIT("unable to open XML stream");
		return error;
		}

	//	Initialize context

	ParserCtx Ctx(pStream, NULL);

	//	Parse the prologue

	if (error = ParsePrologue(&Ctx))
		goto Fail;

	//	Done

	pStream->Close();
	if (retEntityTable)
		retEntityTable->AddTable(Ctx.EntityTable);

	return NOERROR;

Fail:

	pStream->Close();
	*retsError = strPatternSubst(LITERAL("Line(%d): %s"), Ctx.iLine, Ctx.sError);
	return error;
	}

ALERROR CXMLElement::ParseRootElement (IReadBlock *pStream, CXMLElement **retpRoot, CExternalEntityTable *retEntityTable, CString *retsError)

//	ParseRootElement
//
//	Parses the entity definitions and the root element (but not the contents
//	of the root element).

	{
	ALERROR error;

	//	Open the stream

	if (error = pStream->Open())
		{
		*retsError = CONSTLIT("unable to open XML stream");
		return error;
		}

	//	Initialize context

	ParserCtx Ctx(pStream, NULL);

	//	Parse the prologue

	if (error = ParsePrologue(&Ctx))
		goto Fail;

	//	Next token must be an element open tag

	if (Ctx.iToken != tkTagOpen)
		{
		error = ERR_FAIL;
		Ctx.sError = LITERAL("root element expected");
		goto Fail;
		}

	//	Parse the root element

	Ctx.m_bParseRootElement = true;
	if (error = ParseElement(&Ctx, retpRoot))
		goto Fail;

	//	Done

	pStream->Close();
	if (retEntityTable)
		retEntityTable->AddTable(Ctx.EntityTable);

	return NOERROR;

Fail:

	pStream->Close();
	*retsError = strPatternSubst(LITERAL("Line(%d): %s"), Ctx.iLine, Ctx.sError);
	return error;
	}

ALERROR CXMLElement::ParseRootTag (IReadBlock *pStream, CString *retsTag)

//	ParseRootTag
//
//	This function parses only enough to determine the root tag, either by reading
//	as far as the first open tag or by getting the DOCTYPE.
//
//	This function is a hack to allow Transcendence to read the root tag for an
//	extension without loading the whole file.

	{
	ALERROR error;

	//	Open the stream

	if (error = pStream->Open())
		return error;

	//	Initialize context

	ParserCtx Ctx(pStream, NULL);
	Ctx.m_bParseRootTag = true;

	//	Parse the prologue

	if (error = ParsePrologue(&Ctx))
		{
		pStream->Close();
		return error;
		}

	if (!Ctx.m_sRootTag.IsBlank())
		{
		pStream->Close();
		*retsTag = Ctx.m_sRootTag;
		return NOERROR;
		}

	//	Next token must be an element open tag

	if (Ctx.iToken != tkTagOpen)
		{
		pStream->Close();
		return ERR_FAIL;
		}

	//	Parse the root element name

	if (ParseToken(&Ctx) != tkText)
		{
		pStream->Close();
		return ERR_FAIL;
		}

	*retsTag = Ctx.sToken;

	//	Done

	pStream->Close();

	return NOERROR;
	}
