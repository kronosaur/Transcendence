//	CEnvironmentGrid.cpp
//
//	CEnvironmentGrid class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

struct SEdgeDesc
	{
	DWORD dwFlag;
	int xOffset;
	int yOffset;
	DWORD dwOppositeFlag;
	};

const int EDGE_COUNT =						4;

static SEdgeDesc EDGE_DATA[EDGE_COUNT] =
	{
		{	CEnvironmentGrid::edgeRight,	+1,	 0,	CEnvironmentGrid::edgeLeft		},
		{	CEnvironmentGrid::edgeTop,		 0,	-1,	CEnvironmentGrid::edgeBottom	},
		{	CEnvironmentGrid::edgeLeft,		-1,	 0,	CEnvironmentGrid::edgeRight		},
		{	CEnvironmentGrid::edgeBottom,	 0,	+1,	CEnvironmentGrid::edgeTop		},
	};

CEnvironmentGrid::CEnvironmentGrid (DWORD dwAPIVersion) : 
		m_Map((dwAPIVersion >= 14 ? defaultSize : sizeCompatible), (dwAPIVersion >= 14 ? defaultScale : scaleCompatibile))

//	CEnvironmentGrid constructor

	{
	m_iTileCount = m_Map.GetTotalSize();
	m_iTileSize = (dwAPIVersion >= 14 ? defaultTileSize : tileSizeCompatible);
	}

DWORD CEnvironmentGrid::AddEdgeFlag (DWORD dwTile, DWORD dwEdgeFlag) const

//	AddEdgeFlag
//
//	Add the given edge flag to the tile DWORD

	{
	DWORD dwOriginalFlags = (dwTile & 0xF0000);
	DWORD dwNewFlags = dwOriginalFlags | (dwEdgeFlag << 16);
	return dwNewFlags | (dwTile & 0xFFFF);
	}

void CEnvironmentGrid::ConvertSpaceEnvironmentToPointers (CTileMap &UNIDs)

//	ConvertSpaceEnvironmentToPointers
//
//	Converts m_pEnvironment to pointers (restore from save file)

	{
	m_Map.Init(UNIDs.GetSize(), UNIDs.GetScale());

	STileMapEnumerator k;
	while (UNIDs.HasMore(k))
		{
		DWORD dwTile;
		int xTile;
		int yTile;

		UNIDs.GetNext(k, &xTile, &yTile, &dwTile);
		CSpaceEnvironmentType *pEnv = g_pUniverse->FindSpaceEnvironment(dwTile);
		if (pEnv)
			SetTileType(xTile, yTile, pEnv);
		}

	m_iTileCount = m_Map.GetTotalSize();
	}

void AdjustBounds (CVector *pUL, CVector *pLR, const CVector &vPos);
void ChangeVariation (Metric *pVariation, Metric rMaxVariation);

void CEnvironmentGrid::CreateArcNebula (SCreateCtx &Ctx, TArray<STileDesc> *retTiles)

