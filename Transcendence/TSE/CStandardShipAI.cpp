//	CStandardShipAI.cpp
//
//	CStandardShipAI class
//
//	Depending on the current order, the implementation follows 
//	a specific state machine. For example:
//
//	orderGuard
//
//		stateNone:
//			IF docked -> stateWaitingForThreat
//			ELSE -> stateReturningFromThreat
//
//		stateReturningFromThreat:
//			IF enemies appear -> stateAttackingThreat
//			IF docked -> stateNone
//
//		stateWaitingForThreat:
//			IF enemies appear -> stateAttackingThreat
//
//		stateAttackingThreat:
//			IF enemy destroyed -> stateNone
//			IF too far -> stateReturningFromThreat
//
//	ORDERS AND STATES
//
//		orderNone
//			stateNone
//
//		orderGuard
//			stateWaitingForThreat
//			stateReturningFromThreat		oT
//			stateAttackingThreat			oT
//
//		orderDock
//			stateOnCourseForDocking
//
//		orderDestroyTarget
//			stateAttackingTarget
//
//		orderWait
//			stateWaiting
//
//		orderGate
//			stateOnCourseForStargate
//
//		orderGateOnThreat
//			stateWaitingUnarmed
//
//		orderGateOnStationDestroyed
//			stateNone
//
//		orderPatrol
//			stateOnPatrolOrbit				oT
//			stateAttackingOnPatrol			oT
//
//		orderEscort
//			stateEscorting					oT
//			stateAttackingThreat			oT
//
//		orderScavenge
//			stateLookingForLoot
//			stateLooting
//			stateDeterTarget
//
//		orderFollowPlayerThroughGate
//			stateOnCourseForStargate
//
//		orderAttackNearestEnemy
//
//		orderTradeRoute
//			stateOnCourseForDocking
//			stateOnCourseForStargate

#include "PreComp.h"


#ifdef DEBUG
//#define DEBUG_SHIP
#endif

#ifdef DEBUG_COMBAT
#define DEBUG_COMBAT_OUTPUT(x)			if (bDebug) g_pUniverse->DebugOutput("%d> %s", g_iDebugLine++, x)
#else
#define DEBUG_COMBAT_OUTPUT(x)
#endif

const int MULTI_HIT_WINDOW =			20;
const int ATTACK_TIME_THRESHOLD =		150;

const Metric ATTACK_RANGE =				(20.0 * LIGHT_SECOND);
const Metric PATROL_SENSOR_RANGE =		(30.0 * LIGHT_SECOND);
const Metric SCAVENGE_SENSOR_RANGE =	(10.0 * LIGHT_MINUTE);
const Metric THREAT_SENSOR_RANGE =		(10.0 * LIGHT_SECOND);
const Metric THREAT_SENSOR_RANGE2 =		(THREAT_SENSOR_RANGE * THREAT_SENSOR_RANGE);
const Metric WANDER_SAFETY_RANGE =		(20.0 * LIGHT_SECOND);
const Metric WANDER_SAFETY_RANGE2 =		(WANDER_SAFETY_RANGE * WANDER_SAFETY_RANGE);
const Metric NAV_PATH_THRESHOLD =		(4.0 * PATROL_SENSOR_RANGE);
const Metric NAV_PATH_THRESHOLD2 =		(NAV_PATH_THRESHOLD * NAV_PATH_THRESHOLD);
const Metric GOTO_THRESHOLD =			(LIGHT_SECOND / 10.0);
const Metric GOTO_THRESHOLD2 =			(GOTO_THRESHOLD * GOTO_THRESHOLD);
const Metric GOTO_SLOW_RANGE =			(5 * LIGHT_SECOND);
const Metric GOTO_SLOW_RANGE2 =			(GOTO_SLOW_RANGE * GOTO_SLOW_RANGE);
const Metric SAFE_ORBIT_RANGE =			(30.0 * LIGHT_SECOND);
const Metric LONG_THREAT_RANGE =		(120 * LIGHT_SECOND);
const Metric LONG_THREAT_RANGE2 =		(LONG_THREAT_RANGE * LONG_THREAT_RANGE);

#define SPACE_OBJ_SCAVENGE_DATA			CONSTLIT("$scavenge")

static CObjectClass<CStandardShipAI>g_Class(OBJID_CSTANDARDSHIPAI, NULL);

static CShip *g_pDebugShip = NULL;
int g_iDebugLine = 0;

bool EnemyStationsAtBearing (CSpaceObject *pShip, int iBearing, Metric rRange);

CStandardShipAI::CStandardShipAI (void) : CBaseShipAI(&g_Class),
		m_State(stateNone),
		m_pTarget(NULL),
		m_pDest(NULL),
		m_fHasEscorts(false)

//	CStandardShipAI constructor

	{
	}

CStandardShipAI::~CStandardShipAI (void)

//	CStandardShipAI destructor

	{
	}

void CStandardShipAI::OnBehavior (void)

