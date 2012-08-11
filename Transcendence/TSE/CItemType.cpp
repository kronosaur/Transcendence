//	CItemType.cpp
//
//	CItemType class

#include "PreComp.h"

#define NAMES_TAG								CONSTLIT("Names")
#define IMAGE_TAG								CONSTLIT("Image")
#define COCKPIT_USE_TAG							CONSTLIT("Invoke")
#define ARMOR_TAG								CONSTLIT("Armor")
#define WEAPON_CLASS_TAG						CONSTLIT("Weapon")
#define SHIELD_CLASS_TAG						CONSTLIT("Shields")
#define DRIVE_CLASS_TAG							CONSTLIT("DriveDevice")
#define CARGO_HOLD_CLASS_TAG					CONSTLIT("CargoHoldDevice")
#define REPAIRER_CLASS_TAG						CONSTLIT("RepairerDevice")
#define SOLAR_DEVICE_CLASS_TAG					CONSTLIT("SolarDevice")
#define ENHANCER_CLASS_TAG						CONSTLIT("EnhancerDevice")
#define CYBER_DECK_CLASS_TAG					CONSTLIT("CyberDeckDevice")
#define AUTO_DEFENSE_CLASS_TAG					CONSTLIT("AutoDefenseDevice")
#define MISCELLANEOUS_CLASS_TAG					CONSTLIT("MiscellaneousDevice")
#define EVENTS_TAG								CONSTLIT("Events")
#define REACTOR_CLASS_TAG						CONSTLIT("ReactorDevice")
#define DOCK_SCREENS_TAG						CONSTLIT("DockScreens")
#define STATIC_DATA_TAG							CONSTLIT("StaticData")
#define GLOBAL_DATA_TAG							CONSTLIT("GlobalData")

#define INSTANCE_DATA_ATTRIB					CONSTLIT("charges")
#define DATA_ATTRIB								CONSTLIT("data")
#define ENHANCEMENT_ATTRIB						CONSTLIT("enhancement")
#define ES_PLURAL_ATTRIB						CONSTLIT("esPlural")
#define FREQUENCY_ATTRIB						CONSTLIT("frequency")
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
#define USE_INSTALLED_ONLY_ATTRIB				CONSTLIT("useInstalledOnly")
#define USE_KEY_ATTRIB							CONSTLIT("useKey")
#define USE_SCREEN_ATTRIB						CONSTLIT("useScreen")
#define USE_UNINSTALLED_ONLY_ATTRIB				CONSTLIT("useUninstalledOnly")
#define VALUE_ATTRIB							CONSTLIT("value")
#define VALUE_BONUS_PER_CHARGE_ATTRIB			CONSTLIT("valueBonusPerCharge")
#define VALUE_CHARGES_ATTRIB					CONSTLIT("valueCharges")
#define VIRTUAL_ATTRIB							CONSTLIT("virtual")

#define GET_NAME_EVENT							CONSTLIT("GetName")
#define GET_TRADE_PRICE_EVENT					CONSTLIT("GetTradePrice")
#define ON_ENABLED_EVENT						CONSTLIT("OnEnabled")
#define ON_INSTALL_EVENT						CONSTLIT("OnInstall")
#define ON_REFUEL_TAG							CONSTLIT("OnRefuel")

#define STR_FUEL								CONSTLIT("Fuel")
#define STR_MISSILE								CONSTLIT("Missile")

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
#define FIELD_UNKNOWN_TYPE						CONSTLIT("unknownType")

static char g_NameAttrib[] = "name";
static char g_ObjectAttrib[] = "object";
static char g_MassAttrib[] = "mass";
static char g_DescriptionAttrib[] = "description";
static char g_FirstPluralAttrib[] = "firstPlural";
static char g_RandomDamagedAttrib[] = "randomDamaged";

static char *CACHED_EVENTS[CItemType::evtCount] =
	{
		"GetName",
		"GetTradePrice",
		"OnInstall",
		"OnEnabled",
		"OnRefuel",
	};

CItemType::CItemType (void) : 
		m_dwSpare(0),
		m_pUseCode(NULL),
		m_pArmor(NULL),
		m_pDevice(NULL)

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
	}

ALERROR CItemType::ComposeError (const CString &sName, const CString &sError, CString *retsError)

//	ComposeError
//
//	Composes an error string when loading

	{
	*retsError = strPatternSubst("%s: %s", sName, sError);
	return ERR_FAIL;
	}

