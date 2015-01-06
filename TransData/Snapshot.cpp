//	Snapshot.cpp
//
//	Takes a snapshot of an object

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

void GenerateSnapshot (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	ALERROR error;
	int i;

	//	Get some parameters

	int iInitialUpdateTime = 10;
	int iUpdateTime = pCmdLine->GetAttributeInteger(CONSTLIT("wait"));

	//	Criteria

	CString sNode = pCmdLine->GetAttribute(CONSTLIT("node"));
	CString sCriteria = pCmdLine->GetAttribute(CONSTLIT("criteria"));

	//	Output

	int cxWidth;
	int cyHeight;
	if (pCmdLine->FindAttributeInteger(CONSTLIT("size"), &cxWidth))
		{
		cyHeight = cxWidth;
		}
	else
		{
		cxWidth = 1024;
		cyHeight = 1024;
		}

	//	Paint flags

	DWORD dwPaintFlags = 0;
	if (pCmdLine->GetAttributeBool(CONSTLIT("noStars")))
		dwPaintFlags |= CSystem::VWP_NO_STAR_FIELD;

	//	Output file

	CString sFilespec = pCmdLine->GetAttribute(CONSTLIT("output"));
	if (!sFilespec.IsBlank())
		sFilespec = pathAddExtensionIfNecessary(sFilespec, CONSTLIT(".bmp"));

	//	Update context

	SSystemUpdateCtx Ctx;
	Ctx.bForceEventFiring = true;
	Ctx.bForcePainted = true;

	//	Loop over all systems until we find what we're looking for

	int iLoops = 20;
	int iNodeIndex = 0;
	CTopologyNode *pNode = Universe.GetTopologyNode(iNodeIndex);
	while (true)
		{
		//	Create the system

		CSystem *pSystem;
		if (error = Universe.CreateStarSystem(pNode, &pSystem))
			{
			printf("ERROR: Unable to create star system.\n");
			return;
			}

		//	If this is the node we want, then search

		CSpaceObject *pTarget;
		if (sNode.IsBlank() || strEquals(sNode, pNode->GetID()))
			{
			printf("Searching %s...\n", pNode->GetSystemName().GetASCIIZPointer());

			//	Set the POV

			CSpaceObject *pPOV = pSystem->GetObject(0);
			Universe.SetPOV(pPOV);
			pSystem->SetPOVLRS(pPOV);

			//	Prepare system

			Universe.UpdateExtended();
			Universe.GarbageCollectLibraryBitmaps();

			//	Update for a while

			for (i = 0; i < iInitialUpdateTime; i++)
				Universe.Update(Ctx);

			//	Compose the criteria

			CSpaceObject::Criteria Criteria;
			CSpaceObject::ParseCriteria(pPOV, sCriteria, &Criteria);

			//	Get the list of all objects in the system that match the criteria

			CSpaceObject::SCriteriaMatchCtx Ctx(Criteria);
			TArray<CSpaceObject *> Results;
			for (i = 0; i < pSystem->GetObjectCount(); i++)
				{
				CSpaceObject *pObj = pSystem->GetObject(i);
				if (pObj && pObj->MatchesCriteria(Ctx, Criteria))
					Results.Insert(pObj);
				}

			//	Pick the appropriate object from the list

			if (Results.GetCount() == 0)
				pTarget = NULL;
			else if (Criteria.bNearestOnly || Criteria.bFarthestOnly)
				pTarget = Ctx.pBestObj;
			else
				pTarget = Results[mathRandom(0, Results.GetCount() - 1)];
			}
		else
			pTarget = NULL;

		//	If we found the target, then output

		if (pTarget)
			{
			//	If we found the target, take a snapshot

			printf("Found %s.\n", pTarget->GetNounPhrase(0).GetASCIIZPointer());

			//	Wait a bit

			for (i = 0; i < iUpdateTime; i++)
				{
				if ((i % 100) == 99)
					printf(".");

				Universe.Update(Ctx);
				}

			if (iUpdateTime >= 99)
				printf("\n");

			//	Paint

			CG16bitImage Output;
			Output.CreateBlank(cxWidth, cyHeight, false);
			RECT rcViewport;
			rcViewport.left = 0;
			rcViewport.top = 0;
			rcViewport.right = cxWidth;
			rcViewport.bottom = cyHeight;
			
			pSystem->PaintViewport(Output, rcViewport, pTarget, dwPaintFlags);

			//	Write to file

			if (!sFilespec.IsBlank())
				{
				CFileWriteStream OutputFile(sFilespec);
				if (OutputFile.Create() != NOERROR)
					{
					printf("ERROR: Unable to create '%s'\n", sFilespec.GetASCIIZPointer());
					return;
					}

				Output.WriteToWindowsBMP(&OutputFile);
				OutputFile.Close();
				printf("%s\n", sFilespec.GetASCIIZPointer());
				}

			//	Otherwise, clipboard

			else
				{
				if (error = Output.CopyToClipboard())
					{
					printf("ERROR: Unable to copy image to clipboard.\n");
					return;
					}

				printf("Image copied to clipboard.\n");
				}

			//	Done

			break;
			}

		//	Done with old system

		Universe.DestroySystem(pSystem);

		//	Loop to the next node

		do
			{
			iNodeIndex = ((iNodeIndex + 1) % Universe.GetTopologyNodeCount());
			pNode = Universe.GetTopologyNode(iNodeIndex);
			}
		while (pNode == NULL || pNode->IsEndGame());

		//	If we're back to the first node again, restart

		if (iNodeIndex == 0)
			{
			if (--iLoops > 0)
				{
				Universe.Reinit();

				iNodeIndex = 0;
				pNode = Universe.GetTopologyNode(iNodeIndex);
				}
			else
				{
				printf("ERROR: Specified target could not be found.\n");
				return;
				}
			}
		}
	}
