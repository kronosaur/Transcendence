//	CStationType.cpp
//
//	CStationType class

#include "PreComp.h"

#define ANIMATIONS_TAG							CONSTLIT("Animations")
#define COMPOSITE_TAG							CONSTLIT("ImageComposite")
#define CONSTRUCTION_TAG						CONSTLIT("Construction")
#define DEVICES_TAG								CONSTLIT("Devices")
#define DOCKING_PORTS_TAG						CONSTLIT("DockingPorts")
#define DOCK_SCREENS_TAG						CONSTLIT("DockScreens")
#define ENCOUNTERS_TAG							CONSTLIT("Encounters")
#define EVENTS_TAG								CONSTLIT("Events")
#define IMAGE_TAG								CONSTLIT("Image")
#define IMAGE_VARIANTS_TAG						CONSTLIT("ImageVariants")
#define ITEMS_TAG								CONSTLIT("Items")
#define NAMES_TAG								CONSTLIT("Names")
#define REINFORCEMENTS_TAG						CONSTLIT("Reinforcements")
#define SATELLITES_TAG							CONSTLIT("Satellites")
#define SHIPS_TAG								CONSTLIT("Ships")
#define STATION_TAG								CONSTLIT("Station")
#define TRADE_TAG								CONSTLIT("Trade")


#define ABANDONED_SCREEN_ATTRIB					CONSTLIT("abandonedScreen")
#define ALERT_WHEN_ATTACKED_ATTRIB				CONSTLIT("alertWhenAttacked")
#define ALERT_WHEN_DESTROYED_ATTRIB				CONSTLIT("alertWhenDestroyed")
#define ALLOW_ENEMY_DOCKING_ATTRIB				CONSTLIT("allowEnemyDocking")
#define ARMOR_ID_ATTRIB							CONSTLIT("armorID")
#define BACKGROUND_PLANE_ATTRIB					CONSTLIT("backgroundPlane")
#define BARRIER_EFFECT_ATTRIB					CONSTLIT("barrierEffect")
#define BEACON_ATTRIB							CONSTLIT("beacon")
#define CAN_ATTACK_ATTRIB						CONSTLIT("canAttack")
#define CONSTRUCTION_RATE_ATTRIB				CONSTLIT("constructionRate")
#define CONTROLLING_SOVEREIGN_ATTRIB			CONSTLIT("controllingSovereign")
#define DEFAULT_BACKGROUND_ID_ATTRIB			CONSTLIT("defaultBackgroundID")
#define DEST_ENTRY_POINT_ATTRIB					CONSTLIT("destEntryPoint")	
#define DEST_NODE_ATTRIB						CONSTLIT("destNodeID")
#define DESTROY_WHEN_EMPTY_ATTRIB				CONSTLIT("destroyWhenEmpty")
#define DOCK_SCREEN_ATTRIB						CONSTLIT("dockScreen")
#define DOCKING_PORTS_ATTRIB					CONSTLIT("dockingPorts")
#define ENEMY_EXCLUSION_RADIUS_ATTRIB			CONSTLIT("enemyExclusionRadius")
#define EJECTA_ADJ_ATTRIB						CONSTLIT("ejactaAdj")
#define EJECTA_TYPE_ATTRIB						CONSTLIT("ejectaType")
#define EXPLOSION_TYPE_ATTRIB					CONSTLIT("explosionType")
#define FIRE_RATE_ADJ_ATTRIB					CONSTLIT("fireRateAdj")
#define FREQUENCY_ATTRIB						CONSTLIT("frequency")
#define GATE_EFFECT_ATTRIB						CONSTLIT("gateEffect")
#define HIT_POINTS_ATTRIB						CONSTLIT("hitPoints")
#define IMMUTABLE_ATTRIB						CONSTLIT("immutable")
#define INACTIVE_ATTRIB							CONSTLIT("inactive")
#define LEVEL_ATTRIB							CONSTLIT("level")
#define LEVEL_FREQUENCY_ATTRIB					CONSTLIT("levelFrequency")
#define LOCATION_CRITERIA_ATTRIB				CONSTLIT("locationCriteria")
#define MASS_ATTRIB								CONSTLIT("mass")
#define MAX_CONSTRUCTION_ATTRIB					CONSTLIT("maxConstruction")
#define MAX_HIT_POINTS_ATTRIB					CONSTLIT("maxHitPoints")
#define MAX_STRUCTURAL_HIT_POINTS_ATTRIB		CONSTLIT("maxStructuralHitPoints")
#define MIN_SHIPS_ATTRIB						CONSTLIT("minShips")
#define MOBILE_ATTRIB							CONSTLIT("mobile")
#define MULTI_HULL_ATTRIB						CONSTLIT("multiHull")
#define NAME_ATTRIB								CONSTLIT("name")
#define NO_BLACKLIST_ATTRIB						CONSTLIT("noBlacklist")
#define NO_FRIENDLY_FIRE_ATTRIB					CONSTLIT("noFriendlyFire")
#define NO_FRIENDLY_TARGET_ATTRIB				CONSTLIT("noFriendlyTarget")
#define NO_MAP_ICON_ATTRIB						CONSTLIT("noMapIcon")
#define RADIOACTIVE_ATTRIB						CONSTLIT("radioactive")
#define RANDOM_ENCOUNTERS_ATTRIB				CONSTLIT("randomEncounters")
#define REPAIR_RATE_ATTRIB						CONSTLIT("repairRate")
#define REVERSE_ARTICLE_ATTRIB					CONSTLIT("reverseArticle")
#define SCALE_ATTRIB							CONSTLIT("scale")
#define SHIP_ENCOUNTER_ATTRIB					CONSTLIT("shipEncounter")
#define SHIP_REPAIR_RATE_ATTRIB					CONSTLIT("shipRepairRate")
#define SHIPWRECK_UNID_ATTRIB					CONSTLIT("shipwreckID")
#define SIGN_ATTRIB								CONSTLIT("sign")
#define SOVEREIGN_ATTRIB						CONSTLIT("sovereign")
#define SPACE_COLOR_ATTRIB						CONSTLIT("spaceColor")
#define STEALTH_ATTRIB							CONSTLIT("stealth")
#define STRUCTURAL_HIT_POINTS_ATTRIB			CONSTLIT("structuralHitPoints")
#define TIME_STOP_IMMUNE_ATTRIB					CONSTLIT("timeStopImmune")
#define TYPE_ATTRIB								CONSTLIT("type")
#define UNID_ATTRIB								CONSTLIT("UNID")
#define UNIQUE_ATTRIB							CONSTLIT("unique")
#define VIRTUAL_ATTRIB							CONSTLIT("virtual")
#define WALL_ATTRIB								CONSTLIT("barrier")
#define X_ATTRIB								CONSTLIT("x")
#define Y_ATTRIB								CONSTLIT("y")

