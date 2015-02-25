//	CDeviceCounterDisplay.cpp
//
//	CDeviceCounterDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define ICON_WIDTH								96
#define ICON_HEIGHT								96
#define BAR_HEIGHT								14
#define SPACING_X								8

#define RGB_TITLE								(CG32bitPixel(150,255,180))

CDeviceCounterDisplay::CDeviceCounterDisplay (void) : m_pPlayer(NULL),
		m_bInvalid(true),
		m_bEmpty(true)

//	CDeviceCounterDisplay constructor

	{
	}

CDeviceCounterDisplay::~CDeviceCounterDisplay (void)

//	CDeviceCounterDisplay destructor

	{
	CleanUp();
	}

void CDeviceCounterDisplay::CleanUp (void)

//	CleanUp
//
//	Frees up all resources

	{
	m_pPlayer = NULL;
	m_bInvalid = false;
	m_bEmpty = true;
	}

ALERROR CDeviceCounterDisplay::Init (CPlayerShipController *pPlayer, const RECT &rcRect)

//	Init
//
//	Initializes display

	{
	CleanUp();

	m_pPlayer = pPlayer;
	m_rcRect = rcRect;
	m_bInvalid = true;

	return NOERROR;
	}

void CDeviceCounterDisplay::Paint (CG32bitImage &Dest)

//	Paint
//
//	Paints to the screen

	{
	if (m_bInvalid)
		Update();

	if (m_bEmpty)
		return;

	Dest.Blt(0,
			0,
			RectWidth(m_rcBuffer),
			RectHeight(m_rcBuffer),
			200,
			m_Buffer,
			m_rcRect.left + (RectWidth(m_rcRect) - RectWidth(m_rcBuffer)) / 2,
			m_rcRect.bottom - RectHeight(m_rcBuffer));
	}

void CDeviceCounterDisplay::PaintDevice (CInstalledDevice *pDevice, int x)

//	PaintDevice
//
//	Paints the device at the given location

	{
	CShip *pShip = m_pPlayer->GetShip();

	//	Paint the icon

	pDevice->GetClass()->GetItemType()->GetImage().PaintImage(m_Buffer, 
			x + ICON_WIDTH / 2, 
			ICON_HEIGHT / 2, 
			0, 0);

	//	Figure out title and level colors

	CDeviceClass::CounterTypes iType;
	int iLevel = pDevice->GetCounter(pShip, &iType);

	CString sTitle;
    CG32bitPixel rgbLevelColor;
	CG32bitPixel rgbBackColor;

	switch (iType)
		{
		case CDeviceClass::cntTemperature:
			{
			sTitle = CONSTLIT("Temperature");
			if (iLevel > 80)
				{
				rgbLevelColor = CG32bitPixel(255, 0, 0);
				rgbBackColor = CG32bitPixel(64, 0, 0);
				}
			else
				{
				rgbLevelColor = CG32bitPixel(255, 255, 0);
				rgbBackColor = CG32bitPixel(64, 64, 64);
				}
			break;
			}

		case CDeviceClass::cntCapacitor:
			{
			sTitle = CONSTLIT("Charging");
			if (iLevel < 20)
				{
				rgbLevelColor = CG32bitPixel(255, 0, 0);
				rgbBackColor = CG32bitPixel(64, 0, 0);
				}
			else
				{
				rgbLevelColor = CG32bitPixel(0, 255, 0);
				rgbBackColor = CG32bitPixel(64, 64, 64);
				}
			break;
			}

		default:
			rgbLevelColor = CG32bitPixel(255, 255, 0);
			rgbBackColor = CG32bitPixel(64, 64, 64);
		}

	//	Paint title

	int cxWidth = m_pFonts->Medium.MeasureText(sTitle);
	m_Buffer.DrawText(x + (ICON_WIDTH - cxWidth) / 2,
			ICON_HEIGHT,
			m_pFonts->Medium,
			RGB_TITLE,
			sTitle);

	//	Paint the level

	int cxLevelWidth = ICON_WIDTH * iLevel / 100;
	int cxBackWidth = ICON_WIDTH - cxLevelWidth;
	int y = m_pFonts->Medium.GetHeight() + ICON_HEIGHT;

	m_Buffer.Fill(x, y, cxLevelWidth, BAR_HEIGHT, rgbLevelColor);
	m_Buffer.Fill(x + cxLevelWidth, y, cxBackWidth, BAR_HEIGHT, rgbBackColor);
	}

void CDeviceCounterDisplay::Update (void)

//	Update
//
//	Updates buffer

	{
	int i;

	if (m_pPlayer == NULL)
		{
		m_bInvalid = false;
		m_bEmpty = true;
		return;
		}

	//	Loop over all devices and see if any of them need a counter

	CShip *pShip = m_pPlayer->GetShip();
	int iCount = 0;
	for (i = 0; i < pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = pShip->GetDevice(i);
		if (!pDevice->IsEmpty() && pDevice->GetCounter(pShip))
			iCount++;
		}

	//	If no devices need a counter, then we're done

	if (iCount == 0)
		{
		m_bInvalid = false;
		m_bEmpty = true;
		return;
		}

	//	Compute how big we need the bitmap to be. If necessary, allocate a new
	//	bitmap.

	int cxWidth = (ICON_WIDTH * iCount) + (SPACING_X * (iCount - 1));
	int cyHeight = m_pFonts->Medium.GetHeight() + ICON_HEIGHT + BAR_HEIGHT;

	if (cxWidth != m_Buffer.GetWidth())
		{
		if (!m_Buffer.Create(cxWidth, cyHeight, CG32bitImage::alpha8))
			{
			m_bInvalid = false;
			m_bEmpty = true;
			return;
			}

		//m_Buffer.SetBlending(200);

		m_rcBuffer.left = 0;
		m_rcBuffer.right = cxWidth;
		m_rcBuffer.top = 0;
		m_rcBuffer.bottom = cyHeight;
		}

	m_Buffer.Set(CG32bitPixel::Null());

	//	Paint all device counters

	int x = 0;
	for (i = 0; i < pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = pShip->GetDevice(i);
		if (!pDevice->IsEmpty() && pDevice->GetCounter(pShip))
			{
			PaintDevice(pDevice, x);

			x += ICON_WIDTH + SPACING_X;
			}
		}

	//	Done

	m_bInvalid = false;
	m_bEmpty = false;
	}

