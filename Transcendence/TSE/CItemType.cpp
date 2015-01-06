//	CItemType.cpp
//
//	CItemType class

#include "PreComp.h"

#define ARMOR_TAG								CONSTLIT("Armor")
#define AUTO_DEFENSE_CLASS_TAG					CONSTLIT("AutoDefenseDevice")
#define CARGO_HOLD_CLASS_TAG					CONSTLIT("CargoHoldDevice")
#define CYBER_DECK_CLASS_TAG					CONSTLIT("CyberDeckDevice")
#define DOCK_SCREENS_TAG						CONSTLIT("DockScreens")
#define DRIVE_CLASS_TAG							CONSTLIT("DriveDevice")
#define ENHANCER_CLASS_TAG						CONSTLIT("EnhancerDevice")
#define EVENTS_TAG								CONSTLIT("Events")
#define GLOBAL_DATA_TAG							CONSTLIT("GlobalData")
#define IMAGE_TAG								CONSTLIT("Image")
#define COCKPIT_USE_TAG							CONSTLIT("Invoke")
#define LANGUAGE_TAG							CONSTLIT("Language")
#define MISCELLANEOUS_CLASS_TAG					CONSTLIT("MiscellaneousDevice")
#define MISSILE_TAG								CONSTLIT("Missile")
#define NAMES_TAG								CONSTLIT("Names")
#define REACTOR_CLASS_TAG						CONSTLIT("ReactorDevice")
#define REPAIRER_CLASS_TAG						CONSTLIT("RepairerDevice")
#define SHIELD_CLASS_TAG						CONSTLIT("Shields")
#define STATIC_DATA_TAG							CONSTLIT("StaticData")
#define SOLAR_DEVICE_CLASS_TAG					CONSTLIT("SolarDevice")
#define WEAPON_CLASS_TAG						CONSTLIT("Weapon")

#define INSTANCE_DATA_ATTRIB					CONSTLIT("charges")
#define DATA_ATTRIB								CONSTLIT("data")
#define ENHANCEMENT_ATTRIB						CONSTLIT("enhancement")
#define FREQUENCY_ATTRIB						CONSTLIT("frequency")
#define ENABLED_ONLY_ATTRIB						CONSTLIT("enabledOnly")
#define INSTALLED_ONLY_ATTRIB					CONSTLIT("installedOnly")
#define KEY_ATTRIB								CONSTLIT("key")
#define LEVEL_ATTRIB							CONSTLIT("level")
#define MASS_BONUS_PER_CHARGE_ATTRIB			CONSTLIT("massBonusPerCharge")
#define NUMBER_APPEARING_ATTRIB					CONSTLIT("numberAppearing")
#define PLURAL_NAME_ATTRIB						CONSTLIT("pluralName")
#define REVERSE_ARTICLE_ATTRIB					CONSTLIT("reverseArticle")
#define SECOND_PLURAL_ATTRIB					CONSTLIT("secondPlural")
#define SHOW_REFERENCE_ATTRIB					CONSTLIT("showReference")
#define SORT_NAME_ATTRIB						CONSTLIT("sortName")
#define UNID_ATTRIB								CONSTLIT("UNID")
#define UNINSTALLED_ONLY_ATTRIB					CONSTLIT("uninstalledOnly")
#define UNKNOWN_TYPE_ATTRIB						CONSTLIT("unknownType")
#define USE_ENABLED_ONLY_ATTRIB					CONSTLIT("useEnabledOnly")
#define USE_INSTALLED_ONLY_ATTRIB				CONSTLIT("useInstalledOnly")
#define USE_KEY_ATTRIB							CONSTLIT("useKey")
#define USE_SCREEN_ATTRIB						CONSTLIT("useScreen")
#define USE_UNINSTALLED_ONLY_ATTRIB				CONSTLIT("useUninstalledOnly")
#define VALUE_ATTRIB							CONSTLIT("value")
#define VALUE_BONUS_PER_CHARGE_ATTRIB			CONSTLIT("valueBonusPerCharge")
#define VALUE_CHARGES_ATTRIB					CONSTLIT("valueCharges")
#define VIRTUAL_ATTRIB							CONSTLIT("virtual")

#define GET_TRADE_PRICE_EVENT					CONSTLIT("GetTradePrice")
#define ON_ENABLED_EVENT						CONSTLIT("OnEnable")
#define ON_INSTALL_EVENT						CONSTLIT("OnInstall")
#define ON_REFUEL_TAG							CONSTLIT("OnRefuel")

#define STR_FUEL								CONSTLIT("Fuel")
#define STR_MISSILE								CONSTLIT("Missile")

#define CATEGORY_AMMO							CONSTLIT("ammo")
#define CATEGORY_ARMOR							CONSTLIT("armor")
#define CATEGORY_CARGO_HOLD						CONSTLIT("cargoHold")
#define CATEGORY_DRIVE							CONSTLIT("drive")
#define CATEGORY_FUEL							CONSTLIT("fuelItem")
#define CATEGORY_LAUNCHER						CONSTLIT("launcher")
#define CATEGORY_MISC_DEVICE					CONSTLIT("miscDevice")
#define CATEGORY_MISC_ITEM						CONSTLIT("miscItem")
#define CATEGORY_REACTOR						CONSTLIT("reactor")
#define CATEGORY_SHIELDS						CONSTLIT("shields")
#define CATEGORY_USABLE_ITEM					CONSTLIT("usableItem")
#define CATEGORY_WEAPON							CONSTLIT("weapon")

