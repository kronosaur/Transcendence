//	CFrameRateCounter.cpp
//
//	CFrameRateCounter class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

CFrameRateCounter::CFrameRateCounter (int iSize)

//	CFrameRateCounter constructor

	{
	ASSERT(iSize > 0);

	m_iSize = iSize;
	m_pTimes = new DWORD [m_iSize];
	m_iCount = 0;
	m_iPos = 0;
	}

CFrameRateCounter::~CFrameRateCounter (void)

//	CFrameRateCounter destructor

	{
	delete m_pTimes;
	}

float CFrameRateCounter::GetFrameRate (void)

//	GetFrameRate
//
//	Returns the current frame rate

	{
	if (m_iCount > 1)
		{
		DWORD dwStart = m_pTimes[(m_iPos - m_iCount + m_iSize) % m_iSize];
		DWORD dwEnd = m_pTimes[(m_iPos - 1 + m_iSize) % m_iSize];
		if (dwEnd - dwStart > 0)
			return 1000.0f * (float)(m_iCount - 1) / (float)(dwEnd - dwStart);
		else
			return 0.0;
		}
	else
		return 0.0;
	}

void CFrameRateCounter::MarkFrame (void)

//	MarkFrame
//
//	Records the current time of the frame. This should be called
//	once per frame.

	{
	m_pTimes[m_iPos % m_iSize] = ::GetTickCount();
	m_iPos++;
	if (m_iCount < m_iSize)
		m_iCount++;
	}
