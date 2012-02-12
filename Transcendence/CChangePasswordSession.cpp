//	CChangePasswordSession.cpp
//
//	CChangePasswordSession class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

const int DLG_WIDTH =							350;
const int DLG_SPACING_X =						10;
const int DLG_SPACING_Y =						10;

const int DEFAULT_BUTTON_HEIGHT =				64;
const int DEFAULT_BUTTON_WIDTH =				96;

#define CMD_CANCEL								CONSTLIT("cmdCancel")
#define CMD_OK									CONSTLIT("cmdOK")

#define EVENT_ON_CLICK							CONSTLIT("onClick")

#define ID_CTRL_CANCEL_ACTION					CONSTLIT("ctrlCancel")
#define ID_CTRL_MAIN_ACTION						CONSTLIT("ctrlMainAction")
#define ID_CTRL_PASSWORD						CONSTLIT("ctrlPassword")
#define ID_CTRL_PASSWORD_NEW					CONSTLIT("ctrlPasswordNew")
#define ID_CTRL_PASSWORD_CONFIRM				CONSTLIT("ctrlPasswordConfirm")
#define ID_DLG_MESSAGE							CONSTLIT("dlgMessage")

#define PROP_COLOR								CONSTLIT("color")
#define PROP_FONT								CONSTLIT("font")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_TEXT								CONSTLIT("text")

CChangePasswordSession::CChangePasswordSession (CHumanInterface &HI, CCloudService &Service) : IHISession(HI), 
		m_Service(Service)

//	CChangePasswordSession constructor

	{
	}

void CChangePasswordSession::CmdCancel (void)

//	CmdDone
//
//	Close the message session.

	{
	m_HI.ClosePopupSession();
	}

void CChangePasswordSession::CreateDlg (IAnimatron **retpDlg)

//	CreateDlg
//
//	Creates the dialog box

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);

	//	Figure out where the login dialog box will appear

	RECT rcCenter;
	VI.GetWidescreenRect(m_HI.GetScreen(), &rcCenter);

	RECT rcDlg = rcCenter;
	int cyDlg = 15 * VI.GetFont(fontLarge).GetHeight();
	rcDlg.top = rcCenter.top + (RectHeight(rcCenter) - cyDlg) / 2;
	rcDlg.left = rcCenter.left + (RectWidth(rcCenter) - DLG_WIDTH) / 2;
	rcDlg.right = rcDlg.left + DLG_WIDTH;
	rcDlg.bottom = rcDlg.top + cyDlg;

	//	Create the dialog box and a container for the controls

	IAnimatron *pDlg;
	CAniSequencer *pContainer;
	VI.CreateStdDialog(rcDlg, CONSTLIT("Change Password"), &pDlg, &pContainer);

	int y = 0;

	//	Add a password field

	int cyHeight;
	VI.CreateEditControl(pContainer, ID_CTRL_PASSWORD, 0, y, RectWidth(rcDlg), CVisualPalette::OPTION_EDIT_PASSWORD, CONSTLIT("Old password:"), NULL, &cyHeight);
	y += cyHeight + DLG_SPACING_Y;

	//	Add a new password field

	VI.CreateEditControl(pContainer, ID_CTRL_PASSWORD_NEW, 0, y, RectWidth(rcDlg), CVisualPalette::OPTION_EDIT_PASSWORD, CONSTLIT("New password:"), NULL, &cyHeight);
	y += cyHeight + DLG_SPACING_Y;

	//	Add a confirm password field

	VI.CreateEditControl(pContainer, ID_CTRL_PASSWORD_CONFIRM, 0, y, RectWidth(rcDlg), CVisualPalette::OPTION_EDIT_PASSWORD, CONSTLIT("Confirm password:"), NULL, &cyHeight);
	y += cyHeight + DLG_SPACING_Y;

	//	Add sign in button at the bottom

	IAnimatron *pButton;
	int xButtons = (RectWidth(rcDlg) - (2 * DEFAULT_BUTTON_WIDTH + DLG_SPACING_X)) / 2;
	int yButtons = RectHeight(rcDlg) - DEFAULT_BUTTON_HEIGHT;
	VI.CreateButton(pContainer, ID_CTRL_MAIN_ACTION, xButtons, yButtons, DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, CVisualPalette::OPTION_BUTTON_DEFAULT, CONSTLIT("Change"), &pButton);
	RegisterPerformanceEvent(pButton, EVENT_ON_CLICK, CMD_OK);

	//	Add the cancel button

	int x = xButtons + DLG_SPACING_X + DEFAULT_BUTTON_WIDTH;
	VI.CreateButton(pContainer, ID_CTRL_CANCEL_ACTION, x, yButtons, DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, 0, CONSTLIT("Cancel"), &pButton);
	RegisterPerformanceEvent(pButton, EVENT_ON_CLICK, CMD_CANCEL);

	//	Done

	*retpDlg = pDlg;
	}

ALERROR CChangePasswordSession::OnCommand (const CString &sCmd, void *pData)

//	OnCommand
//
//	Handle a command

	{
	if (strEquals(sCmd, CMD_CANCEL))
		CmdCancel();

	return NOERROR;
	}

ALERROR CChangePasswordSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	//	We're showing dialog boxes, so the previous session shows through.

	SetTransparent();

	//	Create the dialog box

	IAnimatron *pDlg;
	CreateDlg(&pDlg);
	StartPerformance(pDlg, ID_DLG_MESSAGE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Done

	return NOERROR;
	}

void CChangePasswordSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//	OnKeyDown
//
//	KeyDown

	{
	switch (iVirtKey)
		{
		case VK_ESCAPE:
			CmdCancel();
			break;

		case VK_RETURN:
			break;
		}
	}

void CChangePasswordSession::OnPaint (CG16bitImage &Screen, const RECT &rcInvalid)

//	OnPaint
//
//	Paint

	{
	Screen.FillTrans(0, 0, Screen.GetWidth(), Screen.GetHeight(), 0, 128);
	}

void CChangePasswordSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash
//
//	Describe current state

	{
	*retsMessage = CONSTLIT("session: CChangePasswordSession\r\n");
	}

