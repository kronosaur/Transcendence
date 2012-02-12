//	CFileBlock.cpp
//
//	CFileBlock class
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"

CFileBlock::CFileBlock (const CString &sFilename) :
		m_sFilename(sFilename),
		m_hFile(NULL),
		m_dwFileSize(0)

//	CFileBlock constructor

	{
	}

CFileBlock::~CFileBlock (void)

//	CFileBlock destructor

	{
	if (m_hFile)
		{
		::UnmapViewOfFile(m_pFile);
		::CloseHandle(m_hFileMap);
		::CloseHandle(m_hFile);
		}
	}

int CFileBlock::GetLength (void)
	{
	OpenFile();
	return (int)m_dwFileSize; 
	}

char *CFileBlock::GetPointer (void)
	{
	OpenFile();
	return m_pFile;
	}

void CFileBlock::OpenFile (void)

//	OpenFile
//
//	Make sure that the file is open

	{
	if (m_hFile == NULL)
		{
		ASSERT(m_sFilename != NULL);

		m_hFile = ::CreateFile(m_sFilename,
				GENERIC_READ,
				0,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
		if (m_hFile == INVALID_HANDLE_VALUE)
			{
			m_hFile = NULL;
			throw CException(errUnableToOpenFile, m_sFilename);
			}

		//	Open a file mapping

		m_hFileMap = ::CreateFileMapping(m_hFile,
				NULL,
				PAGE_READONLY,
				0,
				0,
				NULL);
		if (m_hFileMap == INVALID_HANDLE_VALUE)
			{
			::CloseHandle(m_hFile);
			m_hFile = NULL;
			throw CException(errUnableToOpenFile, m_sFilename);
			}

		//	Map a view of the file

		m_pFile = (char *)::MapViewOfFile(m_hFileMap,
				FILE_MAP_READ,
				0,
				0,
				0);
		if (m_pFile == NULL)
			{
			::CloseHandle(m_hFileMap);
			::CloseHandle(m_hFile);
			m_hFile = NULL;
			throw CException(errUnableToOpenFile, m_sFilename);
			}

		//	Figure out the size of the file

		m_dwFileSize = ::GetFileSize(m_hFile, NULL);
		}
	}

