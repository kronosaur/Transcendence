//	CPickerDisplay.cpp
//
//	CPickerDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define TOP_BORDER									4
#define BOTTOM_BORDER								4

#define TILE_WIDTH									96
#define TILE_HEIGHT									96
#define TILE_SPACING_X								4
#define TILE_SPACING_Y								4

#define MAX_ITEMS_VISIBLE							3

#define RGB_SELECTION								(CG32bitPixel(150,255,180))
#define RGB_TEXT									(CG32bitPixel(150,255,180))
#define RGB_EXTRA									(CG32bitPixel(255,255,255))
#define RGB_HELP									(CG32bitPixel(96,96,96))
#define RGB_HOTKEY_BACKGROUND						(CG32bitPixel(150,255,180))
#define RGB_BACKGROUND								(CG32bitPixel(0,0,16))

const CG32bitPixel RGB_HOVER_BACKGROUND =			CG32bitPixel(40, 40, 40);

CPickerDisplay::CPickerDisplay (void)

//	CPickerDisplay constructor

	{
	}

CPickerDisplay::~CPickerDisplay (void)

//	CPickerDisplay destructor

	{
	}

void CPickerDisplay::CleanUp (void)

//	CleanUp
//
//	Cleanup display
	
	{
	m_pMenu = NULL;
	}

int CPickerDisplay::GetSelection (void)

//	GetSelection
//
//	Returns the currently selected menu item

	{
	return m_iSelection;
	}

int CPickerDisplay::HitTest (int x, int y) const

//	HitTest
//
//	Returns the entry that we are over

	{
	if (m_bInvalid)
		return -1;

	//	Convert to buffer-relative coordinates

	x -= m_rcRect.left;
	y -= m_rcRect.top;

	//	See if we're in the list area vertically

	if (y >= m_rcView.top && y < m_rcView.bottom)
		{
		int iEntry = (x - m_rcView.left) / (TILE_WIDTH + TILE_SPACING_X);
		if (iEntry < 0 || iEntry >= m_iCountInView)
			return -1;

		return m_iFirstInView + iEntry;
		}

	//	Otherwise, no hit

	else
		return -1;
	}

ALERROR CPickerDisplay::Init (CMenuData *pMenu, const RECT &rcRect)

//	Init
//
//	Initializes picker

	{
	CleanUp();

	m_pMenu = pMenu;
	m_rcRect = rcRect;
	m_bInvalid = true;

	//	Create the off-screen buffer

	if (!m_Buffer.Create(RectWidth(m_rcRect), RectHeight(m_rcRect), CG32bitImage::alpha8))
		return ERR_FAIL;

	//m_Buffer.SetBlending(240);

	//	Initialize

	m_iSelection = 0;
	m_iHover = -1;
	m_cxSmoothScroll = 0;
	m_iFirstInView = 0;
	m_iCountInView = 0;

	return NOERROR;
	}

bool CPickerDisplay::LButtonDown (int x, int y)

//	LButtonDown
//
//	Handle click. Returns TRUE if we clicked on a valid entry in the list.

	{
	int iEntry = HitTest(x, y);
	if (iEntry == -1)
		return false;

	m_iSelection = iEntry;
	m_bInvalid = true;
	return true;
	}

bool CPickerDisplay::MouseMove (int x, int y)

//	MouseMove
//
//	Handle mouse move

	{
	int iEntry = HitTest(x, y);
	if (iEntry != m_iHover)
		{
		m_iHover = iEntry;
		m_bInvalid = true;
		}

	return true;
	}

bool CPickerDisplay::MouseWheel (int iDelta, int x, int y)

//	MouseWheel
//
//	Handle mouse wheel.

	{
	if (iDelta > 0)
		SelectPrev();
	else if (iDelta < 0)
		SelectNext();

	return true;
	}

void CPickerDisplay::Paint (CG32bitImage &Dest)

//	Paint
//
//	Paint the display

	{
	Update();

	Dest.Blt(0,
			0,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			240,
			m_Buffer,
			m_rcRect.left,
			m_rcRect.top);
	}

void CPickerDisplay::PaintSelection (CG32bitImage &Dest, int x, int y)

