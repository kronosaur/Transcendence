//	CZoanthropeAI.cpp
//
//	CZoanthropeAI class

#include "PreComp.h"


#define ATTACK_RANGE					(g_KlicksPerPixel * 600.0)
#define PATROL_SENSOR_RANGE				(g_KlicksPerPixel * 1000.0)
#define SEPARATION_RANGE				(g_KlicksPerPixel * 100)
#define SEPARATION_RANGE2				(SEPARATION_RANGE * SEPARATION_RANGE)
#define MAX_FLOCK_DIST					(g_KlicksPerPixel * 600)
#define MAX_FLOCK_DIST2					(MAX_FLOCK_DIST * MAX_FLOCK_DIST)
#define MIN_TARGET_DIST					(6.0 * LIGHT_SECOND)
#define MIN_TARGET_DIST2				(MIN_TARGET_DIST * MIN_TARGET_DIST)
#define COMBAT_RANGE					(g_KlicksPerPixel * 300)
#define COMBAT_RANGE2					(COMBAT_RANGE * COMBAT_RANGE)
#define TAME_CHANCE						50

static CObjectClass<CZoanthropeAI>g_Class(OBJID_CZOANTHROPEAI, NULL);

CZoanthropeAI::CZoanthropeAI (void) : CBaseShipAI(&g_Class),
		m_State(stateNone),
		m_pTarget(NULL),
		m_pBase(NULL)

//	CZoanthropeAI constructor

	{
	}

void CZoanthropeAI::Behavior (void)

//	Behavior

	{
	//	Reset

	ResetBehavior();

	//	Behave according to our state

	switch (m_State)
		{
		case stateNone:
			BehaviorStart();
			break;

		case stateAttackingOnPatrol:
			{
			ASSERT(m_pTarget);
			ImplementCombatManeuvers(m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	See if we're out of our zone

			Metric rRange = LIGHT_SECOND * GetCurrentOrderData();
			if (CheckOutOfZone(GetCurrentOrderTarget(),
					rRange - PATROL_SENSOR_RANGE,
					rRange + PATROL_SENSOR_RANGE,
					30))
				SetState(stateNone);
			break;
			}

		case stateAttackingThreat:
			{
			ASSERT(m_pTarget);
			ImplementCombatManeuvers(m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Every once in a while check to see if we've wandered too far from
			//	our base.

			if (CheckOutOfRange(GetCurrentOrderTarget(), PATROL_SENSOR_RANGE, 20))
				SetState(stateNone);
			break;
			}

		case stateOnCourseForStargate:
			{
			m_AICtx.ImplementGating(m_pShip, m_pBase);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
			break;
			}

		case stateOnEscortCourse:
			{
			ASSERT(m_pBase);

			CVector vFlockPos;
			CVector vFlockVel;
			int iFlockFacing;
			if (m_AICtx.CalcFlockingFormation(m_pShip, m_pBase, MAX_FLOCK_DIST, SEPARATION_RANGE, &vFlockPos, &vFlockVel, &iFlockFacing))
				{
				m_AICtx.ImplementFormationManeuver(m_pShip, vFlockPos, vFlockVel, m_pShip->AlignToRotationAngle(iFlockFacing));
				}
			else
				{
				CVector vTarget = m_pBase->GetPos() - m_pShip->GetPos();
				Metric rTargetDist2 = vTarget.Dot(vTarget);

				m_AICtx.ImplementCloseOnTarget(m_pShip, m_pBase, vTarget, rTargetDist2);
				m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetMaxWeaponRange(), &m_pTarget);
				m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
				}

			//	See if there is anything to attack

			CSpaceObject *pTarget;
			if (CheckForEnemiesInRange(m_pShip, PATROL_SENSOR_RANGE, 30, &pTarget))
				SetState(stateAttackingThreat, m_pBase, pTarget);

			break;
			}

		case stateOnPatrolOrbit:
			{
			ASSERT(m_pBase);

			CVector vFlockPos;
			CVector vFlockVel;
			int iFlockFacing;
			if (m_AICtx.CalcFlockingFormation(m_pShip, NULL, MAX_FLOCK_DIST, SEPARATION_RANGE, &vFlockPos, &vFlockVel, &iFlockFacing))
				{
				m_AICtx.ImplementFormationManeuver(m_pShip, vFlockPos, vFlockVel, m_pShip->AlignToRotationAngle(iFlockFacing));
				}
			else
				{
				m_AICtx.ImplementOrbit(m_pShip, m_pBase, LIGHT_SECOND * GetCurrentOrderData());
				m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetMaxWeaponRange(), &m_pTarget);
				m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
				}

			//	See if there is anything to attack

			CSpaceObject *pTarget;
			if (CheckForEnemiesInRange(m_pShip, PATROL_SENSOR_RANGE, 30, &pTarget))
				SetState(stateAttackingOnPatrol, m_pBase, pTarget);
			break;
			}

		case stateReturningFromThreat:
			{
			ASSERT(m_pBase);
			m_AICtx.ImplementDocking(m_pShip, m_pBase);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetMaxWeaponRange(), &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);

			//	See if there is anything to attack

			CSpaceObject *pTarget;
			if (CheckForEnemiesInRange(m_pBase, PATROL_SENSOR_RANGE, 30, &pTarget))
				SetState(stateAttackingThreat, m_pBase, pTarget);
			break;
			}

		case stateWaiting:
			m_AICtx.ImplementStop(m_pShip);

			if (m_pTarget)
				m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget, true);

			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetMaxWeaponRange(), &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
			break;

		case stateWaitingForThreat:
			{
			ASSERT(m_pBase);
			CSpaceObject *pTarget;
			if (CheckForEnemiesInRange(m_pBase, PATROL_SENSOR_RANGE, 30, &pTarget))
				SetState(stateAttackingThreat, m_pBase, pTarget);
			break;
			}
		}
	}

