//	ClassInfoHelpers.cpp
//
//	CUIHelper class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

const int ITEM_INFO_SPACING_HORZ =				12;

const int SMALL_ICON_HEIGHT =					48;
const int SMALL_ICON_WIDTH =					48;

#define ALIGN_RIGHT								CONSTLIT("right")

#define PROP_COLOR								CONSTLIT("color")
#define PROP_FILL_TYPE							CONSTLIT("fillType")
#define PROP_FONT								CONSTLIT("font")
#define PROP_LINE_COLOR							CONSTLIT("lineColor")
#define PROP_LINE_PADDING						CONSTLIT("linePadding")
#define PROP_LINE_TYPE							CONSTLIT("lineType")
#define PROP_LINE_WIDTH							CONSTLIT("lineWidth")
#define PROP_LL_RADIUS							CONSTLIT("llRadius")
#define PROP_LR_RADIUS							CONSTLIT("lrRadius")
#define PROP_OPACITY							CONSTLIT("opacity")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_TEXT								CONSTLIT("text")
#define PROP_TEXT_ALIGN_HORZ					CONSTLIT("textAlignHorz")
#define PROP_TEXT_ALIGN_VERT					CONSTLIT("textAlignVert")
#define PROP_UL_RADIUS							CONSTLIT("ulRadius")
#define PROP_UR_RADIUS							CONSTLIT("urRadius")

