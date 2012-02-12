//	LL1.cpp
//
//	Library Link: Phase 1

#include "PreComp.h"

#define MAINPAGE_ID							100
#define DEFAULT_CORE_FILENAME				"LibraryLink.cor"

CMainController::CMainController (void)

//	CMainController constructor

	{
	}

ALERROR CMainController::Boot (CUApplication *pApp)

//	Boot
//
//	Create the main window

	{
	ALERROR error;
	AutoSizeDesc AutoSize;
	CUFrame *pFrame;

	//	Set context

	m_pApp = pApp;

	//	Set icon

	m_pApp->SetIconRes("LL1");

	//	Don't open full screen if the shift-key is down

	DWORD dwFlags = WINDOW_FLAG_ANIMATE;
	if (!IsShiftDown())
		dwFlags |= WINDOW_FLAG_FULLSCREEN;

	//	Create the main window

	if (error = m_pApp->CreateNewWindow(this,
			0, 
			dwFlags,
			&m_pWindow))
		return error;

	m_pWindow->SetTitle(LITERAL("Library Link"));
	m_pWindow->SetQueryCloseMsg((ControllerNotifyProc)&CMainController::CmdQueryClose);

	//	Create the main page

	AutoSize.SetFull();
	if (error = m_pWindow->CreateFrame(NULL, MAINPAGE_ID, 0, &AutoSize, &pFrame))
		return error;

	m_pMainPage = new CLibraryPage(this);
	if (m_pMainPage == NULL)
		return ERR_MEMORY;

	if (error = m_pMainPage->Boot(pFrame))
		return error;

	//	Open the default core

	CString sPath = m_pApp->GetExecutablePath();
	sPath = pathAddComponent(sPath, LITERAL(DEFAULT_CORE_FILENAME));
	if (error = m_pMainPage->OpenCore(sPath))
		return error;

	return NOERROR;
	}

int CMainController::CmdQueryClose (DWORD dwTag, DWORD dwDummy2)

//	CmdQueryClose
//
//	Handles closing the main window

	{
	ALERROR error;

	//	Save out the core

	if (error = m_pMainPage->CloseCore())
#ifdef LATER
		report error
#else
		NULL;
#endif

	return (int)TRUE;
	}

BOOL CMainController::Notify (int iFromID, int iNotification, DWORD dwData, CObject *pFrom)

//	Notify
//
//	Handle a notification

	{
	return FALSE;
	}

int CMainController::msgDoCommand (DWORD dwDummy, DWORD dwDummy2)

//	msgDoCommand
//
//	This message is called when the user does a command

	{
	return 0;
	}

int CMainController::msgTimer (DWORD dwDummy, DWORD dwLastTickCount)

//	msgTimer
//
//	This message is called on each frame update

	{
	return 0;
	}

IUController *ALGUIMainController (void)

//	ALGUIMainController
//
//	Create the controller

	{
	return new CMainController;
	}
