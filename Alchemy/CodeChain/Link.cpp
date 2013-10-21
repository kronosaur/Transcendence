//	Link.cpp
//
//	Parsing routines

#include <windows.h>
#include "Kernel.h"
#include "KernelObjID.h"
#include "CodeChain.h"

#define SYMBOL_QUOTE					'\''	//	Quote symbol
#define SYMBOL_OPENPAREN				'('		//	Open list
#define SYMBOL_CLOSEPAREN				')'		//	Close list
#define SYMBOL_OPENQUOTE				'\"'	//	Open quote
#define SYMBOL_CLOSEQUOTE				'\"'	//	Close quote
#define SYMBOL_BACKSLASH				'\\'	//	Back-slash
#define SYMBOL_OPENBRACE				'{'		//	Open literal structure
#define SYMBOL_CLOSEBRACE				'}'		//	Close structure
#define SYMBOL_COLON					':'		//	Key/value separator

ICCItem *CCodeChain::Link (const CString &sString, int iOffset, int *retiLinked, int *ioiCurLine)

//	Link
//
//	Parses the given string and converts it into a linked
//	chain of items

	{
	char *pStart;
	char *pPos;
	ICCItem *pResult = NULL;
	int iCurLine = (ioiCurLine ? *ioiCurLine : 1);

	pStart = sString.GetPointer() + iOffset;
	pPos = pStart;

	//	Skip any whitespace

	pPos = SkipWhiteSpace(pPos, &iCurLine);

	//	If we've reached the end, then we have
	//	nothing

	if (*pPos == '\0')
		pResult = CreateNil();

	//	If we've got a literal quote, then remember it

	else if (*pPos == SYMBOL_QUOTE)
		{
		int iLinked;

		pPos++;

		pResult = Link(sString, iOffset + (pPos - pStart), &iLinked, &iCurLine);
		if (pResult->IsError())
			return pResult;

		pPos += iLinked;

		//	Make it a literal

		pResult->SetQuoted();
		}

	//	If we've got an open paren then we start a list

	else if (*pPos == SYMBOL_OPENPAREN)
		{
		ICCItem *pNew = CreateLinkedList();
		if (pNew->IsError())
			return pNew;

		CCLinkedList *pList = dynamic_cast<CCLinkedList *>(pNew);

		//	Keep reading until we find the end

		pPos++;

		//	If the list is empty, then there's nothing to do

		pPos = SkipWhiteSpace(pPos, &iCurLine);
		if (*pPos == SYMBOL_CLOSEPAREN)
			{
			pList->Discard(this);
			pResult = CreateNil();
			pPos++;
			}

		//	Get all the items in the list

		else
			{
			while (*pPos != SYMBOL_CLOSEPAREN && *pPos != '\0')
				{
				ICCItem *pItem;
				int iLinked;

				pItem = Link(sString, iOffset + (pPos - pStart), &iLinked, &iCurLine);
				if (pItem->IsError())
					return pItem;

				//	Add the item to the list

				pList->Append(this, pItem, NULL);
				pItem->Discard(this);

				//	Move the position

				pPos += iLinked;

				//	Skip whitespace

				pPos = SkipWhiteSpace(pPos, &iCurLine);
				}

			//	If we have a close paren then we're done; Otherwise we've
			//	got an error of some kind

			if (*pPos == SYMBOL_CLOSEPAREN)
				{
				pPos++;
				pResult = pList;
				}
			else
				{
				pList->Discard(this);
				pResult = CreateParseError(iCurLine, CONSTLIT("Mismatched open parenthesis"));
				}
			}
		}

	//	If this is an open brace then we've got a literal structure

	else if (*pPos == SYMBOL_OPENBRACE)
		{
		ICCItem *pNew = CreateSymbolTable();
		if (pNew->IsError())
			return pNew;

		CCSymbolTable *pTable = dynamic_cast<CCSymbolTable *>(pNew);

		//	Always literal

		pTable->SetQuoted();

		//	Keep reading until we find the end

		pPos++;

		//	If the list is empty, then there's nothing to do

		pPos = SkipWhiteSpace(pPos, &iCurLine);
		if (*pPos == SYMBOL_CLOSEBRACE)
			{
			pResult = pTable;
			pPos++;
			}

		//	Get all the items in the list

		else
			{
			while (*pPos != SYMBOL_CLOSEBRACE && *pPos != '\0')
				{
				//	Get the key

				ICCItem *pKey;
				int iLinked;

				pKey = Link(sString, iOffset + (pPos - pStart), &iLinked, &iCurLine);
				if (pKey->IsError())
					{
					pTable->Discard(this);
					return pKey;
					}

				//	Move the position and read a colon

				pPos += iLinked;
				pPos = SkipWhiteSpace(pPos, &iCurLine);
				if (*pPos != SYMBOL_COLON)
					{
					pKey->Discard(this);
					pTable->Discard(this);
					return CreateParseError(iCurLine, CONSTLIT("Struct value not found."));
					}

				pPos++;

				//	Get the value

				ICCItem *pValue;

				pValue = Link(sString, iOffset + (pPos - pStart), &iLinked, &iCurLine);
				if (pValue->IsError())
					{
					pKey->Discard(this);
					pTable->Discard(this);
					return pValue;
					}

				//	Move the position

				pPos += iLinked;

				//	Add the item to the table

				pResult = pTable->AddEntry(this, pKey, pValue);
				pKey->Discard(this);
				pValue->Discard(this);
				if (pResult->IsError())
					{
					pTable->Discard(this);
					return pResult;
					}

				//	Skip whitespace because otherwise we won't know whether we
				//	hit the end brace.

				pPos = SkipWhiteSpace(pPos, &iCurLine);
				}

			//	If we have a close paren then we're done; Otherwise we've
			//	got an error of some kind

			if (*pPos == SYMBOL_CLOSEBRACE)
				{
				pPos++;
				pResult = pTable;
				}
			else
				{
				pTable->Discard(this);
				pResult = CreateParseError(iCurLine, CONSTLIT("Mismatched open brace"));
				}
			}
		}

	//	If this is an open quote, then read everything until
	//	the close quote

	else if (*pPos == SYMBOL_OPENQUOTE)
		{
		//	Parse the string, until the end quote, parsing escape codes

		char *pStartFragment = NULL;
		CString sResultString;

		bool bDone = false;
		while (!bDone)
			{
			pPos++;

			switch (*pPos)
				{
				case SYMBOL_CLOSEQUOTE:
				case '\0':
					{
					if (pStartFragment)
						{
						sResultString.Append(CString(pStartFragment, pPos - pStartFragment));
						pStartFragment = NULL;
						}

					bDone = true;
					break;
					}

				case SYMBOL_BACKSLASH:
					{
					if (pStartFragment)
						{
						sResultString.Append(CString(pStartFragment, pPos - pStartFragment));
						pStartFragment = NULL;
						}

					pPos++;
					if (*pPos == '\0')
						bDone = true;
					else if (*pPos == 'n')
						sResultString.Append(CString("\n", 1));
					else if (*pPos == 'r')
						sResultString.Append(CString("\r", 1));
					else if (*pPos == 't')
						sResultString.Append(CString("\t", 1));
					else if (*pPos == '0')
						sResultString.Append(CString("\0", 1));
					else if (*pPos == 'x' || *pPos == 'X')
						{
						pPos++;
						int iFirstDigit = strGetHexDigit(pPos);
						pPos++;
						int iSecondDigit = 0;
						if (*pPos == '\0')
							bDone = true;
						else
							iSecondDigit = strGetHexDigit(pPos);

						char chChar = (char)(16 * iFirstDigit + iSecondDigit);
						sResultString.Append(CString(&chChar, 1));
						}
					else
						sResultString.Append(CString(pPos, 1));

					break;
					}

				default:
					{
					if (pStartFragment == NULL)
						pStartFragment = pPos;

					break;
					}
				}
			}

		//	If we found the close, then create a string; otherwise,
		//	it is an error

		if (*pPos == SYMBOL_CLOSEQUOTE)
			{
			pResult = CreateString(sResultString);

			//	Always a literal

			pResult->SetQuoted();

			//	Skip past quote

			pPos++;
			}
		else
			pResult = CreateParseError(iCurLine, CONSTLIT("Mismatched quote"));
		}

	//	If this is a close paren, then it is an error

	else if (*pPos == SYMBOL_CLOSEPAREN)
		pResult = CreateParseError(iCurLine, CONSTLIT("Mismatched close parenthesis"));

	//	If this is a close brace, then it is an error

	else if (*pPos == SYMBOL_CLOSEBRACE)
		pResult = CreateParseError(iCurLine, CONSTLIT("Mismatched close brace"));

	//	Colons cannot appear alone

	else if (*pPos == SYMBOL_COLON)
		pResult = CreateParseError(iCurLine, CONSTLIT("':' character must appear inside quotes or in a struct definition."));

	//	Otherwise this is an string of some sort

	else
		{
		char *pStartString;
		CString sIdentifier;
		int iInt;
		bool bNotInteger;

		pStartString = pPos;

		//	Look for whitespace

    	while (*pPos != '\0'
        		&& *pPos != ' ' && *pPos != '\n' && *pPos != '\r' && *pPos != '\t'
            	&& *pPos != SYMBOL_OPENPAREN
				&& *pPos != SYMBOL_CLOSEPAREN
				&& *pPos != SYMBOL_OPENQUOTE
				&& *pPos != SYMBOL_CLOSEQUOTE
				&& *pPos != SYMBOL_OPENBRACE
				&& *pPos != SYMBOL_CLOSEBRACE
				&& *pPos != SYMBOL_COLON
				&& *pPos != SYMBOL_QUOTE
				&& *pPos != ';')
        	pPos++;

		//	If we did not advance, then we clearly hit an error

		if (pStartString == pPos)
			pResult = CreateParseError(iCurLine, strPatternSubst(CONSTLIT("Unexpected character: %s"), CString(pPos, 1)));

		//	If we ended in a quote then that's a bug

		else if (*pPos == SYMBOL_QUOTE)
			pResult = CreateParseError(iCurLine, strPatternSubst(CONSTLIT("Identifiers must not use single quote characters: %s"), 
					strSubString(sString, iOffset + (pStartString - pStart), (pPos + 1 - pStartString))));

		//	Otherwise, get the identifier

		else
			{
			//	Create a string from the portion

			sIdentifier = strSubString(sString, iOffset + (pStartString - pStart), (pPos - pStartString));

			//	Check to see if this is a reserved identifier

			if (strCompareAbsolute(sIdentifier, CONSTLIT("Nil")) == 0)
				pResult = CreateNil();
			else if (strCompareAbsolute(sIdentifier, CONSTLIT("True")) == 0)
				pResult = CreateTrue();
			else
				{
				//	If this is an integer, create an integer; otherwise
				//	create a string

				iInt = strToInt(sIdentifier, 0, &bNotInteger);
				if (bNotInteger)
					pResult = CreateString(sIdentifier);
				else
					pResult = CreateInteger(iInt);
				}
			}
		}

	//	Return the result and the number of characters
	//	that we read

	if (retiLinked)
		*retiLinked = (pPos - pStart);

	if (ioiCurLine)
		*ioiCurLine = iCurLine;

	return pResult;
	}

