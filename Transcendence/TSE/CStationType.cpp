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
#define IMAGE_EFFECT_TAG						CONSTLIT("ImageEffect")
#define IMAGE_VARIANTS_TAG						CONSTLIT("ImageVariants")
#define ITEMS_TAG								CONSTLIT("Items")
#define NAMES_TAG								CONSTLIT("Names")
#define REINFORCEMENTS_TAG						CONSTLIT("Reinforcements")
#define SATELLITES_TAG							CONSTLIT("Satellites")
#define SHIP_TAG								CONSTLIT("Ship")
#define SHIPS_TAG								CONSTLIT("Ships")
#define STATION_TAG								CONSTLIT("Station")
#define TABLE_TAG								CONSTLIT("Table")
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
#define CHANCE_ATTRIB							CONSTLIT("chance")
#define CONSTRUCTION_RATE_ATTRIB				CONSTLIT("constructionRate")
#define CONTROLLING_SOVEREIGN_ATTRIB			CONSTLIT("controllingSovereign")
#define COUNT_ATTRIB							CONSTLIT("count")
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
#define GRAVITY_RADIUS_ATTRIB					CONSTLIT("gravityRadius")
#define HIT_POINTS_ATTRIB						CONSTLIT("hitPoints")
#define IMMUTABLE_ATTRIB						CONSTLIT("immutable")
#define INACTIVE_ATTRIB							CONSTLIT("inactive")
#define LEVEL_ATTRIB							CONSTLIT("level")
#define LEVEL_FREQUENCY_ATTRIB					CONSTLIT("levelFrequency")
#define LOCATION_CRITERIA_ATTRIB				CONSTLIT("locationCriteria")
#define MASS_ATTRIB								CONSTLIT("mass")
#define MAX_CONSTRUCTION_ATTRIB					CONSTLIT("maxConstruction")
#define MAX_HIT_POINTS_ATTRIB					CONSTLIT("maxHitPoints")
#define MAX_LIGHT_DISTANCE						CONSTLIT("maxLightRadius")
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
#define REGEN_ATTRIB							CONSTLIT("regen")
#define REPAIR_RATE_ATTRIB						CONSTLIT("repairRate")
#define REVERSE_ARTICLE_ATTRIB					CONSTLIT("reverseArticle")
#define SCALE_ATTRIB							CONSTLIT("scale")
#define SHIP_ENCOUNTER_ATTRIB					CONSTLIT("shipEncounter")
#define SHIP_REGEN_ATTRIB						CONSTLIT("shipRegen")
#define SHIP_REPAIR_RATE_ATTRIB					CONSTLIT("shipRepairRate")
#define SHIPWRECK_UNID_ATTRIB					CONSTLIT("shipwreckID")
#define SIGN_ATTRIB								CONSTLIT("sign")
#define SIZE_ATTRIB								CONSTLIT("size")
#define SOVEREIGN_ATTRIB						CONSTLIT("sovereign")
#define SPACE_COLOR_ATTRIB						CONSTLIT("spaceColor")
#define STANDING_COUNT_ATTRIB					CONSTLIT("standingCount")
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

#define EVENT_ON_MINING							CONSTLIT("OnMining")

#define FIELD_ABANDONED_DOCK_SCREEN				CONSTLIT("abandonedDockScreen")
#define FIELD_ARMOR_CLASS						CONSTLIT("armorClass")
#define FIELD_ARMOR_LEVEL						CONSTLIT("armorLevel")
#define FIELD_BALANCE							CONSTLIT("balance")
#define FIELD_CAN_ATTACK						CONSTLIT("canAttack")
#define FIELD_CATEGORY							CONSTLIT("category")
#define FIELD_DEFENDER_STRENGTH					CONSTLIT("defenderStrength")
#define FIELD_DOCK_SCREEN						CONSTLIT("dockScreen")
#define FIELD_EXPLOSION_TYPE					CONSTLIT("explosionType")
#define FIELD_FIRE_RATE_ADJ						CONSTLIT("fireRateAdj")
#define FIELD_HITS_TO_DESTROY					CONSTLIT("hitsToDestroy")			//	# of hits by std level weapon to destroy station
#define FIELD_INSTALL_DEVICE_MAX_LEVEL			CONSTLIT("installDeviceMaxLevel")
#define FIELD_HP								CONSTLIT("hp")
#define FIELD_LEVEL								CONSTLIT("level")
#define FIELD_LOCATION_CRITERIA					CONSTLIT("locationCriteria")
#define FIELD_MAX_LIGHT_RADIUS					CONSTLIT("maxLightRadius")
#define FIELD_NAME								CONSTLIT("name")
#define FIELD_REGEN								CONSTLIT("regen")					//	hp repaired per 180 ticks
#define FIELD_SATELLITE_STRENGTH				CONSTLIT("satelliteStrength")
#define FIELD_SIZE								CONSTLIT("size")
#define FIELD_TREASURE_BALANCE					CONSTLIT("treasureBalance")			//	100 = treasure appropriate for defenses, 200 = twice as much treasure
#define FIELD_TREASURE_VALUE					CONSTLIT("treasureValue")
#define FIELD_WEAPON_STRENGTH					CONSTLIT("weaponStrength")			//	Strength of weapons (100 = level weapon @ 1/4 fire rate).

