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

const int PADDING_LEFT =						20;

#define ALIGN_RIGHT								CONSTLIT("right")

#define CMD_CLOSE_SESSION						CONSTLIT("cmdCloseSession")
#define CMD_OK_SESSION							CONSTLIT("cmdOKSession")

#define EVENT_ON_CLICK							CONSTLIT("onClick")

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

void CUIHelper::CreateSessionTitle (IHISession *pSession, CCloudService &Service, const CString &sTitle, DWORD dwOptions, IAnimatron **retpControl) const

//	CreateSessionTitle
//
//	Creates a session title bar, including:
//
//	User icon
//	User name
//	Session title
//	Close button

	{
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
	if (Service.HasCapability(ICIService::getUserProfile))
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
	//
	//	If we have an OK button, then the label is Cancel.

	CString sCloseLabel = ((dwOptions & OPTION_SESSION_OK_BUTTON) ? CONSTLIT("Cancel") : CONSTLIT("Close"));
	const CG16bitImage &CloseIcon = VI.GetImage(imageCloseIcon);

	IAnimatron *pCloseButton;
	VI.CreateImageButton(pRoot, CMD_CLOSE_SESSION, RectWidth(rcRect) - BUTTON_WIDTH, yBottomBar + (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2, &CloseIcon, sCloseLabel, 0, &pCloseButton);

	pSession->RegisterPerformanceEvent(pCloseButton, EVENT_ON_CLICK, CMD_CLOSE_SESSION);

	//	Add an OK button, if necessary

	if (dwOptions & OPTION_SESSION_OK_BUTTON)
		{
		const CG16bitImage &OKIcon = VI.GetImage(imageOKIcon);

		IAnimatron *pOKButton;
		VI.CreateImageButton(pRoot, CMD_OK_SESSION, (RectWidth(rcRect) - BUTTON_WIDTH) / 2, yBottomBar + (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2, &OKIcon, CONSTLIT("OK"), 0, &pOKButton);

		pSession->RegisterPerformanceEvent(pOKButton, EVENT_ON_CLICK, CMD_OK_SESSION);
		}

	//	Done

	*retpControl = pRoot;
	}