//	Behavior
//
//	Fly, fight, die

	{
#ifdef DEBUG
	bool bDebug = m_pShip->IsSelected();
#endif

	//	Behave according to our state

	switch (m_State)
		{
		case stateNone:
			BehaviorStart();
			break;

		case stateApproaching:
			{
			ASSERT(m_pDest);
			CVector vTarget = m_pDest->GetPos() - m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);

			m_AICtx.ImplementCloseOnTarget(m_pShip, m_pDest, vTarget, rTargetDist2);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetBestWeaponRange(), &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			if (vTarget.Length() < m_rDistance)
				CancelCurrentOrder();

			break;
			}

		case stateAttackingOnPatrol:
			{
			ASSERT(m_pTarget);
			m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Check to see if we've wandered outside our patrol zone

			if (m_pShip->IsDestinyTime(20))
				{
				CSpaceObject *pCenter = GetCurrentOrderTarget();
				int iDistance = GetCurrentOrderData();
				Metric rMaxRange2 = (LIGHT_SECOND * iDistance) + PATROL_SENSOR_RANGE;
				rMaxRange2 = rMaxRange2 * rMaxRange2;
				Metric rMinRange2 = max(0.0, (LIGHT_SECOND * iDistance) - PATROL_SENSOR_RANGE);
				rMinRange2 = rMinRange2 * rMinRange2;

				int iTick = m_pShip->GetSystem()->GetTick();
				CVector vRange = pCenter->GetPos() - m_pShip->GetPos();
				Metric rDistance2 = vRange.Dot(vRange);

				//	If we're outside of our patrol range and if we haven't
				//	been hit in a while then stop the attack

				if ((rDistance2 > rMaxRange2 || rDistance2 < rMinRange2)
						&& (iTick - m_AICtx.GetLastAttack()) > ATTACK_TIME_THRESHOLD)
					{
					SetState(stateNone);
					DEBUG_COMBAT_OUTPUT("Patrol: End attack");
					}
				}

			break;
			}

		case stateAimingAtTarget:
			{
			ASSERT(m_pTarget);
			m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget, false, true);
			break;
			}

		case stateAttackingTarget:
			{
			ASSERT(m_pTarget);
			m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);
			break;
			}

		case stateAttackingTargetAndAvoiding:
			{
			ASSERT(m_pTarget);
			m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Every once in a while check to see if we've wandered near
			//	an enemy station

			if (m_pShip->IsDestinyTime(41) && !IsImmobile() && m_pTarget->CanMove())
				{
				CSpaceObject *pEnemy = m_pShip->GetNearestEnemyStation(WANDER_SAFETY_RANGE);
				if (pEnemy 
						&& pEnemy != m_pTarget
						&& m_pTarget->GetDistance2(pEnemy) < WANDER_SAFETY_RANGE2)
					{
					SetState(stateAvoidingEnemyStation);
					m_pDest = pEnemy;
					m_pTarget = GetCurrentOrderTarget();
					}
				}

			//	See if our timer has expired

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				CancelCurrentOrder();

			break;
			}

		case stateAttackingTargetAndHolding:
			{
			ASSERT(m_pTarget);
			m_AICtx.ImplementHold(m_pShip);
			m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget, true);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	See if our timer has expired

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				CancelCurrentOrder();

			break;
			}

		case stateAvoidingEnemyStation:
			{
			ASSERT(m_pDest);
			ASSERT(m_pTarget);

			int iTick = m_pShip->GetSystem()->GetTick();
			CVector vTarget = m_pTarget->GetPos() - m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Length2();
			CVector vDest = m_pDest->GetPos() - m_pShip->GetPos();

			//	We only spiral in/out part of the time (we leave ourselves some time to fight)

			bool bAvoid = (rTargetDist2 > THREAT_SENSOR_RANGE2) || ((iTick + m_pShip->GetDestiny()) % 91) > 55;
			
			if (!bAvoid)
				{
				//	Attack target

				m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget, true);
				m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);
				}
			else
				{
				//	Orbit around the enemy station

				Metric rDestDist2 = vDest.Length2();

				const Metric rMaxDist = SAFE_ORBIT_RANGE * 1.2;
				const Metric rMinDist = SAFE_ORBIT_RANGE * 0.9;

				if (rDestDist2 > (rMaxDist * rMaxDist))
					m_AICtx.ImplementSpiralIn(m_pShip, vDest);
				else if (rDestDist2 < (rMinDist * rMinDist))
					m_AICtx.ImplementSpiralOut(m_pShip, vDest);
				else
					{
					m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget, true);
					m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);
					}
				}

			//	If the target has left the safety of the station, 
			//	and the station is not between the ship and the target, then
			//	we stop avoiding.

			if (m_pShip->IsDestinyTime(23)
					&& rTargetDist2 > WANDER_SAFETY_RANGE2
					&& Absolute(AngleBearing(VectorToPolar(vTarget), VectorToPolar(vDest))) > 45)
				{
				//	Note: We don't set stateNone because we want to preserve the timer value

				SetState(stateAttackingTargetAndAvoiding);
				m_pTarget = GetCurrentOrderTarget();
				}

			//	See if our timer has expired

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				CancelCurrentOrder();

			break;
			}

		case stateAttackingPrincipalThreat:
			{
			ASSERT(m_pTarget);
			m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Every once in a while check to see if we've wandered too far from
			//	our base.

			if (m_pShip->IsDestinyTime(20))
				{
				CSpaceObject *pPrincipal = GetCurrentOrderTarget();
				int iTick = m_pShip->GetSystem()->GetTick();
				CVector vRange = pPrincipal->GetPos() - m_pShip->GetPos();
				Metric rDistance2 = vRange.Dot(vRange);

				//	If we're outside of our patrol range and if we haven't
				//	been hit in a while then stop the attack.

				if (rDistance2 > LONG_THREAT_RANGE2)
					SetState(stateNone);
				}

			break;
			}

		case stateAttackingThreat:
			{
			ASSERT(m_pTarget);
			m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Every once in a while check to see if we've wandered too far from
			//	our base.

			if (m_pShip->IsDestinyTime(20))
				{
				CSpaceObject *pPrincipal = GetCurrentOrderTarget();
				int iTick = m_pShip->GetSystem()->GetTick();
				CVector vRange = pPrincipal->GetPos() - m_pShip->GetPos();
				Metric rDistance2 = vRange.Dot(vRange);

				//	If we're outside of our patrol range and if we haven't
				//	been hit in a while then stop the attack.

				if (rDistance2 > (PATROL_SENSOR_RANGE * PATROL_SENSOR_RANGE)
						&& (iTick - m_AICtx.GetLastAttack()) > ATTACK_TIME_THRESHOLD)
					SetState(stateNone);
				}

			break;
			}

		case stateBombarding:
			{
			bool bInPlace;
			m_AICtx.ImplementHold(m_pShip, &bInPlace);
			if (bInPlace)
				m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget);

			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	See if we're done bombarding

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				{
				if (GetCurrentOrder() == IShipController::orderBombard)
					CancelCurrentOrder();

				SetState(stateNone);
				}
			break;
			}

		case stateDeterTarget:
			{
			ASSERT(m_pTarget);
			m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Check to see if target has hit back. If not, stop the attack

			if (m_pShip->IsDestinyTime(20)
					&& (m_pShip->GetSystem()->GetTick() - m_AICtx.GetLastAttack()) > 6 * ATTACK_TIME_THRESHOLD)
				SetState(stateNone);

			break;
			}

		case stateDeterTargetNoChase:
			{
			ASSERT(m_pTarget);
			bool bInPlace;
			m_AICtx.ImplementHold(m_pShip, &bInPlace);
			if (bInPlace && !m_AICtx.NoAttackOnThreat())
				m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget, true);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Check to see if target has hit back. If not, stop the attack

			if (m_pShip->IsDestinyTime(20)
					&& (m_pShip->GetSystem()->GetTick() - m_AICtx.GetLastAttack()) > 3 * ATTACK_TIME_THRESHOLD)
				SetState(stateNone);

			//	See if we're done holding

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				{
				if (GetCurrentOrder() == IShipController::orderHold)
					CancelCurrentOrder();

				SetState(stateNone);
				}

			break;
			}

		case stateDeterTargetWhileLootDocking:
			{
			ASSERT(m_pTarget);

			CVector vDest = m_pDest->GetPos() - m_pShip->GetPos();
			Metric rDestDist2 = vDest.Dot(vDest);
			m_AICtx.ImplementCloseOnImmobileTarget(m_pShip, m_pDest, vDest, rDestDist2);

			m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget, true);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Check to see if target has hit back. If not, stop the attack

			if (m_pShip->IsDestinyTime(20)
					&& (m_pShip->GetSystem()->GetTick() - m_AICtx.GetLastAttack()) > 3 * ATTACK_TIME_THRESHOLD)
				{
				CSpaceObject *pDest = m_pDest;
				SetState(stateOnCourseForLootDocking);
				m_pDest = pDest;
				}

			break;
			}

		case stateEscorting:
			{
			m_AICtx.ImplementEscort(m_pShip, m_pDest, &m_pTarget);

			//	Check to see if there are enemy ships that we need to attack

			if (m_pShip->IsDestinyTime(30) && !IsWaitingForShieldsToRegen())
				{
				CSpaceObject *pPrincipal = GetCurrentOrderTarget();
				CSpaceObject *pTarget = CalcEnemyShipInRange(pPrincipal, PATROL_SENSOR_RANGE);
				if (pTarget)
					{
					SetState(stateAttackingThreat);
					m_pTarget = pTarget;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					}
				}

			break;
			}

		case stateReturningToEscort:
			{
			CVector vTarget = m_pDest->GetPos() - m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);

			m_AICtx.ImplementCloseOnTarget(m_pShip, m_pDest, vTarget, rTargetDist2);

			//	If we are in range of the ship we are escorting, then we can stop

			if (m_pShip->IsDestinyTime(30) && rTargetDist2 < THREAT_SENSOR_RANGE2)
				SetState(stateNone);

			break;
			}

		case stateFollowing:
			{
			m_AICtx.ImplementEscort(m_pShip, m_pDest, &m_pTarget);
			break;
			}

		case stateFollowNavPath:
			{
			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetBestWeaponRange(), &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			bool bAtDest;
			m_AICtx.ImplementFollowNavPath(m_pShip, &bAtDest);
			if (bAtDest)
				{
				m_AICtx.ClearNavPath();
				CancelCurrentOrder();
				}

			break;
			}

		case stateHolding:
			{
			m_AICtx.ImplementHold(m_pShip);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Check to see if there are enemy ships that we need to attack

			if (m_pShip->IsDestinyTime(30)
					&& !m_AICtx.NoAttackOnThreat())
				{
				CSpaceObject *pTarget = CalcEnemyShipInRange(m_pShip, PATROL_SENSOR_RANGE);
				if (pTarget)
					{
					SetState(stateDeterTargetNoChase);
					m_pTarget = pTarget;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					}
				}

			//	See if we're done holding

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				{
				if (GetCurrentOrder() == IShipController::orderHold)
					CancelCurrentOrder();

				SetState(stateNone);
				}
			break;
			}

		case stateHoldingCourse:
			{
			m_AICtx.ImplementManeuver(m_pShip, m_iCountdown, true, true);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	If we've reached our distance, then we're done

			if (m_rDistance > 0.0 
					&& (m_pShip->GetPos() - m_AICtx.GetPotential()).Length2() > m_rDistance)
				{
				CancelCurrentOrder();
				break;
				}

			break;
			}

		case stateLookingForLoot:
			{
			int i;

			//	Orbit around the center of the system

			CVector vTarget = -m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);

			Metric rMaxDist = m_rDistance * 1.1;
			Metric rMinDist = m_rDistance * 0.9;

			if (rTargetDist2 > (rMaxDist * rMaxDist))
				m_AICtx.ImplementSpiralIn(m_pShip, vTarget);
			else if (rTargetDist2 < (rMinDist * rMinDist))
				m_AICtx.ImplementSpiralOut(m_pShip, vTarget);

			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Look for loot

			if (m_pShip->IsDestinyTime(40))
				{
				Metric rBestDist2 = SCAVENGE_SENSOR_RANGE * SCAVENGE_SENSOR_RANGE;
				CSpaceObject *pBestScrap = NULL;
				for (i = 0; i < m_pShip->GetSystem()->GetObjectCount(); i++)
					{
					CSpaceObject *pObj = m_pShip->GetSystem()->GetObject(i);

					if (pObj 
							&& pObj->SupportsDocking()
							&& pObj->IsAbandoned()
							&& !pObj->IsRadioactive()
							&& !pObj->IsDestroyed()
							&& !pObj->IsInactive()
							&& !pObj->IsVirtual()
							&& pObj->GetData(SPACE_OBJ_SCAVENGE_DATA).IsBlank())
						{
						CVector vRange = pObj->GetPos() - m_pShip->GetPos();
						Metric rDistance2 = vRange.Dot(vRange);

						if (rDistance2 < rBestDist2)
							{
							rBestDist2 = rDistance2;
							pBestScrap = pObj;
							}
						}
					}

				//	If we've found some scrap then dock with it

				if (pBestScrap)
					{
					SetState(stateOnCourseForLootDocking);
					m_pDest = pBestScrap;
					}

				//	Otherwise, there is a small chance that we change orbits

				else if (mathRandom(1, 100) == 1)
					{
					m_rDistance = LIGHT_SECOND * 4 * (mathRandom(30, 120) + mathRandom(30, 120));
					}
				}

			break;
			}

		case stateLooting:
		case stateLootingOnce:
			{
			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			CSpaceObject *pDock = m_pShip->GetDockedObj();
			if (pDock)
				{
				//	See if it is time to take the loot and leave

				if (m_pShip->IsDestinyTime(300))
					{
					//	Take the loot

					bool bGotLoot = false;
					if (pDock->IsAbandoned())
						{
						CItemListManipulator Source(pDock->GetItemList());

						Source.MoveCursorForward();
						while (Source.IsCursorValid())
							{
							Source.TransferAtCursor(Source.GetItemAtCursor().GetCount(),
									m_pShip->GetItemList());
							}

						//	Tell the object that we've modified items in case
						//	the player ship is docked with the same station.

						pDock->OnComponentChanged(comCargo);
						pDock->ItemsModified();
						pDock->InvalidateItemListAddRemove();
						m_pShip->InvalidateItemListAddRemove();
						bGotLoot = true;
						}

					//	Deal with recurring looting

					if (m_State == stateLooting)
						{
						//	Mark so we don't loot this object again

						pDock->SetData(SPACE_OBJ_SCAVENGE_DATA, CONSTLIT("l"));

						//	Undock

						m_pShip->Undock();

						//	If we got some loot, see if we can upgrade devices

						if (bGotLoot)
							{
							UpgradeWeaponBehavior();
							UpgradeShieldBehavior();
							}

						//	Continue looking for loot

						SetState(stateLookingForLoot);
						m_rDistance = LIGHT_SECOND * 4 * (mathRandom(30, 120) + mathRandom(30, 120));
						}

					//	Otherwise, we're done

					else
						{
						CancelCurrentOrder();
						}
					}
				}
			else
				SetState(stateNone);

			break;
			}

		case stateMaintainBearing:
			{
			m_AICtx.ImplementManeuver(m_pShip, (int)m_rDistance, true);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Every so often, check to see if there are enemy stations ahead

			if (m_pShip->IsDestinyTime(10))
				{
				if (--m_iCountdown <= 0
						|| EnemyStationsAtBearing(m_pShip, (int)m_rDistance, WANDER_SAFETY_RANGE))
					SetState(stateNone);
				}

			break;
			}

		case stateOnCourseForDocking:
			{
			ASSERT(m_pDest);
			m_AICtx.ImplementDocking(m_pShip, m_pDest);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetBestWeaponRange(), &m_pTarget, m_pDest);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget, m_pDest);
			break;
			}

		case stateOnCourseForDockingViaNavPath:
			{
			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetBestWeaponRange(), &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			bool bAtDest;
			m_AICtx.ImplementFollowNavPath(m_pShip, &bAtDest);
			if (bAtDest)
				{
				m_AICtx.ClearNavPath();
				SetState(stateOnCourseForDocking);
				m_pDest = GetCurrentOrderTarget();
				}

			break;
			}

		case stateOnCourseForLootDocking:
			{
			ASSERT(m_pDest);
			m_AICtx.ImplementDocking(m_pShip, m_pDest);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetBestWeaponRange(), &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);
			break;
			}

		case stateOnCourseForOrbitViaNavPath:
			{
			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetBestWeaponRange(), &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			bool bAtDest;
			m_AICtx.ImplementFollowNavPath(m_pShip, &bAtDest);

			//	We stop navigation when we are inside the patrol radius

			Metric rPatrolRadius = LIGHT_SECOND * LOWORD(GetCurrentOrderData());
			Metric rPatrolRadius2 = rPatrolRadius * rPatrolRadius;
			Metric rDist2 = GetDistance2(GetCurrentOrderTarget());

			if (bAtDest || rDist2 < rPatrolRadius2)
				{
				m_AICtx.ClearNavPath();
				SetState(stateOrbit);
				m_pDest = GetCurrentOrderTarget();
				m_rDistance = rPatrolRadius;
				DWORD dwTimer = HIWORD(GetCurrentOrderData());
				m_iCountdown = (dwTimer > 0 ? (g_TicksPerSecond * dwTimer) : -1);
				}

			break;
			}

		case stateOnCourseForPatrolViaNavPath:
			{
			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetBestWeaponRange(), &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			bool bAtDest;
			m_AICtx.ImplementFollowNavPath(m_pShip, &bAtDest);

			//	We stop navigation when we are inside the patrol radius

			Metric rPatrolRadius = LIGHT_SECOND * GetCurrentOrderData();
			Metric rPatrolRadius2 = rPatrolRadius * rPatrolRadius;
			Metric rDist2 = GetDistance2(GetCurrentOrderTarget());

			if (bAtDest || rDist2 < rPatrolRadius2)
				{
				m_AICtx.ClearNavPath();
				SetState(stateOnPatrolOrbit);
				m_pDest = GetCurrentOrderTarget();
				m_rDistance = rPatrolRadius;
				}

			break;
			}

		case stateOnCourseForPoint:
			{
			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetBestWeaponRange(), &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			bool bInPosition = false;
			m_AICtx.ImplementFormationManeuver(m_pShip, m_pDest->GetPos(), m_pDest->GetVel(), m_pShip->AlignToRotationAngle(m_iCountdown), &bInPosition);
			if (bInPosition)
				CancelCurrentOrder();

			break;
			}

		case stateOnCourseForPointViaNavPath:
			{
			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetBestWeaponRange(), &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			bool bAtDest;
			m_AICtx.ImplementFollowNavPath(m_pShip, &bAtDest);
			if (bAtDest)
				{
				m_AICtx.ClearNavPath();

				if (GetCurrentOrder() == IShipController::orderApproach)
					{
					SetState(stateApproaching);
					m_pDest = GetCurrentOrderTarget();
					m_rDistance = LIGHT_SECOND * GetCurrentOrderData();
					if (m_rDistance < LIGHT_SECOND)
						m_rDistance = LIGHT_SECOND;
					}
				else if (GetCurrentOrder() == IShipController::orderGoTo)
					{
					SetState(stateOnCourseForPoint);
					m_pDest = GetCurrentOrderTarget();
					m_iCountdown = m_pShip->GetRotation();
					}
				else
					{
					SetState(stateNone);
					}
				}

			break;
			}

		case stateOnCourseForStargate:
			{
			m_AICtx.ImplementGating(m_pShip, m_pDest);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetBestWeaponRange(), &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);
			break;
			}

		case stateOnPatrolOrbit:
			{
			ASSERT(m_pDest);
			CVector vTarget = m_pDest->GetPos() - m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);

			Metric rMaxDist = m_rDistance * 1.1;
			Metric rMinDist = m_rDistance * 0.9;

			if (rTargetDist2 > (rMaxDist * rMaxDist))
				m_AICtx.ImplementSpiralIn(m_pShip, vTarget);
			else if (rTargetDist2 < (rMinDist * rMinDist))
				m_AICtx.ImplementSpiralOut(m_pShip, vTarget);

			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetMaxWeaponRange(), &m_pTarget, m_pDest);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget, m_pDest);

			//	Check to see if any enemy ships appear

			if (m_pShip->IsDestinyTime(30))
				{
				DEBUG_TRY
				
				CSpaceObject *pPrincipal = GetCurrentOrderTarget();
				CSpaceObject *pTarget = CalcEnemyShipInRange(pPrincipal, PATROL_SENSOR_RANGE, m_pDest);
				if (pTarget)
					{
					SetState(stateAttackingOnPatrol);
					m_pTarget = pTarget;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					DEBUG_COMBAT_OUTPUT("Patrol: Attack target");
					}

				DEBUG_CATCH
				}

			break;
			}

		case stateOrbit:
			{
			ASSERT(m_pDest);
			m_AICtx.ImplementOrbit(m_pShip, m_pDest, m_rDistance);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetMaxWeaponRange(), &m_pTarget, m_pDest);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget, m_pDest);

			//	See if our timer has expired

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				CancelCurrentOrder();
			break;
			}

		case stateReturningFromThreat:
			{
			ASSERT(m_pDest);
			m_AICtx.ImplementDocking(m_pShip, m_pDest);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetMaxWeaponRange(), &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Every once in a while check to see if any enemy ships show up

			if (m_pShip->IsDestinyTime(30))
				{
				CSpaceObject *pPrincipal = GetCurrentOrderTarget();
				CSpaceObject *pTarget = CalcEnemyShipInRange(pPrincipal, PATROL_SENSOR_RANGE);
				if (pTarget)
					{
					SetState(stateAttackingThreat);
					m_pTarget = pTarget;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					}
				}
			break;
			}

		case stateReturningViaNavPath:
			{
			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetBestWeaponRange(), &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			bool bAtDest;
			m_AICtx.ImplementFollowNavPath(m_pShip, &bAtDest);
			if (bAtDest)
				{
				m_AICtx.ClearNavPath();
				SetState(stateReturningFromThreat);
				m_pDest = GetCurrentOrderTarget();
				}

			break;
			}

		case stateTurningTo:
			{
			m_AICtx.ImplementManeuver(m_pShip, m_iCountdown, false, true);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Are we done?

			if (m_pShip->IsPointingTo(m_iCountdown))
				CancelCurrentOrder();
			break;
			}

		case stateWaitForPlayerAtGate:
			m_AICtx.ImplementEscort(m_pShip, m_pDest, &m_pTarget);
			break;

		case stateWaiting:
			{
			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				{
				if (GetCurrentOrder() == IShipController::orderWait)
					CancelCurrentOrder();

				SetState(stateNone);
				}
			break;
			}

		case stateWaitingForEnemy:
			{
			m_AICtx.ImplementHold(m_pShip);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Check to see if the target that we're looking for is in sight

			if (m_pShip->IsDestinyTime(17)
					&& m_pShip->GetNearestVisibleEnemy())
				CancelCurrentOrder();

			//	See if we're done waiting

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				CancelCurrentOrder();

			break;
			}

		case stateWaitingForTarget:
			{
			m_AICtx.ImplementHold(m_pShip);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Check to see if the target that we're looking for is in sight

			if (m_pShip->IsDestinyTime(17))
				{
				Metric rDetectRange = m_pDest->GetDetectionRange(m_pShip->GetPerception());
				if (m_rDistance > 0.0)
					rDetectRange = Min(m_rDistance, rDetectRange);

				Metric rDetectRange2 = rDetectRange * rDetectRange;

				Metric rRange2 = (m_pDest->GetPos() - m_pShip->GetPos()).Length2();
				if (rRange2 < rDetectRange2)
					CancelCurrentOrder();
				}

			//	See if we're done waiting

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				CancelCurrentOrder();

			break;
			}

		case stateWaitingForThreat:
			{
			if (m_pShip->IsDestinyTime(30))
				{
				ASSERT(m_pDest);
				CSpaceObject *pTarget = CalcEnemyShipInRange(m_pDest, PATROL_SENSOR_RANGE);

				//	If there are enemy ships in range, then attack them.

				if (pTarget)
					{
					SetState(stateAttackingThreat);
					m_pTarget = pTarget;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					}
				}

			break;
			}

		case stateWaitingUnarmed:
			{
			//	Every once in a while check to see if there are enemy ships
			//	near-by. If so, then gate out.

			if (m_pShip->IsDestinyTime(30))
				{
				CSpaceObject *pPlayer = m_pShip->GetPlayer();
				CSpaceObject *pDock = m_pShip->GetDockedObj();

				//	(We treat the player as a special case because sometimes the
				//	player has been blacklisted)

				if (m_pShip->IsEnemyInRange(THREAT_SENSOR_RANGE)
						|| (pDock && pPlayer && pDock->IsAngryAt(pPlayer) && pDock->GetDistance(pPlayer) <= THREAT_SENSOR_RANGE))
					{
					CancelCurrentOrder();
					AddOrder(IShipController::orderGate, NULL, IShipController::SData());
					}
				}

			break;
			}

		default:
			ASSERT(false);
		}
	}

