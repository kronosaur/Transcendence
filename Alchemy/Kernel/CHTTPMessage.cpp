//	CHTTPMessage.cpp
//
//	CHTTPMessage class

#include "Kernel.h"
#include "KernelObjID.h"

#include "Internets.h"

bool ReadLine (IReadStream &Stream, CString *retsLine);

CHTTPMessage::CHTTPMessage (void) : 
		m_iType(typeUnknown),
		m_dwStatusCode(0),
		m_pBody(NULL)

//	CHTTPMessage constructor

	{
	}

CHTTPMessage::~CHTTPMessage (void)

//	CHTTPMessage destructor

	{
	if (m_pBody)
		delete m_pBody;
	}

void CHTTPMessage::AddHeader (const CString &sField, const CString &sValue)

//	AddHeader
//
//	Adds a header to the message

	{
	SHeader *pHeader = m_Headers.Insert();
	pHeader->sField = sField;
	pHeader->sValue = sValue;
	}

void CHTTPMessage::CleanUp (void)

//	CleanUp
//
//	Reinitialize

	{
	m_iType = typeUnknown;
	m_sMethod = NULL_STR;
	m_sURL = NULL_STR;
	m_sVersion = NULL_STR;
	m_dwStatusCode = 0;
	m_sStatusMsg = NULL_STR;
	m_Headers.DeleteAll();
	if (m_pBody)
		{
		delete m_pBody;
		m_pBody = NULL;
		}
	}

bool CHTTPMessage::FindHeader (const CString &sField, CString *retsValue) const

//	FindHeader
//
//	Looks for the given header and returns its value

	{
	int i;

	for (i = 0; i < m_Headers.GetCount(); i++)
		{
		if (strEquals(m_Headers[i].sField, sField))
			{
			if (retsValue)
				*retsValue = m_Headers[i].sValue;

			return true;
			}
		}

	return false;
	}

ALERROR CHTTPMessage::InitFromStream (IReadStream &Stream, CString *retsError, bool bNoBody)

//	InitFromStream
//
//	Initializes from a stream.

	{
	CleanUp();

	//	Parse and interpret the start line

	if (!ParseHTTPStartLine(Stream, retsError))
		return ERR_FAIL;

	//	Parse the headers

	CString sHeaderLine;
	do
		{
		if (!ReadLine(Stream, &sHeaderLine))
			return ERR_FAIL;

		if (!sHeaderLine.IsBlank())
			{
			//	If this line starts with a space, then it is a continuation of 
			//	the previous field's content

			if ((*sHeaderLine.GetASCIIZPointer()) == ' ' || (*sHeaderLine.GetASCIIZPointer()) == '\t')
				{
				if (m_Headers.GetCount() > 0)
					m_Headers[m_Headers.GetCount() - 1].sValue.Append(sHeaderLine);
				}

			//	Otherwise, we just parse and add

			else
				{
				if (!ParseHTTPHeader(sHeaderLine, retsError))
					return ERR_FAIL;
				}
			}
		}
	while (!sHeaderLine.IsBlank());

	//	Some messages never have a body

	if (bNoBody 
			|| (m_dwStatusCode >= 100 && m_dwStatusCode < 200)
			|| (m_dwStatusCode == 204) || (m_dwStatusCode == 304))
		return NOERROR;

	//	If we have a content length, then see if it is 0

	CString sLength;
	DWORD dwLength = 0;
	if (FindHeader(CONSTLIT("Content-Length"), &sLength) 
			&& (dwLength = strToInt(sLength, 0)) == 0)
		return NOERROR;

	//	Chunked encoding?

	CString sEncoding;
	if (FindHeader(CONSTLIT("Transfer-Encoding"), &sEncoding)
			&& strEquals(sEncoding, CONSTLIT("chunked")))
		{
		//	Build up the body

		CMemoryWriteStream Body;
		if (Body.Create() != NOERROR)
			return ERR_MEMORY;

		//	Loop over all chunks

		CString sChunkLen;
		do
			{
			if (!ReadLine(Stream, &sChunkLen))
				return ERR_FAIL;

			DWORD dwChunkLen = strParseIntOfBase(sChunkLen.GetASCIIZPointer(), 16, 0);
			if (dwChunkLen)
				{
				char *pBuffer = new char [dwChunkLen];
				if (Stream.Read(pBuffer, dwChunkLen) != NOERROR)
					{
					delete [] pBuffer;
					return ERR_FAIL;
					}

				if (Body.Write(pBuffer, dwChunkLen) != NOERROR)
					{
					delete [] pBuffer;
					return ERR_MEMORY;
					}

				delete [] pBuffer;

				//	Expect a CRLF

				CString sCRLF;
				if (!ReadLine(Stream, &sCRLF))
					return ERR_FAIL;
				}
			}
		while (!sChunkLen.IsBlank());

		//	Create the body

		CString sMediaType;
		if (!FindHeader(CONSTLIT("Content-Type"), &sMediaType))
			sMediaType = NULL_STR;

		m_pBody = new CRawMediaType;
		m_pBody->DecodeFromBuffer(sMediaType, CString(Body.GetPointer(), Body.GetLength()));
		}

	//	Otherwise, we have a set length (which must have been set by the
	//	Content-Length header).

	else if (dwLength)
		{
		CString sBody;
		char *pPos = sBody.GetWritePointer(dwLength);
		if (Stream.Read(pPos, dwLength) != NOERROR)
			return ERR_FAIL;

		CString sMediaType;
		if (!FindHeader(CONSTLIT("Content-Type"), &sMediaType))
			sMediaType = NULL_STR;

		m_pBody = new CRawMediaType;
		m_pBody->DecodeFromBuffer(sMediaType, sBody);
		}

	//	Done

	return NOERROR;
	}

