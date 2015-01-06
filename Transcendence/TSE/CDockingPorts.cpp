//	CDockingPorts.cpp
//
//	CDockingPorts class

#include "PreComp.h"

#define FINAL_DOCKING							(8.0 * g_KlicksPerPixel)
#define FINAL_DOCKING2							(FINAL_DOCKING * FINAL_DOCKING)

#define FINAL_APPROACH							(32.0 * g_KlicksPerPixel)
#define FINAL_APPROACH2							(FINAL_APPROACH * FINAL_APPROACH)

#define DOCKING_THRESHOLD						(4.0 * g_KlicksPerPixel)
#define DOCKING_THRESHOLD2						(DOCKING_THRESHOLD * DOCKING_THRESHOLD)

#define DOCKING_PORTS_TAG						CONSTLIT("DockingPorts")

#define BRING_TO_FRONT_ATTRIB					CONSTLIT("bringToFront")
#define DOCKING_PORTS_ATTRIB					CONSTLIT("dockingPorts")
#define MAX_DIST_ATTRIB							CONSTLIT("maxDist")
#define PORT_COUNT_ATTRIB						CONSTLIT("portCount")
#define PORT_RADIUS_ATTRIB						CONSTLIT("portRadius")
#define ROTATION_ATTRIB							CONSTLIT("rotation")
#define SEND_TO_BACK_ATTRIB						CONSTLIT("sendToBack")
#define X_ATTRIB								CONSTLIT("x")
#define Y_ATTRIB								CONSTLIT("y")

#define MIN_PORT_DISTANCE						(8.0 * g_KlicksPerPixel)
#define MIN_PORT_DISTANCE2						(MIN_PORT_DISTANCE * MIN_PORT_DISTANCE)

const int DEFAULT_PORT_POS_RADIUS =				64;
const int DEFAULT_DOCK_DISTANCE_LS =			12;
const Metric GATE_DIST =						KLICKS_PER_PIXEL * 64.0;
const Metric GATE_DIST2 =						GATE_DIST * GATE_DIST;

CDockingPorts::CDockingPorts (void) : 
		m_iPortCount(0),
		m_pPort(NULL),
		m_iMaxDist(DEFAULT_DOCK_DISTANCE_LS)

//	CDockingPorts constructor

	{
	}

CDockingPorts::~CDockingPorts (void)

//	CDockingPorts destructor

	{
	if (m_pPort)
		delete [] m_pPort;
	}

void CDockingPorts::DockAtRandomPort (CSpaceObject *pOwner, CSpaceObject *pObj)

//	DockAtRandomPort
//
//	Dock the ship at a random port. This is used during setup

	{
	int i;
	int iStart = mathRandom(0, m_iPortCount - 1);
	int iDockingPort = -1;

	for (i = 0; i < m_iPortCount; i++)
		{
		int iPort = (iStart + i) % m_iPortCount;
		if (m_pPort[iPort].iStatus == psEmpty)
			{
			iDockingPort = iPort;
			break;
			}
		}

	//	Set ship at the docking port

	if (iDockingPort != -1)
		{
		m_pPort[iDockingPort].pObj = pObj;
		m_pPort[iDockingPort].iStatus = psInUse;
		pObj->Place(GetPortPos(pOwner, m_pPort[iDockingPort], pObj));
		pObj->OnDocked(pOwner);

		//	Set the ship's rotation. We do this because this is only called
		//	during set up

		CShip *pShip = pObj->AsShip();
		if (pShip)
			pShip->SetRotation(m_pPort[iDockingPort].iRotation);
		}
	else
		{
		kernelDebugLogMessage("Warning: Unable to find docking port at %s", pOwner->GetName());
		}
	}

int CDockingPorts::FindNearestEmptyPort (CSpaceObject *pOwner, CSpaceObject *pRequestingObj, CVector *retvDistance, int *retiEmptyPortCount)

