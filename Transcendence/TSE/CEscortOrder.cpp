//	CEscortOrder.cpp
//
//	CEscortOrder class implementation
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

const int ATTACK_TIME_THRESHOLD =		150;
const Metric MAX_FOLLOW_DISTANCE		(g_KlicksPerPixel * 350.0);
const Metric PATROL_SENSOR_RANGE =		(30.0 * LIGHT_SECOND);
const Metric PATROL_SENSOR_RANGE2 =		(PATROL_SENSOR_RANGE * PATROL_SENSOR_RANGE);

void CEscortOrder::OnAttacked (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pAttacker, const DamageDesc &Damage, bool bFriendlyFire)

//	OnAttacked
//
//	We were attacked.

	{
	DEBUG_TRY

	if (pAttacker
			&& !bFriendlyFire
			&& pAttacker->CanAttack())
		{
		//	Tell our principal that we were attacked

		pShip->Communicate(m_Objs[objPrincipal], msgEscortAttacked, pAttacker);

		//	Attack the target

		if (!Ctx.IsNonCombatant()
				&& (m_iState == stateEscorting
					|| m_iState == stateAttackingThreat))
			{
			m_Objs[objTarget] = pAttacker;
			m_iState = stateAttackingThreat;
			}
		}

	DEBUG_CATCH
	}

void CEscortOrder::OnBehavior (CShip *pShip, CAIBehaviorCtx &Ctx)

//	OnBehavior
//
//	Do it

	{
	switch (m_iState)
		{
		case stateEscorting:
			{
			//	If we're a non-combatant, then we just follow

			if (Ctx.IsNonCombatant())
				{
				CVector vTarget = m_Objs[objPrincipal]->GetPos() - pShip->GetPos();
				Metric rTargetDist2 = vTarget.Dot(vTarget);
				Metric rMaxDist = (MAX_FOLLOW_DISTANCE) + (g_KlicksPerPixel * (pShip->GetDestiny() % 120));

				if (rTargetDist2 > (rMaxDist * rMaxDist))
					Ctx.ImplementCloseOnTarget(pShip, m_Objs[objPrincipal], vTarget, rTargetDist2);
				else if (rTargetDist2 < (g_KlicksPerPixel * g_KlicksPerPixel * 1024.0))
					Ctx.ImplementSpiralOut(pShip, vTarget);
				else
					Ctx.ImplementStop(pShip);
				}

			//	Otherwise, we follow the principal

			else
				{
				Ctx.ImplementEscort(pShip, m_Objs[objPrincipal], &m_Objs[objTarget]);

				//	See if we need to defer a threat

				if (!Ctx.NoAttackOnThreat() 
						&& pShip->IsDestinyTime(30) 
						&& !Ctx.IsWaitingForShieldsToRegen())
					{
					m_Objs[objTarget] = pShip->GetVisibleEnemyInRange(m_Objs[objPrincipal], PATROL_SENSOR_RANGE);
					if (m_Objs[objTarget])
						{
						m_iState = stateAttackingThreat;
						ASSERT(m_Objs[objTarget]->DebugIsValid() && m_Objs[objTarget]->NotifyOthersWhenDestroyed());
						}
					}
				}

			break;
			}

		case stateAttackingThreat:
			{
			ASSERT(m_Objs[objTarget]);
			Ctx.ImplementAttackTarget(pShip, m_Objs[objTarget]);
			Ctx.ImplementFireOnTargetsOfOpportunity(pShip, m_Objs[objTarget]);

			//	Every once in a while check to see if we've wandered too far from
			//	our base.

			if (pShip->IsDestinyTime(20))
				{
				int iTick = pShip->GetSystem()->GetTick();
				CVector vRange = m_Objs[objPrincipal]->GetPos() - pShip->GetPos();
				Metric rDistance2 = vRange.Dot(vRange);

				//	If we're outside of our patrol range and if we haven't
				//	been hit in a while then stop the attack.

				if (rDistance2 > (PATROL_SENSOR_RANGE * PATROL_SENSOR_RANGE)
						&& (iTick - Ctx.GetLastAttack()) > ATTACK_TIME_THRESHOLD)
					m_iState = stateEscorting;
				}

			break;
			}

		case stateWaiting:
			{
			Ctx.ImplementHold(pShip);
			Ctx.ImplementFireOnTargetsOfOpportunity(pShip);
			break;
			}
		}
	}