#define FIELD_AVERAGE_COUNT						CONSTLIT("averageCount")
#define FIELD_CATEGORY							CONSTLIT("category")
#define FIELD_COST								CONSTLIT("cost")
#define FIELD_DESCRIPTION						CONSTLIT("description")
#define FIELD_DEVICE_SLOTS						CONSTLIT("deviceSlots")
#define FIELD_FREQUENCY							CONSTLIT("frequency")
#define FIELD_FUEL_CAPACITY						CONSTLIT("fuelCapacity")
#define FIELD_IMAGE_DESC						CONSTLIT("imageDesc")
#define FIELD_INSTALL_COST						CONSTLIT("installCost")
#define FIELD_LEVEL								CONSTLIT("level")
#define FIELD_MASS								CONSTLIT("mass")
#define FIELD_NAME								CONSTLIT("name")
#define FIELD_REFERENCE							CONSTLIT("reference")
#define FIELD_SHORT_NAME						CONSTLIT("shortName")
#define FIELD_SLOT_CATEGORY						CONSTLIT("slotCategory")
#define FIELD_TREASURE_VALUE					CONSTLIT("treasureValue")
#define FIELD_UNKNOWN_TYPE						CONSTLIT("unknownType")
#define FIELD_USE_KEY							CONSTLIT("useKey")

#define SPECIAL_CAN_BE_DAMAGED					CONSTLIT("canBeDamaged:")
#define SPECIAL_DAMAGE_TYPE						CONSTLIT("damageType:")
#define SPECIAL_IS_LAUNCHER						CONSTLIT("isLauncher:")
#define SPECIAL_LAUNCHED_BY						CONSTLIT("launchedBy:")
#define SPECIAL_PROPERTY						CONSTLIT("property:")

#define SPECIAL_TRUE							CONSTLIT("true")

static char g_NameAttrib[] = "name";
static char g_ObjectAttrib[] = "object";
static char g_MassAttrib[] = "mass";
static char g_DescriptionAttrib[] = "description";
static char g_RandomDamagedAttrib[] = "randomDamaged";

const int FLOTSAM_IMAGE_WIDTH =					32;

static char *CACHED_EVENTS[CItemType::evtCount] =
	{
		"GetDescription",
		"GetName",
		"GetTradePrice",
		"OnInstall",
		"OnEnable",
		"OnRefuel",
	};

static CStationType *g_pFlotsamStationType = NULL;

CItemType::CItemType (void) : 
		m_dwSpare(0),
		m_pUseCode(NULL),
		m_pArmor(NULL),
		m_pDevice(NULL),
		m_pMissile(NULL)

//	CItemType constructor

	{
	}

CItemType::~CItemType (void)

//	CItemType destructor

	{
	if (m_pUseCode)
		m_pUseCode->Discard(&g_pUniverse->GetCC());

	if (m_pArmor)
		delete m_pArmor;

	if (m_pDevice)
		delete m_pDevice;

	if (m_pMissile)
		delete m_pMissile;
	}

bool CItemType::IsAmmunition (void) const

//	IsAmmunition
//
//	Determines whether this is a missile for a launcher or just ammunition for a normal weapon.

	{
	if (!IsMissile())
		return false;

	CDeviceClass *pWeapon;
	if (!CDeviceClass::FindWeaponFor((CItemType *)this, &pWeapon))
		return false;

	return (pWeapon->GetItemType()->GetCategory() != itemcatLauncher);
	}

void CItemType::CreateEmptyFlotsam (CSystem *pSystem, 
									const CVector &vPos, 
									const CVector &vVel, 
									CSovereign *pSovereign, 
									CStation **retpFlotsam)

//	CreateEmptyFlotsam
//
//	Creates an empty flotsam station

	{
	SObjCreateCtx CreateCtx;
	CreateCtx.vPos = vPos;
	CreateCtx.vVel = vVel;

	CStation *pFlotsam;
	CStation::CreateFromType(pSystem,
			GetFlotsamStationType(),
			CreateCtx,
			&pFlotsam);

	//	Set properties

	pFlotsam->SetSovereign(pSovereign);
	pFlotsam->SetFlotsamImage(this);

	DWORD dwFlags;
	CString sName = GetName(&dwFlags);
	pFlotsam->SetName(sName, dwFlags);

	//	Done

	if (retpFlotsam)
		*retpFlotsam = pFlotsam;
	}

void CItemType::CreateFlotsamImage (void)

//	CreateFlotsamImage
//
//	Creates a 32x32 image used when the item is floating in space.

	{
	if (!m_Image.IsLoaded())
		return;

	RECT rcImage = m_Image.GetImageRect();
	int cxWidth = RectWidth(rcImage);
	int cyHeight = RectHeight(rcImage);

	//	Create a resized version of the image

	Metric rScale = (Metric)FLOTSAM_IMAGE_WIDTH / (Metric)cxWidth;
	m_FlotsamBitmap.CreateFromImageTransformed(m_Image.GetImage(CONSTLIT("Flotsam image")),
			rcImage.left,
			rcImage.top,
			cxWidth,
			cyHeight,
			rScale,
			rScale,
			0.0);

	//	Initialize an image

	RECT rcRect;
	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = m_FlotsamBitmap.GetWidth();
	rcRect.bottom = m_FlotsamBitmap.GetHeight();
	m_FlotsamImage.Init(&m_FlotsamBitmap, rcRect, 0, 0, false);
	}

