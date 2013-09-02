//	CShipClassDisplay.cpp
//
//	CShipClassDisplay class

#include "PreComp.h"
#include "Transcendence.h"

const WORD RGB_BACKGROUND =					CG16bitImage::RGBValue(0, 0, 0);
const WORD RGB_CLASS_NAME =					CG16bitImage::RGBValue(128, 128, 128);
const WORD RGB_DESCRIPTION =				CG16bitImage::RGBValue(128, 128, 128);
const WORD RGB_ITEM_TITLE =					CG16bitImage::RGBValue(255, 255, 255);
const WORD RGB_ITEM_DESC =					CG16bitImage::RGBValue(128, 128, 128);
const WORD RGB_ITEM_NUMBER =				CG16bitImage::RGBValue(200, 200, 200);

const int MARGIN_TOP =						24;
const int MARGIN_LEFT =						24;
const int MARGIN_RIGHT =					24;
const int MARGIN_BOTTOM =					16;

const int SPACING_X =						16;
const int SPACING_Y =						8;

const int CENTER_PANE_WIDTH =				320;
const int CENTER_PANE_HEIGHT =				400;

const int BOTTOM_PANE_WIDTH =				640;

const int ICON_WIDTH =						96;
const int ICON_HEIGHT =						96;

const int ITEM_SPACING_X =					4;
const int ITEM_SPACING_Y =					4;

const int DATA_BOX_TEXT_WIDTH =				240;
const int DATA_BOX_INNER_SPACING_X =		4;
const int DATA_BOX_WIDTH =					(ICON_WIDTH + DATA_BOX_INNER_SPACING_X + DATA_BOX_TEXT_WIDTH);

const int SPECIAL_ICON_WIDTH =				96;
const int SPECIAL_ICON_HEIGHT =				96;
const int ICON_DEVICE_SLOTS_X =				0;
const int ICON_DEVICE_SLOTS_Y =				0;
const int DAMAGE_ADJ_SPACING_X =			6;

CShipClassDisplay::CShipClassDisplay (void) : 
		m_bInvalid(true), 
		m_iPlayerShipCount(0),
		m_PlayerShipList(NULL),
		m_pShipImage(NULL)

//	CShipClassDisplay constructor

	{
	}

CShipClassDisplay::~CShipClassDisplay (void)

//	CShipClassDisplay destructor

	{
	CleanUp();
	}

CShipClassDisplay::SDataBox *CShipClassDisplay::AddDataBox (void)

//	AddDataBox
//
//	Adds a new data box and returns a pointer to the new box

	{
	SDataBox *pData = &m_DataBoxes[m_iDataBoxCount++];
	pData->dwFlags = FLAG_ALIGN_LEFT;
	pData->pItemIcon = NULL;
	pData->sDesc = NULL_STR;
	pData->sDesc2 = NULL_STR;
	pData->sHeading = NULL_STR;
	pData->sNumber = NULL_STR;

	return pData;
	}

void CShipClassDisplay::CleanUp (void)

//	CleanUp
//
//	Must be called to release resources

	{
	if (m_PlayerShipList)
		{
		delete [] m_PlayerShipList;
		m_PlayerShipList = NULL;
		m_iPlayerShipCount = 0;
		}

	if (m_pShipImage)
		m_pShipImage = NULL;

	//	NOTE: We only do this to save memory. The destructor will
	//	automatically free the memory.

	m_Buffer.Destroy();
	m_Images.Destroy();
	}

ALERROR CShipClassDisplay::Init (CTranscendenceWnd *pTrans, const RECT &rcRect, bool bShowDebugShips, CString *retsError)

//	Init
//
//	Must be called to initialize

	{
	ALERROR error;

	m_rcRect = rcRect;
	m_pTrans = pTrans;
	m_pFonts = &pTrans->GetFonts();

	//	Create the off-screen buffer

	if (error = m_Buffer.CreateBlank(RectWidth(rcRect), RectHeight(rcRect), false))
		{
		*retsError = CONSTLIT("Out of memory");
		return error;
		}

	//	Initialize the list of player ship available

	if (error = InitPlayerShipList(bShowDebugShips, retsError))
		return error;

	InitShipData();

	//	Load images

	HBITMAP hDIB;
	if (error = JPEGLoadFromResource(g_hInst,
			MAKEINTRESOURCE(IDR_SELECT_SHIP_IMAGE),
			JPEG_LFR_DIB, 
			NULL, 
			&hDIB))
		{
		*retsError = CONSTLIT("Unable to find select ship image");
		return error;
		}

	error = m_Images.CreateFromBitmap(hDIB);
	::DeleteObject(hDIB);
	if (error)
		{
		*retsError = CONSTLIT("Unable to find select ship image");
		return error;
		}

	//	Compute some rects

	m_rcCenterPane.left = (RectWidth(rcRect) - CENTER_PANE_WIDTH) / 2;
	m_rcCenterPane.top = (RectHeight(rcRect) - CENTER_PANE_HEIGHT) / 2;
	m_rcCenterPane.right = m_rcCenterPane.left + CENTER_PANE_WIDTH;
	m_rcCenterPane.bottom = m_rcCenterPane.top + CENTER_PANE_HEIGHT;

	m_rcBottomPane.left = (RectWidth(rcRect) - BOTTOM_PANE_WIDTH) / 2;
	m_rcBottomPane.top = m_rcCenterPane.bottom + SPACING_Y;
	m_rcBottomPane.right = m_rcBottomPane.left + BOTTOM_PANE_WIDTH;
	m_rcBottomPane.bottom = RectHeight(rcRect) - MARGIN_BOTTOM;

	m_rcLeftPane.left = MARGIN_LEFT;
	m_rcLeftPane.top = MARGIN_TOP;
	m_rcLeftPane.right = m_rcCenterPane.left - SPACING_X;
	m_rcLeftPane.bottom = m_rcBottomPane.top - SPACING_Y;

	m_rcRightPane.left = m_rcCenterPane.right + SPACING_X;
	m_rcRightPane.top = MARGIN_TOP;
	m_rcRightPane.right = RectWidth(rcRect) - MARGIN_RIGHT;
	m_rcRightPane.bottom = m_rcBottomPane.top - SPACING_Y;

	//	Init

	m_bInvalid = true;

	return NOERROR;
	}

