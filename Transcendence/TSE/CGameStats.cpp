//	CGameStats.cpp
//
//	CGameStats object

#include "PreComp.h"

const DWORD GAME_STAT_VERSION =				1;

void CGameStats::GetEntry (int iIndex, CString *retsStatName, CString *retsStatValue, CString *retsSection) const

//	GetEntry
//
//	Returns the given stat

	{
	if (retsStatName)
		*retsStatName = m_Stats[iIndex].sStatName;

	if (retsStatValue)
		*retsStatValue = m_Stats[iIndex].sStatValue;

	if (retsSection)
		{
		//	Look for the separator

		char *pStart = m_Stats[iIndex].sSortKey.GetASCIIZPointer();
		char *pPos = pStart;
		while (*pPos != '/' && *pPos != '\0')
			pPos++;

		if (*pPos == '/')
			*retsSection = CString(pStart, pPos - pStart);
		else
			*retsSection = NULL_STR;
		}
	}

void CGameStats::Insert (const CString &sStatName, const CString &sStatValue, const CString &sSection, const CString &sSortKey)

//	Insert
//
//	Insert a stat

	{
	ASSERT(!sStatName.IsBlank());

	SStat *pNew = m_Stats.Insert();
	pNew->sStatName = sStatName;
	pNew->sStatValue = sStatValue;

	//	We store the section name in the sort key (OK if sSection is blank)

	pNew->sSortKey = strPatternSubst(CONSTLIT("%s/%s"),
			sSection,
			(sSortKey.IsBlank() ? sStatName : sSortKey));
	}

ALERROR CGameStats::LoadFromStream (IReadStream *pStream)

//	LoadFromStream
//
//	DWORD		game stats version
//	CString		m_sDefaultSectionName
//	DWORD		No of entries
//	CString			sSortKey
//	CString			sStatName
//	CString			sStatValue

	{
	int i;
	DWORD dwLoad;

	DWORD dwVersion;
	pStream->Read((char *)&dwVersion, sizeof(DWORD));
	if (dwVersion > GAME_STAT_VERSION)
		return ERR_FAIL;

	DeleteAll();

	m_sDefaultSectionName.ReadFromStream(pStream);

	pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_Stats.InsertEmpty(dwLoad);

	for (i = 0; i < (int)dwLoad; i++)
		{
		m_Stats[i].sSortKey.ReadFromStream(pStream);
		m_Stats[i].sStatName.ReadFromStream(pStream);
		m_Stats[i].sStatValue.ReadFromStream(pStream);
		}

	return NOERROR;
	}

void CGameStats::ParseSortKey (const CString &sSortKey, CString *retsSection, CString *retsSectionSortKey) const

//	ParseSortKey
//
//	Separates sSortKey into a section and section sort key.

	{
	char *pPos = sSortKey.GetASCIIZPointer();

	char *pStart = pPos;
	while (*pPos != '/' && *pPos != '\0')
		pPos++;

	if (retsSection)
		*retsSection = CString(pStart, (int)(pPos - pStart));

	//	key

	if (*pPos == '/')
		pPos++;

	if (retsSectionSortKey)
		*retsSectionSortKey = CString(pPos);
	}

void CGameStats::SaveToJSON (CJSONValue *retOutput) const

//	SaveToJSON
//
//	Save as a JSON object. The output is an array of stats. Each stat is an 
//	array with the following elements:
//
//	1.	Name of stat
//	2.	Value of stat (or nulll)
//	3.	Section (or null)
//	4.	Sort key (or null)

	{
	int i;

	*retOutput = CJSONValue(CJSONValue::typeArray);

	for (i = 0; i < m_Stats.GetCount(); i++)
		{
		CJSONValue Stat(CJSONValue::typeArray);

		//	Add the stats name

		Stat.InsertHandoff(CJSONValue(m_Stats[i].sStatName));

		//	Parse the value

		int iValue;
		if (m_Stats[i].sStatValue.IsBlank())
			Stat.Insert(CJSONValue(CJSONValue::typeNull));
		else if (strIsInt(m_Stats[i].sStatValue, PARSE_THOUSAND_SEPARATOR, &iValue))
			Stat.InsertHandoff(CJSONValue(iValue));
		else
			Stat.InsertHandoff(CJSONValue(m_Stats[i].sStatValue));

		//	Split the section out of the sort key

		CString sSection;
		CString sSectionSortKey;
		ParseSortKey(m_Stats[i].sSortKey, &sSection, &sSectionSortKey);

		if (sSection.IsBlank())
			Stat.Insert(CJSONValue(CJSONValue::typeNull));
		else
			Stat.InsertHandoff(CJSONValue(sSection));

		//	Add the sort key

		if (!strEquals(sSectionSortKey, m_Stats[i].sStatName))
			Stat.InsertHandoff(CJSONValue(sSectionSortKey));

		//	Append to large array

		retOutput->InsertHandoff(Stat);
		}
	}

ALERROR CGameStats::SaveToStream (IWriteStream *pStream) const

//	SaveToStream
//
//	DWORD		game stats version
//	CString		m_sDefaultSectionName
//	DWORD		No of entries
//	CString			sSortKey
//	CString			sStatName
//	CString			sStatValue

	{
	int i;
	DWORD dwSave;

	dwSave = GAME_STAT_VERSION;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	m_sDefaultSectionName.WriteToStream(pStream);

	dwSave = m_Stats.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < m_Stats.GetCount(); i++)
		{
		m_Stats[i].sSortKey.WriteToStream(pStream);
		m_Stats[i].sStatName.WriteToStream(pStream);
		m_Stats[i].sStatValue.WriteToStream(pStream);
		}

	return NOERROR;
	}

void CGameStats::TakeHandoff (CGameStats &Source)

//	TakeHandoff
//
//	Takes a handoff of the underlying array

	{
	m_Stats.TakeHandoff(Source.m_Stats);
	m_sDefaultSectionName = Source.m_sDefaultSectionName;
	}

ALERROR CGameStats::WriteAsText (IWriteStream *pOutput) const

//	WriteAsText
//
//	Outputs game stats as text suitable for clipboard of text file

	{
	ALERROR error;
	int i;

	CString sPrevSection = CONSTLIT("(start)");
	for (i = 0; i < GetCount(); i++)
		{
		//	Get the stats

		CString sName;
		CString sValue;
		CString sSection;
		GetEntry(i, &sName, &sValue, &sSection);

		//	If necessary, add a new section header

		if (!strEquals(sSection, sPrevSection))
			{
			if (i != 0)
				if (error = pOutput->Write("\r\n", 2))
					return error;

			CString sSectionUpper = (sSection.IsBlank() ? ::strToUpper(GetDefaultSectionName()) : ::strToUpper(sSection));
			if (error = pOutput->Write(sSectionUpper.GetASCIIZPointer(), sSectionUpper.GetLength()))
				return error;

			if (error = pOutput->Write("\r\n\r\n", 4))
				return error;

			sPrevSection = sSection;
			}

		//	Add the two lines (with a tab separator)

		if (pOutput->Write(sName.GetASCIIZPointer(), sName.GetLength()))
			return error;
		if (pOutput->Write("\t", 1))
			return error;
		if (pOutput->Write(sValue.GetASCIIZPointer(), sValue.GetLength()))
			return error;
		if (pOutput->Write("\r\n", 2))
			return error;
		}

	return NOERROR;
	}
