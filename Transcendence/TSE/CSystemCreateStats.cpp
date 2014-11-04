//	CSystemCreateStats.cpp
//
//	CSystemCreateStats class

#include "PreComp.h"

CSystemCreateStats::CSystemCreateStats (void) :
		m_bPermute(false),
		m_LabelAttributeCounts(FALSE, TRUE),
		m_iLabelCount(0)

//	CSystemCreateStats constructor

	{
	}

CSystemCreateStats::~CSystemCreateStats (void)

//	CSystemCreateStats destructor

	{
	int i;

	for (i = 0; i < m_LabelAttributeCounts.GetCount(); i++)
		{
		SLabelAttributeEntry *pEntry = (SLabelAttributeEntry *)m_LabelAttributeCounts.GetValue(i);
		delete pEntry;
		}
	}

void CSystemCreateStats::AddEntry (const CString &sAttributes)

//	AddEntry
//
//	Adds this attribute set

	{
	CString sAttribCap = strToUpper(sAttributes);

	//	Find the entry

	SLabelAttributeEntry *pEntry;
	if (m_LabelAttributeCounts.Lookup(sAttribCap, (CObject **)&pEntry) != NOERROR)
		{
		pEntry = new SLabelAttributeEntry;
		pEntry->iCount = 0;
		pEntry->sAttributes = sAttributes;

		m_LabelAttributeCounts.AddEntry(sAttribCap, (CObject *)pEntry);
		}

	//	Increment the count

	pEntry->iCount++;
	}

void CSystemCreateStats::AddEntryPermutations (const CString &sPrefix, const TArray<CString> &Attribs, int iPos)

//	AddEntryPermutations
//
//	Adds permutions

	{
	//	If nothing, then nothing

	if (iPos >= Attribs.GetCount())
		return;

	//	Get the base case

	CString sNewPrefix;
	if (!sPrefix.IsBlank())
		sNewPrefix = strPatternSubst(CONSTLIT("%s,%s"), sPrefix, Attribs[iPos]);
	else
		sNewPrefix = Attribs[iPos];

	AddEntry(sNewPrefix);

	//	Permute with the base case

	AddEntryPermutations(sNewPrefix, Attribs, iPos + 1);

	//	Permute the remainder

	AddEntryPermutations(sPrefix, Attribs, iPos + 1);
	}

void CSystemCreateStats::AddFillLocationsTable (CSystem *pSystem, const TProbabilityTable<int> &LocationTable, const CString &sStationCriteria)

//	AddFillLocationsTable
//
//	Adds stats about <FillLocations>

	{
	int i, j;

	if (LocationTable.GetCount() == 0)
		return;

	SFillLocationsTable *pEntry = m_FillLocationsTables.Insert();
	pEntry->iLevel = pSystem->GetLevel();
	pEntry->sSystemName = pSystem->GetName();
	pEntry->pSystemType = pSystem->GetType();
	pEntry->sStationCriteria = sStationCriteria;

	//	Parse station criteria if we've got it.
	//	NOTE: For now we only do enemies.

	CString sEnemyStationCriteria = strPatternSubst(CONSTLIT("%s,%s"), sStationCriteria, CONSTLIT("*enemy"));
	CAttributeCriteria StationCriteria;
	StationCriteria.Parse(sEnemyStationCriteria, 0);

	//	Start by generating a base probability for all station encounters 
	//	relative to the system.

	TProbabilityTable<CStationType *> BaseProb;
	for (i = 0; i < g_pUniverse->GetStationTypeCount(); i++)
		{
		CStationType *pType = g_pUniverse->GetStationType(i);
		int iBaseChance = StationCriteria.AdjStationWeight(pType, ((1000 / ftCommon) * pType->GetFrequencyForSystem(pSystem)));
		if (iBaseChance > 0)
			{
			CAttributeCriteria LocationCriteria;
			LocationCriteria.Parse(pType->GetLocationCriteria(), 0);

			//	Average out our chance of ending up at one of the given locations.

			int iTotal = 0;
			for (j = 0; j < LocationTable.GetCount(); j++)
				{
				int iLocID = LocationTable[j];
				CLocationDef *pLoc = pSystem->GetLocation(iLocID);

				iTotal += LocationCriteria.AdjLocationWeight(pSystem, pLoc);
				}

			int iAverageChance = iTotal / LocationTable.GetCount();

			//	Now adjust the base chance

			int iChance = iBaseChance * iAverageChance / 1000;
			if (iChance <= 0)
				continue;

			//	Add it to our table.

			pEntry->Table.Insert(pType, iChance);
			}
		}
	}

void CSystemCreateStats::AddLabel (const CString &sAttributes)

//	AddLabel
//
//	Adds the attributes for the label

	{
#ifdef DEBUG
	if (strFind(sAttributes, CONSTLIT("outerSystem")) != -1
			&& strFind(sAttributes, CONSTLIT("innerSystem")) != -1)
		g_pUniverse->DebugOutput("ERROR: %s", sAttributes.GetASCIIZPointer());
#endif

	if (m_PermuteAttribs.GetCount() > 0)
		AddLabelExpansion(sAttributes);
	else
		AddLabelAttributes(sAttributes);

	m_iLabelCount++;
	}

