//	CGameSettings.cpp
//
//	CGameSettings class

#include "PreComp.h"
#include "Transcendence.h"

#define EXTENSION_FOLDER_TAG					CONSTLIT("ExtensionFolder")
#define EXTENSIONS_TAG							CONSTLIT("Extensions")
#define KEY_MAP_TAG								CONSTLIT("KeyMap")
#define OPTION_TAG								CONSTLIT("Option")

#define NAME_ATTRIB								CONSTLIT("name")
#define PATH_ATTRIB								CONSTLIT("path")
#define VALUE_ATTRIB							CONSTLIT("value")

#define REGISTRY_COMPANY_NAME					CONSTLIT("Neurohack")
#define REGISTRY_PRODUCT_NAME					CONSTLIT("Transcendence")
#define REGISTRY_MUSIC_OPTION					CONSTLIT("Music")
#define REGISTRY_SOUND_VOLUME_OPTION			CONSTLIT("SoundVolume")

#define TRANSCENDENCE_APP_DATA					CONSTLIT("Kronosaur\\Transcendence")

#define OPTION_FLAG_HEX							0x00000001

enum OptionTypes
	{
	optionBoolean,
	optionInteger,
	optionString,
	};

struct SOptionDefaults
	{
	char *pszName;
	int iType;
	char *pszDefaultValue;
	DWORD dwFlags;
	};

struct SCommandLineData
	{
	char *pszParam;
	int iOption;
	DWORD dwFlags;
	};

SOptionDefaults g_OptionData[CGameSettings::OPTIONS_COUNT] =
	{
		//	Game play options

		{	"playerName",				optionString,	"",			0	},
		{	"playerGenome",				optionString,	"",			0	},
		{	"playerShipClass",			optionInteger,	"0",		OPTION_FLAG_HEX	},
		{	"lastAdventure",			optionInteger,	"0",		OPTION_FLAG_HEX	},

		{	"dockPortIndicator",		optionString,	"enabled",	0	},
		{	"invokeLetterHotKeys",		optionBoolean,	"false",	0	},
		{	"noAutoSave",				optionBoolean,	"false",	0	},
		{	"noFullCreate",				optionBoolean,	"false",	0	},
		{	"showManeuverEffects",		optionBoolean,	"false",	0	},
		{	"noMissionCheckpoint",		optionBoolean,	"false",	0	},
		{	"defaultBuyTab",			optionString,	"filterGeneral",	0	},
		{	"noMouseAim",				optionBoolean,	"false",	0	},
		{	"forcePermadeath",			optionBoolean,	"false",	0	},

		//	Installation options

		{	"useTDB",					optionBoolean,	"false",	0	},
		{	"noAutoUpdate",				optionBoolean,	"false",	0	},

		//	Video options
		{	"forceDirectX",				optionBoolean,	"false",	0	},
		{	"forceNonDirectX",			optionBoolean,	"false",	0	},
		{	"forceExclusive",			optionBoolean,	"false",	0	},
		{	"forceNonExclusive",		optionBoolean,	"false",	0	},
		{	"windowedMode",				optionBoolean,	"false",	0	},
		{	"useBackgroundBlt",			optionBoolean,	"true",		0	},
		{	"force1024",				optionBoolean,	"false",	0	},
		{	"force600",					optionBoolean,	"false",	0	},
		{	"graphicsQuality",			optionString,	"auto",		0	},
		{	"no3DSystemMap",			optionBoolean,	"false",	0	},
		{	"maxResolution",			optionInteger,	"1200",		0	},
		{	"noGPUAcceleration",		optionBoolean,	"false",	0	},

		//	Sounds options
		{	"noSound",					optionBoolean,	"false",	0	},
		{	"noMusic",					optionBoolean,	"false",	0	},
		{	"soundVolume",				optionInteger,	"7",		0	},
		{	"musicVolume",				optionInteger,	"7",		0	},
		{	"musicPath",				optionString,	"",			0	},

		//	Debug options
		{	"debugMode",				optionBoolean,	"false",	0	},
		{	"debugGame",				optionBoolean,	"false",	0	},
		{	"noDebugLog",				optionBoolean,	"false",	0	},
		{	"debugVideo",				optionBoolean,	"false",	0	},
		{	"noCrashPost",				optionBoolean,	"false",	0	},
		{	"noCollectionDownload",		optionBoolean,	"false",	0	},
		{	"noCollectionLoad",		    optionBoolean,	"false",	0	},
		{	"debugSoundtrack",			optionBoolean,	"false",	0	},
		{	"introSpoilers",			optionBoolean,	"false",	0	},
	};

