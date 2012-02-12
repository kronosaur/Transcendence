//	CMessageSession.cpp
//
//	CMessageSession class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

const int DLG_WIDTH =							350;

const int DEFAULT_BUTTON_HEIGHT =				64;
const int DEFAULT_BUTTON_WIDTH =				96;

#define CMD_CLOSE								CONSTLIT("cmdClose")

#define EVENT_ON_CLICK							CONSTLIT("onClick")

#define ID_DLG_MESSAGE							CONSTLIT("dlgMessage")

#define PROP_COLOR								CONSTLIT("color")
#define PROP_FONT								CONSTLIT("font")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_TEXT								CONSTLIT("text")

CMessageSession::CMessageSession (CHumanInterface &HI, const CString &sTitle, const CString &sMessage, const CString &sCommand) : IHISession(HI), 
		m_sTitle(sTitle),
		m_sMessage(sMessage),
		m_sCommand(sCommand)

//	CMessageSession constructor

	{
	}

void CMessageSession::CmdDone (void)

//	CmdDone
//
//	Close the message session.

	{
	//	Remember the command because after we close we will have freed the object

	CHumanInterface &HI(m_HI);
	CString sCommand = m_sCommand;

	//	Done with registration

	m_HI.ClosePopupSession();

	//	If we have a command that needed to wait for login, fire it now.

	if (!sCommand.IsBlank())
		HI.HICommand(sCommand);
	}

void CMessageSession::CreateDlgMessage (IAnimatron **retpDlg)

//	CreateDlgMessage
//
//	Creates the message dialog box

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);

	//	Figure out where the login dialog box will appear

	RECT rcCenter;
	VI.GetWidescreenRect(m_HI.GetScreen(), &rcCenter);

	RECT rcDlg = rcCenter;
	int cyDlg = 10 * VI.GetFont(fontLarge).GetHeight();
	rcDlg.top = rcCenter.top + (RectHeight(rcCenter) - cyDlg) / 2;
	rcDlg.left = rcCenter.left + (RectWidth(rcCenter) - DLG_WIDTH) / 2;
	rcDlg.right = rcDlg.left + DLG_WIDTH;
	rcDlg.bottom = rcDlg.top + cyDlg;

	//	Create the dialog box and a container for the controls

	IAnimatron *pDlg;
	CAniSequencer *pContainer;
	VI.CreateStdDialog(rcDlg, m_sTitle, &pDlg, &pContainer);

	int y = 0;

	//	Add the message

	IAnimatron *pMessage = new CAniText;
	pMessage->SetPropertyVector(PROP_POSITION, CVector(0, y));
	pMessage->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcDlg), RectHeight(rcDlg)));
	pMessage->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogInput));
	pMessage->SetPropertyFont(PROP_FONT, &MediumFont);
	pMessage->SetPropertyString(PROP_TEXT, m_sMessage);

	pContainer->AddTrack(pMessage, 0);

	//	Add close button at the bottom

	IAnimatron *pButton;
	int xButtons = (RectWidth(rcDlg) - DEFAULT_BUTTON_WIDTH) / 2;
	int yButtons = RectHeight(rcDlg) - DEFAULT_BUTTON_HEIGHT;
	VI.CreateButton(pContainer, CMD_CLOSE, xButtons, yButtons, DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, CVisualPalette::OPTION_BUTTON_DEFAULT, CONSTLIT("OK"), &pButton);
	RegisterPerformanceEvent(pButton, EVENT_ON_CLICK, CMD_CLOSE);

	//	Done

	*retpDlg = pDlg;
	}

ALERROR CMessageSession::OnCommand (const CString &sCmd, void *pData)

//	OnCommand
//
//	Handle a command

	{
	if (strEquals(sCmd, CMD_CLOSE))
		CmdDone();

	return NOERROR;
	}

ALERROR CMessageSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	//	We're showing dialog boxes, so the previous session shows through.

	SetTransparent();

	//	Create the dialog box

	IAnimatron *pDlg;
	CreateDlgMessage(&pDlg);
	StartPerformance(pDlg, ID_DLG_MESSAGE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Done

	return NOERROR;
	}

void CMessageSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//	OnKeyDown
//
//	KeyDown

	{
	switch (iVirtKey)
		{
		case VK_ESCAPE:
			CmdDone();
			break;

		case VK_RETURN:
			CmdDone();
			break;
		}
	}

void CMessageSession::OnPaint (CG16bitImage &Screen, const RECT &rcInvalid)

//	OnPaint
//
//	Paint

	{
	Screen.FillTrans(0, 0, Screen.GetWidth(), Screen.GetHeight(), 0, 128);
	}

void CMessageSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash
//
//	Describe current state

	{
	*retsMessage = CONSTLIT("session: CMessageSession\r\n");
	}

