//	CFireEventOrder.cpp
//
//	CFireEventOrder class implementation
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

void CFireEventOrder::OnBehavior (CShip *pShip, CAIBehaviorCtx &Ctx)

//	OnBehavior
//
//	Do it

	{
	//	Get the current order data

	CSpaceObject *pListener;
	IShipController::SData Data;
	pShip->GetCurrentOrder(&pListener, &Data);

	//	Done with the order.
	//
	//	NOTE: This will delete our object, so we can't access any member variables after this.

	pShip->CancelCurrentOrder();

	//	Fire the event

	if (pListener && !Data.sData.IsBlank())
		pListener->FireCustomShipOrderEvent(Data.sData, pShip);
	}

