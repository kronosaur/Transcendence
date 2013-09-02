//	Polygon.cpp
//
//	Polygon code

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

const int INIT_ARRAY_ALLOC =						100;

struct SEdgeState
	{
    SEdgeState *pNextEdge;
    int x;
    int yStart;
    int xWholePixelMove;
    int xDirection;
    int iErrorTerm;
    int iErrorTermAdjUp;
    int iErrorTermAdjDown;
    int iCount;
	};

static void AdvanceAET (void);
static void BuildGlobalEdgeTable (int iVertexCount, SPoint *pVertexList, SEdgeState *NextFreeEdgeStruc);
static void MoveXSortedToAET (int YToMove);
static void ScanOutAET (int YToScan);
static void XSortAET (void);

static SEdgeState *g_pGET;
static SEdgeState *g_pAET;
static int g_iRunCount;
static int g_iRunAlloc;
static CG16bitBinaryRegion::SRun *g_pRuns;

int CreateScanLinesFromPolygon (int iVertexCount, SPoint *pVertexList, CG16bitBinaryRegion::SRun **retpLines)

//	CreateScanLinesFromPolygon
//
//	Creates a new array of scanlines from the given polygon. Returns the number of scanlines
//	in the array. The caller is responsible for freeing the newly allocated array.
//
//	NOTE: This function is NOT thread-safe--it must never be called from more than one
//	thread simultaneously.
//
//	Michael Abrash. Graphics Programming Black Book. Chapter 40.

	{
	//	Reject polycons that have less then 3 vertices

	if (iVertexCount < 3)
		return 0;

	//	Initialize the output

	g_iRunAlloc = INIT_ARRAY_ALLOC;
	g_pRuns = new CG16bitBinaryRegion::SRun [g_iRunAlloc];
	g_iRunCount = 0;

	//	Build the global edge table. This initializes g_pGET.

	SEdgeState *EdgeTableBuffer = new SEdgeState [iVertexCount];
	BuildGlobalEdgeTable(iVertexCount, pVertexList, EdgeTableBuffer);
	if (g_pGET == NULL)
		{
		delete [] EdgeTableBuffer;
		delete [] g_pRuns;
		g_pRuns = NULL;
		return 0;
		}

	//	Active Edge Table is empty

	g_pAET = NULL;

	//	Start at the top polygon vertex

	int yCurrent = g_pGET->yStart;

	//	Scan down through the polygon edges, one scan line at a time,
	//	so long as at least one edge remains in either the GET or AET

	while ((g_pGET != NULL) || (g_pAET != NULL))
		{
		//	Update AET for this scan line
		MoveXSortedToAET(yCurrent);

		//	Add to lines
		ScanOutAET(yCurrent);

		//	Advance AET edges 1 scan line
		AdvanceAET();

		//	Resort on X
		XSortAET();

		//	Next
		yCurrent++;
		}

	//	Done

	if (g_iRunCount > 0)
		*retpLines = g_pRuns;
	else
		delete [] g_pRuns;

	delete [] EdgeTableBuffer;

	return g_iRunCount;
	}

void AdvanceAET (void)

//	AdvanceAET
//
//	Advances each edge in the AET by one scan line.
//	Removes edges that have been fully scanned.
	
	{
	SEdgeState *CurrentEdge, **CurrentEdgePtr;

	//	Count down and remove or advance each edge in the AET

	CurrentEdgePtr = &g_pAET;
	while ((CurrentEdge = *CurrentEdgePtr) != NULL)
		{
		//	Count off one scan line for this edge

		if ((--(CurrentEdge->iCount)) == 0)
			{
			//	This edge is finished, so remove it from the AET

			*CurrentEdgePtr = CurrentEdge->pNextEdge;
			}
		else 
			{
			//	Advance the edge’s X coordinate by minimum move

			CurrentEdge->x += CurrentEdge->xWholePixelMove;

			//	Determine whether it’s time for X to advance one extra
			
			if ((CurrentEdge->iErrorTerm +=
				CurrentEdge->iErrorTermAdjUp) > 0)
				{
				CurrentEdge->x += CurrentEdge->xDirection;
				CurrentEdge->iErrorTerm -= CurrentEdge->iErrorTermAdjDown;
				}

			CurrentEdgePtr = &CurrentEdge->pNextEdge;
			}
		}
	}

