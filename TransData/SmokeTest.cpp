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

	DWORD dwUpdateTime = 0;
	DWORD dwUpdateCount = 0;

	//	Generate systems for multiple games

	CSymbolTable AllSystems(TRUE, TRUE);
	for (i = 0; i < iSystemSample; i++)
		{
		printf("sample %d", i+1);

		for (int iNode = 0; iNode < Universe.GetTopologyNodeCount(); iNode++)
			{
			CTopologyNode *pNode = Universe.GetTopologyNode(iNode);
			if (pNode == NULL || pNode->IsEndGame())
				continue;

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
				{
				dwUpdateCount++;
				DWORD dwStart = ::GetTickCount();

				Universe.Update(Ctx);

				dwUpdateTime += ::sysGetTicksElapsed(dwStart);
				}

			//	Done with old system

			Universe.DestroySystem(pSystem);

			printf(".");
			}

		Universe.Reinit();
		printf("\n");

		if (Universe.InitGame(0, &sError) != NOERROR)
			{
			printf("ERROR: %s\n", sError.GetASCIIZPointer());
			return;
			}
		}

	//	Print update performance

	if (dwUpdateCount > 0)
		{
		Metric rTime = (Metric)dwUpdateTime / dwUpdateCount;
		CString sUpdates = strFormatInteger((int)dwUpdateCount, -1, FORMAT_THOUSAND_SEPARATOR);
		printf("Average time per update: %.2f ms [%s updates]\n", rTime, (LPSTR)sUpdates);
		}

	//	Done

	printf("Test successful.\n");
	}

