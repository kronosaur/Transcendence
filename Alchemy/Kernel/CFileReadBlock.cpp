//	CFileReadBlock.cpp
//
//	Implements CFileReadBlock object

#include "Kernel.h"
#include "KernelObjID.h"

static CObjectClass<CFileReadBlock>g_Class(OBJID_CFILEREADBLOCK, NULL);

CFileReadBlock::CFileReadBlock (void) :
		CObject(&g_Class)

//	CFileReadBlock constructor

	{
	}

CFileReadBlock::CFileReadBlock (const CString &sFilename) :
		CObject(&g_Class),
		m_sFilename(sFilename),
		m_hFile(NULL),
		m_dwFileSize(0)

//	CFileReadBlock constructor

	{
	}

CFileReadBlock::~CFileReadBlock (void)

//	CFileReadBlock destructor

	{
	//	Close the file if necessary

	Close();
	}

ALERROR CFileReadBlock::Close (void)

//	CFileReadBlock
//
//	Close the stream

	{
	if (m_hFile == NULL)
		return NOERROR;

	//	Close the file

	UnmapViewOfFile(m_pFile);
	CloseHandle(m_hFileMap);
	CloseHandle(m_hFile);
	m_hFile = NULL;

	return NOERROR;
	}

ALERROR CFileReadBlock::Open (void)

//	Open
//
//	Opens the stream for reading

	{
	if (m_hFile)
		return NOERROR;

	m_hFile = CreateFile(m_sFilename.GetASCIIZPointer(),
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
		{
		m_hFile = NULL;

		switch (::GetLastError())
			{
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
				return ERR_NOTFOUND;

			default:
				return ERR_FAIL;
			}
		}

	//	Open a file mapping

	m_hFileMap = CreateFileMapping(m_hFile,
			NULL,
			PAGE_READONLY,
			0,
			0,
			NULL);
	if (m_hFileMap == INVALID_HANDLE_VALUE)
		{
		CloseHandle(m_hFile);
		m_hFile = NULL;
		return ERR_FAIL;
		}

	//	Map a view of the file

	m_pFile = (char *)MapViewOfFile(m_hFileMap,
			FILE_MAP_READ,
			0,
			0,
			0);
	if (m_pFile == NULL)
		{
		CloseHandle(m_hFileMap);
		CloseHandle(m_hFile);
		m_hFile = NULL;
		return ERR_FAIL;
		}

	//	Figure out the size of the file

	m_dwFileSize = ::GetFileSize(m_hFile, NULL);

	return NOERROR;
	}