SCommandLineData g_CommandLineData[] =
	{
		{	"useTDB",				CGameSettings::useTDB,				0 },
		{	"dx",					CGameSettings::forceDirectX,		0 },
		{	"nodx",					CGameSettings::forceNonDirectX,		0 },
		{	"exclusive",			CGameSettings::forceExclusive,		0 },
		{	"nonExclusive",			CGameSettings::forceNonExclusive,	0 },
		{	"windowed",				CGameSettings::windowedMode,		0 },
		{	"1024",					CGameSettings::force1024Res,		0 },
		{	"600",					CGameSettings::force600Res,			0 },
		{	"nosound",				CGameSettings::noSound,				0 },
		{	"debug",				CGameSettings::debugMode,			0 },
		{	"debug",				CGameSettings::debugGame,			0 },
		{	"nolog",				CGameSettings::noDebugLog,			0 },
		{	"debugVideo",			CGameSettings::debugVideo,			0 },
		{	"debugSoundtrack",		CGameSettings::debugSoundtrack,		0 },
	};

const int COMMAND_LINE_DATA_COUNT =			(sizeof(g_CommandLineData) / sizeof(g_CommandLineData[0]));

int FindOptionData (const CString &sName, const SOptionDefaults **retpData = NULL);

ALERROR CGameSettings::Load (const CString &sFilespec, CString *retsError)

//	Load
//
//	Load game settings from a file. If the file does not exist, then we 
//	set settings to default values

	{
	ALERROR error;
	int i;

	//	Initialize from defaults

	for (i = 0; i < OPTIONS_COUNT; i++)
		SetValue(i, CString(g_OptionData[i].pszDefaultValue, -1, true), true);

	//	Look for a file in the current directory and see if it is writable. If
	//	not, then look in AppData. We remember the place where we found a valid
	//	file as our AppData root (and we base other directories off that).

	if (pathIsWritable(sFilespec))
		{
		//	AppData is current directory
		m_sAppData = NULL_STR;
		}
	else
		{
		m_sAppData = pathAddComponent(pathGetSpecialFolder(folderAppData), TRANSCENDENCE_APP_DATA);
		if (!pathCreate(m_sAppData)
				|| !pathIsWritable(m_sAppData))
			{
			*retsError = strPatternSubst(CONSTLIT("Unable to write to AppData folder: %s"), m_sAppData);
			return ERR_FAIL;
			}
		}

	//	Settings file

	CString sSettingsFilespec = pathAddComponent(m_sAppData, sFilespec);

	//	Load XML

	CFileReadBlock DataFile(sSettingsFilespec);
	CXMLElement *pData;
	CString sError;
	if (error = CXMLElement::ParseXML(&DataFile, &pData, retsError))
		{
		//	ERR_NOTFOUND means that we couldn't find the Settings.xml
		//	file. In that case, initialize from defaults

		if (error == ERR_NOTFOUND)
			{
			LoadFromRegistry();
			m_bModified = true;
			return NOERROR;
			}

		//	Otherwise, it means that we got an error parsing the file.
		//	Return the error, but leave the settings initialized to defaults
		//	(We should be OK to continue, even with an error).

		else
			{
			m_bModified = false;
			return error;
			}
		}

	//	Keep track of which settings we've seen

	bool bLoaded[OPTIONS_COUNT];
	for (i = 0; i < OPTIONS_COUNT; i++)
		bLoaded[i] = false;

	//	Initialize to unmodified (as we load settings we might change this)

	m_bModified = false;

	//	Loop over all elements

	for (i = 0; i < pData->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pData->GetContentElement(i);

		if (strEquals(pItem->GetTag(), OPTION_TAG))
			{
			int iOption = FindOptionData(pItem->GetAttribute(NAME_ATTRIB));
			if (iOption == -1)
				{
				kernelDebugLogPattern("Unknown option: %s", pItem->GetAttribute(NAME_ATTRIB));
				continue;
				}

			SetValue(iOption, pItem->GetAttribute(VALUE_ATTRIB), true);
			bLoaded[iOption] = true;
			}
		else if (strEquals(pItem->GetTag(), KEY_MAP_TAG))
			{
			if (error = m_KeyMap.ReadFromXML(pItem))
				return error;
			}
		else if (strEquals(pItem->GetTag(), EXTENSION_FOLDER_TAG))
			{
			CString sFolder;
			if (pItem->FindAttribute(PATH_ATTRIB, &sFolder))
				m_ExtensionFolders.Insert(sFolder);
			}
		else if (strEquals(pItem->GetTag(), EXTENSIONS_TAG))
			{
			if (error = m_Extensions.ReadFromXML(pItem))
				return error;
			}
		else if (m_pExtra)
			{
			bool bModified;
			if (error = m_pExtra->OnLoadSettings(pItem, &bModified))
				return error;

			if (bModified)
				m_bModified = true;
			}
		}

	//	If we didn't load all options, then mark as modified so that we can save
	//	out all current options. This happens when a new build adds an option.

	for (i = 0; i < OPTIONS_COUNT; i++)
		if (!bLoaded[i])
			{
			m_bModified = true;
			break;
			}

	//	Done

	delete pData;

	return NOERROR;
	}

