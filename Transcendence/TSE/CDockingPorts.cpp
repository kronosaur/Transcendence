//	CDockingPorts.cpp
//
//	CDockingPorts class

#include "PreComp.h"

#define FINAL_DOCKING					(8.0 * g_KlicksPerPixel)
#define FINAL_DOCKING2					(FINAL_DOCKING * FINAL_DOCKING)

#define FINAL_APPROACH					(32.0 * g_KlicksPerPixel)
#define FINAL_APPROACH2					(FINAL_APPROACH * FINAL_APPROACH)

#define DOCKING_THRESHOLD				(4.0 * g_KlicksPerPixel)
#define DOCKING_THRESHOLD2				(DOCKING_THRESHOLD * DOCKING_THRESHOLD)

#define DOCKING_PORTS_TAG				CONSTLIT("DockingPorts")
#define DOCKING_PORTS_ATTRIB			CONSTLIT("dockingPorts")
#define X_ATTRIB						CONSTLIT("x")
#define Y_ATTRIB						CONSTLIT("y")
#define ROTATION_ATTRIB					CONSTLIT("rotation")

#define MIN_PORT_DISTANCE				(8.0 * g_KlicksPerPixel)
#define MIN_PORT_DISTANCE2				(MIN_PORT_DISTANCE * MIN_PORT_DISTANCE)

CDockingPorts::CDockingPorts (void) : 
		m_iPortCount(0),
		m_pPort(NULL)

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
		pObj->Place(pOwner->GetPos() + m_pPort[iDockingPort].vPos);
		pObj->OnDocked(pOwner);

		//	Set the ship's rotation. We do this because this is only called
		//	during set up

		CShip *pShip = pObj->AsShip();
		if (pShip)
			pShip->SetRotation(m_pPort[iDockingPort].iRotation);
		}
	else
		{
		kernelDebugLogMessage("Warning: Unable to find docking port at %s", pOwner->GetName().GetASCIIZPointer());
		}
	}

int CDockingPorts::FindNearestEmptyPort (CSpaceObject *pOwner, CSpaceObject *pRequestingObj, CVector *retvDistance)

//	FindNearestEmptyPort
//
//	FindNearestEmptyPort 

	{
	int i;

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
			CVector vPortPos = pOwner->GetPos() + m_pPort[i].vPos;
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
			}

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

CVector CDockingPorts::GetPortPos (CSpaceObject *pOwner, int iPort)

//	GetPortPos
//
//	Get position of port

	{
	if (pOwner)
		return pOwner->GetPos() + m_pPort[iPort].vPos;
	else
		return m_pPort[iPort].vPos;
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
		m_pPort = new DockingPort[iCount];

		int iAngle = 360 / iCount;
		for (int i = 0; i < iCount; i++)
			{
			m_pPort[i].iStatus = psEmpty;
			m_pPort[i].pObj = NULL;
			m_pPort[i].vPos = PolarToVector(i * iAngle, rRadius);
			m_pPort[i].iRotation = ((i * iAngle) + 180) % 360;
			}
		}
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
		m_pPort = new DockingPort[iCount];

		for (int i = 0; i < iCount; i++)
			{
			m_pPort[i].iStatus = psEmpty;
			m_pPort[i].pObj = NULL;
			m_pPort[i].vPos = pPos[i];
			m_pPort[i].iRotation = (VectorToPolar(pPos[i]) + 180) % 360;
			}
		}
	}

void CDockingPorts::InitPortsFromXML (CSpaceObject *pOwner, CXMLElement *pElement)