#define LARGE_DAMAGE_IMAGE_ID_ATTRIB			CONSTLIT("largeDamageImageID")
#define LARGE_DAMAGE_WIDTH_ATTRIB				CONSTLIT("largeDamageWidth")
#define LARGE_DAMAGE_HEIGHT_ATTRIB				CONSTLIT("largeDamageHeight")
#define LARGE_DAMAGE_COUNT_ATTRIB				CONSTLIT("largeDamageCount")
#define MEDIUM_DAMAGE_IMAGE_ID_ATTRIB			CONSTLIT("mediumDamageImageID")
#define MEDIUM_DAMAGE_WIDTH_ATTRIB				CONSTLIT("mediumDamageWidth")
#define MEDIUM_DAMAGE_HEIGHT_ATTRIB				CONSTLIT("mediumDamageHeight")
#define MEDIUM_DAMAGE_COUNT_ATTRIB				CONSTLIT("mediumDamageCount")

#define STAR_SCALE								CONSTLIT("star")
#define WORLD_SCALE								CONSTLIT("world")
#define STRUCTURE_SCALE							CONSTLIT("structure")
#define SHIP_SCALE								CONSTLIT("ship")
#define FLOTSAM_SCALE							CONSTLIT("flotsam")

#define UNIQUE_IN_SYSTEM						CONSTLIT("inSystem")
#define UNIQUE_IN_UNIVERSE						CONSTLIT("inUniverse")

#define FIELD_ABANDONED_DOCK_SCREEN				CONSTLIT("abandonedDockScreen")
#define FIELD_ARMOR_CLASS						CONSTLIT("armorClass")
#define FIELD_CAN_ATTACK						CONSTLIT("canAttack")
#define FIELD_CATEGORY							CONSTLIT("category")
#define FIELD_DOCK_SCREEN						CONSTLIT("dockScreen")
#define FIELD_EXPLOSION_TYPE					CONSTLIT("explosionType")
#define FIELD_FIRE_RATE_ADJ						CONSTLIT("fireRateAdj")
#define FIELD_HP								CONSTLIT("hp")
#define FIELD_LEVEL								CONSTLIT("level")
#define FIELD_NAME								CONSTLIT("name")

#define VALUE_FALSE								CONSTLIT("false")
#define VALUE_TRUE								CONSTLIT("true")

#define MAX_ATTACK_DISTANCE						(g_KlicksPerPixel * 512)

CStationType::CStationType (void) : 
		m_pDesc(NULL),
		m_pInitialShips(NULL),
		m_pReinforcements(NULL),
		m_pEncounters(NULL),
		m_iEncounterFrequency(ftNotRandom),
		m_pSatellitesDesc(NULL),
		m_iMinShips(0),
		m_pConstruction(NULL),
		m_iShipConstructionRate(0),
		m_iMaxConstruction(0),
		m_Devices(NULL),
		m_iAnimationsCount(0),
		m_pAnimations(NULL),
		m_pItems(NULL),
		m_pTrade(NULL)

//	CStationType constructor

	{
	}

CStationType::~CStationType (void)

//	CStationType destructor

	{
	if (m_Devices)
		delete [] m_Devices;

	if (m_pDesc)
		delete m_pDesc;

	if (m_pItems)
		delete m_pItems;

	if (m_pInitialShips)
		delete m_pInitialShips;

	if (m_pReinforcements)
		delete m_pReinforcements;

	if (m_pEncounters)
		delete m_pEncounters;

	if (m_pSatellitesDesc)
		delete m_pSatellitesDesc;

	if (m_pAnimations)
		delete [] m_pAnimations;

	if (m_pTrade)
		delete m_pTrade;
	}

