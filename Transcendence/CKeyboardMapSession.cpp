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

#define CMD_RESET_DEFAULT 						CONSTLIT("cmdResetDefault")
#define CMD_CLEAR_BINDING 						CONSTLIT("cmdClearBinding")

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

#ifdef DEBUG
//#define DEBUG_KEYMAP
#endif

const int KEY_SPACING = 4;
const int KEY_CORNER_RADIUS = 3;
const int LABEL_SPACING = 4;
const int LABEL_COLUMN_SPACING = 14;

const int LINE_WIDTH = 2;

const int KEYBOARD_HEIGHT = 220;
const int KEYBOARD_ROWS = 6;

const int MAJOR_PADDING_TOP =					20;
const int SMALL_BUTTON_HEIGHT =					48;
const int SMALL_BUTTON_WIDTH =					48;
const int SMALL_SPACING_HORZ =					8;
const int SMALL_SPACING_VERT =					8;

#include "KeyboardMapData.h"

const CKeyboardMapSession::SDeviceData CKeyboardMapSession::DEVICE_DATA[] =
	{
		{
			deviceKeyboard,			//	Device type
			"Keyboard",				//	Name
			KEYBOARD_DATA,			//	Data
			KEYBOARD_DATA_COUNT,

			37,						//	Cols
			6,						//	Rows
			50,						//	x offset
			60,						//	y offset
			},
		{
			deviceNumpad,			//	Device type
			"Number Pad",			//	Name
			NUMPAD_DATA,			//	Data
			NUMPAD_DATA_COUNT,

			8,						//	Cols
			5,						//	Rows
			0,						//	x offset
			30,						//	y offset
			},
		{
			deviceMouse,			//	Device type
			"Mouse",				//	Name
			MOUSE_DATA,				//	Data
			MOUSE_DATA_COUNT,

			7,						//	Cols
			5,						//	Rows
			0,						//	x offset
			45,						//	y offset
			},
	};

const int CKeyboardMapSession::DEVICE_DATA_COUNT = (sizeof(DEVICE_DATA) / sizeof(DEVICE_DATA[0]));

CKeyboardMapSession::CKeyboardMapSession (CHumanInterface &HI, CCloudService &Service, CGameSettings &Settings) : IHISession(HI),
        m_Service(Service),
        m_Settings(Settings),
        m_bEditable(false),
        m_iHoverKey(-1),
        m_iHoverCommand(-1),
        m_iSelectedCommand(-1),
		m_iTick(0),
		m_iFlashKey(-1),
		m_iFlashUntil(0)

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
		Labels[i].sLabel = m_Commands[i].sLabel;
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
		m_Commands[i].Line.DeleteAll();
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

void CKeyboardMapSession::CmdClearBinding (void)

//	CmdClearBinding
//
//	Clear the key binding of the selected command

	{
	if (m_iSelectedCommand != -1 && m_Commands[m_iSelectedCommand].iKeyBinding != -1)
		{
		const SCommandDesc &Command = m_Commands[m_iSelectedCommand];
	    m_Settings.GetKeyMap().SetGameKey(m_Keys[Command.iKeyBinding].sKeyID, CGameKeys::keyNone);
		InitBindings();
		UpdateMenu();
		}
	}

void CKeyboardMapSession::CmdNextLayout (void)

//  CmdNextLayout
//
//  Select the next device

    {
	if (m_iDevice < DEVICE_DATA_COUNT - 1)
        {
		m_iDevice++;
		InitDevice(DEVICE_DATA[m_iDevice]);
		InitBindings();
		m_iSelectedCommand = -1;
		UpdateDeviceSelector();
		UpdateMenu();
        }
    }

void CKeyboardMapSession::CmdPrevLayout (void)

//  CmdPrevLayout
//
//  Select the previous device

    {
	if (m_iDevice > 0)
        {
		m_iDevice--;
		InitDevice(DEVICE_DATA[m_iDevice]);
		InitBindings();
		m_iSelectedCommand = -1;
		UpdateDeviceSelector();
		UpdateMenu();
        }
    }

void CKeyboardMapSession::CmdResetDefault (void)

//	CmdResetDefault
//
//	Resets the bindings to default

	{
	m_Settings.GetKeyMap().SetLayout(CGameKeys::layoutDefault);
	InitBindings();
	m_iSelectedCommand = -1;
	UpdateMenu();
	}

