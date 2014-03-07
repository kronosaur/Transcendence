//	CSpaceEnvironmentType.cpp
//
//	CSpaceEnvironmentType class

#include "PreComp.h"

#define EDGE_MASK_TAG							CONSTLIT("EdgeMask")
#define IMAGE_TAG								CONSTLIT("Image")

#define ATTRIBUTES_ATTRIB						CONSTLIT("attributes")
#define AUTO_EDGES_ATTRIB						CONSTLIT("autoEdges")
#define DRAG_FACTOR_ATTRIB						CONSTLIT("dragFactor")
#define LRS_JAMMER_ATTRIB						CONSTLIT("lrsJammer")
#define MAP_COLOR_ATTRIB						CONSTLIT("mapColor")
#define OPACITY_ATTRIB							CONSTLIT("opacity")
#define SHIELD_JAMMER_ATTRIB					CONSTLIT("shieldJammer")
#define SRS_JAMMER_ATTRIB						CONSTLIT("srsJammer")
#define UNID_ATTRIB								CONSTLIT("UNID")

#define ON_OBJ_UPDATE_EVENT						CONSTLIT("OnObjUpdate")

const int TILES_IN_TILE_SET =					15;

CSpaceEnvironmentType::SEdgeDesc CSpaceEnvironmentType::EDGE_DATA[TILES_IN_TILE_SET] =
	{
		//	EDGE 0: Cloud
		{	edgeCloud,		0,	},

		//	EDGE 1: Peninsula West
		{	edgePeninsula,	180,
			0.0,		-1.0,		0.5,		-1.0,		0.0,		1.0,

			0.5 * g_Pi,
			0.5,
			0.0,
			1.0,
			0.12,
			0.48,
			},

		//	EDGE 2: Peninsula South
		{	edgePeninsula,	270,
			-1.0,		0.0,		0.5,		0.0,		1.0,		0.0,

			0.5 * g_Pi,
			0.5,
			0.0,
			1.0,
			0.12,
			0.48,
			},

		//	EDGE 3: Corner South-West
		{	edgeCorner,		225,
			-0.707107,	-0.707107,	0.707107,	-0.707107,	0.707107,	0.0,

			g_Pi,
			0.707107,
			-1.0,
			0.5,
			0.12,
			0.48,
			},

		//	EDGE 4: Peninsula East
		{	edgePeninsula,	0,
			0.0,		1.0,		-0.5,		1.0,		0.0,		0.0,

			0.5 * g_Pi,
			0.5,
			0.0,
			1.0,
			0.12,
			0.48,
			},

		//	EDGE 5: Narrow East-West
		{	edgeNarrow,		90,
			1.0,		0.0,		-0.5,		0.0,		-1.0,		0.5,

			g_Pi,
			0.5,
			-1.0,
			0.5,
			0.03,
			0.12,
			},

		//	EDGE 6: Corner South-East
		{	edgeCorner,		315,
			-0.707107,	0.707107,	0.0,		0.707107,	0.707107,	-0.707107,

			g_Pi,
			0.707107,
			-1.0,
			0.5,
			0.12,
			0.48,
			},

		//	EDGE 7: Straight South
		{	edgeStraight,	270,
			-1.0,		0.0,		1.0,		0.0,		-1.0,		1.0,
			},

		//	EDGE 8: Peninsula North
		{	edgePeninsula,	90,
			1.0,		0.0,		-0.5,		0.0,		-1.0,		1.0,

			0.5 * g_Pi,
			0.5,
			0.0,
			1.0,
			0.12,
			0.48,
			},

		//	EDGE 9: Corner North-West
		{	edgeCorner,		135,
			0.707107,	-0.707107,	0.0,		-0.707107,	-0.707107,	0.707107,

			g_Pi,
			0.707107,
			-1.0,
			0.5,
			0.12,
			0.48,
			},

		//	EDGE 10: Narrow North-South
		{	edgeNarrow,		0,
			0.0,		-1.0,		0.5,		-1.0,		0.0,		0.5,

			g_Pi,
			0.5,
			-1.0,
			0.5,
			0.03,
			0.12,
			},

		//	EDGE 11: Straight West
		{	edgeStraight,	180,
			0.0,		-1.0,		1.0,		1.0,		0.0,		0.0,
			},

		//	EDGE 12: Corner North-East
		{	edgeCorner,		45,
			0.707107,	0.707107,	-0.707107,	0.707107,	-0.707107,	0.0,

			g_Pi,
			0.707107,
			-1.0,
			0.5,
			0.12,
			0.48,
			},

		//	EDGE 13: Straight North
		{	edgeStraight,	90,
			1.0,		0.0,		0.0,		0.0,		1.0,		0.0,
			},

		//	EDGE 14: Straight East
		{	edgeStraight,	0,
			0.0,		1.0,		0.0,		-1.0,		0.0,		1.0,
			},
	};

