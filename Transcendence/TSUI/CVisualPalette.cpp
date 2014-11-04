//	CVisualPalette.cpp
//
//	CVisualPalette class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

const int DAMAGE_TYPE_ICON_WIDTH =			16;
const int DAMAGE_TYPE_ICON_HEIGHT =			16;

const int SCREEN_BORDER_X =					10;

const int WIDESCREEN_WIDTH =				1024;
const int WIDESCREEN_WIDTH_MAX =			1280;
const int WIDESCREEN_HEIGHT =				512;
const int WIDESCREEN_HEIGHT_MAX =			512;

struct SColorInitTable
	{
	char *pszName;									//	Name of color
	WORD wColor;									//	16-bit color value
	};

static SColorInitTable COLOR_TABLE[] =
	{
		{	"Unknown",				0 },

		{	"AreaDeep",					CG16bitImage::RGBValue(0, 2, 10) },
		{	"AreaDialog",				CG16bitImage::RGBValue(28, 33, 39) },
		{	"AreaDialogHighlight",		CG16bitImage::RGBValue(237, 137, 36) },
		{	"AreaDialogInput",			CG16bitImage::RGBValue(28, 33, 39) },
		{	"AreaDialogInputFocus",		CG16bitImage::RGBValue(42, 49, 57) },
		
		{	"AreaDialogTitle",			CG16bitImage::RGBValue(32, 48, 64) },
		{	"AreaInfoMsg",				CG16bitImage::RGBValue(28, 33, 39) },
		{	"AreaWarningMsg",			CG16bitImage::RGBValue(255, 239, 215) },
		{	"LineDialogFrame",			CG16bitImage::RGBValue(163,191,219) },
		{	"LineFrame",				CG16bitImage::RGBValue(80,80,80) },

		{	"TextHighlight",			CG16bitImage::RGBValue(218, 235, 255) },
		{	"TextNormal",				CG16bitImage::RGBValue(191, 196, 201) },
		{	"TextFade",					CG16bitImage::RGBValue(120, 129, 140) },
		{	"TextAltHighlight",			CG16bitImage::RGBValue(255, 239, 215) },
		{	"TextAltNormal",			CG16bitImage::RGBValue(204, 200, 194) },

		{	"TextAltFade",				CG16bitImage::RGBValue(140, 131, 118) },
		{	"TextWarningMsg",			CG16bitImage::RGBValue(40, 40, 40) },
		{	"TextDialogInput",			CG16bitImage::RGBValue(218, 235, 255) },
		{	"TextDialogLabel",			CG16bitImage::RGBValue(115, 134, 153) },
		{	"TextDialogLink",			CG16bitImage::RGBValue(218, 235, 255) },

		{	"TextDialogTitle",			CG16bitImage::RGBValue(115, 134, 153) },
		{	"TextDialogWarning",		CG16bitImage::RGBValue(255, 127, 0) },
		{	"AreaDockTitle",			CG16bitImage::RGBValue(0x37, 0x3f, 0x47) },	//	H:210 S:23 B:28
		{	"TextDockAction",			CG16bitImage::RGBValue(218, 235, 255) },
		{	"TextDockActionHotKey",		CG16bitImage::RGBValue(218, 235, 255) },

		{	"TextDockText",				CG16bitImage::RGBValue(0xAF, 0xB7, 0xBF) },	//	H:210 S:8   B:75
		{	"TextDockTitle",			CG16bitImage::RGBValue(0xE7, 0xF3, 0xFF) },	//	H:210 S:9   B:100
		{	"TextDockTextWarning",		CG16bitImage::RGBValue(0xFF, 0x33, 0x33) },	//	H:0   S:80  B:100
		{	"AreaAdvantage",			CG16bitImage::RGBValue(0x00, 0x53, 0xA6) },
		{	"AreaDisadvantage",			CG16bitImage::RGBValue(0xA6, 0x21, 0x21) },

		{	"AreaShields",				CG16bitImage::RGBValue(0x45, 0x73, 0x17) },
		{	"TextAdvantage",			CG16bitImage::RGBValue(0xCC, 0xE5, 0xFF) },
		{	"TextDisadvantage",			CG16bitImage::RGBValue(0xFF, 0xCC, 0xCC) },
		{	"TextShields",				CG16bitImage::RGBValue(0xE5, 0xFF, 0xCC) },
	};

