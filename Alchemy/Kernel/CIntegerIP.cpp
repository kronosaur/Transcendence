//	CIntegerIP.cpp
//
//	Implements CIntegerIP class
//	Copyright (c) 2012 by George Moromisato. All Rights Reserved.

#include "Kernel.h"
#include "KernelObjID.h"

#include "Euclid.h"
#include "Internets.h"

CIntegerIP::CIntegerIP (int iCount, BYTE *pNumber)

//	CIntegerIP constructor

	{
	ASSERT(iCount >= 0);

	m_iCount = iCount;
	if (m_iCount)
		{
		m_pNumber = new BYTE [m_iCount];
		if (pNumber)
			utlMemCopy((char *)pNumber, (char *)m_pNumber, m_iCount);
		else
			utlMemSet((char *)m_pNumber, m_iCount, 0);
		}
	else
		m_pNumber = NULL;
	}

CIntegerIP::CIntegerIP (const CIntegerIP &Src)

//	CIntegerIP constructor

	{
	Copy(Src);
	}

CIntegerIP::~CIntegerIP (void)

//	CIntegerIP destructor

	{
	CleanUp();
	}

CIntegerIP &CIntegerIP::operator= (const CIntegerIP &Src)

//	CIntegerIP operator =

	{
	CleanUp();
	Copy(Src);
	return *this;
	}

bool CIntegerIP::operator== (const CIntegerIP &Src) const

//	CIntegerIP operator ==

	{
	if (m_iCount != Src.m_iCount)
		return false;

	//	LATER: Optimize for DWORD compares

	BYTE *pDst = m_pNumber;
	BYTE *pDstEnd = pDst + m_iCount;
	BYTE *pSrc = Src.m_pNumber;
	while (pDst < pDstEnd)
		if (*pDst++ != *pSrc++)
			return false;

	return true;
	}

CString CIntegerIP::AsBase64 (void) const

//	AsBase64
//
//	The number as a base 64 string (big endian)

	{
	CMemoryWriteStream Buffer;
	if (Buffer.Create() != NOERROR)
		return NULL_STR;

	CBase64Encoder Encoder(&Buffer);
	if (Encoder.Create() != NOERROR)
		return NULL_STR;

	Encoder.Write((char *)GetBytes(), m_iCount);
	Encoder.Close();

	//	Done

	return CString(Buffer.GetPointer(), Buffer.GetLength());
	}

void CIntegerIP::CleanUp (void)

//	CleanUp
//
//	Free everything

	{
	if (m_pNumber)
		delete [] m_pNumber;

	m_pNumber = NULL;
	m_iCount = 0;
	}

void CIntegerIP::Copy (const CIntegerIP &Src)

//	Copy
//
//	Copy from source. We assume that we are clean.

	{
	m_iCount = Src.m_iCount;
	if (m_iCount)
		{
		m_pNumber = new BYTE [m_iCount];
		utlMemCopy((char *)Src.m_pNumber, (char *)m_pNumber, m_iCount);
		}
	}

void CIntegerIP::TakeHandoff (CIntegerIP &Src)

//	TakeHandoff
//
//	Takes buffer from source

	{
	CleanUp();

	m_iCount = Src.m_iCount;
	m_pNumber = Src.m_pNumber;
	Src.m_iCount = 0;
	Src.m_pNumber = NULL;
	}
