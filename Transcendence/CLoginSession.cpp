//	CLoginSession.cpp
//
//	CLoginSession class
//	Copyright (c) 2011 by Kronosaur Productions, LLC. All Rights Reserved.
//
//	NOTES
//
//	This session works as follows:
//
//	1.	On init we check to see if there are any background tasks running (for
//		example we could be trying to auto-sign-in in the background). If so
//		then we display a status dlg showing progress of the task. (We can 
//		access the background tasks through the HI object).
//
//	2.	Next we check to see if we're signed in (by asking the CloudService
//		object). If we are then we just display our signed-in status along
//		with buttons to sign out, etc.
//
//	3.	If we're not signed in, we check to see if we have a cached username.
//		If so, we display the sign in dialog box. When the user clicks "Sign In"
//		we add a background task to sign in and show progress until the task
//		completes. Then we either show and error message or close the screen.
//
//	4.	If we don't have a cached username then we display the register user
//		dialog box.

#include "PreComp.h"
#include "Transcendence.h"

const int DLG_HEIGHT =							400;
const int DLG_PADDING_LEFT =					10;
const int DLG_PADDING_RIGHT =					10;
const int DLG_PADDING_TOP =						10;
const int DLG_SPACING_X =						10;
const int DLG_SPACING_Y =						10;
const int DLG_WIDTH =							350;

const int DEFAULT_BUTTON_HEIGHT =				64;
const int DEFAULT_BUTTON_WIDTH =				96;

const int DEFAULT_LINK_HEIGHT =					24;

const int INPUT_ERROR_HEIGHT =					80;
const int INPUT_ERROR_MARGIN_RIGHT =			64;
const int INPUT_ERROR_PADDING_BOTTOM =			10;
const int INPUT_ERROR_PADDING_LEFT =			10;
const int INPUT_ERROR_PADDING_RIGHT =			10;
const int INPUT_ERROR_PADDING_TOP =				10;
const int INPUT_ERROR_WIDTH =					300;

const int INPUT_ERROR_TIME =					(30 * 10);

#define ID_DLG_MESSAGE							CONSTLIT("dlgMessage")
#define ID_DLG_REGISTER							CONSTLIT("dlgRegister")
#define ID_DLG_SIGN_IN							CONSTLIT("dlgSignIn")
#define ID_DLG_INPUT_ERROR						CONSTLIT("dlgInputError")
#define ID_CTRL_PASSWORD						CONSTLIT("ctrlPassword")
#define ID_CTRL_PASSWORD_CONFIRM				CONSTLIT("ctrlPasswordConfirm")
#define ID_CTRL_USERNAME						CONSTLIT("ctrlUsername")
#define ID_CTRL_MAIN_ACTION						CONSTLIT("ctrlMainAction")
#define ID_CTRL_CANCEL_ACTION					CONSTLIT("ctrlCancel")
#define ID_CTRL_AUTO_SIGN_IN					CONSTLIT("ctrlAutoSignIn")
#define ID_CTRL_REGISTER						CONSTLIT("ctrlRegister")
#define ID_CTRL_PASSWORD_RESET					CONSTLIT("ctrlPasswordReset")
#define ID_CTRL_EMAIL							CONSTLIT("ctrlEmail")
#define ID_CTRL_WAIT							CONSTLIT("ctrlWait")

#define CMD_ALL_TASKS_DONE						CONSTLIT("cmdAllTasksDone")
#define CMD_CANCEL								CONSTLIT("cmdCancel")
#define CMD_PASSWORD_RESET						CONSTLIT("cmdPasswordReset")
#define CMD_REGISTER							CONSTLIT("cmdRegister")
#define CMD_REGISTER_COMPLETE					CONSTLIT("cmdRegisterComplete")
#define CMD_SIGN_IN								CONSTLIT("cmdSignIn")
#define CMD_SIGN_IN_COMPLETE					CONSTLIT("cmdSignInComplete")
#define CMD_SWITCH_TO_LOGIN						CONSTLIT("cmdSwitchToLogin")
#define CMD_SWITCH_TO_REGISTER					CONSTLIT("cmdSwitchToRegister")
#define CMD_TOS									CONSTLIT("cmdTOS")

