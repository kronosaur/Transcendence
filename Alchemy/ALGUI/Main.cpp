//	Main.cpp
//
//	Main program file for Alchemy GUI program

#include "Alchemy.h"
#include "ALGUI.h"

static ALERROR DoIt (HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow);

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevIntance, LPSTR lpCmdLine, int nCmdShow)

//	WinMain
//
//	Main entry-point for GUI programs

	{
	//	Initialize the kernel

	if (!kernelInit())
		{
		MessageBox(NULL, "Alchemy: Unable to initialize kernel", "Alchemy", MB_OK | MB_ICONSTOP);
		return 0;
		}

	//	Do it. We need to add this as a separate function because
	//	we need to initialize the kernel before we allocate any
	//	interesting stack local variables (like CUApplication)

	if (DoIt(hInstance, lpCmdLine, nCmdShow) != NOERROR)
		{
		MessageBox(NULL, "Alchemy: Unable to initialize application", "Alchemy", MB_OK | MB_ICONSTOP);
		return 0;
		}

	//	Done

	kernelCleanUp();
	return 0;
	}

ALERROR DoIt (HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow)

//	DoIt
//
//	The actual program code and main loop

	{
	ALERROR error;
	CUApplication App(hInstance, lpCmdLine, nCmdShow);
	IUController *pMainController = NULL;
	CTextFileLog DebugLog(LITERAL("DebugLog.txt"));

	//	Start logging

#ifdef DEBUG
	kernelSetDebugLog(&DebugLog, TRUE);
#endif

	//	Get a pointer to the main controller

	pMainController = ALGUIMainController();
	if (pMainController == NULL)
		{
		kernelDebugLogMessage("Unable to get main controller");
		error = ERR_FAIL;
		goto Fail;
		}

	//	Boot the controller so that we create the main window

	if (error = pMainController->Boot(&App))
		{
		kernelDebugLogMessage("Error booting main controller");
		goto Fail;
		}

	//	Run

	if (error = App.Run(pMainController, 0))
		goto Fail;

	delete pMainController;

	//	Close the log

#ifdef DEBUG
	kernelSetDebugLog(NULL, FALSE);
#endif

	return NOERROR;

Fail:

	if (pMainController)
		delete pMainController;

#ifdef DEBUG
	kernelSetDebugLog(NULL, FALSE);
#endif

	return error;
	}

CString consoleInput (CString sPrompt)

//	consoleInput
//
//	Input a line of text

	{
	return LITERAL("");
	}

void consolePrint (CString sLine, ...)

//	consolePrint
//
//	Prints a string to the console

	{
	}
