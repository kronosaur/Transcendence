//	CGalacticMapPainter.cpp
//
//	CGalacticMapPainter class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

const int NODE_RADIUS =						6;
const int STARGATE_LINE_WIDTH =				3;
const int MIN_NODE_MARGIN =					64;

CGalacticMapPainter::CGalacticMapPainter (const CVisualPalette &VI, CSystemMap *pMap) : m_VI(VI),
		m_pMap(pMap),
		m_cxMap(-1),
		m_cyMap(-1),
		m_bFreeImage(false),
		m_pImage(NULL)

//	CGalacticMapPainter constructor

	{
	}

CGalacticMapPainter::~CGalacticMapPainter (void)

//	CGalacticMapPainter destructor

	{
	if (m_pImage && m_bFreeImage)
		delete m_pImage;
	}

void CGalacticMapPainter::AdjustCenter (const RECT &rcView, int xCenter, int yCenter, int iScale, int *retxCenter, int *retyCenter)

//	AdjustCenter
//
//	Adjusts the center coordinates so that the map is always fully visible

	{
	Init();

	//	Compute the dimensions of the map to paint

	int cxWidth = m_cxMap;
	int cyHeight = m_cyMap;
	int cxMap = (100 * RectWidth(rcView) / iScale);
	int cyMap = (100 * RectHeight(rcView) / iScale);

	//	Compute the given center in map image coordinates

	int xMapCenter = Max(cxMap / 2, Min((cxWidth / 2) + xCenter, cxWidth - (cxMap / 2)));
	int yMapCenter = Max(cyMap / 2, Min((cyHeight / 2) - yCenter, cyHeight - (cyMap / 2)));

	//	Convert back after adjustment

	if ((iScale * cxWidth / 100) < RectWidth(rcView))
		*retxCenter = 0;
	else
		*retxCenter = xMapCenter - (cxWidth / 2);

	if ((iScale * cyHeight / 100) < RectHeight(rcView))
		*retyCenter = 0;
	else
		*retyCenter = (cyHeight / 2) - yMapCenter;
	}

void CGalacticMapPainter::DrawNode (CG16bitImage &Dest, CTopologyNode *pNode, int x, int y, WORD wColor)

//	DrawNode
//
//	Draws a topology node

	{
	DrawAlphaGradientCircle(Dest, x + 2, y + 2, NODE_RADIUS + 1, 0);
	DrawFilledCircle(Dest, x, y, NODE_RADIUS, wColor);
	DrawAlphaGradientCircle(Dest, x - 2, y - 2, NODE_RADIUS - 3, CG16bitImage::RGBValue(255, 255, 255));

	m_VI.GetFont(fontMediumBold).DrawText(Dest,
			x, y + NODE_RADIUS + 2,
			CG16bitImage::RGBValue(255, 255, 255),
			255,
			pNode->GetSystemName(),
			CG16bitFont::AlignCenter);

	//	Debug info

	if (g_pUniverse->InDebugMode() && m_pMap->DebugShowAttributes())
		{
		CString sLine = pNode->GetAttributes();
#if 0
		int iPos = strFind(sLine, CONSTLIT("level"));
		if (iPos != -1)
			sLine = strSubString(sLine, iPos, 7);
#endif

		m_VI.GetFont(fontMedium).DrawText(Dest,
				x,
				y + NODE_RADIUS + 2 + m_VI.GetFont(fontMediumBold).GetHeight(),
				CG16bitImage::RGBValue(128, 128, 128),
				255,
				sLine,
				CG16bitFont::AlignCenter);
		}
	}

void CGalacticMapPainter::GetPos (int x, int y, const RECT &rcView, int xCenter, int yCenter, int iScale, int *retx, int *rety)

//	GetPos
//
//	Returns the position

	{
	int xViewCenter = rcView.left + (RectWidth(rcView) / 2);
	int yViewCenter = rcView.top + (RectHeight(rcView) / 2);

	//	Convert to view coordinates

	*retx = xViewCenter + iScale * (x - xCenter) / 100;
	*rety = yViewCenter + iScale * (yCenter - y) / 100;
	}

void CGalacticMapPainter::Init (void)

//	Init
//
//	Initialize what we need to paint.

	{
	int i;

	if (m_cxMap == -1)
		{
		//	Allocate a new image and initialize it with the background

		m_pImage = m_pMap->CreateBackgroundImage();
		m_bFreeImage = (m_pImage != NULL);

		//	Compute the size of the map based on whatever is greater, then background
		//	image or the position of the nodes

		int xMin = 0;
		int xMax = 0;
		int yMin = 0;
		int yMax = 0;

		for (i = 0; i < g_pUniverse->GetTopologyNodeCount(); i++)
			{
			CTopologyNode *pNode = g_pUniverse->GetTopologyNode(i);
			
			int xPos, yPos;
			if (pNode->GetDisplayPos(&xPos, &yPos) == m_pMap 
					&& pNode->IsKnown() 
					&& !pNode->IsEndGame())
				{
				if (xPos < xMin)
					xMin = xPos;
				if (xPos > xMax)
					xMax = xPos;
				if (yPos < yMin)
					yMin = yPos;
				if (yPos > yMax)
					yMax = yPos;
				}
			}

		int cxExtent = 2 * Max(xMax, -xMin) + MIN_NODE_MARGIN;
		int cyExtent = 2 * Max(yMax, -yMin) + MIN_NODE_MARGIN;
		m_cxMap = Max(cxExtent, (m_pImage ? m_pImage->GetWidth() : 0));
		m_cyMap = Max(cyExtent, (m_pImage ? m_pImage->GetHeight() : 0));
		}
	}

