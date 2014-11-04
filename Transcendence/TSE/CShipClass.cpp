//	CShipClass.cpp
//
//	CShipClass class

#include "PreComp.h"

#define ARMOR_DISPLAY_TAG						CONSTLIT("ArmorDisplay")
#define COMMUNICATIONS_TAG						CONSTLIT("Communications")
#define DEVICES_TAG								CONSTLIT("Devices")
#define DOCK_SCREENS_TAG						CONSTLIT("DockScreens")
#define DRIVE_IMAGES_TAG						CONSTLIT("DriveImages")
#define EFFECTS_TAG								CONSTLIT("Effects")
#define EQUIPMENT_TAG							CONSTLIT("Equipment")
#define ESCORTS_TAG								CONSTLIT("Escorts")
#define EVENTS_TAG								CONSTLIT("Events")
#define FUEL_LEVEL_IMAGE_TAG					CONSTLIT("FuelLevelImage")
#define FUEL_LEVEL_TEXT_TAG						CONSTLIT("FuelLevelText")
#define FUEL_LOW_LEVEL_IMAGE_TAG				CONSTLIT("FuelLowLevelImage")
#define IMAGE_TAG								CONSTLIT("Image")
#define INSTALL_TAG								CONSTLIT("Install")
#define INTERIOR_TAG							CONSTLIT("Interior")
#define ITEMS_TAG								CONSTLIT("Items")
#define NAMES_TAG								CONSTLIT("Names")
#define NOZZLE_IMAGE_TAG						CONSTLIT("NozzleImage")
#define NOZZLE_POS_TAG							CONSTLIT("NozzlePos")
#define PLAYER_SETTINGS_TAG						CONSTLIT("PlayerSettings")
#define POWER_LEVEL_IMAGE_TAG					CONSTLIT("PowerLevelImage")
#define POWER_LEVEL_TEXT_TAG					CONSTLIT("PowerLevelText")
#define REACTOR_DISPLAY_TAG						CONSTLIT("ReactorDisplay")
#define REACTOR_TEXT_TAG						CONSTLIT("ReactorText")
#define REMOVE_TAG								CONSTLIT("Remove")
#define SHIELD_DISPLAY_TAG						CONSTLIT("ShieldDisplay")
#define TRADE_TAG								CONSTLIT("Trade")
#define WRECK_IMAGE_TAG							CONSTLIT("WreckImage")

#define AUTOPILOT_ATTRIB						CONSTLIT("autopilot")
#define ARMOR_ID_ATTRIB							CONSTLIT("armorID")
#define CARGO_SPACE_ATTRIB						CONSTLIT("cargoSpace")
#define CHARACTER_ATTRIB						CONSTLIT("character")
#define CHARACTER_CLASS_ATTRIB					CONSTLIT("characterClass")
#define COUNT_ATTRIB							CONSTLIT("count")
#define CYBER_DEFENSE_LEVEL_ATTRIB				CONSTLIT("cyberDefenseLevel")
#define DEBUG_ONLY_ATTRIB						CONSTLIT("debugOnly")
#define DEFAULT_BACKGROUND_ID_ATTRIB			CONSTLIT("defaultBackgroundID")
#define DESC_ATTRIB								CONSTLIT("desc")
#define DEST_X_ATTRIB							CONSTLIT("destX")
#define DEST_Y_ATTRIB							CONSTLIT("destY")
#define DOCK_SCREEN_ATTRIB						CONSTLIT("dockScreen")
#define EQUIPMENT_ATTRIB						CONSTLIT("equipment")
#define EXPLOSION_TYPE_ATTRIB					CONSTLIT("explosionType")
#define MAX_REACTOR_FUEL_ATTRIB					CONSTLIT("fuelCapacity")
#define HEIGHT_ATTRIB							CONSTLIT("height")
#define HP_X_ATTRIB								CONSTLIT("hpX")
#define HP_Y_ATTRIB								CONSTLIT("hpY")
#define INERTIALESS_DRIVE_ATTRIB				CONSTLIT("inertialessDrive")
#define LARGE_IMAGE_ATTRIB						CONSTLIT("largeImage")
#define LEAVES_WRECK_ATTRIB						CONSTLIT("leavesWreck")
#define LEVEL_ATTRIB							CONSTLIT("level")
#define MAX_ARMOR_ATTRIB						CONSTLIT("maxArmor")
#define MAX_CARGO_SPACE_ATTRIB					CONSTLIT("maxCargoSpace")
#define MAX_DEVICES_ATTRIB						CONSTLIT("maxDevices")
#define MAX_NON_WEAPONS_ATTRIB					CONSTLIT("maxNonWeapons")
#define MAX_REACTOR_POWER_ATTRIB				CONSTLIT("maxReactorPower")
#define MAX_STRUCTURAL_HIT_POINTS_ATTRIB		CONSTLIT("maxStructuralHitPoints")
#define MAX_WEAPONS_ATTRIB						CONSTLIT("maxWeapons")
#define NAME_ATTRIB								CONSTLIT("name")
#define NAME_BREAK_WIDTH						CONSTLIT("nameBreakWidth")
#define NAME_DEST_X_ATTRIB						CONSTLIT("nameDestX")
#define NAME_DEST_Y_ATTRIB						CONSTLIT("nameDestY")
#define NAME_Y_ATTRIB							CONSTLIT("nameY")
#define NON_CRITICAL_ATTRIB						CONSTLIT("nonCritical")
#define PERCEPTION_ATTRIB						CONSTLIT("perception")
#define PLAYER_SHIP_ATTRIB						CONSTLIT("playerShip")
#define RADIOACTIVE_WRECK_ATTRIB				CONSTLIT("radioactiveWreck")
#define REACTOR_POWER_ATTRIB					CONSTLIT("reactorPower")
#define REVERSE_ARTICLE_ATTRIB					CONSTLIT("reverseArticle")
#define ROTATION_COUNT_ATTRIB					CONSTLIT("rotationCount")
#define SCORE_ATTRIB							CONSTLIT("score")
#define SIZE_ATTRIB								CONSTLIT("size")
#define SHIP_SCREEN_ATTRIB						CONSTLIT("shipScreen")
#define START_AT_ATTRIB							CONSTLIT("startAt")
#define STARTING_CREDITS_ATTRIB					CONSTLIT("startingCredits")
#define STARTING_POS_ATTRIB						CONSTLIT("startingPos")
#define STARTING_SYSTEM_ATTRIB					CONSTLIT("startingSystem")
#define STRUCTURAL_HIT_POINTS_ATTRIB			CONSTLIT("structuralHitPoints")
#define THRUST_ATTRIB							CONSTLIT("thrust")
#define THRUST_RATIO_ATTRIB						CONSTLIT("thrustRatio")
#define TIME_STOP_IMMUNE_ATTRIB					CONSTLIT("timeStopImmune")
#define UNID_ATTRIB								CONSTLIT("UNID")
#define VIRTUAL_ATTRIB							CONSTLIT("virtual")
#define WIDTH_ATTRIB							CONSTLIT("width")
#define WRECK_TYPE_ATTRIB						CONSTLIT("wreckType")
#define X_ATTRIB								CONSTLIT("x")
#define Y_ATTRIB								CONSTLIT("y")

#define FIELD_ARMOR_COUNT						CONSTLIT("armorCount")				//	Number of armor segments
#define FIELD_ARMOR_HP							CONSTLIT("armorHP")					//	HP of average armor segment
#define FIELD_ARMOR_ITEMS						CONSTLIT("armorItems")
#define FIELD_BALANCE_TYPE						CONSTLIT("balanceType")
#define FIELD_CARGO_SPACE						CONSTLIT("cargoSpace")
#define FIELD_COMBAT_STRENGTH					CONSTLIT("combatStrength")			//	value reflecting combat power (attack and defense)
#define FIELD_DAMAGE_RATE						CONSTLIT("damage")					//	damage per 180 ticks
#define FIELD_DEFENSE_RATE						CONSTLIT("defenseStrength")			//	value reflecting difficulty killing
#define FIELD_DEVICE_SLOTS						CONSTLIT("deviceSlots")
#define FIELD_DEVICE_SLOTS_NON_WEAPONS			CONSTLIT("deviceSlotsNonWeapons")
#define FIELD_DEVICE_SLOTS_WEAPONS				CONSTLIT("deviceSlotsWeapons")
#define FIELD_DEVICE_ITEMS						CONSTLIT("deviceItems")
#define FIELD_DOCK_SERVICES_SCREEN				CONSTLIT("dockServicesScreen")
#define FIELD_DODGE_RATE						CONSTLIT("dodgeRate")
#define FIELD_DRIVE_IMAGE						CONSTLIT("driveImage")
#define FIELD_EXPLOSION_TYPE					CONSTLIT("explosionType")
#define FIELD_FIRE_ACCURACY						CONSTLIT("fireAccuracy")
#define FIELD_FIRE_RANGE_ADJ					CONSTLIT("fireRangeAdj")
#define FIELD_FIRE_RATE_ADJ						CONSTLIT("fireRateAdj")
#define FIELD_GENERIC_NAME						CONSTLIT("genericName")
#define FIELD_HP								CONSTLIT("hp")
#define FIELD_HULL_MASS							CONSTLIT("hullMass")
#define FIELD_INSTALL_DEVICE_MAX_LEVEL			CONSTLIT("installDeviceMaxLevel")
#define FIELD_LAUNCHER							CONSTLIT("launcher")
#define FIELD_LAUNCHER_UNID						CONSTLIT("launcherUNID")
#define FIELD_LEVEL								CONSTLIT("level")
#define FIELD_MANEUVER							CONSTLIT("maneuver")
#define FIELD_MANUFACTURER						CONSTLIT("manufacturer")
#define FIELD_MASS								CONSTLIT("mass")
#define FIELD_MAX_ARMOR_MASS					CONSTLIT("maxArmorMass")
#define FIELD_MAX_CARGO_SPACE					CONSTLIT("maxCargoSpace")
#define FIELD_MAX_ROTATION						CONSTLIT("maxRotation")
#define FIELD_MAX_SPEED							CONSTLIT("maxSpeed")
#define FIELD_NAME								CONSTLIT("name")
#define FIELD_PRIMARY_ARMOR						CONSTLIT("primaryArmor")
#define FIELD_PRIMARY_ARMOR_UNID				CONSTLIT("primaryArmorUNID")
#define FIELD_PRIMARY_WEAPON					CONSTLIT("primaryWeapon")
#define FIELD_PRIMARY_WEAPON_RANGE				CONSTLIT("primaryWeaponRange")
#define FIELD_PRIMARY_WEAPON_RANGE_ADJ			CONSTLIT("primaryWeaponRangeAdj")
#define FIELD_PRIMARY_WEAPON_UNID				CONSTLIT("primaryWeaponUNID")
#define FIELD_REGEN								CONSTLIT("regen")
#define FIELD_SCORE								CONSTLIT("score")
#define FIELD_SIZE								CONSTLIT("size")
#define FIELD_SHIELD							CONSTLIT("shield")
#define FIELD_SHIELD_UNID						CONSTLIT("shieldsUNID")
#define FIELD_SHIP_STATUS_SCREEN				CONSTLIT("shipStatusScreen")
#define FIELD_STARTING_SYSTEM					CONSTLIT("startingSystem")
#define FIELD_THRUST							CONSTLIT("thrust")
#define FIELD_THRUST_TO_WEIGHT					CONSTLIT("thrustToWeight")
#define FIELD_THRUSTER_POWER					CONSTLIT("thrusterPower")
#define FIELD_TREASURE_VALUE					CONSTLIT("treasureValue")
#define FIELD_WRECK_CHANCE						CONSTLIT("wreckChance")

#define ERR_OUT_OF_MEMORY						CONSTLIT("out of memory")
#define ERR_BAD_IMAGE							CONSTLIT("invalid ship image")
#define ERR_MISSING_ARMOR_TAG					CONSTLIT("missing <Armor> element")
#define ERR_MISSING_DEVICES_TAG					CONSTLIT("missing <Devices> element")
#define ERR_TOO_MANY_DEVICES					CONSTLIT("too many devices")
#define ERR_BAD_EXHAUST_IMAGE					CONSTLIT("invalid drive image")
#define ERR_DRIVE_IMAGE_FORMAT					CONSTLIT("invalid element in <DriveImages>")
#define ERR_DOCK_SCREEN_NEEDED					CONSTLIT("docking ports specified but no docking screen defined")
#define ERR_SHIELD_DISPLAY_NEEDED				CONSTLIT("missing or invalid <ShieldDisplay> element")
#define ERR_ARMOR_DISPLAY_NEEDED				CONSTLIT("missing or invalid <ArmorDisplay> element")
#define ERR_REACTOR_DISPLAY_NEEDED				CONSTLIT("missing or invalid <ReactorDisplay> element")
#define ERR_INVALID_STARTING_CREDITS			CONSTLIT("invalid starting credits")
#define ERR_UNKNOWN_EQUIPMENT					CONSTLIT("unknown equipment: %s")
#define ERR_UNKNOWN_EQUIPMENT_DIRECTIVE			CONSTLIT("unknown equipment directive: %s")

#define SPECIAL_IS_PLAYER_CLASS					CONSTLIT("isPlayerClass:")
#define SPECIAL_MANUFACTURER					CONSTLIT("manufacturer:")

#define SPECIAL_VALUE_TRUE						CONSTLIT("true")

#define WRECK_IMAGE_VARIANTS					3
#define DAMAGE_IMAGE_COUNT						10
#define DAMAGE_IMAGE_WIDTH						24
#define DAMAGE_IMAGE_HEIGHT						24

#define DEFAULT_POWER_USE						20

static char g_ArmorTag[] = "Armor";
static char g_AISettingsTag[] = "AISettings";

static char g_ManufacturerAttrib[] = "manufacturer";
static char g_ClassAttrib[] = "class";
static char g_TypeAttrib[] = "type";
static char g_MassAttrib[] = "mass";
static char g_ManeuverAttrib[] = "maneuver";
static char g_MaxSpeedAttrib[] = "maxSpeed";
static char g_StartAttrib[] = "start";
static char g_SpanAttrib[] = "span";
static char g_DeviceIDAttrib[] = "deviceID";

static char g_FireRateAttrib[] = "fireRate";

static CG16bitImage *g_pDamageBitmap = NULL;
static CStationType *g_pWreckDesc = NULL;

const int DOCK_OFFSET_STD_SIZE =				64;

DWORD ParseNonCritical (const CString &sList);

struct ScoreDesc
	{
	int iBaseXP;
	int iSpecialXP;
	int iExceptionalXP;
	int iDrawbackXP;
	int iScoreLevel;
	};