void CStationType::AddTypesUsedByXML (CXMLElement *pElement, TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsedByXML
//
//	Add type used by the <Station> XML element (recursively)

	{
	int i;

	if (strEquals(pElement->GetTag(), STATION_TAG))
		retTypesUsed->SetAt(pElement->GetAttributeInteger(TYPE_ATTRIB), true);

	for (i = 0; i < pElement->GetContentElementCount(); i++)
		AddTypesUsedByXML(pElement->GetContentElement(i), retTypesUsed);
	}

bool CStationType::CanBeEncountered (CSystem *pSystem)

//	CanBeEncountered
//
//	Returns TRUE if the station type can be encountered in the given system.

	{
	if (pSystem)
		{
		if (IsUniqueInSystem() && pSystem->IsStationInSystem(this))
			return false;
		}

	return (!IsUnique() || !m_fEncountered);
	}

CString CStationType::ComposeLoadError (const CString &sError)

//	ComposeLoadError
//
//	Compose an error loading XML

	{
	return strPatternSubst(CONSTLIT("%s: %s"),
			m_sName,
			sError);
	}

bool CStationType::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	if (strEquals(sField, FIELD_ABANDONED_DOCK_SCREEN))
		*retsValue = m_pAbandonedDockScreen.GetStringUNID(this);
	else if (strEquals(sField, FIELD_CATEGORY))
		{
		if (m_sLevelFrequency.IsBlank())
			*retsValue = CONSTLIT("04-Not Random");
		else if (HasAttribute(CONSTLIT("debris")))
			*retsValue = CONSTLIT("03-Debris");
		else if (HasAttribute(CONSTLIT("enemy")))
			*retsValue = CONSTLIT("02-Enemy");
		else if (HasAttribute(CONSTLIT("friendly")))
			*retsValue = CONSTLIT("01-Friendly");
		else
			*retsValue = CONSTLIT("04-Not Random");
		}
	else if (strEquals(sField, FIELD_DOCK_SCREEN))
		*retsValue = m_pFirstDockScreen.GetStringUNID(this);
	else if (strEquals(sField, FIELD_LEVEL))
		*retsValue = strFromInt(GetLevel());
	else if (strEquals(sField, FIELD_NAME))
		*retsValue = GetName();
	else if (strEquals(sField, FIELD_ARMOR_CLASS))
		{
		if (m_pArmor)
			*retsValue = m_pArmor->GetArmorClass()->GetShortName();
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_HP))
		*retsValue = strFromInt(m_iHitPoints);
	else if (strEquals(sField, FIELD_FIRE_RATE_ADJ))
		*retsValue = strFromInt(10000 / m_iFireRateAdj);
	else if (strEquals(sField, FIELD_CAN_ATTACK))
		*retsValue = (CanAttack() ? VALUE_TRUE : VALUE_FALSE);
	else if (strEquals(sField, FIELD_EXPLOSION_TYPE))
		{
		if (m_pExplosionType)
			{
			DWORD dwUNID = (DWORD)strToInt(m_pExplosionType->m_sUNID, 0);
			CWeaponClass *pClass = (CWeaponClass *)g_pUniverse->FindDeviceClass(dwUNID);
			if (pClass)
				{
				*retsValue = pClass->GetItemType()->GetNounPhrase();
				return true;
				}
			}

		*retsValue = CONSTLIT("none");
		}
	else
		return CDesignType::FindDataField(sField, retsValue);

	return true;
	}

CString CStationType::GenerateRandomName (const CString &sSubst, DWORD *retdwFlags)

//	GenerateRandomName
//
//	Generates a random name

	{
	if (retdwFlags)
		*retdwFlags = m_dwRandomNameFlags;

	return ::GenerateRandomName(m_sRandomNames, sSubst);
	}

CSovereign *CStationType::GetControllingSovereign (void)

//	GetControllingSovereign
//
//	Returns the sovereign that controls the station

	{
	if (m_pControllingSovereign)
		return m_pControllingSovereign;
	else
		return m_pSovereign;
	}

int CStationType::GetFrequencyByLevel (int iLevel)

//	GetFrequencyByLevel
//
//	Returns the chance of this station type appearing at this level

	{
	if (IsUnique() && m_fEncountered)
		return 0;
	else
		return ::GetFrequencyByLevel(m_sLevelFrequency, iLevel);
	}

int CStationType::GetFrequencyForSystem (CSystem *pSystem)

//	GetFrequencyForSystem
//
//	Returns the chance that this station type will appear in the given system

	{
	//	If we're unique in the universe and already encountered, then no chance
	//	of appearing again.

	if (IsUnique() && m_fEncountered)
		return 0;

	//	If this station is unique in the system, see if there are other
	//	stations of this type in the system

	if (IsUniqueInSystem() && pSystem->IsStationInSystem(this))
		return 0;

	//	Otherwise, go by level

	return ::GetFrequencyByLevel(m_sLevelFrequency, pSystem->GetLevel());
	}

int CStationType::GetLevel (void) const

//	GetLevel
//
//	Returns the average level of the station.

	{
	if (m_iLevel)
		return m_iLevel;
	else
		{
		int iLevel = 1;
		int iTotal = 0;
		int iCount = 0;
		char *pPos = m_sLevelFrequency.GetASCIIZPointer();
		while (*pPos != '\0')
			{
			int iFreq = 0;

			switch (*pPos)
				{
				case 'C':
				case 'c':
					iFreq = ftCommon;
					break;

				case 'U':
				case 'u':
					iFreq = ftUncommon;
					break;

				case 'R':
				case 'r':
					iFreq = ftRare;
					break;

				case 'V':
				case 'v':
					iFreq = ftVeryRare;
					break;
				}

			iTotal += iFreq * iLevel;
			iCount += iFreq;

			pPos++;
			if (*pPos != ' ')
				iLevel++;
			}

		if (iCount > 0)
			return (int)(((double)iTotal / (double)iCount) + 0.5);
		else
			return 0;
		}
	}

