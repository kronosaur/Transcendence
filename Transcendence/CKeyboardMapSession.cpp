//	CKeyboardMapSession.cpp
//
//	CKeyboardMapSession class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define ALIGN_CENTER							CONSTLIT("center")

#define CMD_CLOSE_SESSION						CONSTLIT("cmdCloseSession")
#define CMD_NEXT_LAYOUT 						CONSTLIT("cmdNextLayout")
#define CMD_OK_SESSION							CONSTLIT("cmdOKSession")
#define CMD_PREV_LAYOUT 						CONSTLIT("cmdPrevLayout")

#define ID_CTRL_TITLE							CONSTLIT("ctrlTitle")
#define ID_LAYOUT_LABEL							CONSTLIT("idLayoutLabel")
#define ID_SETTINGS								CONSTLIT("idSettings")

#define EVENT_ON_CLICK							CONSTLIT("onClick")

#define PROP_COLOR								CONSTLIT("color")
#define PROP_ENABLED							CONSTLIT("enabled")
#define PROP_FONT								CONSTLIT("font")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_TEXT								CONSTLIT("text")
#define PROP_TEXT_ALIGN_HORZ					CONSTLIT("textAlignHorz")

const int KEY_SPACING = 4;
const int KEY_CORNER_RADIUS = 3;
const int LABEL_SPACING = 4;
const int LABEL_COLUMN_SPACING = 14;

const int LINE_WIDTH = 2;

const int KEYBOARD_HEIGHT = 220;
const int KEYBOARD_ROWS = 6;
const int KEYBOARD_COLS = 37;
const int KEYBOARD_OFFSET_X = 50;
const int KEYBOARD_OFFSET_Y = 60;

const int MAJOR_PADDING_TOP =					20;
const int SMALL_BUTTON_HEIGHT =					48;
const int SMALL_BUTTON_WIDTH =					48;
const int SMALL_SPACING_HORZ =					8;
const int SMALL_SPACING_VERT =					8;

