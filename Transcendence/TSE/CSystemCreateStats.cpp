//	CSystemCreateStats.cpp
//
//	CSystemCreateStats class

#include "PreComp.h"

CSystemCreateStats::CSystemCreateStats (void) :
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

void CSystemCreateStats::AddLabel (const CString &sAttributes)

//	AddLabel
//
//	Adds the attributes for the label

	{
	AddLabelExpansion(sAttributes);
	m_iLabelCount++;
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
	while (*pPos != ',' && *pPos != ' ' && *pPos != '\0')
		pPos++;

	CString sAttrib(pStart, pPos - pStart);

	//	Remember the remainder so that we can recurse

	CString sRemainder;
	if (*pPos != '\0')
		{
		if (*pPos == ',')
			pPos++;

		sRemainder = strTrimWhitespace(CString(pPos));
		}

	//	Combine with prefix and capitalize

	if (!sPrefix.IsBlank())
		sAttrib = strPatternSubst(CONSTLIT("%s,%s"), sPrefix, sAttrib);
	CString sAttribCap = strToUpper(sAttrib);

	//	Find the entry

	SLabelAttributeEntry *pEntry;
	if (m_LabelAttributeCounts.Lookup(sAttribCap, (CObject **)&pEntry) != NOERROR)
		{
		pEntry = new SLabelAttributeEntry;
		pEntry->iCount = 0;
		pEntry->sAttributes = sAttrib;

		m_LabelAttributeCounts.AddEntry(sAttribCap, (CObject *)pEntry);
		}

	//	Increment the count

	pEntry->iCount++;

	//	Recurse

	if (!sRemainder.IsBlank())
		{
		//	Add the remainder alone

		AddLabelExpansion(sRemainder, sPrefix);

		//	Add the remainder plus the attribute

		AddLabelExpansion(sRemainder, sAttrib);
		}
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
