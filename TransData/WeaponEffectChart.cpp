//	WeaponEffectChart.cpp
//
//	Generate a poster of weapon effects.
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define CRITERIA_ATTRIB						CONSTLIT("criteria")

#define STYLE_TITLE							CONSTLIT("title")

const int ITEM_ICON_WIDTH =					96;
const int ITEM_ICON_HEIGHT =				96;
const DWORD PLAYER_SOVEREIGN_UNID =			0x00001001;
const DWORD REACTOR_UNID =					0x0000410A;
const DWORD TARGET_UNID =					0x00020032;
const DWORD WEAPON_PLATFORM_UNID =			0x00003008;

void PaintWeaponFrames (CG32bitImage &Image, CItemType *pType, CShip *pPlatform, int iFrames, int x, int y, int cxCell, int cyCell);

void GenerateWeaponEffectChart (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i;

	//	Compute the list of weapons to show, making sure we filter to weapons
	//	and missiles only.

	CItemTypeTable Selection;
	if (!Selection.Filter(pCmdLine->GetAttribute(CRITERIA_ATTRIB))
			|| !Selection.Filter(CONSTLIT("wm")))
		{
		printf("No entries match criteria.\n");
		return;
		}

	Selection.Sort();

	//	Compute some metrics

	int iFramesPerItem = 10;
	int cxFrameHorzMargin = 10;
	int cxMaxDistPerTick = (int)(STD_SECONDS_PER_UPDATE * (LIGHT_SECOND / g_KlicksPerPixel));
	int cyFrame = 64;
	int cxFrame = (2 * cxFrameHorzMargin) + (iFramesPerItem * cxMaxDistPerTick);

	int iHitEffectFramesPerItem = 5;
	int cxHitEffect = 128;
	int cyHitEffect = 128;
	
	int cyRowTitle = 20;
	int cyRow = cyRowTitle + Max(ITEM_ICON_HEIGHT, cyFrame * iFramesPerItem);
	int cxRow = ITEM_ICON_WIDTH + cxFrame;

	int cxImage = cxRow;
	int cyImage = cyRow * Selection.GetCount();

	//	Initialize the output

	COutputChart Output;
	Output.SetContentSize(cxImage, cyImage);
	Output.SetOutputFilespec(pCmdLine->GetAttribute(CONSTLIT("output")));

	//	Initialize fonts

	Output.SetStyleFont(STYLE_TITLE, pCmdLine->GetAttribute(CONSTLIT("font")));
	Output.SetStyleColor(STYLE_TITLE, CG32bitPixel(0xFF, 0xFF, 0xFF));

	//	Prepare the universe

	CSystem *pSystem;
	if (Universe.CreateEmptyStarSystem(&pSystem) != NOERROR)
		{
		printf("ERROR: Unable to create empty star system.\n");
		return;
		}

	//	Create a target in the center of the system

	CSpaceObject *pStation;
	CStationType *pTargetType = Universe.FindStationType(TARGET_UNID);
	if (pTargetType == NULL 
			|| pSystem->CreateStation(pTargetType, NULL, CVector(), &pStation) != NOERROR)
		{
		printf("ERROR: Unable to create station.\n");
		return;
		}

	//	Create the weapon platform some distance away

	CSovereign *pPlatformSovereign = Universe.FindSovereign(PLAYER_SOVEREIGN_UNID);
	CShip *pPlatform;
	if (pPlatformSovereign == NULL
				|| pSystem->CreateShip(WEAPON_PLATFORM_UNID,
					NULL,
					NULL,
					pPlatformSovereign,
					CVector(-5.0 * LIGHT_SECOND, 0.),
					CVector(),
					0,
					NULL,
					NULL,
					&pPlatform) != NOERROR)
		{
		printf("ERROR: Unable to create weapons platform.\n");
		return;
		}

	//	Set the attacker to hold

	IShipController *pController = pPlatform->GetController();
	if (pController == NULL)
		{
		printf("ERROR: No controller for ship.\n");
		return;
		}

	pController->AddOrder(IShipController::orderHold, NULL, IShipController::SData());
	pPlatform->SetControllerEnabled(false);

	//	Install the largest possible reactor on the ship

	CItem ReactorItem(Universe.FindItemType(REACTOR_UNID), 1);

	CItemListManipulator ItemList(pPlatform->GetItemList());
	ItemList.AddItem(ReactorItem);
	pPlatform->OnComponentChanged(comCargo);
	pPlatform->ItemsModified();
	pPlatform->InvalidateItemListAddRemove();

	pPlatform->InstallItemAsDevice(ItemList);

	//	Set the POV

	Universe.SetPOV(pStation);
	pSystem->SetPOVLRS(pStation);

	//	Prepare system

	Universe.UpdateExtended();
	Universe.GarbageCollectLibraryBitmaps();
	Universe.StartGame(true);

	//	Output each weapon

	int xOrigin;
	int yOrigin;
	CG32bitImage &Image = Output.GetOutputImage(&xOrigin, &yOrigin);
	const CG16bitFont &TitleFont = Output.GetStyleFont(STYLE_TITLE);
	CG32bitPixel rgbTitleColor = Output.GetStyleColor(STYLE_TITLE);

	for (i = 0; i < Selection.GetCount(); i++)
		{
		CItemType *pType = Selection.GetItemType(i);

		//	Compute the metrics of this row

		int xRow = xOrigin;
		int yRow = yOrigin + (i * cyRow);

		//	Paint the weapon title

		Image.Fill(xRow, yRow, cxRow, cyRow, CG32bitPixel(0x40, 0x40, 0x40));
		TitleFont.DrawText(Image, xRow + 8, yRow, rgbTitleColor, pType->GetNounPhrase());

		//	Paint the frames

		PaintWeaponFrames(Image, pType, pPlatform, iFramesPerItem, 
				xRow + ITEM_ICON_WIDTH + cxFrameHorzMargin,
				yRow + cyRowTitle, 
				cxMaxDistPerTick,
				cyFrame);
		}

	//	Done

	Output.Output();
	}

