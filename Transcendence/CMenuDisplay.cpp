//	CMenuDisplay.cpp
//
//	CMenuDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define BLOCK_WIDTH							18
#define BLOCK_HEIGHT						18

CMenuDisplay::CMenuDisplay (void) : m_pMenu(NULL),
		m_bInvalid(true)

//	CMenuDisplay constructor

	{
	}

CMenuDisplay::~CMenuDisplay (void)

//	CMenuDisplay destructor

	{
	CleanUp();
	}

void CMenuDisplay::CleanUp (void)

//	CleanUp
//
//	Frees up all resources

	{
	m_pMenu = NULL;
	}

void CMenuDisplay::ComputeMenuRect (RECT *retrcRect)

//	ComputeMenuRect
//
//	Compute the RECT of the visible menu (based on the menu items)

	{
	ASSERT(m_pMenu);

	int cyHeight = Max(100, 64 + m_pMenu->GetCount() * (BLOCK_HEIGHT + 2));
	retrcRect->left = 0;
	retrcRect->right = RectWidth(m_rcRect);
	retrcRect->top = (RectHeight(m_rcRect) - cyHeight) / 2;
	retrcRect->bottom = retrcRect->top + cyHeight;
	}

CString CMenuDisplay::GetHotKeyFromOrdinal (int iOrdinal, char chKeyToExclude)

//	GetHotKeyFromOrdinal
//
//	Returns a hot key

	{
	char chChar;

	int iExcludeOrdinal = (chKeyToExclude == '\0' ? 1000 : (chKeyToExclude - 'A') + 9);

	if (iOrdinal >= 0 && iOrdinal <= 8)
		chChar = '1' + iOrdinal;
	else if (iOrdinal >= 9 && iOrdinal < iExcludeOrdinal)
		chChar = 'A' + (iOrdinal - 9);
	else if (iOrdinal >= iExcludeOrdinal)
		chChar = chKeyToExclude + 1 + iOrdinal - iExcludeOrdinal;
	else
		return NULL_STR;

	return CString(&chChar, 1);
	}

ALERROR CMenuDisplay::Init (CMenuData *pMenu, const RECT &rcRect)

//	Init
//
//	Initializes display

	{
	ALERROR error;

	CleanUp();

	m_pMenu = pMenu;
	m_rcRect = rcRect;
	m_bInvalid = true;

	//	Create the off-screen buffer

	if (error = m_Buffer.CreateBlank(RectWidth(rcRect), RectHeight(rcRect), false))
		return error;

	m_Buffer.SetBlending(200);

	return NOERROR;
	}

void CMenuDisplay::Paint (CG16bitImage &Dest)

//	Paint
//
//	Paint the buffer

	{
	//	Paint a transparent background

	if (m_pMenu)
		{
		//	Make sure the cached image is up to date

		Update();

		//	Blt

		Dest.ColorTransBlt(0,
				0,
				RectWidth(m_rcRect),
				RectHeight(m_rcRect),
				255,
				m_Buffer,
				m_rcRect.left,
				m_rcRect.top);
		}
	}

void CMenuDisplay::Update (void)

//	Update
//
//	Update the buffer

	{
	if (!m_bInvalid || m_pMenu == NULL)
		return;

	//	Compute the size of the list

	RECT rcMenu;
	ComputeMenuRect(&rcMenu);

	//	Color for menu

	WORD wColor;
	WORD wFadeColor;

	wColor = m_pFonts->wTitleColor;
	wFadeColor = m_pFonts->wTitleColor;

	//	Clear

	m_Buffer.Fill(0, 0, RectWidth(m_rcRect), RectHeight(m_rcRect), CG16bitImage::RGBValue(0, 0, 0));
	m_Buffer.Fill(rcMenu.left, rcMenu.top, RectWidth(rcMenu), RectHeight(rcMenu), m_pFonts->wBackground);

	//	Outline the box

	m_Buffer.DrawLine(rcMenu.left, rcMenu.top, rcMenu.right, rcMenu.top, 1, wColor);
	m_Buffer.DrawBiColorLine(rcMenu.left, rcMenu.top, rcMenu.left, rcMenu.bottom, 1, wColor, wFadeColor);
	m_Buffer.DrawBiColorLine(rcMenu.right - 1, rcMenu.top, rcMenu.right - 1, rcMenu.bottom, 1, wColor, wFadeColor);
	m_Buffer.DrawLine(rcMenu.left, rcMenu.bottom - 1, rcMenu.right, rcMenu.bottom - 1, 1, wFadeColor);

	//	Paint the menu title

	m_pFonts->MediumHeavyBold.DrawText(m_Buffer,
			rcMenu.left + 4,
			rcMenu.top + 4,
			m_pFonts->wTitleColor,
			m_pMenu->GetTitle());

	//	Paint each menu item

	int i;
	int x = rcMenu.left + 40;
	int y = rcMenu.top + 32;
	for (i = 0; i < m_pMenu->GetCount(); i++)
		{
		//	Draw the box and the character

		m_Buffer.Fill(x, y, BLOCK_WIDTH, BLOCK_HEIGHT, m_pFonts->wTitleColor);

		int cyHeight;
		int cxWidth = m_pFonts->Medium.MeasureText(m_pMenu->GetItemKey(i), &cyHeight);
		m_pFonts->Medium.DrawText(m_Buffer,
				x + (BLOCK_WIDTH - cxWidth) / 2,
				y + 1,
				m_pFonts->wBackground,
				m_pMenu->GetItemKey(i));

		//	Draw the name

		m_pFonts->Medium.DrawText(m_Buffer,
				x + BLOCK_WIDTH + 4,
				y + 1,
				m_pFonts->wTitleColor,
				m_pMenu->GetItemLabel(i));

		//	Next

		y += BLOCK_HEIGHT + 2;
		}

	m_bInvalid = false;
	}
