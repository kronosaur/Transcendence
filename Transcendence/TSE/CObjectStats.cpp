//	CObjectStats.cpp
//
//	CObjectStats class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

CObjectStats::SEntry CObjectStats::m_Null;

const CObjectStats::SEntry &CObjectStats::GetEntry (DWORD dwObjID) const

//	GetEntry
//
//	Get the entry for the given object ID

	{
	SEntry *pEntry = m_Stats.GetAt(dwObjID);
	if (pEntry == NULL)
		return m_Null;

	return *pEntry;
	}

CObjectStats::SEntry &CObjectStats::GetEntryActual (DWORD dwObjID)

//	GetEntryActual
//
//	Get the actual entry (for modification)

	{
	return *m_Stats.SetAt(dwObjID);
	}

void CObjectStats::ReadFromStream (SUniverseLoadCtx &Ctx)

//	ReadFromStream
//
//	Read from stream
//
//	DWORD			No of entries
//
//	[For each entry]
//	DWORD			dwObjID
//	DWORD			iPlayerMissionsGiven

	{
	int i;

	int iCount;
	Ctx.pStream->Read((char *)&iCount, sizeof(DWORD));
	for (i = 0; i < iCount; i++)
		{
		DWORD dwObjID;
		Ctx.pStream->Read((char *)&dwObjID, sizeof(DWORD));

		SEntry *pEntry = m_Stats.SetAt(dwObjID);
		Ctx.pStream->Read((char *)&pEntry->iPlayerMissionsGiven, sizeof(DWORD));
		}
	}

void CObjectStats::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write to stream

	{
	int i;
	DWORD dwSave;

	dwSave = m_Stats.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < m_Stats.GetCount(); i++)
		{
		dwSave = m_Stats.GetKey(i);
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		SEntry *pEntry = &m_Stats[i];
		pStream->Write((char *)&pEntry->iPlayerMissionsGiven, sizeof(DWORD));
		}
	}
