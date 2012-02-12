//	TextParser.cpp
//
//	Parses rich text

#include "PreComp.h"
#include "Tags.h"

enum TokenTypes
	{
	PlainText,
	BreakToken,
	TabToken,
	SectionToken,
	ListToken,
	CodeToken,
	QuoteToken,
	AnchorToken,
	EmphasisToken,
	EndStream,
	LinkListToken,
	EvalToken,
	UnknownTag,
	};

class CParseCtx
	{
	public:
		CParseCtx (CRTFormatter *pFmt, CRTFrame *pDestFrame, CString sText) : 
				pFormat(pFmt),
				pFrame(pDestFrame),
				pPos(sText.GetASCIIZPointer()),
				Attributes(TRUE, FALSE),
				iSectLevel(0)
			{ }

		CRTFormatter *pFormat;				//	Current format styles
		char *pPos;							//	Current parse position
		CString sToken;						//	Current token
		TokenTypes iToken;					//	Type of token
		BOOL bCloseTag;						//	TRUE if the token is a close tag
		BOOL bEmptyTag;						//	TRUE if the token is an empty tag
		CSymbolTable Attributes;			//	Current token attributes
		int iSectLevel;						//	Current section level

		CRTFrame *pFrame;					//	Current frame
		CString sError;						//	Parsing error
	};

typedef struct
	{
	char *pTag;
	TokenTypes iToken;
	} TAGTOKENMAP;

static TAGTOKENMAP g_TagTokenMap[] =
	{
		{ BREAK_TAG,		BreakToken },
		{ TAB_TAG,			TabToken },
		{ SECTION_TAG,		SectionToken },
		{ LIST_TAG,			ListToken },
		{ CODE_TAG,			CodeToken },
		{ QUOTE_TAG,		QuoteToken },
		{ ANCHOR_TAG,		AnchorToken },
		{ EMPHASIS_TAG,		EmphasisToken },
		{ LINKLIST_TAG,		LinkListToken },
		{ EVAL_TAG,			EvalToken },
		{ NULL,				UnknownTag },
	};

//	Forwards

static CString GetAttributeValue (CParseCtx &Ctx, CString sAttribute);
static ALERROR ParseCode (CParseCtx &Ctx);
static ALERROR ParseEmphasisText (CParseCtx &Ctx);
static ALERROR ParseEval (CParseCtx &Ctx);
static ALERROR ParseLinkList (CParseCtx &Ctx);
static ALERROR ParseLinkText (CParseCtx &Ctx);
static ALERROR ParseList (CParseCtx &Ctx);
static ALERROR ParseParagraph (CParseCtx &Ctx);
static ALERROR ParseParagraphSeq (CParseCtx &Ctx);
static ALERROR ParsePlainText (CParseCtx &Ctx);
static ALERROR ParseQuote (CParseCtx &Ctx);
static ALERROR ParseRun (CParseCtx &Ctx, BOOL *retbEmpty);
static ALERROR ParseSection (CParseCtx &Ctx);
static ALERROR ParseSectionSeq (CParseCtx &Ctx);
static ALERROR ParseSimpleText (CParseCtx &Ctx);
static ALERROR ParseSimpleTextSeq (CParseCtx &Ctx);
static ALERROR ParseText (CParseCtx &Ctx);
static ALERROR ParseToken (CParseCtx &Ctx);

CString GetAttributeValue (CParseCtx &Ctx, CString sAttribute)

//	GetAttributeValue
//
//	Returns the attribute value (or blank if not found)

	{
	ALERROR error;
	CString sValue;
	CString sParsedValue;
	CObject *pObj;

	//	Get the raw value

	error = Ctx.Attributes.Lookup(sAttribute, &pObj);
	if (error == NOERROR)
		{
		CString *pValue = (CString *)pObj;
		sValue = *pValue;
		}

	//	Parse for escape codes

#if 0
	CString sError;
	char *pEndPos;
	error = XMLParseContent(sValue.GetASCIIZPointer(),
			&sParsedValue,
			&pEndPos,
			&sError);
	if (error)
		sParsedValue = strPatternSubst(CONSTLIT("[%s]"), sError);
#else
	sParsedValue = sValue;
#endif

	return sParsedValue;
	}

