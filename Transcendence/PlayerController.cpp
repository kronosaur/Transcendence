//	PlayerController.cpp
//
//	Implements class to control player's ship

#include "PreComp.h"
#include "Transcendence.h"

#define INVOKE_REFRESH_INTERVAL					100		//	Ticks to gain 1 point of deity rel (if rel is negative)

#define STR_NO_TARGET_FOR_FLEET					CONSTLIT("No target selected")
#define STR_NO_TARGETING_COMPUTER				CONSTLIT("No targeting computer installed")

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_VTABLE,		1,	0 },		//	IShipController virtuals
		{ DATADESC_OPCODE_REFERENCE,	1,	0 },		//	m_pTrans
		{ DATADESC_OPCODE_INT,			4,	0 },		//	ints
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CPlayerShipController>g_Class(OBJID_CPLAYERSHIPCONTROLLER, g_DataDesc);

const Metric MAX_IN_COMBAT_RANGE =				(LIGHT_SECOND * 30.0);
const int UPDATE_HELP_TIME =					31;
const Metric MAX_AUTO_TARGET_DISTANCE =			(LIGHT_SECOND * 30.0);

#define MAX_DOCK_DISTANCE						(g_KlicksPerPixel * 256.0)
#define MAX_GATE_DISTANCE						(g_KlicksPerPixel * 64.0)
#define MAX_STARGATE_HELP_RANGE					(g_KlicksPerPixel * 256.0)

CPlayerShipController::CPlayerShipController (void) : CObject(&g_Class),
		m_pTrans(NULL),
		m_iManeuver(IShipController::NoRotation),
		m_bThrust(false),
		m_bActivate(false),
		m_bStopThrust(false),
		m_iAutoTargetTick(0),
		m_pAutoTarget(NULL),
		m_pStation(NULL),
		m_pTarget(NULL),
		m_pDestination(NULL),
		m_dwWreckObjID(OBJID_NULL),
		m_iLastHelpTick(0),
		m_iLastHelpUseTick(0),
		m_iLastHelpFireMissileTick(0),
		m_bMapHUD(false)

//	CPlayerShipController constructor

	{
	}

CPlayerShipController::~CPlayerShipController (void)

//	CPlayerShipController destructor

	{
	}

void CPlayerShipController::AddOrder(OrderTypes Order, CSpaceObject *pTarget, DWORD dwData, bool bAddBefore)

//	AddOrder
//
//	We get an order

	{
	//	For now, we only deal with one order at a time

	switch (Order)
		{
		case orderGuard:
		case orderEscort:
		case orderDock:
			SetDestination(pTarget);
			break;

		case orderAimAtTarget:
		case orderDestroyTarget:
			if (m_pShip->HasTargetingComputer())
				SetTarget(pTarget);

			SetDestination(pTarget);
			break;
		}
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
		if (pDevice && !pDevice->IsEmpty() && pDevice->CanBeDisabled())
			{
			if (!pDevice->IsEnabled())
				return false;
			}
		}

	return true;
	}

void CPlayerShipController::CancelAllOrders (void)

//	CancelAllOrder
//
//	Cancel all orders

	{
	SetDestination(NULL);
	}

void CPlayerShipController::CancelCurrentOrder (void)