//	FindNearestEmptyPort
//
//	FindNearestEmptyPort 

	{
	int i;

	int iEmptyPortCount = 0;

	int iBestPort = -1;
	Metric rBestDist2 = g_InfiniteDistance;
	CVector vBestDistance;

	int iBestClearPort = -1;
	Metric rBestClearDist2 = g_InfiniteDistance;
	CVector vBestClearDistance;

	CVector vPos = pRequestingObj->GetPos();

	//	See if we can find a clear port

	for (i = 0; i < m_iPortCount; i++)
		if (m_pPort[i].iStatus == psEmpty)
			{
			CVector vPortPos = GetPortPos(pOwner, m_pPort[i], pRequestingObj);
			CVector vDistance = vPos - vPortPos;
			Metric rDistance2 = vDistance.Length2();

			//	Keep track of both the nearest clear port and the nearest
			//	port that is not clear.

			if (rDistance2 < rBestClearDist2 
					&& !ShipsNearPort(pOwner, pRequestingObj, vPortPos))
				{
				rBestClearDist2 = rDistance2;
				vBestClearDistance = vDistance;
				iBestClearPort = i;
				}
			else if (rDistance2 < rBestDist2)
				{
				rBestDist2 = rDistance2;
				vBestDistance = vDistance;
				iBestPort = i;
				}

			iEmptyPortCount++;
			}

	//	Return total number of empty ports

	if (retiEmptyPortCount)
		*retiEmptyPortCount = iEmptyPortCount;

	//	If we've got a clear port, then return that

	if (iBestClearPort != -1)
		{
		if (retvDistance)
			*retvDistance = vBestClearDistance;

		return iBestClearPort;
		}

	//	Otherwise, return the nearest non-clear port

	else
		{
		if (retvDistance)
			*retvDistance = vBestDistance;

		return iBestPort;
		}
	}

bool CDockingPorts::DoesPortPaintInFront (CSpaceObject *pOwner, int iPort) const

//	DoesPortPaintInFront
//
//	Retuns TRUE if we paint this port in front of the owning object.

	{
	const SDockingPort &Port = m_pPort[iPort];

	if (pOwner == NULL)
		return (Port.iLayer != plSendToBack);

	else if (pOwner->GetRotation() == 0)
		{
		switch (Port.iLayer)
			{
			case plBringToFront:
				return true;

			case plSendToBack:
				return false;

			default:
				return (Port.vPos.GetY() < 0.0);
			}
		}
	else
		{
		CVector vPortPos = Port.vPos.Rotate(pOwner->GetRotation());
		return (vPortPos.GetY() < 0.0);
		}
	}

CVector CDockingPorts::GetPortPos (CSpaceObject *pOwner, const SDockingPort &Port, CSpaceObject *pShip, bool *retbPaintInFront, int *retiRotation) const

//	GetPortPos
//
//	Get the absolute position of the port (or relative, if pOwner == NULL)

	{
	if (pOwner == NULL)
		{
		if (retbPaintInFront)
			*retbPaintInFront = (Port.iLayer != plSendToBack);

		if (retiRotation)
			*retiRotation = Port.iRotation;

		return Port.vPos;
		}
	else if (pOwner->GetRotation() == 0)
		{
		const CVector &vOwnerPos = pOwner->GetPos();
		if (retbPaintInFront)
			{
			switch (Port.iLayer)
				{
				case plBringToFront:
					*retbPaintInFront = true;
					break;

				case plSendToBack:
					*retbPaintInFront = false;
					break;

				default:
					*retbPaintInFront = (Port.vPos.GetY() < 0.0);
				}
			}

		if (retiRotation)
			*retiRotation = Port.iRotation;

		return (vOwnerPos + Port.vPos + (pShip ? pShip->GetDockingPortOffset(Port.iRotation) : NullVector));
		}
	else
		{
		const CVector &vOwnerPos = pOwner->GetPos();
		CVector vPortPos = Port.vPos.Rotate(pOwner->GetRotation());
		int iNewRotation = AngleMod(Port.iRotation + pOwner->GetRotation());

		if (retbPaintInFront)
			*retbPaintInFront = (vPortPos.GetY() < 0.0);

		if (retiRotation)
			*retiRotation = iNewRotation;

		return (vOwnerPos + vPortPos + (pShip ? pShip->GetDockingPortOffset(iNewRotation) : NullVector));
		}
	}

