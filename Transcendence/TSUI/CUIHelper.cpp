//	CUIHelper.cpp
//
//	CUIHelper class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

const int TITLE_BAR_HEIGHT =					128;

const int BUTTON_HEIGHT =						96;
const int BUTTON_WIDTH =						96;

const int ICON_CORNER_RADIUS =					8;
const int ICON_HEIGHT =							96;
const int ICON_WIDTH =							96;

const int INPUT_ERROR_CORNER_RADIUS =			8;
const int INPUT_ERROR_PADDING_BOTTOM =			10;
const int INPUT_ERROR_PADDING_LEFT =			10;
const int INPUT_ERROR_PADDING_RIGHT =			10;
const int INPUT_ERROR_PADDING_TOP =				10;
const int INPUT_ERROR_TIME =					(30 * 10);

const int PADDING_LEFT =						20;

#define ALIGN_RIGHT								CONSTLIT("right")

#define CMD_CLOSE_SESSION						CONSTLIT("cmdCloseSession")
#define CMD_OK_SESSION							CONSTLIT("cmdOKSession")

#define EVENT_ON_CLICK							CONSTLIT("onClick")

#define ID_DLG_INPUT_ERROR						CONSTLIT("uiHelperInputErrorMsg")

#define PROP_COLOR								CONSTLIT("color")
#define PROP_FILL_TYPE							CONSTLIT("fillType")
#define PROP_FONT								CONSTLIT("font")
#define PROP_LINE_COLOR							CONSTLIT("lineColor")
#define PROP_LINE_PADDING						CONSTLIT("linePadding")
#define PROP_LINE_TYPE							CONSTLIT("lineType")
#define PROP_LINE_WIDTH							CONSTLIT("lineWidth")
#define PROP_LL_RADIUS							CONSTLIT("llRadius")
#define PROP_LR_RADIUS							CONSTLIT("lrRadius")
#define PROP_OPACITY							CONSTLIT("opacity")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_TEXT								CONSTLIT("text")
#define PROP_TEXT_ALIGN_HORZ					CONSTLIT("textAlignHorz")
#define PROP_TEXT_ALIGN_VERT					CONSTLIT("textAlignVert")
#define PROP_UL_RADIUS							CONSTLIT("ulRadius")
#define PROP_UR_RADIUS							CONSTLIT("urRadius")

#define STYLE_CHECK								CONSTLIT("check")
#define STYLE_DISABLED							CONSTLIT("disabled")
#define STYLE_DOWN								CONSTLIT("down")
#define STYLE_FRAME								CONSTLIT("frame")
#define STYLE_FRAME_FOCUS						CONSTLIT("frameFocus")
#define STYLE_FRAME_DISABLED					CONSTLIT("frameDisabled")
#define STYLE_HOVER								CONSTLIT("hover")
#define STYLE_IMAGE								CONSTLIT("image")
#define STYLE_NORMAL							CONSTLIT("normal")
#define STYLE_TEXT								CONSTLIT("text")

void CUIHelper::CreateInputErrorMessage (IHISession *pSession, const RECT &rcRect, const CString &sTitle, CString &sDesc, IAnimatron **retpMsg) const

