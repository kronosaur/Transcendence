//	OrderModules.cpp
//
//	Implementation of IOrderModule classes
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

const int ATTACK_TIME_THRESHOLD =		150;

const Metric ATTACK_RANGE				(g_KlicksPerPixel * 600.0);
const Metric MAX_FOLLOW_DISTANCE		(g_KlicksPerPixel * 350.0);
const Metric PATROL_SENSOR_RANGE =		(30.0 * LIGHT_SECOND);
const Metric SAFE_ORBIT_RANGE =			(30.0 * LIGHT_SECOND);
const Metric THREAT_SENSOR_RANGE =		(10.0 * LIGHT_SECOND);
const Metric WANDER_SAFETY_RANGE =		(20.0 * LIGHT_SECOND);

const Metric NAV_PATH_THRESHOLD =		(4.0 * PATROL_SENSOR_RANGE);

const Metric PATROL_SENSOR_RANGE2 =		(PATROL_SENSOR_RANGE * PATROL_SENSOR_RANGE);
const Metric NAV_PATH_THRESHOLD2 =		(NAV_PATH_THRESHOLD * NAV_PATH_THRESHOLD);
const Metric THREAT_SENSOR_RANGE2 =		(THREAT_SENSOR_RANGE * THREAT_SENSOR_RANGE);
const Metric WANDER_SAFETY_RANGE2 =		(WANDER_SAFETY_RANGE * WANDER_SAFETY_RANGE);

//	IOrderModule ---------------------------------------------------------------

IOrderModule::IOrderModule (int iObjCount)

//	IOrderModule constructor

	{
	int i;

	ASSERT(iObjCount <= MAX_OBJS);

	m_iObjCount = iObjCount;
	for (i = 0; i < m_iObjCount; i++)
		m_Objs[i] = NULL;
	}

IOrderModule::~IOrderModule (void)

//	IOrderModule destructor

	{
	}

void IOrderModule::Attacked (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pAttacker, const DamageDesc &Damage, bool bFriendlyFire)

//	Attacked
//
//	Handle an attack from another object

	{
	//	Tell our escorts that we were attacked, if necessary

	if (pAttacker
			&& !bFriendlyFire
			&& pAttacker->CanAttack())
		Ctx.CommunicateWithEscorts(pShip, msgAttackDeter, pAttacker);

	//	Let our subclass handle it.

	OnAttacked(pShip, Ctx, pAttacker, Damage, bFriendlyFire);
	}