//	CancelCurrentOrder
//
//	Cancel current order

	{
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
	//	If we're already in the middle of docking, cancel...

	if (m_pStation)
		{
		m_pStation->Undock(m_pShip);
		m_pStation = NULL;
		m_pTrans->DisplayMessage(CONSTLIT("Docking canceled"));
		return;
		}

	CSystem *pSystem = m_pShip->GetSystem();
	CSpaceObject *pStation = NULL;
	Metric rMaxDist2 = MAX_DOCK_DISTANCE * MAX_DOCK_DISTANCE;

	//	See if the targeted object supports docking

	if (m_pTarget 
			&& m_pTarget != m_pShip 
			&& m_pTarget->SupportsDocking()
			&& (!m_pShip->IsEnemy(m_pTarget) || m_pTarget->IsAbandoned()))
		{
		CVector vDist = m_pTarget->GetPos() - m_pShip->GetPos();
		Metric rDist2 = vDist.Length2();

		if (rDist2 < rMaxDist2)
			pStation = m_pTarget;
		}

	//	Find the station closest to the ship

	if (pStation == NULL)
		pStation = FindDockTarget();

	//	If we did not find a station then we're done

	if (pStation == NULL)
		{
		m_pTrans->DisplayMessage(CONSTLIT("No stations in range"));
		return;
		}

	//	Otherwise, request docking

	if (!pStation->RequestDock(m_pShip))
		return;

	//	Station has agreed to allow dock...

	m_pStation = pStation;

	SetActivate(false);
	SetFireMain(false);
	SetFireMissile(false);
	SetManeuver(IShipController::NoRotation);
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
		if (pDevice && !pDevice->IsEmpty() && pDevice->CanBeDisabled() && pDevice->IsEnabled() != bEnable)
			{
			m_pShip->EnableDevice(i, bEnable);

			//	Note: We will get called at OnDeviceEnabledDisabled for each
			//	device that we touch.
			}
		}
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
				&& pObj->SupportsDocking()
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

				CVector vDockPos = pObj->GetNearestDockVector(m_pShip);
				Metric rDockDist2 = vDockPos.Length2();

				if (rDockDist2 < rBestDist)
					{
					rBestDist = rDockDist2;
					pStation = pObj;
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
	Metric rBestDist = MAX_GATE_DISTANCE * MAX_GATE_DISTANCE;
	CSpaceObject *pStation = NULL;
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
			Metric rDist = vDist.Length2();

			if (rDist < rBestDist)
				{
				rBestDist = rDist;
				pStation = pObj;
				}
			}
		}

	//	If we did not find a station then we're done

	if (pStation == NULL)
		{
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
				&& pObj->CanCommunicateWith(m_pShip, true)
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

void CPlayerShipController::InitTargetList (TargetTypes iTargetType, bool bUpdate)

//	InitTargetList
//
//	Initializes the target list

	{
	int i;

	//	Compute some invariants based on our perception

	int iPerception = m_pShip->GetPerception();

	//	Make a list of all targets

	if (!bUpdate)
		m_TargetList.RemoveAll();

	CSystem *pSystem = m_pShip->GetSystem();
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
					if (pObj->CanAttack() || pObj->SupportsDocking())
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
	bool bHandled;
	CString sMessage;

	//	First ask the sender to translate the message

	CString sID = GetMessageID(iMessage);
	if (!sID.IsBlank())
		bHandled = pSender->Translate(sID, &sMessage);
	else
		bHandled = false;

	//	If the sender did not handle it and if it is an older version, then use
	//	the now deprecated <OnTranslateMessage>

	if (!bHandled && pSender->GetVersion() < 3)
		bHandled = pSender->FireOnTranslateMessage(sID, &sMessage);

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
		m_pTrans->DisplayMessage(sMessage);
		if (pSender)
			pSender->Highlight(CG16bitImage::RGBValue(0, 255, 0));
		}

	return resNoAnswer;
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
	int iMaxArmorHP = pArmor->GetMaxHP(m_pShip);

	m_pTrans->Autopilot(false);

	//	Heavy damage (>= 5%) causes screen flash

	if (iDamage >= (iMaxArmorHP / 20) && Damage.CausesSRSFlash())
		m_pTrans->DamageFlash();

	//	If we're down to 25% armor, then warn the player

	if (pArmor->GetHitPoints() < (iMaxArmorHP / 4))
		m_pTrans->DisplayMessage(CONSTLIT("Hull breach imminent!"));

	m_pTrans->UpdateArmorDisplay();
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
	//	Clear various variables

	if (m_pTarget)
		{
		SetTarget(NULL);
		m_pTrans->UpdateWeaponStatus();
		}

	if (m_pDestination)
		SetDestination(NULL);

	//	Reset everything else

	m_iManeuver = IShipController::NoRotation;
	m_bThrust = false;
	m_bStopThrust = false;
	SetFireMain(false);
	SetFireMissile(false);
	m_bActivate = false;

	//	Notify

	CString sEpitaph;
	g_pTrans->GetModel().OnPlayerDestroyed(Ctx, &sEpitaph);

	m_pTrans->PlayerDestroyed(sEpitaph, Ctx.bResurrectPending);
	}

