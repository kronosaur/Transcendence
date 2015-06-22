//	PlayerController.cpp
//
//	Implements class to control player's ship

#include "PreComp.h"
#include "Transcendence.h"

#define INVOKE_REFRESH_INTERVAL					100		//	Ticks to gain 1 point of deity rel (if rel is negative)

#define CMD_PLAYER_COMBAT_MISSION_STARTED		CONSTLIT("playerCombatMisionStarted")
#define CMD_PLAYER_COMBAT_ENDED					CONSTLIT("playerCombatEnded")
#define CMD_PLAYER_COMBAT_STARTED				CONSTLIT("playerCombatStarted")

#define STR_NO_TARGET_FOR_FLEET					CONSTLIT("No target selected")
#define STR_NO_TARGETING_COMPUTER				CONSTLIT("No targeting computer installed")

const Metric MAX_IN_COMBAT_RANGE =				LIGHT_SECOND * 30.0;
const int UPDATE_HELP_TIME =					31;
const Metric MAX_AUTO_TARGET_DISTANCE =			LIGHT_SECOND * 30.0;
const Metric MAX_DOCK_DISTANCE =				KLICKS_PER_PIXEL * 256.0;
const Metric MAX_DOCK_DISTANCE2 =				MAX_DOCK_DISTANCE * MAX_DOCK_DISTANCE;
const Metric MIN_PORT_ANIMATION_DIST =			KLICKS_PER_PIXEL * 2;
const Metric MIN_PORT_ANIMATION_DIST2 =			MIN_PORT_ANIMATION_DIST * MIN_PORT_ANIMATION_DIST;

const DWORD FIRE_THRESHOLD =					150;
const DWORD HIT_THRESHOLD =						90;

const DWORD FIRE_THRESHOLD_EXIT =				600;
const DWORD HIT_THRESHOLD_EXIT =				150;

const DWORD DAMAGE_BAR_TIMER =					30 * 5;

#define MAX_GATE_DISTANCE						(g_KlicksPerPixel * 150.0)
#define MAX_STARGATE_HELP_RANGE					(g_KlicksPerPixel * 256.0)

#define SETTING_ENABLED							CONSTLIT("enabled")
#define SETTING_TRUE							CONSTLIT("true")

CPlayerShipController::CPlayerShipController (void) : 
		m_pTrans(NULL),
		m_iManeuver(NoRotation),
		m_bThrust(false),
		m_bActivate(false),
		m_bStopThrust(false),
		m_pStation(NULL),
		m_bSignalDock(false),
		m_iOrder(orderNone),
		m_pTarget(NULL),
		m_pDestination(NULL),
		m_dwWreckObjID(OBJID_NULL),
		m_iLastHelpTick(0),
		m_iLastHelpUseTick(0),
		m_iLastHelpFireMissileTick(0),
		m_bMapHUD(true),
		m_bDockPortIndicators(true),
		m_bUnderAttack(false),
		m_pAutoDock(NULL),
		m_iAutoDockPort(0),
		m_bShowAutoTarget(false),
		m_pAutoTarget(NULL),
		m_iAutoTargetTick(0),
		m_pAutoDamage(NULL),
		m_dwAutoDamageExpire(0)

//	CPlayerShipController constructor

	{
	}

CPlayerShipController::~CPlayerShipController (void)

//	CPlayerShipController destructor

	{
	}

void CPlayerShipController::AddOrder (OrderTypes Order, CSpaceObject *pTarget, const IShipController::SData &Data, bool bAddBefore)

//	AddOrder
//
//	We get an order

	{
	//	If we've got no system then we ignore this. This can happen if we try to set
	//	an order in the middle of gating. For example, if we fail a mission due
	//	to gating away.

	if (m_pShip->GetSystem() == NULL)
		return;

	//	For now, we only deal with one order at a time

	switch (Order)
		{
		case orderGuard:
		case orderEscort:
			SetDestination(pTarget);
			break;

		case orderDock:
			SetDestination(pTarget);
			if (pTarget)
				pTarget->SetShowHighlight();
			break;

		case orderAimAtTarget:
		case orderDestroyTarget:
			if (m_pShip->HasTargetingComputer())
				SetTarget(pTarget);

			SetDestination(pTarget);
			if (pTarget)
				pTarget->SetShowHighlight();
			break;
		}

	//	Tell controller that we're in combat mode

	if (Order == orderGuard
			|| Order == orderEscort
			|| Order == orderDestroyTarget)
		g_pHI->HICommand(CMD_PLAYER_COMBAT_MISSION_STARTED);

	//	Remember our orders

	m_iOrder = Order;
	}

bool CPlayerShipController::AreAllDevicesEnabled (void)

//	AreAllDevicesEnabled
//
//	Returns TRUE if all devices are enabled. Returns FALSE is one or more devices
//	are disabled.

	{
	int i;

	for (i = 0; i < m_pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = m_pShip->GetDevice(i);
		if (pDevice && !pDevice->IsEmpty() && pDevice->CanBeDisabled(CItemCtx(m_pShip, pDevice)))
			{
			if (!pDevice->IsEnabled())
				return false;
			}
		}

	return true;
	}

bool CPlayerShipController::CanShowShipStatus (void)

//	CanShowShowStatus
//
//	Returns TRUE if we're allowed to show ship's status screen.

	{
	//	If we're trying to dock, then we can't

	if (DockingInProgress())
		return false;

	//	See if we have an overlay preventing us from bring it up

	COverlayList::SImpactDesc Impact;
	m_pShip->GetOverlayImpact(&Impact);
	if (Impact.bShipScreenDisabled)
		return false;

	//	We're OK

	return true;
	}

void CPlayerShipController::CancelAllOrders (void)

//	CancelAllOrder
//
//	Cancel all orders

	{
	m_iOrder = orderNone;
	SetDestination(NULL);
	}

void CPlayerShipController::CancelCurrentOrder (void)

//	CancelCurrentOrder
//
//	Cancel current order

	{
	m_iOrder = orderNone;
	SetDestination(NULL);
	}

void CPlayerShipController::CancelDocking (void)

//	CancelDocking
//
//	Cancel docking (if we're in the middle of docking)

	{
	if (m_pStation)
		{
		m_pStation->Undock(m_pShip);
		m_pStation = NULL;
		m_pTrans->DisplayMessage(CONSTLIT("Docking canceled"));
		}
	}

void CPlayerShipController::Communications (CSpaceObject *pObj, 
											MessageTypes iMsg, 
											DWORD dwData,
											DWORD *iodwFormationPlace)

//	Communications
//
//	Send a message to the given object

	{
	switch (iMsg)
		{
		case msgFormUp:
			{
			DWORD dwFormation;
			if (dwData == 0xffffffff)
				dwFormation = dwData;
			else if (iodwFormationPlace)
				dwFormation = MAKELONG(*iodwFormationPlace, (DWORD)dwData);
			else
				dwFormation = MAKELONG(0, (DWORD)dwData);

			DWORD dwRes = m_pShip->Communicate(pObj, iMsg, m_pShip, dwFormation);
			if (dwRes == resAck)
				if (iodwFormationPlace)
					(*iodwFormationPlace)++;
			break;
			}

		case msgAttack:
			{
			if (m_pTarget)
				m_pShip->Communicate(pObj, msgAttack, m_pTarget);
			else
				m_pTrans->DisplayMessage(STR_NO_TARGET_FOR_FLEET);

			break;
			}

		default:
			m_pShip->Communicate(pObj, iMsg);
			break;
		}
	}

void CPlayerShipController::ClearFireAngle (void)

//	ClearFireAngle
//
//	Clears the fire angle of weapon and launcher

	{
	int i;

	for (i = 0; i < m_pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = m_pShip->GetDevice(i);
		if (!pDevice->IsEmpty()
				&& (pDevice->GetCategory() == itemcatWeapon 
					|| pDevice->GetCategory() == itemcatLauncher))
			pDevice->SetFireAngle(-1);
		}
	}

