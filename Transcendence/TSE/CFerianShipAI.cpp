//	CFerianShipAI.cpp
//
//	CFerianShipAI class

#include "PreComp.h"


#define MAX_MINING_DIST							(40.0 * g_KlicksPerPixel)
#define MAX_MINING_DIST2						(MAX_MINING_DIST * MAX_MINING_DIST)

const Metric MAX_ATTACK_DIST =					(45.0 * LIGHT_SECOND);
const Metric MAX_ATTACK_DIST2 =					(MAX_ATTACK_DIST * MAX_ATTACK_DIST);

const Metric MAX_THREAT_DIST =					(10.0 * LIGHT_SECOND);
const Metric MAX_THREAT_DIST2 =					(MAX_THREAT_DIST * MAX_THREAT_DIST);

#define MINING_APPROACH_DIST					(120.0 * g_KlicksPerPixel)
#define MINING_APPROACH_DIST2					(MINING_APPROACH_DIST * MINING_APPROACH_DIST)

const Metric MAX_MINING_RANGE =					(60.0 * LIGHT_SECOND);
const Metric MAX_MINING_RANGE2 =				(MAX_MINING_RANGE * MAX_MINING_RANGE);
const Metric CLOSE_MINING_RANGE =				(10.0 * LIGHT_SECOND);
const Metric CLOSE_MINING_RANGE2 =				(CLOSE_MINING_RANGE * CLOSE_MINING_RANGE);

const int MAX_RANDOM_COUNT =					300;

const int MINE_CYCLE_DELAY =					37;

#define ATTRIBUTE_ASTEROID						(CONSTLIT("asteroid"))

static CObjectClass<CFerianShipAI>g_Class(OBJID_CFERIANSHIPAI, NULL);

CFerianShipAI::CFerianShipAI (void) : CBaseShipAI(&g_Class),
		m_State(stateNone),
		m_pTarget(NULL),
		m_pBase(NULL)

//	CFerianShipAI constructor

	{
	}

void CFerianShipAI::Behavior (void)

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

		case stateAttackingTarget:
			{
			ASSERT(m_pTarget);
			m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);
			break;
			}

		case stateAttackingThreat:
			{
			ASSERT(m_pTarget);
			ASSERT(m_pBase);
			m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Every once in a while check to see if we've wandered too far from
			//	our base.

			if (m_pShip->IsDestinyTime(20))
				{
				CVector vRange = m_pBase->GetPos() - m_pShip->GetPos();
				Metric rDistance2 = vRange.Length2();

				//	If we're outside of our patrol range and if we haven't
				//	been hit in a while then stop the attack.

				if (rDistance2 > MAX_ATTACK_DIST2)
					{
					SetState(stateNone);
					m_pTarget = NULL;
					}
				}

			break;
			}

		case stateAvoidThreat:
			{
			ASSERT(m_pTarget);

			//	Pick a direction away from the target

			m_AICtx.ImplementEvasiveManeuvers(m_pShip, m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	If we're no longer close to the target, then revert to mining

			if (m_pShip->IsDestinyTime(21))
				{
				CVector vRange = m_pTarget->GetPos() - m_pShip->GetPos();
				Metric rDistance2 = vRange.Length2();
				if (rDistance2 > MAX_THREAT_DIST2)
					{
					SetState(stateNone);
					m_pTarget = NULL;
					}
				}

			break;
			}

		case stateMining:
			{
			ASSERT(m_pTarget);

			//	If there's a threat near-by, scurry away

			CSpaceObject *pThreat;
			if (InRangeOfThreat(&pThreat))
				{
				m_pShip->ResetMaxSpeed();
				SetState(stateAvoidThreat);
				m_pTarget = pThreat;
				}

			//	Otherwise, mine stuff

			else
				{
				if (m_pShip->IsDestinyTime(MINE_CYCLE_DELAY))
					{
					//	There's a chance that we stop mining and proceed to
					//	another asteroid.

					if (mathRandom(1, 100) <= 15)
						{
						SetState(stateNone);
						m_pTarget = NULL;
						}
					}
				}

			break;
			}

		case stateOnCourseForMine:
			{
			ASSERT(m_pTarget);

			//	If there is a threat near-by, scurry away

			CSpaceObject *pThreat;
			if (InRangeOfThreat(&pThreat))
				{
				m_pShip->ResetMaxSpeed();
				SetState(stateAvoidThreat);
				m_pTarget = pThreat;
				}

			//	Otherwise, go to the mine

			else
				{
				//	If we're close enough to the mine, then start mining

				CVector vDist = m_pTarget->GetPos() - m_pShip->GetPos();
				Metric rDist2 = vDist.Length2();
				if (rDist2 < MAX_MINING_DIST2)
					{
					m_pShip->ResetMaxSpeed();
					m_pShip->ClipSpeed(0.0);
					SetState(stateMining);
					}

				//	If we're near the mine, slow down

				else if (rDist2 < MINING_APPROACH_DIST2)
					{
					m_pShip->SetMaxSpeedHalf();
					m_AICtx.ImplementCloseOnTarget(m_pShip, m_pTarget, vDist, rDist2);
					}
				else
					m_AICtx.ImplementCloseOnTarget(m_pShip, m_pTarget, vDist, rDist2);
				}

			break;
			}

		case stateOnCourseForStargate:
			m_AICtx.ImplementGating(m_pShip, m_pBase);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
			break;

		case stateWaitForPlayerAtGate:
			m_AICtx.ImplementEscort(m_pShip, m_pBase, &m_pTarget);
			break;
		}
	}

