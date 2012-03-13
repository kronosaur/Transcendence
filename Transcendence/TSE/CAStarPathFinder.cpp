//	CAStarPathFinder.cpp
//
//	CAStarPathFinder class

#include "PreComp.h"

const int ADJACENT_NODE_AXIS_COST =						6;

const int ADJACENT_NODE_DIAG_COST =						(int)((1.4142 * ADJACENT_NODE_AXIS_COST) + 0.5);
const Metric ADJACENT_NODE_DIST =						(ADJACENT_NODE_AXIS_COST * LIGHT_SECOND);

const int MAP_SIZE =									30;
const int MAP_SCALE =									2;
const int MAP_OFFSET_X =								450;
const int MAP_OFFSET_Y =								450;

const DWORD MAP_NODE_CLOSED_FLAG =						0x01;

//	5000 loops is too small for some scenarios (including Arena)
//	So we set the limit to 10000.
const int MAX_LOOP_COUNT =								10000;

const int ADJACENT_NODE_COUNT =							8;
static int ADJACENT_NODE_DIR_X[] = { -1,  0, +1, -1, +1, -1,  0, +1 };
static int ADJACENT_NODE_DIR_Y[] = { -1, -1, -1,  0,  0, +1, +1, +1 };

CAStarPathFinder::CAStarPathFinder (void) :
		m_pOpenList(NULL),
		m_pClosedList(NULL),
		m_pClosedMap(NULL)

//	CAStarPathFinder constructor

	{
#ifdef DEBUG_ASTAR_PERF
	m_iCallsToIsPathClear = 0;
	m_iClosedListCount = 0;
	m_iOpenListCount = 0;
#endif
	}

CAStarPathFinder::~CAStarPathFinder (void)

//	CAStarPathFinder destructor

	{
	DeleteList(m_pOpenList);
	DeleteList(m_pClosedList);

	if (m_pClosedMap)
		delete m_pClosedMap;
	}

void CAStarPathFinder::AddObstacle (const CVector &vUR, const CVector &vLL)

//	AddObstacle
//
//	Adds an obstacle

	{
	SObstacle *pObstacle = m_Obstacles.Insert();
	pObstacle->vLL = vLL;
	pObstacle->vUR = vUR;
	}

void CAStarPathFinder::AddToClosedList (SNode *pNew)

//	AddToClosedList
//
//	Add a node to the closed list

	{
	AddToList(m_pClosedList, pNew);

	int x = pNew->x + MAP_OFFSET_X;
	int y = pNew->y + MAP_OFFSET_Y;
	DWORD dwFlags = m_pClosedMap->GetTile(x, y);
	m_pClosedMap->SetTile(x, y, dwFlags | MAP_NODE_CLOSED_FLAG);

#ifdef DEBUG_ASTAR_PERF
	m_iClosedListCount++;
#endif
	}

void CAStarPathFinder::AddToList (SNodeRoot &pRoot, SNode *pNew, SNode *pAddAfter)

//	AddToList
//
//	Adds to a list

	{
	if (pAddAfter == NULL)
		{
		pNew->pNext = pRoot;
		pNew->pPrev = NULL;
		pRoot = pNew;
		}
	else
		{
		pNew->pNext = pAddAfter->pNext;
		pNew->pPrev = pAddAfter;
		pAddAfter->pNext = pNew;
		}
	}

void CAStarPathFinder::AddToOpenList (SNode *pNew)

//	AddToOpenList
//
//	Adds a new node to the open list

	{
	//	Figure out where to insert the new node (so that
	//	the list is in total cost order)

	SNode *pNext = m_pOpenList;
	SNode *pPrev = NULL;
	while (pNext && pNext->iTotalCost <= pNew->iTotalCost)
		{
		pPrev = pNext;
		pNext = pNext->pNext;
		}

	//	Insert

	AddToList(m_pOpenList, pNew, pPrev);

#ifdef DEBUG_ASTAR_PERF
	m_iOpenListCount++;
#endif
	}

int CAStarPathFinder::CalcHeuristic (const CVector &vPos, const CVector &vDest)

//	CalcHeuristic
//
//	Estimate the cost of getting from vPos to vDest

	{
	CVector vDist = vDest - vPos;
	return (int)(vDist.Length() / LIGHT_SECOND);
	}

void CAStarPathFinder::CollapsePath (TArray<SNode *> &Path, int iStart, int iEnd)

//	CollapsePath
//
//	Removes superflous nodes in Path

	{
	int i;

	if (iEnd <= iStart + 1)
		return;
	else if (IsPathClear(Path[iStart]->vPos, Path[iEnd]->vPos))
		{
		for (i = iStart + 1; i < iEnd; i++)
			Path[i] = NULL;
		}
	else
		{
		int iMidPoint = iStart + (iEnd - iStart) / 2;
		CollapsePath(Path, iStart, iMidPoint);
		CollapsePath(Path, iMidPoint + 1, iEnd);
		}
	}