void CSpaceEnvironmentType::CreateAutoTileSet (int iVariants)

//	CreateAutoTileSet
//
//	Generates a procedural tile set for edges

	{
	int i, j;

	m_TileSet.DeleteAll();
	m_iVariantCount = iVariants;
	m_TileSet.InsertEmpty(m_iVariantCount * TILES_IN_TILE_SET);

	for (i = 0; i < m_iVariantCount; i++)
		{
		for (j = 0; j < TILES_IN_TILE_SET; j++)
			{
			int iIndex = (i * TILES_IN_TILE_SET) + j;

			CreateEdgeTile(EDGE_DATA[j], &m_TileSet[iIndex]);
			}
		}
	}

void CSpaceEnvironmentType::CreateEdgeTile (const SEdgeDesc &EdgeDesc, STileDesc *retTile)

//	CreateEdgeTile
//
//	Creates an edge mask based on the given descriptor.

	{
	int x, y;
	CG16bitImage Mask;
	Mask.CreateBlankAlpha(m_iTileSize, m_iTileSize, 0);

	switch (EdgeDesc.iType)
		{
		case edgeCloud:
			{
			//	Compute some constants

			Metric rTargetRadius = 0.2 * m_iTileSize;
			Metric rRadiusRange = 0.1 * m_iTileSize;
			Metric rFeatherRange = 0.12 * m_iTileSize;

			//	Loop over all pixels

			for (y = 0; y < m_iTileSize; y++)
				{
				BYTE *pAlphaRow = Mask.GetAlphaRow(y);
				for (x = 0; x < m_iTileSize; x++)
					{
					BYTE *pAlpha = pAlphaRow + x;

					//	Convert to polar coordinates

					Metric rRadius;
					Metric rAngle = VectorToPolarRadians(CVector(x - (0.5 * m_iTileSize), (0.5 * m_iTileSize) - y), &rRadius);

					//	Compute the main wave

					Metric rCos = cos(rAngle);
					Metric rRange = (0.5 * rFeatherRange) + (0.5 * rFeatherRange * (sin(rAngle * 3.0) + 1.0) / 2.0);

					//	Add some turbulence

					Metric rTurb1 = (sin(rAngle * 5.0) + 1.0) / 2.0;
					Metric rTurb2 = (sin(rAngle * 11.0) + 1.0) / 2.0;
					Metric rTurb = (0.25 * rTurb1) + (0.125 * rTurb2);

					//	Compute radii

					Metric rMinRadius = rTargetRadius + (rRadiusRange * (rCos + rTurb));
					Metric rMaxRadius = rMinRadius + rRange;

					//	Outside is empty

					if (rRadius > rMaxRadius)
						*pAlpha = 0x00;

					//	Inside is full nebula

					else if (rRadius < rMinRadius)
						*pAlpha = (BYTE)m_dwOpacity;

					//	Range between

					else
						{
						Metric rRange = rMaxRadius - rMinRadius;
						Metric rValue = Min(Max(0.0, (rRadius - rMinRadius) / rRange), 1.0);
						*pAlpha = ((BYTE)m_dwOpacity - (BYTE)(m_dwOpacity * rValue));
						}
					}
				}

			break;
			}

		case edgeCorner:
		case edgePeninsula:
			{
			//	Compute some constants

			Metric rAnglesPerPixel = EdgeDesc.rHalfWaveAngle / (m_iTileSize * EdgeDesc.rPixelsPerHalfWave);
			Metric rMaxScale = EdgeDesc.rWaveScale * m_iTileSize * EdgeDesc.rMaxAmplitude;
			Metric rMinScale = EdgeDesc.rWaveScale * m_iTileSize * EdgeDesc.rMinAmplitude;

			Metric rXc = m_iTileSize * EdgeDesc.rXc;
			Metric rYc = m_iTileSize * EdgeDesc.rYc;

			//	Loop over all pixels

			for (y = 0; y < m_iTileSize; y++)
				{
				BYTE *pAlphaRow = Mask.GetAlphaRow(y);
				for (x = 0; x < m_iTileSize; x++)
					{
					BYTE *pAlpha = pAlphaRow + x;

					//	Figure out where this pixel is relative to the wave with the
					//	origin at the center.

					Metric x1 = EdgeDesc.rXx * x + EdgeDesc.rXy * y + rXc;
					Metric y1 = EdgeDesc.rYx * x + EdgeDesc.rYy * y + rYc;

					//	Compute the main wave

					Metric rAngle = rAnglesPerPixel * x1;
					Metric rCos = (cos(rAngle) - EdgeDesc.rWaveMin);	//	Scaled from 0 to 1.0
					Metric rMax = rMaxScale * rCos;
					Metric rMin = rMinScale * rCos;

					//	Add some turbulence

					Metric rRange = (rMax - rMin);
					Metric rTurb1 = (sin(rAngle * 5.0) + 1.0) / 2.0;
					Metric rTurb2 = (sin(rAngle * 11.0) + 1.0) / 2.0;
					Metric rTurb = (0.25 * rRange * rTurb1) + (0.125 * rRange * rTurb2);

					rMin += rTurb;
					rMax += rTurb;

					//	Below min, is full nebula

					if (y1 < rMin)
						*pAlpha = (BYTE)m_dwOpacity;

					//	Above max is empty space

					else if (y1 > rMax)
						*pAlpha = 0x00;

					//	Range between

					else
						{
						Metric rRange = rMax - rMin;
						Metric rValue = Min(Max(0.0, (y1 - rMin) / rRange), 1.0);
						*pAlpha = (BYTE)m_dwOpacity - (BYTE)((BYTE)m_dwOpacity * rValue);
						}

					}
				}

			break;
			}

		case edgeStraight:
			{
			//	Compute wave parameters

			Metric rHalfRange = 0.9 * g_Pi;
			Metric rWc = -cos(rHalfRange);
			Metric rAnglePerPixel = rHalfRange / (Metric)(m_iTileSize / 2);
			Metric rMinAmplitude = m_iTileSize * 0.03;
			Metric rMaxAmplitude = m_iTileSize * 0.12;

			Metric rXc = m_iTileSize * EdgeDesc.rXc;
			Metric rYc = m_iTileSize * EdgeDesc.rYc;

			//	Loop over all pixels

			for (y = 0; y < m_iTileSize; y++)
				{
				BYTE *pAlphaRow = Mask.GetAlphaRow(y);
				for (x = 0; x < m_iTileSize; x++)
					{
					BYTE *pAlpha = pAlphaRow + x;

					//	Figure out where this pixel is relative to the sine wave.

					Metric x1 = EdgeDesc.rXx * x + EdgeDesc.rXy * y + rXc;
					Metric y1 = EdgeDesc.rYx * x + EdgeDesc.rYy * y + rYc;

					//	Compute the angle based on the position

					Metric rAngle = rAnglePerPixel * (x1 - (m_iTileSize / 2));
					Metric rCos = cos(rAngle) + rWc;	//	Scaled from 0 to 1.something
					Metric rMax = rMaxAmplitude * rCos;
					Metric rMin = rMinAmplitude * rCos;

					Metric rRange = (rMax - rMin);
					Metric rTurb1 = (sin(rAngle * 5.0) + 1.0) / 2.0;
					Metric rTurb2 = (sin(rAngle * 11.0) + 1.0) / 2.0;
					Metric rTurb = (0.25 * rRange * rTurb1) + (0.125 * rRange * rTurb2);

					rMin += rTurb;
					rMax += rTurb;

					//	Below min, is empty space

					if (y1 < (int)rMin)
						*pAlpha = 0x00;

					//	Above max is full nebula

					else if (y1 > (int)rMax)
						*pAlpha = (BYTE)m_dwOpacity;

					//	Range between

					else
						{
						Metric rRange = rMax - rMin;
						Metric rValue = Min(Max(0.0, (y1 - rMin) / rRange), 1.0);
						*pAlpha = (BYTE)(m_dwOpacity * rValue);
						}
					}
				}

			break;
			}

		case edgeNarrow:
			{
			//	Compute some constants

			Metric rAnglesPerPixel = EdgeDesc.rHalfWaveAngle / (m_iTileSize * EdgeDesc.rPixelsPerHalfWave);
			Metric rMaxScale = EdgeDesc.rWaveScale * m_iTileSize * EdgeDesc.rMaxAmplitude;
			Metric rMinScale = EdgeDesc.rWaveScale * m_iTileSize * EdgeDesc.rMinAmplitude;

			Metric rXc = m_iTileSize * EdgeDesc.rXc;
			Metric rYc = m_iTileSize * EdgeDesc.rYc;

			//	Loop over all pixels

			for (y = 0; y < m_iTileSize; y++)
				{
				BYTE *pAlphaRow = Mask.GetAlphaRow(y);
				for (x = 0; x < m_iTileSize; x++)
					{
					BYTE *pAlpha = pAlphaRow + x;

					//	Figure out where this pixel is relative to the wave with the
					//	origin at the center.

					Metric x1 = EdgeDesc.rXx * x + EdgeDesc.rXy * y + rXc;
					Metric y1 = EdgeDesc.rYx * x + EdgeDesc.rYy * y + rYc;

					//	Compute the main wave

					Metric rAngle = rAnglesPerPixel * x1;
					Metric rCos = (cos(rAngle) - EdgeDesc.rWaveMin);	//	Scaled from 0 to 1.0
					Metric rMax = rMaxScale * rCos;
					Metric rMin = rMinScale * rCos;

					//	Add some turbulence

					Metric rRange = (rMax - rMin);
					Metric rTurb1 = (sin(rAngle * 5.0) + 1.0) / 2.0;
					Metric rTurb2 = (sin(rAngle * 11.0) + 1.0) / 2.0;
					Metric rTurb = (0.25 * rRange * rTurb1) + (0.125 * rRange * rTurb2);

					rMin += rTurb;
					rMax += rTurb;

					//	Compute edges

					Metric rTopOuter = 0.5 * m_iTileSize - rMin;
					Metric rTopInner = 0.5 * m_iTileSize - rMax;
					Metric rBottomInner = -0.5 * m_iTileSize + rMax;
					Metric rBottomOuter = -0.5 * m_iTileSize + rMin;

					//	Above top outer is empty space

					if (y1 > rTopOuter)
						*pAlpha = 0x00;

					//	Above the top inner is a blend

					else if (y1 > rTopInner)
						{
						Metric rRange = rMax - rMin;
						Metric rValue = Min(Max(0.0, (y1 - rTopInner) / rRange), 1.0);
						*pAlpha = (BYTE)m_dwOpacity - (BYTE)(m_dwOpacity * rValue);
						}

					//	Above bottom inner is full nebula

					else if (y1 > rBottomInner)
						*pAlpha = (BYTE)m_dwOpacity;

					//	Above bottom outer is a blend

					else if (y1 > rBottomOuter)
						{
						Metric rRange = rMax - rMin;
						Metric rValue = Min(Max(0.0, (y1 - rBottomOuter) / rRange), 1.0);
						*pAlpha = (BYTE)(m_dwOpacity * rValue);
						}

					//	Else, empty space

					else
						*pAlpha = 0x00;
					}
				}

			break;
			}

		default:
			{
			for (y = 0; y < m_iTileSize; y++)
				{
				BYTE *pAlpha = Mask.GetAlphaRow(y);
				BYTE *pAlphaEnd = pAlpha + m_iTileSize;
				while (pAlpha < pAlphaEnd)
					*pAlpha++ = 0x80;
				}
			}
		}

	retTile->Region.CreateFromMask(Mask, 0, 0, Mask.GetWidth(), Mask.GetHeight());
	}

