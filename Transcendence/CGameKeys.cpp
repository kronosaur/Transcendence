//	CGameKeys.cpp
//
//	CGameKeys class

#include "PreComp.h"
#include "Transcendence.h"

#define COMMAND_ATTRIB						CONSTLIT("command")
#define KEY_ATTRIB							CONSTLIT("key")
#define LAYOUT_ATTRIB						CONSTLIT("layout")

#define LAYOUT_DEFAULT                      CONSTLIT("default")
#define LAYOUT_CUSTOM                       CONSTLIT("custom")

//	Default Key Mapping
//
//	NOTES
//
//	* Do not use F12 as it causes a break when running with a debugger

const CGameKeys::SKeyMapEntry CGameKeys::DEFAULT_MAP[] =
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

const int CGameKeys::DEFAULT_MAP_COUNT = (sizeof(DEFAULT_MAP) / sizeof(DEFAULT_MAP[0]));

struct SVirtKeyData
	{
    enum EFlags
        {
        FLAG_NON_STANDARD =        0x00000001,
        };

	char *pszName;
	char *pszLabel;
    DWORD dwFlags;
	};

SVirtKeyData g_VirtKeyData[] =
	{
		//	0x00 - 0x0F
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},

		{	"Backspace",	"Backspace",   0	},
		{	"Tab",			"Tab",   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	"Clear",		NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"Return",		"Enter",   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},

		//	0x10 - 0x1F
		{	"Shift",		"Shift",   0	},
		{	"Control",		"Ctrl",   0	},
		{	"Menu",			NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"Pause",		NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"Capital",		NULL,   0	},
		{	"Kana",			NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	NULL,			NULL,   0	},
		{	"Junja",		NULL,   SVirtKeyData::FLAG_NON_STANDARD	},

		{	"Final",		NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"Kanji",		NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	NULL,			NULL,   0	},
		{	"Escape",		"Esc",   0	},
		{	"Convert",		NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"NonConvert",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"Accept",		NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"ModeChange",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},

		//	0x20 - 0x2F
		{	"Space",		"SpaceBar",   0	},
		{	"PageUp",		"PgUp",   0	},
		{	"PageDown",		"PgDn",   0	},
		{	"End",			"End",   0	},
		{	"Home",			"Home",   0	},
		{	"Left",			"Left",   0	},
		{	"Up",			"Up",   0	},
		{	"Right",		"Right",   0	},

		{	"Down",			"Down",   0	},
		{	"Select",		NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"Print",		"PrtScn",   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"Execute",		NULL,   0	},
		{	"Snapshot",		NULL,   0	},
		{	"Insert",		"Ins",   0	},
		{	"Delete",		"Del",   0	},
		{	"Help",			NULL,   SVirtKeyData::FLAG_NON_STANDARD	},

		//	0x30 - 0x3F
		{	"0",	NULL,   0	},
		{	"1",	NULL,   0	},
		{	"2",	NULL,   0	},
		{	"3",	NULL,   0	},
		{	"4",	NULL,   0	},
		{	"5",	NULL,   0	},
		{	"6",	NULL,   0	},
		{	"7",	NULL,   0	},

		{	"8",	NULL,   0	},
		{	"9",	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},

		//	0x40 - 0x4F
		{	NULL,	NULL,   0	},
		{	"A",	NULL,   0	},
		{	"B",	NULL,   0	},
		{	"C",	NULL,   0	},
		{	"D",	NULL,   0	},
		{	"E",	NULL,   0	},
		{	"F",	NULL,   0	},
		{	"G",	NULL,   0	},

		{	"H",	NULL,   0	},
		{	"I",	NULL,   0	},
		{	"J",	NULL,   0	},
		{	"K",	NULL,   0	},
		{	"L",	NULL,   0	},
		{	"M",	NULL,   0	},
		{	"N",	NULL,   0	},
		{	"O",	NULL,   0	},

		//	0x50 - 0x5F
		{	"P",	NULL,   0	},
		{	"Q",	NULL,   0	},
		{	"R",	NULL,   0	},
		{	"S",	NULL,   0	},
		{	"T",	NULL,   0	},
		{	"U",	NULL,   0	},
		{	"V",	NULL,   0	},
		{	"W",	NULL,   0	},

		{	"X",	NULL,   0	},
		{	"Y",	NULL,   0	},
		{	"Z",	NULL,   0	},
		{	"LWindows",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"RWindows",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"Apps",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	NULL,	NULL,   0	},
		{	"Sleep",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},

		//	0x60 - 0x6F
		{	"Numpad0",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"Numpad1",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"Numpad2",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"Numpad3",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"Numpad4",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"Numpad5",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"Numpad6",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"Numpad7",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},

		{	"Numpad8",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"Numpad9",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"NumpadStar",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"NumpadPlus",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"NumpadSeparator",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"NumpadMinus",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"NumpadPeriod",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"NumpadSlash",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},

		//	0x70 - 0x7F
		{	"F1",			"F1",   0	},
		{	"F2",			"F2",   0	},
		{	"F3",			"F3",   0	},
		{	"F4",			"F4",   0	},
		{	"F5",			"F5",   0	},
		{	"F6",			"F6",   0	},
		{	"F7",			"F7",   0	},
		{	"F8",			"F8",   0	},

		{	"F9",			"F9",   0	},
		{	"F10",			"F10",   0	},
		{	"F11",			"F11",   0	},
		{	"F12",			"F12",   0	},
		{	"F13",			"F13",   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"F14",			"F14",   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"F15",			"F15",   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"F16",			"F16",   SVirtKeyData::FLAG_NON_STANDARD	},

		//	0x80 - 0x8F
		{	"F17",			"F17",   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"F18",			"F18",   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"F19",			"F19",   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"F20",			"F20",   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"F21",			"F21",   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"F22",			"F22",   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"F23",			"F23",   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"F24",			"F24",   SVirtKeyData::FLAG_NON_STANDARD	},

		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},
		{	NULL,	NULL	},

		//	0x90 - 0x9F
		{	"NumLock",		"NumLock",   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"ScrollLock",	"ScrLock",   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"FJ_Jisho",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"FJ_Masshou",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"FJ_Touroku",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"FJ_Loya",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"FJ_Roya",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	NULL,	NULL,   0	},

		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},

		//	0xA0 - 0xAF
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	"BrowserBack",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"BrowserForward",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},

		{	"BrowserRefresh",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"BrowserStop",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"BrowserSearch",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"BrowserFavorites",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"BrowserHome",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"VolumeMute",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"VolumeDown",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"VolumeUp",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},

		//	0xB0 - 0xBF
		{	"MediaNext",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"MediaPrev",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"MediaStop",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"MediaPlay",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"LaunchMail",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"LaunchMediaSelect",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"LaunchApp1",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},
		{	"LaunchApp2",	NULL,   SVirtKeyData::FLAG_NON_STANDARD	},

		{	NULL	},
		{	NULL	},
		{	"SemiColon",	NULL,   0	},
		{	"Equal",	NULL,   0	},
		{	"Comma",	NULL,   0	},
		{	"Minus",	NULL,   0	},
		{	"Period",	NULL,   0	},
		{	"Slash",	NULL,   0	},

		//	0xC0 - 0xCF
		{	"BackQuote",	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},

		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},

		//	0xD0 - 0xDF
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},

		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	"OpenBracket",	NULL,   0	},
		{	"Backslash",	NULL,   0	},
		{	"CloseBracket",	NULL,   0	},
		{	"Quote",	NULL,   0	},
		{	NULL,	NULL,   0	},

		//	0xE0 - 0xEF
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},

		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},

		//	0xF0 - 0xFF
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},

		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
		{	NULL,	NULL,   0	},
	};