//	CreateArcNebula
//
//	Creates an arc of nebula

	{
	if (Ctx.pEnv == NULL || Ctx.pOrbitDesc == NULL || Ctx.rWidth == 0.0)
		return;

	//	Compute some constants

	Metric rHalfWidth = Ctx.rWidth / 2.0;
	Metric rMaxVariation = Ctx.iWidthVariation * rHalfWidth / 100.0;
	Metric rHalfVariation = rMaxVariation / 2.0;
	Metric rHalfSpan = g_Pi * Ctx.iSpan / 360.0;

	//	Shape of the arc

	CVector vCenter = Ctx.pOrbitDesc->GetFocus();
	Metric rArcCenterAngle = Ctx.pOrbitDesc->GetObjectAngle();
	Metric rMaxRadius = Ctx.pOrbitDesc->GetSemiMajorAxis() + rHalfWidth;
	Metric rMinRadius = Ctx.pOrbitDesc->GetSemiMajorAxis() - rHalfWidth;

	//	Shape parameters

	CWaveGenerator2 OuterWave(
			0.75,	//	Wave 0 amplitude
			5.0,	//	Wave 0 cycles
			0.25,	//	Wave 1 amplitude
			11.0	//	Wave 1 cycles
			);

	CWaveGenerator2 InnerWave(
			0.5,
			6.0,
			0.5,
			15.0
			);

	//	Compute the bounds of the nebula

	Metric rDiag = rMaxRadius + (GetTileSize() * g_KlicksPerPixel);
	CVector vUL = vCenter + CVector(-rDiag, rDiag);
	CVector vLR = vCenter + CVector(rDiag, -rDiag);

	//	Now iterate over every tile in bounds and see if it is within
	//	the band that we have defined.

	int xTileStart, yTileStart, xTileEnd, yTileEnd;
	VectorToTile(vUL, &xTileStart, &yTileStart);
	VectorToTile(vLR, &xTileEnd, &yTileEnd);

	//	Set the granularity of the result array (so that we don't keep on
	//	reallocating).

	if (retTiles)
		{
		retTiles->DeleteAll();
		int iGranularity = Max(DEFAULT_ARRAY_GRANULARITY, (xTileEnd - xTileStart + 1) * (yTileEnd - yTileStart + 1) / 2);
		retTiles->SetGranularity(iGranularity);
		}

	//	Set all tiles

	int x, y;
	for (x = xTileStart; x <= xTileEnd; x++)
		for (y = yTileStart; y <= yTileEnd; y++)
			{
			CVector vTile = TileToVector(x, y);
			CVector vRadius = vTile - vCenter;

			//	Convert to polar coordinates

			Metric rRadius;
			Metric rAngle = VectorToPolarRadians(vRadius, &rRadius);

			//	Skip if outside bounds

			if (rRadius > rMaxRadius || rRadius < rMinRadius)
				continue;

			//	Figure out our distance from the center angle

			Metric rDistAngle = rAngle - rArcCenterAngle;
			if (rDistAngle > g_Pi)
				rDistAngle -= 2.0 * g_Pi;
			else if (rDistAngle < -g_Pi)
				rDistAngle += 2.0 * g_Pi;

			Metric rDistAngleAbs = Absolute(rDistAngle);
			if (rDistAngleAbs > rHalfSpan)
				continue;

			//	Compute the shape at this angle

			Metric rMidRadius;
			Ctx.pOrbitDesc->GetPointAndRadius(rAngle, &rMidRadius);

			Metric rOuterWave = rHalfVariation * OuterWave.GetValue(rAngle);
			Metric rInnerWave = rHalfVariation * InnerWave.GetValue(rAngle);

			//	Taper the shape

			Metric rTaper = 1.0 - (rDistAngleAbs / rHalfSpan);

			//	Calc radius

			Metric rOuterRadius = rMidRadius + rTaper * (rHalfWidth - rOuterWave);
			Metric rInnerRadius = rMidRadius - rTaper * (rHalfWidth + rInnerWave);

			//	If we're inside the ring, then put a nebula down

			if (rRadius > rInnerRadius && rRadius < rOuterRadius)
				{
				SetTileType(x, y, Ctx.pEnv);

				//	Add to list of tiles

				if (retTiles)
					{
					STileDesc *pNewTile = retTiles->Insert();
					pNewTile->x = x;
					pNewTile->y = y;
					pNewTile->pEnv = Ctx.pEnv;
					pNewTile->dwEdgeMask = 0;
					}
				}
			}
	}

void CEnvironmentGrid::CreateCircularNebula (SCreateCtx &Ctx, TArray<STileDesc> *retTiles)

