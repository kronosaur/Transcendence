//	Geometry.cpp
//
//	Classes for vectors and other geometric concepts
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"
#include "Geometry.h"

#include "math.h"

const Dimension CVector::Dot (const CVector &op2) const
	{
	return (m_x * op2.m_x + m_y * op2.m_y + m_z * op2.m_z);
	}

const CVector CVector::Cross (const CVector &op2) const
	{
	return CVector(m_y * op2.m_z - op2.m_y * m_z,
			op2.m_x * m_z - m_x * op2.m_z,
			m_x * op2.m_y - op2.m_x * m_y);
	}

const Dimension CVector::Length (void) const
	{
	return sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
	}

const CVector CVector::Normal (void) const
	{
	return (*this) / Length();
	}

const C2Vector OrthoProject (const CVector &vVec, int iAxis)
	{
	switch (iAxis)
		{
		case axisX:
			return C2Vector(vVec.Y(), vVec.Z());

		case axisY:
			return C2Vector(vVec.X(), vVec.Z());

		case axisZ:
			return C2Vector(vVec.X(), vVec.Y());

		default:
			ASSERT(false);
			return C2Vector();
		}
	}

void PlaneEq (const SRay &Normal, 
			  Dimension *retA,
			  Dimension *retB,
			  Dimension *retC,
			  Dimension *retD)

//	PlaneEq
//
//	Returns the coefficients for the equation of a plane that contains
//	the origin of the normal and is perpendicular to the normal direction.
//
//	Source: Thomas/Finney, p.734

	{
	*retA = Normal.vDir.X();
	*retB = Normal.vDir.Y();
	*retC = Normal.vDir.Z();
	*retD = -((*retA * Normal.vOrigin.X())
			+ (*retB * Normal.vOrigin.Y())
			+ (*retC * Normal.vOrigin.Z()));
	}

int PlaneOrtho (const SRay &Normal)
	{
	int iBestAxis = axisX;
	Dimension rBestAxis = fabs(Normal.vDir.X());
	Dimension rY = fabs(Normal.vDir.Y());
	Dimension rZ = fabs(Normal.vDir.Z());

	if (rY > rBestAxis)
		{
		iBestAxis = axisY;
		rBestAxis = rY;
		}

	if (rZ > rBestAxis)
		{
		iBestAxis = axisZ;
		rBestAxis = rZ;
		}

	return iBestAxis;
	}

bool PointInPolygon (const C2Vector &vVec, const TArray<C2Vector> &Polygon)

//	PointInPolygon
//
//	Returns TRUE if the given point in inside the polygon

	{
	int i;
	int iCount = Polygon.GetCount();
	int iIntersections = 0;

	//	Loop over all edges to see if a line from the point out to
	//	infinite-Y intersects the edge

	for (i = 0; i < iCount; i++)
		{
		//	Get the two points of the edge. Make sure that p1
		//	if always to the left (smaller than) p2.

		C2Vector p1 = Polygon[i];
		C2Vector p2 = Polygon[(i + 1) % iCount];
		if (p1.X() > p2.X())
			{
			C2Vector p3(p2);
			p2 = p1;
			p1 = p3;
			}

		//	We never intersect with vertical lines

		if (EqZero(p2.X() - p1.X()))
			continue;

		//	If the test point is not bounded by the x-coordinates
		//	of the two points then there is no way it can intersect

		if (p1.X() > (vVec.X() + EPSILON) || p2.X() < (vVec.X() + EPSILON))
			continue;

		//	If the test point is above the y-coordinates of both
		//	points, then there is no way it can intersect

		if (p1.Y() < (vVec.Y() - EPSILON) && p2.Y() < (vVec.Y() - EPSILON))
			continue;

		//	If both points are above the test point then we
		//	clearly intersect

		if (p1.Y() > (vVec.Y() - EPSILON) && p2.Y() > (vVec.Y() - EPSILON))
			{
			iIntersections++;
			continue;
			}

		//	If we get this far then one point is above and the other
		//	is below. Compute the line equation

		Dimension dX = p2.X() - p1.X();
		Dimension m = (p2.Y() - p1.Y()) / dX;
		Dimension Yi = m * (vVec.X() - p1.X()) + p1.Y();
		if (Yi > (vVec.Y() - EPSILON))
			iIntersections++;
		}

	//	If we intersect an even number of lines then we are outside
	//	the polygon.

	return ((iIntersections % 2) != 0);
	}

