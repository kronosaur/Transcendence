//	CNPFleet.cpp
//
//	Implements CNPFleet class

#include "Alchemy.h"
#include "NPEngine.h"

CNPFleet::CNPFleet (void) : CObject(NULL)

//	CNPFleet constructor

	{
	}

ALERROR CNPFleet::Create (DWORD dwUNID,
						  DWORD dwLocation,
						  CNPSovereign *pSovereign,
						  CNPFleet **retpFleet)

//	Create
//
//	Creates a new fleet

	{
	CNPFleet *pFleet;

	//	Allocate the new fleet

	pFleet = new CNPFleet;
	if (pFleet == NULL)
		return ERR_MEMORY;

	//	Initialize

	pFleet->m_dwUNID = dwUNID;
	pFleet->m_dwLocation = dwLocation;
	pFleet->m_pSovereign = pSovereign;

	pFleet->m_dwDest = dwLocation;
	pFleet->m_iInTransit = 0;

	//	Done

	*retpFleet = pFleet;

	return NOERROR;
	}

int CNPFleet::GetHyperdrive (CNPUniverse *pUniv)

//	GetHyperdrive
//
//	Returns the hyperdrive rating for the fleet. This is the smallest
//	hyperdrive of all ships in the fleet

	{
	int i;
	int iHyperdrive = 0;

	for (i = 0; i < m_Assets.GetCount(); i++)
		if (m_Assets.IsUnit(i))
			{
			CNPUnit Unit = m_Assets.GetUnitByIndex(i);
			CNPUnitClass *pClass = pUniv->GetUnitClass(Unit.GetClassUNID());

			if (pClass->GetHyperdrive() > iHyperdrive)
				iHyperdrive = pClass->GetHyperdrive();
			}

	return iHyperdrive;
	}