struct SFontInitTable
	{
	int iLen;										//	Length of font name
	char *pszString;								//	Name of the font

	char *pszResID;									//	Resource to load (if NULL, then we create from typeface)

	char *pszTypeface;								//	Typeface (ignored if resource)
	int cyPixelHeight;								//	Height of font (ignored if resource)
	bool bBold;										//	TRUE if we want a bold font (ignored if resource)
	bool bItalic;									//	TRUE if we want an italic font (ignored if resource)
	};

static SFontInitTable FONT_TABLE[] =
	{
		{	CONSTDEF("Unknown"),			NULL,	NULL,				0,		false, false },
		
		{	CONSTDEF("Small"),				NULL,	"Tahoma",			10,		false, false },
		{	CONSTDEF("SmallBold"),			NULL,	"Tahoma",			11,		true, false },
		{	CONSTDEF("Medium"),				NULL,	"Tahoma",			13,		false, false },
		{	CONSTDEF("MediumBold"),			NULL,	"Tahoma",			13,		true, false },
		{	CONSTDEF("MediumHeavyBold"),	NULL,	"Lucida Sans",		14,		true, false },
		{	CONSTDEF("Large"),				NULL,	"Lucida Sans",		16,		false, false },
		{	CONSTDEF("LargeBold"),			NULL,	"Lucida Sans",		16,		true, false },
		{	CONSTDEF("Header"),				"DXFN_HEADER",					NULL,	0,	false, false },
		{	CONSTDEF("HeaderBold"),			"DXFN_HEADER_BOLD",				NULL,	0,	false, false },
		{	CONSTDEF("SubTitle"),			"DXFN_SUBTITLE",				NULL,	0,	false, false },
		{	CONSTDEF("SubTitleBold"),		"DXFN_SUBTITLE_BOLD",			NULL,	0,	false, false },
		{	CONSTDEF("SubTitleHeavyBold"),	"DXFN_SUBTITLE_HEAVY_BOLD",		NULL,	0,	false, false },
		{	CONSTDEF("Title"),				"DXFN_TITLE",					NULL,	0,	false, false },
		{	CONSTDEF("LogoTitle"),			"DXFN_LOGO_TITLE",				NULL,	0,	false, false },
		{	CONSTDEF("ConsoleMediumHeavy"),	NULL,	"Lucida Console",	14,		false, false },
	};

struct SImageInitTable
	{
	char *pszName;									//	Name of image
	char *pszJPEGResID;								//	Image resource to load
	char *pszMaskResID;								//	Mask resource to load (may be NULL)

	int x;
	int y;
	int cx;
	int cy;
	};