#define VALUE_FALSE								CONSTLIT("false")
#define VALUE_TRUE								CONSTLIT("true")

#define MAX_ATTACK_DISTANCE						(g_KlicksPerPixel * 512)

#define SPECIAL_IS_ENEMY_OF						CONSTLIT("isEnemyOf:")
#define SPECIAL_SCALE							CONSTLIT("scale:")
#define SPECIAL_SIZE_CLASS						CONSTLIT("sizeClass:")

struct SSizeData
	{
	int iMinSize;
	int iMaxSize;
	};

//	NOTE: These must match the values of ESizeClass

static SSizeData SIZE_DATA[] = 
	{
		{	0,			0,	},

		{	1,			24,	},
		{	25,			74,	},
		{	75,			299,	},
		{	300,		749,	},

		{	750,		1499,	},
		{	1500,		2999,	},
		{	3000,		4499,	},

		{	4500,		7499,	},
		{	7500,		14999,	},
		{	15000,		29999,	},

		{	30000,		74999,	},
		{	75000,		149999,	},
		{	150000,		1000000,	},
	};

//	Standard station table

struct SStdStationDesc
	{
	CurrencyValue dwTreasureValue;			//	Std value of treasure at this level
	};

static SStdStationDesc STD_STATION_DATA[] =
	{
		{	0,		},

		{	750,	},
		{	1000,	},
		{	1500,	},
		{	2000,	},
		{	4000,	},

		{	8000,	},
		{	16000,	},
		{	32000,	},
		{	64000,	},
		{	128000,	},

		{	256000,	},
		{	512000,	},
		{	1000000,	},
		{	2000000,	},
		{	4100000,	},

		{	8200000,	},
		{	16400000,	},
		{	32800000,	},
		{	65500000,	},
		{	131000000,	},

		{	262000000,	},
		{	524000000,	},
		{	1000000000,	},
		{	2100000000,	},
		{	4200000000,	},
	};

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

Metric CStationType::CalcBalance (int iLevel)

//	CalcBalance
//
//	Calculates the station defense balance assuming the station is at the given level.

	{
	if (iLevel <= 0 || iLevel > MAX_TECH_LEVEL)
		return 0.0;

	Metric rBalance = (CalcDefenderStrength(iLevel) + CalcWeaponStrength(iLevel)) / 3.5;
	rBalance *= sqrt((Metric)CalcHitsToDestroy(iLevel) / 50.0);
	return rBalance;
	}

Metric CStationType::CalcDefenderStrength (int iLevel)

//	CalcDefenderStrength
//
//	Returns the strength of station defenders
//	1.0 = 1 defender of station level.

	{
	Metric rTotal = 0.0;

	//	Add ship defenders

	if (m_pInitialShips)
		rTotal += m_pInitialShips->GetAverageLevelStrength(iLevel);

	//	Add satellite defenses

	if (m_pSatellitesDesc)
		rTotal += CalcSatelliteStrength(m_pSatellitesDesc, iLevel);

	//	Done

	return rTotal;
	}

int CStationType::CalcHitsToDestroy (int iLevel)

//	CalcHitsToDestroy
//
//	Returns the number of hits that it would take to destroy the station when 
//	hit by a standard weapon of the given level.

	{
	//	If station cannot be destroyed, then 0

	if (IsImmutable()
			|| (m_iMaxHitPoints == 0 && m_iMaxStructuralHP == 0))
		return 0;

	//	Compute the weapon that we want to use.

	int iDamageAdj = (m_pArmor ? m_pArmor->GetArmorClass()->GetDamageAdjForWeaponLevel(iLevel) : 100);
	Metric rWeaponDamage = (Metric)CWeaponClass::GetStdDamage(iLevel);

	//	If the station is multi-hulled, then assume WMD4 and adjust weapon damage.

	if (IsMultiHull())
		{
		int iWMD = 34;
		rWeaponDamage = Max(1.0, (iWMD * rWeaponDamage / 100.0));
		}

	//	If we have hit points, then use that. Otherwise, we use structural 
	//	points.

	int iTotalHP;
	if (m_iMaxHitPoints > 0)
		{
		iTotalHP = m_iMaxHitPoints;

		//	Adjust weapon damage for station repairs. The standard fire rate is
		//	once per 8 ticks, and the repair rate is per 30 ticks.

		if (!m_Regen.IsEmpty())
			rWeaponDamage = Max(0.0, rWeaponDamage - (8.0 * m_Regen.GetHPPer180(STATION_REPAIR_FREQUENCY) / 180.0));
		}
	else
		iTotalHP = m_iMaxStructuralHP;

	//	Adjust weapon damage for armor

	rWeaponDamage = iDamageAdj * rWeaponDamage / 100.0;

	//	If weapon does no damage then we can never destroy the station

	if (rWeaponDamage <= 0.0)
		return 0;

	//	Otherwise, divide to figure out the number of hits to destroy.

	Metric rTotalHits = Max(1.0, (Metric)iTotalHP / rWeaponDamage);

	//	Add hits to destroy satellites

	if (m_pSatellitesDesc)
		rTotalHits += CalcSatelliteHitsToDestroy(m_pSatellitesDesc, iLevel);

	//	DOne

	return (int)rTotalHits;
	}

