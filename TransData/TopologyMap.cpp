//	TopologyMap.cpp
//
//	Generates a complete galactic map

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define STYLE_NAME							CONSTLIT("name")

void GenerateTopologyMap (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i;

	//	Some parameters

	int iScale = pCmdLine->GetAttributeIntegerBounded(CONSTLIT("scale"), 100, -1, -1);

	//	We need to create all systems so that the global object table is 
	//	initialized.

	for (i = 0; i < Universe.GetTopologyNodeCount(); i++)
		{
		CTopologyNode *pNode = Universe.GetTopologyNode(i);

		//	Do not try to create end game nodes

		if (pNode->IsEndGame())
			continue;

		//	Node is known

		pNode->SetKnown();

		//	Create this system

		CSystem *pNewSystem;
		CString sError;
		if (Universe.CreateStarSystem(pNode, &pNewSystem, &sError) != NOERROR)
			{
			printf("ERROR: creating system %s: %s\n", (LPSTR)pNode->GetSystemName(), (LPSTR)sError);
			continue;
			}

		//	Delete the system

		Universe.FlushStarSystem(pNode);
		}

	//	Initialize the output

	COutputChart Output;
	Output.SetStyleFont(STYLE_NAME, pCmdLine->GetAttribute(CONSTLIT("font")));
	Output.SetStyleColor(STYLE_NAME, CG32bitPixel(0xFF, 0xFF, 0xFF));

	Output.SetOutputFilespec(pCmdLine->GetAttribute(CONSTLIT("output")));

	//	Get the topology node

	CTopologyNode *pFirstNode = Universe.GetFirstTopologyNode();
	if (pFirstNode == NULL)
		{
		printf("ERROR: Unable to find topology node.\n");
		return;
		}

	//	Get the system map for the node

	CSystemMap *pMap = pFirstNode->GetDisplayPos();
	if (pMap == NULL)
		{
		printf("ERROR: No system map for node %s.\n", pFirstNode->GetID().GetASCIIZPointer());
		return;
		}

	//	If we didn't set a scale, use the map's initial scale

	if (iScale == -1)
		pMap->GetScale(&iScale);

	//	Initialize painting structures

	CSystemMapThumbnails Thumbs;
	Thumbs.Init(Universe.GetGlobalObjects());

	CGalacticMapPainter Painter(Universe, pMap, Thumbs);
	int cxMap = Painter.GetWidth() * iScale / 100;
	int cyMap = Painter.GetHeight() * iScale / 100;

	//	Compute the size of the map

	RECT rcView;
	rcView.left = 0;
	rcView.top = 0;
	rcView.right = cxMap;
	rcView.bottom = cyMap;

	//	Create the output

	Output.SetContentSize(cxMap, cyMap);
	CG32bitImage &Dest = Output.GetOutputImage();

	//	Paint

    Painter.SetViewport(rcView);
    Painter.SetScale(iScale);
    Painter.SetPos(0, 0);
	Painter.Paint(Dest);

	//	Done

	Output.Output();
	}

