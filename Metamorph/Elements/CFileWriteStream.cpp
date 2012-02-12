//	CFileStream.cpp
//
//	CFileStream class
//	Copyright (c) 2002 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"

CFileStream::CFileStream (const CString &sFilename, bool bUnique) :
		m_sFilename(sFilename),
		m_bUnique(bUnique),
		m_hFile(NULL)

//	CFileStream constructor

	{
	OpenFile();
	}

CFileStream::~CFileStream (void)

//	CFileStream destructor

	{
	if (m_hFile)
		::CloseHandle(m_hFile);
	}

void CFileStream::OpenFile (void)

//	OpenFile
//
//	Opens the file

	{
	ASSERT(m_hFile == NULL);

	m_hFile = ::CreateFile(m_sFilename.GetASCIIZ(),
			GENERIC_WRITE,
			0,
			NULL,
			(m_bUnique ? CREATE_NEW : CREATE_ALWAYS),
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
		{
		m_hFile = NULL;
		throw CException(errUnableToSaveFile, m_sFilename);
		}
	}

//	IByteStream virtuals

int CFileStream::GetPos (void)

//	GetPos
//
//	Return the current position

	{
	ASSERT(m_hFile);
	return (int)::SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT);
	}

int CFileStream::GetStreamLength (void)

//	GetStreamLength
//
//	Return the current total length

	{
	ASSERT(m_hFile);
	return (int)::GetFileSize(m_hFile, NULL);
	}

int CFileStream::Read (void *pData, int iLength)

//	Read
//
//	Reads from the pointer

	{
	ASSERT(m_hFile);

	DWORD dwRead;
	if (!::ReadFile(m_hFile,
			pData,
			iLength,
			&dwRead,
			NULL))
		throw CException(errUnableToOpenFile, m_sFilename);

	return (int)dwRead;
	}

void CFileStream::Seek (int iPos, bool bFromEnd)

//	Seek
//
//	Seeks

	{
	ASSERT(m_hFile);
	::SetFilePointer(m_hFile,
			iPos,
			NULL,
			(bFromEnd ? FILE_END : FILE_BEGIN));
	}

int CFileStream::Write (void *pData, int iLength)

//	Write
//
//	Writes out

	{
	ASSERT(m_hFile);

	DWORD dwWritten;
	if (!::WriteFile(m_hFile,
			pData,
			iLength,
			&dwWritten,
			NULL))
		throw CException(errUnableToSaveFile, m_sFilename);

	return (int)dwWritten;
	}
