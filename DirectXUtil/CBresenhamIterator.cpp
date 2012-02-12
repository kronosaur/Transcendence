//	CBresenhamIterator.cpp
//
//	CBresenhamIterator class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

void CBresenhamIterator::Init (int x1, int y1, int x2, int y2)

//	Init
//
//	Initialize the iterator

	{
	m_x2 = x2;
	m_y2 = y2;

	m_dx = x2 - x1;
	m_ax = Absolute(m_dx) * 2;
	m_sx = (m_dx > 0 ? 1 : -1);

	m_dy = y2 - y1;
	m_ay = Absolute(m_dy) * 2;
	m_sy = (m_dy > 0 ? 1 : -1);

	m_x = x1;
	m_y = y1;

	if (m_ax > m_ay)
		{
		m_d = m_ay - m_ax / 2;
		m_bXDominant = true;
		}
	else
		{
		m_d = m_ax - m_ay / 2;
		m_bXDominant = false;
		}
	}

bool CBresenhamIterator::Step (void)

//	Step
//
//	Steps to the next point on the line.
//	Returns FALSE if we've stepped beyond the end of the line

	{
	if (m_bXDominant)
		{
		if (m_x == m_x2)
			return false;
		else if (m_d >= 0)
			{
			m_y = m_y + m_sy;
			m_d = m_d - m_ax;
			}

		m_x = m_x + m_sx;
		m_d = m_d + m_ay;
		}
	else
		{
		if (m_y == m_y2)
			return false;
		else if (m_d >= 0)
			{
			m_x = m_x + m_sx;
			m_d = m_d - m_ay;
			}

		m_y = m_y + m_sy;
		m_d = m_d + m_ax;
		}

	return true;
	}
