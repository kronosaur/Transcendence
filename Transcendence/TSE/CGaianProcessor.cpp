//	CGaianProcessorAI.cpp
//
//	CGaianProcessorAI class

#include "PreComp.h"


#define SCAVENGE_SENSOR_RANGE			(LIGHT_MINUTE * 4)
#define MAX_EATING_DISTANCE				(LIGHT_SECOND * 16.0)
#define MAX_TIME_DESTROYING_WRECK		(g_TicksPerSecond * 8)

#define SPACE_OBJ_TAGGED_DATA			CONSTLIT("gaianprocessor_tagged")

#define MAX_CHARGES						200
#define CHARGE_INCREASE					8

static CObjectClass<CGaianProcessorAI>g_Class(OBJID_CGAIANPROCESSORAI, NULL);

CGaianProcessorAI::CGaianProcessorAI (void) : CBaseShipAI(&g_Class),
		m_State(stateNone),
		m_pTarget(NULL),
		m_pDest(NULL),
		m_iDestructorDev(-1),
		m_iDisablerDev(-1),
		m_dwAmmo(0)

//	CGaianProcessorAI constructor

	{
	}

void CGaianProcessorAI::Behavior (void)

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

		case stateLookingForWreck:
			{
			int i;

			//	Orbit around the center of the system

			CVector vTarget = -m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);

			Metric rMaxDist = m_rOrbitRadius * 1.1;
			Metric rMinDist = m_rOrbitRadius * 0.9;

			if (rTargetDist2 > (rMaxDist * rMaxDist))
				m_AICtx.ImplementSpiralIn(m_pShip, vTarget);
			else if (rTargetDist2 < (rMinDist * rMinDist))
				m_AICtx.ImplementSpiralOut(m_pShip, vTarget);

			//ImplementAttackNearestTarget(ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);

			//	Look for loot

			if (m_pShip->IsDestinyTime(40))
				{
				Metric rBestDist2 = SCAVENGE_SENSOR_RANGE * SCAVENGE_SENSOR_RANGE;
				CSpaceObject *pBestScrap = NULL;
				for (i = 0; i < m_pShip->GetSystem()->GetObjectCount(); i++)
					{
					CSpaceObject *pObj = m_pShip->GetSystem()->GetObject(i);

					if (pObj
							&& (pObj->GetScale() == scaleStructure || pObj->GetScale() == scaleShip)
							&& pObj->IsAbandoned()
							&& pObj->CanBeDestroyed()
							&& !pObj->IsInactive()
							&& !pObj->IsVirtual()
							&& pObj->GetData(SPACE_OBJ_TAGGED_DATA).IsBlank())
						{
						CVector vRange = pObj->GetPos() - m_pShip->GetPos();
						Metric rDistance2 = vRange.Dot(vRange);

						if (rDistance2 < rBestDist2)
							{
							rBestDist2 = rDistance2;
							pBestScrap = pObj;
							}
						}
					}

				//	If we've found some scrap then dock with it

				if (pBestScrap)
					{
					SetState(statePulverizingWreck);

					m_iStartTime = -1;
					m_pDest = pBestScrap;

					//	Tag the object so that no one else goes after it

					m_pDest->SetData(SPACE_OBJ_TAGGED_DATA, CONSTLIT("True"));
					}

				//	Otherwise, there is a small chance that we change orbits

				else if (mathRandom(1, 100) == 1)
					{
					m_rOrbitRadius = LIGHT_SECOND * 4 * (mathRandom(30, 120) + mathRandom(30, 120));
					}
				}

			break;
			}

		case statePulverizingWreck:
			{
			ASSERT(m_pDest);

			CVector vTarget = m_pDest->GetPos() - m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);

			if (rTargetDist2 > (MAX_EATING_DISTANCE * MAX_EATING_DISTANCE))
				m_AICtx.ImplementCloseOnTarget(m_pShip, m_pDest, vTarget, rTargetDist2);
			else
				{
				//	Remember when we first started firing at the wreck

				if (m_iStartTime == -1)
					m_iStartTime = g_pUniverse->GetTicks();
				
				//	If we've tried to destroy this wreck for a while and it still
				//	isn't destroyed, then we give up.

				else if ((g_pUniverse->GetTicks() - m_iStartTime) > MAX_TIME_DESTROYING_WRECK)
					{
					SetState(stateNone);
					break;
					}

				//	Fire

				m_AICtx.ImplementFireWeaponOnTarget(m_pShip,
						m_iDestructorDev,
						0,
						m_pDest,
						vTarget,
						rTargetDist2,
						NULL);
				}

			//ImplementAttackNearestTarget(ATTACK_RANGE, &m_pTarget);
			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
			break;
			}

		case stateConsumingWreck:
			{
			ASSERT(m_pDest);

			CVector vTarget = m_pDest->GetPos() - m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);

			if (rTargetDist2 > (MAX_EATING_DISTANCE * MAX_EATING_DISTANCE))
				m_AICtx.ImplementCloseOnTarget(m_pShip, m_pDest, vTarget, rTargetDist2);
			else
				m_AICtx.ImplementStop(m_pShip);

			m_AICtx.ImplementFireOnTargetsOfOpportunity(m_pShip);
			break;
			}
		}
	}