static ScoreDesc g_XP[] =
	{
		//	Base Score
		//			Special Ability
		//					Exceptional Ability
		//							Drawback
		//									Level Score

		{	30,		3,		20,		-2,		50		},		//	I
		{	55,		5,		40,		-3,		100		},		//	II
		{	110,	10,		75,		-6,		200		},		//	III
		{	200,	20,		130,	-10,	350		},		//	IV
		{	340,	30,		230,	-20,	600		},		//	V
		{	510,	45,		340,	-25,	900		},		//	VI
		{	800,	70,		530,	-40,	1400	},		//	VII
		{	1100,	95,		720,	-55,	1900	},		//	VIII
		{	1500,	130,	990,	-80,	2600	},		//	IX
		{	1850,	160,	1250,	-100,	3250	},		//	X
		{	2400,	210,	1600,	-130,	4200	},		//	XI
		{	3150,	280,	2100,	-170,	5500	},		//	XII
		{	3850,	340,	2550,	-200,	6750	},		//	XIII
		{	4700,	410,	3150,	-250,	8250	},		//	XIV
		{	5700,	500,	3800,	-300,	10000	},		//	XV
		{	6550,	580,	4350,	-350,	11500	},		//	XVI
		{	7550,	660,	5050,	-400,	13250	},		//	XVII
		{	8550,	750,	5700,	-450,	15000	},		//	XVIII
		{	9550,	840,	6350,	-500,	16750	},		//	XIX
		{	10500,	930,	7050,	-560,	18500	},		//	XX
		{	11500,	1050,	7800,	-620,	20500	},		//	XXI
		{	13000,	1150,	8550,	-680,	22500	},		//	XXII
		{	14500,	1250,	9500,	-750,	25000	},		//	XXIII
		{	15000,	1350,	10000,	-800,	26500	},		//	XXIV
		{	17000,	1500,	11500,	-900,	30000	},		//	XXV
	};

#define SCORE_DESC_COUNT							(sizeof(g_XP) / sizeof(g_XP[0]))

CShipClass::CShipClass (void) : 
		m_pDevices(NULL),
		m_pPlayerSettings(NULL),
		m_pItems(NULL),
		m_pEscorts(NULL),
		m_pTrade(NULL),
		m_fInheritedDevices(false),
		m_fInheritedItems(false),
		m_fInheritedEscorts(false),
		m_fInheritedTrade(false)

//	CShipClass constructor

	{
	}

CShipClass::~CShipClass (void)

//	CShip destructor

	{
	if (m_pDevices && !m_fInheritedDevices)
		delete m_pDevices;

	if (m_pPlayerSettings && !m_fInheritedPlayerSettings)
		delete m_pPlayerSettings;

	if (m_pItems && !m_fInheritedItems)
		delete m_pItems;

	if (m_pEscorts && !m_fInheritedEscorts)
		delete m_pEscorts;

	if (m_pTrade && !m_fInheritedTrade)
		delete m_pTrade;
	}

CShipClass::EBalanceTypes CShipClass::CalcBalanceType (CString *retsDesc) const

//	CalcBalanceType
//
//	Calculates the balance type for this ship (based on level and combat strength.

	{
	Metric rCombat = CalcCombatStrength();
	Metric rAttack = CalcDamageRate();
	Metric rDefense = CalcDefenseRate();
	Metric rAttackDefenseRatio = (rDefense > 0.0 ? rAttack / rDefense : 1000.0);

	int iLevel = GetLevel();

	//	Calculate the standard combat strength for ships of this level.

	const Metric k0 = 1.4;
	const Metric k1 = 8.0;
	Metric rStdCombat = k1 * pow(k0, iLevel - 1);

	//	If we're less than 1/4 standard then we're too weak to be a minion

	if (rCombat < 0.25 * rStdCombat
			&& iLevel > 1)
		{
		if (retsDesc) *retsDesc = CONSTLIT("too weak");
		return typeTooWeak;
		}

	//	If we're more than 4 times standard then we're too strong even for a boss

	else if (rCombat >= 4.0 * rStdCombat)
		{
		if (retsDesc) *retsDesc = CONSTLIT("too strong");
		return typeTooStrong;
		}

	//	See if this is a non-combatant

	else if (rAttackDefenseRatio < 0.5 && rAttack < 0.5 * rStdCombat)
		{
		if (retsDesc) *retsDesc = CONSTLIT("non-combatant");
		return typeNonCombatant;
		}

	//	If we're less than 3/4 standard and our defense is low, then we're a minion

	else if (rCombat < 0.75 * rStdCombat
			&& rAttackDefenseRatio > 0.5)
		{
		if (retsDesc) *retsDesc = CONSTLIT("minion");
		return typeMinion;
		}

	//	If we're less than 1.5 standard then we're a standard ship.

	else if (rCombat < 1.5 * rStdCombat)
		{
		if (retsDesc) *retsDesc = CONSTLIT("standard");
		return typeStandard;
		}

	//	If we're less than 2.5 standard then we're elite

	else if (rCombat < 2.5 * rStdCombat)
		{
		if (retsDesc) *retsDesc = CONSTLIT("elite");
		return typeElite;
		}

	//	If we're less than 4 times standard then we're a boss

	else
		{
		if (retsDesc) *retsDesc = CONSTLIT("boss");
		return typeBoss;
		}
	}

CItemType *CShipClass::CalcBestMissile (const SDeviceDesc &Device) const

//	CalcBestMissile
//
//	Computes the best missile item type available to the given device (which 
//	must be a launcher).

	{
	CItemType *pType = Device.Item.GetType();
	CDeviceClass *pDevice = (pType ? pType->GetDeviceClass() : NULL);
	if (pDevice == NULL)
		return NULL;

	//	Look for the highest missile level and use that as the
	//	weapon level.

	CItemType *pBestMissile = NULL;
	IItemGenerator *pItems = GetRandomItemTable();
	if (pItems)
		FindBestMissile(pDevice, pItems, &pBestMissile);

	FindBestMissile(pDevice, Device.ExtraItems, &pBestMissile);

	return pBestMissile;
	}

Metric CShipClass::CalcCombatStrength (void) const

//	CalcCombatStrength
//
//	Computes a rough value for combat strength of the ship.

	{
	//	Get the attack strength (and the best weapon level)

	int iWeaponLevel;
	Metric rAttack = CalcDamageRate(NULL, &iWeaponLevel);

	//	If we're using weapons for a damage type weaker than expected for our
	//	current level, then we adjust.

	int iExpectedDamageTier = (GetLevel() + 2) / 3;
	int iWeaponDamageTier = (iWeaponLevel + 2) / 3;
	switch (iExpectedDamageTier - iWeaponDamageTier)
		{
		case 1:
			rAttack *= 0.85;
			break;

		case 2:
			rAttack *= 0.5;
			break;

		case 3:
		case 4:
		case 5:
		case 6:
			rAttack *= 0.33;
			break;
		}

	//	Get the defense strength

	Metric rDefense = CalcDefenseRate();

	//	Combine attack and defense strengths

	const Metric ATTACK_FACTOR = 0.5;
	const Metric DEFENSE_FACTOR = 0.25;
	const Metric MAX_ATTACK_RATIO = 5.0;

	return (ATTACK_FACTOR * Min(rAttack, rDefense * MAX_ATTACK_RATIO))
		+ (DEFENSE_FACTOR * rDefense);
	}

Metric CShipClass::CalcDamageRate (int *retiAveWeaponLevel, int *retiMaxWeaponLevel) const

//	CalcDamageRate
//
//	Computes the rough number of HP of damage dealt by this ship in 180 ticks.
//	This is based on weapons and maneuverability.
//
//	NOTE: Damage rate is NOT adjusted for damage type because we want this to
//	follow the current weapon damage HP table (which also does not adjust for
//	damage type). Callers should adjust if necessary.

	{
	int i;

	const Metric MANEUVER_FACTOR = 0.5;

	Metric rTotalDamageRate = 0.0;
	Metric rTotalDamageLevels = 0.0;
	Metric rPrimaryDamageRate = 0.0;
	int iPrimaryLevel = 0;
	int iMaxWeaponLevel = 0;

	//	Loop over all weapons and add up their damage.

	for (i = 0; i < m_AverageDevices.GetCount(); i++)
		{
		const SDeviceDesc &Desc = m_AverageDevices.GetDeviceDesc(i);
		CItemType *pItem = Desc.Item.GetType();
		CDeviceClass *pDevice = (pItem ? pItem->GetDeviceClass() : NULL);
		bool bIsLauncher;

		//	Figure out if this is a weapon or a launcher; if neither, then we
		//	skip this device.

		if (pDevice == NULL)
			continue;
		else if (pDevice->GetCategory() == itemcatLauncher)
			bIsLauncher = true;
		else if (pDevice->GetCategory() == itemcatWeapon)
			bIsLauncher = false;
		else
			continue;

		CWeaponClass *pWeapon = pDevice->AsWeaponClass();
		if (pWeapon == NULL)
			continue;

		//	If this is a launcher then we need to figure out the best available
		//	variant.

		int iVariant = 0;
		int iMissileLevel = 0;
		if (bIsLauncher)
			{
			CItemType *pBestMissile = CalcBestMissile(Desc);
			if (pBestMissile)
				{
				iVariant = pDevice->GetAmmoVariant(pBestMissile);
				iMissileLevel = pBestMissile->GetLevel();
				}
			else
				iVariant = -1;
			}

		//	If no valid variant, then skip

		if (iVariant == -1)
			continue;

		//	Compute the damage rate for the weapon

		Metric rDamageRate = (Metric)pDevice->GetDataFieldInteger(strPatternSubst(CONSTLIT("damage:%d"), iVariant));

		//	Compute weapon level

		int iWeaponLevel = (bIsLauncher ? iMissileLevel : pDevice->GetLevel());

		//	Compute the total coverage of the weapon (360 = omnidirectional; 0 = fixed)

		int iDeviceCoverage;
		if (Desc.bOmnidirectional)
			iDeviceCoverage = 360;
		else if (Desc.iMaxFireArc != Desc.iMinFireArc)
			iDeviceCoverage = ::AngleRange(Desc.iMinFireArc, Desc.iMaxFireArc);
		else
			iDeviceCoverage = 0;

		int iMinFireArc;
		int iMaxFireArc;
		int iWeaponCoverage;
		if (!pWeapon->CanRotate(CItemCtx(), &iMinFireArc, &iMaxFireArc))
			iWeaponCoverage = 0;
		else if (iMinFireArc != iMaxFireArc)
			iWeaponCoverage = ::AngleRange(iMinFireArc, iMaxFireArc);
		else
			iWeaponCoverage = 360;

		int iCoverage = Max(iDeviceCoverage, iWeaponCoverage);

		//	For area weapons we adjust damage rate based on coverage.

		if (iCoverage >= 90)
			rDamageRate *= ((Metric)iCoverage / 360.0);

		//	Otherwise, for fixed weapons we need to adjust based on the 
		//	maneuverability of the ship.

		else
			{
			int iManeuverDelay = Max(1, Min(GetManeuverDelay(), 4));
			Metric rManeuver = 1.0 / iManeuverDelay;

			rDamageRate *= rManeuver 
					* MANEUVER_FACTOR
					* (1.0 + (3.0 * iCoverage / 90.0));
			}

		//	Adjust by AI fire rate

		if (m_AISettings.GetFireRateAdj() > 0)
			rDamageRate *= 10.0 / m_AISettings.GetFireRateAdj();

		//	If this is a primary weapon, then keep track of the best rate.
		//	Otherwise, we just add to the total.

		if (bIsLauncher || Desc.bSecondary)
			{
			rTotalDamageRate += rDamageRate;
			rTotalDamageLevels += (rDamageRate * iWeaponLevel);
			}
		else
			{
			if (rDamageRate > rPrimaryDamageRate)
				{
				rPrimaryDamageRate = rDamageRate;
				iPrimaryLevel = iWeaponLevel;
				}
			}

		//	Keep track of max weapon level

		if (iWeaponLevel > iMaxWeaponLevel)
			iMaxWeaponLevel = iWeaponLevel;
		}

	//	Add up primary weapon

	rTotalDamageRate += rPrimaryDamageRate;
	rTotalDamageLevels += (rPrimaryDamageRate * iPrimaryLevel);

	//	Compute average level, if necessary

	if (retiAveWeaponLevel)
		*retiAveWeaponLevel = (rTotalDamageRate > 0.0 ? (int)((rTotalDamageLevels / rTotalDamageRate) + 0.5) : 0);

	if (retiMaxWeaponLevel)
		*retiMaxWeaponLevel = iMaxWeaponLevel;

	//	Done

	return rTotalDamageRate;
	}

Metric CShipClass::CalcDefenseRate (void) const

//	CalcDefenseRate
//
//	Calculates the smallest amount of damage per 180 ticks required to destroy
//	the ship in less than 6 * 180 ticks.

	{
	const Metric T = 6.0;	//	six 180 tick cycles

	//	Compute armor strength values

	Metric Ahp;
	Metric Aregen;
	CArmorClass *pArmor;
	if (m_Hull.GetCount() > 0)
		pArmor = m_Hull[0].pArmor;
	else
		pArmor = NULL;

	if (pArmor)
		{
		Ahp = pArmor->GetMaxHP(CItemCtx());
		Aregen = pArmor->GetItemType()->GetDataFieldInteger(FIELD_REGEN);

		//	Adjust for damage type adjustment (for armor that has more resistance
		//	than standard armor, e.g., meteorsteel).

		int iDamageAdj = pArmor->CalcAverageRelativeDamageAdj();
		Metric rDamageAdj = (iDamageAdj > 0 ? 100.0 / iDamageAdj : 10.0);
		Ahp *= rDamageAdj;
		Aregen *= rDamageAdj;

		//	Adjust for number of segments

		Metric rAdj;
		switch (m_Hull.GetCount())
			{
			case 1:
				rAdj = 0.5;
				break;

			case 2:
			case 3:
				rAdj = 0.67;
				break;

			case 4:
			case 5:
			case 6:
			case 7:
				rAdj = 0.9;
				break;

			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
				rAdj = 1.0;
				break;

			default:
				rAdj = Max(1.0, m_Hull.GetCount() / 12.0);
			}

		Ahp *= rAdj;
		Aregen *= rAdj;
		}
	else
		{
		Ahp = 0.0;
		Aregen = 0.0;
		}

	//	If we have interior structure, we add it to the armor hit points.
	//	NOTE: We don't try to this as a third term in the equation below because
	//	it is too hard to solve the equation. [And because it is rare for
	//	either armor or interior to have high regeneration.]

	if (!m_Interior.IsEmpty())
		{
		Metric Ihp = m_Interior.GetHitPoints();
		if (Ihp > 0.0)
			{
			//	Adjust because we need WMD damage

			Ihp *= 2.0;

			//	Adjust regen proportionally.

			Metric rTotal = Ahp + Ihp;
			Aregen = (Ahp * Aregen) / rTotal;

			//	Adjust armor hp

			Ahp += Ihp;
			}
		}

	//	Compute shield strength values

	Metric Shp;
	Metric Sregen;
	CDeviceClass *pDevice = m_AverageDevices.GetNamedDevice(devShields);
	if (pDevice)
		{
		CItemType *pShieldType = pDevice->GetItemType();
		Shp = pShieldType->GetDataFieldInteger(FIELD_HP);
		Sregen = pShieldType->GetDataFieldInteger(FIELD_REGEN);
		}
	else
		{
		Shp = 0.0;
		Sregen = 0.0;
		}

	//		    Shp			   Ahp
	//	T = ------------ + ------------
	//		 X - Sregen		X - Aregen
	//
	//	Solve for X (both roots)

	Metric i1 = pow(-Ahp - Aregen * T - Shp - Sregen * T, 2) - 4.0 * T * (Ahp * Sregen + Aregen * Shp + Aregen * Sregen * T);
	if (i1 < 0.0)
		return 0.0;

	Metric i2 = Ahp + Aregen * T + Shp + Sregen * T;

	Metric x1 = -(sqrt(i1) + i2) / (2 * T);
	Metric x2 = (sqrt(i1) + i2) / (2 * T);

	//	Take whichever is positive

	Metric rRate;
	if (x1 > 0.0)
		rRate = x1;
	else if (x2 > 0.0)
		rRate = x2;
	else
		return 0.0;

	//	Adjust rate based on maneuverability of ship

	Metric rHitRate = rRate * (1.0 - CalcDodgeRate());
	rRate = rRate * rRate / rHitRate;

	//	Done

	return rRate;
	}