void CUIHelper::CreateClassInfoArmor (CShipClass *pClass, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const

//	CreateClassInfoArmor
//
//	Creates info about the ship class' armor

	{
	//	Edge condition

	if (pClass->GetHullSectionCount() == 0)
		{
		if (retcyHeight)
			*retcyHeight = 0;

		CAniSequencer::Create(CVector(x, y), (CAniSequencer **)retpInfo);
		return;
		}

	//	Get the armor used

	CArmorClass *pArmor = pClass->GetHullSection(0)->pArmor;

	//	Compute the max armor limit

	CString sMaxArmor;
	if ((pClass->GetMaxArmorMass() % 1000) == 0)
		sMaxArmor = strPatternSubst(CONSTLIT("%d"), pClass->GetMaxArmorMass() / 1000);
	else
		sMaxArmor = strPatternSubst(CONSTLIT("%d.%d"), pClass->GetMaxArmorMass() / 1000, ((pClass->GetMaxArmorMass() % 1000) + 50) / 100);

	//	Info

	CreateClassInfoItem(CItem(pArmor->GetItemType(), 1), 
			x, 
			y, 
			cxWidth, 
			dwOptions, 
			strPatternSubst(CONSTLIT("\noptional upgrade up to %s ton segments"), sMaxArmor),
			retcyHeight, 
			retpInfo);
	}

void CUIHelper::CreateClassInfoCargo (CShipClass *pClass, const CDeviceDescList &Devices, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const

//	CreateClassInfoCargo
//
//	Creates info about the ship class' cargo

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	CDeviceClass *pCargoExtension = Devices.GetNamedDevice(devCargo);
	int iCargoSpace = pClass->GetCargoSpace();
	if (pCargoExtension)
		iCargoSpace += pCargoExtension->GetCargoSpace();

	//	Icon

	CItemType *pItemIcon = (pCargoExtension ? pCargoExtension->GetItemType() : g_pUniverse->FindItemType(CARGO_HOLD_EXPANSION_UNID));

	//	Text

	CString sText = strPatternSubst(CONSTLIT("{/rtf {/f:LargeBold;/c:%d; %s} {/f:MediumBold;/c:%d; %s}\n{/f:Medium;/c:%d; %s}}"),
			CG16bitImage::RGBFromPixel(VI.GetColor(colorTextDialogLabel)),
			strFromInt(iCargoSpace),
			CG16bitImage::RGBFromPixel(VI.GetColor(colorTextDialogInput)),
			(pCargoExtension ? strPatternSubst(CONSTLIT("ton %s"), CTextBlock::Escape(pCargoExtension->GetItemType()->GetNounPhrase(nounActual))) : CONSTLIT("ton cargo hold")),
			CG16bitImage::RGBFromPixel(VI.GetColor(colorTextDialogLabel)),
			(iCargoSpace < pClass->GetMaxCargoSpace() ? strPatternSubst(CONSTLIT("optional expansion up to %d tons"), pClass->GetMaxCargoSpace()) : CONSTLIT("cargo space cannot be expanded")));

	CreateClassInfoSpecialItem(pItemIcon, sText, x, y, cxWidth, dwOptions, retcyHeight, retpInfo);
	}

void CUIHelper::CreateClassInfoDeviceSlots (CShipClass *pClass, const CDeviceDescList &Devices, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const

//	CreateClassInfoDeviceSlots
//
//	Creates info about the number of open device slots

	{
	int i;

	const CVisualPalette &VI = m_HI.GetVisuals();

	//	Count the number of slots being used

	int iAll = 0;
	int iWeapons = 0;
	int iNonWeapons = 0;
	for (i = 0; i < Devices.GetCount(); i++)
		{
		CDeviceClass *pDevice = Devices.GetDeviceClass(i);
		int iSlots = pDevice->GetSlotsRequired();
		iAll += iSlots;

		if (pDevice->GetCategory() == itemcatWeapon || pDevice->GetCategory() == itemcatLauncher)
			iWeapons += iSlots;
		else
			iNonWeapons += iSlots;
		}

	int iAllLeft = Max(0, pClass->GetMaxDevices() - iAll);
	int iWeaponsLeft = Max(0, Min(iAllLeft, pClass->GetMaxWeapons() - iWeapons));
	int iNonWeaponsLeft = Max(0, Min(iAllLeft, pClass->GetMaxNonWeapons() - iNonWeapons));

	//	Add the device slot statistic

	CString sNumber = strPatternSubst(CONSTLIT("%d"), iAllLeft);
	CString sHeader = (iAllLeft == 1 ? CONSTLIT("device slot for expansion") : CONSTLIT("device slots for expansion"));
	CString sDesc;
	if (iWeaponsLeft != iAllLeft && iNonWeaponsLeft != iAllLeft)
		sDesc = strPatternSubst(CONSTLIT("only %d for weapons; only %d for non-weapons"), iWeaponsLeft, iNonWeaponsLeft);
	else if (iWeaponsLeft != iAllLeft)
		sDesc = strPatternSubst(CONSTLIT("only %d device slot%p available for weapons"), iWeaponsLeft);
	else if (iNonWeaponsLeft != iAllLeft)
		sDesc = strPatternSubst(CONSTLIT("only %d device slot%p available for non-weapons"), iNonWeaponsLeft);

	//	Figure out some dimensions and metrics. Everything is relative to x, y.

	bool bRightAlign = ((dwOptions & OPTION_ITEM_RIGHT_ALIGN) ? true : false);
	int cxIcon = SMALL_ICON_WIDTH;
	int cyIcon = SMALL_ICON_HEIGHT;
	int xIcon = (bRightAlign ? -cxIcon : 0);
	int yIcon = 0;

	int cxText = cxWidth - (cxIcon + ITEM_INFO_SPACING_HORZ);
	int xText = (bRightAlign ? -cxWidth : cxIcon + ITEM_INFO_SPACING_HORZ);
	int yText = 0;

	//	Create a sequencer to hold all the controls

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(x, y), &pRoot);

	//	Create a small item icon

	IAnimatron *pImageFrame = new CAniRect;
	pImageFrame->SetPropertyVector(PROP_POSITION, CVector(xIcon, yIcon));
	pImageFrame->SetPropertyVector(PROP_SCALE, CVector(SMALL_ICON_WIDTH, SMALL_ICON_HEIGHT));
	pImageFrame->SetFillMethod(new CAniImageFill(&VI.GetImage(imageSlotIcon), false));

	pRoot->AddTrack(pImageFrame, 0);

	//	Create some text

	int cyText = 0;

	CString sText = strPatternSubst(CONSTLIT("{/rtf {/f:LargeBold;/c:%d; %s} {/f:MediumBold;/c:%d; %s}\n{/f:Medium;/c:%d; %s}}"),
			CG16bitImage::RGBFromPixel(VI.GetColor(colorTextDialogLabel)),
			sNumber,
			CG16bitImage::RGBFromPixel(VI.GetColor(colorTextDialogInput)),
			sHeader,
			CG16bitImage::RGBFromPixel(VI.GetColor(colorTextDialogLabel)),
			sDesc);

	//	Add the text item

	IAnimatron *pRef = new CAniRichText(VI);
	pRef->SetPropertyVector(PROP_POSITION, CVector(xText, yText + cyText));
	pRef->SetPropertyVector(PROP_SCALE, CVector(cxText, 1000));
	pRef->SetPropertyString(PROP_TEXT, sText);
	if (bRightAlign)
		pRef->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_RIGHT);

	pRoot->AddTrack(pRef, 0);

	RECT rcRef;
	pRef->GetSpacingRect(&rcRef);
	cyText += RectHeight(rcRef);

	//	Done

	if (retcyHeight)
		*retcyHeight = Max(cyText, SMALL_ICON_HEIGHT);

	*retpInfo = pRoot;
	}

