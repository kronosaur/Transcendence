//	CFleetShipAI.cpp
//
//	CFleetShipAI class

#include "PreComp.h"


#define ATTACK_AT_WILL_RANGE					(g_KlicksPerPixel * 2400.0)
#define PATROL_SENSOR_RANGE						(g_KlicksPerPixel * 1000.0)
const Metric PATROL_ORBIT_DIST =				(LIGHT_SECOND * 10.0);
#define ATTACK_RANGE							(g_KlicksPerPixel * 600.0)

struct SFormationPlace
	{
	int iAngle;									//	Angle with respect to leader
	int iDistance;								//	Distance in pixels
	};

struct SFormationDesc
	{
	SFormationPlace *pFormation;
	int iCount;
	};

static CObjectClass<CFleetShipAI>g_Class(OBJID_CFLEETSHIPAI, NULL);

static SFormationPlace g_ArrowFormation[] =
	{
		{	120,	80	},
		{	240,	80	},
		{	120,	160	},
		{	240,	160 },
		{	150,	75	},		//	5
		{	210,	75	},

		{	120,	240 },
		{	240,	240 },
		{	150,	150	},
		{	210,	150 },		//	10
		{	120,	320	},
		{	240,	320 },

		{	150,	225	},
		{	210,	225 },
		{	120,	400 },		//	15
		{	240,	400 },
		{	135,	200 },
		{	225,	200 },

		{	150,	300 },
		{	210,	300 },		//	20
		{	120,	480 },
		{	240,	480 },
		{	135,	280 },
		{	225,	280 },

		{	135,	360 },		//	25
		{	225,	360 },
		{	180,	125 },
		{	170,	175 },
		{	190,	175 },
		{	180,	250 },		//	30

		{	165,	240 },
		{	195,	240 },
		{	170,	300 },
		{	190,	300	},
		{	135,	130 },		//	35
		{	225,	130 },
	};

#define ARROW_FORMATION_COUNT					(sizeof(g_ArrowFormation) / sizeof(g_ArrowFormation[0]))

static SFormationPlace g_FrontFormation[] =
	{
		{	90,		80	},
		{	270,	80	},
		{	90,		160	},
		{	270,	160	},
		{	90,		240	},
		{	270,	240	},

		{	90,		320	},
		{	270,	320	},
		{	90,		400	},
		{	270,	400	},
		{	90,		480	},
		{	270,	480	},

		{	90,		560	},
		{	270,	560	},
		{	90,		640	},
		{	270,	640	},
		{	135,	57  },
		{	225,	57  },

		{	108,	126 },
		{	252,	126 },
		{	101,	204 },
		{	259,	204 },
		{	98,		283 },
		{	262,	283 },
	};

#define FRONT_FORMATION_COUNT					(sizeof(g_FrontFormation) / sizeof(g_FrontFormation[0]))

static SFormationPlace g_ShellFormation[] =
	{
		{	30,		240	},
		{	330,	240	},
		{	60,		240	},
		{	300,	240	},
		{	90,		240	},
		{	270,	240	},

		{	120,	240	},
		{	240,	240	},
		{	150,	240	},
		{	210,	240	},
		{	180,	240	},
		{	0,		240 },

		{	15,		320 },
		{	345,	320 },
		{	45,		320 },
		{	315,	320 },
		{	75,		320 },
		{	285,	320 },

		{	105,	320 },
		{	255,	320 },
		{	135,	320 },
		{	225,	320 },
		{	165,	320 },
		{	195,	320 },
	};

#define SHELL_FORMATION_COUNT					(sizeof(g_ShellFormation) / sizeof(g_ShellFormation[0]))

SFormationDesc g_Formations[] =
	{
		{	g_ArrowFormation,		ARROW_FORMATION_COUNT	},
		{	g_FrontFormation,		FRONT_FORMATION_COUNT	},
		{	g_ShellFormation,		SHELL_FORMATION_COUNT	},
	};

#define FORMATIONS_COUNT						(sizeof(g_Formations) / sizeof(g_Formations[0]))

CFleetShipAI::CFleetShipAI (void) : CBaseShipAI(&g_Class),
		m_State(stateNone),
		m_pLeader(NULL),
		m_pTarget(NULL),
		m_pDest(NULL)

//	CFleetShipAI constructor

	{
	}

void CFleetShipAI::Behavior (void)

