//	CTextFileLog.cpp
//
//	Implements CTextFileLog object

#include "Kernel.h"
#include "KernelObjID.h"

static CObjectClass<CTextFileLog>g_Class(OBJID_CTEXTFILELOG, NULL);

CTextFileLog::CTextFileLog (void) : CObject(&g_Class),
		m_hFile(NULL)

//	CTextFileLog constructor

	{
	}

CTextFileLog::CTextFileLog (const CString &sFilename) : CObject(&g_Class),
		m_sFilename(sFilename),
		m_hFile(NULL)

//	CTextFileLog constructor

	{
	}

CTextFileLog::~CTextFileLog (void)

//	CTextFileLog destructor

	{
	Close();
	}

ALERROR CTextFileLog::Close (void)

//	Close
//
//	Close the log

	{
	if (m_hFile == NULL)
		return NOERROR;

	CloseHandle(m_hFile);
	m_hFile = NULL;

	return NOERROR;
	}

ALERROR CTextFileLog::Create (BOOL bAppend)

//	Create
//
//	Create a new log file

	{
	ASSERT(m_hFile == NULL);

	m_hFile = CreateFile(m_sFilename.GetASCIIZPointer(),
			GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
		{
		m_hFile = NULL;
		return ERR_FAIL;
		}

	//	If we're appending to an existing log file, move the file pointer
	//	to the end of the file.

	if (bAppend)
		{
		LONG lFileHigh = 0;
		SetFilePointer(m_hFile, 0, &lFileHigh, FILE_END);
		}

	//	Otherwise, truncate the file

	else
		{
		SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN);
		SetEndOfFile(m_hFile);
		}

	return NOERROR;
	}

void CTextFileLog::LogOutput (DWORD dwFlags, char *pszLine, ...)

//	LogOutput
//
//	Output a line to the log

	{
	char *pArgs;
	char szBuffer[1024];
	char szLine[1024];
	int iLen;
	DWORD dwWritten;

	ASSERT(m_hFile);

	pArgs = (char *)&pszLine + sizeof(pszLine);
	wvsprintf(szLine, pszLine, pArgs);

	//	Append time date

	if (dwFlags & ILOG_FLAG_TIMEDATE)
		{
		SYSTEMTIME time;

		GetLocalTime(&time);
		iLen = wsprintf(szBuffer, "%02d/%02d/%04d %02d:%02d:%02d\t%s\r\n",
				time.wMonth,
				time.wDay,
				time.wYear,
				time.wHour,
				time.wMinute,
				time.wSecond,
				szLine);
		}
	else
		iLen = wsprintf(szBuffer, "%s\r\n", szLine);

	//	Write out the line

	WriteFile(m_hFile, szBuffer, iLen, &dwWritten, NULL);

	//	Flush now because we don't want to lose any info if we crash

	FlushFileBuffers(m_hFile);
	}

void CTextFileLog::SetFilename (const CString &sFilename)

//	SetFilename
//
//	Sets the filename of the log file

	{
	ASSERT(m_hFile == NULL);
	m_sFilename = sFilename;
	}