void CZoanthropeAI::BehaviorStart (void)

//	BehaviorStart
//
//	Figure out what to do based on orders

	{
	switch (GetCurrentOrder())
		{
		case IShipController::orderNone:
			{
			if (m_pShip->GetDockedObj() == NULL)
				AddOrder(IShipController::orderGate, NULL, IShipController::SData());
			break;
			}

		case IShipController::orderEscort:
			{
			CSpaceObject *pPrincipal = GetCurrentOrderTarget();
			ASSERT(pPrincipal);

			SetState(stateOnEscortCourse, pPrincipal);
			m_pShip->Communicate(m_pBase, msgEscortReportingIn, m_pShip);
			break;
			}

		case IShipController::orderFollowPlayerThroughGate:
			SetState(stateOnCourseForStargate, m_pShip->GetNearestStargate());
			break;

		case IShipController::orderGate:
			{
			//	Look for the gate

			CSpaceObject *pGate = GetCurrentOrderTarget();
			if (pGate == NULL)
				pGate = m_pShip->GetNearestStargate(true);

			//	Head for the gate

			if (pGate)
				SetState(stateOnCourseForStargate, pGate);

			break;
			}

		case IShipController::orderGuard:
			{
			CSpaceObject *pPrincipal = GetCurrentOrderTarget();
			ASSERT(pPrincipal);

			//	If we're not docked, dock with principal

			if (m_pShip->GetDockedObj() == NULL)
				SetState(stateReturningFromThreat, pPrincipal);

			//	Otherwise, wait for a threat

			else
				SetState(stateWaitingForThreat, pPrincipal);

			break;
			}

		case IShipController::orderPatrol:
			{
			CSpaceObject *pPrincipal = GetCurrentOrderTarget();
			ASSERT(pPrincipal);

			SetState(stateOnPatrolOrbit, pPrincipal);
			break;
			}

		case IShipController::orderWaitForPlayer:
			{
			SetState(stateWaiting);
			break;
			}
		}
	}

CString CZoanthropeAI::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Returns debug crash info

	{
	CString sOrder;
	try
		{
		sOrder = strFromInt((int)GetCurrentOrder());
		}
	catch (...)
		{
		}

	//	If GetCurrentOrder crashes, try to determine why

	if (sOrder.IsBlank())
		{
		try
			{
			sOrder = strPatternSubst(CONSTLIT("crash in GetCurrentOrder; count = %d\r\n"), m_Orders.GetCount());
			}
		catch (...)
			{
			sOrder = CONSTLIT("crash in GetCurrentOrder\r\n");
			}
		}

	CString sResult = CONSTLIT("CZoanthropeAI\r\n");
	sResult.Append(strPatternSubst(CONSTLIT("Order: %s\r\n"), sOrder));
	sResult.Append(strPatternSubst(CONSTLIT("m_State: %d\r\n"), m_State));
	sResult.Append(strPatternSubst(CONSTLIT("m_pBase: %s\r\n"), CSpaceObject::DebugDescribe(m_pBase)));
	sResult.Append(strPatternSubst(CONSTLIT("m_pTarget: %s\r\n"), CSpaceObject::DebugDescribe(m_pTarget)));

	return sResult;
	}

