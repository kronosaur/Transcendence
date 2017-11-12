//	CHost.cpp
//
//	CHost class
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

struct SFontInitTable
	{
	int iLen;										//	Length of font name
	char *pszString;								//	Name of the font

	char *pszTypeface;								//	Typeface (ignored if resource)
	int cyPixelHeight;								//	Height of font (ignored if resource)
	bool bBold;										//	TRUE if we want a bold font (ignored if resource)
	bool bItalic;									//	TRUE if we want an italic font (ignored if resource)
	};

static SFontInitTable FONT_TABLE[] =
	{
		{	CONSTDEF("Unknown"),			NULL,				0,		false, false },
		
		{	CONSTDEF("Small"),				"Tahoma",			10,		false, false },
		{	CONSTDEF("SmallBold"),			"Tahoma",			11,		true, false },
		{	CONSTDEF("Medium"),				"Tahoma",			13,		false, false },
		{	CONSTDEF("MediumBold"),			"Tahoma",			13,		true, false },
		{	CONSTDEF("MediumHeavyBold"),	"Lucida Sans",		14,		true, false },
		{	CONSTDEF("Large"),				"Lucida Sans",		16,		false, false },
		{	CONSTDEF("LargeBold"),			"Lucida Sans",		16,		true, false },
		{	CONSTDEF("Header"),				"Convection",		22,		false, false },
		{	CONSTDEF("HeaderBold"),			"Convection",		22,		true, false },
		{	CONSTDEF("SubTitle"),			"Convection",		29,		false, false },
		{	CONSTDEF("SubTitleBold"),		"Convection",		29,		true, false },
		{	CONSTDEF("SubTitleHeavyBold"),	"Convection",		32,		true, false },
		{	CONSTDEF("Title"),				"Convection",		60,		false, false },
		{	CONSTDEF("LogoTitle"),			"BlueIslandStd",	60,		false, false },
		{	CONSTDEF("ConsoleMediumHeavy"),	"Lucida Console",	14,		false, false },
	};

bool CHost::FindFont (const CString &sFont, const CG16bitFont **retpFont) const

//	FindFont
//
//	Get some default fonts

	{
	int i;

	InitFonts();

	for (i = 0; i < m_Fonts.GetCount(); i++)
		{
		if (strEquals(sFont, CONSTUSE(FONT_TABLE[i])))
			{
			if (retpFont)
				*retpFont = &m_Fonts[i];

			return true;
			}
		}

	return false;
	}

void CHost::InitFonts (void) const

//	InitFonts
//
//	Initialize fonts, if we haven't already

	{
	ALERROR error;
	int i;

	if (m_Fonts.GetCount() > 0)
		return;

	int iFontCount = (sizeof(FONT_TABLE) / sizeof(FONT_TABLE[0]));
	m_Fonts.InsertEmpty(iFontCount);

	//	Initialize fonts

	for (i = 0; i < iFontCount; i++)
		{
		if (error = m_Fonts[i].Create(FONT_TABLE[i].pszTypeface, 
				-FONT_TABLE[i].cyPixelHeight, 
				FONT_TABLE[i].bBold, 
				FONT_TABLE[i].bItalic))
			{
			CString sError = strPatternSubst(CONSTLIT("Unable to create font: %s%s%dpx %s"),
						(FONT_TABLE[i].bBold ? CONSTLIT("bold ") : NULL_STR),
						(FONT_TABLE[i].bItalic ? CONSTLIT("italic ") : NULL_STR),
						FONT_TABLE[i].cyPixelHeight,
						CString(FONT_TABLE[i].pszTypeface));
			printf("ERROR: %s\n", (LPSTR)sError);
			continue;
			}
		}
	}