ALERROR CShipClassDisplay::InitPlayerShipList (bool bShowDebugShips, CString *retsError)

//	InitPlayerShipList
//
//	Initializes the list of player ships

	{
	int i;

	ASSERT(m_PlayerShipList == NULL);
	m_iPlayerShipCount = 0;
	m_iPlayerShip = -1;

	CAdventureDesc *pAdventureDesc = g_pUniverse->GetCurrentAdventureDesc();
	ASSERT(pAdventureDesc);

	//	Add all the player ship classes to a symbol table so that we
	//	can sort them.

	CSymbolTable SortList(FALSE, TRUE);
	for (i = 0; i < g_pUniverse->GetShipClassCount(); i++)
		{
		CShipClass *pClass = g_pUniverse->GetShipClass(i);
		if (pClass->IsShownAtNewGame()
				&& pAdventureDesc->IsValidStartingClass(pClass)
				&& (!pClass->IsDebugOnly() || bShowDebugShips))
			{
			CString sKey = strPatternSubst(CONSTLIT("%d %s !%x"), 
					(pClass->IsDebugOnly() ? 2 : 1),
					pClass->GetName(), 
					pClass->GetUNID());
			SortList.AddEntry(sKey, (CObject *)pClass);
			}
		}

	if (SortList.GetCount() == 0)
		{
		*retsError = CONSTLIT("Unable to find starting ship classes that match adventure criteria");
		return ERR_FAIL;
		}

	//	Allocate our list

	m_iPlayerShipCount = SortList.GetCount();
	m_PlayerShipList = new SPlayerShip [m_iPlayerShipCount];

	for (i = 0; i < m_iPlayerShipCount; i++)
		{
		CShipClass *pClass = (CShipClass *)SortList.GetValue(i);
		m_PlayerShipList[i].pClass = pClass;
		m_PlayerShipList[i].sName = pClass->GetName();

		if (pClass->GetUNID() == m_pTrans->GetPlayerShip())
			m_iPlayerShip = i;
		}

	if (m_iPlayerShip == -1)
		{
		m_iPlayerShip = 0;
		m_pTrans->SetPlayerShip(m_PlayerShipList[0].pClass->GetUNID());
		}

	return NOERROR;
	}

void CShipClassDisplay::InitShipData (void)

