//	CWaitOrder.cpp
//
//	CWaitOrder class implementation
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

void CWaitOrder::OnBehavior (CShip *pShip, CAIBehaviorCtx &Ctx)

//	OnBehavior
//
//	Do it

	{
	//	Handle waiting

	Ctx.ImplementHold(pShip);
	Ctx.ImplementFireOnTargetsOfOpportunity(pShip);

	//	See if our timer has expired

	if (m_iCountdown != -1 && m_iCountdown-- == 0)
		pShip->CancelCurrentOrder();

	//	Otherwise, see if other conditions are fullfilled.

	else
		{
		switch (m_iType)
			{
			case waitForUndock:
				if (m_Objs[objTarget]->GetDockedObj() == NULL)
					pShip->CancelCurrentOrder();
				break;
			}
		}
	}

void CWaitOrder::OnBehaviorStart (CShip *pShip, CAIBehaviorCtx &Ctx, CSpaceObject *pOrderTarget, const IShipController::SData &Data)

//	OnBehaviorStart
//
//	Initialize order module

	{
	//	Set target

	switch (m_iType)
		{
		case waitForUndock:
			m_Objs[objTarget] = pOrderTarget;
			ASSERT(m_Objs[objTarget]->DebugIsValid() && m_Objs[objTarget]->NotifyOthersWhenDestroyed());
			break;
		}

	//	See if we have a time limit

	DWORD dwTimer = Data.AsInteger();
	m_iCountdown = (dwTimer ? 1 + (g_TicksPerSecond * dwTimer) : -1);
	}

void CWaitOrder::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Load data from saved game

	{
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iType = (EWaitTypes)dwLoad;

	Ctx.pStream->Read((char *)&m_iCountdown, sizeof(DWORD));
	}

void CWaitOrder::OnWriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	OnWriteToStream
//
//	Write data to saved game

	{
	DWORD dwSave;

	dwSave = (DWORD)m_iType;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_iCountdown, sizeof(DWORD));
	}
