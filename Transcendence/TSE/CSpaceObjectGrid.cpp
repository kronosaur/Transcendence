//	CSpaceObjectGrid.cpp
//
//	CSpaceObjectGrid class

#include "PreComp.h"

CSpaceObjectGrid::CSpaceObjectGrid (int iGridSize, Metric rCellSize, Metric rCellBorder)

//	CSpaceObjectGrid constructor

	{
	ASSERT(iGridSize > 0);
	ASSERT(rCellSize > 0.0);
	ASSERT(rCellBorder > 0.0);

	int iTotal = iGridSize * iGridSize;
	m_pGrid = new CSpaceObjectList [iTotal];

	m_iGridSize = iGridSize;
	m_rCellSize = rCellSize;
	m_rCellBorder = rCellBorder;

	Metric rGridSize = iGridSize * m_rCellSize;
	m_vGridSize = CVector(rGridSize, rGridSize);
	m_vLL = CVector(-(rGridSize / 2.0), -(rGridSize / 2.0));
	m_vUR = CVector(rGridSize / 2.0, rGridSize / 2.0);
	}

CSpaceObjectGrid::~CSpaceObjectGrid (void)

//	CSpaceObjectGrid destructor

	{
	delete [] m_pGrid;
	}

void CSpaceObjectGrid::DeleteAll (void)

//	DeleteAll
//
//	Remove all objects

	{
	int iTotal = m_iGridSize * m_iGridSize;
	for (int i = 0; i < iTotal; i++)
		m_pGrid[i].RemoveAll();

	m_Outer.RemoveAll();
	}

bool CSpaceObjectGrid::GetGridCoord (const CVector &vPos, int *retx, int *rety)

//	GetGridCoord
//
//	Returns the grid cell coordinates of vPos (which is in grid-origin coords).
//	We return TRUE if inside the valid area of the grid

	{
	int x = (int)(vPos.GetX() / m_rCellSize);
	int y = (int)(vPos.GetY() / m_rCellSize);

	*retx = x;
	*rety = y;

	return (x >= 0 && y >= 0 && x < m_iGridSize && y < m_iGridSize);
	}

CSpaceObjectList &CSpaceObjectGrid::GetList (const CVector &vPos)

//	GetList
//
//	Returns the object list at the given position

	{
	CVector vGridPos = vPos - m_vLL;
	int x = (int)(vGridPos.GetX() / m_rCellSize);
	int y = (int)(vGridPos.GetY() / m_rCellSize);

	if (x < 0 || y < 0 || x >= m_iGridSize || y >= m_iGridSize)
		return m_Outer;
	else
		return GetList(x, y);
	}

void CSpaceObjectGrid::EnumStart (SSpaceObjectGridEnumerator &i, const CVector &vUR, const CVector &vLL, DWORD dwFlags)

//	EnumStart
//
//	Begins enumeration

	{
	//	Init params and options

	i.vLL = vLL;
	i.vUR = vUR;
	i.bCheckBox = ((dwFlags & gridNoBoxCheck) ? false : true);

	//	First we need to generate a box that will contain enough grid cells
	//	so that we can find the objects even if their center is outside
	//	the input range

	CVector vGridLL = vLL - m_vLL - CVector(m_rCellBorder, m_rCellBorder);
	CVector vGridUR = vUR - m_vLL + CVector(m_rCellBorder, m_rCellBorder);

	int xStart = (int)(vGridLL.GetX() / m_rCellSize);
	int yStart = (int)(vGridLL.GetY() / m_rCellSize);

	int xEnd = (int)(vGridUR.GetX() / m_rCellSize);
	int yEnd = (int)(vGridUR.GetY() / m_rCellSize);

	//	Generate a list of all grid cells to traverse

	int iMaxSize = (xEnd - xStart + 1) * (yEnd - yStart + 1);
	i.pGridIndexList = new CSpaceObjectList * [iMaxSize];
	i.iGridIndexCount = 0;
	bool bOuterAdded = (m_Outer.GetCount() == 0);

	//	Always start with the center cell

	int xCenter = xStart + (xEnd - xStart) / 2;
	int yCenter = yStart + (yEnd - yStart) / 2;
	if (xCenter >= 0 && yCenter >= 0 && xCenter < m_iGridSize && yCenter < m_iGridSize)
		{
		CSpaceObjectList *pList = &GetList(xCenter, yCenter);
		if (pList->GetCount() > 0)
			i.pGridIndexList[i.iGridIndexCount++] = pList;
		}
	else if (!bOuterAdded)
		{
		i.pGridIndexList[i.iGridIndexCount++] = &m_Outer;
		bOuterAdded = true;
		}

	//	Add the remaining cells

	for (int y = yStart; y <= yEnd; y++)
		for (int x = xStart; x <= xEnd; x++)
			if (x == xCenter && y == yCenter)
				NULL;
			else if (x >= 0 && y >= 0 && x < m_iGridSize && y < m_iGridSize)
				{
				CSpaceObjectList *pList = &GetList(x, y);
				if (pList->GetCount() > 0)
					i.pGridIndexList[i.iGridIndexCount++] = pList;
				}
			else if (!bOuterAdded)
				{
				i.pGridIndexList[i.iGridIndexCount++] = &m_Outer;
				bOuterAdded = true;
				}

	//	Initialize

	ASSERT(i.iGridIndexCount <= iMaxSize);
	i.bMore = (i.iGridIndexCount > 0);

	//	Set the initial list

	if (i.bMore)
		{
		i.iGridIndex = 0;
		i.pList = i.pGridIndexList[0];
		i.iIndex = -1;
		i.iListCount = i.pList->GetCount();
		i.pObj = NULL;

		//	Start with the first

		EnumGetNext(i);
		}
	}

