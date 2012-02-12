//	STBrowser.cpp
//
//	Main Application
//	(c) Copyright 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"
#include "STBrowser.h"

int WINAPI WinMain (HINSTANCE hInstance, 
					HINSTANCE hPrevInstance, 
					LPSTR lpCmdLine, 
                    int nCmdShow)

//	WinMain
//
//	Main Windows entry-point

	{
	//	Initialize alchemy

	if (!kernelInit(0))
		return 0;

	//	Run the application

	int iResult = HIRunApplication(hInstance, nCmdShow, lpCmdLine, new CSTBrowserController);

	//	Done

	kernelCleanUp();

	return iResult;
	}
