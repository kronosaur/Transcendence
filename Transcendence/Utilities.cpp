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

