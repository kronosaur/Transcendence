//	CFleetCommandAI.cpp
//
//	CFleetCommandAI class

#include "PreComp.h"


#define ALLOC_GRANULARITY				16
#define ATTACK_RANGE					(18.0 * LIGHT_SECOND)

//	At this perception we should be able to detect a normal ship (stealth = 4)
//	at almost 4 light-minutes distance
#define DEFAULT_PERCEPTION				6

#define MAX_DETECTION_RANGE				(240.0 * LIGHT_SECOND)
#define MAX_DETECTION_RANGE2			(MAX_DETECTION_RANGE * MAX_DETECTION_RANGE)
#define MAX_THREAT_RANGE				(120.0 * LIGHT_SECOND)
#define INTERCEPT_THREAT_RANGE			(80.0 * LIGHT_SECOND)
#define INTERCEPT_THREAT_RANGE2			(INTERCEPT_THREAT_RANGE * INTERCEPT_THREAT_RANGE)
#define MIN_THREAT_RANGE				(24.0 * LIGHT_SECOND)
#define MIN_THREAT_RANGE2				(MIN_THREAT_RANGE * MIN_THREAT_RANGE)
#define DEFENSE_RANGE					(18.0 * LIGHT_SECOND)
#define SEPARATION_RANGE				(8.0 * LIGHT_SECOND)
#define SEPARATION_RANGE2				(SEPARATION_RANGE * SEPARATION_RANGE)
#define RALLY_RANGE						(260.0 * LIGHT_SECOND)
#define BREAK_CHARGE_RANGE				(20.0 * LIGHT_SECOND)
#define CHARGE_ATTACK_RANGE				(200.0 * LIGHT_SECOND)

static CObjectClass<CFleetCommandAI>g_Class(OBJID_CFLEETCOMMANDAI, NULL);

CFleetCommandAI::CFleetCommandAI (void) : CBaseShipAI(&g_Class),
		m_State(stateNone),
		m_pObjective(NULL),
		m_pTarget(NULL),
		m_iAssetCount(0),
		m_iAssetAlloc(0),
		m_pAssets(NULL),
		m_iTargetCount(0),
		m_iTargetAlloc(0),
		m_pTargets(NULL),
		m_iCounter(0),
		m_iRallyFacing(0),
		m_iStartingAssetCount(0)

//	CFleetCommandAI constructor

	{
	}

CFleetCommandAI::~CFleetCommandAI (void)

//	CFleetCommandAI destructor

	{
	if (m_pAssets)
		delete [] m_pAssets;

	if (m_pTargets)
		delete [] m_pTargets;
	}

CFleetCommandAI::SAsset *CFleetCommandAI::AddAsset (CSpaceObject *pAsset)

//	AddAsset
//
//	Add an asset to our list

	{
	int i;

	//	Make sure we have enough room in the list

	if (m_iAssetCount + 1 > m_iAssetAlloc)
		{
		if (m_pAssets == NULL)
			{
			m_iAssetAlloc = ALLOC_GRANULARITY;
			m_pAssets = new SAsset [m_iAssetAlloc];
			}
		else
			{
			m_iAssetAlloc += ALLOC_GRANULARITY;
			SAsset *pNewAssets = new SAsset [m_iAssetAlloc];

			for (i = 0; i < m_iAssetCount; i++)
				pNewAssets[i] = m_pAssets[i];

			delete [] m_pAssets;
			m_pAssets = pNewAssets;
			}
		}

	//	Make sure that we are not adding a duplicate

#ifdef DEBUG
	for (i = 0; i < m_iAssetCount; i++)
		if (m_pAssets[i].pAsset == pAsset)
			ASSERT(false);
#endif

	//	Add the ship

	SAsset *pNewAsset = &m_pAssets[m_iAssetCount];
	pNewAsset->pAsset = pAsset;
	pNewAsset->iFormationPos = -1;
	pNewAsset->pTarget = NULL;
	m_iAssetCount++;
	m_iStartingAssetCount++;

	return pNewAsset;
	}

void CFleetCommandAI::AddTarget (CSpaceObject *pTarget)

//	AddTarget
//
//	Add an target to our list

	{
	int i;

	//	Make sure we have enough room in the list

	if (m_iTargetCount + 1 > m_iTargetAlloc)
		{
		if (m_pTargets == NULL)
			{
			m_iTargetAlloc = ALLOC_GRANULARITY;
			m_pTargets = new STarget [m_iTargetAlloc];
			}
		else
			{
			m_iTargetAlloc += ALLOC_GRANULARITY;
			STarget *pNewTargets = new STarget [m_iTargetAlloc];

			for (i = 0; i < m_iTargetCount; i++)
				pNewTargets[i] = m_pTargets[i];

			delete [] m_pTargets;
			m_pTargets = pNewTargets;
			}
		}

	//	Add the ship

	m_pTargets[m_iTargetCount].pTarget = pTarget;
	m_pTargets[m_iTargetCount].iAssignedTo = 0;
	m_pTargets[m_iTargetCount].iKilled = 0;
	m_iTargetCount++;
	}