struct SGameKeyData
	{
    enum EFlags
        {
        FLAG_HIDDEN =       0x00000001,
        FLAG_DEBUG_ONLY =   0x00000002,
        };

	char *pszName;
    char *pszLabel;
    DWORD dwFlags;
	};

SGameKeyData g_GameKeyData[CGameKeys::keyCount] =
	{
		{	NULL },	//	keyNone

		{	"Autopilot",            "Autopilot",                    0 },
		{	"EnableDevices",        "Enable/Disable Devices",       0 },
		{	"Communications",       "Communications",               0 },
		{	"Dock",                 "Request Dock",                 0 },
		{	"TargetNextFriendly",   "Target Next Friendly",         0 },
		{	"EnterGate",            "Enter Stargate",               0 },
		{	"InvokePower",          "Invoke Power",                 0 },
		{	"ShowMap",              "System Map",                   0 },
		{	"Pause",                "Pause Game",                   0 },
		{	"CommandSquadron",      "Squadron Communications",      0 },
		{	"ClearTarget",          "Clear Target",                 0 },
		{	"ShipStatus",           "Ship's Status",                0 },
		{	"TargetNextEnemy",      "Target Next Enemy",            0 },
		{	"UseItem",              "Use Item",                     0 },
		{	"NextWeapon",           "Select Next Weapon",           0 },
		{	"ThrustForward",        "Forward Thrust",               0 },
		{	"Stop",                 "Stop Momentum",                0 },
		{	"RotateLeft",           "Rotate Left",                  0 },
		{	"RotateRight",          "Rotate Right",                 0 },
		{	"FireWeapon",           "Fire Weapon",                  0 },
		{	"FireMissile",          "Fire Missile",                 0 },
		{	"NextMissile",          "Select Next Missile",          0 },
		{	"ShowHelp",             "Help",                         0 },
		{	"ShowGameStats",        "Game Stats",                   0 },
		{	NULL },
		{	NULL },
		{	NULL },
		{	NULL },
		{	"VolumeUp",             "Volume Up",                    0 },
		{	"VolumeDown",           "Volume Down",                  0 },
		{	"ShowConsole",          "Debug Console",                SGameKeyData::FLAG_DEBUG_ONLY },

		{	"Special1",    NULL,                            SGameKeyData::FLAG_HIDDEN  },
		{	"Special2",    NULL,                            SGameKeyData::FLAG_HIDDEN  },
		{	"Special3",    NULL,                            SGameKeyData::FLAG_HIDDEN  },
		{	"Special4",    NULL,                            SGameKeyData::FLAG_HIDDEN  },
		{	"Special5",    NULL,                            SGameKeyData::FLAG_HIDDEN  },
		{	"Special6",    NULL,                            SGameKeyData::FLAG_HIDDEN  },
		{	"Special7",    NULL,                            SGameKeyData::FLAG_HIDDEN  },
		{	"Special8",    NULL,                            SGameKeyData::FLAG_HIDDEN  },
		{	"Special9",    NULL,                            SGameKeyData::FLAG_HIDDEN  },
		{	"Special0",    NULL,                            SGameKeyData::FLAG_HIDDEN  },

		{	"EnableAllDevices",    NULL,                            SGameKeyData::FLAG_HIDDEN },
		{	"DisableAllDevices",    NULL,                            SGameKeyData::FLAG_HIDDEN },
		{	"ToggleEnableAllDevices",    NULL,                            SGameKeyData::FLAG_HIDDEN },
		{	"ToggleEnableDevice00",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice01",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice02",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice03",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice04",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice05",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice06",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice07",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice08",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice09",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice10",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice11",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice12",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice13",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice14",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice15",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice16",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice17",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice18",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice19",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice20",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice21",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice22",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice23",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice24",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice25",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice26",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice27",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice28",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice29",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice30",    NULL,                            SGameKeyData::FLAG_HIDDEN	},
		{	"ToggleEnableDevice31",    NULL,                            SGameKeyData::FLAG_HIDDEN	},

		{	"TargetPreviousFriendly",   "Target Previous Friendly",     SGameKeyData::FLAG_HIDDEN	},
		{	"TargetPreviousEnemy",      "Target Previous Enemy",        SGameKeyData::FLAG_HIDDEN	},
		{	"PreviousWeapon",           "Select Previous Weapon",       SGameKeyData::FLAG_HIDDEN	},
		{	"PreviousMissile",          "Select Previous Missile",      SGameKeyData::FLAG_HIDDEN	},
		{	"ShowGalacticMap",          "Stargate Map",                 0 },
	};

