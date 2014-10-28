//	CBaseShipAI.cpp
//
//	CBaseShipAI class

#include "PreComp.h"

const Metric MAX_AREA_WEAPON_CHECK =	(15.0 * LIGHT_SECOND);
const Metric MAX_AREA_WEAPON_CHECK2 =	(MAX_AREA_WEAPON_CHECK * MAX_AREA_WEAPON_CHECK);
const Metric MIN_TARGET_DIST =			(5.0 * LIGHT_SECOND);
const Metric MIN_TARGET_DIST2 =			(MIN_TARGET_DIST * MIN_TARGET_DIST);
const Metric MIN_STATION_TARGET_DIST =	(10.0 * LIGHT_SECOND);
const Metric MIN_STATION_TARGET_DIST2 =	(MIN_STATION_TARGET_DIST * MIN_STATION_TARGET_DIST);
const Metric HIT_NAV_POINT_DIST =		(8.0 * LIGHT_SECOND);
const Metric HIT_NAV_POINT_DIST2 =		(HIT_NAV_POINT_DIST * HIT_NAV_POINT_DIST);
const Metric MAX_TARGET_OF_OPPORTUNITY_RANGE = (20.0 * LIGHT_SECOND);
const Metric ESCORT_DISTANCE =			(6.0 * LIGHT_SECOND);
const Metric MAX_ESCORT_DISTANCE =		(12.0 * LIGHT_SECOND);
const Metric ATTACK_RANGE =				(20.0 * LIGHT_SECOND);
const Metric CLOSE_RANGE =				(50.0 * LIGHT_SECOND);
const Metric CLOSE_RANGE2 =				(CLOSE_RANGE * CLOSE_RANGE);
const Metric MIN_POTENTIAL2 =			(KLICKS_PER_PIXEL * KLICKS_PER_PIXEL * 25.0);

#define MAX_TARGETS						10
#define MAX_DOCK_DISTANCE				(15.0 * LIGHT_SECOND)
#define MAX_GATE_DISTANCE				(g_KlicksPerPixel * 32)
#define DOCKING_APPROACH_DISTANCE		(g_KlicksPerPixel * 200)
#define DEFAULT_DIST_CHECK				(700.0 * g_KlicksPerPixel)
#define MIN_FLYBY_SPEED					(2.0 * g_KlicksPerPixel)
#define MIN_FLYBY_SPEED2				(MIN_FLYBY_SPEED * MIN_FLYBY_SPEED)

#define MAX_DELTA						(2.0 * g_KlicksPerPixel)
#define MAX_DELTA2						(MAX_DELTA * MAX_DELTA)
#define MAX_DELTA_VEL					(g_KlicksPerPixel / 2.0)
#define MAX_DELTA_VEL2					(MAX_DELTA_VEL * MAX_DELTA_VEL)
#define MAX_DISTANCE					(400 * g_KlicksPerPixel)
#define MAX_IN_FORMATION_DELTA			(2.0 * g_KlicksPerPixel)
#define MAX_IN_FORMATION_DELTA2			(MAX_IN_FORMATION_DELTA * MAX_IN_FORMATION_DELTA)

#ifdef DEBUG_COMBAT
#define DEBUG_COMBAT_OUTPUT(x)			if (m_pShip->IsSelected()) g_pUniverse->DebugOutput("%d> %s", g_iDebugLine++, x)
#else
#define DEBUG_COMBAT_OUTPUT(x)
#endif

extern int g_iDebugLine;

CBaseShipAI::CBaseShipAI (IObjectClass *pClass) : CObject(pClass),
		m_pShip(NULL),
		m_pCommandCode(NULL),
		m_pOrderModule(NULL),
		m_fDeviceActivate(false),
		m_fInOnOrderChanged(false),
		m_fInOnOrdersCompleted(false),
		m_fCheckedForWalls(false),
		m_fAvoidWalls(false),
		m_fIsPlayerWingman(false),
		m_fOldStyleBehaviors(false)

//	CBaseShipAI constructor

	{
	}

CBaseShipAI::~CBaseShipAI (void)

//	CBaseShipAI destructor

	{
	if (m_pCommandCode)
		m_pCommandCode->Discard(&(g_pUniverse->GetCC()));

	if (m_pOrderModule)
		delete m_pOrderModule;
	}

void CBaseShipAI::AddOrder (OrderTypes Order, CSpaceObject *pTarget, const IShipController::SData &Data, bool bAddBefore)

//	AddOrder
//
//	Add an order to the list

	{
	ASSERT(pTarget == NULL || pTarget->NotifyOthersWhenDestroyed());

	//	If the order requires a target, make sure we have one

	bool bRequired;
	if (pTarget == NULL && OrderHasTarget(Order, &bRequired) && bRequired)
		{
		ASSERT(false);
		return;
		}

	//	If we're escorting someone, make sure that we are not already
	//	in the escort chain

	if (Order == orderEscort || Order == orderFollow)
		{
		CSpaceObject *pLeader = pTarget;
		int iLoops = 20;
		while (pLeader && pLeader != m_pShip && iLoops-- > 0)
			{
			CShip *pShip = pLeader->AsShip();
			if (pShip)
				{
				CSpaceObject *pNextLeader;
				OrderTypes iOrder = pShip->GetController()->GetCurrentOrderEx(&pNextLeader);
				if (iOrder == orderEscort || iOrder == orderFollow)
					pLeader = pNextLeader;
				else
					pLeader = NULL;
				}
			else
				pLeader = NULL;
			}

		//	We are in the chain

		if (pLeader == m_pShip)
			return;
		}

	//	Add the order

	bool bChanged = (bAddBefore || m_Orders.GetCount() == 0);
	m_Orders.Insert(Order, pTarget, Data, bAddBefore);
	if (bChanged)
		FireOnOrderChanged();
	}

void CBaseShipAI::Behavior (void)

