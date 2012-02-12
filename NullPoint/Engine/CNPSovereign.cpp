//	CNPSovereign.cpp
//
//	Implements CNPSovereign class

#include "Alchemy.h"
#include "NPEngine.h"

CNPSovereign::CNPSovereign (void) : CObject(NULL)
	{
	}

ALERROR CNPSovereign::Create (DWORD dwUNID, CString sName, CNPSovereign **retpSovereign)

//	Create
//
//	Creates a new sovereign

	{
	CNPSovereign *pSovereign;

	pSovereign = new CNPSovereign;
	if (pSovereign == NULL)
		return ERR_MEMORY;

	//	Initialize

	pSovereign->m_dwUNID = dwUNID;
	pSovereign->m_sName = sName;
	pSovereign->m_dwCapital = NullUNID;

	//	Done

	*retpSovereign = pSovereign;

	return NOERROR;
	}
