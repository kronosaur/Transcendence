//	CGameKeys.cpp
//
//	CGameKeys class

#include "PreComp.h"
#include "Transcendence.h"

#define COMMAND_ATTRIB						CONSTLIT("command")
#define KEY_ATTRIB							CONSTLIT("key")

struct SKeyMapEntry
	{
	int iVirtKey;
	CGameKeys::Keys iGameKey;
	};

//	Default Key Mapping
//
//	NOTES
//
//	* Do not use F12 as it causes a break when running with a debugger

SKeyMapEntry g_DefaultMap[] =
	{
		{	'A',				CGameKeys::keyAutopilot },
		{	'B',				CGameKeys::keyEnableDevice },
		{	'C',				CGameKeys::keyCommunications },
		{	'D',				CGameKeys::keyDock },
		//	'E' unused
		{	'F',				CGameKeys::keyTargetNextFriendly },
		{	'G',				CGameKeys::keyEnterGate },
		//	'H' unused
		{	'I',				CGameKeys::keyInvokePower },
		{	'J',				CGameKeys::keyRotateLeft },
		{	'K',				CGameKeys::keyThrustForward },
		{	'L',				CGameKeys::keyRotateRight },
		{	'M',				CGameKeys::keyShowMap },
		{	'N',				CGameKeys::keyShowGalacticMap },
		//	'O' unused
		{	'P',				CGameKeys::keyPause },
		{	'Q',				CGameKeys::keySquadronCommands },
		{	'R',				CGameKeys::keyClearTarget },
		{	'S',				CGameKeys::keyShipStatus },
		{	'T',				CGameKeys::keyTargetNextEnemy },
		{	'U',				CGameKeys::keyUseItem },
		//	'V' unused
		{	'W',				CGameKeys::keyNextWeapon },
		//	'X' unused
		//	'Y' unused
		//	'Z' unused

		{	VK_CONTROL,			CGameKeys::keyFireWeapon },
		{	VK_DOWN,			CGameKeys::keyThrustForward },
		{	VK_LEFT,			CGameKeys::keyRotateLeft },
		{	VK_OEM_PERIOD,		CGameKeys::keyStop },
		{	VK_PAUSE,			CGameKeys::keyPause },
		{	VK_RETURN,			CGameKeys::keyFireMissile },
		{	VK_RIGHT,			CGameKeys::keyRotateRight },
		{	VK_SHIFT,			CGameKeys::keyFireMissile },
		{	VK_SPACE,			CGameKeys::keyFireWeapon },
		{	VK_TAB,				CGameKeys::keyNextMissile },
		{	VK_UP,				CGameKeys::keyThrustForward },

		{	VK_F1,				CGameKeys::keyShowHelp },
		{	VK_F2,				CGameKeys::keyShowGameStats },
		{	VK_F7,				CGameKeys::keyVolumeDown },
		{	VK_F8,				CGameKeys::keyVolumeUp },
		{	VK_F9,				CGameKeys::keyShowConsole },
	};

const int DEFAULT_MAP_COUNT = (sizeof(g_DefaultMap) / sizeof(g_DefaultMap[0]));

struct SVirtKeyData
	{
	char *pszName;
	char *pszLabel;
	};

