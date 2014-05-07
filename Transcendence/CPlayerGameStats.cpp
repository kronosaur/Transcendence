//	CPlayerGameStats.cpp
//
//	CPlayerGameStats class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

const DWORD INVALID_TIME = 0xffffffff;

#define BEST_ENEMY_SHIPS_DESTROYED_STATS		CONSTLIT("bestEnemyShipsDestroyed")
#define ENEMY_OBJS_DESTROYED_STAT				CONSTLIT("enemyObjsDestroyed")
#define ENEMY_SHIPS_DESTROYED_STAT				CONSTLIT("enemyShipsDestroyed")
#define ENEMY_STATIONS_DESTROYED_STAT			CONSTLIT("enemyStationsDestroyed")
#define FRIENDLY_OBJS_DESTROYED_STAT			CONSTLIT("friendlyObjsDestroyed")
#define FRIENDLY_SHIPS_DESTROYED_STAT			CONSTLIT("friendlyShipsDestroyed")
#define FRIENDLY_STATIONS_DESTROYED_STAT		CONSTLIT("friendlyStationsDestroyed")
#define ITEMS_BOUGHT_COUNT_STAT					CONSTLIT("itemsBoughtCount")
#define ITEMS_BOUGHT_VALUE_STAT					CONSTLIT("itemsBoughtValue")
#define ITEMS_SOLD_COUNT_STAT					CONSTLIT("itemsSoldCount")
#define ITEMS_SOLD_VALUE_STAT					CONSTLIT("itemsSoldValue")
#define MISSION_COMPLETED_STAT					CONSTLIT("missionCompleted")
#define MISSION_FAILURE_STAT					CONSTLIT("missionFailure")
#define MISSION_SUCCESS_STAT					CONSTLIT("missionSuccess")
#define OBJS_DESTROYED_STAT						CONSTLIT("objsDestroyed")
#define RESURRECT_COUNT_STAT					CONSTLIT("resurrectCount")
#define SCORE_STAT								CONSTLIT("score")
#define SYSTEM_DATA_STAT						CONSTLIT("systemData")
#define SYSTEMS_VISITED_STAT					CONSTLIT("systemsVisited")

#define NIL_VALUE								CONSTLIT("Nil")

#define STR_DESTROYED							CONSTLIT("destroyed")
#define STR_ENEMY_DESTROYED						CONSTLIT("enemyDestroyed")
#define STR_FRIEND_DESTROYED					CONSTLIT("friendDestroyed")
#define STR_MISSION_FAILURE						CONSTLIT("missionFailure")
#define STR_MISSION_SUCCESS						CONSTLIT("missionSuccess")
#define STR_SAVED								CONSTLIT("saved")

class CStatCounterArray
	{
	public:
		void DeleteAll (void) { m_Array.DeleteAll(); }
		void GenerateGameStats (CGameStats &Stats);
		void Insert (const CString &sStat, int iCount, const CString &sSection, const CString &sSort);

	private:
		struct SEntry
			{
			int iCount;
			CString sSort;
			};

		TMap<CString, TMap<CString, SEntry>> m_Array;
	};

void WriteTimeValue (CMemoryWriteStream &Output, DWORD dwTime);

CPlayerGameStats::CPlayerGameStats (void) : m_iScore(0),
		m_iResurrectCount(0),
		m_iExtraSystemsVisited(0),
		m_iExtraEnemyShipsDestroyed(0)

//	CPlayerGameStats constructor

	{
	}

bool CPlayerGameStats::AddMatchingKeyEvents (const CString &sNodeID, const CDesignTypeCriteria &Crit, TArray<SKeyEventStats> *pEventList, TArray<SKeyEventStatsResult> *retList) const

//	AddMatchingKeyEvents
//
//	Adds all of the matching events from pEventList to the result

	{
	int i;

	for (i = 0; i < pEventList->GetCount(); i++)
		{
		SKeyEventStats *pStats = &pEventList->GetAt(i);

		CDesignType *pType = g_pUniverse->FindDesignType(pStats->dwObjUNID);
		if (pType == NULL)
			continue;

		if (pType->MatchesCriteria(Crit))
			{
			SKeyEventStatsResult *pResult = retList->Insert();
			pResult->sNodeID = sNodeID;
			pResult->pStats = pStats;
			pResult->bMarked = false;
			}
		}

	return true;
	}

int CPlayerGameStats::CalcEndGameScore (void) const

//	CalcEndGameScore
//
//	Calculates the score if it were end game (by adjusting for
//	resurrection count

	{
	return m_iScore / (1 + min(9, m_iResurrectCount));
	}

void CPlayerGameStats::GenerateGameStats (CGameStats &Stats, CSpaceObject *pPlayerShip, bool bGameOver) const