void CFleetCommandAI::Behavior (void)

//	Behavior
//
//	Fly, fight, die

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
			ImplementAttackAtWill();
			break;

		case stateAttackFromRallyPoint:
			ImplementAttackFromRallyPoint();
			break;

		case stateChargeInFormation:
			ImplementChargeInFormation();
			break;

		case stateFormAtRallyPoint:
			ImplementFormAtRallyPoint();
			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
			break;

		case stateOnCourseForStargate:
			m_AICtx.ImplementGating(m_pShip, m_pObjective);
			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
			break;

		case stateWaiting:
			ImplementWaitAtPosition();
			break;

		case stateWaitingForThreat:
			{
			ASSERT(m_pObjective);

			//	Compute threat vector

			if (m_pShip->IsDestinyTime(31))
				UpdateTargetListAndPotentials();

			//	Maneuver so that we can defend the station against this threat

			CVector vPos = m_pObjective->GetPos() + (m_vThreatPotential.Normal() * DEFENSE_RANGE);
			m_AICtx.ImplementFormationManeuver(m_pShip, vPos, NullVector, m_pShip->AlignToRotationAngle(VectorToPolar(m_vThreatPotential)));

			//	Loop over all our targets and make sure that they have enough ships
			//	taking care of them.

			if (m_pShip->IsDestinyTime(17))
				UpdateAttackTargets();

			//	Attack as appropriate

			m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
			break;
			}

		default:
			ASSERT(false);
		}
	}

void CFleetCommandAI::BehaviorStart (void)

//	BehaviorStart
//
//	Initiate behavior state based on orders

	{
	switch (GetCurrentOrder())
		{
		case IShipController::orderNone:
			if (m_pShip->GetDockedObj() == NULL)
				AddOrder(IShipController::orderGate, NULL, IShipController::SData());
			break;

		case IShipController::orderDestroyTarget:
			{
			CSpaceObject *pTarget = GetCurrentOrderTarget();
			ASSERT(pTarget);

			SetState(stateFormAtRallyPoint);
			m_pObjective = pTarget;
			m_vRallyPoint = ComputeRallyPoint(m_pObjective, &m_iRallyFacing);

			OrderAllFormUp();
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
				m_pObjective = pGate;
				}

			break;
			}

		case IShipController::orderGuard:
			{
			CSpaceObject *pPrincipal = GetCurrentOrderTarget();
			ASSERT(pPrincipal);

			SetState(stateWaitingForThreat);
			m_pObjective = pPrincipal;
			break;
			}

		case IShipController::orderWait:
			{
			DWORD dwWaitTime = GetCurrentOrderData();

			SetState(stateWaiting);
			if (dwWaitTime == 0)
				m_iCounter = -1;
			else
				m_iCounter = 1 + (dwWaitTime * g_TicksPerSecond);
			break;
			}
		}
	}

void CFleetCommandAI::ComputeCombatPower (int *retiAssetPower, int *retiTargetPower)

//	ComputeCombatPower
//
//	Compute the power of assets and targets

	{
	int i;

	if (retiAssetPower)
		{
		int iAssetPower = 0;

		for (i = 0; i < m_iAssetCount; i++)
			iAssetPower += m_pAssets[i].pAsset->GetCombatPower();

		*retiAssetPower = iAssetPower;
		}

	if (retiTargetPower)
		{
		int iTargetPower = 0;

		for (i = 0; i < m_iTargetCount; i++)
			iTargetPower += m_pTargets[i].pTarget->GetCombatPower();

		*retiTargetPower = iTargetPower;
		}
	}

bool CFleetCommandAI::ComputeFormationReady (void)

//	ComputeFormationReady
//
//	Returns TRUE if all our ships are in proper formation

	{
	int i;

	Metric rMaxVel = (g_KlicksPerPixel / 4.0);
	Metric rMaxVel2 = rMaxVel * rMaxVel;

	for (i = 0; i < m_iAssetCount; i++)
		if (m_pAssets[i].pAsset->GetVel().Length2() > rMaxVel2)
			return false;

	return true;
	}

CVector CFleetCommandAI::ComputeRallyPoint (CSpaceObject *pTarget, int *retiFacing)

//	ComputeRallyPoint
//
//	Compute the point at which to rally to attack pTarget.
//	We choose a point that is some distance away and aligned on a directional
//	angle.

	{
	//	Compute the current bearing and distance to the target

	Metric rTargetDist;
	int iTargetBearing = VectorToPolar(pTarget->GetPos() - m_pShip->GetPos(), &rTargetDist);

	//	Compute rally point

	return ComputeRallyPointEx(iTargetBearing, pTarget, retiFacing);
	}