void CSpaceEnvironmentType::CreateTileSet (const CObjectImageArray &Edges)

//	CreateTileSet
//
//	Creates a tile set

	{
	int i, j;

	m_TileSet.DeleteAll();
	m_iVariantCount = 1;
	m_TileSet.InsertEmpty(m_iVariantCount * TILES_IN_TILE_SET);

	for (i = 0; i < m_iVariantCount; i++)
		{
		CG16bitImage &EdgesImage = Edges.GetImage(CONSTLIT("Create tile set"));

		for (j = 0; j < TILES_IN_TILE_SET; j++)
			{
			int iIndex = (i * TILES_IN_TILE_SET) + j;
			RECT rcRect = Edges.GetImageRect(i, j);

			m_TileSet[iIndex].Region.CreateFromMask(EdgesImage, rcRect.left, rcRect.top, RectWidth(rcRect), RectHeight(rcRect));
			}
		}
	}

ALERROR CSpaceEnvironmentType::FireOnUpdate (CSpaceObject *pObj, CString *retsError)

//	FireOnUpdate
//
//	Fire OnUpdate event (once per 15 ticks)

	{
	SEventHandlerDesc Event;
	if (FindEventHandler(ON_OBJ_UPDATE_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.DefineSpaceObject(CONSTLIT("aObj"), pObj);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("SpaceEnv OnUpdate: %s"), pResult->GetStringValue());
			return ERR_FAIL;
			}

		Ctx.Discard(pResult);
		}

	return NOERROR;
	}