Metric CStationType::CalcSatelliteHitsToDestroy (CXMLElement *pSatellites, int iLevel, bool bIgnoreChance)

//	CalcSatelliteHitsToDestroy
//
//	Calculate the total hits to destroy all satellites

	{
	int i;

	const CString &sTag = pSatellites->GetTag();

	int iChance;
	Metric rChanceAdj;
	if (!bIgnoreChance
			&& pSatellites->FindAttributeInteger(CHANCE_ATTRIB, &iChance))
		rChanceAdj = (Metric)iChance / 100.0;
	else
		rChanceAdj = 1.0;

	Metric rTotalHits = 0.0;
	if (strEquals(sTag, SHIP_TAG))
		{
		//	Skip ships
		}
	else if (strEquals(sTag, STATION_TAG))
		{
		CStationType *pStationType = g_pUniverse->FindStationType((DWORD)pSatellites->GetAttributeInteger(TYPE_ATTRIB));
		if (pStationType == NULL || !pStationType->CanAttack())
			return 0.0;

		rTotalHits += (rChanceAdj * pStationType->CalcHitsToDestroy(iLevel));
		}
	else if (strEquals(sTag, TABLE_TAG))
		{
		int iTotalChance = 0;
		for (i = 0; i < pSatellites->GetContentElementCount(); i++)
			iTotalChance += pSatellites->GetContentElement(i)->GetAttributeInteger(CHANCE_ATTRIB);

		if (iTotalChance > 0)
			{
			for (i = 0; i < pSatellites->GetContentElementCount(); i++)
				rTotalHits += rChanceAdj * CalcSatelliteHitsToDestroy(pSatellites->GetContentElement(i), iLevel, true) 
						* (Metric)pSatellites->GetContentElement(i)->GetAttributeInteger(CHANCE_ATTRIB)
						/ 100.0;
			}
		}
	else
		{
		Metric rCount = 1.0;

		CString sAttrib;
		if (pSatellites->FindAttribute(COUNT_ATTRIB, &sAttrib))
			{
			DiceRange Count;
			Count.LoadFromXML(sAttrib);
			rCount = Count.GetAveValueFloat();
			}

		for (i = 0; i < pSatellites->GetContentElementCount(); i++)
			rTotalHits += rCount * rChanceAdj * CalcSatelliteHitsToDestroy(pSatellites->GetContentElement(i), iLevel);
		}

	//	Done

	return rTotalHits;
	}

Metric CStationType::CalcSatelliteStrength (CXMLElement *pSatellites, int iLevel, bool bIgnoreChance)

//	CalcSatelliteStrength
//
//	Computes the total strength of satellites
//	100 = 1 satellite at station level.

	{
	int i;

	const CString &sTag = pSatellites->GetTag();

	int iChance;
	Metric rChanceAdj;
	if (!bIgnoreChance
			&& pSatellites->FindAttributeInteger(CHANCE_ATTRIB, &iChance))
		rChanceAdj = (Metric)iChance / 100.0;
	else
		rChanceAdj = 1.0;

	Metric rTotal = 0.0;
	if (strEquals(sTag, SHIP_TAG))
		{
		//	Load generator

		SDesignLoadCtx Ctx;
		IShipGenerator *pGenerator;
		if (IShipGenerator::CreateFromXML(Ctx, pSatellites, &pGenerator) != NOERROR)
			return 0.0;

		if (pGenerator->OnDesignLoadComplete(Ctx) != NOERROR)
			{
			delete pGenerator;
			return 0.0;
			}

		rTotal += rChanceAdj * pGenerator->GetAverageLevelStrength(iLevel);

		delete pGenerator;
		}
	else if (strEquals(sTag, STATION_TAG))
		{
		CStationType *pStationType = g_pUniverse->FindStationType((DWORD)pSatellites->GetAttributeInteger(TYPE_ATTRIB));
		if (pStationType == NULL)
			return 0.0;

		rTotal += rChanceAdj * pStationType->GetLevelStrength(iLevel);
		}
	else if (strEquals(sTag, TABLE_TAG))
		{
		int iTotalChance = 0;
		for (i = 0; i < pSatellites->GetContentElementCount(); i++)
			iTotalChance += pSatellites->GetContentElement(i)->GetAttributeInteger(CHANCE_ATTRIB);

		if (iTotalChance > 0)
			{
			for (i = 0; i < pSatellites->GetContentElementCount(); i++)
				rTotal += rChanceAdj * CalcSatelliteStrength(pSatellites->GetContentElement(i), iLevel, true) 
						* (Metric)pSatellites->GetContentElement(i)->GetAttributeInteger(CHANCE_ATTRIB)
						/ 100.0;
			}
		}
	else
		{
		Metric rCount = 1.0;

		CString sAttrib;
		if (pSatellites->FindAttribute(COUNT_ATTRIB, &sAttrib))
			{
			DiceRange Count;
			Count.LoadFromXML(sAttrib);
			rCount = Count.GetAveValueFloat();
			}

		for (i = 0; i < pSatellites->GetContentElementCount(); i++)
			rTotal += rCount * rChanceAdj * CalcSatelliteStrength(pSatellites->GetContentElement(i), iLevel);
		}

	//	Done

	return rTotal;
	}