ICCItem *CCodeChain::CreateParseError (int iLine, const CString &sError)

//	CreateParseError
//
//	Utility for creating a parse error

	{
	return CreateError(strPatternSubst(CONSTLIT("Line %d: %s"), iLine, sError));
	}

char *CCodeChain::SkipWhiteSpace (char *pPos, int *ioiLine)

//	SkipWhiteSpace
//
//	Skips white space and comments when parsing

	{
    BOOL bDone = FALSE;
    BOOL bInComment = FALSE;
    
    while (!bDone)
        {
    	/*	If we're inside a comment, keep going until we find the end of the line;
        	Otherwise, just skip white space */
        
        if (*pPos == '\0')
            bDone = TRUE;
        else if (bInComment)
            {
			if (*pPos == '\n')
				{
				*ioiLine += 1;
				bInComment = FALSE;
				}
        	else if (*pPos == '\r')
                bInComment = FALSE;

            pPos++;
        	}
        else
            {
        	if (*pPos == ';')
                {
                bInComment = TRUE;
                pPos++;
            	}
			else if (*pPos == '\n')
				{
				*ioiLine += 1;
				pPos++;
				}
            else if (*pPos == ' ' || *pPos == '\r' || *pPos == '\t')
                pPos++;
            else
                bDone = TRUE;
        	}
    	}
    
    return pPos;
    }

CString CCodeChain::Unlink (ICCItem *pItem)

//	Unlink
//
//	Converts from a linked chain of items to a single
//	string.

	{
	return pItem->Print(this);
	}