//	InitPortsFromXML
//
//	InitPortsFromXML 

	{
	//	See if we've got a special element with docking port geometry

	CXMLElement *pDockingPorts = pElement->GetContentElementByTag(DOCKING_PORTS_TAG);
	if (pDockingPorts)
		{
		m_iPortCount = pDockingPorts->GetContentElementCount();
		if (m_iPortCount > 0)
			{
			m_pPort = new DockingPort[m_iPortCount];

			for (int i = 0; i < m_iPortCount; i++)
				{
				CXMLElement *pPort = pDockingPorts->GetContentElement(i);
				CVector vDockPos((pPort->GetAttributeInteger(X_ATTRIB) * g_KlicksPerPixel),
						(pPort->GetAttributeInteger(Y_ATTRIB) * g_KlicksPerPixel));

				m_pPort[i].iStatus = psEmpty;
				m_pPort[i].pObj = NULL;
				m_pPort[i].vPos = vDockPos;

				if (pPort->FindAttributeInteger(ROTATION_ATTRIB, &m_pPort[i].iRotation))
					m_pPort[i].iRotation = (m_pPort[i].iRotation % 360);
				else
					m_pPort[i].iRotation = (VectorToPolar(vDockPos) + 180) % 360;
				}
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
	CVector vPos = pOwner->GetPos();
	CVector vVel = pOwner->GetVel();

	for (int i = 0; i < m_iPortCount; i++)
		if (m_pPort[i].iStatus == psInUse)
			{
			m_pPort[i].pObj->SetPos(vPos + m_pPort[i].vPos);
			m_pPort[i].pObj->SetVel(vVel);
			}
	}

void CDockingPorts::OnDestroyed (void)

//	OnDestroyed
//
//	If we've been destroyed, then release all objects

	{
	for (int i = 0; i < m_iPortCount; i++)
		if (m_pPort[i].iStatus == psDocking)
			m_pPort[i].pObj->UnfreezeControls();
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
	Ctx.pStream->Read((char *)&m_iPortCount, sizeof(DWORD));
	if (m_iPortCount > 0)
		{
		m_pPort = new DockingPort[m_iPortCount];
		for (int i = 0; i < m_iPortCount; i++)
			{
			Ctx.pStream->Read((char *)&m_pPort[i].iStatus, sizeof(DWORD));
			Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pPort[i].pObj);
			Ctx.pStream->Read((char *)&m_pPort[i].vPos, sizeof(CVector));
			if (Ctx.dwVersion >= 24)
				Ctx.pStream->Read((char *)&m_pPort[i].iRotation, sizeof(DWORD));

			//	In previous versions we did not set rotation, so set it now

			if (m_pPort[i].iRotation == -1)
				m_pPort[i].iRotation = (VectorToPolar(m_pPort[i].vPos) + 180) % 360;
			}
		}
	}

void CDockingPorts::RepairAll (CSpaceObject *pOwner, int iRepairRate)

//	RepairAll
//
//	RepairAll 

	{
	if (iRepairRate > 0)
		{
		for (int i = 0; i < m_iPortCount; i++)
			if (m_pPort[i].iStatus == psInUse
					&& m_pPort[i].pObj->GetSovereign() == pOwner->GetSovereign())
				{
				m_pPort[i].pObj->RepairDamage(iRepairRate);
				m_pPort[i].pObj->Decontaminate();
				}
		}
	}

bool CDockingPorts::RequestDock (CSpaceObject *pOwner, CSpaceObject *pObj)

//	RequestDock
//
//	RequestDock 

	{
	//	Get the nearest free port

	int iPort = FindNearestEmptyPort(pOwner, pObj);

	//	If we could not find a free port then deny docking service

	if (iPort == -1)
		{
		pObj->SendMessage(pOwner, CONSTLIT("No docking ports available"));
		return false;
		}

	//	Commence docking

	pObj->SendMessage(pOwner, CONSTLIT("Docking sequence engaged"));
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

void CDockingPorts::UpdateAll (CSpaceObject *pOwner)

//	UpdateAll
//
//	UpdateAll 

	{
	int i, j;

	for (i = 0; i < m_iPortCount; i++)
		{
		if (m_pPort[i].iStatus == psDocking)
			{
			CShip *pShip = m_pPort[i].pObj->AsShip();

			ASSERT(pShip);
			if (pShip == NULL)
				continue;

			CVector vDest = pOwner->GetPos() + m_pPort[i].vPos;
			CVector vDestVel = pOwner->GetVel();

			//	Figure out how far we are from where we want to be

			CVector vDelta = vDest - pShip->GetPos();

			//	Figure out if we're aligned

			int iFinalRotation = pShip->AlignToRotationAngle(m_pPort[i].iRotation);

			//	If the docking object is within the appropriate threshold 
			//	of the port, then complete the docking sequence.

			Metric rDelta2 = vDelta.Length2();
			if (rDelta2 < DOCKING_THRESHOLD2 
					&& (pShip == g_pUniverse->GetPlayer() || iFinalRotation == pShip->GetRotation()))
				{
				pShip->Place(vDest);
				pShip->UnfreezeControls();
				IShipController *pController = pShip->GetController();
				pController->SetManeuver(IShipController::NoRotation);

				m_pPort[i].iStatus = psInUse;

				//	Tell the owner that somone has docked with it first
				//	(We do this because sometimes we want to handle stuff
				//	in OnObjDocked before we show the player a dock screen)

				if (pOwner && pOwner->HasOnObjDockedEvent() && pOwner != pShip)
					pOwner->OnObjDocked(pShip, pOwner);

				//	Dock

				pShip->OnDocked(pOwner);

				//	Tell all objects in the system that a ship has docked

				CSystem *pSystem = pShip->GetSystem();
				for (j = 0; j < pSystem->GetObjectCount(); j++)
					{
					CSpaceObject *pObj = pSystem->GetObject(j);

					if (pObj && pObj->HasOnObjDockedEvent() && pObj != pShip && pObj != pOwner)
						pObj->OnObjDocked(pShip, pOwner);
					}
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
						pController->SetManeuver(CalcTurnManeuver(iFinalRotation, pShip->GetRotation(), pShip->GetRotationAngle()));

					//	Otherwise, align along delta v

					else
						pController->SetManeuver(CalcTurnManeuver(VectorToPolar(vDeltaV), pShip->GetRotation(), pShip->GetRotationAngle()));

					//	Don't let the AI thrust

					pController->SetThrust(false);
					}

				//	Accelerate

				pShip->Accelerate(vDeltaV * pShip->GetMass() / 10000.0, g_SecondsPerUpdate);
				pShip->ClipSpeed(rSpeed);
				}
			}
		}
	}

void CDockingPorts::WriteToStream (CSpaceObject *pOwner, IWriteStream *pStream)

//	WriteToStream
//
//	WriteToStream 

	{
	pStream->Write((char *)&m_iPortCount, sizeof(DWORD));
	for (int i = 0; i < m_iPortCount; i++)
		{
		pStream->Write((char *)&m_pPort[i].iStatus, sizeof(DWORD));
		pOwner->WriteObjRefToStream(m_pPort[i].pObj, pStream);
		pStream->Write((char *)&m_pPort[i].vPos, sizeof(CVector));
		pStream->Write((char *)&m_pPort[i].iRotation, sizeof(DWORD));
		}
	}