void CKeyboardMapSession::CreateDeviceSelector (void)

//  CreateDeviceSelector
//
//  Create arrows to select a layout

    {
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

    //  Create a sequencer to hold all controls

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(m_rcRect.left, m_rcRect.top), &pRoot);
    int y = MAJOR_PADDING_TOP;

	//	Create buttons to select the layout

	int xCenter = (RectWidth(m_rcRect) / 2);
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
	pLabel->SetPropertyVector(PROP_SCALE, CVector(RectWidth(m_rcRect), 100.0));
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

void CKeyboardMapSession::InitBindings (void)

//	InitBindings
//
//	Initializes bindings between commands and keys. We assume that both InitCommands
//	and InitDevice have been called.

	{
    int i, j;
	const CVisualPalette &VI = m_HI.GetVisuals();

    //  Reset keyboard to remove bindings

    for (i = 0; i < m_Keys.GetCount(); i++)
        m_Keys[i].iCmdBinding = -1;

    //  Load commands. NOTE: These do not change from session to session, so 
	//	they should match when we initialized.

    TArray<CGameKeys::SCommandKeyDesc> Commands;
    m_Settings.GetKeyMap().GetCommands(Commands);
	ASSERT(Commands.GetCount() == m_Commands.GetCount());

    for (i = 0; i < Commands.GetCount(); i++)
        {
		m_Commands[i].sKeyBinding = NULL_STR;
		m_Commands[i].iKeyBinding = -1;

		//	See if we have a binding to one of the keys on this device.

		for (j = 0; j < Commands[i].Keys.GetCount(); j++)
			{
			const CGameKeys::SBindingDesc &Binding = Commands[i].Keys[j];
            int *pKey = m_KeyIDToIndex.GetAt(Binding.sKeyID);
			if (pKey)
				{
				m_Commands[i].sKeyBinding = Binding.sKeyID;
				m_Commands[i].iKeyBinding = *pKey;
				m_Keys[*pKey].iCmdBinding = i;
				break;
				}
			}

		//	If we have a binding, but not to a key in this device, init the
		//	binding ID so that we know that this command is not completely
		//	unbound.

		if (m_Commands[i].sKeyBinding.IsBlank() && Commands[i].Keys.GetCount() > 0)
			m_Commands[i].sKeyBinding = Commands[i].Keys[0].sKeyID;
        }

    //  Figure out where each command label should go

    RECT rcKeyboard;
    rcKeyboard.left = m_xKeyboard;
    rcKeyboard.top = m_yKeyboard;
    rcKeyboard.right = m_xKeyboard + m_cxKeyboard;
    rcKeyboard.bottom = m_yKeyboard + m_cyKeyboard;

    int cxBoundsSpacing = Max(0, (int)Min(m_rcRect.right - rcKeyboard.right, rcKeyboard.left - m_rcRect.left) - (LABEL_COLUMN_SPACING * m_cxKeyCol));
    RECT rcBounds = m_rcRect;
    rcBounds.top += MAJOR_PADDING_TOP + SMALL_BUTTON_HEIGHT + SMALL_SPACING_VERT + VI.GetFont(fontSubTitle).GetHeight();
    rcBounds.left = m_rcRect.left + cxBoundsSpacing;
    rcBounds.right = m_rcRect.right - cxBoundsSpacing;
    ArrangeCommandLabels(rcBounds, rcKeyboard);
	}

void CKeyboardMapSession::InitCommands (void)

//	InitCommands
//
//	Initializes the list of commands.

	{
	int i;
	const CVisualPalette &VI = m_HI.GetVisuals();
    const CG16bitFont &LabelFont = VI.GetFont(fontMedium);

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
        }
	}

void CKeyboardMapSession::InitDevice (const SDeviceData &Device)

