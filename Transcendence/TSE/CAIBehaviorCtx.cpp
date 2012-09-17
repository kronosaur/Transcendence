//	CAIBehaviorCtx.cpp
//
//	CAIBehaviorCtx class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

const Metric MIN_STATION_TARGET_DIST =	(10.0 * LIGHT_SECOND);
const Metric MIN_TARGET_DIST =			(5.0 * LIGHT_SECOND);
const int MULTI_HIT_WINDOW =			20;
const Metric WALL_RANGE =				(g_KlicksPerPixel * 300.0);

const Metric MIN_STATION_TARGET_DIST2 =	(MIN_STATION_TARGET_DIST * MIN_STATION_TARGET_DIST);
const Metric MIN_TARGET_DIST2 =			(MIN_TARGET_DIST * MIN_TARGET_DIST);
const Metric WALL_RANGE2 =				(WALL_RANGE * WALL_RANGE);

CAIBehaviorCtx::CAIBehaviorCtx (void) :
		m_iLastTurn(IShipController::NoRotation),
		m_iLastTurnCount(0),
		m_iManeuverCounter(0),
		m_iLastAttack(0),
		m_pNavPath(NULL),
		m_iNavPathPos(-1),
		m_pShields(NULL),
		m_iBestWeapon(devNone),
		m_fDockingRequested(false),
		m_fImmobile(false),
		m_fWaitForShieldsToRegen(false),
		m_fSuperconductingShields(false),
		m_fHasMultipleWeapons(false),
		m_fHasSecondaryWeapons(false),
		m_fRecalcBestWeapon(true)

//	CAIBehaviorCtx constructor

	{
	}

void CAIBehaviorCtx::CalcAvoidPotential (CShip *pShip, CSpaceObject *pTarget)

//	CalcAvoidPotential
//
//	Update m_vPotential.
//
//	Sets a vector pointing away from any dangers and obstacles. The
//	size of the vector is proportional to the danger.

	{
	if (pShip->IsDestinyTime(11))
		{
		Metric rDist;
		CVector vPotential;
		Metric rMinSeparation2 = GetMinCombatSeparation() * GetMinCombatSeparation();
		Metric rSeparationForce = g_KlicksPerPixel * 40.0 / GetMinCombatSeparation();

		CSystem *pSystem = pShip->GetSystem();
		SSpaceObjectGridEnumerator i;
		pSystem->EnumObjectsInBoxStart(i, pShip->GetPos(), WALL_RANGE, gridNoBoxCheck);

		while (pSystem->EnumObjectsInBoxHasMore(i))
			{
			CSpaceObject *pObj = pSystem->EnumObjectsInBoxGetNextFast(i);

			if (pObj == NULL || pObj == pShip || pObj == pTarget || pObj->IsDestroyed())
				NULL;
			else if (pObj->Blocks(pShip))
				{
				CVector vTarget = pObj->GetPos() - pShip->GetPos();
				Metric rTargetDist2 = vTarget.Dot(vTarget);

				//	There is a sharp potential away from walls

				if (rTargetDist2 < WALL_RANGE2)
					{
					CVector vTargetN = vTarget.Normal(&rDist);
					if (rDist > 0.0)
						vPotential = vPotential - (vTargetN * 100.0 * g_KlicksPerPixel * (WALL_RANGE / rDist));
					}
				}
			else if (pObj->GetCategory() == CSpaceObject::catShip)
				{
				CVector vTarget = pObj->GetPos() - pShip->GetPos();
				Metric rTargetDist2 = vTarget.Dot(vTarget);

				//	If we get too close to this ship, then move away

				if (rTargetDist2 < rMinSeparation2)
					{
					CVector vTargetN = vTarget.Normal(&rDist);
					if (rDist > 0.0)
						{
						Metric rCloseness = GetMinCombatSeparation() - rDist;
						vPotential = vPotential - (vTargetN * rSeparationForce * rCloseness);
						}
					}
				}
			}

		//	Sets the potential

		m_vPotential = vPotential;

#ifdef DEBUG_AVOID_POTENTIAL
		pShip->SetDebugVector(vPotential);
#endif
		}
	}