void CGaianProcessorAI::BehaviorStart (void)

//	BehaviorStart
//
//	Figure out what to do based on orders

	{
	CalcDevices();

	SetState(stateLookingForWreck);
	m_rOrbitRadius = LIGHT_SECOND * 4 * (mathRandom(30, 120) + mathRandom(30, 120));
	}

void CGaianProcessorAI::CalcDevices (void)

//	CalcDevices
//
//	Initializes:
//
//	m_iDestructorDev
//	m_iDisablerDev
//	m_dwAmmo

	{
	int i;

	if (m_iDestructorDev == -1)
		{
		//	Loop over all devices to find the weapons

		for (i = 0; i < m_pShip->GetDeviceCount(); i++)
			{
			CInstalledDevice *pWeapon = m_pShip->GetDevice(i);
			CItemCtx Ctx(m_pShip, pWeapon);

			if (pWeapon->IsEmpty())
				NULL;
			else if (pWeapon->IsSecondaryWeapon())
				{
				if (m_dwAmmo == 0)
					{
					CItemType *pAmmoType = NULL;
					pWeapon->GetSelectedVariantInfo(m_pShip, NULL, NULL, &pAmmoType);
					if (pAmmoType)
						m_dwAmmo = pAmmoType->GetUNID();
					}
				}
			else
				{
				const DamageDesc *pDamage = pWeapon->GetDamageDesc(Ctx);
				if (pDamage)
					{
					if (pDamage->GetMassDestructionAdj())
						m_iDestructorDev = i;
					else if (pDamage->GetEMPDamage())
						m_iDisablerDev = i;
					}
				}
			}
		}
	}