//	CreateCircularNebula
//
//	The nebula is a ring:
//
//	radius: Ctx.pOrbitDesc radius
//	width of ring: Ctx.rWidth

	{
	if (Ctx.pEnv == NULL || Ctx.pOrbitDesc == NULL || Ctx.rWidth == 0.0)
		return;

	//	Compute some constants

	Metric rHalfWidth = Ctx.rWidth / 2.0;
	Metric rMaxVariation = Ctx.iWidthVariation * rHalfWidth / 100.0;
	Metric rHalfVariation = rMaxVariation / 2.0;

	//	Shape parameters

	CWaveGenerator2 OuterWave(
			0.75,	//	Wave 0 amplitude
			5.0,	//	Wave 0 cycles
			0.25,	//	Wave 1 amplitude
			11.0	//	Wave 1 cycles
			);

	CWaveGenerator2 InnerWave(
			0.5,
			6.0,
			0.5,
			15.0
			);

	//	The maximum diameter of the ring is the orbit diameter plus half the
	//	width.

	Metric rMaxRadius = Ctx.pOrbitDesc->GetSemiMajorAxis() + rHalfWidth;
	CVector vCenter = Ctx.pOrbitDesc->GetFocus();

	//	Compute the bounds of the nebula

	Metric rDiag = rMaxRadius + (GetTileSize() * g_KlicksPerPixel);
	CVector vUL = vCenter + CVector(-rDiag, rDiag);
	CVector vLR = vCenter + CVector(rDiag, -rDiag);

	//	Now iterate over every tile in bounds and see if it is within
	//	the band that we have defined.

	int xTileStart, yTileStart, xTileEnd, yTileEnd;
	VectorToTile(vUL, &xTileStart, &yTileStart);
	VectorToTile(vLR, &xTileEnd, &yTileEnd);

	//	Set the granularity of the result array (so that we don't keep on
	//	reallocating).

	if (retTiles)
		{
		retTiles->DeleteAll();
		int iGranularity = Max(DEFAULT_ARRAY_GRANULARITY, (xTileEnd - xTileStart + 1) * (yTileEnd - yTileStart + 1) / 2);
		retTiles->SetGranularity(iGranularity);
		}

	//	Set all tiles

	int x, y;
	for (x = xTileStart; x <= xTileEnd; x++)
		for (y = yTileStart; y <= yTileEnd; y++)
			{
			CVector vTile = TileToVector(x, y);
			CVector vRadius = vTile - vCenter;

			//	Convert to polar coordinates

			Metric rRadius;
			Metric rAngle = VectorToPolarRadians(vRadius, &rRadius);

			//	Compute the shape at this angle

			Metric rMidRadius;
			Ctx.pOrbitDesc->GetPointAndRadius(rAngle, &rMidRadius);

			Metric rOuterWave = rHalfVariation * OuterWave.GetValue(rAngle);
			Metric rInnerWave = rHalfVariation * InnerWave.GetValue(rAngle);

			Metric rOuterRadius = rMidRadius + rHalfWidth - rOuterWave;
			Metric rInnerRadius = rMidRadius - rHalfWidth + rInnerWave;

			//	If we're inside the ring, then put a nebula down

			if (rRadius > rInnerRadius && rRadius < rOuterRadius)
				{
				SetTileType(x, y, Ctx.pEnv);

				//	Add to list of tiles

				if (retTiles)
					{
					STileDesc *pNewTile = retTiles->Insert();
					pNewTile->x = x;
					pNewTile->y = y;
					pNewTile->pEnv = Ctx.pEnv;
					pNewTile->dwEdgeMask = 0;
					}
				}
			}
	}

void CEnvironmentGrid::CreateSquareNebula (SCreateCtx &Ctx, TArray<STileDesc> *retTiles)

