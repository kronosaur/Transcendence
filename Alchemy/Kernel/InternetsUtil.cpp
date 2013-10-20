//	InternetsUtil.cpp
//
//	Miscellaneous utility functions

#include "Kernel.h"

#include "Internets.h"

#define PROTOCOL_HTTP									CONSTLIT("http")
#define PROTOCOL_MAIL_TO								CONSTLIT("mailto")

#define EMPTY_PATH										CONSTLIT("/")

CString urlCompose (const CString &sProtocol, const CString &sHost, const CString &sPath)

//	urlCompose
//
//	Composes an URL

	{
	if (sPath.IsBlank())
		return strPatternSubst(CONSTLIT("%s://%s"), sProtocol, sHost);
	else if (*sPath.GetASCIIZPointer() == '/')
		return strPatternSubst(CONSTLIT("%s://%s%s"), sProtocol, sHost, sPath);
	else
		return strPatternSubst(CONSTLIT("%s://%s/%s"), sProtocol, sHost, sPath);
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

bool urlParse (char *pStart, CString *retsProtocol, CString *retsHost, CString *retsPath, char **retpEnd)

//	urlParse
//
//	Parses an url of the following forms:
//
//	http://www.example.com/dir/file.ext
//	www.example.com/dir/file.ext
//	/dir/file.exe

	{
	enum States
		{
		stateStart,
		stateStartText,
		stateProtocolSlash1,
		stateProtocolSlash2,
		stateProtocol,
		stateMailTo,
		stateHost,
		statePath,
		};

	//	Edge-conditions

	if (pStart == NULL)
		{
		if (retpEnd)
			*retpEnd = NULL;
		return false;
		}

	//	Prepare

	CString sProtocol;
	CString sHost;
	CString sPath;

	//	Parse

	States iState = stateStart;
	char *pToken = pStart;
	char *pPos = pStart;
	while (true)
		{
		switch (iState)
			{
			case stateStart:
				if (*pPos == '/')
					{
					pToken = pPos;
					iState = statePath;
					}
				else if (*pPos == ':' || *pPos == '\0' || strIsWhitespace(pPos))
					return false;
				else
					{
					pToken = pPos;
					iState = stateStartText;
					}

				break;

			case stateStartText:
				if (*pPos == '/' || *pPos == '\0' || strIsWhitespace(pPos))
					{
					sHost = CString(pToken, (int)(pPos - pToken));
					pToken = pPos;
					iState = statePath;
					}
				else if (*pPos == ':')
					{
					CString sToken = CString(pToken, (int)(pPos - pToken));

					//	Handle mailto

					if (strEquals(sToken, PROTOCOL_MAIL_TO))
						{
						sProtocol = sToken;
						iState = stateMailTo;
						}

					//	If a number follows the colon, then this is just the port
					//	on the host

					else if (strIsDigit(pPos + 1))
						break;

					//	Otherwise this is the protocol

					else
						{
						sProtocol = sToken;
						pToken = pPos;
						iState = stateProtocolSlash1;
						}
					}

				break;

			case stateMailTo:
				//	We just skipped the colon; the rest is the "path"
				pToken = pPos;
				iState = statePath;
				break;

			case stateProtocolSlash1:
				//	We expect a first slash
				if (*pPos != '/')
					return false;

				iState = stateProtocolSlash2;
				break;

			case stateProtocolSlash2:
				//	We expect a second slash
				if (*pPos != '/')
					return false;

				iState = stateProtocol;
				break;

			case stateProtocol:
				pToken = pPos;

				//	Another slash means a null host

				if (*pPos == '/')
					iState = statePath;
				else
					iState = stateHost;
				break;

			case stateHost:
				if (*pPos == '/' || *pPos == '\0' || strIsWhitespace(pPos))
					{
					sHost = CString(pToken, (int)(pPos - pToken));
					pToken = pPos;
					iState = statePath;
					}
				break;

			case statePath:
				if (*pPos == '\0' || strIsWhitespace(pPos))
					sPath = CString(pToken, (int)(pPos - pToken));
				break;
			}

		if (*pPos == '\0')
			break;
		else
			pPos++;
		}

	//	Done

	if (retsProtocol)
		{
		if (sProtocol.IsBlank())
			*retsProtocol = PROTOCOL_HTTP;
		else
			*retsProtocol = sProtocol;
		}

	if (retsHost)
		*retsHost = sHost;

	if (retsPath)
		{
		if (sPath.IsBlank())
			*retsPath = EMPTY_PATH;
		else
			*retsPath = sPath;
		}

	if (retpEnd)
		*retpEnd = pPos;

	return true;
	}

void urlParseHostspec (const CString &sHostspec, CString *retsHost, CString *retsPort)

//	urlParseHostspec
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