void CUIHelper::CreateClassInfoDrive (CShipClass *pClass, const CDeviceDescList &Devices, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const

//	CreateClassInfoDrive
//
//	Creates info about the ship class' drive

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	const DriveDesc *pDriveDesc = pClass->GetHullDriveDesc();
	if (pDriveDesc == NULL)
		{
		if (retcyHeight)
			*retcyHeight = 0;

		CAniSequencer::Create(CVector(x, y), (CAniSequencer **)retpInfo);
		return;
		}

	//	Base speed and thrust

	Metric rMaxSpeed = pDriveDesc->rMaxSpeed;
	int iThrust = pDriveDesc->iThrust;

	//	If we have a drive upgrade, we account for it here

	CDeviceClass *pDrive = Devices.GetNamedDevice(devDrive);
	if (pDrive)
		{
		pDriveDesc = pDrive->GetDriveDesc();
		rMaxSpeed = Max(rMaxSpeed, pDriveDesc->rMaxSpeed);
		iThrust += pDriveDesc->iThrust;
		}

	//	Icon

	CItemType *pItemIcon = (pDrive ? pDrive->GetItemType() : g_pUniverse->FindItemType(TRITIUM_PROPULSION_UPGRADE_UNID));

	//	Add speed box

	CString sSpeedNumber;
	if (rMaxSpeed < LIGHT_SECOND)
		sSpeedNumber = strPatternSubst(CONSTLIT(".%02dc"), (int)((100.0 * rMaxSpeed / LIGHT_SECOND) + 0.5));
	else
		{
		int iTotal = (int)((100.0 * rMaxSpeed / LIGHT_SECOND) + 0.5);
		int iFTL = iTotal / 100;
		int iFrac = iTotal % 100;
		sSpeedNumber = strPatternSubst(CONSTLIT("%d.%02dc"), iFTL, iFrac);
		}

	CString sSpeedHeader = (pDrive ? pDrive->GetItemType()->GetNounPhrase(nounActual) : CONSTLIT("main drive"));

	//	Add thrust/mass ratio

	Metric rMass = pClass->CalcMass(Devices);
	int iRatio = (int)((200.0 * (rMass > 0.0 ? iThrust / rMass : 0.0)) + 0.5);

	CString sThrustNumber = strPatternSubst(CONSTLIT("%d.%d"), iRatio / 100, ((iRatio % 100) + 5) / 10);

	//	Add maneuver speed

	int iManeuver = pClass->GetManeuverability() * pClass->GetRotationRange() / STD_ROTATION_COUNT;
	if (iManeuver <= 0)
		iManeuver = 1;

	CString sManeuverNumber = strPatternSubst(CONSTLIT("%d.%d"), 30 / iManeuver, (10 * (30 % iManeuver) + (iManeuver / 2)) / iManeuver);

	//	Compose the text

	CString sText = strPatternSubst(CONSTLIT(
			"{/rtf/f:Medium;/c:%d; {/f:LargeBold;/c:%d; %s} {/f:MediumBold;/c:%d; %s}\n"
			"(max speed as fraction of light-speed)\n"
			"{/f:LargeBold;/c:%d; %s} thrust//mass ratio\n"
			"(thrust as proportion of total mass)\n"
			"{/f:LargeBold;/c:%d; %s} maneuverability\n"
			"(turning speed)}"),

			CG16bitImage::RGBFromPixel(VI.GetColor(colorTextDialogLabel)),
			CG16bitImage::RGBFromPixel(VI.GetColor(colorTextDialogLabel)),
			sSpeedNumber,
			CG16bitImage::RGBFromPixel(VI.GetColor(colorTextDialogInput)),
			sSpeedHeader,

			CG16bitImage::RGBFromPixel(VI.GetColor(colorTextDialogLabel)),
			sThrustNumber,

			CG16bitImage::RGBFromPixel(VI.GetColor(colorTextDialogLabel)),
			sManeuverNumber);

	CreateClassInfoSpecialItem(pItemIcon, sText, x, y, cxWidth, dwOptions, retcyHeight, retpInfo);
	}

void CUIHelper::CreateClassInfoItem (const CItem &Item, int x, int y, int cxWidth, DWORD dwOptions, const CString &sExtraDesc, int *retcyHeight, IAnimatron **retpInfo) const

