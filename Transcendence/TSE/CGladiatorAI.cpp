//	CGladiatorAI.cpp
//
//	CGladiatorAI class

#include "PreComp.h"

#define WALL_RANGE						(g_KlicksPerPixel * 300.0)
#define WALL_RANGE2						(WALL_RANGE * WALL_RANGE)
#define MIN_POTENTIAL2					(g_KlicksPerPixel * g_KlicksPerPixel * 25.0)
#define FACING_TOLERANCE				(0.01)
#define FACING_TOLERANCE2				(FACING_TOLERANCE * FACING_TOLERANCE)
#define SEPARATION_RANGE				(g_KlicksPerPixel * 60)
#define SEPARATION_RANGE2				(SEPARATION_RANGE * SEPARATION_RANGE)
#define ATTACK_RANGE					(g_KlicksPerPixel * 600.0)

static CObjectClass<CGladiatorAI>g_Class(OBJID_CGLADIATORAI, NULL);

CGladiatorAI::CGladiatorAI (void) : CBaseShipAI(&g_Class),
		m_State(stateNone),
		m_pDest(NULL),
		m_pTarget(NULL)

//	CGladiatorAI constructor

	{
	}

void CGladiatorAI::Behavior (void)

//	Behavior
//
//	Fly, fight, die

	{
	//	Reset

	ResetBehavior();

	//	Behave according to our state

	switch (m_State)
		{
		case stateNone:
			BehaviorStart();
			break;

		case stateAttackingTarget:
			{
			ASSERT(m_pTarget);
			int iTick = m_pShip->GetSystem()->GetTick();

			//	Compute the potential

			if (((iTick + 5) % 11) == 0)
				{
				m_vPotential = ComputePotential();
#ifdef DEBUG_COMBAT
				m_pShip->SetDebugVector(m_vPotential);
#endif
				}

			//	Compute target dist

			CVector vTarget = m_pTarget->GetPos() - m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);

			//	Fire on the target as best we can

			int iFireDir;
			m_AICtx.ImplementFireWeaponOnTarget(m_pShip, -1, -1, m_pTarget, vTarget, rTargetDist2, &iFireDir);

			//	If our potential is high, then we want to move along that line

			if (m_vPotential.Length2() > MIN_POTENTIAL2)
				{
				int iDirectionToFace = VectorToPolar(m_vPotential, NULL);
				m_AICtx.ImplementManeuver(m_pShip, iDirectionToFace, true);
				}

			//	Otherwise, turn to face our target

			else
				{
				if (iFireDir != -1)
					m_AICtx.ImplementManeuver(m_pShip, iFireDir, false);
				}

			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);
			break;
			}

		case stateOnCourseForDocking:
			{
			ASSERT(m_pDest);
			m_AICtx.ImplementDocking(m_pShip, m_pDest);
			break;
			}

		case stateOnCourseForStargate:
			{
			m_AICtx.ImplementGating(m_pShip, m_pDest);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
			break;
			}

		default:
			ASSERT(false);
		}
	}

void CGladiatorAI::BehaviorStart (void)

//	BehaviorStart
//
//	Initiate behavior state based on orders

	{
	switch (GetCurrentOrder())
		{
		case IShipController::orderNone:
			break;

		case IShipController::orderDestroyTarget:
			{
			SetState(stateAttackingTarget);
			m_pTarget = GetCurrentOrderTarget();
			ASSERT(m_pTarget);
			ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
			break;
			}

		case IShipController::orderDock:
			{
			CSpaceObject *pDest = GetCurrentOrderTarget();
			ASSERT(pDest);

			//	If we're docked with our destination then we're done.

			if (m_pShip->GetDockedObj() == pDest)
				{
				CancelCurrentOrder();
				}

			//	Otherwise, try to dock

			else
				{
				SetState(stateOnCourseForDocking);
				m_pDest = pDest;
				}

			break;
			}

		case IShipController::orderGate:
			{
			//	Look for the gate

			CSpaceObject *pGate = GetCurrentOrderTarget();
			if (pGate == NULL)
				pGate = m_pShip->GetNearestStargate(true);

			//	Head for the gate

			if (pGate)
				{
				SetState(stateOnCourseForStargate);
				m_pDest = pGate;
				}

			break;
			}
		}
	}

CVector CGladiatorAI::ComputePotential (void)