ALERROR ParseTextBlock (CString sText, CRTFormatter *pFormat, CRTFrame *pFrame, CString *retsError)

//	ParseTextBlock
//
//	Parses the rich text formatted text into the given frame

	{
	ALERROR error;

	CParseCtx Ctx(pFormat, pFrame, sText);

	//	Parse the first token

	if (error = ParseToken(Ctx))
		{
		*retsError = Ctx.sError;
		return error;
		}

	//	Do it

	if (error = ParseText(Ctx))
		{
		*retsError = Ctx.sError;
		return error;
		}

	//	If we've ended with an unknown token, report that
	//	as an error

	if (Ctx.iToken == UnknownTag)
		{
		*retsError = strPatternSubst(LITERAL("Unknown tag: %s"), Ctx.sToken);
		return ERR_FAIL;
		}

	return NOERROR;
	}

ALERROR ParseCode (CParseCtx &Ctx)

//	ParseCode
//
//	Parse a code block

	{
	return NOERROR;
	}

ALERROR ParseEmphasisText (CParseCtx &Ctx)

//	ParseEmphasisText
//
//	emphasis-text :: <I> plain-text </I>

	{
	ALERROR error;

	if (Ctx.iToken == EmphasisToken && !Ctx.bCloseTag)
		{
		CRTTextFormat *pOldFormat;

		//	Remember the previous text format

		pOldFormat = Ctx.pFrame->GetCurrentTextFormat();

		//	Set the new format

		Ctx.pFrame->SetCurrentTextFormat(Ctx.pFormat->ApplyTextFormat(pOldFormat, LITERAL("Emphasis")));

		//	Next

		if (error = ParseToken(Ctx))
			return error;

		//	Parse some plain text

		if (error = ParsePlainText(Ctx))
			return error;

		//	We'd better be left with a close tag

		if (Ctx.iToken != EmphasisToken || !Ctx.bCloseTag)
			{
			Ctx.sError = LITERAL("</I> expected");
			return ERR_FAIL;
			}

		//	Reset the text format

		Ctx.pFrame->SetCurrentTextFormat(pOldFormat);

		//	Next token

		if (error = ParseToken(Ctx))
			return error;
		}

	return NOERROR;
	}

ALERROR ParseEval (CParseCtx &Ctx)

//	ParseEval
//
//	Parses an eval block and then parses the result

	{
	ALERROR error;

	if (Ctx.iToken == EvalToken && !Ctx.bCloseTag)
		{
		CString sResult;

		//	Get the CodeChain expression

		if (error = ParseToken(Ctx))
			return error;

		//	Make sure this is text

		if (Ctx.iToken != PlainText)
			{
			Ctx.sError = LITERAL("CodeChain expression expected");
			return ERR_FAIL;
			}

		//	Evaluate the code

		sResult = Ctx.pFormat->GetCore()->ExecuteCommand(Ctx.sToken);

		//	The next token better be a close tag

		if (error = ParseToken(Ctx))
			return error;

		if (Ctx.iToken != EvalToken || !Ctx.bCloseTag)
			{
			Ctx.sError = LITERAL("</EVAL> expected");
			return ERR_FAIL;
			}

		//	Now parse the results of the formula

		if (error = ParseTextBlock(sResult, Ctx.pFormat, Ctx.pFrame, &Ctx.sError))
			return error;

		//	Prepare to parse the next token

		if (error = ParseToken(Ctx))
			return error;
		}

	return NOERROR;
	}

ALERROR ParseLinkList (CParseCtx &Ctx)

