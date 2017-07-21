//	CHelpSession.cpp
//
//	CHelpSession class

#include "PreComp.h"
#include "Transcendence.h"

const int HELP_PAGE_KEYS =									0;
const int HELP_PAGE_DAMAGE_TYPES =							1;
const int HELP_PAGE_COUNT =									1;

struct SHelpLine
	{
	char *szKey;
	char *szDesc;
	char *szMoreDesc;
	};

static SHelpLine g_szHelp[] =
	{
		{ "Arrows",		"Rotate and thrust",							NULL },
		{ "[Ctrl]",		"Fire primary weapon system",					NULL },
		{ "[Shift]",	"Fire missile launcher (if installed)",			NULL },
		{ "[S]",		"Ship's status (refuel, view cargo, jettison)",	NULL },
		{ "[M]",		"Show map of entire system",					NULL },
		{ "[A]",		"Engage autopilot",								NULL },
		{ "[D]",		"Dock with nearest station",					NULL },
		{ "[G]",		"Enter stargate",								NULL },
		{ "[U]",		"Use item",										NULL },
		{ "[.]",		"Stop the ship",								NULL },
		{ "[W]",		"Select primary weapon to use (if multiple)",	NULL },
		{ "[Tab]",		"Select missile to launch (if installed)",		NULL },
		{ "[T]",		"Select next target (if targeting installed)",	NULL },
		{ "[F]",		"Select next friend (if targeting installed)",	NULL },
		{ "[R]",		"Clear target (if targeting installed)",		NULL },
		{ "[I]",		"Invoke special powers",						NULL },
		{ "[Q]",		"Squadron orders (if leading a squadron)",		NULL },
		{ "[C]",		"Communications",								NULL },
		{ "[B]",		"Enable/disable devices",						NULL },
		{ "[N]",		"Show stargate network (if galactic map installed)",	NULL },
		{ "[P]",		"Pauses the game",								NULL },
		{ "[F2]",		"Show game statistics",							NULL },
		{ "[F8/F7]",	"Increase/decrease volume",						NULL },
		{ "[Esc]",		"Game menu (save, self-destruct)",				NULL }
	};

#define RGB_FRAME							CG32bitPixel(80,80,80)
#define RGB_HELP_TEXT						CG32bitPixel(231,255,227)
#define RGB_BAR_COLOR						CG32bitPixel(0, 2, 10)
#define HELP_LINE_COUNT						(sizeof(g_szHelp) / sizeof(g_szHelp[0]))
const CG32bitPixel RGB_SUBTITLE_TEXT =		CG32bitPixel(128,128,128);
const CG32bitPixel RGB_MORE_HELP_TEXT =		CG32bitPixel(160,160,160);
const CG32bitPixel RGB_IMAGE_BACKGROUND =	CG32bitPixel(35, 41, 37);

const int SECTION_SPACING_Y =				16;

const int KEY_INDENT =						70;
const int KEY_DESC_COLUMN_WIDTH =			200;
const int KEY_COLUMN_SPACING_X =			10;
const int KEY_DESC_SPACING_Y =				5;
const int KEY_DESC_SPACING_X =				12;

const int DAMAGE_TYPE_INDENT =				24;
const int DAMAGE_TYPE_SPACING_Y =			4;

void CHelpSession::OnCleanUp (void)

//	OnCleanUp
//
//	Clean up

	{
	}

ALERROR CHelpSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	ALERROR error;

	//	Load a JPEG of the background image

	HBITMAP hDIB;
	if (error = JPEGLoadFromResource(NULL,
			MAKEINTRESOURCE(IDR_HELP_BACKGROUND),
			JPEG_LFR_DIB, 
			NULL, 
			&hDIB))
		return error;

	bool bSuccess = m_HelpImage.CreateFromBitmap(hDIB);
	::DeleteObject(hDIB);
	if (!bSuccess)
		return ERR_FAIL;

	//	Done

	m_iHelpPage = HELP_PAGE_KEYS;

	return NOERROR;
	}

void CHelpSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//	OnKeyDown
//
//	KeyDown

	{
	//	Done

	m_HI.ClosePopupSession();
	}

void CHelpSession::OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture)

//	OnLButtonDown
//
//	LButtonDown

	{
	//	Done

	m_HI.ClosePopupSession();
	}

void CHelpSession::OnPaint (CG32bitImage &Screen, const RECT &rcInvalid)

