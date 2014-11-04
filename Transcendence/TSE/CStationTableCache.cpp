//	CStationTableCache.cpp
//
//	CStationTableCache class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

void CStationTableCache::DeleteAll (void)

//	DeleteAll
//
//	Delete all entries

	{
	int i;

	for (i = 0; i < m_Cache.GetCount(); i++)
		delete m_Cache[i];

	m_Cache.DeleteAll();
	}

bool CStationTableCache::FindTable (const CString &sDesc, TArray<SEntry> **retpTable)

//	GetTable
//
//	If a table with the given description is found in the cache we return TRUE.
//	If not, we return FALSE.
//
//	Either way, retpTable is initialized with a pointer to a table to read 
//	or write.

	{
	if (m_Cache.Find(sDesc, retpTable))
		{
		m_iCacheHits++;
		return true;
		}

	m_iCacheMisses++;
	return false;
	}

int CStationTableCache::GetCacheHitRate (void) const

//	GetCacheHitRate
//
//	Returns the current hit rate percentage (0-100)

	{
	int iTotal = m_iCacheHits + m_iCacheMisses;
	return (iTotal > 0 ? (int)((100.0 * (double)m_iCacheHits / iTotal) + 0.5) : 100);
	}