void BuildGlobalEdgeTable (int iVertexCount, SPoint *pVertexList, SEdgeState *NextFreeEdgeStruc)

//	BuildGlobalEdgeTable
//
//	Creates a GET in the buffer pointed to by NextFreeEdgeStruc from
//	the vertex list. Edge endpoints are flipped, if necessary, to
//	guarantee all edges go top to bottom. The GET is sorted primarily
//	by ascending Y start coordinate, and secondarily by ascending X
//	start coordinate within edges with common Y coordinates. */

	{
    int i, StartX, StartY, EndX, EndY, DeltaY, DeltaX, Width;
    SEdgeState *NewEdgePtr;
    SEdgeState *FollowingEdge, **FollowingEdgeLink;
    SPoint *VertexPtr;

	//	Initialize the global edge table to empty

    g_pGET = NULL;

	//	Scan through the vertex list and put all non-0-height edges into
	//	the GET, sorted by increasing Y start coordinate

	VertexPtr = pVertexList;
    for (i = 0; i < iVertexCount; i++)
		{
		//	Calculate the edge height and width

		StartX = VertexPtr[i].x;
		StartY = VertexPtr[i].y;

		//	The edge runs from the current point to the previous one

		if (i == 0)
			{
			//	Wrap back around to the end of the list
			EndX = VertexPtr[iVertexCount-1].x;
			EndY = VertexPtr[iVertexCount-1].y;
			}
		else
			{
			EndX = VertexPtr[i-1].x;
			EndY = VertexPtr[i-1].y;
			}

		//	Make sure the edge runs top to bottom

		if (StartY > EndY)
			{
			Swap(StartX, EndX);
			Swap(StartY, EndY);
			}

		//	Skip if this can’t ever be an active edge (has 0 height)

		if ((DeltaY = EndY - StartY) != 0)
			{
			//	Allocate space for this edge’s info, and fill in the structure

			NewEdgePtr = NextFreeEdgeStruc++;
			NewEdgePtr->xDirection =   //	direction in which X moves
				((DeltaX = EndX - StartX) > 0) ? 1 : -1;
			Width = Absolute(DeltaX);
			NewEdgePtr->x = StartX;
			NewEdgePtr->yStart = StartY;
			NewEdgePtr->iCount = DeltaY;
			NewEdgePtr->iErrorTermAdjDown = DeltaY;
			if (DeltaX >= 0)  //	initial error term going L->R
				NewEdgePtr->iErrorTerm = 0;
			else              //	initial error term going R->L
				NewEdgePtr->iErrorTerm = -DeltaY + 1;
			if (DeltaY >= Width)
				{
				//	Y-major edge
				NewEdgePtr->xWholePixelMove = 0;
				NewEdgePtr->iErrorTermAdjUp = Width;
				}
			else
				{
				//	X-major edge
				NewEdgePtr->xWholePixelMove = (Width / DeltaY) * NewEdgePtr->xDirection;
				NewEdgePtr->iErrorTermAdjUp = Width % DeltaY;
				}

			//	Link the new edge into the GET so that the edge list is
			//	still sorted by Y coordinate, and by X coordinate for all
			//	edges with the same Y coordinate

			FollowingEdgeLink = &g_pGET;
			for (;;)
				{
				FollowingEdge = *FollowingEdgeLink;
				if ((FollowingEdge == NULL) ||
						(FollowingEdge->yStart > StartY) ||
						((FollowingEdge->yStart == StartY) &&
						(FollowingEdge->x >= StartX)))
					{
					NewEdgePtr->pNextEdge = FollowingEdge;
					*FollowingEdgeLink = NewEdgePtr;
					break;
					}
			
				FollowingEdgeLink = &FollowingEdge->pNextEdge;
				}
			}
		}
	}

void MoveXSortedToAET (int YToMove)