CString CPlayerShipController::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Returns debug crash info

	{
	int i;
	CString sResult;

	sResult.Append(CONSTLIT("CPlayerShipController\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("m_pStation: %s\r\n"), CSpaceObject::DebugDescribe(m_pStation)));
	sResult.Append(strPatternSubst(CONSTLIT("m_pTarget: %s\r\n"), CSpaceObject::DebugDescribe(m_pTarget)));
	sResult.Append(strPatternSubst(CONSTLIT("m_pDestination: %s\r\n"), CSpaceObject::DebugDescribe(m_pDestination)));

	for (i = 0; i < m_TargetList.GetCount(); i++)
		sResult.Append(strPatternSubst(CONSTLIT("m_TargetList[%d]: %s\r\n"), i, CSpaceObject::DebugDescribe(m_TargetList.Get(i))));

	return sResult;
	}

void CPlayerShipController::Dock (void)

//	Dock
//
//	Docks the ship with the nearest station

	{
	//	If we're already in the middle of docking, then cancel docking

	if (m_pStation)
		{
		m_pStation->Undock(m_pShip);
		m_pStation = NULL;
		m_pTrans->DisplayMessage(CONSTLIT("Docking canceled"));
		return;
		}

	//	Figure out what to dock with. If we have a port to dock with, choose
	//	that. Otherwise, we look for any station with a docking port (even one
	//	that we can't dock with).

	int iDockPort;
	CSpaceObject *pStation;
	if (m_pAutoDock)
		{
		pStation = m_pAutoDock;
		iDockPort = m_iAutoDockPort;
		}
	else if (pStation = FindDockTarget())
		iDockPort = -1;
	else
		{
		m_pTrans->DisplayMessage(CONSTLIT("No stations in range"));
		return;
		}

	//	See if the player ship allows us to dock with this station

	CString sError;
	if (!m_pShip->FireCanDockAsPlayer(pStation, &sError))
		{
		m_pTrans->DisplayMessage(sError);
		return;
		}

	//	Request docking

	if (!pStation->RequestDock(m_pShip, iDockPort))
		return;

	//	Station has agreed to allow dock...

	m_pStation = pStation;

	SetActivate(false);
	SetFireMain(false);
	SetFireMissile(false);
	SetManeuver(NoRotation);
	SetThrust(false);

	SetUIMessageEnabled(uimsgDockHint, false);
	}

void CPlayerShipController::EnableAllDevices (bool bEnable)

//	EnableAllDevices
//
//	Enables or disables all devices

	{
	int i;

	for (i = 0; i < m_pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = m_pShip->GetDevice(i);
		if (pDevice && !pDevice->IsEmpty() && pDevice->CanBeDisabled(CItemCtx(m_pShip, pDevice)) && pDevice->IsEnabled() != bEnable)
			{
			m_pShip->EnableDevice(i, bEnable);

			//	Note: We will get called at OnDeviceEnabledDisabled for each
			//	device that we touch.
			}
		}
	}

CSpaceObject *CPlayerShipController::FindAutoTarget (CItemCtx &ItemCtx) const

//	FindAutoTarget
//
//	Returns the best target to shoot at if we have an omni weapons, etc.

	{
	//	If the weapon is directional, then we look for the best target within
	//	our fire arc.

	CDeviceClass *pWeapon;
	int iMinFireArc;
	int iMaxFireArc;
	if ((pWeapon = ItemCtx.GetDeviceClass()) 
			&& pWeapon->CanRotate(ItemCtx, &iMinFireArc, &iMaxFireArc)
			&& (iMinFireArc != iMaxFireArc))
		{
		//	Adjust for ship rotation

		iMinFireArc = AngleMod(m_pShip->GetRotation() + iMinFireArc);
		iMaxFireArc = AngleMod(m_pShip->GetRotation() + iMaxFireArc);

		//	Find ships

		return m_pShip->GetNearestVisibleEnemyInArc(iMinFireArc, iMaxFireArc, MAX_AUTO_TARGET_DISTANCE, true);
		}

	//	Otherwise, we return the nearest visible enemy

	else
		return m_pShip->GetNearestVisibleEnemy(MAX_AUTO_TARGET_DISTANCE, true);
	}

CSpaceObject *CPlayerShipController::FindDockTarget (void)

//	FindDockTarget
//
//	Finds the closest dock target

	{
	int i;

	CSystem *pSystem = m_pShip->GetSystem();
	CSpaceObject *pStation = NULL;
	Metric rMaxDist2 = MAX_DOCK_DISTANCE * MAX_DOCK_DISTANCE;
	Metric rBestDist = rMaxDist2;

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj 
				&& pObj->SupportsDocking(true)
				&& !pObj->IsInactive()
				&& !pObj->IsDestroyed()
				&& pObj != m_pShip)
			{
			CVector vDist = pObj->GetPos() - m_pShip->GetPos();
			Metric rDist2 = vDist.Length2();

			if (rDist2 < rMaxDist2)
				{
				//	If the station is inside the dock distance, check
				//	to see how close we are to a docking position.

				CVector vDockPos;
				if (pObj->GetNearestDockPort(m_pShip, &vDockPos) != -1)
					{
					Metric rDockDist2 = (vDockPos - m_pShip->GetPos()).Length2();

					if (rDockDist2 < rBestDist)
						{
						rBestDist = rDockDist2;
						pStation = pObj;
						}
					}
				}
			}
		}

	return pStation;
	}

void CPlayerShipController::Gate (void)

//	Gate
//
//	Enter a stargate

	{
	CSystem *pSystem = m_pShip->GetSystem();

	//	Find the stargate closest to the ship

	int i;
	Metric rBestDist2 = MAX_GATE_DISTANCE * MAX_GATE_DISTANCE;
	Metric rNearbyDist2 = 4.0 * MAX_STARGATE_HELP_RANGE * MAX_STARGATE_HELP_RANGE;
	CSpaceObject *pStation = NULL;
	bool bGateNearby = false;
	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj 
				&& pObj->SupportsGating()
				&& !pObj->IsInactive()
				&& !pObj->IsDestroyed()
				&& pObj != m_pShip)
			{
			CVector vDist = pObj->GetPos() - m_pShip->GetPos();
			Metric rDist2 = vDist.Length2();

			if (rDist2 < rBestDist2)
				{
				rBestDist2 = rDist2;
				pStation = pObj;
				}
			else if (rDist2 < rNearbyDist2)
				bGateNearby = true;
			}
		}

	//	If we did not find a station then we're done

	if (pStation == NULL)
		{
		if (bGateNearby)
			m_pTrans->DisplayMessage(CONSTLIT("Too far from stargate"));
		else
			m_pTrans->DisplayMessage(CONSTLIT("No stargates in range"));
		return;
		}

	//	Otherwise, request gating

	pStation->RequestGate(m_pShip);
	}

void CPlayerShipController::GenerateGameStats (CGameStats &Stats, bool bGameOver)

//	GenerateGameStats
//
//	Add to the list of game stats

	{
	m_Stats.GenerateGameStats(Stats, m_pShip, bGameOver);
	}

CString CPlayerShipController::GetAISettingString (const CString &sSetting)

//	GetAISettingString
//
//	Returns custom settings

	{
	if (strEquals(sSetting, CONSTLIT("underAttack")))
		return (m_bUnderAttack ? CONSTLIT("True") : NULL_STR);
	else
		return NULL_STR;
	}

int CPlayerShipController::GetCombatPower (void)

//	GetCombatPower
//
//	Computes the combat strength of this ship
//
//	CombatPower = ((reactor-power)^0.515) / 2.5

	{
	int iReactorPower = m_pShip->GetReactorDesc()->iMaxPower;
	if (iReactorPower <= 100)
		return 4;
	else if (iReactorPower <= 250)
		return 7;
	else if (iReactorPower <= 500)
		return 10;
	else if (iReactorPower <= 1000)
		return 14;
	else if (iReactorPower <= 2500)
		return 22;
	else if (iReactorPower <= 5000)
		return 32;
	else if (iReactorPower <= 10000)
		return 46;
	else if (iReactorPower <= 25000)
		return 74;
	else
		return 100;
	}

DWORD CPlayerShipController::GetCommsStatus (void)

//	GetCommsStatus
//
//	Returns the comms capabilities of all the ships that are escorting
//	the player.

	{
	int i;
	CSystem *pSystem = m_pShip->GetSystem();
	DWORD dwStatus = 0;

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj 
				&& !pObj->IsInactive()
				&& !pObj->IsDestroyed()
				&& pObj != m_pShip)
			dwStatus |= m_pShip->Communicate(pObj, msgQueryCommunications);
		}

	return dwStatus;
	}

IShipController::OrderTypes CPlayerShipController::GetCurrentOrderEx (CSpaceObject **retpTarget, IShipController::SData *retData)

//	GetCurrentOrderEx
//
//	Returns the current order

	{
	switch (m_iOrder)
		{
		case orderGuard:
		case orderEscort:
		case orderDock:
		case orderAimAtTarget:
		case orderDestroyTarget:
			if (m_pDestination == NULL)
				return orderNone;

			if (retpTarget)
				*retpTarget = m_pDestination;

			if (retData)
				*retData = SData();

			return m_iOrder;

		default:
			return orderNone;
		}
	}

void CPlayerShipController::GetWeaponTarget (STargetingCtx &TargetingCtx, CItemCtx &ItemCtx, CSpaceObject **retpTarget, int *retiFireSolution)

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

		//	The principal target is always first.

		CSpaceObject *pMainTarget = GetTarget(ItemCtx, true);
		if (pMainTarget)
			TargetingCtx.Targets.Insert(pMainTarget);

		//	Get other targets

		int iMaxTargets = 10;
		m_pShip->GetNearestVisibleEnemies(iMaxTargets,
				MAX_AUTO_TARGET_DISTANCE,
				&TargetingCtx.Targets,
				pMainTarget,
				FLAG_INCLUDE_NON_AGGRESSORS | FLAG_INCLUDE_STATIONS);

		TargetingCtx.bRecalcTargets = false;
		}

	//	Now find a target for the given weapon.

	for (i = 0; i < TargetingCtx.Targets.GetCount(); i++)
		{
		int iFireAngle;
		if (pWeapon->IsWeaponAligned(m_pShip, pDevice, TargetingCtx.Targets[i], NULL, &iFireAngle))
			{
			*retpTarget = TargetingCtx.Targets[i];
			*retiFireSolution = iFireAngle;
			return;
			}
		}

	//	If we get this far then no target found

	*retpTarget = NULL;
	*retiFireSolution = -1;
	}

bool CPlayerShipController::HasCommsTarget (void)

//	HasCommsTarget
//
//	Returns TRUE if we have at least one comms target to communicate with

	{
	int i;
	CSystem *pSystem = m_pShip->GetSystem();

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj 
				&& pObj->CanCommunicateWith(m_pShip)
				&& !pObj->IsInactive()
				&& !pObj->IsDestroyed()
				&& pObj != m_pShip)
			return true;
		}

	return false;
	}

bool CPlayerShipController::HasFleet (void)