#define CMD_UI_RESET_PASSWORD					CONSTLIT("uiResetPassword")

#define EVENT_ON_CLICK							CONSTLIT("onClick")

#define PROP_CHECKED							CONSTLIT("checked")
#define PROP_COLOR								CONSTLIT("color")
#define PROP_ENABLED							CONSTLIT("enabled")
#define PROP_FONT								CONSTLIT("font")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_TEXT								CONSTLIT("text")

#define STR_REGISTER_TITLE						CONSTLIT("Register")
#define STR_SIGN_IN_TITLE						CONSTLIT("Sign In")

CLoginSession::CLoginSession (CHumanInterface &HI, CCloudService &Service, const CString &sCommand) : IHISession(HI), 
		m_Service(Service),
		m_sCommand(sCommand),
		m_iCurrent(dlgNone),
		m_bBlankEmailWarning(false)

//	CLoginSession constructor

	{
	}

void CLoginSession::CmdCancel (void)

//	CmdCancel
//
//	Cancel sign in

	{
	m_HI.ClosePopupSession();
	}

void CLoginSession::CmdPasswordReset (void)

//	CmdPasswordReset
//
//	Requests a password reset.

	{
	//	Remember the command because after we close we will have freed the object

	CHumanInterface &HI(m_HI);

	//	Done with dialog box

	m_HI.ClosePopupSession();

	//	Send command

	HI.HICommand(CMD_UI_RESET_PASSWORD);
	}

void CLoginSession::CmdRegister (void)

//	CmdRegister
//
//	Handle register command

	{
	CUIHelper Helper(m_HI);

	//	Get the fields

	CString sUsername = GetPropertyString(ID_CTRL_USERNAME, PROP_TEXT);
	CString sPassword = GetPropertyString(ID_CTRL_PASSWORD, PROP_TEXT);
	CString sPasswordConfirm = GetPropertyString(ID_CTRL_PASSWORD_CONFIRM, PROP_TEXT);
	CString sEmail = GetPropertyString(ID_CTRL_EMAIL, PROP_TEXT);
	bool bAutoSignIn = GetPropertyBool(ID_CTRL_AUTO_SIGN_IN, PROP_CHECKED);

	//	Get the text for the username. If blank, then we have an error.

	if (sUsername.IsBlank())
		{
		Helper.CreateInputErrorMessage(this, m_rcInputError, CONSTLIT("Username Missing"), CONSTLIT("You must have a username to register."));
		return;
		}

	//	Make sure the passwords match

	else if (!strEquals(sPassword, sPasswordConfirm))
		{
		Helper.CreateInputErrorMessage(this, m_rcInputError, CONSTLIT("Password Does Not Match"), CONSTLIT("The password you entered does not match the confirmation password."));
		return;
		}

	//	Validate password complexity

	CString sError;
	if (!CHexarc::ValidatePasswordComplexity(sPassword, &sError))
		{
		Helper.CreateInputErrorMessage(this, m_rcInputError, CONSTLIT("Password Is Too Easy"), sError);
		return;
		}

	//	If email is blank, explain why we need it

	if (!m_bBlankEmailWarning && sEmail.IsBlank())
		{
		Helper.CreateInputErrorMessage(this, m_rcInputError, CONSTLIT("Email Address Is Optional, but..."), CONSTLIT("If you provide your email address we will be able to reset your password if you request it."));
		m_bBlankEmailWarning = true;
		return;
		}

	//	Register the name

	m_HI.AddBackgroundTask(new CRegisterUserTask(m_HI, m_Service, sUsername, sPassword, sEmail, bAutoSignIn), 0, this, CMD_REGISTER_COMPLETE);

	//	Disable controls

	SetPropertyBool(ID_CTRL_USERNAME, PROP_ENABLED, false);
	SetPropertyBool(ID_CTRL_PASSWORD, PROP_ENABLED, false);
	SetPropertyBool(ID_CTRL_PASSWORD_CONFIRM, PROP_ENABLED, false);
	SetPropertyBool(ID_CTRL_MAIN_ACTION, PROP_ENABLED, false);
	SetPropertyBool(ID_CTRL_EMAIL, PROP_ENABLED, false);
	SetPropertyBool(ID_CTRL_AUTO_SIGN_IN, PROP_ENABLED, false);
	SetPropertyBool(CMD_SWITCH_TO_LOGIN, PROP_ENABLED, false);
	SetPropertyBool(CMD_TOS, PROP_ENABLED, false);
	}

