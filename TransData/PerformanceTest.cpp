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
void TestPolygons (CUniverse &Universe, CXMLElement *pCmdLine);
void TestUpdate (CUniverse &Universe, CXMLElement *pCmdLine);

void PerformanceTest (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	if (pCmdLine->FindAttribute(CONSTLIT("testPolygons")))
		TestPolygons(Universe, pCmdLine);
	else
		TestUpdate(Universe, pCmdLine);
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

void TestPolygons (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i;

	const int FRAME_WIDTH = 256;
	const int FRAME_HEIGHT = 256;
	const int OUTPUT_WIDTH = FRAME_WIDTH * 2;
	const int OUTPUT_HEIGHT = FRAME_HEIGHT;

	//	Options

	int iCount = pCmdLine->GetAttributeInteger(CONSTLIT("count"));
	if (iCount == 0)
		iCount = 1000;

	int iBltCount = Max(1, iCount / 10);

	//	Create the output image

	CG32bitImage Output;
	Output.Create(OUTPUT_WIDTH, OUTPUT_HEIGHT, CG32bitImage::alpha8);

	//	Create a regular polygon

	const int iSides = 17;
	const Metric rRadius = 100.0;
	const Metric rAngleStep = (TAU / iSides);

	TArray<CVector> Shape1;
	Shape1.InsertEmpty(iSides);
	for (i = 0; i < iSides; i++)
		Shape1[i] = CVector::FromPolar(i * rAngleStep, rRadius);

	//	Create a point array, which we'll use for the binary region

	SPoint Shape1Points[iSides];
	for (i = 0; i < iSides; i++)
		{
		Shape1Points[i].x = (int)Shape1[i].GetX();
		Shape1Points[i].y = (int)Shape1[i].GetY();
		}

	//	Create a path

	CGPath Shape1Path;
	Shape1Path.Init(Shape1);

	//	We do timing tests first

	TNumberSeries<Metric> Timing;

	//	Time rasterization of a binary region

	const int CALLS_PER_SAMPLE = 1000;
	const int BLTS_PER_SAMPLE = 100;
	DWORD dwStart = ::GetTickCount();
	for (i = 0; i < iCount; i++)
		{
		CG16bitBinaryRegion Region;
		Region.CreateFromPolygon(iSides, Shape1Points);

		if (((i + 1) % CALLS_PER_SAMPLE) == 0)
			Timing.Insert((Metric)::sysGetTicksElapsed(dwStart, &dwStart));
		}

	printf("CG16bitBinaryRegion::CreateFromPolygon: %s ms per %d\n", (LPSTR)strFromDouble(Timing.GetMean()), CALLS_PER_SAMPLE);

	//	Time rasterization of path

	Timing.DeleteAll();
	dwStart = ::GetTickCount();
	for (i = 0; i < iCount; i++)
		{

		CGRegion Region;
		Shape1Path.Rasterize(&Region, 1);

		if (((i + 1) % CALLS_PER_SAMPLE) == 0)
			Timing.Insert((Metric)::sysGetTicksElapsed(dwStart, &dwStart));
		}

	printf("CGPath::Rasterize: %s ms per %d\n", (LPSTR)strFromDouble(Timing.GetMean()), CALLS_PER_SAMPLE);

	//	Create the regions

	CG16bitBinaryRegion Shape1BinaryRegion;
	Shape1BinaryRegion.CreateFromPolygon(iSides, Shape1Points);

	CGRegion Shape1Region;
	Shape1Path.Rasterize(&Shape1Region, 4);

	//	Time to blt

	Timing.DeleteAll();
	dwStart = ::GetTickCount();
	for (i = 0; i < iBltCount; i++)
		{
		CGDraw::Region(Output, (FRAME_WIDTH / 2), (FRAME_HEIGHT / 2), Shape1BinaryRegion, CG32bitPixel(255, 255, 255));

		if (((i + 1) % BLTS_PER_SAMPLE) == 0)
			Timing.Insert((Metric)::sysGetTicksElapsed(dwStart, &dwStart));
		}

	printf("CGDraw::Region (CG16bitBinaryRegion): %s ms per %d\n", (LPSTR)strFromDouble(Timing.GetMean()), BLTS_PER_SAMPLE);

	Timing.DeleteAll();
	dwStart = ::GetTickCount();
	for (i = 0; i < iBltCount; i++)
		{
		CGDraw::Region(Output, (FRAME_WIDTH / 2), (FRAME_HEIGHT / 2), Shape1Region, CG32bitPixel(255, 255, 255));

		if (((i + 1) % BLTS_PER_SAMPLE) == 0)
			Timing.Insert((Metric)::sysGetTicksElapsed(dwStart, &dwStart));
		}

	printf("CGDraw::Region (CGRegion): %s ms per %d\n", (LPSTR)strFromDouble(Timing.GetMean()), BLTS_PER_SAMPLE);

	//	Clear

	Output.Fill(CG32bitPixel(0, 0, 0));

	//	Blt result

	int x = 0;
	int y = 0;
	CGDraw::Region(Output, x + (FRAME_WIDTH / 2), y + (FRAME_HEIGHT / 2), Shape1BinaryRegion, CG32bitPixel(255, 255, 255));

	x = FRAME_WIDTH;
	CGDraw::Region(Output, x + (FRAME_WIDTH / 2), y + (FRAME_HEIGHT / 2), Shape1Region, CG32bitPixel(255, 255, 255));

	//	Copy to clipboard

	OutputImage(Output, NULL_STR);
	}

void TestUpdate (CUniverse &Universe, CXMLElement *pCmdLine)
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