//	InitShipData
//
//	Initializes data for ship class

	{
	int i;

	//	We never have context, so we pass the same context block for all items

	CItemCtx Ctx;

	//	Some variables that we need

	CShipClass *pClass = m_PlayerShipList[m_iPlayerShip].pClass;
	const CPlayerSettings *pPlayerSettings = pClass->GetPlayerSettings();

	//	Load the center image

	if (m_pShipImage)
		m_pShipImage = NULL;

	if (pPlayerSettings->GetLargeImage())
		m_pShipImage = g_pUniverse->GetLibraryBitmap(pPlayerSettings->GetLargeImage());

	//	Generate devices

	CDeviceDescList Devices;
	pClass->GenerateDevices(1, Devices);

	//	Generate data boxes

	m_iDataBoxCount = 0;
	int iAddedToLeft = 0;

	//	Show the highest-level weapon

	int iBestWeapon = -1;
	int iBestWeaponLevel = 0;
	for (i = 0; i < Devices.GetCount(); i++)
		{
		CDeviceClass *pDevice = Devices.GetDeviceClass(i);
		if (pDevice->GetCategory() == itemcatWeapon 
				&& pDevice->GetLevel() > iBestWeaponLevel)
			{
			iBestWeapon = i;
			iBestWeaponLevel = pDevice->GetLevel();
			}
		}

	if (iBestWeapon != -1)
		{
		CDeviceClass *pDevice = Devices.GetDeviceClass(iBestWeapon);
		CItemType *pItem = pDevice->GetItemType();

		SDataBox *pData = AddDataBox();
		pData->theItem = CItem(pItem, 1);
		pData->dwFlags = FLAG_ALIGN_LEFT;
		pData->pItemIcon = pItem;
		pData->sHeading = pItem->GetNounPhrase(nounActual);

		if (pData->theItem.GetReferenceDamageType(NULL, -1, CItemType::FLAG_ACTUAL_ITEM, &pData->iDamageType, &pData->sDesc))
			{
			pData->dwFlags |= FLAG_DAMAGE_TYPE;
			pData->sDesc2 = strPatternSubst(CONSTLIT("Level %s — %s"),
					strLevel(pItem->GetLevel()),
					pItem->GetReference(Ctx, -1, CItemType::FLAG_ACTUAL_ITEM));
			}
		else
			pData->sDesc = pItem->GetReference(Ctx, -1, CItemType::FLAG_ACTUAL_ITEM);

		iAddedToLeft++;
		}

	//	Add a launcher if we've got one

	CDeviceClass *pLauncher = Devices.GetNamedDevice(devMissileWeapon);
	if (pLauncher)
		{
		CItemType *pItem = pLauncher->GetItemType();

		SDataBox *pData = AddDataBox();
		pData->theItem = CItem(pItem, 1);
		pData->dwFlags = FLAG_ALIGN_LEFT;
		pData->pItemIcon = pItem;
		pData->sHeading = pItem->GetNounPhrase(nounActual);

		if (pData->theItem.GetReferenceDamageType(NULL, -1, CItemType::FLAG_ACTUAL_ITEM, &pData->iDamageType, &pData->sDesc))
			{
			pData->dwFlags |= FLAG_DAMAGE_TYPE;
			pData->sDesc2 = strPatternSubst(CONSTLIT("Level %s — %s"),
					strLevel(pItem->GetLevel()),
					pItem->GetReference(Ctx, -1, CItemType::FLAG_ACTUAL_ITEM));
			}
		else
			pData->sDesc = pItem->GetReference(Ctx, -1, CItemType::FLAG_ACTUAL_ITEM);

		iAddedToLeft++;
		}

	//	Otherwise, add a second weapon

	else if (iBestWeapon != -1)
		{
		int iSecondBestWeapon = -1;
		int iSecondBestWeaponLevel = 0;
		for (i = 0; i < Devices.GetCount(); i++)
			{
			CDeviceClass *pDevice = Devices.GetDeviceClass(i);
			if (pDevice->GetCategory() == itemcatWeapon 
					&& pDevice->GetLevel() > iSecondBestWeaponLevel
					&& i != iBestWeapon)
				{
				iSecondBestWeapon = i;
				iSecondBestWeaponLevel = pDevice->GetLevel();
				}
			}

		if (iSecondBestWeapon != -1)
			{
			CDeviceClass *pDevice = Devices.GetDeviceClass(iSecondBestWeapon);
			CItemType *pItem = pDevice->GetItemType();

			SDataBox *pData = AddDataBox();
			pData->theItem = CItem(pItem, 1);
			pData->dwFlags = FLAG_ALIGN_LEFT;
			pData->pItemIcon = pItem;
			pData->sHeading = pItem->GetNounPhrase(nounActual);

			if (pData->theItem.GetReferenceDamageType(NULL, -1, CItemType::FLAG_ACTUAL_ITEM, &pData->iDamageType, &pData->sDesc))
				{
				pData->dwFlags |= FLAG_DAMAGE_TYPE;
				pData->sDesc2 = strPatternSubst(CONSTLIT("Level %s — %s"),
						strLevel(pItem->GetLevel()),
						pItem->GetReference(Ctx, -1, CItemType::FLAG_ACTUAL_ITEM));
				}
			else
				pData->sDesc = pItem->GetReference(Ctx, -1, CItemType::FLAG_ACTUAL_ITEM);

			iAddedToLeft++;
			}
		}

	//	Add shields

	CDeviceClass *pShields = Devices.GetNamedDevice(devShields);
	if (pShields)
		{
		CItemType *pItem = pShields->GetItemType();

		SDataBox *pData = AddDataBox();
		pData->theItem = CItem(pItem, 1);
		pData->dwFlags = FLAG_ALIGN_LEFT;
		pData->pItemIcon = pItem;
		pData->sHeading = pItem->GetNounPhrase(nounActual);

		if (pData->theItem.GetReferenceDamageAdj(NULL, CItemType::FLAG_ACTUAL_ITEM, &pData->iHP, pData->iDamageTypeAdj))
			{
			pData->dwFlags |= FLAG_DAMAGE_ADJ;
			pData->sDesc = strPatternSubst(CONSTLIT("Level %s — %s"),
					strLevel(pItem->GetLevel()),
					pItem->GetReference(Ctx, 0, CItemType::FLAG_ACTUAL_ITEM));
			}
		else
			pData->sDesc = pItem->GetReference(Ctx, -1, CItemType::FLAG_ACTUAL_ITEM);

		iAddedToLeft++;
		}

	//	Add armor

	if (pClass->GetHullSectionCount() > 0)
		{
		CArmorClass *pArmor = pClass->GetHullSection(0)->pArmor;
		bool bSame = true;

		//	See if all the armor sections are the same

		for (i = 1; i < pClass->GetHullSectionCount(); i++)
			if (pArmor != pClass->GetHullSection(i)->pArmor)
				{
				bSame = false;
				break;
				}

		//	Add the armor

		SDataBox *pData = AddDataBox();
		CItemType *pItemType = pArmor->GetItemType();
		pData->theItem = CItem(pItemType, 1);
		pData->dwFlags = FLAG_ALIGN_LEFT;
		pData->pItemIcon = pArmor->GetItemType();
		pData->sHeading = strPatternSubst(CONSTLIT("%s (x%d)"), 
				(bSame ? pArmor->GetShortName() : CONSTLIT("armor segments")), 
				pClass->GetHullSectionCount());
		CString sMaxArmor;
		if ((pClass->GetMaxArmorMass() % 1000) == 0)
			sMaxArmor = strPatternSubst(CONSTLIT("%d"), pClass->GetMaxArmorMass() / 1000);
		else
			sMaxArmor = strPatternSubst(CONSTLIT("%d.%d"), pClass->GetMaxArmorMass() / 1000, ((pClass->GetMaxArmorMass() % 1000) + 50) / 100);

		if (bSame && pData->theItem.GetReferenceDamageAdj(NULL, CItemType::FLAG_ACTUAL_ITEM, &pData->iHP, pData->iDamageTypeAdj))
			{
			pData->dwFlags |= FLAG_DAMAGE_ADJ;
			CString sReference = pItemType->GetReference(Ctx, -1, CItemType::FLAG_ACTUAL_ITEM);
			if (!sReference.IsBlank())
				pData->sDesc = strPatternSubst(CONSTLIT("Level %s — %s\noptional upgrade up to %s ton segments"),
						strLevel(pItemType->GetLevel()),
						sReference,
						sMaxArmor);
			else
				pData->sDesc = strPatternSubst(CONSTLIT("Level %s\noptional upgrade up to %s ton segments"), 
						strLevel(pItemType->GetLevel()),
						sMaxArmor);
			}
		else
			pData->sDesc = strPatternSubst(CONSTLIT("%s — optional upgrade up to %s ton segments"),
					(bSame ? pItemType->GetReference(Ctx, -1, CItemType::FLAG_ACTUAL_ITEM) : CONSTLIT("segments of different strengths")),
					sMaxArmor);

		iAddedToLeft++;
		}

	//	Add reactor

	const ReactorDesc *pReactorDesc = pClass->GetReactorDesc();
	if (pReactorDesc)
		{
		CDeviceClass *pReactor = Devices.GetNamedDevice(devReactor);
		if (pReactor)
			pReactorDesc = pReactor->GetReactorDesc();

		SDataBox *pData = AddDataBox();
		pData->dwFlags = FLAG_ALIGN_RIGHT;
		pData->sNumber = ReactorPower2String(pReactorDesc->iMaxPower);
		if (pReactor)
			pData->sHeading = pReactor->GetItemType()->GetNounPhrase();
		else
			pData->sHeading = strPatternSubst(CONSTLIT("%s reactor"), pClass->GetShortName());

		if (pReactor)
			pData->pItemIcon = pReactor->GetItemType();
		else
			pData->pItemIcon = g_pUniverse->FindItemType(NOVA25_REACTOR_UNID);
		}

	//	Add thrust and maneuverability

	const DriveDesc *pDriveDesc = pClass->GetHullDriveDesc();
	if (pDriveDesc)
		{
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

		//	Add speed box

		SDataBox *pData = AddDataBox();
		pData->dwFlags = FLAG_ALIGN_RIGHT | FLAG_CONTINUES;
		pData->sNumber = strPatternSubst(CONSTLIT(".%02dc"), (int)((100.0 * rMaxSpeed / LIGHT_SECOND) + 0.5));
		if (pDrive)
			pData->sHeading = pDrive->GetItemType()->GetNounPhrase(nounActual);
		else
			pData->sHeading = CONSTLIT("main drive");
		pData->sDesc = CONSTLIT("(max speed as fraction of light-speed)");
		if (pDrive)
			pData->pItemIcon = pDrive->GetItemType();
		else
			pData->pItemIcon = g_pUniverse->FindItemType(TRITIUM_PROPULSION_UPGRADE_UNID);

		//	Add thrust/mass ratio

		Metric rMass = pClass->CalcMass(Devices);
		int iRatio = (int)((200.0 * (rMass > 0.0 ? iThrust / rMass : 0.0)) + 0.5);

		pData = AddDataBox();
		pData->dwFlags = FLAG_ALIGN_RIGHT | FLAG_CONTINUES | FLAG_PLAIN_HEADER;
		pData->sNumber = strPatternSubst(CONSTLIT("%d.%d"), iRatio / 100, ((iRatio % 100) + 5) / 10);
		pData->sHeading = CONSTLIT("thrust/mass ratio");
		pData->sDesc = CONSTLIT("(thrust as proportion of total mass)");

		//	Add maneuver speed

		int iManeuver = pClass->GetManeuverability() * pClass->GetRotationRange() / STD_ROTATION_COUNT;
		if (iManeuver <= 0)
			iManeuver = 1;

		pData = AddDataBox();
		pData->dwFlags = FLAG_ALIGN_RIGHT | FLAG_PLAIN_HEADER;
		pData->sNumber = strPatternSubst(CONSTLIT("%d.%d"), 30 / iManeuver, (10 * (30 % iManeuver) + (iManeuver / 2)) / iManeuver);
		pData->sHeading = CONSTLIT("maneuverability");
		pData->sDesc = CONSTLIT("(turning speed)");
		}

	//	Add cargo space

	CDeviceClass *pCargoExtension = Devices.GetNamedDevice(devCargo);
	int iCargoSpace = pClass->GetCargoSpace();
	if (pCargoExtension)
		iCargoSpace += pCargoExtension->GetCargoSpace();

	SDataBox *pData = AddDataBox();
	pData->dwFlags = FLAG_ALIGN_RIGHT;
	pData->sNumber = strFromInt(iCargoSpace, TRUE);
	if (pCargoExtension)
		pData->sHeading = strPatternSubst(CONSTLIT("ton %s"), pCargoExtension->GetItemType()->GetNounPhrase(nounActual));
	else
		pData->sHeading = CONSTLIT("ton cargo hold");
	if (iCargoSpace < pClass->GetMaxCargoSpace())
		pData->sDesc = strPatternSubst(CONSTLIT("optional expansion up to %d tons"), pClass->GetMaxCargoSpace());
	else
		pData->sDesc = CONSTLIT("cargo space cannot be expanded");
	if (pCargoExtension)
		pData->pItemIcon = pCargoExtension->GetItemType();
	else
		pData->pItemIcon = g_pUniverse->FindItemType(CARGO_HOLD_EXPANSION_UNID);

	//	Add remaining devices

	for (i = 0; i < Devices.GetCount() && iAddedToLeft < 6; i++)
		{
		CDeviceClass *pDevice = Devices.GetDeviceClass(i);
		CItemType *pItem = pDevice->GetItemType();

		//	Skip standard devices

		if (pDevice->GetCategory() == itemcatDrive
				|| pDevice->GetCategory() == itemcatReactor
				|| pDevice->GetCategory() == itemcatCargoHold
				|| pDevice->GetCategory() == itemcatWeapon
				|| pDevice->GetCategory() == itemcatLauncher
				|| pDevice->GetCategory() == itemcatShields)
			continue;

		SDataBox *pData = AddDataBox();
		pData->theItem = CItem(pItem, 1);
		pData->dwFlags = (iAddedToLeft == 5 ? FLAG_ALIGN_RIGHT : FLAG_ALIGN_LEFT);
		pData->pItemIcon = pItem;
		pData->sHeading = pItem->GetNounPhrase(nounActual);
		
		CString sReference = pItem->GetReference(Ctx, 0, CItemType::FLAG_ACTUAL_ITEM);
		if (sReference.IsBlank())
			pData->sDesc = strPatternSubst(CONSTLIT("Level %s"), strLevel(pItem->GetLevel()));
		else
			pData->sDesc = strPatternSubst(CONSTLIT("Level %s — %s"),
					strLevel(pItem->GetLevel()),
					sReference);

		iAddedToLeft++;
		}

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

	pData = AddDataBox();
	pData->dwFlags = FLAG_ALIGN_RIGHT;
	pData->sNumber = strPatternSubst(CONSTLIT("%d"), iAllLeft);
	pData->sHeading = (iAllLeft == 1 ? CONSTLIT("device slot for expansion") : CONSTLIT("device slots for expansion"));
	if (iWeaponsLeft != iAllLeft && iNonWeaponsLeft != iAllLeft)
		pData->sDesc = strPatternSubst(CONSTLIT("only %d for weapons; only %d for non-weapons"), iWeaponsLeft, iNonWeaponsLeft);
	else if (iWeaponsLeft != iAllLeft)
		pData->sDesc = strPatternSubst(CONSTLIT("only %d device slot%p available for weapons"), iWeaponsLeft);
	else if (iNonWeaponsLeft != iAllLeft)
		pData->sDesc = strPatternSubst(CONSTLIT("only %d device slot%p available for non-weapons"), iNonWeaponsLeft);
	pData->pItemIcon = (CItemType *)SPECIAL_ICON_DEVICE_SLOTS;

	//	Count the number of data boxes with icons. We use this to roughly
	//	align the sections vertically.

	m_iLeftCount = 0;
	m_iRightCount = 0;
	for (i = 0; i < m_iDataBoxCount; i++)
		if (m_DataBoxes[i].pItemIcon)
			{
			if (m_DataBoxes[i].dwFlags & FLAG_ALIGN_LEFT)
				m_iLeftCount++;
			else
				m_iRightCount++;
			}
	}

