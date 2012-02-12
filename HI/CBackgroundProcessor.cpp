//	CBackgroundProcessor.cpp
//
//	CBackgroundProcessir class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

void CBackgroundProcessor::AddTask (IHITask *pTask, IHICommand *pListener, const CString &sCmd)

//	AddTask
//
//	Adds a task to the background. We take ownership of pTask.
//	This is called on the foreground thread.

	{
	CSmartLock Lock(m_cs);

	ASSERT(IsInitialized());

	if (pTask->HIInit() != NOERROR)
		return;

	STask *pNewTask = m_Tasks.Insert();
	pNewTask->pTask = pTask;
	pNewTask->pListener = pListener;
	pNewTask->sCmd = (sCmd.IsBlank() ? CONSTLIT("cmdTaskDone") : sCmd);

	::SetEvent(m_hWorkAvailableEvent);
	}

void CBackgroundProcessor::CleanUp (void)

//	CleanUp
//
//	Terminate the thread

	{
	if (IsInitialized())
		{
		::SetEvent(m_hQuitEvent);
		::WaitForSingleObject(m_hBackgroundThread, INFINITE);
		m_hBackgroundThread = INVALID_HANDLE_VALUE;

		::CloseHandle(m_hWorkAvailableEvent);
		::CloseHandle(m_hQuitEvent);
		::CloseHandle(m_hBackgroundThread);
		}
	}

ALERROR CBackgroundProcessor::Init (HWND hWnd)

//	Init
//
//	Initialize the background thread

	{
	m_hWnd = hWnd;

	m_bExecuting = false;
	m_hWorkAvailableEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hQuitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hBackgroundThread = ::kernelCreateThread(Thread, this);

	return NOERROR;
	}

void CBackgroundProcessor::OnTaskComplete (LPARAM pData)

//	OnTaskComplete
//
//	Handles a message posted by PostOnTaskComplete

	{
	STaskCompleteMsg *pMsg = (STaskCompleteMsg *)pData;

	//	First call the listener

	if (pMsg->pListener)
		pMsg->pListener->HICommand(pMsg->sCmd, pMsg->pTask);

	//	Delete the task

	pMsg->pTask->HICleanUp();
	delete pMsg->pTask;

	//	Delete the message

	delete pMsg;
	}

void CBackgroundProcessor::PostOnTaskComplete (IHITask *pTask, IHICommand *pListener, const CString &sCmd)

//	PostOnTaskComplete
//
//	Called when a task finished. Note that this will be called on a background thread (and thus
//	we need to move it to the foreground)

	{
	STaskCompleteMsg *pMsg = new STaskCompleteMsg;
	pMsg->pTask = pTask;
	pMsg->pListener = pListener;
	pMsg->sCmd = sCmd;

	::PostMessage(m_hWnd, WM_HI_TASK_COMPLETE, 0, (LPARAM)pMsg);
	}

void CBackgroundProcessor::StopAll()

//	StopAll
//
//	Stop the background thread

	{
	int i;

	if (!IsInitialized())
		return;

	//	Delete all future tasks

	m_cs.Lock();
	for (i = 0; i < m_Tasks.GetCount(); i++)
		delete m_Tasks[i].pTask;

	m_Tasks.DeleteAll();
	m_cs.Unlock();

	//	Wait until we're done processing the current task

	while (m_bExecuting)
		::Sleep(1000);
	}

DWORD WINAPI CBackgroundProcessor::Thread (LPVOID pData)

//	Thread
//
//	This is the background thread

	{
	CBackgroundProcessor *pThis = (CBackgroundProcessor *)pData;

	while (true)
		{
		const DWORD QUIT_EVENT = WAIT_OBJECT_0;
		const DWORD WORK_EVENT = WAIT_OBJECT_0 + 1;

		//	Wait for something to happen

		HANDLE Events[2];
		Events[0] = pThis->m_hQuitEvent;
		Events[1] = pThis->m_hWorkAvailableEvent;
		DWORD dwResult = ::WaitForMultipleObjects(2, Events, FALSE, INFINITE);

		//	Do the work

		if (dwResult == QUIT_EVENT)
			return 0;

		else if (dwResult == WORK_EVENT)
			{
			//	Pull a task out of the array

			pThis->m_cs.Lock();
			STask theTask;
			if (pThis->m_Tasks.GetCount())
				{
				theTask = pThis->m_Tasks[0];
				pThis->m_Tasks.Delete(0);
				}
			else
				theTask.pTask = NULL;
			pThis->m_cs.Unlock();

			//	Do the task

			if (theTask.pTask)
				{
				pThis->m_bExecuting = true;
				theTask.pTask->HIExecute();
				pThis->m_bExecuting = false;
				}

			//	When we're done, call m_HI so that it can do
			//	a notification on the UI thread.

			pThis->PostOnTaskComplete(theTask.pTask, theTask.pListener, theTask.sCmd);

			//	If there are no more tasks, reset

			pThis->m_cs.Lock();
			if (pThis->m_Tasks.GetCount() == 0)
				::ResetEvent(pThis->m_hWorkAvailableEvent);
			pThis->m_cs.Unlock();
			}
		}
	}