void CAStarPathFinder::CreateInOpenList (const CVector &vEnd, SNode *pCurrent, int xDir, int yDir)

//	CreateInOpenList
//
//	Creates a new node in the given direction from pCurrent. If the new node
//	is not already on the Open or Closed lists, and if it not blocked, then we
//	add the node to the Open list (otherwise, we discard)

	{
	int x = pCurrent->x + xDir;
	int y = pCurrent->y + yDir;

	//	If this node is in the closed list, then bail

	if (IsInClosedList(x, y))
		return;

	//	If this node is in the open list, then bail

	SNode *pNext = m_pOpenList;
	while (pNext)
		{
		if (pNext->x == x && pNext->y == y)
			return;

		pNext = pNext->pNext;
		}

	//	Compute the position of the new node

	CVector vPos = pCurrent->vPos + CVector(xDir * ADJACENT_NODE_DIST, yDir * ADJACENT_NODE_DIST);

	//	See if the node is blocked

	if (!IsPointClear(vPos))
		return;

	//	Create a new node

	SNode *pNew = new SNode;
	pNew->x = x;
	pNew->y = y;
	pNew->vPos = vPos;
	pNew->iCostFromStart = pCurrent->iCostFromStart + ((xDir == 0 || yDir == 0) ? ADJACENT_NODE_AXIS_COST : ADJACENT_NODE_DIAG_COST);
	pNew->iHeuristic = CalcHeuristic(vPos, vEnd);
	pNew->iTotalCost = pNew->iCostFromStart + pNew->iHeuristic;
	pNew->pParent = pCurrent;

	//	Add it to the open list

	AddToOpenList(pNew);
	}

void CAStarPathFinder::DeleteFromList (SNodeRoot &pRoot, SNode *pDelete)

//	DeleteFromList
//
//	Delete from a list (note: we do not free the node)

	{
	if (pDelete->pPrev == NULL)
		{
		pRoot = pDelete->pNext;
		if (pRoot)
			pRoot->pPrev = NULL;
		}
	else
		{
		pDelete->pPrev->pNext = pDelete->pNext;
		if (pDelete->pNext)
			pDelete->pNext->pPrev = pDelete->pPrev;
		}

	pDelete->pNext = NULL;
	pDelete->pPrev = NULL;
	}

void CAStarPathFinder::DeleteList (SNodeRoot &pRoot)

//	DeleteList
//
//	Deletes the given list

	{
	SNode *pNext = pRoot;
	while (pNext)
		{
		SNode *pDelete = pNext;
		pNext = pNext->pNext;
		delete pDelete;
		}

	pRoot = NULL;
	}

int CAStarPathFinder::FindPath (const CVector &vStart, const CVector &vEnd, CVector **retPathList)

//	FindPath
//
//	Returns a path that avoids all obstacles

	{
	int i;

#ifdef DEBUG_ASTAR_PERF
	DWORD dwStartTime = ::GetTickCount();
#endif

	//	Initialize the open list and closed map

	Reset();

	//	Start with a node at the start position

	SNode *pStart = new SNode;
	pStart->x = 0;
	pStart->y = 0;
	pStart->vPos = vStart;
	pStart->iCostFromStart = 0;
	pStart->iHeuristic = CalcHeuristic(vStart, vEnd);
	pStart->iTotalCost = pStart->iHeuristic;
	pStart->pParent = NULL;

	AddToOpenList(pStart);

	//	Loop

	int iLoopCount = 0;
	while (m_pOpenList)
		{
		SNode *pCurrent = m_pOpenList;

		//	Are we there yet?

		if (IsPathClear(pCurrent->vPos, vEnd) || iLoopCount >= MAX_LOOP_COUNT)
			{
#ifdef DEBUG_ASTAR_PERF
			char szBuffer[1024];
			wsprintf(szBuffer, "Total Time: %d ms\nLoops: %d\nCalls to IsPathClear: %d\nOpen List: %d\nClosed List: %d\n", 
					::GetTickCount() - dwStartTime,
					iLoopCount, 
					m_iCallsToIsPathClear, 
					m_iOpenListCount, 
					m_iClosedListCount);
			::OutputDebugString(szBuffer);
#endif

			return OptimizePath(vEnd, pCurrent, retPathList);
			}

		//	If not, keep searching

		else
			{
			//	Move to closed list

			DeleteFromList(m_pOpenList, pCurrent);
#ifdef DEBUG_ASTAR_PERF
			m_iOpenListCount--;
#endif
			AddToClosedList(pCurrent);

			for (i = 0; i < ADJACENT_NODE_COUNT; i++)
				CreateInOpenList(vEnd, pCurrent, ADJACENT_NODE_DIR_X[i], ADJACENT_NODE_DIR_Y[i]);
			}

		iLoopCount++;

#ifdef DEBUG_ASTAR_PATH
		//	Create a nav beacon so we know the path
		CStationType *pType = g_pUniverse->FindStationType(0x2004);
		CStation *pBeacon;
		g_pUniverse->GetCurrentSystem()->CreateStation(pType, pCurrent->vPos, (CSpaceObject **)&pBeacon);
		pBeacon->SetName(strPatternSubst(CONSTLIT("Path %d"), iLoopCount));
#endif
		}

	//	If we get this far, no path found

	return -1;
	}

