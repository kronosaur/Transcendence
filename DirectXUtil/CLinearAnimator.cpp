//	CLinearAnimator.cpp
//
//	CLinearAnimator class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

#include "Reanimator.h"

CLinearAnimator::CLinearAnimator (void) : m_iDuration(durationUndefined),
		m_iAnimateTime(0)

//	CLinearAnimator constructor

	{
	}

void CLinearAnimator::SetParams (const CAniProperty &Start, const CAniProperty &End, int iAnimateTime, int iDuration)

//	SetParams
//
//	Sets parameters

	{
	//	Must be a valid type

	if (Start.GetType() != End.GetType()
			|| Start.GetType() == CAniProperty::typeNone)
		return;

	//	Must be a valid time

	if (iAnimateTime <= 0)
		return;

	//	Initialize to basic values

	m_Start = Start;
	m_End = End;
	m_iAnimateTime = iAnimateTime;
	m_iDuration = iDuration;
	}

void CLinearAnimator::SetProperty (int iFrame, CAniProperty &Property)

//	SetProperty
//
//	Set parameters

	{
	if (iFrame >= m_iAnimateTime - 1)
		{
		Property = m_End;
		return;
		}

	//	Intermediat values

	switch (m_Start.GetType())
		{
		case CAniProperty::typeInteger:
			{
			int iDiff = m_End.GetInteger() - m_Start.GetInteger();
			Property.Set(CAniProperty::typeInteger, m_Start.GetInteger() + iFrame * iDiff / m_iAnimateTime);
			break;
			}

		case CAniProperty::typeMetric:
			{
			Metric rDiff = m_End.GetMetric() - m_Start.GetMetric();
			Property.Set(CAniProperty::typeMetric, m_Start.GetMetric() + iFrame * rDiff / m_iAnimateTime);
			break;
			}

		case CAniProperty::typeVector:
			{
			CVector vDiff = m_End.GetVector() - m_Start.GetVector();
			Property.Set(CAniProperty::typeVector, m_Start.GetVector() + (Metric)iFrame * vDiff / (Metric)m_iAnimateTime);
			break;
			}

		default:
			Property = m_End;
			break;
		}
	}