SVirtKeyData g_VirtKeyData[] =
	{
		//	0x00 - 0x0F
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},

		{	"Backspace",	"Backspace"	},
		{	"Tab",			"Tab"	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	"Clear",		NULL	},
		{	"Return",		"Enter"	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},

		//	0x10 - 0x1F
		{	"Shift",		"Shift"	},
		{	"Control",		"Ctrl"	},
		{	"Menu",			NULL	},
		{	"Pause",		NULL	},
		{	"Capital",		NULL	},
		{	"Kana",			NULL	},
		{	NULL,			NULL	},
		{	"Junja",		NULL	},

		{	"Final",		NULL	},
		{	"Kanji",		NULL	},
		{	NULL,			NULL	},
		{	"Escape",		"Esc"	},
		{	"Convert",		NULL	},
		{	"NonConvert",	NULL	},
		{	"Accept",		NULL	},
		{	"ModeChange",	NULL	},

		//	0x20 - 0x2F
		{	"Space",		"SpaceBar"	},
		{	"PageUp",		"PgUp"	},
		{	"PageDown",		"PgDn"	},
		{	"End",			"End"	},
		{	"Home",			"Home"	},
		{	"Left",			"Left"	},
		{	"Up",			"Up"	},
		{	"Right",		"Right"	},

		{	"Down",			"Down"	},
		{	"Select",		NULL	},
		{	"Print",		"PrtScn"	},
		{	"Execute",		NULL	},
		{	"Snapshot",		NULL	},
		{	"Insert",		"Ins"	},
		{	"Delete",		"Del"	},
		{	"Help",			NULL	},

		//	0x30 - 0x3F
		{	"0",	NULL	},
		{	"1",	NULL	},
		{	"2",	NULL	},
		{	"3",	NULL	},
		{	"4",	NULL	},
		{	"5",	NULL	},
		{	"6",	NULL	},
		{	"7",	NULL	},

		{	"8",	NULL	},
		{	"9",	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},

		//	0x40 - 0x4F
		{	NULL,	NULL	},
		{	"A",	NULL	},
		{	"B",	NULL	},
		{	"C",	NULL	},
		{	"D",	NULL	},
		{	"E",	NULL	},
		{	"F",	NULL	},
		{	"G",	NULL	},

		{	"H",	NULL	},
		{	"I",	NULL	},
		{	"J",	NULL	},
		{	"K",	NULL	},
		{	"L",	NULL	},
		{	"M",	NULL	},
		{	"N",	NULL	},
		{	"O",	NULL	},

		//	0x50 - 0x5F
		{	"P",	NULL	},
		{	"Q",	NULL	},
		{	"R",	NULL	},
		{	"S",	NULL	},
		{	"T",	NULL	},
		{	"U",	NULL	},
		{	"V",	NULL	},
		{	"W",	NULL	},

		{	"X",	NULL	},
		{	"Y",	NULL	},
		{	"Z",	NULL	},
		{	"LWindows",	NULL	},
		{	"RWindows",	NULL	},
		{	"Apps",	NULL	},
		{	NULL,	NULL	},
		{	"Sleep",	NULL	},

		//	0x60 - 0x6F
		{	"Numpad0",	NULL	},
		{	"Numpad1",	NULL	},
		{	"Numpad2",	NULL	},
		{	"Numpad3",	NULL	},
		{	"Numpad4",	NULL	},
		{	"Numpad5",	NULL	},
		{	"Numpad6",	NULL	},
		{	"Numpad7",	NULL	},

		{	"Numpad8",	NULL	},
		{	"Numpad9",	NULL	},
		{	"NumpadStar",	NULL	},
		{	"NumpadPlus",	NULL	},
		{	"NumpadSeparator",	NULL	},
		{	"NumpadMinus",	NULL	},
		{	"NumpadPeriod",	NULL	},
		{	"NumpadSlash",	NULL	},

		//	0x70 - 0x7F
		{	"F1",			"F1"	},
		{	"F2",			"F2"	},
		{	"F3",			"F3"	},
		{	"F4",			"F4"	},
		{	"F5",			"F5"	},
		{	"F6",			"F6"	},
		{	"F7",			"F7"	},
		{	"F8",			"F8"	},

		{	"F9",			"F9"	},
		{	"F10",			"F10"	},
		{	"F11",			"F11"	},
		{	"F12",			"F12"	},
		{	"F13",			"F13"	},
		{	"F14",			"F14"	},
		{	"F15",			"F15"	},
		{	"F16",			"F16"	},

		//	0x80 - 0x8F
		{	"F17",			"F17"	},
		{	"F18",			"F18"	},
		{	"F19",			"F19"	},
		{	"F20",			"F20"	},
		{	"F21",			"F21"	},
		{	"F22",			"F22"	},
		{	"F23",			"F23"	},
		{	"F24",			"F24"	},

		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},

		//	0x90 - 0x9F
		{	"NumLock",		"NumLock"	},
		{	"ScrollLock",	"ScrLock"	},
		{	"FJ_Jisho",	NULL	},
		{	"FJ_Masshou",	NULL	},
		{	"FJ_Touroku",	NULL	},
		{	"FJ_Loya",	NULL	},
		{	"FJ_Roya",	NULL	},
		{	NULL,	NULL	},

		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},

		//	0xA0 - 0xAF
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	"BrowserBack",	NULL	},
		{	"BrowserForward",	NULL	},

		{	"BrowserRefresh",	NULL	},
		{	"BrowserStop",	NULL	},
		{	"BrowserSearch",	NULL	},
		{	"BrowserFavorites",	NULL	},
		{	"BrowserHome",	NULL	},
		{	"VolumeMute",	NULL	},
		{	"VolumeDown",	NULL	},
		{	"VolumeUp",	NULL	},

		//	0xB0 - 0xBF
		{	"MediaNext",	NULL	},
		{	"MediaPrev",	NULL	},
		{	"MediaStop",	NULL	},
		{	"MediaPlay",	NULL	},
		{	"LaunchMail",	NULL	},
		{	"LaunchMediaSelect",	NULL	},
		{	"LaunchApp1",	NULL	},
		{	"LaunchApp2",	NULL	},

		{	NULL	},
		{	NULL	},
		{	"SemiColon",	NULL	},
		{	"Equal",	NULL	},
		{	"Comma",	NULL	},
		{	"Minus",	NULL	},
		{	"Period",	NULL	},
		{	"Slash",	NULL	},

		//	0xC0 - 0xCF
		{	"BackQuote",	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},

		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},

		//	0xD0 - 0xDF
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},

		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	"OpenBracket",	NULL	},
		{	"Backslash",	NULL	},
		{	"CloseBracket",	NULL	},
		{	"Quote",	NULL	},
		{	NULL,	NULL	},

		//	0xE0 - 0xEF
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},

		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},

		//	0xF0 - 0xFF
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},

		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
	};