ALERROR CSpaceEnvironmentType::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind the design

	{
	ALERROR error;

	if (error = m_Image.OnDesignLoadComplete(Ctx))
		return error;

	if (error = m_EdgeMask.OnDesignLoadComplete(Ctx))
		return error;

	return NOERROR;
	}

ALERROR CSpaceEnvironmentType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	ALERROR error;

	//	Load basic stuff

	m_bLRSJammer = pDesc->GetAttributeBool(LRS_JAMMER_ATTRIB);
	m_bShieldJammer = pDesc->GetAttributeBool(SHIELD_JAMMER_ATTRIB);
	m_bSRSJammer = pDesc->GetAttributeBool(SRS_JAMMER_ATTRIB);
	m_bAutoEdges = pDesc->GetAttributeBool(AUTO_EDGES_ATTRIB);

	//	Drag

	int iDrag = pDesc->GetAttributeInteger(DRAG_FACTOR_ATTRIB);
	if (iDrag)
		m_rDragFactor = (iDrag / 100.0);
	else
		m_rDragFactor = 1.0;

	//	Load image

	CXMLElement *pImage = pDesc->GetContentElementByTag(IMAGE_TAG);
	if (pImage)
		{
		if (error = m_Image.InitFromXML(Ctx, pImage))
			return error;
		}

	//	Edge mask

	CXMLElement *pEdgeMask = pDesc->GetContentElementByTag(EDGE_MASK_TAG);
	if (pEdgeMask)
		{
		if (error = m_EdgeMask.InitFromXML(Ctx, pEdgeMask))
			return error;
		}

	CString sMapColor;
	if (pDesc->FindAttribute(MAP_COLOR_ATTRIB, &sMapColor))
		m_rgbMapColor = LoadCOLORREF(sMapColor);
	else
		m_rgbMapColor = RGB(0x80, 0x00, 0x80);

	m_dwOpacity = pDesc->GetAttributeIntegerBounded(OPACITY_ATTRIB, 0, 255, 255);

	//	Keep track of the events that we have

	m_bHasOnUpdateEvent = FindEventHandler(ON_OBJ_UPDATE_EVENT);

	//	Done

	return NOERROR;
	}

