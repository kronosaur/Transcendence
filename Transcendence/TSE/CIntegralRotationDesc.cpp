//	CIntegralRotationDesc.cpp
//
//	CIntegralRotationDesc class
//	Copyright (c) 2014 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

#define MANEUVER_TAG							CONSTLIT("Maneuver")
#define THRUSTER_TAG							CONSTLIT("Thruster")
#define THRUST_EFFECT_TAG						CONSTLIT("ThrustEffect")

#define MANEUVER_ATTRIB							CONSTLIT("maneuver")
#define MAX_ROTATION_RATE_ATTRIB				CONSTLIT("maxRotationRate")
#define POS_ANGLE_ATTRIB						CONSTLIT("posAngle")
#define POS_RADIUS_ATTRIB						CONSTLIT("posRadius")
#define POS_Z_ATTRIB							CONSTLIT("posZ")
#define ROTATION_ATTRIB							CONSTLIT("rotation")
#define ROTATION_ACCEL_ATTRIB					CONSTLIT("rotationAccel")
#define ROTATION_COUNT_ATTRIB					CONSTLIT("rotationCount")
#define THRUST_EFFECT_ATTRIB					CONSTLIT("thrustEffect")

#define ROTATE_LEFT_MANEUVER					CONSTLIT("rotateLeft")
#define ROTATE_RIGHT_MANEUVER					CONSTLIT("rotateRight")

ALERROR CIntegralRotationDesc::Bind (SDesignLoadCtx &Ctx)

//	Bind
//
//	Bind the design

	{
	return NOERROR;
	}

int CIntegralRotationDesc::GetFrameIndex (int iAngle) const

//	GetFrameIndex
//
//	Returns the frame index, 0 to m_iCount-1, that corresponds to the given 
//	angle. Remember that frame 0 points straight up and frames rotate clockwise.

	{
	Metric rIndex = AngleMod(90 - iAngle) * m_iCount / 360.0;
	return (int)rIndex;
	}

int CIntegralRotationDesc::GetManeuverDelay (void) const

//	GetManeuverDelay
//
//	For compatibility we convert from our internal units to old style 
//	maneuverability (ticks per rotation angle)

	{
	return (m_iMaxRotationRate > 0 ? (int)(ROTATION_FRACTION / m_iMaxRotationRate) : 0);
	}

int CIntegralRotationDesc::GetManeuverability (void) const

//	GetManeuverability
//
//	For compatibility we convert from our internal units to old style 
//	maneuverability (game seconds per rotation angle)

	{
	return (m_iMaxRotationRate > 0 ? (int)(ROTATION_FRACTION * STD_SECONDS_PER_UPDATE / m_iMaxRotationRate) : 0);
	}

Metric CIntegralRotationDesc::GetMaxRotationSpeedPerTick (void) const

//	GetMaxRotationSpeedPerTick
//
//	Returns the max speed in degrees per tick.

	{
	return 360.0 * m_iMaxRotationRate / (ROTATION_FRACTION * m_iCount); 
	}

Metric CIntegralRotationDesc::GetRotationAccelPerTick (void) const

//	GetRotationAccelPerTick
//
//	Returns the degrees per tick acceleration.

	{
	return 360.0 * m_iRotationAccel / (ROTATION_FRACTION * m_iCount); 
	}

ALERROR CIntegralRotationDesc::InitFromXML (SDesignLoadCtx &Ctx, const CString &sUNID, int iImageScale, int iShipMass, CXMLElement *pDesc)

//	InitFromXML
//
//	Initialize from an XML descriptor

	{
	int i;

	//	If we have a Maneuver element, then use that (and ignore other attributes)

	CXMLElement *pManeuver = pDesc->GetContentElementByTag(MANEUVER_TAG);
	if (pManeuver)
		{
		m_iCount = pManeuver->GetAttributeIntegerBounded(ROTATION_COUNT_ATTRIB, 1, -1, STD_ROTATION_COUNT);

		//	Max rotation rate is in degrees per tick. We need to convert that to
		//	rotation frames per tick.

		Metric rDegreesPerTick = pManeuver->GetAttributeDoubleBounded(MAX_ROTATION_RATE_ATTRIB, 0.01, -1.0, 360.0 / STD_ROTATION_COUNT);
		m_iMaxRotationRate = Max(1, mathRound(ROTATION_FRACTION * rDegreesPerTick * m_iCount / 360.0));

		//	Also convert rotation acceleration

		Metric rAccelPerTick = pManeuver->GetAttributeDoubleBounded(ROTATION_ACCEL_ATTRIB, 0.01, -1.0, rDegreesPerTick);
		m_iRotationAccel = Max(1, mathRound(ROTATION_FRACTION * rAccelPerTick * m_iCount / 360.0));
		}

	//	Otherwise we look for attributes on the root (this is backwards compatible
	//	with version prior to API 20

	else
		{
		m_iCount = pDesc->GetAttributeIntegerBounded(ROTATION_COUNT_ATTRIB, 1, -1, STD_ROTATION_COUNT);

		//	The original maneuverability value is the number of half-ticks that 
		//	we take per rotation frame.
		//
		//	NOTE: For compatibility we don't allow maneuverability less than 2, which was the 
		//	limit using the old method (1 tick delay).

		int iManeuverability = pDesc->GetAttributeIntegerBounded(MANEUVER_ATTRIB, 2, -1, 2);

		//	Convert that to max rotation rate in 1/1000th of rotation per tick

		m_iMaxRotationRate = (int)(ROTATION_FRACTION * STD_SECONDS_PER_UPDATE / iManeuverability);

		//	Default acceleration is equal to rotation rate

		m_iRotationAccel = m_iMaxRotationRate;
		}

	//	Initialize the rotation angles

	Metric rFrameAngle = 360.0 / m_iCount;
	m_Rotations.InsertEmpty(m_iCount);
	for (i = 0; i < m_iCount; i++)
		m_Rotations[i].iRotation = AngleMod(mathRound(90.0 - i * rFrameAngle));

	return NOERROR;
	}
