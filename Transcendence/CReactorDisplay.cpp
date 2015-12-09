//	CReactorDisplay.cpp
//
//	CReactorDisplay class

#include "PreComp.h"
#include "Transcendence.h"

CReactorDisplay::CReactorDisplay (void) : 
		m_pPlayer(NULL),
		m_pHUDPainter(NULL)

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

RECT CReactorDisplay::GetRect (void) const

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

ALERROR CReactorDisplay::Init (CPlayerShipController *pPlayer, const RECT &rcRect, DWORD dwLocation)

//	Init
//
//	Initialize

	{
	CleanUp();

	m_pPlayer = pPlayer;

	//	Create the painter

	CShip *pShip = m_pPlayer->GetShip();

	SDesignLoadCtx Ctx;
	m_pHUDPainter = IHUDPainter::Create(Ctx, pShip->GetClass(), hudReactor);
	if (m_pHUDPainter)
		m_pHUDPainter->SetLocation(rcRect, dwLocation);

	return NOERROR;
	}

void CReactorDisplay::Paint (CG32bitImage &Dest)

//	Paint
//
//	Paints the display

	{
	if (m_pPlayer == NULL
			|| m_pHUDPainter == NULL)
		return;

	SHUDPaintCtx PaintCtx;
	PaintCtx.pSource = m_pPlayer->GetShip();

	m_pHUDPainter->Paint(Dest, PaintCtx);
	}

void CReactorDisplay::Update (int iTick)

//	Update
//
//	Updates the buffer from data

	{
	if (m_pPlayer == NULL
			|| m_pHUDPainter == NULL)
		return;

	SHUDUpdateCtx UpdateCtx;
	UpdateCtx.pSource = m_pPlayer->GetShip();
	UpdateCtx.iTick = iTick;

	m_pHUDPainter->Update(UpdateCtx);
	}