//	GenerateGameStats
//
//	Generates a stats for everything we track

	{
	int j;

	CShip *pShip = (pPlayerShip ? pPlayerShip->AsShip() : NULL);
	if (pShip == NULL)
		return;

	CPlayerShipController *pPlayer = (CPlayerShipController *)pShip->GetController();
	if (pPlayer == NULL)
		return;

	CSovereign *pPlayerSovereign = g_pUniverse->FindSovereign(g_PlayerSovereignUNID);
	if (pPlayerSovereign == NULL)
		return;

	//	Base stats

	Stats.Insert(CONSTLIT("Genome"), strCapitalize(GetGenomeName(pPlayer->GetPlayerGenome())));
	Stats.Insert(CONSTLIT("Score"), strFormatInteger(CalcEndGameScore(), -1, FORMAT_THOUSAND_SEPARATOR | FORMAT_UNSIGNED));
	Stats.Insert(CONSTLIT("Ship class"), pShip->GetNounPhrase(0));

	CTimeSpan Time = GetPlayTime();
	if (!Time.IsBlank())
		Stats.Insert(CONSTLIT("Time played"), Time.Format(NULL_STR));

#ifdef REAL_TIME
	Time = GetGameTime();
	if (!Time.IsBlank())
		Stats.Insert(CONSTLIT("Time elapsed in game"), Time.Format(NULL_STR));
#endif

	//	Some combat stats

	CString sDestroyed = GetStat(ENEMY_SHIPS_DESTROYED_STAT);
	if (!sDestroyed.IsBlank())
		Stats.Insert(CONSTLIT("Enemy ships destroyed"), sDestroyed, CONSTLIT("combat"));

	sDestroyed = GetStat(FRIENDLY_SHIPS_DESTROYED_STAT);
	if (!sDestroyed.IsBlank())
		Stats.Insert(CONSTLIT("Friendly ships destroyed"), sDestroyed, CONSTLIT("combat"));

	sDestroyed = GetStat(ENEMY_STATIONS_DESTROYED_STAT);
	if (!sDestroyed.IsBlank())
		Stats.Insert(CONSTLIT("Enemy stations destroyed"), sDestroyed, CONSTLIT("combat"));

	sDestroyed = GetStat(FRIENDLY_STATIONS_DESTROYED_STAT);
	if (!sDestroyed.IsBlank())
		Stats.Insert(CONSTLIT("Friendly stations destroyed"), sDestroyed, CONSTLIT("combat"));

	//	Add stat for every station destroyed

	CStatCounterArray CounterArray;

	CMapIterator i;
	m_StationStats.Reset(i);
	while (m_StationStats.HasMore(i))
		{
		SStationTypeStats *pStats;
		DWORD dwUNID = m_StationStats.GetNext(i, &pStats);
		CStationType *pType = g_pUniverse->FindStationType(dwUNID);
		if (pType == NULL)
			continue;

		CString sName = pType->GetNounPhrase(0);
		CString sSort = strPatternSubst(CONSTLIT("%03d%s"), 100 - pType->GetLevel(), sName);

		if (pType->GetSovereign()->IsEnemy(pPlayerSovereign))
			CounterArray.Insert(sName, pStats->iDestroyed, CONSTLIT("Enemy stations destroyed"), sSort);
		else
			CounterArray.Insert(sName, pStats->iDestroyed, CONSTLIT("Friendly stations destroyed"), sSort);
		}

	CounterArray.GenerateGameStats(Stats);
		
	//	Add stat for every ship class destroyed

	CounterArray.DeleteAll();
	m_ShipStats.Reset(i);
	while (m_ShipStats.HasMore(i))
		{
		SShipClassStats *pStats;
		DWORD dwUNID = m_ShipStats.GetNext(i, &pStats);
		CShipClass *pClass = g_pUniverse->FindShipClass(dwUNID);
		if (pClass == NULL)
			continue;

		CString sName = pClass->GetNounPhrase(0);
		CString sSort = strPatternSubst(CONSTLIT("%09d%s"), 100000000 - pClass->GetScore(), sName);

		if (pStats->iEnemyDestroyed > 0)
			CounterArray.Insert(sName, pStats->iEnemyDestroyed, CONSTLIT("Enemy ships destroyed"), sSort);

		if (pStats->iFriendDestroyed > 0)
			CounterArray.Insert(sName, pStats->iFriendDestroyed, CONSTLIT("Friendly ships destroyed"), sSort);
		}

	CounterArray.GenerateGameStats(Stats);

	//	Add stat for every weapon fired

	m_ItemStats.Reset(i);
	while (m_ItemStats.HasMore(i))
		{
		SItemTypeStats *pStats;
		DWORD dwUNID = m_ItemStats.GetNext(i, &pStats);
		CItemType *pItemType = g_pUniverse->FindItemType(dwUNID);
		if (pItemType == NULL)
			continue;

		CString sName = pItemType->GetNounPhrase(nounShort);
		CString sSort = strPatternSubst(CONSTLIT("%03d%s"), 100 - pItemType->GetLevel(), sName);

		//	Installed items

		if (pStats->dwFirstInstalled != INVALID_TIME)
			Stats.Insert(sName, NULL_STR, CONSTLIT("Items installed"), sSort);

		if (pStats->iCountFired > 0)
			Stats.Insert(sName, 
					strFormatInteger(pStats->iCountFired, -1, FORMAT_THOUSAND_SEPARATOR | FORMAT_UNSIGNED), 
					CONSTLIT("Weapons fired"), 
					sSort);
		}

	//	Stats for player equipment (but only if the game is done)

	if (bGameOver)
		{
		TSortMap<CString, CItem> InstalledItems;

		//	First we generate a sorted list of installed items
		//	(We do this in case there are multiple of the same device/armor so that
		//	we can coalesce them together into a single line).

		CItemListManipulator ItemList(pShip->GetItemList());
		ItemList.ResetCursor();
		while (ItemList.MoveCursorForward())
			{
			const CItem &Item(ItemList.GetItemAtCursor());

			if (Item.IsInstalled())
				{
				CString sEnhancement = Item.GetEnhancedDesc(pShip);
				CString sItemName = Item.GetNounPhrase(nounActual | nounCountOnly | nounShort);
				CString sLine = (sEnhancement.IsBlank() ? sItemName : strPatternSubst(CONSTLIT("%s [%s]"), sItemName, sEnhancement));

				bool bInserted;
				CItem *pEntry = InstalledItems.SetAt(sLine, &bInserted);
				if (bInserted)
					{
					*pEntry = Item;
					pEntry->SetCount(1);
					}
				else
					pEntry->SetCount(pEntry->GetCount() + 1);
				}
			}

		//	Now add all the installed items to the stats

		for (j = 0; j < InstalledItems.GetCount(); j++)
			{
			//	Redo the line now that we know the proper count

			CString sEnhancement = InstalledItems[j].GetEnhancedDesc(pShip);
			CString sItemName = InstalledItems[j].GetNounPhrase(nounActual | nounCountOnly);
			CString sLine = (sEnhancement.IsBlank() ? sItemName : strPatternSubst(CONSTLIT("%s [%s]"), sItemName, sEnhancement));

			//	Compute the sort order

			int iOrder;
			switch (InstalledItems[j].GetType()->GetCategory())
				{
				case itemcatWeapon:
					iOrder = 0;
					break;

				case itemcatLauncher:
					iOrder = 1;
					break;

				case itemcatShields:
					iOrder = 2;
					break;

				case itemcatArmor:
					iOrder = 3;
					break;

				case itemcatReactor:
					iOrder = 4;
					break;

				case itemcatDrive:
					iOrder = 5;
					break;

				default:
					iOrder = 6;
					break;
				}

			CString sSort = strPatternSubst(CONSTLIT("%d%03d%s"), iOrder, 100 - InstalledItems[j].GetType()->GetLevel(), sLine);
			Stats.Insert(sLine, NULL_STR, CONSTLIT("Final equipment"), sSort);
			}

		//	Add the remaining items

		ItemList.ResetCursor();
		while (ItemList.MoveCursorForward())
			{
			const CItem &Item(ItemList.GetItemAtCursor());

			if (!Item.IsInstalled())
				{
				CString sEnhancement = Item.GetEnhancedDesc(pShip);
				CString sItemName = Item.GetNounPhrase(nounActual | nounCountOnly);
				CString sLine = (sEnhancement.IsBlank() ? sItemName : strPatternSubst(CONSTLIT("%s [%s]"), sItemName, sEnhancement));
				CString sSort = strPatternSubst(CONSTLIT("%03d%s"), 100 - Item.GetType()->GetLevel(), sLine);

				Stats.Insert(sLine, NULL_STR, CONSTLIT("Final items"), sSort);
				}
			}
		}
	}