//	CreateSquareNebula
//
//	The nebula is a square:
//
//	center: Ctx.pOrbitDesc position
//	height: Ctx.rHeight
//	width: Ctx.rWidth

	{
	if (Ctx.pEnv == NULL || Ctx.pOrbitDesc == NULL || Ctx.rWidth == 0.0 || Ctx.rHeight == 0.0)
		return;

	CVector vCenter = Ctx.pOrbitDesc->GetObjectPos();

	//	Compute the bounds of the nebula

	CVector vUL = vCenter + CVector(-0.5 * Ctx.rWidth, -0.5 * Ctx.rHeight);
	CVector vLR = vCenter + CVector(0.5 * Ctx.rWidth, 0.5 * Ctx.rHeight);

	//	Now iterate over every tile in bounds and see if it is within
	//	the band that we have defined.

	int xTileStart, yTileStart, xTileEnd, yTileEnd;
	VectorToTile(vUL, &xTileStart, &yTileStart);
	VectorToTile(vLR, &xTileEnd, &yTileEnd);

	//	Set the granularity of the result array (so that we don't keep on
	//	reallocating).

	if (retTiles)
		{
		retTiles->DeleteAll();
		int iGranularity = Max(DEFAULT_ARRAY_GRANULARITY, (xTileEnd - xTileStart + 1) * (yTileEnd - yTileStart + 1) / 2);
		retTiles->SetGranularity(iGranularity);
		}

	//	Set all tiles

	int x, y;
	for (x = xTileStart; x <= xTileEnd; x++)
		for (y = yTileStart; y <= yTileEnd; y++)
			{
			SetTileType(x, y, Ctx.pEnv);

			//	Add to list of tiles

			if (retTiles)
				{
				STileDesc *pNewTile = retTiles->Insert();
				pNewTile->x = x;
				pNewTile->y = y;
				pNewTile->pEnv = Ctx.pEnv;
				pNewTile->dwEdgeMask = 0;
				}
			}
	}

void CEnvironmentGrid::GetNextTileType (STileMapEnumerator &i, int *retx, int *rety, CSpaceEnvironmentType **retpEnv, DWORD *retdwEdgeMask) const

//	GetNextTileType
//
//	Returns the next tile type

	{
	DWORD dwTile;
	m_Map.GetNext(i, retx, rety, &dwTile);

	if (retpEnv)
		*retpEnv = GetSpaceEnvironmentFromTileDWORD(dwTile);

	if (retdwEdgeMask)
		*retdwEdgeMask = (dwTile & 0xF0000) >> 16;
	}

CSpaceEnvironmentType *CEnvironmentGrid::GetSpaceEnvironmentFromTileDWORD (DWORD dwTile) const

//	GetSpaceEnvironmentFromTileDWORD
//
//	Returns the space environment.

	{
	DWORD dwAtom = (dwTile & 0xffff);
	if (dwAtom == 0)
		return NULL;

	return m_EnvList.atom_GetKey(dwAtom - 1);
	}

CSpaceEnvironmentType *CEnvironmentGrid::GetTileType (int xTile, int yTile, DWORD *retdwEdgeMask)

//	GetTileType
//
//	Returns the space environment type at the given tile.

	{
	DWORD dwTile = m_Map.GetTile(xTile, yTile);
	if (retdwEdgeMask)
		*retdwEdgeMask = (dwTile & 0xF0000) >> 16;

	return GetSpaceEnvironmentFromTileDWORD(dwTile);
	}

DWORD CEnvironmentGrid::MakeTileDWORD (CSpaceEnvironmentType *pEnv, DWORD dwEdgeMask)

//	MakeTileDWORD
//
//	Encodes into a DWORD for the tile map
//
//	The encoding is as follows:
//
//	33222222222211111111110000000000
//	10987654321098765432109876543210
//	            |  ||    16-bits   |
//				|EE||      SEA     |
//
//	EE = Edge bitmask
//
//	SEA = Space Environment Atom. NOTE: We need to bias by 1 because an atom of
//			0 is valid, but a DWORD of 0 means no tile.

	{
	//	If NULL environment, then 0 atom

	if (pEnv == NULL)
		return (dwEdgeMask << 16);

	//	Otherwise, get atom for environment

	else
		{
		//	Get the atom for this environment

		DWORD dwAtom = m_EnvList.atom_Find(pEnv);
		if (dwAtom == NULL_ATOM)
			m_EnvList.atom_Insert(pEnv, &dwAtom);

		//	Combine

		return (dwEdgeMask << 16) | (dwAtom + 1);
		}
	}

void CEnvironmentGrid::MarkImages (void)

//	MarkImages
//
//	Mark images in use.

	{
	int i;

	for (i = 0; i < m_EnvList.GetCount(); i++)
		{
		CSpaceEnvironmentType *pEnv = m_EnvList.GetKey(i);
		pEnv->MarkImages();
		}
	}