bool CShipClassDisplay::OnKeyDown (int iVirtKey)

//	OnKeyDown
//
//	Handle keydown

	{
	switch (iVirtKey)
		{
		case VK_RIGHT:
		case VK_DOWN:
			SelectNext();
			break;

		case VK_LEFT:
		case VK_UP:
			SelectPrev();
			break;
		}

	return false;
	}

void CShipClassDisplay::Paint (CG16bitImage &Dest)

//	Paint
//
//	Paints the display

	{
	if (m_bInvalid)
		{
		PaintBuffer();
		m_bInvalid = false;
		}

	Dest.Blt(0,
			0,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			m_Buffer,
			m_rcRect.left,
			m_rcRect.top);
	}

void CShipClassDisplay::PaintBuffer (void)

//	PaintBuffer
//
//	Paint off-screen buffer

	{
	int i;
	int x, y;
	int cxWidth, cyHeight;
	RECT rcRect;

	//	Paint the background

	m_Buffer.Fill(0, 
			0, 
			RectWidth(m_rcRect), 
			RectHeight(m_rcRect),
			RGB_BACKGROUND);

	//	Some variables that we need

	CShipClass *pClass = m_PlayerShipList[m_iPlayerShip].pClass;
	const CPlayerSettings *pPlayerSettings = pClass->GetPlayerSettings();

	//	Paint the center pane

	if (m_pShipImage)
		{
		x = m_rcCenterPane.left + (RectWidth(m_rcCenterPane) - m_pShipImage->GetWidth()) / 2;
		y = m_rcCenterPane.top;

		m_Buffer.Blt(0, 0, m_pShipImage->GetWidth(), m_pShipImage->GetHeight(), *m_pShipImage, x, y);
		}
	else
		{
		x = m_rcCenterPane.left + RectWidth(m_rcCenterPane) / 2;
		y = m_rcCenterPane.top + RectHeight(m_rcCenterPane) / 2;
		ViewportTransform Trans;

		pClass->Paint(m_Buffer, 
				x, 
				y, 
				Trans, 
				0, 
				0,
				false,
				false
				);
		}

	//	Paint the bottom pane, starting with the name of this ship class

	CString sShipName = m_PlayerShipList[m_iPlayerShip].sName;
	cxWidth = m_pFonts->SubTitle.MeasureText(sShipName, &cyHeight);
	m_pFonts->SubTitle.DrawText(m_Buffer,
			m_rcBottomPane.left + (RectWidth(m_rcBottomPane) - cxWidth) / 2,
			m_rcBottomPane.top,
			m_pFonts->wTitleColor,
			sShipName);

	rcRect = m_rcBottomPane;
	rcRect.top += cyHeight;
	m_pFonts->Medium.DrawText(m_Buffer,
			rcRect,
			RGB_DESCRIPTION,
			pPlayerSettings->GetDesc(),
			0,
			CG16bitFont::AlignCenter);

	//	Paint the left pane data boxes

	x = m_rcLeftPane.left;
	y = m_rcLeftPane.top;
	if (m_iLeftCount < 5)
		y += ((5 - m_iLeftCount) * ICON_HEIGHT) / 2;

	int yLastIcon = y;
	for (i = 0; i < m_iDataBoxCount; i++)
		if (m_DataBoxes[i].dwFlags & FLAG_ALIGN_LEFT)
			PaintNextDataBox(x, y, m_DataBoxes[i], yLastIcon);

	//	Paint the right pane data boxes

	x = m_rcRightPane.right - DATA_BOX_WIDTH;
	y = m_rcRightPane.top;
	if (m_iRightCount < 5)
		y += ((5 - m_iRightCount) * ICON_HEIGHT) / 2;

	for (i = 0; i < m_iDataBoxCount; i++)
		if (m_DataBoxes[i].dwFlags & FLAG_ALIGN_RIGHT)
			PaintNextDataBox(x, y, m_DataBoxes[i], yLastIcon);
	}