CString CPlayerGameStats::GenerateKeyEventStat (TArray<SKeyEventStatsResult> &List) const

//	GenerateKeyEventStat
//
//	Generates a stat from the list of events

	{
	int i;

	CMemoryWriteStream Output;
	if (Output.Create() != NOERROR)
		return NIL_VALUE;

	Output.Write("'(", 2);

	for (i = 0; i < List.GetCount(); i++)
		{
		SKeyEventStatsResult *pResult = &List[i];

		if (pResult->bMarked)
			{
			Output.Write("(", 1);

			//	Type

			switch (pResult->pStats->iType)
				{
				case eventEnemyDestroyedByPlayer:
					Output.Write(STR_ENEMY_DESTROYED.GetASCIIZPointer(), STR_ENEMY_DESTROYED.GetLength());
					break;

				case eventFriendDestroyedByPlayer:
					Output.Write(STR_FRIEND_DESTROYED.GetASCIIZPointer(), STR_FRIEND_DESTROYED.GetLength());
					break;

				case eventMajorDestroyed:
					Output.Write(STR_DESTROYED.GetASCIIZPointer(), STR_DESTROYED.GetLength());
					break;

				case eventMissionFailure:
					Output.Write(STR_MISSION_FAILURE.GetASCIIZPointer(), STR_MISSION_FAILURE.GetLength());
					break;

				case eventMissionSuccess:
					Output.Write(STR_MISSION_SUCCESS.GetASCIIZPointer(), STR_MISSION_SUCCESS.GetLength());
					break;

				case eventSavedByPlayer:
					Output.Write(STR_SAVED.GetASCIIZPointer(), STR_SAVED.GetLength());
					break;

				default:
					Output.Write("?", 1);
					break;
				}

			//	sNodeID time, UNID, cause, name, flags

			CString sValue = strPatternSubst(" %s 0x%x 0x%x 0x%x %s 0x%x) ", 
					CCString::Print(pResult->sNodeID),
					pResult->pStats->dwTime, 
					pResult->pStats->dwObjUNID, 
					pResult->pStats->dwCauseUNID,
					CCString::Print(pResult->pStats->sObjName),
					pResult->pStats->dwObjNameFlags);
			Output.Write(sValue.GetASCIIZPointer(), sValue.GetLength());
			}
		}

	Output.Write(")", 1);

	return CString(Output.GetPointer(), Output.GetLength());
	}

int CPlayerGameStats::GetBestEnemyShipsDestroyed (DWORD *retdwUNID) const

//	GetBestEnemyShipDestroyed
//
//	Returns the number of enemy ships destroyed of the most powerful ship class

	{
	CMapIterator i;
	int iBestScore = 0;
	DWORD dwBestUNID = 0;
	SShipClassStats *pBest = NULL;

	m_ShipStats.Reset(i);
	while (m_ShipStats.HasMore(i))
		{
		SShipClassStats *pStats;
		DWORD dwUNID = m_ShipStats.GetNext(i, &pStats);
		CShipClass *pClass = g_pUniverse->FindShipClass(dwUNID);
		if (pClass)
			{
			int iScore = pClass->GetScore();
			if (iScore > iBestScore
					&& pStats->iEnemyDestroyed > 0)
				{
				dwBestUNID = dwUNID;
				iBestScore = iScore;
				pBest = pStats;
				}
			}
		}

	if (pBest == NULL)
		return 0;

	if (retdwUNID)
		*retdwUNID = dwBestUNID;

	return pBest->iEnemyDestroyed;
	}

CString CPlayerGameStats::GetItemStat (const CString &sStat, const CItemCriteria &Crit) const

//	GetItemStat
//
//	Returns the given stat

	{
	int j;

	struct SEntry
		{
		CItemType *pType;
		SItemTypeStats *pStats;
		};

	//	Generate a list of all item stats that match criteria

	CMapIterator i;
	TArray<SEntry> List;
	m_ItemStats.Reset(i);
	while (m_ItemStats.HasMore(i))
		{
		SItemTypeStats *pStats;
		DWORD dwUNID = m_ItemStats.GetNext(i, &pStats);
		CItemType *pType = g_pUniverse->FindItemType(dwUNID);
		if (pType == NULL)
			continue;

		CItem theItem(pType, 1);
		if (theItem.MatchesCriteria(Crit))
			{
			SEntry *pEntry = List.Insert();
			pEntry->pType = pType;
			pEntry->pStats = pStats;
			}
		}

	//	Handle each case separately

	if (strEquals(sStat, ITEMS_BOUGHT_COUNT_STAT))
		{
		int iTotalCount = 0;
		for (j = 0; j < List.GetCount(); j++)
			iTotalCount += List[j].pStats->iCountBought;
		return ::strFromInt(iTotalCount);
		}
	else if (strEquals(sStat, ITEMS_BOUGHT_VALUE_STAT))
		{
		CurrencyValue iTotal = 0;
		for (j = 0; j < List.GetCount(); j++)
			iTotal += List[j].pStats->iValueBought;
		return ::strFromInt((int)iTotal);
		}
	else if (strEquals(sStat, ITEMS_SOLD_COUNT_STAT))
		{
		int iTotalCount = 0;
		for (j = 0; j < List.GetCount(); j++)
			iTotalCount += List[j].pStats->iCountSold;
		return ::strFromInt(iTotalCount);
		}
	else if (strEquals(sStat, ITEMS_SOLD_VALUE_STAT))
		{
		CurrencyValue iTotal = 0;
		for (j = 0; j < List.GetCount(); j++)
			iTotal += List[j].pStats->iValueSold;
		return ::strFromInt((int)iTotal);
		}
	else
		return NULL_STR;
	}

CPlayerGameStats::SItemTypeStats *CPlayerGameStats::GetItemStats (DWORD dwUNID)

//	GetItemStats
//
//	Get the stats for item type

	{
	SItemTypeStats *pStats = m_ItemStats.Find(dwUNID);
	if (pStats == NULL)
		{
		pStats = m_ItemStats.Insert(dwUNID);
		pStats->iCountSold = 0;
		pStats->iValueSold = 0;
		pStats->iCountBought = 0;
		pStats->iValueBought = 0;

		pStats->iCountInstalled = 0;
		pStats->dwFirstInstalled = INVALID_TIME;
		pStats->dwLastInstalled = INVALID_TIME;
		pStats->dwLastUninstalled = INVALID_TIME;
		pStats->dwTotalInstalledTime = 0;

		pStats->iCountFired = 0;
		}

	return pStats;
	}