int CDockingPorts::GetPortsInUseCount (CSpaceObject *pOwner)

//	GetPortsInUseCount
//
//	GetPortsInUseCount 

	{
	int iCount = 0;

	for (int i = 0; i < m_iPortCount; i++)
		if (m_pPort[i].iStatus != psEmpty)
			iCount++;

	return iCount;
	}

void CDockingPorts::InitPorts (CSpaceObject *pOwner, int iCount, Metric rRadius)

//	InitPorts
//
//	InitPorts 

	{
	ASSERT(m_pPort == NULL);

	if (iCount > 0)
		{
		m_iPortCount = iCount;
		m_pPort = new SDockingPort[iCount];

		int iAngle = 360 / iCount;
		for (int i = 0; i < iCount; i++)
			{
			m_pPort[i].vPos = PolarToVector(i * iAngle, rRadius);
			m_pPort[i].iRotation = ((i * iAngle) + 180) % 360;
			}
		}

	m_iMaxDist = DEFAULT_DOCK_DISTANCE_LS;
	}

void CDockingPorts::InitPorts (CSpaceObject *pOwner, const TArray<CVector> &Desc)

//	InitPorts
//
//	Initialize from array

	{
	ASSERT(m_pPort == NULL);

	if (Desc.GetCount() > 0)
		{
		m_iPortCount = Desc.GetCount();
		m_pPort = new SDockingPort[m_iPortCount];

		for (int i = 0; i < m_iPortCount; i++)
			{
			m_pPort[i].vPos = Desc[i];
			m_pPort[i].iRotation = (VectorToPolar(Desc[i]) + 180) % 360;
			}
		}

	m_iMaxDist = DEFAULT_DOCK_DISTANCE_LS;
	}

void CDockingPorts::InitPorts (CSpaceObject *pOwner, int iCount, CVector *pPos)

//	InitPorts
//
//	Initialize from count and array

	{
	ASSERT(m_pPort == NULL);

	if (iCount > 0)
		{
		m_iPortCount = iCount;
		m_pPort = new SDockingPort[iCount];

		for (int i = 0; i < iCount; i++)
			{
			m_pPort[i].vPos = pPos[i];
			m_pPort[i].iRotation = (VectorToPolar(pPos[i]) + 180) % 360;
			}
		}

	m_iMaxDist = DEFAULT_DOCK_DISTANCE_LS;
	}

void CDockingPorts::InitPortsFromXML (CSpaceObject *pOwner, CXMLElement *pElement)