void CAIBehaviorCtx::CalcBestWeapon (CShip *pShip, CSpaceObject *pTarget, Metric rTargetDist2)

//	CalcBestWeapon
//
//	Initializes:
//
//	m_iBestWeapon
//	m_pBestWeapon
//	m_rBestWeaponRange

	{
	int i;

	if (m_fRecalcBestWeapon)
		{
		ASSERT(pShip);

		m_rMaxWeaponRange = 0.0;

		Metric rBestRange = g_InfiniteDistance;
		int iBestWeapon = -1;
		int iBestWeaponVariant = 0;
		int iBestScore = 0;
		int iPrimaryCount = 0;
		int iBestNonLauncherLevel = 0;
		bool bHasSecondaryWeapons = false;

		//	Loop over all devices to find the best weapon

		for (i = 0; i < pShip->GetDeviceCount(); i++)
			{
			CInstalledDevice *pWeapon = pShip->GetDevice(i);

			//	If this weapon is not working, then skip it

			if (pWeapon->IsEmpty() || pWeapon->IsDamaged() || pWeapon->IsDisrupted() || !pWeapon->IsEnabled())
				continue;

			//	If this is a secondary weapon, remember that we have some and 
			//	keep track of the best range.

			else if (pWeapon->IsSecondaryWeapon())
				{
				//	Remember the range in case we end up with no good weapons and we need to set 
				//	a course towards the target.

				Metric rRange = pWeapon->GetClass()->GetMaxEffectiveRange(pShip, pWeapon, pTarget);
				if (rRange < rBestRange)
					rBestRange = rRange;

				bHasSecondaryWeapons = true;
				continue;
				}

			//	Otherwise, this is a primary weapon or launcher

			else
				{
				//	Compute score

				switch (pWeapon->GetCategory())
					{
					case itemcatWeapon:
						{
						int iScore = CalcWeaponScore(pShip, pTarget, pWeapon, rTargetDist2);
						if (iScore > iBestScore)
							{
							iBestWeapon = i;
							iBestWeaponVariant = 0;
							iBestScore = iScore;
							}

						Metric rMaxRange = pWeapon->GetMaxEffectiveRange(pShip);
						if (rMaxRange > m_rMaxWeaponRange)
							m_rMaxWeaponRange = rMaxRange;

						if (pWeapon->GetClass()->GetLevel() > iBestNonLauncherLevel)
							iBestNonLauncherLevel = pWeapon->GetClass()->GetLevel();

						iPrimaryCount++;
						break;
						}

					case itemcatLauncher:
						{
						int iCount = pShip->GetMissileCount();
						if (iCount > 0)
							{
							pShip->ReadyFirstMissile();

							for (int j = 0; j < iCount; j++)
								{
								int iScore = CalcWeaponScore(pShip, pTarget, pWeapon, rTargetDist2);

								//	If we only score 1 and we've got secondary weapons, then don't
								//	bother with this missile (we don't want to waste it)

								if (iScore == 1 && HasSecondaryWeapons())
									{
									iScore = 0;

									//	Remember the range in case we end up with no good weapons and we need to set 
									//	a course towards the target.

									Metric rRange = pWeapon->GetClass()->GetMaxEffectiveRange(pShip, pWeapon, pTarget);
									if (rRange < rBestRange)
										rBestRange = rRange;

									if (rRange > m_rMaxWeaponRange)
										m_rMaxWeaponRange = rRange;
									}

								if (iScore > iBestScore)
									{
									iBestWeapon = i;
									iBestWeaponVariant = j;
									iBestScore = iScore;
									}

								pShip->ReadyNextMissile();
								}

							iPrimaryCount++;
							}
						break;
						}
					}
				}
			}

		//	Given the best weapon, select the named device

		if (iBestWeapon != -1)
			{
			m_iBestWeapon = pShip->SelectWeapon(iBestWeapon, iBestWeaponVariant);
			m_pBestWeapon = pShip->GetNamedDevice(m_iBestWeapon);
			m_rBestWeaponRange = m_pBestWeapon->GetClass()->GetMaxEffectiveRange(pShip, m_pBestWeapon, pTarget);

			//	Optimum range varies by ship (destiny)

			int iAdj = 100 + ((pShip->GetDestiny() % 60) - 30);
			m_rBestWeaponRange = m_rBestWeaponRange * (iAdj * 0.01);
			}
		else
			{
			m_iBestWeapon = devNone;
			m_pBestWeapon = NULL;

			//	If we can't find a good weapon, at least set the weapon range so that we close
			//	to secondary weapon range.

			if (bHasSecondaryWeapons)
				m_rBestWeaponRange = rBestRange;
			else
				m_rBestWeaponRange = 60.0 * LIGHT_SECOND;
			}

		//	Set some invariants

		m_fHasSecondaryWeapons = bHasSecondaryWeapons;
		m_iBestNonLauncherWeaponLevel = iBestNonLauncherLevel;
		m_fHasMultipleWeapons = (iPrimaryCount > 1);

		//	Done

		m_fRecalcBestWeapon = false;
		}
	}

