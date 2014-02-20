//	CMapViewportCtx.cpp
//
//	CMapViewportCtx Class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "math.h"

#define USE_3D

const int VIEWPORT_EXTRA =								256;
const Metric MAP_VERTICAL_ADJUST =						1.4;
const WORD RGB_GRID_LINE =								CG16bitImage::RGBValue(43, 45, 51);

CMapViewportCtx::CMapViewportCtx (void)

//	CMapViewportCtx constructor

	{
	m_rcView.left = 0;
	m_rcView.top = 0;
	m_rcView.right = 0;
	m_rcView.bottom = 0;

	m_rMapScale = g_KlicksPerPixel;

	m_xCenter = 0;
	m_yCenter = 0;
	}

CMapViewportCtx::CMapViewportCtx (const CVector &vCenter, const RECT &rcView, Metric rMapScale) :
		m_rcView(rcView),
		m_rMapScale(rMapScale)

//	CMapViewportCtx constructor

	{
	//	Round the center on a pixel boundary

	m_vCenter = CVector(floor(vCenter.GetX() / rMapScale) * rMapScale, floor(vCenter.GetY() / rMapScale) * rMapScale);

	//	Figure out the boundary of the viewport in system coordinates

	m_xCenter = m_rcView.left + RectWidth(m_rcView) / 2;
	m_yCenter = m_rcView.top + RectHeight(m_rcView) / 2;

	CVector vDiagonal(m_rMapScale * (Metric)(RectWidth(rcView) + VIEWPORT_EXTRA) / 2,
				m_rMapScale * MAP_VERTICAL_ADJUST * (Metric)(RectHeight(rcView) + VIEWPORT_EXTRA) / 2);
	m_vUR = m_vCenter + vDiagonal;
	m_vLL = m_vCenter - vDiagonal;

	//	Compute the transformation to map world coordinates to the viewport

	m_Trans = ViewportTransform(m_vCenter, 
			m_rMapScale, 
			m_rMapScale * MAP_VERTICAL_ADJUST,
			m_xCenter,
			m_yCenter);
	}

bool CMapViewportCtx::IsInViewport (CSpaceObject *pObj) const 

//	IsInViewport
//
//	Returns TRUE if the given object is in the viewport
	
	{
	return pObj->InBox(m_vUR, m_vLL); 
	}

void CMapViewportCtx::PaintGrid (CG16bitImage &Dest, const CVector &vPos, Metric rWidth, Metric rInterval) const

//	PaintGrid
//
//	Paints a grid around the system center

	{
	Metric rHalfWidth = rWidth;
	CVector vHalfHorz(rHalfWidth, 0.0);
	CVector vHalfVert(0.0, rHalfWidth);

	//	Paint the center line (horz)

	int xFrom, yFrom;
	CVector vLeft = vPos - vHalfHorz;
	Transform(vLeft, &xFrom, &yFrom);

	int xTo, yTo;
	CVector vRight = vPos + vHalfHorz;
	Transform(vRight, &xTo, &yTo);

	Dest.DrawLine(xFrom, yFrom, xTo, yTo, 1, RGB_GRID_LINE);

	//	Paint the center line (vert)

	vLeft = vPos - vHalfVert;
	Transform(vLeft, &xFrom, &yFrom);

	vRight = vPos + vHalfVert;
	Transform(vRight, &xTo, &yTo);

	Dest.DrawLine(xFrom, yFrom, xTo, yTo, 1, RGB_GRID_LINE);

	//	Paint the horizontal lines

	Metric rLine = rInterval;
	Metric rEnd = rHalfWidth;
	while (rLine <= rEnd)
		{
		//	Paint line above

		vLeft = CVector(vPos.GetX() - rHalfWidth, rLine);
		Transform(vLeft, &xFrom, &yFrom);

		vRight = CVector(vPos.GetX() + rHalfWidth, rLine);
		Transform(vRight, &xTo, &yTo);

		Dest.DrawLine(xFrom, yFrom, xTo, yTo, 1, RGB_GRID_LINE);

		//	Paint line below

		vLeft = CVector(vPos.GetX() - rHalfWidth, -rLine);
		Transform(vLeft, &xFrom, &yFrom);

		vRight = CVector(vPos.GetX() + rHalfWidth, -rLine);
		Transform(vRight, &xTo, &yTo);

		Dest.DrawLine(xFrom, yFrom, xTo, yTo, 1, RGB_GRID_LINE);

		//	Paint to the right

		vLeft = CVector(rLine, vPos.GetY() - rHalfWidth);
		Transform(vLeft, &xFrom, &yFrom);

		vRight = CVector(rLine, vPos.GetY() + rHalfWidth);
		Transform(vRight, &xTo, &yTo);

		Dest.DrawLine(xFrom, yFrom, xTo, yTo, 1, RGB_GRID_LINE);

		//	Paint to the left

		vLeft = CVector(-rLine, vPos.GetY() - rHalfWidth);
		Transform(vLeft, &xFrom, &yFrom);
		
		vRight = CVector(-rLine, vPos.GetY() + rHalfWidth);
		Transform(vRight, &xTo, &yTo);

		Dest.DrawLine(xFrom, yFrom, xTo, yTo, 1, RGB_GRID_LINE);

		//	Next

		rLine += rInterval;
		}
	}

void CMapViewportCtx::Transform (const CVector &vPos, int *retx, int *rety) const

//	Transform
//
//	Transforms a global coordinate point to a screen coordinate.

	{
#ifdef USE_3D
	Metric rScale = RectWidth(m_rcView) * m_rMapScale / 2.0;

	CVector vTrans;
	C3DConversion::CalcCoord(rScale, vPos - m_vCenter, 0.0, &vTrans);

	*retx = (int)(m_xCenter + (vTrans.GetX() / m_rMapScale));
	*rety = (int)(m_yCenter - (vTrans.GetY() / m_rMapScale));
#else
	return m_Trans.Transform(vPos, retx, rety);
#endif
	}