ALERROR CHTTPMessage::InitRequest (const CString &sMethod, const CString &sURL)

//	InitRequest
//
//	Initializes an HTTP request message

	{
	CleanUp();

	m_iType = typeRequest;
	m_sMethod = sMethod;
	m_sURL = sURL;
	m_sVersion = CONSTLIT("HTTP/1.1");

	return NOERROR;
	}

ALERROR CHTTPMessage::InitResponse (DWORD dwStatusCode, const CString &sStatusMsg)

//	InitResponse
//
//	Initializes an HTTP response message

	{
	CleanUp();

	m_iType = typeResponse;
	m_sVersion = CONSTLIT("HTTP/1.1");
	m_dwStatusCode = dwStatusCode;
	m_sStatusMsg = sStatusMsg;

	return NOERROR;
	}

bool CHTTPMessage::ParseHTTPHeader (const CString &sHeaderLine, CString *retsError)

//	ParseHTTPHeader
//
//	Parses a header

	{
	char *pPos = sHeaderLine.GetASCIIZPointer();
	while (strIsWhitespace(pPos))
		pPos++;

	//	Field name

	char *pStart = pPos;
	while (!strIsWhitespace(pPos) && *pPos != ':' && *pPos != '\0')
		pPos++;

	if (*pPos == '\0')
		return false;

	CString sField(pStart, (pPos - pStart));

	while (*pPos != ':' && *pPos != '\0')
		pPos++;

	if (*pPos == '\0')
		return false;

	pPos++;
	while (strIsWhitespace(pPos))
		pPos++;

	//	Content

	CString sValue(pPos);

	//	Add it

	AddHeader(sField, sValue);
	return true;
	}

bool CHTTPMessage::ParseHTTPStartLine (IReadStream &Stream, CString *retsError)

//	ParseHTTPStartLine
//
//	Parses the start line of a request or response

	{
	//	Read the line

	CString sLine;
	if (!ReadLine(Stream, &sLine))
		return ERR_FAIL;

	//	If this starts with HTTP then this is a response.

	if (strStartsWith(sLine, CONSTLIT("HTTP")))
		{
		m_iType = typeResponse;

		//	Parse the version

		char *pPos = sLine.GetASCIIZPointer();
		char *pStart = pPos;
		while (*pPos != ' ' && *pPos != '\0')
			pPos++;

		if (*pPos == '\0')
			return false;

		m_sVersion = CString(pStart, (pPos - pStart));

		//	Next is a status code

		pPos++;
		pStart = pPos;
		while (*pPos != ' ' && *pPos != '\0')
			pPos++;

		if (*pPos == '\0')
			return false;

		m_dwStatusCode = strToInt(CString(pStart, (pPos - pStart)), 0);

		//	Next is the status message

		pPos++;
		pStart = pPos;
		while (*pPos != '\0')
			pPos++;

		m_sStatusMsg = CString(pStart, (pPos - pStart));

		//	Unused for responses

		m_sMethod = NULL_STR;
		m_sURL = NULL_STR;
		}

	//	Otherwise, this is a request

	else
		{
		m_iType = typeRequest;

		//	Parse the method

		char *pPos = sLine.GetASCIIZPointer();
		char *pStart = pPos;
		while (*pPos != ' ' && *pPos != '\0')
			pPos++;

		if (*pPos == '\0')
			return false;

		m_sMethod = CString(pStart, (pPos - pStart));

		//	Parse the URI

		pPos++;
		pStart = pPos;
		while (*pPos != ' ' && *pPos != '\0')
			pPos++;

		if (*pPos == '\0')
			return false;

		m_sURL = CString(pStart, (pPos - pStart));

		//	Parse the version

		pPos++;
		pStart = pPos;
		while (*pPos != '\0')
			pPos++;

		m_sVersion = CString(pStart, (pPos - pStart));

		//	Unused

		m_sStatusMsg = NULL_STR;
		m_dwStatusCode = 0;
		}

	return true;
	}