void CStandardShipAI::BehaviorStart (void)

//	BehaviorStart
//
//	Figure out what we need to do to achieve our current orders

	{
	switch (GetCurrentOrder())
		{
		case IShipController::orderAimAtTarget:
			{
			SetState(stateAimingAtTarget);
			m_pTarget = GetCurrentOrderTarget();
			ASSERT(m_pTarget);
			ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
			break;
			}

		case IShipController::orderAttackNearestEnemy:
			{
			//	Look for the nearest target (if we can't find one, then
			//	just gate out).

			CSpaceObject *pTarget = m_pShip->GetNearestVisibleEnemy();
			if (pTarget)
				{
				//	Note: We don't switch to stateAttackingTargetAndAvoiding because
				//	we really should create a new state that switches to a different
				//	enemy if one hide over a station.

				SetState(stateAttackingTarget);
				m_pTarget = pTarget;
				ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
				}
			else
				{
				CancelCurrentOrder();
				}

			break;
			}

		case IShipController::orderDestroyPlayerOnReturn:
			{
			CSpaceObject *pGate = m_pShip->GetNearestStargate();
			if (pGate)
				{
				SetState(stateWaitForPlayerAtGate);
				m_pDest = pGate;
				}
			
			break;
			}

		case IShipController::orderDestroyTarget:
			{
			SetState(stateAttackingTargetAndAvoiding);
			m_pTarget = GetCurrentOrderTarget();
			ASSERT(m_pTarget);
			ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());

			//	See if we have a time limit

			if (GetCurrentOrderData() > 0)
				m_iCountdown = 1 + (g_TicksPerSecond * GetCurrentOrderData());
			else
				m_iCountdown = -1;
			break;
			}

		case IShipController::orderDestroyTargetHold:
			{
			SetState(stateAttackingTargetAndHolding);
			m_pTarget = GetCurrentOrderTarget();
			ASSERT(m_pTarget);
			ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());

			//	See if we have a time limit

			if (GetCurrentOrderData() > 0)
				m_iCountdown = 1 + (g_TicksPerSecond * GetCurrentOrderData());
			else
				m_iCountdown = -1;
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

			//	Otherwise, see if we should take a nav path

			else if (GetDistance2(pDest) > NAV_PATH_THRESHOLD2
					&& m_AICtx.CalcNavPath(m_pShip, pDest))
				{
				SetState(stateOnCourseForDockingViaNavPath);
				}

			//	Otherwise, try to dock

			else
				{
				SetState(stateOnCourseForDocking);
				m_pDest = pDest;
				}

			break;
			}

		case IShipController::orderEscort:
			{
			//	If this is a support ship, then we follow. Otherwise we
			//	are an armed escort.

			if (m_AICtx.IsNonCombatant())
				SetState(stateFollowing);
			else
				SetState(stateEscorting);

			m_pDest = GetCurrentOrderTarget();
			ASSERT(m_pDest);

			m_pShip->Communicate(m_pDest, msgEscortReportingIn, m_pShip);
			break;
			}

		case IShipController::orderFollow:
			{
			SetState(stateFollowing);
			m_pDest = GetCurrentOrderTarget();
			ASSERT(m_pDest);

			m_pShip->Communicate(m_pDest, msgEscortReportingIn, m_pShip);
			break;
			}

		case IShipController::orderFollowPlayerThroughGate:
			{
			SetState(stateOnCourseForStargate);
			m_pDest = m_pShip->GetNearestStargate();
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
				if (GetDistance2(pGate) > NAV_PATH_THRESHOLD2
						&& !m_AICtx.NoNavPaths()
						&& m_AICtx.CalcNavPath(m_pShip, pGate))
					{
					SetState(stateOnCourseForPointViaNavPath);
					}
				else
					{
					SetState(stateOnCourseForStargate);
					m_pDest = pGate;
					}
				}

			break;
			}

		case IShipController::orderGoTo:
			{
			CSpaceObject *pDest = GetCurrentOrderTarget();
			ASSERT(pDest);

			//	See if we should take a nav path

			if (GetDistance2(pDest) > NAV_PATH_THRESHOLD2
					&& m_AICtx.CalcNavPath(m_pShip, pDest))
				{
				SetState(stateOnCourseForPointViaNavPath);
				}

			//	Otherwise, go there

			else
				{
				SetState(stateOnCourseForPoint);
				m_pDest = pDest;

				//	Use m_iCountdown to store the desired rotation

				CVector vBearing = m_pDest->GetPos() - m_pShip->GetPos();
				m_iCountdown = m_pShip->AlignToRotationAngle(VectorToPolar(vBearing));
				}

			break;
			}

		case IShipController::orderApproach:
			{
			CSpaceObject *pDest = GetCurrentOrderTarget();
			ASSERT(pDest);

			//	See if we should take a nav path

			if (GetDistance2(pDest) > NAV_PATH_THRESHOLD2
					&& m_AICtx.CalcNavPath(m_pShip, pDest))
				{
				SetState(stateOnCourseForPointViaNavPath);
				}

			//	Otherwise, go there

			else
				{
				SetState(stateApproaching);
				m_pDest = GetCurrentOrderTarget();
				m_rDistance = LIGHT_SECOND * GetCurrentOrderData();
				if (m_rDistance < LIGHT_SECOND)
					m_rDistance = LIGHT_SECOND;
				}

			break;
			}

		case IShipController::orderGuard:
			{
			CSpaceObject *pPrincipal = GetCurrentOrderTarget();
			ASSERT(pPrincipal);

			//	If we're docked, wait for threat

			if (m_pShip->GetDockedObj())
				{
				SetState(stateWaitingForThreat);
				m_pDest = pPrincipal;
				}

			//	If we're very far from our principal and we can use a nav
			//	path, do it

			else if (GetDistance2(pPrincipal) > NAV_PATH_THRESHOLD2
					&& m_AICtx.CalcNavPath(m_pShip, pPrincipal))
				{
				SetState(stateReturningViaNavPath);
				}

			//	Otherwise, return directly to base

			else
				{
				SetState(stateReturningFromThreat);
				m_pDest = pPrincipal;
				}

			break;
			}

		case IShipController::orderGateOnStationDestroyed:
			break;

		case IShipController::orderGateOnThreat:
			SetState(stateWaitingUnarmed);
			break;

		case IShipController::orderHold:
			SetState(stateHolding);
			if (GetCurrentOrderData())
				m_iCountdown = 1 + (g_TicksPerSecond * GetCurrentOrderData());
			else
				m_iCountdown = -1;
			break;

		case IShipController::orderHoldCourse:
			SetState(stateHoldingCourse);
			//	Use m_iCountdown as course
			m_iCountdown = m_pShip->AlignToRotationAngle(LOWORD(GetCurrentOrderData()));
			m_rDistance = LIGHT_SECOND * HIWORD(GetCurrentOrderData());
			m_rDistance *= m_rDistance;
			//	Remember our current position so that we can compute distance traveled
			//	HACK: We use the potential vector to store it. When we 
			//	transition this to an order module this should be a member
			//	variable of the module.
			m_AICtx.SetPotential(m_pShip->GetPos());
			break;

		case IShipController::orderTurnTo:
			SetState(stateTurningTo);
			//	Use m_iCountdown as direction angle
			m_iCountdown = m_pShip->AlignToRotationAngle(GetCurrentOrderData());
			break;

		case IShipController::orderBombard:
			SetState(stateBombarding);
			m_pTarget = GetCurrentOrderTarget();
			ASSERT(m_pTarget);
			ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());

			if (GetCurrentOrderData())
				m_iCountdown = 1 + (g_TicksPerSecond * GetCurrentOrderData());
			else
				m_iCountdown = -1;
			break;

		case IShipController::orderLoot:
			{
			CSpaceObject *pDest = GetCurrentOrderTarget();
			ASSERT(pDest);

			if (m_pShip->GetDockedObj() == pDest)
				SetState(stateLootingOnce);
			else
				{
				SetState(stateOnCourseForDocking);
				m_pDest = pDest;
				}
			break;
			}

		case IShipController::orderNavPath:
			{
			CNavigationPath *pPath = m_pShip->GetSystem()->GetNavPathByID(GetCurrentOrderData());
			if (pPath)
				{
				SetState(stateFollowNavPath);
				m_AICtx.CalcNavPath(m_pShip, pPath);
				}
			else
				CancelCurrentOrder();

			break;
			}

		case IShipController::orderOrbit:
			{
			CSpaceObject *pBase = GetCurrentOrderTarget();
			Metric rPatrolRadius = LIGHT_SECOND * LOWORD(GetCurrentOrderData());

			ASSERT(pBase);

			Metric rDist2 = GetDistance2(pBase);
			if (rDist2 > NAV_PATH_THRESHOLD2
					&& rDist2 > (rPatrolRadius * rPatrolRadius)
					&& m_AICtx.CalcNavPath(m_pShip, pBase))
				{
				SetState(stateOnCourseForOrbitViaNavPath);
				}
			else
				{
				SetState(stateOrbit);
				m_pDest = pBase;
				m_rDistance = rPatrolRadius;
				DWORD dwTimer = HIWORD(GetCurrentOrderData());
				m_iCountdown = (dwTimer > 0 ? (g_TicksPerSecond * dwTimer) : -1);
				}

			break;
			}

		case IShipController::orderPatrol:
			{
			CSpaceObject *pBase = GetCurrentOrderTarget();
			Metric rPatrolRadius = LIGHT_SECOND * GetCurrentOrderData();
			ASSERT(pBase);

			Metric rDist2 = GetDistance2(pBase);
			if (rDist2 > NAV_PATH_THRESHOLD2
					&& rDist2 > (rPatrolRadius * rPatrolRadius)
					&& m_AICtx.CalcNavPath(m_pShip, pBase))
				{
				SetState(stateOnCourseForPatrolViaNavPath);
				}
			else
				{
				SetState(stateOnPatrolOrbit);
				m_pDest = pBase;
				m_rDistance = rPatrolRadius;
				}

			break;
			}

		case IShipController::orderScavenge:
			{
			if (m_pShip->GetDockedObj())
				SetState(stateLooting);
			else
				{
				SetState(stateLookingForLoot);
				m_rDistance = LIGHT_SECOND * 4 * (mathRandom(30, 120) + mathRandom(30, 120));
				}
			break;
			}

		case IShipController::orderTradeRoute:
			{
			int i;

			//	Look for a new station to go to.

			Metric rBestDist2 = 100.0 * LIGHT_SECOND * LIGHT_SECOND;
			CSpaceObject *pBestDest = NULL;
			for (i = 0; i < m_pShip->GetSystem()->GetObjectCount(); i++)
				{
				CSpaceObject *pObj = m_pShip->GetSystem()->GetObject(i);

				if (pObj
						&& pObj->SupportsDocking()
						&& !pObj->IsRadioactive()
						&& !pObj->IsEnemy(m_pShip)
						&& !pObj->IsAbandoned()
						&& !pObj->IsInactive()
						&& !pObj->IsVirtual()
						&& pObj->HasAttribute(CONSTLIT("populated")))
					{
					CVector vRange = pObj->GetPos() - m_pShip->GetPos();
					Metric rDistance2 = vRange.Dot(vRange);

					if (rDistance2 > rBestDist2)
						{
						if (pBestDest == NULL || mathRandom(1, 2) == 1)
							{
							rBestDist2 = rDistance2;
							pBestDest = pObj;
							}
						}
					}
				}

			//	If we've found some place to go, then dock

			if (pBestDest)
				{
				AddOrder(IShipController::orderWait, NULL, IShipController::SData(mathRandom(4, 24)), true);
				AddOrder(IShipController::orderDock, pBestDest, IShipController::SData(), true);
				}

			//	Otherwise, gate out of here

			else
				{
				CSpaceObject *pGate = m_pShip->GetNearestStargate(true);

				//	Head for the gate

				if (pGate)
					{
					SetState(stateOnCourseForStargate);
					m_pDest = pGate;
					}
				}

			break;
			}

		case IShipController::orderWait:
			{
			SetState(stateWaiting);
			if (GetCurrentOrderData())
				m_iCountdown = 1 + (g_TicksPerSecond * GetCurrentOrderData());
			else
				m_iCountdown = -1;
			break;
			}

		case IShipController::orderWaitForPlayer:
			{
			SetState(stateWaiting);
			break;
			}

		case IShipController::orderWaitForEnemy:
			{
			SetState(stateWaitingForEnemy);
			if (GetCurrentOrderData())
				m_iCountdown = 1 + (g_TicksPerSecond * GetCurrentOrderData());
			else
				m_iCountdown = -1;
			break;
			}

		case IShipController::orderWaitForTarget:
			{
			CSpaceObject *pTarget = GetCurrentOrderTarget();
			SData Data;
			GetCurrentOrderEx(&pTarget, &Data);
			ASSERT(pTarget);

			//	If we have a pair of numbers, then the first is the range at 
			//	which to react (which must be less than detection range).

			m_rDistance = LIGHT_SECOND * Data.AsInteger();
			DWORD dwTimer = Data.AsInteger2();

			SetState(stateWaitingForTarget);
			m_pDest = pTarget;
			if (dwTimer)
				m_iCountdown = 1 + (g_TicksPerSecond * dwTimer);
			else
				m_iCountdown = -1;
			break;
			}

		case IShipController::orderWander:
			{
			int i;

			//	Figure out which bearing to take. We set m_rDistance with
			//	the direction that we want to move. We will continue moving
			//	in that direction until we are blocked by an enemy or until
			//	the countdown expires.
			//
			//	First we figure out where we are relative to the center
			//	of the system.

			Metric rSunRadius;
			int iSunBearing = VectorToPolar(m_pShip->GetPos(), &rSunRadius);

			//	Figure out where we want to be

			Metric rBestDist = LIGHT_SECOND * (100 + (900 * m_pShip->GetDestiny() / g_DestinyRange));
			int iSign = ((m_pShip->GetDestiny() % 2) == 1 ? 1 : -1);

			//	Move outwards if we're too close; inward if we're too far

			int iBearing;
			if (rSunRadius > rBestDist)
				iBearing = (iSunBearing + iSign * (90 + 30) + 360) % 360;
			else
				iBearing = (iSunBearing + iSign * (90 - 15) + 360) % 360;

			//	Adjust the bearing if it turns out that there are enemy
			//	stations in that direction.

			iBearing = m_pShip->AlignToRotationAngle(iBearing);
			int iBearingStart = iBearing;
			for (i = 0; i < m_pShip->GetRotationRange(); i++)
				{
				if (!EnemyStationsAtBearing(m_pShip, iBearing, WANDER_SAFETY_RANGE))
					break;

				//	Adjust bearing

				iBearing = (iBearing + m_pShip->GetRotationAngle()) % 360;
				if (iBearing == iBearingStart)
					break;
				}

			//	Set state

			SetState(stateMaintainBearing);
			m_rDistance = (Metric)iBearing;
			m_iCountdown = 150 + mathRandom(0, 50);
			break;
			}
		}
	}

