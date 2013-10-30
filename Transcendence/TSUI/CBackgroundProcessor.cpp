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

	CString sError;
	if (pTask->HIInit(&sError) != NOERROR)
		return;

	STask *pNewTask = m_Tasks.Insert();
	pNewTask->iStatus = statusReady;
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

ALERROR CBackgroundProcessor::GetLastResult (CString *retsResult)

//	GetLastResult
//
//	Returns the result of the last task

	{
	CSmartLock Lock(m_cs);
	if (retsResult)
		*retsResult = m_sLastResult;
	return m_LastError;
	}

int CBackgroundProcessor::GetProgress (CString *retsActivity)

//	GetProgress
//
//	Returns the current progress status of the processor

	{
	CSmartLock Lock(m_cs);

	if (!m_bExecuting)
		{
		if (retsActivity)
			*retsActivity = NULL_STR;
		return -1;
		}

	if (retsActivity)
		*retsActivity = m_sCurActivity;

	return m_iPercentDone;
	}

ALERROR CBackgroundProcessor::Init (HWND hWnd, DWORD dwID)

//	Init
//
//	Initialize the background thread

	{
	m_hWnd = hWnd;
	m_dwID = dwID;

	m_bExecuting = false;
	m_hWorkAvailableEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hQuitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hBackgroundThread = ::kernelCreateThread(Thread, this);

	return NOERROR;
	}

void CBackgroundProcessor::ListenerDestroyed (IHICommand *pListener)

//	ListenerDestroyed
//
//	The listener has been destroyed, so we remove it from our notification
//	lists.

	{
	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < m_Tasks.GetCount(); i++)
		if (m_Tasks[i].pListener == pListener)
			m_Tasks[i].pListener = NULL;

	for (i = 0; i < m_GlobalListeners.GetCount(); i++)
		if (m_GlobalListeners[i].pListener == pListener)
			{
			m_GlobalListeners.Delete(i);
			i--;
			}
	}

void CBackgroundProcessor::OnTaskComplete (LPARAM pData)

//	OnTaskComplete
//
//	Handles a message posted by PostOnTaskComplete

	{
	int i;
	STaskCompleteMsg *pMsg = (STaskCompleteMsg *)pData;

	//	If we've got a task then it means that we need to finish processing it
	//	and calll the listener.

	if (pMsg)
		{
		m_cs.Lock();

		//	Look for the task in the array. We need to do this because the array may
		//	have changed while we were processing.

		bool bFound = false;
		STask theTask;
		for (i = 0; i < m_Tasks.GetCount(); i++)
			if (m_Tasks[i].pTask == pMsg->pTask)
				{
				theTask = m_Tasks[i];

				m_Tasks.Delete(i);
				bFound = true;
				break;
				}

		m_cs.Unlock();

		//	Outside the lock we call the listener. Since we are in the UI thread, we
		//	can guarantee that the listener is still around. [Only the UI thread
		//	destroys listeners.]

		if (bFound && theTask.pListener)
			theTask.pListener->HICommand(theTask.sCmd, theTask.pTask);

		//	Delete the task

		pMsg->pTask->HICleanUp();
		delete pMsg->pTask;
		delete pMsg;
		}

	//	Otherwise, this is a global message to all listeners who want to know 
	//	that there are no tasks left.

	else
		{
		//	No need to lock because the UI thread is the only one who touches
		//	m_GlobalListeners.

		for (i = 0; i < m_GlobalListeners.GetCount(); i++)
			m_GlobalListeners[i].pListener->HICommand(m_GlobalListeners[i].sCmd, NULL);

		//	Done with global listeners.

		m_GlobalListeners.DeleteAll();
		}
	}

void CBackgroundProcessor::PostOnAllTasksComplete (void)

//	PostOnAllTasksComplete
//
//	Post messages to the UI thread that all tasks are done

	{
	::PostMessage(m_hWnd, WM_HI_TASK_COMPLETE, m_dwID, (LPARAM)NULL);
	}

void CBackgroundProcessor::PostOnTaskComplete (IHITask *pTask)