CString CPlayerGameStats::GetKeyEventStat (const CString &sStat, const CString &sNodeID, const CDesignTypeCriteria &Crit) const

//	GetKeyEventStat
//
//	Returns the given key event stat

	{
	int i;

	//	Get the list of stats

	TArray<SKeyEventStatsResult> List;
	if (!GetMatchingKeyEvents(sNodeID, Crit, &List))
		return NIL_VALUE;

	if (strEquals(sStat, OBJS_DESTROYED_STAT))
		{
		//	Mark the events that we're interested in

		for (i = 0; i < List.GetCount(); i++)
			List[i].bMarked = ((List[i].pStats->iType == eventEnemyDestroyedByPlayer) 
					|| (List[i].pStats->iType == eventFriendDestroyedByPlayer)
					|| (List[i].pStats->iType == eventMajorDestroyed));

		}

	else if (strEquals(sStat, MISSION_COMPLETED_STAT))
		{
		//	Mark the events that we're interested in

		for (i = 0; i < List.GetCount(); i++)
			List[i].bMarked = ((List[i].pStats->iType == eventMissionFailure) 
					|| (List[i].pStats->iType == eventMissionSuccess));
		}

	//	Otherwise we check for the type that we want

	else
		{
		EEventTypes iEvent;

		if (strEquals(sStat, ENEMY_OBJS_DESTROYED_STAT))
			iEvent = eventEnemyDestroyedByPlayer;
		else if (strEquals(sStat, FRIENDLY_OBJS_DESTROYED_STAT))
			iEvent = eventFriendDestroyedByPlayer;
		else if (strEquals(sStat, MISSION_FAILURE_STAT))
			iEvent = eventMissionFailure;
		else if (strEquals(sStat, MISSION_SUCCESS_STAT))
			iEvent = eventMissionSuccess;
		else
			return NIL_VALUE;

		//	Mark the events that we're interested in

		for (i = 0; i < List.GetCount(); i++)
			List[i].bMarked = (List[i].pStats->iType == iEvent);
		}

	//	Done

	return GenerateKeyEventStat(List);
	}

bool CPlayerGameStats::GetMatchingKeyEvents (const CString &sNodeID, const CDesignTypeCriteria &Crit, TArray<SKeyEventStatsResult> *retList) const

//	GetMatchingKeyEvents
//
//	Returns a list of all key events that match the criteria

	{
	//	If no nodeID then match all

	if (sNodeID.IsBlank())
		{
		CMapIterator i;

		m_KeyEventStats.Reset(i);
		while (m_KeyEventStats.HasMore(i))
			{
			TArray<SKeyEventStats> *pEventList;
			const CString &sNodeID = m_KeyEventStats.GetNext(i, &pEventList);

			//	Recurse

			if (!AddMatchingKeyEvents(sNodeID, Crit, pEventList, retList))
				return false;
			}
		}

	//	Otherwise look up the node ID

	else
		{
		TArray<SKeyEventStats> *pEventList = m_KeyEventStats.Find(sNodeID);
		if (pEventList == NULL)
			return true;

		if (!AddMatchingKeyEvents(sNodeID, Crit, pEventList, retList))
			return false;
		}

	//	Done

	return true;
	}

CPlayerGameStats::SShipClassStats *CPlayerGameStats::GetShipStats (DWORD dwUNID)

//	GetShipStats
//
//	Get the stats

	{
	SShipClassStats *pStats = m_ShipStats.Find(dwUNID);
	if (pStats == NULL)
		{
		pStats = m_ShipStats.Insert(dwUNID);
		pStats->iEnemyDestroyed = 0;
		pStats->iFriendDestroyed = 0;
		}

	return pStats;
	}

CString CPlayerGameStats::GetStat (const CString &sStat) const

//	GetStat
//
//	Returns the given stat

	{
	if (strEquals(sStat, BEST_ENEMY_SHIPS_DESTROYED_STATS))
		{
		DWORD dwUNID;
		int iCount = GetBestEnemyShipsDestroyed(&dwUNID);
		if (iCount == 0)
			return NULL_STR;

		return strPatternSubst(CONSTLIT("'(%d %d)"), dwUNID, iCount);
		}
	else if (strEquals(sStat, ENEMY_SHIPS_DESTROYED_STAT))
		{
		CMapIterator i;
		int iCount = 0;
		m_ShipStats.Reset(i);
		while (m_ShipStats.HasMore(i))
			{
			SShipClassStats *pStats;
			DWORD dwUNID = m_ShipStats.GetNext(i, &pStats);

			iCount += pStats->iEnemyDestroyed;
			}

		if (iCount + m_iExtraEnemyShipsDestroyed == 0)
			return NULL_STR;

		return ::strFromInt(iCount + m_iExtraEnemyShipsDestroyed);
		}
	else if (strEquals(sStat, ENEMY_STATIONS_DESTROYED_STAT))
		{
		CSovereign *pPlayerSovereign = g_pUniverse->FindSovereign(g_PlayerSovereignUNID);
		if (pPlayerSovereign == NULL)
			return NULL_STR;

		CMapIterator i;
		int iCount = 0;
		m_StationStats.Reset(i);
		while (m_StationStats.HasMore(i))
			{
			SStationTypeStats *pStats;
			DWORD dwUNID = m_StationStats.GetNext(i, &pStats);
			CStationType *pType = g_pUniverse->FindStationType(dwUNID);
			if (pType == NULL)
				continue;

			if (pType->GetSovereign()->IsEnemy(pPlayerSovereign))
				iCount += pStats->iDestroyed;
			}

		if (iCount == 0)
			return NULL_STR;

		return ::strFromInt(iCount);
		}
	else if (strEquals(sStat, FRIENDLY_SHIPS_DESTROYED_STAT))
		{
		CMapIterator i;
		int iCount = 0;
		m_ShipStats.Reset(i);
		while (m_ShipStats.HasMore(i))
			{
			SShipClassStats *pStats;
			DWORD dwUNID = m_ShipStats.GetNext(i, &pStats);

			iCount += pStats->iFriendDestroyed;
			}

		if (iCount == 0)
			return NULL_STR;

		return ::strFromInt(iCount);
		}
	else if (strEquals(sStat, FRIENDLY_STATIONS_DESTROYED_STAT))
		{
		CSovereign *pPlayerSovereign = g_pUniverse->FindSovereign(g_PlayerSovereignUNID);
		if (pPlayerSovereign == NULL)
			return NULL_STR;

		CMapIterator i;
		int iCount = 0;
		m_StationStats.Reset(i);
		while (m_StationStats.HasMore(i))
			{
			SStationTypeStats *pStats;
			DWORD dwUNID = m_StationStats.GetNext(i, &pStats);
			CStationType *pType = g_pUniverse->FindStationType(dwUNID);
			if (pType == NULL)
				continue;

			if (!pType->GetSovereign()->IsEnemy(pPlayerSovereign))
				iCount += pStats->iDestroyed;
			}

		if (iCount == 0)
			return NULL_STR;

		return ::strFromInt(iCount);
		}
	else if (strEquals(sStat, RESURRECT_COUNT_STAT))
		return ::strFromInt(m_iResurrectCount);
	else if (strEquals(sStat, SCORE_STAT))
		return ::strFromInt(m_iScore);
	else if (strEquals(sStat, SYSTEM_DATA_STAT))
		{
		CMemoryWriteStream Output;
		if (Output.Create() != NOERROR)
			return NIL_VALUE;

		Output.Write("'(", 2);

		CMapIterator i;
		m_SystemStats.Reset(i);
		while (m_SystemStats.HasMore(i))
			{
			SSystemStats *pStats;
			const CString &sNodeID = m_SystemStats.GetNext(i, &pStats);

			Output.Write("(", 1);
			Output.Write(sNodeID.GetASCIIZPointer(), sNodeID.GetLength());
			Output.Write(" ", 1);
			WriteTimeValue(Output, pStats->dwFirstEntered);
			Output.Write(" ", 1);
			WriteTimeValue(Output, pStats->dwLastEntered);
			Output.Write(" ", 1);
			WriteTimeValue(Output, pStats->dwLastLeft);
			Output.Write(" ", 1);
			WriteTimeValue(Output, pStats->dwTotalTime);
			Output.Write(") ", 2);
			}

		Output.Write(")", 1);

		return CString(Output.GetPointer(), Output.GetLength());
		}
	else if (strEquals(sStat, SYSTEMS_VISITED_STAT))
		{
		CMapIterator i;
		int iCount = 0;
		m_SystemStats.Reset(i);
		while (m_SystemStats.HasMore(i))
			{
			SSystemStats *pStats;
			const CString &sNodeID = m_SystemStats.GetNext(i, &pStats);

			if (pStats->dwLastEntered != INVALID_TIME)
				iCount++;
			}

		return ::strFromInt(iCount + m_iExtraSystemsVisited);
		}
	else
		return NULL_STR;
	}

