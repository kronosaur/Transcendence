//	Utilities.cpp
//
//	Utility functions

#include "PreComp.h"
#include "Transcendence.h"

#define FONT_SMALL							CONSTLIT("Small")
#define FONT_MEDIUM							CONSTLIT("Medium")
#define FONT_MEDIUM_BOLD					CONSTLIT("MediumBold")
#define FONT_MEDIUM_HEAVY_BOLD				CONSTLIT("MediumHeavyBold")
#define FONT_LARGE							CONSTLIT("Large")
#define FONT_LARGE_BOLD						CONSTLIT("LargeBold")
#define FONT_HEADER							CONSTLIT("Header")
#define FONT_HEADER_BOLD					CONSTLIT("HeaderBold")
#define FONT_SUB_TITLE						CONSTLIT("SubTitle")
#define FONT_SUB_TITLE_BOLD					CONSTLIT("SubTitleBold")
#define FONT_SUB_TITLE_HEAVY_BOLD			CONSTLIT("SubTitleHeavyBold")
#define FONT_TITLE							CONSTLIT("Title")
#define FONT_CONSOLE						CONSTLIT("Console")

CString ComposePlayerNameString (const CString &sString, const CString &sPlayerName, int iGenome, ICCItem *pArgs)

//	ComposePlayerNameString
//
//	Replaces the following variables:
//
//		%name%				player name
//		%he%				he or she
//		%his%				his or her (matching case)
//		%hers%				his or hers (matching case)
//		%him%				him or her (matching case)
//		%sir%				sir or ma'am (matching case)
//		%man%				man or woman (matching case)
//		%brother%			brother or sister (matching case)
//		%%					%

	{
	//	Prepare output

	CString sOutput;
	int iOutLeft = sString.GetLength() * 2;
	char *pOut = sOutput.GetWritePointer(iOutLeft);

	//	Compose. Loop once for each segment that we need to add

	bool bDone = false;
	bool bVar = false;
	char *pPos = sString.GetASCIIZPointer();
	while (!bDone)
		{
		CString sVar;
		char *pSeg;
		char *pSegEnd;

		if (bVar)
			{
			ASSERT(*pPos == '%');

			//	Skip %

			pPos++;
			char *pStart = pPos;
			while (*pPos != '%' && *pPos != '\0')
				pPos++;

			sVar = CString(pStart, pPos - pStart);

			//	Skip the next %

			if (*pPos == '%')
				{
				pPos++;
				bVar = false;
				}
			else
				bDone = true;

			bool bCapitalize = (*sVar.GetASCIIZPointer() >= 'A' && *sVar.GetASCIIZPointer() <= 'Z');

			//	Setup the segment depending on the variable

			if (sVar.IsBlank())
				sVar = CONSTLIT("%");
			else if (strEquals(sVar, CONSTLIT("name")))
				sVar = sPlayerName;
			else if (strEquals(sVar, CONSTLIT("he")))
				{
				if (iGenome == genomeHumanMale)
					sVar = CONSTLIT("he");
				else
					sVar = CONSTLIT("she");
				}
			else if (strEquals(sVar, CONSTLIT("sir")))
				{
				if (iGenome == genomeHumanMale)
					sVar = CONSTLIT("sir");
				else
					sVar = CONSTLIT("ma'am");
				}
			else if (strEquals(sVar, CONSTLIT("man")))
				{
				if (iGenome == genomeHumanMale)
					sVar = CONSTLIT("man");
				else
					sVar = CONSTLIT("woman");
				}
			else if (strEquals(sVar, CONSTLIT("his")))
				{
				if (iGenome == genomeHumanMale)
					sVar = CONSTLIT("his");
				else
					sVar = CONSTLIT("her");
				}
			else if (strEquals(sVar, CONSTLIT("him")))
				{
				if (iGenome == genomeHumanMale)
					sVar = CONSTLIT("him");
				else
					sVar = CONSTLIT("her");
				}
			else if (strEquals(sVar, CONSTLIT("hers")))
				{
				if (iGenome == genomeHumanMale)
					sVar = CONSTLIT("his");
				else
					sVar = CONSTLIT("hers");
				}
			else if (strEquals(sVar, CONSTLIT("brother")))
				{
				if (iGenome == genomeHumanMale)
					sVar = CONSTLIT("brother");
				else
					sVar = CONSTLIT("sister");
				}
			else if (strEquals(sVar, CONSTLIT("son")))
				{
				if (iGenome == genomeHumanMale)
					sVar = CONSTLIT("son");
				else
					sVar = CONSTLIT("daughter");
				}
			else if (pArgs)
				{
				int iArg = strToInt(sVar, 0);
				if (iArg < 0)
					{
					iArg = -iArg;
					bCapitalize = true;
					}

				if (iArg > 0)
					{
					ICCItem *pArg = pArgs->GetElement(iArg + 1);
					if (pArg)
						sVar = pArg->GetStringValue();
					}
				}

			//	If we could not find a valid var, then we assume a
			//	single % sign.

			else
				{
				sVar = CONSTLIT("%");
				pPos = pStart;
				bDone = (*pPos == '\0');
				bVar = false;
				bCapitalize = false;
				}

			//	Capitalize, if necessary

			if (bCapitalize)
				sVar = strCapitalize(sVar);

			//	Setup segment

			pSeg = sVar.GetASCIIZPointer();
			pSegEnd = pSeg + sVar.GetLength();
			}
		else
			{
			//	Skip to the next variable or the end of the string

			pSeg = pPos;
			while (*pPos != '%' && *pPos != '\0')
				pPos++;

			if (*pPos == '\0')
				bDone = true;
			else
				bVar = true;

			pSegEnd = pPos;
			}

		//	Add the next segment

		int iLen = pSegEnd - pSeg;
		if (iLen > 0)
			{
			if (iLen > iOutLeft)
				{
				int iAlloc = sOutput.GetLength();
				int iCurLen = iAlloc - iOutLeft;
				int iNewAlloc = max(iAlloc * 2, iAlloc + iLen);
				pOut = sOutput.GetWritePointer(iNewAlloc);
				pOut += iCurLen;
				iOutLeft = iNewAlloc - iCurLen;
				}

			while (pSeg < pSegEnd)
				*pOut++ = *pSeg++;

			iOutLeft -= iLen;
			}
		}

	//	Done

	int iAlloc = sOutput.GetLength();
	int iCurLen = iAlloc - iOutLeft;
	sOutput.Truncate(iCurLen);
	return sOutput;
	}

