//	CPeriodicWaiter.cpp
//
//	Implements CPeriodicWaiter object
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "Kernel.h"
#include "KernelObjID.h"

CPeriodicWaiter::CPeriodicWaiter (DWORD dwPeriod) : m_dwPeriod(dwPeriod)

//	CPeriodicWaiter constructor

	{
	ASSERT(dwPeriod > 0);
	m_LastCounter = 0;

	//	See if we can use the performance counters

	LARGE_INTEGER PCFreq;
	if (::QueryPerformanceFrequency(&PCFreq))
		{
		m_PCFreq = PCFreq.QuadPart;
		m_PCCountsPerPeriod = m_PCFreq * m_dwPeriod / 1000;
		}
	else
		{
		m_PCFreq = 0;
		m_PCCountsPerPeriod = 0;
		}
	}

void CPeriodicWaiter::Wait (void)

//	Wait
//
//	Wait an amount of time such that the time between the end of the
//	last call to Wait and the end of this call to Wait is equal to m_dwPeriod

	{
	if (m_LastCounter > 0)
		{
		if (m_PCFreq)
			{
			LARGE_INTEGER Now;
			::QueryPerformanceCounter(&Now);
			LONGLONG CountsElapsed = Now.QuadPart - m_LastCounter;
			if (CountsElapsed < m_PCCountsPerPeriod)
				{
				LONGLONG CountsToWait = m_PCCountsPerPeriod - CountsElapsed;
				::Sleep((DWORD)(1000 * CountsToWait / m_PCFreq));
				}
			}
		else
			{
			LONGLONG TimeElapsed = ::GetTickCount() - m_LastCounter;
			if (TimeElapsed < m_dwPeriod)
				::Sleep(m_dwPeriod - (DWORD)TimeElapsed);
			}
		}

	if (m_PCFreq)
		{
		LARGE_INTEGER Count;
		::QueryPerformanceCounter(&Count);
		m_LastCounter = Count.QuadPart;
		}
	else
		m_LastCounter = ::GetTickCount();
	}