int CShipClass::CalcLevel (void) const

//	CalcLevel
//
//	Calculate the ship level based on devices, etc.

	{
	int i;

	int iBestLevel = 1;

	//	Figure out what armor we've got

	int iArmorSections = GetHullSectionCount();
	int iArmorLevel = (iArmorSections > 0 ? GetHullSection(0)->pArmor->GetItemType()->GetLevel() : 1);
	if (iArmorLevel > iBestLevel)
		iBestLevel = iArmorLevel;

	//	Loop over all devices and figure out the highest level.

	for (i = 0; i < m_AverageDevices.GetCount(); i++)
		{
		const SDeviceDesc &Desc = m_AverageDevices.GetDeviceDesc(i);
		CDeviceClass *pDevice = Desc.Item.GetType()->GetDeviceClass();

		//	Compute the device level

		int iDeviceLevel = ComputeDeviceLevel(Desc);

		//	Keep track of the best device level

		if (iDeviceLevel > iBestLevel)
			iBestLevel = iDeviceLevel;
		}

	//	Done

	return iBestLevel;
	}

Metric CShipClass::CalcManeuverValue (bool bDodge) const

//	CalcManeuverValue
//
//	Computes the rough probability (0.0-1.0) that the ship can dodge an enemy shot,
//	based on maneuverability, speed, and thrust.

	{
	const Metric PIVOT_SPEED = 12.0;
	const Metric PIVOT_DODGE = 0.5;
	const Metric K1 = 0.5;
	const Metric K2 = 1.0 / g_Pi;
	const Metric K3 = 1.25 / g_Pi;

	const Metric PIVOT_THRUST = 12.0;
	const Metric MAX_THRUST_ADJ = 8.0;

	const int SIZE_FACTOR = 16;

	//	Get some statistics

	DriveDesc Drive;
	GetDriveDesc(&Drive);

	Metric rMass = CalcMass(m_AverageDevices);
	Metric rThrustRatio = 2.0 * (rMass > 0.0 ? Drive.iThrust / rMass : 0.0);
	int iManeuverDelay = Max(1, GetManeuverDelay());
	Metric rSpeed = 100.0 * Drive.rMaxSpeed / LIGHT_SPEED;

	//	Speed of 12 is a pivot point. Above this speed it gets easy to dodge
	//	shots; below this speed it gets increasingly harder.

	Metric rDodge;
	if (rSpeed >= PIVOT_SPEED)
		rDodge = PIVOT_DODGE + (K2 * atan(K1 * (rSpeed - PIVOT_SPEED)));
	else
		rDodge = Max(0.0, PIVOT_DODGE + (K3 * atan(K1 * (rSpeed - PIVOT_SPEED))));

	//	Generate an adjustment based on maneuverability:
	//	1 = normal dodge rate; 10 = very low dodge rate

	int iManeuverAdj = iManeuverDelay;

	//	Generate an adjustment based on thrust-ratio:
	//	1 = normal dodge rate; 10 = very low dodge rate

	int iThrustAdj = (rThrustRatio < PIVOT_THRUST ? (int)(1.0 + (MAX_THRUST_ADJ * (PIVOT_THRUST - rThrustRatio) / PIVOT_THRUST)) : 1);

	//	Generate an adjustment based on ship size:
	//	1 = normal dodge rate; 10 = very low dodge rate

	int cxWidth = RectWidth(GetImage().GetImageRect());
	int iSizeAdj = (bDodge ? Max(1, cxWidth / SIZE_FACTOR) : 1);

	//	Adjust dodge rate

	rDodge = pow(rDodge, Max(Max(iManeuverAdj, iThrustAdj), iSizeAdj));

	//	Done

	return rDodge;
	}

Metric CShipClass::CalcMass (const CDeviceDescList &Devices) const

//	CalcMass
//
//	Returns the total mass of the ship class, including devices and armor
//	(in tons).

	{
	int i;
	Metric rMass = GetHullMass();

	for (i = 0; i < GetHullSectionCount(); i++)
		{
		CItem Item(GetHullSection(i)->pArmor->GetItemType(), 1);
		rMass += Item.GetMass();
		}

	for (i = 0; i < Devices.GetCount(); i++)
		{
		CItem Item(Devices.GetDeviceClass(i)->GetItemType(), 1);
		rMass += Item.GetMass();
		}

	return rMass;
	}

int CShipClass::CalcScore (void)

//	CalcScore
//
//	Computes the score of the ships class

	{
	//	Compute the movement stats

	int iSpeed, iThrust, iManeuver;
	ComputeMovementStats(m_AverageDevices, &iSpeed, &iThrust, &iManeuver);

	//	Figure out what armor we've got

	int iArmorSections = GetHullSectionCount();
	int iArmorLevel = (iArmorSections > 0 ? GetHullSection(0)->pArmor->GetItemType()->GetLevel() : 1);

	//	Figure out what devices we've got

	int iWeaponLevel = 0;
	int iWeapon = -1;
	int iLauncherLevel = 0;
	int iLauncher = -1;
	int iShieldLevel = 0;

	for (int j = 0; j < m_AverageDevices.GetCount(); j++)
		{
		CDeviceClass *pDevice = m_AverageDevices.GetDeviceDesc(j).Item.GetType()->GetDeviceClass();
		switch (pDevice->GetCategory())
			{
			case itemcatWeapon:
				{
				if (pDevice->GetLevel() > iWeaponLevel)
					{
					iWeaponLevel = pDevice->GetLevel();
					iWeapon = j;
					}
				break;
				}

			case itemcatLauncher:
				{
				iLauncherLevel = ComputeDeviceLevel(m_AverageDevices.GetDeviceDesc(j));
				iLauncher = j;
				break;
				}

			case itemcatShields:
				{
				iShieldLevel = pDevice->GetLevel();
				break;
				}
			}
		}

	int iPrimaryWeapon;
	if (iLauncherLevel > iWeaponLevel)
		iPrimaryWeapon = iLauncher;
	else
		iPrimaryWeapon = iWeapon;

	//	Compute score and level

	return ComputeScore(m_AverageDevices,
			iArmorLevel, 
			iPrimaryWeapon, 
			iSpeed, 
			iThrust, 
			iManeuver,
			iLauncherLevel > iWeaponLevel);
	}

int CShipClass::ComputeDeviceLevel (const SDeviceDesc &Device) const

//	ComputeDeviceLevel
//
//	Computes the level of the given device

	{
	CItemType *pType = Device.Item.GetType();
	CDeviceClass *pDevice = (pType ? pType->GetDeviceClass() : NULL);
	if (pDevice == NULL)
		return 0;

	if (pDevice->GetCategory() == itemcatLauncher)
		{
		//	Look for the highest missile level and use that as the
		//	weapon level.

		CItemType *pBestMissile = CalcBestMissile(Device);
		return (pBestMissile ? pBestMissile->GetLevel() : 0);
		}
	else
		return pDevice->GetLevel();
	}

enum LowMediumHigh
	{
	enumLow = 0,
	enumMedium = 1,
	enumHigh = 2,
	};

void CShipClass::ComputeMovementStats (CDeviceDescList &Devices, int *retiSpeed, int *retiThrust, int *retiManeuver)
	{
	const DriveDesc *pDrive = GetHullDriveDesc();

	//	Figure out the speed of the ship

	if (pDrive->rMaxSpeed > 0.20 * LIGHT_SPEED)
		*retiSpeed = enumHigh;
	else if (pDrive->rMaxSpeed > 0.15 * LIGHT_SPEED)
		*retiSpeed = enumMedium;
	else
		*retiSpeed = enumLow;

	//	Figure out the mass of the ship (including all installed
	//	weapons and armor)

	Metric rFullMass = CalcMass(Devices);

	//	Figure out the thrust of the ship

	Metric rRatio = (Metric)pDrive->iThrust / rFullMass;
	if (rRatio >= 7.0)
		*retiThrust = enumHigh;
	else if (rRatio >= 3.0)
		*retiThrust = enumMedium;
	else
		*retiThrust = enumLow;

	//	Figure out the maneuverability of the ship

	if (m_RotationDesc.GetMaxRotationTimeTicks() >= 90)
		*retiManeuver = enumLow;
	else if (m_RotationDesc.GetMaxRotationTimeTicks() > 30)
		*retiManeuver = enumMedium;
	else
		*retiManeuver = enumHigh;
	}

int CShipClass::ComputeScore (const CDeviceDescList &Devices,
							  int iArmorLevel,
							  int iPrimaryWeapon,
							  int iSpeed,
							  int iThrust,
							  int iManeuver,
							  bool bPrimaryIsLauncher)

//	ComputeScore
//
//	Compute the score of the class based on equipment

	{
	int i;
	int iSpecial = 0;
	int iExceptional = 0;
	int iDrawback = 0;
	int iStdLevel = iArmorLevel;
	int iWeaponLevel = (iPrimaryWeapon == -1 ? 0 : ComputeDeviceLevel(Devices.GetDeviceDesc(iPrimaryWeapon)));

	//	If our weapon is better than our armor then adjust the level
	//	depending on the difference.

	if (iWeaponLevel > iArmorLevel)
		{
		switch (iWeaponLevel - iArmorLevel)
			{
			case 1:
				iStdLevel = iWeaponLevel;
				iDrawback++;
				break;

			case 3:
				iStdLevel = iWeaponLevel - 2;
				iSpecial += 2;
				break;

			default:
				iStdLevel = (iWeaponLevel + iArmorLevel) / 2;
			}
		}

	//	If our best weapon is 2 or more levels below our standard
	//	level then take drawbacks exponentially.

	if (iStdLevel > iWeaponLevel + 1)
		iDrawback += min(16, (1 << (iStdLevel - (iWeaponLevel + 2))));
	else if (iStdLevel > iWeaponLevel)
		iDrawback++;

	//	If all movement stats are high then this counts as an
	//	exceptional ability

	if (iSpeed == enumHigh && iThrust == enumHigh && iManeuver == enumHigh)
		iExceptional++;

	//	Otherwise, treat them as special abilities or drawbacks

	else
		{
		if (iSpeed == enumLow)
			iDrawback++;
		else if (iSpeed == enumHigh)
			iSpecial++;

		if (iThrust == enumLow)
			iDrawback++;
		else if (iThrust == enumHigh)
			iSpecial++;

		if (iManeuver == enumLow)
			iDrawback++;
		else if (iManeuver == enumHigh)
			iSpecial++;
		}

	//	1 armor segment is a drawback

	int iArmorSections = GetHullSectionCount();
	if (iArmorSections <= 1)
		iDrawback++;

	//	2-3 armor segments is normal

	else if (iArmorSections < 4)
		;

	//	4 or more armor segments is special

	else if (iArmorSections < 8 )
		iSpecial++;
	else if (iArmorSections < 16)
		iSpecial += 2;
	else if (iArmorSections < 32)
		iSpecial += 3;
	else if (iArmorSections < 64)
		iSpecial += 4;
	else
		iSpecial += 5;

	//	Checkout all the devices

	bool bDirectionalBonus = false;
	bool bGoodSecondary = false;
	int iDirectionalBonus = 0;
	for (i = 0; i < Devices.GetCount(); i++)
		{
		const SDeviceDesc &Dev = Devices.GetDeviceDesc(i);
		CDeviceClass *pDevice = Dev.Item.GetType()->GetDeviceClass();
		int iDeviceLevel = ComputeDeviceLevel(Dev);

		//	Specific devices

		switch (pDevice->GetCategory())
			{
			case itemcatWeapon:
			case itemcatLauncher:
				{
				int iWeaponAdj = (iDeviceLevel - iStdLevel);

				//	If this is a secondary weapon, then add it to the score

				if (i != iPrimaryWeapon)
					{
					//	Calculate any potential bonus based on the weapon level
					//	compared to the base level

					iSpecial += max(iWeaponAdj + 3, 0);
					}

				//	Compute fire arc

				int iFireArc = (Dev.bOmnidirectional ? 360 : AngleRange(Dev.iMinFireArc, Dev.iMaxFireArc));

				//	Adjust for turret-mount

				iDirectionalBonus += (max(iWeaponAdj + 3, 0) * iFireArc);
				break;
				}

			case itemcatReactor:
			case itemcatDrive:
			case itemcatCargoHold:
				//	Reactors don't count as improvements
				break;

			default:
				{
				//	Other devices are special abilities depending on level

				if (iDeviceLevel > iStdLevel+1)
					iExceptional++;
				else if (iDeviceLevel > iStdLevel)
					iSpecial += 4;
				else if (iDeviceLevel >= iStdLevel-1)
					iSpecial += 2;
				else
					iSpecial++;
				}
			}
		}

	//	If we have no weapons then we have some drawbacks

	if (iPrimaryWeapon == -1)
		iDrawback += 3;

	//	Add bonus if weapon is omnidirectional

	iSpecial += (int)((iDirectionalBonus / 270.0) + 0.5);

	//	Checkout AI settings

	const CAISettings &AI = GetAISettings();
	int iFireAccuracy = AI.GetFireAccuracy();
	int iFireRateAdj = AI.GetFireRateAdj();

	int iFireAccuracyScore, iFireRateScore;

	if (iFireAccuracy > 97)
		iFireAccuracyScore = 5;
	else if (iFireAccuracy >= 93)
		iFireAccuracyScore = 4;
	else if (iFireAccuracy >= 90)
		iFireAccuracyScore = 3;
	else if (iFireAccuracy < 75)
		iFireAccuracyScore = 1;
	else
		iFireAccuracyScore = 2;

	if (iFireRateAdj <= 10)
		iFireRateScore = 5;
	else if (iFireRateAdj <= 20)
		iFireRateScore = 4;
	else if (iFireRateAdj <= 30)
		iFireRateScore = 3;
	else if (iFireRateAdj >= 60)
		iFireRateScore = 1;
	else
		iFireRateScore = 2;

	int iFireControlScore = iFireRateScore * iFireAccuracyScore;
	if (iFireControlScore >= 20)
		iExceptional++;
	else if (iFireControlScore > 6)
		iSpecial += ((iFireControlScore - 5) / 2);
	else if (iFireControlScore < 2)
		iDrawback += 4;
	else if (iFireControlScore < 4)
		iDrawback += 2;

	//	Compute final score

	ScoreDesc *pBase = &g_XP[iStdLevel-1];
	int iScore = pBase->iBaseXP
			+ iSpecial * pBase->iSpecialXP
			+ iExceptional * pBase->iExceptionalXP
			+ iDrawback * pBase->iDrawbackXP;

	return iScore;
	}