//	Behavior
//
//	Fly, fight, die.
//
//	NOTE: Most custom controllers override this. Only the standard controller 
//	uses this. [And eventually, once we use order modules for everything,
//	we should be able to delete the standard controller.]

	{
	//	Reset

	ResetBehavior();

	//	Use basic items

	UseItemsBehavior();

	//	If we don't have an order then we need to come up with something.

	IShipController::OrderTypes iOrder = GetCurrentOrder();
	if (iOrder == IShipController::orderNone)
		{
		//	If we've got command code, execute it now

		if (m_pCommandCode)
			{
			CCodeChainCtx Ctx;
			Ctx.SaveAndDefineSourceVar(m_pShip);

			ICCItem *pResult = Ctx.RunLambda(m_pCommandCode);
			if (pResult->IsError())
				m_pShip->ReportEventError(CONSTLIT("CommandCode"), pResult);
			Ctx.Discard(pResult);
			}

		//	If we still got no orders, let the ship class deal with this

		if ((iOrder = GetCurrentOrder()) == IShipController::orderNone)
			FireOnOrdersCompleted();

		//	If we've still got no orders, we need to come up with something

		if ((iOrder = GetCurrentOrder()) == IShipController::orderNone)
			{
			iOrder = ((m_pShip->GetDockedObj() == NULL) ? IShipController::orderGate : IShipController::orderWait);
			AddOrder(iOrder, NULL, IShipController::SData());
			}
		}

	ASSERT(iOrder != IShipController::orderNone);

	//	If we're using old style behavior, then just call our derrived class to
	//	handle it.

	if (m_fOldStyleBehaviors)
		{
		OnBehavior();

		//	This method is incompatible with order modules so we just return here.

		return;
		}

	//	If we don't have an order module, see if we can create one from the order

	if (m_pOrderModule == NULL)
		{
		m_pOrderModule = IOrderModule::Create(iOrder);
		
		//	If there is no module for this order then we need to do the old-
		//	style behavior.

		if (m_pOrderModule == NULL)
			{
			m_fOldStyleBehaviors = true;
			OnBehavior();
			return;
			}

		//	Tell our descendants to clean up. We need to do this because we don't
		//	want our descendants to hold on to object pointers (since the order
		//	module will handle everything, including getting destroyed object
		//	notifications).

		OnCleanUp();

		//	Initialize order module

		CSpaceObject *pTarget;
		SData Data;
		GetCurrentOrderEx(&pTarget, &Data);
		m_pOrderModule->BehaviorStart(m_pShip, m_AICtx, pTarget, Data);

		//	NOTE: We might have cancelled the order inside BehaviorStart, so we
		//	return in that case.

		if (m_pOrderModule == NULL)
			return;
		}

	//	Implement orders

	m_pOrderModule->Behavior(m_pShip, m_AICtx);
	}

CSpaceObject *CBaseShipAI::CalcEnemyShipInRange (CSpaceObject *pCenter, Metric rRange, CSpaceObject *pExcludeObj)

//	CalcEnemyShipInRange
//
//	Returns the first enemy ship that it finds in range of pCenter. Returns NULL if none
//	are found.

	{
	DEBUG_TRY

	int i;
	Metric rMaxRange2 = rRange * rRange;

	//	Compute this object's perception and perception range

	int iPerception = m_pShip->GetPerception();
	Metric rRange2[RANGE_INDEX_COUNT];
	for (i = 0; i < RANGE_INDEX_COUNT; i++)
		{
		rRange2[i] = RangeIndex2Range(i);
		rRange2[i] = rRange2[i] * rRange2[i];
		}

	//	The player is a special case (because sometimes a station is angry at the 
	//	player even though she is not an enemy)

	CSpaceObject *pPlayer = m_pShip->GetPlayer();
	if (pPlayer 
			&& pCenter->IsAngryAt(pPlayer)
			&& pPlayer != pExcludeObj
			&& !pPlayer->IsEscortingFriendOf(m_pShip))
		{
		CVector vRange = pPlayer->GetPos() - pCenter->GetPos();
		Metric rDistance2 = vRange.Dot(vRange);

		if (rDistance2 < rMaxRange2
				&& rDistance2 < rRange2[pPlayer->GetDetectionRangeIndex(iPerception)])
			return pPlayer;
		}

	//	Get the sovereign

	CSovereign *pSovereign = m_pShip->GetSovereignToDefend();
	if (pSovereign == NULL || m_pShip->GetSystem() == NULL)
		return NULL;

	//	Loop

	const CSpaceObjectList &ObjList = pSovereign->GetEnemyObjectList(m_pShip->GetSystem());
	int iCount = ObjList.GetCount();
	for (i = 0; i < iCount; i++)
		{
		CSpaceObject *pObj = ObjList.GetObj(i);

		if (pObj->GetCategory() == CSpaceObject::catShip
				&& pObj->CanAttack()
				&& pObj != m_pShip)
			{
			CVector vRange = pObj->GetPos() - pCenter->GetPos();
			Metric rDistance2 = vRange.Dot(vRange);

			if (rDistance2 < rMaxRange2
					&& rDistance2 < rRange2[pObj->GetDetectionRangeIndex(iPerception)]
					&& pObj != pExcludeObj
					&& !pObj->IsEscortingFriendOf(m_pShip))
				return pObj;
			}
		}

	return NULL;

	DEBUG_CATCH_CONTINUE

	CSpaceObject *pPlayer = m_pShip->GetPlayer();
	::kernelDebugLogMessage("Player Ship: %s", CSpaceObject::DebugDescribe(pPlayer));

	CSovereign *pSovereign = m_pShip->GetSovereignToDefend();
	if (pSovereign)
		{
		int i;

		::kernelDebugLogMessage("Sovereign: %x", pSovereign->GetUNID());

		const CSpaceObjectList &ObjList = pSovereign->GetEnemyObjectList(m_pShip->GetSystem());
		for (i = 0; i < ObjList.GetCount(); i++)
			::kernelDebugLogMessage("Enemy Obj %d: %s", i, CSpaceObject::DebugDescribe(ObjList.GetObj(i)));
		}
	else
		::kernelDebugLogMessage("Sovereign: none");

	throw CException(ERR_FAIL);
	}

Metric CBaseShipAI::CalcShipIntercept (const CVector &vRelPos, const CVector &vAbsVel, Metric rMaxSpeed)