void CLoginSession::CmdRegisterComplete (CRegisterUserTask *pTask)

//	CmdRegisterComplete
//
//	Registration task has returned.

	{
	CString sError;
	if (pTask->GetResult(&sError) != NOERROR)
		{
		CUIHelper Helper(m_HI);
		Helper.CreateInputErrorMessage(this, m_rcInputError, CONSTLIT("Unable to Register"), sError);

		//	Re-enable buttons so user can continue;

		SetPropertyBool(ID_CTRL_USERNAME, PROP_ENABLED, true);
		SetPropertyBool(ID_CTRL_PASSWORD, PROP_ENABLED, true);
		SetPropertyBool(ID_CTRL_PASSWORD_CONFIRM, PROP_ENABLED, true);
		SetPropertyBool(ID_CTRL_MAIN_ACTION, PROP_ENABLED, true);
		SetPropertyBool(ID_CTRL_EMAIL, PROP_ENABLED, true);
		SetPropertyBool(ID_CTRL_AUTO_SIGN_IN, PROP_ENABLED, true);
		SetPropertyBool(CMD_SWITCH_TO_LOGIN, PROP_ENABLED, true);
		SetPropertyBool(CMD_TOS, PROP_ENABLED, true);

		return;
		}

	//	Done

	CmdSuccess();
	}

void CLoginSession::CmdSignIn (void)

//	CmdSignIn
//
//	Sign in the user

	{
	CUIHelper Helper(m_HI);

	//	Get the fields

	CString sUsername = GetPropertyString(ID_CTRL_USERNAME, PROP_TEXT);
	CString sPassword = GetPropertyString(ID_CTRL_PASSWORD, PROP_TEXT);
	bool bAutoSignIn = GetPropertyBool(ID_CTRL_AUTO_SIGN_IN, PROP_CHECKED);

	//	Get the text for the username. If blank, then we have an error.

	if (sUsername.IsBlank())
		{
		Helper.CreateInputErrorMessage(this, m_rcInputError, CONSTLIT("Username Missing"), CONSTLIT("You must have a username to sign in."));
		return;
		}

	//	Register the name

	m_HI.AddBackgroundTask(new CSignInUserTask(m_HI, m_Service, sUsername, sPassword, bAutoSignIn), 0, this, CMD_SIGN_IN_COMPLETE);

	//	Disable controls

	SetPropertyBool(ID_CTRL_USERNAME, PROP_ENABLED, false);
	SetPropertyBool(ID_CTRL_PASSWORD, PROP_ENABLED, false);
	SetPropertyBool(ID_CTRL_MAIN_ACTION, PROP_ENABLED, false);
	SetPropertyBool(ID_CTRL_AUTO_SIGN_IN, PROP_ENABLED, false);
	SetPropertyBool(ID_CTRL_REGISTER, PROP_ENABLED, false);
	SetPropertyBool(ID_CTRL_PASSWORD_RESET, PROP_ENABLED, false);
	}

void CLoginSession::CmdSignInComplete (CSignInUserTask *pTask)