CPlayerGameStats::SStationTypeStats *CPlayerGameStats::GetStationStats (DWORD dwUNID)

//	GetStationStats
//
//	Get station stats

	{
	SStationTypeStats *pStats = m_StationStats.Find(dwUNID);
	if (pStats == NULL)
		{
		pStats = m_StationStats.Insert(dwUNID);
		pStats->iDestroyed = 0;
		}

	return pStats;
	}

DWORD CPlayerGameStats::GetSystemEnteredTime (const CString &sNodeID)

//	GetSystemEnteredTime
//
//	Returns the tick on which the player last entered the system.
//	0xffffffff if the player has never entered it.

	{
	SSystemStats *pStats = m_SystemStats.Find(sNodeID);
	if (pStats == NULL)
		return INVALID_TIME;

	return pStats->dwLastEntered;
	}

CPlayerGameStats::SSystemStats *CPlayerGameStats::GetSystemStats (const CString &sNodeID)

//	GetSystemStats
//
//	Get system stats

	{
	SSystemStats *pStats = m_SystemStats.Find(sNodeID);
	if (pStats == NULL)
		{
		pStats = m_SystemStats.Insert(sNodeID);
		pStats->dwFirstEntered = INVALID_TIME;
		pStats->dwLastEntered = INVALID_TIME;
		pStats->dwLastLeft = INVALID_TIME;
		pStats->dwTotalTime = 0;
		}

	return pStats;
	}

int CPlayerGameStats::IncStat (const CString &sStat, int iInc)

//	IncStat
//
//	Increments the given stat (and returns the new value)

	{
	if (strEquals(sStat, SCORE_STAT))
		return (m_iScore += iInc);
	else
		return 0;
	}

void CPlayerGameStats::OnGameEnd (CSpaceObject *pPlayer)

//	OnGameEnd
//
//	Game over

	{
	//	Mark how long we spent playing the game

	m_GameTime = g_pUniverse->GetElapsedGameTime();
	m_PlayTime = g_pUniverse->StopGameTime();

	//	This counts as leaving the system

	OnSystemLeft(g_pUniverse->GetCurrentSystem());

	//	Set the final time for all installed items
	}

void CPlayerGameStats::OnItemBought (const CItem &Item, CurrencyValue iTotalPrice)

//	OnItemBought
//
//	Player bought an item

	{
	if (iTotalPrice <= 0)
		return;

	SItemTypeStats *pStats = GetItemStats(Item.GetType()->GetUNID());
	pStats->iCountBought += Item.GetCount();
	pStats->iValueBought += iTotalPrice;
	}

void CPlayerGameStats::OnItemFired (const CItem &Item)

//	OnItemFired
//
//	Player fired the item (weapon or missile)

	{
	SItemTypeStats *pStats = GetItemStats(Item.GetType()->GetUNID());
	pStats->iCountFired++;
	}

void CPlayerGameStats::OnItemInstalled (const CItem &Item)

//	OnItemInstalled
//
//	Player installed an item

	{
	SItemTypeStats *pStats = GetItemStats(Item.GetType()->GetUNID());

	if (pStats->iCountInstalled == 0)
		{
		pStats->dwLastInstalled = g_pUniverse->GetTicks();
		if (pStats->dwFirstInstalled == INVALID_TIME)
			pStats->dwFirstInstalled = pStats->dwLastInstalled;
		pStats->dwLastUninstalled = INVALID_TIME;
		}

	pStats->iCountInstalled++;
	}

void CPlayerGameStats::OnItemSold (const CItem &Item, CurrencyValue iTotalPrice)

//	OnItemSold
//
//	Player sold an item

	{
	if (iTotalPrice <= 0)
		return;

	SItemTypeStats *pStats = GetItemStats(Item.GetType()->GetUNID());
	pStats->iCountSold += Item.GetCount();
	pStats->iValueSold += iTotalPrice;
	}

void CPlayerGameStats::OnItemUninstalled (const CItem &Item)