CSpaceObject *CZoanthropeAI::GetBase (void) const

//	GetBase
//
//	Returns this ship's base

	{
	switch (GetCurrentOrder())
		{
		case IShipController::orderGuard:
		case IShipController::orderPatrol:
			return GetCurrentOrderTarget();

		default:
			return NULL;
		}
	}

CSpaceObject *CZoanthropeAI::GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget) const

//	GetTarget
//
//	Returns the target that this ship is attacking
	
	{
	switch (m_State)
		{
		case stateNone:
		case stateWaitingForThreat:
			return NULL;

		default:
			return m_pTarget;
		}
	}

void CZoanthropeAI::ImplementCombatManeuvers (CSpaceObject *pTarget)

//	ImplementCombatManeuvers
//
//	Move and fire

	{
	CVector vTarget = pTarget->GetPos() - m_pShip->GetPos();
	Metric rTargetDist2 = vTarget.Dot(vTarget);

	//	Fire on the target as best we can (if iFireDir is not -1, then it
	//	is the angle that we should turn towards to be able to fire).

	int iFireDir;
	m_AICtx.ImplementFireWeaponOnTarget(m_pShip, -1, -1, pTarget, vTarget, rTargetDist2, &iFireDir);

	//	Compute our flocking position

	CVector vFlockPos;
	CVector vFlockVel;
	int iFlockFacing;

	if (rTargetDist2 > COMBAT_RANGE2
			&& m_AICtx.CalcFlockingFormation(m_pShip, NULL, MAX_FLOCK_DIST, SEPARATION_RANGE, &vFlockPos, &vFlockVel, &iFlockFacing))
		{
		m_AICtx.ImplementFormationManeuver(m_pShip, vFlockPos, vFlockVel, m_pShip->AlignToRotationAngle(iFlockFacing));
		}

	//	Otherwise, if we are the leader, then fight

	else
		{
		//	If we're not well in range of our primary weapon then
		//	get closer to the target. (Or if we are not moving)

		if (rTargetDist2 > m_AICtx.GetPrimaryAimRange2())
			{
			//	Try to flank our target, if we are faster

			bool bFlank = (m_pShip->GetMaxSpeed() > pTarget->GetMaxSpeed());
			m_AICtx.ImplementCloseOnTarget(m_pShip, pTarget, vTarget, rTargetDist2, bFlank);

			iFireDir = -1;
			}
		else if (pTarget->CanMove()
				&& (m_pShip->GetVel().Length2() < (0.01 * 0.01 * LIGHT_SPEED * LIGHT_SPEED)))
			{
			m_AICtx.ImplementSpiralOut(m_pShip, vTarget);
			iFireDir = -1;
			}

		//	If necessary, turn to aim our weapon based on the fire solution

		if (iFireDir != -1
				&& !m_AICtx.NoDogfights())
			m_AICtx.ImplementManeuver(m_pShip, iFireDir, false);
		}
	}

void CZoanthropeAI::OnAttacked (CSpaceObject *pAttacker, const DamageDesc &Damage)