//	Behavior

	{
	//	Reset

	ResetBehavior();

	//	Use basic items

	UseItemsBehavior();

	//	Behave according to our state

	switch (m_State)
		{
		case stateNone:
			BehaviorStart();
			break;

		case stateAttackAtWill:
			{
			//	If we don't have a target, find one

			if (m_pTarget == NULL && m_pShip->IsDestinyTime(13))
				m_pTarget = m_pShip->GetNearestVisibleEnemy(ATTACK_AT_WILL_RANGE, true);

			//	If we have a target, attack

			if (m_pTarget)
				{
				m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget);
				m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);
				}

			//	Otherwise, stay in formation

			else
				{
				ImplementKeepFormation();
				m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
				ImplementFireOnNearestTarget();
				}

			break;
			}

		case stateAttackInFormation:
			{
			bool bInFormation;
			ImplementKeepFormation(&bInFormation);

			//	Fire the primary weapon (most of the time)

			if (bInFormation && mathRandom(1, 3) > 1)
				m_AICtx.ImplementFireWeapon(m_pShip);

			//	Decrement counter

			if (m_iCounter > 0)
				{
				if (--m_iCounter == 0)
					{
					SetState(stateKeepFormation);
					m_iCounter = 0;
					}
				}

			break;
			}

		case stateAttackOnPatrol:
			ASSERT(m_pTarget);
			m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);

			//	Check to see if we've wandered outside our patrol zone

			if (m_pShip->IsDestinyTime(20))
				{
				CSpaceObject *pCenter = GetCurrentOrderTarget();
				Metric rMaxRange2 = PATROL_ORBIT_DIST + PATROL_SENSOR_RANGE;
				rMaxRange2 = rMaxRange2 * rMaxRange2;
				Metric rMinRange2 = max(0.0, PATROL_ORBIT_DIST - PATROL_SENSOR_RANGE);
				rMinRange2 = rMinRange2 * rMinRange2;

				int iTick = m_pShip->GetSystem()->GetTick();
				CVector vRange = pCenter->GetPos() - m_pShip->GetPos();
				Metric rDistance2 = vRange.Dot(vRange);

				//	If we're outside of our patrol range then stop the attack

				if (rDistance2 > rMaxRange2 || rDistance2 < rMinRange2)
					SetState(stateNone);
				}
			break;

		case stateAttackTarget:
			{
			ASSERT(m_pTarget);
			if (m_pTarget->CanMove())
				m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget);
			else
				{
				//	Attack the target as best we can

				CVector vTarget = m_pTarget->GetPos() - m_pShip->GetPos();
				Metric rTargetDist2 = vTarget.Dot(vTarget);
				m_AICtx.ImplementFireWeaponOnTarget(m_pShip, -1, -1, m_pTarget, vTarget, rTargetDist2);

				//	Maneuver to a proper position near the target

				Metric rRange;
				int iCounterAdj;
				if (m_AICtx.GetCombatStyle() == aicombatStandOff)
					{
					rRange = m_AICtx.GetBestWeaponRange() / 2.0;
					iCounterAdj = 2;
					}
				else
					{
					rRange = m_AICtx.GetBestWeaponRange() / 6.0;
					iCounterAdj = 1;
					}

				if (m_iCounter == 0)
					{
					m_vVector = ComputeAttackPos(m_pTarget, rRange, &m_iAngle);
					m_iCounter = mathRandom(130, 160) * iCounterAdj;
					}
				else
					m_iCounter--;

				m_AICtx.ImplementFormationManeuver(m_pShip, m_vVector, NullVector, m_pShip->AlignToRotationAngle(m_iAngle));
				}

			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);
			break;
			}

		case stateKeepFormation:
			ImplementKeepFormation();
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
			ImplementFireOnNearestTarget();
			break;

		case stateOnCourseForDocking:
			ASSERT(m_pDest);
			m_AICtx.ImplementDocking(m_pShip, m_pDest);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
			ImplementFireOnNearestTarget();
			break;

		case stateOnCourseForStargate:
			ASSERT(m_pDest);
			m_AICtx.ImplementGating(m_pShip, m_pDest);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
			ImplementFireOnNearestTarget();
			break;

		case stateOnPatrolOrbit:
			ASSERT(m_pDest);
			m_AICtx.ImplementOrbit(m_pShip, m_pDest, PATROL_ORBIT_DIST);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, m_AICtx.GetMaxWeaponRange(), &m_pTarget, m_pDest);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, NULL, m_pDest);

			//	Check to see if any enemy ships appear

			if (m_pShip->IsDestinyTime(30))
				{
				CSpaceObject *pPrincipal = GetCurrentOrderTarget();
				CSpaceObject *pTarget = CalcEnemyShipInRange(pPrincipal, PATROL_SENSOR_RANGE, m_pDest);
				if (pTarget)
					{
					SetState(stateAttackOnPatrol);
					m_pTarget = pTarget;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					}
				}
			break;

		case stateWaiting:
			m_AICtx.ImplementHold(m_pShip);

			if (m_pTarget)
				m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget, true);

			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
			break;
		}
	}

