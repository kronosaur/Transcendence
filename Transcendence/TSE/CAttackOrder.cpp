//	CAttackOrder.cpp
//
//	CAttackOrder class implementation
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

const Metric SAFE_ORBIT_RANGE =			(30.0 * LIGHT_SECOND);
const Metric THREAT_SENSOR_RANGE =		(10.0 * LIGHT_SECOND);
const Metric THREAT_SENSOR_RANGE2 =		(THREAT_SENSOR_RANGE * THREAT_SENSOR_RANGE);

const Metric WANDER_SAFETY_RANGE =		(20.0 * LIGHT_SECOND);
const Metric WANDER_SAFETY_RANGE2 =		(WANDER_SAFETY_RANGE * WANDER_SAFETY_RANGE);

CAttackOrder::CAttackOrder (IShipController::OrderTypes iOrder) : IOrderModule(objCount),
		m_iOrder(iOrder)

//	CAttackOrder constructor

	{
	switch (m_iOrder)
		{
		case IShipController::orderDestroyTarget:
			m_fNearestTarget = false;
			m_fInRangeOfObject = false;
			m_fHold = false;
			break;

		case IShipController::orderAttackNearestEnemy:
			m_fNearestTarget = true;
			m_fInRangeOfObject = false;
			m_fHold = false;
			break;

		case IShipController::orderAttackArea:
			m_fNearestTarget = true;
			m_fInRangeOfObject = true;
			m_fHold = false;
			break;

		case IShipController::orderHoldAndAttack:
			m_fNearestTarget = false;
			m_fInRangeOfObject = false;
			m_fHold = true;
			break;

		default:
			ASSERT(false);
		}
	}

CSpaceObject *CAttackOrder::GetBestTarget (CShip *pShip)

//	GetBestTarget
//
//	Returns the best target that we should pursue right now. We return NULL if
//	there is no appropriate target.

	{
	int i;

	if (m_fInRangeOfObject)
		{
		//	Get the range information

		Metric rRange;
		CSpaceObject *pCenter = GetTargetArea(pShip, &rRange);
		if (pCenter == NULL)
			return NULL;

		Metric rRange2 = rRange * rRange;
		CVector vCenter = pCenter->GetPos();

		//	Get the list of all visible enemy objects

		TArray<CSpaceObject *> Targets;
		pShip->GetVisibleEnemies(0, &Targets);

		//	Find the nearest enemy to us that is inside our range

		CSpaceObject *pBestObj = NULL;
		Metric rBestDist2;

		for (i = 0; i < Targets.GetCount(); i++)
			{
			CSpaceObject *pObj = Targets[i];

			//	Make sure this is within the target area

			Metric rDist2 = (pObj->GetPos() - vCenter).Length2();
			if (rDist2 < rRange2)
				{
				//	See if this is closer than our current candidate

				rDist2 = (pObj->GetPos() - pShip->GetPos()).Length2();
				if (pBestObj == NULL || rDist2 < rBestDist2)
					{
					pBestObj = pObj;
					rBestDist2 = rDist2;
					}
				}
			}

		return pBestObj;
		}
	else
		return pShip->GetNearestVisibleEnemy();
	}

CSpaceObject *CAttackOrder::GetTargetArea (CShip *pShip, Metric *retrRange)

//	GetTargetArea
//
//	Returns the target area that we need to clear.

	{
	if (!m_fInRangeOfObject)
		return NULL;

	CSpaceObject *pCenter;
	IShipController::SData Data;
	pShip->GetCurrentOrder(&pCenter, &Data);

	if (Data.iDataType == IShipController::dataInteger
			|| Data.iDataType == IShipController::dataPair)
		*retrRange = LIGHT_SECOND * Data.dwData1;
	else
		*retrRange = LIGHT_SECOND * 100.0;

	return pCenter;
	}

bool CAttackOrder::IsBetterTarget (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pOldTarget, CSpaceObject *pNewTarget)

//	IsBetterTarget
//
//	Returns TRUE if pNewTarget is a better target.

	{
	//	Check to see if the new target is generically better.

	if (!Ctx.CalcIsBetterTarget(pShip, pOldTarget, pNewTarget))
		return false;

	//	If we're clearing out an area, then make sure the new target is inside
	//	the area we're trying to clear.

	if (m_fInRangeOfObject && !IsInTargetArea(pShip, pNewTarget))
		return false;

	//	New target is better

	return true;
	}

bool CAttackOrder::IsInTargetArea (CShip *pShip, CSpaceObject *pObj)