void PaintWeaponFrames (CG32bitImage &Image, CItemType *pType, CShip *pPlatform, int iFrames, int x, int y, int cxCell, int cyCell)
	{
	int i, j;

	Metric rFireDist = 18.0 * LIGHT_SECOND;

	//	Make sure we're refueled and in position

	pPlatform->Place(CVector(-rFireDist, 0.0));
	pPlatform->Refuel(pPlatform->GetMaxFuel());
	CItemListManipulator ItemList(pPlatform->GetItemList());

	//	Get the device and weapon fire desc for this item. The call will do the
	//	right thing if this is a missile.

	CDeviceClass *pWeapon;
	int iVariant;
	CWeaponFireDesc *pDesc;
	if (!CDeviceClass::FindWeaponFor(pType, &pWeapon, &iVariant, &pDesc))
		return;

	//	Compute the number of ticks that we need to cover the distance

	Metric rDist = rFireDist - (60 * g_KlicksPerPixel);
	Metric rSpeed = pDesc->GetRatedSpeed();
	Metric rTime = (rSpeed > 0.0 ? (rDist / pDesc->GetRatedSpeed()) : 0.0);
	int iFrameToSkip = iFrames / 4;
	int iTicksToHit = (int)(rTime / STD_SECONDS_PER_UPDATE) - iFrameToSkip - 4;

	//	If the item is a missile, create one to fire

	CItemType *pAmmoType = pDesc->GetAmmoType();
	if (pAmmoType)
		{
		CItem MissileItem(pAmmoType, 1);
		ItemList.AddItem(MissileItem);

		pPlatform->OnComponentChanged(comCargo);
		pPlatform->ItemsModified();
		pPlatform->InvalidateItemListAddRemove();
		}

	//	Install the appropriate weapon on the platform

	CItem WeaponItem(pWeapon->GetItemType(), 1);
	ItemList.AddItem(WeaponItem);

	pPlatform->OnComponentChanged(comCargo);
	pPlatform->ItemsModified();
	pPlatform->InvalidateItemListAddRemove();

	pPlatform->InstallItemAsDevice(ItemList);

	//	Select the weapon (we rely on the fact that this is the current
	//	item in the list).

	DeviceNames iDev = pPlatform->SelectWeapon(ItemList.GetItemAtCursor().GetInstalled(), iVariant);

	//	Fire the weapon

	pPlatform->SetWeaponTriggered(iDev);

	//	Update context

	SSystemUpdateCtx Ctx;
	Ctx.bForceEventFiring = true;
	Ctx.bForcePainted = true;

	//	Now loop over the appropriate number of frames

	int xDest = x;
	int yDest = y;
	for (i = 0; i < iFrames; i++)
		{
		//	Update the universe

		g_pUniverse->Update(Ctx);

		//	Paint

		RECT rcView;
		rcView.left = xDest;
		rcView.top = yDest;
		rcView.right = xDest + (iFrames * cxCell * 2);
		rcView.bottom = yDest + cyCell;

		g_pUniverse->PaintPOV(Image, rcView, CSystem::VWP_NO_STAR_FIELD);

		//	Next

		pPlatform->ClearAllTriggered();

		//	At the half-way mark, update until the missile hits

		if (i == iFrameToSkip)
			{
			for (j = 0; j < iTicksToHit; j++)
				g_pUniverse->Update(Ctx);
			}

		yDest += cyCell;
		}

	//	Skip for a while

	for (j = 0; j < 20; j++)
		g_pUniverse->Update(Ctx);

	//	Uninstall weapon

	ItemList.Refresh(CItem::NullItem());
	while (ItemList.MoveCursorForward() 
			&& ItemList.GetItemAtCursor().GetType() != pWeapon->GetItemType())
		;

	pPlatform->RemoveItemAsDevice(ItemList);
	}