void CFleetShipAI::BehaviorStart (void)

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

		case IShipController::orderAttackNearestEnemy:
			{
			CSpaceObject *pTarget = m_pShip->GetNearestVisibleEnemy(ATTACK_AT_WILL_RANGE, true);
			if (pTarget)
				{
				SetState(stateAttackTarget);
				m_pTarget = pTarget;
				m_iCounter = 0;
				}
			else
				CancelCurrentOrder();
			break;
			}

		case IShipController::orderDestroyTarget:
			{
			SetState(stateAttackTarget);
			m_pTarget = GetCurrentOrderTarget();
			m_iCounter = 0;
			ASSERT(m_pTarget);
			break;
			}

		case IShipController::orderDock:
			{
			CSpaceObject *pDest = GetCurrentOrderTarget();
			ASSERT(pDest);

			//	If we're docked with our destination then we're done.

			if (m_pShip->GetDockedObj() == pDest)
				CancelCurrentOrder();

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
			SetState(stateKeepFormation);

			m_pLeader = GetCurrentOrderTarget();
			ASSERT(m_pLeader);

			m_iFormation = (int)HIWORD(GetCurrentOrderData());
			m_iPlace = (int)LOWORD(GetCurrentOrderData());
			ASSERT(m_iFormation < FORMATIONS_COUNT);

			//	If there is no place for this ship in the formation, then
			//	gate-out

			if (m_iFormation >= FORMATIONS_COUNT
					|| m_iPlace >= g_Formations[m_iFormation].iCount
					|| m_pLeader == NULL)
				{
				CancelCurrentOrder();
				break;
				}

			//	Keep formation

			m_pShip->Communicate(m_pLeader, msgEscortReportingIn, m_pShip);
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
				SetState(stateOnCourseForStargate);
				m_pDest = pGate;
				}

			break;
			}

		case IShipController::orderPatrol:
		case IShipController::orderGuard:
			{
			SetState(stateOnPatrolOrbit);
			m_pDest = GetCurrentOrderTarget();
			ASSERT(m_pDest);
			break;
			}
		}
	}

CVector CFleetShipAI::ComputeAttackPos (CSpaceObject *pTarget, Metric rDist, int *retiFacing)

//	ComputeAttackPos
//
//	Computes a random point to attack a static target

	{
	int iAngle = m_pShip->AlignToRotationAngle(mathRandom(0, 359));

	if (retiFacing)
		*retiFacing = (iAngle + 180) % 360;

	Metric rVariation = mathRandom(90, 110) / 100.0f;
	return pTarget->GetPos() + PolarToVector(iAngle, rVariation * rDist);
	}

