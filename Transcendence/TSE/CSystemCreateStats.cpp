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

void CSystemCreateStats::AddLabel (const CString &sAttributes)

//	AddLabel
//
//	Adds the attributes for the label

	{
	if (m_bPermute)
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
	//	Parse the first attribute

	char *pPos = sAttributes.GetASCIIZPointer();
	strParseWhitespace(pPos, &pPos);

	char *pStart = pPos;
	while (*pPos != ',' && *pPos != ';' && *pPos != ' ' && *pPos != '\0')
		pPos++;

	CString sAttrib(pStart, pPos - pStart);

	//	Remember the remainder so that we can recurse

	CString sRemainder;
	if (*pPos != '\0')
		{
		if (*pPos == ',' || *pPos == ';')
			pPos++;

		sRemainder = strTrimWhitespace(CString(pPos));
		}

	//	Combine with prefix and capitalize

	if (!sPrefix.IsBlank())
		sAttrib = strPatternSubst(CONSTLIT("%s,%s"), sPrefix, sAttrib);

	//	Add

	AddEntry(sAttrib);

	//	Recurse

	if (!sRemainder.IsBlank())
		{
		//	Add the remainder alone

		AddLabelExpansion(sRemainder, sPrefix);

		//	Add the remainder plus the attribute

		AddLabelExpansion(sRemainder, sAttrib);
		}
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