void CAIBehaviorCtx::CalcInvariants (CShip *pShip)

//	CalcInvariants
//
//	Calculates some invariant properties of the ship

	{
	int i;

	//	Basic properties

	m_fImmobile = (pShip->GetMaxSpeed() == 0.0);

	//	Primary aim range

	Metric rPrimaryRange = pShip->GetWeaponRange(devPrimaryWeapon);
	Metric rAimRange = (GetFireRangeAdj() * rPrimaryRange) / (100.0 * ((pShip->GetDestiny() % 8) + 4));
	if (rAimRange < 1.5 * MIN_TARGET_DIST)
		rAimRange = 1.5 * MIN_TARGET_DIST;
	m_rPrimaryAimRange2 = 4.0 * rAimRange * rAimRange;

	//	Flank distance

	int iAdj = 2 * Max(0, (8 - pShip->GetClass()->GetManeuverability()));
	m_rFlankDist = (iAdj * ((pShip->GetDestiny() % 61) - 30) * g_KlicksPerPixel);

	//	Max turn count

	m_iMaxTurnCount =  (10 + (pShip->GetDestiny() % 20)) * pShip->GetClass()->GetManeuverability();

	//	Compute some properties of installed devices

	m_pShields = NULL;
	m_fSuperconductingShields = false;
	m_iBestNonLauncherWeaponLevel = 0;
	m_fHasSecondaryWeapons = false;

	for (i = 0; i < pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = pShip->GetDevice(i);

		if (pDevice->IsEmpty() || pDevice->IsDamaged() || pDevice->IsDisrupted() || !pDevice->IsEnabled())
			continue;

		switch (pDevice->GetCategory())
			{
			case itemcatWeapon:
			case itemcatLauncher:
				{
				//	Figure out the best non-launcher level

				if (pDevice->GetCategory() != itemcatLauncher
						&& pDevice->GetClass()->GetLevel() > m_iBestNonLauncherWeaponLevel)
					{
					m_iBestNonLauncherWeaponLevel = pDevice->GetClass()->GetLevel();
					}

				//	Secondary

				if (pDevice->IsSecondaryWeapon())
					m_fHasSecondaryWeapons = true;

				break;
				}

			case itemcatShields:
				m_pShields = pDevice;
				if (pDevice->GetClass()->GetUNID() == g_SuperconductingShieldsUNID)
					m_fSuperconductingShields = true;
				break;
			}
		}

	//	Flags

	m_fThrustThroughTurn = ((pShip->GetDestiny() % 100) < 50);
	m_fAvoidExplodingStations = (rAimRange > MIN_STATION_TARGET_DIST);

	//	Weapon

	m_fRecalcBestWeapon = true;
	CalcBestWeapon(pShip, NULL, 0.0);
	}

bool CAIBehaviorCtx::CalcNavPath (CShip *pShip, CSpaceObject *pTo)