Dimension PointToLineDistance (const CVector &vPoint, const SRay &Line)

//	PointToLineDistance
//
//	Returns the distance from the point to the nearest point on the
//	given ray.

	{
	//	Intersect the line with the plane passing through vPoint and
	//	whose normal is Line.vDir.

	SRay PlaneNormal;
	PlaneNormal.vOrigin = vPoint;
	PlaneNormal.vDir = Line.vDir;
	Dimension rDist = RayIntersectionWithPlane(Line, PlaneNormal);

	//	The resulting distance is the distance from Line.vOrigin to
	//	the point on the line nearest vPoint.

	CVector vNearestPointOnLine = Line.vOrigin + Line.vDir * rDist;

	//	Done

	return (vPoint - vNearestPointOnLine).Length();
	}

Dimension RayIntersectionWithBox (const SRay &Ray, const CVector &vNearCorner, const CVector &vFarCorner)

//	RayIntersectionWithBox
//
//	Returns the intersection distance of a ray with the box
//
//	Source: Alan & Mark Watt, p.226

	{
	Dimension Near = (vNearCorner.X() - Ray.vOrigin.X()) / Ray.vDir.X();
	Dimension Far = (vFarCorner.X() - Ray.vOrigin.X()) / Ray.vDir.X();
	if (Near > Far)
		{
		Dimension Swap = Near;
		Near = Far;
		Far = Swap;
		}

	Dimension NearY = (vNearCorner.Y() - Ray.vOrigin.Y()) / Ray.vDir.Y();
	Dimension FarY = (vFarCorner.Y() - Ray.vOrigin.Y()) / Ray.vDir.Y();
	if (NearY > FarY)
		{
		Dimension Swap = NearY;
		NearY = FarY;
		FarY = Swap;
		}

	if (NearY > Near)
		Near = NearY;

	if (FarY < Far)
		Far = FarY;

	Dimension NearZ = (vNearCorner.Z() - Ray.vOrigin.Z()) / Ray.vDir.Z();
	Dimension FarZ = (vFarCorner.Z() - Ray.vOrigin.Z()) / Ray.vDir.Z();
	if (NearZ > FarZ)
		{
		Dimension Swap = NearZ;
		NearZ = FarZ;
		FarZ = Swap;
		}

	if (NearZ > Near)
		Near = NearZ;

	if (FarZ < Far)
		Far = FarZ;

	if (Near < Far)
		return Near;
	else
		return INFINITY;
	}

Dimension RayIntersectionWithPlane (const SRay &Ray, const SRay &PlaneNormal)

//	RayIntersectionWithPlane
//
//	Returns the distance along the ray at which it intersects the given plane
//	If the two do not intersect, we return INFINITY
//
//	Source: Foley/vanDam, p.703

	{
	//	Get the coefficients for the plane equation

	Dimension A, B, C, D;
	PlaneEq(PlaneNormal, &A, &B, &C, &D);

	//	Compute ray deltas

	Dimension dX = Ray.vDir.X();
	Dimension dY = Ray.vDir.Y();
	Dimension dZ = Ray.vDir.Z();

	//	Compute intersection

	Dimension rDenominator = A * dX + B * dY + C * dZ;
	if (EqZero(rDenominator))
		return INFINITY;

	//	Done

	return -(A * Ray.vOrigin.X() + B * Ray.vOrigin.Y() + C * Ray.vOrigin.Z() + D) / rDenominator;
	}

Dimension RayNearestPointToPoint (const SRay &Ray, const CVector &vPoint)

//	RayNearestPointToPoint
//
//	Returns the distance along the ray to the point that is nearest
//	to the given point.

	{
	//	Intersect the line with the plane passing through vPoint and
	//	whose normal is Line.vDir.

	SRay PlaneNormal;
	PlaneNormal.vOrigin = vPoint;
	PlaneNormal.vDir = Ray.vDir;
	Dimension rDist = RayIntersectionWithPlane(Ray, PlaneNormal);

	return rDist;
	}

