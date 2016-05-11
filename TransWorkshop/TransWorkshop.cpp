//	TransWorkshop
//
//	This program is used to publish extensions to the Steam Workshop
//
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

int main (int argc, char *argv[ ], char *envp[ ])

//	main
//
//	main entry-point

	{
	if (!kernelInit(KERNEL_FLAG_INTERNETS))
		{
		printf("error : Unable to initialize Alchemy kernel.\n");
		return 1;
		}

	//	Do it

	{
	ALERROR error;
	CXMLElement *pCmdLine;
	if (error = CreateXMLElementFromCommandLine(argc, argv, &pCmdLine))
		{
		printf("error : Invalid command line.\n");
		kernelCleanUp();
		return 1;
		}

    CTransWorkshop App;
    App.Run(pCmdLine);

	delete pCmdLine;
	}

	//	Done

	kernelCleanUp();
	return 0;
	}
