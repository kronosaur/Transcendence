//	SystemImages.cpp
//
//	Generate a poster of system images.
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

struct SNodeEntry
	{
	CTopologyNode *pNode;					//	Node to draw
	CString sLabel;							//	Label
	};

const int DEFAULT_ZOOM =					25;
const int OFFSET_Y =						-16;

void GenerateSystemImages (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i, j;

	//	Image size

	int cxDesiredWidth;
	if (pCmdLine->FindAttributeInteger(CONSTLIT("width"), &cxDesiredWidth))
		cxDesiredWidth = Max(512, cxDesiredWidth);
	else
		cxDesiredWidth = 3100;

	//	Spacing

	int cxSpacing = pCmdLine->GetAttributeInteger(CONSTLIT("xSpacing"));
	int cxExtraMargin = pCmdLine->GetAttributeInteger(CONSTLIT("xMargin"));

	//	Font for text

	CString sTypeface;
	int iSize;
	bool bBold;
	bool bItalic;

	if (!CG16bitFont::ParseFontDesc(pCmdLine->GetAttribute(CONSTLIT("font")),
			&sTypeface,
			&iSize,
			&bBold,
			&bItalic))
		{
		sTypeface = CONSTLIT("Arial");
		iSize = 10;
		bBold = false;
		bItalic = false;
		}

	CG16bitFont NameFont;
	NameFont.Create(sTypeface, -PointsToPixels(iSize), bBold, bItalic);
	CG32bitPixel rgbNameColor = CG32bitPixel(255, 255, 255);

	//	Output file

	CString sFilespec = pCmdLine->GetAttribute(CONSTLIT("output"));
	if (!sFilespec.IsBlank())
		sFilespec = pathAddExtensionIfNecessary(sFilespec, CONSTLIT(".bmp"));

	//	Zoom

	Metric rZoom = (pCmdLine->GetAttributeIntegerBounded(CONSTLIT("zoom"), 1, 100, DEFAULT_ZOOM) / 100.0);
	Metric rScale = g_AU / (800.0 * rZoom);

	int cxImage = 1000;
	int cyImage = 800;

	//	Figure out how many nodes to show

	TArray<SNodeEntry> NodeList;
	for (i = 0; i < Universe.GetTopologyNodeCount(); i++)
		{
		CTopologyNode *pNode = Universe.GetTopologyNode(i);
		if (pNode->IsEndGame())
			continue;

		SNodeEntry *pEntry = NodeList.Insert();
		pEntry->pNode = pNode;
		pEntry->sLabel = strPatternSubst(CONSTLIT("%s (%08x)"), pNode->GetSystemName(), pNode->GetSystemTypeUNID());
		}
	
	if (NodeList.GetCount() == 0)
		{
		printf("ERROR: No systems found.\n");
		return;
		}

	//	Options

	int iInitialUpdateTime = 10;
	bool bTextBoxesOnly = false;

	//	Allocate an arranger that tracks where to paint each world.

	CImageArranger Arranger;

	//	Settings for the overall arrangement

	CImageArranger::SArrangeDesc Desc;
	Desc.cxDesiredWidth = Max(512, cxDesiredWidth - (2 * (cxSpacing + cxExtraMargin)));
	Desc.cxSpacing = cxSpacing;
	Desc.cxExtraMargin = cxExtraMargin;
	Desc.pHeader = &NameFont;

	//	Generate a table of cells for the arranger

	TArray<CCompositeImageSelector> Selectors;
	Selectors.InsertEmpty(NodeList.GetCount());

	TArray<CImageArranger::SCellDesc> Cells;
	for (i = 0; i < NodeList.GetCount(); i++)
		{
		CImageArranger::SCellDesc *pNewCell = Cells.Insert();
		pNewCell->cxWidth = cxImage;
		pNewCell->cyHeight = cyImage;
		pNewCell->sText = NodeList[i].sLabel;
		}

	//	Arrange

	Arranger.ArrangeByRow(Desc, Cells);

	//	Create a large image

	CG32bitImage Output;
	int cxWidth = Max(cxDesiredWidth, Arranger.GetWidth());
	int cyHeight = Arranger.GetHeight();
	Output.Create(cxWidth, cyHeight);
	printf("Creating %dx%d image.\n", cxWidth, cyHeight);

	//	Update context

	SSystemUpdateCtx UpdateCtx;
	UpdateCtx.bForceEventFiring = true;
	UpdateCtx.bForcePainted = true;

	//	Loop over all nodes and paint each system

	for (i = 0; i < NodeList.GetCount(); i++)
		{
		CTopologyNode *pNode = NodeList[i].pNode;

		//	Create the system

		CString sError;
		CSystem *pSystem;
		if (Universe.CreateStarSystem(pNode, &pSystem, &sError) != NOERROR)
			{
			printf("ERROR: node %s: %s\n", pNode->GetID().GetASCIIZPointer(), sError.GetASCIIZPointer());
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

		for (j = 0; j < iInitialUpdateTime; j++)
			Universe.Update(UpdateCtx);

		//	All objects are scanned

		for (j = 0; j < pSystem->GetObjectCount(); j++)
			{
			CSpaceObject *pObj = pSystem->GetObject(j);
			if (pObj == NULL || pObj->IsDestroyed())
				continue;

			pObj->SetKnown();
			}

		//	Paint

		RECT rcMap;
		rcMap.left = Arranger.GetX(i);
		rcMap.top = Arranger.GetY(i);
		rcMap.right = rcMap.left + Arranger.GetWidth(i);
		rcMap.bottom = rcMap.top + Arranger.GetHeight(i);

		Output.SetClipRect(rcMap);

		int iOffset = (int)(OFFSET_Y / rZoom);
		rcMap.top += iOffset;
		rcMap.bottom += iOffset;

		Universe.PaintPOVMap(Output, rcMap, rScale);

		//	Paint the name

		Output.ResetClipRect();
		int xText = Arranger.GetTextX(i);
		int yText = Arranger.GetTextY(i);
		if (xText != -1)
			{
			if (bTextBoxesOnly)
				Output.Fill(xText, yText, Arranger.GetTextWidth(i), Arranger.GetTextHeight(i), 0xffff);

			if (!bTextBoxesOnly)
				{
				NameFont.DrawText(Output,
						xText,
						yText,
						rgbNameColor,
						NodeList[i].sLabel);
				}
			}

		//	Progress

		printf("%s\n", pSystem->GetName().GetASCIIZPointer());

		//	Done with old system

		Universe.DestroySystem(pSystem);
		}

	//	Write to file or clipboard

	OutputImage(Output, sFilespec);
	}