void CShipClass::CreateEmptyWreck (CSystem *pSystem, 
								   CShip *pShip,
								   const CVector &vPos, 
								   const CVector &vVel,
								   CSovereign *pSovereign, 
								   CStation **retpWreck)

//	CreateEmptyWreck
//
//	Create an empty wreck of the given ship class

	{
	SObjCreateCtx CreateCtx;
	CreateCtx.vPos = vPos;
	CreateCtx.vVel = vVel;

	//	Create the wreck

	CStation *pWreck;
	CStation::CreateFromType(pSystem,
			GetWreckDesc(),
			CreateCtx,
			&pWreck);

	//	Set properties of the wreck

	pWreck->SetSovereign(pSovereign);
	pWreck->SetWreckImage(this);
	pWreck->SetWreckParams(this, pShip);

	//	Done

	if (retpWreck)
		*retpWreck = pWreck;
	}

void CShipClass::CreateExplosion (CShip *pShip, CSpaceObject *pWreck)

//	CreateExplosion
//
//	Creates an explosion for the given ship

	{
	DEBUG_TRY

	//	Explosion effect and damage

	SExplosionType Explosion;
	pShip->FireGetExplosionType(&Explosion);
	if (Explosion.pDesc == NULL)
		Explosion.pDesc = GetExplosionType();

	if (Explosion.pDesc)
		{
		CItemEnhancementStack *pEnhancements = NULL;
		if (Explosion.iBonus != 0)
			{
			pEnhancements = new CItemEnhancementStack;
			pEnhancements->InsertHPBonus(Explosion.iBonus);
			}

		pShip->GetSystem()->CreateWeaponFire(Explosion.pDesc,
				pEnhancements,
				Explosion.iCause,
				CDamageSource(pShip, Explosion.iCause, pWreck),
				pShip->GetPos(),
				pShip->GetVel(),
				0,
				NULL,
				CSystem::CWF_EXPLOSION,
				NULL);

		if (pEnhancements)
			pEnhancements->Delete();
		}

	//	Otherwise, if no defined explosion, we create a default one

	else
		{
		DWORD dwEffectID;

		//	If this is a large ship, use a large explosion

		if (RectWidth(m_Image.GetImageRect()) > 64)
			dwEffectID = g_LargeExplosionUNID;
		else
			dwEffectID = g_ExplosionUNID;

		CEffectCreator *pEffect = g_pUniverse->FindEffectType(dwEffectID);
		if (pEffect)
			pEffect->CreateEffect(pShip->GetSystem(),
					pWreck,
					pShip->GetPos(),
					pShip->GetVel(),
					0);

		//	Particles

		CObjectImageArray Image;
		RECT rcRect;
		rcRect.left = 0;
		rcRect.top = 0;
		rcRect.right = 4;
		rcRect.bottom = 4;
		Image.Init(g_ShipExplosionParticlesUNID,
				rcRect,
				8,
				3);

		CParticleEffect::CreateExplosion(pShip->GetSystem(),
				//pWreck,
				NULL,
				pShip->GetPos(),
				pShip->GetVel(),
				mathRandom(1, 50),
				LIGHT_SPEED * 0.25,
				0,
				300,
				Image,
				NULL);

		//	HACK: No image means paint smoke particles

		CObjectImageArray Dummy;
		CParticleEffect::CreateExplosion(pShip->GetSystem(),
				//pWreck,
				NULL,
				pShip->GetPos(),
				pShip->GetVel(),
				mathRandom(25, 150),
				LIGHT_SPEED * 0.1,
				20 + mathRandom(10, 30),
				45,
				Dummy,
				NULL);
		}

	//	Always play default sound

	g_pUniverse->PlaySound(pShip, g_pUniverse->FindSound(g_ShipExplosionSoundUNID));

	DEBUG_CATCH
	}

void CShipClass::CreateWreck (CShip *pShip, CSpaceObject **retpWreck)

//	CreateWreck
//
//	Creates a wreck for the given ship

	{
	DEBUG_TRY

	//	Create the wreck

	CStation *pWreck;
	CreateEmptyWreck(pShip->GetSystem(),
			pShip,
			pShip->GetPos(),
			pShip->GetVel(),
			pShip->GetSovereign(),
			&pWreck);

	//	The chance that an installed item survives is related to
	//	the wreck chance.

	int iDestroyArmorChance = 100 - (GetWreckChance() / 2);
	int iDestroyDeviceChance = 100 - Min(GetWreckChance(), 50);

	//	Decrease the chance of armor surviving if this ship class
	//	has lots of armor segments

	iDestroyArmorChance = Min(Max(iDestroyArmorChance, 100 - (100 / (1 + GetHullSectionCount()))), 95);

	//	Add items to the wreck

	CItemListManipulator Source(pShip->GetItemList());
	CItemListManipulator Dest(pWreck->GetItemList());

	while (Source.MoveCursorForward())
		{
		CItem WreckItem = Source.GetItemAtCursor();
		CItemType *pType = WreckItem.GetType();

		//	Skip virtual items

		if (pType->IsVirtual())
			continue;

		//	Installed items may or may not be damaged.

		if (WreckItem.IsInstalled())
			{
			//	Make sure that the armor item reflects the current
			//	state of the ship's armor.

			if (pType->GetCategory() == itemcatArmor)
				{
				//	Most armor is destroyed

				if (mathRandom(1, 100) <= iDestroyArmorChance)
					continue;

				WreckItem.ClearDamaged();

				if (!pShip->IsArmorRepairable(WreckItem.GetInstalled()))
					continue;
				else if (pShip->IsArmorDamaged(WreckItem.GetInstalled()))
					WreckItem.SetDamaged();
				}

			//	Other installed devices have a chance of being
			//	damaged or destroyed.

			else
				{
				int iRoll = mathRandom(1, 100);

				//	Sometimes the device is destroyed

				if (iRoll <= iDestroyDeviceChance)
					continue;

				//	80% of the remaining time, the device is damaged

				else if (mathRandom(1, 100) <= 80)
					{
					if (WreckItem.IsDamaged())
						continue;
					else if (CItemEnhancement(WreckItem.GetMods()).IsEnhancement())
						{
						CItemEnhancement Mods(WreckItem.GetMods());
						Mods.Combine(WreckItem, etLoseEnhancement);
						WreckItem.AddEnhancement(Mods);
						}
					else if (WreckItem.IsEnhanced())
						WreckItem.ClearEnhanced();
					else
						WreckItem.SetDamaged();
					}
				}

			WreckItem.SetInstalled(-1);
			}

		//	Add to wreck

		Dest.AddItem(WreckItem);
		}

	//	The wreck is radioactive if the ship is radioactive (or if this
	//	ship class always has radioactive wrecks)

	if (pShip->IsRadioactive() || m_fRadioactiveWreck)
		pWreck->MakeRadioactive();

	//	Done

	if (retpWreck)
		*retpWreck = pWreck;

	DEBUG_CATCH
	}

void CShipClass::CreateWreckImage (void)

//	CreateWreckImage
//
//	Creates a wreck image randomly

	{
	if (!m_Image.IsLoaded())
		return;

	int cxWidth = RectWidth(m_Image.GetImageRect());
	int cyHeight = RectHeight(m_Image.GetImageRect());

	//	Get the image for damage

	if (g_pDamageBitmap == NULL)
		{
		CObjectImage *pDamageImage = g_pUniverse->FindLibraryImage(g_DamageImageUNID);
		if (pDamageImage == NULL)
			return;

		//	Lock the image because we keep it around in a global

		SDesignLoadCtx Ctx;
		if (pDamageImage->Lock(Ctx) != NOERROR)
			return;

		//	Get the image

		g_pDamageBitmap = pDamageImage->GetImage(strFromInt(GetUNID()));
		if (g_pDamageBitmap == NULL)
			return;
		}

	//	Create the bitmap

	m_WreckBitmap.CreateBlank(cxWidth, cyHeight * WRECK_IMAGE_VARIANTS, true);

	//	Blt the images

	for (int i = 0; i < WRECK_IMAGE_VARIANTS; i++)
		{
		//	Pick a random rotation

		m_Image.CopyImage(m_WreckBitmap,
				0,
				i * cyHeight,
				0,
				mathRandom(0, m_RotationDesc.GetFrameCount() - 1));

		//	Add some destruction

		int iCount = cxWidth * 2;
		for (int j = 0; j < iCount; j++)
			{
			m_WreckBitmap.ColorTransBlt(DAMAGE_IMAGE_WIDTH * mathRandom(0, DAMAGE_IMAGE_COUNT-1),
					0,
					DAMAGE_IMAGE_WIDTH,
					DAMAGE_IMAGE_COUNT,
					255,
					*g_pDamageBitmap,
					mathRandom(0, cxWidth-1) - (DAMAGE_IMAGE_WIDTH / 2),
					(i * cyHeight) + mathRandom(0, cyHeight-1) - (DAMAGE_IMAGE_HEIGHT / 2));
			}
		}

	if (!m_Image.HasAlpha())
		m_WreckBitmap.SetTransparentColor();

	//	Initialize an image

	RECT rcRect;
	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = cxWidth;
	rcRect.bottom = cyHeight;
	m_WreckImage.Init(&m_WreckBitmap, rcRect, 0, 0, false);
	}

void CShipClass::FindBestMissile (CDeviceClass *pLauncher, IItemGenerator *pItems, CItemType **retpBestMissile) const

//	FindBestMissile
//
//	Finds the best missile in the table

	{
	int i;

	for (i = 0; i < pItems->GetItemTypeCount(); i++)
		{
		CItemType *pType = pItems->GetItemType(i);
		if (pType->GetCategory() == itemcatMissile
				&& (pLauncher->GetAmmoVariant(pType) != -1)
				&& (*retpBestMissile == NULL
					|| pType->GetLevel() > (*retpBestMissile)->GetLevel()))
			*retpBestMissile = pType;
		}

	//	Recurse

	for (i = 0; i < pItems->GetGeneratorCount(); i++)
		FindBestMissile(pLauncher, pItems->GetGenerator(i), retpBestMissile);
	}

void CShipClass::FindBestMissile (CDeviceClass *pLauncher, const CItemList &Items, CItemType **retpBestMissile) const

//	FindBestMissile
//
//	Finds the best missile in the item list

	{
	int i;

	for (i = 0; i < Items.GetCount(); i++)
		{
		CItemType *pType = Items.GetItem(i).GetType();
		if (pType->GetCategory() == itemcatMissile
				&& (pLauncher->GetAmmoVariant(pType) != -1)
				&& (*retpBestMissile == NULL
					|| pType->GetLevel() > (*retpBestMissile)->GetLevel()))
			*retpBestMissile = pType;
		}
	}