//	PostOnTaskComplete
//
//	Called when a task finished. Note that this will be called on a background thread (and thus
//	we need to move it to the foreground)

	{
	//	Post a message to the foreground thread.

	STaskCompleteMsg *pMsg = new STaskCompleteMsg;
	pMsg->pTask = pTask;

	::PostMessage(m_hWnd, WM_HI_TASK_COMPLETE, m_dwID, (LPARAM)pMsg);
	}

void CBackgroundProcessor::SetProgress (const CString &sActivity, int iPercentDone)

//	SetProgress
//
//	This may be called from inside the execution of any task

	{
	CSmartLock Lock(m_cs);
	m_iPercentDone = iPercentDone;
	m_sCurActivity = sActivity;
	}

void CBackgroundProcessor::SetResult (ALERROR error, const CString &sResult)

//	SetResult
//
//	Sets the latest result

	{
	CSmartLock Lock(m_cs);
	m_LastError = error;
	m_sLastResult = sResult;
	}

bool CBackgroundProcessor::RegisterOnAllTasksComplete (IHICommand *pListener, const CString &sCmd)

//	RegisterOnAllTasksComplete
//
//	If there are tasks currently running then it registers the listener and 
//	calls it back (OnCommand) when all tasks have completed.
//
//	If no tasks are currently running then it returns FALSE.

	{
	CSmartLock Lock(m_cs);

	//	If there is no work available then we're done

	if (::WaitForSingleObject(m_hWorkAvailableEvent, 0) == WAIT_TIMEOUT)
		return false;

	//	Add this to the global listeners table

	SListener *pEntry = m_GlobalListeners.Insert();
	pEntry->pListener = pListener;
	pEntry->sCmd = (sCmd.IsBlank() ? CONSTLIT("cmdAllTasksDone") : sCmd);

	//	Done

	return true;
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
		if (m_Tasks[i].iStatus == statusReady)
			delete m_Tasks[i].pTask;

	//	We can delete this array because any tasks that are in progress will be
	//	deleted when the OnTaskComplete message arrives.

	m_Tasks.DeleteAll();
	m_cs.Unlock();

	//	Wait until we're done processing the current task

	::SetEvent(m_hQuitEvent);
	while (m_bExecuting)
		::Sleep(1000);
	}

DWORD WINAPI CBackgroundProcessor::Thread (LPVOID pData)

//	Thread
//
//	This is the background thread

	{
	int i;
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
			IHITask *pTask = NULL;

			for (i = 0; i < pThis->m_Tasks.GetCount(); i++)
				if (pThis->m_Tasks[i].iStatus == statusReady)
					{
					pTask = pThis->m_Tasks[i].pTask;
					pThis->m_Tasks[i].iStatus = statusProcessing;
					break;
					}

			pThis->m_cs.Unlock();

			//	Do the task

			if (pTask)
				{
				pThis->m_bExecuting = true;
				pThis->SetProgress(CONSTLIT("Running"), 0);

				CString sResult;
				ALERROR error;
				try
					{
					error = pTask->HIExecute(pThis, &sResult);
					}
				catch (...)
					{
					sResult = CONSTLIT("Crash executing task.");
					error = ERR_FAIL;
					}

				pThis->SetProgress(NULL_STR, -1);
				pThis->SetResult(error, (sResult.IsBlank() ? CONSTLIT("Done") : sResult));
				pThis->m_bExecuting = false;

				//	When we're done, call m_HI so that it can do
				//	a notification on the UI thread.

				pThis->PostOnTaskComplete(pTask);
				}

			//	If there are no more tasks, reset

			pThis->m_cs.Lock();
			bool bTasksLeft = false;
			for (i = 0; i < pThis->m_Tasks.GetCount(); i++)
				if (pThis->m_Tasks[i].iStatus == statusReady)
					{
					bTasksLeft = true;
					break;
					}

			if (!bTasksLeft)
				{
				pThis->PostOnAllTasksComplete();
				::ResetEvent(pThis->m_hWorkAvailableEvent);
				}
			pThis->m_cs.Unlock();
			}
		}
	}