//	HasFleet
//
//	Returns TRUE if the player has a fleet of ships to command

	{
	int i;
	CSystem *pSystem = m_pShip->GetSystem();

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj 
				&& (m_pShip->Communicate(pObj, msgQueryFleetStatus, m_pShip) == resAck)
				&& !pObj->IsInactive()
				&& !pObj->IsDestroyed()
				&& pObj != m_pShip)
			return true;
		}

	return false;
	}

void CPlayerShipController::Init (CTranscendenceWnd *pTrans)

//	Init
//
//	Initialize object before we play.

	{
	m_pTrans = pTrans;

	m_bDockPortIndicators = (strEquals(pTrans->GetSettings().GetString(CGameSettings::dockPortIndicator), SETTING_ENABLED)
			|| strEquals(pTrans->GetSettings().GetString(CGameSettings::dockPortIndicator), SETTING_TRUE));
	}

void CPlayerShipController::InitTargetList (TargetTypes iTargetType, bool bUpdate)

//	InitTargetList
//
//	Initializes the target list

	{
	int i;

	//	Make a list of all targets

	if (!bUpdate)
		m_TargetList.RemoveAll();

	//	Make sure we're valid

	CSystem *pSystem;
	if (m_pShip == NULL
			|| (pSystem = m_pShip->GetSystem()) == NULL)
		{
		m_TargetList.RemoveAll();
		return;
		}

	//	Compute some invariants based on our perception

	int iPerception = m_pShip->GetPerception();

	char szBuffer[1024];
	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj 
				&& pObj->CanBeHit()
				&& !pObj->IsDestroyed()
				&& pObj != m_pShip)
			{
			bool bInList = false;

			//	Figure out if we this object matches what we're looking for.
			//	If this is an enemy that can attack, then only show it if
			//	we're looking for enemy targets; otherwise, show it when
			//	we're looking for friendly targets

			int iMainKey = -1;
			if ((iTargetType == targetEnemies) == (m_pShip->IsEnemy(pObj) && pObj->CanAttack()))
				{
				if (iTargetType == targetEnemies)
					{
					if (pObj->GetScale() == scaleShip || pObj->GetScale() == scaleStructure)
						iMainKey = 0;
					}
				else
					{
					if (pObj->CanAttack() || pObj->SupportsDocking(true))
						{
						if (pObj->GetScale() == scaleShip || pObj->GetScale() == scaleStructure)
							iMainKey = 0;
						}
					}
				}

			//	If not what we're looking for, skip

			if (iMainKey != -1)
				{
				//	Figure out the distance to the object

				CVector vDist = pObj->GetPos() - m_pShip->GetPos();
				Metric rDist2 = vDist.Length2();

				//	If we can detect the object then it belongs on the list

				if (rDist2 < pObj->GetDetectionRange2(iPerception) || pObj->IsPlayerDestination())
					{
					//	Compute the distance in light-seconds

					int iDist = (int)((vDist.Length() / LIGHT_SECOND) + 0.5);

					//	Compute the sort order based on the main key and the distance

					wsprintf(szBuffer, "%d%08d%8x", iMainKey, iDist, (DWORD)pObj);
					bInList = true;
					}
				}

			//	Add or update the list

			if (bUpdate)
				{
				int iIndex;
				bool bFound = m_TargetList.Find(pObj, &iIndex);

				if (bInList)
					{
					if (!bFound)
						m_TargetList.Add(CString(szBuffer), pObj);
					}
				else
					{
					if (bFound)
						m_TargetList.Remove(iIndex);
					}
				}
			else if (bInList)
				m_TargetList.Add(CString(szBuffer), pObj);
			}
		}
	}

void CPlayerShipController::InsuranceClaim (void)

//	InsuranceClaim
//
//	Repair the ship and update insurance claim

	{
	ASSERT(m_pShip);

	CSystem *pSystem = g_pUniverse->GetCurrentSystem();
	ASSERT(pSystem);

	//	If necessary, remove the ship from the system first. This can happen if
	//	we are imprisoned by a station (instead of destroyed)

	if (m_pShip->GetIndex() != -1)
		m_pShip->Remove(removedFromSystem, CDamageSource());

	//	Place the ship back in the system

	m_pShip->AddToSystem(pSystem);

	//	Empty out the wreck

	if (m_dwWreckObjID != OBJID_NULL)
		{
		CSpaceObject *pWreck = pSystem->FindObject(m_dwWreckObjID);
		if (pWreck)
			{
			pWreck->GetItemList().DeleteAll();
			pWreck->InvalidateItemListAddRemove();
			}
		}
	}

void CPlayerShipController::OnArmorRepaired (int iSection)

//	OnArmorRepaired
//
//	Armor repaired or replaced

	{
	m_pTrans->UpdateArmorDisplay();
	}

void CPlayerShipController::OnBlindnessChanged (bool bBlind, bool bNoMessage)

//	OnBlindnessChanged
//
//	Player is blind or not blind

	{
	if (!bBlind && !bNoMessage)
		m_pTrans->DisplayMessage(CONSTLIT("Visual display repaired"));
	}

DWORD CPlayerShipController::OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	OnCommunicate
//
//	Message from another object

	{
	switch (iMessage)
		{
		case msgDockingSequenceEngaged:
			pSender->Highlight();
			m_pTrans->DisplayMessage(CONSTLIT("Docking sequence engaged"));
			return resAck;

		default:
			{
			bool bHandled;
			CString sMessage;

			//	First ask the sender to translate the message

			CString sID = GetMessageID(iMessage);

			if (pSender)
				{
				if (!sID.IsBlank())
					bHandled = pSender->Translate(sID, NULL, &sMessage);
				else
					bHandled = false;

				//	If the sender did not handle it and if it is an older version, then use
				//	the now deprecated <OnTranslateMessage>

				if (!bHandled && pSender->GetAPIVersion() < 3)
					bHandled = pSender->FireOnTranslateMessage(sID, &sMessage);
				}
			else
				bHandled = false;

			//	If we got no translation, then see if we can ask the sovereign
			//	to translate

			if (!bHandled)
				{
				CSovereign *pSovereign = NULL;

				if (pSender)
					pSovereign = pSender->GetSovereign();

				//	Make sure we have a sovereign

				if (pSovereign == NULL)
					pSovereign = g_pUniverse->FindSovereign(g_PlayerSovereignUNID);

				//	Get the message based on the sovereign

				sMessage = pSovereign->GetText(iMessage);
				}

			//	If we have a message, display it

			if (!sMessage.IsBlank())
				{
				if (pSender)
					pSender->Highlight(sMessage);
				else
					m_pTrans->DisplayMessage(sMessage);
				}

			return resNoAnswer;
			}
		}
	}

void CPlayerShipController::OnComponentChanged (ObjectComponentTypes iComponent)

//	OnComponentChanged
//
//	Component has changed

	{
	switch (iComponent)
		{
		case comDeviceCounter:
			m_pTrans->UpdateDeviceCounterDisplay();
			break;
		}
	}

void CPlayerShipController::OnDamaged (const CDamageSource &Cause, CInstalledArmor *pArmor, const DamageDesc &Damage, int iDamage)

//	OnDamaged
//
//	We get called here when the ship takes damage

	{
	if (pArmor == NULL)
		return;

	int iMaxArmorHP = pArmor->GetMaxHP(m_pShip);

	m_pTrans->Autopilot(false);

	//	Heavy damage (>= 10%) causes screen flash

	if (iDamage >= (iMaxArmorHP / 10) && Damage.CausesSRSFlash())
		m_pTrans->DamageFlash();

	//	If we're down to 25% armor, then warn the player

	if (pArmor->GetHitPoints() < (iMaxArmorHP / 4) && Damage.CausesSRSFlash())
		{
		m_pTrans->DisplayMessage(CONSTLIT("Hull breach imminent!"));
		g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_HULL_BREACH_ALARM));
		}

	//	Update display

	m_pTrans->UpdateArmorDisplay();

	//	Register stats

	CItem ArmorItem(pArmor->GetClass()->GetItemType(), 1);
	OnItemDamaged(ArmorItem, iDamage);
	}

bool CPlayerShipController::OnDestroyCheck (DestructionTypes iCause, const CDamageSource &Attacker)

//	OnDestroyCheck
//
//	Check to see if the player ship can be destroyed

	{
	int i;

	//	Loop over powers

	for (i = 0; i < g_pUniverse->GetPowerCount(); i++)
		{
		CPower *pPower = g_pUniverse->GetPower(i);
		if (!pPower->OnDestroyCheck(m_pShip, iCause, Attacker))
			return false;
		}

	return true;
	}

void CPlayerShipController::OnDestroyed (SDestroyCtx &Ctx)

//	OnDestroyed
//
//	Ship has been destroyed

	{
	DEBUG_TRY

	//	Clear various variables

	Reset();

	//	Reset everything else

	m_iManeuver = NoRotation;
	m_bThrust = false;
	m_bStopThrust = false;
	SetFireMain(false);
	SetFireMissile(false);
	m_bActivate = false;

	//	Notify

	CString sEpitaph;
	g_pTrans->GetModel().OnPlayerDestroyed(Ctx, &sEpitaph);

	m_pTrans->PlayerDestroyed(sEpitaph, Ctx.bResurrectPending);

	DEBUG_CATCH
	}

void CPlayerShipController::OnDeviceEnabledDisabled (int iDev, bool bEnable, bool bSilent)