//	CalcShipIntercept
//
//	Calculates (heuristically) the time that it will take us
//	to intercept at target at vRelPos (relative to ship) and moving
//	at vAbsVel (in absolute terms). If we return < 0.0 then we
//	cannot intercept even at maximum velocity.

	{
	CVector vInterceptPoint;

	//	Compute the speed with which the target is closing
	//	the distance (this may be a negative number). Note
	//	that we care about the target's absolute velocity
	//	(not its relative velocity because we are trying to
	//	adjust our velocity).

	Metric rClosingSpeed = -vAbsVel.Dot(vRelPos.Normal());

	//	Figure out how long it will take to overtake the target's
	//	current position at maximum speed. (This is just a heuristic
	//	that gives us an approximation of the length of an intercept
	//	course.)

	rClosingSpeed = rClosingSpeed + rMaxSpeed;
	if (rClosingSpeed > 0.0)
		return vRelPos.Length() / (rClosingSpeed);
	else
		return -1.0;
	}

void CBaseShipAI::CancelAllOrders (void)

//	CancelAllOrders
//
//	Cancel orders

	{
	m_AICtx.ClearNavPath();
	m_Orders.DeleteAll();
	FireOnOrderChanged();
	}

void CBaseShipAI::CancelCurrentOrder (void)

//	CancelCurrentOrder
//
//	Cancels the current order

	{
	if (m_Orders.GetCount() > 0)
		{
		m_Orders.DeleteCurrent();
		FireOnOrderChanged();
		}
	}

void CBaseShipAI::CancelDocking (CSpaceObject *pTarget)

//	CancelDocking
//
//	Cancel docking with the given station

	{
	m_AICtx.CancelDocking(m_pShip, pTarget);
	}

bool CBaseShipAI::CheckForEnemiesInRange (CSpaceObject *pCenter, Metric rRange, int iInterval, CSpaceObject **retpTarget)

//	CheckForEnemiesInRange
//
//	Checks every given interval to see if there are enemy ships in range

	{
	if (m_pShip->IsDestinyTime(iInterval))
		{
		*retpTarget = CalcEnemyShipInRange(pCenter, rRange);

		if (*retpTarget)
			return true;
		else
			return false;
		}
	else
		return false;
	}

bool CBaseShipAI::CheckOutOfRange (CSpaceObject *pTarget, Metric rRange, int iInterval)

//	CheckOutOfRange
//
//	Checks to see if the ship is out of range. Returns TRUE if we are known to be
//	out of range.

	{
	if (m_pShip->IsDestinyTime(iInterval))
		{
		CVector vRange = pTarget->GetPos() - m_pShip->GetPos();
		Metric rDistance2 = vRange.Length2();

		return (rDistance2 > (rRange * rRange));
		}
	else
		return false;
	}

bool CBaseShipAI::CheckOutOfZone (CSpaceObject *pBase, Metric rInnerRadius, Metric rOuterRadius, int iInterval)

//	CheckOutOfZone
//
//	Checks to see if the ship is out of its zone.

	{
	if (m_pShip->IsDestinyTime(iInterval))
		{
		Metric rMaxRange2 = rOuterRadius * rOuterRadius;
		Metric rMinRange2 = rInnerRadius * rInnerRadius;

		CVector vRange = pBase->GetPos() - m_pShip->GetPos();
		Metric rDistance2 = vRange.Length2();

		return (rDistance2 > rMaxRange2 || rDistance2 < rMinRange2);
		}
	else
		return false;
	}

CString CBaseShipAI::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Returns debug crash info

	{
	//	If we have an order module then let it dump its data.

	if (m_pOrderModule)
		return m_pOrderModule->DebugCrashInfo();

	//	Otherwise, let our descendant output

	else
		return OnDebugCrashInfo();
	}