//	ComputePotential
//
//	Analyses all surrouding objects and returns a vector for the path of
//	least resistance.

	{
	int i;
	Metric rDist;
	CVector vPotential;

	for (i = 0; i < m_pShip->GetSystem()->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = m_pShip->GetSystem()->GetObject(i);

		if (pObj  == NULL || pObj == m_pShip || pObj->IsInactive() || pObj->IsVirtual())
			NULL;
		else if (pObj->Blocks(m_pShip))
			{
			CVector vTarget = pObj->GetPos() - m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);

			//	There is a sharp potential away from walls

			if (rTargetDist2 < WALL_RANGE2)
				{
				CVector vTargetN = vTarget.Normal(&rDist);
				if (rDist > 0.0)
					vPotential = vPotential - (vTargetN * 100.0 * g_KlicksPerPixel * (WALL_RANGE / rDist));
				}
			}
		else if (pObj == m_pTarget)
			{
			CVector vTarget = pObj->GetPos() - m_pShip->GetPos();
			CVector vTargetN = vTarget.Normal(&rDist);
			Metric rTargetDist2 = rDist * rDist;

			if (m_AICtx.GetCombatStyle() == aicombatStandOff)
				{
				Metric rMaxRange2 = m_AICtx.GetBestWeaponRange2();
				Metric rIdealRange2 = 0.45 * rMaxRange2;

				//	If we're beyond our weapon's max range, then close on target

				if (rTargetDist2 > rMaxRange2)
					vPotential = vPotential + (vTargetN * 100.0 * g_KlicksPerPixel);

				//	If we're inside the ideal range, then move away from the target

				else if (rTargetDist2 < rIdealRange2)
					vPotential = vPotential - (vTargetN * 100.0 * g_KlicksPerPixel);
				}
			else
				{
				//	If we're not inside our primary weapon range, then there
				//	is a potential towards the player ship

				if (rTargetDist2 > m_AICtx.GetPrimaryAimRange2())
					vPotential = vPotential + (vTargetN * 100.0 * g_KlicksPerPixel);

				//	If we're not facing towards each other, and the enemy is facing towards
				//	us, then we probably want to move away from her line of fire.

				else if ((!AreAnglesAligned((pObj->GetRotation() + 180) % 360, m_pShip->GetRotation(), 36)
						&& ((vTargetN + PolarToVector(pObj->GetRotation(), 1.0)).Length2() < FACING_TOLERANCE2)))
					{
					CVector vPerpN = vTargetN.Perpendicular();
					vPotential = vPotential + (vPerpN * 100.0 * g_KlicksPerPixel);
					}
				}
			}
		else if (pObj->GetCategory() == CSpaceObject::catShip)
			{
			CVector vTarget = pObj->GetPos() - m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);

			//	If we get too close to other ships, move away

			if (rTargetDist2 < SEPARATION_RANGE2)
				{
				CVector vTargetN = vTarget.Normal(&rDist);
				if (rDist > 0.0)
					vPotential = vPotential - (vTargetN * 20.0 * g_KlicksPerPixel);
				}
			}
		}

	return vPotential;
	}

CString CGladiatorAI::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Returns debug crash info

	{
	CString sResult;

	sResult.Append(CONSTLIT("CGladiatorAI\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("Order: %d\r\n"), (int)GetCurrentOrder()));
	sResult.Append(strPatternSubst(CONSTLIT("m_State: %d\r\n"), m_State));
	sResult.Append(strPatternSubst(CONSTLIT("m_pDest: %s\r\n"), CSpaceObject::DebugDescribe(m_pDest)));
	sResult.Append(strPatternSubst(CONSTLIT("m_pTarget: %s\r\n"), CSpaceObject::DebugDescribe(m_pTarget)));

	return sResult;
	}

CSpaceObject *CGladiatorAI::GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget) const

//	GetTarget
//
//	Returns the target that this ship is attacking
	
	{
	switch (m_State)
		{
		case stateNone:
			return NULL;

		default:
			return m_pTarget;
		}
	}

void CGladiatorAI::OnObjDestroyedNotify (const SDestroyCtx &Ctx)

//	OnObjDestroyedNotify
//
//	Handle the case where another object is destroyed

	{
	switch (m_State)
		{
		//	No need to do anything in these cases because our order will get
		//	canceled and we will be called at OnOrderChanged.

		case stateAttackingTarget:
		case stateOnCourseForDocking:
			break;

		case stateOnCourseForStargate:
			SetState(stateNone);
			break;

		default:
			//	Generic reset

			if (m_pDest == Ctx.pObj)
				m_pDest = NULL;
			if (m_pTarget == Ctx.pObj)
				m_pTarget = NULL;
		}
	}

void CGladiatorAI::OnOrderChanged (void)

//	OnOrderChanged
//
//	The order list has changed

	{
	SetState(stateNone);
	}

void CGladiatorAI::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read our data
//
//	DWORD		m_State
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pDest (CSpaceObject ref)

	{
	Ctx.pStream->Read((char *)&m_State, sizeof(DWORD));
	CSystem::ReadObjRefFromStream(Ctx, &m_pTarget);
	CSystem::ReadObjRefFromStream(Ctx, &m_pDest);
	}

void CGladiatorAI::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write our data
//
//	DWORD		m_State
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pDest (CSpaceObject ref)

	{
	pStream->Write((char *)&m_State, sizeof(DWORD));
	m_pShip->WriteObjRefToStream(m_pTarget, pStream);
	m_pShip->WriteObjRefToStream(m_pDest, pStream);
	}

void CGladiatorAI::SetState (StateTypes State)

//	SetState
//
//	Sets the current state

	{
	//	Set state (NOTE: We do this before we undock because the Undock
	//	call may destroy the station and cause us to recurse into SetState.
	//	This happens when a ship is guarding an empty cargo crate).

	m_State = State;
	m_pTarget = NULL;
	m_pDest = NULL;

	//	If we're currently docked and we're changing to a state that
	//	does not support docking, then we undock first.

	if (m_pShip->GetDockedObj()
			&& State != stateNone) 
		m_pShip->Undock();
	}