bool CItemType::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	//	Deal with the meta-data that we know about

	if (strEquals(sField, FIELD_LEVEL))
		{
		*retsValue = strFromInt(GetLevel());
		return true;
		}
	else if (strEquals(sField, FIELD_CATEGORY))
		{
		switch (GetCategory())
			{
			case itemcatArmor:
				*retsValue = CONSTLIT("Armor");
				break;

			case itemcatCargoHold:
				*retsValue = CONSTLIT("CargoHold");
				break;

			case itemcatDrive:
				*retsValue = CONSTLIT("Device");
				break;

			case itemcatFuel:
				*retsValue = CONSTLIT("Fuel");
				break;

			case itemcatLauncher:
				*retsValue = CONSTLIT("Launcher");
				break;

			case itemcatMisc:
				*retsValue = CONSTLIT("Misc");
				break;

			case itemcatMissile:
				*retsValue = CONSTLIT("Missile");
				break;

			case itemcatMiscDevice:
				*retsValue = CONSTLIT("Device");
				break;

			case itemcatReactor:
				*retsValue = CONSTLIT("Reactor");
				break;

			case itemcatShields:
				*retsValue = CONSTLIT("Shield");
				break;

			case itemcatUseful:
				*retsValue = CONSTLIT("Useful");
				break;

			case itemcatWeapon:
				*retsValue = CONSTLIT("Weapon");
				break;

			default:
				*retsValue = CONSTLIT("Unknown");
				break;
			}

		return true;
		}
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

		return true;
		}
	else if (strEquals(sField, FIELD_FUEL_CAPACITY) && IsFuel())
		{
		int iFuelPerItem = strToInt(GetData(), 0);
		*retsValue = strFromInt(iFuelPerItem / FUEL_UNITS_PER_STD_ROD);
		return true;
		}
	else if (strEquals(sField, FIELD_NAME))
		{
		*retsValue = GetNounPhrase();
		return true;
		}
	else if (strEquals(sField, FIELD_MASS))
		{
		*retsValue = strFromInt(CItem(this, 1).GetMassKg());
		return true;
		}
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

		return true;
		}
	else if (strEquals(sField, FIELD_REFERENCE))
		{
		CItemCtx Ctx;
		*retsValue = GetReference(Ctx);
		return true;
		}
	else if (strEquals(sField, FIELD_COST))
		{
		CItemCtx Ctx;
		*retsValue = strFromInt(GetValue(Ctx));
		return true;
		}
	else if (strEquals(sField, FIELD_INSTALL_COST))
		{
		int iCost = GetInstallCost();
		if (iCost == -1)
			*retsValue = NULL_STR;
		else
			*retsValue = strFromInt(iCost);
		return true;
		}
	else if (strEquals(sField, FIELD_AVERAGE_COUNT))
		{
		*retsValue = strFromInt(m_NumberAppearing.GetAveValue());
		return true;
		}
	else if (strEquals(sField, FIELD_DESCRIPTION))
		{
		*retsValue = m_sDescription;
		return true;
		}
	else if (strEquals(sField, FIELD_IMAGE_DESC))
		{
		const RECT &rcImage = m_Image.GetImageRect();
		*retsValue = strPatternSubst(CONSTLIT("%s (%d,%d)"),
				m_Image.GetFilename(),
				rcImage.left,
				rcImage.top);
		return true;
		}
	else if (strEquals(sField, FIELD_DEVICE_SLOTS))
		{
		if (m_pDevice)
			*retsValue = strFromInt(m_pDevice->GetSlotsRequired());
		else
			*retsValue = NULL_STR;
		return true;
		}
	else if (strEquals(sField, FIELD_UNKNOWN_TYPE))
		{
		*retsValue = (m_pUnknownType ? strPatternSubst(CONSTLIT("0x%x"), m_pUnknownType->GetUNID()) : NULL_STR);
		return true;
		}

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

	return false;
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
		*retdwFlags = 0;
		if (m_fFirstPlural)
			*retdwFlags |= nounFirstPlural;
		if (m_fSecondPlural)
			*retdwFlags |= nounSecondPlural;
		if (m_fESPlural)
			*retdwFlags |= nounPluralES;
		if (m_fReverseArticle)
			*retdwFlags |= nounVowelArticle;

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
	return HasAttribute(STR_FUEL);
	}

bool CItemType::IsMissile (void) const

