//	CAniSolidLine.cpp
//
//	CAniSolidLine class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

#include "Reanimator.h"

#define PROP_LINE_COLOR						CONSTLIT("lineColor")
#define PROP_LINE_WIDTH						CONSTLIT("lineWidth")

void CAniSolidLine::Arc (SAniPaintCtx &Ctx, int xCenter, int yCenter, int iRadius, int iStartAngle, int iEndAngle)

//	Arc
//
//	Draw an arc
//
//	LATER: For now we only support axis-aligned arcs.

	{
	//	Temporaries

	int iHalfWidth = m_iWidth / 2;
	Metric rOuterRadius = iRadius + iHalfWidth;
	Metric rInnerRadius = rOuterRadius - m_iWidth;
	Metric rOuterRadius2 = rOuterRadius * rOuterRadius;
	Metric rInnerRadius2 = rInnerRadius * rInnerRadius;

	//	Figure out which quadrants we paint

	bool bUpperRight;
	bool bUpperLeft;
	bool bLowerLeft;
	bool bLowerRight;
	if (iStartAngle <= iEndAngle)
		{
		bUpperRight = (iStartAngle < 90);
		bUpperLeft = (iStartAngle < 180) && (iEndAngle > 90);
		bLowerLeft = (iStartAngle < 270) && (iEndAngle > 180);
		bLowerRight = (iStartAngle < 360) && (iEndAngle > 270);
		}
	else
		{
		bUpperRight = (iEndAngle > 0);
		bUpperLeft = (iEndAngle > 90) || (iStartAngle < 180);
		bLowerLeft = (iEndAngle > 180) || (iStartAngle < 270);
		bLowerRight = (iEndAngle > 270) || (iStartAngle < 360);
		}

	//	Iterate from the center up (and use symmetry for the four quadrants)

	int iRow = 0;
	Metric rRow = 0.5;
	Metric rRowEnd = rOuterRadius;
	while (rRow < rRowEnd)
		{
		Metric rRow2 = rRow * rRow;

		Metric rOuterLen = sqrt(rOuterRadius2 - rRow2);
		Metric rInnerLen = (rRow < rInnerRadius ? sqrt(rInnerRadius2 - rRow2) : 0.0);
		Metric rWidth = rOuterLen - rInnerLen;

		int iSolidOuter = (int)rOuterLen;
		Metric rOuterFraction = rOuterLen - (Metric)iSolidOuter;
		int iSolidWidth = (int)(rWidth - rOuterFraction);
		Metric rInnerFraction = (rWidth - rOuterFraction) - (Metric)iSolidWidth;

		//	If we have a solid width, then paint the solid part of the edge

		if (iSolidWidth > 0)
			{
			if (bUpperRight)
				Ctx.Dest.FillLine(xCenter + iSolidOuter - iSolidWidth, yCenter - 1 - iRow, iSolidWidth, m_wColor);
			if (bLowerRight)
				Ctx.Dest.FillLine(xCenter + iSolidOuter - iSolidWidth, yCenter + iRow, iSolidWidth, m_wColor);
			if (bUpperLeft)
				Ctx.Dest.FillLine(xCenter - iSolidOuter, yCenter - 1 - iRow, iSolidWidth, m_wColor);
			if (bLowerLeft)
				Ctx.Dest.FillLine(xCenter - iSolidOuter, yCenter + iRow, iSolidWidth, m_wColor);
			}

		//	Paint the edges

		DWORD byOuterEdge = (DWORD)(255 * rOuterFraction);
		if (byOuterEdge)
			{
			if (bUpperRight)
				Ctx.Dest.SetPixelTrans(xCenter + iSolidOuter, yCenter - 1 - iRow, m_wColor, byOuterEdge);
			if (bLowerRight)
				Ctx.Dest.SetPixelTrans(xCenter + iSolidOuter, yCenter + iRow, m_wColor, byOuterEdge);
			if (bUpperLeft)
				Ctx.Dest.SetPixelTrans(xCenter - iSolidOuter - 1, yCenter - 1 - iRow, m_wColor, byOuterEdge);
			if (bLowerLeft)
				Ctx.Dest.SetPixelTrans(xCenter - iSolidOuter - 1, yCenter + iRow, m_wColor, byOuterEdge);
			}

		DWORD byInnerEdge = (DWORD)(255 * rInnerFraction);
		if (byInnerEdge)
			{
			if (bUpperRight)
				Ctx.Dest.SetPixelTrans(xCenter + iSolidOuter - iSolidWidth - 1, yCenter - 1 - iRow, m_wColor, byInnerEdge);
			if (bLowerRight)
				Ctx.Dest.SetPixelTrans(xCenter + iSolidOuter - iSolidWidth - 1, yCenter + iRow, m_wColor, byInnerEdge);
			if (bUpperLeft)
				Ctx.Dest.SetPixelTrans(xCenter - iSolidOuter + iSolidWidth, yCenter - 1 - iRow, m_wColor, byInnerEdge);
			if (bLowerLeft)
				Ctx.Dest.SetPixelTrans(xCenter - iSolidOuter + iSolidWidth, yCenter + iRow, m_wColor, byInnerEdge);
			}

		//	Next

		iRow++;
		rRow = (Metric)iRow + 0.5;
		}
	}