struct SGameKeyData
	{
	char *pszName;
	};

SGameKeyData g_GameKeyData[CGameKeys::keyCount] =
	{
		{	NULL },	//	keyNone

		{	"Autopilot" },
		{	"EnableDevices" },
		{	"Communications" },
		{	"Dock" },
		{	"TargetNextFriendly" },
		{	"EnterGate" },
		{	"InvokePower" },
		{	"ShowMap" },
		{	"Pause" },
		{	"CommandSquadron" },
		{	"ClearTarget" },
		{	"ShipStatus" },
		{	"TargetNextEnemy" },
		{	"UseItem" },
		{	"NextWeapon" },
		{	"ThrustForward" },
		{	"Stop" },
		{	"RotateLeft" },
		{	"RotateRight" },
		{	"FireWeapon" },
		{	"FireMissile" },
		{	"NextMissile" },
		{	"ShowHelp" },
		{	"ShowGameStats" },
		{	NULL },
		{	NULL },
		{	NULL },
		{	NULL },
		{	"VolumeUp" },
		{	"VolumeDown" },
		{	"ShowConsole" },

		{	"Special1" },
		{	"Special2" },
		{	"Special3" },
		{	"Special4" },
		{	"Special5" },
		{	"Special6" },
		{	"Special7" },
		{	"Special8" },
		{	"Special9" },
		{	"Special0" },

		{	"EnableAllDevices" },
		{	"DisableAllDevices" },
		{	"ToggleEnableAllDevices" },
		{	"ToggleEnableDevice00"	},
		{	"ToggleEnableDevice01"	},
		{	"ToggleEnableDevice02"	},
		{	"ToggleEnableDevice03"	},
		{	"ToggleEnableDevice04"	},
		{	"ToggleEnableDevice05"	},
		{	"ToggleEnableDevice06"	},
		{	"ToggleEnableDevice07"	},
		{	"ToggleEnableDevice08"	},
		{	"ToggleEnableDevice09"	},
		{	"ToggleEnableDevice10"	},
		{	"ToggleEnableDevice11"	},
		{	"ToggleEnableDevice12"	},
		{	"ToggleEnableDevice13"	},
		{	"ToggleEnableDevice14"	},
		{	"ToggleEnableDevice15"	},
		{	"ToggleEnableDevice16"	},
		{	"ToggleEnableDevice17"	},
		{	"ToggleEnableDevice18"	},
		{	"ToggleEnableDevice19"	},
		{	"ToggleEnableDevice20"	},
		{	"ToggleEnableDevice21"	},
		{	"ToggleEnableDevice22"	},
		{	"ToggleEnableDevice23"	},
		{	"ToggleEnableDevice24"	},
		{	"ToggleEnableDevice25"	},
		{	"ToggleEnableDevice26"	},
		{	"ToggleEnableDevice27"	},
		{	"ToggleEnableDevice28"	},
		{	"ToggleEnableDevice29"	},
		{	"ToggleEnableDevice30"	},
		{	"ToggleEnableDevice31"	},

		{	"TargetPreviousFriendly"	},
		{	"TargetPreviousEnemy"		},
		{	"PreviousWeapon"			},
		{	"PreviousMissile"			},
		{	"ShowGalacticMap"			},
	};

const int GAME_KEY_DATA_COUNT = (sizeof(g_GameKeyData) / sizeof(g_GameKeyData[0]));

CGameKeys::CGameKeys (void)

//	CGameKeys constructor

	{
	int i;

	ASSERT(GAME_KEY_DATA_COUNT == CGameKeys::keyCount);
	
	for (i = 0; i < 256; i++)
		m_iMap[i] = keyNone;

	for (i = 0; i < DEFAULT_MAP_COUNT; i++)
		m_iMap[g_DefaultMap[i].iVirtKey] = g_DefaultMap[i].iGameKey;
	}