CString CGaianProcessorAI::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Returns debug crash info

	{
	CString sResult;

	sResult.Append(CONSTLIT("CGaianProcessorAI\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("Order: %d\r\n"), (int)GetCurrentOrder()));
	sResult.Append(strPatternSubst(CONSTLIT("m_State: %d\r\n"), m_State));
	sResult.Append(strPatternSubst(CONSTLIT("m_pTarget: %s\r\n"), CSpaceObject::DebugDescribe(m_pTarget)));
	sResult.Append(strPatternSubst(CONSTLIT("m_pDest: %s\r\n"), CSpaceObject::DebugDescribe(m_pDest)));

	return sResult;
	}

CSpaceObject *CGaianProcessorAI::GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget) const

//	GetTarget
//
//	Returns the current target

	{
	switch (m_State)
		{
		case statePulverizingWreck:
			return m_pDest;

		default:
			return NULL;
		}
	}

DWORD CGaianProcessorAI::OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	Communicate
//
//	Handle communications from other objects

	{
	return resNoAnswer;
	}

void CGaianProcessorAI::OnObjDestroyedNotify (const SDestroyCtx &Ctx)

//	OnObjDestroyedNotify
//
//	Deal with an object that has been destroyed

	{
	int i;

	switch (m_State)
		{
		case statePulverizingWreck:
			{
			//	If we just pulverized a wreck, suck in all the particles

			if (Ctx.pObj == m_pDest)
				{
				Metric rDist2 = (m_pDest->GetPos() - m_pShip->GetPos()).Length2();
				if (rDist2 < (MAX_EATING_DISTANCE * MAX_EATING_DISTANCE))
					{
					//	Find the fragment effect in the area

					Metric rBestDist2 = g_InfiniteDistance * g_InfiniteDistance;
					CSpaceObject *pBestScrap = NULL;
					for (i = 0; i < m_pShip->GetSystem()->GetObjectCount(); i++)
						{
						CSpaceObject *pObj = m_pShip->GetSystem()->GetObject(i);

						if (pObj && pObj->GetCategory() == CSpaceObject::catFractureEffect)
							{
							CVector vRange = pObj->GetPos() - m_pDest->GetPos();
							Metric rDistance2 = vRange.Dot(vRange);

							if (rDistance2 < rBestDist2)
								{
								rBestDist2 = rDistance2;
								pBestScrap = pObj;
								}
							}
						}

					//	If we found the fracture effect, then set the state

					if (pBestScrap)
						{
						SetState(stateConsumingWreck);
						m_pDest = pBestScrap;

						m_pDest->SetAttractor(m_pShip);

						//	Recharge our demolition cannon

						CInstalledDevice *pWeapon = m_pShip->GetDevice(m_iDestructorDev);
						if (pWeapon)
							{
							int iCharges = pWeapon->GetCharges(m_pShip);
							if (iCharges < MAX_CHARGES)
								pWeapon->IncCharges(m_pShip, CHARGE_INCREASE);
							}

						//	Manufacture ammunition for our thermo cannons

						if (m_dwAmmo)
							{
							CItem MoreAmmo(g_pUniverse->FindItemType(m_dwAmmo), 25);
							CItemListManipulator ObjList(m_pShip->GetItemList());
							ObjList.AddItem(MoreAmmo);

							m_pShip->OnComponentChanged(comCargo);
							m_pShip->ItemsModified();
							m_pShip->InvalidateItemListAddRemove();
							}
						}
					}
				}

			break;
			}
		}

	//	If we haven't handled it, make sure we rest

	if (m_pTarget == Ctx.pObj)
		SetState(stateNone);

	if (m_pDest == Ctx.pObj)
		SetState(stateNone);
	}

void CGaianProcessorAI::OnOrderChanged (void)

//	OnOrderChanged
//
//	The order list has changed

	{
	SetState(stateNone);
	}

void CGaianProcessorAI::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read our data
//
//	DWORD		m_State
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pDest (CSpaceObject ref)
//	Metric		m_rOrbitRadius
//	DWORD		m_iStartTime

	{
	Ctx.pStream->Read((char *)&m_State, sizeof(DWORD));
	CSystem::ReadObjRefFromStream(Ctx, &m_pTarget);
	CSystem::ReadObjRefFromStream(Ctx, &m_pDest);
	Ctx.pStream->Read((char *)&m_rOrbitRadius, sizeof(Metric));
	Ctx.pStream->Read((char *)&m_iStartTime, sizeof(DWORD));
	}

void CGaianProcessorAI::OnSystemLoadedNotify (void)

//	OnSystemLoadedNotify
//
//	System has finished loading

	{
	CalcDevices();
	}

void CGaianProcessorAI::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write our data
//
//	DWORD		m_State
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pDest (CSpaceObject ref)
//	Metric		m_rOrbitRadius
//	DWORD		m_iStartTime

	{
	pStream->Write((char *)&m_State, sizeof(DWORD));
	m_pShip->WriteObjRefToStream(m_pTarget, pStream);
	m_pShip->WriteObjRefToStream(m_pDest, pStream);
	pStream->Write((char *)&m_rOrbitRadius, sizeof(Metric));
	pStream->Write((char *)&m_iStartTime, sizeof(DWORD));
	}

void CGaianProcessorAI::SetState (StateTypes State)

//	SetState
//
//	Sets the current state

	{
	m_State = State;

	m_pTarget = NULL;
	m_pDest = NULL;

	//	If we're currently docked and we're changing to a state that
	//	does not support docking, then we undock first.

	if (m_pShip->GetDockedObj()
			&& State != stateNone)
		m_pShip->Undock();
	}