//	CalcNavPath
//
//	Initializes m_pNavPath and m_iNavPathPos. Call this function
//	before using ImplementFollowNavPath

	{
	int i;
	CSystem *pSystem = pShip->GetSystem();

	ASSERT(pTo);

	//	Figure out an appropriate starting point

	CSpaceObject *pBestObj = NULL;
	Metric rBestDist2 = pShip->GetDistance2(pTo);
	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);
		if (pObj
				&& pObj != pTo
				&& (pObj->IsActiveStargate()
					|| pObj->IsMarker()
					|| (pObj->GetCategory() == CSpaceObject::catStation
						&& pObj->GetScale() == scaleStructure
						&& !pObj->IsInactive()
						&& pObj->HasAttribute(CONSTLIT("populated"))
						&& pObj->IsFriend(pShip))))
			{
			Metric rDist2 = (pObj->GetPos() - pShip->GetPos()).Length2();
			if (rDist2 < rBestDist2)
				{
				pBestObj = pObj;
				rBestDist2 = rDist2;
				}
			}
		}

	if (pBestObj == NULL)
		return false;

	CSpaceObject *pFrom = pBestObj;

	//	Get the appropriate nav path from the system

	CNavigationPath *pPath = pSystem->GetNavPath(pShip->GetSovereign(), pFrom, pTo);

	//	Done

	CalcNavPath(pShip, pPath);
	return true;
	}

void CAIBehaviorCtx::CalcNavPath (CShip *pShip, CSpaceObject *pFrom, CSpaceObject *pTo)

//	CalcNavPath
//
//	Initializes m_pNavPath and m_iNavPathPos. Call this function
//	before using ImplementFollowNavPath

	{
	ASSERT(pFrom);
	ASSERT(pTo);

	//	Get the appropriate nav path from the system

	CSystem *pSystem = pShip->GetSystem();
	CNavigationPath *pPath = pSystem->GetNavPath(pShip->GetSovereign(), pFrom, pTo);

	//	Done

	CalcNavPath(pShip, pPath);
	}

void CAIBehaviorCtx::CalcNavPath (CShip *pShip, CNavigationPath *pPath)

//	CalcNavPath
//
//	Initializes ship state to follow the given path

	{
	int i;

	ASSERT(pPath);

	//	Set the path

	m_pNavPath = pPath;

	//	Figure out which nav position we are closest to

	const Metric CLOSE_ENOUGH_DIST = (LIGHT_SECOND * 10.0);
	const Metric CLOSE_ENOUGH_DIST2 = CLOSE_ENOUGH_DIST * CLOSE_ENOUGH_DIST;
	Metric rBestDist2 = (g_InfiniteDistance * g_InfiniteDistance);
	int iBestPoint = -1;

	for (i = 0; i < m_pNavPath->GetNavPointCount(); i++)
		{
		CVector vDist = m_pNavPath->GetNavPoint(i) - pShip->GetPos();
		Metric rDist2 = vDist.Length2();

		if (rDist2 < rBestDist2)
			{
			rBestDist2 = rDist2;
			iBestPoint = i;

			if (rDist2 < CLOSE_ENOUGH_DIST2)
				break;
			}
		}

	//	Done

	ASSERT(iBestPoint != -1);
	if (iBestPoint == -1)
		iBestPoint = 0;

	m_iNavPathPos = iBestPoint;
	}

void CAIBehaviorCtx::CalcShieldState (CShip *pShip)

//	CalcShieldState
//
//	Updates m_fWaitForShieldsToRegen

	{
	if (m_pShields
			&& !NoShieldRetreat()
			&& pShip->IsDestinyTime(17) 
			&& !m_fSuperconductingShields)
		{
		int iHPLeft, iMaxHP;
		m_pShields->GetStatus(pShip, &iHPLeft, &iMaxHP);

		//	If iMaxHP is 0 then we treat the shields as up. This can happen
		//	if a ship with (e.g.) hull-plate ionizer gets its armor destroyed

		int iLevel = (iMaxHP > 0 ? (iHPLeft * 100 / iMaxHP) : 100);

		//	If we're waiting for shields to regenerate, see if
		//	they are back to 60% or better

		if (m_fWaitForShieldsToRegen)
			{
			if (iLevel >= 60)
				m_fWaitForShieldsToRegen = false;
			}

		//	Otherwise, if shields are down, then we need to wait
		//	until they regenerate

		else
			{
			if (iLevel <= 0)
				m_fWaitForShieldsToRegen = true;
			}
		}
	}

