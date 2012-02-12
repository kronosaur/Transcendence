//	CNPNullPoint.cpp
//
//	Implements CNPNullPoint class

#include "Alchemy.h"
#include "NPEngine.h"

CNPNullPoint::CNPNullPoint (void) : CObject(NULL),
		m_pWorld(NULL)
	{
	}

CNPNullPoint::~CNPNullPoint (void)
	{
	}

ALERROR CNPNullPoint::AddWorld (CNPWorld *pWorld)

//	AddWorld
//
//	Adds a world to this null point

	{
	ASSERT(m_pWorld == NULL);

	m_pWorld = pWorld;

	return NOERROR;
	}

ALERROR CNPNullPoint::Create (DWORD dwUNID, int x, int y, CNPNullPoint **retpNullPoint)

//	Create
//
//	Creates a new null point

	{
	CNPNullPoint *pNP;

	//	Allocate the new null point

	pNP = new CNPNullPoint;
	if (pNP == NULL)
		return ERR_MEMORY;

	//	Initialize

	pNP->m_dwUNID = dwUNID;
	pNP->m_x = x;
	pNP->m_y = y;
	pNP->m_pWorld = NULL;
	pNP->m_pFirstFleet = NULL;

	//	Done

	*retpNullPoint = pNP;

	return NOERROR;
	}

ALERROR CNPNullPoint::CreateLinkTo (CNPNullPoint *pDest)

//	CreateLinkTo
//
//	Creates a bi-directional link to pDest

	{
	ALERROR error;

	if (error = m_Links.AppendObject(pDest, NULL))
		return error;

	if (error = pDest->m_Links.AppendObject(this, NULL))
		return error;

	return NOERROR;
	}

bool CNPNullPoint::IsLinkedTo (CNPNullPoint *pDest)

//	IsLinkedTo
//
//	Returns TRUE if the destination is linked to this null point

	{
	for (int i = 0; i < GetLinkCount(); i++)
		if (pDest->m_dwUNID == GetLinkDest(i)->m_dwUNID)
			return true;

	return false;
	}

void CNPNullPoint::PlaceFleet (CNPFleet *pFleet)

//	PlaceFleet
//
//	Places the given fleet here

	{
	pFleet->SetNextFleet(m_pFirstFleet);
	m_pFirstFleet = pFleet;
	}

void CNPNullPoint::RemoveFleet (CNPFleet *pFleet)

//	RemoveFleet
//
//	Removes the fleet

	{
	CNPFleet *pNextFleet = m_pFirstFleet;
	CNPFleet *pPrevFleet = NULL;

	//	Look for the fleet

	while (pNextFleet != NULL && pNextFleet != pFleet)
		{
		CNPFleet *pPrevFleet = pNextFleet;
		pNextFleet = pNextFleet->GetNextFleet();
		}

	if (pNextFleet == pFleet)
		{
		if (pPrevFleet == NULL)
			m_pFirstFleet = pFleet->GetNextFleet();
		else
			pPrevFleet->SetNextFleet(pFleet->GetNextFleet());
		}
	}
