//	CExplorerController.cpp
//
//	CExplorerController class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

#define CMD_LOAD_UNIVERSE_DONE							CONSTLIT("cmdLoadUniverseDone")

ALERROR CExplorerController::OnBoot (char *pszCommandLine, SHIOptions &Options)

//	OnBoot
//
//	Boot up

	{
#if 0
	::CreateWindowEx(0,
			"edit",
			"",
			WS_CHILD | WS_VISIBLE,
			0, 0, 500, 200,
			m_HI.GetHWND(),
			NULL,
			NULL,
			NULL);
#endif

	Options.m_bWindowedMode = true;

	return NOERROR;
	}

void CExplorerController::OnCleanUp (void)

//	OnCleanUp
//
//	Clean up

	{
	m_Model.CleanUp();
	}

ALERROR CExplorerController::OnCommand (const CString &sCmd, void *pData)

//	OnCommand
//
//	Handle HI commands

	{
	ALERROR error;

	if (strEquals(sCmd, CMD_LOAD_UNIVERSE_DONE))
		{
		//	Check for error

		CLoadUniverseTask *pTask = (CLoadUniverseTask *)pData;
		CString sError;
		if (error = pTask->GetResult(&sError))
			{
			m_HI.ShowHardCrashSession(CONSTLIT("Unable to Load"), sError);
			return error;
			}

		//	Start the picker session

#if 0
		CBattlesSession *pBattles = new CBattlesSession(m_HI, m_Model);
		if (error = m_HI.ShowSession(pBattles))
			{
			m_HI.ShowHardCrashSession(CONSTLIT("Start Failed"), CONSTLIT("Unable to start the battles session"));
			return error;
			}
#endif
		}

	return NOERROR;
	}

ALERROR CExplorerController::OnInit (void)

//	OnInit
//
//	Initialize

	{
	ALERROR error;

	//	Initialize the model

	if (error = m_Model.Init())
		return error;

	//	Create a task to load the universe in the background
	//	m_HI owns the task

	CLoadUniverseTask *pTask = new CLoadUniverseTask(m_HI, m_Model);
	m_HI.AddBackgroundTask(pTask, 0, this, CMD_LOAD_UNIVERSE_DONE);

#if 0
	//	Show the loading screen
	//	m_HI owns the session

	CLoadingSession *pSession = new CLoadingSession(m_HI, m_Model);
	m_HI.ShowSession(pSession);
#endif

	return NOERROR;
	}