//	InitPortsFromXML
//
//	InitPortsFromXML 

	{
	int i;

	//	See if we've got a special element with docking port geometry

	CXMLElement *pDockingPorts = pElement->GetContentElementByTag(DOCKING_PORTS_TAG);
	if (pDockingPorts)
		{
		//	Initialize max dist
		//	NOTE: pOwner can be NULL because sometimes we init ports from a ship class 
		//	(without an object).

		int iDefaultDist = Max(DEFAULT_DOCK_DISTANCE_LS, (pOwner ? 8 + (int)((pOwner->GetBoundsRadius() / LIGHT_SECOND) + 0.5) : 0));
		m_iMaxDist = pDockingPorts->GetAttributeIntegerBounded(MAX_DIST_ATTRIB, 1, -1, iDefaultDist);


		//	If we have sub-elements then these are port definitions.

		m_iPortCount = pDockingPorts->GetContentElementCount();
		if (m_iPortCount > 0)
			{
			m_pPort = new SDockingPort[m_iPortCount];

			for (i = 0; i < m_iPortCount; i++)
				{
				CXMLElement *pPort = pDockingPorts->GetContentElement(i);
				CVector vDockPos((pPort->GetAttributeInteger(X_ATTRIB) * g_KlicksPerPixel),
						(pPort->GetAttributeInteger(Y_ATTRIB) * g_KlicksPerPixel));

				m_pPort[i].vPos = vDockPos;

				if (pPort->FindAttributeInteger(ROTATION_ATTRIB, &m_pPort[i].iRotation))
					m_pPort[i].iRotation = (m_pPort[i].iRotation % 360);
				else
					m_pPort[i].iRotation = (VectorToPolar(vDockPos) + 180) % 360;

				if (pPort->GetAttributeBool(BRING_TO_FRONT_ATTRIB))
					m_pPort[i].iLayer = plBringToFront;
				else if (pPort->GetAttributeBool(SEND_TO_BACK_ATTRIB))
					m_pPort[i].iLayer = plSendToBack;
				}
			}

		//	Otherwise, we expect a port count and radius

		else if ((m_iPortCount = pDockingPorts->GetAttributeIntegerBounded(PORT_COUNT_ATTRIB, 0, -1, 0)) > 0)
			{
			int iRadius = pDockingPorts->GetAttributeIntegerBounded(PORT_RADIUS_ATTRIB, 0, -1, DEFAULT_PORT_POS_RADIUS);
			int iAngle = 360 / m_iPortCount;

			//	We need the image scale to adjust coordinates

			int iScale = (pOwner ? pOwner->GetImage().GetImageViewportSize() : 512);

			//	Initialize ports

			m_pPort = new SDockingPort[m_iPortCount];
			for (i = 0; i < m_iPortCount; i++)
				{
				C3DConversion::CalcCoord(iScale, i * iAngle, iRadius, 0, &m_pPort[i].vPos);
				m_pPort[i].iRotation = ((i * iAngle) + 180) % 360;
				}
			}

		//	Otherwise, no ports

		else
			{
			m_iPortCount = 0;
			m_pPort = NULL;
			}
		}

	//	Otherwise, initialize ports based on a count

	else
		InitPorts(pOwner,
				pElement->GetAttributeInteger(DOCKING_PORTS_ATTRIB),
				64 * g_KlicksPerPixel);
	}

bool CDockingPorts::IsDocked (CSpaceObject *pObj)

//	IsDocked
//
//	Returns TRUE if the given object is docked here

	{
	for (int i = 0; i < m_iPortCount; i++)
		if (m_pPort[i].iStatus == psInUse)
			if (pObj == m_pPort[i].pObj)
				return true;

	return false;
	}

bool CDockingPorts::IsDockedOrDocking (CSpaceObject *pObj)

//	IsDockedOrDocking
//
//	Returns TRUE if the given object is docked (or docking) here

	{
	for (int i = 0; i < m_iPortCount; i++)
		if (m_pPort[i].iStatus != psEmpty)
			if (pObj == m_pPort[i].pObj)
				return true;

	return false;
	}

void CDockingPorts::MoveAll (CSpaceObject *pOwner)

//	MoveAll
//
//	Move all docked objects to follow the owner

	{
	CVector vVel = pOwner->GetVel();

	for (int i = 0; i < m_iPortCount; i++)
		if (m_pPort[i].iStatus == psInUse)
			{
			m_pPort[i].pObj->SetPos(GetPortPos(pOwner, m_pPort[i], m_pPort[i].pObj));
			m_pPort[i].pObj->SetVel(vVel);
			}
	}

void CDockingPorts::OnDestroyed (void)

//	OnDestroyed
//
//	If we've been destroyed, then release all objects

	{
	DEBUG_TRY

	for (int i = 0; i < m_iPortCount; i++)
		if (m_pPort[i].iStatus == psDocking)
			m_pPort[i].pObj->UnfreezeControls();

	DEBUG_CATCH
	}

void CDockingPorts::OnObjDestroyed (CSpaceObject *pOwner, CSpaceObject *pObj, bool *retbDestroyed)

//	OnObjDestroyed
//
//	OnObjDestroyed 

	{
	bool bDestroyed = false;

	for (int i = 0; i < m_iPortCount; i++)
		if (m_pPort[i].pObj == pObj)
			{
			m_pPort[i].pObj = NULL;
			m_pPort[i].iStatus = psEmpty;
			bDestroyed = true;
			}

	if (retbDestroyed)
		*retbDestroyed = bDestroyed;
	}

void CDockingPorts::ReadFromStream (CSpaceObject *pOwner, SLoadCtx &Ctx)