void CGameSettings::LoadFromRegistry (void)

//	LoadFromRegistry
//
//	Load settings from the registry

	{
	//	Look for the reg key. If we can't find it, then
	//	don't worry about it (we'll take the defaults)

	CRegKey Key;
	if (CRegKey::OpenUserAppKey(REGISTRY_COMPANY_NAME,
			REGISTRY_PRODUCT_NAME,
			&Key) != NOERROR)
		return;

	//	Music

	CString sMusic;
	if (Key.FindStringValue(REGISTRY_MUSIC_OPTION, &sMusic))
		SetValueBoolean(noMusic, !strEquals(sMusic, CONSTLIT("on")), true);

	//	Volume

	CString sSoundVolume;
	if (Key.FindStringValue(REGISTRY_SOUND_VOLUME_OPTION, &sSoundVolume))
		SetValueInteger(soundVolume, Min(10, Max(0, strToInt(sSoundVolume, 10, NULL))), true);
	}

ALERROR CGameSettings::ParseCommandLine (char *pszCmdLine)

//	ParseCommandLine
//
//	Allow command line to override settings

	{
	ALERROR error;
	int i;

	char *argv[2];
	argv[0] = "Transcendence";
	argv[1] = pszCmdLine;
	CXMLElement *pCmdLine;
	if (error = CreateXMLElementFromCommandLine(2, argv, &pCmdLine))
		return error;

	//	Loop over all command line arguments

	for (i = 0; i < COMMAND_LINE_DATA_COUNT; i++)
		{
		bool bValue;
		if (pCmdLine->FindAttributeBool(CString(g_CommandLineData[i].pszParam, -1, true), &bValue))
			SetValueBoolean(g_CommandLineData[i].iOption, bValue);
		}

	//	If we have an arg then use it as the save file name

	if (pCmdLine->GetContentElementCount() > 0)
		m_sSaveFile = pCmdLine->GetContentText(0);

	//	Done

	delete pCmdLine;

	return NOERROR;
	}

ALERROR CGameSettings::Save (const CString &sFilespec)