CSpaceObject *CSpaceObjectGrid::EnumGetNext (SSpaceObjectGridEnumerator &i)

//	EnumGetNext
//
//	Returns the current object in the enumeration and advances
//	to the next one

	{
	//	Get the current object

	CSpaceObject *pCurrentObj = i.pObj;

	//	Next

	do
		{
		i.iIndex++;
		if (i.iIndex < i.iListCount)
			{
			ASSERT(i.iIndex >= 0);

			i.pObj = i.pList->GetObj(i.iIndex);
			if (!i.pObj->IsDestroyed() 
					&& (!i.bCheckBox || i.pObj->InBox(i.vUR, i.vLL)))
				return pCurrentObj;
			}
		else
			{
			i.iGridIndex++;
			if (i.iGridIndex < i.iGridIndexCount)
				{
				ASSERT(i.iGridIndex >= 0);

				i.pList = i.pGridIndexList[i.iGridIndex];
				i.iListCount = i.pList->GetCount();
				i.iIndex = -1;
				}
			else
				{
				i.bMore = false;
				return pCurrentObj;
				}
			}
		}
	while (true);
	}

CSpaceObject *CSpaceObjectGrid::EnumGetNextInBoxPoint (SSpaceObjectGridEnumerator &i)

//	EnumGetNextInBoxPoint
//
//	Returns the next object whose center fits inside the defined box

	{
	//	Get the current object

	CSpaceObject *pCurrentObj = i.pObj;

	//	Next

	do
		{
		i.iIndex++;
		if (i.iIndex < i.iListCount)
			{
			ASSERT(i.iIndex >= 0);

			i.pObj = i.pList->GetObj(i.iIndex);
			if (!i.pObj->IsDestroyed() && i.pObj->InBoxPoint(i.vUR, i.vLL))
				return pCurrentObj;
			}
		else
			{
			i.iGridIndex++;
			if (i.iGridIndex < i.iGridIndexCount)
				{
				ASSERT(i.iGridIndex >= 0);

				i.pList = i.pGridIndexList[i.iGridIndex];
				i.iListCount = i.pList->GetCount();
				i.iIndex = -1;
				}
			else
				{
				i.bMore = false;
				return pCurrentObj;
				}
			}
		}
	while (true);
	}

bool CSpaceObjectGrid::EnumGetNextList (SSpaceObjectGridEnumerator &i)

//	EnumGetNextList
//
//	Sets up the next list. Returns FALSE if we have reached the end

	{
	i.iGridIndex++;
	if (i.iGridIndex >= i.iGridIndexCount)
		{
		i.bMore = false;
		return false;
		}

	i.pList = i.pGridIndexList[i.iGridIndex];
	i.iListCount = i.pList->GetCount();
	i.iIndex = 0;

	return true;
	}

void CSpaceObjectGrid::GetObjectsInBox (const CVector &vUR, const CVector &vLL, CSpaceObjectList &Result)

//	GetObjectsInBox
//
//	Returns all the objects in the given box 
//
//	Note: We do not check for duplicates before adding to Result

	{
	int i, x, y;

	//	First we need to generate a box that will contain enough grid cells
	//	so that we can find the objects even if their center is outside
	//	the input range

	CVector vGridLL = vLL - m_vLL - CVector(m_rCellBorder, m_rCellBorder);
	CVector vGridUR = vUR - m_vLL + CVector(m_rCellBorder, m_rCellBorder);

	int xLL = (int)(vGridLL.GetX() / m_rCellSize);
	int yLL = (int)(vGridLL.GetY() / m_rCellSize);

	int xUR = (int)(vGridUR.GetX() / m_rCellSize);
	int yUR = (int)(vGridUR.GetY() / m_rCellSize);

	bool bCheckOuter = true;
	for (y = yLL; y <= yUR; y++)
		for (x = xLL; x <= xUR; x++)
			{
			CSpaceObjectList *pList;
			if (x >= 0 && y >= 0 && x < m_iGridSize && y < m_iGridSize)
				pList = &GetList(x, y);
			else if (bCheckOuter)
				{
				pList = &m_Outer;
				bCheckOuter = false;
				}
			else
				pList = NULL;

			if (pList)
				{
				for (i = 0; i < pList->GetCount(); i++)
					{
					CSpaceObject *pObj = pList->GetObj(i);
					if (!pObj->IsDestroyed() && pObj->InBox(vUR, vLL))
						Result.FastAdd(pObj);
					}
				}
			}
	}