bool CShipClass::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	int i;

	if (strEquals(sField, FIELD_ARMOR_COUNT))
		*retsValue = strFromInt(GetHullSectionCount());
	else if (strEquals(sField, FIELD_ARMOR_HP))
		{
		CShipClass::HullSection *pSection = (GetHullSectionCount() > 0 ? GetHullSection(0) : NULL);
		if (pSection)
			pSection->pArmor->FindDataField(FIELD_HP, retsValue);
		else
			*retsValue = NULL_STR;
		return true;
		}
	else if (strEquals(sField, FIELD_BALANCE_TYPE))
		CalcBalanceType(retsValue);
	else if (strEquals(sField, FIELD_CARGO_SPACE))
		*retsValue = strFromInt(GetCargoSpace());
	else if (strEquals(sField, FIELD_COMBAT_STRENGTH))
		*retsValue = strFromInt((int)(CalcCombatStrength() + 0.5));
	else if (strEquals(sField, FIELD_DAMAGE_RATE))
		*retsValue = strFromInt((int)(CalcDamageRate() + 0.5));
	else if (strEquals(sField, FIELD_DEFENSE_RATE))
		*retsValue = strFromInt((int)(CalcDefenseRate() + 0.5));
	else if (strEquals(sField, FIELD_DOCK_SERVICES_SCREEN))
		{
		const CPlayerSettings *pPlayer = GetPlayerSettings();
		if (pPlayer)
			*retsValue = pPlayer->GetDockServicesScreen().GetStringUNID(this);
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_DODGE_RATE))
		*retsValue = strFromInt((int)(100.0 * CalcDodgeRate()));

	else if (strEquals(sField, FIELD_INSTALL_DEVICE_MAX_LEVEL))
		{
		int iMaxLevel = (m_pTrade ? m_pTrade->GetMaxLevelMatched(serviceInstallDevice) : -1);
		*retsValue = (iMaxLevel != -1 ? strFromInt(iMaxLevel) : NULL_STR);
		}
	else if (strEquals(sField, FIELD_MANUFACTURER))
		*retsValue = m_sManufacturer;
	else if (strEquals(sField, FIELD_MASS))
		*retsValue = strFromInt(m_iMass);
	else if (strEquals(sField, FIELD_MAX_ROTATION))
		*retsValue = strFromInt(mathRound(m_RotationDesc.GetMaxRotationSpeedPerTick()));
	else if (strEquals(sField, FIELD_MAX_SPEED))
		{
		DriveDesc Desc;
		GetDriveDesc(&Desc);
		*retsValue = strFromInt((int)((100.0 * Desc.rMaxSpeed / LIGHT_SPEED) + 0.5));
		}
	else if (strEquals(sField, FIELD_NAME))
		*retsValue = GetNounPhrase(0x00);
	else if (strEquals(sField, FIELD_PRIMARY_ARMOR))
		{
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_PRIMARY_ARMOR_UNID), 0));
		if (pItem)
			*retsValue = pItem->GetNounPhrase(0x80);
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_PRIMARY_ARMOR_UNID))
		{
		CArmorClass *pArmor = NULL;
		for (i = 0; i < GetHullSectionCount(); i++)
			{
			HullSection *pHull = GetHullSection(i);
			if (pArmor == NULL || pHull->pArmor->GetItemType()->GetLevel() > pArmor->GetItemType()->GetLevel())
				pArmor = pHull->pArmor;
			}
		if (pArmor)
			*retsValue = strFromInt(pArmor->GetItemType()->GetUNID());
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_SHIELD))
		{
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_SHIELD_UNID), 0));
		if (pItem)
			*retsValue = pItem->GetNounPhrase(0x80);
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_SHIELD_UNID))
		{
		CDeviceClass *pDevice = m_AverageDevices.GetNamedDevice(devShields);
		if (pDevice)
			*retsValue = strFromInt(pDevice->GetUNID());
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_SHIP_STATUS_SCREEN))
		{
		const CPlayerSettings *pPlayer = GetPlayerSettings();
		if (pPlayer)
			*retsValue = pPlayer->GetShipScreen().GetStringUNID(this);
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_STARTING_SYSTEM))
		{
		const CPlayerSettings *pPlayer = GetPlayerSettings();
		if (pPlayer)
			{
			*retsValue = pPlayer->GetStartingNode();
			if (retsValue->IsBlank())
				*retsValue = g_pUniverse->GetCurrentAdventureDesc()->GetStartingNodeID();
			}
		else
			*retsValue = NULL_STR;
		}
	else if (strEquals(sField, FIELD_LAUNCHER))
		{
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_LAUNCHER_UNID), 0));
		if (pItem)
			*retsValue = pItem->GetNounPhrase(0x80);
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_LAUNCHER_UNID))
		{
		CDeviceClass *pDevice = m_AverageDevices.GetNamedDevice(devMissileWeapon);
		if (pDevice)
			*retsValue = strFromInt(pDevice->GetUNID());
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_PRIMARY_WEAPON))
		{
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_PRIMARY_WEAPON_UNID), 0));
		if (pItem)
			*retsValue = pItem->GetNounPhrase(0x80);
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_PRIMARY_WEAPON_UNID))
		{
		CDeviceClass *pDevice = m_AverageDevices.GetNamedDevice(devPrimaryWeapon);
		if (pDevice)
			*retsValue = strFromInt(pDevice->GetUNID());
		else
			*retsValue = CONSTLIT("none");
		}
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
	else if (strEquals(sField, FIELD_FIRE_ACCURACY))
		*retsValue = strFromInt(m_AISettings.GetFireAccuracy());
	else if (strEquals(sField, FIELD_FIRE_RANGE_ADJ))
		*retsValue = strFromInt(m_AISettings.GetFireRangeAdj());
	else if (strEquals(sField, FIELD_FIRE_RATE_ADJ))
		*retsValue = strFromInt((int)((1000.0 / m_AISettings.GetFireRateAdj()) + 0.5));
	else if (strEquals(sField, FIELD_MANEUVER))
		{
		int iManeuver = GetManeuverability() * GetRotationRange() / STD_ROTATION_COUNT;
		if (iManeuver <= 0)
			iManeuver = 1;

		*retsValue = strFromInt(30000 / iManeuver);
		}
	else if (strEquals(sField, FIELD_THRUST))
		{
		DriveDesc Drive;
		GetDriveDesc(&Drive);
		*retsValue = strFromInt(Drive.iThrust);
		}
	else if (strEquals(sField, FIELD_THRUST_TO_WEIGHT))
		{
		DriveDesc Drive;
		GetDriveDesc(&Drive);

		Metric rMass = CalcMass(m_AverageDevices);
		int iRatio = (int)((200.0 * (rMass > 0.0 ? Drive.iThrust / rMass : 0.0)) + 0.5);
		*retsValue = strFromInt(10 * iRatio);
		}
	else if (strEquals(sField, FIELD_TREASURE_VALUE))
		*retsValue = strFromInt(m_pItems ? (int)m_pItems->GetAverageValue(GetLevel()) : 0);

	else if (strEquals(sField, FIELD_WRECK_CHANCE))
		*retsValue = strFromInt(m_iLeavesWreck);

	else if (strEquals(sField, FIELD_PRIMARY_WEAPON_RANGE))
		{
		int iRange = 0;
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_PRIMARY_WEAPON_UNID), 0));
		if (pItem)
			{
			CDeviceClass *pDevice = pItem->GetDeviceClass();
			if (pDevice)
				{
				CWeaponClass *pWeapon = pDevice->AsWeaponClass();
				if (pWeapon)
					iRange = (int)((pWeapon->GetMaxEffectiveRange(NULL, NULL, NULL) / LIGHT_SECOND) + 0.5);
				}
			}
		*retsValue = strFromInt(iRange);
		}
	else if (strEquals(sField, FIELD_PRIMARY_WEAPON_RANGE_ADJ))
		{
		int iRange = 0;
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_PRIMARY_WEAPON_UNID), 0));
		if (pItem)
			{
			CDeviceClass *pDevice = pItem->GetDeviceClass();
			if (pDevice)
				{
				CWeaponClass *pWeapon = pDevice->AsWeaponClass();
				if (pWeapon)
					{
					iRange = (int)((pWeapon->GetMaxEffectiveRange(NULL, NULL, NULL) / LIGHT_SECOND) + 0.5);
					iRange = iRange * m_AISettings.GetFireRangeAdj() / 100;
					}
				}
			}
		*retsValue = strFromInt(iRange);
		}
	else if (strEquals(sField, FIELD_SCORE))
		*retsValue = strFromInt(m_iScore);
	else if (strEquals(sField, FIELD_SIZE))
		*retsValue = strFromInt(m_iSize);
	else if (strEquals(sField, FIELD_LEVEL))
		*retsValue = strFromInt(m_iLevel);
	else if (strEquals(sField, FIELD_MAX_CARGO_SPACE))
		*retsValue = strFromInt(GetMaxCargoSpace());
	else if (strEquals(sField, FIELD_GENERIC_NAME))
		*retsValue = GetGenericName();
	else if (strEquals(sField, FIELD_MAX_ARMOR_MASS))
		*retsValue = strFromInt(GetMaxArmorMass());
	else if (strEquals(sField, FIELD_HULL_MASS))
		*retsValue = strFromInt(GetHullMass());
	else if (strEquals(sField, FIELD_DEVICE_SLOTS))
		{
		int iSlots = GetMaxDevices();
		if (iSlots == -1)
			iSlots = m_AverageDevices.GetCount();

		*retsValue = strFromInt(iSlots);
		}
	else if (strEquals(sField, FIELD_DEVICE_SLOTS_NON_WEAPONS))
		{
		int iSlots = GetMaxNonWeapons();
		if (iSlots == -1)
			return FindDataField(FIELD_DEVICE_SLOTS, retsValue);

		*retsValue = strFromInt(iSlots);
		}
	else if (strEquals(sField, FIELD_DEVICE_SLOTS_WEAPONS))
		{
		int iSlots = GetMaxWeapons();
		if (iSlots == -1)
			return FindDataField(FIELD_DEVICE_SLOTS, retsValue);

		*retsValue = strFromInt(iSlots);
		}
	else if (strEquals(sField, FIELD_ARMOR_ITEMS))
		{
		//	Generate a list of armor items

		TArray<CItem> Items;
		for (i = 0; i < GetHullSectionCount(); i++)
			{
			HullSection *pSect = GetHullSection(i);
			CItem theItem(pSect->pArmor->GetItemType(), 1);
			pSect->Enhanced.EnhanceItem(theItem);

			Items.Insert(theItem);
			}

		//	Convert to a data field

		*retsValue = CreateDataFieldFromItemList(Items);
		}
	else if (strEquals(sField, FIELD_DEVICE_ITEMS))
		{
		//	Generate a list of device items

		TArray<CItem> Items;
		for (i = 0; i < m_AverageDevices.GetCount(); i++)
			{
			const SDeviceDesc &Desc = m_AverageDevices.GetDeviceDesc(i);
			Items.Insert(Desc.Item);
			}

		//	Convert to a data field

		*retsValue = CreateDataFieldFromItemList(Items);
		}
	else if (strEquals(sField, FIELD_DRIVE_IMAGE))
		{
		if (m_Exhaust.GetCount() == 0)
			*retsValue = CONSTLIT("none");
		else
			*retsValue = CONSTLIT("Image");
		}
	else if (CReactorClass::FindDataField(m_ReactorDesc, sField, retsValue))
		return true;
	else
		return CDesignType::FindDataField(sField, retsValue);

	return true;
	}

void CShipClass::GenerateDevices (int iLevel, CDeviceDescList &Devices)

//	GenerateDevices
//
//	Generate a list of devices
	
	{
	DEBUG_TRY

	Devices.RemoveAll();

	if (m_pDevices)
		{
		SDeviceGenerateCtx Ctx;
		Ctx.iLevel = iLevel;
		Ctx.pRoot = m_pDevices;
		Ctx.pResult = &Devices;

		m_pDevices->AddDevices(Ctx);
		}

	DEBUG_CATCH
	}

CString CShipClass::GenerateShipName (DWORD *retdwFlags)

//	GenerateShipName
//
//	Generate a ship name

	{
	if (!m_sShipNames.IsBlank())
		{
		if (retdwFlags)
			*retdwFlags = m_dwShipNameFlags;

		if (m_ShipNamesIndices.GetCount() > 0)
			{
			CString sNameTemplate = strDelimitGet(
					m_sShipNames,
					';',
					DELIMIT_TRIM_WHITESPACE,
					m_ShipNamesIndices[m_iShipName++ % m_ShipNamesIndices.GetCount()]);

			return GenerateRandomNameFromTemplate(sNameTemplate);
			}
		else
			return GenerateRandomName(m_sShipNames, NULL_STR);
		}
	else
		{
		if (retdwFlags)
			*retdwFlags = 0;
		return NULL_STR;
		}
	}

const SArmorImageDesc *CShipClass::GetArmorDescInherited (void)

//	GetArmorDescInherited
//
//	Returns the armor desc from this class or base classes

	{
	CDesignType *pBase;

	const SArmorImageDesc *pDesc = (m_pPlayerSettings ? m_pPlayerSettings->GetArmorImageDescRaw() : NULL);
	if (pDesc)
		return pDesc;
	else if (pBase = GetInheritFrom())
		{
		CShipClass *pBaseClass = CShipClass::AsType(pBase);
		return pBaseClass->GetArmorDescInherited();
		}
	else
		return NULL;
	}

CCommunicationsHandler *CShipClass::GetCommsHandler (void)

//	GetCommsHandler
//
//	Returns the comms handler to use

	{
	CDesignType *pParent = GetInheritFrom();
	CCommunicationsHandler *pParentHandler;

	if (pParent && (pParentHandler = pParent->GetCommsHandler()))
		{
		if (!m_fCommsHandlerInit)
			{
			m_CommsHandler.Merge(m_OriginalCommsHandler);
			m_CommsHandler.Merge(*pParentHandler);

			m_fCommsHandlerInit = true;
			}

		return (m_CommsHandler.GetCount() ? &m_CommsHandler : NULL);
		}
	else
		return (m_OriginalCommsHandler.GetCount() ? &m_OriginalCommsHandler : NULL);
	}

CVector CShipClass::GetDockingPortOffset (int iRotation)

//	GetDockingPortOffset
//
//	Returns an offset to the ship center from the desired position of the docking
//	port when the ship is docked.
//
//	[In other words, the resulting vector points from the station's docking port
//	to the ship center.]

	{
	//	For small ships we just go with the ship center.

	int iImageSize = m_Image.GetImageViewportSize();
	if (iImageSize <= DOCK_OFFSET_STD_SIZE)
		return NullVector;

	return PolarToVector(iRotation + 180, (0.8 * g_KlicksPerPixel * ((iImageSize - DOCK_OFFSET_STD_SIZE) / 2)));
	}

void CShipClass::GetDriveDesc (DriveDesc *retDriveDesc) const

//	GetDriveDesc
//
//	Returns the drive desc for the hull plus any device

	{
	*retDriveDesc = *GetHullDriveDesc();

	CDeviceClass *pDrive = m_AverageDevices.GetNamedDevice(devDrive);
	if (pDrive)
		{
		const DriveDesc *pDriveDesc = pDrive->GetDriveDesc();

		retDriveDesc->dwUNID = pDriveDesc->dwUNID;
		retDriveDesc->fInertialess = pDriveDesc->fInertialess;
		retDriveDesc->iPowerUse = pDriveDesc->iPowerUse;
		retDriveDesc->iThrust += pDriveDesc->iThrust;
		retDriveDesc->rMaxSpeed = Max(retDriveDesc->rMaxSpeed, pDriveDesc->rMaxSpeed);
		}
	}

CString CShipClass::GetGenericName (DWORD *retdwFlags)

//	GetGenericName
//
//	Returns the generic name of the ship class

	{
	if (retdwFlags)
		*retdwFlags = m_dwClassNameFlags;

	if (GetClassName().IsBlank())
		return strPatternSubst(CONSTLIT("%s %s"), GetManufacturerName(), GetShipTypeName());
	else
		{
		if (GetShipTypeName().IsBlank())
			return GetClassName();
		else
			return strPatternSubst(CONSTLIT("%s-class %s"), GetClassName(), GetShipTypeName());
		}
	}

int CShipClass::GetHullSectionAtAngle (int iAngle)

//	GetHullSectionAtAngle
//
//	Returns the index of the hull section at the given
//	angle.

	{
	int i;

	for (i = 0; i < GetHullSectionCount(); i++)
		{
		HullSection *pSect = GetHullSection(i);

		int iStart = pSect->iStartAt;
		int iEnd = (pSect->iStartAt + pSect->iSpan) % 360;

		if (iEnd > iStart)
			{
			if (iAngle >= iStart && iAngle < iEnd)
				return i;
			}
		else
			{
			if (iAngle < iEnd || iAngle >= iStart)
				return i;
			}
		}

	//	The last hull section may wrap around to the beginning again. If we haven't
	//	found the angle yet, assume it is the last section.

	return GetHullSectionCount() - 1;
	}

int CShipClass::GetMaxStructuralHitPoints (void) const

//	GetMaxStructuralHitPoints
//
//	Returns the HP of the wreck created by this class

	{
	//	If it is set, return that
	
	if (m_iStructuralHP)
		return m_iStructuralHP;

	//	Otherwise we have to compute it based on level and mass

	return (m_iLevel + 1) * (mathSqrt(m_iMass) + 10) * mathRandom(75, 125) / 200;
	}