//	CmdSignInComplete
//
//	Sign in task has returned.

	{
	CString sError;
	if (pTask->GetResult(&sError) != NOERROR)
		{
		CUIHelper Helper(m_HI);
		Helper.CreateInputErrorMessage(this, m_rcInputError, CONSTLIT("Unable to Sign In"), sError);

		//	Re-enable buttons so user can continue;

		SetPropertyBool(ID_CTRL_USERNAME, PROP_ENABLED, true);
		SetPropertyBool(ID_CTRL_PASSWORD, PROP_ENABLED, true);
		SetPropertyBool(ID_CTRL_MAIN_ACTION, PROP_ENABLED, true);
		SetPropertyBool(ID_CTRL_AUTO_SIGN_IN, PROP_ENABLED, true);
		SetPropertyBool(ID_CTRL_REGISTER, PROP_ENABLED, true);
		SetPropertyBool(ID_CTRL_PASSWORD_RESET, PROP_ENABLED, true);

		return;
		}

	//	Done

	CmdSuccess();
	}

void CLoginSession::CmdSuccess (void)

//	CmdSuccess
//
//	We have successfully signed in. Close the dialog box and fire the subsequent
//	command, if necessary.

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

void CLoginSession::CreateDlgMessage (const CVisualPalette &VI, const RECT &rcRect, IAnimatron **retpAni)

//	CreateDlgMessage
//
//	Creates a message dialog box

	{
	//	Start with a sequencer as a parent of everything

	CAniSequencer *pDlg;
	CAniSequencer::Create(CVector(rcRect.left, rcRect.top), &pDlg);

	//	Add a rectangle as a background

	IAnimatron *pRect;
	CAniRect::Create(CVector(), 
			CVector(RectWidth(rcRect), RectHeight(rcRect)),
			VI.GetColor(colorAreaDialog),
			255,
			&pRect);
	pDlg->AddTrack(pRect, 0);

	//	Done

	*retpAni = pDlg;
	}

void CLoginSession::CreateDlgRegister (const CVisualPalette &VI, IAnimatron **retpAni)

