//	CIntroShipController.cpp
//
//	Implements class to control ships on intro screen

#include "PreComp.h"
#include "Transcendence.h"

#define CMD_INTRO_SHIP_DESTROYED				CONSTLIT("introShipDestroyed")

CIntroShipController::CIntroShipController (void)

//	CIntroShipController constructor

	{
	}

CIntroShipController::~CIntroShipController (void)

//	CIntroShipController destructor

	{
	if (m_pDelegate)
		delete dynamic_cast<CObject *>(m_pDelegate);
	}

CIntroShipController::CIntroShipController (IShipController *pDelegate) : 
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
		{
		IHISession *pSession = g_pHI->GetSession();
		if (pSession)
			pSession->HICommand(CMD_INTRO_SHIP_DESTROYED, m_pShip);
		}
	}