void CStandardShipAI::CommunicateWithEscorts (MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	CommunicateWithEscorts
//
//	Sends a message to the ship's escorts

	{
	m_AICtx.CommunicateWithEscorts(m_pShip, iMessage, pParam1, dwParam2);
	}

bool CStandardShipAI::IsBeingAttacked (void)

//	IsBeingAttacked
//
//	Returns TRUE if we've been attacked recently. This debounces multishot weapons.

	{
	int iInterval = m_pShip->GetSystem()->GetTick() - m_AICtx.GetLastAttack();
	return (iInterval > MULTI_HIT_WINDOW && iInterval < 3 * ATTACK_TIME_THRESHOLD);
	}

void CStandardShipAI::OnAttackedNotify (CSpaceObject *pAttacker, const DamageDesc &Damage)

//	OnAttackedNotify
//
//	Our ship was attacked. Note that pAttacker can either be an enemy ship/station
//	or it can be the missile/beam that hit us.

	{
	CSpaceObject *pOrderGiver = (pAttacker ? pAttacker->GetOrderGiver(Damage.GetCause()) : NULL);

	if (pAttacker)
		{
		//	If we were attacked by a friend, then warn them off
		//	(Unless we're explicitly targeting the friend)

		if (pOrderGiver && m_pShip->IsFriend(pOrderGiver) 
				&& pOrderGiver != m_pTarget
				&& !IsAngryAt(pAttacker))
			{
			//	Leave if necessary

			switch (GetCurrentOrder())
				{
				case IShipController::orderGateOnThreat:
					CancelCurrentOrder();
					AddOrder(IShipController::orderGate, NULL, IShipController::SData());
					break;

				case IShipController::orderGuard:
					if (pAttacker->GetBase() == GetBase())
						{
						CSpaceObject *pTarget = pAttacker->GetTarget(CItemCtx());
						if (pTarget)
							{
							SetState(stateAttackingThreat);
							m_pTarget = pTarget;
							ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
							}
						}
					break;
				}
			}

		//	Else if we were attacked by an enemy/neutral, see if we need
		//	to attack them (or flee). In this case, we take care of the immediate
		//	problem (attackers) instead of the order giver.

		else if (pAttacker->CanAttack())
			{
			//	Tell our escorts that we were attacked

			CommunicateWithEscorts(msgAttackDeter, pAttacker);

			//	Tell others that we were attacked

			switch (m_State)
				{
				case stateEscorting:
				case stateReturningToEscort:
					{
					m_pShip->Communicate(m_pDest, msgEscortAttacked, pAttacker);
					break;
					}
				}

			//	Change state to deal with the attack

			switch (m_State)
				{
				case stateEscorting:
				case stateReturningFromThreat:
				case stateWaitingForThreat:
					{
					SetState(stateAttackingThreat);
					m_pTarget = pAttacker;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					break;
					}

				case stateLookingForLoot:
				case stateMaintainBearing:
					SetState(stateDeterTarget);
					m_pTarget = pAttacker;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					break;

				case stateOnPatrolOrbit:
					SetState(stateAttackingOnPatrol);
					m_pTarget = pAttacker;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					break;

				case stateWaitingUnarmed:
					CancelCurrentOrder();
					AddOrder(IShipController::orderGate, NULL, IShipController::SData());
					break;

				case stateOnCourseForLootDocking:
					{
					CSpaceObject *pDest = m_pDest;
					SetState(stateDeterTargetWhileLootDocking);
					m_pDest = pDest;
					m_pTarget = pAttacker;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					break;
					}

				case stateOnCourseForPointViaNavPath:
				case stateOnCourseForStargate:
					{
					if (m_pTarget == NULL)
						m_pTarget = pAttacker;
					break;
					}

				case stateHolding:
					{
					if (m_pTarget == NULL && mathRandom(1, 3) == 1)
						{
						SetState(stateDeterTargetNoChase);
						m_pTarget = pAttacker;
						ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
						}
					break;
					}
				}

			//	Handle based on orders

			switch (GetCurrentOrder())
				{
				case IShipController::orderGuard:
					{
					//	If we were attacked twice (excluding multi-shot weapons)
					//	then we tell our station about this

					if (IsBeingAttacked())
						{
						CSpaceObject *pBase = GetCurrentOrderTarget();
						if (!pBase->IsEnemy(pAttacker)
								&& !pBase->IsAngryAt(pAttacker))
							m_pShip->Communicate(pBase, msgAttackDeter, pAttacker);
						}

					break;
					}

				case IShipController::orderWaitForEnemy:
					{
					//	If we're waiting for an enemy, then we've found one

					if (IsBeingAttacked())
						CancelCurrentOrder();
					break;
					}

				case IShipController::orderWaitForTarget:
					{
					//	If we're waiting for a target and the target attacked us
					//	then we've found it
					//
					//	We don't debouce hits (with IsBeingAttacked) because even
					//	a stray shot from the target means that the target is here.

					if (pAttacker == GetCurrentOrderTarget()
							|| pOrderGiver == GetCurrentOrderTarget())
						CancelCurrentOrder();
					break;
					}
				}
			}
		}
	}

DWORD CStandardShipAI::OnCommunicateNotify (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	OnCommunicateNotify
//
//	Handle communications from other objects

	{
	switch (iMessage)
		{
		case msgAbort:
			{
			switch (m_State)
				{
				case stateAttackingThreat:
				case stateAttackingPrincipalThreat:
				case stateOnPatrolOrbit:
					if (pParam1 == NULL || pParam1 == m_pTarget)
						SetState(stateNone);
					return resAck;

				default:
					return resNoAnswer;
				}
			}

		case msgAttack:
			{
			if (m_AICtx.IsNonCombatant())
				return resNoAnswer;

			switch (m_State)
				{
				case stateEscorting:
				case stateReturningFromThreat:
				case stateWaitingForThreat:
				case stateAttackingTarget:
				case stateAttackingThreat:
				case stateAttackingPrincipalThreat:
				case stateReturningToEscort:
					SetState(stateAttackingTarget);
					m_pTarget = pParam1;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					return resAck;

				case stateLookingForLoot:
					SetState(stateDeterTarget);
					m_pTarget = pParam1;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					return resAck;

				case stateOnPatrolOrbit:
					SetState(stateAttackingOnPatrol);
					m_pTarget = pParam1;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					return resAck;

				default:
					return resNoAnswer;
				}
			}

		case msgAttackDeter:
			{
			if (m_AICtx.IsNonCombatant())
				return resNoAnswer;

			switch (m_State)
				{
				case stateAttackingTarget:
					SetState(stateAttackingTarget);
					m_pTarget = pParam1;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					return resAck;

				case stateEscorting:
				case stateReturningToEscort:
					SetState(stateAttackingThreat);
					m_pTarget = pParam1;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					return resAck;

				case stateReturningFromThreat:
				case stateWaitingForThreat:
				case stateAttackingThreat:
					SetState(stateAttackingPrincipalThreat);
					m_pTarget = pParam1;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					return resAck;

				case stateLookingForLoot:
					SetState(stateDeterTarget);
					m_pTarget = pParam1;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					return resAck;

				case stateOnPatrolOrbit:
					SetState(stateAttackingOnPatrol);
					m_pTarget = pParam1;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					return resAck;

				default:
					return resNoAnswer;
				}
			}

		case msgDestroyBroadcast:
			{
			switch (m_State)
				{
				case stateEscorting:
				case stateReturningFromThreat:
				case stateWaitingForThreat:
				case stateOnPatrolOrbit:
				case stateReturningToEscort:
					if (pParam1)
						{
						AddOrder(IShipController::orderDestroyTarget,
								pParam1,
								IShipController::SData(),
								true);
						return resAck;
						}
					else
						return resNoAnswer;

				default:
					return resNoAnswer;
				}
			}

		case msgEscortAttacked:
			{
			//	Treat this as an attack on ourselves

			DamageDesc Dummy;
			OnAttacked(pParam1, Dummy);
			return resAck;
			}

		case msgEscortReportingIn:
			m_AICtx.SetHasEscorts(true);
			m_fHasEscorts = true;
			return resAck;

		case msgFormUp:
			{
			switch (GetCurrentOrder())
				{
				case IShipController::orderEscort:
					if (m_State != stateEscorting)
						{
						SetState(stateReturningToEscort);
						m_pDest = GetCurrentOrderTarget();
						}
					return resAck;

				default:
					return resNoAnswer;
				}
			}

		case msgQueryCommunications:
			{
			if (GetCurrentOrder() == IShipController::orderEscort)
				{
				DWORD dwRes = 0;
				if (!m_AICtx.IsNonCombatant())
					dwRes |= resCanAttack;
				if (m_State == stateAttackingTarget)
					dwRes |= (resCanAbortAttack | resCanFormUp);
				if (m_State != stateWaiting)
					dwRes |= resCanWait;
				else
					dwRes |= resCanFormUp;

				return dwRes;
				}
			else
				return 0;
			}

		case msgQueryEscortStatus:
			{
			if (GetEscortPrincipal() == pParam1)
				return resAck;
			else
				return resNoAnswer;
			}

		case msgQueryWaitStatus:
			return (m_State == stateWaiting ? resAck : resNoAnswer);

		case msgQueryAttackStatus:
			{
			switch (m_State)
				{
				case stateAttackingOnPatrol:
				case stateAttackingTarget:
				case stateAttackingThreat:
				case stateAttackingPrincipalThreat:
				case stateDeterTarget:
				case stateAttackingTargetAndAvoiding:
				case stateAttackingTargetAndHolding:
				case stateAvoidingEnemyStation:
				case stateBombarding:
					return resAck;

				default:
					return resNoAnswer;
				}
			}

		case msgWait:
			{
			if (GetCurrentOrder() == IShipController::orderEscort)
				{
				SetState(stateWaiting);
				m_pDest = GetCurrentOrderTarget();
				return resAck;
				}
			else
				return resNoAnswer;
			}

		default:
			return resNoAnswer;
		}
	}

CString CStandardShipAI::OnDebugCrashInfo (void)

//	OnDebugCrashInfo
//
//	Returns debug crash info

	{
	CString sResult;

	sResult.Append(CONSTLIT("CStandardShipAI\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("Order: %d\r\n"), (int)GetCurrentOrder()));
	sResult.Append(strPatternSubst(CONSTLIT("m_State: %d\r\n"), m_State));
	sResult.Append(strPatternSubst(CONSTLIT("m_pDest: %s\r\n"), CSpaceObject::DebugDescribe(m_pDest)));
	sResult.Append(strPatternSubst(CONSTLIT("m_pTarget: %s\r\n"), CSpaceObject::DebugDescribe(m_pTarget)));
	sResult.Append(strPatternSubst(CONSTLIT("m_pNavPath: %s\r\n"), CNavigationPath::DebugDescribe(m_pShip, m_AICtx.GetNavPath())));

	return sResult;
	}

void CStandardShipAI::OnDockedEvent (CSpaceObject *pObj)

//	OnDockedEvent
//
//	Event when the ship is docked

	{
	SetState(stateNone);
	}

CSpaceObject *CStandardShipAI::OnGetBase (void) const

//	GetBase
//
//	Returns this ship's base

	{
	switch (GetCurrentOrder())
		{
		case IShipController::orderDock:
		case IShipController::orderGuard:
		case IShipController::orderOrbit:
		case IShipController::orderPatrol:
			return GetCurrentOrderTarget();

		default:
			return NULL;
		}
	}

CSpaceObject *CStandardShipAI::OnGetTarget (bool bNoAutoTarget) const

//	GetTarget
//
//	Returns the target that this ship is attacking
	
	{
	switch (m_State)
		{
		case stateNone:
		case stateLooting:
		case stateLootingOnce:
		case stateWaitingForEnemy:
		case stateWaitingForThreat:
		case stateWaitingUnarmed:
			return NULL;

		default:
			return m_pTarget;
		}
	}

void CStandardShipAI::OnObjDestroyedNotify (const SDestroyCtx &Ctx)

//	OnObjDestroyedNotify
//
//	Handle the case where another object is destroyed

	{
	//	Alter our goals

	if (Ctx.pObj == m_pDest)
		{
		switch (m_State)
			{
			case stateOnPatrolOrbit:
			case stateEscorting:
			case stateOnCourseForDocking:
			case stateOnCourseForDockingViaNavPath:
			case stateOnCourseForLootDocking:
			case stateOnCourseForPatrolViaNavPath:
			case stateOnCourseForOrbitViaNavPath:
			case stateDeterTargetWhileLootDocking:
			case stateReturningFromThreat:
			case stateReturningViaNavPath:
			case stateOnCourseForStargate:
			case stateWaitingForThreat:
			case stateFollowing:
			case stateWaitForPlayerAtGate:
			case stateOnCourseForPoint:
			case stateWaitingForTarget:
			case stateAvoidingEnemyStation:
			case stateApproaching:
			case stateReturningToEscort:
				{
				//	If the object that we were heading for has been destroyed, then
				//	choose a new order.

				SetState(stateNone);
				break;
				}
			}

		m_pDest = NULL;
		}

	//	Alter our goals

	if (Ctx.pObj == m_pTarget)
		{
		switch (m_State)
			{
			case stateAimingAtTarget:
			case stateAttackingOnPatrol:
			case stateAttackingTarget:
			case stateAttackingTargetAndAvoiding:
			case stateAttackingTargetAndHolding:
			case stateAttackingThreat:
			case stateAttackingPrincipalThreat:
			case stateDeterTarget:
			case stateDeterTargetNoChase:
			case stateBombarding:
				{
				//	If a friend destroyed our target then thank them

				if (Ctx.Attacker.IsCausedByFriendOf(m_pShip) && Ctx.Attacker.GetObj())
					m_pShip->Communicate(Ctx.Attacker.GetObj(), msgNiceShooting);

				SetState(stateNone);
				break;
				}

			case stateDeterTargetWhileLootDocking:
				{
				CSpaceObject *pDest = m_pDest;
				SetState(stateOnCourseForLootDocking);
				m_pDest = pDest;
				break;
				}
			}

		m_pTarget = NULL;
		}

	//	Alter our orders

	if (GetOrderCount() > 0)
		{
		//	If our orders involve this target, then we've got to deal with
		//	that appropriately.

		if (Ctx.pObj == GetCurrentOrderTarget())
			{
			//	Deal with the consequences

			switch (GetCurrentOrder())
				{
				//	If we're looting then we don't remove the order. (If the
				//	object is really going away, the order will get deleted by
				//	our ancestor class).

				case IShipController::orderLoot:
				case IShipController::orderAttackArea:
					break;

				//	In these cases we avenge the target

				case IShipController::orderPatrol:
				case IShipController::orderGuard:
				case IShipController::orderEscort:
					{
					CancelCurrentOrder();

					if (Ctx.Attacker.IsCausedByNonFriendOf(m_pShip) && Ctx.Attacker.GetObj())
						AddOrder(IShipController::orderDestroyTarget, Ctx.Attacker.GetObj(), IShipController::SData());
					else
						AddOrder(IShipController::orderAttackNearestEnemy, NULL, IShipController::SData());

					break;
					}

				default:
					{
					CancelCurrentOrder();
					}
				}
			}
		}

	//	If we're docked with the object that got destroyed then react

	if (Ctx.pObj == m_pShip->GetDockedObj())
		{
		switch (GetCurrentOrder())
			{
			//	Gate out

			case IShipController::orderGateOnStationDestroyed:
			case IShipController::orderGateOnThreat:
				CancelCurrentOrder();
				AddOrder(IShipController::orderGate, NULL, IShipController::SData());
				break;

			//	Avenge

			default:
				if (Ctx.Attacker.IsCausedByEnemyOf(m_pShip))
					{
					CancelCurrentOrder();
					AddOrder(IShipController::orderDestroyTarget, Ctx.Attacker.GetObj(), IShipController::SData());
					}
			}
		}

	//	Clean up debug ship

	if (Ctx.pObj == g_pDebugShip)
		g_pDebugShip = NULL;
	}

void CStandardShipAI::OnOrderChanged (void)

//	OnOrderChanged
//
//	The current order has changed

	{
	SetState(stateNone);
	}

void CStandardShipAI::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Reads controller data from stream
//
//	DWORD		m_State
//	DWORD		m_pDest (CSpaceObject ref)
//	DWORD		m_pTarget (CSpaceObject ref)
//	Metric		m_rDistance
//
//	DWORD		m_iCountdown
//	DWORD		flags

	{
	DWORD dwLoad;

	//	Read stuff

	Ctx.pStream->Read((char *)&m_State, sizeof(DWORD));

	//	If we don't have an order module, then we expect to need these.

	if (m_pOrderModule == NULL)
		{
		CSystem::ReadObjRefFromStream(Ctx, &m_pDest);
		CSystem::ReadObjRefFromStream(Ctx, &m_pTarget);
		}

	//	Otherwise, we ignore them, since the order module should be
	//	handling it.
	//
	//	[In previous versions we neglected to clear them, so we could
	//	get cases where these were non-NULL but invalid (because we never
	//	got called on destruction).]

	else
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));

		m_pDest = NULL;
		m_pTarget = NULL;
		}

	Ctx.pStream->Read((char *)&m_rDistance, sizeof(Metric));

	//	In previous versions we didn't used to initialize m_rDistance 
	//	for this state.

	if (Ctx.dwVersion < 106 && m_State == stateWaitingForTarget)
		m_rDistance = 0.0;

	//	Read code

	if (Ctx.dwVersion >= 13 && Ctx.dwVersion < 75)
		{
		CString sCode;
		sCode.ReadFromStream(Ctx.pStream);
		if (!sCode.IsBlank())
			{
			ICCItem *pItem = g_pUniverse->GetCC().Link(sCode, 0, NULL);
			SetCommandCode(pItem);
			pItem->Discard(&g_pUniverse->GetCC());
			}
		}

	//	Read more stuff

	if (Ctx.dwVersion < 75)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_AICtx.SetLastAttack(dwLoad);
		}

	if (Ctx.dwVersion >= 4)
		Ctx.pStream->Read((char *)&m_iCountdown, sizeof(DWORD));
	else
		m_iCountdown = 1;

	//	Flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_fHasEscorts =			((dwLoad & 0x00000001) ? true : false);
	}

