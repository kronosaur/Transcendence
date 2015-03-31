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

CString CMenuDisplay::GetHotKeyFromOrdinal (int *ioOrdinal, const TSortMap<CString, bool> &Exclude)

//	GetHotKeyFromOrdinal
//
//	Returns a hot key based on the ordinal and making sure that the key is not
//	already being used. Increments ioOrdinal to the next available.
//
//	If we return NULL_STR then there are no more keys available.

	{
	CString sKey;

	do
		{
		char chChar;

		if (*ioOrdinal >= 0 && *ioOrdinal < 9)
			chChar = '1' + *ioOrdinal;
		else if (*ioOrdinal < 35)
			chChar = 'A' + (*ioOrdinal - 9);
		else
			return NULL_STR;

		//	Next

		sKey = CString(&chChar, 1);
		*ioOrdinal += 1;
		}
	while (Exclude.GetAt(sKey) != NULL);

	return sKey;
	}

ALERROR CMenuDisplay::Init (CMenuData *pMenu, const RECT &rcRect)

//	Init
//
//	Initializes display

	{
	CleanUp();

	m_pMenu = pMenu;
	m_rcRect = rcRect;
	m_bInvalid = true;

	//	Create the off-screen buffer

	if (!m_Buffer.Create(RectWidth(rcRect), RectHeight(rcRect), CG32bitImage::alpha8))
		return ERR_FAIL;

	//m_Buffer.SetBlending(200);

	return NOERROR;
	}

void CMenuDisplay::Paint (CG32bitImage &Dest)

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

		Dest.Blt(0,
				0,
				RectWidth(m_rcRect),
				RectHeight(m_rcRect),
				200,
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

	CG32bitPixel rgbColor;
	CG32bitPixel rgbFadeColor;

	rgbColor = m_pFonts->rgbTitleColor;
	rgbFadeColor = m_pFonts->rgbTitleColor;

	//	Clear

	m_Buffer.Set(CG32bitPixel::Null());
	m_Buffer.Fill(rcMenu.left, rcMenu.top, RectWidth(rcMenu), RectHeight(rcMenu), m_pFonts->rgbBackground);

	//	Outline the box

	m_Buffer.DrawLine(rcMenu.left, rcMenu.top, rcMenu.right, rcMenu.top, 1, rgbColor);
	CGDraw::LineGradient(m_Buffer, rcMenu.left, rcMenu.top, rcMenu.left, rcMenu.bottom, 1, rgbColor, rgbFadeColor);
	CGDraw::LineGradient(m_Buffer, rcMenu.right - 1, rcMenu.top, rcMenu.right - 1, rcMenu.bottom, 1, rgbColor, rgbFadeColor);
	m_Buffer.DrawLine(rcMenu.left, rcMenu.bottom - 1, rcMenu.right, rcMenu.bottom - 1, 1, rgbFadeColor);

	//	Paint the menu title

	m_pFonts->MediumHeavyBold.DrawText(m_Buffer,
			rcMenu.left + 4,
			rcMenu.top + 4,
			m_pFonts->rgbTitleColor,
			m_pMenu->GetTitle());

	//	Paint each menu item

	int i;
	int x = rcMenu.left + 40;
	int y = rcMenu.top + 32;
	for (i = 0; i < m_pMenu->GetCount(); i++)
		{
		//	Draw the box and the character

		m_Buffer.Fill(x, y, BLOCK_WIDTH, BLOCK_HEIGHT, m_pFonts->rgbTitleColor);

		int cyHeight;
		int cxWidth = m_pFonts->Medium.MeasureText(m_pMenu->GetItemKey(i), &cyHeight);
		m_pFonts->Medium.DrawText(m_Buffer,
				x + (BLOCK_WIDTH - cxWidth) / 2,
				y + 1,
				m_pFonts->rgbBackground,
				m_pMenu->GetItemKey(i));

		//	Draw the name

		m_pFonts->Medium.DrawText(m_Buffer,
				x + BLOCK_WIDTH + 4,
				y + 1,
				m_pFonts->rgbTitleColor,
				m_pMenu->GetItemLabel(i));

		//	Next

		y += BLOCK_HEIGHT + 2;
		}

	m_bInvalid = false;
	}
