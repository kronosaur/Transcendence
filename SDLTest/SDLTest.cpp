// SDLTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main (int argc, char *argv[ ], char *envp[ ])

//	main
//
//	main entry-point

	{
	if (!kernelInit(KERNEL_FLAG_INTERNETS))
		{
		printf("ERROR: Unable to initialize Alchemy kernel.\n");
		return 1;
		}

	if (!sdlInit())
		{
		printf("ERROR: Unable to initialize SDL.\n");
		return 1;
		}

	//	Do it

	{

	}

	//	Done

	sdlCleanUp();
	kernelCleanUp();
	return 0;
	}