void CEnvironmentGrid::Paint (CG16bitImage &Dest, SViewportPaintCtx &Ctx, const CVector &vUR, const CVector &vLL)

//	Paint
//
//	Paint the environment

	{
	DEBUG_TRY

	int x, y, x1, y1, x2, y2;

	VectorToTile(vUR, &x2, &y1);
	VectorToTile(vLL, &x1, &y2);
		
	//	Increase bounds (so we can paint the edges)

	x1--; y1--;
	x2++; y2++;

	for (x = x1; x <= x2; x++)
		for (y = y1; y <= y2; y++)
			{
			DWORD dwEdgeMask;
			CSpaceEnvironmentType *pEnv = GetTileType(x, y, &dwEdgeMask);
			if (pEnv)
				{
				int xCenter, yCenter;
				CVector vCenter = TileToVector(x, y);
				Ctx.XForm.Transform(vCenter, &xCenter, &yCenter);

				pEnv->Paint(Dest, xCenter, yCenter, x, y, dwEdgeMask);
				}
			}

	DEBUG_CATCH
	}

void CEnvironmentGrid::PaintMap (CMapViewportCtx &Ctx, CG16bitImage &Dest)

//	PaintMap
//
//	Paint the environment on the map

	{
	int cxHalfTileCount = m_iTileCount / 2;
	int cyHalfTileCount = m_iTileCount / 2;

	//	Compute the size of each tile.

	int cxOrigin;
	int cyOrigin;
	Ctx.Transform(Ctx.GetCenterPos(), &cxOrigin, &cyOrigin);

	int cxTile;
	int cyTile;
	CVector vTile(m_iTileSize * g_KlicksPerPixel, m_iTileSize * g_KlicksPerPixel);
	Ctx.Transform(Ctx.GetCenterPos() + vTile, &cxTile, &cyTile);

	//	+1 because the floating-point conversion is sometimes off.

	cxTile = AlignUp(Absolute(cxTile - cxOrigin), 2) + 1;
	cyTile = AlignUp(Absolute(cyTile - cyOrigin), 2) + 1;

	int cxHalfTile = cxTile / 2;
	int cyHalfTile = cyTile / 2;

	//	Paint all tiles

	STileMapEnumerator k;
	while (HasMoreTiles(k))
		{
		//	Get the tile

		int xTile;
		int yTile;
		CSpaceEnvironmentType *pEnv;
		DWORD dwEdgeMask;

		GetNextTileType(k, &xTile, &yTile, &pEnv, &dwEdgeMask);
		if (pEnv == NULL)
			continue;

		//	Get the position of the tile

		CVector vPos = TileToVector(xTile, yTile);

		//	Fade out based on distance

		int iDist = Max(Absolute(xTile - cxHalfTileCount), Absolute(yTile - cyHalfTileCount));
		DWORD dwFade = (iDist > 5 ? Min(8 * (iDist - 5), 0xC0) : 0);

		//	Transform to map coords

		int x;
		int y;
		Ctx.Transform(vPos, &x, &y);

		//	Paint the tile

		pEnv->PaintMap(Dest, x, y, cxTile, cyTile, dwFade, dwEdgeMask);
		}
	}

