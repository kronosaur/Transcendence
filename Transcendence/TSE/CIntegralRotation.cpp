//	CIntegralRotation.cpp
//
//	CIntegralRotation class
//	Copyright (c) 2014 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

const Metric MANEUVER_MASS_FACTOR =				1.0;
const Metric MAX_INERTIA_RATIO =				9.0;

EManeuverTypes CIntegralRotation::GetManeuverToFace (const CIntegralRotationDesc &Desc, int iAngle) const

//	GetManeuverToFace
//
//	Returns the maneuver required to face the given angle (or NoRotation if we're
//	already facing in that (rough) direction).

	{
	//	Convert to a frame index

	int iCurrentFrameIndex = GetFrameIndex();
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

void CIntegralRotation::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads data

	{
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iRotationFrame = (int)dwLoad;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iRotationSpeed = (int)dwLoad;
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
					m_iRotationSpeed = Max(0, m_iRotationSpeed - m_iRotationAccel);
				else
					m_iRotationSpeed = Min(0, m_iRotationSpeed + m_iRotationAccel);

				//	If we've stopped rotating, align to center of frame

				if (m_iRotationSpeed == 0)
					m_iRotationFrame = ((m_iRotationFrame / CIntegralRotationDesc::ROTATION_FRACTION) * CIntegralRotationDesc::ROTATION_FRACTION) + (CIntegralRotationDesc::ROTATION_FRACTION / 2);
				}
			break;

		case RotateRight:
			m_iRotationSpeed = Min(m_iMaxRotationRate, m_iRotationSpeed + m_iRotationAccel);
			break;

		case RotateLeft:
			m_iRotationSpeed = Max(-m_iMaxRotationRate, m_iRotationSpeed - m_iRotationAccel);
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
		m_iRotationAccel = (int)mathRound(rRatio * Desc.GetRotationAccel());
		m_iMaxRotationRate = (int)mathRound(pow(rRatio, 0.3) * Desc.GetMaxRotationSpeed());
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