void CGalacticMapPainter::Paint (CG16bitImage &Dest, const RECT &rcView, int xCenter, int yCenter, int iScale)

//	Paint
//
//	Paints the galactic map to the given destination, centered at the
//	given coordinates.
//
//	iScale 100 = normal
//	iScale 200 = x2
//	iScale 400 = x4

	{
	int i;
	int j;

	ASSERT(iScale > 0);

	//	Initialize

	Init();

	//	Paint the image, if we have it

	if (m_cxMap > 0 && m_cyMap > 0)
		{
		//	Compute some metrics

		WORD wNodeColor = CG16bitImage::RGBValue(255, 200, 128);
		WORD wStargateColor = CG16bitImage::RGBValue(160, 255, 128);

		int cxWidth = (m_pImage ? m_pImage->GetWidth() : 0);
		int cyHeight = (m_pImage ? m_pImage->GetHeight() : 0);

		//	Compute the dimensions of the map to paint

		int cxMap = (100 * RectWidth(rcView) / iScale);
		int cyMap = (100 * RectHeight(rcView) / iScale);
		int xViewCenter = rcView.left + (RectWidth(rcView) / 2);
		int yViewCenter = rcView.top + (RectHeight(rcView) / 2);

		//	Compute the given center in map image coordinates

		int xMapCenter = (cxWidth / 2) + xCenter;
		int yMapCenter = (cyHeight / 2) - yCenter;

		//	Compute the upper-left corner of the map

		int xMap = xMapCenter - (cxMap / 2);
		int yMap = yMapCenter - (cyMap / 2);

		//	Fill the borders, in case we the image won't fit

		if (xMap < 0)
			Dest.Fill(rcView.left, rcView.top, -(xMap * iScale) / 100, RectHeight(rcView), 0);

		if (yMap < 0)
			Dest.Fill(rcView.left, rcView.top, RectWidth(rcView), (-yMap * iScale) / 100, 0);

		if (xMap + cxMap > cxWidth)
			{
			int cx = iScale * ((xMap + cxMap) - cxWidth) / 100;
			Dest.Fill(rcView.right - cx, rcView.top, cx, RectHeight(rcView), 0);
			}

		if (yMap + cyMap > cyHeight)
			{
			int cy = iScale * ((yMap + cyMap) - cyHeight) / 100;
			Dest.Fill(rcView.left, rcView.bottom - cy, RectWidth(rcView), cy, 0);
			}

		//	Blt

		if (m_pImage)
			DrawBltScaledFast(Dest, rcView.left, rcView.top, RectWidth(rcView), RectHeight(rcView),
					*m_pImage, xMap, yMap, cxMap, cyMap);

		//	Loop over all nodes and clear marks on the ones that we need to draw

		for (i = 0; i < g_pUniverse->GetTopologyNodeCount(); i++)
			{
			CTopologyNode *pNode = g_pUniverse->GetTopologyNode(i);
			
			int xPos, yPos;
			pNode->SetMarked(pNode->GetDisplayPos(&xPos, &yPos) != m_pMap 
					|| !pNode->IsKnown() 
					|| pNode->IsEndGame());
			}

		//	Paint the nodes

		for (i = 0; i < g_pUniverse->GetTopologyNodeCount(); i++)
			{
			CTopologyNode *pNode = g_pUniverse->GetTopologyNode(i);
			if (!pNode->IsMarked())
				{
				int xPos, yPos;
				pNode->GetDisplayPos(&xPos, &yPos);

				//	Convert to view coordinates

				int x = xViewCenter + iScale * (xPos - xCenter) / 100;
				int y = yViewCenter + iScale * (yCenter - yPos) / 100;

				//	Draw gate connections

				for (j = 0; j < pNode->GetStargateCount(); j++)
					{
					CTopologyNode *pDestNode = pNode->GetStargateDest(j);
					if (pDestNode && !pDestNode->IsMarked())
						{
						int xPos, yPos;
						pDestNode->GetDisplayPos(&xPos, &yPos);

						int xDest = xViewCenter + iScale * (xPos - xCenter) / 100;
						int yDest = yViewCenter + iScale * (yCenter - yPos) / 100;

						Dest.DrawLine(x, y, xDest, yDest, STARGATE_LINE_WIDTH, wStargateColor);
						}
					}

				//	Draw star system

				if (x >= rcView.left && x < rcView.right && y >= rcView.top && y < rcView.bottom)
					DrawNode(Dest, pNode, x, y, wNodeColor);

				pNode->SetMarked();
				}
			}
		}
	else
		Dest.Fill(rcView.left, rcView.top, RectWidth(rcView), RectHeight(rcView), 0);
	}