//	CreateInputErrorMessage
//
//	Creates an input error message box

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &TitleFont = VI.GetFont(fontLargeBold);
	const CG16bitFont &DescFont = VI.GetFont(fontMedium);

	//	Start with a sequencer as a parent of everything

	CAniSequencer *pMsg;
	CAniSequencer::Create(CVector(rcRect.left, rcRect.top), &pMsg);

	//	Create a controller to handle dismissing the message

	CInputErrorMessageController *pController = new CInputErrorMessageController(pSession);
	pMsg->AddTrack(pController, 0);

	//	Add a button to handle a click

	CAniButton *pButton = new CAniButton;
	pButton->SetPropertyVector(PROP_POSITION, CVector(0, 0));
	pButton->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), RectHeight(rcRect)));
	pButton->SetStyle(STYLE_DOWN, NULL);
	pButton->SetStyle(STYLE_HOVER, NULL);
	pButton->SetStyle(STYLE_NORMAL, NULL);
	pButton->SetStyle(STYLE_DISABLED, NULL);
	pButton->SetStyle(STYLE_TEXT, NULL);
	pButton->AddListener(EVENT_ON_CLICK, pController);

	pMsg->AddTrack(pButton, 0);

	//	Figure out where the text goes

	int x = INPUT_ERROR_PADDING_LEFT;
	int cxWidth = RectWidth(rcRect) - (INPUT_ERROR_PADDING_LEFT + INPUT_ERROR_PADDING_RIGHT);
	int y = INPUT_ERROR_PADDING_TOP;
	int yEnd = RectHeight(rcRect) - INPUT_ERROR_PADDING_BOTTOM;
	int cxText = 0;

	//	Title text

	IAnimatron *pTitle = new CAniText;
	pTitle->SetPropertyVector(PROP_POSITION, CVector(x, y));
	pTitle->SetPropertyVector(PROP_SCALE, CVector(cxWidth, TitleFont.GetHeight()));
	((CAniText *)pTitle)->SetPropertyFont(PROP_FONT, &TitleFont);
	pTitle->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextWarningMsg));
	pTitle->SetPropertyString(PROP_TEXT, sTitle);

	y += TitleFont.GetHeight();
	cxText += TitleFont.GetHeight();

	//	Description

	IAnimatron *pDesc = new CAniText;
	pDesc->SetPropertyVector(PROP_POSITION, CVector(x, y));
	pDesc->SetPropertyVector(PROP_SCALE, CVector(cxWidth, 1000));
	((CAniText *)pDesc)->SetPropertyFont(PROP_FONT, &DescFont);
	pDesc->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextWarningMsg));
	pDesc->SetPropertyString(PROP_TEXT, sDesc);

	RECT rcDesc;
	pDesc->GetSpacingRect(&rcDesc);
	cxText += RectHeight(rcDesc);

	//	Now that we know the height of the text, add a rectangle as a background

	int cxFrame = Max(RectHeight(rcRect), cxText + INPUT_ERROR_PADDING_TOP + INPUT_ERROR_PADDING_BOTTOM);

	IAnimatron *pRect = new CAniRoundedRect;
	pRect->SetPropertyVector(PROP_POSITION, CVector());
	pRect->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), cxFrame));
	pRect->SetPropertyColor(PROP_COLOR, VI.GetColor(colorAreaWarningMsg));
	pRect->SetPropertyOpacity(PROP_OPACITY, 255);
	pRect->SetPropertyInteger(PROP_UL_RADIUS, INPUT_ERROR_CORNER_RADIUS);
	pRect->SetPropertyInteger(PROP_UR_RADIUS, INPUT_ERROR_CORNER_RADIUS);
	pRect->SetPropertyInteger(PROP_LL_RADIUS, INPUT_ERROR_CORNER_RADIUS);
	pRect->SetPropertyInteger(PROP_LR_RADIUS, INPUT_ERROR_CORNER_RADIUS);

	pMsg->AddTrack(pRect, 0);

	//	Add title and desc

	pMsg->AddTrack(pTitle, 0);
	pMsg->AddTrack(pDesc, 0);

	//	Fade after some time

	pMsg->AnimateLinearFade(INPUT_ERROR_TIME, 5, 30);

	//	If we already have an input error box, delete it

	pSession->StopPerformance(ID_DLG_INPUT_ERROR);

	//	Start a new one

	pSession->StartPerformance(pMsg, ID_DLG_INPUT_ERROR, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Done

	if (retpMsg)
		*retpMsg = pMsg;
	}

void CUIHelper::CreateSessionTitle (IHISession *pSession, 
									CCloudService &Service, 
									const CString &sTitle, 
									const TArray<SMenuEntry> *pMenu,
									DWORD dwOptions, 
									IAnimatron **retpControl) const

