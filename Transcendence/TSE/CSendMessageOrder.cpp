//	CSendMessageOrder.cpp
//
//	CSendMessageOrder class implementation
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

void CSendMessageOrder::OnBehavior (CShip *pShip, CAIBehaviorCtx &Ctx)

//	OnBehavior
//
//	Do it

	{
	//	Get the current order data

	CSpaceObject *pRecipient;
	IShipController::SData Data;
	pShip->GetCurrentOrder(&pRecipient, &Data);

	//	Send the message

	if (pRecipient && Data.sData)
		pRecipient->SendMessage(pShip, Data.sData);

	//	Done with the order.

	pShip->CancelCurrentOrder();
	}

