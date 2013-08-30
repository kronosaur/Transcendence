//	CWaveGenerator.cpp
//
//	CWaveGenerator classes
//	Copyright (c) 2013 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

CWaveGenerator2::CWaveGenerator2 (Metric rWave0Amp, Metric rWave0Cycles, Metric rWave1Amp, Metric rWave1Cycles, bool b0to1)

//	CWaveGenerator2 constructor

	{
	m_rWave0Cycles = rWave0Cycles;
	m_rWave1Cycles = rWave1Cycles;

	m_rWave0Amp = rWave0Amp * (b0to1 ? 0.5 : 1.0);
	m_rWave1Amp = rWave1Amp * (b0to1 ? 0.5 : 1.0);

	m_rOffset = (b0to1 ? 1.0 : 0.0);
	}

Metric CWaveGenerator2::GetValue (Metric rAngle)

//	GetValue
//
//	Gets the value at the angle

	{
	return m_rWave0Amp * (sin(rAngle * m_rWave0Cycles) + m_rOffset)
			+ m_rWave1Amp * (sin(rAngle * m_rWave1Cycles) + m_rOffset);
	}
