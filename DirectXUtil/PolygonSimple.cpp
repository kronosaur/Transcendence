//	PolygonSimple.cpp
//
//	Polygon code

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

#define SIGNUM(a) ((a>0)?1:((a<0)?-1:0))

//	Advances the index by one vertex forward through the vertex list,
//	wrapping at the end of the list

#define INDEX_FORWARD(Index) \
	Index = (Index + 1) % iVertexCount;

//	Advances the index by one vertex backward through the vertex list,
//	wrapping at the start of the list

#define INDEX_BACKWARD(Index) \
	Index = (Index - 1 + iVertexCount) % iVertexCount;

//	Advances the index by one vertex either forward or backward through
//	the vertex list, wrapping at either end of the list

#define INDEX_MOVE(Index,Direction)                                  \
	if (Direction > 0)                                                \
		Index = (Index + 1) % iVertexCount;                      \
	else                                                              \
		Index = (Index - 1 + iVertexCount) % iVertexCount;

static void ScanEdge (int X1,
					  int Y1,
					  int X2,
					  int Y2,
					  bool bSetX,
					  int iSkipFirst,
					  CG16bitBinaryRegion::SRun **iopRun);

int CreateScanLinesFromSimplePolygon (int iVertexCount, SPoint *pVertexList, CG16bitBinaryRegion::SRun **retpLines)

//	CreateScanLinesFromSimplePolygon
//
//	Creates a new array of scanlines from the given polygon. Returns the number of scanlines
//	in the array. The caller is responsible for freeing the newly allocated array.
//
//	The polygon must be monotone-vertical, meaning polygons in which a scanline intersects the edges
//	exactly twice.
//
//	Michael Abrash. Graphics Programming Black Book. Chapter 41.

	{
	int i, MinIndex, MaxIndex, MinPoint_Y, MaxPoint_Y;
	int CurrentIndex, PreviousIndex;
	CG16bitBinaryRegion::SRun *EdgePointPtr;
	SPoint *VertexPtr;

	//	Point to the vertex list

	VertexPtr = pVertexList;

	//	Scan the list to find the top and bottom of the polygon

	if (iVertexCount == 0)
		return 0;		//	reject null polygons

	MaxPoint_Y = MinPoint_Y = VertexPtr[MinIndex = MaxIndex = 0].y;
	for (i = 1; i < iVertexCount; i++)
		{
		if (VertexPtr[i].y < MinPoint_Y)
			MinPoint_Y = VertexPtr[MinIndex = i].y;	//	new top
		else if (VertexPtr[i].y > MaxPoint_Y)
			MaxPoint_Y = VertexPtr[MaxIndex = i].y;	//	new bottom
		}

	//	Set the # of scan lines in the polygon, skipping the bottom edge

	int iLineCount;
	if ((iLineCount = MaxPoint_Y - MinPoint_Y) <= 0)
		return 0;		//	there’s nothing to draw, so we’re done

	int yStart = MinPoint_Y;

	//	Allocate and initialize result

	CG16bitBinaryRegion::SRun *pLine = new CG16bitBinaryRegion::SRun [iLineCount];
	for (i = 0; i < iLineCount; i++)
		pLine[i].y = yStart + i;

	//	Scan the first edge and store the boundary points in the list
	//	Initial pointer for storing scan converted first-edge coords

	EdgePointPtr = pLine;

	//	Start from the top of the first edge

	PreviousIndex = CurrentIndex = MinIndex;

	//	Scan convert each line in the first edge from top to bottom

	do
		{
		INDEX_BACKWARD(CurrentIndex);
		ScanEdge(VertexPtr[PreviousIndex].x,
			VertexPtr[PreviousIndex].y,
			VertexPtr[CurrentIndex].x,
			VertexPtr[CurrentIndex].y, true, 0, &EdgePointPtr);
		PreviousIndex = CurrentIndex;
		}
	while (CurrentIndex != MaxIndex);

	//	Scan the second edge and store the boundary points in the list

	EdgePointPtr = pLine;
	PreviousIndex = CurrentIndex = MinIndex;
	
	//	Scan convert the second edge, top to bottom

	do
		{
		INDEX_FORWARD(CurrentIndex);
		ScanEdge(VertexPtr[PreviousIndex].x,
			VertexPtr[PreviousIndex].y,
			VertexPtr[CurrentIndex].x,
			VertexPtr[CurrentIndex].y, false, 0, &EdgePointPtr);
		PreviousIndex = CurrentIndex;
		}
	while (CurrentIndex != MaxIndex);

	//	Make sure that xStart and xEnd are oriented correctly

	for (i = 0; i < iLineCount; i++)
		{
		if (pLine[i].xStart > pLine[i].xEnd)
			Swap(pLine[i].xStart, pLine[i].xEnd);
		}

	//	Done

	*retpLines = pLine;
	return iLineCount;
	}

