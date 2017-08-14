//	CManeuverController.cpp
//
//	CManeuverController class
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

const Metric MAX_DELTA_VEL =			(g_KlicksPerPixel / 2.0);
const Metric MAX_DELTA_VEL2 =			(MAX_DELTA_VEL * MAX_DELTA_VEL);

const Metric MAX_IN_FORMATION_DELTA	=	(2.0 * KLICKS_PER_PIXEL);
const Metric MAX_IN_FORMATION_DELTA2 =	(MAX_IN_FORMATION_DELTA * MAX_IN_FORMATION_DELTA);

CManeuverController::CManeuverController (void) :
		m_iCommand(cmdNone),
		m_iAngle(0),
		m_pTarget(NULL)

//	CManeuverController constructor

	{
	}

bool CManeuverController::CmdCancel (void)

//	CmdCancel
//
//	Cancel current maneuver

	{
	m_iCommand = cmdNone;
	m_iAngle = -1;
	m_vPos = CVector();
	m_pTarget = NULL;

	m_iManeuver = NoRotation;
	m_bThrust = false;

	return true;
	}

bool CManeuverController::CmdMouseAim (int iAngle)

//	CmdMouseAim
//
//	Turn to the given angle

	{
	CmdCancel();

	m_iCommand = cmdMouseAim;
	m_iAngle = iAngle;

	return true;
	}

bool CManeuverController::CmdMoveTo (const CVector &vPos)

//	CmdMoveTo
//
//	The ship moves to the given position.

	{
	CmdCancel();

	m_iCommand = cmdMoveTo;
	m_vPos = vPos;

	return true;
	}

EManeuverTypes CManeuverController::GetManeuver (CShip *pShip) const

//	GetManeuver
//
//	Returns the maneuver for this tick.

	{
	switch (m_iCommand)
		{
		case cmdMouseAim:
			if (m_iAngle == -1
					|| pShip->IsOutOfPower()
					|| pShip->IsTimeStopped())
				return NoRotation;

			return pShip->GetManeuverToFace(m_iAngle);

		case cmdMoveTo:
			return m_iManeuver;
			
		default:
			return NoRotation;
		}
	}

bool CManeuverController::GetThrust (CShip *pShip) const

//	GetThrust
//
//	Returns the thrust state

	{
	switch (m_iCommand)
		{
		case cmdMoveTo:
			return m_bThrust;

		default:
			return false;
		}
	}

bool CManeuverController::IsThrustActive (void) const

//	IsThrustActive
//
//	Returns TRUE if we are controlling main engine

	{
	switch (m_iCommand)
		{
		case cmdDockWith:
		case cmdMoveTo:
			return true;

		default:
			return false;
		}
	}

void CManeuverController::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads from a stream
//
//	DWORD			m_iCommand
//	DWORD			m_iAngle
//	CVector			m_vPos
//	CSpaceObject	m_pTarget

	{
	DWORD dwLoad;

	Ctx.pStream->Read(dwLoad);
	m_iCommand = (ECommands)dwLoad;

	Ctx.pStream->Read(m_iAngle);
	m_vPos.ReadFromStream(*Ctx.pStream);

	CSystem::ReadObjRefFromStream(Ctx, &m_pTarget);
	}

void CManeuverController::Update (SUpdateCtx &Ctx, CShip *pShip)

//	Update
//
//	Update behavior

	{
	switch (m_iCommand)
		{
		case cmdMoveTo:
			UpdateMoveTo(Ctx, pShip);
			break;
		}
	}

void CManeuverController::UpdateMoveTo (SUpdateCtx &Ctx, CShip *pShip)

//	UpdateMoveTo
//
//	Player ship moves to m_vPos

	{
	//	Compute our formation parameters

	CVector vDeltaV;
	Metric rDeltaPos2;
	Metric rDeltaV2;
	bool bCloseEnough = CShipAIHelper::CalcFormationParams(pShip, m_vPos, NullVector, &vDeltaV, &rDeltaPos2, &rDeltaV2);

	//	If we're close enough to the velocity, cheat a little by
	//	accelerating without using the main engine

	if (bCloseEnough)
		{
		if (!pShip->IsParalyzed())
			pShip->Accelerate(vDeltaV * pShip->GetMass() / 2000.0, g_SecondsPerUpdate);

		m_iManeuver = NoRotation;
		m_bThrust = false;

		//	If we're close enough to the destination, then the command is done

		if ((rDeltaPos2 < MAX_IN_FORMATION_DELTA2) && (rDeltaV2 < MAX_DELTA_VEL2))
			CmdCancel();
		}

	//	Otherwise, thrust with the main engines

	else
		{
		//	Figure out which direction we need to thrust in to achieve the delta v

		int iAngle = VectorToPolar(vDeltaV);

		//	Turn towards the angle

		m_iManeuver = pShip->GetManeuverToFace(iAngle);

		//	If we don't need to turn, engage thrust

		m_bThrust = (m_iManeuver == NoRotation);
		}
	}

void CManeuverController::WriteToStream (IWriteStream &Stream, CSystem *pSystem)

//	WriteToStream
//
//	Writes to a stream
//
//	DWORD			m_iCommand
//	DWORD			m_iAngle
//	CVector			m_vPos
//	CSpaceObject	m_pTarget

	{
	DWORD dwSave;

	dwSave = (DWORD)m_iCommand;
	Stream.Write(dwSave);

	Stream.Write(m_iAngle);
	m_vPos.WriteToStream(Stream);

	pSystem->WriteObjRefToStream(m_pTarget, &Stream);
	}