void CBaseShipAI::DebugPaintInfo (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	DebugPaintInfo
//
//	Paint debug information

	{
	m_AICtx.DebugPaintInfo(Dest, x, y, Ctx);
	}

void CBaseShipAI::FireOnOrderChanged (void)

//	FireOnOrderChanged
//
//	Ship's current order has changed

	{
	if (m_pShip->GetClass()->HasOnOrderChangedEvent() && !m_fInOnOrderChanged)
		{
		m_fInOnOrderChanged = true;
 		m_pShip->FireOnOrderChanged();
		m_fInOnOrderChanged = false;
		}

	//	Reset some internal variables

	m_AICtx.SetManeuverCounter(0);

	//	Reset the old style flag because we don't know if the next order
	//	will have an order module or not.

	m_fOldStyleBehaviors = false;

	//	Reset the order module

	if (m_pOrderModule)
		{
		//	If the current order module handles the current order, then we don't have
		//	to reallocate everything; we just need to restart it.

		if (m_pOrderModule->GetOrder() == GetCurrentOrder())
			{
			CSpaceObject *pTarget;
			SData Data;
			GetCurrentOrderEx(&pTarget, &Data);
			m_pOrderModule->BehaviorStart(m_pShip, m_AICtx, pTarget, Data);
			}

		//	Otherwise, we delete the order module and allow it to be recreated.

		else
			{
			delete m_pOrderModule;
			m_pOrderModule = NULL;
			}
		}

	//	Give descendents a chance

	OnOrderChanged();
	}

void CBaseShipAI::FireOnOrdersCompleted (void)

//	FireOnOrdersCompleted
//
//	Ship has no more orders

	{
	if (m_pShip->HasOnOrdersCompletedEvent() && !m_fInOnOrdersCompleted)
		{
		m_fInOnOrdersCompleted = true;
 		m_pShip->FireOnOrdersCompleted();
		m_fInOnOrdersCompleted = false;
		}
	}

bool CBaseShipAI::FollowsObjThroughGate (CSpaceObject *pLeader)

//	FollowsObjThroughGate
//
//	Returns true if we will follow the leader through a gate

	{
	return (GetCurrentOrder() == IShipController::orderFollowPlayerThroughGate);
	}

CSpaceObject *CBaseShipAI::GetBase (void) const

//	GetBase
//
//	Returns this ship's base

	{
	if (m_pOrderModule)
		return m_pOrderModule->GetBase();
	else
		return OnGetBase();
	}

int CBaseShipAI::GetCombatPower (void)

//	GetCombatPower
//
//	Returns the relative strength of this object in combat
//	Scale from 0 to 100 (Note: This works for ships up to
//	level XV).

	{
	int iPower = m_pShip->GetScore() / 100;
	if (iPower > 100)
		return 100;
	else if (iPower == 0)
		return 1;
	else
		return iPower;
	}

IShipController::OrderTypes CBaseShipAI::GetCurrentOrder (void) const

//	GetCurrentOrder
//
//	Returns current order

	{
	return m_Orders.GetCurrentOrder();
	}

IShipController::OrderTypes CBaseShipAI::GetCurrentOrderEx (CSpaceObject **retpTarget, IShipController::SData *retData)

//	GetCurrentOrderEx
//
//	Returns current order and related data

	{
	return m_Orders.GetCurrentOrder(retpTarget, retData);
	}

CSpaceObject *CBaseShipAI::GetEscortPrincipal (void) const

//	GetEscortPrincipal
//
//	Returns the ship that is being escorted (or NULL)

	{
	if (m_fIsPlayerWingman)
		return g_pUniverse->GetPlayer();

	switch (GetCurrentOrder())
		{
		case IShipController::orderEscort:
		case IShipController::orderFollow:
		case IShipController::orderFollowPlayerThroughGate:
			return GetCurrentOrderTarget();

		default:
			return NULL;
		}
	}

void CBaseShipAI::GetWeaponTarget (STargetingCtx &TargetingCtx, CItemCtx &ItemCtx, CSpaceObject **retpTarget, int *retiFireSolution)

//	GetNearestTargets
//
//	Returns a list of nearest targets

	{
	int i;
	CInstalledDevice *pDevice = ItemCtx.GetDevice();
	CDeviceClass *pWeapon = ItemCtx.GetDeviceClass();

	//	Get targets, if necessary

	if (TargetingCtx.bRecalcTargets)
		{
		TargetingCtx.Targets.DeleteAll();

		//	If we are aggressive, then include ships that haven't fired 
		//	their weapons recently

		DWORD dwFlags = 0;
		if (m_AICtx.IsAggressor())
			dwFlags |= FLAG_INCLUDE_NON_AGGRESSORS;

		//	First build a list of the nearest enemy ships within
		//	range of the ship.

		m_pShip->GetNearestVisibleEnemies(MAX_TARGETS,
				m_AICtx.GetBestWeaponRange(),
				&TargetingCtx.Targets,
				GetBase(),
				dwFlags);

		//	If we've got a target, add it to the list. Sometimes this will be 
		//	a duplicate, but that's OK.

		CSpaceObject *pTarget = GetTarget(ItemCtx, true);
		if (pTarget)
			TargetingCtx.Targets.Insert(pTarget);

		TargetingCtx.bRecalcTargets = false;
		}

	//	Now find a target for the given weapon.

	Metric rMaxRange = pDevice->GetClass()->GetMaxEffectiveRange(m_pShip, pDevice, NULL);
	Metric rMaxRange2 = rMaxRange * rMaxRange;
	for (i = 0; i < TargetingCtx.Targets.GetCount(); i++)
		{
		int iFireAngle;
		CSpaceObject *pTarget = TargetingCtx.Targets[i];
		Metric rDist2 = (pTarget->GetPos() - m_pShip->GetPos()).Length2();

		if (rDist2 < rMaxRange2 
				&& pDevice->GetWeaponEffectiveness(m_pShip, pTarget) >= 0
				&& pDevice->IsWeaponAligned(m_pShip, pTarget, NULL, &iFireAngle)
				&& m_AICtx.CheckForFriendsInLineOfFire(m_pShip, pDevice, pTarget, iFireAngle, rMaxRange))
			{
			*retpTarget = pTarget;
			*retiFireSolution = iFireAngle;
			return;
			}
		}

	//	If we get this far then no target found

	*retpTarget = NULL;
	*retiFireSolution = -1;
	}

CSpaceObject *CBaseShipAI::GetOrderGiver (void)

//	GetOrderGiver
//
//	Returns the ship that orders this one (if the ship is
//	independent, then we return m_pShip)

	{
	//	If there is no order giver then we are responsible

	if (m_AICtx.NoOrderGiver())
		return m_pShip;

	//	If we're the player's wingman, then the player is
	//	the order-giver.

	if (m_fIsPlayerWingman)
		return GetPlayerOrderGiver();

	//	Figure it out based on the order

	switch (GetCurrentOrder())
		{
		case IShipController::orderEscort:
			return GetCurrentOrderTarget();

		case IShipController::orderFollowPlayerThroughGate:
		case IShipController::orderWaitForPlayer:
			return GetPlayerOrderGiver();

		default:
			return m_pShip;
		}
	}

CSpaceObject *CBaseShipAI::GetPlayerOrderGiver (void) const

//	GetPlayerOrderGiver
//
//	If the player is in the system, returns the player; otherwise
//	we return the ship.

	{
	CSpaceObject *pPlayer = m_pShip->GetPlayer();
	if (pPlayer)
		return pPlayer;
	else
		return m_pShip;
	}

CSpaceObject *CBaseShipAI::GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget) const

//	GetTarget
//
//	Returns the target that this ship is attacking
	
	{
	if (m_pOrderModule)
		return m_pOrderModule->GetTarget();
	else
		return OnGetTarget(bNoAutoTarget);
	}

void CBaseShipAI::HandleFriendlyFire (CSpaceObject *pOrderGiver)

//	HandleFriendlyFire
//
//	Ship has been hit by friendly fire

	{
	//	If the player hit us (and it seems to be on purpose) then raise an event

	if (pOrderGiver->IsPlayer() 
			&& m_Blacklist.Hit(m_pShip->GetSystem()->GetTick())
			&& m_pShip->GetClass()->HasOnAttackedByPlayerEvent())
		m_pShip->FireOnAttackedByPlayer();

	//	Otherwise, send the standard message

	else
		m_pShip->Communicate(pOrderGiver, msgWatchTargets);
	}

bool CBaseShipAI::IsAngryAt (CSpaceObject *pObj) const

//	IsAngryAt
//
//	Returns TRUE if we're angry at the given object

	{
	if (GetTarget(CItemCtx()) == pObj)
		return true;

	switch (GetCurrentOrder())
		{
		case IShipController::orderGuard:
		case IShipController::orderPatrol:
			return GetCurrentOrderTarget()->IsAngryAt(pObj);

		default:
			return false;
		}
	}

