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

		{	VK_LBUTTON,			CGameKeys::keyFireWeapon },
		{	VK_RBUTTON,			CGameKeys::keyThrustForward },

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

struct SGameKeyData
	{
    enum EFlags
        {
        FLAG_HIDDEN =       0x00000001,
        FLAG_DEBUG_ONLY =   0x00000002,
		FLAG_STATEFULL =	0x00000004,		//	Tracks when key is down vs. up (thrust, fire, etc.).
		FLAG_NO_REPEAT =	0x00000008,		//	Ignore repeated keys
        };

	char *pszName;
    char *pszLabel;
    DWORD dwFlags;
	};

SGameKeyData g_GameKeyData[CGameKeys::keyCount] =
	{
		{	NULL },	//	keyNone

		{	"Autopilot",            "Autopilot",                    SGameKeyData::FLAG_NO_REPEAT },
		{	"EnableDevices",        "Enable/Disable Devices",       SGameKeyData::FLAG_NO_REPEAT },
		{	"Communications",       "Communications",               SGameKeyData::FLAG_NO_REPEAT },
		{	"Dock",                 "Request Dock",                 SGameKeyData::FLAG_NO_REPEAT },
		{	"TargetNextFriendly",   "Target Next Friendly",         0 },
		{	"EnterGate",            "Enter Stargate",               SGameKeyData::FLAG_NO_REPEAT },
		{	"InvokePower",          "Invoke Power",                 SGameKeyData::FLAG_NO_REPEAT },
		{	"ShowMap",              "System Map",                   SGameKeyData::FLAG_NO_REPEAT },
		{	"Pause",                "Pause Game",                   SGameKeyData::FLAG_NO_REPEAT },
		{	"CommandSquadron",      "Squadron Communications",      SGameKeyData::FLAG_NO_REPEAT },
		{	"ClearTarget",          "Clear Target",                 0 },
		{	"ShipStatus",           "Ship's Status",                SGameKeyData::FLAG_NO_REPEAT },
		{	"TargetNextEnemy",      "Target Next Enemy",            0 },
		{	"UseItem",              "Use Item",                     SGameKeyData::FLAG_NO_REPEAT },
		{	"NextWeapon",           "Select Next Weapon",           SGameKeyData::FLAG_NO_REPEAT },
		{	"ThrustForward",        "Forward Thrust",               SGameKeyData::FLAG_STATEFULL },
		{	"Stop",                 "Stop Momentum",                SGameKeyData::FLAG_STATEFULL },
		{	"RotateLeft",           "Rotate Left",                  SGameKeyData::FLAG_STATEFULL },
		{	"RotateRight",          "Rotate Right",                 SGameKeyData::FLAG_STATEFULL },
		{	"FireWeapon",           "Fire Weapon",                  SGameKeyData::FLAG_STATEFULL },
		{	"FireMissile",          "Fire Missile",                 SGameKeyData::FLAG_STATEFULL },
		{	"NextMissile",          "Select Next Missile",          SGameKeyData::FLAG_NO_REPEAT },
		{	"ShowHelp",             "Help",                         SGameKeyData::FLAG_NO_REPEAT },
		{	"ShowGameStats",        "Game Stats",                   SGameKeyData::FLAG_NO_REPEAT },
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
		{	"DisableAllDevices",    NULL,                           SGameKeyData::FLAG_HIDDEN },
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

		{	"TargetPreviousFriendly",   "Target Previous Friendly",     SGameKeyData::FLAG_NO_REPEAT | SGameKeyData::FLAG_HIDDEN	},
		{	"TargetPreviousEnemy",      "Target Previous Enemy",        SGameKeyData::FLAG_NO_REPEAT | SGameKeyData::FLAG_HIDDEN	},
		{	"PreviousWeapon",           "Select Previous Weapon",       SGameKeyData::FLAG_NO_REPEAT | SGameKeyData::FLAG_HIDDEN	},
		{	"PreviousMissile",          "Select Previous Missile",      SGameKeyData::FLAG_NO_REPEAT | SGameKeyData::FLAG_HIDDEN	},
		{	"ShowGalacticMap",          "Stargate Map",                 SGameKeyData::FLAG_NO_REPEAT },
	};

const int GAME_KEY_DATA_COUNT = (sizeof(g_GameKeyData) / sizeof(g_GameKeyData[0]));

CGameKeys::CGameKeys (void) :
        m_bModified(false)

//	CGameKeys constructor

	{
	ASSERT(GAME_KEY_DATA_COUNT == CGameKeys::keyCount);
    SetLayout(layoutDefault);

	for (int i = 0; i < 256; i++)
		m_CustomMap[i] = keyNone;
	}

