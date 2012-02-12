//	CIntroShipController.cpp
//
//	Implements class to control ships on intro screen

#include "PreComp.h"
#include "Transcendence.h"

CIntroShipController::CIntroShipController (void) : CObject(NULL)

//	CIntroShipController constructor

	{
	}

CIntroShipController::~CIntroShipController (void)

//	CIntroShipController destructor

	{
	if (m_pDelegate)
		delete dynamic_cast<CObject *>(m_pDelegate);
	}

CIntroShipController::CIntroShipController (CTranscendenceWnd *pTrans, IShipController *pDelegate) : CObject(NULL),
		m_pTrans(pTrans),
		m_pDelegate(pDelegate)

//	CIntroShipController constructor

	{
	}

void CIntroShipController::OnDestroyed (SDestroyCtx &Ctx)

//	OnDestroyed
//
//	Handle on destroyed

	{
	if (Ctx.iCause != removedFromSystem)
		m_pTrans->CreateIntroShips(0, 0, m_pShip);
	}