//	ReadFromStream
//
//	ReadFromStream 

	{
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&m_iPortCount, sizeof(DWORD));
	if (m_iPortCount > 0)
		{
		m_pPort = new SDockingPort[m_iPortCount];
		for (int i = 0; i < m_iPortCount; i++)
			{
			Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
			m_pPort[i].iStatus = (DockingPortStatus)LOWORD(dwLoad);
			m_pPort[i].iLayer = (DockingPortLayer)HIWORD(dwLoad);

			CSystem::ReadObjRefFromStream(Ctx, &m_pPort[i].pObj);
			Ctx.pStream->Read((char *)&m_pPort[i].vPos, sizeof(CVector));
			if (Ctx.dwVersion >= 24)
				Ctx.pStream->Read((char *)&m_pPort[i].iRotation, sizeof(DWORD));

			//	In previous versions we did not set rotation, so set it now

			if (m_pPort[i].iRotation == -1)
				m_pPort[i].iRotation = (VectorToPolar(m_pPort[i].vPos) + 180) % 360;
			}
		}

	if (Ctx.dwVersion >= 81)
		Ctx.pStream->Read((char *)&m_iMaxDist, sizeof(DWORD));
	else
		m_iMaxDist = DEFAULT_DOCK_DISTANCE_LS;
	}

void CDockingPorts::RepairAll (CSpaceObject *pOwner, int iRepairRate)

//	RepairAll
//
//	RepairAll 

	{
	DEBUG_TRY

	if (iRepairRate > 0)
		{
		for (int i = 0; i < m_iPortCount; i++)
			if (m_pPort[i].iStatus == psInUse
					&& !m_pPort[i].pObj->IsPlayer()
					&& !pOwner->IsEnemy(m_pPort[i].pObj))
				{
				m_pPort[i].pObj->RepairDamage(iRepairRate);
				m_pPort[i].pObj->Decontaminate();
				}
		}

	DEBUG_CATCH
	}

bool CDockingPorts::RequestDock (CSpaceObject *pOwner, CSpaceObject *pObj, int iPort)

//	RequestDock
//
//	RequestDock 

	{
	//	If the requested dock is full, then we fail.

	if (iPort != -1 && m_pPort[iPort].iStatus != psEmpty)
		{
		pObj->SendMessage(pOwner, CONSTLIT("Docking port no longer available"));
		return false;
		}

	//	Get the nearest free port

	int iEmptyPortsLeft;
	if (iPort == -1)
		iPort = FindNearestEmptyPort(pOwner, pObj, NULL, &iEmptyPortsLeft);
	else
		//	If the caller specifies a port then it must be the player, so we
		//	don't worry about leaving an empty port.
		iEmptyPortsLeft = 2;

	//	If we could not find a free port then deny docking service

	if (iPort == -1)
		{
		pObj->SendMessage(pOwner, CONSTLIT("No docking ports available"));
		return false;
		}

	//	If the requester is not the player and there is only one port left, then
	//	fail (we always reserve one port for the player).
	//
	//	[We also make an exception for any ship that the player is escorting.]

	CSpaceObject *pPlayer = g_pUniverse->GetPlayer();
	if (iEmptyPortsLeft < 2
			&& pPlayer
			&& pObj != pPlayer
			&& pObj != pPlayer->GetDestination())
		{
		pObj->SendMessage(pOwner, CONSTLIT("No docking ports available"));
		return false;
		}

	//	Commence docking 

	pOwner->Communicate(pObj, msgDockingSequenceEngaged);
	pObj->FreezeControls();

	m_pPort[iPort].iStatus = psDocking;
	m_pPort[iPort].pObj = pObj;

	//	Done

	return true;
	}

bool CDockingPorts::ShipsNearPort (CSpaceObject *pOwner, CSpaceObject *pRequestingObj, const CVector &vPortPos)