CString CShipClass::GetName (DWORD *retdwFlags)

//	GetName
//
//	Returns the name of the ship class

	{
	if (!m_sShipNames.IsBlank() && ::IsConstantName(m_sShipNames))
		return GenerateShipName(retdwFlags);
	else
		return GetGenericName(retdwFlags);
	}

CString CShipClass::GetNounPhrase (DWORD dwFlags)

//	GetNounPhrase
//
//	Returns the generic name of the ship

	{
	DWORD dwNameFlags;

	CString sName;
	if (dwFlags & nounShort)
		{
		sName = GetShortName();
		dwNameFlags = m_dwClassNameFlags;
		}
	else
		sName = GetName(&dwNameFlags);

	return ::ComposeNounPhrase(sName, 1, NULL_STR, dwNameFlags, dwFlags);
	}

CPlayerSettings *CShipClass::GetPlayerSettingsInherited (void) const

//	GetPlayerSettingsInherited
//
//	Returns player settings from us or a base class

	{
	CDesignType *pBase;

	if (m_pPlayerSettings)
		return m_pPlayerSettings;
	else if (pBase = GetInheritFrom())
		{
		CShipClass *pBaseClass = CShipClass::AsType(pBase);
		return pBaseClass->GetPlayerSettingsInherited();
		}
	else
		return NULL;
	}

const SReactorImageDesc *CShipClass::GetReactorDescInherited (void)

//	GetReactorDescInherited
//
//	Returns the reator desc from this class or base classes

	{
	CDesignType *pBase;

	const SReactorImageDesc *pDesc = (m_pPlayerSettings ? m_pPlayerSettings->GetReactorImageDescRaw() : NULL);
	if (pDesc)
		return pDesc;
	else if (pBase = GetInheritFrom())
		{
		CShipClass *pBaseClass = CShipClass::AsType(pBase);
		return pBaseClass->GetReactorDescInherited();
		}
	else
		return NULL;
	}

const SShieldImageDesc *CShipClass::GetShieldDescInherited (void)

//	GetShieldDescInherited
//
//	Returns the shield desc from this class or base classes

	{
	CDesignType *pBase;

	const SShieldImageDesc *pDesc = (m_pPlayerSettings ? m_pPlayerSettings->GetShieldImageDescRaw() : NULL);
	if (pDesc)
		return pDesc;
	else if (pBase = GetInheritFrom())
		{
		CShipClass *pBaseClass = CShipClass::AsType(pBase);
		return pBaseClass->GetShieldDescInherited();
		}
	else
		return NULL;
	}

CString CShipClass::GetShortName (void)

//	GetShortName
//
//	Returns the short name of the class

	{
	if (GetClassName().IsBlank())
		return strPatternSubst(LITERAL("%s %s"), GetManufacturerName(), GetShipTypeName());
	else
		return GetClassName();
	}

const SWeaponImageDesc *CShipClass::GetWeaponDescInherited (void)

//	GetWeaponDescInherited
//
//	Returns the shield desc from this class or base classes

	{
	CDesignType *pBase;

	const SWeaponImageDesc *pDesc = (m_pPlayerSettings ? m_pPlayerSettings->GetWeaponImageDescRaw() : NULL);
	if (pDesc)
		return pDesc;
	else if (pBase = GetInheritFrom())
		{
		CShipClass *pBaseClass = CShipClass::AsType(pBase);
		return pBaseClass->GetWeaponDescInherited();
		}
	else
		return NULL;
	}

CStationType *CShipClass::GetWreckDesc (void)
	{
	if (m_pWreckType)
		return m_pWreckType;
	else
		{
		if (g_pWreckDesc == NULL)
			g_pWreckDesc = g_pUniverse->FindStationType(g_ShipWreckUNID);

		return g_pWreckDesc;
		}
	}

void CShipClass::GetWreckImage (CObjectImageArray *retWreckImage)

//	GetWreckImage
//
//	Returns wreck image

	{
	RECT rcRect;
	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = RectWidth(m_Image.GetImageRect());
	rcRect.bottom = RectHeight(m_Image.GetImageRect());
	retWreckImage->Init(&m_WreckBitmap, rcRect, 0, 0, false);
	}

int CShipClass::GetWreckImageVariants (void)

//	GetWreckImageVariants
//
//	Returns the number of wreck images

	{
	return WRECK_IMAGE_VARIANTS;
	}

void CShipClass::InitEffects (CShip *pShip, CObjectEffectList *retEffects)

//	InitEffects
//
//	Initializes effects (like thrust, etc.)

	{
	int i;

	CObjectEffectDesc &Effects = GetEffectsDesc();
	if (Effects.GetEffectCount() > 0)
		{
		TArray<IEffectPainter *> Painters;
		Painters.InsertEmpty(Effects.GetEffectCount());

		//	Compute the thrust and max speed

		int iThrust;
		Metric rMaxSpeed;
		if (pShip)
			{
			iThrust = (int)pShip->GetThrust();
			rMaxSpeed = pShip->GetMaxSpeed();
			}
		else
			{
			iThrust = m_DriveDesc.iThrust;
			rMaxSpeed = m_DriveDesc.rMaxSpeed;
			}

		//	Compute power of maneuvering thrusters

		int iThrustersPerSide = Max(1, Effects.GetEffectCount(CObjectEffectDesc::effectThrustLeft));
		int iThrusterPower = Max(1, mathRound((GetHullMass() / iThrustersPerSide) * GetRotationDesc().GetRotationAccelPerTick()));

		//	Compute power of main thruster

		int iMainThrusters = Max(1, Effects.GetEffectCount(CObjectEffectDesc::effectThrustMain));
		int iMainPower = Max(1, iThrust / iMainThrusters);
		int iMaxSpeed = mathRound(100.0 * rMaxSpeed / LIGHT_SPEED);

		//	Create painters

		for (i = 0; i < Effects.GetEffectCount(); i++)
			{
			const CObjectEffectDesc::SEffectDesc &EffectDesc = Effects.GetEffectDesc(i);

			switch (EffectDesc.iType)
				{
				case CObjectEffectDesc::effectThrustLeft:
				case CObjectEffectDesc::effectThrustRight:
					{
					//	For now we only create maneuvering thruster effects on 
					//	player ships

					if (IsPlayerShip())
						{
						CCreatePainterCtx CreateCtx;
						CreateCtx.AddDataInteger(FIELD_THRUSTER_POWER, iThrusterPower);
						CreateCtx.SetUseObjectCenter();
						Painters[i] = Effects.CreatePainter(CreateCtx, i);
						}
					else
						Painters[i] = NULL;
					break;
					}

				case CObjectEffectDesc::effectThrustMain:
					{
					CCreatePainterCtx CreateCtx;
					CreateCtx.AddDataInteger(FIELD_MAX_SPEED, iMaxSpeed);
					CreateCtx.AddDataInteger(FIELD_THRUSTER_POWER, iMainPower);
					CreateCtx.SetUseObjectCenter();
					Painters[i] = Effects.CreatePainter(CreateCtx, i);
					break;
					}

				default:
					Painters[i] = NULL;
					break;
				}
			}

		//	Initialize
		//	NOTE: the m_Effects structure takes ownership of all painters.

		retEffects->Init(Effects, Painters);
		}
	}

void CShipClass::InitShipNamesIndices (void)

//	InitShipNamesIndices
//
//	Initializes m_ShipNamesIndices and m_iShipName from m_sShipNames

	{
	int i;
	int iCount = strDelimitCount(m_sShipNames, ';', DELIMIT_TRIM_WHITESPACE);

	m_ShipNamesIndices.DeleteAll();
	m_ShipNamesIndices.InsertEmpty(iCount);
	for (i = 0; i < iCount; i++)
		m_ShipNamesIndices[i] = i;

	m_ShipNamesIndices.Shuffle();

	m_iShipName = 0;
	}

void CShipClass::InstallEquipment (CShip *pShip)

//	InstallEquipment
//
//	Install initial equipment on the given ship
//
//	NOTE: We assume that the ship was just created, so we assume that equipment
//	is at default state.

	{
	int i;

	for (i = 0; i < m_Equipment.GetCount(); i++)
		{
		if (m_Equipment[i].bInstall)
			pShip->SetAbility(m_Equipment[i].iEquipment, ablInstall, -1, 0);
		else
			pShip->SetAbility(m_Equipment[i].iEquipment, ablRemove, -1, 0);
		}
	}

void CShipClass::UnbindGlobal (void)

//	UnbindGlobal
//
//	We're unbinding, which means we have to release any resources.

	{
	g_pDamageBitmap = NULL;
	g_pWreckDesc = NULL;
	}

void CShipClass::MarkImages (bool bMarkDevices)

//	MarkImages
//
//	Marks images used by the ship

	{
	int i;

	m_Image.MarkImage();

	if (m_pExplosionType)
		m_pExplosionType->MarkImages();

	//	If necessary mark images for all our installed devices

	if (bMarkDevices)
		{
		for (i = 0; i < m_AverageDevices.GetCount(); i++)
			{
			CDeviceClass *pDevice = m_AverageDevices.GetDeviceClass(i);
			pDevice->MarkImages();
			}
		}

	//	Wreck images

	if (!m_WreckImage.IsLoaded())
		CreateWreckImage();

	//	Effects

	m_Effects.MarkImages();
	}

void CShipClass::OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	OnAddTypesUsed
//
//	Adds types used by the class

	{
	int i;

	retTypesUsed->SetAt(m_pWreckType.GetUNID(), true);

	for (i = 0; i < GetHullSectionCount(); i++)
		retTypesUsed->SetAt(GetHullSection(i)->pArmor->GetItemType()->GetUNID(), true);

	if (m_pDevices)
		m_pDevices->AddTypesUsed(retTypesUsed);

	if (m_pPlayerSettings)
		m_pPlayerSettings->AddTypesUsed(retTypesUsed);

	if (m_pItems)
		m_pItems->AddTypesUsed(retTypesUsed);

	if (m_pEscorts)
		m_pEscorts->AddTypesUsed(retTypesUsed);

	retTypesUsed->SetAt(strToInt(m_pDefaultScreen.GetUNID(), 0), true);
	retTypesUsed->SetAt(m_dwDefaultBkgnd, true);
	retTypesUsed->SetAt(m_Image.GetBitmapUNID(), true);
	retTypesUsed->SetAt(m_WreckImage.GetBitmapUNID(), true);
	retTypesUsed->SetAt(m_pExplosionType.GetUNID(), true);
	retTypesUsed->SetAt(m_ExhaustImage.GetBitmapUNID(), true);
	}

ALERROR CShipClass::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	ALERROR error;
	int i;

	//	Image

	if (error = m_Image.OnDesignLoadComplete(Ctx))
		goto Fail;

	//	Now that we have the image we can bind the rotation desc, because it needs
	//	the rotation count, etc.

	if (error = m_RotationDesc.Bind(Ctx, m_Image))
		goto Fail;

	//	Thruster effects

	if (error = m_Effects.Bind(Ctx, m_Image))
		goto Fail;

	//	Drive images

	if (m_Exhaust.GetCount() > 0)
		{
		int iRotationCount = m_RotationDesc.GetFrameCount();
		int iScale = m_Image.GetImageViewportSize();

		m_ExhaustImage.SetRotationCount(iRotationCount);
		if (error = m_ExhaustImage.OnDesignLoadComplete(Ctx))
			goto Fail;

		for (i = 0; i < m_Exhaust.GetCount(); i++)
			m_Exhaust[i].PosCalc.InitComplete(iRotationCount, iScale, 180);
		}

	//	Hull

	for (i = 0; i < GetHullSectionCount(); i++)
		{
		if (error = GetHullSection(i)->pArmor.Bind(Ctx))
			goto Fail;

		//	Must have armor

		if (GetHullSection(i)->pArmor == NULL)
			{
			Ctx.sError = CONSTLIT("ArmorSection must specify valid armor.");
			goto Fail;
			}
		}

	if (error = m_pExplosionType.Bind(Ctx))
		goto Fail;

	//	More

	if (error = m_Character.Bind(Ctx))
		goto Fail;

	if (error = m_CharacterClass.Bind(Ctx))
		goto Fail;

	if (error = m_pDefaultScreen.Bind(Ctx, GetLocalScreens()))
		goto Fail;

	if (m_pTrade)
		if (error = m_pTrade->OnDesignLoadComplete(Ctx))
			goto Fail;

	//	Load player settings

	CPlayerSettings *pBasePlayerSettings;
	if (m_pPlayerSettings)
		{
		if (error = m_pPlayerSettings->Bind(Ctx, this))
			goto Fail;
		}
	else if (pBasePlayerSettings = GetPlayerSettingsInherited())
		{
		m_pPlayerSettings = pBasePlayerSettings;
		m_fInheritedPlayerSettings = true;
		}

	//	AI Settings

	if (m_AISettings.GetMinCombatSeparation() < 0.0)
		{
		if (m_Image.IsLoaded())
			m_AISettings.SetMinCombatSeparation(RectWidth(m_Image.GetImageRect()) * g_KlicksPerPixel);
		else
			m_AISettings.SetMinCombatSeparation(60.0 * g_KlicksPerPixel);
		}

	//	Items

	if (m_pItems)
		if (error = m_pItems->OnDesignLoadComplete(Ctx))
			goto Fail;

	if (m_pDevices)
		if (error = m_pDevices->OnDesignLoadComplete(Ctx))
			goto Fail;

	//	Escorts

	if (m_pEscorts)
		if (error = m_pEscorts->OnDesignLoadComplete(Ctx))
			return error;

	//	Create the random wreck images

	if (error = m_pWreckType.Bind(Ctx))
		goto Fail;

	//	Generate an average set of devices

	GenerateDevices(1, m_AverageDevices);

	//	Initialize thrust, if necessary

	if (m_rThrustRatio != 0.0)
		{
		Metric rMass = CalcMass(m_AverageDevices);
		if (rMass > 0.0)
			m_DriveDesc.iThrust = (int)(((m_rThrustRatio * rMass) / 2.0) + 0.5);
		}

	//	Bind structures

	if (error = m_Interior.BindDesign(Ctx))
		goto Fail;

	//	Events

	m_fHasOnOrderChangedEvent = FindEventHandler(CONSTLIT("OnOrderChanged"));
	m_fHasOnAttackedByPlayerEvent = FindEventHandler(CONSTLIT("OnAttackedByPlayer"));
	m_fHasOnOrdersCompletedEvent = FindEventHandler(CONSTLIT("OnOrdersCompleted"));

	return NOERROR;

Fail:

	return ComposeLoadError(Ctx, Ctx.sError);
	}

ALERROR CShipClass::OnFinishBindDesign (SDesignLoadCtx &Ctx)

