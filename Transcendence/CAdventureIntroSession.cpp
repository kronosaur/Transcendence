//	CAdventureIntroSession.cpp
//
//	CAdventureIntroSession class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define CMD_CLOSE_SESSION						CONSTLIT("cmdCloseSession")

#define ID_CTRL_TITLE							CONSTLIT("ctrlTitle")
#define ID_CTRL_WAIT							CONSTLIT("ctrlWait")
#define ID_MESSAGE								CONSTLIT("idMessage")

#define PROP_COLOR								CONSTLIT("color")
#define PROP_FONT								CONSTLIT("font")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_TEXT								CONSTLIT("text")

CAdventureIntroSession::CAdventureIntroSession (CHumanInterface &HI, CCloudService &Service, const CString &sAdventureName) : IHISession(HI), 
		m_Service(Service),
		m_sAdventureName(sAdventureName)

//	CAdventureIntroSession constructor

	{
	}

void CAdventureIntroSession::CmdDone (void)

//	CmdDone
//
//	Close the session.

	{
	m_HI.ClosePopupSession();
	}

ALERROR CAdventureIntroSession::OnCommand (const CString &sCmd, void *pData)

//	OnCommand
//
//	Handle a command

	{
	return NOERROR;
	}

ALERROR CAdventureIntroSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	RECT rcRect;
	VI.GetWidescreenRect(&rcRect);

	//	Create the title

	CUIHelper Helper(m_HI);
	IAnimatron *pTitle;
	Helper.CreateSessionTitle(this, m_Service, m_sAdventureName, NULL, CUIHelper::OPTION_SESSION_NO_CANCEL_BUTTON, &pTitle);
	StartPerformance(pTitle, ID_CTRL_TITLE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Create a wait animation

	IAnimatron *pAni;
	VI.CreateWaitAnimation(NULL, ID_CTRL_WAIT, rcRect, &pAni);
	StartPerformance(pAni, ID_CTRL_WAIT, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Done

	return NOERROR;
	}

void CAdventureIntroSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//	OnKeyDown
//
//	KeyDown

	{
	}

void CAdventureIntroSession::OnPaint (CG32bitImage &Screen, const RECT &rcInvalid)

//	OnPaint
//
//	Paint

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	RECT rcCenter;
	VI.DrawSessionBackground(Screen, CG32bitImage(), CG32bitPixel(), CVisualPalette::OPTION_SESSION_DLG_BACKGROUND, &rcCenter);
	}

void CAdventureIntroSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash
//
//	Describe current state

	{
	*retsMessage = CONSTLIT("session: CAdventureIntroSession\r\n");
	}
