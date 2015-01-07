//	Main.cpp
//
//	Main Windows program entry-point

#include "PreComp.h"
#include "Transcendence.h"

int WINAPI WinMain (HINSTANCE hInstance, 
					HINSTANCE hPrevInstance, 
					LPSTR lpCmdLine, 
                    int nCmdShow)

//	WinMain
//
//	Main Windows entry-point

	{
	//	Initialize the kernel

	if (!::kernelInit(KERNEL_FLAG_INTERNETS))
		{
		::MessageBox(NULL, "Unable to initialize Alchemy.", "Human Interface", MB_OK);
		return 0;
		}

	//	Create the controller

	IHIController *pController = new CTranscendenceController;

	//	Run

	CHumanInterface::Run(pController, hInstance, nCmdShow, lpCmdLine);

	//	Done

	kernelCleanUp();

	return 0;
	}