//	OnAttacked
//
//	Our ship was attacked. Note that pAttacker can either be an enemy ship/station
//	or it can be the missile/beam that hit us.

	{
	DEBUG_TRY

	if (pAttacker)
		{
		//	If we were attacked by a friend, then warn them off
		//	(Unless we're explicitly targeting the friend)

		if (m_pShip->IsFriend(pAttacker) && pAttacker != m_pTarget)
			{
			//	Note: In this case, we include automated attacks, since
			//	zoanthropes can't tell the difference.

			m_pShip->Communicate(pAttacker, msgWatchTargets);
			}

		//	Else if we were attacked by an enemy/neutral, see if we need
		//	to attack them (or flee)

		else if (pAttacker->CanAttack())
			{
			//	Change state to deal with the attack

			switch (m_State)
				{
				case stateReturningFromThreat:
				case stateWaitingForThreat:
					{
					SetState(stateAttackingThreat, m_pBase, pAttacker);
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					break;
					}

				case stateOnEscortCourse:
					{
					SetState(stateAttackingThreat, m_pBase, pAttacker);
					m_pShip->Communicate(m_pBase, msgEscortAttacked, pAttacker);
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					break;
					}

				case stateOnPatrolOrbit:
					{
					SetState(stateAttackingOnPatrol, m_pBase, pAttacker);
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					break;
					}
				}
			}
		}

	//	Remember the last time we were attacked (debounce quick hits)

	m_AICtx.SetLastAttack(m_pShip->GetSystem()->GetTick());

	DEBUG_CATCH
	}

DWORD CZoanthropeAI::OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	Communicate
//
//	Handle communications from other objects

	{
	switch (iMessage)
		{
		case msgAttack:
		case msgAttackDeter:
			{
			switch (m_State)
				{
				case stateOnEscortCourse:
				case stateReturningFromThreat:
				case stateWaitingForThreat:
					SetState(stateAttackingThreat, m_pBase, pParam1);
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					return resAck;

				case stateOnPatrolOrbit:
					SetState(stateAttackingOnPatrol, m_pBase, pParam1);
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					return resAck;

				default:
					return resNoAnswer;
				}
			}

		case msgQueryEscortStatus:
			{
			if (GetEscortPrincipal() == pParam1)
				return resAck;
			else
				return resNoAnswer;
			}

		default:
			return resNoAnswer;
		}
	}

void CZoanthropeAI::OnDockedEvent (CSpaceObject *pObj)

//	OnDockedEvent
//
//	Event when the ship is docked

	{
	SetState(stateNone);
	}

void CZoanthropeAI::OnObjDestroyedNotify (const SDestroyCtx &Ctx)

//	OnObjDestroyedNotify
//
//	Deal with an object that has been destroyed

	{
	//	Reset

	if (m_pBase == Ctx.pObj)
		SetState(stateNone);

	if (m_pTarget == Ctx.pObj)
		SetState(stateNone);
	}

void CZoanthropeAI::OnOrderChanged (void)

//	OnOrderChanged
//
//	The order list has changed

	{
	SetState(stateNone);
	}

void CZoanthropeAI::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read our data
//
//	DWORD		m_State
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pBase (CSpaceObject ref)
//	Vector		m_vPotential

	{
	Ctx.pStream->Read((char *)&m_State, sizeof(DWORD));
	CSystem::ReadObjRefFromStream(Ctx, &m_pTarget);
	CSystem::ReadObjRefFromStream(Ctx, &m_pBase);
	Ctx.pStream->Read((char *)&m_vPotential, sizeof(m_vPotential));
	}

void CZoanthropeAI::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write our data
//
//	DWORD		m_State
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pBase (CSpaceObject ref)
//	Vector		m_vPotential

	{
	pStream->Write((char *)&m_State, sizeof(DWORD));
	m_pShip->WriteObjRefToStream(m_pTarget, pStream);
	m_pShip->WriteObjRefToStream(m_pBase, pStream);
	pStream->Write((char *)&m_vPotential, sizeof(m_vPotential));
	}

void CZoanthropeAI::SetState (StateTypes State, CSpaceObject *pBase, CSpaceObject *pTarget)

//	SetState
//
//	Sets the current state

	{
	//	If docking has been requested and we're trying to change state
	//	then cancel docking.

	if (IsDockingRequested())
		CancelDocking(m_pBase);

	//	Set state (NOTE: We do this before we undock because the Undock
	//	call may destroy the station and cause us to recurse into SetState.
	//	This happens when a ship is guarding an empty cargo crate).

	m_State = State;
	m_pBase = pBase;
	m_pTarget = pTarget;

	//	If we're currently docked and we're changing to a state that
	//	does not support docking, then we undock first.

	if (m_pShip->GetDockedObj()
			&& State != stateNone 
			&& State != stateWaitingForThreat)
		m_pShip->Undock();
	}