const int GAME_KEY_DATA_COUNT = (sizeof(g_GameKeyData) / sizeof(g_GameKeyData[0]));

CGameKeys::CGameKeys (void) :
        m_bModified(false)

//	CGameKeys constructor

	{
	ASSERT(GAME_KEY_DATA_COUNT == CGameKeys::keyCount);
    SetLayout(layoutDefault);
	}

void CGameKeys::GetCommands (TArray<SCommandKeyDesc> &Result) const

//  GetCommands
//
//  Returns a list of all commands and their key bindings.

    {
    int i;

    Result.DeleteAll();
    Result.GrowToFit(keyCount);

    for (i = 0; i < keyCount; i++)
        {
        const SGameKeyData &Data = g_GameKeyData[i];

        //  If no name, or if it is hidden, then we skip

        if (Data.pszName == NULL
                || (Data.dwFlags & SGameKeyData::FLAG_HIDDEN))
            continue;

        //  Add the command

        SCommandKeyDesc *pNewCmd = Result.Insert();
        pNewCmd->iCmd = (Keys)i;
        pNewCmd->sCmdID = CString(Data.pszName);
        if (Data.pszLabel)
            pNewCmd->sCmdLabel = CString(Data.pszLabel);
        else
            pNewCmd->sCmdLabel = CONSTLIT("(unknown)");

        //  See what key we're bound to

        pNewCmd->dwVirtKey = GetKey(pNewCmd->iCmd);
        pNewCmd->sKeyID = GetKeyID(pNewCmd->dwVirtKey);
        }
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

DWORD CGameKeys::GetKey (Keys iCommand) const

//  GetKey
//
//  Returns the virtual key mapped to the given command.

    {
	int i;

	for (i = 0; i < 256; i++)
        if (m_iMap[i] == iCommand)
            {
            //  If this is a non-standard key, then skip it because
            //  we won't be able to see it in the keyboard UI.

            if (g_VirtKeyData[i].dwFlags & SVirtKeyData::FLAG_NON_STANDARD)
                continue;

            //  Found it

            return i;
            }

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

CString CGameKeys::GetKeyID (DWORD dwVirtKey)

//	GetKeyID
//
//	Returns the label for the given key

	{
    if (dwVirtKey >= 256)
        return NULL_STR;

	return CString(g_VirtKeyData[dwVirtKey].pszName, -1, true);
	}

CString CGameKeys::GetKeyLabel (DWORD dwVirtKey)

//	GetKeyLabel
//
//	Returns the label for the given key

	{
    if (dwVirtKey >= 256)
        return NULL_STR;

	return CString(g_VirtKeyData[dwVirtKey].pszLabel, -1, true);
	}

CString CGameKeys::GetLayoutID (ELayouts iLayout)

//  GetLayoutID
//
//  Returns the layout ID (which is used to identify a layout in XML).

    {
    switch (iLayout)
        {
        case layoutDefault:
            return LAYOUT_DEFAULT;

        case layoutCustom:
            return LAYOUT_CUSTOM;

        default:
            return NULL_STR;
        }
    }

CGameKeys::ELayouts CGameKeys::GetLayoutFromID (const CString &sLayoutID)

//  GetLayoutFromID
//
//  Parses a layout ID and return a layout. Returns layoutNone if we cannot
//  recognize the ID.

    {
    if (sLayoutID.IsBlank() || strEquals(sLayoutID, LAYOUT_DEFAULT))
        return layoutDefault;
    else if (strEquals(sLayoutID, LAYOUT_CUSTOM))
        return layoutCustom;
    else
        return layoutNone;
    }

CString CGameKeys::GetLayoutName (ELayouts iLayout) const

//  GetLayoutName
//
//  Returns the human readable name of the layout.

    {
    switch (iLayout)
        {
        case layoutDefault:
            return CONSTLIT("Default");

        case layoutCustom:
            return CONSTLIT("Custom");

        default:
            return CONSTLIT("(Unknown)");
        }
    }

ALERROR CGameKeys::ReadFromXML (CXMLElement *pDesc)

//	ReadFromXML
//
//	Read the key map from XML

	{
	int i;

    //  For backwards compatibility, we assume a custom layout if we don't
    //  have a layout specified.

    ELayouts iLayout = layoutCustom;
    bool bCompatible = true;

    //  Load the layout

    CString sLayout;
    if (pDesc->FindAttribute(LAYOUT_ATTRIB, &sLayout))
        {
        iLayout = GetLayoutFromID(sLayout);
        if (iLayout == layoutNone)
            iLayout = layoutDefault;
        bCompatible = false;
        }

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

	//	Copy to our custom map (we initialize to default for any key not 
    //  specified in the XML).

	for (i = 0; i < 256; i++)
		m_CustomMap[i] = keyNone;

    if (bCompatible)
        {
	    for (i = 0; i < DEFAULT_MAP_COUNT; i++)
		    m_CustomMap[DEFAULT_MAP[i].iVirtKey] = DEFAULT_MAP[i].iGameKey;
        }

	for (i = 0; i < 256; i++)
		if (iNewMap[i] != keyError)
			m_CustomMap[i] = iNewMap[i];

    //  Set the layout

    SetLayout(iLayout);
    m_bModified = false;

	return NOERROR;
	}

void CGameKeys::SetGameKey (const CString &sKeyID, Keys iCommand)

//  SetGameKey
//
//  Binds the command to the given key (in the custom layout only).

    {
    DWORD dwVirtKey = GetKey(sKeyID);
    if (dwVirtKey == INVALID_VIRT_KEY)
        return;

    m_CustomMap[(dwVirtKey < 256 ? dwVirtKey : 0)] = iCommand;

    //  If the custom map is currently selected, then change it.

    if (m_iLayout == layoutCustom)
        m_iMap[(dwVirtKey < 256 ? dwVirtKey : 0)] = iCommand;

    m_bModified = true;
    }

void CGameKeys::SetLayout (ELayouts iLayout)

//  SetLayout
//
//  Switches to the given layout

    {
    int i;

    switch (iLayout)
        {
        case layoutDefault:
            SetLayoutFromStatic(DEFAULT_MAP, DEFAULT_MAP_COUNT);
            break;

        case layoutCustom:
            for (i = 0; i < 256; i++)
                m_iMap[i] = m_CustomMap[i];
            break;

        default:
            //  Unknown layout, so do nothing

            return;
        }

    m_iLayout = iLayout;
    m_bModified = true;
    }

void CGameKeys::SetLayoutFromStatic (const SKeyMapEntry *pLayout, int iLayoutCount)

//  SetLayoutFromStatic
//
//  Sets the current layout from the given mapping

    {
    int i;

	for (i = 0; i < 256; i++)
		m_iMap[i] = keyNone;

	for (i = 0; i < iLayoutCount; i++)
		m_iMap[pLayout[i].iVirtKey] = pLayout[i].iGameKey;
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
	sData = strPatternSubst(CONSTLIT("\t<KeyMap layout='%s'>\r\n"), GetLayoutID(m_iLayout));
	if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	//	Write out the custom map

	for (i = 0; i < 256; i++)
		if (m_CustomMap[i] != keyNone)
			{
			CString sKey = (g_VirtKeyData[i].pszName ? CString(g_VirtKeyData[i].pszName, -1, true) : strPatternSubst(CONSTLIT("%x"), i));

			sData = strPatternSubst(CONSTLIT("\t\t<Map key=\"%s\" command=\"%s\"/>\r\n"),
					sKey,
					CString(g_GameKeyData[m_CustomMap[i]].pszName, -1, true));

			if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
				return error;
			}

	//	Close tag

	sData = CONSTLIT("\t</KeyMap>\r\n");
	if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	return NOERROR;
	}
