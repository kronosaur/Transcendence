//	CProfileSession.cpp
//
//	CProfileSession class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define CMD_CLOSE_SESSION						CONSTLIT("cmdCloseSession")
#define CMD_READ_COMPLETE						CONSTLIT("cmdReadComplete")

#define ID_CTRL_TITLE							CONSTLIT("ctrlTitle")
#define ID_CTRL_WAIT							CONSTLIT("ctrlWait")
#define ID_MESSAGE								CONSTLIT("idMessage")
#define ID_PROFILE								CONSTLIT("idProfile")

#define ERR_TITLE								CONSTLIT("Unable to retrieve data")
#define ERR_DESC								CONSTLIT("Unable to retrive data from server: %s")

#define PROP_MAX_SCROLL_POS						CONSTLIT("maxScrollPos")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCROLL_POS							CONSTLIT("scrollPos")
#define PROP_VIEWPORT_HEIGHT					CONSTLIT("viewportHeight")

const int MAX_RECORD_WIDTH =					1024;

const int PADDING_BOTTOM =						20;

CProfileSession::CProfileSession (CHumanInterface &HI, CCloudService &Service) : IHISession(HI), 
		m_Service(Service)

//	CProfileSession constructor

	{
	}

void CProfileSession::CmdReadComplete (CReadProfileTask *pTask)

//	CmdReadComplete
//
//	Done reading the profile

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	//	Done with wait animation

	StopPerformance(ID_CTRL_WAIT);

	//	Check for error

	CString sError;
	if (pTask->GetResult(&sError) != NOERROR)
		{
		IAnimatron *pMsg;
		VI.CreateMessagePane(NULL, ID_MESSAGE, ERR_TITLE, strPatternSubst(ERR_DESC, sError), m_rcRecords, 0, &pMsg);

		StartPerformance(pMsg, ID_MESSAGE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
		return;
		}

	//	Show the profile

	IAnimatron *pList = pTask->GetListHandoff();
	pList->SetPropertyVector(PROP_POSITION, CVector(m_rcRecords.left, m_rcRecords.top));
	pList->SetPropertyMetric(PROP_VIEWPORT_HEIGHT, (Metric)RectHeight(m_rcRecords));

	StartPerformance(pList, ID_PROFILE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
	}

void CProfileSession::GetPaneRect (RECT *retrcRect)

//	GetPaneRect
//
//	Returns the RECT of the pane

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	RECT rcCenter;
	RECT rcFull;
	VI.GetWidescreenRect(&rcCenter, &rcFull);

	*retrcRect = rcCenter;
	}

void CProfileSession::OnCleanUp (void)

//	OnCleanUp
//
//	Clean up

	{
	}

ALERROR CProfileSession::OnCommand (const CString &sCmd, void *pData)

//	OnCommand
//
//	Handle a command

	{
	if (strEquals(sCmd, CMD_READ_COMPLETE))
		CmdReadComplete((CReadProfileTask *)pData);
	else if (strEquals(sCmd, CMD_CLOSE_SESSION))
		m_HI.ClosePopupSession();

	return NOERROR;
	}

ALERROR CProfileSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	GetPaneRect(&m_rcPane);

	int cxWidth = Min(RectWidth(m_rcPane), MAX_RECORD_WIDTH);
	m_rcRecords.left = m_rcPane.left + (RectWidth(m_rcPane) - cxWidth) / 2;
	m_rcRecords.right = m_rcRecords.left + cxWidth;
	m_rcRecords.top = m_rcPane.top;
	m_rcRecords.bottom = m_rcPane.bottom;

	//	Create a task to read the profile information and wait until it
	//	finishes.

	m_HI.AddBackgroundTask(new CReadProfileTask(m_HI, m_Service, RectWidth(m_rcRecords)), 0, this, CMD_READ_COMPLETE);

	//	Create the title

	CUIHelper Helper(m_HI);
	IAnimatron *pTitle;
	Helper.CreateSessionTitle(this, m_Service, CONSTLIT("Scores & Records"), NULL, 0, &pTitle);
	StartPerformance(pTitle, ID_CTRL_TITLE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Create a wait animation

	IAnimatron *pAni;
	VI.CreateWaitAnimation(NULL, ID_CTRL_WAIT, m_rcPane, &pAni);
	StartPerformance(pAni, ID_CTRL_WAIT, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Done

	return NOERROR;
	}

void CProfileSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//	OnKeyDown
//
//	KeyDown

	{
	switch (iVirtKey)
		{
		case VK_ESCAPE:
			m_HI.ClosePopupSession();
			break;

		default:
			HandlePageScrollKeyDown(ID_PROFILE, iVirtKey, dwKeyData);
			break;
		}
	}

void CProfileSession::OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture)

//	OnLButtonDown
//
//	LButtonDown

	{
	//	Done

//	m_HI.ClosePopupSession();
	}

void CProfileSession::OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags)

//	OnMouseWheel
//
//	Handle mouse wheel

	{
	HandlePageScrollMouseWheel(ID_PROFILE, iDelta);
	}

void CProfileSession::OnPaint (CG32bitImage &Screen, const RECT &rcInvalid)

//	OnPaint
//
//	Paint

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	RECT rcCenter;
	VI.DrawSessionBackground(Screen, CG32bitImage(), CG32bitPixel(), CVisualPalette::OPTION_SESSION_DLG_BACKGROUND, &rcCenter);
	}

void CProfileSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash
//
//	Describe current state

	{
	*retsMessage = CONSTLIT("session: CProfileSession\r\n");
	}

void CProfileSession::OnUpdate (bool bTopMost)

//	OnUpdate
//
//	Update

	{
	}
