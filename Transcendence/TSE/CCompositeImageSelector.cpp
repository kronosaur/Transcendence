//	CCompositeImageSelector.cpp
//
//	CCompositeImageSelector class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

static CObjectImageArray EMPTY_IMAGE;

bool CCompositeImageSelector::operator== (const CCompositeImageSelector &Val) const

//	Operator ==

	{
	int i;

	if (m_Sel.GetCount() != Val.m_Sel.GetCount())
		return false;

	for (i = 0; i < m_Sel.GetCount(); i++)
		{
		SEntry *pThis = &m_Sel[i];
		SEntry *pVal = &Val.m_Sel[i];

		if (pThis->dwID != pVal->dwID)
			return false;

		if (pThis->iVariant != pVal->iVariant)
			return false;

		if (pThis->dwExtra != pVal->dwExtra)
			return false;
		}

	return true;
	}

void CCompositeImageSelector::AddShipwreck (DWORD dwID, CShipClass *pWreckClass, int iVariant)

//	AddShipwreck
//
//	Adds a shipwreck selection

	{
	SEntry *pEntry = m_Sel.Insert();

	pEntry->dwID = dwID;
	pEntry->dwExtra = (DWORD)pWreckClass;
	if (pWreckClass)
		pEntry->iVariant = (iVariant == -1 ? mathRandom(0, pWreckClass->GetWreckImageVariants() - 1) : iVariant);
	else
		pEntry->iVariant = (iVariant == -1 ? 0 : iVariant);
	}

void CCompositeImageSelector::AddVariant (DWORD dwID, int iVariant)

//	AddVariant
//
//	Adds a variant selection

	{
	SEntry *pEntry = m_Sel.Insert();
	pEntry->dwID = dwID;
	pEntry->iVariant = iVariant;
	pEntry->dwExtra = 0;
	}

CCompositeImageSelector::SEntry *CCompositeImageSelector::FindEntry (DWORD dwID) const

//	FindEntry
//
//	Finds the entry with the given ID (or NULL)

	{
	int i;

	for (i = 0; i < m_Sel.GetCount(); i++)
		if (m_Sel[i].dwID == dwID)
			return &m_Sel[i];

	return NULL;
	}

CShipClass *CCompositeImageSelector::GetShipwreckClass (DWORD dwID) const

//	GetShipwreckClass
//
//	Returns the shipwreck class for the given selection

	{
	SEntry *pEntry = FindEntry(dwID);
	if (pEntry == NULL)
		return NULL;

	return (CShipClass *)pEntry->dwExtra;
	}

CObjectImageArray &CCompositeImageSelector::GetShipwreckImage (DWORD dwID) const

//	GetShipwreckImage
//
//	Returns the shipwreck image for the given selection

	{
	CShipClass *pWreckClass = GetShipwreckClass(dwID);
	if (pWreckClass == NULL)
		return EMPTY_IMAGE;

	return pWreckClass->GetWreckImage();
	}

int CCompositeImageSelector::GetVariant (DWORD dwID) const

//	GetVariant
//
//	Returns the variant for the given selection

	{
	SEntry *pEntry = FindEntry(dwID);
	if (pEntry == NULL)
		return 0;

	return pEntry->iVariant;
	}

void CCompositeImageSelector::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Load from a stream
//
//	DWORD		No of entries
//	For each
//	DWORD		dwID
//	DWORD		iVariant
//	DWORD		Shipwreck UNID

	{
	int i;
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad > 0)
		{
		ASSERT(m_Sel.GetCount() == 0);
		m_Sel.InsertEmpty(dwLoad);

		for (i = 0; i < m_Sel.GetCount(); i++)
			{
			Ctx.pStream->Read((char *)&m_Sel[i].dwID, sizeof(DWORD));
			Ctx.pStream->Read((char *)&m_Sel[i].iVariant, sizeof(DWORD));

			Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
			m_Sel[i].dwExtra = (dwLoad ? (DWORD)g_pUniverse->FindShipClass(dwLoad) : 0);
			}
		}
	}

void CCompositeImageSelector::WriteToStream (IWriteStream *pStream) const

//	WriteToStream
//
//	Writes to a stream
//
//	DWORD		No of entries
//	For each
//	DWORD		dwID
//	DWORD		iVariant
//	DWORD		Shipwreck UNID

	{
	int i;
	DWORD dwSave;

	dwSave = m_Sel.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	
	//	Save each entry

	for (i = 0; i < m_Sel.GetCount(); i++)
		{
		pStream->Write((char *)&m_Sel[i].dwID, sizeof(DWORD));
		pStream->Write((char *)&m_Sel[i].iVariant, sizeof(DWORD));

		CShipClass *pWreckClass = (CShipClass *)m_Sel[i].dwExtra;
		dwSave = (pWreckClass ? pWreckClass->GetUNID() : 0);
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}
	}
