//	CArtifactResultPainter.cpp
//
//	CArtifactResultPainter class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

CArtifactResultPainter::CArtifactResultPainter (CArtifactProgram *pSource,
												int xSource,
												int ySource,
												CArtifactAwakening::EEventTypes iEvent,
												CArtifactProgram *pTarget,
												int xTarget,
												int yTarget,
												int iDelay) :
		m_pSource(pSource),
		m_xSource(xSource),
		m_ySource(ySource),
		m_iEvent(iEvent),
		m_pTarget(pTarget),
		m_xTarget(xTarget),
		m_yTarget(yTarget),
		m_iDelay(iDelay),
		m_bMarked(false)

	//	CArtifactResultPainter constructor

	{
	switch (m_iEvent)
		{
		case CArtifactAwakening::eventActivated:
			m_iStyle = styleActivateProgram;
			break;

		case CArtifactAwakening::eventHalted:
			m_iStyle = styleArcLightning;
			m_rgbPrimaryColor = CG32bitPixel(255, 255, 255);
			m_rgbSecondaryColor = CG32bitPixel(255, 128, 128);
			break;

		case CArtifactAwakening::eventEgoChanged:
		case CArtifactAwakening::eventIntelligenceChanged:
		case CArtifactAwakening::eventWillpowerChanged:
			{
			m_iStyle = styleCircuit;
			m_rgbPrimaryColor = CG32bitPixel(255, 255, 255);
			m_rgbSecondaryColor = CG32bitPixel(255, 128, 255);

			CircuitLineCreate(CVector(xSource, ySource), CVector(xTarget, yTarget), CVector(), m_Line);

			CGPath LinePath;
			LinePath.Init(m_Line, true);
			CGPath ThickPath;
			LinePath.CreateSubPathStrokePath(0, 3.0, &ThickPath);

			ThickPath.Rasterize(&m_Region);
			break;
			}
		}
	}

CVector CArtifactResultPainter::CircuitLineBendPoint (const CVector &vFrom, const CVector &vTo)

//	CircuitLineBendPoint
//
//	Returns the bend point.

	{
	Metric rdY = vTo.GetY() - vFrom.GetY();
	Metric rdX = vTo.GetX() - vFrom.GetX();

	return CVector(vFrom.GetX() + ((rdX < 0.0 ? -0.5 : 0.5) * Absolute(rdY)), vTo.GetY());
	}

void CArtifactResultPainter::CircuitLineCreate (const CVector &vFrom, const CVector &vTo, const CVector &vMiddle, TArray<CVector> &Result)

//	CircuitLineCreate
//
//	Creates a set of points forming a circuit line.

	{
	Result.DeleteAll();
	Result.Insert(vFrom);
	Result.Insert(CircuitLineBendPoint(vFrom, vTo));
	Result.Insert(vTo);

	CircuitLineSplit(Result, 0, 2, 1);
	}

void CArtifactResultPainter::CircuitLineSplit (TArray<CVector> &Result, int iStart, int iEnd, int iDepth, int *retiNewEnd)

//	CircuitLineSplit
//
//	Splits a circuit

	{
	//	Compute the split points

	CVector vA = CircuitLineSplitPoint(Result[iStart], Result[iStart + 1]);
	CVector vB = CircuitLineSplitPoint(Result[iStart + 1], Result[iEnd]);

	CVector vC;
	if (vA.GetY() == Result[iStart].GetY())
		vC = CircuitLineBendPoint(vA, vB);
	else
		vC = CircuitLineBendPoint(vB, vA);

	//	Splite the line

	Result.InsertEmpty(2, iStart + 1);
	Result[iStart + 1] = vA;
	Result[iStart + 2] = vC;
	Result[iStart + 3] = vB;

	//	Recurse, if necessary

	if (iDepth > 0)
		{
		int iNewMid;
		CircuitLineSplit(Result, iStart, iStart + 2, iDepth - 1, &iNewMid);
		CircuitLineSplit(Result, iNewMid, iNewMid + 2, iDepth - 1, retiNewEnd);
		}
	else
		{
		if (retiNewEnd)
			*retiNewEnd = iStart + 4;
		}
	}

CVector CArtifactResultPainter::CircuitLineSplitPoint (const CVector &vFrom, const CVector &vTo)

//	CircuitLineSplitPoint
//
//	Returns a random point along the line to split

	{
	Metric rdX = vTo.GetX() - vFrom.GetX();
	Metric rdY = vTo.GetY() - vFrom.GetY();

	Metric rSplit = Max(0.15, Min(mathRandomDouble(), 0.85));

	return CVector(vFrom.GetX() + rSplit * rdX, vFrom.GetY() + rSplit * rdY);
	}

CArtifactProgram *CArtifactResultPainter::GetProgramActivated (void) const

//	GetProgramActivated
//
//	If this effect activates a program, and if the effect is active, then we return
//	the program being activated. Otherwise, we return NULL.

	{
	if (m_iDelay > 0)
		return NULL;

	switch (m_iEvent)
		{
		case CArtifactAwakening::eventActivated:
			return m_pSource;

		default:
			return NULL;
		}
	}

CArtifactProgram *CArtifactResultPainter::GetProgramHalted (void) const

//	GetProgramHalted
//
//	If this effect halts a program, and if the effect is active, then we return
//	the program being halted. Otherwise, we return NULL.

	{
	if (m_iDelay > 0)
		return NULL;

	switch (m_iEvent)
		{
		case CArtifactAwakening::eventHalted:
			return m_pTarget;

		default:
			return NULL;
		}
	}

bool CArtifactResultPainter::IsEqualTo (const CArtifactResultPainter &Src) const

//	IsEqualTo
//
//	Returns TRUE if these are the same effect

	{
	return (m_pSource == Src.m_pSource
			&& m_iEvent == Src.m_iEvent
			&& m_pTarget == Src.m_pTarget);
	}

void CArtifactResultPainter::Paint (CG32bitImage &Dest) const

//	Paint
//
//	Paints the effect

	{
	//	If we're delayed, don't paint

	if (m_iDelay > 0)
		return;

	//	Paint

	switch (m_iStyle)
		{
		case styleArcLightning:
			DrawLightning(Dest, m_xSource, m_ySource, m_xTarget, m_yTarget, m_rgbPrimaryColor, m_rgbSecondaryColor, 0.5);
			break;

		case styleCircuit:
			CGDraw::Region(Dest, 0, 0, m_Region, m_rgbSecondaryColor);
			break;
		}
	}

void CArtifactResultPainter::Update (void)

//	Update
//
//	Updates the effect

	{
	if (m_iDelay > 0)
		m_iDelay--;
	}