void CSystemCreateStats::AddLabelAttributes (const CString &sAttributes)

//	AddLabelAttributes
//
//	Add each of the attributes alone.

	{
	char *pPos = sAttributes.GetASCIIZPointer();
	char *pStart = NULL;
	while (true)
		{
		if (pStart == NULL)
			{
			if (*pPos == '\0')
				break;
			else if (*pPos == ',' || *pPos == ';' || *pPos == ' ')
				pPos++;
			else
				{
				pStart = pPos;
				pPos++;
				}
			}
		else
			{
			if (*pPos == '\0' || *pPos == ',' || *pPos == ';' || *pPos == ' ')
				{
				CString sAttrib = CString(pStart, (int)(pPos - pStart));
				AddEntry(sAttrib);

				pStart = NULL;
				if (*pPos == '\0')
					break;
				else
					pPos++;
				}
			else
				pPos++;
			}
		}
	}

void CSystemCreateStats::AddLabelExpansion (const CString &sAttributes, const CString &sPrefix)

//	AddLabelExpansion
//
//	Expands and adds the given attributes to the label counter

	{
	int i;

	TArray<CString> Attribs;
	ParseAttributes(sAttributes, &Attribs);

	//	Add each of the attributes alone (and make a list of permutations)

	TArray<CString> Permutable;
	for (i = 0; i < Attribs.GetCount(); i++)
		{
		if (m_PermuteAttribs.Find(Attribs[i]))
			Permutable.Insert(Attribs[i]);
		else
			AddEntry(Attribs[i]);
		}

	//	Now add all permutations

	if (Permutable.GetCount() >= 1)
		AddEntryPermutations(NULL_STR, Permutable, 0);
	}

void CSystemCreateStats::AddStationTable (CSystem *pSystem, const CString &sStationCriteria, const CString &sLocationAttribs, TArray<CStationTableCache::SEntry> &Table)

//	AddStationTable
//
//	Adds the station table.

	{
	int i;

	//	See if we already have an entry for this table.
	//	If we don't we add it.

	SEncounterTable *pEntry;
	if (!FindEncounterTable(Table, &pEntry))
		{
		pEntry = m_EncounterTables.Insert();
		pEntry->iLevel = pSystem->GetLevel();
		pEntry->pSystemType = pSystem->GetType();
		pEntry->sStationCriteria = sStationCriteria;
		pEntry->iCount = 1;

		ParseAttributes(sLocationAttribs, &pEntry->LabelAttribs);

		pEntry->bHasStation = false;
		for (i = 0; i < Table.GetCount(); i++)
			{
			pEntry->Table.Insert(Table[i].pType, Table[i].iChance);
			if (Table[i].pType->GetScale() == scaleStructure
					|| Table[i].pType->GetScale() == scaleShip)
				pEntry->bHasStation = true;
			}

		return;
		}

	//	If we already have the table we need to aggregate the entry. We start
	//	by incrementing the count.

	pEntry->iCount++;

	//	Next we remove any location/label attributes that are not common to 
	//	both tables. [We assume that if we have two identical tables then only
	//	the attributes in common count to make the table unique.]

	TArray<CString> NewAttribs;
	ParseAttributes(sLocationAttribs, &NewAttribs);
	for (i = 0; i < pEntry->LabelAttribs.GetCount(); i++)
		{
		if (!NewAttribs.Find(pEntry->LabelAttribs[i]))
			{
			pEntry->LabelAttribs.Delete(i);
			i--;
			}
		}
	}

bool CSystemCreateStats::FindEncounterTable (TArray<CStationTableCache::SEntry> &Src, SEncounterTable **retpTable) const

//	FindEncounterTable
//
//	Looks for an encounter table that matches the source. If we find one, we
//	return a pointer to the table. Otherwise we return FALSE.

	{
	int i, j;

	for (i = 0; i < m_EncounterTables.GetCount(); i++)
		{
		const TProbabilityTable<CStationType *> &Table = m_EncounterTables[i].Table;
		
		//	If we have different counts, then we're different

		if (Table.GetCount() != Src.GetCount())
			continue;

		//	Compare each entry

		bool bMatches = true;
		for (j = 0; j < Src.GetCount(); j++)
			{
			if (Src[j].pType != Table.GetAt(j)
					|| Src[j].iChance != Table.GetChance(j))
				{
				bMatches = false;
				break;
				}
			}

		//	If we match, then we found a table.

		if (bMatches)
			{
			if (retpTable)
				*retpTable = &m_EncounterTables[i];
			return true;
			}
		}

	//	Otherwise, we did not find it.

	return false;
	}

void CSystemCreateStats::GetLabelAttributes (int iIndex, CString *retsAttribs, int *retiCount)

//	GetLabelAttributes
//
//	Returns the label attributes

	{
	SLabelAttributeEntry *pEntry = (SLabelAttributeEntry *)m_LabelAttributeCounts.GetValue(iIndex);
	if (retsAttribs)
		*retsAttribs = pEntry->sAttributes;
	if (retiCount)
		*retiCount = pEntry->iCount;
	}