void CShipClassDisplay::PaintNextDataBox (int x, int &y, const SDataBox &DataBox, int &yLastIcon)

//	PaintNextDataBox
//
//	Paints a data box within a loop

	{
	//	If we're painting an icon and we don't have enough room from
	//	the previous icon, skip a bit

	if (DataBox.pItemIcon && yLastIcon < y && (y - yLastIcon) < ICON_HEIGHT)
		y = yLastIcon + ICON_HEIGHT;

	//	Otherwise, if we're painting an icon and we're flush to the top, and the previous
	//	item is long text, then add a bit of space

	else if (DataBox.pItemIcon && (DataBox.dwFlags & FLAG_CONTINUES) && (y - yLastIcon) > ICON_HEIGHT)
		y += 2 * ITEM_SPACING_Y;

	//	Adjust x coordinate so that entries curve inward

	if (y < 200)
		{
		int xCurve = ((200 - y) * (200 - y)) / 500;
		if (DataBox.dwFlags & FLAG_ALIGN_LEFT)
			x += xCurve;
		else
			x -= xCurve;
		}

	int cyHeight;
	PaintDataBox(m_Buffer, x, y, DataBox, &cyHeight);

	if (DataBox.pItemIcon)
		yLastIcon = y;

	y += cyHeight;
	}