//	CreateDlgRegister
//
//	Creates a dialog box

	{
	//	Figure out where the login dialog box will appear

	RECT rcCenter;
	RECT rcFull;
	VI.GetWidescreenRect(&rcCenter, &rcFull);

	RECT rcDlg = rcCenter;
	int cyDlg = 22 * VI.GetFont(fontLarge).GetHeight();
	rcDlg.top = rcCenter.top + (RectHeight(rcCenter) - cyDlg) / 2;
	rcDlg.left = rcCenter.left + (RectWidth(rcCenter) - DLG_WIDTH) / 2;
	rcDlg.right = rcDlg.left + DLG_WIDTH;
	rcDlg.bottom = rcDlg.top + cyDlg;

	//	Create the dialog box and a container for the controls

	IAnimatron *pDlg;
	CAniSequencer *pContainer;
	VI.CreateStdDialog(rcDlg, STR_REGISTER_TITLE, &pDlg, &pContainer);

	int y = 0;

	//	Add a username field

	int cyHeight;
	VI.CreateEditControl(pContainer, ID_CTRL_USERNAME, 0, y, RectWidth(rcDlg), 0, CONSTLIT("Username:"), NULL, &cyHeight);
	y += cyHeight + DLG_SPACING_Y;

	//	Add a password field

	VI.CreateEditControl(pContainer, ID_CTRL_PASSWORD, 0, y, RectWidth(rcDlg), CVisualPalette::OPTION_EDIT_PASSWORD, CONSTLIT("Password:"), NULL, &cyHeight);
	y += cyHeight + DLG_SPACING_Y;

	//	Add a confirm password field

	VI.CreateEditControl(pContainer, ID_CTRL_PASSWORD_CONFIRM, 0, y, RectWidth(rcDlg), CVisualPalette::OPTION_EDIT_PASSWORD, CONSTLIT("Confirm password:"), NULL, &cyHeight);
	y += cyHeight + DLG_SPACING_Y;

	//	Add email field

	VI.CreateEditControl(pContainer, ID_CTRL_EMAIL, 0, y, RectWidth(rcDlg),	0, CONSTLIT("Email:"), NULL, &cyHeight);
	y += cyHeight + DLG_SPACING_Y;

	//	Auto sign in checkbox

	IAnimatron *pControl;
	VI.CreateCheckbox(pContainer, ID_CTRL_AUTO_SIGN_IN, 0, y, RectWidth(rcDlg), 0, CONSTLIT("Sign in automatically"), &pControl, &cyHeight);
	y += cyHeight + DLG_SPACING_Y;

	//	Some extra space

	y += DLG_SPACING_Y;

	//	Register a new account

	IAnimatron *pButton;
	VI.CreateLink(pContainer, CMD_SWITCH_TO_LOGIN, 0, y, CONSTLIT("I already have an account"), 0, &pButton, NULL, &cyHeight);
	y += cyHeight;
	RegisterPerformanceEvent(pButton, EVENT_ON_CLICK, CMD_SWITCH_TO_LOGIN);

	//	Terms of service

	VI.CreateLink(pContainer, CMD_TOS, 0, y, CONSTLIT("Terms of Service"), 0, &pButton, NULL, &cyHeight);
	y += cyHeight + DLG_SPACING_Y;
	RegisterPerformanceEvent(pButton, EVENT_ON_CLICK, CMD_TOS);

	//	Add register button at the bottom

	int xButtons = (RectWidth(rcDlg) - (2 * DEFAULT_BUTTON_WIDTH + DLG_SPACING_X)) / 2;
	int yButtons = RectHeight(rcDlg) - DEFAULT_BUTTON_HEIGHT;
	VI.CreateButton(pContainer, ID_CTRL_MAIN_ACTION, xButtons, yButtons, DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, CVisualPalette::OPTION_BUTTON_DEFAULT, CONSTLIT("Register"), &pButton);
	RegisterPerformanceEvent(pButton, EVENT_ON_CLICK, CMD_REGISTER);

	//	Add the cancel button

	int x = xButtons + DLG_SPACING_X + DEFAULT_BUTTON_WIDTH;
	VI.CreateButton(pContainer, ID_CTRL_CANCEL_ACTION, x, yButtons, DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, 0, CONSTLIT("Cancel"), &pButton);
	RegisterPerformanceEvent(pButton, EVENT_ON_CLICK, CMD_CANCEL);

	//	Error messages are to the left of the button

	m_rcInputError.left = rcDlg.left + xButtons - metricsInputErrorMsgMarginHorz - metricsInputErrorMsgWidth;
	m_rcInputError.top = rcDlg.top + yButtons;
	m_rcInputError.right = m_rcInputError.left + metricsInputErrorMsgWidth;
	m_rcInputError.bottom = m_rcInputError.top + metricsInputErrorMsgHeight;

	//	Done

	*retpAni = pDlg;
	}

void CLoginSession::CreateDlgSignIn (const CVisualPalette &VI, IAnimatron **retpAni)