CVector CFleetCommandAI::ComputeRallyPointEx (int iBearing, CSpaceObject *pTarget, int *retiFacing)

//	ComputeRallyPointEx
//
//	Compute the point at which to rally to attack pTarget based on an initial angle

	{
	//	Reverse the angle and align to a direction

	int iAngle = m_pShip->AlignToRotationAngle((iBearing + 180) % 360);

	//	Randomly offset (+/- rotation angle)

	iAngle = ((iAngle + (mathRandom(-1, 1) * 18) + 360) % 360);

	//	Compute the distance from target. If we're a stand-off platform, make sure we
	//	rally inside of our weapon range.

	Metric rDistance;
	if (m_AICtx.GetCombatStyle() == aicombatStandOff)
		{
		m_AICtx.CalcBestWeapon(m_pShip, NULL, 0.0);
		CInstalledDevice *pBestWeapon = m_AICtx.GetBestWeapon();
		if (pBestWeapon)
			rDistance = pBestWeapon->GetMaxEffectiveRange(m_pShip, pTarget);
		else
			rDistance = RALLY_RANGE;
		}
	else
		rDistance = RALLY_RANGE;

	//	Return the facing

	if (retiFacing)
		*retiFacing = ((iAngle + 180) % 360);

	//	Return vector

	return pTarget->GetPos() + PolarToVector(iAngle, rDistance);
	}