//	OnItemUninstalled
//
//	Player uninstalled an item

	{
	SItemTypeStats *pStats = GetItemStats(Item.GetType()->GetUNID());
	ASSERT(pStats->iCountInstalled > 0);
	if (pStats->iCountInstalled <= 0)
		return;

	pStats->iCountInstalled--;

	if (pStats->iCountInstalled == 0)
		{
		ASSERT(pStats->dwLastInstalled != INVALID_TIME);

		pStats->dwLastUninstalled = g_pUniverse->GetTicks();
		pStats->dwTotalInstalledTime += (pStats->dwLastUninstalled - pStats->dwLastInstalled);
		}
	}

void CPlayerGameStats::OnKeyEvent (EEventTypes iType, CSpaceObject *pObj, DWORD dwCauseUNID)

//	OnKeyEvent
//
//	Adds a key event involving an object

	{
	ASSERT(pObj);

	CSystem *pSystem = pObj->GetSystem();
	if (pSystem == NULL)
		{
		pSystem = g_pUniverse->GetCurrentSystem();
		if (pSystem == NULL)
			return;
		}

	//	Get the NodeID where the event happened

	CTopologyNode *pNode = pSystem->GetTopology();
	if (pNode == NULL)
		return;

	const CString &sNodeID = pNode->GetID();

	//	Get the object's type

	CDesignType *pType = pObj->GetType();
	if (pType == NULL)
		return;

	//	Get the object's name

	DWORD dwNameFlags;
	CString sName = pObj->GetName(&dwNameFlags);
	
	//	If the object name is the same as the type name then we don't bother
	//	storing it in the event (to save memory)

	if (sName.IsBlank() || strEquals(sName, pType->GetTypeName()))
		{
		sName = NULL_STR;
		dwNameFlags = 0;
		}

	//	Look for the list of events for this NodeID

	TArray<SKeyEventStats> *pEventList = m_KeyEventStats.Set(sNodeID);
	SKeyEventStats *pStats = pEventList->Insert();
	pStats->iType = iType;
	pStats->dwTime = g_pUniverse->GetTicks();
	pStats->dwObjUNID = pType->GetUNID();
	pStats->sObjName = sName;
	pStats->dwObjNameFlags = dwNameFlags;
	pStats->dwCauseUNID = dwCauseUNID;
	}

void CPlayerGameStats::OnObjDestroyedByPlayer (const SDestroyCtx &Ctx, CSpaceObject *pPlayer)

//	OnDestroyedByPlayer
//
//	Object destroyed by player

	{
	bool bIsEnemy = Ctx.pObj->IsEnemy(pPlayer);

	//	Is this a ship?

	CShip *pShip;
	if (Ctx.pObj->GetCategory() == CSpaceObject::catShip && (pShip = Ctx.pObj->AsShip()))
		{
		CShipClass *pClass = pShip->GetClass();
		SShipClassStats *pStats = GetShipStats(pClass->GetUNID());

		if (bIsEnemy)
			{
			pStats->iEnemyDestroyed++;

			m_iScore += pClass->GetScore();
			}
		else
			pStats->iFriendDestroyed++;
		}

	//	Is this a station?

	else if (Ctx.pObj->GetCategory() == CSpaceObject::catStation)
		{
		if (Ctx.pObj->HasAttribute(CONSTLIT("populated")))
			{
			SStationTypeStats *pStats = GetStationStats(Ctx.pObj->GetType()->GetUNID());

			pStats->iDestroyed++;
			}
		}
	}

void CPlayerGameStats::OnSystemEntered (CSystem *pSystem, int *retiLastVisit)

//	OnSystemEntered
//
//	Player just entered the system

	{
	CTopologyNode *pNode = pSystem->GetTopology();
	if (pNode == NULL)
		{
		if (retiLastVisit)
			*retiLastVisit = -1;
		return;
		}

	SSystemStats *pStats = GetSystemStats(pNode->GetID());
	pStats->dwLastEntered = g_pUniverse->GetTicks();
	if (pStats->dwFirstEntered == INVALID_TIME)
		{
		pStats->dwFirstEntered = pStats->dwLastEntered;
		if (retiLastVisit)
			*retiLastVisit = -1;
		}
	else
		{
		if (retiLastVisit)
			*retiLastVisit = (pStats->dwLastLeft != INVALID_TIME ? pStats->dwLastLeft : -1);
		}

	pStats->dwLastLeft = INVALID_TIME;
	}

void CPlayerGameStats::OnSystemLeft (CSystem *pSystem)

//	OnSystemLeft
//
//	Player just left the system

	{
	CTopologyNode *pNode = pSystem->GetTopology();
	if (pNode == NULL)
		return;

	SSystemStats *pStats = GetSystemStats(pNode->GetID());
	ASSERT(pStats->dwLastEntered != INVALID_TIME);
	pStats->dwLastLeft = g_pUniverse->GetTicks();
	pStats->dwTotalTime += pStats->dwLastLeft - pStats->dwLastEntered;
	}