bool CBaseShipAI::IsPlayerOrPlayerFollower (CSpaceObject *pObj, int iRecursions)

//	IsPlayerOrPlayerFollower
//
//	Returns TRUE if pObj is the player or some other object that
//	follows the player through gates

	{
	if (pObj->IsPlayer())
		return true;

	//	If we've recursed too much, then we are not the player. This can
	//	happen if there is a bug that makes two ships follow each other

	if (iRecursions > 20)
		return false;

	//	Get the orders for this ship

	CShip *pShip = pObj->AsShip();
	if (pShip)
		{
		IShipController *pController = pShip->GetController();
		
		CSpaceObject *pTarget;
		OrderTypes iOrder = pController->GetCurrentOrderEx(&pTarget);
		switch (iOrder)
			{
			case IShipController::orderFollowPlayerThroughGate:
				return true;

			case IShipController::orderEscort:
			case IShipController::orderFollow:
				return IsPlayerOrPlayerFollower(pTarget, iRecursions + 1);
			}
		}

	return false;
	}

void CBaseShipAI::OnAttacked (CSpaceObject *pAttacker, const DamageDesc &Damage)

//	OnAttacked
//
//	Our ship was attacked. Note that pAttacker can either be an enemy ship/station
//	or it can be the missile/beam that hit us.

	{
	DEBUG_TRY

	bool bFriendlyFire = false;

	if (pAttacker)
		{
		CSpaceObject *pOrderGiver = pAttacker->GetOrderGiver(Damage.GetCause());

		//	If we were attacked by a friend, then warn them off
		//	(Unless we're explicitly targeting the friend)

		if (pOrderGiver 
				&& m_pShip->IsFriend(pOrderGiver) 
				&& !m_pShip->IsAngryAt(pOrderGiver))
			{
			//	We deal with the order giver instead of the attacker because we want to get
			//	at the root problem (the player instead of her autons)
			//
			//	Also, we ignore damage from automated weapons

			if (!Damage.IsAutomatedWeapon())
				HandleFriendlyFire(pOrderGiver);

			bFriendlyFire = true;
			}
		}

	//	Notify our order module (or derrived class if we're doing it old-style)

	if (m_pOrderModule)
		m_pOrderModule->Attacked(m_pShip, m_AICtx, pAttacker, Damage, bFriendlyFire);
	else
		OnAttackedNotify(pAttacker, Damage);

	//	Remember the last time we were attacked (debounce quick hits)

	m_AICtx.SetLastAttack(m_pShip->GetSystem()->GetTick());

	DEBUG_CATCH
	}

DWORD CBaseShipAI::OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	Communicate
//
//	Handle communications from other objects

	{
	if (m_pOrderModule)
		return m_pOrderModule->Communicate(m_pShip, m_AICtx, pSender, iMessage, pParam1, dwParam2);
	else
		return OnCommunicateNotify(pSender, iMessage, pParam1, dwParam2);
	}

void CBaseShipAI::OnDocked (CSpaceObject *pObj)

//	OnDocked
//
//	Event when the ship is docked

	{
	//	NOTE: At SystemCreate time we dock ships to their station without them
	//	requesting it, thus we cannot assume that m_bDockingRequested is TRUE.
	//	(However, we set state to None no matter what).

	m_AICtx.SetDockingRequested(false);

	//	Call descendants

	OnDockedEvent(pObj);
	}

void CBaseShipAI::OnEnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate, bool bAscend)

//	OnEnterGate
//
//	Ship enters a stargate
	
	{
	//	If we're ascending, then leave the ship alone (it will get pulled out of
	//	the system by our caller).

	if (bAscend)
		;

	//	If our orders are to follow the player through a gate, then suspend
	//	until the player removes us from the system in TransferGateFollowers

	else if (GetCurrentOrder() == IShipController::orderFollowPlayerThroughGate)
		m_pShip->Suspend();

	//	Otherwise, we destroy ourselves

	else
		m_pShip->SetDestroyInGate();
	}

void CBaseShipAI::OnNewSystem (CSystem *pSystem)

//	OnNewSystem
//
//	Ship has moved from one system to another

	{
	//	Reset the nav path (because it was for the last system)

	m_AICtx.ClearNavPath();

	//	Let our subclass deal with it

	OnNewSystemNotify();

	//	Loop over all our future orders and make sure that we
	//	delete any that refer objects that are not in the new system
	//
	//	Note that this is called before the player is in the system
	//	but after all other player followers have been transfered.

	bool bChanged;
	m_Orders.OnNewSystem(pSystem, &bChanged);
	if (bChanged)
		FireOnOrderChanged();
	}

void CBaseShipAI::OnObjEnteredGate (CSpaceObject *pObj, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate)

//	OnObjEnteredGate
//
//	Handle the case where another object entered a stargate

	{
	//	If the player has left and we have inter system events, then see if we
	//	need to follow the player

	if (pObj->IsPlayer() && m_pShip->HasInterSystemEvent())
		{
		CSpaceObject::InterSystemResults iResult = m_pShip->FireOnPlayerLeftSystem(pObj, pDestNode, sDestEntryPoint, pStargate);
		switch (iResult)
			{
			case CSpaceObject::interFollowPlayer:
				AddOrder(IShipController::orderFollowPlayerThroughGate, pObj, IShipController::SData(), true);
				break;

			case CSpaceObject::interWaitForPlayer:
				AddOrder(IShipController::orderWaitForPlayer, NULL, IShipController::SData(), true);
				break;
			}
		}

	//	Otherwise, figure out what to do based on our order

	switch (GetCurrentOrder())
		{
		case IShipController::orderEscort:
		case IShipController::orderFollow:
			if (pObj->IsPlayer() && IsPlayerOrPlayerFollower(GetCurrentOrderTarget()))
				{
				//	If the player just entered a stargate then see if we need to follow
				//	her or wait for her to return.

				if (m_pShip->Communicate(m_pShip, msgQueryWaitStatus) == resAck)
					AddOrder(IShipController::orderWaitForPlayer, NULL, IShipController::SData(), true);
				else
					AddOrder(IShipController::orderFollowPlayerThroughGate, pObj, IShipController::SData(), true);
				}
			else if (pObj == GetCurrentOrderTarget())
				{
				//	Otherwise, we cancel our order and follow the ship through the gate

				CancelCurrentOrder();
				AddOrder(IShipController::orderGate, pStargate, IShipController::SData(), true);
				}
			break;

		//	If we're trying to destroy the player, then wait until she returns.

		case IShipController::orderDestroyTarget:
			if (pObj->IsPlayer() && pObj == GetCurrentOrderTarget())
				AddOrder(IShipController::orderDestroyPlayerOnReturn, NULL, IShipController::SData(), true);
			break;
		}
	}

