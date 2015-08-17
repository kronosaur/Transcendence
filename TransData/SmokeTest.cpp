//	SmokeTest.cpp
//
//	Test system creation

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define DEFAULT_SYSTEM_SAMPLE				100

void DoRandomNumberTest (void)
	{
	int i;
	int Results[100];
	for (i = 0; i < 100; i++) 
		Results[i] = 0;

	for (i = 0; i < 1000000; i++)
		Results[mathRandom(1, 100)-1]++;

	for (i = 0; i < 100; i++)
		printf("%d: %d\n", i, Results[i]);
	}

void DoSmokeTest (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	ALERROR error;
	CString sError;
	int i, j;

	int iSystemSample = pCmdLine->GetAttributeInteger(CONSTLIT("count"));
	if (iSystemSample == 0)
		iSystemSample = DEFAULT_SYSTEM_SAMPLE;

	int iSystemUpdateTime = 1000;

	//	Update context

	SSystemUpdateCtx Ctx;
	Ctx.bForceEventFiring = true;
	Ctx.bForcePainted = true;

	//	Generate systems for multiple games

	CSymbolTable AllSystems(TRUE, TRUE);
	for (i = 0; i < iSystemSample; i++)
		{
		printf("sample %d", i+1);

		CTopologyNode *pNode = Universe.GetFirstTopologyNode();

		while (true)
			{
			//	Create the system

			CSystem *pSystem;
			if (error = Universe.CreateStarSystem(pNode, &pSystem, &sError))
				{
				printf("ERROR: Unable to create star system: %s.\n", (LPSTR)sError);
				return;
				}

			//	Set the POV

			CSpaceObject *pPOV = pSystem->GetObject(0);
			Universe.SetPOV(pPOV);
			pSystem->SetPOVLRS(pPOV);

			//	Prepare system

			Universe.UpdateExtended();
			Universe.GarbageCollectLibraryBitmaps();

			//	Update for a while

			for (j = 0; j < iSystemUpdateTime; j++)
				Universe.Update(Ctx);

			//	Get the next node

			CString sEntryPoint;
			pNode = pSystem->GetStargateDestination(CONSTLIT("Outbound"), &sEntryPoint);
			if (pNode == NULL || pNode->IsEndGame())
				break;

			//	Done with old system

			Universe.DestroySystem(pSystem);

			printf(".");
			}

		Universe.Reinit();
		printf("\n");
		}

	//	Done

	printf("Test successful.\n");
	}