const CKeyboardMapSession::SKeyData CKeyboardMapSession::KEYBOARD_DATA[] =
    {
        //  Row 0

        {   "Escape",       1,   0,  2,  "Esc",         symbolNone,     CKeyboardMapSession::FLAG_RESERVED  },
        {   "F1",           3,   0,  2,  "F1",          symbolNone,     0   },
        {   "F2",           5,   0,  2,  "F2",          symbolNone,     0   },
        {   "F3",           7,   0,  2,  "F3",          symbolNone,     0   },
        {   "F4",           9,   0,  2,  "F4",          symbolNone,     0   },
        {   "F5",           11,  0,  2,  "F5",          symbolNone,     0   },
        {   "F6",           13,  0,  2,  "F6",          symbolNone,     0   },
        {   "F7",           15,  0,  2,  "F7",          symbolNone,     0   },
        {   "F8",           17,  0,  2,  "F8",          symbolNone,     0   },
        {   "F9",           19,  0,  2,  "F9",          symbolNone,     0   },
        {   "F10",          21,  0,  2,  "F10",         symbolNone,     0   },
        {   "F11",          23,  0,  2,  "F11",         symbolNone,     0   },
        {   "F12",          25,  0,  2,  "F12",         symbolNone,     0   },

        //  Row 1

        {   "BackQuote",    0,   1,  2,  "`",           symbolNone,     0    },
        {   "1",            2,   1,  2,  "1",           symbolNone,     0    },
        {   "2",            4,   1,  2,  "2",           symbolNone,     0    },
        {   "3",            6,   1,  2,  "3",           symbolNone,     0    },
        {   "4",            8,   1,  2,  "4",           symbolNone,     0    },
        {   "5",            10,  1,  2,  "5",           symbolNone,     0    },
        {   "6",            12,  1,  2,  "6",           symbolNone,     0    },
        {   "7",            14,  1,  2,  "7",           symbolNone,     0    },
        {   "8",            16,  1,  2,  "8",           symbolNone,     0    },
        {   "9",            18,  1,  2,  "9",           symbolNone,     0    },
        {   "0",            20,  1,  2,  "0",           symbolNone,     0   },
        {   "Minus",        22,  1,  2,  "-",           symbolNone,     0   },
        {   "Equal",        24,  1,  2,  "=",           symbolNone,     0   },
        {   "Backspace",    26,  1,  3,  "Back",        symbolNone,     0   },

        //  Row 2

        {   "Tab",          0,   2,  3,  "Tab",         symbolNone,     0    },
        {   "Q",            3,   2,  2,  "Q",           symbolNone,     0    },
        {   "W",            5,   2,  2,  "W",           symbolNone,     0    },
        {   "E",            7,   2,  2,  "E",           symbolNone,     0    },
        {   "R",            9,   2,  2,  "R",           symbolNone,     0    },
        {   "T",            11,  2,  2,  "T",           symbolNone,     0    },
        {   "Y",            13,  2,  2,  "Y",           symbolNone,     0    },
        {   "U",            15,  2,  2,  "U",           symbolNone,     0    },
        {   "I",            17,  2,  2,  "I",           symbolNone,     0    },
        {   "O",            19,  2,  2,  "O",           symbolNone,     0    },
        {   "P",            21,  2,  2,  "P",           symbolNone,     0    },
        {   "OpenBracket",  23,  2,  2,  "[",           symbolNone,     0    },
        {   "CloseBracket", 25,  2,  2,  "]",           symbolNone,     0    },
        {   "Backslash",    27,  2,  2,  "\\",          symbolNone,     0    },

        //  Row 3

        {   "Capital",      1,   3,  3,  "Caps",        symbolNone,     CKeyboardMapSession::FLAG_RESERVED    },
        {   "A",            4,   3,  2,  "A",           symbolNone,     0    },
        {   "S",            6,   3,  2,  "S",           symbolNone,     0    },
        {   "D",            8,   3,  2,  "D",           symbolNone,     0    },
        {   "F",            10,  3,  2,  "F",           symbolNone,     0    },
        {   "G",            12,  3,  2,  "G",           symbolNone,     0    },
        {   "H",            14,  3,  2,  "H",           symbolNone,     0    },
        {   "J",            16,  3,  2,  "J",           symbolNone,     0    },
        {   "K",            18,  3,  2,  "K",           symbolNone,     0    },
        {   "L",            20,  3,  2,  "L",           symbolNone,     0    },
        {   "SemiColon",    22,  3,  2,  ";",           symbolNone,     0    },
        {   "Quote",        24,  3,  2,  "'",           symbolNone,     0    },
        {   "Return",       26,  3,  3,  "Enter",       symbolNone,     0    },

        //  Row 4

        {   "Shift",        2,   4,  3,  "Shift",       symbolNone,     0    },
        {   "Z",            5,   4,  2,  "Z",           symbolNone,     0    },
        {   "X",            7,   4,  2,  "X",           symbolNone,     0    },
        {   "C",            9,   4,  2,  "C",           symbolNone,     0    },
        {   "V",            11,  4,  2,  "V",           symbolNone,     0    },
        {   "B",            13,  4,  2,  "B",           symbolNone,     0    },
        {   "N",            15,  4,  2,  "N",           symbolNone,     0    },
        {   "M",            17,  4,  2,  "M",           symbolNone,     0    },
        {   "Comma",        19,  4,  2,  ",",           symbolNone,     0    },
        {   "Period",       21,  4,  2,  ".",           symbolNone,     0    },
        {   "Slash",        23,  4,  2,  "/",           symbolNone,     0    },
        {   NULL,           25,  4,  3,  "Shift",       symbolNone,     0    },

        //  Row 5

        {   "Control",      6,   5,  2,  "Ctrl",        symbolNone,     0    },
        {   "Space",        8,   5,  14, "",            symbolNone,     0    },
        {   NULL,           22,  5,  2,  "Ctrl",        symbolNone,     0    },

        //  Number Pad

        {   "Insert",       31,  1,  2,  "Ins",         symbolNone,     0  },
        {   "Home",         33,  1,  2,  "Home",        symbolNone,     0 },
        {   "PageUp",       35,  1,  2,  "PgUp",        symbolNone,     0 },

        {   "Delete",       31,  2,  2,  "Del",         symbolNone,     0  },
        {   "End",          33,  2,  2,  "End",         symbolNone,     0 },
        {   "PageDown",     35,  2,  2,  "PgDn",        symbolNone,     0 },

        {   "Up",           33,  4,  2,  NULL,          symbolArrowUp,      0  },
        {   "Left",         31,  5,  2,  NULL,          symbolArrowLeft,    0  },
        {   "Down",         33,  5,  2,  NULL,          symbolArrowDown,    0 },
        {   "Right",        35,  5,  2,  NULL,          symbolArrowRight,   0 },
    };

const int CKeyboardMapSession::KEYBOARD_DATA_COUNT = (sizeof(KEYBOARD_DATA) / sizeof(KEYBOARD_DATA[0]));

CKeyboardMapSession::CKeyboardMapSession (CHumanInterface &HI, CCloudService &Service, CGameSettings &Settings) : IHISession(HI),
        m_Service(Service),
        m_Settings(Settings),
        m_bEditable(false),
        m_iHoverKey(-1),
        m_iHoverCommand(-1),
        m_iSelectedCommand(-1)

//  CKeyboardMapSession constructor

    {
    }

void CKeyboardMapSession::ArrangeCommandLabels (const RECT &rcRect, const RECT &rcKeyboard)