//	IsInTargetArea
//
//	Returns TRUE if pObj is in the area that we're trying to clear.

	{
	//	If we're not trying to clear an area, then pObj is always valid.

	if (!m_fInRangeOfObject)
		return true;

	//	Get the original order so we can figure out the area that we're trying
	//	to clear.

	Metric rRange;
	CSpaceObject *pCenter = GetTargetArea(pShip, &rRange);
	if (pCenter == NULL)
		return true;

	//	Check the range

	Metric rDist2 = (pObj->GetPos() - pCenter->GetPos()).Length2();
	if (rDist2 > rRange * rRange)
		return false;

	//	In range

	return true;
	}

void CAttackOrder::OnAttacked (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pAttacker, const DamageDesc &Damage, bool bFriendlyFire)

//	OnAttacked
//
//	Attacked by something

	{
	DEBUG_TRY

	//	If the ship that attacked us is an enemy and it is closer than the 
	//	current target, then switch to attack it.

	if (m_fNearestTarget 
			&& pAttacker
			&& !bFriendlyFire
			&& pAttacker != m_Objs[objTarget]
			&& IsBetterTarget(pShip, Ctx, m_Objs[objTarget], pAttacker))
		m_Objs[objTarget] = pAttacker;

	DEBUG_CATCH
	}

void CAttackOrder::OnBehavior (CShip *pShip, CAIBehaviorCtx &Ctx)

//	OnBehavior
//
//	Do it

	{
	switch (m_iState)
		{
		case stateAttackingTargetAndAvoiding:
			{
			ASSERT(m_Objs[objTarget]);
			Ctx.ImplementAttackTarget(pShip, m_Objs[objTarget]);
			Ctx.ImplementFireOnTargetsOfOpportunity(pShip, m_Objs[objTarget]);

			//	Every once in a while check to see if we've wandered near
			//	an enemy station

			if (pShip->IsDestinyTime(41) && !Ctx.IsImmobile() && m_Objs[objTarget]->CanMove())
				{
				CSpaceObject *pEnemy = pShip->GetNearestEnemyStation(WANDER_SAFETY_RANGE);
				if (pEnemy 
						&& pEnemy != m_Objs[objTarget]
						&& m_Objs[objTarget]->GetDistance2(pEnemy) < WANDER_SAFETY_RANGE2)
					{
					m_iState = stateAvoidingEnemyStation;
					m_Objs[objAvoid] = pEnemy;
					}
				}

			//	See if our timer has expired

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				pShip->CancelCurrentOrder();

			break;
			}

		case stateAttackingTargetAndHolding:
			{
			ASSERT(m_Objs[objTarget]);
			Ctx.ImplementHold(pShip);
			Ctx.ImplementAttackTarget(pShip, m_Objs[objTarget], true);
			Ctx.ImplementFireOnTargetsOfOpportunity(pShip, m_Objs[objTarget]);

			//	See if our timer has expired

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				pShip->CancelCurrentOrder();

			break;
			}

		case stateAvoidingEnemyStation:
			{
			ASSERT(m_Objs[objTarget]);
			ASSERT(m_Objs[objAvoid]);

			int iTick = pShip->GetSystem()->GetTick();
			CVector vTarget = m_Objs[objTarget]->GetPos() - pShip->GetPos();
			Metric rTargetDist2 = vTarget.Length2();
			CVector vDest = m_Objs[objAvoid]->GetPos() - pShip->GetPos();

			//	We only spiral in/out part of the time (we leave ourselves some time to fight)

			bool bAvoid = (rTargetDist2 > THREAT_SENSOR_RANGE2) || ((iTick + pShip->GetDestiny()) % 91) > 55;
			
			if (!bAvoid)
				{
				//	Attack target

				Ctx.ImplementAttackTarget(pShip, m_Objs[objTarget], true);
				Ctx.ImplementFireOnTargetsOfOpportunity(pShip, m_Objs[objTarget]);
				}
			else
				{
				//	Orbit around the enemy station

				Metric rDestDist2 = vDest.Length2();

				const Metric rMaxDist = SAFE_ORBIT_RANGE * 1.2;
				const Metric rMinDist = SAFE_ORBIT_RANGE * 0.9;

				if (rDestDist2 > (rMaxDist * rMaxDist))
					Ctx.ImplementSpiralIn(pShip, vDest);
				else if (rDestDist2 < (rMinDist * rMinDist))
					Ctx.ImplementSpiralOut(pShip, vDest);
				else
					{
					Ctx.ImplementAttackTarget(pShip, m_Objs[objTarget], true);
					Ctx.ImplementFireOnTargetsOfOpportunity(pShip, m_Objs[objTarget]);
					}
				}

			//	Check to see if we should do something else

			if (pShip->IsDestinyTime(23))
				{
				int iAngleToStation = VectorToPolar(vDest);

				//	If the target has left the safety of the station, 
				//	and the station is not between the ship and the target, then
				//	we stop avoiding.

				if (rTargetDist2 > WANDER_SAFETY_RANGE2
						&& Absolute(AngleBearing(VectorToPolar(vTarget), iAngleToStation)) > 45)
					{
					//	Note: We don't set stateNone because we want to preserve the timer value

					m_iState = stateAttackingTargetAndAvoiding;
					m_Objs[objAvoid] = NULL;
					ASSERT(m_Objs[objTarget]);
					}

				//	Otherwise, if we're attacking any target, see if there is something 
				//	else that we can tackle.

				else if (m_fNearestTarget)
					{
					CSpaceObject *pNewTarget = GetBestTarget(pShip);
					if (pNewTarget && pNewTarget != m_Objs[objTarget])
						{
						CVector vNewTarget = pNewTarget->GetPos() - pShip->GetPos();
						Metric rNewTargetDist2 = vNewTarget.Length2();
						if (rNewTargetDist2 > WANDER_SAFETY_RANGE2
								&& Absolute(AngleBearing(VectorToPolar(vNewTarget), iAngleToStation)) > 45)
							{
							m_iState = stateAttackingTargetAndAvoiding;
							m_Objs[objAvoid] = NULL;
							m_Objs[objTarget] = pNewTarget;
							}
						}
					}
				}

			//	See if our timer has expired

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				pShip->CancelCurrentOrder();

			break;
			}
		}
	}

