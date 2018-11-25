//	SmokeTest.cpp
//
//	Test system creation

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define DEFAULT_SYSTEM_SAMPLE				100
#define DEFAULT_UPDATES						1000

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

	//	Options

	int iSystemSample = pCmdLine->GetAttributeIntegerBounded(CONSTLIT("count"), 1, -1, DEFAULT_SYSTEM_SAMPLE);
	int iSystemUpdateTime = pCmdLine->GetAttributeIntegerBounded(CONSTLIT("updates"), 0, -1, DEFAULT_UPDATES);
	bool bNoDiagnostics = pCmdLine->GetAttributeBool(CONSTLIT("noDiagnostics"));

	//	Update context

	SSystemUpdateCtx Ctx;
	Ctx.bForceEventFiring = true;
	Ctx.bForcePainted = true;

	DWORD dwUpdateTime = 0;
	DWORD dwUpdateCount = 0;

	//	Generate systems for multiple games

	for (i = 0; i < iSystemSample; i++)
		{
		bool bRunStartDiag = !bNoDiagnostics;
		int iSample = i + 1;
		printf("SAMPLE %d\n", iSample);

		//	Create all systems

		TSortMap<CString, CSystem *> AllSystems;
		for (int iNode = 0; iNode < Universe.GetTopologyNodeCount(); iNode++)
			{
			CTopologyNode *pNode = Universe.GetTopologyNode(iNode);
			if (pNode == NULL || pNode->IsEndGame())
				continue;

			printf("%s\n", (LPSTR)pNode->GetSystemName());

			//	Create the system

			CSystem *pSystem;
			if (error = Universe.CreateStarSystem(pNode, &pSystem, &sError))
				{
				printf("ERROR: Unable to create star system: %s.\n", (LPSTR)sError);
				return;
				}

			AllSystems.SetAt(pNode->GetID(), pSystem);
			}

		//	Now update all system

		for (int iSystem = 0; iSystem < AllSystems.GetCount(); iSystem++)
			{
			CSystem *pSystem = AllSystems[iSystem];

			//	Set the POV

			CSpaceObject *pPOV = pSystem->GetObject(0);
			Universe.SetPOV(pPOV);
			pSystem->SetPOVLRS(pPOV);

			//	Prepare system

			Universe.UpdateExtended();
			Universe.GarbageCollectLibraryBitmaps();

			//	Run diagnostics start

			if (bRunStartDiag)
				{
				printf("\nSTART DIAGNOSTICS\n");
				Universe.GetDesignCollection().FireOnGlobalStartDiagnostics();
				bRunStartDiag = false;
				}

			//	Update for a while

			for (j = 0; j < iSystemUpdateTime; j++)
				{
				dwUpdateCount++;
				DWORD dwStart = ::GetTickCount();

				Universe.Update(Ctx);

				dwUpdateTime += ::sysGetTicksElapsed(dwStart);
				}

			//	Run diagnostics code

			if (!bNoDiagnostics)
				{
				printf("DIAGNOSTICS: %s\n", (LPSTR)pSystem->GetName());
				Universe.GetDesignCollection().FireOnGlobalSystemDiagnostics();
				}

			//	Done with system

			Universe.DestroySystem(pSystem);
			}

		if (!bNoDiagnostics)
			{
			printf("END DIAGNOSTICS\n");
			Universe.GetDesignCollection().FireOnGlobalEndDiagnostics();
			}

		Universe.Reinit();

		if (Universe.InitGame(0, &sError) != NOERROR)
			{
			printf("ERROR: %s\n", sError.GetASCIIZPointer());
			return;
			}

		printf("-------------------------------------------------------------------------------\n");
		}

	//	Print update performance

	if (dwUpdateCount > 0)
		{
		Metric rTime = (Metric)dwUpdateTime / dwUpdateCount;
		CString sUpdates = strFormatInteger((int)dwUpdateCount, -1, FORMAT_THOUSAND_SEPARATOR);
		printf("Average time per update: %.2f ms [%s updates]\n", rTime, (LPSTR)sUpdates);
		}

	//	Done

	printf("Test complete.\n");
	}

