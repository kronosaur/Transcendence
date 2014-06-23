//	CAutonAI.cpp
//
//	CAutonAI class

#include "PreComp.h"

#define PATROL_SENSOR_RANGE				(g_KlicksPerPixel * 1000.0)
#define MAX_ESCORT_DISTANCE				(g_KlicksPerPixel * 400.0)
#define MAX_FOLLOW_DISTANCE				(g_KlicksPerPixel * 350.0)
#define ATTACK_RANGE					(g_KlicksPerPixel * 600.0)

static CObjectClass<CAutonAI>g_Class(OBJID_CAUTONAI, NULL);

CAutonAI::CAutonAI (void) : CBaseShipAI(&g_Class),
		m_State(stateNone),
		m_pDest(NULL),
		m_pTarget(NULL)

//	CAutonAI constructor

	{
	}

void CAutonAI::Behavior (void)

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
			ASSERT(m_pTarget);
			m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip, m_pTarget);
			break;

		case stateEscorting:
			m_AICtx.ImplementEscort(m_pShip, m_pDest, &m_pTarget);
			break;

		case stateFollowing:
			{
			ASSERT(m_pDest);
			CVector vTarget = m_pDest->GetPos() - m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);
			Metric rMaxDist = (MAX_FOLLOW_DISTANCE) + (g_KlicksPerPixel * (m_pShip->GetDestiny() % 120));

			if (rTargetDist2 > (rMaxDist * rMaxDist))
				m_AICtx.ImplementCloseOnTarget(m_pShip, m_pDest, vTarget, rTargetDist2);
			else if (rTargetDist2 < (g_KlicksPerPixel * g_KlicksPerPixel * 1024.0))
				m_AICtx.ImplementSpiralOut(m_pShip, vTarget);
			else
				m_AICtx.ImplementStop(m_pShip);

			if (m_pTarget)
				m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget, true);

			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
			break;
			}

		case stateOnCourseForStargate:
			m_AICtx.ImplementGating(m_pShip, m_pDest);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
			break;

		case stateWaiting:
			m_AICtx.ImplementHold(m_pShip);

			if (m_pTarget)
				m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget, true);

			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
			break;

		default:
			ASSERT(false);
		}
	}

void CAutonAI::BehaviorStart (void)

//	BehaviorStart
//
//	Initiate behavior state based on orders

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

		case IShipController::orderWaitForPlayer:
			{
			SetState(stateWaiting);
			break;
			}
		}
	}

CString CAutonAI::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Returns debug crash info

	{
	CString sResult;

	sResult.Append(CONSTLIT("CAutonAI\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("Order: %d\r\n"), (int)GetCurrentOrder()));
	sResult.Append(strPatternSubst(CONSTLIT("m_State: %d\r\n"), m_State));
	sResult.Append(strPatternSubst(CONSTLIT("m_pDest: %s\r\n"), CSpaceObject::DebugDescribe(m_pDest)));
	sResult.Append(strPatternSubst(CONSTLIT("m_pTarget: %s\r\n"), CSpaceObject::DebugDescribe(m_pTarget)));

	return sResult;
	}

CSpaceObject *CAutonAI::GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget) const

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

DWORD CAutonAI::OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	Communicate
//
//	Handle communications from other objects

	{
	switch (iMessage)
		{
		case msgAttack:
		case msgAttackDeter:
			{
			if (GetCurrentOrder() == IShipController::orderEscort
					&& !m_AICtx.IsNonCombatant())
				{
				SetState(stateAttackingTarget);
				m_pTarget = pParam1;
				return resAck;
				}
			else
				return resNoAnswer;
			}

		case msgAbort:
			{
			SetState(stateNone);
			return resAck;
			}

		case msgFormUp:
			{
			if (m_State == stateWaiting || m_State == stateAttackingTarget)
				{
				SetState(stateNone);
				return resAck;
				}
			else
				return resNoAnswer;
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

void CAutonAI::OnObjDestroyedNotify (const SDestroyCtx &Ctx)

//	OnObjDestroyedNotify
//
//	Handle the case where another object is destroyed

	{
	//	If our target gets destroyed...

	if (m_State == stateAttackingTarget)
		if (Ctx.pObj == m_pTarget)
			SetState(stateNone);

	//	Reset

	if (m_pDest == Ctx.pObj)
		m_pDest = NULL;
	if (m_pTarget == Ctx.pObj)
		m_pTarget = NULL;
	}

void CAutonAI::OnOrderChanged (void)

//	OnOrderChanged
//
//	The order list has changed

	{
	SetState(stateNone);
	}

void CAutonAI::OnReadFromStream (SLoadCtx &Ctx)

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

void CAutonAI::OnSystemLoadedNotify (void)

//	OnSystemLoadedNotify
//
//	Deal with stuff after the system is loaded.

	{
#ifdef DEBUG
	//	Due to a bug in previous versions, we ended up with m_pDest NULL.
	//	I'm still not sure how this happened, but it could have been due
	//	to a bug with not being able to save objects out of the system.
	//
	//	See:
	//	http://multiverse.kronosaur.com/crashReport.hexm?clientVersion=Transcendence%2fWindows%2f1.1&reportedOn=2013-11-26T09%3a19%3a51.0976
	//	http://multiverse.kronosaur.com/crashReport.hexm?clientVersion=Transcendence%2fWindows%2f1.1&reportedOn=2013-11-26T08%3a58%3a43.0510

	if (m_State == stateOnCourseForStargate
			&& m_pDest == NULL)
		m_pDest = m_pShip->GetNearestStargate();
#endif
	}

void CAutonAI::OnWriteToStream (IWriteStream *pStream)

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

void CAutonAI::SetState (StateTypes State)

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