DWORD IOrderModule::Communicate (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	Communicate
//
//	Handle communications from another ship

	{
	switch (iMessage)
		{
		case msgEscortAttacked:
			{
			//	Treat this as an attack on ourselves

			DamageDesc Dummy;
			pShip->GetController()->OnAttacked(pParam1, Dummy);
			return resAck;
			}

		case msgEscortReportingIn:
			Ctx.SetHasEscorts(true);
			return resAck;

		case msgQueryAttackStatus:
			return (IsAttacking() ? resAck : resNoAnswer);

		default:
			return OnCommunicate(pShip, Ctx, pSender, iMessage, pParam1, dwParam2);
		}
	}

IOrderModule *IOrderModule::Create (IShipController::OrderTypes iOrder)

//	Create
//
//	Creates an order module

	{
	switch (iOrder)
		{
		case IShipController::orderAttackArea:
		case IShipController::orderAttackNearestEnemy:
		case IShipController::orderDestroyTarget:
			return new CAttackOrder(iOrder);

		case IShipController::orderAttackStation:
			return new CAttackStationOrder;

		case IShipController::orderEscort:
			return new CEscortOrder;

		case IShipController::orderFireEvent:
			return new CFireEventOrder;

		case IShipController::orderSendMessage:
			return new CSendMessageOrder;

		case IShipController::orderWaitForUndock:
			return new CWaitOrder(CWaitOrder::waitForUndock);

		default:
			return NULL;
		}
	}

CString IOrderModule::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Output debug info

	{
	int i;

	CString sResult;

	sResult.Append(CONSTLIT("IOrderModule\r\n"));

	for (i = 0; i < m_iObjCount; i++)
		sResult.Append(strPatternSubst(CONSTLIT("m_Objs[%d]: %s\r\n"), i, CSpaceObject::DebugDescribe(m_Objs[i])));

	sResult.Append(OnDebugCrashInfo());

	return sResult;
	}

void IOrderModule::ObjDestroyed (CShip *pShip, const SDestroyCtx &Ctx)

//	ObjDestroyed
//
//	And object was destroyed

	{
	int i;
	bool bCancelOrder = false;

	for (i = 0; i < m_iObjCount; i++)
		if (Ctx.pObj == m_Objs[i])
			{
			//	If this object is a target, and a friendly ship destroyed it, then
			//	thank the object who helped.

			if (IsTarget(i) && Ctx.Attacker.IsCausedByFriendOf(pShip) && Ctx.Attacker.GetObj())
				pShip->Communicate(Ctx.Attacker.GetObj(), msgNiceShooting);

			//	Clear out the variable. We do this first because the derrived class
			//	might set it to something else (thus we don't want to clear it after
			//	the OnObjDestroyed call).

			m_Objs[i] = NULL;

			//	Let our derrived class handle it
				
			OnObjDestroyed(pShip, Ctx, i, &bCancelOrder);

			//	If our derrived class wants us to cancel the order, then we're done.
			//	(After we cancel the order, the order module will be invalid, so
			//	we need to leave.

			if (bCancelOrder)
				{
				pShip->CancelCurrentOrder();
				return;
				}
			}
	}

void IOrderModule::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Load save file
	
	{
	int i;

	//	Load the objects

	DWORD dwCount;
	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		if (i < m_iObjCount)
			CSystem::ReadObjRefFromStream(Ctx, &m_Objs[i]);
		else
			{
			CSpaceObject *pDummy;
			CSystem::ReadObjRefFromStream(Ctx, &pDummy);
			}
		}

	//	Let our derrived class load

	OnReadFromStream(Ctx); 
	}

