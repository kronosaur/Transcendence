//	CStationEncounterCtx.cpp
//
//	CStationEncounterCtx class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

void CStationEncounterCtx::AddEncounter (int iLevel)

//	AddEncounter
//
//	Adds an encounter at the given level
	
	{
	m_Total.iCount++;

	if (iLevel >= 1 && iLevel <= MAX_TECH_LEVEL)
		{
		SEncounterStats *pCount = m_ByLevel.SetAt(iLevel);
		pCount->iCount++;
		}
	}

bool CStationEncounterCtx::CanBeEncountered (const CStationEncounterDesc &Desc)

//	CanBeEncountered
//
//	Returns TRUE if we can encounter this station type.

	{
	//	If not limits then we're OK

	if (m_Total.iLimit == -1)
		return true;

	//	Otherwise, only if we haven't exceeded the limit

	return (m_Total.iCount < m_Total.iLimit);
	}

bool CStationEncounterCtx::CanBeEncounteredInSystem (CSystem *pSystem, CStationType *pStationType, const CStationEncounterDesc &Desc)

//	CanBeEncounteredInSystem
//
//	Returns TRUE if we can encounter the given station type in the system.

	{
	if (pSystem)
		{
		if (Desc.IsUniqueInSystem() && pSystem->IsStationInSystem(pStationType))
			return false;

		//	Check for a level limit

		SEncounterStats *pCount = m_ByLevel.GetAt(pSystem->GetLevel());
		if (pCount && pCount->iLimit != -1 && pCount->iCount >= pCount->iLimit)
			return false;
		}

	return CanBeEncountered(Desc);
	}

int CStationEncounterCtx::GetFrequencyByLevel (int iLevel, const CStationEncounterDesc &Desc)

//	GetFrequencyByLevel
//
//	Returns the chance of an encounter at the given level.

	{
	if (!CanBeEncountered(Desc))
		return 0;

	return Desc.GetFrequencyByLevel(iLevel);
	}

int CStationEncounterCtx::GetFrequencyForNode (CTopologyNode *pNode, CStationType *pStation, const CStationEncounterDesc &Desc)

//	GetFrequencyForNode
//
//	Returns the chance that this station type will appear in the given node.

	{
	//	If we cannot be encountered at all, then we're done

	if (!CanBeEncountered(Desc))
		return 0;

	//	Check for a level limit

	SEncounterStats *pCount = m_ByLevel.GetAt(pNode->GetLevel());
	if (pCount && pCount->iLimit != -1 && pCount->iCount >= pCount->iLimit)
		return 0;

	//	Otherwise, let the descriptor figure out the chance

	return Desc.GetFrequencyByNode(pNode, pStation);
	}

int CStationEncounterCtx::GetFrequencyForSystem (CSystem *pSystem, CStationType *pStation, const CStationEncounterDesc &Desc)

//	GetFrequencyForSystem
//
//	Returns the chance that this station type will appear in the given system

	{
	//	If we cannot be encountered at all, then we're done

	if (!CanBeEncountered(Desc))
		return 0;

	//	If this station is unique in the system, see if there are other
	//	stations of this type in the system

	if (Desc.IsUniqueInSystem() && pSystem->IsStationInSystem(pStation))
		return 0;

	//	Check for a level limit

	SEncounterStats *pCount = m_ByLevel.GetAt(pSystem->GetLevel());
	if (pCount && pCount->iLimit != -1 && pCount->iCount >= pCount->iLimit)
		return 0;

	//	Otherwise, let the descriptor figure out the chance

	return Desc.GetFrequencyByNode(pSystem->GetTopology(), pStation);
	}

void CStationEncounterCtx::ReadFromStream (SUniverseLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads from a stream

	{
	int i;

	Ctx.pStream->Read((char *)&m_Total.iCount, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_Total.iLimit, sizeof(DWORD));

	int iCount;
	Ctx.pStream->Read((char *)&iCount, sizeof(DWORD));

	m_ByLevel.DeleteAll();
	for (i = 0; i < iCount; i++)
		{
		int iKey;
		Ctx.pStream->Read((char *)&iKey, sizeof(DWORD));

		SEncounterStats *pStat = m_ByLevel.SetAt(iKey);

		Ctx.pStream->Read((char *)&pStat->iCount, sizeof(DWORD));
		Ctx.pStream->Read((char *)&pStat->iLimit, sizeof(DWORD));
		}
	}

void CStationEncounterCtx::Reinit (const CStationEncounterDesc &Desc)

//	Reinit
//
//	Reinitialize

	{
	m_Total.iCount = 0;
	m_Total.iLimit = Desc.GetMaxAppearing();

	//	No level limits for now

	m_ByLevel.DeleteAll();
	}

void CStationEncounterCtx::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Writes to a stream
//
//	DWORD			m_Total.iCount
//	DWORD			m_Total.iLimit
//
//	DWORD			Number of level entries
//	For each level entry:
//	DWORD			Level
//	DWORD			iCount
//	DWORD			iLimit

	{
	int i;
	DWORD dwSave;

	pStream->Write((char *)&m_Total.iCount, sizeof(DWORD));
	pStream->Write((char *)&m_Total.iLimit, sizeof(DWORD));

	dwSave = m_ByLevel.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < m_ByLevel.GetCount(); i++)
		{
		dwSave = m_ByLevel.GetKey(i);
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		dwSave = m_ByLevel[i].iCount;
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		dwSave = m_ByLevel[i].iLimit;
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}
	}