void CSpaceEnvironmentType::OnMarkImages (void)

//	OnMarkImages
//
//	Mark images in use

	{
	m_Image.MarkImage();

	//	Ask the system for the tile size

	CSystem *pSystem = g_pUniverse->GetCurrentSystem();
	m_iTileSize = (pSystem ? pSystem->GetTileSize() : 512);

	//	If we a large tile then we assume that it consists of multiple,
	//	compatible tiles.

	RECT rcImage = m_Image.GetImageRect();
	int cxTiles = RectWidth(rcImage) / m_iTileSize;
	int cyTiles = RectHeight(rcImage) / m_iTileSize;
	if (cxTiles == cyTiles && cxTiles > 1)
		m_iImageTileCount = cxTiles;
	else
		m_iImageTileCount = 1;

	//	Create the tileset, if necessary

	if (m_TileSet.GetCount() == 0)
		{
		if (m_bAutoEdges)
			CreateAutoTileSet(1);
		else if (m_EdgeMask.IsLoaded())
			CreateTileSet(m_EdgeMask);
		}
	}

void CSpaceEnvironmentType::Paint (CG16bitImage &Dest, int x, int y, int xTile, int yTile, DWORD dwEdgeMask)

//	Paint
//
//	Paint the space environment

	{
	//	Get the image

	CG16bitImage &TileImage = m_Image.GetImage(CONSTLIT("Paint space environment"));
	if (TileImage.IsEmpty())
		return;

	//	Compute the source tile position

	int xCenter, yCenter;
	RECT rcTileSource;
	if (m_iImageTileCount == 1)
		rcTileSource = m_Image.GetImageRect(0, 0, &xCenter, &yCenter);
	else
		{
		int xSrcTile = ClockMod(xTile, m_iImageTileCount);
		int ySrcTile = ClockMod(yTile, m_iImageTileCount);

		rcTileSource.left = xSrcTile * m_iTileSize;
		rcTileSource.top = ySrcTile * m_iTileSize;
		rcTileSource.right = rcTileSource.left + m_iTileSize;
		rcTileSource.bottom = rcTileSource.top + m_iTileSize;

		xCenter = m_iTileSize / 2;
		yCenter = m_iTileSize / 2;
		}

	//	If this is a solid tile, paint it

	if (dwEdgeMask == 0x0F || m_TileSet.GetCount() == 0)
		{
		Dest.ColorTransBlt(rcTileSource.left,
				rcTileSource.top,
				RectWidth(rcTileSource),
				RectHeight(rcTileSource),
				m_dwOpacity,
				TileImage,
				x - xCenter,
				y - yCenter);
		}

	//	Otherwise, paint edge through a mask

	else
		{
		STileDesc &Desc = m_TileSet[(int)dwEdgeMask];

		Desc.Region.ColorTransBlt(Dest,
				x - xCenter,
				y - yCenter,
				TileImage,
				rcTileSource.left,
				rcTileSource.top,
				m_iTileSize,
				m_iTileSize);
		}
	}

