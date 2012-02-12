//	CInvokeDisplay.cpp
//
//	CInvokeDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define DISPLAY_WIDTH						200
#define DISPLAY_HEIGHT						300

#define BLOCK_WIDTH							18
#define BLOCK_HEIGHT						18

CInvokeDisplay::CInvokeDisplay (void) : m_pPlayer(NULL),
		m_bInvalid(true)

//	CInvokeDisplay constructor

	{
	}

CInvokeDisplay::~CInvokeDisplay (void)

//	CInvokeDisplay destructor

	{
	CleanUp();
	}

void CInvokeDisplay::CleanUp (void)

//	CleanUp
//
//	Frees up all resources

	{
	m_pPlayer = NULL;
	}

ALERROR CInvokeDisplay::Init (CPlayerShipController *pPlayer, const RECT &rcRect)

//	Init
//
//	Initializes display

	{
	ALERROR error;

	CleanUp();

	m_pPlayer = pPlayer;
	m_rcRect = rcRect;
	m_bInvalid = true;

	//	Create the off-screen buffer

	if (error = m_Buffer.CreateBlank(DISPLAY_WIDTH, DISPLAY_HEIGHT, false))
		return error;

	m_Buffer.SetTransparentColor(CG16bitImage::RGBValue(0,0,0));

	return NOERROR;
	}

void CInvokeDisplay::Paint (CG16bitImage &Dest)

//	Paint
//
//	Paint the buffer

	{
	Update();

	Dest.ColorTransBlt(0,
			0,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			255,
			m_Buffer,
			m_rcRect.left,
			m_rcRect.top);
	}

void CInvokeDisplay::Update (void)

//	Update
//
//	Update the buffer

	{
	if (!m_bInvalid)
		return;

	//	Who is our deity?

	int iDeity = m_pPlayer->GetDeity();

	//	Color depends on our deity

	COLORREF rgbColor;
	WORD wColor;
	WORD wFadeColor;
	CString sTitle;
	if (iDeity == deityDomina)
		{
		rgbColor = RGB(0, 160, 221);
		wColor = CG16bitImage::RGBValue(0, 160, 221);
		wFadeColor = CG16bitImage::RGBValue(0, 80, 110);
		sTitle = CONSTLIT("Invoke powers of Domina");
		}
	else
		{
		rgbColor = RGB(185, 0, 0);
		wColor = CG16bitImage::RGBValue(185, 0, 0);
		wFadeColor = CG16bitImage::RGBValue(92, 0, 0);
		sTitle = CONSTLIT("Invoke powers of Oracus");
		}

	//	Clear

	m_Buffer.Fill(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, CG16bitImage::RGBValue(0, 0, 0));

	//	Outline the box

	m_Buffer.DrawLine(0, 0, DISPLAY_WIDTH - 1, 0, 1, wColor);
	m_Buffer.DrawBiColorLine(0, 0, 0, DISPLAY_HEIGHT - 1, 1, wColor, wFadeColor);
	m_Buffer.DrawBiColorLine(DISPLAY_WIDTH - 1, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1, 1, wColor, wFadeColor);
	m_Buffer.DrawLine(0, DISPLAY_HEIGHT - 1, DISPLAY_WIDTH, DISPLAY_HEIGHT - 1, 1, wFadeColor);

	//	Paint the menu title

	m_pFonts->MediumHeavyBold.DrawText(m_Buffer,
			4,
			4,
			wColor,
			sTitle);

	//	Make sure the power table is up-to-date

	m_pPlayer->UpdatePowerTable();

	//	Paint each menu item

	int i;
	int x = 40;
	int y = 32;
	for (i = 0; i < m_pPlayer->GetPowerCount(); i++)
		{
		CPower *pPower = m_pPlayer->GetPower(i);

		//	Draw the box and the character

		m_Buffer.FillRGB(x, y, BLOCK_WIDTH, BLOCK_HEIGHT, rgbColor);

		int cyHeight;
		int cxWidth = m_pFonts->Medium.MeasureText(pPower->GetInvokeKey(), &cyHeight);
		m_pFonts->Medium.DrawText(m_Buffer,
				x + (BLOCK_WIDTH - cxWidth) / 2,
				y + 1,
				CG16bitImage::RGBValue(0, 0, 0),
				pPower->GetInvokeKey());

		//	Draw the name

		m_pFonts->Medium.DrawText(m_Buffer,
				x + BLOCK_WIDTH + 4,
				y + 1,
				wColor,
				pPower->GetName());

		//	Next

		y += BLOCK_HEIGHT + 2;
		}

	m_bInvalid = false;
	}
