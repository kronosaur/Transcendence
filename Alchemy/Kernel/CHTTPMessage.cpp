//	CHTTPMessage.cpp
//
//	CHTTPMessage class

#include "Kernel.h"
#include "KernelObjID.h"

#include "Internets.h"

CString GetToken (char *pPos, char *pEndPos, char chDelimiter, char **retpPos);
bool ParseHeader (char *pPos, char *pEndPos, CString *retpField, CString *retpValue, char **retpPos);

CHTTPMessage::CHTTPMessage (void) : 
		m_iType(typeUnknown),
		m_dwStatusCode(0),
		m_pBody(NULL),
		m_iState(stateStart), 
		m_iPos(0)

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

ALERROR CHTTPMessage::InitFromBuffer (const CString &sBuffer, bool bNoBody)

//	InitFromBuffer
//
//	This function parses the given buffer into a message. The function
//	may be called multiple times as long as it is always called on the
//	same buffer (though the buffer may have additional content on 
//	each call).

	{
	char *pPos = sBuffer.GetASCIIZPointer() + m_iPos;
	char *pEndPos = sBuffer.GetASCIIZPointer() + sBuffer.GetLength();
	while (pPos < pEndPos && m_iState != stateDone)
		{
		switch (m_iState)
			{
			case stateStart:
				{
				//	Get the first token

				CString sToken = GetToken(pPos, pEndPos, ' ', &pPos);

				//	If this starts with HTTP then this is a version (which means that
				//	this is a response).

				if (strStartsWith(sToken, CONSTLIT("HTTP")))
					{
					m_iType = typeResponse;
					m_sVersion = sToken;

					//	Parse the rest of the response line

					m_dwStatusCode = strToInt(GetToken(pPos, pEndPos, ' ', &pPos), 0);
					m_sStatusMsg = GetToken(pPos, pEndPos, '\r', &pPos);
					m_sMethod = NULL_STR;
					m_sURL = NULL_STR;
					}

				//	Otherwise, this is a request

				else
					{
					m_iType = typeRequest;
					m_sMethod = sToken;

					//	Parse the rest of the request line

					m_sURL = GetToken(pPos, pEndPos, ' ', &pPos);
					m_sVersion = GetToken(pPos, pEndPos, '\r', &pPos);
					m_sStatusMsg = NULL_STR;
					m_dwStatusCode = 0;
					}

				//	Reset everything

				m_Headers.DeleteAll();
				m_sBody = NULL_STR;
				if (m_pBody)
					{
					delete m_pBody;
					m_pBody = NULL;
					}

				m_iState = stateHeaders;
				break;
				}

			case stateHeaders:
				{
				CString sField;
				CString sValue;
				while (ParseHeader(pPos, pEndPos, &sField, &sValue, &pPos))
					AddHeader(sField, sValue);

				//	Some messages never have a body

				CString sLength;

				if (bNoBody 
						|| (m_dwStatusCode >= 100 && m_dwStatusCode < 200)
						|| (m_dwStatusCode == 204) || (m_dwStatusCode == 304))
					m_iState = stateDone;

				//	If content length is 0 then we have no body

				else if (FindHeader(CONSTLIT("Content-Length"), &sLength) && strToInt(sLength, 0) == 0)
					m_iState = stateDone;

				//	Otherwise, parse the body

				else
					m_iState = stateBody;
				break;
				}

			case stateBody:
				{
				CString sEncoding;
				CString sLength;

				//	Chunked transfer encoding

				if (FindHeader(CONSTLIT("Transfer-Encoding"), &sEncoding)
						&& !strEquals(sEncoding, CONSTLIT("identity")))
					{
					//	Read the chunk size line

					CString sLine = GetToken(pPos, pEndPos, '\r', &pPos);
					int iTotalLength = strParseIntOfBase(sLine.GetASCIIZPointer(), 16, 0);

					//	0-length means we're done

					if (iTotalLength == 0)
						{
						m_iState = stateDone;
						break;
						}

					//	Otherwise, read as much as we can

					int iLength = Min(iTotalLength, pEndPos - pPos);
					m_sBody.Append(CString(pPos, iLength, true));
					pPos += iLength;

					//	We don't deal with partial chunks...

					if (iLength < iTotalLength)
						m_iState = stateDone;

					//	Keep looping for the next chunk
					}

				//	Otherwise, look for content length

				else if (FindHeader(CONSTLIT("Content-Length"), &sLength))
					{
					int iTotalLength = strToInt(sLength, 0);
					int iRemaining = iTotalLength - m_sBody.GetLength();
					int iLength = Min(iRemaining, pEndPos - pPos);

					//	Append the remainder of the buffer to the body

					if (iLength > 0)
						{
						m_sBody.Append(CString(pPos, iLength, true));
						pPos += iLength;
						}

					//	If we hit the end, then we're done

					if (iLength == iRemaining)
						m_iState = stateDone;

					//	Otherwise exit the look and get more data
					}

				//	Otherwise, we are done

				else
					m_iState = stateDone;

				break;
				}
			}
		}

	//	If we're done, decode the body into an IMediaType object

	if (m_pBody == NULL && m_iState == stateDone && !m_sBody.IsBlank())
		{
		CString sMediaType;
		if (!FindHeader(CONSTLIT("Content-Type"), &sMediaType))
			sMediaType = NULL_STR;

		m_pBody = new CRawMediaType;
		m_pBody->DecodeFromBuffer(sMediaType, m_sBody);
		m_sBody = NULL_STR;
		}

	//	Remember our position

	m_iPos = pPos - sBuffer.GetASCIIZPointer();

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

CString GetToken (char *pPos, char *pEndPos, char chDelimiter, char **retpPos)
	{
	char *pStart = pPos;
	while (pPos < pEndPos && *pPos != chDelimiter)
		pPos++;

	int iLen = pPos - pStart;

	//	If we hit the end, then we're done

	if (pPos == pEndPos)
		{
		if (retpPos)
			*retpPos = pPos;
		return CString(pStart, iLen);
		}

	//	Otherwise, skip the delimiter

	if (chDelimiter == ' ')
		{
		while (*pPos == ' ' && pPos < pEndPos)
			pPos++;
		}
	else if (chDelimiter == '\r')
		{
		pPos++;
		if (pPos < pEndPos && *pPos == '\n')
			pPos++;
		}
	else
		pPos++;

	//	Return the new position

	if (retpPos)
		*retpPos = pPos;

	return CString(pStart, iLen);
	}

bool ParseHeader (char *pPos, char *pEndPos, CString *retpField, CString *retpValue, char **retpPos)
	{
	//	If we hit CRLF, then no more headers

	if (pPos < pEndPos && *pPos == '\r')
		{
		pPos++;
		if (pPos < pEndPos && *pPos == '\n')
			pPos++;
		if (retpPos)
			*retpPos = pPos;
		return false;
		}

	//	Skip leading whitespace

	while (pPos < pEndPos && (*pPos == ' ' || *pPos == '\t'))
		pPos++;

	//	Parse the field name

	char *pStart = pPos;
	while (pPos < pEndPos && *pPos != ':' && *pPos != ' ' && *pPos != '\t')
		pPos++;

	CString sField(pStart, pPos - pStart);

	//	Find the colon

	while (pPos < pEndPos && *pPos != ':')
		pPos++;

	if (*pPos == ':')
		pPos++;

	//	Skip any whitespace

	while (pPos < pEndPos && (*pPos == ' ' || *pPos == '\t'))
		pPos++;

	//	Accumulate the value

	pStart = pPos;
	while (pPos < pEndPos)
		{
		//	If our next two characters are CRLF...

		if (pPos[0] == '\r' && pPos + 1 < pEndPos && pPos[1] == '\n')
			{
			//	If the character after that is whitepace, then just keep
			//	going...

			if (pPos + 2 < pEndPos && (pPos[2] == ' ' || pPos[2] == '\t'))
				pPos += 3;

			//	Otherwise, we've reached the end, so we break out

			else
				break;
			}
		else
			pPos++;
		}

	//	Value

	CString sValue(pStart, pPos - pStart);

	//	Swallow the CRLF

	if (pPos + 1 < pEndPos)
		pPos += 2;

	if (retpPos)
		*retpPos = pPos;

	//	Done

	*retpField = sField;
	*retpValue = sValue;

	return true;
	}