bool CAStarPathFinder::IsInClosedList (int x, int y)

//	IsInClosedList
//
//	Returns TRUE if these coordinates are in the closed list

	{
	DWORD dwFlags = m_pClosedMap->GetTile(x + MAP_OFFSET_X, y + MAP_OFFSET_Y);
	return (dwFlags & MAP_NODE_CLOSED_FLAG ? true : false);
	}

bool CAStarPathFinder::IsPathClear (const CVector &vStart, const CVector &vEnd)

//	IsPathClear
//
//	Returns TRUE if the straight path from vStart to vEnd is unobstructed

	{
	int i;

#ifdef DEBUG_ASTAR_PERF
	m_iCallsToIsPathClear++;
#endif

	//	Check to see if the line intersects any of the obstacles

	for (i = 0; i < m_Obstacles.GetCount(); i++)
		{
		if (LineIntersectsRect(vStart, vEnd, m_Obstacles[i].vUR, m_Obstacles[i].vLL))
			return false;
		}

	return true;
	}

bool CAStarPathFinder::IsPointClear (const CVector &vPos)

//	IsPointClear
//
//	Returns TRUE if the point is clear (not in an obstacle)

	{
	int i;

	//	Check to see if the line intersects any of the obstacles

	for (i = 0; i < m_Obstacles.GetCount(); i++)
		{
		if (IntersectRect(m_Obstacles[i].vUR, m_Obstacles[i].vLL, vPos))
			return false;
		}

	return true;
	}

bool CAStarPathFinder::LineIntersectsRect (const CVector &vStart, const CVector &vEnd, const CVector &vUR, const CVector &vLL)

//	LineIntersectsRect
//
//	Returns TRUE if the line intersects the given rect

	{
	if (IntersectLine(vStart, vEnd, vUR, CVector(vUR.GetX(), vLL.GetY())))
		return true;

	if (IntersectLine(vStart, vEnd, CVector(vUR.GetX(), vLL.GetY()), vLL))
		return true;

	if (IntersectLine(vStart, vEnd, vLL, CVector(vLL.GetX(), vUR.GetY())))
		return true;

	if (IntersectLine(vStart, vEnd, CVector(vLL.GetX(), vUR.GetY()), vUR))
		return true;

	return false;
	}

int CAStarPathFinder::OptimizePath (const CVector &vEnd, SNode *pFinal, CVector **retPathList)

//	OptimizePath
//
//	Collapses adjacent nodes to form an optimized path

	{
	int i;
	TArray<SNode *> Path;

#ifdef DEBUG_ASTAR_PERF
	DWORD dwStartTime = ::GetTickCount();
#endif

	//	Follow the path backwards to the beginning

	SNode *pPrev = pFinal;
	while (pPrev)
		{
		Path.Insert(pPrev, 0);
		pPrev = pPrev->pParent;
		}

	//	Try to collapse the path

	CollapsePath(Path, 0, Path.GetCount() - 1);

	//	Count the number of points left (skipping the first point)

	int iCount = 1;
	for (i = 1; i < Path.GetCount(); i++)
		{
		if (Path[i])
			iCount++;
		}

	CVector *pPathList = new CVector [iCount];
	int iDest = 0;
	for (i = 1; i < Path.GetCount(); i++)
		if (Path[i])
			pPathList[iDest++] = Path[i]->vPos;

	pPathList[iDest] = vEnd;

	//	Done

#ifdef DEBUG_ASTAR_PERF
	char szBuffer[1024];
	wsprintf(szBuffer, "OptimizePath time: %d ms\n", ::GetTickCount() - dwStartTime);
	::OutputDebugString(szBuffer);
#endif

	*retPathList = pPathList;
	return iCount;
	}

void CAStarPathFinder::Reset (void)

//	Reset
//
//	Resets internal variables

	{
	DeleteList(m_pOpenList);
	DeleteList(m_pClosedList);
	if (m_pClosedMap)
		delete m_pClosedMap;

	m_pClosedMap = new CTileMap(MAP_SIZE, MAP_SCALE);
	}
