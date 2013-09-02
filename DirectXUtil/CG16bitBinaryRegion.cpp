//	CG16bitBinaryRegion.cpp
//
//	Implementation of raw 16-bit image region

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

inline int IndexForward (int iIndex, int iCount) { return ((iIndex + 1) % iCount); }
inline int IndexBackward (int iIndex, int iCount) { return ((iIndex - 1 + iCount) % iCount); }
inline int IndexMove (int iIndex, int iDir, int iCount)
	{
	if (iDir > 0)
		return IndexForward(iIndex, iCount);
	else
		return IndexBackward(iIndex, iCount);
	}

CG16bitBinaryRegion::CG16bitBinaryRegion (void) : m_pList(NULL), m_iCount(0)

//	CG16bitBinaryRegion constructor

	{
	}

CG16bitBinaryRegion::~CG16bitBinaryRegion (void)

//	CG16bitBinaryRegion destructor

	{
	if (m_pList)
		delete m_pList;
	}

void CG16bitBinaryRegion::CreateFromPolygon (int iVertexCount, SPoint *pVertexList)

//	CreateFromPolygon
//
//	Creates a new region from the given polygon.
//
//	Michael Abrash. Graphics Programming Black Book. Chapter 40.

	{
	if (m_pList)
		{
		delete m_pList;
		m_pList = NULL;
		}

	m_iCount = CreateScanLinesFromPolygon(iVertexCount, pVertexList, &m_pList);
	if (m_iCount == 0)
		m_pList = NULL;
	}

void CG16bitBinaryRegion::CreateFromConvexPolygon (int iVertexCount, SPoint *pVertexList)