//  ArrangeCommandLabels
//
//  Arrange command labels properly

    {
    bool bDirectLine = false;

    int i;
	const CVisualPalette &VI = m_HI.GetVisuals();
    const CG16bitFont &LabelFont = VI.GetFont(fontMedium);

    int xCenter = rcRect.left + (RectWidth(rcRect) / 2);
    int yCenter = rcRect.top + (RectHeight(rcRect) / 2);

    int cxLabelSpacing = 2 * LABEL_SPACING;
    int cyLabelSpacing = 2 * LABEL_SPACING;

    //  Generate an array describing the labels for the arranger.

    TArray<CLabelArranger::SLabelDesc> Labels;
    Labels.InsertEmpty(m_Commands.GetCount());
    for (i = 0; i < m_Commands.GetCount(); i++)
        {
        Labels[i].cxWidth = m_Commands[i].cxLabel + cxLabelSpacing;
        Labels[i].cyHeight = LabelFont.GetHeight() + cyLabelSpacing;

        if (m_Commands[i].iKeyBinding != -1)
            {
            const SKeyDesc &Key = m_Keys[m_Commands[i].iKeyBinding];
            Labels[i].xDest = Key.rcRect.left + (RectWidth(Key.rcRect) / 2);
            Labels[i].yDest = Key.rcRect.top + (RectHeight(Key.rcRect) / 2);
            }
        else
            {
            Labels[i].xDest = xCenter;
            Labels[i].yDest = yCenter;
            }
        }

    //  Initialize the arranger and arrange the labels.

    CLabelArranger Arranger;
    Arranger.SetStyle(CLabelArranger::styleSideColumns);
    Arranger.SetBounds(rcRect);
    Arranger.SetRadius(1024);
    Arranger.AddExclusion(rcKeyboard);
    Arranger.Arrange(Labels);

    //  Now initialize the command positions

    for (i = 0; i < m_Commands.GetCount(); i++)
        {
        m_Commands[i].rcRect = Labels[i].rcRect;
        m_Commands[i].rcRect.bottom -= cyLabelSpacing;
        }

    //  Now compute lines to connect commands to keys

    if (bDirectLine)
        {
        for (i = 0; i < m_Commands.GetCount(); i++)
            {
            SCommandDesc &Command = m_Commands[i];
            if (Command.iKeyBinding != -1)
                {
                const SKeyDesc &Key = m_Keys[m_Commands[i].iKeyBinding];

                //  Are we on the left or right?

                bool bLeftOfKey = (Command.rcRect.right < Key.rcRect.left);

                //  Start in the middle of the command

                POINT *pPt = Command.Line.Insert();
                pPt->x = (bLeftOfKey ? Command.rcRect.right : Command.rcRect.left);
                pPt->y = Command.rcRect.top + (RectHeight(Command.rcRect) / 2);

                //  Move to the key

                pPt = Command.Line.Insert();
                pPt->x = (bLeftOfKey ? Key.rcRect.left : Key.rcRect.right);
                pPt->y = Key.rcRect.top + (RectHeight(Key.rcRect) / 2);
                }
            }
        }
    else
        {
        for (i = 0; i < m_Commands.GetCount(); i++)
            {
            SCommandDesc &Command = m_Commands[i];
            if (Command.iKeyBinding != -1)
                {
                const SKeyDesc &Key = m_Keys[m_Commands[i].iKeyBinding];
                int yCommand = Command.rcRect.top + (RectHeight(Command.rcRect) / 2);

                //  Are we on the left or right?

                bool bLeftOfKey = (Command.rcRect.right < Key.rcRect.left);
                bool bInsideKey = (yCommand >= Key.rcRect.top + 2 && yCommand < Key.rcRect.bottom - 2);
                bool bAboveKey = (yCommand < Key.rcRect.top + 2);

                //  Start in the middle of the command

                POINT *pPt = Command.Line.Insert();
                pPt->x = (bLeftOfKey ? Command.rcRect.right : Command.rcRect.left);
                pPt->y = yCommand;
            
                //  Move to the key x coord

                int xOffset = (bInsideKey ? 0 : KEY_SPACING);
                pPt = Command.Line.Insert();
                pPt->x = (bLeftOfKey ? Key.rcRect.left + xOffset : Key.rcRect.right - xOffset - 1);
                pPt->y = Command.Line[0].y;

                //  Now move to the key

                if (!bInsideKey)
                    {
                    pPt = Command.Line.Insert();
                    pPt->x = Command.Line[1].x;
                    pPt->y = (bAboveKey ? Key.rcRect.top : Key.rcRect.bottom - 1);
                    }
                }
            }
        }
    }

void CKeyboardMapSession::CmdNextLayout (void)

