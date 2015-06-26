//	CModExchangeSession.cpp
//
//	CModExchangeSession class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define CMD_CLOSE_SESSION						CONSTLIT("cmdCloseSession")
#define CMD_OK_SESSION							CONSTLIT("cmdOKSession")
#define CMD_REFRESH								CONSTLIT("cmdRefresh")
#define CMD_REFRESH_COMPLETE					CONSTLIT("cmdRefreshComplete")

#define CMD_SERVICE_EXTENSION_LOADED			CONSTLIT("serviceExtensionLoaded")

#define ID_CTRL_TITLE							CONSTLIT("ctrlTitle")
#define ID_CTRL_WAIT							CONSTLIT("ctrlWait")
#define ID_MESSAGE								CONSTLIT("idMessage")
#define ID_LIST									CONSTLIT("idList")

#define EVENT_ON_DOUBLE_CLICK					CONSTLIT("onDoubleClick")

#define PROP_COLOR								CONSTLIT("color")
#define PROP_FONT								CONSTLIT("font")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_TEXT								CONSTLIT("text")
#define PROP_VIEWPORT_HEIGHT					CONSTLIT("viewportHeight")

#define URL_MULTIVERSE_CATALOG					CONSTLIT("http://multiverse.kronosaur.com/catalog.hexm")

#define ERR_DESC								CONSTLIT("Unable to retrive collection from Multiverse: %s")
#define ERR_TITLE								CONSTLIT("Unable to list collection")

const int ENTRY_WIDTH =							720;

CModExchangeSession::CModExchangeSession (CHumanInterface &HI, CCloudService &Service, CMultiverseModel &Multiverse, CExtensionCollection &Extensions, bool bDebugMode) : IHISession(HI), 
		m_Service(Service),
		m_Multiverse(Multiverse),
		m_Extensions(Extensions),
		m_bDebugMode(bDebugMode),
		m_bWaitingForRefresh(false),
		m_bRefreshAgain(false)

//	CModExchangeSession constructor

	{
	}

void CModExchangeSession::CmdDone (void)

//	CmdDone
//
//	Close the session.

	{
	m_HI.ClosePopupSession();
	}

void CModExchangeSession::CmdRefresh (bool bFullRefresh)

//	CmdRefresh
//
//	Refresh the list

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	RECT rcCenter;
	VI.GetWidescreenRect(m_HI.GetScreen(), &rcCenter);

	//	Done with current list

	StopPerformance(ID_LIST);

	//	Create a task to read the list of save files from disk

	m_HI.AddBackgroundTask(new CListCollectionTask(m_HI, m_Extensions, m_Multiverse, m_Service, ENTRY_WIDTH, !bFullRefresh, m_bDebugMode), 0, this, CMD_REFRESH_COMPLETE);

	//	Create a wait animation

	IAnimatron *pAni;
	VI.CreateWaitAnimation(NULL, ID_CTRL_WAIT, rcCenter, &pAni);
	StartPerformance(pAni, ID_CTRL_WAIT, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
	}

void CModExchangeSession::CmdRefreshComplete (CListCollectionTask *pTask)