//	CreateFromConvexPolygon
//
//	Creates a new region from the given polygon.
//
//	Michael Abrash. Graphics Programming Black Book. Chapter 38.
//	"The Polygon Primeval" pp.709-721
//
//	Note that this code seems to produce a more accurate polygon than the simple
//	polygon code. [In particular, it has fewer gaps at the edges.]

	{
	if (m_pList)
		{
		delete m_pList;
		m_pList = NULL;
		}

	int i;

	ASSERT(iVertexCount >= 0);
	if (iVertexCount <= 0)
		return;

	//	Scan the vertex list to find the top and bottom of the
	//	polygon.

	int iMinIndexL = 0;
	int iMaxIndex = 0;
	int iMaxPointY = pVertexList[0].y;
	int iMinPointY = pVertexList[0].y;
	for (i = 1; i < iVertexCount; i++)
		{
		if (pVertexList[i].y < iMinPointY)
			iMinPointY = pVertexList[iMinIndexL = i].y;
		else if (pVertexList[i].y > iMaxPointY)
			iMaxPointY = pVertexList[iMaxIndex = i].y;
		}

	//	Avoid 0-height polygons

	if (iMinPointY == iMaxPointY)
		return;

	//	Scan in ascending order to find the last top-edge point

	int iMinIndexR = iMinIndexL;
	while (pVertexList[iMinIndexR].y == iMinPointY)
		iMinIndexR = IndexForward(iMinIndexR, iVertexCount);
	iMinIndexR = IndexBackward(iMinIndexR, iVertexCount);	//	Back up to last top-edge point

	//	Now scan in descending order to find the first top-edge point

	while (pVertexList[iMinIndexL].y == iMinPointY)
		iMinIndexL = IndexBackward(iMinIndexL, iVertexCount);
	iMinIndexL = IndexForward(iMinIndexL, iVertexCount);

	//	Figure out which direction through the vertex list from the
	//	top vertex is the left edge and which is the right.

	int iLeftEdgeDir = -1;
	int iTopIsFlat;
	int iNextIndex, iPreviousIndex, iCurrentIndex;
	if ((iTopIsFlat = (pVertexList[iMinIndexL].x != pVertexList[iMinIndexR].x) ? 1 : 0) == 1)
		{
		//	If the top is flat see which other ends is leftmost

		if (pVertexList[iMinIndexL].x > pVertexList[iMinIndexR].x)
			{
			iLeftEdgeDir = 1;			//	Left edge runs through vertex list.
			int iTemp = iMinIndexL;		//	Swap the indeces so iMinIndexL
			iMinIndexL = iMinIndexR;	//		points to the start of the left
			iMinIndexR = iTemp;			//		edge, similarly for iMinIndexR
			}
		}
	else
		{
		//	Point to the downward end of the first line of each of the 
		//	two edges down from the top

		iNextIndex = IndexForward(iMinIndexR, iVertexCount);
		iPreviousIndex = IndexBackward(iMinIndexL, iVertexCount);

		//	Calculate the x and y lengths from the top vertex to the end
		//	of the first line down each edge; use those to compare slopes
		//	and see which line is leftmost.

		int iDeltaXN = pVertexList[iNextIndex].x - pVertexList[iMinIndexL].x;
		int iDeltaYN = pVertexList[iNextIndex].y - pVertexList[iMinIndexL].y;
		int iDeltaXP = pVertexList[iPreviousIndex].x - pVertexList[iMinIndexL].x;
		int iDeltaYP = pVertexList[iPreviousIndex].y - pVertexList[iMinIndexL].y;
		if ((iDeltaXN * iDeltaYP - iDeltaYN * iDeltaXP) < 0)
			{
			iLeftEdgeDir = 1;			//	Left edge runs through vertex list.
			int iTemp = iMinIndexL;		//	Swap the indeces so iMinIndexL
			iMinIndexL = iMinIndexR;	//		points to the start of the left
			iMinIndexR = iTemp;			//		edge, similarly for iMinIndexR
			}
		}

	//	Set the # of scan lines in the polygon, skipping the bottom edge
	//	and also skipping the top vertex if the top isn't flat because in that
	//	case the top vertex has a right edge component, and set
	//	the top scan line to draw, which is likewise the second line of
	//	the polygon unless the top is flat.

	int iLineCount;
	if ((iLineCount = iMaxPointY - iMinPointY - 1 + iTopIsFlat) <= 0)
		return;

	int iYStart = iMinPointY + 1 - iTopIsFlat;

	//	Allocate space for all the lines and initialize the y coordinate
	//	of each run.

	m_pList = new SRun [iLineCount];
	m_iCount = iLineCount;
	for (i = 0; i < iLineCount; i++)
		m_pList[i].y = iYStart + i;

	//	Initial pointer for storing scan converted left-edge coords

	SRun *pRun = m_pList;

	//	Start from the top of the left edge

	iPreviousIndex = iCurrentIndex = iMinIndexL;

	//	Skip the first point of the first line unless the top is flat;
	//	if the top isn't flat, the top vertex is exactly on a right edge
	//	and isn't drawn.

	int iSkipFirst = (iTopIsFlat ? 0 : 1);

	//	Scan convert each line in the left edge from top to bottom

#ifdef DEBUG_SCAN
	::OutputDebugString("Scan convert left edge\n");
#endif

	do
		{
		iCurrentIndex = IndexMove(iCurrentIndex, iLeftEdgeDir, iVertexCount);

#ifdef DEBUG_SCAN
		{
		char szBuffer[1024];
		wsprintf(szBuffer, "iLineCount: %d; iPreviousIndex: %d; iCurrentIndex: %d; iMaxIndex: %d; iHeight: %d; iSkipFirst: %d\n",
				m_iCount, iPreviousIndex, iCurrentIndex, iMaxIndex,
				pVertexList[iCurrentIndex].y - pVertexList[iPreviousIndex].y,
				iSkipFirst);
		::OutputDebugString(szBuffer);
		}
#endif

		ScanEdge(pVertexList[iPreviousIndex].x,
				pVertexList[iPreviousIndex].y,
				pVertexList[iCurrentIndex].x,
				pVertexList[iCurrentIndex].y,
				true,
				iSkipFirst,
				&pRun);
		iPreviousIndex = iCurrentIndex;
		iSkipFirst = 0;
		}
	while (iCurrentIndex != iMaxIndex);

	//	Scan the right edge and store the boundary points in the list

	pRun = m_pList;
	iPreviousIndex = iCurrentIndex = iMinIndexR;
	iSkipFirst = (iTopIsFlat ? 0 : 1);

	//	Scan convert the right edge, top to bottom. x coordinates are
	//	adjusted 1 to the left, effectively causing scan conversion of
	//	the nearest points to the left of but not exactly on the edge

#ifdef DEBUG_SCAN
	::OutputDebugString("Scan convert right edge\n");
#endif

	do
		{
		iCurrentIndex = IndexMove(iCurrentIndex, -iLeftEdgeDir, iVertexCount);

#ifdef DEBUG_SCAN
		{
		char szBuffer[1024];
		wsprintf(szBuffer, "iLineCount: %d; iPreviousIndex: %d; iCurrentIndex: %d; iMaxIndex: %d; iHeight: %d; iSkipFirst: %d\n",
				m_iCount, iPreviousIndex, iCurrentIndex, iMaxIndex,
				pVertexList[iCurrentIndex].y - pVertexList[iPreviousIndex].y,
				iSkipFirst);
		::OutputDebugString(szBuffer);
		}
#endif

		ScanEdge(pVertexList[iPreviousIndex].x - 1,
				pVertexList[iPreviousIndex].y,
				pVertexList[iCurrentIndex].x - 1,
				pVertexList[iCurrentIndex].y,
				false,
				iSkipFirst,
				&pRun);
		iPreviousIndex = iCurrentIndex;
		iSkipFirst = 0;
		}
	while (iCurrentIndex != iMaxIndex);
	}

