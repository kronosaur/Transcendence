//	CIntegralRotation.cpp
//
//	CIntegralRotation class
//	Copyright (c) 2014 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

#define FIELD_THRUSTER_POWER					CONSTLIT("thrusterPower")

const Metric MANEUVER_MASS_FACTOR =				1.0;
const Metric MAX_INERTIA_RATIO =				9.0;

CIntegralRotation::~CIntegralRotation (void)

//	CIntegralRotation destructor

	{
	}

int CIntegralRotation::CalcFinalRotationFrame (const CIntegralRotationDesc &Desc) const

//	CalcFinalRotationFrame
//
//	Try to figure out what our final rotation frame would be if we stopped
//	thrusting and turned on inertia only.

	{
	int iRotationFrame = m_iRotationFrame;
	int iRotationSpeed = m_iRotationSpeed;

	while (iRotationSpeed > m_iRotationAccelStop || iRotationSpeed < -m_iRotationAccelStop)
		{
		if (iRotationSpeed > 0)
			iRotationSpeed = Max(0, iRotationSpeed - m_iRotationAccelStop);
		else
			iRotationSpeed = Min(0, iRotationSpeed + m_iRotationAccelStop);

		if (iRotationSpeed != 0)
			{
			int iFrameMax = Desc.GetFrameCount() * CIntegralRotationDesc::ROTATION_FRACTION;

			iRotationFrame = (iRotationFrame + iRotationSpeed) % iFrameMax;
			if (iRotationFrame < 0)
				iRotationFrame += iFrameMax;
			}
		}

	return iRotationFrame;
	}

EManeuverTypes CIntegralRotation::GetManeuverToFace (const CIntegralRotationDesc &Desc, int iAngle) const

//	GetManeuverToFace
//
//	Returns the maneuver required to face the given angle (or NoRotation if we're
//	already facing in that (rough) direction).

	{
	//	Convert to a frame index. NOTE: We figure out what our rotation would be
	//	if we stopped thrusting right now.

	int iCurrentFrameIndex = GetFrameIndex(CalcFinalRotationFrame(Desc));
	int iDesiredFrameIndex = Desc.GetFrameIndex(iAngle);

	//	See if we need to rotate

	if (iCurrentFrameIndex == iDesiredFrameIndex)
		return NoRotation;
	else
		{
		int iHalfFrames = Desc.GetFrameCount() / 2;
		int iDiff = iDesiredFrameIndex - iCurrentFrameIndex;
		if (iDiff > 0)
			{
			if (iDiff <= iHalfFrames)
				return RotateRight;
			else
				return RotateLeft;
			}
		else
			{
			if (-iDiff <= iHalfFrames)
				return RotateLeft;
			else
				return RotateRight;
			}
		}
	}

int CIntegralRotation::GetRotationAngle (const CIntegralRotationDesc &Desc) const

//	GetRotationAngle
//
//	Converts from our rotation frame to an angle

	{
	return Desc.GetRotationAngle(GetFrameIndex(m_iRotationFrame));
	}

void CIntegralRotation::Init (const CIntegralRotationDesc &Desc, int iRotationAngle)

//	Init
//
//	Initialize

	{
	//	Defaults

	m_iMaxRotationRate = Desc.GetMaxRotationSpeed();
	m_iRotationAccel = Desc.GetRotationAccel();
	m_iRotationAccelStop = Desc.GetRotationAccelStop();

	if (iRotationAngle != -1)
		SetRotationAngle(Desc, iRotationAngle);
	}

void CIntegralRotation::ReadFromStream (SLoadCtx &Ctx, const CIntegralRotationDesc &Desc)

//	ReadFromStream
//
//	Reads data

	{
	DWORD dwLoad;

	Init(Desc);

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iRotationFrame = (int)dwLoad;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iRotationSpeed = (int)dwLoad;

	//	Make sure our frame is within bounds; this can change if the ship's
	//	rotation count is altered in the XML.

	if (GetFrameIndex() >= Desc.GetFrameCount())
		SetRotationAngle(Desc, 0);
	}

void CIntegralRotation::SetRotationAngle (const CIntegralRotationDesc &Desc, int iAngle)

//	SetRotationAngle
//
//	Sets the rotation angle

	{
	m_iRotationFrame = CIntegralRotationDesc::ROTATION_FRACTION * Desc.GetFrameIndex(iAngle) + (CIntegralRotationDesc::ROTATION_FRACTION / 2);
	}

