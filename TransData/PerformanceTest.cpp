//	PerformanceTest.cpp
//
//	Test system performance

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

struct SSample
	{
	DWORD dwTime;				//	How long this sample took
	int iObjCount;				//	Number of objects at end of sample
	};

const int DEFAULT_UPDATE =		1000;
const int SAMPLE_SIZE =			300;

int GetValidObjCount (CSystem *pSystem);

void PerformanceTest (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i;
	int iTrial;

	TArray<DWORD> Trials;
	DWORD dwLastAverage;

	//	How many tests

	int iCount = pCmdLine->GetAttributeInteger(CONSTLIT("count"));
	if (iCount == 0)
		iCount = 1;

	//	How long do we update

	int iUpdateCount = pCmdLine->GetAttributeInteger(CONSTLIT("updateCount"));
	if (iUpdateCount == 0)
		iUpdateCount = DEFAULT_UPDATE;

	//	Create the fist system

	for (iTrial = 0; iTrial < iCount; iTrial++)
		{
		TArray<SSample> Timings;

		printf("Trial: %d\n", iTrial + 1);

		//	Initialize the universe

		CString sError;
		if (Universe.InitGame(0, &sError) != NOERROR)
			{
			printf("ERROR: %s", sError.GetASCIIZPointer());
			return;
			}

		//	Create the first system

		CTopologyNode *pNode = Universe.GetFirstTopologyNode();
		if (pNode == NULL)
			{
			printf("ERROR: Cannot find first node.\n");
			return;
			}

		CSystem *pSystem;
		if (Universe.CreateStarSystem(pNode, &pSystem) != NOERROR)
			{
			printf("ERROR: Unable to create star system.\n");
			return;
			}

		//	Set the POV

		CSpaceObject *pPOV = pSystem->GetObject(0);
		Universe.SetPOV(pPOV);
		pSystem->SetPOVLRS(pPOV);

		//	Prepare system

		Universe.UpdateExtended();
		Universe.GarbageCollectLibraryBitmaps();
		Universe.StartGame(true);

		//	Update context

		SSystemUpdateCtx Ctx;
		Ctx.bForceEventFiring = true;
		Ctx.bForcePainted = true;

		//	Update for a while

		DWORD dwStart = ::GetTickCount();
		for (i = 0; i < iUpdateCount; i++)
			{
			Universe.Update(Ctx);

			if (i > 0 && (i % SAMPLE_SIZE) == 0)
				{
				DWORD dwEnd = ::GetTickCount();
				DWORD dwTime = dwEnd - dwStart;
				SSample *pSample = Timings.Insert();
				pSample->dwTime = dwTime;
				pSample->iObjCount = GetValidObjCount(pSystem);

				CString sTime = strFormatMilliseconds(dwTime);
				CString sObjTime = strFormatMicroseconds(1000 * dwTime / pSample->iObjCount);
				printf("Objs: %d  Total time: %s  Per obj: %s\n", 
						pSample->iObjCount, 
						sTime.GetASCIIZPointer(),
						sObjTime.GetASCIIZPointer());

				dwStart = ::GetTickCount();
				}
			}

		//	Compute trial average

		DWORD dwTotal = 0;
		DWORD dwAverage = 0;
		for (i = 0; i < Timings.GetCount(); i++)
			dwTotal += Timings[i].dwTime;

		if (Timings.GetCount() > 0)
			{
			dwAverage = dwTotal / Timings.GetCount();
			CString sTime = strFormatMilliseconds(dwAverage);
			printf("Trial average: %s\n", sTime.GetASCIIZPointer());
			}

		Trials.Insert(dwAverage);

		//	Compute total average

		dwTotal = 0;
		for (i = 0; i < Trials.GetCount(); i++)
			dwTotal += Trials[i];

		dwLastAverage = dwTotal / Trials.GetCount();
		CString sTime = strFormatMilliseconds(dwLastAverage);
		printf("\n");
		printf("Running average: %s\n", sTime.GetASCIIZPointer());
		printf("\n");
		}

	//	Final tally

	CString sTime = strFormatMilliseconds(dwLastAverage);
	CString sTime2 = strFormatMicroseconds(1000 * dwLastAverage / SAMPLE_SIZE);

	printf("Total updates: %d\n", iUpdateCount * iCount);
	printf("Average time for %d updates: %s\n", SAMPLE_SIZE, sTime.GetASCIIZPointer());
	printf("Average time per update: %s\n", sTime2.GetASCIIZPointer());
	}

int GetValidObjCount (CSystem *pSystem)
	{
	int i;
	int iCount = 0;

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);
		if (pObj && !pObj->IsDestroyed())
			iCount++;
		}

	return iCount;
	}