const CG16bitFont &GetFontByName (const SFontTable &Fonts, const CString &sFontName)

//	GetFontByName
//
//	Returns the font in the font table by name

	{
	if (strEquals(sFontName, FONT_SMALL))
		return Fonts.Small;
	else if (strEquals(sFontName, FONT_MEDIUM))
		return Fonts.Medium;
	else if (strEquals(sFontName, FONT_MEDIUM_BOLD))
		return Fonts.MediumBold;
	else if (strEquals(sFontName, FONT_MEDIUM_HEAVY_BOLD))
		return Fonts.MediumHeavyBold;
	else if (strEquals(sFontName, FONT_LARGE))
		return Fonts.Large;
	else if (strEquals(sFontName, FONT_LARGE_BOLD))
		return Fonts.LargeBold;
	else if (strEquals(sFontName, FONT_HEADER))
		return Fonts.Header;
	else if (strEquals(sFontName, FONT_HEADER_BOLD))
		return Fonts.HeaderBold;
	else if (strEquals(sFontName, FONT_SUB_TITLE))
		return Fonts.SubTitle;
	else if (strEquals(sFontName, FONT_SUB_TITLE_BOLD))
		return Fonts.SubTitleBold;
	else if (strEquals(sFontName, FONT_SUB_TITLE_HEAVY_BOLD))
		return Fonts.SubTitleHeavyBold;
	else if (strEquals(sFontName, FONT_TITLE))
		return Fonts.Title;
	else
		return Fonts.Console;
	}

CString TransPath (const CString &sPath)
	{
	return sPath;
	}
