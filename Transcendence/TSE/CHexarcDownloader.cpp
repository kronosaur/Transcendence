//	CHexarcDownloader.cpp
//
//	CHexarcDownloader class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define FIELD_AUTH_TOKEN						CONSTLIT("authToken")
#define FIELD_DATA								CONSTLIT("data")
#define FIELD_FILE_DESC							CONSTLIT("fileDesc")
#define FIELD_FILE_DOWNLOAD_DESC				CONSTLIT("fileDownloadDesc")
#define FIELD_FILE_PATH							CONSTLIT("filePath")
#define FIELD_PARTIAL_MAX_SIZE					CONSTLIT("partialMaxSize")
#define FIELD_PARTIAL_POS						CONSTLIT("partialPos")
#define FIELD_SIZE								CONSTLIT("size")

#define METHOD_POST								CONSTLIT("POST")

#define ERR_BINARY_EXPECTED						CONSTLIT("%s: Binary data expected.")
#define ERR_INVALID_FILE_SIZE					CONSTLIT("%s: Invalid file size.")
#define ERR_CANT_CREATE_FILE					CONSTLIT("%s: Unable to create file for writing.")
#define ERR_CANT_OPEN_FILE						CONSTLIT("%s: Unable to open file for writing.")
#define ERR_CANT_WRITE_FILE						CONSTLIT("%s: Unable to write to file.")

CHexarcDownloader::~CHexarcDownloader (void)

//	CHexarcDownloader destructor

	{
	//	Delete all entries in the queue

	while (m_Requests.GetCount())
		{
		delete m_Requests.Head();
		m_Requests.Dequeue();
		}

	if (m_pCurrent)
		delete m_pCurrent;
	}

void CHexarcDownloader::AddRequest (const CString &sAPI,
									const CString &sFilePath,
									const CJSONValue &AuthToken,
									const CString &sFilespec,
									const CIntegerIP &FileDigest)

//	AddRequest
//
//	Adds a request to download a file.

	{
	CSmartLock Lock(m_cs);

	SRequest *pNewRequest = new SRequest;
	pNewRequest->sAPI = sAPI;
	pNewRequest->sFilePath = sFilePath;
	pNewRequest->AuthToken = AuthToken;
	pNewRequest->sFilespec = sFilespec;
	pNewRequest->FileDigest = FileDigest;

	pNewRequest->dwTotalLen = 0;
	pNewRequest->dwDownload = 0;

	m_Requests.TryEnqueue(pNewRequest);
	}

void CHexarcDownloader::GetStatus (SStatus *retStatus)

//	GetStatus
//
//	Returns current status.

	{
	UpdateCurrent();

	if (m_pCurrent == NULL)
		{
		retStatus->sFilespec = NULL_STR;
		retStatus->iProgress = -1;
		return;
		}

	retStatus->sFilespec = m_pCurrent->sFilespec;
	if (m_pCurrent->dwTotalLen == 0)
		retStatus->iProgress = 0;
	else
		retStatus->iProgress = (100 * m_pCurrent->dwDownload / m_pCurrent->dwTotalLen);
	}

ALERROR CHexarcDownloader::Update (CHexarcSession &Session, SStatus *retStatus, CString *retsError)