//	CreateSessionTitle
//
//	Creates a session title bar, including:
//
//	User icon
//	User name
//	Session title
//	Close button

	{
	int i;
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	RECT rcRect;
	VI.GetWidescreenRect(m_HI.GetScreen(), &rcRect);

	//	Create a sequencer to hold all the controls

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(rcRect.left, rcRect.top - TITLE_BAR_HEIGHT), &pRoot);

	//	The user icon is centered

	CAniRoundedRect *pIcon = new CAniRoundedRect;
	pIcon->SetPropertyVector(PROP_POSITION, CVector(0, (TITLE_BAR_HEIGHT - ICON_HEIGHT) / 2));
	pIcon->SetPropertyVector(PROP_SCALE, CVector(ICON_HEIGHT, ICON_WIDTH));
	pIcon->SetPropertyColor(PROP_COLOR, CG16bitImage::RGBValue(128, 128, 128));
	pIcon->SetPropertyOpacity(PROP_OPACITY, 255);
	pIcon->SetPropertyInteger(PROP_UL_RADIUS, ICON_CORNER_RADIUS);
	pIcon->SetPropertyInteger(PROP_UR_RADIUS, ICON_CORNER_RADIUS);
	pIcon->SetPropertyInteger(PROP_LL_RADIUS, ICON_CORNER_RADIUS);
	pIcon->SetPropertyInteger(PROP_LR_RADIUS, ICON_CORNER_RADIUS);

	pRoot->AddTrack(pIcon, 0);

	//	The user name baseline is centered.

	CString sUsername;
	WORD wUsernameColor;
	if (Service.HasCapability(ICIService::canGetUserProfile))
		{
		sUsername = Service.GetUsername();
		wUsernameColor = VI.GetColor(colorTextDialogInput);
		}
	else
		{
		sUsername = CONSTLIT("Offline");
		wUsernameColor = VI.GetColor(colorTextDialogLabel);
		}

	int y = (TITLE_BAR_HEIGHT / 2) - SubTitleFont.GetAscent();

	IAnimatron *pName = new CAniText;
	pName->SetPropertyVector(PROP_POSITION, CVector(ICON_WIDTH + PADDING_LEFT, y));
	pName->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), RectHeight(rcRect)));
	pName->SetPropertyColor(PROP_COLOR, wUsernameColor);
	pName->SetPropertyFont(PROP_FONT, &SubTitleFont);
	pName->SetPropertyString(PROP_TEXT, sUsername);

	pRoot->AddTrack(pName, 0);
	y += SubTitleFont.GetHeight();

	//	Add the session title

	IAnimatron *pTitle = new CAniText;
	pTitle->SetPropertyVector(PROP_POSITION, CVector(ICON_WIDTH + PADDING_LEFT, y));
	pTitle->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), RectHeight(rcRect)));
	pTitle->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogTitle));
	pTitle->SetPropertyFont(PROP_FONT, &SubTitleFont);
	pTitle->SetPropertyString(PROP_TEXT, sTitle);

	pRoot->AddTrack(pTitle, 0);

	//	Add command buttons at the bottom

	int yBottomBar = TITLE_BAR_HEIGHT + RectHeight(rcRect);

	//	Add a close button.

	if (!(dwOptions & OPTION_SESSION_NO_CANCEL_BUTTON))
		{
		//	If we have an OK button, then the label is Cancel.

		CString sCloseLabel = ((dwOptions & OPTION_SESSION_OK_BUTTON) ? CONSTLIT("Cancel") : CONSTLIT("Close"));
		const CG16bitImage &CloseIcon = VI.GetImage(imageCloseIcon);

		IAnimatron *pCloseButton;
		VI.CreateImageButton(pRoot, CMD_CLOSE_SESSION, RectWidth(rcRect) - BUTTON_WIDTH, yBottomBar + (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2, &CloseIcon, sCloseLabel, 0, &pCloseButton);

		pSession->RegisterPerformanceEvent(pCloseButton, EVENT_ON_CLICK, CMD_CLOSE_SESSION);
		}

	//	Add an OK button, if necessary

	if (dwOptions & OPTION_SESSION_OK_BUTTON)
		{
		const CG16bitImage &OKIcon = VI.GetImage(imageOKIcon);

		IAnimatron *pOKButton;
		VI.CreateImageButton(pRoot, CMD_OK_SESSION, (RectWidth(rcRect) - BUTTON_WIDTH) / 2, yBottomBar + (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2, &OKIcon, CONSTLIT("OK"), 0, &pOKButton);

		pSession->RegisterPerformanceEvent(pOKButton, EVENT_ON_CLICK, CMD_OK_SESSION);
		}

	//	Add menu items, if necessary

	if (pMenu)
		{
		//	Menu is to the left.

		int xMenu = 0;
		int yMenu = yBottomBar + (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2;

		for (i = 0; i < pMenu->GetCount(); i++)
			{
			const SMenuEntry &Entry = pMenu->GetAt(i);

			IAnimatron *pButton;
			int cyHeight;
			VI.CreateLink(pRoot, Entry.sCommand, xMenu, yMenu, Entry.sLabel, 0, &pButton, NULL, &cyHeight);
			yMenu += cyHeight;

			pSession->RegisterPerformanceEvent(pButton, EVENT_ON_CLICK, Entry.sCommand);
			}
		}

	//	Done

	*retpControl = pRoot;
	}

void CInputErrorMessageController::OnAniCommand (const CString &sID, const CString &sEvent, const CString &sCmd, DWORD dwData)

//	OnAniCommand
//
//	Handles a command

	{
	m_pSession->StopPerformance(ID_DLG_INPUT_ERROR);
	}