//	PaintSelection
//
//	Paints the selection at the coordinate

	{
	CG32bitPixel rgbSelectColor = CG32bitPixel::Darken(m_pFonts->rgbSelectBackground, 192);

	Dest.Fill(x - TILE_SPACING_X, 
			y - TILE_SPACING_Y,
			TILE_SPACING_X,
			TILE_HEIGHT + 2 * TILE_SPACING_Y,
			rgbSelectColor);

	Dest.Fill(x,
			y - TILE_SPACING_Y,
			TILE_SPACING_X,
			TILE_SPACING_Y,
			rgbSelectColor);

	Dest.Fill(x,
			y + TILE_HEIGHT,
			TILE_SPACING_X,
			TILE_SPACING_Y,
			rgbSelectColor);

	Dest.Fill(x + TILE_WIDTH,
			y - TILE_SPACING_Y,
			TILE_SPACING_X,
			TILE_HEIGHT + 2 * TILE_SPACING_Y,
			rgbSelectColor);

	Dest.Fill(x + TILE_WIDTH - TILE_SPACING_X,
			y - TILE_SPACING_Y,
			TILE_SPACING_X,
			TILE_SPACING_Y,
			rgbSelectColor);

	Dest.Fill(x + TILE_WIDTH - TILE_SPACING_X,
			y + TILE_HEIGHT,
			TILE_SPACING_X,
			TILE_SPACING_Y,
			rgbSelectColor);
	}

void CPickerDisplay::SelectNext (void)

//	SelectNext
//
//	Select the next item

	{
	if (m_iSelection + 1 < m_pMenu->GetCount())
		{
		m_iSelection++;
		m_bInvalid = true;
		}
	}

void CPickerDisplay::SelectPrev (void)

//	SelectPrev
//
//	Select the previous item

	{
	if (m_iSelection > 0)
		{
		m_iSelection--;
		m_bInvalid = true;
		}
	}

void CPickerDisplay::Update (void)

