//	CAttributeCriteria.cpp
//
//	CAttributeCrtieria class
//	Copryight (c) 2011 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

#define MATCH_ALL						CONSTLIT("*")

inline bool IsWeightChar (char *pPos) { return (*pPos == '+' || *pPos == '-' || *pPos == '*' || *pPos == '!'); }
inline bool IsDelimiterChar (char *pPos) { return (*pPos == '\0' || *pPos == ',' || *pPos == ';' || strIsWhitespace(pPos)); }

const CString &CAttributeCriteria::GetAttribAndRequired (int iIndex, bool *retbRequired) const

//	GetAttribAndRequired
//
//	Returns the attrib and whether or not it is required

	{
	if (retbRequired)
		*retbRequired = (m_Weights[iIndex] >= 0);

	return m_Attribs[iIndex];
	}

const CString &CAttributeCriteria::GetAttribAndWeight (int iIndex, int *retiWeight) const

//	GetAttribAndWeight
//
//	Returns the attrib and its weight

	{
	if (retiWeight)
		*retiWeight = m_Weights[iIndex];

	return m_Attribs[iIndex];
	}

ALERROR CAttributeCriteria::Parse (const CString &sCriteria, DWORD dwFlags, CString *retsError)

//	Parse
//
//	Parses the criteria

	{
	m_Attribs.DeleteAll();
	m_Weights.DeleteAll();
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

			char *pStart = pPos;
			while (!IsDelimiterChar(pPos))
				pPos++;

			//	Add to our list

			m_Attribs.Insert(CString(pStart, pPos - pStart));
			m_Weights.Insert(iMatchWeight);
			}
		else
			pPos++;
		}

	return NOERROR;
	}
