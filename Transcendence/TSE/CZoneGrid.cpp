//	CZoneGrid.cpp
//
//	CZoneGrid class

#include "PreComp.h"

#define NAME_ATTRIB							CONSTLIT("name")
#define SIZE_X_ATTRIB						CONSTLIT("sizeX")
#define SIZE_Y_ATTRIB						CONSTLIT("sizeY")
#define CELL_SIZE_ATTRIB					CONSTLIT("cellSize")
#define X_ATTRIB							CONSTLIT("x")
#define Y_ATTRIB							CONSTLIT("y")
#define RADIUS_ATTRIB						CONSTLIT("radius")
#define CENTER_ATTRIB						CONSTLIT("center")
#define EDGE_ATTRIB							CONSTLIT("edge")
#define FOREGROUND_ATTRIB					CONSTLIT("foreground")
#define BACKGROUND_ATTRIB					CONSTLIT("background")
#define SCALE_ATTRIB						CONSTLIT("scale")

#define CIRCLE_OP							CONSTLIT("Circle")
#define CIRCLE_GRADIENT_OP					CONSTLIT("CircleGradient")
#define MULTIPLY_OP							CONSTLIT("Multiply")
#define NOISE_OP							CONSTLIT("Noise")

ALERROR CZoneGrid::ApplyZoneOperation (CXMLElement *pOp, CG16bitImage &DestMap)

//	ApplyZoneOperation
//
//	Applies a zone operation on the given image

	{
	if (strEquals(pOp->GetTag(), CIRCLE_OP))
		return ZoneOpCircle(pOp, DestMap);
	else if (strEquals(pOp->GetTag(), CIRCLE_GRADIENT_OP))
		return ZoneOpCircleGradient(pOp, DestMap);
	else if (strEquals(pOp->GetTag(), MULTIPLY_OP))
		return ZoneOpMultiply(pOp, DestMap);
	else if (strEquals(pOp->GetTag(), NOISE_OP))
		return ZoneOpNoise(pOp, DestMap);
	else
		return ERR_FAIL;
	}

bool CZoneGrid::ChooseRandomPoint (int *retx, int *rety) const

//	ChooseRandomPoint
//
//	Picks a random point in the zone in proportion to the zone value
//	Returns FALSE if a point could not be found (e.g., if the zone
//	is empty)

	{
	int iTries = 100;
	int cxRange = m_cxSize / 2;
	int cyRange = m_cySize / 2;

	while (iTries--)
		{
		int x = mathRandom(-cxRange, cxRange - 1);
		int y = mathRandom(-cyRange, cyRange - 1);
		int iChance = GetValueRaw(x, y);
		if (iChance > 0 && mathRandom(1, 100) <= iChance)
			{
			*retx = x;
			*rety = y;
			return true;
			}

		//	Loop and try again
		}

	return false;
	}

ALERROR CZoneGrid::CreateFromXML (CXMLElement *pDesc, int cxWidth, int cyHeight, int iCellSize)

//	CreateFromXML
//
//	Creates a zone grid from XML descriptor

	{
	ALERROR error;

	ASSERT(m_ZoneMap.IsEmpty());
	ASSERT(cxWidth > 0 && cyHeight > 0 && iCellSize > 0);

	m_cxSize = cxWidth;
	m_cySize = cyHeight;
	m_iCellSize = iCellSize;

	if (error = CreateZoneMap(m_ZoneMap))
		return error;

	//	Get and apply the zone definition element

	if (pDesc && pDesc->GetContentElementCount() > 0)
		if (error = ApplyZoneOperation(pDesc->GetContentElement(0), m_ZoneMap))
			return error;

	return NOERROR;
	}

ALERROR CZoneGrid::CreateZone (int cxSize, int cySize, int iCellSize)

//	CreateZone
//
//	Creates a zone if necessary

	{
	ALERROR error;

	//	If the caller does not pass in sizes, then we assume that this
	//	cell has been loaded by LoadFromXML. Otherwise, we use
	//	the given dimensions

	if (cxSize != -1)
		m_cxSize = cxSize;
	if (cySize != -1)
		m_cySize = cySize;
	if (iCellSize != -1)
		m_iCellSize = iCellSize;

	ASSERT(m_cxSize != -1 && m_cySize != -1 && m_iCellSize != -1);

	if (!m_ZoneMap.IsEmpty())
		return NOERROR;

	//	Allocate a zone map

	if (error = CreateZoneMap(m_ZoneMap))
		return error;

	//	Now apply operations on the zone map

	if (m_pDesc)
		{
		if (m_pDesc->GetContentElementCount() > 0)
			if (error = ApplyZoneOperation(m_pDesc->GetContentElement(0), m_ZoneMap))
				return error;
		}

	return NOERROR;
	}

ALERROR CZoneGrid::CreateZoneMap (CG16bitImage &ZoneMap)

//	CreateZoneMap
//
//	Allocates a zone map

	{
	ASSERT(ZoneMap.IsEmpty());

	int cxAlloc = AlignUp(m_cxSize, m_iCellSize) / m_iCellSize;
	int cyAlloc = AlignUp(m_cySize, m_iCellSize) / m_iCellSize;
	return ZoneMap.CreateBlankAlpha(cxAlloc, cyAlloc);
	}

int CZoneGrid::GetValue (int x, int y) const

