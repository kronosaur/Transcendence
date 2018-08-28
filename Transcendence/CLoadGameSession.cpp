//	CLoadGameSession.cpp
//
//	CLoadGameSession class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define CMD_CLOSE_SESSION						CONSTLIT("cmdCloseSession")
#define CMD_DELETE_FILE							CONSTLIT("cmdDeleteFile")
#define CMD_OK_SESSION							CONSTLIT("cmdOKSession")
#define CMD_READ_COMPLETE						CONSTLIT("cmdReadComplete")

#define CMD_GAME_LOAD							CONSTLIT("gameLoad")

#define ERR_NO_ENTRIES							CONSTLIT("No save files")
#define ERR_NO_ENTRIES_DESC						CONSTLIT("There are no old games to load.")
#define ERR_TITLE								CONSTLIT("Unable to list files")
#define ERR_DESC								CONSTLIT("Unable to retrive a list of save files: %s")

#define EVENT_ON_DOUBLE_CLICK					CONSTLIT("onDoubleClick")

#define ID_CTRL_TITLE							CONSTLIT("ctrlTitle")
#define ID_CTRL_WAIT							CONSTLIT("ctrlWait")
#define ID_MESSAGE								CONSTLIT("idMessage")
#define ID_LIST									CONSTLIT("idList")

#define PROP_COLOR								CONSTLIT("color")
#define PROP_COUNT								CONSTLIT("count")
#define PROP_ENABLED							CONSTLIT("enabled")
#define PROP_FONT								CONSTLIT("font")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_SELECTION_ID						CONSTLIT("selectionID")
#define PROP_TEXT								CONSTLIT("text")
#define PROP_VIEWPORT_HEIGHT					CONSTLIT("viewportHeight")

const int SAVE_ENTRY_WIDTH =					720;

CLoadGameSession::CLoadGameSession (CHumanInterface &HI, CCloudService &Service, const TArray<CString> &Folders, bool bFilterPermadeath) : IHISession(HI),
		m_Service(Service),
		m_Folders(Folders),
		m_bFilterPermadeath(bFilterPermadeath)

//	CLoadGameSession constructor

	{
	}

void CLoadGameSession::CmdCancel (void)

//	CmdCancel
//
//	Close the session.

	{
	m_HI.ClosePopupSession();
	}

void CLoadGameSession::CmdDeleteFile (void)

//	CmdDeleteFile
//
//	Send file to recycle bin

	{
	//	Get the filename to delete.

	IAnimatron *pList = GetElement(ID_LIST);
	if (pList == NULL)
		return;

	CString sFilename = pList->GetPropertyString(PROP_SELECTION_ID);
	if (sFilename.IsBlank())
		return;

	//	Delete the file

	if (!fileDelete(sFilename, true))
		{
		m_HI.OpenPopupSession(new CMessageSession(m_HI, CONSTLIT("Unable to Recycle"), strPatternSubst(CONSTLIT("Unable to send file to recycle bin: %s"), sFilename)));
		return;
		}

	//	Remove from list

	pList->DeleteElement(sFilename);
	}

void CLoadGameSession::CmdOK (void)

//	CmdOK
//
//	Load the selected saved game

	{
	//	Get the list of files. If there is no list then it is because there are
	//	no save files.

	IAnimatron *pList = GetElement(ID_LIST);
	if (pList == NULL)
		return CmdCancel();

	CString sFilename = pList->GetPropertyString(PROP_SELECTION_ID);
	if (sFilename.IsBlank())
		return CmdCancel();
	
	//	Remember some variables because after we close the session this object
	//	will be gone.

	CHumanInterface &HI(m_HI);

	//	Close the session

	m_HI.ClosePopupSession();

	//	Send a command to the controller

	HI.HICommand(CMD_GAME_LOAD, &sFilename);
	}

void CLoadGameSession::CmdReadComplete (CListSaveFilesTask *pTask)