//	Save
//
//	Save game settings to a file (if necessary)

	{
	int i;
	ALERROR error;

	if (!m_bModified && !m_KeyMap.IsModified())
		return NOERROR;

	//	Settings file

	CString sSettingsFilespec = pathAddComponent(m_sAppData, sFilespec);

	//	Create the file

	CFileWriteStream DataFile(sSettingsFilespec, FALSE);
	if (error = DataFile.Create())
		return error;

	//	Write the XML header

	CString sData = CONSTLIT("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n\r\n<TranscendenceSettings>\r\n\r\n");
	if (error = DataFile.Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	//	Write extension folders

	if (m_ExtensionFolders.GetCount() > 0)
		{
		for (i = 0; i < m_ExtensionFolders.GetCount(); i++)
			{
			sData = strPatternSubst(CONSTLIT("\t<ExtensionFolder path=\"%s\"/>\r\n"), m_ExtensionFolders[i]);
			if (error = DataFile.Write(sData.GetPointer(), sData.GetLength()))
				return error;
			}

		if (error = DataFile.Write("\r\n", 2, NULL))
			return error;
		}

	//	Sort the options

	TSortMap<CString, int> Sorted;
	for (i = 0; i < OPTIONS_COUNT; i++)
		Sorted.Insert(CString(g_OptionData[i].pszName, -1, true), i);

	//	Loop over options

	for (i = 0; i < Sorted.GetCount(); i++)
		{
		int iOption = Sorted[i];

		//	Compose option element and write

		sData = strPatternSubst(CONSTLIT("\t<Option name=\"%s\"\tvalue=\"%s\"/>\r\n"),
				Sorted.GetKey(i),
				strToXMLText(m_Options[iOption].sSettingsValue));

		if (error = DataFile.Write(sData.GetPointer(), sData.GetLength(), NULL))
			return error;
		}

	//	Write the key map

	if (error = DataFile.Write("\r\n", 2, NULL))
		return error;

	if (error = m_KeyMap.WriteAsXML(&DataFile))
		return error;

	//	Write the extensions list

	if (error = DataFile.Write("\r\n", 2))
		return error;

	if (error = m_Extensions.WriteAsXML(&DataFile))
		return error;

	//	Write additional settings

	if (m_pExtra)
		{
		if (error = DataFile.Write("\r\n", 2, NULL))
			return error;

		if (error = m_pExtra->OnSaveSettings(&DataFile))
			return error;
		}

	//	Done

	sData = CONSTLIT("\r\n</TranscendenceSettings>\r\n");
	if (error = DataFile.Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	if (error = DataFile.Close())
		return error;

	return NOERROR;
	}

void CGameSettings::SetValue (int iOption, const CString &sValue, bool bSetSettings)

//	SetValue
//
//	Sets the boolean, integer, or string value of the option

	{
	switch (g_OptionData[iOption].iType)
		{
		case optionBoolean:
			m_Options[iOption].bValue = (strEquals(sValue, CONSTLIT("true")) || strEquals(sValue, CONSTLIT("on")) || strEquals(sValue, CONSTLIT("yes")));
			break;

		case optionInteger:
			m_Options[iOption].iValue = strToInt(sValue, 0);
			break;

		case optionString:
			m_Options[iOption].sValue = sValue;
			break;

		default:
			ASSERT(false);
		}

	//	Set the settings value, if appropriate

	if (bSetSettings)
		m_Options[iOption].sSettingsValue = sValue;
	}

void CGameSettings::SetValueBoolean (int iOption, bool bValue, bool bSetSettings)

//	SetValueBoolean
//
//	Sets a boolean value

	{
	if (g_OptionData[iOption].iType != optionBoolean)
		return;

	m_Options[iOption].bValue = bValue;

	if (bSetSettings)
		m_Options[iOption].sSettingsValue = (bValue ? CONSTLIT("true") : CONSTLIT("false"));
	}

void CGameSettings::SetValueInteger (int iOption, int iValue, bool bSetSettings)

//	SetValueInteger
//
//	Sets an integer value

	{
	if (g_OptionData[iOption].iType != optionInteger)
		return;

	m_Options[iOption].iValue = iValue;

	if (bSetSettings)
		{
		if (g_OptionData[iOption].dwFlags & OPTION_FLAG_HEX)
			m_Options[iOption].sSettingsValue = strPatternSubst("0x%08x", iValue);
		else
			m_Options[iOption].sSettingsValue = strFromInt(iValue);
		}
	}

void CGameSettings::SetValueString (int iOption, const CString &sValue, bool bSetSettings)

//	SetValueInteger
//
//	Sets an integer value

	{
	if (g_OptionData[iOption].iType != optionString)
		return;

	m_Options[iOption].sValue = sValue;

	if (bSetSettings)
		m_Options[iOption].sSettingsValue = sValue;
	}

//	Utilities

int FindOptionData (const CString &sName, const SOptionDefaults **retpData)
	{
	int i;

	for (i = 0; i < CGameSettings::OPTIONS_COUNT; i++)
		if (strEquals(sName, CString(g_OptionData[i].pszName, -1, true)))
			{
			if (retpData)
				*retpData = &g_OptionData[i];
			return i;
			}

	return -1;
	}