void CEscortOrder::OnBehaviorStart (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pOrderTarget, const IShipController::SData &Data)

//	OnBehaviorStart
//
//	Initialize order module

	{
	ASSERT(pOrderTarget);

	//	Set state

	m_iState = stateEscorting;

	//	Remember the principal and report in

	m_Objs[objPrincipal] = pOrderTarget;
	pShip->Communicate(pOrderTarget, msgEscortReportingIn, pShip);
	}

DWORD CEscortOrder::OnCommunicate (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	OnCommunicate
//
//	Handle communications

	{
	switch (iMessage)
		{
		case msgAbort:
			{
			switch (m_iState)
				{
				case stateAttackingThreat:
					if (pParam1 == NULL || pParam1 == m_Objs[objTarget])
						m_iState = stateEscorting;
					return resAck;

				default:
					return resNoAnswer;
				}
			}

		case msgAttack:
		case msgAttackDeter:
			{
			if (Ctx.IsNonCombatant()
					|| pParam1 == NULL
					|| pParam1->IsDestroyed())
				return resNoAnswer;

			switch (m_iState)
				{
				case stateEscorting:
				case stateAttackingThreat:
					m_iState = stateAttackingThreat;
					m_Objs[objTarget] = pParam1;
					ASSERT(m_Objs[objTarget]->DebugIsValid() && m_Objs[objTarget]->NotifyOthersWhenDestroyed());
					return resAck;

				default:
					return resNoAnswer;
				}
			}

		case msgDestroyBroadcast:
			{
			if (pParam1 == NULL
					|| !pParam1->IsDestroyed())
				return resNoAnswer;

			pShip->GetController()->AddOrder(IShipController::orderDestroyTarget,
						pParam1,
						IShipController::SData(),
						true);

			return resAck;
			}

		case msgFormUp:
			{
			if (m_iState != stateEscorting)
				{
				m_iState = stateEscorting;
				return resAck;
				}
			else
				return resNoAnswer;
			}

		case msgQueryCommunications:
			{
			DWORD dwRes = 0;

			if (!Ctx.IsNonCombatant())
				dwRes |= resCanAttack;
			if (m_iState == stateAttackingThreat)
				dwRes |= (resCanAbortAttack | resCanFormUp);
			if (m_iState != stateWaiting)
				dwRes |= resCanWait;
			else
				dwRes |= resCanFormUp;

			return dwRes;
			}

		case msgQueryEscortStatus:
			{
			if (m_Objs[objPrincipal] == pParam1)
				return resAck;
			else
				return resNoAnswer;
			}

		case msgQueryWaitStatus:
			return (m_iState == stateWaiting ? resAck : resNoAnswer);

		case msgQueryAttackStatus:
			return (m_iState == stateAttackingThreat ? resAck : resNoAnswer);

		case msgWait:
			m_iState = stateWaiting;
			return resAck;

		default:
			return resNoAnswer;
		}
	}

CSpaceObject *CEscortOrder::OnGetBase (void)

//	OnGetBase
//
//	Returns our base, which is the base of the principal.

	{
	if (m_Objs[objPrincipal] == NULL)
		return NULL;

	return m_Objs[objPrincipal]->GetBase();
	}

void CEscortOrder::OnObjDestroyed (CShip *pShip, const SDestroyCtx &Ctx, int iObj, bool *retbCancelOrder)

//	OnObjDestroyed
//
//	Object destroyed

	{
	if (iObj == objPrincipal)
		{
		//	Retaliate

		if (Ctx.Attacker.IsCausedByNonFriendOf(pShip) && Ctx.Attacker.GetObj())
			pShip->GetController()->AddOrder(IShipController::orderDestroyTarget, Ctx.Attacker.GetObj(), IShipController::SData());
		else
			pShip->GetController()->AddOrder(IShipController::orderAttackNearestEnemy, NULL, IShipController::SData());

		//	Cancel our order

		*retbCancelOrder = true;
		}
	else if (iObj == objTarget)
		{
		if (m_iState == stateAttackingThreat)
			m_iState = stateEscorting;
		}
	}

void CEscortOrder::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Load data from saved game

	{
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iState = (States)dwLoad;
	}

void CEscortOrder::OnWriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	OnWriteToStream
//
//	Write data to saved game

	{
	DWORD dwSave;

	dwSave = (DWORD)m_iState;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