void CSpaceEnvironmentType::PaintLRS (CG16bitImage &Dest, int x, int y)

//	PaintLRS
//
//	Paint environment in LRS

	{
	int i;

	for (i = 0; i < 20; i++)
		{
		int x1 = x + mathRandom(-16, 16);
		int y1 = y + mathRandom(-16, 16);

		int r = 85 + mathRandom(-17, 17);
		int g = 100 + mathRandom(-20, 20);
		int b = 90 + mathRandom(-18, 18);

		Dest.DrawPixel(x1, y1, CG16bitImage::RGBValue(r,g,b));
		}
	}

void CSpaceEnvironmentType::PaintMap (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, DWORD dwFade, DWORD dwEdgeMask)

//	PaintMap
//
//	Paints a map tile.

	{
	int cxHalfWidth = cxWidth / 2;
	int cyHalfHeight = cyHeight / 2;
	WORD wColor = CG16bitImage::BlendPixel((WORD)CG16bitImage::PixelFromRGB(m_rgbMapColor), 0, dwFade);

	switch (dwEdgeMask)
		{
		case 0:
			DrawFilledCircle(Dest, x + cxHalfWidth, y + cyHalfHeight, Min(cxHalfWidth, cyHalfHeight), wColor);
			break;

		case 1:
			DrawFilledCircle(Dest, x + cyHalfHeight, y + cyHalfHeight, cyHalfHeight, wColor);
			Dest.Fill(x + cyHalfHeight, y, cxWidth - cyHalfHeight + 1, cyHeight + 1, wColor);
			break;

		case 2:
			DrawFilledCircle(Dest, x + cxHalfWidth, y + cyHeight - cxHalfWidth, cxHalfWidth, wColor);
			Dest.Fill(x, y, cxWidth + 1, cyHeight - cxHalfWidth + 1, wColor);
			break;

		case 3:
			DrawFilledCircle(Dest, x + cyHalfHeight, y + cyHalfHeight, cyHalfHeight, wColor);
			Dest.Fill(x + cyHalfHeight, y, cxWidth - cyHalfHeight + 1, cyHeight + 1, wColor);
			Dest.Fill(x, y, cyHalfHeight + 1, cyHalfHeight + 1, wColor);
			break;

		case 4:
			DrawFilledCircle(Dest, x + cxWidth - cyHalfHeight, y + cyHalfHeight, cyHalfHeight, wColor);
			Dest.Fill(x, y, cxWidth - cyHalfHeight + 1, cyHeight + 1, wColor);
			break;

		case 6:
			DrawFilledCircle(Dest, x + cxWidth - cyHalfHeight, y + cyHalfHeight, cyHalfHeight, wColor);
			Dest.Fill(x, y, cxWidth - cyHalfHeight + 1, cyHeight + 1, wColor);
			Dest.Fill(x + cxWidth - cyHalfHeight, y, cyHalfHeight + 1, cyHalfHeight + 1, wColor);
			break;

		case 8:
			DrawFilledCircle(Dest, x + cxHalfWidth, y + cxHalfWidth, cxHalfWidth, wColor);
			Dest.Fill(x, y + cxHalfWidth, cxWidth + 1, cyHeight - cxHalfWidth + 1, wColor);
			break;

		case 9:
			DrawFilledCircle(Dest, x + cyHalfHeight, y + cyHalfHeight, cyHalfHeight, wColor);
			Dest.Fill(x + cyHalfHeight, y, cxWidth - cyHalfHeight + 1, cyHeight + 1, wColor);
			Dest.Fill(x, y + cyHalfHeight, cyHalfHeight + 1, cyHalfHeight + 1, wColor);
			break;

		case 12:
			DrawFilledCircle(Dest, x + cxWidth - cyHalfHeight, y + cyHalfHeight, cyHalfHeight, wColor);
			Dest.Fill(x, y, cxWidth - cyHalfHeight + 1, cyHeight + 1, wColor);
			Dest.Fill(x + cxWidth - cyHalfHeight, y + cyHalfHeight, cyHalfHeight + 1, cyHalfHeight + 1, wColor);
			break;

		default:
			Dest.Fill(x, y, cxWidth + 1, cyHeight + 1, wColor);
		}
	}