//	ParseLinkList
//
//	link-list :: <LINKS category="xxxx"/>

	{
	ALERROR error;

	if (Ctx.iToken == LinkListToken)
		{
		int i;
		CString sCategory;
		CSymbolTable List(TRUE, FALSE);
		CRTTextFormat *pOldTextFmt;
		CRTParagraphFormat *pOldParagraphFmt;

		//	Get the category

		sCategory = GetAttributeValue(Ctx, CONSTLIT(CATEGORY_ATTRIBUTE));

		//	Get a list of all the links for the given category

		if (error = Ctx.pFormat->GetCore()->GetCategoryLinks(sCategory, &List))
			{
			Ctx.sError = strPatternSubst(LITERAL("Unable to load category \"%s\""), sCategory);
			return ERR_FAIL;
			}

		//	Set the format to link format

		pOldTextFmt = Ctx.pFrame->GetCurrentTextFormat();
		pOldParagraphFmt = Ctx.pFrame->GetCurrentParagraphFormat();
		Ctx.pFrame->SetCurrentTextFormat(Ctx.pFormat->ApplyTextFormat(pOldTextFmt, LITERAL("Index")));
		Ctx.pFrame->SetCurrentParagraphFormat(Ctx.pFormat->ApplyParagraphFormat(pOldParagraphFmt, LITERAL("Index")));

		//	Next paragraph is the first in a block

		Ctx.pFrame->SetFirstOfBlock();

		//	Loop over all links and add them

		for (i = 0; i < List.GetCount(); i++)
			{
			CString sTitle;
			CString sLink = *(CString *)List.GetValue(i);
			char *pEndPos;
			CString sError;

			//	Process the title

			if (error = XMLParseContent(List.GetKey(i).GetASCIIZPointer(),
					&sTitle,
					&pEndPos,
					&sError))
				{
				Ctx.pFrame->AddText(sError);
				return error;
				}

			//	Set the link

			if (error = Ctx.pFrame->SetCurrentLink(sLink))
				return error;

			//	Add the text

			if (error = Ctx.pFrame->AddText(sTitle))
				return error;

			if (error = Ctx.pFrame->NewParagraph())
				return error;

			//	Done

			if (error = Ctx.pFrame->SetCurrentLink(LITERAL("")))
				return error;
			}

		//	Restore format

		Ctx.pFrame->SetCurrentTextFormat(pOldTextFmt);
		Ctx.pFrame->SetCurrentParagraphFormat(pOldParagraphFmt);

		//	This is the last of the block

		Ctx.pFrame->SetLastOfBlock();

		//	This had better be an empty token

		if (!Ctx.bEmptyTag)
			{
			Ctx.sError = LITERAL("<LINKS> must be an empty tag");
			return ERR_FAIL;
			}

		//	Parse token

		if (error = ParseToken(Ctx))
			return error;
		}

	return NOERROR;
	}

ALERROR ParseLinkText (CParseCtx &Ctx)

//	ParseLinkText
//
//	link-text :: <A> plain-text </A>

	{
	ALERROR error;

	if (Ctx.iToken == AnchorToken && !Ctx.bCloseTag)
		{
		CString sLink;
		CObject *pObj;
		CRTTextFormat *pOldFormat;

		//	Get the UNID

		error = Ctx.Attributes.Lookup(LITERAL(HREF_ATTRIBUTE), &pObj);
		if (error == NOERROR)
			{
			CString *pValue = (CString *)pObj;
			sLink = *pValue;
			}

		//	Set the current link

		if (error = Ctx.pFrame->SetCurrentLink(sLink))
			return error;

		//	Remember the previous text format

		pOldFormat = Ctx.pFrame->GetCurrentTextFormat();

		//	Set the new format

		Ctx.pFrame->SetCurrentTextFormat(Ctx.pFormat->ApplyTextFormat(pOldFormat, LITERAL("Link")));

		//	Next

		if (error = ParseToken(Ctx))
			return error;

		//	Parse some plain text

		if (error = ParsePlainText(Ctx))
			return error;

		//	We'd better be left with a close anchor tag

		if (Ctx.iToken != AnchorToken || !Ctx.bCloseTag)
			{
			Ctx.sError = LITERAL("</A> expected");
			return ERR_FAIL;
			}

		//	Reset the text format

		Ctx.pFrame->SetCurrentLink(LITERAL(""));
		Ctx.pFrame->SetCurrentTextFormat(pOldFormat);

		//	Next token

		if (error = ParseToken(Ctx))
			return error;
		}

	return NOERROR;
	}