void CPlayerShipController::OnDeviceEnabledDisabled (int iDev, bool bEnable)

//	OnDeviceEnabledDisabled
//
//	Device has been disabled (generally by reactor overload)

	{
	CInstalledDevice *pDevice = m_pShip->GetDevice(iDev);
	if (pDevice && !pDevice->IsEmpty())
		{
		if (!bEnable)
			{
			if (m_UIMsgs.IsEnabled(uimsgEnableDeviceHint))
				m_pTrans->DisplayMessage(CONSTLIT("(press [B] to enable/disable devices)"));
			m_pTrans->DisplayMessage(strCapitalize(strPatternSubst(CONSTLIT("%s disabled"),
					pDevice->GetClass()->GetName())));
			}
		else
			{
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
		SetManeuver(IShipController::NoRotation);
		SetFireMain(false);
		SetFireMissile(false);
		}

	//	Don't warn the player every time

	else if ((iSeq % 15) == 0)
		{
		if (m_UIMsgs.IsEnabled(uimsgRefuelHint))
			m_pTrans->DisplayMessage(CONSTLIT("(press [S] to access refueling screen)"));
		m_pTrans->DisplayMessage(CONSTLIT("Fuel low!"));
		}
	}

void CPlayerShipController::OnEnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate)

//	OnEnterGate
//
//	Enter stargate

	{
	//	Clear our targeting computer (otherwise, we crash since we archive ships
	//	in the old system)

	if (m_pTarget)
		{
		SetTarget(NULL);
		m_pTrans->UpdateWeaponStatus();
		}

	if (m_pDestination)
		SetDestination(NULL);

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

void CPlayerShipController::OnRadiationWarning (int iSecondsLeft)

//	OnRadiationWarning
//
//	Handle radiation warning

	{
	if (iSecondsLeft > 10 && ((iSecondsLeft % 5) != 0))
		NULL;
	else if (iSecondsLeft > 1)
		m_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("Radiation Warning: Fatal exposure in %d seconds"), iSecondsLeft));
	else if (iSecondsLeft == 1)
		m_pTrans->DisplayMessage(CONSTLIT("Radiation Warning: Fatal exposure in 1 second"));
	else
		m_pTrans->DisplayMessage(CONSTLIT("Radiation Warning: Fatal exposure received"));
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
		m_pTrans->DisplayMessage(CONSTLIT("Warning: Reactor overload"));
	}

void CPlayerShipController::OnStartGame (void)

//	OnStartGame
//
//	Game (either new or loaded) has just started

	{
	m_iManeuver = IShipController::NoRotation;
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

		if (pDevice && pDevice->CanBeDisabled())
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

IShipController::ManeuverTypes CPlayerShipController::GetManeuver (void)
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

CSpaceObject *CPlayerShipController::GetTarget (bool bNoAutoTarget) const

//	GetTarget
//
//	Returns the target for the player ship

	{
	if (m_pTarget || bNoAutoTarget)
		return m_pTarget;
	else
		{
		int iTick = g_pUniverse->GetTicks();
		if (iTick == m_iAutoTargetTick)
			return m_pAutoTarget;

		m_iAutoTargetTick = iTick;
		m_pAutoTarget = m_pShip->GetNearestVisibleEnemy(MAX_AUTO_TARGET_DISTANCE, true);

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

		return m_pAutoTarget;
		}
	}

void CPlayerShipController::OnDocked (CSpaceObject *pObj)
	{
	g_pTrans->GetModel().OnPlayerDocked(pObj);
	}

void CPlayerShipController::OnDockedObjChanged (CSpaceObject *pLocation)
	{
	g_pTrans->GetModel().OnDockedObjChanged(pLocation);
	}

void CPlayerShipController::OnMessage (CSpaceObject *pSender, const CString &sMsg)

//	OnMessage
//
//	Receive a message from some other object

	{
	if (pSender)
		pSender->Highlight(CG16bitImage::RGBValue(0, 255, 0));

	m_pTrans->DisplayMessage(sMsg);
	}

void CPlayerShipController::OnNewSystem (void)

//	OnNewSystem
//
//	Player has arrived at a new system

	{
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
		g_pTrans->GetModel().ExitScreenSession(true);
		ASSERT(m_pStation == NULL);
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
		pHacker->Highlight(CG16bitImage::RGBValue(0, 255, 0));

	m_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("Cyberattack detected: %s"), Program.sProgramName));
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
	Ctx.pSystem->ReadObjRefFromStream(Ctx, (CSpaceObject **)&m_pShip);
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pStation);
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pTarget);
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pDestination);
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
	int i;

	//	Get the primary weapon

	CInstalledDevice *pPrimaryDevice = m_pShip->GetNamedDevice(devPrimaryWeapon);

	//	Loop over all devices and activate the appropriate ones

	for (i = 0; i < m_pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = m_pShip->GetDevice(i);
		CItemCtx Ctx(m_pShip, pDevice);

		//	If this is the primary device, or if it is a device that
		//	is linked to the primary device, then activate it.

		if (!pDevice->IsEmpty()
				&& (pDevice == pPrimaryDevice 
					|| (pDevice->IsLinkedFire(Ctx, itemcatWeapon))))
			{
			pDevice->SetTriggered(bFire);

			//	If we don't have a target, set our fire angle to fire straight ahead
			//	we need this for omni-directional weapons

			if (bFire && m_pTarget == NULL)
				pDevice->SetFireAngle(-1);
			}
		}
	}