void CStandardShipAI::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Save the AI data to a stream
//
//	DWORD		m_State
//	DWORD		m_pDest (CSpaceObject ref)
//	DWORD		m_pTarget (CSpaceObject ref)
//	Metric		m_rDistance
//
//	DWORD		m_iCountdown
//	DWORD		flags

	{
	DWORD dwSave;

	pStream->Write((char *)&m_State, sizeof(DWORD));
	m_pShip->WriteObjRefToStream(m_pDest, pStream);
	m_pShip->WriteObjRefToStream(m_pTarget, pStream);
	pStream->Write((char *)&m_rDistance, sizeof(Metric));

	pStream->Write((char *)&m_iCountdown, sizeof(DWORD));

	dwSave = 0;
	dwSave |= (m_fHasEscorts ?			0x00000001 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CStandardShipAI::SetDebugShip (CShip *pShip)
	{
	g_pDebugShip = pShip;
	}

void CStandardShipAI::SetState (StateTypes State)

//	SetState
//
//	Sets the current state

	{
	//	If docking has been requested and we're trying to change state
	//	then cancel docking.

	if (IsDockingRequested())
		CancelDocking(m_pDest);

	//	Set state (NOTE: We do this before we undock because the Undock
	//	call may destroy the station and cause us to recurse into SetState.
	//	This happens when a ship is guarding an empty cargo crate).

	m_State = State;
	m_pTarget = NULL;
	m_pDest = NULL;

	//	If we're currently docked and we're changing to a state that
	//	does not support docking, then we undock first.

	if (m_pShip->GetDockedObj()
			&& State != stateNone 
			&& State != stateWaiting
			&& State != stateWaitingForThreat
			&& State != stateWaitingUnarmed
			&& State != stateLooting
			&& State != stateLootingOnce)
		m_pShip->Undock();
	}

//	Helpers

bool EnemyStationsAtBearing (CSpaceObject *pShip, int iBearing, Metric rRange)

//	EnemyStationsAtBearing
//
//	Examines a point at 2*rRange from the the ship, bearing iBearing. If there are
//	any enemies within an rRange radius of that point, we return TRUE.

	{
	DEBUG_TRY

	int i;

	//	Get the sovereign

	CSovereign *pSovereign = pShip->GetSovereign();
	if (pSovereign == NULL || pShip->GetSystem() == NULL)
		return false;

	//	Get the list of enemy objects

	const CSpaceObjectList &ObjList = pSovereign->GetEnemyObjectList(pShip->GetSystem());

	Metric rRange2 = rRange * rRange;
	CVector vCenter = pShip->GetPos() + PolarToVector(iBearing, 2.0 * rRange);

	int iObjCount = ObjList.GetCount();
	for (i = 0; i < iObjCount; i++)
		{
		CSpaceObject *pObj = ObjList.GetObj(i);

		if ((pObj->GetScale() == scaleStructure)
				&& pObj->CanAttack()
				&& pObj != pShip)
			{
			CVector vRange = pObj->GetPos() - vCenter;
			Metric rDistance2 = vRange.Dot(vRange);

			if (rDistance2 < rRange2)
				return true;
			}
		}

	DEBUG_CATCH

	return false;
	}

