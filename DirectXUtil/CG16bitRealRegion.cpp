//	CG16bitRealRegion.cpp
//
//	Implementation of raw 16-bit image region

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

int g_iCG16bitRealRegionDummy = 0;

#ifdef LATER
void CG16bitRealRegion::CreateRect (const CVector &vFrom, const CVector &vTo, Metric rHeight)

//	CreateRect
//
//	Creates a rectangular region formed by drawing a line from vFrom to vTo with
//	rHeight thickness.

	{
	//	Initialize

	DeleteAll();

	//	Compute the four corners of the rectangle

	CVector vWidth = vTo - vFrom;
	CVector vHalfHeight = (rHeight / 2.0) * vWidth.Perpendicular().Normal();

	CVector vA = vFrom - vHalfHeight;
	CVector vB = vTo - vHalfHeight
	CVector vC = vFrom + vHalfHeight;
	CVector vD = vTo + vHalfHeight;

	//	Compute the bounds of the rectangle

	const CVector &vABTop = (vA.GetY() < vB.GetY() ? vA : vB);
	const CVector &vCDTop = (vC.GetY() < vD.GetY() ? vC : vD);
	const CVector &vTop = (vABTop.GetY() < vCDTop.GetY() ? vABTop : vCDTop);

	const CVector &vABBottom = (vA.GetY() > vB.GetY() ? vA : vB);
	const CVector &vCDBottom = (vC.GetY() > vD.GetY() ? vC : vD);
	const CVector &vBottom = (vABBottom.GetY() > vCDBottom.GetY() ? vABBottom : vCDBottom);

	const CVector &vABLeft = (vA.GetX() < vB.GetX() ? vA : vB);
	const CVector &vCDLeft = (vC.GetX() < vD.GetX() ? vC : vD);
	const CVector &vLeft = (vABLeft.GetX() < vCDLeft.GetX() ? vABLeft : vCDLeft);

	const CVector &vABRight = (vA.GetX() > vB.GetX() ? vA : vB);
	const CVector &vCDRight = (vC.GetX() > vD.GetX() ? vC : vD);
	const CVector &vRight = (vABRight.GetX() > vCDRight.GetX() ? vABRight : vCDRight);

	}
#endif