void CAniSolidLine::Corner (SAniPaintCtx &Ctx, int xCenter, int yCenter)

//	Corner
//
//	Draw the corner where two 90-degree lines join.

	{
	int iHalfWidth = m_iWidth / 2;
	Ctx.Dest.Fill(xCenter - iHalfWidth, yCenter - iHalfWidth, m_iWidth, m_iWidth, m_wColor);
	}

void CAniSolidLine::InitDefaults (CAniPropertySet &Properties)

//	InitDefaults
//
//	Initialize properties

	{
	if (Properties.FindProperty(PROP_LINE_COLOR) == -1)
		Properties.SetColor(PROP_LINE_COLOR, 0xffff);

	if (Properties.FindProperty(PROP_LINE_WIDTH) == -1)
		Properties.SetOpacity(PROP_LINE_WIDTH, 1);
	}

void CAniSolidLine::InitPaint (SAniPaintCtx &Ctx, int xOrigin, int yOrigin, CAniPropertySet &Properties)

//	InitPaint
//
//	Initialize paint context

	{
	m_wColor = Properties.GetColor(PROP_LINE_COLOR);
	m_iWidth = Properties.GetInteger(PROP_LINE_WIDTH);
	}

void CAniSolidLine::Line (SAniPaintCtx &Ctx, int xFrom, int yFrom, int xTo, int yTo)

//	Line
//
//	Draw a line

	{
	if (m_iWidth == 0)
		;
	else if (m_iWidth == 1)
		{
		if (xFrom == xTo)
			Ctx.Dest.FillColumn(xFrom, yFrom, yTo - yFrom, m_wColor);
		else if (yFrom == yTo)
			Ctx.Dest.FillLine(xFrom, yFrom, xTo - xFrom, m_wColor);
		else
			Ctx.Dest.DrawLine(xFrom, yFrom, xTo, yTo, m_iWidth, m_wColor);
		}
	else
		{
		if (xFrom == xTo)
			{
			int iHalfWidth = m_iWidth / 2;
			Ctx.Dest.Fill(xFrom - iHalfWidth, yFrom, m_iWidth, yTo - yFrom, m_wColor);
			}
		else if (yFrom == yTo)
			{
			int iHalfWidth = m_iWidth / 2;
			Ctx.Dest.Fill(xFrom, yFrom - iHalfWidth, xTo - xFrom, m_iWidth, m_wColor);
			}
		else
			Ctx.Dest.DrawLine(xFrom, yFrom, xTo, yTo, m_iWidth, m_wColor);
		}
	}