void CPlayerGameStats::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read from stream
//
//	DWORD		m_iScore
//	DWORD		m_iExtraSystemsVisited
//	DWORD		m_iExtraEnemyShipsDestroyed
//	CTimeSpan	m_PlayTime
//	CTimeSpan	m_GameTime
//
//	DWORD		Count of item types
//	DWORD			UNID
//	DWORD			iCountSold
//	LONGLONG		iValueSold
//	DWORD			iCountBought
//	LONGLONG		iValueBought
//	DWORD			iCountInstalled
//	DWORD			dwFirstInstalled
//	DWORD			dwLastInstalled
//	DWORD			dwLastUninstalled
//	DWORD			dwTotalInstalledTime
//	DWORD			iCountFired
//
//	DWORD		Count of ship classes
//	DWORD			UNID
//	DWORD			iEnemyDestroyed
//	DWORD			iFriendDestroyed
//
//	DWORD		Count of station types
//	DWORD			UNID
//	DWORD			iDestroyed
//
//	DWORD		Count of systems
//	CString			NodeID
//	DWORD			dwFirstEntered
//	DWORD			dwLastEntered
//	DWORD			dwLastLeft
//	DWORD			dwTotalTime
//
//	DWORD		Count of systems
//	CString			NodeID
//	DWORD			count of key events
//	DWORD				iType
//	DWORD				dwTime
//	DWORD				dwObjUNID
//	DWORD				dwCauseUNID
//	DWORD				dwObjNameFlags
//	CString				sObjName

	{
	int i, j;
	DWORD dwLoad, dwCount;

	//	Prior to version 49 we only saved:
	//
	//	DWORD		m_iScore
	//	DWORD		m_iSystemsVisited
	//	DWORD		m_iEnemiesDestroyed
	//	DWORD		m_pBestEnemyDestroyed (UNID)
	//	DWORD		m_iBestEnemyDestroyedCount

	if (Ctx.dwVersion < 49)
		{
		Ctx.pStream->Read((char *)&m_iScore, sizeof(DWORD));
		Ctx.pStream->Read((char *)&m_iExtraSystemsVisited, sizeof(DWORD));
		Ctx.pStream->Read((char *)&m_iExtraEnemyShipsDestroyed, sizeof(DWORD));

		//	Load the ship class and count of the best enemy

		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));

		if (dwLoad)
			{
			SShipClassStats *pShipStats = GetShipStats(dwLoad);
			pShipStats->iEnemyDestroyed += dwCount;

			//	We subtract from extra ships (because these ships are already
			//	included there)

			m_iExtraEnemyShipsDestroyed -= dwCount;
			ASSERT(m_iExtraEnemyShipsDestroyed >= 0);
			}

		return;
		}

	//	New version

	Ctx.pStream->Read((char *)&m_iScore, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iExtraSystemsVisited, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iExtraEnemyShipsDestroyed, sizeof(DWORD));
	if (Ctx.dwVersion >= 53)
		{
		Ctx.pStream->Read((char *)&m_PlayTime, sizeof(CTimeSpan));
		Ctx.pStream->Read((char *)&m_GameTime, sizeof(CTimeSpan));
		}

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		SItemTypeStats *pStats = m_ItemStats.Insert(dwLoad);

		Ctx.pStream->Read((char *)&pStats->iCountSold, sizeof(DWORD));
		Ctx.pStream->Read((char *)&pStats->iValueSold, sizeof(CurrencyValue));
		Ctx.pStream->Read((char *)&pStats->iCountBought, sizeof(DWORD));
		Ctx.pStream->Read((char *)&pStats->iValueBought, sizeof(CurrencyValue));
		Ctx.pStream->Read((char *)&pStats->iCountInstalled, sizeof(DWORD));
		Ctx.pStream->Read((char *)&pStats->dwFirstInstalled, sizeof(DWORD));
		Ctx.pStream->Read((char *)&pStats->dwLastInstalled, sizeof(DWORD));
		Ctx.pStream->Read((char *)&pStats->dwLastUninstalled, sizeof(DWORD));
		Ctx.pStream->Read((char *)&pStats->dwTotalInstalledTime, sizeof(DWORD));
		Ctx.pStream->Read((char *)&pStats->iCountFired, sizeof(DWORD));
		}

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		SShipClassStats *pStats = m_ShipStats.Insert(dwLoad);

		Ctx.pStream->Read((char *)&pStats->iEnemyDestroyed, sizeof(DWORD));
		Ctx.pStream->Read((char *)&pStats->iFriendDestroyed, sizeof(DWORD));
		}

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		SStationTypeStats *pStats = m_StationStats.Insert(dwLoad);

		Ctx.pStream->Read((char *)&pStats->iDestroyed, sizeof(DWORD));
		}

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		CString sNodeID;
		sNodeID.ReadFromStream(Ctx.pStream);
		SSystemStats *pStats = m_SystemStats.Insert(sNodeID);

		Ctx.pStream->Read((char *)&pStats->dwFirstEntered, sizeof(DWORD));
		Ctx.pStream->Read((char *)&pStats->dwLastEntered, sizeof(DWORD));
		Ctx.pStream->Read((char *)&pStats->dwLastLeft, sizeof(DWORD));
		Ctx.pStream->Read((char *)&pStats->dwTotalTime, sizeof(DWORD));
		}

	//	Read the m_KeyEventStats

	if (Ctx.dwVersion >= 74)
		{
		Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
		for (i = 0; i < (int)dwCount; i++)
			{
			CString sNodeID;
			sNodeID.ReadFromStream(Ctx.pStream);
			TArray<SKeyEventStats> *pEventList = m_KeyEventStats.Insert(sNodeID);

			DWORD dwListCount;
			Ctx.pStream->Read((char *)&dwListCount, sizeof(DWORD));
			for (j = 0; j < (int)dwListCount; j++)
				{
				SKeyEventStats *pStats = pEventList->Insert();

				Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
				pStats->iType = (EEventTypes)dwLoad;

				Ctx.pStream->Read((char *)&pStats->dwTime, sizeof(DWORD));
				Ctx.pStream->Read((char *)&pStats->dwObjUNID, sizeof(DWORD));
				Ctx.pStream->Read((char *)&pStats->dwCauseUNID, sizeof(DWORD));
				Ctx.pStream->Read((char *)&pStats->dwObjNameFlags, sizeof(DWORD));
				pStats->sObjName.ReadFromStream(Ctx.pStream);
				}
			}
		}
	}

void CPlayerGameStats::SetStat (const CString &sStat, const CString &sValue)

//	SetStat
//
//	Set the stat value

	{
	if (strEquals(sStat, RESURRECT_COUNT_STAT))
		{
		bool bFailed;
		int iValue = strToInt(sValue, 0, &bFailed);
		if (!bFailed)
			m_iResurrectCount = iValue;
		}
	else if (strEquals(sStat, SCORE_STAT))
		{
		bool bFailed;
		int iValue = strToInt(sValue, 0, &bFailed);
		if (!bFailed)
			m_iScore = iValue;
		}
	}