void CBaseShipAI::OnObjDestroyed (const SDestroyCtx &Ctx)

//	OnObjDestroyed
//
//	Handle the case where an object has been removed from the system

	{
	//	If our current order is to follow the player through and we get
	//	a notification that the player has been removed, then do nothing
	//	(we will be updated when we enter the new system).

	if (GetCurrentOrder() == IShipController::orderFollowPlayerThroughGate
			&& Ctx.pObj == GetCurrentOrderTarget())
		return;

	//	If we're following the player through a gate and the object
	//	got destroyed because it is passing through a gate, then we
	//	do nothing.

	if (GetCurrentOrder() == IShipController::orderFollowPlayerThroughGate
			&& Ctx.iCause == enteredStargate)
		return;

	//	If we are following an object and it is about to be resurrected,
	//	then don't cancel the escort order

	if (GetCurrentOrder() == IShipController::orderEscort
			&& Ctx.pObj == GetCurrentOrderTarget()
			&& Ctx.bResurrectPending)
		return;

	//	If we have an order module, let it handle it...

	if (m_pOrderModule)
		m_pOrderModule->ObjDestroyed(m_pShip, Ctx);

	//	Otherwise, let our subclass deal with it...

	else
		OnObjDestroyedNotify(Ctx);

	//	Loop over all our future orders and make sure that we
	//	delete any that refer to this object.

	bool bChanged;
	m_Orders.OnObjDestroyed(Ctx.pObj, &bChanged);
	if (bChanged)
		FireOnOrderChanged();
	}

void CBaseShipAI::OnPlayerChangedShips (CSpaceObject *pOldShip)

//	OnPlayerChangedShips
//
//	Player has changed ships

	{
	//	Get the new player ship

	CSpaceObject *pPlayerShip = g_pUniverse->GetPlayer();
	if (pPlayerShip == NULL)
		{
		ASSERT(false);
		return;
		}

	//	Loop over all orders and see if we need to change the target

	bool bChanged;
	m_Orders.OnPlayerChangedShips(pOldShip, pPlayerShip, &bChanged);
	if (bChanged)
		FireOnOrderChanged();
	}

void CBaseShipAI::OnPlayerObj (CSpaceObject *pPlayer)

//	OnPlayerObj
//
//	Player has entered the system

	{
	IShipController::OrderTypes iOrder = GetCurrentOrder();

	//	First, cancel the order that was making us wait for the player

	switch (iOrder)
		{
		case IShipController::orderDestroyPlayerOnReturn:
		case IShipController::orderWaitForPlayer:
			CancelCurrentOrder();
			break;
		}

	//	If we have events, then let the event handle everything

	bool bHandled = false;
	if (m_pShip->HasInterSystemEvent())
		{
		m_pShip->FireOnPlayerEnteredSystem(pPlayer);
		bHandled = (GetCurrentOrder() != IShipController::orderNone);
		}

	//	If not handled, add orders

	if (!bHandled)
		{
		switch (iOrder)
			{
			case IShipController::orderDestroyPlayerOnReturn:
				AddOrder(IShipController::orderDestroyTarget, pPlayer, IShipController::SData(), true);
				break;

			case IShipController::orderWaitForPlayer:
				//	We assume that if the ship is a wingman of the player, then
				//	it is responsible for setting its own orders. All other ships
				//	need to have an escort order (for backwards compatibility).

				if (!m_fIsPlayerWingman)
					{
					AddOrder(IShipController::orderEscort, pPlayer, IShipController::SData(), true);
					m_pShip->Communicate(m_pShip, msgWait);
					}
				break;
			}
		}
	}

void CBaseShipAI::OnStationDestroyed (const SDestroyCtx &Ctx)

//	OnStationDestroyed
//
//	Station has been destroyed

	{
	ASSERT(!Ctx.pObj->IsDestroyed());

	//	In some cases we ignore the notification because the station still
	//	exists

	DWORD dwFlags = GetOrderFlags(GetCurrentOrder());
	if (dwFlags & (ORDER_FLAG_DELETE_ON_STATION_DESTROYED | ORDER_FLAG_NOTIFY_ON_STATION_DESTROYED))
		{
		if (m_pOrderModule)
			m_pOrderModule->ObjDestroyed(m_pShip, Ctx);
		else
			OnObjDestroyedNotify(Ctx);
		}

	//	Remove orders as appropriate

	bool bChanged;
	m_Orders.OnStationDestroyed(Ctx.pObj, &bChanged);
	if (bChanged)
		FireOnOrderChanged();
	}

void CBaseShipAI::ReadFromStream (SLoadCtx &Ctx, CShip *pShip)

