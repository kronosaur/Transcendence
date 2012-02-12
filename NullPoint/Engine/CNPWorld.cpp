//	CNPWorld.cpp
//
//	Implements CNPWorld class

#include "Alchemy.h"
#include "NPEngine.h"

CNPWorld::CNPWorld (void) : CObject(NULL)

//	CNPWorld constructor

	{
	}

ALERROR CNPWorld::Create (DWORD dwUNID,
					   DWORD dwLocation,
					   CNPSovereign *pSovereign,
					   CNPWorld **retpWorld)

//	Create
//
//	Creates a new world

	{
	CNPWorld *pWorld;

	//	Allocate the new world

	pWorld = new CNPWorld;
	if (pWorld == NULL)
		return ERR_MEMORY;

	//	Initialize

	pWorld->m_dwUNID = dwUNID;
	pWorld->m_dwLocation = dwLocation;
	pWorld->m_pSovereign = pSovereign;

	pWorld->m_iPopulation = 0;
	pWorld->m_iTechLevel = techNone;
	pWorld->m_iEfficiency = 0;
	pWorld->m_iDestiny = mathRandom(0, 3599);

	//	Done

	*retpWorld = pWorld;

	return NOERROR;
	}

NPTechLevels CNPWorld::GetMinTech (void)

//	GetMinTech
//
//	Returns the minimum tech level that can support a
//	population on this world.

	{
	if (HasTrait(traitAirless))
		return techFusion;
	else if (HasTrait(traitRadioactive))
		return techBio;
	else if (HasTrait(traitPermafrost) || HasTrait(traitDesert))
		return techSteam;
	else
		return techAgricultural;
	}