//	OnPaint
//
//	Paint

	{
	int i;
	const CVisualPalette &VI = m_HI.GetVisuals();

	RECT rcCenter;
	VI.DrawSessionBackground(Screen, m_HelpImage, RGB_IMAGE_BACKGROUND, 0, &rcCenter);

	//	Paint help page

	RECT rcPage;
	rcPage.left = rcCenter.left + (4 * RectWidth(rcCenter) / 9);
	rcPage.top = rcCenter.top + VI.GetFont(fontLargeBold).GetHeight();
	rcPage.right = rcCenter.right - VI.GetFont(fontLargeBold).GetHeight();
	rcPage.bottom = rcCenter.bottom - VI.GetFont(fontLargeBold).GetHeight();

	//	Paint help page

	switch (m_iHelpPage)
		{
		case HELP_PAGE_KEYS:
			{
			int x = rcPage.left;
			int y = rcPage.top;

			//	Paint keys

			Screen.DrawText(x, y, VI.GetFont(fontHeaderBold), RGB_SUBTITLE_TEXT, CONSTLIT("Keys"));
			y += VI.GetFont(fontHeaderBold).GetHeight();

			int yCol = y;
			int yMax = y;

			for (i = 0; i < HELP_LINE_COUNT; i++)
				{
				//	Draw the keys

				RECT rcRect;
				rcRect.left = x;
				rcRect.top = y;
				rcRect.right = rcRect.left + KEY_INDENT - KEY_DESC_SPACING_X;
				rcRect.bottom = rcPage.bottom;
				VI.GetFont(fontMediumBold).DrawText(Screen, rcRect, RGB_HELP_TEXT, CString(g_szHelp[i].szKey), 0, CG16bitFont::AlignRight);

				//	Draw the description

				int cyHeight;
				rcRect.left = x + KEY_INDENT;
				rcRect.top = y;
				rcRect.right = rcRect.left + KEY_DESC_COLUMN_WIDTH;
				rcRect.bottom = rcPage.bottom;
				VI.GetFont(fontMedium).DrawText(Screen, rcRect, RGB_HELP_TEXT, CString(g_szHelp[i].szDesc), 0, 0, &cyHeight);

				//	Extra description

				if (g_szHelp[i].szMoreDesc)
					{
					int cyHeight2;
					rcRect.top = y + cyHeight;
					VI.GetFont(fontMedium).DrawText(Screen, rcRect, RGB_MORE_HELP_TEXT, CString(g_szHelp[i].szMoreDesc), 0, 0, &cyHeight2);

					cyHeight += cyHeight2;
					}

				//	Next line down

				y += Max(cyHeight, VI.GetFont(fontMediumBold).GetHeight()) + KEY_DESC_SPACING_Y;

				//	Keep track of high-water mark

				yMax = Max(yMax, y);

				//	Wrap to next column

				if (i == (HELP_LINE_COUNT + 1) / 2)
					{
					y = yCol;
					x += KEY_INDENT + KEY_DESC_COLUMN_WIDTH + KEY_COLUMN_SPACING_X;
					}
				}

			x = rcPage.left;
			y = yMax + SECTION_SPACING_Y;

			//	Paint damage types

			int cxColWidth = 20 * VI.GetFont(fontMediumBold).GetAverageWidth();

			Screen.DrawText(x, y, VI.GetFont(fontHeaderBold), RGB_SUBTITLE_TEXT, CONSTLIT("Damage Types"));
			y += VI.GetFont(fontHeaderBold).GetHeight();

			yCol = y;

			for (i = 0; i < damageCount; i++)
				{
				VI.DrawDamageTypeIcon(Screen, x, y, (DamageTypes)i);
				
				Screen.DrawText(x + DAMAGE_TYPE_INDENT, 
						y, 
						VI.GetFont(fontMedium), 
						RGB_HELP_TEXT, 
						strCapitalize(::GetDamageName((DamageTypes)i)));

				//	Increment

				if (((i + 1) % 4) != 0)
					y += VI.GetFont(fontMediumBold).GetHeight() + DAMAGE_TYPE_SPACING_Y;
				else
					{
					x += cxColWidth;
					y = yCol;
					}
				}

			break;
			}

		case HELP_PAGE_DAMAGE_TYPES:
			{
			int x = rcPage.left;
			int y = rcPage.top;
			int cxColWidth = 20 * VI.GetFont(fontMediumBold).GetAverageWidth();

			Screen.DrawText(x, y, VI.GetFont(fontHeaderBold), RGB_SUBTITLE_TEXT, CONSTLIT("Damage Types"));
			y += VI.GetFont(fontHeaderBold).GetHeight();

			int yCol = y;

			for (i = 0; i < damageCount; i++)
				{
				VI.DrawDamageTypeIcon(Screen, x, y, (DamageTypes)i);
				
				Screen.DrawText(x + DAMAGE_TYPE_INDENT, 
						y, 
						VI.GetFont(fontMediumBold), 
						RGB_HELP_TEXT, 
						strCapitalize(::GetDamageName((DamageTypes)i)));

				//	Increment

				if (((i + 1) % 4) != 0)
					y += VI.GetFont(fontMediumBold).GetHeight() + DAMAGE_TYPE_SPACING_Y;
				else
					{
					x += cxColWidth;
					y = yCol;
					}
				}

			break;
			}
		}
	}

void CHelpSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash
//
//	Describe current state

	{
	*retsMessage = CONSTLIT("session: CHelpSession\r\n");
	}

void CHelpSession::OnUpdate (bool bTopMost)

//	OnUpdate
//
//	Update

	{
	}