//	ReadFromStream
//
//	Reads controller data from stream
//
//	DWORD		Controller ObjID
//	DWORD		ship class UNID (needed to set AISettings)
//	DWORD		m_pShip (CSpaceObject ref)
//	DWORD		m_Blacklist
//
//	DWORD		order (for order module)
//	IOrderModule
//
//	DWORD		No of orders
//	DWORD		order: Order
//	DWORD		order: pTarget
//	DWORD		order: dwData
//
//	CString		m_pCommandCode (unlinked)
//
//	DWORD		flags

	{
	DWORD dwLoad;

	//	Read stuff

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	CShipClass *pClass = g_pUniverse->FindShipClass(dwLoad);
	CSystem::ReadObjRefFromStream(Ctx, (CSpaceObject **)&m_pShip);
	if (m_pShip == NULL)
		m_pShip = pShip;

	//	Ship controls moved to m_AICtx

	if (Ctx.dwVersion < 75)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_AICtx.SetManeuver((EManeuverTypes)dwLoad);

		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_AICtx.SetThrustDir((int)dwLoad);

		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_AICtx.SetLastTurn((EManeuverTypes)dwLoad);

		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_AICtx.SetLastTurnCount(dwLoad);
		}

	//	Read blacklist

	if (Ctx.dwVersion >= 6)
		m_Blacklist.ReadFromStream(Ctx);

	//	Older versions

	if (Ctx.dwVersion < 75)
		{
		//	Read manuever counter

		if (Ctx.dwVersion >= 33)
			{
			Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
			m_AICtx.SetManeuverCounter(dwLoad);
			}

		//	Read potential 

		if (Ctx.dwVersion >= 34)
			{
			CVector vPotential;
			Ctx.pStream->Read((char *)&vPotential, sizeof(CVector));
			m_AICtx.SetPotential(vPotential);
			}
		}

	//	AI settings

	if (Ctx.dwVersion >= 75)
		;//	No need to read because this is part of m_AICtx
	else if (Ctx.dwVersion >= 68)
		m_AICtx.ReadFromStreamAISettings(Ctx);
	else
		m_AICtx.SetAISettings(pClass->GetAISettings());

	//	AI context

	if (Ctx.dwVersion >= 75)
		m_AICtx.ReadFromStream(Ctx);

	//	Order module

	if (Ctx.dwVersion >= 75)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_pOrderModule = IOrderModule::Create((IShipController::OrderTypes)dwLoad);
		if (m_pOrderModule)
			m_pOrderModule->ReadFromStream(Ctx);
		}

	//	Read navpath info

	if (Ctx.dwVersion < 75)
		{
		if (Ctx.dwVersion >= 11)
			{
			CNavigationPath *pNavPath;
			int iNavPathPos;

			Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
			if (dwLoad)
				pNavPath = Ctx.pSystem->GetNavPathByID(dwLoad);
			else
				pNavPath = NULL;

			Ctx.pStream->Read((char *)&iNavPathPos, sizeof(DWORD));
			if (pNavPath == NULL)
				iNavPathPos = -1;

			m_AICtx.SetNavPath(pNavPath, iNavPathPos);
			}
		}

	//	Read orders

	m_Orders.ReadFromStream(Ctx);

	//	Command code

	if (Ctx.dwVersion >= 75)
		{
		CString sCode;
		sCode.ReadFromStream(Ctx.pStream);
		if (!sCode.IsBlank())
			m_pCommandCode = g_pUniverse->GetCC().Link(sCode, 0, NULL);
		else
			m_pCommandCode = NULL;
		}
	else
		m_pCommandCode = NULL;

	//	Flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_fDeviceActivate =			((dwLoad & 0x00000001) ? true : false);
	//	0x00000002 unused at 75
	//	0x00000004 unused at 75
	m_fCheckedForWalls =		((dwLoad & 0x00000008) ? true : false);
	m_fAvoidWalls =				((dwLoad & 0x00000010) ? true : false);
	m_fIsPlayerWingman =		((dwLoad & 0x00000020) ? true : false);

	//	Before version 75 we always used old style behaviors

	if (Ctx.dwVersion < 75)
		m_fOldStyleBehaviors = true;
	else
		{
		//	Unfortunately, the only way to tell if we're using an order
		//	modules it to try to create one. [There is an edge condition
		//	if we saved the game before we got to create an order module,
		//	which happens when saving missions.]

		if (m_pOrderModule)
			m_fOldStyleBehaviors = false;
		else
			{
			IOrderModule *pDummy = IOrderModule::Create(GetCurrentOrder());
			if (pDummy)
				{
				m_fOldStyleBehaviors = false;
				delete pDummy;
				}
			else
				m_fOldStyleBehaviors = true;
			}
		}

	//	In version 75 some flags were moved to the AI context

	if (Ctx.dwVersion < 75)
		{
		m_AICtx.SetDockingRequested((dwLoad & 0x00000002) ? true : false);
		m_AICtx.SetWaitingForShieldsToRegen((dwLoad & 0x00000004) ? true : false);
		}

	//	Subclasses

	OnReadFromStream(Ctx);
	}

void CBaseShipAI::ResetBehavior (void)

//	ResetBehavior
//
//	Resets fire and motion

	{
	if (!IsDockingRequested())
		{
		m_AICtx.SetManeuver(NoRotation);
		m_AICtx.SetThrustDir(CAIShipControls::constNeverThrust);
		}

	m_pShip->ClearAllTriggered();
	m_Blacklist.Update(g_pUniverse->GetTicks());
	}

void CBaseShipAI::SetCommandCode (ICCItem *pCode)

//	SetCommandCode
//
//	Sets the command code for the ship

	{
	if (m_pCommandCode)
		{
		m_pCommandCode->Discard(&(g_pUniverse->GetCC()));
		m_pCommandCode = NULL;
		}

	if (pCode)
		m_pCommandCode = pCode->Reference();
	}

void CBaseShipAI::SetCurrentOrderData (const SData &Data)

//	SetCurrentOrderData
//
//	Set the data for current order

	{
	m_Orders.SetCurrentOrderData(Data);
	}

void CBaseShipAI::SetShipToControl (CShip *pShip)

//	SetShipToControl
//
//	Set the ship that this controller will be controlling

	{
	ASSERT(m_pShip == NULL);

	m_pShip = pShip;
	m_AICtx.SetAISettings(pShip->GetClass()->GetAISettings());
	}

void CBaseShipAI::UpgradeShieldBehavior (void)

