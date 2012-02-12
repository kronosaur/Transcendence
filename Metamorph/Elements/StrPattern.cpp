//	StrPattern.cpp
//
//	String pattern package
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.
//
//	Pattern format:
//
//	%type
//
//	where type is one of the following:
//
//	%	Evaluates to a single percent sign
//
//	s	Argument is a CString. The string is substituted
//
//	d	Argument is a signed 32-bit integer. The number is substituted
//
//	p	If the last integer argument not 1, then 's' is substituted.
//		This is used to pluralize words in the English language.

#include "Elements.h"

CString strPattern (char *pPattern, LPVOID *pArgs)

//	strPattern
//
//	Returns a string with a pattern substitution

	{
	CWORM Stream;
	char *pPos = pPattern;
	char *pRunStart;
	int iRunLength;
	int iLastInteger = 1;

	//	Start

	pRunStart = pPos;
	iRunLength = 0;

	//	Loop

	while (*pPos != '\0')
		{
		if (*pPos == '%')
			{
			//	Save out what we've got til now

			if (iRunLength > 0)
				Stream.Write(pRunStart, iRunLength);

			//	Check the actual pattern code

			pPos++;
			if (*pPos != '\0')
				{
				if (*pPos == 's')
					{
					CString *pParam = (CString *)pArgs;

					Stream.Write(pParam->GetASCIIZ(), pParam->GetLength());

					pArgs += AlignUp(sizeof(CString), sizeof(LPVOID)) / sizeof(LPVOID);
					pPos++;
					}
				else if (*pPos == 'd')
					{
					int *pInt = (int *)pArgs;
					CString sNew;

					sNew = strFromInt(*pInt, true);
					Stream.Write(sNew.GetASCIIZ(), sNew.GetLength());

					//	Remember the last integer

					iLastInteger = *pInt;

					//	Next

					pArgs++;
					pPos++;
					}
				else if (*pPos == 'p')
					{
					if (iLastInteger != 1)
						Stream.Write("s", 1);

					pPos++;
					}
				else if (*pPos == '%')
					{
					Stream.Write("%", 1);

					pPos++;
					}
				}

			pRunStart = pPos;
			iRunLength = 0;
			}
		else
			{
			iRunLength++;
			pPos++;
			}
		}

	//	Save out the last run

	if (iRunLength > 0)
		Stream.Write(pRunStart, iRunLength);

	//	Convert the stream to a string

	CString sResult(Stream.GetPointer(), Stream.GetLength());
	return sResult;
	}

CString strPatternSubst (char *pLine, ...)

//	strPatternSubst
//
//	Substitutes patterns

	{
	char *pArgs;
	pArgs = (char *) &pLine + sizeof(pLine);
	return strPattern(pLine, (void **)pArgs);
	}