//	Update
//
//	Download the next piece. The return value can be one of the following:
//
//	NOERROR:		We've successfully downloaded a file (retStatus	is valid).
//
//	ERR_MORE:		We're downloading a file and need to continue (retStatus is
//					valid).
//
//	ERR_NOTFOUND:	There are no more files to download.
//
//	ERR_FAIL:		An error occurred while downloading (retsError is valid).

	{
	UpdateCurrent();

	if (m_pCurrent == NULL)
		return ERR_NOTFOUND;

	//	Put together a payload to request a piece of the file.

	CJSONValue Result;
	CJSONValue Payload(CJSONValue::typeObject);
	Payload.Insert(FIELD_AUTH_TOKEN, m_pCurrent->AuthToken);
	Payload.InsertHandoff(FIELD_FILE_PATH, CJSONValue(m_pCurrent->sFilePath));

	//	A download desc describes what we want

	CJSONValue DownloadDesc(CJSONValue::typeObject);
	DownloadDesc.InsertHandoff(FIELD_PARTIAL_POS, CJSONValue((int)m_pCurrent->dwDownload));
	DownloadDesc.InsertHandoff(FIELD_PARTIAL_MAX_SIZE, CJSONValue((int)m_dwChunkSize));
	Payload.InsertHandoff(FIELD_FILE_DOWNLOAD_DESC, DownloadDesc);

	//	Download

	if (Session.ServerCommand(METHOD_POST, m_pCurrent->sAPI, Payload, &Result) != NOERROR)
		{
		*retsError = Result.AsString();
		return ERR_FAIL;
		}

	//	Get the file descriptor

	const CJSONValue &FileDesc = Result.GetElement(FIELD_FILE_DESC);

	//	If necessary set the total size

	if (m_pCurrent->dwTotalLen == 0)
		{
		m_pCurrent->dwTotalLen = FileDesc.GetElement(FIELD_SIZE).AsInt32();
		if (m_pCurrent->dwTotalLen == 0)
			{
			*retsError = strPatternSubst(ERR_INVALID_FILE_SIZE, m_pCurrent->sFilePath);
			return ERR_FAIL;
			}
		}

	//	Get the data

	const CJSONValue &Data = Result.GetElement(FIELD_DATA);
	CString sData;
	if (!CHexarc::IsBinary(Data, &sData))
		{
		*retsError = strPatternSubst(ERR_BINARY_EXPECTED, m_pCurrent->sFilePath);
		return ERR_FAIL;
		}

	//	Open up the destination file.

	CFileWriteStream File(m_pCurrent->sFilespec);

	//	If this is the first chunk then we create a new file (overwriting the
	//	destination).

	if (m_pCurrent->dwDownload == 0)
		{
		if (File.Create() != NOERROR)
			{
			*retsError = strPatternSubst(ERR_CANT_CREATE_FILE, m_pCurrent->sFilespec);
			return ERR_FAIL;
			}
		}

	//	Otherwise, we open it (which will seek to the end).

	else
		{
		if (File.Open() != NOERROR)
			{
			*retsError = strPatternSubst(ERR_CANT_OPEN_FILE, m_pCurrent->sFilespec);
			return ERR_FAIL;
			}
		}

	//	Write the out data.

	if (File.Write(sData.GetASCIIZPointer(), sData.GetLength()) != NOERROR)
		{
		*retsError = strPatternSubst(ERR_CANT_WRITE_FILE, m_pCurrent->sFilespec);
		return ERR_FAIL;
		}

	File.Close();

	//	Update state

	m_pCurrent->dwDownload += sData.GetLength();
	retStatus->sFilespec = m_pCurrent->sFilespec;
	retStatus->iProgress = (100 * m_pCurrent->dwDownload / m_pCurrent->dwTotalLen);
	retStatus->FileDigest = m_pCurrent->FileDigest;

	//	Are we done? If not, then ask for more

	if (m_pCurrent->dwDownload < m_pCurrent->dwTotalLen)
		return ERR_MORE;

	//	Otherwise, we're done. Clean up the current request.

	delete m_pCurrent;
	m_pCurrent = NULL;

	return NOERROR;
	}

void CHexarcDownloader::UpdateCurrent (void)

//	UpdateCurrent
//
//	If m_pCurrent is empty and there is stuff in the queue, pull it.

	{
	CSmartLock Lock(m_cs);

	if (m_pCurrent == NULL && m_Requests.GetCount())
		{
		m_pCurrent = m_Requests.Head();
		m_Requests.Dequeue();
		}
	}
