//	Markers.cpp
//
//	Marker classes

#include "PreComp.h"


//	CMarker -------------------------------------------------------------------

static CObjectClass<CMarker>g_MarkerClass(OBJID_CMARKER, NULL);

CMarker::CMarker (void) : CSpaceObject(&g_MarkerClass)

//	CMarker constructor

	{
	}

ALERROR CMarker::Create (CSystem *pSystem,
						 CSovereign *pSovereign,
						 const CVector &vPos,
						 const CVector &vVel,
						 const CString &sName,
						 CMarker **retpMarker)

//	Create
//
//	Create a marker

	{
	ALERROR error;
	CMarker *pMarker;

	pMarker = new CMarker;
	if (pMarker == NULL)
		return ERR_MEMORY;

	pMarker->Place(vPos, vVel);
	pMarker->SetCannotBeHit();

	//	Note: Cannot disable destruction notification because some markers
	//	are used as destination targets for ships

	pMarker->m_sName = sName;
	pMarker->m_pSovereign = pSovereign;
	if (pMarker->m_pSovereign == NULL)
		pMarker->m_pSovereign = g_pUniverse->FindSovereign(g_PlayerSovereignUNID);

	//	Add to system

	if (error = pMarker->AddToSystem(pSystem))
		{
		delete pMarker;
		return error;
		}

	//	Done

	if (retpMarker)
		*retpMarker = pMarker;

	return NOERROR;
	}

CSovereign *CMarker::GetSovereign (void) const
	{
	//	Return player

	return m_pSovereign;
	}

void CMarker::OnObjLeaveGate (CSpaceObject *pObj)

//	OnObjLeaveGate
//
//	Object leaves a gate

	{
	CEffectCreator *pEffect = g_pUniverse->FindEffectType(g_StargateOutUNID);
	if (pEffect)
		pEffect->CreateEffect(GetSystem(),
				NULL,
				GetPos(),
				NullVector,
				0);
	}

void CMarker::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read the object's data from a stream
//
//	CString		m_sName

	{
	if (Ctx.dwVersion >= 8)
		{
		m_sName.ReadFromStream(Ctx.pStream);
		CSystem::ReadSovereignRefFromStream(Ctx, &m_pSovereign);
		}
	}

void CMarker::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write the object's data to a stream
//
//	CString		m_sName
//	DWORD		m_pSovereign (CSovereign ref)

	{
	m_sName.WriteToStream(pStream);
	GetSystem()->WriteSovereignRefToStream(m_pSovereign, pStream);
	}

//	CPOVMarker ----------------------------------------------------------------

static CObjectClass<CPOVMarker>g_POVMarkerClass(OBJID_CPOVMARKER, NULL);

CPOVMarker::CPOVMarker (void) : CSpaceObject(&g_POVMarkerClass)

//	CPOVMarker constructor

	{
	}

ALERROR CPOVMarker::Create (CSystem *pSystem,
							const CVector &vPos,
							const CVector &vVel,
							CPOVMarker **retpMarker)

//	Create
//
//	Create a marker

	{
	ALERROR error;
	CPOVMarker *pMarker;

	pMarker = new CPOVMarker;
	if (pMarker == NULL)
		return ERR_MEMORY;

	pMarker->Place(vPos, vVel);
	pMarker->DisableObjectDestructionNotify();
	pMarker->SetCannotBeHit();

	//	Add to system

	if (error = pMarker->AddToSystem(pSystem))
		{
		delete pMarker;
		return error;
		}

	//	Done

	if (retpMarker)
		*retpMarker = pMarker;

	return NOERROR;
	}

CSovereign *CPOVMarker::GetSovereign (void) const
	{
	//	Return player

	return g_pUniverse->FindSovereign(g_PlayerSovereignUNID);
	}

void CPOVMarker::OnLosePOV (void)

//	OnLosePOV
//
//	Marker is no longer part of POV

	{
	Destroy(removedFromSystem, CDamageSource());
	}