//	CmdRefreshComplete
//
//	Done with refresh

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	RECT rcRect;
	VI.GetWidescreenRect(m_HI.GetScreen(), &rcRect);

	//	Done with wait animation

	StopPerformance(ID_CTRL_WAIT);
	m_bWaitingForRefresh = false;

	//	Check for error

	CString sError;
	if (pTask->GetResult(&sError) != NOERROR)
		{
		IAnimatron *pMsg;
		VI.CreateMessagePane(NULL, ID_MESSAGE, ERR_TITLE, strPatternSubst(ERR_DESC, sError), rcRect, 0, &pMsg);

		StartPerformance(pMsg, ID_MESSAGE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
		return;
		}

	//	Get the list

	IAnimatron *pList = pTask->GetListHandoff();

	//	Show it

	RECT rcList;
	pList->GetSpacingRect(&rcList);

	pList->SetPropertyVector(PROP_POSITION, CVector(rcRect.left + (RectWidth(rcRect) - ENTRY_WIDTH) / 2, rcRect.top));
	pList->SetPropertyVector(PROP_SCALE, CVector(ENTRY_WIDTH, RectHeight(rcRect)));
	pList->SetPropertyMetric(PROP_VIEWPORT_HEIGHT, (Metric)RectHeight(rcRect));

	StartPerformance(pList, ID_LIST, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	If we were asked to refresh while waiting, refresh again.

	if (m_bRefreshAgain)
		{
		m_bRefreshAgain = false;
		CmdRefresh(false);
		}
	}

ALERROR CModExchangeSession::OnCommand (const CString &sCmd, void *pData)

//	OnCommand
//
//	Handle a command

	{
	if (strEquals(sCmd, CMD_CLOSE_SESSION))
		CmdDone();
	else if (strEquals(sCmd, CMD_OK_SESSION))
#ifdef STEAM_BUILD
		CmdDone();
#else
		sysOpenURL(URL_MULTIVERSE_CATALOG);
#endif
	else if (strEquals(sCmd, CMD_REFRESH))
		CmdRefresh();
	else if (strEquals(sCmd, CMD_REFRESH_COMPLETE))
		CmdRefreshComplete((CListCollectionTask *)pData);
	else if (strEquals(sCmd, CMD_SERVICE_EXTENSION_LOADED))
		{
		if (!m_bWaitingForRefresh)
			CmdRefresh(false);
		else
			m_bRefreshAgain = true;
		}

	return NOERROR;
	}

ALERROR CModExchangeSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	RECT rcCenter;
	VI.GetWidescreenRect(m_HI.GetScreen(), &rcCenter);

	//	Create a task to read the list of save files from disk

	m_HI.AddBackgroundTask(new CListCollectionTask(m_HI, m_Extensions, m_Multiverse, m_Service, ENTRY_WIDTH, false, m_bDebugMode), 0, this, CMD_REFRESH_COMPLETE);
	m_bWaitingForRefresh = true;

	//	Create the title and menu

	CUIHelper Helper(m_HI);
	TArray<CUIHelper::SMenuEntry> Menu;
	DWORD dwOptions = 0;

#ifdef STEAM_BUILD
	dwOptions = CUIHelper::OPTION_SESSION_OK_BUTTON | CUIHelper::OPTION_SESSION_NO_CANCEL_BUTTON;
#else
	CUIHelper::SMenuEntry *pEntry = Menu.Insert();
	pEntry->sCommand = CMD_REFRESH;
	pEntry->sLabel = CONSTLIT("Refresh");

	dwOptions = CUIHelper::OPTION_SESSION_ADD_EXTENSION_BUTTON;
#endif

	IAnimatron *pTitle;
	Helper.CreateSessionTitle(this, m_Service, CONSTLIT("Mod Collection"), &Menu, dwOptions, &pTitle);
	StartPerformance(pTitle, ID_CTRL_TITLE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Create a wait animation

	IAnimatron *pAni;
	VI.CreateWaitAnimation(NULL, ID_CTRL_WAIT, rcCenter, &pAni);
	StartPerformance(pAni, ID_CTRL_WAIT, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Done

	return NOERROR;
	}

void CModExchangeSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

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

void CModExchangeSession::OnPaint (CG32bitImage &Screen, const RECT &rcInvalid)

//	OnPaint
//
//	Paint

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	RECT rcCenter;
	VI.DrawSessionBackground(Screen, CG32bitImage(), CVisualPalette::OPTION_SESSION_DLG_BACKGROUND, &rcCenter);
	}

void CModExchangeSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash
//
//	Describe current state

	{
	*retsMessage = CONSTLIT("session: CModExchangeSession\r\n");
	}
