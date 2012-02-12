//	Shapes.cpp
//
//	Simple shapes
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "Kernel.h"
#include "KernelObjID.h"

#include "Euclid.h"
#include "math.h"

const int DEGREES_PER_SEGMENT =			10;

void CreateArcPolygon (Metric rInnerRadius, Metric rOuterRadius, int iArc, TArray<CVector> *retPoints)

//	CreateArcPolygon
//
//	Creates a polygon arc

	{
	int i;

	//	Initialize

	retPoints->DeleteAll();

	//	Handle some edge cases

	if (rInnerRadius >= rOuterRadius || rInnerRadius <= 0.0 || iArc <= 0 || iArc >= 360)
		return;

	//	Compute the number of segments

	int iSegCount = (AlignUp(iArc, DEGREES_PER_SEGMENT) / DEGREES_PER_SEGMENT) + 1;
	Metric rAnglePerSegment = (Metric)iArc / (iSegCount - 1);

	//	We have twice the number of points

	retPoints->InsertEmpty(iSegCount * 2);

	//	Add the outer points (from low angle to high)

	for (i = 0; i < iSegCount; i++)
		{
		CVector &Point = retPoints->GetAt(i);

		Metric rAngle = g_Pi * (rAnglePerSegment * i) / 180.0;
		Point.SetX(rOuterRadius * cos(rAngle));
		Point.SetY(rOuterRadius * sin(rAngle));
		}

	//	Add the inner points (from high angle to low)

	for (i = 0; i < iSegCount; i++)
		{
		CVector &Point = retPoints->GetAt(iSegCount + i);

		Metric rAngle = g_Pi * (rAnglePerSegment * (iSegCount - i - 1)) / 180.0;
		Point.SetX(rInnerRadius * cos(rAngle));
		Point.SetY(rInnerRadius * sin(rAngle));
		}
	}