//  CmdNextLayout
//
//  Select the next layout

    {
    CGameKeys::ELayouts iCurLayout = m_Settings.GetKeyMap().GetLayout();
    if (iCurLayout < CGameKeys::layoutCount - 1)
        {
        CGameKeys::ELayouts iNewLayout = (CGameKeys::ELayouts)(iCurLayout + 1);
        m_Settings.GetKeyMap().SetLayout(iNewLayout);
        UpdateLayoutControls();
        }
    }

void CKeyboardMapSession::CmdPrevLayout (void)

//  CmdPrevLayout
//
//  Select the previous layout

    {
    CGameKeys::ELayouts iCurLayout = m_Settings.GetKeyMap().GetLayout();
    if (iCurLayout > CGameKeys::layoutDefault)
        {
        CGameKeys::ELayouts iNewLayout = (CGameKeys::ELayouts)(iCurLayout - 1);
        m_Settings.GetKeyMap().SetLayout(iNewLayout);
        UpdateLayoutControls();
        }
    }

void CKeyboardMapSession::CreateLayoutControls (void)

//  CreateLayoutControls
//
//  Create arrows to select a layout

    {
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	RECT rcRect;
	VI.GetWidescreenRect(&rcRect);

    //  Create a sequencer to hold all controls

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(rcRect.left, rcRect.top), &pRoot);
    int y = MAJOR_PADDING_TOP;

	//	Create buttons to select the layout

	int xCenter = (RectWidth(rcRect) / 2);
	int xHalfSpacing = SMALL_SPACING_HORZ / 2;

	IAnimatron *pLeftButton;
	VI.CreateImageButtonSmall(NULL, 
			CMD_PREV_LAYOUT, 
			xCenter - xHalfSpacing - SMALL_BUTTON_WIDTH, 
			y,
			&VI.GetImage(imageSmallLeftIcon),
			0,
			&pLeftButton);

	RegisterPerformanceEvent(pLeftButton, EVENT_ON_CLICK, CMD_PREV_LAYOUT);
	pRoot->AddTrack(pLeftButton, 0);

	IAnimatron *pRightButton;
	VI.CreateImageButtonSmall(NULL, 
			CMD_NEXT_LAYOUT, 
			xCenter + xHalfSpacing, 
			y,
			&VI.GetImage(imageSmallRightIcon),
			0,
			&pRightButton);

	RegisterPerformanceEvent(pRightButton, EVENT_ON_CLICK, CMD_NEXT_LAYOUT);
	pRoot->AddTrack(pRightButton, 0);

	//	Label

	IAnimatron *pLabel = new CAniText;
    pLabel->SetID(ID_LAYOUT_LABEL);
	pLabel->SetPropertyVector(PROP_POSITION, CVector(0.0, y + SMALL_BUTTON_HEIGHT + SMALL_SPACING_VERT));
	pLabel->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), 100.0));
	pLabel->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
	pLabel->SetPropertyFont(PROP_FONT, &SubTitleFont);
	pLabel->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_CENTER);

	pRoot->AddTrack(pLabel, 0);
	StartPerformance(pRoot, ID_SETTINGS, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
    }

bool CKeyboardMapSession::HitTest (int x, int y, STargetCtx &Ctx)

//  HitTest
//
//  Set if we've clicked on an element.

    {
    int i;
    POINT pt;
    pt.x = x;
    pt.y = y;

    //  Check all keys

    for (i = 0; i < m_Keys.GetCount(); i++)
        {
        const SKeyDesc &Key = m_Keys[i];
        if (PtInRect(&Key.rcRect, pt))
            {
            Ctx.bInKey = true;
            Ctx.iKeyIndex = i;
            Ctx.iCmdIndex = Key.iCmdBinding;
            return true;
            }
        }

    //  Check all commands

    for (i = 0; i < m_Commands.GetCount(); i++)
        {
        const SCommandDesc &Command = m_Commands[i];
        if (PtInRect(&Command.rcRect, pt))
            {
            Ctx.bInKey = false;
            Ctx.iCmdIndex = i;
            Ctx.iKeyIndex = Command.iKeyBinding;
            return true;
            }
        }

    //  Noting hit

    return false;
    }

void CKeyboardMapSession::LoadCommandMapping (void)