void CIntegralRotation::Update (const CIntegralRotationDesc &Desc, EManeuverTypes iManeuver)

//	Update
//
//	Updates once per tick

	{
	//	Change the rotation velocity

	switch (iManeuver)
		{
		case NoRotation:
			if (m_iRotationSpeed != 0)
				{
				//	Slow down rotation

				if (m_iRotationSpeed > 0)
					{
					m_iRotationSpeed = Max(0, m_iRotationSpeed - m_iRotationAccelStop);
					m_iLastManeuver = RotateLeft;
					}
				else
					{
					m_iRotationSpeed = Min(0, m_iRotationSpeed + m_iRotationAccelStop);
					m_iLastManeuver = RotateRight;
					}

				//	If we've stopped rotating, align to center of frame

				if (m_iRotationSpeed == 0)
					m_iRotationFrame = ((m_iRotationFrame / CIntegralRotationDesc::ROTATION_FRACTION) * CIntegralRotationDesc::ROTATION_FRACTION) + (CIntegralRotationDesc::ROTATION_FRACTION / 2);
				}
			else
				m_iLastManeuver = NoRotation;
			break;

		case RotateRight:
			if (m_iRotationSpeed < m_iMaxRotationRate)
				{
				if (m_iRotationSpeed < 0)
					m_iRotationSpeed = Min(m_iMaxRotationRate, m_iRotationSpeed + m_iRotationAccelStop);
				else
					m_iRotationSpeed = Min(m_iMaxRotationRate, m_iRotationSpeed + m_iRotationAccel);
				m_iLastManeuver = RotateRight;
				}
			else
				m_iLastManeuver = NoRotation;
			break;

		case RotateLeft:
			if (m_iRotationSpeed > -m_iMaxRotationRate)
				{
				if (m_iRotationSpeed > 0)
					m_iRotationSpeed = Max(-m_iMaxRotationRate, m_iRotationSpeed - m_iRotationAccelStop);
				else
					m_iRotationSpeed = Max(-m_iMaxRotationRate, m_iRotationSpeed - m_iRotationAccel);
				m_iLastManeuver = RotateLeft;
				}
			else
				m_iLastManeuver = NoRotation;
			break;
		}

	//	Now rotate

	if (m_iRotationSpeed != 0)
		{
		int iFrameMax = Desc.GetFrameCount() * CIntegralRotationDesc::ROTATION_FRACTION;

		m_iRotationFrame = (m_iRotationFrame + m_iRotationSpeed) % iFrameMax;
		if (m_iRotationFrame < 0)
			m_iRotationFrame += iFrameMax;
		}
	}

void CIntegralRotation::UpdateAccel (const CIntegralRotationDesc &Desc, Metric rHullMass, Metric rItemMass)

//	UpdateAccel
//
//	Recalculates rotation acceleration based on the mass of the ship.

	{
	//	If we have no mass, then we just take the default acceleration

	if (rHullMass == 0.0)
		{
		m_iMaxRotationRate = Desc.GetMaxRotationSpeed();
		m_iRotationAccel = Desc.GetRotationAccel();
		}

	//	Otherwise we compute based on the mass

	else
		{
		Metric rExtraMass = (rItemMass - rHullMass) * MANEUVER_MASS_FACTOR;

		//	If we don't have too much extra mass, then rotation is not affected.

		if (rExtraMass <= 0.0)
			{
			m_iMaxRotationRate = Desc.GetMaxRotationSpeed();
			m_iRotationAccel = Desc.GetRotationAccel();
			return;
			}

		//	Otherwise, we slow down

		Metric rRatio = 1.0f / Min(MAX_INERTIA_RATIO, (1.0f + (rExtraMass / rHullMass)));
		m_iRotationAccel = Max(1, (int)mathRound(rRatio * Desc.GetRotationAccel()));
		m_iRotationAccelStop = Max(1, (int)mathRound(rRatio * Desc.GetRotationAccelStop()));
		m_iMaxRotationRate = Max(1, (int)mathRound(pow(rRatio, 0.3) * Desc.GetMaxRotationSpeed()));
		}
	}

void CIntegralRotation::WriteToStream (IWriteStream *pStream) const

//	WriteToStream
//
//	Writes data

	{
	DWORD dwSave = m_iRotationFrame;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = m_iRotationSpeed;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}