Metric CStationType::CalcSatelliteTreasureValue (CXMLElement *pSatellites, int iLevel, bool bIgnoreChance)

//	CalcSatelliteTreasureValue
//
//	Computes the total treasure value in satellites

	{
	int i;

	const CString &sTag = pSatellites->GetTag();

	int iChance;
	Metric rChanceAdj;
	if (!bIgnoreChance
			&& pSatellites->FindAttributeInteger(CHANCE_ATTRIB, &iChance))
		rChanceAdj = (Metric)iChance / 100.0;
	else
		rChanceAdj = 1.0;

	Metric rTotal = 0.0;
	if (strEquals(sTag, SHIP_TAG))
		{
		//	Skip
		}
	else if (strEquals(sTag, STATION_TAG))
		{
		CStationType *pStationType = g_pUniverse->FindStationType((DWORD)pSatellites->GetAttributeInteger(TYPE_ATTRIB));
		if (pStationType == NULL)
			return 0.0;

		Metric rTreasure = rChanceAdj * pStationType->CalcTreasureValue(iLevel);

		//	Treasure in asteroids should be discounted (since not everyone has
		//	a mining cannon).

		if (!pStationType->CanAttack() 
				&& pStationType->HasAttribute(CONSTLIT("asteroid")))
			rTreasure *= 0.2;

		rTotal += rTreasure;
		}
	else if (strEquals(sTag, TABLE_TAG))
		{
		int iTotalChance = 0;
		for (i = 0; i < pSatellites->GetContentElementCount(); i++)
			iTotalChance += pSatellites->GetContentElement(i)->GetAttributeInteger(CHANCE_ATTRIB);

		if (iTotalChance > 0)
			{
			for (i = 0; i < pSatellites->GetContentElementCount(); i++)
				rTotal += rChanceAdj * CalcSatelliteTreasureValue(pSatellites->GetContentElement(i), iLevel, true) 
						* (Metric)pSatellites->GetContentElement(i)->GetAttributeInteger(CHANCE_ATTRIB)
						/ 100.0;
			}
		}
	else
		{
		Metric rCount = 1.0;

		CString sAttrib;
		if (pSatellites->FindAttribute(COUNT_ATTRIB, &sAttrib))
			{
			DiceRange Count;
			Count.LoadFromXML(sAttrib);
			rCount = Count.GetAveValueFloat();
			}

		for (i = 0; i < pSatellites->GetContentElementCount(); i++)
			rTotal += rCount * rChanceAdj * CalcSatelliteTreasureValue(pSatellites->GetContentElement(i), iLevel);
		}

	//	Done

	return rTotal;
	}

Metric CStationType::CalcTreasureValue (int iLevel)

//	CalcTreasureValue
//
//	Calculates the value of total treasure, include treasure on satellites.

	{
	Metric rTotal = 0.0;

	if (m_pItems)
		rTotal += m_pItems->GetAverageValue(iLevel);

	if (m_pSatellitesDesc)
		rTotal += CalcSatelliteTreasureValue(m_pSatellitesDesc, iLevel);

	return rTotal;
	}

Metric CStationType::CalcWeaponStrength (int iLevel)