void IOrderModule::WriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	WriteToStream
//
//	Write to save file
	
	{
	int i;

	//	Save the objects

	DWORD dwCount = m_iObjCount;
	pStream->Write((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		pSystem->WriteObjRefToStream(m_Objs[i], pStream);

	//	Let our derrived class save

	OnWriteToStream(pSystem, pStream);
	}

//	CAttackOrder ---------------------------------------------------------------

CAttackOrder::CAttackOrder (IShipController::OrderTypes iOrder) : IOrderModule(objCount),
		m_iOrder(iOrder)

//	CAttackOrder constructor

	{
	switch (m_iOrder)
		{
		case IShipController::orderDestroyTarget:
			m_fNearestTarget = false;
			m_fInRangeOfObject = false;
			break;

		case IShipController::orderAttackNearestEnemy:
			m_fNearestTarget = true;
			m_fInRangeOfObject = false;
			break;

		case IShipController::orderAttackArea:
			m_fNearestTarget = true;
			m_fInRangeOfObject = true;
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

	m_iState = stateAttackingTargetAndAvoiding;
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

//	CAttackStationOrder --------------------------------------------------------

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

//	CEscortOrder -----------------------------------------------------------------

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

//	CFireEventOrder ------------------------------------------------------------

void CFireEventOrder::OnBehavior (CShip *pShip, CAIBehaviorCtx &Ctx)

//	OnBehavior
//
//	Do it

	{
	//	Get the current order data

	CSpaceObject *pListener;
	IShipController::SData Data;
	pShip->GetCurrentOrder(&pListener, &Data);

	//	Done with the order.
	//
	//	NOTE: This will delete our object, so we can't access any member variables after this.

	pShip->CancelCurrentOrder();

	//	Fire the event

	if (pListener && !Data.sData.IsBlank())
		pListener->FireCustomShipOrderEvent(Data.sData, pShip);
	}

//	CGuardOrder ----------------------------------------------------------------

void CGuardOrder::OnBehavior (CShip *pShip, CAIBehaviorCtx &Ctx)

//	OnBehavior
//
//	Do it

	{
	}

void CGuardOrder::OnBehaviorStart (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pOrderTarget, const IShipController::SData &Data)

//	OnBehaviorStart
//
//	Start/restart behavior

	{
#if 0
	ASSERT(pOrderTarget);
	m_pBase = pOrderTarget;

	//	If we're docked, wait for threat

	if (pShip->GetDockedObj())
		m_iState = stateWaitingForThreat;

	//	If we're very far from our principal and we can use a nav
	//	path, do it

	else if (pShip->GetDistance2(m_pBase) > NAV_PATH_THRESHOLD2
			&& CalcNavPath(m_pBase))
		m_iState = stateReturningViaNavPath;

	//	Otherwise, return directly to base

	else
		m_iState = stateReturningFromThreat;
#endif
	}

void CGuardOrder::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Load from stream

	{
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iState = (States)dwLoad;

	CSystem::ReadObjRefFromStream(Ctx, &m_pBase);
	}

void CGuardOrder::OnWriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	OnWriteToStream
//
//	Save to stream

	{
	DWORD dwSave;

	dwSave = (DWORD)m_iState;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pSystem->WriteObjRefToStream(m_pBase, pStream);
	}

//	CSendMessageOrder ------------------------------------------------------------

void CSendMessageOrder::OnBehavior (CShip *pShip, CAIBehaviorCtx &Ctx)

//	OnBehavior
//
//	Do it

	{
	//	Get the current order data

	CSpaceObject *pRecipient;
	IShipController::SData Data;
	pShip->GetCurrentOrder(&pRecipient, &Data);

	//	Send the message

	if (pRecipient && Data.sData)
		pRecipient->SendMessage(pShip, Data.sData);

	//	Done with the order.

	pShip->CancelCurrentOrder();
	}

//	CWaitOrder -----------------------------------------------------------------

void CWaitOrder::OnBehavior (CShip *pShip, CAIBehaviorCtx &Ctx)

//	OnBehavior
//
//	Do it

	{
	//	Handle waiting

	Ctx.ImplementHold(pShip);
	Ctx.ImplementFireOnTargetsOfOpportunity(pShip);

	//	See if our timer has expired

	if (m_iCountdown != -1 && m_iCountdown-- == 0)
		pShip->CancelCurrentOrder();

	//	Otherwise, see if other conditions are fullfilled.

	else
		{
		switch (m_iType)
			{
			case waitForUndock:
				if (m_Objs[objTarget]->GetDockedObj() == NULL)
					pShip->CancelCurrentOrder();
				break;
			}
		}
	}

void CWaitOrder::OnBehaviorStart (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pOrderTarget, const IShipController::SData &Data)

//	OnBehaviorStart
//
//	Initialize order module

	{
	//	Set target

	switch (m_iType)
		{
		case waitForUndock:
			m_Objs[objTarget] = pOrderTarget;
			ASSERT(m_Objs[objTarget]->DebugIsValid() && m_Objs[objTarget]->NotifyOthersWhenDestroyed());
			break;
		}

	//	See if we have a time limit

	DWORD dwTimer = Data.AsInteger();
	m_iCountdown = (dwTimer ? 1 + (g_TicksPerSecond * dwTimer) : -1);
	}

void CWaitOrder::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Load data from saved game

	{
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iType = (EWaitTypes)dwLoad;

	Ctx.pStream->Read((char *)&m_iCountdown, sizeof(DWORD));
	}

void CWaitOrder::OnWriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	OnWriteToStream
//
//	Write data to saved game

	{
	DWORD dwSave;

	dwSave = (DWORD)m_iType;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_iCountdown, sizeof(DWORD));
	}
