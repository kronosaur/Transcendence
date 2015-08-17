//	TopologyMap.cpp
//
//	Generates a complete galactic map

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define STYLE_NAME							CONSTLIT("name")

const int STARGATE_LINE_WIDTH =				3;
const CG32bitPixel STARGATE_LINE_COLOR =	CG32bitPixel(160, 255, 128);

const int NODE_RADIUS =						6;
const CG32bitPixel NODE_COLOR =				CG32bitPixel(255, 200, 128);
const int MIN_NODE_MARGIN =					64;

struct STopologyMapCtx
	{
	STopologyMapCtx (void) :
			pMap(NULL)
		{ }

	~STopologyMapCtx (void)
		{
		}

	CSystemMap *pMap;

	int cxMap;
	int cyMap;
	int xCenter;
	int yCenter;
	};

static void DrawNode (STopologyMapCtx &Ctx, COutputChart &Output, CTopologyNode *pNode, int x, int y);

void GenerateTopologyMap (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i, j;

	STopologyMapCtx Ctx;

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

	Ctx.pMap = pFirstNode->GetDisplayPos();
	if (Ctx.pMap == NULL)
		{
		printf("ERROR: No system map for node %s.\n", pFirstNode->GetID().GetASCIIZPointer());
		return;
		}

	//	Create a background image

	CG32bitImage *pImage = Ctx.pMap->CreateBackgroundImage();

	//	Create the output

	Output.SetContentSize((pImage ? pImage->GetWidth() : 1024), (pImage ? pImage->GetHeight() : 1024));
	CG32bitImage &Dest = Output.GetOutputImage();

	//	Blt

	if (pImage)
		Dest.Blt(0, 0, pImage->GetWidth(), pImage->GetHeight(), *pImage, 0, 0);

	//	Done with background image

	delete pImage;
	pImage = NULL;

	//	Compute the size of the map (in pixels)

	Ctx.cxMap = Dest.GetWidth();
	Ctx.cyMap = Dest.GetHeight();
	Ctx.xCenter = Ctx.cxMap / 2;
	Ctx.yCenter = Ctx.cyMap / 2;

	//	Loop over all nodes and clear marks on the ones that we need to draw

	for (i = 0; i < Universe.GetTopologyNodeCount(); i++)
		{
		CTopologyNode *pNode = Universe.GetTopologyNode(i);
			
		int xPos, yPos;
		pNode->SetMarked(pNode->GetDisplayPos(&xPos, &yPos) != Ctx.pMap 
				|| pNode->IsEndGame());
		}

	//	Paint the nodes

	for (i = 0; i < Universe.GetTopologyNodeCount(); i++)
		{
		CTopologyNode *pNode = Universe.GetTopologyNode(i);
		if (!pNode->IsMarked())
			{
			int xPos, yPos;
			pNode->GetDisplayPos(&xPos, &yPos);

			//	Convert to view coordinates

			int x = Ctx.xCenter + xPos;
			int y = Ctx.yCenter - yPos;

			//	Draw gate connections

			for (j = 0; j < pNode->GetStargateCount(); j++)
				{
				CTopologyNode *pDestNode = pNode->GetStargateDest(j);
				if (pDestNode && !pDestNode->IsMarked())
					{
					int xPos, yPos;
					pDestNode->GetDisplayPos(&xPos, &yPos);

					int xDest = Ctx.xCenter + xPos;
					int yDest = Ctx.yCenter - yPos;

					Dest.DrawLine(x, y, xDest, yDest, STARGATE_LINE_WIDTH, STARGATE_LINE_COLOR);
					}
				}

			//	Draw star system

			DrawNode(Ctx, Output, pNode, x, y);

			pNode->SetMarked();
			}
		}

	//	Done

	Output.Output();
	}

void DrawNode (STopologyMapCtx &Ctx, COutputChart &Output, CTopologyNode *pNode, int x, int y)

//	DrawNode
//
//	Draws a topology node

	{
	CG32bitImage &Dest = Output.GetOutputImage();
	const CG16bitFont &NameFont = Output.GetStyleFont(STYLE_NAME);
	CG32bitPixel rgbNameColor = Output.GetStyleColor(STYLE_NAME);

	CGDraw::CircleGradient(Dest, x + 2, y + 2, NODE_RADIUS + 1, 0);
	CGDraw::Circle(Dest, x, y, NODE_RADIUS, NODE_COLOR);
	CGDraw::CircleGradient(Dest, x - 2, y - 2, NODE_RADIUS, CG32bitPixel(0xff, 0xff, 0xff));

	NameFont.DrawText(Dest,
			x, y + NODE_RADIUS + 2,
			rgbNameColor,
			pNode->GetSystemName(),
			CG16bitFont::AlignCenter);
	}
