//	CTimerRegistry.cpp
//
//	CTimerRegistry class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DWORD CTimerRegistry::AddTimer (HWND hWnd, DWORD dwMilliseconds, IHICommand *pListener, const CString &sCmd, bool bRecurring)

//	AddTimer
//
//	Adds a timer and returns an ID.

	{
	if (!::SetTimer(hWnd, m_dwNextID, dwMilliseconds, NULL))
		return 0;

	SEntry *pEntry = m_Timers.Insert();
	pEntry->dwID = m_dwNextID++;
	pEntry->pListener = pListener;
	pEntry->sCmd = sCmd;
	pEntry->bRecurring = bRecurring;

	return pEntry->dwID;
	}

void CTimerRegistry::DeleteTimer (HWND hWnd, DWORD dwID)

//	DeleteTimer
//
//	Deletes a timer by ID.

	{
	int i;

	for (i = 0; i < m_Timers.GetCount(); i++)
		if (m_Timers[i].dwID == dwID)
			{
			::KillTimer(hWnd, dwID);
			m_Timers.Delete(i);
			return;
			}
	}

void CTimerRegistry::FireTimer (HWND hWnd, DWORD dwID)

//	FireTimer
//
//	Fires the timer event.

	{
	int i;

	for (i = 0; i < m_Timers.GetCount(); i++)
		if (m_Timers[i].dwID == dwID)
			{
			m_Timers[i].pListener->HICommand(m_Timers[i].sCmd);
			if (!m_Timers[i].bRecurring)
				{
				::KillTimer(hWnd, dwID);
				m_Timers.Delete(i);
				}

			return;
			}
	}

void CTimerRegistry::ListenerDestroyed (HWND hWnd, IHICommand *pListener)

//	ListenerDestroyed
//
//	A listener has been destroyed.

	{
	int i;

	for (i = 0; i < m_Timers.GetCount(); i++)
		if (m_Timers[i].pListener == pListener)
			{
			::KillTimer(hWnd, m_Timers[i].dwID);
			m_Timers.Delete(i);
			i--;
			}
	}