//	OnFinishBindDesign
//
//	All types bound.

	{
	//	Compute score and level

	if (!m_fScoreOverride)
		m_iScore = CalcScore();

	if (!m_fLevelOverride)
		m_iLevel = CalcLevel();

	if (!m_fCyberDefenseOverride)
		m_iCyberDefenseLevel = m_iLevel;

	//	Done

	return NOERROR;
	}

void CShipClass::OnInitFromClone (CDesignType *pSource)

//	OnInitFromClone
//
//	Initializes from pSource

	{
	CShipClass *pClass = CShipClass::AsType(pSource);
	if (pClass == NULL)
		{
		ASSERT(false);
		return;
		}

	m_sManufacturer = pClass->m_sManufacturer;
	m_sName = pClass->m_sName;
	m_sTypeName = pClass->m_sTypeName;
	m_dwClassNameFlags = pClass->m_dwClassNameFlags;

	m_sShipNames = pClass->m_sShipNames;
	m_dwShipNameFlags = pClass->m_dwShipNameFlags;
	m_ShipNamesIndices = pClass->m_ShipNamesIndices;
	m_iShipName = pClass->m_iShipName;

	m_iScore = pClass->m_iScore;
	m_iLevel = pClass->m_iLevel;
	m_fScoreOverride = pClass->m_fScoreOverride;
	m_fLevelOverride = pClass->m_fLevelOverride;

	m_iMass = pClass->m_iMass;
	m_iSize = pClass->m_iSize;
	m_iCargoSpace = pClass->m_iCargoSpace;
	m_RotationDesc = pClass->m_RotationDesc;
	m_rThrustRatio = pClass->m_rThrustRatio;
	m_DriveDesc = pClass->m_DriveDesc;
	m_ReactorDesc = pClass->m_ReactorDesc;
	m_iCyberDefenseLevel = pClass->m_iCyberDefenseLevel;
	m_fCyberDefenseOverride = pClass->m_fCyberDefenseOverride;

	m_iMaxArmorMass = pClass->m_iMaxArmorMass;
	m_iMaxCargoSpace = pClass->m_iMaxCargoSpace;
	m_iMaxReactorPower = pClass->m_iMaxReactorPower;
	m_iMaxDevices = pClass->m_iMaxDevices;
	m_iMaxWeapons = pClass->m_iMaxWeapons;
	m_iMaxNonWeapons = pClass->m_iMaxNonWeapons;

	m_iLeavesWreck = pClass->m_iLeavesWreck;
	m_iStructuralHP = pClass->m_iStructuralHP;
	m_pWreckType = pClass->m_pWreckType;
	m_Hull = pClass->m_Hull;
	m_Interior = pClass->m_Interior;

	if (pClass->m_pDevices)
		{
		m_pDevices = pClass->m_pDevices;
		m_fInheritedDevices = true;
		}

	m_AverageDevices = pClass->m_AverageDevices;

	m_Equipment = pClass->m_Equipment;

	m_AISettings = pClass->m_AISettings;

	if (pClass->m_pPlayerSettings)
		{
		m_pPlayerSettings = pClass->m_pPlayerSettings;
		m_fInheritedPlayerSettings = true;
		}

	if (pClass->m_pItems)
		{
		m_pItems = pClass->m_pItems;
		m_fInheritedItems = true;
		}

	if (pClass->m_pEscorts)
		{
		m_pEscorts = pClass->m_pEscorts;
		m_fInheritedEscorts = true;
		}

	m_CharacterClass = pClass->m_CharacterClass;
	m_Character = pClass->m_Character;

	m_DockingPorts = pClass->m_DockingPorts;
	m_pDefaultScreen = pClass->m_pDefaultScreen;
	m_dwDefaultBkgnd = pClass->m_dwDefaultBkgnd;
	m_fHasDockingPorts = pClass->m_fHasDockingPorts;

	if (pClass->m_pTrade)
		{
		m_pTrade = pClass->m_pTrade;
		m_fInheritedTrade = true;
		}

	m_OriginalCommsHandler = pClass->m_OriginalCommsHandler;
	m_CommsHandler = pClass->m_CommsHandler;
	m_fCommsHandlerInit = pClass->m_fCommsHandlerInit;

	m_Image = pClass->m_Image;
	m_Effects = pClass->m_Effects;

	//	No need to copy m_WreckImage or m_WreckBitmap because they are just
	//	caches.

	m_pExplosionType = pClass->m_pExplosionType;
	m_ExhaustImage = pClass->m_ExhaustImage;
	m_Exhaust = pClass->m_Exhaust;

	m_fRadioactiveWreck = pClass->m_fRadioactiveWreck;
	m_fTimeStopImmune = pClass->m_fTimeStopImmune;

	//	m_fHasOn... are computed during bind

	m_fVirtual = pClass->m_fVirtual;
	}

ALERROR CShipClass::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	ALERROR error;
	int i;

	//	Initialize basic info

	CString sAttrib;
	m_sManufacturer = pDesc->GetAttribute(CONSTLIT(g_ManufacturerAttrib));
	m_sName = pDesc->GetAttribute(CONSTLIT(g_ClassAttrib));
	m_sTypeName = pDesc->GetAttribute(CONSTLIT(g_TypeAttrib));
	m_dwClassNameFlags = LoadNameFlags(pDesc);
	m_fVirtual = pDesc->GetAttributeBool(VIRTUAL_ATTRIB);
	m_fInheritedPlayerSettings = false;

	//	Score and level

	m_fScoreOverride = pDesc->FindAttributeInteger(SCORE_ATTRIB, &m_iScore);
	m_fLevelOverride = pDesc->FindAttributeInteger(LEVEL_ATTRIB, &m_iLevel);

	//	Names

	CXMLElement *pNames = pDesc->GetContentElementByTag(NAMES_TAG);
	if (pNames)
		{
		m_sShipNames = pNames->GetContentText(0);
		m_dwShipNameFlags = LoadNameFlags(pNames);
		}
	else
		m_dwShipNameFlags = 0;

	InitShipNamesIndices();

	//	Load the images

	CXMLElement *pImage = pDesc->GetContentElementByTag(IMAGE_TAG);
	if (pImage)
		if (error = m_Image.InitFromXML(Ctx, pImage, false, STD_ROTATION_COUNT))
			return ComposeLoadError(Ctx, Ctx.sError);

	//	Initialize design

	m_iMass = pDesc->GetAttributeInteger(CONSTLIT(g_MassAttrib));
	m_iSize = pDesc->GetAttributeIntegerBounded(SIZE_ATTRIB, 1, -1, 0);
	m_iCargoSpace = pDesc->GetAttributeInteger(CARGO_SPACE_ATTRIB);
	m_iMaxCargoSpace = Max(m_iCargoSpace, pDesc->GetAttributeInteger(MAX_CARGO_SPACE_ATTRIB));
	m_iMaxArmorMass = pDesc->GetAttributeInteger(MAX_ARMOR_ATTRIB);
	m_iMaxReactorPower = pDesc->GetAttributeInteger(MAX_REACTOR_POWER_ATTRIB);

	if (error = m_RotationDesc.InitFromXML(Ctx, 
			strPatternSubst(CONSTLIT("%d:r"), GetUNID()), 
			pDesc))
		return ComposeLoadError(Ctx, Ctx.sError);

//	m_Image.SetRotationCount(m_RotationDesc.GetFrameCount());

	m_DriveDesc.dwUNID = GetUNID();
	m_DriveDesc.rMaxSpeed = (double)pDesc->GetAttributeInteger(CONSTLIT(g_MaxSpeedAttrib)) * LIGHT_SPEED / 100;

	//	Load effects

	CXMLElement *pEffects = pDesc->GetContentElementByTag(EFFECTS_TAG);
	if (pEffects)
		{
		if (error = m_Effects.InitFromXML(Ctx, 
				strPatternSubst(CONSTLIT("%d"), GetUNID()), 
				pEffects))
			return ComposeLoadError(Ctx, Ctx.sError);
		}

	//	We also accept a thrust ratio

	if (pDesc->FindAttributeDouble(THRUST_RATIO_ATTRIB, &m_rThrustRatio))
		m_DriveDesc.iThrust = 0;
	else
		{
		m_DriveDesc.iThrust = pDesc->GetAttributeInteger(THRUST_ATTRIB);
		m_rThrustRatio = 0.0;
		}

	m_DriveDesc.iPowerUse = DEFAULT_POWER_USE;
	m_DriveDesc.fInertialess = pDesc->GetAttributeBool(INERTIALESS_DRIVE_ATTRIB);

	if (error = CReactorClass::InitReactorDesc(Ctx, pDesc, &m_ReactorDesc, true))
		return error;

	if ((m_fCyberDefenseOverride = pDesc->FindAttributeInteger(CYBER_DEFENSE_LEVEL_ATTRIB, &m_iCyberDefenseLevel)))
		m_iCyberDefenseLevel = Max(1, m_iCyberDefenseLevel);
	else
		m_iCyberDefenseLevel = 0;

	m_fTimeStopImmune = pDesc->GetAttributeBool(TIME_STOP_IMMUNE_ATTRIB);

	//	Load armor

	CXMLElement *pArmor = pDesc->GetContentElementByTag(CONSTLIT(g_ArmorTag));
	if (pArmor)
		{
		//	If no content, then we assume a regular distribution of armor

		if (pArmor->GetContentElementCount() == 0)
			{
			int iSegCount = pArmor->GetAttributeIntegerBounded(COUNT_ATTRIB, 1, -1, 4);
			CString sArmorUNID = pArmor->GetAttribute(ARMOR_ID_ATTRIB);

			int iSegSize = Max(1, 360 / iSegCount);

			int iSegPos;
			if (!pArmor->FindAttributeInteger(START_AT_ATTRIB, &iSegPos))
				iSegPos = 360 - (iSegSize / 2);

			m_Hull.InsertEmpty(iSegCount);
			for (i = 0; i < iSegCount; i++)
				{
				HullSection &Section = m_Hull[i];

				Section.iStartAt = AngleMod(iSegPos);
				Section.iSpan = iSegSize;

				if (error = Section.pArmor.LoadUNID(Ctx, sArmorUNID))
					return error;

				Section.dwAreaSet = CShipClass::sectCritical;

				if (error = Section.Enhanced.InitFromXML(Ctx, pArmor))
					return ComposeLoadError(Ctx, Ctx.sError);

				iSegPos += iSegSize;
				}
			}

		//	Otherwise, we load each segment separately.

		else
			{
			m_Hull.InsertEmpty(pArmor->GetContentElementCount());
			for (i = 0; i < pArmor->GetContentElementCount(); i++)
				{
				CXMLElement *pSectionDesc = pArmor->GetContentElement(i);
				HullSection &Section = m_Hull[i];

				Section.iStartAt = pSectionDesc->GetAttributeInteger(CONSTLIT(g_StartAttrib));
				Section.iSpan = pSectionDesc->GetAttributeInteger(CONSTLIT(g_SpanAttrib));

				if (error = Section.pArmor.LoadUNID(Ctx, pSectionDesc->GetAttribute(ARMOR_ID_ATTRIB)))
					return error;

				Section.dwAreaSet = ParseNonCritical(pSectionDesc->GetAttribute(NON_CRITICAL_ATTRIB));

				if (error = Section.Enhanced.InitFromXML(Ctx, pSectionDesc))
					return ComposeLoadError(Ctx, Ctx.sError);
				}
			}
		}

	//	Load devices

	CXMLElement *pDevices = pDesc->GetContentElementByTag(DEVICES_TAG);
	if (pDevices)
		{
		if (error = IDeviceGenerator::CreateFromXML(Ctx, pDevices, &m_pDevices))
			return ComposeLoadError(Ctx, Ctx.sError);
		}

	m_iMaxDevices = pDesc->GetAttributeInteger(MAX_DEVICES_ATTRIB);
	if (m_iMaxDevices == 0)
		m_iMaxDevices = -1;
	m_iMaxWeapons = pDesc->GetAttributeInteger(MAX_WEAPONS_ATTRIB);
	if (m_iMaxWeapons == 0)
		m_iMaxWeapons = m_iMaxDevices;
	m_iMaxNonWeapons = pDesc->GetAttributeInteger(MAX_NON_WEAPONS_ATTRIB);
	if (m_iMaxNonWeapons == 0)
		m_iMaxNonWeapons = m_iMaxDevices;

	//	Load interior structure

	CXMLElement *pInterior = pDesc->GetContentElementByTag(INTERIOR_TAG);
	if (pInterior)
		{
		if (error = m_Interior.InitFromXML(Ctx, pInterior))
			return ComposeLoadError(Ctx, Ctx.sError);
		}

	//	Load AI settings

	CXMLElement *pAISettings = pDesc->GetContentElementByTag(CONSTLIT(g_AISettingsTag));
	if (pAISettings)
		{
		if (error = m_AISettings.InitFromXML(Ctx, pAISettings))
			return ComposeLoadError(Ctx, Ctx.sError);
		}
	else
		m_AISettings.InitToDefault();

	//	Load items

	CXMLElement *pItems = pDesc->GetContentElementByTag(ITEMS_TAG);
	if (pItems)
		{
		if (error = IItemGenerator::CreateFromXML(Ctx, pItems, &m_pItems))
			return ComposeLoadError(Ctx, Ctx.sError);
		}

	//	Load equipment

	CXMLElement *pEquipment = pDesc->GetContentElementByTag(EQUIPMENT_TAG);
	if (pEquipment)
		{
		for (i = 0; i < pEquipment->GetContentElementCount(); i++)
			{
			CXMLElement *pLine = pEquipment->GetContentElement(i);

			//	Allocate a descriptor

			SEquipmentDesc *pDesc = m_Equipment.Insert();

			//	Figure out what equipment we're talking about

			pDesc->iEquipment = AbilityDecode(pLine->GetAttribute(EQUIPMENT_ATTRIB));
			if (pDesc->iEquipment == ::ablUnknown)
				return ComposeLoadError(Ctx, strPatternSubst(ERR_UNKNOWN_EQUIPMENT, pLine->GetAttribute(EQUIPMENT_ATTRIB)));

			//	Figure out what to do with the equipment

			if (strEquals(pLine->GetTag(), INSTALL_TAG))
				pDesc->bInstall = true;
			else if (strEquals(pLine->GetTag(), REMOVE_TAG))
				pDesc->bInstall = false;
			else
				return ComposeLoadError(Ctx, strPatternSubst(ERR_UNKNOWN_EQUIPMENT_DIRECTIVE, pLine->GetTag()));
			}
		}

	//	Drive images

	CXMLElement *pDriveImages = pDesc->GetContentElementByTag(DRIVE_IMAGES_TAG);
	if (pDriveImages)
		{
		int iScale = m_Image.GetImageViewportSize();

		for (i = 0; i < pDriveImages->GetContentElementCount(); i++)
			{
			CXMLElement *pItem = pDriveImages->GetContentElement(i);
			if (strEquals(pItem->GetTag(), NOZZLE_IMAGE_TAG))
				{
				if (error = m_ExhaustImage.InitFromXML(Ctx, pItem))
					return ComposeLoadError(Ctx, ERR_BAD_EXHAUST_IMAGE);
				}
			else if (strEquals(pItem->GetTag(), NOZZLE_POS_TAG))
				{
				//	Initialize structure

				SExhaustDesc *pExhaust = m_Exhaust.Insert();

				//	Load the position

				if (error = pExhaust->PosCalc.Init(pItem))
					return ComposeLoadError(Ctx, ERR_DRIVE_IMAGE_FORMAT);
				}
			else
				return ComposeLoadError(Ctx, ERR_DRIVE_IMAGE_FORMAT);
			}
		}

	//	Escorts

	CXMLElement *pEscorts = pDesc->GetContentElementByTag(ESCORTS_TAG);
	if (pEscorts)
		{
		if (error = IShipGenerator::CreateFromXMLAsGroup(Ctx, pEscorts, &m_pEscorts))
			return ComposeLoadError(Ctx, Ctx.sError);
		}
	else
		m_pEscorts = NULL;

	//	Characters

	if (error = m_Character.LoadUNID(Ctx, pDesc->GetAttribute(CHARACTER_ATTRIB)))
		return error;

	if (error = m_CharacterClass.LoadUNID(Ctx, pDesc->GetAttribute(CHARACTER_CLASS_ATTRIB)))
		return error;

	//	Initialize docking data

	CDockingPorts DockingPorts;
	DockingPorts.InitPortsFromXML(NULL, pDesc);
	m_DockingPorts.InsertEmpty(DockingPorts.GetPortCount(NULL));
	if (m_DockingPorts.GetCount() > 0)
		{
		for (i = 0; i < m_DockingPorts.GetCount(); i++)
			m_DockingPorts[i] = DockingPorts.GetPortPos(NULL, i, NULL);

		//	Load the default screen

		m_pDefaultScreen.LoadUNID(Ctx, pDesc->GetAttribute(DOCK_SCREEN_ATTRIB));
		if (m_pDefaultScreen.GetUNID().IsBlank())
			return ComposeLoadError(Ctx, ERR_DOCK_SCREEN_NEEDED);

		//	Background screens

		if (error = LoadUNID(Ctx, pDesc->GetAttribute(DEFAULT_BACKGROUND_ID_ATTRIB), &m_dwDefaultBkgnd))
			return error;

		m_fHasDockingPorts = true;
		}
	else
		{
		m_dwDefaultBkgnd = 0;
		m_fHasDockingPorts = false;
		}

	//	Load trade

	CXMLElement *pTrade = pDesc->GetContentElementByTag(TRADE_TAG);
	if (pTrade)
		{
		if (error = CTradingDesc::CreateFromXML(Ctx, pTrade, &m_pTrade))
			return error;
		}
	else
		m_pTrade = NULL;

	//	Load communications

	CXMLElement *pComms = pDesc->GetContentElementByTag(COMMUNICATIONS_TAG);
	if (pComms)
		if (error = m_OriginalCommsHandler.InitFromXML(pComms, &Ctx.sError))
			return ComposeLoadError(Ctx, Ctx.sError);

	m_fCommsHandlerInit = false;

	//	Miscellaneous

	if (pDesc->FindAttributeInteger(LEAVES_WRECK_ATTRIB, &m_iLeavesWreck))
		m_iLeavesWreck = Max(0, m_iLeavesWreck);
	else
		{
		//	Chance of wreck is a function of mass:
		//
		//	prob = 5 * MASS^0.45

		m_iLeavesWreck = Max(0, Min((int)(5.0 * pow((Metric)m_iMass, 0.45)), 100));
		}

	if (error = m_pWreckType.LoadUNID(Ctx, pDesc->GetAttribute(WRECK_TYPE_ATTRIB)))
		return error;

	m_fRadioactiveWreck = pDesc->GetAttributeBool(RADIOACTIVE_WRECK_ATTRIB);
	m_iStructuralHP = pDesc->GetAttributeIntegerBounded(STRUCTURAL_HIT_POINTS_ATTRIB, 0, -1, -1);
	if (m_iStructuralHP == -1)
		m_iStructuralHP = pDesc->GetAttributeIntegerBounded(MAX_STRUCTURAL_HIT_POINTS_ATTRIB, 0, -1, 0);

	//	Explosion

	if (error = m_pExplosionType.LoadUNID(Ctx, pDesc->GetAttribute(EXPLOSION_TYPE_ATTRIB)))
		return error;

	//	Load player settings

	CXMLElement *pPlayer = pDesc->GetContentElementByTag(PLAYER_SETTINGS_TAG);
	if (pPlayer)
		{
		m_pPlayerSettings = new CPlayerSettings;
		if (error = m_pPlayerSettings->InitFromXML(Ctx, this, pPlayer))
			return ComposeLoadError(Ctx, Ctx.sError);
		}

	//	Done

	return NOERROR;
	}