const CString &CStationType::GetName (DWORD *retdwFlags)

//	GetName
//
//	Returns the name of the type

	{
	if (retdwFlags)
		*retdwFlags = m_dwNameFlags;

	return m_sName;
	}

CString CStationType::GetNounPhrase (DWORD dwFlags)

//	GetNounPhrase
//
//	Gets the station name

	{
	DWORD dwNameFlags;
	CString sName = GetName(&dwNameFlags);
	return ::ComposeNounPhrase(sName, 1, NULL_STR, dwNameFlags, dwFlags);
	}

void CStationType::MarkImages (const CCompositeImageSelector &Selector)

//	MarkImages
//
//	Marks images used by the station

	{
	m_Image.MarkImage(Selector);

	//	Cache the destroyed station image, if necessary

	if (HasWreckImage())
		m_Image.MarkImage(Selector, CCompositeImageDesc::modStationDamage);

	//	Explosions and other effects

	if (m_pExplosionType)
		m_pExplosionType->MarkImages();

	if (m_pEjectaType)
		m_pEjectaType->MarkImages();

	if (m_pGateEffect)
		m_pGateEffect->MarkImages();
	}

void CStationType::OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	OnAddTypesUsed
//
//	Add design types used by this station type

	{
	int i;

	retTypesUsed->SetAt(m_pSovereign.GetUNID(), true);
	retTypesUsed->SetAt(m_pArmor.GetUNID(), true);

	for (i = 0; i < m_iDevicesCount; i++)
		{
		CItem *pItem = m_Devices[i].GetItem();
		if (pItem)
			retTypesUsed->SetAt(pItem->GetType()->GetUNID(), true);
		}

	if (m_pItems)
		m_pItems->AddTypesUsed(retTypesUsed);

	m_Image.AddTypesUsed(retTypesUsed);

	for (i = 0; i < m_ShipWrecks.GetCount(); i++)
		retTypesUsed->SetAt(m_ShipWrecks.GetElement(i), true);

	for (i = 0; i < m_iAnimationsCount; i++)
		retTypesUsed->SetAt(m_pAnimations[i].m_Image.GetBitmapUNID(), true);

	retTypesUsed->SetAt(strToInt(m_pFirstDockScreen.GetUNID(), 0), true);
	retTypesUsed->SetAt(strToInt(m_pAbandonedDockScreen.GetUNID(), 0), true);
	retTypesUsed->SetAt(m_dwDefaultBkgnd, true);

	if (m_pSatellitesDesc)
		AddTypesUsedByXML(m_pSatellitesDesc, retTypesUsed);

	if (m_pInitialShips)
		m_pInitialShips->AddTypesUsed(retTypesUsed);

	if (m_pReinforcements)
		m_pReinforcements->AddTypesUsed(retTypesUsed);

	if (m_pEncounters)
		m_pEncounters->AddTypesUsed(retTypesUsed);

	if (m_pConstruction)
		m_pConstruction->AddTypesUsed(retTypesUsed);

	retTypesUsed->SetAt(m_pExplosionType.GetUNID(), true);
	retTypesUsed->SetAt(m_pEjectaType.GetUNID(), true);
	retTypesUsed->SetAt(m_pBarrierEffect.GetUNID(), true);
	retTypesUsed->SetAt(m_pControllingSovereign.GetUNID(), true);
	retTypesUsed->SetAt(m_pGateEffect.GetUNID(), true);
	}