ALERROR ParseList (CParseCtx &Ctx)

//	ParseList
//
//	Parse a list

	{
	ALERROR error;

	if (Ctx.iToken == ListToken && !Ctx.bCloseTag)
		{
		CRTTextFormat *pOldTextFmt;
		CRTParagraphFormat *pOldParagraphFmt;
		int iListType = 1;
		CObject *pObj;

		//	Get the bullet type

		error = Ctx.Attributes.Lookup(LITERAL(LISTTYPE_ATTRIBUTE), &pObj);
		if (error == NOERROR)
			{
			CString *pValue = (CString *)pObj;
			iListType = strToInt(*pValue, 1, NULL);
			}

		//	Remember the previous format

		pOldTextFmt = Ctx.pFrame->GetCurrentTextFormat();
		pOldParagraphFmt = Ctx.pFrame->GetCurrentParagraphFormat();

		//	Set the new format

		CString sListFormat = strPatternSubst(LITERAL("List %d"), iListType);
		Ctx.pFrame->SetCurrentTextFormat(Ctx.pFormat->ApplyTextFormat(pOldTextFmt, sListFormat));
		Ctx.pFrame->SetCurrentParagraphFormat(Ctx.pFormat->ApplyParagraphFormat(pOldParagraphFmt, sListFormat));

		//	Next paragraph is the first in a block

		Ctx.pFrame->SetFirstOfBlock();

		//	Next

		if (error = ParseToken(Ctx))
			return error;

		//	Parse some text

		if (error = ParseSimpleTextSeq(Ctx))
			return error;

		//	We'd better be left with a close tag

		if (Ctx.iToken != ListToken || !Ctx.bCloseTag)
			{
			Ctx.sError = LITERAL("</LIST> expected");
			return ERR_FAIL;
			}

		//	Reset the text format

		Ctx.pFrame->SetCurrentTextFormat(pOldTextFmt);
		Ctx.pFrame->SetCurrentParagraphFormat(pOldParagraphFmt);

		//	This is the last of the block

		Ctx.pFrame->SetLastOfBlock();

		//	Next token

		if (error = ParseToken(Ctx))
			return error;
		}

	return NOERROR;
	}

ALERROR ParseParagraph (CParseCtx &Ctx)

//	ParseParagraph
//
//	paragraph :: simple-text-seq | list-block | code-block | quote-block

	{
	ALERROR error;

	//	Parse various kinds of paragraphs

	if (Ctx.iToken == ListToken)
		{
		if (error = ParseList(Ctx))
			return error;
		}
	else if (Ctx.iToken == LinkListToken)
		{
		if (error = ParseLinkList(Ctx))
			return error;
		}
	else if (Ctx.iToken == CodeToken)
		{
		if (error = ParseCode(Ctx))
			return error;
		}
	else if (Ctx.iToken == QuoteToken)
		{
		if (error = ParseQuote(Ctx))
			return error;
		}

	//	Else, parse a text sequence

	else
		{
		if (error = ParseSimpleTextSeq(Ctx))
			return error;
		}

	return NOERROR;
	}

ALERROR ParseParagraphSeq (CParseCtx &Ctx)

//	ParseParagraphSeq
//
//	paragraph-seq :: paragraph [ paragraph-seq ]

	{
	ALERROR error;

	//	Keep parsing while the token is a paragraph token

	while (TRUE)
		{
		switch (Ctx.iToken)
			{
			case PlainText:
			case BreakToken:
			case TabToken:
			case ListToken:
			case CodeToken:
			case QuoteToken:
			case AnchorToken:
			case EmphasisToken:
			case LinkListToken:
			case EvalToken:
				{
				if (error = ParseParagraph(Ctx))
					return error;

				break;
				}

			default:
				return NOERROR;
			}
		}

	return NOERROR;
	}

