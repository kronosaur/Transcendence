//	CAniPolygon.cpp
//
//	CAniPolygon class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

#include "Reanimator.h"

const int INDEX_VISIBLE =					0;
#define PROP_VISIBLE						CONSTLIT("visible")

const int INDEX_POSITION =					1;
#define PROP_POSITION						CONSTLIT("position")

const int INDEX_SCALE =						2;
#define PROP_SCALE							CONSTLIT("scale")

const int INDEX_ROTATION =					3;
#define PROP_ROTATION						CONSTLIT("rotation")

#define PROP_COLOR							CONSTLIT("color")
#define PROP_OPACITY						CONSTLIT("opacity")

CAniPolygon::CAniPolygon (TArray<CVector> &Points)

//	CAniPolygon constructor

	{
	m_Points.TakeHandoff(Points);

	m_Properties.SetInteger(PROP_VISIBLE, 1);
	m_Properties.SetVector(PROP_POSITION, CVector());
	m_Properties.SetVector(PROP_SCALE, CVector(1.0, 1.0));
	m_Properties.SetInteger(PROP_ROTATION, 0);
	}

void CAniPolygon::GetContentRect (RECT *retrcRect)

//	GetContentRect
//
//	Returns a RECT of the content area (relative to the rect itself)

	{
	GetSpacingRect(retrcRect);
	}

void CAniPolygon::GetSpacingRect (RECT *retrcRect)

//	GetSpacingRect
//
//	Returns the size

	{
	//	Rasterize

	Rasterize(m_Properties[INDEX_SCALE].GetVector(), m_Properties[INDEX_ROTATION].GetInteger());

	//	Get the size

	m_Region.GetBounds(retrcRect);
	}

void CAniPolygon::Paint (SAniPaintCtx &Ctx)

//	Paint
//
//	Paints the element

	{
	//	Position and size

	CVector vPos = Ctx.ToDest.Transform(m_Properties[INDEX_POSITION].GetVector());
	CVector vPos2 = Ctx.ToDest.Transform(m_Properties[INDEX_POSITION].GetVector() + m_Properties[INDEX_SCALE].GetVector());
	CVector vSize = vPos2 - vPos;

	//	Rotation

	int iRotation = m_Properties[INDEX_ROTATION].GetInteger();

	//	Rasterize the polygon (if not already done)

	Rasterize(vSize, iRotation);

	//	Position

	int x = (int)vPos.GetX();
	int y = (int)vPos.GetY();

	//	Fill method

	IAniFillMethod *pFill = GetFillMethod();
	if (pFill)
		{
		//	Init

		pFill->InitPaint(Ctx, x, y, m_Properties);

		//	Paint

		pFill->Fill(Ctx, x, y, m_Region);
		}
	}

void CAniPolygon::Rasterize (const CVector &vScale, int iRotation)

//	Rasterize
//
//	Rasterize the polygon, if necessary

	{
	int i;

	if (vScale == m_vScale && iRotation == m_iRotation)
		return;

	//	If necessary we need to transform the polygon points

	int iCount = m_Points.GetCount();
	if (iCount == 0)
		return;

	SPoint *pPoints = new SPoint [iCount];

	if (vScale.GetX() != 1.0 || vScale.GetY() != 1.0 || iRotation != 0)
		{
		CXForm Xform = CXForm(xformRotate, iRotation) * CXForm(xformScale, vScale);

		for (i = 0; i < iCount; i++)
			{
			CVector vTrans = Xform.Transform(m_Points[i]);

			pPoints[i].x = (int)vTrans.GetX();
			pPoints[i].y = (int)vTrans.GetY();
			}
		}
	else
		{
		for (i = 0; i < iCount; i++)
			{
			pPoints[i].x = (int)m_Points[i].GetX();
			pPoints[i].y = (int)m_Points[i].GetY();
			}
		}

	//	Create a region

	m_Region.CreateFromPolygon(iCount, pPoints);

	//	Done with points

	delete [] pPoints;

	//	Remember the parameters used to construct the region

	m_vScale = vScale;
	m_iRotation = iRotation;
	}