void CGameKeys::GetCommands (TArray<SCommandKeyDesc> &Result) const

//  GetCommands
//
//  Returns a list of all commands and their key bindings.

    {
    int i;

	TSortMap<int, int> CmdToIndex;

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

		CmdToIndex.SetAt(i, Result.GetCount() - 1);
		}

	//	Now add all bindings

	for (i = 0; i < 256; i++)
		{
		if (m_iMap[i] != keyNone)
			{
			//	Find the command

			int *pIndex = CmdToIndex.GetAt(m_iMap[i]);
			if (pIndex == NULL)
				continue;

			SCommandKeyDesc &Cmd = Result[*pIndex];

			//	Add the key

			SBindingDesc *pBinding = Cmd.Keys.Insert();
			pBinding->dwVirtKey = (DWORD)i;
			pBinding->sKeyID = CVirtualKeyData::GetKeyID(i);
			}
		}
    }

CGameKeys::Keys CGameKeys::GetGameCommand (const CString &sCmd)

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

CGameKeys::Keys CGameKeys::GetGameCommandFromChar (char chChar) const

//  GetGameCommand
//
//  Returns game command from character

    {
    const DWORD VK_FIRST_NUMBER = 0x30;
    const DWORD VK_FIRST_LETTER = 0x41;

    //  Convert from character to virtual key code

    if (chChar >= '0' && chChar <= '9')
        return GetGameCommand(VK_FIRST_NUMBER + (DWORD)(chChar - '0'));
    else if (chChar >= 'A' && chChar <= 'Z')
        return GetGameCommand(VK_FIRST_LETTER + (DWORD)(chChar - 'A'));
    else if (chChar >= 'a' && chChar <= 'z')
        return GetGameCommand(VK_FIRST_LETTER + (DWORD)(chChar - 'a'));
    else
        return keyError;
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

            if (CVirtualKeyData::GetKeyFlags(i) & CVirtualKeyData::FLAG_NON_STANDARD)
                continue;

            //  Found it

            return i;
            }

    return CVirtualKeyData::INVALID_VIRT_KEY;
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

bool CGameKeys::IsNonRepeatCommand (Keys iCommand) const

//	IsNonRepeatCommand
//
//	Returns TRUE if this command ignored repeated keys.

	{
	ASSERT(iCommand > 0 && iCommand < keyCount);
    const SGameKeyData &Data = g_GameKeyData[iCommand];
	return (Data.dwFlags & SGameKeyData::FLAG_NO_REPEAT ? true : false);
	}

bool CGameKeys::IsStatefulCommand (Keys iCommand) const

//	IsStatefulCommand
//
//	Returns TRUE if this command tracks the state of a key (up or down).

	{
	ASSERT(iCommand > 0 && iCommand < keyCount);
    const SGameKeyData &Data = g_GameKeyData[iCommand];
	return (Data.dwFlags & SGameKeyData::FLAG_STATEFULL ? true : false);
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
		
		DWORD dwVirtKey = CVirtualKeyData::GetKey(pMap->GetAttribute(KEY_ATTRIB));
		if (dwVirtKey == CVirtualKeyData::INVALID_VIRT_KEY)
			{
			kernelDebugLogPattern("Unknown key: %s", pMap->GetAttribute(KEY_ATTRIB));
			continue;
			}

		Keys iCommand = GetGameCommand(pMap->GetAttribute(COMMAND_ATTRIB));
		if (iCommand == keyError)
			{
			kernelDebugLogPattern("Unknown game command: %s", pMap->GetAttribute(COMMAND_ATTRIB));
			continue;
			}

		if (iNewMap[dwVirtKey] != keyError)
			{
			kernelDebugLogPattern("Duplicate key mapping: %s", pMap->GetAttribute(COMMAND_ATTRIB));
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
	int i;

    DWORD dwVirtKey = CVirtualKeyData::GetKey(sKeyID);
    if (dwVirtKey == CVirtualKeyData::INVALID_VIRT_KEY)
        return;

	//	If we're trying to change the default layout, switch over to the custom
	//	layout.

	if (m_iLayout == layoutDefault)
		{
        for (i = 0; i < 256; i++)
            m_CustomMap[i] = m_iMap[i];

		m_iLayout = layoutCustom;
		}

	//	Update the key

    m_CustomMap[(dwVirtKey < 256 ? dwVirtKey : 0)] = iCommand;
    m_iMap[(dwVirtKey < 256 ? dwVirtKey : 0)] = iCommand;

	//	Need to save out

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
			CString sKey = CVirtualKeyData::GetKeyID(i);
			if (sKey.IsBlank())
				sKey = strPatternSubst(CONSTLIT("0x%02x"), i);

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

