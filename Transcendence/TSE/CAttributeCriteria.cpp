//	CAttributeCriteria.cpp
//
//	CAttributeCrtieria class
//	Copryight (c) 2011 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

#define MATCH_ALL						CONSTLIT("*")
#define MATCH_DEFAULT					CONSTLIT("*~")

inline bool IsWeightChar (char *pPos) { return (*pPos == '+' || *pPos == '-' || *pPos == '*' || *pPos == '!'); }
inline bool IsDelimiterChar (char *pPos, bool bIsSpecialAttrib = false) { return (*pPos == '\0' || *pPos == ',' || *pPos == ';' || (!bIsSpecialAttrib && strIsWhitespace(pPos))); }

int CAttributeCriteria::AdjLocationWeight (CSystem *pSystem, CLocationDef *pLoc, int iOriginalWeight) const

//	AdjLocationWeight
//
//	Returns the adjusted weight for the given location.

	{
	int i;
	int iResult = iOriginalWeight;

	for (i = 0; i < GetCount(); i++)
		{
		DWORD dwMatchStrength;
		const CString &sAttrib = GetAttribAndWeight(i, &dwMatchStrength);

		int iAdj = CalcLocationWeight(pSystem, pLoc->GetAttributes(), pLoc->GetOrbit().GetObjectPos(), sAttrib, dwMatchStrength);
		if (iAdj == 0)
			return 0;

		iResult = iResult * iAdj / 1000;
		}

	return iResult;
	}

int CAttributeCriteria::AdjStationWeight (CStationType *pType, int iOriginalWeight) const

//	AdjStationWeight
//
//	Returns the adjusted weight for the station given this set of criteria.

	{
	int i;
	int iResult = iOriginalWeight;

	for (i = 0; i < GetCount(); i++)
		{
		DWORD dwMatchStrength;
		const CString &sAttrib = GetAttribAndWeight(i, &dwMatchStrength);

		int iAdj = CalcWeightAdj(pType->HasAttribute(sAttrib), dwMatchStrength);
		if (iAdj == 0)
			return 0;

		iResult = iResult * iAdj / 1000;
		}

	return iResult;
	}

int CAttributeCriteria::CalcLocationWeight (CSystem *pSystem, const CString &sLocationAttribs, const CVector &vPos) const

//	CalcLocationWeight
//
//	Computes the location weight 1000 = full.

	{
	int i;

	//	Special values

	if (MatchesDefault())
		return 0;
	else if (MatchesAll())
		return 1000;

	//	Compute

	int iChance = 1000;
	for (i = 0; i < GetCount(); i++)
		{
		DWORD dwMatchStrength;
		const CString &sAttrib = GetAttribAndWeight(i, &dwMatchStrength);

		int iAdj = CAttributeCriteria::CalcLocationWeight(pSystem, 
				sLocationAttribs,
				vPos,
				sAttrib,
				dwMatchStrength);

		iChance = (iChance * iAdj) / 1000;
		}

	return iChance;
	}

int CAttributeCriteria::CalcLocationWeight (CSystem *pSystem, const CString &sLocationAttribs, const CVector &vPos, const CString &sAttrib, DWORD dwMatchStrength)

//	CalcLocationWeight
//
//	Computes the location weight

	{
	//	Check to see if either the label
	//	or the node/system has the attribute.

	bool bHasAttrib = (::HasModifier(sLocationAttribs, sAttrib)
			|| (pSystem && pSystem->HasAttribute(vPos, sAttrib)));

	//	Compute the frequency of the given attribute

	int iAttribFreq = g_pUniverse->GetAttributeDesc().GetLocationAttribFrequency(sAttrib);

	//	Adjust probability based on the match strength

	return CalcWeightAdj(bHasAttrib, dwMatchStrength, iAttribFreq);
	}

int CAttributeCriteria::CalcWeightAdj (bool bHasAttrib, DWORD dwMatchStrength, int iAttribFreq)