//	ShipsNearPort
//
//	Returns TRUE if there are ships near the given port

	{
	int i;
	CSystem *pSystem = pOwner->GetSystem();

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);
		if (pObj
				&& pObj->GetCategory() == CSpaceObject::catShip
				&& !pObj->IsInactive()
				&& !pObj->IsVirtual()
				&& pObj != pRequestingObj)
			{
			Metric rDist2 = (pObj->GetPos() - vPortPos).Length2();
			if (rDist2 < MIN_PORT_DISTANCE2 && !IsDockedOrDocking(pObj))
				return true;
			}
		}

	return false;
	}

void CDockingPorts::Undock (CSpaceObject *pOwner, CSpaceObject *pObj)

//	Undock
//
//	Undock the object

	{
	int i;

	for (i = 0; i < m_iPortCount; i++)
		if (m_pPort[i].pObj == pObj)
			{
			//	Unfreeze controls if we're trying to dock
			if (m_pPort[i].iStatus == psDocking)
				pObj->UnfreezeControls();

			m_pPort[i].iStatus = psEmpty;
			m_pPort[i].pObj = NULL;
			}
	}

void CDockingPorts::UpdateAll (SUpdateCtx &Ctx, CSpaceObject *pOwner)

//	UpdateAll
//
//	UpdateAll 

	{
	DEBUG_TRY

	int i;

	CSpaceObject *pPlayer = Ctx.pPlayer;
	Metric rDist2 = (pPlayer ? pPlayer->GetDistance2(pOwner) : 0.0);
	Metric rMaxDist = m_iMaxDist * LIGHT_SECOND;
	Metric rMaxDist2 = rMaxDist * rMaxDist;

	//	If owner is destroyed then don't bother checking for nearest player
	//	port.

	if (pPlayer 
			&& (pOwner->IsDestroyed() 
				|| pOwner->IsInactive() 
				|| pOwner == pPlayer
				|| pPlayer->IsDestroyed()))
		pPlayer = NULL;

	//	Also, don't bother checking if the owner is an enemy of the player.

	if (pPlayer && pPlayer->IsEnemy(pOwner) && !pOwner->IsAbandoned())
		pPlayer = NULL;

	//	Don't bother checking if the station is too far

	if (pPlayer && rDist2 > rMaxDist2)
		pPlayer = NULL;

	//	If this is a stargate and we are at the center (just came through) 
	//	then don't bother showing docking ports.

	if (pPlayer && pOwner->IsStargate() && rDist2 < GATE_DIST2)
		pPlayer = NULL;

	//	Don't bother if the docking is disabled (for the player)

	if (pPlayer && !pOwner->SupportsDocking(true))
		pPlayer = NULL;

	//	Loop over all ports

	for (i = 0; i < m_iPortCount; i++)
		{
		//	If a ship is docking with this port, then maneuver the ship towards
		//	the docking port.

		if (m_pPort[i].iStatus == psDocking)
			UpdateDockingManeuvers(pOwner, m_pPort[i]);

		//	Otherwise, if the port is open, see if this is the nearest port to
		//	the current player position.

		else if (m_pPort[i].iStatus == psEmpty)
			{
			if (pPlayer)
				{
				//	Compute the distance from the player to the port

				CVector vPortPos = GetPortPos(pOwner, m_pPort[i], pPlayer);
				Metric rDist2 = (vPortPos - pPlayer->GetPos()).Length2();

				//	If this is a better port, then replace the existing 
				//	solution.

				if (Ctx.pDockingObj == NULL
							|| rDist2 < Ctx.rDockingPortDist2)
					{
					Ctx.pDockingObj = pOwner;
					Ctx.iDockingPort = i;
					Ctx.rDockingPortDist2 = rDist2;
					Ctx.vDockingPort = vPortPos;
					}
				}
			}
		}

	DEBUG_CATCH
	}

void CDockingPorts::UpdateDockingManeuvers (CSpaceObject *pOwner, SDockingPort &Port)