CEffectCreator *CShipClass::OnFindEffectCreator (const CString &sUNID)

//	OnFindEffectCreator
//
//	Find the effect creator. sUNID is the remainder after the item type UNID has been removed
//
//	{unid}:p:s		Player settings shield effect
//	      ^

	{
	//	We start after the class UNID

	char *pPos = sUNID.GetASCIIZPointer();
	if (*pPos != ':')
		return NULL;
	
	pPos++;

	//	Figure out what

	switch (*pPos)
		{
		case 'p':
			if (m_pPlayerSettings == NULL)
				return NULL;

			return m_pPlayerSettings->FindEffectCreator(CString(pPos + 1));

		default:
			return NULL;
		}
	}

bool CShipClass::OnHasSpecialAttribute (const CString &sAttrib) const

//	OnHasSpecialAttribute
//
//	Returns TRUE if we have the special attribute

	{
	if (strStartsWith(sAttrib, SPECIAL_IS_PLAYER_CLASS))
		{
		CString sValue = strSubString(sAttrib, SPECIAL_IS_PLAYER_CLASS.GetLength());
		if (strEquals(sValue, SPECIAL_VALUE_TRUE))
			return (GetPlayerSettings() != NULL);
		else
			return (GetPlayerSettings() == NULL);
		}
	else if (strStartsWith(sAttrib, SPECIAL_MANUFACTURER))
		{
		CString sValue = strSubString(sAttrib, SPECIAL_MANUFACTURER.GetLength());
		return strEquals(sValue, GetManufacturerName());
		}
	else
		return false;
	}

void CShipClass::OnMergeType (CDesignType *pSource)

//	OnMergeType
//
//	Merges the definitions from pSource into our class

	{
	CShipClass *pClass = CShipClass::AsType(pSource);
	if (pClass == NULL)
		{
		ASSERT(false);
		return;
		}

	//	Merge image

	if (!pClass->m_Image.IsEmpty())
		m_Image = pClass->m_Image;

	//	Merge player settings

	if (pClass->m_pPlayerSettings)
		{
		if (m_pPlayerSettings == NULL)
			{
			m_pPlayerSettings = new CPlayerSettings;
			*m_pPlayerSettings = *pClass->m_pPlayerSettings;
			}
		else if (m_fInheritedPlayerSettings)
			{
			CPlayerSettings *pNew = new CPlayerSettings;
			*pNew = *m_pPlayerSettings;

			pNew->MergeFrom(*pClass->m_pPlayerSettings);

			m_pPlayerSettings = pNew;
			}
		else
			m_pPlayerSettings->MergeFrom(*pClass->m_pPlayerSettings);

		m_fInheritedPlayerSettings = false;
		}

	//	Comms handler
	//	LATER: Currently only adds new handlers; does not replace existing ones.

	if (pClass->m_OriginalCommsHandler.GetCount() > 0)
		m_OriginalCommsHandler.Merge(pClass->m_OriginalCommsHandler);
	}

void CShipClass::OnReadFromStream (SUniverseLoadCtx &Ctx)

//	ReadFromStream
//
//	Read from stream
//
//	DWORD		flags
//	DWORD		ship name indices count
//	DWORD[]		ship name index
//	DWORD		m_iShipName

	{
	int i;
	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));

	//	Load opaque data (for previous versions)

	ReadGlobalData(Ctx);

	//	Load random ship name indices

	if (Ctx.dwVersion >= 4)
		{
		int iCount;
		Ctx.pStream->Read((char *)&iCount, sizeof(DWORD));
		m_ShipNamesIndices.DeleteAll();
		m_ShipNamesIndices.InsertEmpty(iCount);
		for (i = 0; i < iCount; i++)
			{
			Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
			m_ShipNamesIndices[i] = dwLoad;
			}

		Ctx.pStream->Read((char *)&m_iShipName, sizeof(DWORD));
		}
	else
		{
		InitShipNamesIndices();
		}
	}

void CShipClass::OnReinit (void)

//	OnReinit
//
//	Reinitialize the class

	{
	InitShipNamesIndices();
	m_WreckImage.CleanUp();
	}

void CShipClass::OnUnbindDesign (void)

//	OnUnbindDesign
//
//	Undo binding

	{
	//	Undo inheritance

	if (m_fInheritedPlayerSettings)
		{
		m_fInheritedPlayerSettings = false;
		m_pPlayerSettings = NULL;
		}

	//	Reset comms handler because our inheritance chain might
	//	have changed.

	m_fCommsHandlerInit = false;
	m_CommsHandler.DeleteAll();
	}

void CShipClass::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes out the variable portions of the item type
//
//	DWORD		flags
//	DWORD		ship name indices count
//	DWORD[]		ship name index
//	DWORD		m_iShipName

	{
	int i;
	DWORD dwSave = 0;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Write out ship name indices

	dwSave = m_ShipNamesIndices.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < m_ShipNamesIndices.GetCount(); i++)
		{
		dwSave = (DWORD)m_ShipNamesIndices[i];
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}

	pStream->Write((char *)&m_iShipName, sizeof(DWORD));
	}

void CShipClass::Paint (CG16bitImage &Dest, 
						int x, 
						int y, 
						const ViewportTransform &Trans, 
						int iDirection, 
						int iTick,
						bool bThrusting,
						bool bRadioactive,
						DWORD byInvisible)

//	Paint
//
//	Paints the ship class

	{
	//	If we're facing down paint the thrust first.

	if (bThrusting)
		PaintThrust(Dest, x, y, Trans, iDirection, iTick, true /* bInFrontOnly */);

	//	Paint the body of the ship

	if (byInvisible)
		m_Image.PaintImageShimmering(Dest, x, y, iTick, iDirection, byInvisible);
	else if (bRadioactive)
		m_Image.PaintImageWithGlow(Dest, x, y, iTick, iDirection, RGB(0, 255, 0));
	else
		m_Image.PaintImage(Dest, x, y, iTick, iDirection);

	//	If we need to paint the thrust (because we didn't earlier) do it now.

	if (bThrusting)
		PaintThrust(Dest, x, y, Trans, iDirection, iTick, false /* bInFrontOnly */);

#ifdef DEBUG_3D_ADJ
	{
	int i;
	int iScale = m_Image.GetImageViewportSize();
	for (i = 0; i < 360; i++)
		{
		int xP, yP;
		IntPolarToVector(i, 24, &xP, &yP);
		Dest.DrawPixel(x + xP, y + yP, CG16bitImage::RGBValue(0, 0, 255));

		C3DConversion::CalcCoord(iScale, i, 8, 0, &xP, &yP);
		Dest.DrawPixel(x + xP, y + yP, CG16bitImage::RGBValue(255, 255, 0));

		C3DConversion::CalcCoord(iScale, i, 16, 0, &xP, &yP);
		Dest.DrawPixel(x + xP, y + yP, CG16bitImage::RGBValue(255, 255, 0));

		C3DConversion::CalcCoord(iScale, i, 24, 0, &xP, &yP);
		Dest.DrawPixel(x + xP, y + yP, CG16bitImage::RGBValue(255, 255, 0));
		}
	}
#endif
	}

void CShipClass::PaintMap (CMapViewportCtx &Ctx, 
						CG16bitImage &Dest, 
						int x, 
						int y, 
						int iDirection, 
						int iTick,
						bool bThrusting,
						bool bRadioactive)

//	PaintMap
//
//	Paints the ship class on the map

	{
	m_Image.PaintScaledImage(Dest, x, y, iTick, iDirection, 24, 24);
	}

void CShipClass::PaintThrust (CG16bitImage &Dest, 
							  int x, 
							  int y, 
							  const ViewportTransform &Trans, 
							  int iDirection, 
							  int iTick,
							  bool bInFrontOnly)

//	PaintThrust
//
//	Paints the thrust effect

	{
	int i;

	for (i = 0; i < m_Exhaust.GetCount(); i++)
		{
		if (m_Exhaust[i].PosCalc.PaintFirst(iDirection) == bInFrontOnly)
			{
			int xThrust;
			int yThrust;

			m_Exhaust[i].PosCalc.GetCoordFromDir(iDirection, &xThrust, &yThrust);
			m_ExhaustImage.PaintImage(Dest,
					x + xThrust,
					y + yThrust,
					iTick,
					iDirection);
			}
		}
	}

DWORD ParseNonCritical (const CString &sList)

//	ParseNonCritical
//
//	Returns the set of non-critical areas

	{
	//	These must match the order of VitalSections in TSE.h

	static char *g_pszNonCritical[] =
		{
		"dev0",
		"dev1",
		"dev2",
		"dev3",
		"dev4",
		"dev5",
		"dev6",
		"dev7",

		"maneuver",
		"drive",
		"scanners",
		"tactical",
		"cargo",

		"",
		};

	//	Blank means critical

	if (sList.IsBlank())
		return CShipClass::sectCritical;

	//	Loop over list

	DWORD dwSet = 0;
	char *pPos = sList.GetASCIIZPointer();
	while (*pPos != '\0')
		{
		//	Trim spaces

		while (*pPos != '\0' && *pPos == ' ')
			pPos++;

		//	Which of the items do we match?

		int i = 0;
		DWORD dwArea = 0x1;
		char *pFind;
		while (*(pFind = g_pszNonCritical[i]))
			{
			char *pSource = pPos;

			while (*pFind != '\0' && *pFind == *pSource)
				{
				pFind++;
				pSource++;
				}

			//	If we matched then we've got this area

			if (*pFind == '\0' && (*pSource == ' ' || *pSource == ';' || *pSource == '\0'))
				{
				dwSet |= dwArea;
				pPos = pSource;
				break;
				}

			//	Next

			i++;
			dwArea = dwArea << 1;
			}

		//	Skip to the next modifier

		while (*pPos != '\0' && *pPos != ';')
			pPos++;

		if (*pPos == ';')
			pPos++;
		}

	//	Done

	return dwSet;
	}
