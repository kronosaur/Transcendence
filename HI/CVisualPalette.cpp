//	CVisualPalette.cpp
//
//	CVisualPalette class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

const int DAMAGE_TYPE_ICON_WIDTH =			16;
const int DAMAGE_TYPE_ICON_HEIGHT =			16;

const int WIDESCREEN_WIDTH =				1024;
const int WIDESCREEN_HEIGHT =				512;

struct SColorInitTable
	{
	char *pszName;									//	Name of color
	WORD wColor;									//	16-bit color value
	};

static SColorInitTable COLOR_TABLE[] =
	{
		{	"Unknown",				0 },

		{	"BackgroundDeep",		CG16bitImage::RGBValue(0, 2, 10) },
		{	"LineFrame",			CG16bitImage::RGBValue(80,80,80) },
		{	"TextHighlight",		CG16bitImage::RGBValue(218, 235, 255) },
		{	"TextNormal",			CG16bitImage::RGBValue(191, 196, 201) },
		{	"TextFade",				CG16bitImage::RGBValue(120, 129, 140) },
	};

struct SFontInitTable
	{
	char *pszName;									//	Name of the font

	char *pszResID;									//	Resource to load (if NULL, then we create from typeface)

	char *pszTypeface;								//	Typeface (ignored if resource)
	int cyPixelHeight;								//	Height of font (ignored if resource)
	bool bBold;										//	TRUE if we want a bold font (ignored if resource)
	bool bItalic;									//	TRUE if we want an italic font (ignored if resource)
	};

static SFontInitTable FONT_TABLE[] =
	{
		{	"Unknown",				NULL,	NULL,				0,		false, false },
		
		{	"Small",				NULL,	"Tahoma",			10,		false, false },
		{	"Medium",				NULL,	"Tahoma",			13,		false, false },
		{	"MediumBold",			NULL,	"Tahoma",			13,		true, false },
		{	"MediumHeavyBold",		NULL,	"Lucida Sans",		14,		true, false },
		{	"Large",				NULL,	"Lucida Sans",		16,		false, false },
		{	"LargeBold",			NULL,	"Lucida Sans",		16,		true, false },
		{	"Header",				"DXFN_HEADER",					NULL,	0,	false, false },
		{	"HeaderBold",			"DXFN_HEADER_BOLD",				NULL,	0,	false, false },
		{	"SubTitle",				"DXFN_SUBTITLE",				NULL,	0,	false, false },
		{	"SubTitleBold",			"DXFN_SUBTITLE_BOLD",			NULL,	0,	false, false },
		{	"SubTitleHeavyBold",	"DXFN_SUBTITLE_HEAVY_BOLD",		NULL,	0,	false, false },
		{	"Title",				"DXFN_TITLE",					NULL,	0,	false, false },
		{	"LogoTitle",			"DXFN_LOGO_TITLE",				NULL,	0,	false, false },
		{	"ConsoleMediumHeavy",	NULL,	"Lucida Console",	14,		false, false },
	};

struct SImageInitTable
	{
	char *pszName;									//	Name of image
	char *pszJPEGResID;								//	Image resource to load
	char *pszMaskResID;								//	Mask resource to load (may be NULL)
	};

static SImageInitTable IMAGE_TABLE[] =
	{
		{	"Unknown",				NULL,							NULL },
		{	"DamageTypeIcons",		"JPEG_DAMAGE_TYPE_ICONS",		"BMP_DAMAGE_TYPE_ICONS_MASK" },
	};

void CVisualPalette::DrawSessionBackground (CG16bitImage &Screen, CG16bitImage &Background, RECT *retrcCenter) const

//	DrawSessionBackground
//
//	Draws a widescreen background with the image in the center

	{
	int cxScreen = Screen.GetWidth();
	int cyScreen = Screen.GetHeight();

	WORD wBackgroundColor = GetColor(colorBackgroundDeep);
	WORD wLineColor = GetColor(colorLineFrame);

	if (!Background.IsEmpty())
		{
		//	Compute the RECT of the background image

		int cxBackground = Min(cxScreen, Background.GetWidth());
		int cyBackground = Min(WIDESCREEN_HEIGHT, Background.GetHeight());

		RECT rcBackgroundDest;
		rcBackgroundDest.left = (cxScreen - cxBackground) / 2;
		rcBackgroundDest.top = (cyScreen - cyBackground) / 2;
		rcBackgroundDest.right = rcBackgroundDest.left + cxBackground;
		rcBackgroundDest.bottom = rcBackgroundDest.top + cyBackground;

		//	Paint everything except the background image

		Screen.Fill(0, 0, cxScreen, rcBackgroundDest.top, wBackgroundColor);
		Screen.Fill(0, rcBackgroundDest.top, rcBackgroundDest.left, cyBackground, wBackgroundColor);
		Screen.Fill(rcBackgroundDest.right, rcBackgroundDest.top, cxScreen - rcBackgroundDest.right, cyBackground, wBackgroundColor);
		Screen.Fill(0, rcBackgroundDest.bottom, cxScreen, cyScreen - rcBackgroundDest.bottom, wBackgroundColor);

		//	Now paint the background image

		Screen.Blt((Background.GetWidth() - cxBackground) / 2,
				(Background.GetHeight() - cyBackground) / 2,
				cxBackground,
				cyBackground,
				Background,
				rcBackgroundDest.left,
				rcBackgroundDest.top);
		}
	else
		Screen.Fill(0, 0, cxScreen, cyScreen, wBackgroundColor);

	//	Paint the frame

	int yLine = (cyScreen - WIDESCREEN_HEIGHT) / 2;
	Screen.FillLine(0, yLine, cxScreen, wLineColor);
	Screen.FillLine(0, yLine + WIDESCREEN_HEIGHT, cxScreen, wLineColor);

	//	Return the center rect

	if (retrcCenter)
		{
		retrcCenter->left = (cxScreen - WIDESCREEN_WIDTH) / 2;
		retrcCenter->top = yLine;
		retrcCenter->right = retrcCenter->left + WIDESCREEN_WIDTH;
		retrcCenter->bottom = retrcCenter->top + WIDESCREEN_HEIGHT;
		}
	}