void CPlayerShipController::SetFireMissile (bool bFire)

//	SetFireMissile
//
//	Fire launcher weapon

	{
	int i;

	//	Get the primary weapon

	CInstalledDevice *pLauncherDevice = m_pShip->GetNamedDevice(devMissileWeapon);

	//	Loop over all devices and activate the appropriate ones

	for (i = 0; i < m_pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = m_pShip->GetDevice(i);
		CItemCtx Ctx(m_pShip, pDevice);

		//	If this is the launcher device, or if it is a device that
		//	is linked to the launcher device, then activate it.

		if (!pDevice->IsEmpty()
				&& (pDevice == pLauncherDevice
					|| (pDevice->IsLinkedFire(Ctx, itemcatLauncher))))
			{
			pDevice->SetTriggered(bFire);

			//	If we don't have a target, set our fire angle to fire straight ahead
			//	we need this for omni-directional weapons

			if (bFire && m_pTarget == NULL)
				pDevice->SetFireAngle(-1);
			}
		}
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

	pOldShip->SetController(new CStandardShipAI, false);
	pOldShip->GetController()->AddOrder(IShipController::orderWait, NULL, 0);

	//	Old ship stops tracking fuel (otherwise, it would run out)

	pOldShip->TrackFuel(false);

	//	Free the new ship's controller

	IShipController *pNewShipController = pNewShip->GetController();
	if (pNewShipController)
		delete pNewShipController;
	
	//	Now set this controller to drive the new ship. gPlayer and gPlayerShip
	//	will be set inside of SetPlayer.

	pNewShip->SetController(this);
	m_pShip = pNewShip;
	g_pUniverse->SetPlayer(pNewShip);
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
	m_iManeuver = IShipController::NoRotation;
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

	if (iTick % UPDATE_HELP_TIME)
		UpdateHelp(iTick);
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
				&& FindDockTarget())
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
//	DWORD		m_iManeuver
//	CCurrencyBlock m_Credits
//	CPlayerGameStats m_Stats
//	DWORD		UNUSED (m_iInsuranceClaims)
//	DWORD		flags

	{
	DWORD dwSave;

	dwSave = (DWORD)GetClass()->GetObjID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_iGenome, sizeof(DWORD));
	pStream->Write((char *)&m_dwStartingShipClass, sizeof(DWORD));
	m_pShip->WriteObjRefToStream(m_pShip, pStream);
	m_pShip->WriteObjRefToStream(m_pStation, pStream);
	m_pShip->WriteObjRefToStream(m_pTarget, pStream);
	m_pShip->WriteObjRefToStream(m_pDestination, pStream);
	pStream->Write((char *)&m_iManeuver, sizeof(DWORD));
	m_Credits.WriteToStream(pStream);
	m_Stats.WriteToStream(pStream);
	dwSave = 0;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = 0;
	//	0x00000001
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