//	CalcWeaponStrength
//
//	Computes the total strength of all station weapons.
//	1.0 = level weapon at 1/4 fire rate.

	{
	int i;

	//	Start by adding up all weapons.

	Metric rTotal = 0.0;
	for (i = 0; i < m_iDevicesCount; i++)
		{
		if (m_Devices[i].IsEmpty()
				|| (m_Devices[i].GetCategory() != itemcatWeapon
					&& m_Devices[i].GetCategory() != itemcatLauncher))
			continue;

		CDeviceClass *pDeviceClass = m_Devices[i].GetClass();
		int iDevLevel = pDeviceClass->GetLevel();

		//	Lower level weapons count less; higher level weapons count more.

		rTotal += ::CalcLevelDiffStrength(iDevLevel - iLevel);
		}

	//	Adjust for fire rate. Double the fire rate means double the strength.

	rTotal *= (40.0 / (Metric)m_iFireRateAdj);

	//	Done

	return rTotal;
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
	else if (strEquals(sField, FIELD_BALANCE))
		*retsValue = strFromInt((int)(CalcBalance(GetLevel()) * 100.0));
	else if (strEquals(sField, FIELD_CATEGORY))
		{
		if (!CanBeEncounteredRandomly())
			*retsValue = CONSTLIT("04-Not Random");
		else if (HasLiteralAttribute(CONSTLIT("debris")))
			*retsValue = CONSTLIT("03-Debris");
		else if (HasLiteralAttribute(CONSTLIT("enemy")))
			*retsValue = CONSTLIT("02-Enemy");
		else if (HasLiteralAttribute(CONSTLIT("friendly")))
			*retsValue = CONSTLIT("01-Friendly");
		else
			*retsValue = CONSTLIT("04-Not Random");
		}
	else if (strEquals(sField, FIELD_DEFENDER_STRENGTH))
		*retsValue = strFromInt((int)(100.0 * CalcDefenderStrength(GetLevel())));
	else if (strEquals(sField, FIELD_DOCK_SCREEN))
		*retsValue = m_pFirstDockScreen.GetStringUNID(this);
	else if (strEquals(sField, FIELD_LEVEL))
		*retsValue = strFromInt(GetLevel());
	else if (strEquals(sField, FIELD_LOCATION_CRITERIA))
		*retsValue = GetLocationCriteria();
	else if (strEquals(sField, FIELD_NAME))
		*retsValue = GetName();
	else if (strEquals(sField, FIELD_ARMOR_CLASS))
		{
		if (m_pArmor)
			*retsValue = m_pArmor->GetArmorClass()->GetShortName();
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_ARMOR_LEVEL))
		{
		if (m_pArmor)
			*retsValue = strFromInt(m_pArmor->GetLevel());
		else
			*retsValue = NULL_STR;
		}
	else if (strEquals(sField, FIELD_HP))
		*retsValue = strFromInt(m_iHitPoints);
	else if (strEquals(sField, FIELD_FIRE_RATE_ADJ))
		*retsValue = strFromInt(10000 / m_iFireRateAdj);
	else if (strEquals(sField, FIELD_HITS_TO_DESTROY))
		*retsValue = strFromInt(CalcHitsToDestroy(GetLevel()));
	else if (strEquals(sField, FIELD_INSTALL_DEVICE_MAX_LEVEL))
		{
		int iMaxLevel = (m_pTrade ? m_pTrade->GetMaxLevelMatched(serviceInstallDevice) : -1);
		*retsValue = (iMaxLevel != -1 ? strFromInt(iMaxLevel) : NULL_STR);
		}

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
	else if (strEquals(sField, FIELD_MAX_LIGHT_RADIUS))
		*retsValue = strFromInt(m_iMaxLightDistance);
	else if (strEquals(sField, FIELD_REGEN))
		*retsValue = strFromInt((int)(m_Regen.GetHPPer180(STATION_REPAIR_FREQUENCY) + 0.5));
	else if (strEquals(sField, FIELD_SATELLITE_STRENGTH))
		*retsValue = strFromInt((m_pSatellitesDesc ? (int)(100.0 * CalcSatelliteStrength(m_pSatellitesDesc, GetLevel())) : 0));
	else if (strEquals(sField, FIELD_SIZE))
		*retsValue = strFromInt(m_iSize);
	else if (strEquals(sField, FIELD_TREASURE_BALANCE))
		{
		int iLevel = GetLevel();
		Metric rExpected = CalcBalance(iLevel) * (Metric)STD_STATION_DATA[iLevel].dwTreasureValue;
		Metric rTreasure = CalcTreasureValue(iLevel);
		*retsValue = strFromInt((int)(100.0 * (rExpected > 0.0 ? rTreasure / rExpected : 0.0)));
		}
	else if (strEquals(sField, FIELD_TREASURE_VALUE))
		*retsValue = strFromInt((int)CalcTreasureValue(GetLevel()));
	else if (strEquals(sField, FIELD_WEAPON_STRENGTH))
		*retsValue = strFromInt((int)(100.0 * CalcWeaponStrength(GetLevel())));
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

int CStationType::GetLevel (int *retiMinLevel, int *retiMaxLevel) const

