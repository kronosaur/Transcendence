//	CShipClass.cpp
//
//	CShipClass class

#include "PreComp.h"

#define ARMOR_DISPLAY_TAG						CONSTLIT("ArmorDisplay")
#define COMMUNICATIONS_TAG						CONSTLIT("Communications")
#define DEVICES_TAG								CONSTLIT("Devices")
#define DOCK_SCREENS_TAG						CONSTLIT("DockScreens")
#define DRIVE_IMAGES_TAG						CONSTLIT("DriveImages")
#define EQUIPMENT_TAG							CONSTLIT("Equipment")
#define ESCORTS_TAG								CONSTLIT("Escorts")
#define EVENTS_TAG								CONSTLIT("Events")
#define FUEL_LEVEL_IMAGE_TAG					CONSTLIT("FuelLevelImage")
#define FUEL_LEVEL_TEXT_TAG						CONSTLIT("FuelLevelText")
#define FUEL_LOW_LEVEL_IMAGE_TAG				CONSTLIT("FuelLowLevelImage")
#define IMAGE_TAG								CONSTLIT("Image")
#define INSTALL_TAG								CONSTLIT("Install")
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
#define WRECK_IMAGE_TAG							CONSTLIT("WreckImage")

#define AUTOPILOT_ATTRIB						CONSTLIT("autopilot")
#define ARMOR_ID_ATTRIB							CONSTLIT("armorID")
#define CARGO_SPACE_ATTRIB						CONSTLIT("cargoSpace")
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
#define INITIAL_CLASS_ATTRIB					CONSTLIT("initialClass")
#define LARGE_IMAGE_ATTRIB						CONSTLIT("largeImage")
#define LEAVES_WRECK_ATTRIB						CONSTLIT("leavesWreck")
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
#define SHIP_SCREEN_ATTRIB						CONSTLIT("shipScreen")
#define STARTING_CREDITS_ATTRIB					CONSTLIT("startingCredits")
#define STARTING_POS_ATTRIB						CONSTLIT("startingPos")
#define STARTING_SYSTEM_ATTRIB					CONSTLIT("startingSystem")
#define STRUCTURAL_HIT_POINTS_ATTRIB			CONSTLIT("structuralHitPoints")
#define TIME_STOP_IMMUNE_ATTRIB					CONSTLIT("timeStopImmune")
#define UNID_ATTRIB								CONSTLIT("UNID")
#define VIRTUAL_ATTRIB							CONSTLIT("virtual")
#define WIDTH_ATTRIB							CONSTLIT("width")
#define WRECK_TYPE_ATTRIB						CONSTLIT("wreckType")
#define X_ATTRIB								CONSTLIT("x")
#define Y_ATTRIB								CONSTLIT("y")

#define FIELD_FUEL_CAPACITY						CONSTLIT("fuelCapacity")
#define FIELD_FUEL_CRITERIA						CONSTLIT("fuelCriteria")
#define FIELD_FUEL_EFFICIENCY					CONSTLIT("fuelEfficiency")
#define FIELD_POWER								CONSTLIT("power")