void CShipClassDisplay::PaintDataBox (CG16bitImage &Dest, int x, int y, const SDataBox &DataBox, int *retcyHeight)

//	PaintDataBox
//
//	Paints a data box

	{
	int cyHeight = 0;

	//	Paint the device icon

	if (DataBox.pItemIcon)
		{
		int xDest;
		if (DataBox.dwFlags & FLAG_ALIGN_LEFT)
			xDest = x;
		else
			xDest = x + DATA_BOX_WIDTH - ICON_WIDTH;

		if ((int)DataBox.pItemIcon <= SPECIAL_ICON_DEVICE_SLOTS)
			{
			int xSrc = -1;
			int ySrc = -1;

			switch ((int)DataBox.pItemIcon)
				{
				case SPECIAL_ICON_DEVICE_SLOTS:
					xSrc = ICON_DEVICE_SLOTS_X;
					ySrc = ICON_DEVICE_SLOTS_Y;
					break;
				}

			if (xSrc != -1)
				{
				Dest.Blt(xSrc, ySrc, SPECIAL_ICON_WIDTH, SPECIAL_ICON_HEIGHT, m_Images, xDest, y);
				}
			}
		else
			DrawItemTypeIcon(Dest, xDest, y, DataBox.pItemIcon);

		cyHeight += ICON_HEIGHT;
		}

	//	Figure out the font and color of the header

	const CG16bitFont *pHeaderFont;
	WORD HeaderRGB;
	if (DataBox.dwFlags & FLAG_PLAIN_HEADER)
		{
		pHeaderFont = &m_pFonts->Medium;
		HeaderRGB = RGB_ITEM_DESC;
		}
	else
		{
		pHeaderFont = &m_pFonts->MediumHeavyBold;
		HeaderRGB = RGB_ITEM_TITLE;
		}

	//	Compute the rect for the text

	RECT rcRect;
	if (DataBox.dwFlags & FLAG_ALIGN_LEFT)
		rcRect.left = x + ICON_WIDTH + DATA_BOX_INNER_SPACING_X;
	else
		rcRect.left = x;
	rcRect.right = rcRect.left + DATA_BOX_TEXT_WIDTH;
	rcRect.top = y;
	rcRect.bottom = m_rcRect.bottom;

	//	Measure the height of the desc text

	int cyDescText;
	if (DataBox.dwFlags & FLAG_DAMAGE_TYPE)
		{
		m_pFonts->Medium.DrawText(Dest, rcRect, 0, DataBox.sDesc2, 0, CG16bitFont::MeasureOnly, &cyDescText);
		cyDescText += m_pFonts->Medium.GetHeight();
		}
	else if (DataBox.dwFlags & FLAG_DAMAGE_ADJ)
		{
		m_pFonts->Medium.DrawText(Dest, rcRect, 0, DataBox.sDesc, 0, CG16bitFont::MeasureOnly, &cyDescText);
		cyDescText += m_pFonts->Medium.GetHeight();
		}
	else
		m_pFonts->Medium.DrawText(Dest, rcRect, 0, DataBox.sDesc, 0, CG16bitFont::MeasureOnly, &cyDescText);

	//	Compute the size of the overall box

	int cyText = cyDescText;

	//	Account for the heading

	if (!DataBox.sNumber.IsBlank())
		cyText += m_pFonts->LargeBold.GetHeight();
	else
		cyText += pHeaderFont->GetHeight();

	//	Center the text on the icon

	if (cyText < ICON_HEIGHT && DataBox.pItemIcon && !(DataBox.dwFlags & FLAG_CONTINUES))
		rcRect.top = y + (ICON_HEIGHT - cyText) / 2;

	//	Paint the heading

	if (DataBox.dwFlags & FLAG_ALIGN_LEFT)
		{
		int xOldLeft = rcRect.left;
		if (!DataBox.sNumber.IsBlank())
			{
			m_pFonts->LargeBold.DrawText(Dest, rcRect.left, rcRect.top, RGB_ITEM_NUMBER, DataBox.sNumber, 0, (int *)&rcRect.left);
			rcRect.left += ITEM_SPACING_X;
			rcRect.top += m_pFonts->LargeBold.GetAscent() - pHeaderFont->GetAscent();
			}

		pHeaderFont->DrawText(Dest,
				rcRect,
				HeaderRGB,
				DataBox.sHeading,
				0,
				CG16bitFont::SmartQuotes | CG16bitFont::TruncateLine);
		rcRect.left = xOldLeft;
		rcRect.top += pHeaderFont->GetHeight();
		}
	else
		{
		int xOldLeft = rcRect.left;
		int yOldTop = rcRect.top;

		int cxNumber = 0;
		if (!DataBox.sNumber.IsBlank())
			{
			cxNumber = m_pFonts->LargeBold.MeasureText(DataBox.sNumber);
			rcRect.top += m_pFonts->LargeBold.GetAscent() - pHeaderFont->GetAscent();
			rcRect.left += cxNumber + ITEM_SPACING_X;
			}

		int cxHeading = Min(pHeaderFont->MeasureText(DataBox.sHeading), RectWidth(rcRect));
		pHeaderFont->DrawText(Dest,
				rcRect,
				HeaderRGB,
				DataBox.sHeading,
				0,
				CG16bitFont::SmartQuotes | CG16bitFont::TruncateLine | CG16bitFont::AlignRight);
		rcRect.top = yOldTop;
		rcRect.left = xOldLeft;

		if (!DataBox.sNumber.IsBlank())
			{
			//	COMPILER BUG: VS2005 Version 8.0.50727.762  (SP.050727-7600)
			//	Adding an extra call here prevents a code generation bug that
			//	is triggered by call optimization (or something)
			CG16bitImage Temp;

			m_pFonts->LargeBold.DrawText(Dest, 
					rcRect.right - (cxNumber + cxHeading + ITEM_SPACING_X), 
					rcRect.top, 
					RGB_ITEM_NUMBER, 
					DataBox.sNumber, 
					0);
			rcRect.top += m_pFonts->LargeBold.GetHeight();
			}
		else
			rcRect.top += pHeaderFont->GetHeight();
		}

	//	Paint the description

	if (DataBox.dwFlags & FLAG_DAMAGE_TYPE)
		{
		if (DataBox.dwFlags & FLAG_ALIGN_RIGHT)
			{
			int x = rcRect.left;
			m_pFonts->Medium.DrawText(Dest, x, rcRect.top, 0, DataBox.sDesc, CG16bitFont::MeasureOnly, &x);
			int cx = (x - rcRect.left) + 18;
			m_pTrans->GetUIRes().DrawReferenceDamageType(Dest, rcRect.right - cx, rcRect.top, DataBox.iDamageType, DataBox.sDesc);
			}
		else
			m_pTrans->GetUIRes().DrawReferenceDamageType(Dest, rcRect.left, rcRect.top, DataBox.iDamageType, DataBox.sDesc);

		rcRect.top += m_pFonts->Medium.GetHeight();

		if (!DataBox.sDesc2.IsBlank())
			{
			DWORD dwFlags = CG16bitFont::SmartQuotes;
			if (DataBox.dwFlags & FLAG_ALIGN_RIGHT)
				dwFlags |= CG16bitFont::AlignRight;

			m_pFonts->Medium.DrawText(Dest,
					rcRect,
					RGB_ITEM_DESC,
					DataBox.sDesc2,
					0,
					dwFlags);
			}
		}
	else if (DataBox.dwFlags & FLAG_DAMAGE_ADJ)
		{
		int x = rcRect.left;
		CString sText = strPatternSubst(CONSTLIT("hp: %d"), DataBox.iHP);
		m_pFonts->Medium.DrawText(Dest, x, rcRect.top, m_pFonts->wItemRef, sText, 0, &x);

		m_pTrans->GetUIRes().DrawReferenceDamageAdj(Dest, 
				x + DAMAGE_ADJ_SPACING_X, 
				rcRect.top, 
				DataBox.theItem.GetType()->GetLevel(), 
				DataBox.iHP,
				DataBox.iDamageTypeAdj);

		rcRect.top += m_pFonts->Medium.GetHeight();

		if (!DataBox.sDesc.IsBlank())
			{
			DWORD dwFlags = CG16bitFont::SmartQuotes;
			if (DataBox.dwFlags & FLAG_ALIGN_RIGHT)
				dwFlags |= CG16bitFont::AlignRight;

			m_pFonts->Medium.DrawText(Dest,
					rcRect,
					RGB_ITEM_DESC,
					DataBox.sDesc,
					0,
					dwFlags);
			}
		}
	else
		{
		if (!DataBox.sDesc.IsBlank())
			{
			DWORD dwFlags = CG16bitFont::SmartQuotes;
			if (DataBox.dwFlags & FLAG_ALIGN_RIGHT)
				dwFlags |= CG16bitFont::AlignRight;

			m_pFonts->Medium.DrawText(Dest,
					rcRect,
					RGB_ITEM_DESC,
					DataBox.sDesc,
					0,
					dwFlags);

			rcRect.top += cyDescText;
			}
		}

	//	If the text continues in the next data box, then don't include the
	//	icon in the height. Otherwise, increment the height if more than the icon

	if (DataBox.dwFlags & FLAG_CONTINUES)
		cyHeight = (rcRect.top - y) + ITEM_SPACING_Y;
	else if (rcRect.top - y > cyHeight)
		cyHeight = (rcRect.top - y) + ITEM_SPACING_Y;

	//	Done

	if (retcyHeight)
		*retcyHeight = cyHeight;
	}

void CShipClassDisplay::SelectNext (void)

//	SelectNext
//
//	Select the next ship

	{
	m_iPlayerShip = (m_iPlayerShip + 1) % m_iPlayerShipCount;
	m_pTrans->SetPlayerShip(m_PlayerShipList[m_iPlayerShip].pClass->GetUNID());
	InitShipData();
	m_bInvalid = true;
	}

void CShipClassDisplay::SelectPrev (void)

//	SelectPrev
//
//	Select the previous ship

	{
	m_iPlayerShip = (m_iPlayerShip + m_iPlayerShipCount - 1) % m_iPlayerShipCount;
	m_pTrans->SetPlayerShip(m_PlayerShipList[m_iPlayerShip].pClass->GetUNID());
	InitShipData();
	m_bInvalid = true;
	}

void CShipClassDisplay::Update (void)

//	Update
//
//	Updates the display

	{
	}