static SImageInitTable IMAGE_TABLE[] =
	{
		{	"Unknown",				NULL,							NULL,	0, 0, 0, 0 },

		{	"DamageTypeIcons",		"JPEG_DAMAGE_TYPE_ICONS",		"BMP_DAMAGE_TYPE_ICONS_MASK",	0,	0,	0,	0, },

		{	"ProfileIcon",			"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			  0,	  0,	 64,	 64, },
		{	"ModExchangeIcon",		"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			  0,	 64,	 64,	 64, },
		{	"MusicIconOn",			"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			  0,	128,	 64,	 64, },
		{	"MusicIconOff",			"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			 64,	128,	 64,	 64, },

		{	"CancelIcon",			"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			  0,	192,	 64,	 64, },
		{	"OKIcon",				"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			 64,	192,	 64,	 64, },
		{	"LeftIcon",				"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			 64,	256,	 64,	 64, },
		{	"RightIcon",			"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			  0,	256,	 64,	 64, },
		{	"UpIcon",				"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			 64,	320,	 64,	 64, },
		{	"DownIcon",				"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			  0,	320,	 64,	 64, },

		{	"SmallCancelIcon",		"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			 64,	384,	 32,	 32, },
		{	"SmallOKIcon",			"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			 96,	384,	 32,	 32, },
		{	"SmallLeftIcon",		"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			 32,	384,	 32,	 32, },
		{	"SmallRightIcon",		"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			  0,	384,	 32,	 32, },
		{	"SmallUpIcon",			"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			 32,	416,	 32,	 32, },
		{	"SmallDownIcon",		"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			  0,	416,	 32,	 32, },

		{	"SlotIcon",				"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			  8,	456,	 48,	 48, },
		{	"SmallSlotIcon",		"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			 64,	416,	 32,	 32, },

		{	"HumanMaleIcon",		"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			 64,	  0,	 64,	 64, },
		{	"HumanFemaleIcon",		"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			 64,	 64,	 64,	 64, },
		{	"SmallHumanMaleIcon",	"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			 64,	448,	 32,	 32, },
		{	"SmallHumanFemaleIcon",	"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			 64,	480,	 32,	 32, },

		{	"SmallEditIcon",		"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			 96,	416,	 32,	 32, },

		{	"PlayIcon",				"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			  0,	512,	 64,	 64, },
		{	"DebugIcon",			"JPEG_UI_ICONS",				"BMP_UI_ICONS_MASK",			 64,	512,	 64,	 64, },
	};

class CResourceImageCache
	{
	public:
		CResourceImageCache (HMODULE hModule) : m_hModule(hModule) { }
		~CResourceImageCache (void);

		CG16bitImage *GetImage (const CString &sImage, const CString &sMask);

	private:
		HMODULE m_hModule;
		TSortMap<CString, CG16bitImage> m_Cache;
	};

void CVisualPalette::DrawDamageTypeIcon (CG16bitImage &Screen, int x, int y, DamageTypes iDamageType) const

//	DrawDamageTypeIcon
//
//	Draws the given damage type icon

	{
	if (iDamageType == damageGeneric)
		return;

	Screen.ColorTransBlt((int)iDamageType * DAMAGE_TYPE_ICON_WIDTH,
			0,
			DAMAGE_TYPE_ICON_WIDTH,
			DAMAGE_TYPE_ICON_HEIGHT,
			255,
			m_Image[imageDamageTypeIcons],
			x,
			y);
	}

void CVisualPalette::DrawSessionBackground (CG16bitImage &Screen, const CG16bitImage &Background, DWORD dwFlags, RECT *retrcCenter) const

//	DrawSessionBackground
//
//	Draws a widescreen background with the image in the center

	{
	int cxScreen = Screen.GetWidth();
	int cyScreen = Screen.GetHeight();

	//	Get some metrics

	RECT rcCenter;
	RECT rcFull;
	GetWidescreenRect(Screen, &rcCenter, &rcFull);
	WORD wBackgroundColor = GetColor(colorAreaDeep);
	WORD wLineColor = GetColor(colorLineFrame);
	int cyCenter = RectHeight(rcCenter);

	if (!Background.IsEmpty())
		{
		//	Compute the RECT of the background image

		int cxBackground = Min(cxScreen, Background.GetWidth());
		int cyBackground = Min(cyCenter, Background.GetHeight());

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
		{
		if (dwFlags & OPTION_SESSION_DLG_BACKGROUND)
			{
			Screen.Fill(rcFull.left, 0, RectWidth(rcFull), rcFull.top, wBackgroundColor);
			Screen.Fill(rcFull.left, rcFull.bottom, RectWidth(rcFull), cyScreen - rcFull.bottom, wBackgroundColor);

			Screen.Fill(rcFull.left, rcFull.top, RectWidth(rcFull), RectHeight(rcFull), GetColor(colorAreaDialog));
			}
		else
			Screen.Fill(0, 0, cxScreen, cyScreen, wBackgroundColor);
		}

	//	Paint the frame

	int yLine = (cyScreen - cyCenter) / 2;
	Screen.FillLine(0, yLine - 1, cxScreen, wLineColor);
	Screen.FillLine(0, yLine + cyCenter, cxScreen, wLineColor);

	//	Done

	if (retrcCenter)
		*retrcCenter = rcCenter;
	}

const CG16bitFont &CVisualPalette::GetFont (const CString &sName, bool *retFound) const

//	GetFont
//
//	Returns the font by name

	{
	int i;

	for (i = 0; i < fontCount; i++)
		{
		if (strEquals(sName, CONSTUSE(FONT_TABLE[i])))
			{
			if (retFound)
				*retFound = true;
			return m_Font[i];
			}
		}

	//	For compatibility we map "Console" to "ConsoleMediumHeavy"

	if (strEquals(sName, CONSTLIT("Console")))
		{
		if (retFound)
			*retFound = true;
		return m_Font[fontConsoleMediumHeavy];
		}

	//	If not found, we return a console font

	else
		{
		if (retFound)
			*retFound = false;
		return m_Font[fontConsoleMediumHeavy];
		}
	}

void CVisualPalette::GetWidescreenRect (CG16bitImage &Screen, RECT *retrcCenter, RECT *retrcFull) const

//	GetWidescreenRect
//
//	Returns the center widescreen rect

	{
	int cxScreen = Screen.GetWidth();
	int cyScreen = Screen.GetHeight();

	//	Compute the size of the center pane

	int cxCenter = Min(cxScreen - (2 * SCREEN_BORDER_X), WIDESCREEN_WIDTH_MAX);
	int cyCenter = Min(cyScreen - 256, WIDESCREEN_HEIGHT_MAX);

	//	Size the rects

	if (retrcCenter)
		{
		retrcCenter->left = (cxScreen - cxCenter) / 2;
		retrcCenter->top = (cyScreen - cyCenter) / 2;
		retrcCenter->right = retrcCenter->left + cxCenter;
		retrcCenter->bottom = retrcCenter->top + cyCenter;
		}

	if (retrcFull)
		{
		retrcFull->left = 0;
		retrcFull->top = (cyScreen - cyCenter) / 2;
		retrcFull->right = cxScreen;
		retrcFull->bottom = retrcFull->top + cyCenter;
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

	CResourceImageCache ImageCache(hModule);

	for (i = 0; i < imageCount; i++)
		{
		//	Load the image

		CG16bitImage *pSource = ImageCache.GetImage(CString(IMAGE_TABLE[i].pszJPEGResID), CString(IMAGE_TABLE[i].pszMaskResID));
		if (pSource == NULL)
			continue;

		//	Figure out how much to blt

		int cx = (IMAGE_TABLE[i].cx == 0 ? pSource->GetWidth() : IMAGE_TABLE[i].cx);
		int cy = (IMAGE_TABLE[i].cy == 0 ? pSource->GetHeight() : IMAGE_TABLE[i].cy);

		//	Create the destination

		m_Image[i].CreateBlank(cx, cy, pSource->HasAlpha());

		//	Copy from the appropriate position

		m_Image[i].Blt(IMAGE_TABLE[i].x, IMAGE_TABLE[i].y, cx, cy, *pSource, 0, 0);
		if (pSource->HasAlpha())
			m_Image[i].CopyAlpha(IMAGE_TABLE[i].x, IMAGE_TABLE[i].y, cx, cy, *pSource, 0, 0);
		}

	return NOERROR;
	}

CResourceImageCache::~CResourceImageCache (void)

//	CResourceImageCache destructor

	{
	}

CG16bitImage *CResourceImageCache::GetImage (const CString &sImage, const CString &sMask)

//	GetImage
//
//	Returns the image

	{
	CG16bitImage *pImage = m_Cache.GetAt(sImage);
	if (pImage)
		return pImage;

	HBITMAP hImage;
	if (JPEGLoadFromResource(m_hModule,
			sImage.GetASCIIZPointer(),
			JPEG_LFR_DIB, 
			NULL, 
			&hImage) != NOERROR)
		return NULL;
	
	HBITMAP hMask = NULL;
	if (!sMask.IsBlank())
		{
		if (dibLoadFromResource(m_hModule,
				sMask.GetASCIIZPointer(),
				&hMask) != NOERROR)
			{
			::DeleteObject(hImage);
			return NULL;
			}
		}

	pImage = m_Cache.Insert(sImage);
	if (pImage->CreateFromBitmap(hImage, hMask) != NOERROR)
		return NULL;

	if (hMask)
		::DeleteObject(hMask);
	::DeleteObject(hImage);

	return pImage;
	}

