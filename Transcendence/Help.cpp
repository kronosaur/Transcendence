//	Help.cpp
//
//	Show help screen

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

const int HELP_PAGE_KEYS =									0;
const int HELP_PAGE_DAMAGE_TYPES =							1;
const int HELP_PAGE_COUNT =									1;

#define RGB_FRAME							CG16bitImage::RGBValue(80,80,80)

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
		{ "[I]",		"Invoke the powers of Domina",					NULL },
		{ "[Q]",		"Squadron orders (if leading a squadron)",		NULL },
		{ "[C]",		"Communications",								NULL },
		{ "[B]",		"Enable/disable devices",						NULL },
		{ "[P]",		"Pauses the game",								NULL },
		{ "[F8/F7]",	"Increase/decrease volume",						NULL },
		{ "[Esc]",		"Game menu (save, self-destruct)",				NULL }
	};

#define RGB_HELP_TEXT					CG16bitImage::RGBValue(231,255,227)
#define RGB_BAR_COLOR					CG16bitImage::RGBValue(0, 2, 10)
#define HELP_LINE_COUNT					(sizeof(g_szHelp) / sizeof(g_szHelp[0]))
const WORD RGB_SUBTITLE_TEXT =			CG16bitImage::RGBValue(128,128,128);
const WORD RGB_MORE_HELP_TEXT =			CG16bitImage::RGBValue(160,160,160);

const int SECTION_SPACING_Y =			16;

const int KEY_INDENT =					70;
const int KEY_DESC_COLUMN_WIDTH =		200;
const int KEY_COLUMN_SPACING_X =		10;
const int KEY_DESC_SPACING_Y =			5;
const int KEY_DESC_SPACING_X =			12;

const int DAMAGE_TYPE_INDENT =			24;
const int DAMAGE_TYPE_SPACING_Y =		4;

void CTranscendenceWnd::OnKeyDownHelp (int iVirtKey, DWORD dwKeyData)

//	OnKeyDownHelp
//
//	Handle WM_KEYDOWN

	{
	switch (iVirtKey)
		{
		case VK_UP:
		case VK_LEFT:
			if (m_iHelpPage > 0)
				{
				m_iHelpPage--;
				m_bHelpInvalid = true;
				}
			break;

		case VK_DOWN:
		case VK_RIGHT:
			if (m_iHelpPage + 1 < HELP_PAGE_COUNT)
				{
				m_iHelpPage++;
				m_bHelpInvalid = true;
				}
			break;

		default:
			StopHelp();
		}
	}

void CTranscendenceWnd::PaintHelpScreen (void)