//	GetValue
//
//	Returns the zone value at the given coordinates

	{
	int xMap, yMap;
	MapCoord(x, y, &xMap, &yMap);
	if (xMap < 0 || xMap >= m_ZoneMap.GetWidth() || yMap < 0 || yMap >= m_ZoneMap.GetHeight())
		return 0;

	BYTE *pPos = m_ZoneMap.GetAlphaRow(yMap) + xMap;
	return (int)*pPos;
	}

int CZoneGrid::GetValueRaw (int x, int y) const

//	GetValue
//
//	Returns the zone value at the given coordinates without checking for
//	bounds or initialization.

	{
	int xMap, yMap;
	MapCoord(x, y, &xMap, &yMap);
	BYTE *pPos = m_ZoneMap.GetAlphaRow(yMap) + xMap;
	return (int)*pPos;
	}

ALERROR CZoneGrid::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Loads from XML

	{
	m_sName = pDesc->GetAttribute(NAME_ATTRIB);
	m_cxSize = pDesc->GetAttributeIntegerBounded(SIZE_X_ATTRIB, 1, -1);
	m_cySize = pDesc->GetAttributeIntegerBounded(SIZE_Y_ATTRIB, 1, -1);
	m_iCellSize = pDesc->GetAttributeIntegerBounded(CELL_SIZE_ATTRIB, 1, Min(m_cxSize, m_cySize));

	m_pDesc = pDesc->OrphanCopy();

	return NOERROR;
	}

void CZoneGrid::MapCoord (int x, int y, int *retx, int *rety) const

//	MapCoord
//
//	Converts from global coords to zone map coords (does not check to see if we are
//	inside the map bounds)

	{
	*retx = (x + (m_cxSize / 2)) / m_iCellSize;
	*rety = ((m_cySize / 2) - y) / m_iCellSize;
	}

ALERROR CZoneGrid::ZoneOpCircle (CXMLElement *pOp, CG16bitImage &DestMap)

//	ZoneOpCircle
//
//	Define a solid circle zone

	{
	int xCenter, yCenter;

	MapCoord(pOp->GetAttributeInteger(X_ATTRIB), pOp->GetAttributeInteger(Y_ATTRIB), &xCenter, &yCenter);
	int iRadius = MapDim(pOp->GetAttributeInteger(RADIUS_ATTRIB));
	BYTE byFore = pOp->GetAttributeIntegerBounded(FOREGROUND_ATTRIB, 0, 100);
	BYTE byBack = pOp->GetAttributeIntegerBounded(BACKGROUND_ATTRIB, 0, 100);

	::DrawFilledRect8bit(DestMap, 0, 0, DestMap.GetWidth(), DestMap.GetHeight(), byBack);
	::DrawFilledCircle8bit(DestMap, xCenter, yCenter, iRadius, byFore);

	return NOERROR;
	}

ALERROR CZoneGrid::ZoneOpCircleGradient (CXMLElement *pOp, CG16bitImage &DestMap)

//	ZoneOpCircleGradient
//
//	Define a circle gradient zone

	{
	int xCenter, yCenter;

	MapCoord(pOp->GetAttributeInteger(X_ATTRIB), pOp->GetAttributeInteger(Y_ATTRIB), &xCenter, &yCenter);
	int iRadius = MapDim(pOp->GetAttributeInteger(RADIUS_ATTRIB));
	BYTE byCenter = pOp->GetAttributeIntegerBounded(CENTER_ATTRIB, 0, 100);
	BYTE byEdge = pOp->GetAttributeIntegerBounded(EDGE_ATTRIB, 0, 100);

	::DrawFilledRect8bit(DestMap, 0, 0, DestMap.GetWidth(), DestMap.GetHeight(), byEdge);
	::DrawGradientCircle8bit(DestMap, xCenter, yCenter, iRadius, byCenter, byEdge);

	return NOERROR;
	}

ALERROR CZoneGrid::ZoneOpMultiply (CXMLElement *pOp, CG16bitImage &DestMap)

//	ZoneOpMultiply
//
//	Multiple a set of zones together

	{
	ALERROR error;
	int i;

	if (pOp->GetContentElementCount() == 0)
		return NOERROR;

	//	Apply the first operation to the destination

	if (error = ApplyZoneOperation(pOp->GetContentElement(0), DestMap))
		return error;

	//	Now apply subsequent operations, multiplying as we go

	for (i = 1; i < pOp->GetContentElementCount(); i++)
		{
		CXMLElement *pSubOp = pOp->GetContentElement(i);

		CG16bitImage NewMap;
		if (error = CreateZoneMap(NewMap))
			return error;

		if (error = ApplyZoneOperation(pSubOp, NewMap))
			return error;

		//	Multiply with destination

		BYTE *pSrc = NewMap.GetAlphaRow(0);
		BYTE *pDest = DestMap.GetAlphaRow(0);
		BYTE *pDestEnd = DestMap.GetAlphaRow(DestMap.GetHeight());
		while (pDest < pDestEnd)
			{
			DWORD dwResult = ((DWORD)(*pSrc) * (DWORD)(*pDest)) / 100;
			pSrc++;
			*pDest++ = (BYTE)dwResult;
			}
		}

	return NOERROR;
	}

ALERROR CZoneGrid::ZoneOpNoise (CXMLElement *pOp, CG16bitImage &DestMap)

//	ZoneOpNoise
//
//	Fill area with perlin noise

	{
	int iScale = pOp->GetAttributeIntegerBounded(SCALE_ATTRIB, 1, -1);
	return NOERROR;
	}