bool IsConvexPolygon (int iVertexCount, SPoint *pVertexList)

//	IsConvexPolygon
//
//	Returns TRUE if the given polygon is convex

	{
	int i,j,k;
	int flag = 0;
	double z;

	if (iVertexCount < 3)
		return true;

	for (i=0; i<iVertexCount; i++)
		{
		j = (i + 1) % iVertexCount;
		k = (i + 2) % iVertexCount;
		z  = (pVertexList[j].x - pVertexList[i].x) * (pVertexList[k].y - pVertexList[j].y);
		z -= (pVertexList[j].y - pVertexList[i].y) * (pVertexList[k].x - pVertexList[j].x);
		if (z < 0)
			flag |= 1;
		else if (z > 0)
			flag |= 2;

		if (flag == 3)
			return false;
		}

	return true;
	}

bool IsSimplePolygon (int iVertexCount, SPoint *pVertexList)

//	IsSimplePolygon
//
//	Returns TRUE if the given polygon is monotone-vertical

	{
	int i, Length, DeltaYSign, PreviousDeltaYSign;
	int NumYReversals = 0;
	SPoint *VertexPtr = pVertexList;

	//	Three or fewer points can’t make a non-vertical-monotone polygon

	if ((Length = iVertexCount) < 4)
		return true;

	//	Scan to the first non-horizontal edge 

	PreviousDeltaYSign = SIGNUM(VertexPtr[Length-1].y - VertexPtr[0].y);
	i = 0;
	while ((PreviousDeltaYSign == 0) && (i < (Length-1)))
		{
		PreviousDeltaYSign = SIGNUM(VertexPtr[i].y - VertexPtr[i+1].y);
		i++;
		}

	if (i == (Length-1))
		return true;	//	polygon is a flat line

	//	Now count Y reversals. Might miss one reversal, at the last vertex, but 
	//	because reversal counts must be even, being off by one isn’t a problem

	do
		{
		if ((DeltaYSign = SIGNUM(VertexPtr[i].y - VertexPtr[i+1].y)) != 0)
			{
			if (DeltaYSign != PreviousDeltaYSign)
				{
				//	Switched Y direction; not vertical-monotone if
				//	reversed Y direction as many as three times */
				if (++NumYReversals > 2) return false;
				PreviousDeltaYSign = DeltaYSign;
				}
			}
		}
	while (i++ < (Length-1));

	return true;	//	it’s a vertical-monotone polygon
	}

void ScanEdge (int X1,
			   int Y1,
			   int X2,
			   int Y2,
			   bool bSetX,
			   int iSkipFirst,
			   CG16bitBinaryRegion::SRun **iopRun)

//	ScanEdge
//
//	Scan converts a polygon edge (see: CreateFromConvexPolygon)

	{
	int i;
	CG16bitBinaryRegion::SRun *pRun = *iopRun;

	//	Direction in which x moves (y2 is always > y1, so y always counts up)

	int iDeltaX;
	int iAdvanceAmt = ((iDeltaX = X2 - X1) > 0) ? 1 : -1;

	int iHeight, iWidth;
	if ((iHeight = Y2 - Y1) <= 0)
		return;

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

	*iopRun = pRun;
	}
