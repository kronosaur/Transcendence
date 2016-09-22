//	CArtifactAICorePainter.cpp
//
//	CArtifactAICorePainter class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

const int MAX_RADIUS =						80;

CArtifactAICorePainter::CArtifactAICorePainter (void) :
		m_Camera(0.25 * PI)

//	CArtifactAICorePainter constructor

	{
	m_vRotationRate = CVector3D(0.01 * PI, 0.003 * PI, 0.0);

	//	Generate a random sphere mesh.

	CreateSphereRandom(500, 1.0, m_SpherePoints);
	CGeometry3D::CalcConvexHull(m_SpherePoints, m_SphereEdges);
	}

void CArtifactAICorePainter::CreateSphere (Metric rRadius, TArray<CVector3D> &Result)

//	CreateSphere
//
//	Creates a sphere with a series of points.

	{
	int i, j;

	int iLongitude = 24;
	int iLatitude = 18;

	Result.DeleteAll();
	Result.GrowToFit(iLongitude * iLatitude);

	Metric rLongInc = TAU / (Metric)iLongitude;
	Metric rLatInc = PI / (Metric)iLatitude;

	Metric rTheta = rLatInc;
	for (i = 0; i < iLatitude - 2; i++)
		{
		Metric rPhi = 0.0;
		for (j = 0; j < iLongitude; j++)
			{
			Result.Insert(CSphericalCoordinates::ToCartessian(CVector3D(rRadius, rTheta, rPhi)));
			rPhi += rLongInc;
			}

		rTheta += rLatInc;
		}
	}

void CArtifactAICorePainter::CreateSphereRandom (int iPoints, Metric rRadius, TArray<CVector3D> &Result)

//	CreateSphereRandom
//
//	Creates a sphere with a series of randomly distributed points

	{
	int i;

	Result.DeleteAll();
	Result.GrowToFit(iPoints);

	for (i = 0; i < iPoints; i++)
		{
		while (true)
			{
			CVector3D vTest(mathRandomMinusOneToOne(), mathRandomMinusOneToOne(), mathRandomMinusOneToOne());
			Metric rLength = vTest.Length();
			if (rLength > 1.0)
				continue;

			Result.Insert((rRadius / rLength) * vTest);
			break;
			}
		}
	}

void CArtifactAICorePainter::Paint (CG32bitImage &Dest, int x, int y)

//	Paint
//
//	Paints a frame

	{
	int i;

	//CGDraw::Circle(Dest, x, y, MAX_RADIUS, STYLECOLOR(colorAICoreBack));

	CG32bitPixel rgbCore = AA_STYLECOLOR(colorAICoreFore);

	//	Transform all sphere points in the projection

	TArray<CVector> Points = m_Camera.Transform(m_Xform, m_SpherePoints);

	//	Paint all edges

	for (i = 0; i < m_SphereEdges.GetCount(); i++)
		{
		const CVector &vFrom = Points[m_SphereEdges[i].iStart];
		const CVector &vTo = Points[m_SphereEdges[i].iEnd];
		Dest.DrawLine(x + (int)vFrom.GetX(), y - (int)vFrom.GetY(), x + (int)vTo.GetX(), y - (int)vTo.GetY(), 1, rgbCore);
		}

	//	Paint all points

	for (i = 0; i < Points.GetCount(); i++)
		{
		Dest.DrawDot(x + (int)Points[i].GetX(), y - (int)Points[i].GetY(), rgbCore, markerSmallRound);
		}
	}

void CArtifactAICorePainter::Update (void)

//	Update
//
//	Update animation

	{
	CVector3D vPos(0.0, 0.0, 2.0);
	CVector3D vScale(70.0, 70.0, 70.0);

	m_vRotation = m_vRotation + m_vRotationRate;

	CXForm3D ToGlobal(xformRotate, m_vRotation);
	ToGlobal = ToGlobal * CXForm3D(xformTranslate, vPos);

	m_Xform = ToGlobal * m_Camera;
	m_Xform = m_Xform * CXForm3D(xformScale, vScale);
	}