//	OnDeviceEnabledDisabled
//
//	Device has been disabled (generally by reactor overload)

	{
	CInstalledDevice *pDevice = m_pShip->GetDevice(iDev);
	if (pDevice 
			&& !pDevice->IsEmpty())
		{
		if (!bEnable)
			{
			if (m_UIMsgs.IsEnabled(uimsgEnableDeviceHint))
				m_pTrans->DisplayMessage(CONSTLIT("(press [B] to enable/disable devices)"));

			if (!bSilent)
				m_pTrans->DisplayMessage(strCapitalize(strPatternSubst(CONSTLIT("%s disabled"),
						pDevice->GetClass()->GetName())));
			}
		else
			{
			if (!bSilent)
				m_pTrans->DisplayMessage(strCapitalize(strPatternSubst(CONSTLIT("%s enabled"),
						pDevice->GetClass()->GetName())));
			}
		}
	}

void CPlayerShipController::OnDeviceStatus (CInstalledDevice *pDev, int iEvent)

//	OnDeviceStatus
//
//	Device has failed in some way

	{
	switch (iEvent)
		{
		case CDeviceClass::statusDisruptionRepaired:
			m_pTrans->DisplayMessage(strCapitalize(strPatternSubst(CONSTLIT("%s repaired"), pDev->GetClass()->GetName())));
			break;

		case CDeviceClass::failDamagedByDisruption:
			m_pTrans->DisplayMessage(strCapitalize(strPatternSubst(CONSTLIT("%s damaged"), pDev->GetClass()->GetName())));
			break;

		case CDeviceClass::failWeaponJammed:
			m_pTrans->DisplayMessage(CONSTLIT("Weapon jammed!"));
			break;

		case CDeviceClass::failWeaponMisfire:
			m_pTrans->DisplayMessage(CONSTLIT("Weapon misfire!"));
			break;

		case CDeviceClass::failWeaponExplosion:
			m_pTrans->DisplayMessage(CONSTLIT("Weapon chamber explosion!"));
			break;

		case CDeviceClass::failShieldFailure:
			m_pTrans->DisplayMessage(CONSTLIT("Shield failure"));
			break;

		case CDeviceClass::failDeviceHitByDamage:
			m_pTrans->DisplayMessage(strCapitalize(strPatternSubst(CONSTLIT("%s damaged"), pDev->GetClass()->GetName())));
			break;

		case CDeviceClass::failDeviceHitByDisruption:
			m_pTrans->DisplayMessage(strCapitalize(strPatternSubst(CONSTLIT("%s ionized"), pDev->GetClass()->GetName())));
			break;

		case CDeviceClass::failDeviceOverheat:
			m_pTrans->DisplayMessage(strCapitalize(strPatternSubst(CONSTLIT("%s damaged by overheating"), pDev->GetClass()->GetName())));
			break;
		}
	}

void CPlayerShipController::OnEnemyShipsDetected (void)

//	OnEnemyShipsDetected
//
//	Enemy ships

	{
	m_pTrans->DisplayMessage(CONSTLIT("Enemy ships detected"));
	g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_ENEMY_SHIP_ALARM));
	}

void CPlayerShipController::OnFuelLowWarning (int iSeq)

//	OnFuelLowWarning
//
//	Warn of low fuel

	{
	//	If -1, we are out of fuel

	if (iSeq == -1)
		{
		m_pTrans->DisplayMessage(CONSTLIT("Out of fuel!"));

		//	Stop

		SetThrust(false);
		SetManeuver(NoRotation);
		SetFireMain(false);
		SetFireMissile(false);
		}

	//	Don't warn the player every time

	else if ((iSeq % 15) == 0)
		{
		if (m_UIMsgs.IsEnabled(uimsgRefuelHint))
			m_pTrans->DisplayMessage(CONSTLIT("(press [S] to access refueling screen)"));
		m_pTrans->DisplayMessage(CONSTLIT("Fuel low!"));
		if ((iSeq % 30) == 0)
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_FUEL_LOW_ALARM));
		}
	}

void CPlayerShipController::OnEnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate, bool bAscend)

//	OnEnterGate
//
//	Enter stargate

	{
	//	Clear our targeting computer (otherwise, we crash since we archive ships
	//	in the old system)

	Reset();

	//	Reset help

	SetUIMessageEnabled(uimsgGateHint, false);

	//	Let the model handle everything

	g_pTrans->GetModel().OnPlayerEnteredGate(pDestNode, sDestEntryPoint, pStargate);
	}

void CPlayerShipController::OnLifeSupportWarning (int iSecondsLeft)

//	OnLifeSupportWarning
//
//	Handle life support warning

	{
	if (iSecondsLeft > 10 && ((iSecondsLeft % 5) != 0))
		NULL;
	else if (iSecondsLeft > 1)
		m_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("Life support failure in %d seconds"), iSecondsLeft));
	else if (iSecondsLeft == 1)
		m_pTrans->DisplayMessage(CONSTLIT("Life support failure in 1 second"));
	}

void CPlayerShipController::OnPaintSRSEnhancements (CG32bitImage &Dest, SViewportPaintCtx &Ctx)

//	OnPaintSRSEnhancements
//
//	Paint SRS enhancements.

	{
	DEBUG_TRY

	//	Skip if we're not in the proper state

	if (m_pShip == NULL
			|| m_pShip->IsDestroyed())
		return;

	//	Paint the docking target, if necessary

	if (m_bDockPortIndicators
			&& m_pAutoDock 
			&& (m_vAutoDockPort - m_pShip->GetPos()).Length2() > MIN_PORT_ANIMATION_DIST2)
		{
		int x, y;
		Ctx.XForm.Transform(m_vAutoDockPort, &x, &y);

		int iSpeed = 3;
		int iRange = 10;
		int iMin = 3;

		int iPos = (iRange - 1) - ((g_pUniverse->GetPaintTick() / iSpeed) % iRange);
		int iSize = iMin + iPos;
		DWORD dwOpacity = 255 - (iPos * 20);

		CG32bitPixel rgbColor = CG32bitPixel(m_pAutoDock->GetSymbolColor(), (BYTE)dwOpacity);

		//	Draw animating brackets

		Dest.FillColumn(x - iSize, y - iSize, iPos + 1, rgbColor);
		Dest.FillColumn(x + iSize - 1, y - iSize, iPos + 1, rgbColor);
		Dest.FillColumn(x - iSize, y + iMin - 1, iPos + 1, rgbColor);
		Dest.FillColumn(x + iSize - 1, y + iMin - 1, iPos + 1, rgbColor);

		Dest.FillLine(x - iSize + 1, y - iSize, iPos, rgbColor);
		Dest.FillLine(x - iSize + 1, y + iSize - 1, iPos, rgbColor);
		Dest.FillLine(x + 2, y - iSize, iPos, rgbColor);
		Dest.FillLine(x + 2, y + iSize - 1, iPos, rgbColor);
		}

	//	If we have a target, then paint a target reticle.
	//	NOTE: We do this even if friendly because weapons will still aim at them.

	if (m_pTarget)
		PaintTargetingReticle(Ctx, Dest, m_pTarget);

	//	If we have an auto target and we want to show it, paint a reticle

	else if (m_pAutoTarget 
			&& m_bShowAutoTarget)
		PaintTargetingReticle(Ctx, Dest, m_pAutoTarget);

	//	If necessary, show damage bar

	if (m_pAutoDamage
			&& (DWORD)g_pUniverse->GetTicks() > m_dwAutoDamageExpire)
		{
		m_pAutoDamage->ClearShowDamageBar();
		m_pAutoDamage = NULL;
		}

	DEBUG_CATCH
	}

void CPlayerShipController::OnRadiationWarning (int iTicksLeft)

//	OnRadiationWarning
//
//	Handle radiation warning

	{
	if ((iTicksLeft % 10) == 0)
		{
		int iSecondsLeft = iTicksLeft / g_TicksPerSecond;

		if (iSecondsLeft > 10 && ((iSecondsLeft % 5) != 0))
			NULL;
		else if (iSecondsLeft > 1)
			m_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("Radiation Warning: Fatal exposure in %d seconds"), iSecondsLeft));
		else if (iSecondsLeft == 1)
			m_pTrans->DisplayMessage(CONSTLIT("Radiation Warning: Fatal exposure in 1 second"));
		else
			m_pTrans->DisplayMessage(CONSTLIT("Radiation Warning: Fatal exposure received"));

		if ((iTicksLeft % 150) == 0)
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_RADIATION_ALARM));
		}
	}

void CPlayerShipController::OnRadiationCleared (void)

//	OnRadiationCleared
//
//	Handler radiation cleared

	{
	m_pTrans->DisplayMessage(CONSTLIT("Decontamination complete"));
	}

void CPlayerShipController::OnReactorOverloadWarning (int iSeq)

//	OnReactorOverlordWarning
//
//	Handler reactor overload

	{
	//	Warn every 60 ticks

	if ((iSeq % 6) == 0)
		{
		m_pTrans->DisplayMessage(CONSTLIT("Warning: Reactor overload"));
		if ((iSeq % 24) == 0)
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_REACTOR_OVERLOAD_ALARM));
		}
	}

void CPlayerShipController::OnStartGame (void)

//	OnStartGame
//
//	Game (either new or loaded) has just started

	{
	m_iManeuver = NoRotation;
	m_bThrust = false;
	m_bStopThrust = false;
	SetFireMain(false);
	SetFireMissile(false);
	m_bActivate = false;

	//	Clear the POVLRS flag for all objects (so that we don't get the
	//	"Enemy Ships Detected" message when entering a system

	m_pShip->GetSystem()->SetPOVLRS(m_pShip);
	}

void CPlayerShipController::OnStationDestroyed (const SDestroyCtx &Ctx)