//	MoveXSortedToAET
//
//	Moves all edges that start at the specified Y coordinate from the
//	GET to the AET, maintaining the X sorting of the AET.

	{
	SEdgeState *AETEdge, **AETEdgePtr, *TempEdge;
	int CurrentX;

	//	The GET is Y sorted. Any edges that start at the desired Y
	//	coordinate will be first in the GET, so we’ll move edges from
	//	the GET to AET until the first edge left in the GET is no longer
	//	at the desired Y coordinate. Also, the GET is X sorted within
	//	each Y coordinate, so each successive edge we add to the AET is
	//	guaranteed to belong later in the AET than the one just added.

	AETEdgePtr = &g_pAET;
	while ((g_pGET != NULL) && (g_pGET->yStart == YToMove))
		{
		CurrentX = g_pGET->x;

		//	Link the new edge into the AET so that the AET is still
		//	sorted by X coordinate

		for (;;)
			{
			AETEdge = *AETEdgePtr;
			if ((AETEdge == NULL) || (AETEdge->x >= CurrentX))
				{
				TempEdge = g_pGET->pNextEdge;
				*AETEdgePtr = g_pGET;	//	link the edge into the AET
				g_pGET->pNextEdge = AETEdge;
				AETEdgePtr = &g_pGET->pNextEdge;
				g_pGET = TempEdge;		//	unlink the edge from the GET
				break;
				}
			else
				{
				AETEdgePtr = &AETEdge->pNextEdge;
				}
			}
		}
	}

void ScanOutAET (int YToScan)

//	ScanOutAET
//
//	Adds new runs to the scanline structure
	{
	int LeftX;
	SEdgeState *CurrentEdge;

	//	Scan through the AET, drawing line segments as each pair of edge
	//	crossings is encountered. The nearest pixel on or to the right
	//	of left edges is drawn, and the nearest pixel to the left of but
	//	not on right edges is drawn

	CurrentEdge = g_pAET;
	while (CurrentEdge != NULL)
		{
		LeftX = CurrentEdge->x;
		CurrentEdge = CurrentEdge->pNextEdge;

		//	If we don't have room to add a new run, increase allocation

		if (g_iRunCount == g_iRunAlloc)
			{
			if (g_iRunAlloc < 1000)
				g_iRunAlloc = g_iRunAlloc * 2;
			else
				g_iRunAlloc += 1000;

			CG16bitBinaryRegion::SRun *pNewAlloc = new CG16bitBinaryRegion::SRun [g_iRunAlloc];
			utlMemCopy((char *)g_pRuns, (char *)pNewAlloc, g_iRunCount * sizeof(CG16bitBinaryRegion::SRun));
			delete [] g_pRuns;
			g_pRuns = pNewAlloc;
			}

		//	Add new run

		g_pRuns[g_iRunCount].y = YToScan;
		g_pRuns[g_iRunCount].xStart = LeftX;
		g_pRuns[g_iRunCount].xEnd = CurrentEdge->x - 1;
		g_iRunCount++;

		//	Next

		CurrentEdge = CurrentEdge->pNextEdge;
		}
	}

void XSortAET (void)

//	XSortAET
//
//	Sorts all edges currently in the active edge table into ascending
//	order of current X coordinates

	{
    SEdgeState *CurrentEdge, **CurrentEdgePtr, *TempEdge;
    int SwapOccurred;

	//	Scan through the AET and swap any adjacent edges for which the
	//	second edge is at a lower current X coord than the first edge.
	//	Repeat until no further swapping is needed

    if (g_pAET != NULL)
		{
		do
			{
			SwapOccurred = 0;
			CurrentEdgePtr = &g_pAET;
			while ((CurrentEdge = *CurrentEdgePtr)->pNextEdge != NULL)
				{
				if (CurrentEdge->x > CurrentEdge->pNextEdge->x)
					{
					//	The second edge has a lower X than the first;
					//	swap them in the AET

					TempEdge = CurrentEdge->pNextEdge->pNextEdge;
					*CurrentEdgePtr = CurrentEdge->pNextEdge;
					CurrentEdge->pNextEdge->pNextEdge = CurrentEdge;
					CurrentEdge->pNextEdge = TempEdge;
					SwapOccurred = 1;
					}

				CurrentEdgePtr = &(*CurrentEdgePtr)->pNextEdge;
				}
			}
		while (SwapOccurred != 0);
		}
	}