int CAIBehaviorCtx::CalcWeaponScore (CShip *pShip, CSpaceObject *pTarget, CInstalledDevice *pWeapon, Metric rTargetDist2)

//	CalcWeaponScore
//
//	Calculates a score for this weapon

	{
	int iScore = 0;

	//	If this is an EMP weapon adjust the score based on the state of
	//	the target.

	int iEffectiveness = pWeapon->GetClass()->GetWeaponEffectiveness(pShip, pWeapon, pTarget);
	if (iEffectiveness < 0)
		return 0;

	//	Get the range of this weapon

	Metric rRange2 = pWeapon->GetClass()->GetMaxEffectiveRange(pShip, pWeapon, pTarget);
	rRange2 *= rRange2;

	//	If the weapon is out of range of the target then we score 1
	//	(meaning that it is better than nothing (0) but we would rather any
	//	other weapon)

	if (rRange2 < rTargetDist2)
		return 1;

	//	If this weapon will take a while to get ready, then 
	//	lower the score.

	if (pWeapon->GetTimeUntilReady() >= 15)
		return 1;

	//	Get the item for the selected variant (either the weapon
	//	or the ammo)

	CItemType *pType;
	pWeapon->GetClass()->GetSelectedVariantInfo(pShip,
			pWeapon,
			NULL,
			NULL,
			&pType);

	//	Base score is based on the level of the variant

	iScore += pType->GetLevel() * 10;

	//	Missiles count for more

	if (pWeapon->GetCategory() == itemcatLauncher)
		{
		//	Don't waste missiles on "lesser" targets

		if (pTarget 
				&& pTarget->GetCategory() == CSpaceObject::catShip
				&& pTarget->GetLevel() <= (m_iBestNonLauncherWeaponLevel + 2)
				&& pTarget->GetLevel() <= (pType->GetLevel() - 2)
				&& !pTarget->IsPlayer())
			return 1;

		//	Otherwise, count for more

		else
			iScore += 20;
		}

	//	Adjust score based on effectiveness

	iScore += iEffectiveness;

	//	If this weapon has a fire arc and the target is in the arc, then prefer this weapon

	if (pTarget && pWeapon->IsDirectional() && pWeapon->IsWeaponAligned(pShip, pTarget))
		iScore += 10;

	//	If this is an area weapon then make sure there aren't too many friendlies around

#if 0
	if (iScore > 0 && pWeapon->IsAreaWeapon(pShip))
		{
		int i;
		CSystem *pSystem = pShip->GetSystem();
		int iFireAngle = (pShip->GetRotation() + pWeapon->GetRotation()) % 360;

		for (i = 0; i < pSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = pSystem->GetObject(i);

			if (pObj && pObj != pShip
					&& pObj->CanAttack()
					&& !IsEnemy(pObj))
				{
				CVector vDist = GetPos() - pObj->GetPos();
				Metric rDist2 = vDist.Length2();

				if (rDist2 < MAX_AREA_WEAPON_CHECK2)
					{
					int iBearing = VectorToPolar(vDist);
					if (AreAnglesAligned(iFireAngle, iBearing, 170))
						return 1;
					}
				}
			}
		}
#endif

	return iScore;
	}

void CAIBehaviorCtx::CancelDocking (CShip *pShip, CSpaceObject *pBase)