//	OnStationDestroyed
//
//	A station has been destroyed

	{
	bool bIsMajorStation = Ctx.pObj->HasAttribute(CONSTLIT("majorStation"));

	//	If we are the cause of the destruction and then record stat

	if (Ctx.Attacker.IsCausedByPlayer())
		{
		m_Stats.OnObjDestroyedByPlayer(Ctx, m_pShip);

		//	If this is a major station then add it to the list of key events

		if (bIsMajorStation)
			m_Stats.OnKeyEvent((Ctx.pObj->IsEnemy(m_pShip) ? CPlayerGameStats::eventEnemyDestroyedByPlayer : CPlayerGameStats::eventFriendDestroyedByPlayer),
					Ctx.pObj,
					Ctx.Attacker.GetSovereignUNID());
		}

	//	Otherwise, record the event if this is a major station

	else if (bIsMajorStation)
		m_Stats.OnKeyEvent(CPlayerGameStats::eventMajorDestroyed, Ctx.pObj, Ctx.Attacker.GetSovereignUNID());

	//	If the station was targeted, then clear the target

	if (m_pTarget == Ctx.pObj)
		{
		SetTarget(NULL);
		m_pTrans->UpdateWeaponStatus();
		}

	if (m_pAutoDamage == Ctx.pObj)
		{
		m_pAutoDamage->ClearShowDamageBar();
		m_pAutoDamage = NULL;
		}
	}

void CPlayerShipController::OnWreckCreated (CSpaceObject *pWreck)

//	OnWreckCreated
//
//	Called when the ship has been destroyed and a wreck has been
//	created.

	{
	ASSERT(pWreck);

	//	Change our POV to the wreck

	g_pUniverse->SetPOV(pWreck);

	//	We remember the wreck, but only by ID because we might not
	//	get any notifications of its destruction after this
	//	(since the player ship has been taken out of the system)

	m_dwWreckObjID = pWreck->GetID();
	}

void CPlayerShipController::PaintTargetingReticle (SViewportPaintCtx &Ctx, CG32bitImage &Dest, CSpaceObject *pTarget)

//	PaintTargetingReticle
//
//	Paints a targeting reticle on the given object

	{
	int x, y;
	Ctx.XForm.Transform(pTarget->GetPos(), &x, &y);

	CG32bitPixel rgbColor = pTarget->GetSymbolColor();
	int iSize = 8;
	int iIndent = iSize / 4;

	const RECT &rcImage = pTarget->GetImage().GetImageRect();
	int cxWidth = RectWidth(rcImage);
	int cyHeight = RectHeight(rcImage);

	int cxHorz = Max(0, (cxWidth / 2) - iIndent);
	int cyVert = Max(0, (cyHeight / 2) - iIndent);

	//	Draw

	Dest.FillColumn(x, y - cyVert - iSize, iSize, rgbColor);
	Dest.FillColumn(x, y + cyVert, iSize, rgbColor);

	Dest.FillLine(x + cxHorz, y, iSize, rgbColor);
	Dest.FillLine(x - cxHorz - iSize, y, iSize, rgbColor);
	}

bool CPlayerShipController::ToggleEnableDevice (int iDeviceIndex)

//	ToggleEnableDevice
//
//	Toggles the enable/disable state of a given device. Returns TRUE if the device was toggled.
//	
//	iDeviceIndex is the index of the device as it show up in the
//	enable/disable menu.

	{
	int i;

	//	First we need to look for the device by index. We use the same
	//	order as listed in the Enable/Disable menu.

	CItemList &List = m_pShip->GetItemList();
	List.SortItems();

	for (i = 0; i < List.GetCount(); i++)
		{
		CItem &Item = List.GetItem(i);
		CInstalledDevice *pDevice = m_pShip->FindDevice(Item);

		if (pDevice && pDevice->CanBeDisabled(CItemCtx(m_pShip, pDevice)))
			{
			if (iDeviceIndex == 0)
				{
				m_pShip->EnableDevice(Item.GetInstalled(), !pDevice->IsEnabled());
				return true;
				}
			else
				iDeviceIndex--;
			}
		}

	return false;
	}

EManeuverTypes CPlayerShipController::GetManeuver (void)
	{
	return m_iManeuver;
	}

bool CPlayerShipController::GetThrust (void)
	{
	return m_bThrust;
	}

bool CPlayerShipController::GetReverseThrust (void)
	{
	return false;
	}

bool CPlayerShipController::GetStopThrust (void)
	{
	return m_bStopThrust;
	}

bool CPlayerShipController::GetDeviceActivate (void)
	{
	return m_bActivate;
	}

CSpaceObject *CPlayerShipController::GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget) const

//	GetTarget
//
//	Returns the target for the player ship

	{
	if (m_pTarget || bNoAutoTarget)
		return m_pTarget;
	else
		{
		//	Return the autotarget

		int iTick = g_pUniverse->GetTicks();
		if (iTick == m_iAutoTargetTick)
			return ((m_pAutoTarget && !m_pAutoTarget->IsDestroyed()) ? m_pAutoTarget : NULL);

		m_iAutoTargetTick = iTick;

		//	Make sure the fire angle is set to -1 if we don't have a target.
		//	Otherwise, we will keep firing at the wrong angle after we destroy
		//	a target.

		if (m_pAutoTarget == NULL)
			{
			CInstalledDevice *pDevice = m_pShip->GetNamedDevice(devPrimaryWeapon);
			if (pDevice)
				pDevice->SetFireAngle(-1);

			pDevice = m_pShip->GetNamedDevice(devMissileWeapon);
			if (pDevice)
				pDevice->SetFireAngle(-1);
			}

		return ((m_pAutoTarget && !m_pAutoTarget->IsDestroyed()) ? m_pAutoTarget : NULL);
		}
	}

void CPlayerShipController::OnDocked (CSpaceObject *pObj)
	{
	ASSERT(pObj == m_pStation);

	//	Set a flag because we don't want to transition to dock screen from
	//	inside this event.

	m_bSignalDock = true;
	}

void CPlayerShipController::OnDockedObjChanged (CSpaceObject *pLocation)
	{
	g_pTrans->GetModel().OnDockedObjChanged(pLocation);
	}

void CPlayerShipController::OnMissionCompleted (CMission *pMission, bool bSuccess)

//	OnMissionCompleted
//
//	A player mission has finished.

	{
	if (pMission->KeepsStats())
		{
		if (bSuccess)
			m_Stats.OnKeyEvent(CPlayerGameStats::eventMissionSuccess, pMission, 0);
		else
			m_Stats.OnKeyEvent(CPlayerGameStats::eventMissionFailure, pMission, 0);
		}
	}

void CPlayerShipController::OnNewSystem (CSystem *pSystem)

//	OnNewSystem
//
//	Player has arrived at a new system

	{
	}

void CPlayerShipController::OnObjDamaged (const SDamageCtx &Ctx)

//	OnObjDamaged
//
//	We damaged an object

	{
	//	Skip if we're dead

	if (m_pShip == NULL || m_pShip->IsDestroyed())
		return;

	//	Remember that we caused damage to this object (but only if it is an 
	//	enemy station or a capital ship of some sort).

	if (Ctx.pObj 
			&& !Ctx.pObj->IsDestroyed()
			&& (m_pShip->IsEnemy(Ctx.pObj)
				|| Ctx.pObj->IsAngryAt(m_pShip))
			&& (Ctx.pObj->GetCategory() == CSpaceObject::catStation
				|| Ctx.pObj->IsMultiHull()))
		{
		if (m_pAutoDamage)
			m_pAutoDamage->ClearShowDamageBar();

		m_pAutoDamage = Ctx.pObj;
		m_pAutoDamage->SetShowDamageBar();
		m_dwAutoDamageExpire = g_pUniverse->GetTicks() + DAMAGE_BAR_TIMER;
		}
	}

void CPlayerShipController::OnObjDestroyed (const SDestroyCtx &Ctx)

//	OnObjDestroyed
//
//	An object was destroyed

	{
	//	If we are the cause of the destruction and the ship is
	//	not a friend, then increase our score
	//
	//	NOTE: We don't care about stations here because those will be
	//	handled in OnStationDestroyed

	if (Ctx.pObj->GetCategory() == CSpaceObject::catShip)
		{
		bool bIsMajorShip = Ctx.pObj->HasAttribute(CONSTLIT("majorShip"));

		if (Ctx.Attacker.IsCausedByPlayer())
			{
			m_Stats.OnObjDestroyedByPlayer(Ctx, m_pShip);

			if (bIsMajorShip)
				m_Stats.OnKeyEvent((Ctx.pObj->IsEnemy(m_pShip) ? CPlayerGameStats::eventEnemyDestroyedByPlayer : CPlayerGameStats::eventFriendDestroyedByPlayer),
						Ctx.pObj,
						Ctx.Attacker.GetSovereignUNID());
			}
		else if (bIsMajorShip)
			m_Stats.OnKeyEvent(CPlayerGameStats::eventMajorDestroyed, Ctx.pObj, Ctx.Attacker.GetSovereignUNID());
		}

	//	If the object we're docked with got destroyed, then undock

	if (m_pStation == Ctx.pObj || Ctx.pObj->IsPlayerDocked())
		{
		if (Ctx.pObj->IsPlayerDocked())
			g_pTrans->GetModel().ExitScreenSession(true);

		m_pStation = NULL;
		}

	//	Clear out some variables

	if (m_pTarget == Ctx.pObj)
		{
		m_pTarget = NULL;
		ClearFireAngle();
		m_pTrans->UpdateWeaponStatus();
		}

	if (m_pDestination == Ctx.pObj)
		m_pDestination = NULL;

	if (m_pAutoDock == Ctx.pObj)
		m_pAutoDock = NULL;

	if (m_pAutoTarget == Ctx.pObj)
		m_pAutoTarget = NULL;

	if (m_pAutoDamage == Ctx.pObj)
		{
		m_pAutoDamage->ClearShowDamageBar();
		m_pAutoDamage = NULL;
		}

	//	Clear out the target list

	m_TargetList.Remove(Ctx.pObj);

	//	Let the UI deal with destroyed objects

	m_pTrans->OnObjDestroyed(Ctx);
	}