bool CItemType::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	//	Deal with the meta-data that we know about

	if (strEquals(sField, FIELD_LEVEL))
		*retsValue = strFromInt(GetLevel());

	else if (strEquals(sField, FIELD_CATEGORY))
		*retsValue = GetItemCategory(GetCategory());
	
	else if (strEquals(sField, FIELD_SLOT_CATEGORY))
		*retsValue = GetItemCategory(GetSlotCategory());
	
	else if (strEquals(sField, FIELD_FREQUENCY))
		{
		switch (GetFrequency())
			{
			case ftCommon:
				*retsValue = CONSTLIT("C");
				break;

			case ftUncommon:
				*retsValue = CONSTLIT("UC");
				break;

			case ftRare:
				*retsValue = CONSTLIT("R");
				break;

			case ftVeryRare:
				*retsValue = CONSTLIT("VR");
				break;

			default:
				*retsValue = CONSTLIT("NR");
			}
		}
	else if (strEquals(sField, FIELD_FUEL_CAPACITY) && IsFuel())
		{
		int iFuelPerItem = strToInt(GetData(), 0);
		*retsValue = strFromInt(iFuelPerItem / FUEL_UNITS_PER_STD_ROD);

		//	NOTE: When IsFuel() is FALSE we fall through to the else
		//	case, which asks reactor devices.
		}
	else if (strEquals(sField, FIELD_NAME))
		*retsValue = GetNounPhrase();

	else if (strEquals(sField, FIELD_MASS))
		*retsValue = strFromInt(CItem(this, 1).GetMassKg());
	
	else if (strEquals(sField, FIELD_SHORT_NAME))
		{
		CString sName = GetNounPhrase();
		int iPos = strFind(sName, CONSTLIT(" of "));
		if (iPos != -1)
			{
			CString sLead = strSubString(sName, 0, iPos);
			if (strEquals(sLead, CONSTLIT("barrel"))
					|| strEquals(sLead, CONSTLIT("bolt"))
					|| strEquals(sLead, CONSTLIT("case"))
					|| strEquals(sLead, CONSTLIT("chest"))
					|| strEquals(sLead, CONSTLIT("container"))
					|| strEquals(sLead, CONSTLIT("crate"))
					|| strEquals(sLead, CONSTLIT("keg"))
					|| strEquals(sLead, CONSTLIT("kilo"))
					|| strEquals(sLead, CONSTLIT("nodule"))
					|| strEquals(sLead, CONSTLIT("pair"))
					|| strEquals(sLead, CONSTLIT("ROM"))
					|| strEquals(sLead, CONSTLIT("segment"))
					|| strEquals(sLead, CONSTLIT("tank"))
					|| strEquals(sLead, CONSTLIT("tin"))
					|| strEquals(sLead, CONSTLIT("ton"))
					|| strEquals(sLead, CONSTLIT("vial")))
				*retsValue = strSubString(sName, iPos+4, -1);
			else
				*retsValue = sName;
			}
		else
			*retsValue = sName;
		}
	else if (strEquals(sField, FIELD_REFERENCE))
		*retsValue = GetReference(CItemCtx());

	else if (strEquals(sField, FIELD_COST))
		*retsValue = strFromInt(GetValue(CItemCtx()));

	else if (strEquals(sField, FIELD_INSTALL_COST))
		{
		int iCost = GetInstallCost();
		if (iCost == -1)
			*retsValue = NULL_STR;
		else
			*retsValue = strFromInt(iCost);
		}
	else if (strEquals(sField, FIELD_AVERAGE_COUNT))
		*retsValue = strFromInt(m_NumberAppearing.GetAveValue());

	else if (strEquals(sField, FIELD_DESCRIPTION))
		*retsValue = m_sDescription;

	else if (strEquals(sField, FIELD_IMAGE_DESC))
		{
		const RECT &rcImage = m_Image.GetImageRect();
		*retsValue = strPatternSubst(CONSTLIT("%s (%d,%d)"),
				m_Image.GetFilename(),
				rcImage.left,
				rcImage.top);
		}
	else if (strEquals(sField, FIELD_DEVICE_SLOTS))
		{
		if (m_pDevice)
			*retsValue = strFromInt(m_pDevice->GetSlotsRequired());
		else
			*retsValue = NULL_STR;
		}
	else if (strEquals(sField, FIELD_TREASURE_VALUE))
		*retsValue = strFromInt((int)CEconomyType::ExchangeToCredits(GetCurrencyType(), GetValue(CItemCtx(), true)));

	else if (strEquals(sField, FIELD_UNKNOWN_TYPE))
		*retsValue = (m_pUnknownType ? strPatternSubst(CONSTLIT("0x%x"), m_pUnknownType->GetUNID()) : NULL_STR);

	else if (strEquals(sField, FIELD_USE_KEY))
		*retsValue = m_sUseKey;

	//	Otherwise, see if the device class knows

	else
		{
		bool bHandled;

		if (m_pDevice)
			bHandled = m_pDevice->FindDataField(sField, retsValue);
		else if (m_pArmor)
			bHandled = m_pArmor->FindDataField(sField, retsValue);

		//	If this is a missile, then find the device that launches
		//	it and ask it for the properties.

		else if (IsMissile())
			bHandled = CDeviceClass::FindAmmoDataField(this, sField, retsValue);
		else
			bHandled = false;
		
		//	If someone handled it, then we're done. Otherwise, pass it back to
		//	the design type.
		
		if (bHandled)
			return true;

		return CDesignType::FindDataField(sField, retsValue);
		}

	return true;
	}

CDeviceClass *CItemType::GetAmmoLauncher (int *retiVariant) const