//	CmdReadComplete
//
//	We've finished loading the list of games

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	RECT rcRect;
	VI.GetWidescreenRect(&rcRect);

	//	Done with wait animation

	StopPerformance(ID_CTRL_WAIT);

	//	Check for error

	CString sError;
	if (pTask->GetResult(&sError) != NOERROR)
		{
		IAnimatron *pMsg;
		VI.CreateMessagePane(NULL, ID_MESSAGE, ERR_TITLE, strPatternSubst(ERR_DESC, sError), rcRect, 0, &pMsg);

		StartPerformance(pMsg, ID_MESSAGE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
		return;
		}

	//	If we have no entries, then show a message

	IAnimatron *pList = pTask->GetListHandoff();
	if (pList->GetPropertyInteger(PROP_COUNT) == 0)
		{
		delete pList;

		IAnimatron *pMsg;
		VI.CreateMessagePane(NULL, ID_MESSAGE, ERR_NO_ENTRIES, ERR_NO_ENTRIES_DESC, rcRect, 0, &pMsg);

		StartPerformance(pMsg, ID_MESSAGE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
		return;
		}

	//	Enable the delete menu item

	SetPropertyBool(CMD_DELETE_FILE, PROP_ENABLED, true);

	//	Show the profile

	RECT rcList;
	pList->GetSpacingRect(&rcList);

	pList->SetPropertyVector(PROP_POSITION, CVector(rcRect.left + (RectWidth(rcRect) - RectWidth(rcList)) / 2, rcRect.top));
	pList->SetPropertyVector(PROP_SCALE, CVector(SAVE_ENTRY_WIDTH, RectHeight(rcRect)));
	pList->SetPropertyMetric(PROP_VIEWPORT_HEIGHT, (Metric)RectHeight(rcRect));
	RegisterPerformanceEvent(pList, EVENT_ON_DOUBLE_CLICK, CMD_OK_SESSION);

	StartPerformance(pList, ID_LIST, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
	}

ALERROR CLoadGameSession::OnCommand (const CString &sCmd, void *pData)

//	OnCommand
//
//	Handle a command

	{
	if (strEquals(sCmd, CMD_CLOSE_SESSION))
		CmdCancel();
	else if (strEquals(sCmd, CMD_READ_COMPLETE))
		CmdReadComplete((CListSaveFilesTask *)pData);
	else if (strEquals(sCmd, CMD_OK_SESSION))
		CmdOK();
	else if (strEquals(sCmd, CMD_DELETE_FILE))
		CmdDeleteFile();

	return NOERROR;
	}

ALERROR CLoadGameSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	RECT rcRect;
	VI.GetWidescreenRect(&rcRect);

	//	Create a task to read the list of save files from disk

	m_HI.AddBackgroundTask(new CListSaveFilesTask(m_HI, m_Folders, m_Service.GetUsername(), m_bFilterPermadeath, SAVE_ENTRY_WIDTH), 0, this, CMD_READ_COMPLETE);

	//	Create the title and menu

	CUIHelper Helper(m_HI);
	TArray<CUIHelper::SMenuEntry> Menu;
	CUIHelper::SMenuEntry *pEntry = Menu.Insert();
	pEntry->sCommand = CMD_DELETE_FILE;
	pEntry->sLabel = CONSTLIT("Send game to recycle bin");

	IAnimatron *pTitle;
	Helper.CreateSessionTitle(this, m_Service, CONSTLIT("Load Game"), &Menu, CUIHelper::OPTION_SESSION_OK_BUTTON, &pTitle);
	StartPerformance(pTitle, ID_CTRL_TITLE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Disable the delete file button. We enable it again after we load.

	SetPropertyBool(CMD_DELETE_FILE, PROP_ENABLED, false);

	//	Create a wait animation

	IAnimatron *pAni;
	VI.CreateWaitAnimation(NULL, ID_CTRL_WAIT, rcRect, &pAni);
	StartPerformance(pAni, ID_CTRL_WAIT, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Done

	return NOERROR;
	}

void CLoadGameSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

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
			CmdOK();
			break;
		}
	}

void CLoadGameSession::OnPaint (CG32bitImage &Screen, const RECT &rcInvalid)

//	OnPaint
//
//	Paint

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	RECT rcCenter;
	VI.DrawSessionBackground(Screen, CG32bitImage(), CG32bitPixel(), CVisualPalette::OPTION_SESSION_DLG_BACKGROUND, &rcCenter);
	}

void CLoadGameSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash
//
//	Describe current state

	{
	*retsMessage = CONSTLIT("session: CLoadGameSession\r\n");
	}