//	CreateClassInfoItem
//
//	Creates an item info animation

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);
	const CG16bitFont &MediumBoldFont = VI.GetFont(fontMediumBold);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	//	Handle some edge conditions

	CItemType *pType = Item.GetType();
	if (pType == NULL)
		{
		if (retcyHeight)
			*retcyHeight = 0;

		CAniSequencer::Create(CVector(x, y), (CAniSequencer **)retpInfo);
		return;
		}

	//	Figure out some dimensions and metrics. Everything is relative to x, y.

	bool bRightAlign = ((dwOptions & OPTION_ITEM_RIGHT_ALIGN) ? true : false);
	int cxIcon = SMALL_ICON_WIDTH;
	int cyIcon = SMALL_ICON_HEIGHT;
	int xIcon = (bRightAlign ? -cxIcon : 0);
	int yIcon = 0;

	int cxText = cxWidth - (cxIcon + ITEM_INFO_SPACING_HORZ);
	int xText = (bRightAlign ? -cxWidth : cxIcon + ITEM_INFO_SPACING_HORZ);
	int yText = 0;

	//	Create a sequencer to hold all the controls

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(x, y), &pRoot);

	//	Create a small item icon

	const CObjectImageArray &Image = pType->GetImage();
	RECT rcImage = Image.GetImageRect();
	if (Image.IsLoaded())
		{
		CG16bitImage *pIcon = new CG16bitImage;
		pIcon->CreateFromImageTransformed(Image.GetImage(pType->GetName(0)), 
				rcImage.left, 
				rcImage.top, 
				RectWidth(rcImage), 
				RectHeight(rcImage), 
				(Metric)SMALL_ICON_WIDTH / RectWidth(rcImage),
				(Metric)SMALL_ICON_HEIGHT / RectHeight(rcImage),
				0.0);

		IAnimatron *pImageFrame = new CAniRect;
		pImageFrame->SetPropertyVector(PROP_POSITION, CVector(xIcon, yIcon));
		pImageFrame->SetPropertyVector(PROP_SCALE, CVector(SMALL_ICON_WIDTH, SMALL_ICON_HEIGHT));
		pImageFrame->SetFillMethod(new CAniImageFill(pIcon, true));

		pRoot->AddTrack(pImageFrame, 0);
		}

	//	Create text

	int cyText = 0;

	IAnimatron *pName = new CAniText;
	pName->SetPropertyVector(PROP_POSITION, CVector(xText, yText + cyText));
	pName->SetPropertyVector(PROP_SCALE, CVector(cxText, 1000));
	pName->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
	pName->SetPropertyFont(PROP_FONT, &MediumBoldFont);
	pName->SetPropertyString(PROP_TEXT, pType->GetNounPhrase(nounActual));
	if (bRightAlign)
		pName->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_RIGHT);

	pRoot->AddTrack(pName, 0);
	cyText += MediumBoldFont.GetHeight();

	//	Add the damage type adjustment

	CItemDataAnimatron *pDamageDesc = new CItemDataAnimatron(VI, Item);
	if (!pDamageDesc->IsEmpty())
		{
		pDamageDesc->SetPropertyVector(PROP_POSITION, CVector(xText, yText + cyText));
		pDamageDesc->SetPropertyVector(PROP_SCALE, CVector(cxText, 1000));
		if (bRightAlign)
			pDamageDesc->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_RIGHT);

		pRoot->AddTrack(pDamageDesc, 0);

		RECT rcRect;
		pDamageDesc->GetSpacingRect(&rcRect);
		cyText += RectHeight(rcRect);
		}
	else
		delete pDamageDesc;

	//	Add the reference text

	CItemCtx ItemCtx;
	CString sRef = pType->GetReference(ItemCtx, -1, CItemType::FLAG_ACTUAL_ITEM);
	if (sRef.IsBlank())
		sRef = strPatternSubst(CONSTLIT("Level %s%s"), strLevel(pType->GetLevel()), sExtraDesc);
	else
		sRef = strPatternSubst(CONSTLIT("Level %s — %s%s"), strLevel(pType->GetLevel()), sRef, sExtraDesc);

	IAnimatron *pRef = new CAniText;
	pRef->SetPropertyVector(PROP_POSITION, CVector(xText, yText + cyText));
	pRef->SetPropertyVector(PROP_SCALE, CVector(cxText, 1000));
	pRef->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogLabel));
	pRef->SetPropertyFont(PROP_FONT, &MediumFont);
	pRef->SetPropertyString(PROP_TEXT, sRef);
	if (bRightAlign)
		pRef->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_RIGHT);

	pRoot->AddTrack(pRef, 0);

	RECT rcRef;
	pRef->GetSpacingRect(&rcRef);
	cyText += RectHeight(rcRef);

	//	Done

	if (retcyHeight)
		*retcyHeight = Max(cyText, SMALL_ICON_HEIGHT);

	*retpInfo = pRoot;
	}