void CPlayerGameStats::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write to stream
//
//	DWORD		m_iScore
//	DWORD		m_iExtraSystemsVisited
//	DWORD		m_iExtraEnemyShipsDestroyed
//	CTimeSpan	m_PlayTime
//	CTimeSpan	m_GameTime
//
//	DWORD		Count of item types
//	DWORD			UNID
//	DWORD			iCountSold
//	LONGLONG		iValueSold
//	DWORD			iCountBought
//	LONGLONG		iValueBought
//	DWORD			iCountInstalled
//	DWORD			dwFirstInstalled
//	DWORD			dwLastInstalled
//	DWORD			dwLastUninstalled
//	DWORD			dwTotalInstalledTime
//	DWORD			iCountFired
//
//	DWORD		Count of ship classes
//	DWORD			UNID
//	DWORD			iEnemyDestroyed
//	DWORD			iFriendDestroyed
//
//	DWORD		Count of station types
//	DWORD			UNID
//	DWORD			iDestroyed
//
//	DWORD		Count of systems
//	CString			NodeID
//	DWORD			dwFirstEntered
//	DWORD			dwLastEntered
//	DWORD			dwLastLeft
//	DWORD			dwTotalTime
//
//	DWORD		Count of systems
//	CString			NodeID
//	DWORD			count of key events
//	DWORD				iType
//	DWORD				dwTime
//	DWORD				dwObjUNID
//	DWORD				dwCauseUNID
//	DWORD				dwObjNameFlags
//	CString				sObjName

	{
	DWORD dwSave;
	CMapIterator i;
	int j;

	pStream->Write((char *)&m_iScore, sizeof(DWORD));
	pStream->Write((char *)&m_iExtraSystemsVisited, sizeof(DWORD));
	pStream->Write((char *)&m_iExtraEnemyShipsDestroyed, sizeof(DWORD));
	pStream->Write((char *)&m_PlayTime, sizeof(CTimeSpan));
	pStream->Write((char *)&m_GameTime, sizeof(CTimeSpan));

	dwSave = m_ItemStats.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	m_ItemStats.Reset(i);
	while (m_ItemStats.HasMore(i))
		{
		SItemTypeStats *pStats;
		DWORD dwUNID = m_ItemStats.GetNext(i, &pStats);

		pStream->Write((char *)&dwUNID, sizeof(DWORD));
		pStream->Write((char *)&pStats->iCountSold, sizeof(DWORD));
		pStream->Write((char *)&pStats->iValueSold, sizeof(CurrencyValue));
		pStream->Write((char *)&pStats->iCountBought, sizeof(DWORD));
		pStream->Write((char *)&pStats->iValueBought, sizeof(CurrencyValue));
		pStream->Write((char *)&pStats->iCountInstalled, sizeof(DWORD));
		pStream->Write((char *)&pStats->dwFirstInstalled, sizeof(DWORD));
		pStream->Write((char *)&pStats->dwLastInstalled, sizeof(DWORD));
		pStream->Write((char *)&pStats->dwLastUninstalled, sizeof(DWORD));
		pStream->Write((char *)&pStats->dwTotalInstalledTime, sizeof(DWORD));
		pStream->Write((char *)&pStats->iCountFired, sizeof(DWORD));
		}

	dwSave = m_ShipStats.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	m_ShipStats.Reset(i);
	while (m_ShipStats.HasMore(i))
		{
		SShipClassStats *pStats;
		DWORD dwUNID = m_ShipStats.GetNext(i, &pStats);

		pStream->Write((char *)&dwUNID, sizeof(DWORD));
		pStream->Write((char *)&pStats->iEnemyDestroyed, sizeof(DWORD));
		pStream->Write((char *)&pStats->iFriendDestroyed, sizeof(DWORD));
		}

	dwSave = m_StationStats.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	m_StationStats.Reset(i);
	while (m_StationStats.HasMore(i))
		{
		SStationTypeStats *pStats;
		DWORD dwUNID = m_StationStats.GetNext(i, &pStats);

		pStream->Write((char *)&dwUNID, sizeof(DWORD));
		pStream->Write((char *)&pStats->iDestroyed, sizeof(DWORD));
		}

	dwSave = m_SystemStats.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	m_SystemStats.Reset(i);
	while (m_SystemStats.HasMore(i))
		{
		SSystemStats *pStats;
		const CString &sNodeID = m_SystemStats.GetNext(i, &pStats);

		sNodeID.WriteToStream(pStream);
		pStream->Write((char *)&pStats->dwFirstEntered, sizeof(DWORD));
		pStream->Write((char *)&pStats->dwLastEntered, sizeof(DWORD));
		pStream->Write((char *)&pStats->dwLastLeft, sizeof(DWORD));
		pStream->Write((char *)&pStats->dwTotalTime, sizeof(DWORD));
		}

	dwSave = m_KeyEventStats.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	m_KeyEventStats.Reset(i);
	while (m_KeyEventStats.HasMore(i))
		{
		TArray<SKeyEventStats> *pEventList;
		const CString &sNodeID = m_KeyEventStats.GetNext(i, &pEventList);

		sNodeID.WriteToStream(pStream);

		DWORD dwCount = pEventList->GetCount();
		pStream->Write((char *)&dwCount, sizeof(DWORD));
		for (j = 0; j < (int)dwCount; j++)
			{
			SKeyEventStats *pStats = &pEventList->GetAt(j);

			dwSave = (DWORD)pStats->iType;
			pStream->Write((char *)&dwSave, sizeof(DWORD));

			pStream->Write((char *)&pStats->dwTime, sizeof(DWORD));
			pStream->Write((char *)&pStats->dwObjUNID, sizeof(DWORD));
			pStream->Write((char *)&pStats->dwCauseUNID, sizeof(DWORD));
			pStream->Write((char *)&pStats->dwObjNameFlags, sizeof(DWORD));
			pStats->sObjName.WriteToStream(pStream);
			}
		}
	}

//	CStatCounterArray ---------------------------------------------------------

void CStatCounterArray::GenerateGameStats (CGameStats &Stats)

//	GenerateGameStats
//
//	Adds the list of stats to the target

	{
	//	Loop over all sections

	CMapIterator i;
	m_Array.Reset(i);
	while (m_Array.HasMore(i))
		{
		TMap<CString, SEntry> *pSection;
		CString sSection = m_Array.GetNext(i, &pSection);

		//	Loop over all stats

		CMapIterator j;
		pSection->Reset(j);
		while (pSection->HasMore(j))
			{
			SEntry *pEntry;
			CString sStat = pSection->GetNext(j, &pEntry);

			//	Add it

			Stats.Insert(sStat, 
					::strFormatInteger(pEntry->iCount, -1, FORMAT_THOUSAND_SEPARATOR | FORMAT_UNSIGNED), 
					sSection, 
					pEntry->sSort);
			}
		}
	}

void CStatCounterArray::Insert (const CString &sStat, int iCount, const CString &sSection, const CString &sSort)

//	Insert
//
//	Adds the given stat. If the stat already exists (by name) the counts are added

	{
	if (iCount == 0)
		return;

	ASSERT(!sStat.IsBlank());
	ASSERT(!sSection.IsBlank());

	//	First look for the section

	TMap<CString, SEntry> *pSection = m_Array.Find(sSection);
	if (pSection == NULL)
		pSection = m_Array.Insert(sSection);

	//	Next look for the entry

	SEntry *pEntry = pSection->Find(sStat);
	if (pEntry == NULL)
		{
		pEntry = pSection->Insert(sStat);
		pEntry->iCount = iCount;
		pEntry->sSort = sSort;
		}
	else
		{
		if (iCount > pEntry->iCount)
			pEntry->sSort = sSort;

		pEntry->iCount += iCount;
		}
	}

void WriteTimeValue (CMemoryWriteStream &Output, DWORD dwTime)
	{
	if (dwTime == INVALID_TIME)
		Output.Write(NIL_VALUE.GetASCIIZPointer(), NIL_VALUE.GetLength());
	else
		{
		CString sInt = strFromInt(dwTime);
		Output.Write(sInt.GetASCIIZPointer(), sInt.GetLength());
		}
	}