void CPlayerShipController::OnProgramDamage (CSpaceObject *pHacker, const ProgramDesc &Program)

//	OnProgramDamage
//
//	Hit by a program

	{
	if (pHacker)
		pHacker->Highlight();

	m_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("Cyberattack detected: %s"), Program.sProgramName));
	}

void CPlayerShipController::OnUpdatePlayer (SUpdateCtx &Ctx)

//	OnUpdatePlayer
//
//	This is called every tick after all other objects have been updated.

	{
	DEBUG_TRY

	//	Remember the AutoTarget. NOTE: We need to check again to see if the
	//	target is destroyed because it could have gotten destroyed after it
	//	was picked.

	if (Ctx.pTargetObj && !Ctx.pTargetObj->IsDestroyed())
		{
		m_pAutoTarget = Ctx.pTargetObj;
		if (Ctx.bNeedsAutoTarget)
			m_bShowAutoTarget = true;
		else
			m_bShowAutoTarget = false;
		}
	else
		{
		m_pAutoTarget = NULL;
		m_bShowAutoTarget = false;
		}

	//	Compute the AutoDock target.
	//
	//	If we're already in the middle of docking then we don't change anything.

	if (m_pStation)
		NULL;

	//	We calculate the best port to dock with at this moment. We start by 
	//	seeing if the player has targeted a friendly station:

	else if (m_pTarget 
			&& m_pTarget != m_pShip 
			&& m_pTarget->SupportsDocking(true)
			&& (!m_pShip->IsEnemy(m_pTarget) || m_pTarget->IsAbandoned())
			&& (m_pTarget->GetPos() - m_pShip->GetPos()).Length2() < MAX_DOCK_DISTANCE2)
		{
		m_pAutoDock = m_pTarget;
		m_iAutoDockPort = m_pTarget->GetNearestDockPort(m_pShip, &m_vAutoDockPort);
		if (m_iAutoDockPort == -1)
			m_pAutoDock = NULL;
		}

	//	Otherwise, if we are close to a port then we use that.

	else if (Ctx.pDockingObj && !Ctx.pDockingObj->IsDestroyed())
		{
		m_pAutoDock = Ctx.pDockingObj;
		m_iAutoDockPort = Ctx.iDockingPort;
		m_vAutoDockPort = Ctx.vDockingPort;
		}

	//	Otherwise, nothing to dock with

	else
		m_pAutoDock = NULL;

	//	Notify the game controller when we transition in/out of combat.
	//
	//	If we're currently in combat, figure out if we're still in combat. If 
	//	not, then we transition out.

	if (m_bUnderAttack)
		{
		bool bStillUnderAttack = Ctx.pSystem->EnemiesInLRS()
				&& (Ctx.pSystem->IsPlayerUnderAttack()
					|| m_pShip->HasBeenHitLately(HIT_THRESHOLD_EXIT)
					|| m_pShip->HasFiredLately(FIRE_THRESHOLD_EXIT));
		if (!bStillUnderAttack)
			{
			g_pHI->HICommand(CMD_PLAYER_COMBAT_ENDED);
			m_bUnderAttack = false;
			}
		}

	//	Otherwise, if we're not in combat, figure out if we're in combat.
	//	NOTE: We have different criteria for entering combat vs. staying in 
	//	comabt.

	else
		{
		bool bUnderAttack = Ctx.pSystem->EnemiesInLRS()
				&& m_pShip->HasFiredLately(FIRE_THRESHOLD)
				&& (m_pShip->HasBeenHitLately(HIT_THRESHOLD) || Ctx.pSystem->IsPlayerUnderAttack());
		if (bUnderAttack)
			{
			g_pHI->HICommand(CMD_PLAYER_COMBAT_STARTED);
			m_bUnderAttack = true;
			}
		}

	//	Gravity warning

	if (Ctx.bGravityWarning)
		{
		int iTicks = g_pUniverse->GetTicks();

		if ((iTicks % 150) == 0)
			{
			m_pTrans->DisplayMessage(CONSTLIT("Warning: Deep gravity zone"));
			g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_GRAVITY_ALARM));
			}
		}

	//	If we've succeeded in docking, then tell the model

	if (m_bSignalDock)
		{
		if (m_pStation && !m_pStation->IsDestroyed())
			g_pTrans->GetModel().OnPlayerDocked(m_pStation);

		m_bSignalDock = false;
		}

	DEBUG_CATCH
	}

void CPlayerShipController::OnWeaponStatusChanged (void)

//	OnWeaponStatusChanged
//
//	Weapon status has changed

	{
	m_pTrans->UpdateWeaponStatus();
	}

void CPlayerShipController::ReadFromStream (SLoadCtx &Ctx, CShip *pShip)

//	ReadFromStream
//
//	Reads data from stream
//
//	DWORD		m_iGenome
//	DWORD		m_dwStartingShipClass
//	DWORD		m_pShip (CSpaceObject ref)
//	DWORD		m_pStation (CSpaceObject ref)
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pDestination (CSpaceObject ref)
//	DWORD		m_iOrder
//	DWORD		m_iManeuver
//	CCurrencyBlock m_Credits
//	CPlayerGameStats m_Stats
//	DWORD		UNUSED (m_iInsuranceClaims)
//	DWORD		flags

	{
	int i;
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&m_iGenome, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_dwStartingShipClass, sizeof(DWORD));
	CSystem::ReadObjRefFromStream(Ctx, (CSpaceObject **)&m_pShip);
	CSystem::ReadObjRefFromStream(Ctx, &m_pStation);
	CSystem::ReadObjRefFromStream(Ctx, &m_pTarget);
	CSystem::ReadObjRefFromStream(Ctx, &m_pDestination);

	if (Ctx.dwVersion >= 100)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_iOrder = (OrderTypes)dwLoad;
		}
	else
		m_iOrder = orderNone;

	Ctx.pStream->Read((char *)&m_iManeuver, sizeof(DWORD));

	if (Ctx.dwVersion >= 49)
		m_Credits.ReadFromStream(Ctx);
	else
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_Credits.SetCredits(CONSTLIT("credit"), dwLoad);
		}

	m_Stats.ReadFromStream(Ctx);

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));

	//	Flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_bThrust = false;
	m_bActivate = false;
	m_bStopThrust = false;
	m_bUnderAttack =			((dwLoad & 0x00000001) ? true : false);
	m_bMapHUD =					((dwLoad & 0x00001000) ? true : false);
	m_iLastHelpTick = 0;
	m_iLastHelpUseTick = 0;
	m_iLastHelpFireMissileTick = 0;

	//	Message controller

	if (Ctx.dwVersion >= 63)
		m_UIMsgs.ReadFromStream(Ctx);
	else
		{
		//	Initialize from flags

		m_UIMsgs.SetEnabled(uimsgDockHint,			((dwLoad & 0x00000020) ? true : false));
		m_UIMsgs.SetEnabled(uimsgMapHint,			((dwLoad & 0x00000040) ? true : false));
		m_UIMsgs.SetEnabled(uimsgAutopilotHint,		((dwLoad & 0x00000080) ? true : false));
		m_UIMsgs.SetEnabled(uimsgGateHint,			((dwLoad & 0x00000100) ? true : false));
		m_UIMsgs.SetEnabled(uimsgUseItemHint,		((dwLoad & 0x00000200) ? true : false));
		m_UIMsgs.SetEnabled(uimsgRefuelHint,		((dwLoad & 0x00000400) ? true : false));
		m_UIMsgs.SetEnabled(uimsgEnableDeviceHint,	((dwLoad & 0x00000800) ? true : false));
		m_UIMsgs.SetEnabled(uimsgSwitchMissileHint,	((dwLoad & 0x00002000) ? true : false));
		m_UIMsgs.SetEnabled(uimsgFireMissileHint,	((dwLoad & 0x00004000) ? true : false));
		m_UIMsgs.SetEnabled(uimsgCommsHint,			((dwLoad & 0x00008000) ? true : false));
		}

	//	Deities

	if (Ctx.dwVersion < 25)
		{
		int iDummy;

		for (i = 0; i < 2; i++)
			{
			Ctx.pStream->Read((char *)&iDummy, sizeof(DWORD));
			Ctx.pStream->Read((char *)&iDummy, sizeof(DWORD));

			DWORD dwCount;
			Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
			for (int j = 0; j < (int)dwCount; j++)
				Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
			}
		}

	//	Deferred destruction string

	if (Ctx.dwVersion >= 3 && Ctx.dwVersion < 50)
		{
		CString sDummy;
		sDummy.ReadFromStream(Ctx.pStream);
		}

	//	For backwards compatibility we move rin from object data to
	//	the dedicated currency block structure

	if (Ctx.dwVersion < 62)
		{
		int iRin = strToInt(pShip->GetData(CONSTLIT("rins")), 0);
		if (iRin > 0)
			{
			CEconomyType *pRinEcon = g_pUniverse->FindEconomyType(CONSTLIT("rin"));
			if (pRinEcon)
				m_Credits.SetCredits(pRinEcon->GetUNID(), iRin);

			pShip->SetData(CONSTLIT("rins"), NULL_STR);
			}
		}

	m_pTrans = g_pTrans;
	}