CString CFleetCommandAI::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Returns debug crash info

	{
	int i;
	CString sResult;

	sResult.Append(CONSTLIT("CFleetCommandAI\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("Order: %d\r\n"), (int)GetCurrentOrder()));
	sResult.Append(strPatternSubst(CONSTLIT("m_State: %d\r\n"), m_State));
	sResult.Append(strPatternSubst(CONSTLIT("m_pObjective: %s\r\n"), CSpaceObject::DebugDescribe(m_pObjective)));
	sResult.Append(strPatternSubst(CONSTLIT("m_pTarget: %s\r\n"), CSpaceObject::DebugDescribe(m_pTarget)));

	for (i = 0; i < m_iAssetCount; i++)
		{
		sResult.Append(strPatternSubst(CONSTLIT("Asset[%d].pAsset: %s\r\n"), i, CSpaceObject::DebugDescribe(m_pAssets[i].pAsset)));
		sResult.Append(strPatternSubst(CONSTLIT("Asset[%d].pAsset: %s\r\n"), i, CSpaceObject::DebugDescribe(m_pAssets[i].pTarget)));
		}

	for (i = 0; i < m_iTargetCount; i++)
		sResult.Append(strPatternSubst(CONSTLIT("Target[%d].pTarget: %s\r\n"), i, CSpaceObject::DebugDescribe(m_pTargets[i].pTarget)));

	return sResult;
	}

CFleetCommandAI::SAsset *CFleetCommandAI::FindAsset (CSpaceObject *pAsset, int *retiIndex)

//	FindAsset
//
//	Returns the asset record

	{
	int i;

	for (i = 0; i < m_iAssetCount; i++)
		if (m_pAssets[i].pAsset == pAsset)
			{
			if (retiIndex)
				*retiIndex = i;
			return &m_pAssets[i];
			}

	return NULL;
	}

CFleetCommandAI::STarget *CFleetCommandAI::FindTarget (CSpaceObject *pTarget, int *retiIndex)

//	FindTarget
//
//	Returns the target record

	{
	int i;

	for (i = 0; i < m_iTargetCount; i++)
		if (m_pTargets[i].pTarget == pTarget)
			{
			if (retiIndex)
				*retiIndex = i;
			return &m_pTargets[i];
			}

	return NULL;
	}

void CFleetCommandAI::ImplementAttackAtWill (void)

//	ImplementAttackAtWill
//
//	Attack objective and all enemies

	{
	//	Attack our objective

	m_AICtx.ImplementAttackTarget(m_pShip, m_pObjective);

	//	Recalc our targets

	if (m_pShip->IsDestinyTime(31))
		UpdateTargetList();

	//	Loop over all our targets and make sure that they have enough ships
	//	taking care of them.

	if (m_pShip->IsDestinyTime(17))
		{
		UpdateAttackTargets();
		OrderAttackTarget(m_pObjective);
		}

	//	If we are outclassed, then we should re-group

	if (m_pShip->IsDestinyTime(53))
		{
		int iAssetPower, iEnemyPower;
		ComputeCombatPower(&iAssetPower, &iEnemyPower);

		if (iEnemyPower > iAssetPower)
			{
			OrderAllFormUp(true);
			SetState(stateNone);
			}
		}
	}

void CFleetCommandAI::ImplementAttackFromRallyPoint (void)

//	ImplementAttackFromRallyPoint
//
//	Attack objective from rally point

	{
	//	Stay in position

	m_AICtx.ImplementFormationManeuver(m_pShip, m_vRallyPoint, NullVector, m_pShip->AlignToRotationAngle(m_iRallyFacing));

	//	Fire our weapon
	
	m_AICtx.ImplementFireWeapon(m_pShip);

	//	Order all ships to open fire

	if (!m_fOpenFireOrdered)
		{
		OrderAllOpenFire();
		m_fOpenFireOrdered = true;
		}

	//	If we're done, move to a new location

	if (--m_iCounter <= 0)
		{
		SetState(stateFormAtRallyPoint);
		m_pObjective = GetCurrentOrderTarget();

		int iAngle = (m_iRallyFacing + 360 + mathRandom(-90, 90)) % 360;
		m_vRallyPoint = ComputeRallyPointEx(iAngle, m_pObjective, &m_iRallyFacing);

		OrderAllFormUp();
		}
	}

void CFleetCommandAI::ImplementChargeInFormation (void)

//	ImplementChargeInFormation
//
//	Charge towards objective

	{
	m_AICtx.ImplementFormationManeuver(m_pShip, m_pObjective->GetPos(), NullVector, m_pShip->AlignToRotationAngle(m_iRallyFacing));

	//	Compute our distance to the target

	CVector vDist = m_pObjective->GetPos() - m_pShip->GetPos();
	Metric rDist = vDist.Length();

	//	If we're within the break distance, then scatter

	if (rDist < BREAK_CHARGE_RANGE)
		{
#ifdef DEBUG_FLEET_COMMAND
		g_pUniverse->GetPlayer()->SendMessage(m_pShip, CONSTLIT("Break charge range reached!"));
#endif

		//	Order fleet to break and attack targets

		SetState(stateAttackAtWill);
		m_pObjective = GetCurrentOrderTarget();
		UpdateTargetList();
		UpdateAttackTargets();
		OrderAttackTarget(m_pObjective);
		}

	//	Otherwise, fire as appropriate

	else if (rDist < CHARGE_ATTACK_RANGE)
		{
		//	Fire blindly

		m_AICtx.ImplementFireWeapon(m_pShip);

		//	Order all ships to open fire

		if (!m_fOpenFireOrdered)
			{
#ifdef DEBUG_FLEET_COMMAND
			g_pUniverse->GetPlayer()->SendMessage(m_pShip, CONSTLIT("Open fire!"));
#endif

			OrderAllOpenFire();
			m_fOpenFireOrdered = true;
			}
		}
	}

void CFleetCommandAI::ImplementFormAtRallyPoint (void)

//	ImplementFormAtRallyPoint
//
//	Assemble the fleet at the rally point

	{
	//	Move to rally point

	m_AICtx.ImplementFormationManeuver(m_pShip, m_vRallyPoint, NullVector, m_pShip->AlignToRotationAngle(m_iRallyFacing));

	//	Every once in a while check to see if all our ships are in formation

	if (m_pShip->IsDestinyTime(29))
		{
		//	See if we are at the proper position. If not, bail out

		CVector vDist = (m_vRallyPoint - m_pShip->GetPos());
		Metric rDist2 = vDist.Length2();
		if (rDist2 > (4.0 * g_KlicksPerPixel * g_KlicksPerPixel))
			return;

		//	See if all our ships are in proper formation. If not, we're done

		if (!ComputeFormationReady())
			return;

		//	We are at the rally point, do the appropriate action based on 
		//	the kind of ship that we are.

		if (m_AICtx.GetCombatStyle() == aicombatStandOff)
			{
			SetState(stateAttackFromRallyPoint);
			m_pObjective = GetCurrentOrderTarget();
			m_iCounter = mathRandom(180, 360);
			m_fOpenFireOrdered = false;
			}
		else
			{
			SetState(stateChargeInFormation);
			m_pObjective = GetCurrentOrderTarget();
			m_fOpenFireOrdered = false;
			}

#ifdef DEBUG_FLEET_COMMAND
		g_pUniverse->GetPlayer()->SendMessage(m_pShip, CONSTLIT("Rally point reached"));
#endif
		}
	}

void CFleetCommandAI::ImplementWaitAtPosition (void)

//	ImplementWaitAtPosition
//
//	Wait at the current location

	{
	//	If we've lost a lot of ships, then attack a target

	if (m_iStartingAssetCount > 2 * m_iAssetCount)
		{
		if (m_pTarget == NULL)
			m_pTarget = m_pShip->GetNearestVisibleEnemy(ATTACK_RANGE);
		if (m_pTarget)
			m_AICtx.ImplementAttackTarget(m_pShip, m_pTarget);
		else
			m_AICtx.ImplementHold(m_pShip);
		}
	else
		{
		m_AICtx.ImplementAttackNearestTarget(m_pShip, ATTACK_RANGE, &m_pTarget);
		m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);

		//	Avoid target

		if (m_pTarget)
			{
			CVector vTarget = m_pTarget->GetPos() - m_pShip->GetPos();
			m_AICtx.ImplementSpiralOut(m_pShip, vTarget);
			}
		else
			m_AICtx.ImplementHold(m_pShip);
		}

	//	Recalc our targets

	if (m_pShip->IsDestinyTime(31))
		UpdateTargetList();

	//	Loop over all our targets and make sure that they have enough ships
	//	taking care of them.

	if (m_pShip->IsDestinyTime(17))
		UpdateAttackTargets();

	//	See if we're done waiting

	if (m_iCounter > 0)
		{
		if (--m_iCounter == 0)
			{
			if (GetCurrentOrder() == IShipController::orderWait)
				CancelCurrentOrder();

			SetState(stateNone);
			}
		}
	}

DWORD CFleetCommandAI::OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	Communicate
//
//	Handle communications from other objects

	{
	switch (iMessage)
		{
		case msgEscortReportingIn:
			{
			if (FindAsset(pSender) == NULL)
				{
				SAsset *pAsset = AddAsset(pSender);

				//	Enter formation

				pAsset->iFormationPos = m_iAssetCount - 1;
				DWORD dwFormation = MAKELONG(pAsset->iFormationPos, 0);
				m_pShip->Communicate(pSender, msgFormUp, NULL, dwFormation);
				}
			return resAck;
			}

		default:
			return resNoAnswer;
		}
	}

void CFleetCommandAI::OnDockedEvent (CSpaceObject *pObj)

//	OnDockedEvent
//
//	Event when the ship is docked

	{
	SetState(stateNone);
	}

void CFleetCommandAI::OnNewSystemNotify (void)

//	OnNewSystemNotify
//
//	We find ourselves in a different system

	{
	int i;
	CSystem *pNewSystem = m_pShip->GetSystem();

	//	Remove any assets that are not in the new system

	int iNewCount = m_iAssetCount;
	for (i = 0; i < m_iAssetCount; i++)
		{
		if (m_pAssets[i].pAsset->GetSystem() != pNewSystem)
			{
			IShipController::OrderTypes iOrder = m_pAssets[i].pAsset->AsShip()->GetController()->GetCurrentOrderEx();
			m_pAssets[i].pAsset = NULL;
			iNewCount--;
			}
		else
			m_pAssets[i].pTarget = NULL;
		}

	//	New array

	if (iNewCount != m_iAssetCount)
		{
		SAsset *pNewArray = new SAsset [m_iAssetAlloc];
		int j = 0;
		for (i = 0; i < m_iAssetCount; i++)
			if (m_pAssets[i].pAsset)
				pNewArray[j++] = m_pAssets[i];

		delete [] m_pAssets;
		m_pAssets = pNewArray;
		m_iAssetCount = j;
		}

	//	Remove all targets

	m_iTargetCount = 0;
	}

void CFleetCommandAI::OnObjDestroyedNotify (const SDestroyCtx &Ctx)

//	OnObjDestroyedNotify
//
//	Handle the case where another object is destroyed

	{
	//	Look for the object in the asset and target arrays

	STarget *pTarget;
	int iIndex;

	//	Remove the object from the asset data structure. If the
	//	object was a target, this also clears the pTarget variable
	//	(which makes the ship eligible for re-targeting).

	if (RemoveAssetObj(Ctx.pObj))
		{
		//	If one of our assets was destroyed, check to see if it was
		//	destroyed by a target.

		if (Ctx.Attacker.GetObj() 
				&& (pTarget = FindTarget(Ctx.Attacker.GetObj())) != NULL)
			pTarget->iKilled += Ctx.pObj->GetCombatPower();

		//	If our asset was attacking a target, remove it from the assignedTo
		//	field. (Obviously this could be a different enemy than the one
		//	that killed the asset).

		if ((pTarget = FindTarget(Ctx.pObj->GetTarget(CItemCtx()))) != NULL)
			pTarget->iAssignedTo -= Ctx.pObj->GetCombatPower();
		}

	//	Otherwise, check to see if a target was destroyed

	else if ((pTarget = FindTarget(Ctx.pObj, &iIndex)) != NULL)
		RemoveTarget(iIndex);

	//	Act based on state

	switch (m_State)
		{
		case stateAttackAtWill:
		case stateChargeInFormation:
		case stateFormAtRallyPoint:
		case stateAttackFromRallyPoint:
		case stateWaitingForThreat:
			{
			if (m_pObjective == Ctx.pObj)
				CancelCurrentOrder();
			break;
			}
		}

	//	Generic reset

	if (m_pObjective == Ctx.pObj)
		m_pObjective = NULL;
	if (m_pTarget == Ctx.pObj)
		m_pTarget = NULL;
	}

void CFleetCommandAI::OnOrderChanged (void)

//	OnOrderChanged
//
//	The order list has changed

	{
	SetState(stateNone);
	}

void CFleetCommandAI::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read our data
//
//	DWORD		m_State
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pObjective (CSpaceObject ref)
//	DWORD		m_iCounter
//	DWORD		m_iStartingAssetCount
//
//	DWORD		no. of assets
//	SAsset[]
//
//	DWORD		no. of targets
//	STarget[]
//
//	CVector		m_vThreatPotential
//	CVector		m_vRallyPoint
//	DWORD		m_iRallyFacing
//
//	DWORD		flags

	{
	int i;
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&m_State, sizeof(DWORD));
	CSystem::ReadObjRefFromStream(Ctx, &m_pTarget);
	CSystem::ReadObjRefFromStream(Ctx, &m_pObjective);
	Ctx.pStream->Read((char *)&m_iCounter, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iStartingAssetCount, sizeof(DWORD));

	Ctx.pStream->Read((char *)&m_iAssetCount, sizeof(DWORD));
	if (m_iAssetCount)
		{
		m_iAssetAlloc = AlignUp(m_iAssetCount, ALLOC_GRANULARITY);
		m_pAssets = new SAsset [m_iAssetAlloc];

		for (i = 0; i < m_iAssetCount; i++)
			{
			CSystem::ReadObjRefFromStream(Ctx, &m_pAssets[i].pAsset);
			Ctx.pStream->Read((char *)&m_pAssets[i].iFormationPos, sizeof(DWORD));
			CSystem::ReadObjRefFromStream(Ctx, &m_pAssets[i].pTarget);
			}
		}

	Ctx.pStream->Read((char *)&m_iTargetCount, sizeof(DWORD));
	if (m_iTargetCount)
		{
		m_iTargetAlloc = AlignUp(m_iTargetCount, sizeof(DWORD));
		m_pTargets = new STarget [m_iTargetAlloc];

		for (i = 0; i < m_iTargetCount; i++)
			{
			CSystem::ReadObjRefFromStream(Ctx, &m_pTargets[i].pTarget);
			Ctx.pStream->Read((char *)&m_pTargets[i].iAssignedTo, sizeof(DWORD));
			Ctx.pStream->Read((char *)&m_pTargets[i].iKilled, sizeof(DWORD));
			}
		}

	Ctx.pStream->Read((char *)&m_vThreatPotential, sizeof(CVector));
	Ctx.pStream->Read((char *)&m_vRallyPoint, sizeof(CVector));
	Ctx.pStream->Read((char *)&m_iRallyFacing, sizeof(DWORD));

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_fOpenFireOrdered =		((dwLoad & 0x00000001) ? true : false);
	}