void CVisualPalette::GetWidescreenRect (CG16bitImage &Screen, RECT *retrcCenter, RECT *retrcFull) const

//	GetWidescreenRect
//
//	Returns the center widescreen rect

	{
	int cxScreen = Screen.GetWidth();
	int cyScreen = Screen.GetHeight();

	if (retrcCenter)
		{
		retrcCenter->left = (cxScreen - WIDESCREEN_WIDTH) / 2;
		retrcCenter->top = (cyScreen - WIDESCREEN_HEIGHT) / 2;
		retrcCenter->right = retrcCenter->left + WIDESCREEN_WIDTH;
		retrcCenter->bottom = retrcCenter->top + WIDESCREEN_HEIGHT;
		}

	if (retrcFull)
		{
		retrcFull->left = 0;
		retrcFull->top = (cyScreen - WIDESCREEN_HEIGHT) / 2;
		retrcFull->right = cxScreen;
		retrcFull->bottom = retrcFull->top + WIDESCREEN_HEIGHT;
		}
	}

ALERROR CVisualPalette::Init (HMODULE hModule, CString *retsError)

//	Init
//
//	Initialize the resources

	{
	ALERROR error;
	int i;

	ASSERT(colorCount == (sizeof(COLOR_TABLE) / sizeof(COLOR_TABLE[0])));
	ASSERT(fontCount == (sizeof(FONT_TABLE) / sizeof(FONT_TABLE[0])));

	//	Initialize colors

	for (i = 0; i < colorCount; i++)
		m_Color[i] = COLOR_TABLE[i].wColor;

	//	Initialize fonts

	for (i = 0; i < fontCount; i++)
		{
		if (FONT_TABLE[i].pszResID)
			{
			if (error = m_Font[i].CreateFromResource(hModule, FONT_TABLE[i].pszResID))
				{
				if (retsError)
					*retsError = strPatternSubst(CONSTLIT("Unable to load font resource: %s"), CString(FONT_TABLE[i].pszResID));
				return error;
				}
			}
		else
			{
			if (error = m_Font[i].Create(FONT_TABLE[i].pszTypeface, 
					-FONT_TABLE[i].cyPixelHeight, 
					FONT_TABLE[i].bBold, 
					FONT_TABLE[i].bItalic))
				{
				if (retsError)
					*retsError = strPatternSubst(CONSTLIT("Unable to create font: %s%s%dpx %s"),
							(FONT_TABLE[i].bBold ? CONSTLIT("bold ") : NULL_STR),
							(FONT_TABLE[i].bItalic ? CONSTLIT("italic ") : NULL_STR),
							FONT_TABLE[i].cyPixelHeight,
							CString(FONT_TABLE[i].pszTypeface));
				return error;
				}
			}
		}

	//	Initialize images

	for (i = 0; i < imageCount; i++)
		{
		//	Load the image

		HBITMAP hDIB = NULL;
		if (IMAGE_TABLE[i].pszJPEGResID)
			{
			if (error = JPEGLoadFromResource(hModule,
					IMAGE_TABLE[i].pszJPEGResID,
					JPEG_LFR_DIB, 
					NULL, 
					&hDIB))
				{
				//	If we did not find it, then its OK because not all EXEs
				//	have all the images (e.g., TransSaver doesn't)

				if (error == ERR_NOTFOUND)
					continue;

				if (retsError)
					*retsError = strPatternSubst(CONSTLIT("Unable to load image resource: %s"), CString(IMAGE_TABLE[i].pszJPEGResID));
				return error;
				}
			}
		else
			continue;

		//	Load mask

		HBITMAP hBitmask = NULL;
		if (IMAGE_TABLE[i].pszMaskResID)
			{
			if (error = dibLoadFromResource(hModule,
					IMAGE_TABLE[i].pszMaskResID,
					&hBitmask))
				{
				if (retsError)
					*retsError = strPatternSubst(CONSTLIT("Unable to load mask resource: %s"), CString(IMAGE_TABLE[i].pszMaskResID));
				return error;
				}
			}

		//	Create the image

		error = m_Image[i].CreateFromBitmap(hDIB, hBitmask);
		::DeleteObject(hDIB);
		::DeleteObject(hBitmask);
		if (error)
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Unable to create image: %s"), CString(IMAGE_TABLE[i].pszName));
			return error;
			}
		}

	return NOERROR;
	}
