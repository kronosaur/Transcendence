//	CTransSaverController.cpp
//
//	CTransSaverController class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

#define CMD_LOAD_UNIVERSE_DONE							CONSTLIT("cmdLoadUniverseDone")

ALERROR CTransSaverController::OnBoot (char *pszCommandLine, SHIOptions &Options)

//	OnBoot
//
//	Set options

	{
	Options.m_bMultiMonitorMode = true;
#ifdef DEBUG
	Options.m_bForceNonDX = true;
#endif
	return NOERROR;
	}

void CTransSaverController::OnCleanUp (void)

//	OnCleanUp
//
//	Clean up

	{
	m_Model.CleanUp();
	}

ALERROR CTransSaverController::OnCommand (const CString &sCmd, void *pData)

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

		//	Start the battles

		CBattlesSession *pBattles = new CBattlesSession(m_HI, m_Model);
		if (error = m_HI.ShowSession(pBattles))
			{
			m_HI.ShowHardCrashSession(CONSTLIT("Start Failed"), CONSTLIT("Unable to start the battles session"));
			return error;
			}
		}

	return NOERROR;
	}

ALERROR CTransSaverController::OnInit (CString *retsError)

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

	//	Show the loading screen
	//	m_HI owns the session

	CLoadingSession *pSession = new CLoadingSession(m_HI, m_Model);
	m_HI.ShowSession(pSession);

	return NOERROR;
	}