ALERROR ParsePlainText (CParseCtx &Ctx)

//	ParsePlainText
//
//	Parse plain text

	{
	ALERROR error;

	//	If this is plain text, add it

	if (Ctx.iToken == PlainText)
		{
		if (error = Ctx.pFrame->AddText(Ctx.sToken))
			return error;

		//	Next

		if (error = ParseToken(Ctx))
			return error;
		}

	return NOERROR;
	}

ALERROR ParseQuote (CParseCtx &Ctx)

//	ParseQuote
//
//	Parse a quote block

	{
	return NOERROR;
	}

ALERROR ParseRun (CParseCtx &Ctx, BOOL *retbEmpty)

//	ParseRun
//
//	run :: plain-text | emphasis-text | link-text | tab

	{
	ALERROR error;
	BOOL bEmpty = FALSE;

	if (Ctx.iToken == PlainText)
		{
		if (error = ParsePlainText(Ctx))
			return error;
		}
	else if (Ctx.iToken == EmphasisToken)
		{
		if (error = ParseEmphasisText(Ctx))
			return error;
		}
	else if (Ctx.iToken == AnchorToken)
		{
		if (error = ParseLinkText(Ctx))
			return error;
		}
	else if (Ctx.iToken == TabToken)
		{
		if (error = Ctx.pFrame->AddTab())
			return error;

		if (error = ParseToken(Ctx))
			return error;
		}
	else if (Ctx.iToken == EvalToken)
		{
		if (error = ParseEval(Ctx))
			return error;
		}
	//	Nothing to parse
	else
		bEmpty = TRUE;

	//	Done

	if (retbEmpty)
		*retbEmpty = bEmpty;

	return NOERROR;
	}

ALERROR ParseSection (CParseCtx &Ctx)

//	ParseSection
//
//	section :: <SECT> simple-text <BR> paragraph-seq [ section-seq ] </SECT>

	{
	ALERROR error;

	//	The token had better be an open section tag

	if (Ctx.iToken == SectionToken && !Ctx.bCloseTag)
		{
		CString sFormat;
		CRTTextFormat *pOldFormat;
		CRTParagraphFormat *pOldParagraph;

		//	Next

		if (error = ParseToken(Ctx))
			return error;

		//	Start a new section

		Ctx.iSectLevel++;
		sFormat = strPatternSubst(LITERAL("Heading %d"), Ctx.iSectLevel);

		//	Set the paragraph and text attributes

		pOldFormat = Ctx.pFrame->GetCurrentTextFormat();
		Ctx.pFrame->SetCurrentTextFormat(Ctx.pFormat->ApplyTextFormat(pOldFormat, sFormat));

		pOldParagraph = Ctx.pFrame->GetCurrentParagraphFormat();
		Ctx.pFrame->SetCurrentParagraphFormat(Ctx.pFormat->ApplyParagraphFormat(pOldParagraph, sFormat));

		//	Tell the frame that this is a section

		Ctx.pFrame->SetParagraphHeading();

		//	Parse some simple text

		if (error = ParseSimpleText(Ctx))
			return error;

		//	This better be a break token

		if (Ctx.iToken != BreakToken)
			{
			Ctx.sError = LITERAL("Section title break expected");
			return ERR_FAIL;
			}

		if (error = ParseToken(Ctx))
			return error;

		//	Back to the old formats

		Ctx.pFrame->SetCurrentTextFormat(pOldFormat);
		Ctx.pFrame->SetCurrentParagraphFormat(pOldParagraph);
		Ctx.pFrame->ClearParagraphHeading();

		//	Parse a paragraph sequence

		if (error = ParseParagraphSeq(Ctx))
			return error;

		//	If this is another open section, then we recurse

		if (Ctx.iToken == SectionToken && !Ctx.bCloseTag)
			{
			if (error = ParseSectionSeq(Ctx))
				return error;
			}

		//	This better be a close section

		if (Ctx.iToken != SectionToken || !Ctx.bCloseTag)
			{
			Ctx.sError = LITERAL("</SECT> expected");
			return ERR_FAIL;
			}

		//	Next

		if (error = ParseToken(Ctx))
			return error;

		//	Done

		Ctx.iSectLevel--;
		}

	return NOERROR;
	}