//	CancelDocking
//
//	Cancel docking that we've previously requested.

	{
	//	HACK: If pBase is NULL then we need to search the entire system for the
	//	base that we're trying to dock with. This is because we don't bother
	//	storing the obj that we've request to dock with.

	if (pBase == NULL)
		{
		int i;
		CSystem *pSystem = pShip->GetSystem();
		for (i = 0; i < pSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = pSystem->GetObject(i);
			if (pObj && pObj->IsObjDockedOrDocking(pShip))
				{
				pBase = pObj;
				break;
				}
			}

		if (pBase == NULL)
			return;
		}

	//	Cancel

	pBase->Undock(pShip);
	SetDockingRequested(false);
	}

void CAIBehaviorCtx::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Load the structure from a stream
//
//	CAISettings	m_AISettings
//	CAIShipControls m_ShipControls
//
//	DWORD		m_iLastTurn
//	DWORD		m_iLastTurnCount
//	DWORD		m_iManeuverCounter
//	DWORD		m_iLastAttack
//	CVector		m_vPotential
//	DWORD		m_pNavPath (ID)
//	DWORD		m_iNavPathPos
//
//	DWORD		flags

	{
	DWORD dwLoad;

	//	CAISettings

	m_AISettings.ReadFromStream(Ctx);
	m_ShipControls.ReadFromStream(Ctx);

	//	State

	Ctx.pStream->Read((char *)&m_iLastTurn, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iLastTurnCount, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iManeuverCounter, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iLastAttack, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_vPotential, sizeof(CVector));

	//	Nav path

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad)
		m_pNavPath = Ctx.pSystem->GetNavPathByID(dwLoad);
	else
		m_pNavPath = NULL;

	Ctx.pStream->Read((char *)&m_iNavPathPos, sizeof(DWORD));
	if (m_pNavPath == NULL)
		m_iNavPathPos = -1;

	//	Flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_fDockingRequested =		((dwLoad & 0x00000001) ? true : false);
	m_fWaitForShieldsToRegen =	((dwLoad & 0x00000002) ? true : false);

	//	These flags do not need to be saved

	m_fRecalcBestWeapon = true;
	}

void CAIBehaviorCtx::SetLastAttack (int iTick)

//	SetLastAttack
//
//	Set the last attack time (debouncing for quick hits)

	{
	if (iTick - m_iLastAttack > MULTI_HIT_WINDOW)
		m_iLastAttack = iTick;
	}

void CAIBehaviorCtx::Undock (CShip *pShip)

//	Undock
//
//	Make sure we are undocked so that we can fly around

	{
	//	If docking has been requested the we need to cancel it

	if (IsDockingRequested())
		CancelDocking(pShip, NULL);
	
	//	Otherwise, make sure we are undocked. We rely on the fact that Undock
	//	is a NoOp if we're not docked.

	else
		pShip->Undock();
	}

void CAIBehaviorCtx::WriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	WriteToStream
//
//	Write the structure
//
//	CAISettings	m_AISettings
//	CAIShipControls m_ShipControls
//
//	DWORD		m_iLastTurn
//	DWORD		m_iLastTurnCount
//	DWORD		m_iManeuverCounter
//	DWORD		m_iLastAttack
//	CVector		m_vPotential
//	DWORD		m_pNavPath (ID)
//	DWORD		m_iNavPathPos
//
//	DWORD		flags

	{
	DWORD dwSave;

	//	CAISettings

	m_AISettings.WriteToStream(pStream);
	m_ShipControls.WriteToStream(pSystem, pStream);

	//	State

	pStream->Write((char *)&m_iLastTurn, sizeof(DWORD));
	pStream->Write((char *)&m_iLastTurnCount, sizeof(DWORD));
	pStream->Write((char *)&m_iManeuverCounter, sizeof(DWORD));
	pStream->Write((char *)&m_iLastAttack, sizeof(DWORD));
	pStream->Write((char *)&m_vPotential, sizeof(CVector));

	//	Nav path

	dwSave = (m_pNavPath ? m_pNavPath->GetID() : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&m_iNavPathPos, sizeof(DWORD));

	//	Flags

	dwSave = 0;
	dwSave |= (m_fDockingRequested ?		0x00000001 : 0);
	dwSave |= (m_fWaitForShieldsToRegen ?	0x00000002 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}