//	IsMissile
//
//	Returns TRUE if this is a missile

	{
	return HasAttribute(STR_MISSILE);
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

	m_sSortName = pDesc->GetAttribute(SORT_NAME_ATTRIB);
	if (m_sSortName.IsBlank())
		m_sSortName = m_sName;
	m_iLevel = pDesc->GetAttributeIntegerBounded(LEVEL_ATTRIB, 1, MAX_ITEM_LEVEL, 1);
	m_iMass = pDesc->GetAttributeInteger(CONSTLIT(g_MassAttrib));

	if (error = m_iValue.InitFromXML(Ctx, pDesc->GetAttribute(VALUE_ATTRIB)))
		return ComposeError(m_sName, Ctx.sError, &Ctx.sError);

	//	Initialize frequency

	m_Frequency = (FrequencyTypes)::GetFrequency(pDesc->GetAttribute(FREQUENCY_ATTRIB));
	CString sNumberAppearing = pDesc->GetAttribute(NUMBER_APPEARING_ATTRIB);
	if (sNumberAppearing.IsBlank())
		m_NumberAppearing = DiceRange(0, 0, 1);
	else
		{
		if (error = m_NumberAppearing.LoadFromXML(sNumberAppearing))
			return ComposeError(m_sName, CONSTLIT("Unable to parse numberAppearing"), &Ctx.sError);
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

	m_fFirstPlural = pDesc->GetAttributeBool(CONSTLIT(g_FirstPluralAttrib));
	m_fSecondPlural = pDesc->GetAttributeBool(SECOND_PLURAL_ATTRIB);
	m_fESPlural = pDesc->GetAttributeBool(ES_PLURAL_ATTRIB);
	m_fRandomDamaged = pDesc->GetAttributeBool(CONSTLIT(g_RandomDamagedAttrib));
	m_fReverseArticle = pDesc->GetAttributeBool(REVERSE_ARTICLE_ATTRIB);
	m_fVirtual = pDesc->GetAttributeBool(VIRTUAL_ATTRIB);
	if (m_fVirtual)
		m_Frequency = ftNotRandom;

	//	Use screen

	m_pUseScreen.LoadUNID(Ctx, pDesc->GetAttribute(USE_SCREEN_ATTRIB));
	m_sUseKey = pDesc->GetAttribute(USE_KEY_ATTRIB);
	m_fUseInstalled = pDesc->GetAttributeBool(USE_INSTALLED_ONLY_ATTRIB);
	m_fUseUninstalled = pDesc->GetAttributeBool(USE_UNINSTALLED_ONLY_ATTRIB);

	//	Process sub-elements

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pSubDesc = pDesc->GetContentElement(i);

		//	Process image

		if (strEquals(pSubDesc->GetTag(), IMAGE_TAG))
			{
			if (error = m_Image.InitFromXML(Ctx, pSubDesc))
				return ComposeError(m_sName, CONSTLIT("Unable to load image"), &Ctx.sError);
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
			}

		//	Process events (skip processing here since we processed above)

		else if (strEquals(pSubDesc->GetTag(), EVENTS_TAG))
			;

		else if (strEquals(pSubDesc->GetTag(), DOCK_SCREENS_TAG))
			;

		else if (strEquals(pSubDesc->GetTag(), GLOBAL_DATA_TAG))
			;

		else if (strEquals(pSubDesc->GetTag(), STATIC_DATA_TAG))
			;

		//	Process on refuel code

		else if (strEquals(pSubDesc->GetTag(), ON_REFUEL_TAG))
			{
			if (error = AddEventHandler(ON_REFUEL_TAG, pSubDesc->GetContentText(0), &Ctx.sError))
				return ComposeError(m_sName, CONSTLIT("Unable to load OnRefuel event"), &Ctx.sError);
			}

		//	Armor

		else if (strEquals(pSubDesc->GetTag(), ARMOR_TAG))
			{
			if (error = CArmorClass::CreateFromXML(Ctx, pSubDesc, this, &m_pArmor))
				return ComposeError(m_sName, CONSTLIT("Unable to load Armor descriptor"), &Ctx.sError);
			}

		//	Devices

		else if (strEquals(pSubDesc->GetTag(), WEAPON_CLASS_TAG))
			{
			if (error = CWeaponClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeError(m_sName, Ctx.sError, &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), SHIELD_CLASS_TAG))
			{
			if (error = CShieldClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeError(m_sName, CONSTLIT("Unable to load shield descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), DRIVE_CLASS_TAG))
			{
			if (error = CDriveClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeError(m_sName, CONSTLIT("Unable to load drive descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), CARGO_HOLD_CLASS_TAG))
			{
			if (error = CCargoSpaceClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeError(m_sName, CONSTLIT("Unable to load cargo hold descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), ENHANCER_CLASS_TAG))
			{
			if (error = CEnhancerClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeError(m_sName, CONSTLIT("Unable to load enhancer descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), REPAIRER_CLASS_TAG))
			{
			if (error = CRepairerClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeError(m_sName, CONSTLIT("Unable to load repairer descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), SOLAR_DEVICE_CLASS_TAG))
			{
			if (error = CSolarDeviceClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeError(m_sName, CONSTLIT("Unable to load solar device descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), CYBER_DECK_CLASS_TAG))
			{
			if (error = CCyberDeckClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeError(m_sName, CONSTLIT("Unable to load cyber device descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), AUTO_DEFENSE_CLASS_TAG))
			{
			if (error = CAutoDefenseClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeError(m_sName, CONSTLIT("Unable to load defense device descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), MISCELLANEOUS_CLASS_TAG))
			{
			if (error = CMiscellaneousClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeError(m_sName, CONSTLIT("Unable to load miscellaneous device descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), REACTOR_CLASS_TAG))
			{
			if (error = CReactorClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice))
				return ComposeError(m_sName, CONSTLIT("Unable to load reactor descriptor"), &Ctx.sError);
			}
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
	else
		return NULL;
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

//	CItemCtx -------------------------------------------------------------------

ICCItem *CItemCtx::CreateItemVariable (CCodeChain &CC)

//	CreateItemVariable
//
//	Creates a variable for the item (or Nil)

	{
	const CItem *pItem = GetItemPointer();
	if (pItem == NULL)
		return CC.CreateNil();

	return CreateListFromItem(CC, *pItem);
	}

CInstalledArmor *CItemCtx::GetArmor (void)

//	GetArmor
//
//	Returns the installed armor struct

	{
	//	If we've got it, done

	if (m_pArmor)
		return m_pArmor;

	//	Otherwise, try to get it from the item and source

	CShip *pShip;
	if (m_pSource && m_pItem && m_pItem->IsInstalled() && (pShip = m_pSource->AsShip()))
		{
		//	Cache it in case someone asks later
		m_pArmor = pShip->GetArmorSection(m_pItem->GetInstalled());
		return m_pArmor;
		}

	//	Couldn't get it

	return NULL;
	}

CInstalledDevice *CItemCtx::GetDevice (void)

//	GetDevice
//
//	Returns the installed device struct

	{
	//	If we've got it, done

	if (m_pDevice)
		return m_pDevice;

	//	Otherwise, try to get it from the item and source

	if (m_pSource && m_pItem && m_pItem->IsInstalled())
		{
		//	Cache it in case someone asks later
		m_pDevice = m_pSource->FindDevice(*m_pItem);
		return m_pDevice;
		}

	//	Couldn't get it

	return NULL;
	}

CDeviceClass *CItemCtx::GetDeviceClass (void)

//	GetDeviceClass
//
//	Returns the device class

	{
	//	Get it from the installed device

	if (m_pDevice)
		return m_pDevice->GetClass();

	//	Otherwise, get it from the item

	if (m_pItem)
		{
		CItemType *pType = m_pItem->GetType();
		if (pType)
			return pType->GetDeviceClass();
		}

	//	Couldn't get it

	return NULL;
	}

const CItem &CItemCtx::GetItem (void)

//	GetItem
//
//	Returns the item struct

	{
	//	If we've got an item, then return a reference to it
	//	Whoever set this is responsible to guaranteeing its lifespan

	if (m_pItem)
		return *m_pItem;

	//	Otherwise, we need to cons one up

	const CItem *pItem = GetItemPointer();
	if (pItem == NULL)
		return CItem::GetNullItem();

	//	We cache it for later

	m_Item = *pItem;
	m_pItem = &m_Item;

	//	Done

	return *m_pItem;
	}

const CItem *CItemCtx::GetItemPointer (void)

//	GetItemPointer
//
//	Returns a pointer to an item or NULL.
//	Note: We can't guarantee that the pointer will be valid across calls
//	to script (because an ItemTable might be moved around) so this
//	pointer should never be returned to callers.

	{
	//	If we've got it, done

	if (m_pItem)
		return m_pItem;

	//	If we have a source and device, find the item

	if (m_pDevice && m_pSource)
		return m_pDevice->GetItem();

	//	If we have a source and armor, find the item

	if (m_pArmor && m_pSource)
		{
		CItemListManipulator ItemList(m_pSource->GetItemList());
		m_pSource->SetCursorAtArmor(ItemList, m_pArmor);
		if (!ItemList.IsCursorValid())
			return NULL;

		return &ItemList.GetItemAtCursor();
		}

	//	Couldn't get it

	return NULL;
	}

const CItemEnhancement &CItemCtx::GetMods (void)

//	GetMods
//
//	Returns mods for the item

	{
	//	If we've got an item, get mods from there

	if (m_pItem)
		return m_pItem->GetMods();

	//	If we've got a device structure, then get mods from that

	if (m_pDevice)
		return m_pDevice->GetMods();

	//	If we've got an armor structure, then that also has mods

	if (m_pArmor)
		return m_pArmor->GetMods();

	//	Else, we have to get an item

	return GetItem().GetMods();
	}

