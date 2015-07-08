//	CLRSDisplay.cpp
//
//	CLRSDisplay class
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

void CLRSDisplay::CleanUp (void)

//	CleanUp
//
//	Clean up from a session

	{
	m_pPlayer = NULL;
	}

ALERROR CLRSDisplay::Init (CPlayerShipController *pPlayer, const RECT &rcRect)

//	Init
//
//	Initialize the display

	{
	//	Initialize

	int cxWidth = RectWidth(rcRect);
	int cyHeight = RectHeight(rcRect);
	m_pPlayer = pPlayer;
	m_rcRect = rcRect;
	m_iDiameter = Min(cxWidth, cyHeight);

	//	Create a buffer

	if (!m_Buffer.Create(cxWidth, cyHeight, CG32bitImage::alpha8))
		return ERR_FAIL;

	//	Create a circular mask for the LRS

	if (!m_Mask.Create(cxWidth, cyHeight, 0xff))
		return ERR_FAIL;

	CGDraw::Circle(m_Mask, (m_iDiameter / 2), (m_iDiameter / 2), (m_iDiameter / 2), 0x00);

	//	Done

	return NOERROR;
	}

void CLRSDisplay::Paint (CG32bitImage &Dest)

//	Paint
//
//	Paints the display

	{
	Dest.Blt(0,
			0,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			200,
			m_Buffer,
			m_rcRect.left,
			m_rcRect.top);
	}

void CLRSDisplay::Update (void)

//	Update
//
//	Updates the display

	{
	DEBUG_TRY

	bool bNewEnemies;
	int cxWidth = RectWidth(m_rcRect);
	int cyHeight = RectHeight(m_rcRect);

	if (m_pBackground)
		CGDraw::BltTiled(m_Buffer, 0, 0, cxWidth, cyHeight, *m_pBackground);
	else
		m_Buffer.Fill(m_rgbBackground);

	//	If we're not blind, paint the LRS

	if (m_pPlayer == NULL 
			|| !m_pPlayer->GetShip()->IsLRSBlind())
		{
		RECT rcView;
		rcView.left = 0;
		rcView.top = 0;
		rcView.right = cxWidth;
		rcView.bottom = cyHeight;

		Metric rKlicksPerPixel = g_LRSRange * 2 / RectWidth(rcView);
		g_pUniverse->PaintPOVLRS(m_Buffer, rcView, rKlicksPerPixel, 0, &bNewEnemies);

		//	Notify player of enemies

		if (bNewEnemies && m_pPlayer)
			m_pPlayer->OnEnemyShipsDetected();
		}

	//	If we're blind, paint snow

	else
		{
		if (m_pSnow)
			CGDraw::BltTiled(m_Buffer, 0, 0, cxWidth, cyHeight, *m_pSnow);

		int iCount = mathRandom(1, 8);
		for (int i = 0; i < iCount; i++)
			{
			m_Buffer.Fill(0, mathRandom(0, cyHeight),
					cxWidth,
					mathRandom(1, 20),
					CG32bitPixel(108, 252, 128));
			}
		}

	//	Mask out the border

	m_Buffer.SetMask(0,
			0,
			cxWidth,
			cyHeight,
			m_Mask,
			CG32bitPixel::Null(),
			0,
			0);

	DEBUG_CATCH
	}