void CPlayerShipController::ReadyNextMissile (int iDir)

//	ReadyNextMissile
//
//	Select the next missile
	
	{
	m_pShip->ReadyNextMissile(iDir);

	CInstalledDevice *pLauncher = m_pShip->GetNamedDevice(devMissileWeapon);
	if (pLauncher)
		{
		if (pLauncher->GetValidVariantCount(m_pShip) == 0)
			m_pTrans->DisplayMessage(CONSTLIT("No missiles on board"));
		else
			{
			CString sVariant;
			int iAmmoLeft;
			pLauncher->GetSelectedVariantInfo(m_pShip, &sVariant, &iAmmoLeft);
			if (sVariant.IsBlank())
				sVariant = pLauncher->GetName();
			m_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("%s ready"), sVariant));
			}
		}
	else
		m_pTrans->DisplayMessage(CONSTLIT("No launcher installed"));
	}

void CPlayerShipController::ReadyNextWeapon (int iDir)

//	ReadyNextWeapon
//
//	Select the next primary weapon

	{
	CInstalledDevice *pCurWeapon = m_pShip->GetNamedDevice(devPrimaryWeapon);
	CInstalledDevice *pNewWeapon = pCurWeapon;

	//	Keep switching until we find a weapon that is not disabled

	do
		{
		m_pShip->ReadyNextWeapon(iDir);
		pNewWeapon = m_pShip->GetNamedDevice(devPrimaryWeapon);
		}
	while (pNewWeapon 
			&& pCurWeapon
			&& pNewWeapon != pCurWeapon
			&& !pNewWeapon->IsEnabled());

	//	Done

	if (pNewWeapon)
		{
		//	There is a delay in activation

		m_pShip->SetFireDelay(pNewWeapon);

		//	Feedback to player

		if (pNewWeapon->IsEnabled())
			m_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("%s ready"), strCapitalize(pNewWeapon->GetName())));
		else
			m_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("Disabled %s selected"), pNewWeapon->GetName()));
		}
	}

void CPlayerShipController::Reset (void)

//	Reset
//
//	Resets the ship (used when changing systems, etc.)

	{
	DEBUG_TRY

	//	Clear target

	if (m_pTarget)
		{
		SetTarget(NULL);
		m_pTrans->UpdateWeaponStatus();
		}

	//	Clear destination

	if (m_pDestination)
		SetDestination(NULL);

	//	Clear target list

	m_TargetList.RemoveAll();

	//	Clear orders

	m_iOrder = orderNone;

	//	Clear auto

	if (m_pAutoDamage)
		{
		m_pAutoDamage->ClearShowDamageBar();
		m_pAutoDamage = NULL;
		}

	m_pAutoDock = NULL;
	m_pAutoTarget = NULL;
	m_bSignalDock = false;

	DEBUG_CATCH
	}

void CPlayerShipController::SelectNearestTarget (void)

//	SelectNearestTarget
//
//	Selects the nearest enemy target

	{
	//	Must have a targeting computer

	if (!m_pShip->HasTargetingComputer())
		{
		m_pTrans->DisplayMessage(STR_NO_TARGETING_COMPUTER);
		return;
		}

	//	Initialize target list

	InitTargetList(targetEnemies);
	if (m_TargetList.GetCount() > 0)
		SetTarget(m_TargetList.Get(0));
	else
		SetTarget(NULL);
	}

void CPlayerShipController::SetDestination (CSpaceObject *pTarget)

//	SetDestination
//
//	Sets the destination for the player

	{
	//	Select

	if (m_pDestination)
		m_pDestination->ClearPlayerDestination();

	m_pDestination = pTarget;

	if (m_pDestination)
		m_pDestination->SetPlayerDestination();
	}

void CPlayerShipController::SetFireMain (bool bFire)

//	SetFireMain
//
//	Fire main weapon

	{
	m_pShip->SetWeaponTriggered(devPrimaryWeapon, bFire);
	}

void CPlayerShipController::SetFireMissile (bool bFire)

//	SetFireMissile
//
//	Fire launcher weapon

	{
	m_pShip->SetWeaponTriggered(devMissileWeapon, bFire);
	}

void CPlayerShipController::SetTarget (CSpaceObject *pTarget)

//	SetTarget
//
//	Sets the target

	{
	//	Select

	if (m_pTarget)
		{
		m_pTarget->ClearSelection();
		m_pTarget->ClearPlayerTarget();
		}

	m_pTarget = pTarget;

	if (m_pTarget)
		{
		m_pTarget->SetSelection();
		m_pTarget->SetPlayerTarget();
		}
	else
		{
		m_TargetList.RemoveAll();
		ClearFireAngle();
		}
	}

void CPlayerShipController::SelectNextFriendly (int iDir)

//	SelectNextFriendly
//
//	Sets the target to the next nearest friendly object

	{
	//	Must have a targeting computer

	if (!m_pShip->HasTargetingComputer())
		{
		m_pTrans->DisplayMessage(STR_NO_TARGETING_COMPUTER);
		return;
		}

	//	If a friendly is already selected, then cycle
	//	to the next friendly.

	if (m_pTarget && !(m_pShip->IsEnemy(m_pTarget) && m_pTarget->CanAttack()))
		{
		InitTargetList(targetFriendlies, true);

		//	If we're going forwards, we default to first target; otherwise
		//	we default to last target
		
		int iDefault = (iDir == 1 ? 0 : m_TargetList.GetCount() - 1);

		//	Select

		if (m_TargetList.GetCount() > 0)
			{
			if (m_pTarget)
				{
				//	Look for the current target

				int iIndex;
				if (m_TargetList.Find(m_pTarget, &iIndex))
					{
					iIndex += iDir;

					if (iIndex >= 0 && iIndex < m_TargetList.GetCount())
						SetTarget(m_TargetList.Get(iIndex));
					else
						SetTarget(m_TargetList.Get(iDefault));
					}
				else
					SetTarget(m_TargetList.Get(iDefault));
				}
			else
				SetTarget(m_TargetList.Get(iDefault));
			}
		else
			SetTarget(NULL);
		}

	//	Otherwise, select the nearest friendly

	else
		{
		InitTargetList(targetFriendlies);

		if (m_TargetList.GetCount() > 0)
			SetTarget(m_TargetList.Get(iDir == 1 ? 0 : m_TargetList.GetCount() - 1));
		else
			SetTarget(NULL);
		}
	}

void CPlayerShipController::SelectNextTarget (int iDir)

//	SelectNextTarget
//
//	Selects the next target

	{
	//	Must have a targeting computer

	if (!m_pShip->HasTargetingComputer())
		{
		m_pTrans->DisplayMessage(STR_NO_TARGETING_COMPUTER);
		return;
		}

	//	If an enemy target is already selected, then cycle
	//	to the next enemy.

	if (m_pTarget && m_pShip->IsEnemy(m_pTarget) && m_pTarget->CanAttack())
		{
		InitTargetList(targetEnemies, true);

		//	If we're going forwards, we default to first target; otherwise
		//	we default to last target
		
		int iDefault = (iDir == 1 ? 0 : m_TargetList.GetCount() - 1);

		//	Select

		if (m_TargetList.GetCount() > 0)
			{
			if (m_pTarget)
				{
				//	Look for the current target

				int iIndex;
				if (m_TargetList.Find(m_pTarget, &iIndex))
					{
					iIndex += iDir;

					if (iIndex >= 0 && iIndex < m_TargetList.GetCount())
						SetTarget(m_TargetList.Get(iIndex));
					else
						SetTarget(m_TargetList.Get(iDefault));
					}
				else
					SetTarget(m_TargetList.Get(iDefault));
				}
			else
				SetTarget(m_TargetList.Get(iDefault));
			}
		else
			SetTarget(NULL);
		}

	//	Otherwise, select the nearest enemy target

	else
		{
		InitTargetList(targetEnemies);

		if (m_TargetList.GetCount() > 0)
			SetTarget(m_TargetList.Get(iDir == 1 ? 0 : m_TargetList.GetCount() - 1));
		else
			SetTarget(NULL);
		}
	}

ALERROR CPlayerShipController::SwitchShips (CShip *pNewShip)

//	SwitchShips
//
//	Switches the player's ship

	{
	int i;

	ASSERT(m_pShip);
	CShip *pOldShip = m_pShip;

	//	If the new ship is docked with a station then make sure that it
	//	is not a subordinate of the station.

	CSpaceObject *pStation = pNewShip->GetDockedObj();
	if (pStation)
		pStation->RemoveSubordinate(pNewShip);

	//	If we're docked and the new ship is also docked with the same
	//	object, then we stay docked; otherwise, we undock

	if (m_pStation && m_pStation != pNewShip->GetDockedObj())
		{
		g_pTrans->GetModel().ExitScreenSession(true);
		pNewShip->Undock();
		}

	//	Set a new controller for the old ship (but do not free
	//	the previous controller, which is us)

	pOldShip->SetController(g_pUniverse->CreateShipController(NULL_STR), false);
	pOldShip->GetController()->AddOrder(IShipController::orderWait, NULL, IShipController::SData());

	//	Old ship stops tracking fuel (otherwise, it would run out)

	pOldShip->TrackFuel(false);

	//	Now set this controller to drive the new ship. gPlayer and gPlayerShip
	//	will be set inside of SetPlayerShip.

	pNewShip->SetController(this);
	m_pShip = pNewShip;
	g_pUniverse->SetPlayerShip(pNewShip);
	g_pUniverse->SetPOV(pNewShip);
	pNewShip->SetSovereign(g_pUniverse->FindSovereign(g_PlayerSovereignUNID));

	//	Move any data from the old ship to the new ship
	//	(we leave it on the old ship just in case)

	pNewShip->CopyDataFromObj(pOldShip);

	//	New ship gets some properties by default

	pNewShip->TrackFuel();
	pNewShip->TrackMass();

	//	Reset all other settings

	SetTarget(NULL);
	SetDestination(NULL);
	m_pAutoDock = NULL;
	m_iManeuver = NoRotation;
	m_bThrust = false;
	m_bStopThrust = false;
	SetFireMain(false);
	SetFireMissile(false);
	m_bActivate = false;

	//	Call all types and tell them that we've switched ships

	g_pUniverse->FireOnGlobalPlayerChangedShips(pOldShip);

	//	Call all objects in the system and tell them that we've
	//	changed ships.

	CSystem *pSystem = g_pUniverse->GetCurrentSystem();
	ASSERT(pSystem);

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);
		if (pObj 
				&& !pObj->IsDestroyed()
				&& pObj != pOldShip
				&& pObj != pNewShip)
			pObj->OnPlayerChangedShips(pOldShip);
		}

	//	Update displays

	m_pTrans->UpdateArmorDisplay();

	return NOERROR;
	}