//	UpgradeShieldBehavior
//
//	Upgrade the ship's shields with a better one in cargo

	{
	CItem BestItem;
	int iBestLevel = 0;
	bool bReplace = false;

	//	Figure out the currently installed shields

	CInstalledDevice *pDevice = m_pShip->GetNamedDevice(devShields);
	if (pDevice)
		{
		iBestLevel = pDevice->GetClass()->GetLevel();
		bReplace = true;
		}

	//	Look for better shields

	CItemListManipulator ItemList(m_pShip->GetItemList());
	while (ItemList.MoveCursorForward())
		{
		const CItem &Item = ItemList.GetItemAtCursor();
		if (Item.GetType()->GetCategory() == itemcatShields
				&& !Item.IsInstalled()
				&& !Item.IsDamaged()
				&& Item.GetType()->GetLevel() >= iBestLevel)
			{
			//	If this device requires some item, then skip it for now
			//	(later we can add code to figure out if we've got enough
			//	of the item)

			if (Item.GetType()->GetDeviceClass()->RequiresItems())
				;

			//	If we cannot install this item, skip it

			else if (!m_pShip->CanInstallItem(Item))
				;

			//	Otherwise, remember this item

			else
				{
				BestItem = Item;
				iBestLevel = Item.GetType()->GetLevel();
				}
			}
		}

	//	If we found a better device, upgrade

	if (BestItem.GetType())
		{
		//	Uninstall the device weapon

		if (pDevice)
			{
			m_pShip->SetCursorAtNamedDevice(ItemList, devShields);
			m_pShip->RemoveItemAsDevice(ItemList);
			}

		//	Install the new item

		ItemList.SetCursorAtItem(BestItem);
		m_pShip->InstallItemAsDevice(ItemList);

		//	Recalc stuff

		m_AICtx.CalcInvariants(m_pShip);
		}
	}

void CBaseShipAI::UpgradeWeaponBehavior (void)

//	UpgradeWeaponBehavior
//
//	Upgrade the ship's weapon with a better one in cargo

	{
	int i;

	//	Loop over all currently installed weapons

	bool bWeaponsInstalled = false;
	for (i = 0; i < m_pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = m_pShip->GetDevice(i);
		if (!pDevice->IsEmpty() 
				&& pDevice->GetCategory() == itemcatWeapon)
			{
			//	Loop over all uninstalled weapons and see if we can
			//	find something better than this one.

			CItem BestItem;
			int iBestLevel = pDevice->GetClass()->GetLevel();

			CItemListManipulator ItemList(m_pShip->GetItemList());
			while (ItemList.MoveCursorForward())
				{
				const CItem &Item = ItemList.GetItemAtCursor();
				if (Item.GetType()->GetCategory() == itemcatWeapon
						&& !Item.IsInstalled()
						&& !Item.IsDamaged()
						&& Item.GetType()->GetLevel() >= iBestLevel)
					{
					//	If this weapon is an ammo weapon, then skip it for now
					//	(later we can add code to figure out if we've got enough
					//	ammo for the weapon)

					if (Item.GetType()->GetDeviceClass()->RequiresItems())
						;

					//	If we cannot install this item, skip it

					else if (!m_pShip->CanInstallItem(Item))
						;

					//	Otherwise, remember this item

					else
						{
						BestItem = Item;
						iBestLevel = Item.GetType()->GetLevel();
						}
					}
				}

			//	If we found a better weapon, upgrade

			if (BestItem.GetType())
				{
				//	Uninstall the previous weapon

				m_pShip->SetCursorAtDevice(ItemList, i);
				m_pShip->RemoveItemAsDevice(ItemList);

				//	Install the new item

				ItemList.SetCursorAtItem(BestItem);
				m_pShip->InstallItemAsDevice(ItemList, i);

				bWeaponsInstalled = true;
				}
			}
		}

	//	If we installed weapons, recalc invariants

	if (bWeaponsInstalled)
		m_AICtx.CalcInvariants(m_pShip);
	}

void CBaseShipAI::UseItemsBehavior (void)

//	UseItemsBehavior
//
//	Use various items appropriately

	{
	DEBUG_TRY

	if (m_pShip->IsDestinyTime(ITEM_ON_AI_UPDATE_CYCLE, ITEM_ON_AI_UPDATE_OFFSET))
		m_pShip->FireOnItemAIUpdate();

	if (m_AICtx.HasSuperconductingShields()
			&& m_pShip->IsDestinyTime(61)
			&& m_pShip->GetShieldLevel() < 40)
		{
		//	Look for superconducting coils

		CItemType *pType = g_pUniverse->FindItemType(g_SuperconductingCoilUNID);
		if (pType)
			{
			CItem Coils(pType, 1);
			CItemListManipulator ItemList(m_pShip->GetItemList());
			
			if (ItemList.SetCursorAtItem(Coils))
				{
				m_pShip->UseItem(Coils);
				m_pShip->OnComponentChanged(comCargo);
				}
			}
		}

	DEBUG_CATCH
	}

void CBaseShipAI::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Save the AI data to a stream
//
//	DWORD		Controller ObjID
//	DWORD		ship class UNID (needed to set AISettings)
//	DWORD		m_pShip (CSpaceObject ref)
//	DWORD		m_Blacklist
//
//	DWORD		order (for order module)
//	IOrderModule
//
//	DWORD		No of orders
//	DWORD		order: Order
//	DWORD		order: pTarget
//	DWORD		order: dwData
//
//	CString		m_pCommandCode (unlinked)
//
//	DWORD		flags
//
//	Subclasses...

	{
	DWORD dwSave;

	dwSave = (DWORD)GetClass()->GetObjID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = m_pShip->GetClass()->GetUNID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	m_pShip->WriteObjRefToStream(m_pShip, pStream);
	m_Blacklist.WriteToStream(pStream);

	//	Context

	m_AICtx.WriteToStream(m_pShip->GetSystem(), pStream);

	//	Order module

	dwSave = (DWORD)(m_pOrderModule ? m_pOrderModule->GetOrder() : IShipController::orderNone);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	if (m_pOrderModule)
		m_pOrderModule->WriteToStream(m_pShip->GetSystem(), pStream);

	//	Orders

	m_Orders.WriteToStream(pStream, m_pShip->GetSystem());

	//	Command code

	CString sCode;
	if (m_pCommandCode)
		sCode = g_pUniverse->GetCC().Unlink(m_pCommandCode);
	sCode.WriteToStream(pStream);

	//	Flags

	dwSave = 0;
	dwSave |= (m_fDeviceActivate ?			0x00000001 : 0);
	//	0x00000002 unused at version 75.
	//	0x00000004 unused at version 75.
	dwSave |= (m_fCheckedForWalls ?			0x00000008 : 0);
	dwSave |= (m_fAvoidWalls ?				0x00000010 : 0);
	dwSave |= (m_fIsPlayerWingman ?			0x00000020 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Subclasses

	OnWriteToStream(pStream);
	}