#define FIELD_ARMOR_ITEMS						CONSTLIT("armorItems")
#define FIELD_CARGO_SPACE						CONSTLIT("cargoSpace")
#define FIELD_DEVICE_SLOTS						CONSTLIT("deviceSlots")
#define FIELD_DEVICE_SLOTS_NON_WEAPONS			CONSTLIT("deviceSlotsNonWeapons")
#define FIELD_DEVICE_SLOTS_WEAPONS				CONSTLIT("deviceSlotsWeapons")
#define FIELD_DEVICE_ITEMS						CONSTLIT("deviceItems")
#define FIELD_DRIVE_IMAGE						CONSTLIT("driveImage")
#define FIELD_EXPLOSION_TYPE					CONSTLIT("explosionType")
#define FIELD_FIRE_ACCURACY						CONSTLIT("fireAccuracy")
#define FIELD_FIRE_RANGE_ADJ					CONSTLIT("fireRangeAdj")
#define FIELD_FIRE_RATE_ADJ						CONSTLIT("fireRateAdj")
#define FIELD_GENERIC_NAME						CONSTLIT("genericName")
#define FIELD_HULL_MASS							CONSTLIT("hullMass")
#define FIELD_LAUNCHER							CONSTLIT("launcher")
#define FIELD_LAUNCHER_UNID						CONSTLIT("launcherUNID")
#define FIELD_LEVEL								CONSTLIT("level")
#define FIELD_MAX_ARMOR_MASS					CONSTLIT("maxArmorMass")
#define FIELD_MAX_CARGO_SPACE					CONSTLIT("maxCargoSpace")
#define FIELD_MAX_SPEED							CONSTLIT("maxSpeed")
#define FIELD_MANEUVER							CONSTLIT("maneuver")
#define FIELD_MANUFACTURER						CONSTLIT("manufacturer")
#define FIELD_NAME								CONSTLIT("name")
#define FIELD_PRIMARY_ARMOR						CONSTLIT("primaryArmor")
#define FIELD_PRIMARY_ARMOR_UNID				CONSTLIT("primaryArmorUNID")
#define FIELD_PRIMARY_WEAPON					CONSTLIT("primaryWeapon")
#define FIELD_PRIMARY_WEAPON_RANGE				CONSTLIT("primaryWeaponRange")
#define FIELD_PRIMARY_WEAPON_RANGE_ADJ			CONSTLIT("primaryWeaponRangeAdj")
#define FIELD_PRIMARY_WEAPON_UNID				CONSTLIT("primaryWeaponUNID")
#define FIELD_SCORE								CONSTLIT("score")
#define FIELD_SHIELD							CONSTLIT("shield")
#define FIELD_SHIELD_UNID						CONSTLIT("shieldsUNID")
#define FIELD_SHIP_STATUS_SCREEN				CONSTLIT("shipStatusScreen")
#define FIELD_THRUST_TO_WEIGHT					CONSTLIT("thrustToWeight")

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

#define SPECIAL_VALUE_TRUE						CONSTLIT("true")

#define WRECK_IMAGE_VARIANTS					3
#define DAMAGE_IMAGE_COUNT						10
#define DAMAGE_IMAGE_WIDTH						24
#define DAMAGE_IMAGE_HEIGHT						24

#define DEFAULT_POWER_USE						20

static char g_ArmorTag[] = "Armor";
static char g_ArmorSectionTag[] = "ArmorSection";
static char g_AISettingsTag[] = "AISettings";

static char g_ManufacturerAttrib[] = "manufacturer";
static char g_ClassAttrib[] = "class";
static char g_TypeAttrib[] = "type";
static char g_MassAttrib[] = "mass";
static char g_ThrustAttrib[] = "thrust";
static char g_ManeuverAttrib[] = "maneuver";
static char g_MaxSpeedAttrib[] = "maxSpeed";
static char g_StartAttrib[] = "start";
static char g_SpanAttrib[] = "span";
static char g_DeviceIDAttrib[] = "deviceID";

static char g_FireRateAttrib[] = "fireRate";

static CG16bitImage *g_pDamageBitmap = NULL;
static CStationType *g_pWreckDesc = NULL;

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

		{	20,		5,		20,		0,		50 },			//	I
		{	50,		10,		50,		-5,		100 },			//	II
		{	115,	15,		100,	-10,	200 },			//	III
		{	200,	20,		170,	-20,	350 },			//	IV
		{	340,	30,		260,	-35,	600 },			//	V
		{	500,	45,		370,	-50,	900 },			//	VI
		{	750,	60,		500,	-65,	1400 },			//	VII
		{	1050,	80,		650,	-85,	1900 },			//	VIII
		{	1450,	100,	820,	-105,	2600 },			//	IX
		{	1900,	125,	1010,	-130,	3250 },			//	X
		{	2400,	150,	1220,	-155,	4200 },			//	XI
		{	3000,	180,	1450,	-185,	5500 },			//	XII
		{	3600,	210,	1700,	-215,	6750 },			//	XIII
		{	4250,	245,	1970,	-250,	8250 },			//	XIV
		{	5000,	280,	2260,	-285,	10000 },		//	XV
		{	6000,	320,	2570,	-325,	11500 },		//	XVI
		{	7000,	360,	2900,	-365,	13250 },		//	XVII
		{	8000,	405,	3250,	-410,	15000 },		//	XVIII
		{	9000,	450,	3620,	-455,	16750 },		//	XIX
		{	10000,	500,	4010,	-505,	18500 },		//	XX
		{	11000,	550,	4420,	-555,	20500 },		//	XXI
		{	12000,	605,	4850,	-610,	22500 },		//	XXII
		{	13000,	660,	5300,	-665,	25000 },		//	XXIII
		{	14000,	720,	5770,	-725,	26500 },		//	XXIV
		{	15000,	780,	6260,	-785,	30000 },		//	XXV
	};