void CFleetCommandAI::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write our data
//
//	DWORD		m_State
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pObjective (CSpaceObject ref)
//	DWORD		m_iCounter
//	DWORD		m_iStartingAssetCount
//
//	DWORD		no. of assets
//	SAsset[]
//
//	DWORD		no. of targets
//	STarget[]
//
//	CVector		m_vThreatPotential
//	CVector		m_vRallyPoint
//	DWORD		m_iRallyFacing
//
//	DWORD		flags

	{
	int i;
	DWORD dwSave;

	pStream->Write((char *)&m_State, sizeof(DWORD));
	m_pShip->WriteObjRefToStream(m_pTarget, pStream);
	m_pShip->WriteObjRefToStream(m_pObjective, pStream);
	pStream->Write((char *)&m_iCounter, sizeof(DWORD));
	pStream->Write((char *)&m_iStartingAssetCount, sizeof(DWORD));

	pStream->Write((char *)&m_iAssetCount, sizeof(DWORD));
	for (i = 0; i < m_iAssetCount; i++)
		{
		m_pShip->WriteObjRefToStream(m_pAssets[i].pAsset, pStream);
		pStream->Write((char *)&m_pAssets[i].iFormationPos, sizeof(DWORD));
		m_pShip->WriteObjRefToStream(m_pAssets[i].pTarget, pStream);
		}

	pStream->Write((char *)&m_iTargetCount, sizeof(DWORD));
	for (i = 0; i < m_iTargetCount; i++)
		{
		m_pShip->WriteObjRefToStream(m_pTargets[i].pTarget, pStream);
		pStream->Write((char *)&m_pTargets[i].iAssignedTo, sizeof(DWORD));
		pStream->Write((char *)&m_pTargets[i].iKilled, sizeof(DWORD));
		}

	pStream->Write((char *)&m_vThreatPotential, sizeof(CVector));
	pStream->Write((char *)&m_vRallyPoint, sizeof(CVector));
	pStream->Write((char *)&m_iRallyFacing, sizeof(DWORD));

	dwSave = 0;
	dwSave |= (m_fOpenFireOrdered ?			0x00000001 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CFleetCommandAI::OrderAllFormUp (bool bResetFormation)

//	OrderAllFormUp
//
//	Order all ships to form up

	{
	int i;

	//	Form up

	for (i = 0; i < m_iAssetCount; i++)
		{
		m_pAssets[i].pTarget = NULL;

		if (bResetFormation)
			m_pAssets[i].iFormationPos = i;

		DWORD dwFormation = MAKELONG(m_pAssets[i].iFormationPos, 0);
		m_pShip->Communicate(m_pAssets[i].pAsset, msgFormUp, m_pShip, dwFormation);
		}

	//	No targets are being attacked

	for (i = 0; i < m_iTargetCount; i++)
		m_pTargets[i].iAssignedTo = 0;
	}

void CFleetCommandAI::OrderAllOpenFire (void)

//	OrderAllOpenFire
//
//	Order all ships to open fire in formation

	{
	int i;

	for (i = 0; i < m_iAssetCount; i++)
		m_pShip->Communicate(m_pAssets[i].pAsset, msgAttackInFormation);
	}

void CFleetCommandAI::OrderAttackTarget (CSpaceObject *pTarget)

//	OrderAttackTarget
//
//	Order all ships that are not assigned to attack the target

	{
	int i;

	for (i = 0; i < m_iAssetCount; i++)
		if (m_pAssets[i].pTarget == NULL)
			{
			m_pShip->Communicate(m_pAssets[i].pAsset, msgAttack, pTarget);
			m_pAssets[i].pTarget = pTarget;
			}
	}

void CFleetCommandAI::RemoveAsset (int iIndex)

//	RemoveAsset
//
//	Removes the entry from the array

	{
	for (int i = iIndex; i < (m_iAssetCount - 1); i++)
		m_pAssets[i] = m_pAssets[i + 1];

	m_iAssetCount--;
	}

bool CFleetCommandAI::RemoveAssetObj (CSpaceObject *pObj)

//	RemoveAssetObj
//
//	Removes the object from the asset data structure (either as an asset or
//	as a target). Returns TRUE if an asset was removed.

	{
	int i;

	for (i = 0; i < m_iAssetCount; i++)
		{
		if (m_pAssets[i].pAsset == pObj)
			{
			RemoveAsset(i);
			return true;
			}
		else if (m_pAssets[i].pTarget == pObj)
			{
			m_pAssets[i].pTarget = NULL;

			//	Order this asset to form-up

			DWORD dwFormation = MAKELONG(m_pAssets[i].iFormationPos, 0);
			m_pShip->Communicate(m_pAssets[i].pAsset, msgFormUp, m_pShip, dwFormation);
			}
		}

	return false;
	}

void CFleetCommandAI::RemoveTarget (int iIndex)

//	RemoveTarget
//
//	Removes the entry from the array

	{
	for (int i = iIndex; i < (m_iTargetCount - 1); i++)
		m_pTargets[i] = m_pTargets[i + 1];

	m_iTargetCount--;
	}

void CFleetCommandAI::SetState (StateTypes State)

//	SetState
//
//	Sets the current state

	{
	//	Set state (NOTE: We do this before we undock because the Undock
	//	call may destroy the station and cause us to recurse into SetState.
	//	This happens when a ship is guarding an empty cargo crate).

	m_State = State;
	m_pTarget = NULL;
	m_pObjective = NULL;

	//	If we're currently docked and we're changing to a state that
	//	does not support docking, then we undock first.

	if (m_pShip->GetDockedObj()
			&& State != stateNone) 
		m_pShip->Undock();
	}

void CFleetCommandAI::UpdateAttackTargets (void)

//	UpdateAttackTargets
//
//	Updates to make sure that all targets are being attacked

	{
	int i;

	//	See if any targets have wandered out of our intercept range
	//	If so, call off the assets

	for (i = 0; i < m_iTargetCount; i++)
		{
		CVector vDist = m_pTargets[i].pTarget->GetPos() - m_pShip->GetPos();
		if (vDist.Length2() > INTERCEPT_THREAT_RANGE2)
			{
			RemoveAssetObj(m_pTargets[i].pTarget);

			RemoveTarget(i);
			i--;
			}
		}

	//	Make sure all targets are being attacked

	int iNextTarget = 0;
	int iNextAsset = 0;

	while (iNextAsset < m_iAssetCount && iNextTarget < m_iTargetCount)
		{
		int iPower = m_pTargets[iNextTarget].pTarget->GetCombatPower();
		
		while (iPower > m_pTargets[iNextTarget].iAssignedTo
				&& iNextAsset < m_iAssetCount)
			{
			if (m_pAssets[iNextAsset].pTarget == NULL)
				{
				//	Assign the asset to attack the target

				m_pShip->Communicate(m_pAssets[iNextAsset].pAsset, msgAttack, m_pTargets[iNextTarget].pTarget);
				m_pAssets[iNextAsset].pTarget = m_pTargets[iNextTarget].pTarget;
				m_pTargets[iNextTarget].iAssignedTo += m_pAssets[iNextAsset].pAsset->GetCombatPower();
				}

			iNextAsset++;
			}

		iNextTarget++;
		}
	}

void CFleetCommandAI::UpdateTargetList (void)

//	UpdateTargetList
//
//	Updates the targets array 

	{
	int i;
	CVector vPotential;

	//	Get the sovereign

	CSovereign *pSovereign = m_pShip->GetSovereignToDefend();
	if (pSovereign == NULL || m_pShip->GetSystem() == NULL)
		return;

	//	Get the list of enemy objects

	const CSpaceObjectList &ObjList = pSovereign->GetEnemyObjectList(m_pShip->GetSystem());
	CSpaceObject *pPrincipal = m_pShip->GetEscortPrincipal();

	//	Iterate over all objects

	int iCount = ObjList.GetCount();
	for (i = 0; i < iCount; i++)
		{
		CSpaceObject *pObj = ObjList.GetObj(i);

		if (pObj != m_pObjective
				&& pObj->CanAttack()
				&& pObj != m_pShip)
			{
			CVector vTarget = pObj->GetPos() - m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);

			if (rTargetDist2 < INTERCEPT_THREAT_RANGE2
					&& rTargetDist2 < pObj->GetDetectionRange2(DEFAULT_PERCEPTION)
					&& !pObj->IsEscortingFriendOf(m_pShip)
					&& (pPrincipal == NULL || !pPrincipal->IsFriend(pObj)))
				{
				if (FindTarget(pObj) == NULL)
					AddTarget(pObj);
				}
			}
		}
	}

void CFleetCommandAI::UpdateTargetListAndPotentials (void)

//	UpdateTargetListAndPotentials
//
//	Updates the targets array and the potential threat vector

	{
	int i;
	CVector vPotential;

	//	Iterate over all objects

	for (i = 0; i < m_pShip->GetSystem()->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = m_pShip->GetSystem()->GetObject(i);

		if (pObj  == NULL || pObj == m_pObjective || pObj == m_pShip || pObj->IsInactive() || pObj->IsVirtual())
			NULL;

		//	Compute the influence of ships (both friendly and enemy)

		else if (pObj->GetCategory() == CSpaceObject::catShip)
			{
			CVector vTarget = pObj->GetPos() - m_pObjective->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);

			if (rTargetDist2 < MAX_DETECTION_RANGE2)
				{
				//	Friendly ships (that are not part of this fleet) count as
				//	protection.

				if (pObj->GetSovereign() == m_pShip->GetSovereign()
						&& rTargetDist2 < INTERCEPT_THREAT_RANGE2
						&& m_pShip->Communicate(pObj, msgQueryFleetStatus, m_pShip) != resAck)
					{
					CVector vTargetN = vTarget.Normal();
					int iPower = pObj->GetCombatPower();

					//	Reduce threat level in that direction, proportional to power of friend

					vPotential = vPotential - (vTargetN * (Metric)iPower * g_KlicksPerPixel);

					//	If we're really close, then avoid the friendly ship

					CVector vSeparation = pObj->GetPos() - m_pShip->GetPos();
					Metric rSeparationDist2 = vSeparation.Dot(vSeparation);
					if (rSeparationDist2 < SEPARATION_RANGE2)
						{
						Metric rSeparationDist;
						CVector vSeparationN = vSeparation.Normal(&rSeparationDist);
						if (rSeparationDist == 0.0)
							{
							rSeparationDist = g_KlicksPerPixel;
							vSeparationN = PolarToVector(m_pShip->GetDestiny(), 1.0);
							}
						vPotential = vPotential - (vSeparationN * (SEPARATION_RANGE / rSeparationDist) * g_KlicksPerPixel);
						}
					}

				//	Enemy ships within our range of detection count as threats

				else if (pObj->IsEnemy(m_pObjective)
						&& rTargetDist2 < pObj->GetDetectionRange2(DEFAULT_PERCEPTION))
					{
					//	If the threat is inside a certain range, then we add it as a target
					//	to our list. Otherwise, it contributes to the potential threat vector

					if (rTargetDist2 < INTERCEPT_THREAT_RANGE2)
						{
						if (FindTarget(pObj) == NULL)
							AddTarget(pObj);
						}
					else
						{
						Metric rDist;
						CVector vTargetN = vTarget.Normal(&rDist);
						int iPower = pObj->GetCombatPower();

						vPotential = vPotential + (vTargetN * (Metric)iPower * (MAX_THREAT_RANGE / rDist) * g_KlicksPerPixel);
						}
					}
				}
			}
		}

	//	Set threat potential vector

	m_vThreatPotential = vPotential;
	}