void CUIHelper::CreateClassInfoReactor (CShipClass *pClass, const CDeviceDescList &Devices, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const

//	CreateClassInfoReactor
//
//	Creates info about the ship class' reactor

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	//	Get reactor info from the ship class

	const ReactorDesc *pReactorDesc = pClass->GetReactorDesc();
	if (pReactorDesc == NULL)
		{
		if (retcyHeight)
			*retcyHeight = 0;

		CAniSequencer::Create(CVector(x, y), (CAniSequencer **)retpInfo);
		return;
		}

	CDeviceClass *pReactor = Devices.GetNamedDevice(devReactor);
	if (pReactor)
		pReactorDesc = pReactor->GetReactorDesc();

	//	Get the icon (OK if this is NULL)

	CItemType *pItemIcon = (pReactor ? pReactor->GetItemType() : g_pUniverse->FindItemType(NOVA25_REACTOR_UNID));

	//	Create the info

	CString sText = strPatternSubst(CONSTLIT("{/rtf {/f:LargeBold;/c:%d; %s} {/f:MediumBold;/c:%d; %s}}"),
			CG16bitImage::RGBFromPixel(VI.GetColor(colorTextDialogLabel)),
			CTextBlock::Escape(ReactorPower2String(pReactorDesc->iMaxPower)),
			CG16bitImage::RGBFromPixel(VI.GetColor(colorTextDialogInput)),
			(pReactor ? CTextBlock::Escape(pReactor->GetItemType()->GetNounPhrase()) : strPatternSubst(CONSTLIT("%s reactor"), CTextBlock::Escape(pClass->GetShortName()))));

	CreateClassInfoSpecialItem(pItemIcon, sText, x, y, cxWidth, dwOptions, retcyHeight, retpInfo);
	}

void CUIHelper::CreateClassInfoSpecialItem (CItemType *pItemIcon, const CString &sText, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpInfo) const

//	CreateClassInfoSpecialItem
//
//	Creates a special item info animation

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	//	Figure out some dimensions and metrics. Everything is relative to x, y.

	bool bRightAlign = ((dwOptions & OPTION_ITEM_RIGHT_ALIGN) ? true : false);
	int cxIcon = SMALL_ICON_WIDTH;
	int cyIcon = SMALL_ICON_HEIGHT;
	int xIcon = (bRightAlign ? -cxIcon : 0);
	int yIcon = 0;

	int cxText = cxWidth - (cxIcon + ITEM_INFO_SPACING_HORZ);
	int xText = (bRightAlign ? -cxWidth : cxIcon + ITEM_INFO_SPACING_HORZ);
	int yText = 0;

	//	Create a sequencer to hold all the controls

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(x, y), &pRoot);

	//	Create a small item icon

	if (pItemIcon)
		{
		const CObjectImageArray &Image = pItemIcon->GetImage();
		RECT rcImage = Image.GetImageRect();
		if (Image.IsLoaded())
			{
			CG16bitImage *pIcon = new CG16bitImage;
			pIcon->CreateFromImageTransformed(Image.GetImage(pItemIcon->GetName(0)), 
					rcImage.left, 
					rcImage.top, 
					RectWidth(rcImage), 
					RectHeight(rcImage), 
					(Metric)SMALL_ICON_WIDTH / RectWidth(rcImage),
					(Metric)SMALL_ICON_HEIGHT / RectHeight(rcImage),
					0.0);

			IAnimatron *pImageFrame = new CAniRect;
			pImageFrame->SetPropertyVector(PROP_POSITION, CVector(xIcon, yIcon));
			pImageFrame->SetPropertyVector(PROP_SCALE, CVector(SMALL_ICON_WIDTH, SMALL_ICON_HEIGHT));
			pImageFrame->SetFillMethod(new CAniImageFill(pIcon, true));

			pRoot->AddTrack(pImageFrame, 0);
			}
		}

	//	Create some text

	int cyText = 0;

	IAnimatron *pRef = new CAniRichText(VI);
	pRef->SetPropertyVector(PROP_POSITION, CVector(xText, yText + cyText));
	pRef->SetPropertyVector(PROP_SCALE, CVector(cxText, 1000));
	pRef->SetPropertyString(PROP_TEXT, sText);
	if (bRightAlign)
		pRef->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_RIGHT);

	pRoot->AddTrack(pRef, 0);

	RECT rcRef;
	pRef->GetSpacingRect(&rcRef);
	cyText += RectHeight(rcRef);

	//	Done

	if (retcyHeight)
		*retcyHeight = Max(cyText, SMALL_ICON_HEIGHT);

	*retpInfo = pRoot;
	}