//	GetAmmoLauncher
//
//	Returns a weapon that can launch this ammo (or NULL)

	{
	int i;

	for (i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
		{
		CItemType *pType = g_pUniverse->GetItemType(i);
		CDeviceClass *pWeapon;

		if (pType->IsDevice() 
				&& (pWeapon = pType->GetDeviceClass()))
			{
			int iVariant = pWeapon->GetAmmoVariant(this);
			if (iVariant != -1)
				{
				if (retiVariant)
					*retiVariant = iVariant;
				return pWeapon;
				}
			}
		}

	return NULL;
	}

int CItemType::GetApparentLevel (void) const

//	GetApparentLevel
//
//	Returns the level that the item appears to be

	{
	if (!IsKnown())
		return m_pUnknownType->GetLevel();

	return GetLevel();
	}

ItemCategories CItemType::GetCategory (void) const

//	GetCategory
//
//	Returns the category of the item

	{
	if (m_pArmor)
		return itemcatArmor;
	else if (m_pDevice)
		return m_pDevice->GetCategory();
	else
		{
		if (IsUsable())
			return itemcatUseful;
		else if (IsFuel())
			return itemcatFuel;
		else if (IsMissile())
			return itemcatMissile;
		else
			return itemcatMisc;
		}
	}

const CString &CItemType::GetDesc (void) const

//	GetDesc
//
//	Get description for the item
	
	{
	if (!IsKnown())
		return m_pUnknownType->GetDesc();

	return m_sDescription; 
	}

CStationType *CItemType::GetFlotsamStationType (void)

//	GetFlotsamStationType
//
//	Returns the station type to use for flotsam

	{
	if (g_pFlotsamStationType == NULL)
		g_pFlotsamStationType = g_pUniverse->FindStationType(FLOTSAM_UNID);

	return g_pFlotsamStationType;
	}

int CItemType::GetFrequencyByLevel (int iLevel)

//	GetFrequencyByLevel
//
//	Returns the frequency of the item at the given system level.

	{
	int iFrequency = GetFrequency();
	int iItemLevel = GetLevel();
	int iDiff = Absolute(iItemLevel - iLevel);

	switch (iDiff)
		{
		case 0:
			return iFrequency;

		case 1:
			return iFrequency * ftUncommon / ftCommon;

		case 2:
			return iFrequency * ftRare / ftCommon;

		case 3:
			return iFrequency * ftVeryRare / ftCommon;

		default:
			return 0;
		}
	}

int CItemType::GetInstallCost (void) const

//	GetInstallCost
//
//	Returns the cost to install the item (or -1 if the item cannot be installed)

	{
	if (m_pArmor)
		return m_pArmor->GetInstallCost();
	else if (m_pDevice)
		return 100 * (((GetApparentLevel() * GetApparentLevel()) + 4) / 5);
	else
		return -1;
	}

CString CItemType::GetItemCategory (ItemCategories iCategory)

//	GetItemCategory
//
//	Returns the identifier for an item category

	{
	switch (iCategory)
		{
		case itemcatArmor:
			return CATEGORY_ARMOR;

		case itemcatCargoHold:
			return CATEGORY_CARGO_HOLD;

		case itemcatDrive:
			return CATEGORY_DRIVE;

		case itemcatFuel:
			return CATEGORY_FUEL;

		case itemcatLauncher:
			return CATEGORY_LAUNCHER;

		case itemcatMisc:
			return CATEGORY_MISC_ITEM;

		case itemcatMissile:
			return CATEGORY_AMMO;

		case itemcatMiscDevice:
			return CATEGORY_MISC_DEVICE;

		case itemcatReactor:
			return CATEGORY_REACTOR;

		case itemcatShields:
			return CATEGORY_SHIELDS;

		case itemcatUseful:
			return CATEGORY_USABLE_ITEM;

		case itemcatWeapon:
			return CATEGORY_WEAPON;

		default:
			ASSERT(false);
			return CONSTLIT("unknown");
		}
	}

int CItemType::GetMassKg (CItemCtx &Ctx) const

//	GetMassKg
//
//	Returns the mass of the item in kilograms

	{
	if (m_iExtraMassPerCharge)
		{
		if (Ctx.IsItemNull())
			return m_iMass + (m_InitDataValue.GetAveValue() * m_iExtraMassPerCharge);
		else
			return m_iMass + (Ctx.GetItem().GetCharges() * m_iExtraMassPerCharge);
		}
	else
		return m_iMass;
	}

int CItemType::GetMaxHPBonus (void) const

//	GetMaxHPBonus
//
//	Returns the maximum possible HP bonus for this item.

	{
	if (m_pArmor)
		return m_pArmor->GetMaxHPBonus();
	else if (m_pDevice)
		return m_pDevice->GetMaxHPBonus();
	else
		return 150;
	}

CString CItemType::GetName (DWORD *retdwFlags, bool bActualName) const

//	GetName
//
//	Returns the name of the item and flags about the name

	{
	if (!IsKnown() && !bActualName && !m_sUnknownName.IsBlank())
		{
		if (retdwFlags)
			*retdwFlags = 0;
		return m_sUnknownName;
		}

	if (retdwFlags)
		{
		*retdwFlags = m_dwNameFlags;
		return m_sName;
		}
	else
		return GetNounPhrase();
	}

CString CItemType::GetNounPhrase (DWORD dwFlags) const

//	GetNounPhrase
//
//	Returns the generic name of the item

	{
	DWORD dwNameFlags;
	CString sName = GetName(&dwNameFlags, (dwFlags & nounActual) != 0);

	return ::ComposeNounPhrase(sName, 1, NULL_STR, dwNameFlags, dwFlags);
	}

CString CItemType::GetReference (CItemCtx &Ctx, int iVariant, DWORD dwFlags) const

//	GetReference
//
//	Returns reference string

	{
	CArmorClass *pArmor;
	CDeviceClass *pDevice;
	int iShotVariant;

	//	No reference if unknown

	if (!IsKnown() && !(dwFlags & FLAG_ACTUAL_ITEM))
		return NULL_STR;

	//	Return armor reference, if this is armor

	if (pArmor = GetArmorClass())
		return pArmor->GetReference(Ctx, iVariant);

	//	Return device reference, if this is a device

	else if (pDevice = GetDeviceClass())
		return pDevice->GetReference(Ctx, iVariant);

	//	If a missile, then get the reference from the weapon

	else if (IsMissile() && (pDevice = GetAmmoLauncher(&iShotVariant)))
		{
		//	We only return reference for missiles (not for ammo, since that is already
		//	accounted for in the weapon)

		if (pDevice->GetCategory() == itemcatLauncher)
			return pDevice->GetReference(Ctx, iShotVariant);
		else
			return NULL_STR;
		}

	//	Otherwise, nothing

	else
		return NULL_STR;
	}

CString CItemType::GetSortName (void) const

//	GetSortName
//
//	Returns the sort name

	{
	if (!IsKnown() && !m_sUnknownName.IsBlank())
		return m_sUnknownName;
	else
		return m_sSortName;
	}

ItemCategories CItemType::GetSlotCategory (void) const

//	GetSlotCategory
//
//	Returns the slot category for device install purposes
	
	{
	CDeviceClass *pDevice = GetDeviceClass();
	if (pDevice)
		return pDevice->GetSlotCategory();

	return GetCategory();
	}

CString CItemType::GetUnknownName (int iIndex, DWORD *retdwFlags)

//	GetUnknownName
//
//	Returns the unknown name of the item

	{
	if (iIndex != -1 && iIndex < m_UnknownNames.GetCount())
		return m_UnknownNames[iIndex];

	return GetName(retdwFlags);
	}

int CItemType::GetValue (CItemCtx &Ctx, bool bActual) const

//	GetValue
//
//	Returns the value of the item in its currency

	{
	//	NOTE: We have got that guaranteed m_pUnknownType is non-NULL if IsKnown is FALSE.

	if (!IsKnown() && !bActual)
		return m_pUnknownType->GetValue(Ctx);

	//	Value in the item's currency.

	int iValue = (int)m_iValue.GetValue();

	//	If we need to account for charges, then do it

	if (m_iExtraValuePerCharge != 0)
		{
		if (Ctx.IsItemNull())
			return Max(0, iValue + (m_InitDataValue.GetAveValue() * m_iExtraValuePerCharge));
		else
			return Max(0, iValue + (Ctx.GetItem().GetCharges() * m_iExtraValuePerCharge));
		}

	else if (m_fValueCharges && !Ctx.IsItemNull())
		{
		int iMaxCharges = GetMaxCharges();

		if (iMaxCharges > 0)
			return (iValue * (1 + Ctx.GetItem().GetCharges())) / (1 + iMaxCharges);
		else
			return iValue;
		}

	//	Otherwise, just the fixed price

	else
		return iValue;
	}

void CItemType::InitRandomNames (void)

//	InitRandomNames
//
//	Initialize random names

	{
	int i;

	//	If we don't have random names for other items then we're done

	int iCount = m_UnknownNames.GetCount();
	if (iCount == 0)
		return;

	//	Randomize the names

	CIntArray Randomize;
	for (i = 0; i < iCount; i++)
		Randomize.AppendElement(i, NULL);

	Randomize.Shuffle();

	//	Keep track of randomly generated names

	TArray<CString> RetiredNames;

	//	Loop over all items and assign each item that has us as the
	//	unknown placeholder.

	int j = 0;
	for (i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
		{
		CItemType *pType = g_pUniverse->GetItemType(i);
		if (pType->m_pUnknownType.GetUNID() == GetUNID())
			{
			CString sTemplate = m_UnknownNames[Randomize.GetElement(j % iCount)];

			//	See if the template has replaceable parameters.

			char *pPos = sTemplate.GetASCIIZPointer();
			while (*pPos != '\0' && *pPos != '%')
				pPos++;

			//	If the template has replaceable parameters, then generate
			//	a unique name

			if (*pPos == '%')
				{
				int iMaxLoops = 100;

				bool bDuplicate;
				do
					{
					pType->m_sUnknownName = ::GenerateRandomNameFromTemplate(sTemplate);
					bDuplicate = false;

					for (int k = 0; k < RetiredNames.GetCount(); k++)
						if (strEquals(pType->m_sUnknownName, RetiredNames[k]))
							{
							bDuplicate = true;
							break;
							}
					}
				while (bDuplicate && iMaxLoops-- > 0);

				RetiredNames.Insert(pType->m_sUnknownName);
				}
			else
				pType->m_sUnknownName = sTemplate;

			j++;
			}
		}
	}

bool CItemType::IsFuel (void) const

//	IsFuel
//
//	Returns TRUE if this is fuel

	{
	return HasLiteralAttribute(STR_FUEL);
	}

bool CItemType::IsMissile (void) const

//	IsMissile
//
//	Returns TRUE if this is a missile

	{
	if (m_pMissile)
		return true;

	return HasLiteralAttribute(STR_MISSILE);
	}

void CItemType::OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	OnAddTypesUsed
//
//	Adds to the list of types used by this type.

	{
	retTypesUsed->SetAt(m_Image.GetBitmapUNID(), true);
	retTypesUsed->SetAt(m_pUnknownType.GetUNID(), true);
	retTypesUsed->SetAt(strToInt(m_pUseScreen.GetUNID(), 0), true);

	if (m_pArmor)
		m_pArmor->AddTypesUsed(retTypesUsed);

	if (m_pDevice)
		m_pDevice->AddTypesUsed(retTypesUsed);

	if (m_pMissile)
		m_pMissile->AddTypesUsed(retTypesUsed);
	}

ALERROR CItemType::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind all pointers to other design elements

	{
	ALERROR error;

	InitRandomNames();

	//	Images

	if (error = m_Image.OnDesignLoadComplete(Ctx))
		return error;

	//	Cache some events

	InitCachedEvents(evtCount, CACHED_EVENTS, m_CachedEvents);

	//	Resolve some pointers

	if (error = m_pUnknownType.Bind(Ctx))
		return error;

	//	Others

	if (error = m_pUseScreen.Bind(Ctx, GetLocalScreens()))
		return error;

	if (error = m_iValue.Bind(Ctx))
		return error;

	//	Call contained objects

	if (m_pDevice)
		if (error = m_pDevice->Bind(Ctx))
			return error;

	if (m_pArmor)
		if (error = m_pArmor->OnBindDesign(Ctx))
			return error;

	if (m_pMissile)
		if (error = m_pMissile->OnDesignLoadComplete(Ctx))
			return error;

	return NOERROR;
	}

ALERROR CItemType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Load data from XML

	{
	ALERROR error;
	int i;

	//	Initialize basic info

	if (!pDesc->FindAttribute(PLURAL_NAME_ATTRIB, &m_sName))
		m_sName = pDesc->GetAttribute(CONSTLIT(g_NameAttrib));

	m_dwNameFlags = ::LoadNameFlags(pDesc);

	m_sSortName = pDesc->GetAttribute(SORT_NAME_ATTRIB);
	if (m_sSortName.IsBlank())
		m_sSortName = m_sName;
	m_iLevel = pDesc->GetAttributeIntegerBounded(LEVEL_ATTRIB, 1, MAX_ITEM_LEVEL, 1);
	m_iMass = pDesc->GetAttributeInteger(CONSTLIT(g_MassAttrib));

	if (error = m_iValue.InitFromXML(Ctx, pDesc->GetAttribute(VALUE_ATTRIB)))
		return ComposeLoadError(Ctx, Ctx.sError);

	//	Initialize frequency

	m_Frequency = (FrequencyTypes)::GetFrequency(pDesc->GetAttribute(FREQUENCY_ATTRIB));
	CString sNumberAppearing = pDesc->GetAttribute(NUMBER_APPEARING_ATTRIB);
	if (sNumberAppearing.IsBlank())
		m_NumberAppearing = DiceRange(0, 0, 1);
	else
		{
		if (error = m_NumberAppearing.LoadFromXML(sNumberAppearing))
			return ComposeLoadError(Ctx, CONSTLIT("Unable to parse numberAppearing"));
		}

	//	Get the unknown type info

	if (error = m_pUnknownType.LoadUNID(Ctx, pDesc->GetAttribute(UNKNOWN_TYPE_ATTRIB)))
		return error;

	m_fKnown = (m_pUnknownType.GetUNID() == 0);

	//	Get the reference info

	m_fDefaultReference = pDesc->GetAttributeBool(SHOW_REFERENCE_ATTRIB);
	m_fReference = m_fDefaultReference;

	//	More fields

	m_sDescription = strCEscapeCodes(pDesc->GetAttribute(CONSTLIT(g_DescriptionAttrib)));
	m_sData = pDesc->GetAttribute(DATA_ATTRIB);
	m_dwModCode = pDesc->GetAttributeInteger(ENHANCEMENT_ATTRIB);

	//	Handle charges

	CString sInstData;
	if (m_fInstanceData = pDesc->FindAttribute(INSTANCE_DATA_ATTRIB, &sInstData))
		m_InitDataValue.LoadFromXML(sInstData);

	m_iExtraMassPerCharge = pDesc->GetAttributeIntegerBounded(MASS_BONUS_PER_CHARGE_ATTRIB, 0, -1, 0);
	m_iExtraValuePerCharge = pDesc->GetAttributeInteger(VALUE_BONUS_PER_CHARGE_ATTRIB);	//	May be negative
	m_fValueCharges = pDesc->GetAttributeBool(VALUE_CHARGES_ATTRIB);

	//	Flags

	m_fRandomDamaged = pDesc->GetAttributeBool(CONSTLIT(g_RandomDamagedAttrib));
	m_fVirtual = pDesc->GetAttributeBool(VIRTUAL_ATTRIB);
	if (m_fVirtual)
		m_Frequency = ftNotRandom;

	//	Use screen

	m_pUseScreen.LoadUNID(Ctx, pDesc->GetAttribute(USE_SCREEN_ATTRIB));
	m_sUseKey = pDesc->GetAttribute(USE_KEY_ATTRIB);
	m_fUseInstalled = pDesc->GetAttributeBool(USE_INSTALLED_ONLY_ATTRIB);
	m_fUseUninstalled = pDesc->GetAttributeBool(USE_UNINSTALLED_ONLY_ATTRIB);
	m_fUseEnabled = pDesc->GetAttributeBool(USE_ENABLED_ONLY_ATTRIB);

	//	Process sub-elements

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pSubDesc = pDesc->GetContentElement(i);

		//	Process image

		if (strEquals(pSubDesc->GetTag(), IMAGE_TAG))
			{
			if (error = m_Image.InitFromXML(Ctx, pSubDesc))
				return ComposeLoadError(Ctx, CONSTLIT("Unable to load image"));
			}

		//	Process unknown names

		else if (strEquals(pSubDesc->GetTag(), NAMES_TAG))
			strDelimitEx(pSubDesc->GetContentText(0), ';', DELIMIT_TRIM_WHITESPACE, 0, &m_UnknownNames);

		//	Process use code

		else if (strEquals(pSubDesc->GetTag(), COCKPIT_USE_TAG))
			{
			m_pUseCode = g_pUniverse->GetCC().Link(pSubDesc->GetContentText(0), 0, NULL);

			//	These are also set in the main desc; we only override if we find them here

			pSubDesc->FindAttribute(KEY_ATTRIB, &m_sUseKey);

			bool bValue;
			if (pSubDesc->FindAttributeBool(INSTALLED_ONLY_ATTRIB, &bValue))
				m_fUseInstalled = bValue;

			if (pSubDesc->FindAttributeBool(UNINSTALLED_ONLY_ATTRIB, &bValue))
				m_fUseUninstalled = bValue;

			if (pSubDesc->FindAttributeBool(ENABLED_ONLY_ATTRIB, &bValue))
				m_fUseEnabled = bValue;
			}

		//	Process on refuel code

		else if (strEquals(pSubDesc->GetTag(), ON_REFUEL_TAG))
			{
			if (error = AddEventHandler(ON_REFUEL_TAG, pSubDesc->GetContentText(0), &Ctx.sError))
				return ComposeLoadError(Ctx, CONSTLIT("Unable to load OnRefuel event"));
			}

		//	Armor

		else if (strEquals(pSubDesc->GetTag(), ARMOR_TAG))
			{
			if (error = CArmorClass::CreateFromXML(Ctx, pSubDesc, this, &m_pArmor))
				return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unable to load %s: %s"), pSubDesc->GetTag(), Ctx.sError));
			}

		//	Devices

		else if (strEquals(pSubDesc->GetTag(), WEAPON_CLASS_TAG))
			{
			if (error = CWeaponClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unable to load %s: %s"), pSubDesc->GetTag(), Ctx.sError));
			}
		else if (strEquals(pSubDesc->GetTag(), SHIELD_CLASS_TAG))
			{
			if (error = CShieldClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unable to load %s: %s"), pSubDesc->GetTag(), Ctx.sError));
			}
		else if (strEquals(pSubDesc->GetTag(), DRIVE_CLASS_TAG))
			{
			if (error = CDriveClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unable to load %s: %s"), pSubDesc->GetTag(), Ctx.sError));
			}
		else if (strEquals(pSubDesc->GetTag(), CARGO_HOLD_CLASS_TAG))
			{
			if (error = CCargoSpaceClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unable to load %s: %s"), pSubDesc->GetTag(), Ctx.sError));
			}
		else if (strEquals(pSubDesc->GetTag(), ENHANCER_CLASS_TAG))
			{
			if (error = CEnhancerClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unable to load %s: %s"), pSubDesc->GetTag(), Ctx.sError));
			}
		else if (strEquals(pSubDesc->GetTag(), REPAIRER_CLASS_TAG))
			{
			if (error = CRepairerClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unable to load %s: %s"), pSubDesc->GetTag(), Ctx.sError));
			}
		else if (strEquals(pSubDesc->GetTag(), SOLAR_DEVICE_CLASS_TAG))
			{
			if (error = CSolarDeviceClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unable to load %s: %s"), pSubDesc->GetTag(), Ctx.sError));
			}
		else if (strEquals(pSubDesc->GetTag(), CYBER_DECK_CLASS_TAG))
			{
			if (error = CCyberDeckClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unable to load %s: %s"), pSubDesc->GetTag(), Ctx.sError));
			}
		else if (strEquals(pSubDesc->GetTag(), AUTO_DEFENSE_CLASS_TAG))
			{
			if (error = CAutoDefenseClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unable to load %s: %s"), pSubDesc->GetTag(), Ctx.sError));
			}
		else if (strEquals(pSubDesc->GetTag(), MISCELLANEOUS_CLASS_TAG))
			{
			if (error = CMiscellaneousClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unable to load %s: %s"), pSubDesc->GetTag(), Ctx.sError));
			}
		else if (strEquals(pSubDesc->GetTag(), REACTOR_CLASS_TAG))
			{
			if (error = CReactorClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unable to load %s: %s"), pSubDesc->GetTag(), Ctx.sError));
			}

		//	Missiles

		else if (strEquals(pSubDesc->GetTag(), MISSILE_TAG))
			{
			m_pMissile = new CWeaponFireDesc;
			if (error = m_pMissile->InitFromXML(Ctx, pSubDesc, strPatternSubst(CONSTLIT("%d"), GetUNID())))
				return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unable to load %s: %s"), pSubDesc->GetTag(), Ctx.sError));

			//	Make sure we point to us

			m_pMissile->m_pAmmoType = this;
			}

		//	Other elements

		else if (IsValidLoadXML(pSubDesc->GetTag()))
			;

		//	Otherwise, error

		else
			kernelDebugLogMessage("Unknown sub-element for ItemType: %s", pSubDesc->GetTag());
		}

	//	Done

	return NOERROR;
	}

CEffectCreator *CItemType::OnFindEffectCreator (const CString &sUNID)

//	OnFindEffectCreator
//
//	Find the effect creator. sUNID is the remainder after the item type UNID has been removed

	{
	if (m_pDevice)
		return m_pDevice->FindEffectCreator(sUNID);
	else if (m_pMissile)
		return m_pMissile->FindEffectCreator(sUNID);
	else
		return NULL;
	}

bool CItemType::OnHasSpecialAttribute (const CString &sAttrib) const

//	OnHasSpecialAttribute
//
//	Returns TRUE if we have the special attribute

	{
	if (strStartsWith(sAttrib, SPECIAL_DAMAGE_TYPE))
		{
		//	Get the device

		CDeviceClass *pDevice;
		int iVariant;
		if (IsMissile())
			pDevice = GetAmmoLauncher(&iVariant);
		else
			{
			pDevice = GetDeviceClass();
			iVariant = 0;
			}

		if (pDevice == NULL)
			return false;

		DamageTypes iType = LoadDamageTypeFromXML(strSubString(sAttrib, SPECIAL_DAMAGE_TYPE.GetLength(), -1));
		if (iType == damageError)
			return false;

		return (iType == pDevice->GetDamageType(NULL, iVariant));
		}
	else if (strStartsWith(sAttrib, SPECIAL_CAN_BE_DAMAGED))
		{
		bool bValue = strEquals(strSubString(sAttrib, SPECIAL_CAN_BE_DAMAGED.GetLength(), -1), SPECIAL_TRUE);

		CDeviceClass *pDevice;
		if (pDevice = GetDeviceClass())
			return (pDevice->CanBeDamaged() == bValue);
		else if (IsArmor())
			return (true == bValue);
		else
			return (false == bValue);
		}
	else if (strStartsWith(sAttrib, SPECIAL_IS_LAUNCHER))
		{
		bool bValue = strEquals(strSubString(sAttrib, SPECIAL_IS_LAUNCHER.GetLength(), -1), SPECIAL_TRUE);
		return ((GetCategory() == itemcatLauncher) == bValue);
		}
	else if (strStartsWith(sAttrib, SPECIAL_LAUNCHED_BY))
		{
		DWORD dwLauncher = strToInt(strSubString(sAttrib, SPECIAL_LAUNCHED_BY.GetLength(), -1), 0);
		if (dwLauncher == 0 || !IsMissile())
			return false;

		CDeviceClass *pDevice = GetAmmoLauncher();
		if (pDevice == NULL)
			return false;

		return (pDevice->GetUNID() == dwLauncher);
		}
	else if (strStartsWith(sAttrib, SPECIAL_PROPERTY))
		{
		CString sProperty = strSubString(sAttrib, SPECIAL_PROPERTY.GetLength());
		CItem Item((CItemType *)this, 1);
		CItemCtx ItemCtx(Item);
		CCodeChainCtx Ctx;

		ICCItem *pValue = Item.GetProperty(&Ctx, ItemCtx, sProperty);
		bool bResult = !pValue->IsNil();
		pValue->Discard(&g_pUniverse->GetCC());

		return bResult;
		}
	else
		return false;
	}

void CItemType::OnReadFromStream (SUniverseLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read from stream
//
//	DWORD		flags
//	DWORD		m_sUnknownName

	{
	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	
	m_fKnown =		((dwLoad & 0x00000001) ? true : false);
	m_fReference =	((dwLoad & 0x00000002) ? true : false);

	if (Ctx.dwVersion >= 10)
		m_sUnknownName.ReadFromStream(Ctx.pStream);
	else
		{
		int iUnknownItem;
		Ctx.pStream->Read((char *)&iUnknownItem, sizeof(DWORD));

		if (iUnknownItem != -1 && m_pUnknownType)
			m_sUnknownName = m_pUnknownType->GetUnknownName(iUnknownItem);
		}

	//	This can happen if we change the known/unknown status of an
	//	item type across save games

	if (m_pUnknownType == NULL)
		m_fKnown = true;
	}

void CItemType::OnReinit (void)

//	Reinit
//
//	Reinitialize when the game starts again

	{
	g_pFlotsamStationType = NULL;

	if (GetUnknownType())
		m_fKnown = false;

	m_fReference = m_fDefaultReference;

	InitRandomNames();
	}

void CItemType::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes out the variable portions of the item type
//
//	DWORD		flags
//	DWORD		m_sUnknownName

	{
	DWORD dwSave;

	dwSave = 0;
	dwSave |= (m_fKnown ?		0x00000001 : 0);
	dwSave |= (m_fReference ?	0x00000002 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	m_sUnknownName.WriteToStream(pStream);
	}

bool CItemType::ParseItemCategory (const CString &sCategory, ItemCategories *retCategory)

//	ParseItemCategory
//
//	Parses an item category

	{
	ItemCategories iCat;

	if (strEquals(sCategory, CATEGORY_AMMO))
		iCat = itemcatMissile;
	else if (strEquals(sCategory, CATEGORY_ARMOR))
		iCat = itemcatArmor;
	else if (strEquals(sCategory, CATEGORY_CARGO_HOLD))
		iCat = itemcatCargoHold;
	else if (strEquals(sCategory, CATEGORY_DRIVE))
		iCat = itemcatDrive;
	else if (strEquals(sCategory, CATEGORY_FUEL))
		iCat = itemcatFuel;
	else if (strEquals(sCategory, CATEGORY_LAUNCHER))
		iCat = itemcatLauncher;
	else if (strEquals(sCategory, CATEGORY_MISC_DEVICE))
		iCat = itemcatMiscDevice;
	else if (strEquals(sCategory, CATEGORY_MISC_ITEM))
		iCat = itemcatMisc;
	else if (strEquals(sCategory, CATEGORY_REACTOR))
		iCat = itemcatReactor;
	else if (strEquals(sCategory, CATEGORY_SHIELDS))
		iCat = itemcatShields;
	else if (strEquals(sCategory, CATEGORY_USABLE_ITEM))
		iCat = itemcatUseful;
	else if (strEquals(sCategory, CATEGORY_WEAPON))
		iCat = itemcatWeapon;
	else
		return false;

	if (retCategory)
		*retCategory = iCat;

	return true;
	}

