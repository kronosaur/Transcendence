//	InternetsUtil.cpp
//
//	Miscellaneous utility functions

#include "Kernel.h"

#include "Internets.h"

void strParseHostspec (const CString &sHostspec, CString *retsHost, CString *retsPort)

//	strParseHostspec
//
//	Parses a spec of the form:
//
//	hostname
//	hostname:port

	{
	char *pPos = sHostspec.GetASCIIZPointer();
	char *pStart = pPos;
	while (*pPos != ':' && *pPos != '\0')
		pPos++;

	if (retsHost)
		*retsHost = CString(pStart, (int)(pPos - pStart));

	if (retsPort)
		{
		if (*pPos != ':')
			{
			*retsPort = NULL_STR;
			return;
			}

		pPos++;
		pStart = pPos;
		while (*pPos != '\0')
			pPos++;

		*retsPort = CString(pStart, (int)(pPos - pStart));
		}
	}

CString urlDecode (const CString &sURL, DWORD dwFlags)

//	urlDecode
//
//	Decodes an URL encoded string

	{
	ASSERT(false);
	return sURL;
	}

CString urlEncode (const CString &sText, DWORD dwFlags)

//	urlEncode
//
//	Encodes the given string into an URL

	{
	CMemoryWriteStream Output;
	if (Output.Create() != NOERROR)
		return NULL_STR;

	char *pPos = sText.GetASCIIZPointer();
	char *pEndPos = pPos + sText.GetLength();
	while (pPos < pEndPos)
		{
		if ((*pPos >= 'A' && *pPos <= 'Z')
				|| (*pPos >= 'a' && *pPos <= 'z')
				|| (*pPos >= '0' && *pPos <= '9'))
			Output.Write(pPos, 1);
		else
			{
			DWORD dwHex = *pPos;
			CString sEscape = strPatternSubst(CONSTLIT("%%%02X"), dwHex);
			Output.Write(sEscape.GetASCIIZPointer(), sEscape.GetLength());
			}

		pPos++;
		}

	//	Done

	return CString(Output.GetPointer(), Output.GetLength());
	}