ALERROR CStationType::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	int i;
	ALERROR error;

	//	Images

	if (error = m_Image.OnDesignLoadComplete(Ctx))
		goto Fail;

	for (i = 0; i < m_iAnimationsCount; i++)
		if (error = m_pAnimations[i].m_Image.OnDesignLoadComplete(Ctx))
			goto Fail;

	//	Sovereigns

	if (error = m_pSovereign.Bind(Ctx))
		goto Fail;

	if (error = m_pControllingSovereign.Bind(Ctx))
		goto Fail;

	//	Armor

	if (error = m_pArmor.Bind(Ctx, itemcatArmor))
		goto Fail;

	//	Resolve screen

	if (error = m_pAbandonedDockScreen.Bind(Ctx, GetLocalScreens()))
		goto Fail;

	if (error = m_pFirstDockScreen.Bind(Ctx, GetLocalScreens()))
		goto Fail;

	//	Resolve the devices pointer

	Metric rBestRange = MAX_ATTACK_DISTANCE;
	for (i = 0; i < m_iDevicesCount; i++)
		{
		if (error = m_Devices[i].OnDesignLoadComplete(Ctx))
			goto Fail;

		if (m_Devices[i].GetCategory() == itemcatWeapon
				|| m_Devices[i].GetCategory() == itemcatLauncher)
			{
			Metric rRange = m_Devices[i].GetMaxEffectiveRange(NULL);
			if (rRange > rBestRange)
				rBestRange = rRange;
			}
		}
	m_rMaxAttackDistance = rBestRange;

	//	Items

	if (m_pItems)
		if (error = m_pItems->OnDesignLoadComplete(Ctx))
			goto Fail;

	if (m_pTrade)
		if (error = m_pTrade->OnDesignLoadComplete(Ctx))
			goto Fail;

	//	Ships

	if (m_pInitialShips)
		if (error = m_pInitialShips->OnDesignLoadComplete(Ctx))
			goto Fail;

	if (m_pReinforcements)
		if (error = m_pReinforcements->OnDesignLoadComplete(Ctx))
			goto Fail;

	if (m_pConstruction)
		if (error = m_pConstruction->OnDesignLoadComplete(Ctx))
			goto Fail;

	if (m_pEncounters)
		if (error = m_pEncounters->OnDesignLoadComplete(Ctx))
			goto Fail;

	//	Resolve the explosion pointer

	if (error = m_pExplosionType.Bind(Ctx))
		goto Fail;

	//	Resolve ejecta pointer

	if (error = m_pEjectaType.Bind(Ctx))
		goto Fail;

	//	Resolve effects

	if (error = m_pBarrierEffect.Bind(Ctx))
		goto Fail;

	if (error = m_pGateEffect.Bind(Ctx))
		goto Fail;

	//	Virtual objects always get some settings by default

	if (IsVirtual())
		{
		m_fImmutable = true;
		m_fNoMapIcon = true;
		}

	//	Figure out if this is static

	m_fStatic = (m_iMaxHitPoints == 0)
			&& (m_iStructuralHP == 0)
			&& (m_iDevicesCount == 0)
			&& (GetAbandonedScreen() == NULL)
			&& (GetFirstDockScreen() == NULL)
			&& (m_pInitialShips == NULL)
			&& (m_pReinforcements == NULL)
			&& (m_pEncounters == NULL)
			&& (m_pConstruction == NULL)
			&& (m_pItems == NULL)
			&& (!HasEvents())
			&& (m_pBarrierEffect == NULL)
			&& (!m_fMobile)
			&& (!m_fWall)
			&& (!m_fSign)
			&& (!m_fBeacon)
			&& (!m_fShipEncounter);

	//	Any object that's static and the size of a planet or larger is immutable
	//	by default.

	if (m_fStatic && (m_iScale == scaleStar || m_iScale == scaleWorld))
		m_fImmutable = true;

	return NOERROR;

Fail:

	Ctx.sError = ComposeLoadError(Ctx.sError);
	return ERR_FAIL;
	}