void CG16bitBinaryRegion::CreateFromSimplePolygon (int iVertexCount, SPoint *pVertexList)

//	CreateFromSimplePolygon
//
//	Creates a new region from the given polygon.
//
//	The polygon must be monotone-vertical, meaning polygons in which a scanline intersects the edges
//	exactly twice.
//
//	Michael Abrash. Graphics Programming Black Book. Chapter 41.

	{
	if (m_pList)
		{
		delete m_pList;
		m_pList = NULL;
		}

	m_iCount = CreateScanLinesFromSimplePolygon(iVertexCount, pVertexList, &m_pList);
	if (m_iCount == 0)
		m_pList = NULL;
	}

void CG16bitBinaryRegion::GetBounds (RECT *retrcRect) const

//	GetBounds
//
//	Returns the bounds

	{
	int xMax = MIN_COORD;
	int xMin = MAX_COORD;
	int yMax = MIN_COORD;
	int yMin = MAX_COORD;

	if (m_pList == NULL)
		return;

	SRun *pRun = m_pList;
	SRun *pRunEnd = pRun + m_iCount;
	for (; pRun < pRunEnd; pRun++)
		{
		if (pRun->xStart < xMin)
			xMin = pRun->xStart;

		if (pRun->xEnd > xMax)
			xMax = pRun->xEnd;

		if (pRun->y < yMin)
			yMin = pRun->y;

		if (pRun->y > yMax)
			yMax = pRun->y;
		}

	//	Done

	retrcRect->left = xMin;
	retrcRect->top = yMin;
	retrcRect->right = xMax;

	//	If we had a single line, yMin == yMax, so we always need to add 1
	//	to the bottom.

	retrcRect->bottom = yMax + 1;
	}

void CG16bitBinaryRegion::Fill (CG16bitImage &Dest, int x, int y, WORD wColor) const

