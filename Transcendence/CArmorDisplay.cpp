//	CArmorDisplay.cpp
//
//	CArmorDisplay class

#include "PreComp.h"
#include "Transcendence.h"

CArmorDisplay::CArmorDisplay (void) : 
		m_pPlayer(NULL),
		m_iSelection(-1),
		m_pArmorPainter(NULL),
		m_pShieldsPainter(NULL),

		m_dwCachedShipID(0),
		m_pShieldPainter(NULL)

//	CArmorDisplay constructor

	{
	}

CArmorDisplay::~CArmorDisplay (void)

//	CArmorDisplay destructor

	{
	CleanUp();
	}

void CArmorDisplay::CleanUp (void)

//	CleanUp
//
//	Delete relevant stuff

	{
	if (m_pArmorPainter)
		{
		delete m_pArmorPainter;
		m_pArmorPainter = NULL;
		}

	if (m_pShieldsPainter)
		{
		delete m_pShieldsPainter;
		m_pShieldsPainter = NULL;
		}

	if (m_pShieldPainter)
		{
		m_pShieldPainter->Delete();
		m_pShieldPainter = NULL;
		}

	m_pPlayer = NULL;
	}

ALERROR CArmorDisplay::Init (CPlayerShipController *pPlayer, const RECT &rcRect)

//	Init
//
//	Initialize

	{
	CleanUp();

	m_pPlayer = pPlayer;
	m_rcRect = rcRect;

	//	Create the two painters

	CShip *pShip = m_pPlayer->GetShip();

	SDesignLoadCtx Ctx;
	m_pArmorPainter = IHUDPainter::Create(Ctx, pShip->GetClass(), IHUDPainter::hudArmor);
	m_pShieldsPainter = IHUDPainter::Create(Ctx, pShip->GetClass(), IHUDPainter::hudShields);

	return NOERROR;
	}

void CArmorDisplay::Paint (CG32bitImage &Dest)

//	Paint
//
//	Paints to the destination

	{
	if (m_pPlayer == NULL
			|| m_pArmorPainter == NULL)
		return;

	SHUDPaintCtx PaintCtx;
	PaintCtx.pSource = m_pPlayer->GetShip();
	PaintCtx.iSegmentSelected = m_iSelection;
	PaintCtx.pShieldsHUD = m_pShieldsPainter;

	m_pArmorPainter->Paint(Dest, m_rcRect.left, m_rcRect.top, PaintCtx);
	}

void CArmorDisplay::SetSelection (int iSelection)

//	SetSelection
//
//	Selects an armor segment

	{
	if (iSelection != m_iSelection)
		{
		m_iSelection = iSelection;
		Update();
		}
	}

void CArmorDisplay::Update (void)

//	Update
//
//	Updates buffer from data

	{
	if (m_pArmorPainter)
		m_pArmorPainter->Invalidate();

	if (m_pShieldsPainter)
		m_pShieldsPainter->Invalidate();
	}
