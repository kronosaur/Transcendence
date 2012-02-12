//	XMLUtil.cpp
//
//	XML Utilities

#include "PreComp.h"

ALERROR XMLValidate (CString sData, CString *retsError, int *retiErrorPos)

//	XMLValidate
//
//	Make sure that the give data is valid XML

	{
	ALERROR error;
	char *pStart = sData.GetASCIIZPointer();
	char *pPos = pStart;
	CStringArray sTagStack;

	//	Parse the entire string

	while (*pPos != '\0')
		{
		//	Look for an open tag

		while (*pPos != '<' && *pPos != '\0')
			pPos++;

		//	Parse the tag

		if (*pPos == '<')
			{
			CString sTag;
			BOOL bClose;
			BOOL bEmpty;
			CSymbolTable Attributes(TRUE, FALSE);

			if (error = XMLParseTag(pPos, &sTag, &bClose, &bEmpty, &Attributes, &pPos))
				{
				*retsError = LITERAL("Unable to parse tag");
				*retiErrorPos = pPos - pStart;
				return error;
				}

			//	If this is the BR tag, then we just ignore it. The BR
			//	tag is a special tag that doesn't need a close

			if (strCompare(sTag, LITERAL("BR")) == 0)
				NULL;

			//	If this is an empty tag, just ignore it

			else if (bEmpty)
				NULL;

			//	If this is a close tag, make sure that it matches the
			//	last tag on our stack

			else if (bClose)
				{
				int iLastTag = sTagStack.GetCount() - 1;

				if (iLastTag < 0 || strCompare(sTag, sTagStack.GetStringValue(iLastTag)) != 0)
					{
					*retsError = strPatternSubst(LITERAL("</%s> found without corresponding open"), sTag);
					*retiErrorPos = pPos - pStart;
					return ERR_FAIL;
					}

				//	Remove the last tag now that we have a close

				sTagStack.RemoveString(iLastTag);
				}

			//	Otherwise, add this tag to the stack

			else
				{
				if (error = sTagStack.AppendString(sTag, NULL))
					return error;
				}
			}
		}

	//	If we've still got tags on our stack, then we never closed them

	if (sTagStack.GetCount() > 0)
		{
		*retsError = strPatternSubst(LITERAL("No close for <%s> tag"), sTagStack.GetStringValue(0));
		*retiErrorPos = pPos - pStart;
		return ERR_FAIL;
		}

	return NOERROR;
	}

ALERROR XMLParseTag (char *pStart, CString *retsTag, BOOL *retbClose, BOOL *retbEmpty, CSymbolTable *retpAttributes, char **retpEnd)

//	XMLParseTag
//
//	Parses an XML tag and returns its contents:
//
//	pStart: Position to start parsing at (start at '<')
//	retsTag: Returns the tag name
//	retbClose: TRUE if this is a close tag (</TAG>)
//	retbEmpty: TRUE if this is an empty tag (<TAG/>)
//	retpAttributes: Filled-in with tag attributes (must own the values)
//	retpEnd: Points to character after tag (after '>')

	{
	ALERROR error;
	char *pPos;
	char *pTagStart;
	CString sTag;
	BOOL bClose;
	BOOL bEmpty = FALSE;

	pPos = pStart;

	//	Must be open tag

	if (*pPos != '<')
		return ERR_FAIL;

	pPos++;

	//	Check to see if this is a close tag

	if (*pPos == '/')
		{
		bClose = TRUE;
		pPos++;
		}
	else
		bClose = FALSE;

	//	Parse the tag

	pTagStart = pPos;
	while (!strIsWhitespace(pPos) && *pPos != '>' && *pPos != '/' && *pPos != '\0')
		pPos++;

	if (pPos - pTagStart > 0 && *pPos != '\0')
		sTag = CString(pTagStart, pPos - pTagStart);
	else
		return ERR_FAIL;

	//	If this is a slash, then this must be an empty tag. If so, that's
	//	all we need to parse

	if (*pPos == '/')
		{
		pPos++;
		bEmpty = TRUE;
		}
	else
		{
		//	Skip any whitespace

		strParseWhitespace(pPos, &pPos);

		//	Check to see if we have any attributes

		while (*pPos != '>' && *pPos != '\0' && *pPos != '/')
			{
			CString sAttribute;
			CString *pValue;
			char *pStart;

			pStart = pPos;
			while (!strIsWhitespace(pPos) && *pPos != '=' && *pPos != '\0' && *pPos != '>')
				pPos++;

			sAttribute = CString(pStart, pPos - pStart);

			//	Skip whitespace

			strParseWhitespace(pPos, &pPos);

			//	This better be an equal sign

			if (*pPos != '=')
				return ERR_FAIL;

			//	Look for the value now

			pPos++;
			strParseWhitespace(pPos, &pPos);

			if (*pPos != '\"')
				return ERR_FAIL;

			pPos++;
			pStart = pPos;
			while (*pPos != '\"' && *pPos != '>' && *pPos != '\0')
				pPos++;

			if (*pPos != '\"')
				return ERR_FAIL;

			pValue = new CString(pStart, pPos - pStart);
			if (pValue == NULL)
				return ERR_MEMORY;

			pPos++;

			//	Add the attribute value pair to the symbol table

			if (error = retpAttributes->AddEntry(sAttribute, pValue))
				{
				delete pValue;
				return error;
				}

			//	Skip whitespace

			strParseWhitespace(pPos, &pPos);
			}
		}

	//	Is this an end tag?

	if (*pPos == '/')
		{
		bEmpty = TRUE;
		pPos++;
		}

	//	Skip ending brace

	if (*pPos != '>')
		return ERR_FAIL;

	pPos++;

	//	Done

	*retsTag = sTag;
	*retbClose = bClose;
	if (retbEmpty)
		*retbEmpty = bEmpty;
	if (retpEnd)
		*retpEnd = pPos;

	return NOERROR;
	}