//  LoadCommandMapping
//
//  Loads the current keyboard mapping and creates command entries and 
//  connections appropriately.

    {
    int i;
	const CVisualPalette &VI = m_HI.GetVisuals();
    const CG16bitFont &LabelFont = VI.GetFont(fontMedium);

	RECT rcRect;
	VI.GetWidescreenRect(&rcRect);

    //  Reset keyboard to remove bindings

    for (i = 0; i < m_Keys.GetCount(); i++)
        m_Keys[i].iCmdBinding = -1;

    //  Load commands

    TArray<CGameKeys::SCommandKeyDesc> Commands;
    m_Settings.GetKeyMap().GetCommands(Commands);

    m_Commands.DeleteAll();
    m_Commands.GrowToFit(Commands.GetCount());
    for (i = 0; i < Commands.GetCount(); i++)
        {
        int iCmdIndex = m_Commands.GetCount();
        SCommandDesc *pNewCmd = m_Commands.Insert();
        pNewCmd->iCmd = Commands[i].iCmd;
        pNewCmd->sLabel = Commands[i].sCmdLabel;
        pNewCmd->cxLabel = LabelFont.MeasureText(pNewCmd->sLabel);

        //  See what key we're bound to

        if (!Commands[i].sKeyID.IsBlank())
            {
            int *pKey = m_KeyIDToIndex.GetAt(Commands[i].sKeyID);
            if (pKey && m_Keys[*pKey].iCmdBinding == -1)
                {
                pNewCmd->iKeyBinding = *pKey;
                m_Keys[*pKey].iCmdBinding = iCmdIndex;
                }
            else
                pNewCmd->iKeyBinding = -1;
            }
        else
            pNewCmd->iKeyBinding = -1;
        }

    //  Figure out where each command label should go

    RECT rcKeyboard;
    rcKeyboard.left = m_xKeyboard;
    rcKeyboard.top = m_yKeyboard;
    rcKeyboard.right = m_xKeyboard + m_cxKeyboard;
    rcKeyboard.bottom = m_yKeyboard + m_cyKeyboard;

    int cxBoundsSpacing = Max(0, (int)Min(rcRect.right - rcKeyboard.right, rcKeyboard.left - rcRect.left) - (LABEL_COLUMN_SPACING * m_cxKeyCol));
    RECT rcBounds = rcRect;
    rcBounds.top += MAJOR_PADDING_TOP + SMALL_BUTTON_HEIGHT + SMALL_SPACING_VERT + VI.GetFont(fontSubTitle).GetHeight();
    rcBounds.left = rcRect.left + cxBoundsSpacing;
    rcBounds.right = rcRect.right - cxBoundsSpacing;
    ArrangeCommandLabels(rcBounds, rcKeyboard);
    }

ALERROR CKeyboardMapSession::OnCommand (const CString &sCmd, void *pData)

//  OnCommand
//
//  Handle commands

    {
	if (strEquals(sCmd, CMD_CLOSE_SESSION))
       	m_HI.ClosePopupSession();
	else if (strEquals(sCmd, CMD_OK_SESSION))
       	m_HI.ClosePopupSession();
	else if (strEquals(sCmd, CMD_NEXT_LAYOUT))
		CmdNextLayout();
	else if (strEquals(sCmd, CMD_PREV_LAYOUT))
		CmdPrevLayout();

    return NOERROR;
    }

ALERROR CKeyboardMapSession::OnInit (CString *retsError)

//  OnInit
//
//  Initialize the session.

    {
    int i;

	const CVisualPalette &VI = m_HI.GetVisuals();
    const CG16bitFont &LabelFont = VI.GetFont(fontMedium);

	RECT rcRect;
	VI.GetWidescreenRect(&rcRect);

    //  Keyboard metrics

    m_cyKeyRow = AlignDown((KEYBOARD_HEIGHT + KEY_SPACING) / KEYBOARD_ROWS, 2);
    m_cxKeyCol = m_cyKeyRow / 2;

    m_cxKeyboard = (KEYBOARD_COLS * m_cxKeyCol) - KEY_SPACING;
    m_cyKeyboard = (KEYBOARD_ROWS * m_cyKeyRow) - KEY_SPACING;

    int cyKey = m_cyKeyRow - KEY_SPACING;

    m_xKeyboard = rcRect.left + (RectWidth(rcRect) - m_cxKeyboard) / 2 + KEYBOARD_OFFSET_X;
    m_yKeyboard = rcRect.top + (RectHeight(rcRect) - m_cyKeyboard) / 2 + KEYBOARD_OFFSET_Y;

    //  Initialize all the keys metrics

    m_Keys.InsertEmpty(KEYBOARD_DATA_COUNT);
    m_KeyIDToIndex.GrowToFit(KEYBOARD_DATA_COUNT);
    for (i = 0; i < m_Keys.GetCount(); i++)
        {
        if (KEYBOARD_DATA[i].pszKeyID)
            m_Keys[i].sKeyID = CString(KEYBOARD_DATA[i].pszKeyID);

        if (KEYBOARD_DATA[i].pszLabel)
            m_Keys[i].sLabel = CString(KEYBOARD_DATA[i].pszLabel);

        m_Keys[i].iSymbol = KEYBOARD_DATA[i].iSymbol;

        m_Keys[i].rcRect.left = m_xKeyboard + m_cxKeyCol * KEYBOARD_DATA[i].xCol;
        m_Keys[i].rcRect.top = m_yKeyboard + m_cyKeyRow * KEYBOARD_DATA[i].yRow;
        m_Keys[i].rcRect.right = m_Keys[i].rcRect.left + (m_cxKeyCol * KEYBOARD_DATA[i].cxWidth) - KEY_SPACING;
        m_Keys[i].rcRect.bottom = m_Keys[i].rcRect.top + cyKey;

        m_Keys[i].iCmdBinding = -1;
        m_Keys[i].dwFlags = KEYBOARD_DATA[i].dwFlags;

        //  Add this key ID to the index

        if (!m_Keys[i].sKeyID.IsBlank())
            m_KeyIDToIndex.Insert(m_Keys[i].sKeyID, i);
        }

    //  Initialize the list of layouts

    CreateLayoutControls();

    //  Initialize all commands (including bindings)

    UpdateLayoutControls();

    //  Create the background and title

	CUIHelper Helper(m_HI);
	IAnimatron *pTitle;
	DWORD dwOptions = CUIHelper::OPTION_SESSION_OK_BUTTON | CUIHelper::OPTION_SESSION_NO_CANCEL_BUTTON;
	Helper.CreateSessionTitle(this, m_Service, CONSTLIT("Keyboard"), NULL, dwOptions, &pTitle);
	StartPerformance(pTitle, ID_CTRL_TITLE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

    return NOERROR;
    }

void CKeyboardMapSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//  OnKeyDown
//
//  Handle keys

    {
	switch (iVirtKey)
		{
		case VK_RETURN:
		case VK_ESCAPE:
        	m_HI.ClosePopupSession();
			break;

		case VK_LEFT:
			CmdPrevLayout();
			break;

		case VK_RIGHT:
			CmdNextLayout();
			break;
		}
    }

void CKeyboardMapSession::OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture)

