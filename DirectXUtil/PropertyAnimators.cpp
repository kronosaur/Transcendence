//	PropertyAnimators.cpp
//
//	Property animator classes

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

#include "Reanimator.h"

//	CLinearFade ----------------------------------------------------------------

void CLinearFade::SetParams (int iDuration, int iFadeIn, int iFadeOut, DWORD dwMaxOpacity)

//	SetParams
//
//	Sets parameters

	{
	ASSERT(iFadeIn >= 0);
	ASSERT(iFadeOut >= 0);
	ASSERT(iDuration >= 0 || iFadeOut == 0);
	ASSERT(iDuration < 0 || (iFadeOut + iFadeIn) <= iDuration);

	m_iDuration = iDuration;
	m_iFadeIn = iFadeIn;
	m_iFadeOut = iFadeOut;
	m_dwMaxOpacity = dwMaxOpacity;
	}

void CLinearFade::SetProperty (int iFrame, CAniProperty &Property)

//	SetProperty
//
//	Sets the property

	{
	DWORD dwOpacity;
	if (iFrame < m_iFadeIn)
		dwOpacity = m_dwMaxOpacity * iFrame / m_iFadeIn;
	else if (m_iDuration >= 0)
		{
		if (iFrame + m_iFadeOut > m_iDuration)
			dwOpacity = m_dwMaxOpacity * (m_iDuration - iFrame) / m_iFadeOut;
		else if (iFrame > m_iDuration)
			dwOpacity = 0;
		else
			dwOpacity = m_dwMaxOpacity;
		}
	else
		dwOpacity = m_dwMaxOpacity;

	Property.Set(CAniProperty::typeOpacity, dwOpacity);
	}

//	CLinearMetric ---------------------------------------------------------------

void CLinearMetric::SetParams (Metric rStart, Metric rEnd, Metric rIncrement)

//	SetParams
//
//	Sets parameters

	{
	ASSERT((rStart <= rEnd && rIncrement >= 0.0) || (rStart >= rEnd && rIncrement <= 0.0));

	m_rStart = rStart;
	m_rEnd = rEnd;
	m_rIncrement = rIncrement;

	if (rIncrement == 0.0)
		m_iDuration = 0;
	else
		m_iDuration = (int)((m_rEnd - m_rStart) / m_rIncrement);
	}

void CLinearMetric::SetProperty (int iFrame, CAniProperty &Property)

//	SetProperty
//
//	Set parameters

	{
	if (iFrame >= m_iDuration - 1)
		Property.Set(CAniProperty::typeMetric, m_rEnd);
	else
		Property.Set(CAniProperty::typeMetric, (Metric)(m_rStart + (iFrame * m_rIncrement)));
	}

//	CLinearRotation ------------------------------------------------------------

void CLinearRotation::SetParams (Metric rStartAngle, Metric rRate, int iAnimateTime, int iDuration)

//	SetParams
//
//	Sets parameters

	{
	if (rStartAngle < 0.0)
		return;

	m_rStartAngle = rStartAngle;
	m_rRate = rRate;
	m_iAnimateTime = iAnimateTime;
	m_iDuration = iDuration;
	}

void CLinearRotation::SetProperty (int iFrame, CAniProperty &Property)

//	SetProperty
//
//	Set parameters

	{
	Metric rAngle;
	if (m_iAnimateTime > 0 && iFrame >= m_iAnimateTime - 1)
		rAngle = (m_rStartAngle + m_iAnimateTime * m_rRate);
	else
		rAngle = (m_rStartAngle + iFrame * m_rRate);

	if (rAngle > 0.0)
		Property.Set(CAniProperty::typeInteger, (int)rAngle % 360);
	else
		Property.Set(CAniProperty::typeInteger, 360 - ((int)-rAngle % 360));
	}