ALERROR XMLParseContent (char *pStart, CString *retsText, char **retpEnd, CString *retsError)

//	XMLParseContent
//
//	Parses element content and translate &xxx; syntax

	{
	ALERROR error;
	char *pStartRun;
	char *pPos;
	CString sText;

	//	Initialize

	*retsError = LITERAL("");

	pStartRun = pStart;
	pPos = pStart;
	while (*pPos != '<' && *pPos != '\0')
		{
		//	Handle &

		if (*pPos == '&')
			{
			char *pEscStart;

			//	Add text so far

			if (pPos > pStartRun)
				if (error = sText.Append(CString(pStartRun, pPos - pStartRun)))
					return error;

			//	Skip &

			pPos++;
			pEscStart = pPos;

			//	Parse until we reach a semi-colon

			while (*pPos != ';' && *pPos != '\0')
				pPos++;

			if (*pPos == '\0')
				{
				*retsError = LITERAL("Invalid escape sequence");
				*retpEnd = pPos;
				return ERR_FAIL;
				}

			//	See what it is

			CString sEsc(pEscStart, pPos - pEscStart);
			if (strCompare(sEsc, LITERAL("amp")) == 0)
				error = sText.Append(LITERAL("&"));
			else if (strCompare(sEsc, LITERAL("lt")) == 0)
				error = sText.Append(LITERAL("<"));
			else if (strCompare(sEsc, LITERAL("gt")) == 0)
				error = sText.Append(LITERAL(">"));
			else if (strCompare(sEsc, LITERAL("apos")) == 0)
				error = sText.Append(LITERAL("'"));
			else if (strCompare(sEsc, LITERAL("quot")) == 0)
				error = sText.Append(LITERAL("\""));
			else
				{
				*retsError = strPatternSubst(LITERAL("Unknown escape sequence %s"), sEsc);
				*retpEnd = pPos;
				return ERR_FAIL;
				}

			if (error)
				return error;

			//	Done

			pPos++;
			pStartRun = pPos;
			}

		//	Skip carriage returns and line feeds and tabs

		else if (*pPos == '\r' || *pPos == '\n' || *pPos == '\t')
			{
			if (pPos > pStartRun)
				if (error = sText.Append(CString(pStartRun, pPos - pStartRun)))
					return error;

			pPos++;
			pStartRun = pPos;
			}
		else
			pPos++;
		}

	//	Add

	if (pPos > pStartRun)
		if (error = sText.Append(CString(pStartRun, pPos - pStartRun)))
			return error;

	//	Done

	*retsText = sText;
	*retpEnd = pPos;

	return NOERROR;
	}

