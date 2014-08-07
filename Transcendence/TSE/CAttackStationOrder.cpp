//	CAttackStationOrder.cpp
//
//	CAttackStationOrder class implementation
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

const Metric PATROL_SENSOR_RANGE =		(30.0 * LIGHT_SECOND);
const Metric PATROL_SENSOR_RANGE2 =		(PATROL_SENSOR_RANGE * PATROL_SENSOR_RANGE);
const Metric NAV_PATH_THRESHOLD =		(4.0 * PATROL_SENSOR_RANGE);
const Metric NAV_PATH_THRESHOLD2 =		(NAV_PATH_THRESHOLD * NAV_PATH_THRESHOLD);

void CAttackStationOrder::OnAttacked (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pAttacker, const DamageDesc &Damage, bool bFriendlyFire)

//	OnAttacked
//
//	We've been attacked

	{
	DEBUG_TRY

	//	Nothing to do if no attacker

	if (pAttacker == NULL)
		;

	//	If we're currently attacking the station, see if we should switch our
	//	attention to this defender.

	else if (m_iState == stateAttackingTarget
			
			&& !bFriendlyFire

			//	If we have secondary weapons, then we keep attacking the
			//	station and let the secondaries deal with any defenders
			&& !Ctx.HasSecondaryWeapons()

			//	If the attacker is too low level, then we concentrate on the
			//	station (we assume that we can ignore their attacks).
			&& pAttacker->GetLevel() >= pShip->GetLevel() - 3

			//	If the station is about to be destroyed, then we keep up
			//	the attack.
			&& m_Objs[objTarget]->GetVisibleDamage() < 75
			
			//	Make sure this is a valid target otherwise
			&& Ctx.CalcIsBetterTarget(pShip, NULL, pAttacker))
		{
		m_iState = stateAttackingDefender;
		m_Objs[objDefender] = pAttacker;
		}

	//	If we're attacking a defender and some other defender attack us,
	//	see if we should switch to them.

	else if (m_iState == stateAttackingDefender
			&& !bFriendlyFire
			&& pAttacker != m_Objs[objTarget]
			&& pAttacker != m_Objs[objDefender]
			&& Ctx.CalcIsBetterTarget(pShip, m_Objs[objDefender], pAttacker))
		{
		m_Objs[objDefender] = pAttacker;
		}

	DEBUG_CATCH
	}

void CAttackStationOrder::OnBehavior (CShip *pShip, CAIBehaviorCtx &Ctx)

//	OnBehavior
//
//	Do it

	{
	//	See if our timer has expired

	if (m_iCountdown != -1 && m_iCountdown-- == 0)
		pShip->CancelCurrentOrder();

	//	Act based on current state

	switch (m_iState)
		{
		case stateAttackingTarget:
			{
			ASSERT(m_Objs[objTarget]);
			Ctx.ImplementAttackTarget(pShip, m_Objs[objTarget]);
			Ctx.ImplementFireOnTargetsOfOpportunity(pShip, m_Objs[objTarget]);
			break;
			}

		case stateAttackingDefender:
			{
			ASSERT(m_Objs[objDefender]);
			Ctx.ImplementAttackTarget(pShip, m_Objs[objDefender]);
			Ctx.ImplementFireOnTargetsOfOpportunity(pShip, m_Objs[objTarget]);

			//	If we've wandered too far away from the target, go back

			if (pShip->IsDestinyTime(20))
				{
				CVector vRange = m_Objs[objTarget]->GetPos() - pShip->GetPos();
				Metric rDistance2 = vRange.Dot(vRange);

				//	If we're outside of our patrol range and if we haven't
				//	been hit in a while then stop the attack.

				if (rDistance2 > PATROL_SENSOR_RANGE2 && Ctx.IsBeingAttacked(pShip->GetSystem()->GetTick()))
					{
					m_iState = stateAttackingTarget;
					m_Objs[objDefender] = NULL;
					}
				}

			break;
			}

		case stateFollowingNavPath:
			{
			Ctx.ImplementAttackNearestTarget(pShip, Ctx.GetBestWeaponRange(), &m_Objs[objDefender]);
			Ctx.ImplementFireOnTargetsOfOpportunity(pShip, m_Objs[objDefender]);

			bool bAtDest;
			Ctx.ImplementFollowNavPath(pShip, &bAtDest);
			if (bAtDest)
				{
				Ctx.ClearNavPath();
				m_iState = stateAttackingTarget;
				}

			break;
			}
		}
	}

void CAttackStationOrder::OnBehaviorStart (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pOrderTarget, const IShipController::SData &Data)

//	OnBehaviorStart
//
//	Initialize order module

	{
	//	Make sure we're undocked because we're going flying

	Ctx.Undock(pShip);

	//	If our target is already destroyed, then we're done

	if (pOrderTarget == NULL 
			|| pOrderTarget->IsDestroyed() 
			|| pOrderTarget->IsAbandoned())
		{
		pShip->CancelCurrentOrder();
		return;
		}

	//	Remember our target.

	m_Objs[objTarget] = pOrderTarget;
	m_Objs[objDefender] = NULL;
	ASSERT(m_Objs[objTarget]);
	ASSERT(m_Objs[objTarget]->DebugIsValid() && m_Objs[objTarget]->NotifyOthersWhenDestroyed());

	//	Set our state. If we are too far away from the target then we need to use
	//	a nav path.

	if (pShip->GetDistance2(pOrderTarget) > NAV_PATH_THRESHOLD2
			&& Ctx.CalcNavPath(pShip, pOrderTarget))
		m_iState = stateFollowingNavPath;
	else
		m_iState = stateAttackingTarget;

	//	See if we have a time limit

	DWORD dwTimer = Data.AsInteger();
	m_iCountdown = (dwTimer ? 1 + (g_TicksPerSecond * dwTimer) : -1);
	}

CString CAttackStationOrder::OnDebugCrashInfo (void)

//	OnDebugCrashInfo
//
//	Output crash information

	{
	CString sResult;

	sResult.Append(CONSTLIT("CAttackStationOrder\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("m_State: %d\r\n"), m_iState));
	sResult.Append(strPatternSubst(CONSTLIT("m_iCountdown: %d\r\n"), m_iCountdown));

	return sResult;
	}

void CAttackStationOrder::OnObjDestroyed (CShip *pShip, const SDestroyCtx &Ctx, int iObj, bool *retbCancelOrder)

//	OnObjDestroyed
//
//	Notification that an object was destroyed

	{
	//	If the defender was destroyed, switch back

	if (iObj == objDefender)
		{
		if (m_iState == stateAttackingDefender)
			m_iState = stateAttackingTarget;
		}
	}

void CAttackStationOrder::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Load data from saved game

	{
	DWORD dwLoad;

	//	Because of a bug, old versions did not save m_iState

	if (Ctx.dwVersion >= 76)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_iState = (States)dwLoad;
		}
	else
		{
		if (m_Objs[objDefender])
			m_iState = stateAttackingDefender;
		else
			m_iState = stateAttackingTarget;
		}

	//	Read the rest

	Ctx.pStream->Read((char *)&m_iCountdown, sizeof(DWORD));
	}

void CAttackStationOrder::OnWriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	OnWriteToStream
//
//	Write data to saved game

	{
	DWORD dwSave;

	dwSave = (DWORD)m_iState;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_iCountdown, sizeof(DWORD));
	}

