//	Help.cpp
//
//	Show help

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define ARMOR_TABLE_SWITCH					CONSTLIT("armortable")
#define CLEAR_REGISTERED_SWITCH				CONSTLIT("clearRegistered")
#define DEBUG_SWITCH						CONSTLIT("debug")
#define ENCOUNTER_SIM_SWITCH				CONSTLIT("encountersim")
#define ENCOUNTER_TABLE_SWITCH				CONSTLIT("encountertable")
#define ENTITIES_SWITCH						CONSTLIT("entities")
#define IMAGES_SWITCH						CONSTLIT("images")
#define ITEM_FREQUENCY_SWITCH				CONSTLIT("itemsim")
#define ITEM_TABLE_SWITCH					CONSTLIT("itemtable")
#define PERF_TEST_SWITCH					CONSTLIT("perftest")
#define RANDOM_ITEMS_SWITCH					CONSTLIT("randomitems")
#define RANDOM_NUMBER_TEST					CONSTLIT("randomnumbertest")
#define RUN_SWITCH							CONSTLIT("run")
#define SHIELD_TEST_SWITCH					CONSTLIT("shieldtest")
#define SHIP_IMAGE_SWITCH					CONSTLIT("shipimage")
#define SHIP_IMAGES_SWITCH					CONSTLIT("shipimages")
#define SHIP_TABLE_SWITCH					CONSTLIT("shiptable")
#define SIM_TABLES_SWITCH					CONSTLIT("simTables")
#define SNAPSHOT_SWITCH						CONSTLIT("snapshot")
#define STATION_FREQUENCY_SWITCH			CONSTLIT("stationfrequency")
#define STATION_PLACE_SIM_SWITCH			CONSTLIT("stationSeparationSim")
#define STATS_SWITCH						CONSTLIT("stats")
#define SYSTEM_LABELS_SWITCH				CONSTLIT("systemlabels")
#define SYSTEM_TEST_SWITCH					CONSTLIT("systemtest")
#define TRADE_SIM_SWITCH					CONSTLIT("tradeSim")
#define TYPE_TABLE_SWITCH					CONSTLIT("typeTable")
#define WEAPON_IMAGES_SWITCH				CONSTLIT("weaponImages")
#define WORD_GENERATOR_SWITCH				CONSTLIT("wordgenerator")
#define WORLD_IMAGES_SWITCH					CONSTLIT("worldImages")

void ShowHelp (CXMLElement *pCmdLine)