//  OnLButtonDown
//
//  Handle mouse click

    {
    //  If not editable, nothing to do

    if (!m_bEditable)
        return;

    //  If we've got a selection, then we might be clicking on a new key

    else if (m_iSelectedCommand != -1)
        {
        STargetCtx Result;
        if (HitTest(x, y, Result))
            {
            //  If we clicked on a key, then we remap

            if (Result.bInKey)
                {
                const SCommandDesc &Command = m_Commands[m_iSelectedCommand];

                //  If this is a new binding, set it

                if (Result.iKeyIndex != -1
                        && Result.iKeyIndex != Command.iKeyBinding
                        && !(m_Keys[Result.iKeyIndex].dwFlags & FLAG_RESERVED))
                    {
                    CGameKeys::Keys iCmd = Command.iCmd;
                    m_Settings.GetKeyMap().SetGameKey(m_Keys[Result.iKeyIndex].sKeyID, Command.iCmd);

                    //  Clear the old key

                    if (Command.iKeyBinding != -1)
                        m_Settings.GetKeyMap().SetGameKey(m_Keys[Command.iKeyBinding].sKeyID, CGameKeys::keyNone);

                    //  Reload mappings

                    LoadCommandMapping();

                    //  NOTE: The list of commands does not change within a session, so the
                    //  command index should be the same.
#if 0
                    //  Find the index of the command and keep it selected

                    m_iSelectedCommand = -1;
                    for (i = 0; i < m_Commands.GetCount(); i++)
                        if (m_Commands[i].iCmd == iCmd)
                            {
                            m_iSelectedCommand = i;
                            break;
                            }
#endif
                    }
                }

            //  Otherwise, if we click on a different command, then we need
            //  to select that command.

            else
                {
                m_iSelectedCommand = Result.iCmdIndex;
                }
            }
        else
            m_iSelectedCommand = -1;
        }

    //  Otherwise, select

    else
        {
        STargetCtx Result;
        if (HitTest(x, y, Result))
            m_iSelectedCommand = Result.iCmdIndex;
        else
            m_iSelectedCommand = -1;
        }
    }

void CKeyboardMapSession::OnLButtonUp (int x, int y, DWORD dwFlags)

//  OnLButtonUp
//
//  Handle mouse click

    {
    }

void CKeyboardMapSession::OnMouseMove (int x, int y, DWORD dwFlags)

//  OnMouseMove
//
//  Handle mouse move

    {
    STargetCtx Result;
    if (HitTest(x, y, Result))
        {
        //  If we've got a selection and this is a key, then we show the
        //  hover key.

        if (m_iSelectedCommand != -1 && Result.bInKey)
            {
            //  Not if this is a reserved key (which cannot be bound)

            if (!(m_Keys[Result.iKeyIndex].dwFlags & FLAG_RESERVED))
                m_iHoverKey = Result.iKeyIndex;
            else
                m_iHoverKey = -1;

            m_iHoverCommand = -1;
            }

        //  Otherwise, we select the whole command

        else
            {
            m_iHoverKey = -1;
            m_iHoverCommand = Result.iCmdIndex;
            }
        }
    else
        {
        m_iHoverKey = -1;
        m_iHoverCommand = -1;
        }
    }