//	GetLevel
//
//	Returns the average level of the station.
//
//	NOTE: We don't return a level range yet because we're not sure of the 
//	backwards compatibility issues.

	{
	int i;
	int iLevel;

	if (m_iLevel)
		{
		if (retiMinLevel) *retiMinLevel = m_iLevel;
		if (retiMaxLevel) *retiMaxLevel = m_iLevel;
		return m_iLevel;
		}
	else if (iLevel = m_RandomPlacement.CalcLevelFromFrequency())
		{
		if (retiMinLevel) *retiMinLevel = iLevel;
		if (retiMaxLevel) *retiMaxLevel = iLevel;
		return iLevel;
		}
	else
		{
		//	Take the highest level of armor or devices

		iLevel = (m_pArmor ? m_pArmor->GetLevel() : 1);
		for (i = 0; i < m_iDevicesCount; i++)
			{
			if (!m_Devices[i].IsEmpty())
				{
				int iDeviceLevel = m_Devices[i].GetClass()->GetLevel();
				if (iDeviceLevel > iLevel)
					iLevel = iDeviceLevel;
				}
			}

		//	Done

		if (retiMinLevel) *retiMinLevel = iLevel;
		if (retiMaxLevel) *retiMaxLevel = iLevel;
		return iLevel;
		}
	}

Metric CStationType::GetLevelStrength (int iLevel)