//	CalcWeightAdj
//
//	OPTION	CODE		HAVE ATTRIB			DON'T HAVE ATTRIB
//	---------------------------------------------------------
//	!		-4			0					1000
//	---		-3			100					1000
//	--		-2			250					1000
//	-		-1			500 				1000
//	+		+1			1000				500
//	++		+2			1000				250
//	+++		+3			1000				100
//	*		+4			1000				0
//
//	NOTE: The above numbers assume iAttribFreq is 20.

	{
	//	Adjust for attribute frequency

	if (iAttribFreq != -1)
		return CalcWeightAdjWithAttribFreq(bHasAttrib, dwMatchStrength, iAttribFreq);

	//	Default

	switch (dwMatchStrength)
		{
		case matchExcluded:
			return (bHasAttrib ? 0 : 1000);

		case matchAvoid3:
			return (bHasAttrib ? 100 : 1000);

		case matchAvoid2:
			return (bHasAttrib ? 250 : 1000);

		case matchAvoid1:
			return (bHasAttrib ? 500 : 1000);

		case matchSeek1:
			return (bHasAttrib ? 1000 : 500);

		case matchSeek2:
			return (bHasAttrib ? 1000 : 250);

		case matchSeek3:
			return (bHasAttrib ? 1000 : 100);

		case matchRequired:
			return (bHasAttrib ? 1000 : 0);

		default:
			return CalcWeightAdjCustom(bHasAttrib, dwMatchStrength);
		}
	}

int CAttributeCriteria::CalcWeightAdjCustom (bool bHasAttrib, DWORD dwMatchStrength)

//	CalcWeightAdjCustom
//
//	Handles the case where match strength is custom

	{
	DWORD dwCode = (dwMatchStrength & CODE_MASK);
	DWORD dwValue = (dwMatchStrength & VALUE_MASK);

	switch (dwCode)
		{
		case CODE_INCREASE_IF:
			if (bHasAttrib)
				return 1000 * (100 + dwValue) / 100;
			else
				return 1000;

		case CODE_DECREASE_IF:
			if (bHasAttrib)
				return (dwValue >= 100 ? 0 : 1000 * (100 - dwValue) / 100);
			else
				return 1000;

		case CODE_INCREASE_UNLESS:
			if (bHasAttrib)
				return 1000;
			else
				return 1000 * (100 + dwValue) / 100;

		case CODE_DECREASE_UNLESS:
			if (bHasAttrib)
				return 1000;
			else
				return (dwValue >= 100 ? 0 : 1000 * (100 - dwValue) / 100);

		default:
			return 1000;
		}
	}

int CAttributeCriteria::CalcWeightAdjWithAttribFreq (bool bHasAttrib, DWORD dwMatchStrength, int iAttribFreq)

//	CalcWeightAdjWithAttribFreq
//
//	If iMatchStrength is positive, then it means we want a certain
//	attribute. If we have the attribute, then we increase our probability
//	but if we don't have the attribute, our probability is normal.
//
//	If iMatchStrength is negative, then it means we DO NOT want a
//	certain attribute. If we have the attribute, then decrease our
//	probability. Otherwise, our probability is normal.
//
//	OPTION	CODE		HAVE ATTRIB			DON'T HAVE ATTRIB
//	---------------------------------------------------------
//	!		-4			0					1250
//	---		-3			50					1238
//	--		-2			160					1210
//	-		-1			500 				1125
//	+		+1			3000				500
//	++		+2			4360				160
//	+++		+3			4800				50
//	*		+4			5000				0
//
//	NOTE: The above numbers assume iAttribFreq is 20.

	{
	//	Attribute frequency must be >0 and <100.

	if (iAttribFreq <= 0 || iAttribFreq >= 100)
		{
		::kernelDebugLogMessage("ERROR: Invalid attribute frequency: %d", iAttribFreq);
		return 1000;
		}

	switch (dwMatchStrength)
		{
		//	We only appear at locations with the attribute

		case matchRequired:
			{
			//	If this location has the attribute, then we increase our chance inversely with
			//	the frequency of the attribute. That is, if we MUST be at a location with
			//	a rare attribute, then we INCREASE our probability relative to other
			//	objects, so that we keep our overall frequency relatively constant.

			if (bHasAttrib)
				return 100000 / iAttribFreq;

			//	If this location DOES NOT have the attribute, then we have 0 chance of
			//	appearing here.

			else
				return 0;
			}

		//	We never appear at locations with the attribute

		case matchExcluded:
			{
			//	If the location has the attribute, we have 0 chance of appearing here.

			if (bHasAttrib)
				return 0;

			//	Otherwise, our chance of appearing is proportional to the frequency
			//	of the attribute. 

			else
				return 100000 / (100 - iAttribFreq);
			}

		//	Compute based on match strength

		default:
			{
			//	First we compute the % of objects that should be at locations with the
			//	given attribute, based on the match strength and the global probability
			//	that a location has the given attribute.

			Metric rF;
			switch (dwMatchStrength)
				{
				case matchSeek3:
					rF = 100.0 - ((100 - iAttribFreq) * 0.05);
					break;

				case matchSeek2:
					rF = 100.0 - ((100 - iAttribFreq) * 0.16);
					break;

				case matchSeek1:
					rF = 100.0 - ((100 - iAttribFreq) * 0.5);
					break;

				case matchAvoid1:
					rF = iAttribFreq * 0.5;
					break;

				case matchAvoid2:
					rF = iAttribFreq * 0.16;
					break;

				case matchAvoid3:
					rF = iAttribFreq * 0.05;
					break;

				//	This means we've got a custom match strength

				default:
					return CalcWeightAdjCustom(bHasAttrib, dwMatchStrength);
				}

			//	Compute our probability based on whether we have the attribute our not

			if (bHasAttrib)
				return (int)(1000.0 * rF / iAttribFreq);
			else
				return (int)(1000.0 * (100.0 - rF) / (100 - iAttribFreq));
			}
		}
	}