void CKeyboardMapSession::OnPaint (CG32bitImage &Screen, const RECT &rcInvalid)

//  OnPaint
//
//  Paint session

    {
    int i, j;
	const CVisualPalette &VI = m_HI.GetVisuals();
    const CG16bitFont &LabelFont = VI.GetFont(fontMedium);

    CG32bitPixel rgbEditable = CG32bitPixel::Darken(VI.GetColor(colorTextHighlight), 220);
    CG32bitPixel rgbBoundKey = (m_bEditable ? rgbEditable : CG32bitPixel::Darken(VI.GetColor(colorTextNormal), 220));
    CG32bitPixel rgbUnboundKey = CG32bitPixel::Darken(VI.GetColor(colorTextNormal), 128);
    CG32bitPixel rgbText = VI.GetColor(colorAreaDialog);
    CG32bitPixel rgbHoverColor = VI.GetColor(colorTextQuote);
    CG32bitPixel rgbSelected = VI.GetColor(colorAreaDialogHighlight);

	RECT rcCenter;
	VI.DrawSessionBackground(Screen, CG32bitImage(), CVisualPalette::OPTION_SESSION_DLG_BACKGROUND, &rcCenter);

    //  Paint all keys

    for (i = 0; i < m_Keys.GetCount(); i++)
        {
        const SKeyDesc &Key = m_Keys[i];

        //  Skip if this key is the hover key (we paint it last)

        if ((m_iHoverCommand != -1 && m_iHoverCommand == Key.iCmdBinding)
                || (m_iSelectedCommand != -1 && m_iSelectedCommand == Key.iCmdBinding)
                || (m_iHoverKey != -1 && m_iHoverKey == i))
            continue;

        //  Draw the key

        PaintKey(Screen, Key, (Key.iCmdBinding != -1 ? rgbBoundKey : rgbUnboundKey), rgbText);
        }

    //  Paint all the commands

    for (i = 0; i < m_Commands.GetCount(); i++)
        {
        const SCommandDesc &Command = m_Commands[i];

        //  Set up colors

        CG32bitPixel rgbBack;
        if (i == m_iSelectedCommand)
            rgbBack = rgbSelected;
        else if (i == m_iHoverCommand)
            rgbBack = rgbHoverColor;
        else if (m_bEditable && Command.iKeyBinding == -1)
            rgbBack = VI.GetColor(colorTextDockWarning);
        else
            rgbBack = rgbBoundKey;

        CG32bitPixel rgbLine = (m_bEditable ? VI.GetColor(colorTextHighlight) : VI.GetColor(colorTextNormal));
        CG32bitPixel rgbLabel = VI.GetColor(colorAreaDialog);

        //  Draw command box and label

        CGDraw::RoundedRect(Screen, Command.rcRect.left, Command.rcRect.top, RectWidth(Command.rcRect), RectHeight(Command.rcRect), KEY_CORNER_RADIUS, rgbBack);
        LabelFont.DrawText(Screen, Command.rcRect, rgbLabel, Command.sLabel, 0, CG16bitFont::AlignCenter | CG16bitFont::AlignMiddle);

        //  Paint lines connecting the command

        if (m_iHoverCommand != i)
            {
            for (j = 0; j < Command.Line.GetCount() - 1; j++)
                Screen.DrawLine(Command.Line[j].x, Command.Line[j].y, Command.Line[j + 1].x, Command.Line[j + 1].y, LINE_WIDTH, rgbLine);
            }
        }

    //  Paint the hover line last

    if (m_iSelectedCommand != -1)
        {
        const SCommandDesc &Command = m_Commands[m_iSelectedCommand];

        if (Command.iKeyBinding != -1)
            PaintKey(Screen, m_Keys[Command.iKeyBinding], rgbSelected, rgbText);

        for (j = 0; j < Command.Line.GetCount() - 1; j++)
            Screen.DrawLine(Command.Line[j].x, Command.Line[j].y, Command.Line[j + 1].x, Command.Line[j + 1].y, LINE_WIDTH, rgbSelected);
        }

    if (m_iHoverKey != -1)
        {
        const SKeyDesc &Key = m_Keys[m_iHoverKey];
        PaintKey(Screen, m_Keys[m_iHoverKey], rgbSelected, rgbText);
        }

    if (m_iHoverCommand != -1 && m_iSelectedCommand != m_iHoverCommand)
        {
        const SCommandDesc &Command = m_Commands[m_iHoverCommand];

        if (Command.iKeyBinding != -1)
            PaintKey(Screen, m_Keys[Command.iKeyBinding], rgbHoverColor, rgbText);

        for (j = 0; j < Command.Line.GetCount() - 1; j++)
            Screen.DrawLine(Command.Line[j].x, Command.Line[j].y, Command.Line[j + 1].x, Command.Line[j + 1].y, LINE_WIDTH, rgbHoverColor);
        }
    }