//	PaintHelpScreen
//
//	Paints the help screen

	{
	int i;
	int cyBarHeight = (g_cyScreen - m_HelpImage.GetHeight()) / 2;

	RECT rcImage;
	rcImage.left = (g_cxScreen - m_HelpImage.GetWidth()) / 2;
	rcImage.top = (g_cyScreen - m_HelpImage.GetHeight()) / 2;
	rcImage.right = rcImage.left + m_HelpImage.GetWidth();
	rcImage.bottom = rcImage.top + m_HelpImage.GetHeight();

	//	Paint the parts that don't change

	if (m_bHelpInvalid)
		{
		//	Paint bars across top and bottom

		m_Screen.Fill(0, 
				0, 
				g_cxScreen, 
				g_cyScreen,
				RGB_BAR_COLOR);

		//	Paint image

		m_Screen.Blt(0,
				0,
				m_HelpImage.GetWidth(),
				m_HelpImage.GetHeight(),
				m_HelpImage,
				rcImage.left,
				rcImage.top);

		//	Paint a frame around viewscreen

		m_Screen.FillLine(0, rcImage.top, g_cxScreen, RGB_FRAME);
		m_Screen.FillLine(0, rcImage.top + RectHeight(rcImage), g_cxScreen, RGB_FRAME);
		//	Paint help page

		RECT rcPage;
		rcPage.left = rcImage.left + (4 * RectWidth(rcImage) / 9);
		rcPage.top = rcImage.top + m_Fonts.LargeBold.GetHeight();
		rcPage.right = rcImage.right - m_Fonts.LargeBold.GetHeight();
		rcPage.bottom = rcImage.bottom - m_Fonts.LargeBold.GetHeight();

		//	Paint help page

		switch (m_iHelpPage)
			{
			case HELP_PAGE_KEYS:
				{
				int x = rcPage.left;
				int y = rcPage.top;

				//	Paint keys

				m_Screen.DrawText(x, y, m_Fonts.HeaderBold, RGB_SUBTITLE_TEXT, CONSTLIT("Keys"));
				y += m_Fonts.HeaderBold.GetHeight();

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
					m_Fonts.MediumBold.DrawText(m_Screen, rcRect, RGB_HELP_TEXT, CString(g_szHelp[i].szKey), 0, CG16bitFont::AlignRight);

					//	Draw the description

					int cyHeight;
					rcRect.left = x + KEY_INDENT;
					rcRect.top = y;
					rcRect.right = rcRect.left + KEY_DESC_COLUMN_WIDTH;
					rcRect.bottom = rcPage.bottom;
					m_Fonts.Medium.DrawText(m_Screen, rcRect, RGB_HELP_TEXT, CString(g_szHelp[i].szDesc), 0, 0, &cyHeight);

					//	Extra description

					if (g_szHelp[i].szMoreDesc)
						{
						int cyHeight2;
						rcRect.top = y + cyHeight;
						m_Fonts.Medium.DrawText(m_Screen, rcRect, RGB_MORE_HELP_TEXT, CString(g_szHelp[i].szMoreDesc), 0, 0, &cyHeight2);

						cyHeight += cyHeight2;
						}

					//	Next line down

					y += Max(cyHeight, m_Fonts.MediumBold.GetHeight()) + KEY_DESC_SPACING_Y;

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

				int cxColWidth = 20 * m_Fonts.MediumBold.GetAverageWidth();

				m_Screen.DrawText(x, y, m_Fonts.HeaderBold, RGB_SUBTITLE_TEXT, CONSTLIT("Damage Types"));
				y += m_Fonts.HeaderBold.GetHeight();

				yCol = y;

				for (i = 0; i < damageCount; i++)
					{
					m_UIRes.DrawDamageTypeIcon(m_Screen, x, y, i);
					
					m_Screen.DrawText(x + DAMAGE_TYPE_INDENT, 
							y, 
							m_Fonts.MediumBold, 
							RGB_HELP_TEXT, 
							strCapitalize(::GetDamageName((DamageTypes)i)));

					//	Increment

					if (((i + 1) % 4) != 0)
						y += m_Fonts.MediumBold.GetHeight() + DAMAGE_TYPE_SPACING_Y;
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
				int cxColWidth = 20 * m_Fonts.MediumBold.GetAverageWidth();

				m_Screen.DrawText(x, y, m_Fonts.HeaderBold, RGB_SUBTITLE_TEXT, CONSTLIT("Damage Types"));
				y += m_Fonts.HeaderBold.GetHeight();

				int yCol = y;

				for (i = 0; i < damageCount; i++)
					{
					m_UIRes.DrawDamageTypeIcon(m_Screen, x, y, i);
					
					m_Screen.DrawText(x + DAMAGE_TYPE_INDENT, 
							y, 
							m_Fonts.MediumBold, 
							RGB_HELP_TEXT, 
							strCapitalize(::GetDamageName((DamageTypes)i)));

					//	Increment

					if (((i + 1) % 4) != 0)
						y += m_Fonts.MediumBold.GetHeight() + DAMAGE_TYPE_SPACING_Y;
					else
						{
						x += cxColWidth;
						y = yCol;
						}
					}

				break;
				}
			}

		//	Done

		m_bHelpInvalid = false;
		}
	}

ALERROR CTranscendenceWnd::StartHelp (void)

//	StartHelp
//
//	Show the help screen

	{
	ALERROR error;

	//	Load a JPEG of the epilog screen

	HBITMAP hDIB;
	if (error = JPEGLoadFromResource(g_hInst,
			MAKEINTRESOURCE(IDR_HELP_BACKGROUND),
			JPEG_LFR_DIB, 
			NULL, 
			&hDIB))
		return error;

	error = m_HelpImage.CreateFromBitmap(hDIB);
	::DeleteObject(hDIB);
	if (error)
		return error;

	//	Done

	m_iHelpPage = HELP_PAGE_KEYS;
	m_OldState = m_State;
	m_State = gsHelp;
	m_bHelpInvalid = true;

	return NOERROR;
	}

void CTranscendenceWnd::StopHelp (void)

//	StopHelp
//
//	Clean up the help screen

	{
	ASSERT(m_State == gsHelp);

	m_HelpImage.Destroy();
	m_State = m_OldState;
	}