//	InitDevice
//
//	Initializes the given device. NOTE: We assume that m_cxKeyCol and m_cyKeyRow
//	are initialized.

	{
	int i;

	//	Center the keyboard on the screen

    m_cxKeyboard = (Device.iCols * m_cxKeyCol) - KEY_SPACING;
    m_cyKeyboard = (Device.iRows * m_cyKeyRow) - KEY_SPACING;

    m_xKeyboard = m_rcRect.left + (RectWidth(m_rcRect) - m_cxKeyboard) / 2 + Device.xOffset;
    m_yKeyboard = m_rcRect.top + (RectHeight(m_rcRect) - m_cyKeyboard) / 2 + Device.yOffset;

    //  Initialize all the keys metrics

	m_Keys.DeleteAll();
	m_KeyIDToIndex.DeleteAll();

    m_Keys.InsertEmpty(Device.iKeyCount);
    m_KeyIDToIndex.GrowToFit(Device.iKeyCount);
    for (i = 0; i < m_Keys.GetCount(); i++)
        {
        if (Device.pKeys[i].pszKeyID)
			{
            m_Keys[i].sKeyID = CString(Device.pKeys[i].pszKeyID);
			m_Keys[i].dwVirtKey = CVirtualKeyData::GetKey(m_Keys[i].sKeyID);
			}
		else
			m_Keys[i].dwVirtKey = CVirtualKeyData::INVALID_VIRT_KEY;

        if (Device.pKeys[i].pszLabel)
            m_Keys[i].sLabel = CString(Device.pKeys[i].pszLabel);

        m_Keys[i].iSymbol = Device.pKeys[i].iSymbol;

        m_Keys[i].rcRect.left = m_xKeyboard + m_cxKeyCol * Device.pKeys[i].xCol;
        m_Keys[i].rcRect.top = m_yKeyboard + m_cyKeyRow * Device.pKeys[i].yRow;
        m_Keys[i].rcRect.right = m_Keys[i].rcRect.left + (m_cxKeyCol * Device.pKeys[i].cxWidth) - KEY_SPACING;
        m_Keys[i].rcRect.bottom = m_Keys[i].rcRect.top + (m_cyKeyRow * Device.pKeys[i].cyHeight) - KEY_SPACING;

        m_Keys[i].iCmdBinding = -1;
        m_Keys[i].dwFlags = Device.pKeys[i].dwFlags;

        //  Add this key ID to the index

        if (!m_Keys[i].sKeyID.IsBlank())
            m_KeyIDToIndex.Insert(m_Keys[i].sKeyID, i);
        }
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
	else if (strEquals(sCmd, CMD_RESET_DEFAULT))
		CmdResetDefault();
	else if (strEquals(sCmd, CMD_CLEAR_BINDING))
		CmdClearBinding();

    return NOERROR;
    }

ALERROR CKeyboardMapSession::OnInit (CString *retsError)