void CKeyboardMapSession::OnReportHardCrash (CString *retsMessage)

//  OnReportHardCrash
//
//  Report a crash

    {
	*retsMessage = CONSTLIT("session: CKeyboardMapSession\r\n");
    }

void CKeyboardMapSession::PaintKey (CG32bitImage &Screen, const SKeyDesc &Key, CG32bitPixel rgbBack, CG32bitPixel rgbText)

//  PaintKey
//
//  Paint a key

    {
	const CVisualPalette &VI = m_HI.GetVisuals();
    const CG16bitFont &KeyFont = VI.GetFont(fontMediumHeavyBold);
    const CG16bitFont &SmallKeyFont = VI.GetFont(fontSmall);

    //  Draw the key

    CGDraw::RoundedRect(Screen, Key.rcRect.left, Key.rcRect.top, RectWidth(Key.rcRect), RectHeight(Key.rcRect), KEY_CORNER_RADIUS, rgbBack);

    //  Draw the key label

    if (Key.iSymbol != symbolNone)
        {
        const int INSET = 8;

        int yTop = Key.rcRect.top + INSET;
        int xLeft = Key.rcRect.left + INSET;
        int xRight = Key.rcRect.right - INSET;
        int yBottom = Key.rcRect.bottom - INSET;
        int cxWidth = xRight - xLeft;
        int cyHeight = yBottom - yTop;
        int cxHalfWidth = cxWidth / 2;
        int cyHalfHeight = cyHeight / 2;
        int cxQuarterWidth = cxHalfWidth / 2;
        int cyQuarterHeight = cyHalfHeight / 2;

        switch (Key.iSymbol)
            {
            case symbolArrowUp:
                CGDraw::TriangleCorner(Screen, xLeft + cxHalfWidth, yTop, 2, cyHalfHeight, rgbText);
                Screen.Fill(xLeft + cxQuarterWidth + 1, yTop + cyHalfHeight, cxHalfWidth - 1, cyHalfHeight, rgbText);
                break;

            case symbolArrowDown:
                CGDraw::TriangleCorner(Screen, xLeft + cxHalfWidth, yBottom - 1, 6, cyHalfHeight, rgbText);
                Screen.Fill(xLeft + cxQuarterWidth + 1, yTop, cxHalfWidth - 1, cyHalfHeight, rgbText);
                break;

            case symbolArrowLeft:
                CGDraw::TriangleCorner(Screen, xLeft, yTop + cyHalfHeight, 4, cxHalfWidth, rgbText);
                Screen.Fill(xLeft + cxHalfWidth, yTop + cyQuarterHeight + 1, cxHalfWidth, cyHalfHeight - 1, rgbText);
                break;

            case symbolArrowRight:
                CGDraw::TriangleCorner(Screen, xRight - 1, yTop + cyHalfHeight, 0, cxHalfWidth, rgbText);
                Screen.Fill(xLeft, yTop + cyQuarterHeight + 1, cxHalfWidth, cyHalfHeight - 1, rgbText);
                break;
            }
        }
    else if (Key.sLabel.GetLength() == 1)
        KeyFont.DrawText(Screen, Key.rcRect, rgbText, Key.sLabel, 0, CG16bitFont::AlignCenter | CG16bitFont::AlignMiddle);
    else
        SmallKeyFont.DrawText(Screen, Key.rcRect, rgbText, Key.sLabel, 0, CG16bitFont::AlignCenter | CG16bitFont::AlignMiddle);
    }

void CKeyboardMapSession::UpdateLayoutControls (void)

//  UpdateLayoutControls
//
//  Update layout controls when the layout has changed.

    {
    CGameKeys::ELayouts iCurLayout = m_Settings.GetKeyMap().GetLayout();

    //  Set the new label

	IAnimatron *pLabel = GetElement(ID_LAYOUT_LABEL);
    if (pLabel)
        pLabel->SetPropertyString(PROP_TEXT, m_Settings.GetKeyMap().GetLayoutName(iCurLayout));

    //  Enable/disable next/prev button

    IAnimatron *pNext = GetElement(CMD_NEXT_LAYOUT);
    if (pNext)
        pNext->SetPropertyBool(PROP_ENABLED, iCurLayout < CGameKeys::layoutCount - 1);

	IAnimatron *pPrev = GetElement(CMD_PREV_LAYOUT);
	if (pPrev)
		pPrev->SetPropertyBool(PROP_ENABLED, iCurLayout > CGameKeys::layoutDefault);

    //  Reload commands

    LoadCommandMapping();

    //  If this is the custom layout, then we can edit it

    m_bEditable = (iCurLayout == CGameKeys::layoutCustom);
    m_iSelectedCommand = -1;
    }