//	CreateDlgSingIn
//
//	Creates a message dialog box

	{
	//	Figure out where the login dialog box will appear

	RECT rcCenter;
	RECT rcFull;
	VI.GetWidescreenRect(&rcCenter, &rcFull);

	RECT rcDlg = rcCenter;
	int cyDlg = 18 * VI.GetFont(fontLarge).GetHeight();
	rcDlg.top = rcCenter.top + (RectHeight(rcCenter) - cyDlg) / 2;
	rcDlg.left = rcCenter.left + (RectWidth(rcCenter) - DLG_WIDTH) / 2;
	rcDlg.right = rcDlg.left + DLG_WIDTH;
	rcDlg.bottom = rcDlg.top + cyDlg;

	//	Create the dialog box and a container for the controls

	IAnimatron *pDlg;
	CAniSequencer *pContainer;
	VI.CreateStdDialog(rcDlg, STR_SIGN_IN_TITLE, &pDlg, &pContainer);

	int y = 0;

	//	Add a username field

	int cyHeight;
	IAnimatron *pControl;
	VI.CreateEditControl(pContainer, ID_CTRL_USERNAME, 0, y, RectWidth(rcDlg), 0, CONSTLIT("Username:"), &pControl, &cyHeight);
	pControl->SetPropertyString(PROP_TEXT, m_Service.GetDefaultUsername());
	y += cyHeight + DLG_SPACING_Y;

	//	Add a password field

	VI.CreateEditControl(pContainer, ID_CTRL_PASSWORD, 0, y, RectWidth(rcDlg), CVisualPalette::OPTION_EDIT_PASSWORD, CONSTLIT("Password:"), NULL, &cyHeight);
	y += cyHeight + DLG_SPACING_Y;

	//	Auto sign in checkbox

	VI.CreateCheckbox(pContainer, ID_CTRL_AUTO_SIGN_IN, 0, y, RectWidth(rcDlg), 0, CONSTLIT("Sign in automatically"), &pControl, &cyHeight);
	y += cyHeight + DLG_SPACING_Y;
	if (m_Service.HasCapability(ICIService::autoLoginUser))
		pControl->SetPropertyBool(PROP_CHECKED, true);

	//	Some extra space

	y += DLG_SPACING_Y;

	//	Register a new account

	IAnimatron *pButton;
	VI.CreateLink(pContainer, ID_CTRL_REGISTER, 0, y, CONSTLIT("Register a new account"), 0, &pButton, NULL, &cyHeight);
	y += cyHeight;
	RegisterPerformanceEvent(pButton, EVENT_ON_CLICK, CMD_SWITCH_TO_REGISTER);

	//	Forgot password

	VI.CreateLink(pContainer, ID_CTRL_PASSWORD_RESET, 0, y, CONSTLIT("Forgot my password"), 0, &pButton, NULL, &cyHeight);
	y += cyHeight + DLG_SPACING_Y;
	RegisterPerformanceEvent(pButton, EVENT_ON_CLICK, CMD_PASSWORD_RESET);

	//	Add sign in button at the bottom

	int xButtons = (RectWidth(rcDlg) - (2 * DEFAULT_BUTTON_WIDTH + DLG_SPACING_X)) / 2;
	int yButtons = RectHeight(rcDlg) - DEFAULT_BUTTON_HEIGHT;
	VI.CreateButton(pContainer, ID_CTRL_MAIN_ACTION, xButtons, yButtons, DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, CVisualPalette::OPTION_BUTTON_DEFAULT, CONSTLIT("Sign In"), &pButton);
	RegisterPerformanceEvent(pButton, EVENT_ON_CLICK, CMD_SIGN_IN);

	//	Add the cancel button

	int x = xButtons + DLG_SPACING_X + DEFAULT_BUTTON_WIDTH;
	VI.CreateButton(pContainer, ID_CTRL_CANCEL_ACTION, x, yButtons, DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, 0, CONSTLIT("Cancel"), &pButton);
	RegisterPerformanceEvent(pButton, EVENT_ON_CLICK, CMD_CANCEL);

	//	Error messages are to the left of the button

	m_rcInputError.left = rcDlg.left + xButtons - metricsInputErrorMsgMarginHorz - metricsInputErrorMsgWidth;
	m_rcInputError.top = rcDlg.top + yButtons;
	m_rcInputError.right = m_rcInputError.left + metricsInputErrorMsgWidth;
	m_rcInputError.bottom = m_rcInputError.top + metricsInputErrorMsgHeight;

	//	Done

	*retpAni = pDlg;
	}

void CLoginSession::OnCleanUp (void)

//	OnCleanUp
//
//	Clean up

	{
	}

ALERROR CLoginSession::OnCommand (const CString &sCmd, void *pData)