void CFerianShipAI::BehaviorStart (void)

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

		case IShipController::orderDestroyTarget:
			{
			SetState(stateAttackingTarget);
			m_pTarget = GetCurrentOrderTarget();
			ASSERT(m_pTarget);
			ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
			break;
			}

		case IShipController::orderMine:
			{
			m_pBase = GetCurrentOrderTarget();
			ASSERT(m_pBase);

			SetState(stateOnCourseForMine);
			m_pTarget = FindRandomAsteroid();
			if (m_pTarget == NULL)
				{
				SetState(stateOnCourseForStargate);
				m_pBase = m_pShip->GetNearestStargate(true);
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
				m_pBase = pGate;
				}

			break;
			}

		case IShipController::orderDestroyPlayerOnReturn:
			{
			CSpaceObject *pGate = m_pShip->GetNearestStargate();
			if (pGate)
				{
				SetState(stateWaitForPlayerAtGate);
				m_pBase = pGate;
				}
			break;
			}
		}
	}

CString CFerianShipAI::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Returns debug crash info

	{
	CString sResult;

	sResult.Append(CONSTLIT("CFerianShipAI\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("Order: %d\r\n"), (int)GetCurrentOrder()));
	sResult.Append(strPatternSubst(CONSTLIT("m_State: %d\r\n"), m_State));
	sResult.Append(strPatternSubst(CONSTLIT("m_pBase: %s\r\n"), CSpaceObject::DebugDescribe(m_pBase)));
	sResult.Append(strPatternSubst(CONSTLIT("m_pTarget: %s\r\n"), CSpaceObject::DebugDescribe(m_pTarget)));

	return sResult;
	}

CSpaceObject *CFerianShipAI::FindRandomAsteroid (void)

//	FindRandomAsteroid
//
//	Returns a random asteroid within 60 light-seconds

	{
	int i;
	Metric rMaxDist2 = MAX_MINING_RANGE2;
	Metric rCloseDist2 = CLOSE_MINING_RANGE2;
	CSpaceObject *Table[MAX_RANDOM_COUNT];
	int iCount = 0;

	for (i = 0; 
			(i < m_pShip->GetSystem()->GetObjectCount() && iCount < MAX_RANDOM_COUNT); 
			i++)
		{
		CSpaceObject *pObj = m_pShip->GetSystem()->GetObject(i);

		if (pObj && pObj->HasAttribute(ATTRIBUTE_ASTEROID))
			{
			CVector vRange = pObj->GetPos() - m_pShip->GetPos();
			Metric rDistance2 = vRange.Dot(vRange);

			//	If we're within the max dist, add it to the list

			if (rDistance2 < rMaxDist2)
				Table[iCount++] = pObj;

			//	If we're within 10 light-seconds, add it twice more (to increase
			//	the probability)

			if (rDistance2 < rCloseDist2)
				{
				Table[iCount++] = pObj;
				Table[iCount++] = pObj;
				}
			}
		}

	//	Pick a random entry from the list

	if (iCount == 0)
		return NULL;
	else
		return Table[mathRandom(0, iCount-1)];
	}

bool CFerianShipAI::InRangeOfThreat (CSpaceObject **retpThreat)

//	InRangeOfThreat
//
//	Returns the nearest threat

	{
	if (m_pShip->IsDestinyTime(19))
		{
		CSystem *pSystem = m_pShip->GetSystem();
		int iDestiny = m_pShip->GetDestiny();

		//	Get the list of objects that intersect the object

		SSpaceObjectGridEnumerator i;
		pSystem->EnumObjectsInBoxStart(i, m_pShip->GetPos(), MAX_THREAT_DIST);

		//	Loop over all objects

		Metric rNearestDist2 = MAX_THREAT_DIST * MAX_THREAT_DIST2;
		CSpaceObject *pNearestObj = NULL;

		while (pSystem->EnumObjectsInBoxHasMore(i))
			{
			CSpaceObject *pObj = pSystem->EnumObjectsInBoxGetNext(i);

			//	If the object is in the bounding box then remember
			//	it so that we can do a more accurate calculation.

			if (pObj->GetCategory() == CSpaceObject::catShip
					&& (!m_pShip->IsFriend(pObj) || pObj->GetDestiny() < iDestiny)
					&& pObj->CanAttack())
				{
				Metric rDist2 = m_pShip->GetDistance2(pObj);
				if (rDist2 < rNearestDist2)
					{
					pNearestObj = pObj;
					rNearestDist2 = rDist2;
					}
				}
			}

		//	Done

		if (pNearestObj)
			{
			*retpThreat = pNearestObj;
			return true;
			}
		}

	return false;
	}

DWORD CFerianShipAI::OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	Communicate
//
//	Handle communications from other objects

	{
	switch (iMessage)
		{
		case msgAttack:
		case msgAttackDeter:
			{
			if (GetCurrentOrder() == IShipController::orderMine
					&& pSender == m_pBase)
				{
				SetState(stateAttackingThreat);
				m_pTarget = pParam1;
				return resAck;
				}
			else
				return resNoAnswer;
			}

		default:
			return resNoAnswer;
		}
	}

void CFerianShipAI::OnObjDestroyedNotify (const SDestroyCtx &Ctx)

//	OnObjDestroyedNotify
//
//	Deal with an object that has been destroyed

	{
	switch (GetCurrentOrder())
		{
		case IShipController::orderMine:
			{
			if (Ctx.pObj == GetCurrentOrderTarget())
				{
				//	Avenge the base

				if (Ctx.Attacker.IsCausedByNonFriendOf(m_pShip) && Ctx.Attacker.GetObj())
					AddOrder(IShipController::orderDestroyTarget, Ctx.Attacker.GetObj(), IShipController::SData());
				else if (m_State == stateAttackingThreat)
					AddOrder(IShipController::orderDestroyTarget, m_pTarget, IShipController::SData());

				//	Stop mining

				CancelCurrentOrder();
				}
			break;
			}
		}

	//	Reset

	if (m_pBase == Ctx.pObj)
		{
		SetState(stateNone);
		m_pBase = NULL;
		}

	if (m_pTarget == Ctx.pObj)
		{
		SetState(stateNone);
		m_pTarget = NULL;
		}
	}

void CFerianShipAI::OnOrderChanged (void)

//	OnOrderChanged
//
//	The order list has changed

	{
	SetState(stateNone);
	m_pTarget = NULL;
	m_pBase = NULL;
	}

void CFerianShipAI::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read our data
//
//	DWORD		m_State
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pBase (CSpaceObject ref)

	{
	Ctx.pStream->Read((char *)&m_State, sizeof(DWORD));
	CSystem::ReadObjRefFromStream(Ctx, &m_pTarget);
	CSystem::ReadObjRefFromStream(Ctx, &m_pBase);
	}

void CFerianShipAI::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write our data
//
//	DWORD		m_State
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pBase (CSpaceObject ref)

	{
	pStream->Write((char *)&m_State, sizeof(DWORD));
	m_pShip->WriteObjRefToStream(m_pTarget, pStream);
	m_pShip->WriteObjRefToStream(m_pBase, pStream);
	}

void CFerianShipAI::SetState (StateTypes State)

//	SetState
//
//	Sets the current state

	{
	m_State = State;
	}