//	GetLevelStrength
//
//	Returns the level strength of the given station (relative to iLevel).
//	1.0 = station with level appropriate weapon and 25 hits to destroy.

	{
	Metric rTotal = 0.0;

	//	Strength is based on weapons and ships

	rTotal = CalcWeaponStrength(iLevel) + CalcDefenderStrength(iLevel);

	//	Adjust by armor. 1.0 = 25 hits to destroy.

	rTotal *= (CalcHitsToDestroy(iLevel) / 25.0);

	//	Done

	return rTotal;
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

IShipGenerator *CStationType::GetReinforcementsTable (void)

//	GetReinforcementsTable
//
//	Return the reinforcements table.
	
	{
	//	If we have a ship count structure, then we always use the main ship 
	//	table.

	if (!m_ShipsCount.IsEmpty())
		return m_pInitialShips;

	//	Otherwise, if we have an explicit reinforcements table, use that.

	else if (m_pReinforcements)
		return m_pReinforcements;

	//	Otherwise, we use the main table.

	else
		return m_pInitialShips;
	}

bool CStationType::IsSizeClass (ESizeClass iClass) const

//	IsSizeClass
//
//	Returns TRUE if we are the given size class.

	{
	switch (GetScale())
		{
		case scaleWorld:
			return (iClass >= worldSizeA && iClass <= worldSizeM
					&& m_iSize >= SIZE_DATA[iClass].iMinSize 
					&& m_iSize <= SIZE_DATA[iClass].iMaxSize);

		default:
			return false;
		}
	}

void CStationType::MarkImages (const CCompositeImageSelector &Selector)

//	MarkImages
//
//	Marks images used by the station

	{
	m_Image.MarkImage(Selector);

	//	Cache the destroyed station image, if necessary

	if (HasWreckImage())
		{
		CCompositeImageModifiers Modifiers;
		Modifiers.SetStationDamage();
		m_Image.MarkImage(Selector, Modifiers);
		}

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

	//	Any object has not HP and is a star or a world is immutable
	//	by default.

	if (!m_fImmutable
			&& (m_iScale == scaleStar || m_iScale == scaleWorld)
			&& m_iMaxHitPoints == 0
			&& m_iStructuralHP == 0
			&& m_iEjectaAdj == 0
			&& !FindEventHandler(EVENT_ON_MINING))
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
	m_fNoMapIcon = pDesc->GetAttributeBool(NO_MAP_ICON_ATTRIB);
	m_fMultiHull = pDesc->GetAttributeBool(MULTI_HULL_ATTRIB);
	m_fTimeStopImmune = pDesc->GetAttributeBool(TIME_STOP_IMMUNE_ATTRIB);
	m_fCanAttack = pDesc->GetAttributeBool(CAN_ATTACK_ATTRIB);
	m_fReverseArticle = pDesc->GetAttributeBool(REVERSE_ARTICLE_ATTRIB);
	m_fShipEncounter = pDesc->GetAttributeBool(SHIP_ENCOUNTER_ATTRIB);
	m_fImmutable = pDesc->GetAttributeBool(IMMUTABLE_ATTRIB);
	m_fNoBlacklist = pDesc->GetAttributeBool(NO_BLACKLIST_ATTRIB);
	m_iAlertWhenAttacked = pDesc->GetAttributeInteger(ALERT_WHEN_ATTACKED_ATTRIB);
	m_iAlertWhenDestroyed = pDesc->GetAttributeInteger(ALERT_WHEN_DESTROYED_ATTRIB);
	m_rMaxAttackDistance = MAX_ATTACK_DISTANCE;
	m_iStealth = pDesc->GetAttributeIntegerBounded(STEALTH_ATTRIB, CSpaceObject::stealthMin, CSpaceObject::stealthMax, CSpaceObject::stealthNormal);

	//	Repair rate

	CString sRegen;
	int iRepairRate;
	if (pDesc->FindAttribute(REGEN_ATTRIB, &sRegen))
		{
		if (error = m_Regen.InitFromRegenString(Ctx, sRegen, STATION_REPAIR_FREQUENCY))
			return error;
		}
	else if (pDesc->FindAttributeInteger(REPAIR_RATE_ATTRIB, &iRepairRate) && iRepairRate > 0)
		m_Regen.InitFromRegen(6.0 * iRepairRate, STATION_REPAIR_FREQUENCY);

	//	Ship repair rate

	if (pDesc->FindAttribute(SHIP_REGEN_ATTRIB, &sRegen))
		{
		if (error = m_ShipRegen.InitFromRegenString(Ctx, sRegen, STATION_REPAIR_FREQUENCY))
			return error;
		}
	else if (pDesc->FindAttributeInteger(SHIP_REPAIR_RATE_ATTRIB, &iRepairRate) && iRepairRate > 0)
		m_ShipRegen.InitFromRegen(6.0 * iRepairRate, STATION_REPAIR_FREQUENCY);

	//	Starting in API 23 we change the default to 40 instead of 80

	int iDefaultFireRateAdj = (GetAPIVersion() >= 23 ? 40 : 80);
	m_iFireRateAdj = strToInt(pDesc->GetAttribute(FIRE_RATE_ADJ_ATTRIB), iDefaultFireRateAdj);

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

	//	Placement

	if (error = m_RandomPlacement.InitFromStationTypeXML(Ctx, pDesc))
		return error;

	m_EncounterRecord.Reinit(m_RandomPlacement);

	//	Background objects

	m_rParallaxDist = pDesc->GetAttributeIntegerBounded(BACKGROUND_PLANE_ATTRIB, 0, -1, 100) / 100.0;
	m_fOutOfPlane = (m_rParallaxDist != 1.0);

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

	//	Get the scale (default to structure)

	m_iScale = ParseScale(pDesc->GetAttribute(SCALE_ATTRIB));
	if (m_iScale == scaleNone)
		m_iScale = scaleStructure;

	//	Mass & Size
	
	m_iSize = pDesc->GetAttributeIntegerBounded(SIZE_ATTRIB, 1, -1, 0);

	if (!pDesc->FindAttributeDouble(MASS_ATTRIB, &m_rMass))
		{
		if (m_iScale == scaleWorld || m_iScale == scaleStar)
			m_rMass = 1.0;	//	1 Earth mass or 1 solar mass.
		else
			m_rMass = 1000000.0;
		}

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
	if (pImage == NULL)
		pImage = pDesc->GetContentElementByTag(IMAGE_EFFECT_TAG);

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

		//	If defined, we use this count to create initial ships AND reinforcements.

		CString sCount;
		if (pShips->FindAttribute(STANDING_COUNT_ATTRIB, &sCount))
			{
			if (error = m_ShipsCount.LoadFromXML(sCount))
				{
				Ctx.sError = ComposeLoadError(CONSTLIT("Invalid count attribute in <Ships>"));
				return error;
				}
			}

		//	Otherwise, see if we define minShips, in which case we use that value for
		//	reinforcements only.

		else
			m_iMinShips = pShips->GetAttributeInteger(MIN_SHIPS_ATTRIB);
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

	//	Stellar objects

	m_rgbSpaceColor = LoadCOLORREF(pDesc->GetAttribute(SPACE_COLOR_ATTRIB));
	m_iMaxLightDistance = pDesc->GetAttributeIntegerBounded(MAX_LIGHT_DISTANCE, 1, -1, 500);

	int iGravity;
	if (pDesc->FindAttributeInteger(GRAVITY_RADIUS_ATTRIB, &iGravity))
		m_rGravityRadius = iGravity * LIGHT_SECOND;
	else
		m_rGravityRadius = 0.0;

	//	Stargates

	m_sStargateDestNode = pDesc->GetAttribute(DEST_NODE_ATTRIB);
	m_sStargateDestEntryPoint = pDesc->GetAttribute(DEST_ENTRY_POINT_ATTRIB);

	if (error = m_pGateEffect.LoadUNID(Ctx, pDesc->GetAttribute(GATE_EFFECT_ATTRIB)))
		return error;

	//	Miscellaneous

	if (error = m_pBarrierEffect.LoadUNID(Ctx, pDesc->GetAttribute(BARRIER_EFFECT_ATTRIB)))
		return error;

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

bool CStationType::OnHasSpecialAttribute (const CString &sAttrib) const

//	OnHasSpecialAttribute
//
//	Returns TRUE if we have the special attribute

	{
	if (strStartsWith(sAttrib, SPECIAL_IS_ENEMY_OF))
		{
		CString sValue = strSubString(sAttrib, SPECIAL_IS_ENEMY_OF.GetLength());
		DWORD dwSovereign = (DWORD)strToInt(sValue, 0);
		CSovereign *pSovereign = g_pUniverse->FindSovereign(dwSovereign);
		if (pSovereign == NULL)
			return false;

		return m_pSovereign->IsEnemy(pSovereign);
		}
	else if (strStartsWith(sAttrib, SPECIAL_SCALE))
		{
		CString sValue = strSubString(sAttrib, SPECIAL_SCALE.GetLength());
		return (ParseScale(sValue) == GetScale());
		}
	else if (strStartsWith(sAttrib, SPECIAL_SIZE_CLASS))
		{
		CString sValue = strSubString(sAttrib, SPECIAL_SIZE_CLASS.GetLength());
		ESizeClass iClass = ParseSizeClass(sValue);
		return IsSizeClass(iClass);
		}
	else
		return false;
	}

void CStationType::OnMarkImages (void)

//	OnMarkImages
//
//	Mark images in use.

	{
	//	Since we are generally loading images for this type, we need to create
	//	a default image selector

	SSelectorInitCtx InitCtx;
	CCompositeImageSelector Selector;
	SetImageSelector(InitCtx, &Selector);

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
	
	bool bEncountered;
	bEncountered =		((dwLoad & 0x00000001) ? true : false);

	//	Load encounter record

	if (Ctx.dwVersion >= 25)
		m_RandomPlacement.ReadFromStream(Ctx);

	if (Ctx.dwVersion >= 19)
		m_EncounterRecord.ReadFromStream(Ctx);
	else
		{
		if (bEncountered)
			m_EncounterRecord.AddEncounter(NULL);
		}

	//	Load opaque data

	ReadGlobalData(Ctx);
	}

void CStationType::OnReinit (void)

//	OnReinit
//
//	Reinitialize the type

	{
	m_EncounterRecord.Reinit(m_RandomPlacement);
	m_Image.Reinit();
	}

void CStationType::OnTopologyInitialized (void)

//	OnTopologyInitialize
//
//	The topology has been initialized.

	{
	//	We take this opportunity to resolve the level of certain encounters
	//	(now that we know the topology).

	m_RandomPlacement.InitLevelFrequency();
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
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	m_RandomPlacement.WriteToStream(pStream);
	m_EncounterRecord.WriteToStream(pStream);
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

void CStationType::PaintDockPortPositions (CG16bitImage &Dest, int x, int y)

//	PaintDockPortPositions
//
//	Paints the position of all the docking ports for this type.

	{
	int i;

	//	We need to initialize a docking ports structure

	CDockingPorts Ports;
	Ports.InitPortsFromXML(NULL, m_pDesc);

	//	Paint all ports

	for (i = 0; i < Ports.GetPortCount(NULL); i++)
		{
		int iRotation;
		bool bInFront;
		CVector vPos = Ports.GetPortPos(NULL, i, NULL, &bInFront, &iRotation);

		//	Colors

		WORD wArrowColor = (bInFront ? CG16bitImage::RGBValue(0x00, 0x40, 0x80) : CG16bitImage::RGBValue(0x80, 0x40, 0x00));
		WORD wCenterColor = (bInFront ? CG16bitImage::RGBValue(0x00, 0x7f, 0xff) : CG16bitImage::RGBValue(0xff, 0x7f, 0x00));

		//	Get the position

		int xPos = x + (int)(vPos.GetX() / g_KlicksPerPixel);
		int yPos = y - (int)(vPos.GetY() / g_KlicksPerPixel);

		//	Paint arrow

		CPaintHelper::PaintArrow(Dest, xPos, yPos, iRotation, wArrowColor);

		//	Paint center crosshairs

		Dest.DrawDot(xPos, yPos, wCenterColor, CG16bitImage::markerMediumCross);
		}
	}

ScaleTypes CStationType::ParseScale (const CString sValue)

//	ParseScale
//
//	Parses a scale value

	{
	if (strEquals(sValue, STAR_SCALE))
		return scaleStar;
	else if (strEquals(sValue, WORLD_SCALE))
		return scaleWorld;
	else if (strEquals(sValue, STRUCTURE_SCALE))
		return scaleStructure;
	else if (strEquals(sValue, SHIP_SCALE))
		return scaleShip;
	else if (strEquals(sValue, FLOTSAM_SCALE))
		return scaleFlotsam;
	else
		return scaleNone;
	}

CStationType::ESizeClass CStationType::ParseSizeClass (const CString sValue)

//	ParseSizeClass
//
//	Parses size class values

	{
	char *pPos = sValue.GetASCIIZPointer();

	if (*pPos >= 'a' && *pPos <= 'm')
		return (ESizeClass)(*pPos - 'a' + worldSizeA);
	else if (*pPos >= 'A' && *pPos <= 'M')
		return (ESizeClass)(*pPos - 'A' + worldSizeA);
	else
		return sizeNone;
	}

void CStationType::Reinit (void)

//	Reinit
//
//	Reinitialize global data

	{
	}

void CStationType::SetImageSelector (SSelectorInitCtx &InitCtx, CCompositeImageSelector *retSelector)

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
		m_Image.InitSelector(InitCtx, retSelector);

#if 0
		int iVariantCount = GetImageVariants();
		if (iVariantCount)
			retSelector->AddVariant(mathRandom(0, iVariantCount-1));
		else
			retSelector->AddVariant(0);
#endif
		}
	}