ALERROR CStationType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	int i;
	ALERROR error;

	//	Initialize basic info

	m_iLevel = pDesc->GetAttributeInteger(LEVEL_ATTRIB);
	if (error = m_pSovereign.LoadUNID(Ctx, pDesc->GetAttribute(SOVEREIGN_ATTRIB)))
		return error;

	if (error = m_pControllingSovereign.LoadUNID(Ctx, pDesc->GetAttribute(CONTROLLING_SOVEREIGN_ATTRIB)))
		return error;

	m_fVirtual = pDesc->GetAttributeBool(VIRTUAL_ATTRIB);
	m_iRepairRate = pDesc->GetAttributeInteger(REPAIR_RATE_ATTRIB);
	m_fMobile = pDesc->GetAttributeBool(MOBILE_ATTRIB);
	m_fWall = pDesc->GetAttributeBool(WALL_ATTRIB);
	m_fNoFriendlyFire = pDesc->GetAttributeBool(NO_FRIENDLY_FIRE_ATTRIB);
	m_fNoFriendlyTarget = pDesc->GetAttributeBool(NO_FRIENDLY_TARGET_ATTRIB);
	m_fInactive = pDesc->GetAttributeBool(INACTIVE_ATTRIB);
	m_fDestroyWhenEmpty = pDesc->GetAttributeBool(DESTROY_WHEN_EMPTY_ATTRIB);
	m_fAllowEnemyDocking = pDesc->GetAttributeBool(ALLOW_ENEMY_DOCKING_ATTRIB);
	m_fSign = pDesc->GetAttributeBool(SIGN_ATTRIB);
	m_fBeacon = pDesc->GetAttributeBool(BEACON_ATTRIB);
	m_fRadioactive = pDesc->GetAttributeBool(RADIOACTIVE_ATTRIB);
	m_fEncountered = false;
	m_fNoMapIcon = pDesc->GetAttributeBool(NO_MAP_ICON_ATTRIB);
	m_fMultiHull = pDesc->GetAttributeBool(MULTI_HULL_ATTRIB);
	m_fTimeStopImmune = pDesc->GetAttributeBool(TIME_STOP_IMMUNE_ATTRIB);
	m_fCanAttack = pDesc->GetAttributeBool(CAN_ATTACK_ATTRIB);
	m_fReverseArticle = pDesc->GetAttributeBool(REVERSE_ARTICLE_ATTRIB);
	m_fShipEncounter = pDesc->GetAttributeBool(SHIP_ENCOUNTER_ATTRIB);
	m_fImmutable = pDesc->GetAttributeBool(IMMUTABLE_ATTRIB);
	m_fNoBlacklist = pDesc->GetAttributeBool(NO_BLACKLIST_ATTRIB);
	m_iShipRepairRate = pDesc->GetAttributeInteger(SHIP_REPAIR_RATE_ATTRIB);
	m_rgbSpaceColor = LoadCOLORREF(pDesc->GetAttribute(SPACE_COLOR_ATTRIB));
	m_iAlertWhenAttacked = pDesc->GetAttributeInteger(ALERT_WHEN_ATTACKED_ATTRIB);
	m_iAlertWhenDestroyed = pDesc->GetAttributeInteger(ALERT_WHEN_DESTROYED_ATTRIB);
	m_iFireRateAdj = strToInt(pDesc->GetAttribute(FIRE_RATE_ADJ_ATTRIB), 80);
	m_rMaxAttackDistance = MAX_ATTACK_DISTANCE;
	m_iStealth = pDesc->GetAttributeIntegerBounded(STEALTH_ATTRIB, CSpaceObject::stealthMin, CSpaceObject::stealthMax, CSpaceObject::stealthNormal);

	//	Load names

	m_sName = pDesc->GetAttribute(NAME_ATTRIB);
	m_dwNameFlags = LoadNameFlags(pDesc);

	CXMLElement *pNames = pDesc->GetContentElementByTag(NAMES_TAG);
	if (pNames)
		{
		m_sRandomNames = pNames->GetContentText(0);
		m_dwRandomNameFlags = LoadNameFlags(pNames);
		}
	else
		{
		m_sRandomNames = NULL_STR;
		m_dwRandomNameFlags = 0;
		}

	//	Get unique attributes

	CString sUnique = pDesc->GetAttribute(UNIQUE_ATTRIB);
	m_fUnique = false;
	m_fUniqueInSystem = false;
	if (strEquals(sUnique, UNIQUE_IN_SYSTEM))
		m_fUniqueInSystem = true;
	else if (strEquals(sUnique, UNIQUE_IN_UNIVERSE))
		m_fUnique = true;
	else if (strEquals(sUnique, VALUE_TRUE))
		m_fUnique = true;

	//	Placement

	m_sLevelFrequency = pDesc->GetAttribute(LEVEL_FREQUENCY_ATTRIB);
	m_sLocationCriteria = pDesc->GetAttribute(LOCATION_CRITERIA_ATTRIB);
	int iRadius;
	if (pDesc->FindAttributeInteger(ENEMY_EXCLUSION_RADIUS_ATTRIB, &iRadius)
			&& iRadius >= 0)
		m_rEnemyExclusionRadius = iRadius * LIGHT_SECOND;
	else
		m_rEnemyExclusionRadius = 30 * LIGHT_SECOND;

	//	Background objects

	m_rParallaxDist = pDesc->GetAttributeIntegerBounded(BACKGROUND_PLANE_ATTRIB, 0, -1, 0) / 100.0;
	m_fBackground = (m_rParallaxDist > 0.0);

	//	Get hit points and max hit points

	if (error = m_pArmor.LoadUNID(Ctx, pDesc->GetAttribute(ARMOR_ID_ATTRIB)))
		return error;

	m_iHitPoints = pDesc->GetAttributeIntegerBounded(HIT_POINTS_ATTRIB, 0, -1, -1);
	m_iMaxHitPoints = pDesc->GetAttributeIntegerBounded(MAX_HIT_POINTS_ATTRIB, 0, -1, -1);

	if (m_iHitPoints == -1 && m_iMaxHitPoints == -1)
		{
		m_iHitPoints = 0;
		m_iMaxHitPoints = 0;
		}
	else if (m_iHitPoints == -1)
		m_iHitPoints = m_iMaxHitPoints;
	else if (m_iMaxHitPoints == -1)
		m_iMaxHitPoints = m_iHitPoints;

	//	Structural hit points

	if (m_fImmutable)
		{
		m_iStructuralHP = 0;
		m_iMaxStructuralHP = 0;
		}
	else
		{
		m_iStructuralHP = pDesc->GetAttributeIntegerBounded(STRUCTURAL_HIT_POINTS_ATTRIB, 0, -1, -1);
		m_iMaxStructuralHP = pDesc->GetAttributeIntegerBounded(MAX_STRUCTURAL_HIT_POINTS_ATTRIB, 0, -1, -1);

		if (m_iStructuralHP == -1 && m_iMaxStructuralHP == -1)
			{
			m_iStructuralHP = 0;
			m_iMaxStructuralHP = 0;
			}
		else if (m_iStructuralHP == -1)
			m_iStructuralHP = m_iMaxStructuralHP;
		else if (m_iMaxStructuralHP == -1)
			m_iMaxStructuralHP = m_iStructuralHP;
		}

	//	Get the scale

	CString sScale = pDesc->GetAttribute(SCALE_ATTRIB);
	if (strEquals(sScale, STAR_SCALE))
		m_iScale = scaleStar;
	else if (strEquals(sScale, WORLD_SCALE))
		m_iScale = scaleWorld;
	else if (strEquals(sScale, SHIP_SCALE))
		m_iScale = scaleShip;
	else if (strEquals(sScale, FLOTSAM_SCALE))
		m_iScale = scaleFlotsam;
	else
		m_iScale = scaleStructure;

	//	Mass
	
	int iMass = pDesc->GetAttributeInteger(MASS_ATTRIB);
	if (iMass != 0)
		m_rMass = iMass;
	else
		m_rMass = 1000000;

	//	Load devices

	CXMLElement *pDevices = pDesc->GetContentElementByTag(DEVICES_TAG);
	if (pDevices)
		{
		m_iDevicesCount = pDevices->GetContentElementCount();
		if (m_iDevicesCount)
			{
			m_Devices = new CInstalledDevice [m_iDevicesCount];

			for (i = 0; i < m_iDevicesCount; i++)
				{
				CXMLElement *pDeviceDesc = pDevices->GetContentElement(i);
				m_Devices[i].InitFromXML(Ctx, pDeviceDesc);
				}
			}
		}
	else
		m_iDevicesCount = 0;

	//	Make a copy of the descriptor (We should stop doing this when we
	//	move all the data in the descriptor to private member variables.
	//	The only problem will be fixing the m_pReinforcements and m_pEncounters
	//	field which assumes that we have a copy of this).
	
	m_pDesc = pDesc->OrphanCopy();
	if (m_pDesc == NULL)
		{
		Ctx.sError = CONSTLIT("Out of memory");
		return ERR_MEMORY;
		}

	//	Load items

	CXMLElement *pItems = pDesc->GetContentElementByTag(ITEMS_TAG);
	if (pItems)
		{
		if (error = IItemGenerator::CreateFromXML(Ctx, pItems, &m_pItems))
			return error;
		}

	//	Load trade

	CXMLElement *pTrade = pDesc->GetContentElementByTag(TRADE_TAG);
	if (pTrade)
		{
		if (error = CTradingDesc::CreateFromXML(Ctx, pTrade, &m_pTrade))
			return error;
		}

	//	Find an element describing the image
	//	HACK: At some point, this should be handled by m_Image.

	CXMLElement *pImage = pDesc->GetContentElementByTag(IMAGE_TAG);
	if (pImage == NULL)
		pImage = pDesc->GetContentElementByTag(IMAGE_VARIANTS_TAG);
	if (pImage == NULL)
		pImage = pDesc->GetContentElementByTag(COMPOSITE_TAG);

	//	Load the image

	if (pImage)
		{
		//	If this image comes from a shipwreck then load it from
		//	a ship class object. Otherwise, initialize the image from the XML

		if (pImage->AttributeExists(SHIPWRECK_UNID_ATTRIB))
			{
			if (error = pImage->GetAttributeIntegerList(SHIPWRECK_UNID_ATTRIB, &m_ShipWrecks))
				{
				Ctx.sError = ComposeLoadError(CONSTLIT("Unable to load ship wreck list"));
				return error;
				}

			if (m_ShipWrecks.GetCount() == 0)
				{
				Ctx.sError = ComposeLoadError(CONSTLIT("Expected ship wreck list"));
				return ERR_FAIL;
				}

			m_iImageVariants = 0;
			}

		//	Otherwise, load the image

		else
			{
			if (error = m_Image.InitFromXML(Ctx, pImage))
				{
				Ctx.sError = ComposeLoadError(Ctx.sError);
				return error;
				}

			m_iImageVariants = m_Image.GetVariantCount();
			}
		}

	//	Load animations

	CXMLElement *pAnimations = pDesc->GetContentElementByTag(ANIMATIONS_TAG);
	if (pAnimations)
		{
		m_iAnimationsCount = pAnimations->GetContentElementCount();
		m_pAnimations = new SAnimationSection [m_iAnimationsCount];

		for (i = 0; i < m_iAnimationsCount; i++)
			{
			CXMLElement *pSection = pAnimations->GetContentElement(i);
			m_pAnimations[i].m_x = pSection->GetAttributeInteger(X_ATTRIB);
			m_pAnimations[i].m_y = pSection->GetAttributeInteger(Y_ATTRIB);

			if (pSection->GetContentElementCount() > 0)
				{
				CXMLElement *pImage = pSection->GetContentElement(0);
				if (error = m_pAnimations[i].m_Image.InitFromXML(Ctx, pImage))
					{
					Ctx.sError = ComposeLoadError(CONSTLIT("Unable to load animation image"));
					return error;
					}
				}
			}
		}

	//	Now get the first dock screen

	m_pFirstDockScreen.LoadUNID(Ctx, pDesc->GetAttribute(DOCK_SCREEN_ATTRIB));
	m_pAbandonedDockScreen.LoadUNID(Ctx, pDesc->GetAttribute(ABANDONED_SCREEN_ATTRIB));

	//	Background screens

	if (error = LoadUNID(Ctx, pDesc->GetAttribute(DEFAULT_BACKGROUND_ID_ATTRIB), &m_dwDefaultBkgnd))
		return error;
	
	//	Load initial ships

	CXMLElement *pShips = pDesc->GetContentElementByTag(SHIPS_TAG);
	if (pShips)
		{
		if (error = IShipGenerator::CreateFromXMLAsGroup(Ctx, pShips, &m_pInitialShips))
			{
			Ctx.sError = ComposeLoadError(strPatternSubst(CONSTLIT("<Ships>: %s"), Ctx.sError));
			return error;
			}
		}

	//	Load reinforcements

	CXMLElement *pReinforcements = m_pDesc->GetContentElementByTag(REINFORCEMENTS_TAG);
	if (pReinforcements)
		{
		if (error = IShipGenerator::CreateFromXMLAsGroup(Ctx, pReinforcements, &m_pReinforcements))
			{
			Ctx.sError = ComposeLoadError(strPatternSubst(CONSTLIT("<Reinforcements>: %s"), Ctx.sError));
			return error;
			}

		//	Figure out the minimum number of reinforcements at this base

		m_iMinShips = pReinforcements->GetAttributeInteger(MIN_SHIPS_ATTRIB);
		}

	//	Load encounter table

	CXMLElement *pEncounters = m_pDesc->GetContentElementByTag(ENCOUNTERS_TAG);
	if (pEncounters)
		{
		if (error = IShipGenerator::CreateFromXMLAsGroup(Ctx, pEncounters, &m_pEncounters))
			{
			Ctx.sError = ComposeLoadError(strPatternSubst(CONSTLIT("<Encounters>: %s"), Ctx.sError));
			return error;
			}

		m_iEncounterFrequency = GetFrequency(pEncounters->GetAttribute(FREQUENCY_ATTRIB));
		}

	//	If we don't have an encounter table then we might have a custom encounter event

	else
		{
		m_iEncounterFrequency = GetFrequency(pDesc->GetAttribute(RANDOM_ENCOUNTERS_ATTRIB));
		m_pEncounters = NULL;
		}

	//	Load construction table

	CXMLElement *pConstruction = m_pDesc->GetContentElementByTag(CONSTRUCTION_TAG);
	if (pConstruction)
		{
		if (error = IShipGenerator::CreateFromXMLAsGroup(Ctx, pConstruction, &m_pConstruction))
			{
			Ctx.sError = ComposeLoadError(strPatternSubst(CONSTLIT("<Construction>: %s"), Ctx.sError));
			return error;
			}

		m_iShipConstructionRate = pConstruction->GetAttributeInteger(CONSTRUCTION_RATE_ATTRIB);
		m_iMaxConstruction = pConstruction->GetAttributeInteger(MAX_CONSTRUCTION_ATTRIB);
		}

	//	Load satellites

	CXMLElement *pSatellitesDesc = pDesc->GetContentElementByTag(SATELLITES_TAG);
	if (pSatellitesDesc)
		m_pSatellitesDesc = pSatellitesDesc->OrphanCopy();

	//	Explosion

	if (error = m_pExplosionType.LoadUNID(Ctx, pDesc->GetAttribute(EXPLOSION_TYPE_ATTRIB)))
		return error;

	//	Ejecta

	if (error = m_pEjectaType.LoadUNID(Ctx, pDesc->GetAttribute(EJECTA_TYPE_ATTRIB)))
		return error;

	if (m_pEjectaType.GetUNID())
		{
		m_iEjectaAdj = pDesc->GetAttributeInteger(EJECTA_ADJ_ATTRIB);
		if (m_iEjectaAdj == 0)
			m_iEjectaAdj = 100;
		}
	else
		m_iEjectaAdj = 0;

	//	Miscellaneous

	if (error = m_pBarrierEffect.LoadUNID(Ctx, pDesc->GetAttribute(BARRIER_EFFECT_ATTRIB)))
		return error;

	m_sStargateDestNode = pDesc->GetAttribute(DEST_NODE_ATTRIB);
	m_sStargateDestEntryPoint = pDesc->GetAttribute(DEST_ENTRY_POINT_ATTRIB);

	if (error = m_pGateEffect.LoadUNID(Ctx, pDesc->GetAttribute(GATE_EFFECT_ATTRIB)))
		return error;

	m_fHasOnObjDockedEvent = FindEventHandler(CONSTLIT("OnObjDocked"));

	//	Done

	return NOERROR;
	}

