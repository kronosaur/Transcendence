//	CTerritoryDef.cpp
//
//	CTerritoryDef class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

#define ATTRIBUTES_ATTRIB				CONSTLIT("attributes")
#define CRITERIA_ATTRIB					CONSTLIT("criteria")
#define ID_ATTRIB						CONSTLIT("id")
#define MAX_RADIUS_ATTRIB				CONSTLIT("maxRadius")
#define MIN_RADIUS_ATTRIB				CONSTLIT("minRadius")

CTerritoryDef::CTerritoryDef (void) : m_bMarked(false)

//	CTerritoryDef constructor

	{
	}

void CTerritoryDef::AddAttributes (const CString &sAttribs)

//	AddAttributes
//
//	Adds the attributes to the territory

	{
	m_sAttributes = ::AppendModifiers(m_sAttributes, sAttribs);
	}

void CTerritoryDef::AddRegion (const COrbit &Orbit, Metric rMinRadius, Metric rMaxRadius)

//	AddRegion
//
//	Adds a region to the territory

	{
	SRegion *pRegion = m_Regions.Insert();
	pRegion->OrbitDesc = Orbit;
	pRegion->rMinRadius = rMinRadius;
	pRegion->rMaxRadius = rMaxRadius;

	pRegion->rMinRadius2 = rMinRadius * rMinRadius;
	pRegion->rMaxRadius2 = rMaxRadius * rMaxRadius;
	}

ALERROR CTerritoryDef::CreateFromXML (CXMLElement *pDesc, const COrbit &OrbitDesc, CTerritoryDef **retpTerritory)

//	CreateFromXML
//
//	Creates a territory from XML

	{
	CTerritoryDef *pTerritory = new CTerritoryDef;

	pTerritory->m_sID = pDesc->GetAttribute(ID_ATTRIB);

	//	Define the territory

	Metric rScale = GetScale(pDesc);
	pTerritory->AddRegion(OrbitDesc,
			rScale * pDesc->GetAttributeInteger(MIN_RADIUS_ATTRIB),
			rScale * pDesc->GetAttributeInteger(MAX_RADIUS_ATTRIB));

	//	Set the criteria (if any)

	pTerritory->SetCriteria(pDesc->GetAttribute(CRITERIA_ATTRIB));

	//	Define the attributes

	CString sAttribs;
	if (pDesc->FindAttribute(ATTRIBUTES_ATTRIB, &sAttribs))
		pTerritory->AddAttributes(sAttribs);

	//	Done

	if (retpTerritory)
		*retpTerritory = pTerritory;

	return NOERROR;
	}

bool CTerritoryDef::HasAttribute (const CString &sAttrib)

//	HasAttribute
//
//	Returns TRUE if we have the given attribute

	{
	return ::HasModifier(m_sAttributes, sAttrib);
	}

bool CTerritoryDef::HasAttribute (const CVector &vPos, const CString &sAttrib)

//	HasAttribute
//
//	Returns TRUE if we have the given attribute

	{
	return (PointInTerritory(vPos) && ::HasModifier(m_sAttributes, sAttrib));
	}

bool CTerritoryDef::PointInTerritory (const CVector &vPos)

//	PointInTerritory
//
//	Returns TRUE if the given point is in the territory

	{
	int i;

	for (i = 0; i < m_Regions.GetCount(); i++)
		{
		SRegion *pRegion = &m_Regions[i];
		CVector vDist = vPos - pRegion->OrbitDesc.GetObjectPos();
		Metric rDist2 = vDist.Length2();

		if ((rDist2 < pRegion->rMaxRadius2 || pRegion->rMaxRadius2 == 0.0)
				&& rDist2 >= pRegion->rMinRadius2)
			return true;
		}

	return false;
	}

void CTerritoryDef::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read data from stream
//
//	CString			m_sID
//	CString			m_sAttributes
//	CString			m_sCriteria
//
//	DWORD			count of regions
//	SRegion			m_Regions[]

	{
	DWORD dwLoad;
	int i;

	m_sID.ReadFromStream(Ctx.pStream);
	m_sAttributes.ReadFromStream(Ctx.pStream);
	m_sCriteria.ReadFromStream(Ctx.pStream);

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_Regions.InsertEmpty(dwLoad);

	for (i = 0; i < (int)dwLoad; i++)
		{
		SRegion *pRegion = &m_Regions[i];
		Ctx.pStream->Read((char *)&pRegion->OrbitDesc, sizeof(COrbit));
		Ctx.pStream->Read((char *)&pRegion->rMaxRadius, sizeof(Metric));
		Ctx.pStream->Read((char *)&pRegion->rMinRadius, sizeof(Metric));

		pRegion->rMinRadius2 = pRegion->rMinRadius * pRegion->rMinRadius;
		pRegion->rMaxRadius2 = pRegion->rMaxRadius * pRegion->rMaxRadius;
		}

	m_bMarked = false;
	}

void CTerritoryDef::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write out to stream
//
//	CString			m_sID
//	CString			m_sAttributes
//	CString			m_sCriteria
//
//	DWORD			count of regions
//	SRegion			m_Regions[]

	{
	DWORD dwSave;
	int i;

	m_sID.WriteToStream(pStream);
	m_sAttributes.WriteToStream(pStream);
	m_sCriteria.WriteToStream(pStream);

	dwSave = m_Regions.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < m_Regions.GetCount(); i++)
		{
		SRegion *pRegion = &m_Regions[i];
		pStream->Write((char *)&pRegion->OrbitDesc, sizeof(COrbit));
		pStream->Write((char *)&pRegion->rMaxRadius, sizeof(Metric));
		pStream->Write((char *)&pRegion->rMinRadius, sizeof(Metric));
		}
	}