#define SCORE_DESC_COUNT							(sizeof(g_XP) / sizeof(g_XP[0]))

CShipClass::CShipClass (void) : 
		m_Hull(sizeof(HullSection), 2),
		m_pDevices(NULL),
		m_pPlayerSettings(NULL),
		m_pItems(NULL),
		m_pEscorts(NULL),
		m_DockingPorts(NULL)

//	CShipClass constructor

	{
	}

CShipClass::~CShipClass (void)

//	CShip destructor

	{
	if (m_pDevices)
		delete m_pDevices;

	if (m_pPlayerSettings && !m_fInheritedPlayerSettings)
		delete m_pPlayerSettings;

	if (m_pItems)
		delete m_pItems;

	if (m_pEscorts)
		delete m_pEscorts;

	if (m_DockingPorts)
		delete [] m_DockingPorts;
	}

Metric CShipClass::CalcMass (const CDeviceDescList &Devices)

//	CalcMass
//
//	Returns the total mass of the ship class, including devices and armor

	{
	int i;
	Metric rMass = GetHullMass();

	for (i = 0; i < GetHullSectionCount(); i++)
		rMass += GetHullSection(i)->pArmor->GetMass();

	for (i = 0; i < Devices.GetCount(); i++)
		rMass += Devices.GetDeviceClass(i)->GetMass();

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

int CShipClass::ComputeDeviceLevel (const SDeviceDesc &Device)

//	ComputeDeviceLevel
//
//	Computes the level of the given device

	{
	CDeviceClass *pDevice = Device.Item.GetType()->GetDeviceClass();
	if (pDevice == NULL)
		return 0;

	if (pDevice->GetCategory() == itemcatLauncher)
		{
		//int iLevel = pDevice->GetItemType()->GetLevel();
		int iLevel = 0;

		//	Look for the highest missile level and use that as the
		//	weapon level.

		IItemGenerator *pItems = GetRandomItemTable();
		if (pItems)
			FindBestMissile(pItems, &iLevel);

		FindBestMissile(Device.ExtraItems, &iLevel);

		return iLevel;
		}
	else
		return pDevice->GetLevel();
	}


int CShipClass::ComputeLevel (int iScore)

//	ComputeLevel
//
//	Returns the level of the ship class

	{
	int i = 0;
	while (i < SCORE_DESC_COUNT-1 && g_XP[i].iScoreLevel < iScore)
		i++;

	return i+1;
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

	if (GetManeuverability() >= 7)
		*retiManeuver = enumLow;
	else if (GetManeuverability() > 2)
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
	//	Create the wreck

	CStation *pWreck;
	CStation::CreateFromType(pSystem,
			GetWreckDesc(),
			vPos,
			vVel,
			NULL,
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
	//	Explosion effect and damage

	SExplosionType Explosion;
	pShip->FireGetExplosionType(&Explosion);
	if (Explosion.pDesc == NULL)
		Explosion.pDesc = GetExplosionType();

	if (Explosion.pDesc)
		{
		pShip->GetSystem()->CreateWeaponFire(Explosion.pDesc,
				Explosion.iBonus,
				Explosion.iCause,
				CDamageSource(pShip, Explosion.iCause, pWreck),
				pShip->GetPos(),
				pShip->GetVel(),
				0,
				NULL,
				CSystem::CWF_EXPLOSION,
				NULL);
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
	}

void CShipClass::CreateWreck (CShip *pShip, CSpaceObject **retpWreck)

//	CreateWreck
//
//	Creates a wreck for the given ship

	{
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
						Mods.Combine(etLoseEnhancement);
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
	}

void CShipClass::CreateWreckImage (void)

//	CreateWreckImage
//
//	Creates a wreck image randomly

	{
	ASSERT(!m_Image.IsEmpty());
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

		g_pDamageBitmap = pDamageImage->GetImage();
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
				mathRandom(0, m_iRotationRange - 1));

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
		m_WreckBitmap.SetTransparentColor(0x0000);

	//	Initialize an image

	RECT rcRect;
	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = cxWidth;
	rcRect.bottom = cyHeight;
	m_WreckImage.Init(&m_WreckBitmap, rcRect, 0, 0, false);
	}

void CShipClass::FindBestMissile (IItemGenerator *pItems, int *ioLevel)

//	FindBestMissile
//
//	Finds the best missile in the table

	{
	int i;

	for (i = 0; i < pItems->GetItemTypeCount(); i++)
		{
		CItemType *pType = pItems->GetItemType(i);
		if (pType->GetCategory() == itemcatMissile
				&& pType->GetLevel() > *ioLevel)
			*ioLevel = pType->GetLevel();
		}

	//	Recurse

	for (i = 0; i < pItems->GetGeneratorCount(); i++)
		FindBestMissile(pItems->GetGenerator(i), ioLevel);
	}

void CShipClass::FindBestMissile (const CItemList &Items, int *ioLevel)

//	FindBestMissile
//
//	Finds the best missile in the item list

	{
	int i;

	for (i = 0; i < Items.GetCount(); i++)
		{
		CItemType *pType = Items.GetItem(i).GetType();
		if (pType->GetCategory() == itemcatMissile
				&& pType->GetLevel() > *ioLevel)
			*ioLevel = pType->GetLevel();
		}
	}

bool CShipClass::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	int i;

	if (strEquals(sField, FIELD_CARGO_SPACE))
		*retsValue = strFromInt(GetCargoSpace());
	else if (strEquals(sField, FIELD_MAX_SPEED))
		{
		DriveDesc Desc;
		GetDriveDesc(&Desc);
		*retsValue = strFromInt((int)((100.0 * Desc.rMaxSpeed / LIGHT_SPEED) + 0.5), FALSE);
		}
	else if (strEquals(sField, FIELD_NAME))
		*retsValue = GetNounPhrase(0x00);
	else if (strEquals(sField, FIELD_MANUFACTURER))
		*retsValue = m_sManufacturer;
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
	else if (strEquals(sField, FIELD_THRUST_TO_WEIGHT))
		{
		DriveDesc Drive;
		GetDriveDesc(&Drive);

		Metric rMass = CalcMass(m_AverageDevices);
		int iRatio = (int)((200.0 * (rMass > 0.0 ? Drive.iThrust / rMass : 0.0)) + 0.5);
		*retsValue = strFromInt(10 * iRatio);
		}
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
	else if (strEquals(sField, FIELD_FUEL_CAPACITY))
		*retsValue = strFromInt(m_ReactorDesc.iMaxFuel / FUEL_UNITS_PER_STD_ROD);
	else if (strEquals(sField, FIELD_FUEL_CRITERIA))
		*retsValue = strPatternSubst(CONSTLIT("f L:%d-%d;"), m_ReactorDesc.iMinFuelLevel, m_ReactorDesc.iMaxFuelLevel);
	else if (strEquals(sField, FIELD_FUEL_EFFICIENCY))
		*retsValue = strFromInt(m_ReactorDesc.iPowerPerFuelUnit);
	else if (strEquals(sField, FIELD_POWER))
		*retsValue = strFromInt(m_ReactorDesc.iMaxPower * 100);
	else if (strEquals(sField, FIELD_DRIVE_IMAGE))
		{
		if (m_Exhaust.GetCount() == 0)
			*retsValue = CONSTLIT("none");
		else
			*retsValue = CONSTLIT("Image");
		}
	else
		return CDesignType::FindDataField(sField, retsValue);

	return true;
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
					m_ShipNamesIndices.GetElement(m_iShipName++ % m_ShipNamesIndices.GetCount()));

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

void CShipClass::GetDriveDesc (DriveDesc *retDriveDesc)

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
		return strPatternSubst(CONSTLIT("%s %s"), GetManufacturerName(), GetTypeName());
	else
		{
		if (GetTypeName().IsBlank())
			return GetClassName();
		else
			return strPatternSubst(CONSTLIT("%s-class %s"), GetClassName(), GetTypeName());
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
		return strPatternSubst(LITERAL("%s %s"), GetManufacturerName(), GetTypeName());
	else
		return GetClassName();
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

void CShipClass::InitShipNamesIndices (void)

//	InitShipNamesIndices
//
//	Initializes m_ShipNamesIndices and m_iShipName from m_sShipNames

	{
	int i;
	int iCount = strDelimitCount(m_sShipNames, ';', DELIMIT_TRIM_WHITESPACE);

	m_ShipNamesIndices.RemoveAll();
	for (i = 0; i < iCount; i++)
		m_ShipNamesIndices.AppendElement(i);

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

void CShipClass::LoadImages (void)

//	LoadImages
//
//	Loads images used by the ship

	{
	m_Image.LoadImage();

	if (m_pExplosionType)
		m_pExplosionType->LoadImages();
	}

void CShipClass::MarkImages (void)

//	MarkImages
//
//	Marks images used by the ship

	{
	m_Image.MarkImage();

	if (m_pExplosionType)
		m_pExplosionType->MarkImages();
	}

ALERROR CShipClass::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	ALERROR error;
	int i;

	if (error = m_Image.OnDesignLoadComplete(Ctx))
		goto Fail;

	if (error = m_ExhaustImage.OnDesignLoadComplete(Ctx))
		goto Fail;

	if (error = m_pDefaultScreen.Bind(Ctx, GetLocalScreens()))
		goto Fail;

	for (i = 0; i < GetHullSectionCount(); i++)
		if (error = GetHullSection(i)->pArmor.Bind(Ctx))
			goto Fail;

	if (error = m_pExplosionType.Bind(Ctx))
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

	//	If we don't have player settings but we have a base class then get the
	//	player settings from the base class

	else if (pBasePlayerSettings = GetPlayerSettingsInherited())
		{
		m_pPlayerSettings = new CPlayerSettings;
		*m_pPlayerSettings = *pBasePlayerSettings;
		}

	//	AI Settings

	if (m_AISettings.GetMinCombatSeparation() < 0.0)
		{
		if (!m_Image.IsEmpty())
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

	if (!m_Image.IsEmpty() && !Ctx.bNoResources)
		CreateWreckImage();

	//	Generate an average set of devices

	GenerateDevices(1, m_AverageDevices);

	//	Events

	m_fHasOnOrderChangedEvent = FindEventHandler(CONSTLIT("OnOrderChanged"));
	m_fHasOnAttackedByPlayerEvent = FindEventHandler(CONSTLIT("OnAttackedByPlayer"));
	m_fHasOnObjDockedEvent = FindEventHandler(CONSTLIT("OnObjDocked"));
	m_fHasOnOrdersCompletedEvent = FindEventHandler(CONSTLIT("OnOrdersCompleted"));

	//	Compute score and level

	if (m_iScore == 0)
		m_iScore = CalcScore();

	m_iLevel = ComputeLevel(m_iScore);

	return NOERROR;

Fail:

	return ComposeLoadError(Ctx, Ctx.sError);
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
	m_iScore = pDesc->GetAttributeInteger(SCORE_ATTRIB);
	m_fVirtual = pDesc->GetAttributeBool(VIRTUAL_ATTRIB);
	m_fInheritedPlayerSettings = false;

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
		if (error = m_Image.InitFromXML(Ctx, pImage))
			return ComposeLoadError(Ctx, Ctx.sError);

	//	Initialize design

	m_iMass = pDesc->GetAttributeInteger(CONSTLIT(g_MassAttrib));
	m_iCargoSpace = pDesc->GetAttributeInteger(CARGO_SPACE_ATTRIB);
	m_iMaxCargoSpace = Max(m_iCargoSpace, pDesc->GetAttributeInteger(MAX_CARGO_SPACE_ATTRIB));
	m_iManeuverability = pDesc->GetAttributeInteger(CONSTLIT(g_ManeuverAttrib));
	m_iManeuverDelay = (int)(((Metric)m_iManeuverability / STD_SECONDS_PER_UPDATE) + 0.5);
	m_iMaxArmorMass = pDesc->GetAttributeInteger(MAX_ARMOR_ATTRIB);
	m_iMaxReactorPower = pDesc->GetAttributeInteger(MAX_REACTOR_POWER_ATTRIB);

	m_iRotationRange = pDesc->GetAttributeInteger(ROTATION_COUNT_ATTRIB);
	if (m_iRotationRange <= 0)
		m_iRotationRange = STD_ROTATION_COUNT;
	m_iRotationAngle = (360 / m_iRotationRange);
	m_Image.SetRotationCount(m_iRotationRange);

	m_DriveDesc.dwUNID = GetUNID();
	m_DriveDesc.rMaxSpeed = (double)pDesc->GetAttributeInteger(CONSTLIT(g_MaxSpeedAttrib)) * LIGHT_SPEED / 100;
	m_DriveDesc.iThrust = pDesc->GetAttributeInteger(CONSTLIT(g_ThrustAttrib));
	m_DriveDesc.iPowerUse = DEFAULT_POWER_USE;
	m_DriveDesc.fInertialess = pDesc->GetAttributeBool(INERTIALESS_DRIVE_ATTRIB);

	m_ReactorDesc.iMaxPower = pDesc->GetAttributeInteger(REACTOR_POWER_ATTRIB);
	if (m_ReactorDesc.iMaxPower == 0)
		m_ReactorDesc.iMaxPower = 100;
	m_ReactorDesc.iMaxFuel = pDesc->GetAttributeInteger(MAX_REACTOR_FUEL_ATTRIB);
	if (m_ReactorDesc.iMaxFuel == 0)
		m_ReactorDesc.iMaxFuel = m_ReactorDesc.iMaxPower * 250;
	m_ReactorDesc.iMinFuelLevel = 1;
	m_ReactorDesc.iMaxFuelLevel = 3;
	m_ReactorDesc.iPowerPerFuelUnit = g_MWPerFuelUnit;
	m_ReactorDesc.fDamaged = false;
	m_ReactorDesc.fEnhanced = false;

	m_iCyberDefenseLevel = Max(1, pDesc->GetAttributeInteger(CYBER_DEFENSE_LEVEL_ATTRIB));

	m_fTimeStopImmune = pDesc->GetAttributeBool(TIME_STOP_IMMUNE_ATTRIB);

	//	Load armor

	CXMLElement *pArmor = pDesc->GetContentElementByTag(CONSTLIT(g_ArmorTag));
	if (pArmor)
		{
		for (i = 0; i < pArmor->GetContentElementCount(); i++)
			{
			CXMLElement *pSectionDesc = pArmor->GetContentElement(i);
			HullSection Section;

			Section.iStartAt = pSectionDesc->GetAttributeInteger(CONSTLIT(g_StartAttrib));
			Section.iSpan = pSectionDesc->GetAttributeInteger(CONSTLIT(g_SpanAttrib));
			Section.pArmor.LoadUNID(Ctx, pSectionDesc->GetAttribute(ARMOR_ID_ATTRIB));
			Section.dwAreaSet = ParseNonCritical(pSectionDesc->GetAttribute(NON_CRITICAL_ATTRIB));

			if (error = Section.Enhanced.InitFromXML(Ctx, pSectionDesc))
				return ComposeLoadError(Ctx, Ctx.sError);

			//	Add the section

			if (error = m_Hull.AppendStruct(&Section, NULL))
				return ComposeLoadError(Ctx, ERR_OUT_OF_MEMORY);
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
		int iScale = RectWidth(m_Image.GetImageRect());

		for (i = 0; i < pDriveImages->GetContentElementCount(); i++)
			{
			CXMLElement *pItem = pDriveImages->GetContentElement(i);
			if (strEquals(pItem->GetTag(), NOZZLE_IMAGE_TAG))
				{
				if (error = m_ExhaustImage.InitFromXML(Ctx, pItem))
					return ComposeLoadError(Ctx, ERR_BAD_EXHAUST_IMAGE);

				m_ExhaustImage.SetRotationCount(m_iRotationRange);
				}
			else if (strEquals(pItem->GetTag(), NOZZLE_POS_TAG))
				{
				//	Initialize structure

				SExhaustDesc *pExhaust = m_Exhaust.Insert();

				//	Load the position

				if (error = pExhaust->PosCalc.Init(pItem, m_iRotationRange, iScale))
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

	//	Initialize docking data

	CDockingPorts DockingPorts;
	DockingPorts.InitPortsFromXML(NULL, pDesc);
	m_iDockingPortsCount = DockingPorts.GetPortCount(NULL);
	if (m_iDockingPortsCount > 0)
		{
		m_DockingPorts = new CVector [m_iDockingPortsCount];
		for (i = 0; i < m_iDockingPortsCount; i++)
			m_DockingPorts[i] = DockingPorts.GetPortPos(NULL, i);

		//	Load the default screen

		m_pDefaultScreen.LoadUNID(Ctx, pDesc->GetAttribute(DOCK_SCREEN_ATTRIB));
		if (m_pDefaultScreen.GetUNID().IsBlank())
			return ComposeLoadError(Ctx, ERR_DOCK_SCREEN_NEEDED);

		//	Background screens

		m_dwDefaultBkgnd = LoadUNID(Ctx, pDesc->GetAttribute(DEFAULT_BACKGROUND_ID_ATTRIB));

		m_fHasDockingPorts = true;
		}
	else
		{
		m_DockingPorts = NULL;
		m_dwDefaultBkgnd = 0;

		m_fHasDockingPorts = false;
		}

	//	Load communications

	CXMLElement *pComms = pDesc->GetContentElementByTag(COMMUNICATIONS_TAG);
	if (pComms)
		if (error = m_OriginalCommsHandler.InitFromXML(pComms, &Ctx.sError))
			return ComposeLoadError(Ctx, Ctx.sError);

	m_fCommsHandlerInit = false;

	//	Miscellaneous

	m_iLeavesWreck = pDesc->GetAttributeInteger(LEAVES_WRECK_ATTRIB);
	m_pWreckType.LoadUNID(Ctx, pDesc->GetAttribute(WRECK_TYPE_ATTRIB));
	m_fRadioactiveWreck = pDesc->GetAttributeBool(RADIOACTIVE_WRECK_ATTRIB);
	m_iStructuralHP = pDesc->GetAttributeIntegerBounded(STRUCTURAL_HIT_POINTS_ATTRIB, 0, -1, -1);
	if (m_iStructuralHP == -1)
		m_iStructuralHP = pDesc->GetAttributeIntegerBounded(MAX_STRUCTURAL_HIT_POINTS_ATTRIB, 0, -1, 0);

	//	Explosion

	m_pExplosionType.LoadUNID(Ctx, pDesc->GetAttribute(EXPLOSION_TYPE_ATTRIB));

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
	else
		return false;
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
		m_ShipNamesIndices.RemoveAll();

		for (i = 0; i < iCount; i++)
			{
			Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
			m_ShipNamesIndices.AppendElement(dwLoad);
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
	g_pWreckDesc = NULL;

	InitShipNamesIndices();
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
		dwSave = (DWORD)m_ShipNamesIndices.GetElement(i);
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
						bool bRadioactive)

//	Paint
//
//	Paints the ship class

	{
	//	If we're facing down paint the thrust first.

	if (bThrusting)
		PaintThrust(Dest, x, y, Trans, iDirection, iTick, true /* bInFrontOnly */);

	//	Paint the body of the ship

	if (bRadioactive)
		m_Image.PaintImageWithGlow(Dest, x, y, iTick, iDirection, RGB(0, 255, 0));
	else
		m_Image.PaintImage(Dest, x, y, iTick, iDirection);

	//	If we need to paint the thrust (because we didn't earlier) do it now.

	if (bThrusting)
		PaintThrust(Dest, x, y, Trans, iDirection, iTick, false /* bInFrontOnly */);

#ifdef DEBUG_3D_ADJ
	{
	int i;
	int iScale = RectWidth(m_Image.GetImageRect());
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

void CShipClass::PaintMap (CG16bitImage &Dest, 
						int x, 
						int y, 
						const ViewportTransform &Trans, 
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
