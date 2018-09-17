//	Snapshot.cpp
//
//	Takes a snapshot of an object

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

void GenerateSnapshot (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	ALERROR error;
	int i;

	//	Get some parameters

	int iInitialUpdateTime = pCmdLine->GetAttributeIntegerBounded(CONSTLIT("initialUpdate"), 0, -1, 10);
	int iUpdateTime = pCmdLine->GetAttributeInteger(CONSTLIT("wait"));
	bool bObjOnly = pCmdLine->GetAttributeBool(CONSTLIT("objOnly"));

	//	Criteria

	CString sNode = pCmdLine->GetAttribute(CONSTLIT("node"));
	CString sCriteria = pCmdLine->GetAttribute(CONSTLIT("criteria"));

	//	Number of snapshots

	int iTotalCount = pCmdLine->GetAttributeIntegerBounded(CONSTLIT("count"), 1, -1, 1);

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
		sFilespec = pathStripExtension(sFilespec);

	//	Output image

	CG32bitImage Output;
	Output.Create(cxWidth, cyHeight);

	//	Update context

	SSystemUpdateCtx Ctx;
	Ctx.bForceEventFiring = true;
	Ctx.bForcePainted = true;

	RECT rcViewport;
	rcViewport.left = 0;
	rcViewport.top = 0;
	rcViewport.right = cxWidth;
	rcViewport.bottom = cyHeight;

	//	Loop over all systems until we find what we're looking for

	int iLoops = 20;
	int iNodeIndex = 0;
	int iSnapshotIndex = 0;
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
				{
				Universe.Update(Ctx);
				Universe.PaintPOV(Output, rcViewport, 0);
				}

			//	Compose the criteria

			CSpaceObjectCriteria Criteria(pPOV, sCriteria);

			//	Get the list of all objects in the system that match the criteria

			CSpaceObjectCriteria::SCtx Ctx(Criteria);
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
			else if (Criteria.MatchesNearestOnly() || Criteria.MatchesFarthestOnly())
				pTarget = Ctx.pBestObj;
			else
				pTarget = Results[mathRandom(0, Results.GetCount() - 1)];
			}
		else
			pTarget = NULL;

		//	If we found the target, then output

		if (pTarget)
			{
			Universe.SetPOV(pTarget);

			//	Wait a bit
			//
			//	NOTE: After we update, pTarget could be invalid (i.e., destroyed)
			//	so we can't use it again.

			CString sTargetName = pTarget->GetNounPhrase(0);

			for (i = 0; i < iUpdateTime; i++)
				{
				if ((i % 100) == 99)
					printf(".");

				Universe.Update(Ctx);
				Universe.PaintPOV(Output, rcViewport, 0);
				}

			if (iUpdateTime >= 99)
				printf("\n");

			//	Paint

			if (bObjOnly)
				{
				SViewportPaintCtx Ctx;
				Ctx.pObj = Universe.GetPOV();
				Ctx.XForm = ViewportTransform(Universe.GetPOV()->GetPos(), 
						g_KlicksPerPixel, 
						cxWidth / 2, 
						cyHeight / 2);
				Ctx.XFormRel = Ctx.XForm;
				Ctx.fNoRecon = true;
				Ctx.fNoDockedShips = true;
				Ctx.fNoSelection = true;
				Ctx.fNoStarfield = true;

				CSpaceObject *pPOV = pSystem->GetObject(0);
				CSpaceObjectCriteria Criteria(pPOV, sCriteria);

				//	Paint all objects that match the criteria

				CSpaceObjectCriteria::SCtx CriteriaCtx(Criteria);
				for (i = 0; i < pSystem->GetObjectCount(); i++)
					{
					CSpaceObject *pObj = pSystem->GetObject(i);
					if (pObj && pObj->MatchesCriteria(CriteriaCtx, Criteria))
						{
						Ctx.pObj = pObj;
						int xObj;
						int yObj;
						Ctx.XForm.Transform(pObj->GetPos(), &xObj, &yObj);

						pObj->Paint(Output, xObj, yObj, Ctx);
						}
					}
				}
			else
				{
			
				Universe.PaintPOV(Output, rcViewport, 0);
				}

			//	Write to file

			if (!sFilespec.IsBlank())
				{
				CString sBmpFilespec;
				if (iTotalCount > 100)
					sBmpFilespec = pathAddExtensionIfNecessary(strPatternSubst(CONSTLIT("%s%03d"), sFilespec, iSnapshotIndex + 1), CONSTLIT(".bmp"));
				else if (iTotalCount > 1)
					sBmpFilespec = pathAddExtensionIfNecessary(strPatternSubst(CONSTLIT("%s%02d"), sFilespec, iSnapshotIndex + 1), CONSTLIT(".bmp"));
				else
					sBmpFilespec = pathAddExtensionIfNecessary(sFilespec, CONSTLIT(".bmp"));

				CFileWriteStream OutputFile(sBmpFilespec);
				if (OutputFile.Create() != NOERROR)
					{
					printf("ERROR: Unable to create '%s'\n", sBmpFilespec.GetASCIIZPointer());
					return;
					}

				Output.WriteToWindowsBMP(&OutputFile);
				OutputFile.Close();
				printf("Found %s: Saved to %s\n", sTargetName.GetASCIIZPointer(), sBmpFilespec.GetASCIIZPointer());
				}

			//	Otherwise, clipboard

			else
				{
				if (!Output.CopyToClipboard())
					{
					printf("ERROR: Unable to copy image to clipboard.\n");
					return;
					}

				printf("Found %s: Copied to clipboard.\n", sTargetName.GetASCIIZPointer());
				}

			//	Reset maximum loops

			iLoops = 20;

			//	Done

			iSnapshotIndex++;
			if (iSnapshotIndex >= iTotalCount)
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
				//	Reinitialize

				Universe.Reinit();
				CString sError;
				if (Universe.InitGame(0, &sError) != NOERROR)
					{
					printf("ERROR: %s\n", sError.GetASCIIZPointer());
					return;
					}

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