//  OnInit
//
//  Initialize the session.

    {
	const CVisualPalette &VI = m_HI.GetVisuals();
	VI.GetWidescreenRect(&m_rcRect);

	//	Load the commands. The set of commands don't change during the session.

	InitCommands();

    //  Keyboard metrics

    m_cyKeyRow = AlignDown((KEYBOARD_HEIGHT + KEY_SPACING) / KEYBOARD_ROWS, 2);
    m_cxKeyCol = m_cyKeyRow / 2;

	//	Initialize device

	m_iDevice = 0;
	InitDevice(DEVICE_DATA[m_iDevice]);

	//	Initialize bindings between keys and commands

	InitBindings();
	m_iSelectedCommand = -1;

	//	Buttons to switch to different devices

	CreateDeviceSelector();
	UpdateDeviceSelector();

	//	State

	m_bEditable = true;

	//	Set up a menu

	TArray<CUIHelper::SMenuEntry> Menu;
	CUIHelper::SMenuEntry *pEntry = Menu.Insert();
	pEntry->sCommand = CMD_RESET_DEFAULT;
	pEntry->sLabel = CONSTLIT("Reset to default");

	pEntry = Menu.Insert();
	pEntry->sCommand = CMD_CLEAR_BINDING;
	pEntry->sLabel = CONSTLIT("Clear key binding");

    //  Create the background and title

	CUIHelper Helper(m_HI);
	IAnimatron *pTitle;
	DWORD dwOptions = CUIHelper::OPTION_SESSION_OK_BUTTON | CUIHelper::OPTION_SESSION_NO_CANCEL_BUTTON;
	Helper.CreateSessionTitle(this, m_Service, CONSTLIT("Settings & Options"), &Menu, dwOptions, &pTitle);
	StartPerformance(pTitle, ID_CTRL_TITLE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	UpdateMenu();

    return NOERROR;
    }

void CKeyboardMapSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//  OnKeyDown
//
//  Handle keys

    {
	//	Flash the key

#ifdef DEBUG_KEYMAP
	int i;

	DWORD dwTVirtKey = CGameKeys::TranslateVirtKey(iVirtKey, dwKeyData);
	for (i = 0; i < m_Keys.GetCount(); i++)
		if (m_Keys[i].dwVirtKey == dwTVirtKey)
			{
			m_iFlashKey = i;
			m_iFlashUntil = m_iTick + 150;
			}
#endif

	//	Command

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

                    InitBindings();

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

	UpdateMenu();
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
	VI.DrawSessionBackground(Screen, CG32bitImage(), CG32bitPixel(), CVisualPalette::OPTION_SESSION_DLG_BACKGROUND, &rcCenter);

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

        PaintKey(Screen, Key, (Key.iCmdBinding != -1 ? rgbBoundKey : rgbUnboundKey), rgbText, i == m_iFlashKey);
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
        else if (m_bEditable && Command.sKeyBinding.IsBlank())
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
            PaintKey(Screen, m_Keys[Command.iKeyBinding], rgbSelected, rgbText, Command.iKeyBinding == m_iFlashKey);

        for (j = 0; j < Command.Line.GetCount() - 1; j++)
            Screen.DrawLine(Command.Line[j].x, Command.Line[j].y, Command.Line[j + 1].x, Command.Line[j + 1].y, LINE_WIDTH, rgbSelected);
        }

    if (m_iHoverKey != -1)
        {
        const SKeyDesc &Key = m_Keys[m_iHoverKey];
        PaintKey(Screen, m_Keys[m_iHoverKey], rgbSelected, rgbText, m_iHoverKey == m_iFlashKey);
        }

    if (m_iHoverCommand != -1 && m_iSelectedCommand != m_iHoverCommand)
        {
        const SCommandDesc &Command = m_Commands[m_iHoverCommand];

        if (Command.iKeyBinding != -1)
            PaintKey(Screen, m_Keys[Command.iKeyBinding], rgbHoverColor, rgbText, Command.iKeyBinding == m_iFlashKey);

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

void CKeyboardMapSession::OnUpdate (bool bTopMost)

//	OnUpdate
//
//	Update once per frame

	{
	m_iTick++;
	}

void CKeyboardMapSession::PaintKey (CG32bitImage &Screen, const SKeyDesc &Key, CG32bitPixel rgbBack, CG32bitPixel rgbText, bool bFlash)

//  PaintKey
//
//  Paint a key

    {
	const CVisualPalette &VI = m_HI.GetVisuals();
    const CG16bitFont &KeyFont = VI.GetFont(fontMediumHeavyBold);
    const CG16bitFont &SmallKeyFont = VI.GetFont(fontSmall);

	//	Flash?

	if (bFlash && m_iTick < m_iFlashUntil && (m_iTick % 2) == 0)
		rgbBack = CG32bitPixel(0, 0, 0);

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

void CKeyboardMapSession::UpdateDeviceSelector (void)

//	UpdateDeviceSelector
//
//	Updates the device selector

	{
    //  Set the new label

	IAnimatron *pLabel = GetElement(ID_LAYOUT_LABEL);
    if (pLabel)
        pLabel->SetPropertyString(PROP_TEXT, CString(DEVICE_DATA[m_iDevice].pszLabel));

    //  Enable/disable next/prev button

    IAnimatron *pNext = GetElement(CMD_NEXT_LAYOUT);
    if (pNext)
        pNext->SetPropertyBool(PROP_ENABLED, m_iDevice < DEVICE_DATA_COUNT - 1);

	IAnimatron *pPrev = GetElement(CMD_PREV_LAYOUT);
	if (pPrev)
		pPrev->SetPropertyBool(PROP_ENABLED, m_iDevice > 0);
	}

void CKeyboardMapSession::UpdateMenu (void)

//	UpdateMenu
//
//	Enables/disables menu items.

	{
	IAnimatron *pItem = GetElement(CMD_RESET_DEFAULT);
	if (pItem)
		pItem->SetPropertyBool(PROP_ENABLED, m_Settings.GetKeyMap().GetLayout() != CGameKeys::layoutDefault);

	pItem = GetElement(CMD_CLEAR_BINDING);
	if (pItem)
		pItem->SetPropertyBool(PROP_ENABLED, (m_iSelectedCommand != -1 && m_Commands[m_iSelectedCommand].iKeyBinding != -1));
	}