//	UpdateDockingManeuvers
//
//	Updates the motion of a ship docking with a port

	{
	DEBUG_TRY

	CShip *pShip = Port.pObj->AsShip();

	ASSERT(pShip);
	if (pShip == NULL)
		return;

	CVector vDest = GetPortPos(pOwner, Port, pShip);
	CVector vDestVel = pOwner->GetVel();

	//	Figure out how far we are from where we want to be

	CVector vDelta = vDest - pShip->GetPos();

	//	If the docking object is within the appropriate threshold 
	//	of the port, then complete the docking sequence.

	Metric rDelta2 = vDelta.Length2();
	if (rDelta2 < DOCKING_THRESHOLD2 
			&& (pShip == g_pUniverse->GetPlayer() || pShip->IsPointingTo(Port.iRotation)))
		{
		pShip->Place(vDest);
		pShip->UnfreezeControls();
		IShipController *pController = pShip->GetController();
		pController->SetManeuver(NoRotation);

		Port.iStatus = psInUse;

		//	Tell the owner that somone has docked with it first
		//	(We do this because sometimes we want to handle stuff
		//	in OnObjDocked before we show the player a dock screen)

		if (pOwner 
				&& pOwner->HasOnObjDockedEvent() 
				&& pOwner != pShip
				&& !pOwner->IsDestroyed()
				&& pShip->IsSubscribedToEvents(pOwner))
			pOwner->FireOnObjDocked(pShip, pOwner);

		//	Set the owner as "explored" (if the ship is the player)

		if (pShip->IsPlayer())
			{
			pOwner->SetExplored();

			if (pOwner->IsAutoClearDestinationOnDock())
				pOwner->ClearPlayerDestination();
			}

		//	Dock

		pShip->OnDocked(pOwner);

		//	Notify other subscribers

		pShip->NotifyOnObjDocked(pOwner);
		}

	//	Otherwise accelerate the ship towards the docking port

	else
		{
		Metric rMaxSpeed = pShip->GetMaxSpeed();
		Metric rMinSpeed = rMaxSpeed / 10.0;

		//	We slow down as we get closer

		Metric rSpeed;
		if (rDelta2 < FINAL_DOCKING2)
			rSpeed = rMinSpeed;
		else if (rDelta2 < FINAL_APPROACH2)
			{
			Metric rSpeedRange = rMaxSpeed - rMinSpeed;
			Metric rDelta = sqrt(rDelta2);
			rSpeed = rMinSpeed + (rSpeedRange * (rDelta - FINAL_DOCKING) / (FINAL_APPROACH - FINAL_DOCKING));
			}
		else
			rSpeed = rMaxSpeed;

		//	Figure out the ideal velocity vector that we want to
		//	be following.

		CVector vIdealVel = vDelta.Normal() * rSpeed;

		//	Calculate the delta v that we need

		CVector vDeltaV = vIdealVel - pShip->GetVel();

		//	Rotate

		if (!pShip->IsPlayer())
			{
			IShipController *pController = pShip->GetController();

			//	If we're close enough, align to rotation angle

			if (rDelta2 < FINAL_APPROACH2)
				pController->SetManeuver(pShip->GetManeuverToFace(Port.iRotation));

			//	Otherwise, align along delta v

			else
				pController->SetManeuver(pShip->GetManeuverToFace(VectorToPolar(vDeltaV)));

			//	Don't let the AI thrust

			pController->SetThrust(false);
			}

		//	Accelerate

		pShip->Accelerate(vDeltaV * pShip->GetMass() / 10000.0, g_SecondsPerUpdate);
		pShip->ClipSpeed(rSpeed);
		}

	DEBUG_CATCH
	}

void CDockingPorts::WriteToStream (CSpaceObject *pOwner, IWriteStream *pStream)

//	WriteToStream
//
//	WriteToStream 

	{
	DWORD dwSave;

	pStream->Write((char *)&m_iPortCount, sizeof(DWORD));
	for (int i = 0; i < m_iPortCount; i++)
		{
		dwSave = MAKELONG((WORD)m_pPort[i].iStatus, (WORD)m_pPort[i].iLayer);
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		pOwner->WriteObjRefToStream(m_pPort[i].pObj, pStream);
		pStream->Write((char *)&m_pPort[i].vPos, sizeof(CVector));
		pStream->Write((char *)&m_pPort[i].iRotation, sizeof(DWORD));
		}

	pStream->Write((char *)&m_iMaxDist, sizeof(DWORD));
	}