ALERROR CHTTPMessage::WriteToBuffer (IWriteStream *pOutput) const

//	WriteToBuffer
//
//	Writes the message to a buffer suitable for transmission.
//	We assume that the stream has already been created.

	{
	ALERROR error;
	int i;

	ASSERT(m_iType != typeUnknown);

	//	Write the status line

	CString sLine;
	if (m_iType == typeRequest)
		{
		ASSERT(!m_sMethod.IsBlank());
		ASSERT(!m_sURL.IsBlank());
		sLine = strPatternSubst(CONSTLIT("%s %s HTTP/1.1\r\n"), m_sMethod, m_sURL);
		}
	else
		{
		ASSERT(m_dwStatusCode > 0);
		ASSERT(!m_sStatusMsg.IsBlank());
		sLine = strPatternSubst(CONSTLIT("HTTP/1.1 %d %s\r\n"), m_dwStatusCode, m_sStatusMsg);
		}

	if (error = pOutput->Write(sLine.GetASCIIZPointer(), sLine.GetLength()))
		return error;

	//	Write the headers

	for (i = 0; i < m_Headers.GetCount(); i++)
		{
		if (!m_Headers[i].sField.IsBlank())
			{
			sLine = strPatternSubst(CONSTLIT("%s: %s\r\n"), m_Headers[i].sField, m_Headers[i].sValue);
			if (error = pOutput->Write(sLine.GetASCIIZPointer(), sLine.GetLength()))
				return error;
			}
		}

	//	If we don't have a Content-Type header then output our own

	if (!FindHeader(CONSTLIT("Content-Type")) && m_pBody)
		{
		sLine = strPatternSubst(CONSTLIT("Content-Type: %s\r\n"), m_pBody->GetMediaType());
		if (error = pOutput->Write(sLine.GetASCIIZPointer(), sLine.GetLength()))
			return error;
		}

	//	If we don't have a Content-Length header then we output our own

	if (!FindHeader(CONSTLIT("Content-Length")) && m_pBody)
		{
		sLine = strPatternSubst(CONSTLIT("Content-Length: %d\r\n"), m_pBody->GetMediaLength());
		if (error = pOutput->Write(sLine.GetASCIIZPointer(), sLine.GetLength()))
			return error;
		}

	//	Done with headers

	if (error = pOutput->Write("\r\n", 2))
		return error;

	//	Output the body

	if (m_pBody)
		{
		if (error = m_pBody->EncodeToBuffer(pOutput))
			return error;
		}

	return NOERROR;
	}

//	Utilities ------------------------------------------------------------------

bool ReadLine (IReadStream &Stream, CString *retsLine)
	{
	int iAlloc = 1024;

	CString sLine;
	char *pDest;
	int iTotalAlloc = 0;
	int iTotalLen = 0;

	bool bFoundCR = false;

	while (true)
		{
		//	If we need more room, reallocate

		if (iTotalLen == iTotalAlloc)
			{
			pDest = sLine.GetWritePointer(iTotalAlloc + iAlloc);
			pDest += iTotalAlloc;
			iTotalAlloc += iAlloc;
			}

		//	Read a character

		if (Stream.Read(pDest, 1) != NOERROR)
			return false;

		//	If we've already found a CR, then see if we have an LF

		if (bFoundCR)
			{
			if (*pDest == '\n')
				{
				//	We're done; omit the ending CR in the result.

				sLine.Truncate(iTotalLen - 1);
				if (retsLine)
					*retsLine = sLine;

				//	Done

				return true;
				}
			else
				{
				pDest++;
				iTotalLen++;
				bFoundCR = false;
				}
			}

		//	Otherwise, see if we have a CR

		else if (*pDest == '\r')
			{
			pDest++;
			iTotalLen++;
			bFoundCR = true;
			}

		//	Otherwise, keep building the string

		else
			{
			pDest++;
			iTotalLen++;
			}
		}
	}
