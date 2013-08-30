//	CAttributeCriteria.cpp
//
//	CAttributeCrtieria class
//	Copryight (c) 2011 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

#define MATCH_ALL						CONSTLIT("*")

inline bool IsWeightChar (char *pPos) { return (*pPos == '+' || *pPos == '-' || *pPos == '*' || *pPos == '!'); }
inline bool IsDelimiterChar (char *pPos, bool bIsSpecialAttrib = false) { return (*pPos == '\0' || *pPos == ',' || *pPos == ';' || (!bIsSpecialAttrib && strIsWhitespace(pPos))); }

const CString &CAttributeCriteria::GetAttribAndRequired (int iIndex, bool *retbRequired) const

//	GetAttribAndRequired
//
//	Returns the attrib and whether or not it is required

	{
	if (retbRequired)
		*retbRequired = (m_Attribs[iIndex].iWeight >= 0);

	return m_Attribs[iIndex].sAttrib;
	}

const CString &CAttributeCriteria::GetAttribAndWeight (int iIndex, int *retiWeight, bool *retbIsSpecial) const

//	GetAttribAndWeight
//
//	Returns the attrib and its weight

	{
	const SEntry &Entry = m_Attribs[iIndex];

	if (retiWeight)
		*retiWeight = Entry.iWeight;

	if (retbIsSpecial)
		*retbIsSpecial = Entry.bIsSpecial;

	return Entry.sAttrib;
	}

ALERROR CAttributeCriteria::Parse (const CString &sCriteria, DWORD dwFlags, CString *retsError)

//	Parse
//
//	Parses the criteria

	{
	m_Attribs.DeleteAll();
	m_dwFlags = dwFlags;

	//	If we match all, then we have no individual criteria

	if (strEquals(sCriteria, MATCH_ALL))
		return NOERROR;

	//	Parse

	char *pPos = sCriteria.GetASCIIZPointer();
	while (*pPos != '\0')
		{
		if (IsWeightChar(pPos))
			{
			//	Parse the weight

			int iMatchWeight = 0;
			while (IsWeightChar(pPos))
				{
				switch (*pPos)
					{
					case '*':
						iMatchWeight = critRequired;
						break;

					case '!':
						iMatchWeight = critExcluded;
						break;

					case '+':
						if (iMatchWeight < critSeek3)
							iMatchWeight++;
						break;

					case '-':
						if (iMatchWeight > critAvoid3)
							iMatchWeight--;
						break;
					}

				pPos++;
				}

			//	Whitespace or empty attributes are invalid here.

			if (IsDelimiterChar(pPos))
				{
				if (retsError)
					*retsError = strPatternSubst(CONSTLIT("Invalid criteria: \"%s\""), sCriteria);

				return ERR_FAIL;
				}

			//	Get the attribute until the delimeter

			bool bIsSpecialAttrib = false;
			char *pStart = pPos;
			while (!IsDelimiterChar(pPos, bIsSpecialAttrib))
				{
				if (*pPos == ':')
					bIsSpecialAttrib = true;
				pPos++;
				}

			//	Add to our list

			SEntry *pEntry = m_Attribs.Insert();
			pEntry->sAttrib = CString(pStart, pPos - pStart);
			pEntry->iWeight = iMatchWeight;
			pEntry->bIsSpecial = bIsSpecialAttrib;
			}
		else
			pPos++;
		}

	return NOERROR;
	}