//	Fill
//
//	Draws the region to the destination

	{
	if (m_pList == NULL)
		return;

	DWORD dwColor = MAKELONG(wColor, wColor);
	const RECT &rcClip = Dest.GetClipRect();

	SRun *pRun = m_pList;
	SRun *pRunEnd = pRun + m_iCount;
	for (; pRun < pRunEnd; pRun++)
		{
		int yAdj = y + pRun->y;
		int xStartAdj = x + pRun->xStart;
		int xEndAdj = x + pRun->xEnd;

		//	Make sure this row is in range

		if (yAdj < rcClip.top || yAdj >= rcClip.bottom
				|| xEndAdj <= rcClip.left || xStartAdj >= rcClip.right)
			continue;

		int xStart = max(xStartAdj, rcClip.left);
		int xEnd = min(xEndAdj, rcClip.right);

		//	Get the pointers

		WORD *pRow = Dest.GetRowStart(yAdj);
		WORD *pPos = pRow + xStart;
		WORD *pEnd = pRow + xEnd;

		//	Do the first pixel (if odd)

		if (xStart % 2)
			*pPos++ = wColor;

		if (pPos == pEnd)
			continue;

		DWORD *pPosDW = (DWORD *)pPos;

		//	Do the last pixel (if odd)

		if (xEnd % 2)
			{
			pEnd--;
			*pEnd = wColor;
			}

		DWORD *pEndDW = (DWORD *)pEnd;

		//	Do the middle

		while (pPosDW < pEndDW)
			*pPosDW++ = dwColor;
		}
	}

void CG16bitBinaryRegion::FillTrans (CG16bitImage &Dest, int x, int y, WORD wColor, DWORD byOpacity) const

//	FillTrans
//
//	Draws the region to the destination with the given transparency
//	255 = opaque

	{
	if (m_pList == NULL)
		return;

	DWORD dwAlpha = byOpacity;
	DWORD dwInvTrans = (byOpacity ^ 0xff);
	DWORD pxSource = CG16bitImage::BlendPixel(0, wColor, dwAlpha);
	DWORD dwRedGreenS = ((pxSource << 8) & 0x00f80000) | (pxSource & 0x000007e0);
	DWORD dwBlueS = (pxSource & 0x1f);

	const RECT &rcClip = Dest.GetClipRect();

	SRun *pRun = m_pList;
	SRun *pRunEnd = pRun + m_iCount;
	for (; pRun < pRunEnd; pRun++)
		{
		int yAdj = y + pRun->y;
		int xStartAdj = x + pRun->xStart;
		int xEndAdj = x + pRun->xEnd;

		//	Make sure this row is in range

		if (yAdj < rcClip.top || yAdj >= rcClip.bottom
				|| xEndAdj <= rcClip.left || xStartAdj >= rcClip.right)
			continue;

		int xStart = max(xStartAdj, rcClip.left);
		int xEnd = min(xEndAdj, rcClip.right);

		//	Get the pointers

		WORD *pRow = Dest.GetRowStart(yAdj);
		WORD *pPos = pRow + xStart;
		WORD *pEnd = pRow + xEnd;

		//	Do the line

		while (pPos < pEnd)
			{
			DWORD pxDest = *pPos;
			DWORD dwRedGreen = (((((pxDest << 8) & 0x00f80000) | (pxDest & 0x000007e0)) * dwInvTrans) >> 8) + dwRedGreenS;
			DWORD dwBlue = (((pxDest & 0x1f) * dwInvTrans) >> 8) + dwBlueS;
			*pPos++ = (WORD)(((dwRedGreen & 0x00f80000) >> 8) | (dwRedGreen & 0x000007e0) | dwBlue);
			}
		}
	}

void CG16bitBinaryRegion::ScanEdge (int X1,
							  int Y1,
							  int X2,
							  int Y2,
							  bool bSetX,
							  int iSkipFirst,
							  SRun **iopRun)

