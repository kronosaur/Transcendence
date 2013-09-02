//	XMLLoader.cpp
//
//	Initializes the universe from XML

#include "PreComp.h"

#ifdef DEBUG
//#define DEBUG_TIME_IMAGE_LOAD
#endif

#define ADVENTURE_DESC_TAG					CONSTLIT("AdventureDesc")
#define ENCOUNTER_TABLES_TAG				CONSTLIT("EncounterTables")
#define TABLES_TAG							CONSTLIT("Tables")
#define POWERS_TAG							CONSTLIT("Powers")
#define SOUNDS_TAG							CONSTLIT("Sounds")
#define GLOBALS_TAG							CONSTLIT("Globals")
#define SPACE_ENVIRONMENT_TYPE_TAG			CONSTLIT("SpaceEnvironmentType")
#define ITEM_TABLE_TAG						CONSTLIT("ItemTable")
#define DOCK_SCREEN_TAG						(CONSTLIT("DockScreen"))
#define ITEM_TYPE_TAG						(CONSTLIT("ItemType"))
#define SHIP_CLASS_TAG						(CONSTLIT("ShipClass"))
#define IMAGE_TAG							(CONSTLIT("Image"))
#define STATION_TYPE_TAG					(CONSTLIT("StationType"))
#define SYSTEM_TYPE_TAG						(CONSTLIT("SystemType"))
#define SOVEREIGN_TAG						(CONSTLIT("Sovereign"))
#define SOVEREIGNS_TAG						(CONSTLIT("Sovereigns"))
#define IMAGES_TAG							CONSTLIT("Images")
#define STAR_SYSTEM_TYPES_TAG				CONSTLIT("SystemTypes")
#define SYSTEM_NODE_TAG						CONSTLIT("SystemNode")
#define POWER_TAG							CONSTLIT("Power")
#define TRANSCENDENCE_ADVENTURE_TAG			CONSTLIT("TranscendenceAdventure")
#define TRANSCENDENCE_MODULE_TAG			CONSTLIT("TranscendenceModule")

#define ID_ATTRIB							CONSTLIT("ID")
#define UNID_ATTRIB							CONSTLIT("UNID")
#define FOLDER_ATTRIB						CONSTLIT("folder")
#define FILENAME_ATTRIB						CONSTLIT("filename")
#define MIN_VERSION_ATTRIB					CONSTLIT("minVersion")
#define VERSION_ATTRIB						CONSTLIT("version")

#define FILE_TYPE_XML						CONSTLIT("xml")
#define FILE_TYPE_TDB						CONSTLIT("tdb")
#define STORAGE_FILESPEC					CONSTLIT("Storage.xml")

static char g_ShipClassTag[] = "ShipClasses";
static char g_StationTypesTag[] = "StationTypes";
static char g_ArmorClassesTag[] = "ArmorClasses";
static char g_DockScreensTag[] = "DockScreens";
static char g_ItemTypesTag[] = "ItemTypes";

static char g_BitmapAttrib[] = "bitmap";
static char g_BitmaskAttrib[] = "bitmask";
static char g_TransColorAttrib[] = "backColor";

ALERROR CUniverse::InitCodeChain (void)

//	InitCodeChain
//
//	Initialize CodeChain

	{
	ALERROR error;

	//	Initialize code chain

	if (error = m_CC.Boot())
		return error;

	if (error = InitCodeChainPrimitives())
		return error;

	return NOERROR;
	}

ALERROR CUniverse::InitDeviceStorage (CString *retsError)

//	InitDeviceStorage
//
//	Initializes cross-game local storage

	{
	ALERROR error;

	if (error = m_DeviceStorage.Load(STORAGE_FILESPEC, retsError))
		return error;

	return NOERROR;
	}

ALERROR CUniverse::InitLevelEncounterTables (void)

//	InitLevelEncounterTables
//
//	Initializes the m_LevelEncounterTables array based on the encounter
//	tables of all the stations for each level.

	{
	m_LevelEncounterTables.RemoveAll();

	for (int i = 1; i <= MAX_ITEM_LEVEL; i++)
		{
		CStructArray *pTable = new CStructArray(sizeof(SLevelEncounter), 8);

		for (int j = 0; j < GetStationTypeCount(); j++)
			{
			CStationType *pType = GetStationType(j);

			//	Figure out the frequency of an encounter from this station based
			//	on the frequency of the station at this level and the frequency
			//	of encounters for this station.

			int iEncounterFreq = pType->GetEncounterFrequency();
			int iStationFreq = pType->GetFrequencyByLevel(i);
			int iFreq = iEncounterFreq * iStationFreq / ftCommon;

			//	Add to the table

			if (iFreq > 0)
				{
				pTable->ExpandArray(pTable->GetCount(), 1);
				SLevelEncounter *pEntry = (SLevelEncounter *)pTable->GetStruct(pTable->GetCount()-1);

				pEntry->pType = pType;
				pEntry->iWeight = iFreq;
				pEntry->pBaseSovereign = pType->GetSovereign();
				pEntry->pTable = pType->GetEncountersTable();
				}
			}

		m_LevelEncounterTables.AppendObject(pTable, NULL);
		}

	return NOERROR;
	}

ALERROR CUniverse::SaveDeviceStorage (void)

//	SaveDeviceStorage
//
//	Saves the cross-game device storage file.

	{
	ALERROR error;

	if (error = m_DeviceStorage.Save(STORAGE_FILESPEC))
		return error;

	return NOERROR;
	}