CString CFleetShipAI::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Returns debug crash info

	{
	CString sResult;

	sResult.Append(CONSTLIT("CFleetShipAI\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("Order: %d\r\n"), (int)GetCurrentOrder()));
	sResult.Append(strPatternSubst(CONSTLIT("m_State: %d\r\n"), m_State));
	sResult.Append(strPatternSubst(CONSTLIT("m_pLeader: %s\r\n"), CSpaceObject::DebugDescribe(m_pLeader)));
	sResult.Append(strPatternSubst(CONSTLIT("m_pDest: %s\r\n"), CSpaceObject::DebugDescribe(m_pDest)));
	sResult.Append(strPatternSubst(CONSTLIT("m_pTarget: %s\r\n"), CSpaceObject::DebugDescribe(m_pTarget)));

	return sResult;
	}

CSpaceObject *CFleetShipAI::GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget) const

//	GetTarget
//
//	Returns the target for this ship

	{
	return m_pTarget;
	}

void CFleetShipAI::ImplementFireOnNearestTarget (void)

//	ImplementFireOnNearestTarget
//
//	Fire on the nearest target

	{
	if (m_pShip->IsDestinyTime(20))
		m_pTarget = m_pShip->GetNearestVisibleEnemy();

	if (m_pTarget)
		{
		bool bOutOfRange;

		m_AICtx.ImplementFireOnTarget(m_pShip, m_pTarget, &bOutOfRange);
		if (bOutOfRange)
			m_pTarget = NULL;
		}
	}

void CFleetShipAI::ImplementKeepFormation (bool *retbInFormation)

//	ImplementKeepFormation
//
//	Stay in formation

	{
	if (m_pLeader == NULL)
		{
		if (retbInFormation)
			*retbInFormation = true;
		return;
		}
	
	SFormationPlace *pFormation = g_Formations[m_iFormation].pFormation;

	//	Figure out where our place in the formation is

	int iAngle = pFormation[m_iPlace].iAngle;
	int iDist = pFormation[m_iPlace].iDistance;
	CVector vDest = m_pLeader->GetPos() 
			+ PolarToVector(m_pLeader->GetRotation() + iAngle, iDist * g_KlicksPerPixel);

	CVector vDestVel = m_pLeader->GetVel();

	//	Form up

	m_AICtx.ImplementFormationManeuver(m_pShip, vDest, vDestVel, m_pShip->AlignToRotationAngle(m_pLeader->GetRotation()), retbInFormation);
	}

DWORD CFleetShipAI::OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	Communicate
//
//	Handle communications from other objects

	{
	switch (iMessage)
		{
		case msgAbort:
			{
			if (m_State == stateAttackTarget
					|| m_State == stateAttackAtWill
					|| m_State == stateAttackInFormation)
				SetState(stateNone);
			return resAck;
			}

		case msgAttack:
			{
			if (GetCurrentOrder() == IShipController::orderEscort)
				{
				SetState(stateAttackTarget);
				m_pTarget = pParam1;
				m_iCounter = 0;
				ASSERT(m_pTarget);
				return resAck;
				}
			else
				return resNoAnswer;
			}

		case msgAttackInFormation:
			{
			if (GetCurrentOrder() == IShipController::orderEscort)
				{
				SetState(stateAttackInFormation);
				m_iCounter = dwParam2;
				return resAck;
				}
			else
				return resNoAnswer;
			}

		case msgBreakAndAttack:
			{
			if (GetCurrentOrder() == IShipController::orderEscort)
				{
				SetState(stateAttackAtWill);
				return resAck;
				}
			else
				return resNoAnswer;
			}

		case msgFormUp:
			{
			if (GetCurrentOrder() == IShipController::orderEscort)
				{
				if (dwParam2 != 0xffffffff)
					SetCurrentOrderData(SData(dwParam2));

				SetState(stateNone);
				return resAck;
				}
			else
				return resNoAnswer;
			}

		case msgQueryCommunications:
			{
			if (GetCurrentOrder() == IShipController::orderEscort && GetCurrentOrderTarget() == pSender)
				{
				DWORD dwRes = (resCanBeInFormation | resCanBreakAndAttack);
				if (pSender->GetTarget(CItemCtx(), true))
					dwRes |= resCanAttack;

				if (m_State == stateAttackTarget
						|| m_State == stateAttackAtWill
						|| m_State == stateAttackInFormation)
					dwRes |= resCanAbortAttack;
				else if (m_State == stateKeepFormation)
					dwRes |= resCanAttackInFormation;

				if (m_State != stateWaiting)
					dwRes |= resCanWait;

				return dwRes;
				}
			else
				return resNoAnswer;
			}

		case msgQueryEscortStatus:
		case msgQueryFleetStatus:
			{
			if (GetEscortPrincipal() == pParam1)
				return resAck;
			else
				return resNoAnswer;
			}

		case msgQueryWaitStatus:
			return (m_State == stateWaiting ? resAck : resNoAnswer);

		case msgWait:
			{
			if (GetCurrentOrder() == IShipController::orderEscort)
				{
				SetState(stateWaiting);
				return resAck;
				}
			else
				return resNoAnswer;
			}

		default:
			return resNoAnswer;
		}
	}

void CFleetShipAI::OnDockedEvent (CSpaceObject *pObj)

//	OnDockedEvent
//
//	Event when the ship is docked

	{
	SetState(stateNone);
	}

CSpaceObject *CFleetShipAI::OnGetBase (void) const

//	OnGetBase
//
//	Returns the base for this ship.

	{
	switch (GetCurrentOrder())
		{
		case IShipController::orderPatrol:
		case IShipController::orderGuard:
			return GetCurrentOrderTarget();

		default:
			return NULL;
		}
	}

void CFleetShipAI::OnObjDestroyedNotify (const SDestroyCtx &Ctx)

//	OnObjDestroyedNotify
//
//	Deal with an object that has been destroyed

	{
	switch (GetCurrentOrder())
		{
		case IShipController::orderEscort:
			if (Ctx.pObj == GetCurrentOrderTarget())
				{
				CancelCurrentOrder();

				//	Get the orders of the leader

				IShipController::OrderTypes iLeaderOrders = IShipController::orderNone;
				CSpaceObject *pLeaderTarget = NULL;
				if (Ctx.pObj && Ctx.pObj->GetCategory() == CSpaceObject::catShip)
					{
					CShip *pLeader = Ctx.pObj->AsShip();
					if (pLeader)
						iLeaderOrders = pLeader->GetController()->GetCurrentOrderEx(&pLeaderTarget);
					}

				//	Avenge the leader

				int iAvengeChance = (pLeaderTarget ? 40 : 100);
				CSpaceObject *pAttacker = Ctx.Attacker.GetObj();
				if (pAttacker
						&& Ctx.Attacker.IsCausedByNonFriendOf(m_pShip) 
						&& pAttacker != pLeaderTarget
						&& mathRandom(1, 100) <= iAvengeChance)
					AddOrder(IShipController::orderDestroyTarget, pAttacker, IShipController::SData());

				//	Take on leader's orders

				switch (iLeaderOrders)
					{
					case IShipController::orderDestroyTarget:
					case IShipController::orderGuard:
						if (pLeaderTarget)
							AddOrder(iLeaderOrders, pLeaderTarget, IShipController::SData());
						break;
					}

				//	Attack other enemies

				AddOrder(IShipController::orderAttackNearestEnemy, NULL, IShipController::SData());
				}
			break;

		case IShipController::orderDock:
		case IShipController::orderDestroyTarget:
		case IShipController::orderPatrol:
		case IShipController::orderGuard:
			if (Ctx.pObj == GetCurrentOrderTarget())
				CancelCurrentOrder();
			break;
		}

	//	If our target gets destroyed...

	switch (m_State)
		{
		case stateAttackTarget:
		case stateAttackOnPatrol:
			if (Ctx.pObj == m_pTarget)
				SetState(stateNone);
			break;
		}

	//	Reset

	if (m_pDest == Ctx.pObj)
		m_pDest = NULL;
	if (m_pTarget == Ctx.pObj)
		m_pTarget = NULL;
	}

void CFleetShipAI::OnOrderChanged (void)

//	OnOrderChanged
//
//	The order list has changed

	{
	SetState(stateNone);
	m_pLeader = NULL;
	m_pDest = NULL;
	}

void CFleetShipAI::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read our data
//
//	DWORD		m_State
//	DWORD		m_pLeader (CSpaceObject ref)
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pDest (CSpaceObject ref)
//	DWORD		m_iFormation
//	DWORD		m_iPlace
//	DWORD		m_iCounter
//	DWORD		m_iAngle
//	DWORD		m_vVector

	{
	Ctx.pStream->Read((char *)&m_State, sizeof(DWORD));
	CSystem::ReadObjRefFromStream(Ctx, &m_pLeader);
	CSystem::ReadObjRefFromStream(Ctx, &m_pTarget);
	CSystem::ReadObjRefFromStream(Ctx, &m_pDest);
	Ctx.pStream->Read((char *)&m_iFormation, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iPlace, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iCounter, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iAngle, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_vVector, sizeof(CVector));
	}

void CFleetShipAI::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write our data
//
//	DWORD		m_State
//	DWORD		m_pLeader (CSpaceObject ref)
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pDest (CSpaceObject ref)
//	DWORD		m_iFormation
//	DWORD		m_iPlace
//	DWORD		m_iCounter
//	DWORD		m_iAngle
//	DWORD		m_vVector

	{
	pStream->Write((char *)&m_State, sizeof(DWORD));
	m_pShip->WriteObjRefToStream(m_pLeader, pStream);
	m_pShip->WriteObjRefToStream(m_pTarget, pStream);
	m_pShip->WriteObjRefToStream(m_pDest, pStream);
	pStream->Write((char *)&m_iFormation, sizeof(DWORD));
	pStream->Write((char *)&m_iPlace, sizeof(DWORD));
	pStream->Write((char *)&m_iCounter, sizeof(DWORD));
	pStream->Write((char *)&m_iAngle, sizeof(DWORD));
	pStream->Write((char *)&m_vVector, sizeof(CVector));
	}

void CFleetShipAI::SetState (StateTypes State)

//	SetState
//
//	Sets the current state

	{
	//	Set state (NOTE: We do this before we undock because the Undock
	//	call may destroy the station and cause us to recurse into SetState.
	//	This happens when a ship is guarding an empty cargo crate).

	m_State = State;
	m_pTarget = NULL;

	//	If we're currently docked and we're changing to a state that
	//	does not support docking, then we undock first.

	if (m_pShip->GetDockedObj()	
			&& State != stateNone)
		m_pShip->Undock();
	}