const CString &CAttributeCriteria::GetAttribAndRequired (int iIndex, bool *retbRequired) const

//	GetAttribAndRequired
//
//	Returns the attrib and whether or not it is required

	{
	if (retbRequired)
		{
		switch (m_Attribs[iIndex].dwMatchStrength & CODE_MASK)
			{
			case CODE_REQUIRED:
			case CODE_SEEK:
			case CODE_INCREASE_IF:
			case CODE_DECREASE_UNLESS:
				*retbRequired = true;

			default:
				*retbRequired = false;
			}
		}

	return m_Attribs[iIndex].sAttrib;
	}

const CString &CAttributeCriteria::GetAttribAndWeight (int iIndex, DWORD *dwMatchStrength, bool *retbIsSpecial) const

//	GetAttribAndWeight
//
//	Returns the attrib and its weight

	{
	const SEntry &Entry = m_Attribs[iIndex];

	if (dwMatchStrength)
		*dwMatchStrength = Entry.dwMatchStrength;

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
	else if (strEquals(sCriteria, MATCH_DEFAULT))
		{
		m_dwFlags |= flagDefault;
		return NOERROR;
		}

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
						iMatchWeight = 4;
						break;

					case '!':
						iMatchWeight = -4;
						break;

					case '+':
						if (iMatchWeight < 3)
							iMatchWeight++;
						break;

					case '-':
						if (iMatchWeight > -3)
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
			char *pCustomWeight = NULL;
			while (!IsDelimiterChar(pPos, bIsSpecialAttrib))
				{
				if (*pPos == ':')
					bIsSpecialAttrib = true;
				else if (*pPos == '|')
					pCustomWeight = pPos;

				pPos++;
				}

			//	Add to our list

			SEntry *pEntry = m_Attribs.Insert();
			pEntry->bIsSpecial = bIsSpecialAttrib;

			//	If we have a custom weight, then we need to parse the weight.

			if (pCustomWeight)
				{
				pEntry->sAttrib = CString(pStart, pCustomWeight - pStart);

				int iWeight = strParseInt(pCustomWeight + 1, 0);
				if (iMatchWeight >= 0)
					{
					if (iWeight > 0)
						pEntry->dwMatchStrength = CODE_INCREASE_IF | (iWeight & VALUE_MASK);
					else
						pEntry->dwMatchStrength = CODE_DECREASE_IF | (-iWeight & VALUE_MASK);
					}
				else
					{
					if (iWeight > 0)
						pEntry->dwMatchStrength = CODE_INCREASE_UNLESS | (iWeight & VALUE_MASK);
					else
						pEntry->dwMatchStrength = CODE_DECREASE_UNLESS | (-iWeight & VALUE_MASK);
					}
				}

			//	Otherwise, we just take match weight

			else
				{
				pEntry->sAttrib = CString(pStart, pPos - pStart);

				switch (iMatchWeight)
					{
					case -4:
						pEntry->dwMatchStrength = matchExcluded;
						break;

					case -3:
						pEntry->dwMatchStrength = matchAvoid3;
						break;

					case -2:
						pEntry->dwMatchStrength = matchAvoid2;
						break;

					case -1:
						pEntry->dwMatchStrength = matchAvoid1;
						break;

					case 1:
						pEntry->dwMatchStrength = matchSeek1;
						break;

					case 2:
						pEntry->dwMatchStrength = matchSeek2;
						break;

					case 3:
						pEntry->dwMatchStrength = matchSeek3;
						break;

					case 4:
						pEntry->dwMatchStrength = matchRequired;
						break;

					default:
						pEntry->dwMatchStrength = 0;
					}
				}
			}
		else
			pPos++;
		}

	return NOERROR;
	}
