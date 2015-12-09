//	CTargetDisplay.cpp
//
//	CTargetDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define DISPLAY_WIDTH						360
#define DISPLAY_HEIGHT						120

#define PRIMARY_WEAPON_X					300
#define PRIMARY_WEAPON_Y					80
#define MISSILE_WEAPON_X					300
#define MISSILE_WEAPON_Y					100

#define TARGET_IMAGE_X						60
#define TARGET_IMAGE_Y						60

#define TARGET_NAME_X						122
#define TARGET_NAME_Y						27
#define TARGET_NAME_COLOR					(CG32bitPixel(0,128,0))

#define TARGET_INFO_COLOR					(CG32bitPixel(0,255,0))

#define DEVICE_STATUS_HEIGHT				20

#define DEVICE_LABEL_COLOR					(CG32bitPixel(255,255,255))
#define DEVICE_LABEL_FAINT_COLOR			(CG32bitPixel(0,0,0))
#define DISABLED_LABEL_COLOR				CG32bitPixel(128,0,0)

#define STR_UNKNOWN_HOSTILE					CONSTLIT("Unknown Hostile")
#define STR_UNKNOWN_FRIENDLY				CONSTLIT("Unknown Friendly")

CTargetDisplay::CTargetDisplay (void) : 
		m_pPlayer(NULL),
		m_pHUDPainter(NULL)

//	CTargetDisplay constructor

	{
	}

CTargetDisplay::~CTargetDisplay (void)

//	CTargetDisplay destructor

	{
	CleanUp();
	}

void CTargetDisplay::CleanUp (void)

//	CleanUp
//
//	Frees up all resources

	{
	if (m_pHUDPainter)
		{
		delete m_pHUDPainter;
		m_pHUDPainter = NULL;
		}

	m_pPlayer = NULL;
	}

RECT CTargetDisplay::GetRect (void) const

//	GetRect
//
//	Returns the RECT for the display. Only valid after Init.

	{
	RECT rcRect;

	if (m_pHUDPainter == NULL)
		{
		rcRect.left = 0;
		rcRect.right = 0;
		rcRect.top = 0;
		rcRect.bottom = 0;
		}
	else
		m_pHUDPainter->GetRect(&rcRect);

	return rcRect;
	}

ALERROR CTargetDisplay::Init (CPlayerShipController *pPlayer, const RECT &rcRect, DWORD dwLocation)

//	Init
//
//	Initializes display

	{
	CleanUp();

	m_pPlayer = pPlayer;

	//	Create the painter

	CShip *pShip = m_pPlayer->GetShip();

	SDesignLoadCtx Ctx;
	m_pHUDPainter = IHUDPainter::Create(Ctx, pShip->GetClass(), hudTargeting);
	if (m_pHUDPainter)
		m_pHUDPainter->SetLocation(rcRect, dwLocation);

	return NOERROR;
	}

void CTargetDisplay::Paint (CG32bitImage &Dest)

//	Paint
//
//	Paints to the screen

	{
	if (m_pPlayer == NULL
			|| m_pHUDPainter == NULL)
		return;

	SHUDPaintCtx PaintCtx;
	PaintCtx.pSource = m_pPlayer->GetShip();

	m_pHUDPainter->Paint(Dest, PaintCtx);
	}