void CAttackOrder::OnBehaviorStart (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pOrderTarget, const IShipController::SData &Data)

//	OnBehaviorStart
//
//	Initialize order module

	{
	//	Make sure we're undocked because we're going flying

	Ctx.Undock(pShip);

	//	If we don't have a target and we're looking for the nearest enemy, then
	//	find it now.

	if (m_fNearestTarget)
		{
		pOrderTarget = GetBestTarget(pShip);
		if (pOrderTarget == NULL)
			{
			pShip->CancelCurrentOrder();
			return;
			}
		}
	else if (pOrderTarget == NULL || pOrderTarget->IsDestroyed())
		{
		pShip->CancelCurrentOrder();
		return;
		}

	//	Set our state

	m_iState = (m_fHold ? stateAttackingTargetAndHolding : stateAttackingTargetAndAvoiding);
	m_Objs[objTarget] = pOrderTarget;
	m_Objs[objAvoid] = NULL;
	ASSERT(m_Objs[objTarget]);
	ASSERT(m_Objs[objTarget]->DebugIsValid() && m_Objs[objTarget]->NotifyOthersWhenDestroyed());

	//	See if we have a time limit

	DWORD dwTimer = (m_fInRangeOfObject ? Data.AsInteger2() : Data.AsInteger());
	m_iCountdown = (dwTimer ? 1 + (g_TicksPerSecond * dwTimer) : -1);
	}

CString CAttackOrder::OnDebugCrashInfo (void)

//	OnDebugCrashInfo
//
//	Output crash information

	{
	CString sResult;

	sResult.Append(CONSTLIT("CAttackOrder\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("m_State: %d\r\n"), m_iState));
	sResult.Append(strPatternSubst(CONSTLIT("m_iCountdown: %d\r\n"), m_iCountdown));

	return sResult;
	}

void CAttackOrder::OnObjDestroyed (CShip *pShip, const SDestroyCtx &Ctx, int iObj, bool *retbCancelOrder)

//	OnObjDestroyed
//
//	Notification that an object was destroyed

	{
	//	If the object we're avoiding was destroyed

	if (iObj == objAvoid)
		{
		//	No need to avoid anymore. Reset our state

		if (m_iState == stateAvoidingEnemyStation)
			m_iState = stateAttackingTargetAndAvoiding;
		ASSERT(m_Objs[objTarget]);
		}

	//	If our target was destroyed and we need to attack the nearest
	//	target, then go for it.

	else if (m_fNearestTarget && iObj == objTarget)
		{
		CSpaceObject *pNewTarget = GetBestTarget(pShip);
		if (pNewTarget == NULL)
			{
			*retbCancelOrder = true;
			return;
			}

		m_Objs[objTarget] = pNewTarget;
		}
	}

void CAttackOrder::OnReadFromStream (SLoadCtx &Ctx)

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
		if (m_Objs[objAvoid])
			m_iState = stateAvoidingEnemyStation;
		else
			m_iState = stateAttackingTargetAndAvoiding;
		}

	//	Read the rest

	Ctx.pStream->Read((char *)&m_iCountdown, sizeof(DWORD));
	}

void CAttackOrder::OnWriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	OnWriteToStream
//
//	Write data to saved game

	{
	DWORD dwSave;

	dwSave = (DWORD)m_iState;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_iCountdown, sizeof(DWORD));
	}

