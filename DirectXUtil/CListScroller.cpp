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

void CListScroller::AddScrollPos (Metric rPos, Metric rAnchor)

//	AddScrollPos
//
//	Adds a new line position

	{
	if (GetCount() == 0)
		m_rLastPos = rPos;

	SEntry *pEntry = m_ScrollPos.Insert();
	pEntry->rLinePos = rAnchor;
	pEntry->rScrollPos = rPos;
	}

int CListScroller::GetCurScrollPos (void)

//	GetCurScrollPos
//
//	Gets the current scroll position

	{
	int i;

	for (i = 0; i < GetCount(); i++)
		{
		if (m_rEnd <= m_ScrollPos[i].rScrollPos)
			return i;
		}

	return GetCount() - 1;
	}

void CListScroller::SetScrollToPos (int iPos, Metric rSpeed)

//	SetScrollToPos
//
//	Sets parameters so we animate a scroll to the given line

	{
	if (rSpeed == 0.0)
		SetAtPos(iPos);
	else if (GetCount() == 0)
		{
		m_rStart = 0.0;
		m_rEnd = 0.0;
		m_rIncrement = rSpeed;
		m_iDuration = 1;
		}
	else
		{
		m_rStart = m_rLastPos;
		m_rEnd = (iPos != -1 ? m_ScrollPos[iPos].rScrollPos : m_ScrollPos[GetCount() - 1].rScrollPos);

		if (rSpeed < 0.0)
			m_rIncrement = (m_rEnd - m_rStart) / -rSpeed;
		else
			m_rIncrement = (m_rEnd >= m_rStart ? rSpeed : -rSpeed);

		m_iDuration = (int)((m_rEnd - m_rStart) / m_rIncrement);
		}
	}

void CListScroller::SetAtPos (int iPos)

//	SetAtPos
//
//	Sets the scroll position at the given line

	{
	m_rStart = m_ScrollPos[iPos].rScrollPos;
	m_rEnd = m_ScrollPos[iPos].rScrollPos;
	m_rIncrement = 0.0;
	m_iDuration = 1;
	m_rLastPos = m_rStart;
	}

void CListScroller::SetProperty (int iFrame, CAniProperty &Property)

//	SetProperty
//
//	Sets the property

	{
	if (iFrame >= m_iDuration - 1)
		m_rLastPos = m_rEnd;
	else
		m_rLastPos = (Metric)(m_rStart + (iFrame * m_rIncrement));

	Property.Set(CAniProperty::typeMetric, m_rLastPos);
	}
