//	Euclid.cpp
//
//	Basic 2D Geometry

#include "Kernel.h"
#include "KernelObjID.h"

#include "Euclid.h"
#include "math.h"

const CVector NullVector = CVector();

Metric g_Cosine[360];
Metric g_Sine[360];
static bool g_bTrigInit = false;

bool AngleInArc (int iAngle, int iMinAngle, int iMaxAngle)

//	AngleInArc
//
//	Returns TRUE if iAngle is inside the given arc

	{
	if (iMinAngle <= iMaxAngle)
		return (iAngle >= iMinAngle && iAngle <= iMaxAngle);
	else
		return (iAngle >= iMinAngle || iAngle <= iMaxAngle);
	}

void EuclidInit (void)

//	EuclidInit
//
//	Initializes sine and cosine tables

	{
	if (!g_bTrigInit)
		{
		for (int i = 0; i < 360; i++)
			{
			Metric rRadian = 2 * g_Pi * i / 360;

			g_Sine[i] = sin(rRadian);
			g_Cosine[i] = cos(rRadian);
			}

		g_bTrigInit = true;
		}
	}

inline bool IntCCW (int Ax, int Ay, int Bx, int By, int Cx, int Cy)

//	IntCCW
//
//	Returns TRUE if the three points are in counter-clockwise order

	{
	return (Cy - Ay) * (Bx - Ax) > (By - Ay) * (Cx - Ax);
	}

bool IntSegmentsIntersect (int A1x, int A1y, int A2x, int A2y, int B1x, int B1y, int B2x, int B2y)

//	IntSegmentsIntersect
//
//	Returns TRUE if the two line segments intersect
//
//	See: http://compgeom.cs.uiuc.edu/~jeffe/teaching/373/notes/x06-sweepline.pdf

	{
	return (IntCCW(A1x, A1y, B1x, B1y, B2x, B2y) != IntCCW(A2x, A2y, B1x, B1y, B2x, B2y))
			&& (IntCCW(A1x, A1y, A2x, A2y, B1x, B1y) != IntCCW(A1x, A1y, A2x, A2y, B2x, B2y));
	}
