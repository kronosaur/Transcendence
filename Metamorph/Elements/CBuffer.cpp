//	CBuffer.cpp
//
//	CBuffer class.
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"

CBuffer::CBuffer (int iSize)

//	CBuffer constructor

	{
	m_pBuffer = new char [iSize];
	m_iLength = iSize;
	m_pPointer = m_pBuffer;
	m_bAllocated = true;
	}

CBuffer::CBuffer (const CString &sString, int iPos, int iLength)

//	CBuffer constructor

	{
	m_pBuffer = sString.GetASCIIZ() + iPos;
	if (iLength == -1)
		m_iLength = sString.GetLength() - iPos;
	else
		m_iLength = iLength;
	m_pPointer = m_pBuffer;
	m_bAllocated = false;
	}

CBuffer::~CBuffer (void)

//	CBuffer destructor

	{
	if (m_bAllocated)
		delete [] m_pBuffer;
	}

int CBuffer::Read (void *pData, int iLength)

//	Read
//
//	Reads from the buffer at the current position

	{
	int iLeft = m_iLength - (m_pPointer - m_pBuffer);
	int iRead = min(iLeft, iLength);

	utlMemCopy(m_pPointer, pData, iRead);
	m_pPointer += iRead;
	return iRead;
	}

void CBuffer::Seek (int iPos, bool bFromEnd)

//	Seek
//
//	Seeks to a position in the buffer

	{
	if (bFromEnd)
		m_pPointer = (m_pBuffer + m_iLength) + max(iPos, -m_iLength);
	else
		m_pPointer = m_pBuffer + min(iPos, m_iLength);
	}

int CBuffer::Write (void *pData, int iLength)

//	Write
//
//	Writes to the buffer at the current position

	{
	int iLeft = m_iLength - (m_pPointer - m_pBuffer);
	int iWrite = min(iLeft, iLength);

	utlMemCopy(pData, m_pPointer, iWrite);
	m_pPointer += iWrite;
	return iWrite;
	}
