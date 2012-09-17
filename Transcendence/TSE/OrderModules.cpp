//	OrderModules.cpp
//
//	Implementation of IOrderModule classes
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

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

void IOrderModule::Attacked (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pAttacker, const DamageDesc &Damage)

//	Attacked
//
//	Handle an attack from another object

	{
	OnAttacked(pShip, Ctx, pAttacker, Damage);
	}

DWORD IOrderModule::Communicate (CShip *pShip, CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	Communicate
//
//	Handle communications from another ship

	{
	switch (iMessage)
		{
		case msgQueryAttackStatus:
			return (IsAttacking() ? resAck : resNoAnswer);

		default:
			return resNoAnswer;
		}
	}

IOrderModule *IOrderModule::Create (IShipController::OrderTypes iOrder)

//	Create
//
//	Creates an order module

	{
	switch (iOrder)
		{
		case IShipController::orderAttackStation:
			return new CAttackStationOrder;

		case IShipController::orderDestroyTarget:
			return new CAttackOrder;

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

	for (i = 0; i < m_iObjCount; i++)
		if (Ctx.pObj == m_Objs[i])
			{
			//	If this object is a target, and a friendly ship destroyed it, then
			//	thank the object who helped.

			if (IsTarget(i) && Ctx.Attacker.IsCausedByFriendOf(pShip) && Ctx.Attacker.GetObj())
				pShip->Communicate(Ctx.Attacker.GetObj(), msgNiceShooting);

			//	Let our derrived class handle it
				
			OnObjDestroyed(pShip, Ctx, i);

			//	Clear out the variable

			m_Objs[i] = NULL;
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
			Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_Objs[i]);
		else
			{
			CSpaceObject *pDummy;
			Ctx.pSystem->ReadObjRefFromStream(Ctx, &pDummy);
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

			//	If the target has left the safety of the station, 
			//	and the station is not between the ship and the target, then
			//	we stop avoiding.

			if (pShip->IsDestinyTime(23)
					&& rTargetDist2 > WANDER_SAFETY_RANGE2
					&& Absolute(AngleBearing(VectorToPolar(vTarget), VectorToPolar(vDest))) > 45)
				{
				//	Note: We don't set stateNone because we want to preserve the timer value

				m_iState = stateAttackingTargetAndAvoiding;
				m_Objs[objAvoid] = NULL;
				}

			//	See if our timer has expired

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				pShip->CancelCurrentOrder();

			break;
			}
		}
	}

void CAttackOrder::OnBehaviorStart (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pOrderTarget, DWORD dwOrderData)

//	OnBehaviorStart
//
//	Initialize order module

	{
	//	Make sure we're undocked because we're going flying

	Ctx.Undock(pShip);

	//	Set our state

	m_iState = stateAttackingTargetAndAvoiding;
	m_Objs[objTarget] = pOrderTarget;
	m_Objs[objAvoid] = NULL;
	ASSERT(m_Objs[objTarget]);
	ASSERT(m_Objs[objTarget]->DebugIsValid() && m_Objs[objTarget]->NotifyOthersWhenDestroyed());

	//	See if we have a time limit

	if (dwOrderData > 0)
		m_iCountdown = 1 + (g_TicksPerSecond * dwOrderData);
	else
		m_iCountdown = -1;
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

void CAttackOrder::OnObjDestroyed (CShip *pShip, const SDestroyCtx &Ctx, int iObj)

//	OnObjDestroyed
//
//	Notification that an object was destroyed

	{
	//	If the object we're avoiding was destroyed

	if (iObj == objAvoid)
		{
		//	No need to avoid anymore. Reset our state

		m_iState = stateAttackingTargetAndAvoiding;
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

void CAttackStationOrder::OnAttacked (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pAttacker, const DamageDesc &Damage)

//	OnAttacked
//
//	We've been attacked

	{
	//	If we're currently attacking the station, see if we should switch our
	//	attention to this defender.

	if (m_iState == stateAttackingTarget
			&& pAttacker 
			&& !pAttacker->IsDestroyed()

			//	If we have secondary weapons, then we keep attacking the
			//	station and let the secondaries deal with any defenders
			&& !Ctx.HasSecondaryWeapons()

			//	Don't bother with any ship that is not a friend of the
			//	station (we assume that this was just a stray shot)
			&& pAttacker->IsFriend(m_Objs[objTarget])

			//	If the attacker is too low level, then we concentrate on the
			//	station (we assume that we can ignore their attacks).
			&& pAttacker->GetLevel() >= pShip->GetLevel() - 3

			//	If the station is about to be destroyed, then we keep up
			//	the attack.
			&& m_Objs[objTarget]->GetVisibleDamage() < 75)
		{
		m_iState = stateAttackingDefender;
		m_Objs[objDefender] = pAttacker;
		}
	}

void CAttackStationOrder::OnBehavior (CShip *pShip, CAIBehaviorCtx &Ctx)

//	OnBehavior
//
//	Do it

	{
	switch (m_iState)
		{
		case stateAttackingTarget:
			{
			ASSERT(m_Objs[objTarget]);
			Ctx.ImplementAttackTarget(pShip, m_Objs[objTarget]);
			Ctx.ImplementFireOnTargetsOfOpportunity(pShip, m_Objs[objTarget]);

			//	See if our timer has expired

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				pShip->CancelCurrentOrder();

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

			//	See if our timer has expired

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				pShip->CancelCurrentOrder();

			break;
			}
		}
	}

void CAttackStationOrder::OnBehaviorStart (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pOrderTarget, DWORD dwOrderData)

//	OnBehaviorStart
//
//	Initialize order module

	{
	//	Make sure we're undocked because we're going flying

	Ctx.Undock(pShip);

	//	Set our state

	m_iState = stateAttackingTarget;
	m_Objs[objTarget] = pOrderTarget;
	m_Objs[objDefender] = NULL;
	ASSERT(m_Objs[objTarget]);
	ASSERT(m_Objs[objTarget]->DebugIsValid() && m_Objs[objTarget]->NotifyOthersWhenDestroyed());

	//	See if we have a time limit

	if (dwOrderData > 0)
		m_iCountdown = 1 + (g_TicksPerSecond * dwOrderData);
	else
		m_iCountdown = -1;
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

void CAttackStationOrder::OnObjDestroyed (CShip *pShip, const SDestroyCtx &Ctx, int iObj)

//	OnObjDestroyed
//
//	Notification that an object was destroyed

	{
	//	If the defender was destroyed, switch back

	if (iObj == objDefender)
		m_iState = stateAttackingTarget;
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

//	CGuardOrder ----------------------------------------------------------------

void CGuardOrder::OnBehavior (CShip *pShip, CAIBehaviorCtx &Ctx)

//	OnBehavior
//
//	Do it

	{
	}

void CGuardOrder::OnBehaviorStart (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pOrderTarget, DWORD dwOrderData)

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

	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pBase);
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
