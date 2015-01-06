//	TransBench
//
//	This program is used to test the performance of various routines
//
//	Copyright (c) 2007 by George Moromisato. All Rights Reserved.

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransBench.h"

#define NOARGS									CONSTLIT("noArgs")
#define SWITCH_HELP								CONSTLIT("help")
#define SWITCH_BLEND_TEST						CONSTLIT("blendTest")
#define SWITCH_PARTICLE_BLT_TEST				CONSTLIT("particleBltTest")

void TransBench (CXMLElement *pCmdLine)

//	TransBench
//
//	Run various tests

	{
	printf("TransBench v1.0\n");
	printf("Copyright (c) 2007 by George Moromisato. All Rights Reserved.\n\n");

	if (pCmdLine->GetAttributeBool(NOARGS) || pCmdLine->GetAttributeBool(SWITCH_HELP))
		{
		printf("  /blendTest            Tests the speed of pixel blending\n");
		printf("\n");
		return;
		}

	if (pCmdLine->GetAttributeBool(SWITCH_BLEND_TEST))
		TestPixelBlendComparison();
	else if (pCmdLine->GetAttributeBool(SWITCH_PARTICLE_BLT_TEST))
		TestParticleBltComparison();
	}

int main (int argc, char *argv[ ], char *envp[ ])

//	main
//
//	main entry-point

	{
	if (!kernelInit())
		{
		printf("ERROR: Unable to initialize Alchemy kernel.\n");
		return 1;
		}

	//	Do it

	{
	ALERROR error;
	CXMLElement *pCmdLine;
	if (error = CreateXMLElementFromCommandLine(argc, argv, &pCmdLine))
		{
		printf("ERROR: Invalid command line.\n");
		kernelCleanUp();
		return 1;
		}

	TransBench(pCmdLine);

	delete pCmdLine;
	}

	//	Done

	kernelCleanUp();
	return 0;
	}