//	OnCommand
//
//	Handle a command

	{
	if (strEquals(sCmd, CMD_ALL_TASKS_DONE))
		{
		StopPerformance(ID_CTRL_WAIT);
		ShowInitialDlg();
		}

	else if (strEquals(sCmd, CMD_CANCEL))
		CmdCancel();

	else if (strEquals(sCmd, CMD_REGISTER))
		CmdRegister();

	else if (strEquals(sCmd, CMD_REGISTER_COMPLETE))
		CmdRegisterComplete((CRegisterUserTask *)pData);

	else if (strEquals(sCmd, CMD_SIGN_IN))
		CmdSignIn();

	else if (strEquals(sCmd, CMD_SIGN_IN_COMPLETE))
		CmdSignInComplete((CSignInUserTask *)pData);

	else if (strEquals(sCmd, CMD_SWITCH_TO_LOGIN))
		{
		StopPerformance(ID_DLG_REGISTER);

		const CVisualPalette &VI = m_HI.GetVisuals();
		IAnimatron *pDlg;

		CreateDlgSignIn(VI, &pDlg);
		m_iCurrent = dlgSignIn;
		StartPerformance(pDlg, ID_DLG_SIGN_IN, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
		}

	else if (strEquals(sCmd, CMD_SWITCH_TO_REGISTER))
		{
		StopPerformance(ID_DLG_SIGN_IN);

		const CVisualPalette &VI = m_HI.GetVisuals();
		IAnimatron *pDlg;

		CreateDlgRegister(VI, &pDlg);
		m_iCurrent = dlgRegister;
		StartPerformance(pDlg, ID_DLG_REGISTER, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
		}

	else if (strEquals(sCmd, CMD_PASSWORD_RESET))
		CmdPasswordReset();

	else if (strEquals(sCmd, CMD_TOS))
		sysOpenURL(CONSTLIT("http://www.kronosaur.com/legal/TermsofService.html"));

	return NOERROR;
	}

ALERROR CLoginSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	//	We're showing dialog boxes, so the previous session shows through.

	SetTransparent();

	//	See if any background tasks are running. If so, then we show the status
	//	dialog box and wait for tasks to complete.

	if (m_HI.RegisterOnAllBackgroundTasksComplete(this))
		{
		const CVisualPalette &VI = m_HI.GetVisuals();
		RECT rcRect;
		VI.GetWidescreenRect(&rcRect);

		//	Create a wait animation

		IAnimatron *pAni;
		VI.CreateWaitAnimation(NULL, ID_CTRL_WAIT, rcRect, &pAni);
		StartPerformance(pAni, ID_CTRL_WAIT, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

		return NOERROR;
		}
	
	//	Otherwise, we show the sign in dialog box

	ShowInitialDlg();

	//	Done

	return NOERROR;
	}

void CLoginSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

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
			if (m_iCurrent == dlgSignIn)
				CmdSignIn();
			else if (m_iCurrent == dlgRegister)
				CmdRegister();

			break;
		}
	}

void CLoginSession::OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture)

//	OnLButtonDown
//
//	LButtonDown

	{
	}

void CLoginSession::OnPaint (CG32bitImage &Screen, const RECT &rcInvalid)

//	OnPaint
//
//	Paint

	{
	Screen.Fill(0, 0, Screen.GetWidth(), Screen.GetHeight(), CG32bitPixel(0, 0, 0, 128));
	}

void CLoginSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash
//
//	Describe current state

	{
	*retsMessage = CONSTLIT("session: CLoginSession\r\n");
	}

void CLoginSession::OnUpdate (bool bTopMost)

//	OnUpdate
//
//	Update

	{
	}

void CLoginSession::ShowInitialDlg (void)

//	ShowInitialDlg
//
//	Shows the first dialog box

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	IAnimatron *pDlg;

	//	If we're already signed in then we're done.

	if (m_Service.HasCapability(ICIService::canGetUserProfile))
		CmdCancel();

	//	Otherwise, if we have a username

	else if (m_Service.HasCapability(ICIService::cachedUser))
		{
		CreateDlgSignIn(VI, &pDlg);
		m_iCurrent = dlgSignIn;
		StartPerformance(pDlg, ID_DLG_SIGN_IN, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

		if (!m_Service.GetDefaultUsername().IsBlank())
			SetInputFocus(ID_CTRL_PASSWORD);
		}

	//	Otherwise, register

	else if (m_Service.HasCapability(ICIService::registerUser))
		{
		CreateDlgRegister(VI, &pDlg);
		m_iCurrent = dlgRegister;
		StartPerformance(pDlg, ID_DLG_REGISTER, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
		}

	//	Otherwise, cannot sign in

	else
		CmdCancel();
	}

