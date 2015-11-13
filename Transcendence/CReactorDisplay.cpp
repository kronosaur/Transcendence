//	CReactorDisplay.cpp
//
//	CReactorDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define FUEL_MARKER_COUNT			20
#define FUEL_MARKER_HEIGHT			5
#define FUEL_MARKER_WIDTH			12
#define FUEL_MARKER_GAP				1

#define TEXT_COLOR					CG32bitPixel(150,180,255)
#define DAMAGED_TEXT_COLOR			CG32bitPixel(128,0,0)

CReactorDisplay::CReactorDisplay (void) : m_pPlayer(NULL),
		m_iTickCount(0),
		m_pFonts(NULL),
		m_iOverloading(0)

//	CReactorDisplay contructor

	{
	}

CReactorDisplay::~CReactorDisplay (void)

//	CReactorDisplay destructor

	{
	CleanUp();
	}

void CReactorDisplay::CleanUp (void)

//	CleanUp
//
//	Delete relevant stuff

	{
	m_pPlayer = NULL;
	}

ALERROR CReactorDisplay::Init (CPlayerShipController *pPlayer, const RECT &rcRect)

//	Init
//
//	Initialize

	{
	CleanUp();

	m_pPlayer = pPlayer;
	m_rcRect = rcRect;

	//	Create the off-screen buffer

	if (!m_Buffer.Create(RectWidth(rcRect), RectHeight(rcRect), CG32bitImage::alpha8))
		return ERR_FAIL;

	//m_Buffer.SetTransparentColor();

	return NOERROR;
	}

void CReactorDisplay::Paint (CG32bitImage &Dest)

//	Paint
//
//	Paints the display

	{
	Dest.Blt(0,
			0,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			255,
			m_Buffer,
			m_rcRect.left,
			m_rcRect.top);
	}

void CReactorDisplay::Update (void)

//	Update
//
//	Updates the buffer from data

	{
	m_Buffer.Set(CG32bitPixel::Null());

	if (m_pPlayer == NULL)
		return;

	CShip *pShip = m_pPlayer->GetShip();
	const CPlayerSettings *pSettings = pShip->GetClass()->GetPlayerSettings();
	const SReactorImageDesc &ReactorDesc = pSettings->GetReactorDesc();

	int yOffset = (RectHeight(m_rcRect) - RectHeight(ReactorDesc.ReactorImage.GetImageRect())) / 2;

	//	Paint the background

	ReactorDesc.ReactorImage.PaintImageUL(m_Buffer, 0, yOffset, 0, 0);

	//	Calculate fuel values

	Metric rMaxFuel = pShip->GetMaxFuel();
	Metric rFuelLeft = Min(pShip->GetFuelLeft(), rMaxFuel);
	int iFuelLevel = (rMaxFuel > 0.0 ? ((int)Min((rFuelLeft * 100.0 / rMaxFuel) + 1.0, 100.0)) : 0);

	//	Paint the fuel level

	CG32bitImage *pFuelImage = NULL;
	RECT rcSrcRect;
	bool bBlink;
	if (iFuelLevel < 15)
		{
		pFuelImage = &ReactorDesc.FuelLowLevelImage.GetImage(NULL_STR);
		rcSrcRect = ReactorDesc.FuelLowLevelImage.GetImageRect();
		bBlink = true;
		}
	else
		{
		pFuelImage = &ReactorDesc.FuelLevelImage.GetImage(NULL_STR);
		rcSrcRect = ReactorDesc.FuelLevelImage.GetImageRect();
		bBlink = false;
		}

	if (!bBlink || ((m_iTickCount % 2) == 0))
		{
		int cxFuelLevel = RectWidth(rcSrcRect) * iFuelLevel / 100;
		m_Buffer.Blt(rcSrcRect.left,
				rcSrcRect.top,
				cxFuelLevel,
				RectHeight(rcSrcRect),
				255,
				*pFuelImage,
				ReactorDesc.xFuelLevelImage,
				yOffset + ReactorDesc.yFuelLevelImage);
		}

	//	Paint fuel level text

	m_Buffer.Set(ReactorDesc.rcFuelLevelText.left,
			yOffset + ReactorDesc.rcFuelLevelText.top,
			RectWidth(ReactorDesc.rcFuelLevelText),
			RectHeight(ReactorDesc.rcFuelLevelText),
			CG32bitPixel::Null());

	CString sFuelLevel = strPatternSubst(CONSTLIT("fuel"));
	int cxWidth = m_pFonts->Small.MeasureText(sFuelLevel);
	m_pFonts->Small.DrawText(m_Buffer,
			ReactorDesc.rcFuelLevelText.left,
			yOffset + ReactorDesc.rcFuelLevelText.top,
			m_pFonts->rgbHelpColor,
			sFuelLevel,
			0);

	//	Calculate the power level

	int iPowerUsage = pShip->GetPowerConsumption();
	int iMaxPower = pShip->GetMaxPower();
	int iPowerLevel;
	if (iMaxPower)
		iPowerLevel = Min((iPowerUsage * 100 / iMaxPower) + 1, 120);
	else
		iPowerLevel = 0;

	if (iPowerLevel >= 100)
		m_iOverloading++;
	else
		m_iOverloading = 0;

	bBlink = (m_iOverloading > 0);

	//	Paint the power level

	if (!bBlink || ((m_iOverloading % 2) == 1))
		{
		CG32bitImage *pPowerImage = &ReactorDesc.PowerLevelImage.GetImage(NULL_STR);
		rcSrcRect = ReactorDesc.PowerLevelImage.GetImageRect();
		int cxPowerLevel = RectWidth(rcSrcRect) * iPowerLevel / 120;
		m_Buffer.Blt(rcSrcRect.left,
				rcSrcRect.top,
				cxPowerLevel,
				RectHeight(rcSrcRect),
				255,
				*pPowerImage,
				ReactorDesc.xPowerLevelImage,
				yOffset + ReactorDesc.yPowerLevelImage);
		}

	//	Paint power level text

	m_Buffer.Set(ReactorDesc.rcPowerLevelText.left,
			yOffset + ReactorDesc.rcPowerLevelText.top,
			RectWidth(ReactorDesc.rcPowerLevelText),
			RectHeight(ReactorDesc.rcPowerLevelText),
			CG32bitPixel::Null());

	CString sPowerLevel = strPatternSubst(CONSTLIT("power usage"));
	int cyHeight;
	cxWidth = m_pFonts->Small.MeasureText(sPowerLevel, &cyHeight);
	m_pFonts->Small.DrawText(m_Buffer,
			ReactorDesc.rcPowerLevelText.left,
			yOffset + ReactorDesc.rcPowerLevelText.bottom - cyHeight,
			m_pFonts->rgbHelpColor,
			sPowerLevel,
			0);

	//	Paint the reactor name (we paint on top of the levels)

	CG32bitPixel rgbColor;
	if (pShip->GetReactorDesc()->fDamaged)
		rgbColor = DAMAGED_TEXT_COLOR;
	else
		rgbColor = m_pFonts->rgbTitleColor;

	CString sReactorName = strPatternSubst(CONSTLIT("%s (%s)"), 
			pShip->GetReactorName(),
			ReactorPower2String(iMaxPower));

	m_pFonts->Medium.DrawText(m_Buffer,
			ReactorDesc.rcReactorText.left,
			yOffset + ReactorDesc.rcReactorText.top,
			rgbColor,
			sReactorName,
			0);

	m_iTickCount++;
	}