void CPlayerShipController::Undock (void)
	{
	if (m_pStation)
		{
		m_pShip->Undock();
		m_pStation = NULL;
		}
	}

void CPlayerShipController::Update (int iTick)

//	Update
//
//	Updates each tick

	{
	//	Update help

	if ((iTick % UPDATE_HELP_TIME) == 0)
		UpdateHelp(iTick);

#ifdef DEBUG_TARGET_LIST
//	if ((iTick % 30) == 0)
//		InitTargetList(targetEnemies, true);

	for (int i = 0; i < m_TargetList.GetCount(); i++)
		{
		CSpaceObject *pObj = m_TargetList.Get(i);
		ASSERT(!pObj->IsDestroyed());
		ASSERT(pObj->GetType());

		if ((iTick % 30) == 0)
			::kernelDebugLogMessage("Target %d: %s (%x)", i, pObj->GetName(), (DWORD)pObj);
		}
#endif
	}

void CPlayerShipController::UpdateHelp (int iTick)

//	UpdateHelp
//
//	Checks to see if we should show a help message to the player

	{
	//	If we just showed help, then skip

	if ((iTick - m_iLastHelpTick) < 240)
		return;

	//	See if we need to show help and if we can show help at this point

	if (!m_UIMsgs.IsEnabled(uimsgAllHints))
		return;

	//	Don't bother if we're docking, or gating, etc.

	if (!m_pTrans->InGameState() 
			|| DockingInProgress() 
			|| m_pTrans->InAutopilot()
			|| m_pTrans->InMenu()
			|| m_pShip->IsDestroyed()
			|| m_pShip->GetSystem() == NULL)
		return;

	//	See if there are enemies in the area

	bool bEnemiesInRange = m_pShip->IsEnemyInRange(MAX_IN_COMBAT_RANGE, true);

	//	If we've never docked and we're near a dockable station, then tell
	//	the player about docking.

	if (m_UIMsgs.IsEnabled(uimsgDockHint))
		{
		if (!bEnemiesInRange
				&& !m_pTrans->InMap()
				&& m_pAutoDock)
			{
			m_pTrans->DisplayMessage(CONSTLIT("(press [D] to dock with stations and wrecks)"));
			m_iLastHelpTick = iTick;
			return;
			}
		}

	//	If we've never used an item, and we've got a usable item in cargo
	//	and we're not in the middle of anything, then tell the player.

	if (m_UIMsgs.IsEnabled(uimsgUseItemHint) && !bEnemiesInRange)
		{
		CItemListManipulator ItemList(m_pShip->GetItemList());
		CItemCriteria UsableItems;
		CItem::ParseCriteria(CONSTLIT("u"), &UsableItems);
		ItemList.SetFilter(UsableItems);
		bool bHasUsableItems = ItemList.MoveCursorForward();

		if (!m_pTrans->InMap() 
				&& bHasUsableItems
				&& (m_iLastHelpUseTick == 0 || (iTick - m_iLastHelpUseTick) > 9000))
			{
			m_pTrans->DisplayMessage(CONSTLIT("(press [U] to use items in your cargo hold)"));
			m_iLastHelpTick = iTick;
			m_iLastHelpUseTick = iTick;
			return;
			}
		}

	//	If we've never used the map, and then tell the player about the map

	if (m_UIMsgs.IsEnabled(uimsgMapHint) && !bEnemiesInRange)
		{
		m_pTrans->DisplayMessage(CONSTLIT("(press [M] to see a map of the system)"));
		m_iLastHelpTick = iTick;
		return;
		}

	//	If we've never used autopilot, and we're not in the middle of
	//	anything, then tell the player about autopilot

	if (m_UIMsgs.IsEnabled(uimsgAutopilotHint) && !bEnemiesInRange && m_pShip->HasAutopilot())
		{
		Metric rSpeed = m_pShip->GetVel().Length();

		if (rSpeed > 0.9 * m_pShip->GetMaxSpeed())
			{
			m_pTrans->DisplayMessage(CONSTLIT("(press [A] to engage autopilot and travel faster)"));
			m_iLastHelpTick = iTick;
			return;
			}
		}

	//	If we've never entered a gate, and there is a gate nearby
	//	and we're not in the middle of anything, then tell the player.

	if (m_UIMsgs.IsEnabled(uimsgGateHint))
		{
		if (!bEnemiesInRange
				&& !m_pTrans->InMap() 
				&& m_pShip->IsStargateInRange(MAX_STARGATE_HELP_RANGE))
			{
			m_pTrans->DisplayMessage(CONSTLIT("(press [G] over stargate to travel to next system)"));
			m_iLastHelpTick = iTick;
			return;
			}
		}

	//	If we've never communicated and there are objects to communicate with

	if (m_UIMsgs.IsEnabled(uimsgCommsHint))
		{
		if (!bEnemiesInRange
				&& HasCommsTarget())
			{
			m_pTrans->DisplayMessage(CONSTLIT("(press [C] to communicate)"));
			m_iLastHelpTick = iTick;
			return;
			}
		}

	//	If we've never switched missiles and we have more than one

	if (m_UIMsgs.IsEnabled(uimsgSwitchMissileHint))
		{
		if (!bEnemiesInRange &&
				m_pShip->GetMissileCount() > 1)
			{
			m_pTrans->DisplayMessage(CONSTLIT("(press [Tab] to switch missiles)"));
			m_iLastHelpTick = iTick;
			return;
			}
		}

	//	If we've never fired a missile and we have one and we're under attack

	if (m_UIMsgs.IsEnabled(uimsgFireMissileHint))
		{
		if (bEnemiesInRange
				&& m_pShip->GetMissileCount() > 0 
				&& (m_iLastHelpFireMissileTick == 0 || (iTick - m_iLastHelpFireMissileTick) > 9000))
			{
			m_pTrans->DisplayMessage(CONSTLIT("(press [Shift] to fire missiles)"));
			m_iLastHelpTick = iTick;
			m_iLastHelpFireMissileTick = iTick;
			return;
			}
		}

	//	If we've never used the galactic map, and then tell the player about the map

	if (m_UIMsgs.IsEnabled(uimsgGalacticMapHint) 
			&& !m_UIMsgs.IsEnabled(uimsgGateHint)
			&& !bEnemiesInRange
			&& IsGalacticMapAvailable())
		{
		m_pTrans->DisplayMessage(CONSTLIT("(press [N] to see the stargate network)"));
		m_iLastHelpTick = iTick;
		return;
		}
	}

void CPlayerShipController::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write data to stream
//
//	DWORD		m_iGenome
//	DWORD		m_dwStartingShipClass
//	DWORD		m_pShip (CSpaceObject ref)
//	DWORD		m_pStation (CSpaceObject ref)
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pDestination (CSpaceObject ref)
//	DWORD		m_iOrder
//	DWORD		m_iManeuver
//	CCurrencyBlock m_Credits
//	CPlayerGameStats m_Stats
//	DWORD		UNUSED (m_iInsuranceClaims)
//	DWORD		flags

	{
	DWORD dwSave;

	GetClass().WriteToStream(pStream);

	pStream->Write((char *)&m_iGenome, sizeof(DWORD));
	pStream->Write((char *)&m_dwStartingShipClass, sizeof(DWORD));
	m_pShip->WriteObjRefToStream(m_pShip, pStream);
	m_pShip->WriteObjRefToStream(m_pStation, pStream);
	m_pShip->WriteObjRefToStream(m_pTarget, pStream);
	m_pShip->WriteObjRefToStream(m_pDestination, pStream);

	dwSave = (DWORD)m_iOrder;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_iManeuver, sizeof(DWORD));
	m_Credits.WriteToStream(pStream);
	m_Stats.WriteToStream(pStream);
	dwSave = 0;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = 0;
	dwSave |= (m_bUnderAttack ?			0x00000001 : 0);
	//	0x00000002
	//	0x00000004
	//	0x00000008
	//	0x00000010
	//	0x00000020
	//	0x00000040
	//	0x00000080
	//	0x00000100
	//	0x00000200
	//	0x00000400
	//	0x00000800
	dwSave |= (m_bMapHUD ?				0x00001000 : 0);
	//	0x00002000
	//	0x00004000
	//	0x00008000
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	m_UIMsgs.WriteToStream(pStream);
	}