CGameKeys::Keys CGameKeys::GetGameCommand (const CString &sCmd) const

//	GetGameCommand
//
//	Returns game command from string

	{
	int i;

	for (i = 0; i < CGameKeys::keyCount; i++)
		if (strEquals(sCmd, CString(g_GameKeyData[i].pszName, -1, true)))
			return (CGameKeys::Keys)i;

	return CGameKeys::keyError;
	}

DWORD CGameKeys::GetKey (const CString &sKey)

//	GetKey
//
//	Returns virtual key from string

	{
	int i;

	//	Handle single digits specially (otherwise we get them
	//	confused with Hex codes)

	if (sKey.GetLength() == 1)
		{
		char chChar = *sKey.GetASCIIZPointer();
		if (chChar >= '0' && chChar <= '9')
			return chChar;
		else if (chChar >= 'A' && chChar <= 'Z')
			return chChar;
		}

	//	See if this is a hex code

	DWORD dwVirtKey = ::strToInt(sKey, INVALID_VIRT_KEY);
	if (dwVirtKey != INVALID_VIRT_KEY)
		return dwVirtKey;

	//	Otherwise, look up in table

	for (i = 0; i < 256; i++)
		if (g_VirtKeyData[i].pszName && strEquals(sKey, CString(g_VirtKeyData[i].pszName, -1, true)))
			return i;

	return INVALID_VIRT_KEY;
	}

char CGameKeys::GetKeyIfChar (Keys iCommand) const

//	GetKeyIfChar
//
//	If the given command is bound to a character key, then return the character.
//	Otherwise we return 0.

	{
	int i;

	for (i = 0; i < 256; i++)
		if (m_iMap[i] == iCommand)
			{
			if (i >= 'A' && i <= 'Z')
				return (char)i;
			else
				return '\0';
			}

	return '\0';
	}

CString CGameKeys::GetKeyLabel (DWORD dwVirtKey)

//	GetKeyLabel
//
//	Returns the label for the given key

	{
	return CString(g_VirtKeyData[dwVirtKey].pszLabel, -1, true);
	}

ALERROR CGameKeys::ReadFromXML (CXMLElement *pDesc)

//	ReadFromXML
//
//	Read the key map from XML

	{
	int i;

	//	Load the map into a separate array so that we can detect
	//	duplicates

	Keys iNewMap[256];
	for (i = 0; i < 256; i++)
		iNewMap[i] = keyError;

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pMap = pDesc->GetContentElement(i);
		
		DWORD dwVirtKey = GetKey(pMap->GetAttribute(KEY_ATTRIB));
		if (dwVirtKey == INVALID_VIRT_KEY)
			{
			kernelDebugLogMessage("Unknown key: %s", pMap->GetAttribute(KEY_ATTRIB));
			continue;
			}

		Keys iCommand = GetGameCommand(pMap->GetAttribute(COMMAND_ATTRIB));
		if (iCommand == keyError)
			{
			kernelDebugLogMessage("Unknown game command: %s", pMap->GetAttribute(COMMAND_ATTRIB));
			continue;
			}

		if (iNewMap[dwVirtKey] != keyError)
			{
			kernelDebugLogMessage("Duplicate key mapping: %s", pMap->GetAttribute(COMMAND_ATTRIB));
			continue;
			}

		iNewMap[dwVirtKey] = iCommand;
		}

	//	Copy to map

	for (i = 0; i < 256; i++)
		if (iNewMap[i] != keyError)
			m_iMap[i] = iNewMap[i];

	return NOERROR;
	}

ALERROR CGameKeys::WriteAsXML (IWriteStream *pOutput)

//	WriteAsXML
//
//	Write the key map as an XML sub-element

	{
	ALERROR error;
	int i;

	//	Open tag

	CString sData;
	sData = CONSTLIT("\t<KeyMap>\r\n");
	if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	//	Keys

	for (i = 0; i < 256; i++)
		if (m_iMap[i] != keyNone)
			{
			CString sKey = (g_VirtKeyData[i].pszName ? CString(g_VirtKeyData[i].pszName, -1, true) : strPatternSubst(CONSTLIT("%x"), i));

			sData = strPatternSubst(CONSTLIT("\t\t<Map key=\"%s\" command=\"%s\"/>\r\n"),
					sKey,
					CString(g_GameKeyData[m_iMap[i]].pszName, -1, true));

			if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
				return error;
			}

	//	Close tag

	sData = CONSTLIT("\t</KeyMap>\r\n");
	if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	return NOERROR;
	}

