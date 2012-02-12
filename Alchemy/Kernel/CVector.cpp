//	CVector.cpp
//
//	Implements CVector object

#include "Kernel.h"
#include "KernelObjID.h"

#include "Euclid.h"

bool CVector::Clip (Metric rLength)

//	Clip
//
//	Clips the vector so that it is no longer than the given length.
//	Returns TRUE if we had to clip it

	{
	Metric rCurLen2 = (x * x + y * y);
	if (rCurLen2 <= (rLength * rLength))
		return false;

	//	Clip

	Metric rFactor = rLength / sqrt(rCurLen2);
	x = rFactor * x;
	y = rFactor * y;

	return true;
	}

void CVector::GenerateOrthogonals (const CVector &vNormal, Metric *retvPara, Metric *retvPerp) const

//	GenerateOrthogonals
//
//	Generates two orthogonal vectors that would generate this vector
//	where one orthogonal is parallel to the given normal.
//
//	vNormal must be a unit vector.

	{
	*retvPara = Dot(vNormal);
	*retvPerp = Dot(vNormal.Perpendicular());
	}

CVector CVector::Rotate (int iAngle) const

//	Rotate
//
//	Returns the vector rotated by the given angle

	{
	return CVector(x * g_Cosine[iAngle % 360] - y * g_Sine[iAngle % 360],
			x * g_Sine[iAngle % 360] + y * g_Cosine[iAngle % 360]);
	}

void IntPolarToVector (int iAngle, Metric rRadius, int *iox, int *ioy)

//	IntPolarToVector
//
//	PolarToVector using integers

	{
	*iox = (int)(rRadius * g_Cosine[iAngle % 360]);
	*ioy = (int)(rRadius * g_Sine[iAngle % 360]);
	}

int IntVectorToPolar (int x, int y, int *retiRadius)

//	IntVectorToPolar
//
//	Returns the angle from 0-359 and radius

	{
	int iAngle;
	int iRadius;
	int iSqrRadius = (x * x) + (y * y);

	//	If we are at the origin then the angle is undefined

	if (iSqrRadius == 0)
		{
		iAngle = 0;
		iRadius = 0;
		}
	else
		{
		iRadius = mathSqrt(iSqrRadius);
		if (x >= 0.0)
			iAngle = (((int)(180 * asin((float)y / (float)iRadius) / g_Pi)) + 360) % 360;
		else
			iAngle = 180 - ((int)(180 * asin((float)y / (float)iRadius) / g_Pi));
		}

	ASSERT(iAngle >= 0 && iAngle < 360);

	//	Done

	if (retiRadius)
		*retiRadius = iRadius;

	return iAngle;
	}

CVector PolarToVector (int iAngle, Metric rRadius)

//	PolarToVector
//
//	Creates a vector from polar coordinates. iAngle is an angle from
//	0-359 with 0 pointing to the right and going counter-clockwise.
//	rRadius is a magnitude.

	{
	return CVector(rRadius * g_Cosine[iAngle % 360], rRadius * g_Sine[iAngle % 360]);
	}

int VectorToPolar (const CVector &vP, Metric *retrRadius)

//	VectorToPolar
//
//	Converts from a vector to polar coordinates (see PolarToVector)

	{
	int iAngle;
	Metric rRadius;
	Metric rSqrRadius = vP.Dot(vP);

	//	If we are at the origin then the angle is undefined

	if (rSqrRadius == 0.0)
		{
		iAngle = 0;
		rRadius = 0.0;
		}
	else
		{
		rRadius = sqrt(rSqrRadius);
		if (vP.GetX() >= 0.0)
			iAngle = (((int)(180 * asin(vP.GetY() / rRadius) / g_Pi)) + 360) % 360;
		else
			iAngle = 180 - ((int)(180 * asin(vP.GetY() / rRadius) / g_Pi));
		}

	//	Done

	if (retrRadius)
		*retrRadius = rRadius;

	return iAngle;
	}

Metric VectorToPolarRadians (const CVector &vP, Metric *retrRadius)

//	VectorToPolarRadians
//
//	Converts from a vector to polar coordinates (see PolarToVector)

	{
	Metric rAngle;
	Metric rRadius;
	Metric rSqrRadius = vP.Dot(vP);

	//	If we are at the origin then the angle is undefined

	if (rSqrRadius == 0.0)
		{
		rAngle = 0.0;
		rRadius = 0.0;
		}
	else
		{
		rRadius = sqrt(rSqrRadius);
		if (vP.GetX() >= 0.0)
			rAngle = (vP.GetY() >= 0.0 ? asin(vP.GetY() / rRadius) : (2 * g_Pi) + asin(vP.GetY() / rRadius));
		else
			rAngle = g_Pi - asin(vP.GetY() / rRadius);
		}

	//	Done

	if (retrRadius)
		*retrRadius = rRadius;

	return rAngle;
	}

