//	KeyboardMapData.h
//
//	Data for keyboard mapping.
//	Copyright (c) 2017 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

const CKeyboardMapSession::SKeyData CKeyboardMapSession::KEYBOARD_DATA[] =
    {
        //  Row 0

        {   "Escape",       1,   0,  2,  1,	"Esc",         symbolNone,     CKeyboardMapSession::FLAG_RESERVED  },
        {   "F1",           3,   0,  2,  1,	"F1",          symbolNone,     0   },
        {   "F2",           5,   0,  2,  1,	"F2",          symbolNone,     0   },
        {   "F3",           7,   0,  2,  1,	"F3",          symbolNone,     0   },
        {   "F4",           9,   0,  2,  1,	"F4",          symbolNone,     0   },
        {   "F5",           11,  0,  2,  1,	"F5",          symbolNone,     0   },
        {   "F6",           13,  0,  2,  1,	"F6",          symbolNone,     0   },
        {   "F7",           15,  0,  2,  1,	"F7",          symbolNone,     0   },
        {   "F8",           17,  0,  2,  1,	"F8",          symbolNone,     0   },
        {   "F9",           19,  0,  2,  1,	"F9",          symbolNone,     0   },
        {   "F10",          21,  0,  2,  1,	"F10",         symbolNone,     0   },
        {   "F11",          23,  0,  2,  1,	"F11",         symbolNone,     0   },
        {   "F12",          25,  0,  2,  1,	"F12",         symbolNone,     0   },

        //  Row 1

        {   "BackQuote",    0,   1,  2,  1,	"`",           symbolNone,     0    },
        {   "1",            2,   1,  2,  1,	"1",           symbolNone,     0    },
        {   "2",            4,   1,  2,  1,	"2",           symbolNone,     0    },
        {   "3",            6,   1,  2,  1,	"3",           symbolNone,     0    },
        {   "4",            8,   1,  2,  1,	"4",           symbolNone,     0    },
        {   "5",            10,  1,  2,  1,	"5",           symbolNone,     0    },
        {   "6",            12,  1,  2,  1,	"6",           symbolNone,     0    },
        {   "7",            14,  1,  2,  1,	"7",           symbolNone,     0    },
        {   "8",            16,  1,  2,  1,	"8",           symbolNone,     0    },
        {   "9",            18,  1,  2,  1,	"9",           symbolNone,     0    },
        {   "0",            20,  1,  2,  1,	"0",           symbolNone,     0   },
        {   "Minus",        22,  1,  2,  1,	"-",           symbolNone,     0   },
        {   "Equal",        24,  1,  2,  1,	"=",           symbolNone,     0   },
        {   "Backspace",    26,  1,  3,  1,	"Back",        symbolNone,     0   },

        //  Row 2

        {   "Tab",          0,   2,  3,  1,	"Tab",         symbolNone,     0    },
        {   "Q",            3,   2,  2,  1,	"Q",           symbolNone,     0    },
        {   "W",            5,   2,  2,  1,	"W",           symbolNone,     0    },
        {   "E",            7,   2,  2,  1,	"E",           symbolNone,     0    },
        {   "R",            9,   2,  2,  1,	"R",           symbolNone,     0    },
        {   "T",            11,  2,  2,  1,	"T",           symbolNone,     0    },
        {   "Y",            13,  2,  2,  1,	"Y",           symbolNone,     0    },
        {   "U",            15,  2,  2,  1,	"U",           symbolNone,     0    },
        {   "I",            17,  2,  2,  1,	"I",           symbolNone,     0    },
        {   "O",            19,  2,  2,  1,	"O",           symbolNone,     0    },
        {   "P",            21,  2,  2,  1,	"P",           symbolNone,     0    },
        {   "OpenBracket",  23,  2,  2,  1,	"[",           symbolNone,     0    },
        {   "CloseBracket", 25,  2,  2,  1,	"]",           symbolNone,     0    },
        {   "Backslash",    27,  2,  2,  1,	"\\",          symbolNone,     0    },

        //  Row 3

        {   "Capital",      1,   3,  3,  1,	"Caps",        symbolNone,     CKeyboardMapSession::FLAG_RESERVED    },
        {   "A",            4,   3,  2,  1,	"A",           symbolNone,     0    },
        {   "S",            6,   3,  2,  1,	"S",           symbolNone,     0    },
        {   "D",            8,   3,  2,  1,	"D",           symbolNone,     0    },
        {   "F",            10,  3,  2,  1,	"F",           symbolNone,     0    },
        {   "G",            12,  3,  2,  1,	"G",           symbolNone,     0    },
        {   "H",            14,  3,  2,  1,	"H",           symbolNone,     0    },
        {   "J",            16,  3,  2,  1,	"J",           symbolNone,     0    },
        {   "K",            18,  3,  2,  1,	"K",           symbolNone,     0    },
        {   "L",            20,  3,  2,  1,	"L",           symbolNone,     0    },
        {   "SemiColon",    22,  3,  2,  1,	";",           symbolNone,     0    },
        {   "Quote",        24,  3,  2,  1,	"'",           symbolNone,     0    },
        {   "Return",       26,  3,  3,  1,	"Enter",       symbolNone,     0    },

        //  Row 4

        {   "Shift",        2,   4,  3,  1,	"Shift",       symbolNone,     0    },
        {   "Z",            5,   4,  2,  1,	"Z",           symbolNone,     0    },
        {   "X",            7,   4,  2,  1,	"X",           symbolNone,     0    },
        {   "C",            9,   4,  2,  1,	"C",           symbolNone,     0    },
        {   "V",            11,  4,  2,  1,	"V",           symbolNone,     0    },
        {   "B",            13,  4,  2,  1,	"B",           symbolNone,     0    },
        {   "N",            15,  4,  2,  1,	"N",           symbolNone,     0    },
        {   "M",            17,  4,  2,  1,	"M",           symbolNone,     0    },
        {   "Comma",        19,  4,  2,  1,	",",           symbolNone,     0    },
        {   "Period",       21,  4,  2,  1,	".",           symbolNone,     0    },
        {   "Slash",        23,  4,  2,  1,	"/",           symbolNone,     0    },
        {   NULL,           25,  4,  3,  1,	"Shift",       symbolNone,     0    },

        //  Row 5

        {   "Control",      6,   5,  2,  1,	"Ctrl",        symbolNone,     0    },
        {   "Space",        8,   5,  14, 1,	"",            symbolNone,     0    },
        {   NULL,           22,  5,  2,  1,	"Ctrl",        symbolNone,     0    },

        //  Number Pad

        {   "Insert",       31,  1,  2,  1,	"Ins",         symbolNone,     0  },
        {   "Home",         33,  1,  2,  1,	"Home",        symbolNone,     0 },
        {   "PageUp",       35,  1,  2,  1,	"PgUp",        symbolNone,     0 },

        {   "Delete",       31,  2,  2,  1,	"Del",         symbolNone,     0  },
        {   "End",          33,  2,  2,  1,	"End",         symbolNone,     0 },
        {   "PageDown",     35,  2,  2,  1,	"PgDn",        symbolNone,     0 },

        {   "Up",           33,  4,  2,  1,	NULL,          symbolArrowUp,      0  },
        {   "Left",         31,  5,  2,  1,	NULL,          symbolArrowLeft,    0  },
        {   "Down",         33,  5,  2,  1,	NULL,          symbolArrowDown,    0 },
        {   "Right",        35,  5,  2,  1,	NULL,          symbolArrowRight,   0 },
    };

