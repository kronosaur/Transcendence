//	CNoiseGenerator.cpp
//
//	CNoiseGenerator class

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

CNoiseGenerator::CNoiseGenerator (int iScale) : m_iScale(iScale)

//	CNoiseGenerator constructor

	{
	int i;

	ASSERT(iScale >= 1);

	//	Generate tables

	m_Frac = new float [m_iScale];
	m_InvFrac = new float [m_iScale];
	m_Smooth = new float [m_iScale];

	for (i = 0; i < m_iScale; i++)
		{
		m_Frac[i] = (float)i / (float)m_iScale;

		//	If iScale is not a multiple of 2 then we offset so that
		//	we always hit 0.5. This is because the greatest noise magnitude
		//	is in the middle of grid points.

		if ((m_iScale % 2) == 1)
			m_Frac[i] += (1.0f / (float)(2 * m_iScale));

		m_InvFrac[i] = m_Frac[i] - 1.0f;
		m_Smooth[i] = NoiseSmoothStep(m_Frac[i]);
		}
	}

CNoiseGenerator::~CNoiseGenerator (void)

//	CNoiseGenerator destructor

	{
	delete m_Frac;
	delete m_InvFrac;
	delete m_Smooth;
	}