//	ScanEdge
//
//	Scan converts a polygon edge (see: CreateFromConvexPolygon)

	{
	int i;
	SRun *pRun = *iopRun;

	//	Direction in which x moves (y2 is always > y1, so y always counts up)

	int iDeltaX;
	int iAdvanceAmt = ((iDeltaX = X2 - X1) > 0) ? 1 : -1;

	int iHeight, iWidth;
	if ((iHeight = Y2 - Y1) <= 0)
		return;

	ASSERT(pRun >= m_pList && pRun < &m_pList[m_iCount]);
	ASSERT(&pRun[iHeight - iSkipFirst] < &m_pList[m_iCount+1]);

	//	Figure out whether the edge is vertical, diagonal, x-minor
	//	(mostly horizontal), or y-major (mostly vertical) and handle
	//	appropriately.

	if ((iWidth = abs(iDeltaX)) == 0)
		{
		//	The edge is vertical; special-case by just storing the same
		//	x coordinate for every scan line

		if (bSetX)
			for (i = iHeight - iSkipFirst; i-- > 0; pRun++)
				pRun->xStart = X1;
		else
			{
			X1++;
			for (i = iHeight - iSkipFirst; i-- > 0; pRun++)
				pRun->xEnd = X1;
			}
		}
	else if (iWidth == iHeight)
		{
		//	The edge is diagonal; special-case by advancing the x
		//	coordinate 1 pixel for each scan line

		if (iSkipFirst)
			X1 += iAdvanceAmt;

		if (bSetX)
			for (i = iHeight - iSkipFirst; i-- > 0; pRun++)
				{
				pRun->xStart = X1;
				X1 += iAdvanceAmt;
				}
		else
			{
			X1++;
			for (i = iHeight - iSkipFirst; i-- > 0; pRun++)
				{
				pRun->xEnd = X1;
				X1 += iAdvanceAmt;
				}
			}
		}
	else if (iHeight > iWidth)
		{
		//	Edge is closer to vertical than horizontal (y-major)

		int iErrorTerm;
		if (iDeltaX >= 0)
			iErrorTerm = 0;
		else
			iErrorTerm = -iHeight + 1;

		if (iSkipFirst)
			{
			if ((iErrorTerm += iWidth) > 0)
				{
				X1 += iAdvanceAmt;
				iErrorTerm -= iHeight;
				}
			}

		//	Scan the edge for each scan line in trun

		if (bSetX)
			for (i = iHeight - iSkipFirst; i-- > 0; pRun++)
				{
				pRun->xStart = X1;
				if ((iErrorTerm += iWidth) > 0)
					{
					X1 += iAdvanceAmt;
					iErrorTerm -= iHeight;
					}
				}
		else
			{
			X1++;
			for (i = iHeight - iSkipFirst; i-- > 0; pRun++)
				{
				pRun->xEnd = X1;
				if ((iErrorTerm += iWidth) > 0)
					{
					X1 += iAdvanceAmt;
					iErrorTerm -= iHeight;
					}
				}
			}
		}
	else
		{
		//	Edge is closer to horizontal than vertical

		int iMajorAdvanceAmt = (iWidth / iHeight) * iAdvanceAmt;
		int iErrorTermAdvance = iWidth % iHeight;
		int iErrorTerm;
		if (iDeltaX >= 0)
			iErrorTerm = 0;
		else
			iErrorTerm = -iHeight + 1;

		if (iSkipFirst)
			{
			X1 += iMajorAdvanceAmt;
			if ((iErrorTerm += iErrorTermAdvance) > 0)
				{
				X1 += iAdvanceAmt;
				iErrorTerm -= iHeight;
				}
			}

		//	Scan the edge for each scanl line in turn

		if (bSetX)
			for (i = iHeight - iSkipFirst; i-- > 0; pRun++)
				{
				pRun->xStart = X1;
				X1 += iMajorAdvanceAmt;
				if ((iErrorTerm += iErrorTermAdvance) > 0)
					{
					X1 += iAdvanceAmt;
					iErrorTerm -= iHeight;
					}
				}
		else
			{
			X1++;
			for (i = iHeight - iSkipFirst; i-- > 0; pRun++)
				{
				pRun->xEnd = X1;
				X1 += iMajorAdvanceAmt;
				if ((iErrorTerm += iErrorTermAdvance) > 0)
					{
					X1 += iAdvanceAmt;
					iErrorTerm -= iHeight;
					}
				}
			}
		}

	ASSERT(pRun >= m_pList && pRun < &m_pList[m_iCount+1]);
	*iopRun = pRun;
	}