//	Update
//
//	Update the off-screen buffer

	{
	int i;

	if (!m_bInvalid || m_pMenu == NULL)
		return;

	//	Remember the previous first in view

	int iPrevFirstInView = m_iFirstInView;

	//	Clear the area

	m_Buffer.Set(CG32bitPixel::Null());

	//	Figure out how many items we could show

	int iMaxCount = m_Buffer.GetWidth() / (TILE_WIDTH + TILE_SPACING_X);
	if ((iMaxCount % 2) == 0)
		iMaxCount--;

	//	Figure out which items we're showing

	int iLeft = Max(0, m_iSelection - (iMaxCount / 2));
	int iRight = Min(m_pMenu->GetCount() - 1, m_iSelection + (iMaxCount / 2));

	//	Figure out how many items we're showing

	int iCount = (iRight - iLeft) + 1;

	//	Adjust

	if ((iRight < m_pMenu->GetCount() - 1) && iCount < iMaxCount)
		{
		iRight = Min(m_pMenu->GetCount() - 1, iRight + (iMaxCount - iCount));
		iCount = (iRight - iLeft) + 1;
		}

	if (iLeft > 0 && iCount < iMaxCount)
		{
		iLeft = Max(0, iLeft - (iMaxCount - iCount));
		iCount = (iRight - iLeft) + 1;
		}

	//	Figure out the rect

	int cxView = (iCount * (TILE_WIDTH + TILE_SPACING_X)) - TILE_SPACING_X;
	m_rcView.left = (m_Buffer.GetWidth() - cxView) / 2;
	m_rcView.right = m_rcView.left + cxView;
	m_rcView.top = TOP_BORDER;
	m_rcView.bottom = m_rcView.top + TILE_HEIGHT;

	m_iFirstInView = iLeft;
	m_iCountInView = (iRight - iLeft) + 1;

	//	If the view has scrolled, add a smooth scroll offset

	if (m_iFirstInView != iPrevFirstInView)
		m_cxSmoothScroll = (m_iFirstInView - iPrevFirstInView) * (TILE_WIDTH + TILE_SPACING_X);

	//	Clear out an area

	m_Buffer.Fill(m_rcView.left - (TILE_SPACING_X), 
			m_rcView.top - (TILE_SPACING_Y), 
			RectWidth(m_rcView) + (2 * TILE_SPACING_X),
			RectHeight(m_rcView) + (2 * TILE_SPACING_Y),
			m_pFonts->rgbBackground);

	//	Paint the items

	int x = m_rcView.left + m_cxSmoothScroll;
	for (i = iLeft; i <= iRight; i++)
		{
		//	Paint a background, if selected

		if (i == m_iHover)
			m_Buffer.Fill(x, m_rcView.top, TILE_WIDTH, TILE_HEIGHT, RGB_HOVER_BACKGROUND);

		//	Paint the item

		if (m_pMenu->GetItemFlags(i) & CMenuData::FLAG_GRAYED)
			m_pMenu->GetItemImage(i)->PaintImageGrayed(m_Buffer,
					x + (TILE_WIDTH / 2),
					m_rcView.top + (TILE_HEIGHT / 2),
					0,
					0);
		else
			m_pMenu->GetItemImage(i)->PaintImage(m_Buffer,
					x + (TILE_WIDTH / 2),
					m_rcView.top + (TILE_HEIGHT / 2),
					0,
					0);

		//	Paint the number of items

		CString sExtra = m_pMenu->GetItemExtra(i);
		if (!sExtra.IsBlank() && !m_sHelpText.IsBlank())
			{
			int cyExtra;
			int cxExtra = m_pFonts->LargeBold.MeasureText(sExtra, &cyExtra);

			m_pFonts->LargeBold.DrawText(m_Buffer,
					x + TILE_WIDTH - cxExtra - TILE_SPACING_X,
					m_rcView.top + TILE_HEIGHT - cyExtra,
					RGB_EXTRA,
					sExtra);
			}

		//	Paint the hotkey

		CString sKey = m_pMenu->GetItemKey(i);
		if (!sKey.IsBlank())
			{
			int cyKey;
			int cxKey = m_pFonts->LargeBold.MeasureText(sKey, &cyKey);

			int xKey = x + TILE_SPACING_X;
			int yKey = m_rcView.top + TILE_SPACING_Y;
			m_Buffer.Fill(xKey, yKey, cxKey + 4, cyKey, m_pFonts->rgbTitleColor);

			m_pFonts->LargeBold.DrawText(m_Buffer,
					xKey + 2,
					yKey,
					m_pFonts->rgbBackground,
					sKey);
			}

		//	If this item is selected, paint the selection

		if (i == m_iSelection)
			PaintSelection(m_Buffer, x, m_rcView.top);

		x += TILE_WIDTH + TILE_SPACING_X;
		}

	//	Compute the sizes of the labels

	CString sTitle = m_pMenu->GetItemLabel(m_iSelection);
	int cxTitle = m_pFonts->Header.MeasureText(sTitle);

	CString sHelp = (m_sHelpText.IsBlank() ? m_pMenu->GetItemExtra(m_iSelection) : m_sHelpText);
	int cxHelp = m_pFonts->Medium.MeasureText(sHelp);

	int cxBack = Max(cxTitle, cxHelp) + 2 * TILE_SPACING_X;

	//	Paint the name of the selected item

	m_Buffer.Fill((m_Buffer.GetWidth() - cxBack) / 2,
			m_rcView.bottom + TILE_SPACING_Y,
			cxBack,
			m_Buffer.GetHeight() - (m_rcView.bottom + TILE_SPACING_Y),
			m_pFonts->rgbBackground);

	m_pFonts->Header.DrawText(m_Buffer,
			(m_Buffer.GetWidth() - cxTitle) / 2,
			m_rcView.bottom + TILE_SPACING_Y,
			m_pFonts->rgbTitleColor,
			sTitle);

	//	Paint help text (if help text is blank, then use menu extra
	//	string for help).

	m_pFonts->Medium.DrawText(m_Buffer,
			(m_Buffer.GetWidth() - cxHelp) / 2,
			m_rcView.bottom + TILE_SPACING_Y + m_pFonts->Header.GetHeight(),
			m_pFonts->rgbHelpColor,
			sHelp);

	//	Update smooth scroll

	if (m_cxSmoothScroll != 0)
		{
		m_cxSmoothScroll = m_cxSmoothScroll / 2;
		m_bInvalid = true;
		}
	else
		m_bInvalid = false;
	}