const int CKeyboardMapSession::KEYBOARD_DATA_COUNT = (sizeof(KEYBOARD_DATA) / sizeof(KEYBOARD_DATA[0]));

const CKeyboardMapSession::SKeyData CKeyboardMapSession::NUMPAD_DATA[] =
    {
        //  Row 0

        {   "NumLock",		0,	0,	2,	1,	"Num Lock",		symbolNone,     CKeyboardMapSession::FLAG_RESERVED  },
        {   "NumpadSlash",	2,	0,	2,	1,	"/",			symbolNone,		0  },
        {   "NumpadStar",	4,	0,	2,	1,	"*",			symbolNone,		0  },
        {   "NumpadMinus",	6,	0,	2,	1,	"-",			symbolNone,		0  },

        //  Row 1

        {   "Numpad7",		0,	1,	2,	1,	"7",			symbolNone,		0  },
        {   "Numpad8",		2,	1,	2,	1,	"8",			symbolNone,		0  },
        {   "Numpad9",		4,	1,	2,	1,	"9",			symbolNone,		0  },
        {   "NumpadPlus",	6,	1,	2,	2,	"+",			symbolNone,		0  },

		//	Row 2

        {   "Numpad4",		0,	2,	2,	1,	"4",			symbolNone,		0  },
        {   "Numpad5",		2,	2,	2,	1,	"5",			symbolNone,		0  },
        {   "Numpad6",		4,	2,	2,	1,	"6",			symbolNone,		0  },

		//	Row 3

        {   "Numpad1",		0,	3,	2,	1,	"1",			symbolNone,		0  },
        {   "Numpad2",		2,	3,	2,	1,	"2",			symbolNone,		0  },
        {   "Numpad3",		4,	3,	2,	1,	"3",			symbolNone,		0  },
        {   "NumpadEnter",	6,	3,	2,	2,	"Enter",		symbolNone,		0  },

		//	Row 4

        {   "Numpad0",		0,	4,	4,	1,	"0",			symbolNone,		0  },
        {   "NumpadPeriod",	4,	4,	2,	1,	".",			symbolNone,		0  },
	};

const int CKeyboardMapSession::NUMPAD_DATA_COUNT = (sizeof(NUMPAD_DATA) / sizeof(NUMPAD_DATA[0]));

const CKeyboardMapSession::SKeyData CKeyboardMapSession::MOUSE_DATA[] =
    {
        {   "LButton",		0,	0,	3,	2,	"",				symbolNone,     0  },
        {   "MButton",		3,	0,	1,	2,	"",				symbolNone,		0  },
        {   "RButton",		4,	0,	3,	2,	"",				symbolNone,		0  },

        {   "",				0,	2,	7,	3,	"",				symbolNone,		CKeyboardMapSession::FLAG_RESERVED  },
	};

const int CKeyboardMapSession::MOUSE_DATA_COUNT = (sizeof(MOUSE_DATA) / sizeof(MOUSE_DATA[0]));