//	ShowHelp
//
//	Show help for each command

	{
	bool bDebug = pCmdLine->GetAttributeBool(DEBUG_SWITCH);

	if (pCmdLine->GetAttributeBool(ENCOUNTER_SIM_SWITCH))
		{
		printf("  /encountersim         Simulate an attack on the station.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("      /attackers:xxx        Criteria specifying attacking ships.\n");
		printf("      /defender:xxx         Encounter criteria.\n");
		printf("\n");
		printf("      [/count]              Number of runs per attacker.\n");
		printf("      [/viewer]             Open viewer to watch the battle.\n");
		}
	else if (pCmdLine->GetAttributeBool(CLEAR_REGISTERED_SWITCH))
		{
		printf("  /clearRegistered      Clears registered flag on game file.\n");
		printf("       /gameFile:filename   Save file to clear.\n");
		}
	else if (pCmdLine->GetAttributeBool(ENCOUNTER_TABLE_SWITCH))
		{
		printf("  /encountertable       Encounter table.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("      [/all]                include non-random stations.\n");
		printf("      [/criteria:xxx]       only encounters that match criteria.\n");
		printf("                            e.g., \"+enemy\"\n");
		printf("      [/onlyNotRandom]      Only non-random stations.\n");
		printf("\n");
		printf("      [/abandonedDockScreen] station dock screen when abandoned.\n");
		printf("      [/armorClass]         armor class.\n");
		printf("      [/armorLevel]         level of armor.\n");
		printf("      [/balance]            100 = defenses balanced for level.\n");
		printf("      [/canAttack]          station can attack.\n");
		printf("      [/category]           friendly/enemy/debris.\n");
		printf("      [/countDistribution]  the number appearing in different games.\n");
		printf("      [/defenderStrength]   strength of defenders (relative to level).\n");
		printf("      [/dockScreen]         station dock screen.\n");
		printf("      [/explosionType]      type of explosion when destroyed.\n");
		printf("      [/fireRateAdj]        fire rate adjustment.\n");
		printf("      [/hitsToDestroy]      no. of hits to destroy with weapon of encounter level.\n");
		printf("      [/hp]                 initial hit points.\n");
		printf("      [/maxLightRadius]     light radius for suns (in light-seconds).\n");
		printf("      [/regen]              repair rate in hp per 180 ticks.\n");
		printf("      [/satelliteStrength]  strength of satellites (relative to level).\n");
		printf("      [/size]               size of object (in kilometers).\n");
		printf("      [/totalCount]         total number appearing per game.\n");
		printf("      [/treasureBalance]    treasure relative to defense (100 = balanced).\n");
		printf("      [/treasureValue]      value of average treasure (in credits).\n");
		printf("      [/weaponStrength]     strength of weapons (relative to level).\n");
		}
	else if (pCmdLine->GetAttributeBool(IMAGES_SWITCH))
		{
		printf("  /images               Generates chart of images.\n");
		printf("      /criteria:n           Type criteria of images to show.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("      [/font:s]             Use given font for labels.\n");
		printf("      [/output:file]        Saves image to given filespec.\n");
		printf("      [/sort:xxx]           Sort worlds in given order.\n");
		printf("         largest               Larger objects first.\n");
		printf("         name                  By name.\n");
		printf("         smallest              Smaller objects first.\n");
		printf("      [/textBoxesOnly]      Output textboxes only.\n");
		printf("      [/width:n]            Width of image in pixels.\n");
		printf("      [/xMargin:n]          Margin around images.\n");
		printf("      [/xSpacing:n]         Spacing between images.\n");
		}
	else if (pCmdLine->GetAttributeBool(ITEM_FREQUENCY_SWITCH))
		{
		printf("  /itemsim              Simulation of items encountered.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("       /count:n             n iterations.\n");
		}
	else if (pCmdLine->GetAttributeBool(ITEM_TABLE_SWITCH))
		{
		printf("  /itemtable            Item table.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("      [/byAttribute]        Categorize by attribute.\n");
		printf("      [/byShipClass]        Categorize by ship class using the item.\n");
		printf("      [/byShipClassUsage]   Show ship classes using given item.\n");
		printf("      [/criteria:xxx]       only items that match criteria.\n");
		printf("\n");
		printf("      [/adjustedHP]         hp adjusted for damage type.\n");
		printf("      [/ammoType]           UNID of ammo.\n");
		printf("      [/averageCount]       average number appearing.\n");
		printf("      [/averageDamage]      average damage (in HP).\n");
		printf("      [/balanceDamage]      std damage for level and type (in HP per 180 ticks).\n");
		printf("      [/cost]               cost (in credits).\n");
		printf("      [/damage]             average damage (in HP per 180 ticks).\n");
		printf("      [/damageAdj]          damage type adjustment.\n");
		printf("      [/description]        item description.\n");
		printf("      [/effectiveHP]        effective hit points.\n");
		printf("      [/entity]             XML entity to use.\n");
		printf("      [/fireDelay]          time between shots (in ticks).\n");
		printf("      [/frequency]			frequency found.\n");
		printf("      [/fuelCapacity]       fuel capacity (He3 fuel rod = 1).\n");
		printf("      [/hp]                 hit points.\n");
		printf("      [/hpBonus]            bonus by damage type adjustment.\n");
		printf("      [/installCost]        cost to install (in credits).\n");
		printf("      [/mass]               mass (in kg).\n");
		printf("      [/maxSpeed]           maximum speed (in %% of lightspeed).\n");
		printf("      [/power]              power consumed (in MW).\n");
		printf("      [/powerPerShot]       power per shot (in MW-minutes).\n");
		printf("      [/range]              range of weapon (in light-seconds).\n");
		printf("      [/reference]          reference column.\n");
		printf("      [/regen]              regeneration (in HP per 180 ticks).\n");
		printf("      [/repairCost]         cost to repair 1 hp of damage.\n");
		printf("      [/shieldInterference] True if armor prevents shield use.\n");
		printf("      [/thrust]             thrust (in giganewtons).\n");
		printf("      [/totalCount]         total number appearing per game.\n");
		printf("      [/unid]               unid of item type.\n");
		printf("      [/variantCount]       number of weapon variants.\n");
		printf("      [/weaponSuppress]     shield prevents firing these damage types.\n");
		}
	else if (pCmdLine->GetAttributeBool(PERF_TEST_SWITCH))
		{
		printf("  /perftest             Performance test.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		}
	else if (pCmdLine->GetAttributeBool(RUN_SWITCH))
		{
		printf("  /run:command          Run a TransLisp command.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		}
	else if (pCmdLine->GetAttributeBool(SHIELD_TEST_SWITCH))
		{
		printf("  /shieldtest           Outputs capabilities of shield against weapons.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("       /unid:n              unid or name of shield to test.\n");
		printf("\n");
		printf("      [/verbose]            output weapon stats.\n");
		}
	else if (pCmdLine->GetAttributeBool(SHIP_IMAGE_SWITCH))
		{
		printf("  /shipimage            Generates image of a specific ship.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("       /unid:n              unid of ship.\n");
		printf("\n");
		printf("      [/driveimages]		Show all rotations with exhaust images.\n");
		printf("      [/grid]               Show a grid centered on ship.\n");
		printf("      [/output:file]        Saves image to given filespec.\n");
		printf("      [/weaponpos]          Show all rotations with weapon positions.\n");
		}
	else if (pCmdLine->GetAttributeBool(SHIP_IMAGES_SWITCH))
		{
		printf("  /shipimages           Generates ship images.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("      [/font:s]             Use given font for labels.\n");
		printf("      [/output:file]        Saves image to given filespec.\n");
		printf("      [/rotation:n]         Specifies the rotation of all ships.\n");
		printf("      [/sort:xxx]           Sort ships in given order.\n");
		printf("         largest               Larger ships first.\n");
		printf("         name                  By ship name.\n");
		printf("         smallest              Smaller ships first.\n");
		printf("      [/textBoxesOnly]      Output textboxes only.\n");
		printf("      [/width:n]            Width of image in pixels.\n");
		printf("      [/xMargin:n]          Margin around ships.\n");
		printf("      [/xSpacing:n]         Spacing between ships.\n");
		}
	else if (pCmdLine->GetAttributeBool(SHIP_TABLE_SWITCH))
		{
		printf("  /shiptable            Ship table.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("      [/allClasses]         include special classes (such as wingmen).\n");
		printf("      [/criteria:xxx]       Ships must match criteria.\n");
		printf("\n");
		printf("      [/armorItems]         installed armor items.\n");
		printf("      [/balanceType]        designation based on combat strength.\n");
		printf("      [/cargoSpace]         cargo space available (in tons).\n");
		printf("      [/combatStrength]     combat power (absolute).\n");
		printf("      [/damage]             damage done per 180 ticks.\n");
		printf("      [/defenseStrength]    defense strength (absolute).\n");
		printf("      [/deviceSlots]        number of device slots.\n");
		printf("      [/deviceSlotsNonWeapons] number of slots for non-weapons.\n");
		printf("      [/deviceSlotsWeapons] number of slots for weapons.\n");
		printf("      [/deviceItems]        installed devices.\n");
		printf("      [/fireAccuracy]       AI fire accuracy (%%).\n");
		printf("      [/fireRangeAdj]       AI fire range adjustment (%% of normal).\n");
		printf("      [/fireRateAdj]        AI fire rate adjustment (%% of normal).\n");
		printf("      [/genericName]        generic name.\n");
		printf("      [/hullMass]           mass of hull only (in tons).\n");
		printf("      [/launcher]           launcher.\n");
		printf("      [/maneuver]           time for a complete rotation.\n");
		printf("      [/manufacturer]       ship manufacturer.\n");
		printf("      [/maxArmorMass]       max mass for armor (in kilograms).\n");
		printf("      [/maxCargoSpace]      max cargo space (in tons).\n");
		printf("      [/maxSpeed]           maximum speed (in %% of lightspeed).\n");
		printf("      [/primaryArmor]       primary armor.\n");
		printf("      [/primaryWeapon]      primary weapon.\n");
		printf("      [/primaryWeaponRange] max effective weapon range (in ls).\n");
		printf("      [/primaryWeaponRangeAdj] adjusted effective weapon range (in ls).\n");
		printf("      [/score]              score.\n");
		printf("      [/shield]             shield generator.\n");
		printf("      [/thrustToWeight]     thrust-to-weight ratio.\n");
		printf("      [/unid]               unid of class.\n");
		}
	else if (pCmdLine->GetAttributeBool(SIM_TABLES_SWITCH))
		{
		printf("  /simtables            Generates tables for sim counts.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("       /count:n             n iterations.\n");
		}
	else if (pCmdLine->GetAttributeBool(SNAPSHOT_SWITCH))
		{
		printf("  /snapshot             Generates a snapshot of an object.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("      [/node:ID]            Look for object in given node.\n");
		printf("      [/criteria:xxx]       Object must match criteria.\n");
		printf("      [/output:file]        Saves image to given filespec.\n");
		printf("      [/size:n]             Size of snapshot in pixels.\n");
		}
	else if (pCmdLine->GetAttributeBool(STATION_PLACE_SIM_SWITCH))
		{
		printf("  /stationSeparationSim Generate enemy station separation statistics.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("      [/count:n]            n iterations.\n");
		}
	else if (pCmdLine->GetAttributeBool(SYSTEM_LABELS_SWITCH))
		{
		printf("  /systemlabels         Generate counts for all labels.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("       /count:n             n iterations.\n");
		printf("      [/nodes]              Show node attribute counts instead.\n");
		printf("      [/nodeDebug]          Output sample nodes with attributes instead.\n");
		printf("      [/permutations]       Include attribute permutations.\n");
		}
	else if (pCmdLine->GetAttributeBool(SYSTEM_TEST_SWITCH))
		{
		printf("  /systemtest           Generates sample of all systems.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("       /count:n             n iterations.\n");
		}
	else if (pCmdLine->GetAttributeBool(TYPE_TABLE_SWITCH))
		{
		printf("  /typeTable            Generates sample of all systems.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("      [/criteria:xxx]       Type of given criteria.\n");
		printf("\n");
		printf("      criteria:\n");
		printf("          $                     EconomyType.\n");
		printf("          a                     AdventureDesc.\n");
		printf("          b                     ItemTable.\n");
		printf("          c                     EffectType.\n");
		printf("          d                     DockScreen.\n");
		printf("          e                     SpaceEnvironmentType.\n");
		printf("          f                     OverlayType.\n");
		printf("          h                     ShipTable.\n");
		printf("          i                     ItemType.\n");
		printf("          m                     Image.\n");
		printf("          n                     MissionType.\n");
		printf("          p                     Power.\n");
		printf("          s                     ShipClass.\n");
		printf("          t                     StationType.\n");
		printf("          u                     SoundType.\n");
		printf("          v                     Sovereign.\n");
		printf("          x                     GenericType.\n");
		printf("          y                     SystemType.\n");
		printf("          z                     SystemMap.\n");
		printf("          _                     TemplateType.\n");
		}
	else if (pCmdLine->GetAttributeBool(WEAPON_IMAGES_SWITCH))
		{
		printf("  /weaponimages         Generates images of weapon effects.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("      [/criteria:xxx]       only items that match criteria.\n");
		printf("      [/output:file]        Saves image to given filespec.\n");
		}
	else if (pCmdLine->GetAttributeBool(WORD_GENERATOR_SWITCH))
		{
		printf("  /wordgenerator        Generate a list of random words.\n");
		printf("       /count:n             Number of words to generate.\n");
		printf("       /input:filename      List of words to use as examples.\n");
		printf("      [/novel]              Exclude words in the input file.\n");
		}
	else if (pCmdLine->GetAttributeBool(WORLD_IMAGES_SWITCH))
		{
		printf("  /worldimages          Generates world images.\n");
		printf("      [/adventure:n]		Load the given adventure (by UNID).\n");
		printf("      [/font:s]             Use given font for labels.\n");
		printf("      [/output:file]        Saves image to given filespec.\n");
		printf("      [/sort:xxx]           Sort worlds in given order.\n");
		printf("         largest               Larger worlds first.\n");
		printf("         name                  By name.\n");
		printf("         smallest              Smaller worlds first.\n");
		printf("      [/textBoxesOnly]      Output textboxes only.\n");
		printf("      [/width:n]            Width of image in pixels.\n");
		printf("      [/xMargin:n]          Margin around images.\n");
		printf("      [/xSpacing:n]         Spacing between images.\n");
		}
	else
		{
		printf("  /attributelist        List of attributes used by types.\n");
		if (bDebug)
			printf("  /clearRegistered      Clears registered flag on game file.\n");
		printf("  /decompile            Extracts resources from .tdb (overwrites existing).\n");
		if (bDebug)
			printf("  /itemsim              Simulation of items encountered.\n");
		printf("  /itemtable            Item table.\n");
		printf("  /encountersim         Simulate an attack on the station.\n");
		printf("  /encountertable       Encounter table.\n");
		if (bDebug)
			printf("  /perftest             Performance test.\n");
		printf("  /run                  Run a TransLisp command.\n");
		if (bDebug)
			printf("  /shieldtest           Outputs capabilities of shield against weapons.\n");
		printf("  /shipimage            Generates image of a specific ship class.\n");
		printf("  /shipimages           Generates images of ship classes.\n");
		printf("  /shiptable            Ship class information.\n");
		printf("  /simtables            Generate tables for sim counts.\n");
		if (bDebug)
			printf("  /smoketest            Tests the game.\n");
		printf("  /snapshot             Generates a snapshot of the given object.\n");
		printf("  /stationfrequency     Table of station types by level.\n");
		if (bDebug)
			printf("  /stationSeparationSim Generate enemy station separation statistics.\n");
		printf("  /stats                Shows a list of basic stats.\n");
		printf("  /stdarmor             Shows stats about standard armors.\n");
		printf("  /stdshield            Shows stats about standard shields.\n");
		printf("  /systemlabels         Generate counts for all labels.\n");
		if (bDebug)
			printf("  /systemtest           Generates sample of all systems.\n");
		printf("  /tradesim             Generates statistics of item trading.\n");
		if (bDebug)
			printf("  /typeDependencies     Show a list of all types and their dependencies.\n");
		if (bDebug)
			printf("  /typeIslands          Show groups of types that depend on each other.\n");
		printf("  /typeTable            Generates a table of type information.\n");
		printf("  /weaponimages         Generates images of weapon effects.\n");
		if (bDebug)
			printf("  /wordlist             Lists all unique words.\n");
		printf("  /worldimages          Generates images of worlds.\n");
		}

	printf("\n");
	}