ALERROR ParseSectionSeq (CParseCtx &Ctx)

//	ParseSectionSeq
//
//	section-seq :: section [ section-seq ]

	{
	ALERROR error;

	//	Keep parsing while the token is a section token

	while (Ctx.iToken == SectionToken && !Ctx.bCloseTag)
		{
		if (error = ParseSection(Ctx))
			return error;
		}

	return NOERROR;
	}

ALERROR ParseSimpleText (CParseCtx &Ctx)

//	ParseSimpleText
//
//	simple-text :: run [ simple-text ]

	{
	ALERROR error;
	BOOL bDone = FALSE;

	//	Keep parsing while the token is a text token

	while (!bDone)
		{
		if (error = ParseRun(Ctx, &bDone))
			return error;
		}

	return NOERROR;
	}

ALERROR ParseSimpleTextSeq (CParseCtx &Ctx)

//	ParseSimpleTextSeq
//
//	simple-text-seq :: simple-text [ <BR> simple-text-seq ]

	{
	ALERROR error;

	while (TRUE)
		{
		if (error = ParseSimpleText(Ctx))
			return error;

		//	If we don't have a break token, then we're done

		if (Ctx.iToken == BreakToken)
			{
			if (error = Ctx.pFrame->NewParagraph())
				return error;

			//	Next

			if (error = ParseToken(Ctx))
				return error;
			}
		else
			break;
		}

	return NOERROR;
	}

ALERROR ParseText (CParseCtx &Ctx)

//	ParseText
//
//	text :: paragraph-seq [ section-seq ]

	{
	ALERROR error;

	//	If this is something other than a section token, then parse
	//	it as a paragraph sequence

	if (Ctx.iToken != SectionToken)
		{
		if (error = ParseParagraphSeq(Ctx))
			return error;
		}

	//	If the token is a section tag then we parse a section sequence

	if (Ctx.iToken == SectionToken)
		{
		//	Parse a section sequence

		if (error = ParseSectionSeq(Ctx))
			return error;
		}

	return NOERROR;
	}

ALERROR ParseToken (CParseCtx &Ctx)

//	ParseToken
//
//	Parses a token
//
//	This method initializes Ctx.iToken to a valid token type.

	{
	ALERROR error;

	//	Skip any line feeds and tabs

	while (*Ctx.pPos == '\r' || *Ctx.pPos == '\n' || *Ctx.pPos == '\t')
		Ctx.pPos++;

	//	If we're at the end, then this is the end of the stream

	if (*Ctx.pPos == '\0')
		Ctx.iToken = EndStream;

	//	If this is a tag then figure out which tag it is

	else if (*Ctx.pPos == '<')
		{
		TAGTOKENMAP *pMap;

		//	Clean-up previous attributes

		Ctx.Attributes.RemoveAll();

		//	Parse

		if (error = XMLParseTag(Ctx.pPos, &Ctx.sToken, &Ctx.bCloseTag, &Ctx.bEmptyTag, &Ctx.Attributes, &Ctx.pPos))
			return error;

		//	Figure out which token this is

		pMap = g_TagTokenMap;
		while (pMap->pTag)
			{
			if (strCompare(Ctx.sToken, LITERAL(pMap->pTag)) == 0)
				break;

			pMap++;
			}

		Ctx.iToken = pMap->iToken;
		}

	//	Otherwise, this must be plain text. Keep parsing until we reach
	//	a tag or the end of the stream

	else
		{
		if (error = XMLParseContent(Ctx.pPos, &Ctx.sToken, &Ctx.pPos, &Ctx.sError))
			return error;

		Ctx.iToken = PlainText;
		}

	return NOERROR;
	}