ALERROR CStationType::OnFinishBindDesign (SDesignLoadCtx &Ctx)

//	OnFinishBindDesign
//
//	Do stuff after all types bound

	{
	return NOERROR;
	}

void CStationType::OnMarkImages (void)

//	OnMarkImages
//
//	Mark images in use.

	{
	//	Since we are generally loading images for this type, we need to create
	//	a default image selector

	CCompositeImageSelector Selector;
	SetImageSelector(NULL, &Selector);

	//	Mark

	MarkImages(Selector);
	}

void CStationType::OnReadFromStream (SUniverseLoadCtx &Ctx)

//	ReadFromStream
//
//	Read from stream
//
//	DWORD		flags
//	CAttributeDataBlock	m_Data

	{
	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	
	m_fEncountered =	((dwLoad & 0x00000001) ? true : false);

	//	Load opaque data

	ReadGlobalData(Ctx);
	}

void CStationType::OnReinit (void)

//	OnReinit
//
//	Reinitialize the type

	{
	m_fEncountered = false;
	}

void CStationType::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes out the variable portions of the item type
//
//	DWORD		flags

	{
	DWORD dwSave;

	dwSave = 0;
	dwSave |= (m_fEncountered ?	0x00000001 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CStationType::PaintAnimations (CG16bitImage &Dest, int x, int y, int iTick)

//	PaintAnimations
//
//	Paint animations

	{
	int i;

	for (i = 0; i < m_iAnimationsCount; i++)
		{
		m_pAnimations[i].m_Image.PaintImage(Dest,
				x + m_pAnimations[i].m_x,
				y - m_pAnimations[i].m_y,
				iTick,
				0);
		}
	}

void CStationType::Reinit (void)

//	Reinit
//
//	Reinitialize global data

	{
	}

void CStationType::SetImageSelector (CStation *pStation, CCompositeImageSelector *retSelector)

//	SetImageSelector
//
//	Sets the image for the station (if necessary). Also, sets the variant

	{
	if (m_ShipWrecks.GetCount())
		{
		DWORD dwShipwreckID = (DWORD)m_ShipWrecks.GetElement(mathRandom(0, m_ShipWrecks.GetCount()-1));
		CShipClass *pClass = g_pUniverse->FindShipClass(dwShipwreckID);
		if (pClass == NULL)
			return;

		retSelector->AddShipwreck(DEFAULT_SELECTOR_ID, pClass);
		}
	else
		{
		m_Image.InitSelector(retSelector);

#if 0
		int iVariantCount = GetImageVariants();
		if (iVariantCount)
			retSelector->AddVariant(mathRandom(0, iVariantCount-1));
		else
			retSelector->AddVariant(0);
#endif
		}
	}