void CEnvironmentGrid::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read
//
//	DWORD			m_iTileSize
//	DWORD			No. of atom entries
//	For each entry
//		DWORD		UNID of space environment
//
//	CTileMap		Tile Map

	{
	ALERROR error;
	int i;

	if (Ctx.dwVersion >= 82)
		{
		//	Read the size of each tile

		if (Ctx.dwVersion >= 88)
			Ctx.pStream->Read((char *)&m_iTileSize, sizeof(DWORD));
		else
			m_iTileSize = tileSizeCompatible;

		//	Read the environment list

		m_EnvList.DeleteAll();

		//	Since we saved the list in atom-order, we can reload and expect to
		//	add them and get back the same atom. We rely on the fact that the
		//	atom is the same as the order in which they are added.

		int iCount;
		Ctx.pStream->Read((char *)&iCount, sizeof(DWORD));
		for (i = 0; i < iCount; i++)
			{
			DWORD dwUNID;
			Ctx.pStream->Read((char *)&dwUNID, sizeof(DWORD));

			CSpaceEnvironmentType *pEnv = g_pUniverse->FindSpaceEnvironment(dwUNID);
			DWORD dwAtom;
			m_EnvList.atom_Insert(pEnv, &dwAtom);
			ASSERT(dwAtom == i);
			}

		//	Now load the tile map.

		if (error = m_Map.ReadFromStream(Ctx.pStream))
			throw CException(error);

		m_iTileCount = m_Map.GetTotalSize();
		}

	//	In old versions we just save the UNID of the environment type for
	//	each tile.

	else
		{
		//	Load into a separate map

		CTileMap Env;
		if (error = Env.ReadFromStream(Ctx.pStream))
			throw CException(error);

		//	Convert to the new storage method

		ConvertSpaceEnvironmentToPointers(Env);

		m_iTileSize = tileSizeCompatible;
		}
	}

void CEnvironmentGrid::SetTileType (int xTile, int yTile, CSpaceEnvironmentType *pEnv)

//	SetTileType
//
//	Sets a tile.

	{
	int i;

	//	For now there is no way to delete a tile.

	ASSERT(pEnv);

	if (InBounds(xTile, yTile))
		{
		//	Loop over all edges to see if there is a tile there and to tell the
		//	tile that a new tile has appeared on THEIR edge.

		DWORD dwEdgeMask = 0;
		for (i = 0; i < EDGE_COUNT; i++)
			{
			int xEdgeTile = xTile + EDGE_DATA[i].xOffset;
			int yEdgeTile = yTile + EDGE_DATA[i].yOffset;

			DWORD *pTile = m_Map.GetTilePointer(xEdgeTile, yEdgeTile);
			if (pTile && GetSpaceEnvironmentFromTileDWORD(*pTile) == pEnv)
				{
				//	This is an edge for the new tile.

				dwEdgeMask |= EDGE_DATA[i].dwFlag;

				//	Add the new tile as an edge for this tile.

				*pTile = AddEdgeFlag(*pTile, EDGE_DATA[i].dwOppositeFlag);
				}
			}

		//	Set the new tile

		m_Map.SetTile(xTile, yTile, MakeTileDWORD(pEnv, dwEdgeMask));
		}
	}

CVector CEnvironmentGrid::TileToVector (int x, int y) const

//	TileToVector
//
//	Returns the coordinates of the center of the given tile

	{
	int xyHalfSize = m_iTileCount / 2;
	return CVector(
			(x - xyHalfSize + 0.5) * m_iTileSize * g_KlicksPerPixel,
			-((y - xyHalfSize + 0.5) * m_iTileSize * g_KlicksPerPixel)
			);
	}

void CEnvironmentGrid::VectorToTile (const CVector &vPos, int *retx, int *rety) const

//	VectorToTile
//
//	Converts from a vector to a tile coordinate

	{
	*retx = (int)((vPos.GetX() / (m_iTileSize * g_KlicksPerPixel)) + (m_iTileCount / 2));
	*rety = (int)((-vPos.GetY() / (m_iTileSize * g_KlicksPerPixel)) + (m_iTileCount / 2));
	}

void CEnvironmentGrid::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write
//
//	DWORD			m_iTileSize
//	DWORD			No. of atom entries
//	For each entry
//		DWORD		UNID of space environment
//
//	CTileMap		Tile Map

	{
	int i;

	pStream->Write((char *)&m_iTileSize, sizeof(DWORD));

	//	Write the entry list. We write out in atom-order. Since we never delete
	//	atoms, we can guarantee that this is a contiguous list.

	DWORD dwSave = m_EnvList.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < m_EnvList.GetCount(); i++)
		{
		CSpaceEnvironmentType *pEnv = m_EnvList.atom_GetKey((DWORD)i);

		dwSave = pEnv->GetUNID();
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}

	//	Now write the tile map

	m_Map.WriteToStream(pStream);
	}
