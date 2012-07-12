//	Pattern.cpp
//
//	String pattern package
//
//	Pattern format:
//
//	%type
//
//	where type is one of the following:
//
//	%	Evaluates to a single percent sign
//
//	d	Argument is a signed 32-bit integer. The number is substituted
//
//	p	If the last integer argument not 1, then 's' is substituted.
//		This is used to pluralize words in the English language.
//
//	s	Argument is a CString. The string is substituted
//
//	x	Argument is an unsigned 32-bit integer. The hex value is substituted

#include "Kernel.h"

void WritePadding (CMemoryWriteStream &Stream, char chChar, int iLen);

CString strPattern (const CString &sPattern, LPVOID *pArgs)

//	strPattern
//
//	Returns a string with a pattern substitution

	{
	ALERROR error;
	CMemoryWriteStream Stream(0);
	char *pPos = sPattern.GetPointer();
	int iLength = sPattern.GetLength();
	char *pRunStart;
	int iRunLength;
	int iLastInteger = 1;

	//	Initialize the memory stream that will hold the result

	if (error = Stream.Create())
		return LITERAL("%ERROR");

	//	Start

	pRunStart = pPos;
	iRunLength = 0;

	//	Loop

	while (iLength > 0)
		{
		if (*pPos == '%')
			{
			//	Save out what we've got til now

			if (iRunLength > 0)
				{
				if (error = Stream.Write(pRunStart, iRunLength, NULL))
					return LITERAL("%ERROR");
				}

			//	Check the actual pattern code

			pPos++;
			iLength--;
			if (iLength > 0)
				{
				int iMinFieldWidth = 0;
				bool bPadWithZeros = false;
				bool b1000Separator = false;

				//	A leading comma means add a thousands separator

				if (*pPos == ',')
					{
					b1000Separator = true;
					pPos++;
					iLength--;
					}

				//	See if we've got a field width value

				if (*pPos >= '0' && *pPos <= '9')
					{
					char *pNewPos;
					bPadWithZeros = (*pPos == '0');
					iMinFieldWidth = strParseInt(pPos, 0, &pNewPos);

					if (iLength > (pNewPos - pPos))
						{
						iLength -= (pNewPos - pPos);
						pPos = pNewPos;
						}
					}

				//	Parse the type

				if (*pPos == 's')
					{
					CString *pParam = (CString *)pArgs;

					if (iMinFieldWidth > 0)
						WritePadding(Stream, (bPadWithZeros ? '0' : ' '), iMinFieldWidth - pParam->GetLength());

					if (error = Stream.Write(pParam->GetPointer(), pParam->GetLength(), NULL))
						return LITERAL("%ERROR");

					pArgs += AlignUp(sizeof(CString), sizeof(LPVOID)) / sizeof(LPVOID);
					pPos++;
					iLength--;
					}
				else if (*pPos == 'd')
					{
					int *pInt = (int *)pArgs;

					DWORD dwFlags = (b1000Separator ? FORMAT_THOUSAND_SEPARATOR : 0)
							| (bPadWithZeros ? FORMAT_LEADING_ZERO : 0);

					CString sNew = strFormatInteger(*pInt, iMinFieldWidth, dwFlags);

					if (error = Stream.Write(sNew.GetPointer(), sNew.GetLength(), NULL))
						return LITERAL("%ERROR");

					//	Remember the last integer

					iLastInteger = *pInt;

					//	Next

					pArgs++;
					pPos++;
					iLength--;
					}
				else if (*pPos == 'x' || *pPos == 'X')
					{
					int *pInt = (int *)pArgs;
					char szBuffer[256];
					int iLen = wsprintf(szBuffer, (*pPos == 'x' ? "%x" : "%X"), *pInt);

					if (iMinFieldWidth > 0)
						WritePadding(Stream, (bPadWithZeros ? '0' : ' '), iMinFieldWidth - iLen);

					if (error = Stream.Write(szBuffer, iLen, NULL))
						return LITERAL("%ERROR");

					//	Remember the last integer

					iLastInteger = *pInt;

					//	Next

					pArgs++;
					pPos++;
					iLength--;
					}
				else if (*pPos == 'p')
					{
					if (iLastInteger != 1)
						{
						if (error = Stream.Write("s", 1, NULL))
							return LITERAL("%ERROR");
						}

					pPos++;
					iLength--;
					}
				else if (*pPos == '%')
					{
					if (error = Stream.Write("%", 1, NULL))
						return LITERAL("%ERROR");

					pPos++;
					iLength--;
					}
				}

			pRunStart = pPos;
			iRunLength = 0;
			}
		else
			{
			iRunLength++;
			iLength--;
			pPos++;
			}
		}

	//	Save out the last run

	if (iRunLength > 0)
		{
		if (error = Stream.Write(pRunStart, iRunLength, NULL))
			return LITERAL("%ERROR");
		}

	//	Convert the stream to a string

	return CString(Stream.GetPointer(), Stream.GetLength());
	}

CString strPatternSubst (CString sLine, ...)

//	strPatternSubst
//
//	Substitutes patterns

	{
	char *pArgs;
	CString sParsedLine;

	pArgs = (char *) &sLine + sizeof(sLine);
	sParsedLine = strPattern(sLine, (void **)pArgs);
	return sParsedLine;
	}

void WritePadding (CMemoryWriteStream &Stream, char chChar, int iLen)
	{
	if (iLen > 0)
		{
		char szBuffer[256];
		char *pBuffer;
		if (iLen <= sizeof(szBuffer))
			pBuffer = szBuffer;
		else
			pBuffer = new char [iLen];

		char *pPos = pBuffer;
		char *pEndPos = pPos + iLen;
		while (pPos < pEndPos)
			*pPos++ = chChar;

		Stream.Write(pBuffer, iLen, NULL);

		if (pBuffer != szBuffer)
			delete [] pBuffer;
		}
	}
