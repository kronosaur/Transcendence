//	CModExchangeSession.cpp
//
//	CModExchangeSession class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define CMD_CLOSE_SESSION						CONSTLIT("cmdCloseSession")
#define CMD_DISABLE_EXTENSION					CONSTLIT("cmdDisableExtension")
#define CMD_ENABLE_EXTENSION					CONSTLIT("cmdEnableExtension")
#define CMD_LOAD_COLLECTION						CONSTLIT("cmdLoadCollection")
#define CMD_OK_SESSION							CONSTLIT("cmdOKSession")
#define CMD_ON_SELECTION_CHANGED				CONSTLIT("cmdOnSelectionChanged")
#define CMD_REFRESH								CONSTLIT("cmdRefresh")
#define CMD_REFRESH_COMPLETE					CONSTLIT("cmdRefreshComplete")

#define CMD_SERVICE_EXTENSION_LOADED			CONSTLIT("serviceExtensionLoaded")

#define ID_CTRL_TITLE							CONSTLIT("ctrlTitle")
#define ID_CTRL_WAIT							CONSTLIT("ctrlWait")
#define ID_MESSAGE								CONSTLIT("idMessage")
#define ID_LIST									CONSTLIT("idList")

#define EVENT_ON_DOUBLE_CLICK					CONSTLIT("onDoubleClick")
#define EVENT_ON_SELECTION_CHANGED				CONSTLIT("onSelectionChanged")

#define PROP_COLOR								CONSTLIT("color")
#define PROP_ENABLED							CONSTLIT("enabled")
#define PROP_FONT								CONSTLIT("font")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_SELECTION_ID						CONSTLIT("selectionID")
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

bool CModExchangeSession::CanBeEnabledDisabled (const CMultiverseCatalogEntry *pCatalogEntry) const

//	CanBeEnabledDisabled
//
//	Returns TRUE if the given entry can be enabled/disabled.

	{
	if (pCatalogEntry == NULL)
		return false;

	//	Libraries cannot be disabled separately.

	if (pCatalogEntry->GetType() == extLibrary)
		return false;

	//	Core and Steam entries cannot be disabled.

	if (pCatalogEntry->GetLicenseType() == CMultiverseCatalogEntry::licenseCore
			|| pCatalogEntry->GetLicenseType() == CMultiverseCatalogEntry::licenseSteam
			|| pCatalogEntry->GetLicenseType() == CMultiverseCatalogEntry::licenseSteamUGC)
		return false;

	//	Otherwise, we can disable it.

	return true;
	}

void CModExchangeSession::CmdDisableExtension (void)

//	CmdDisableExtension
//
//	Disable the currently selected extension.

	{
	CMultiverseCatalogEntry CatalogEntry;
	if (!GetCurrentSelection(CatalogEntry))
		return;

	//	Tell our controller to enable the extension and then refresh

	m_HI.HICommand(CMD_DISABLE_EXTENSION, (void *)CatalogEntry.GetUNID());
	CmdRefresh(CListCollectionTask::FLAG_NO_COLLECTION_REFRESH);
	}

void CModExchangeSession::CmdDone (void)

//	CmdDone
//
//	Close the session.

	{
	m_HI.ClosePopupSession();
	}

void CModExchangeSession::CmdEnableExtension (void)

//	CmdEnableExtension
//
//	Enable the currently selected extension.

	{
	CMultiverseCatalogEntry CatalogEntry;
	if (!GetCurrentSelection(CatalogEntry))
		return;

	//	Tell our controller to enable the extension and then refresh

	m_HI.HICommand(CMD_ENABLE_EXTENSION, (void *)CatalogEntry.GetUNID());
	CmdReload();
	}

void CModExchangeSession::CmdOnSelectionChanged (void)

//	CmdOnSelectionChanged
//
//	The list selection has changed, so we need to update our menus.

	{
	CMultiverseCatalogEntry CatalogEntry;
	if (!GetCurrentSelection(CatalogEntry))
		return;

	IAnimatron *pRoot = GetPerformance(ID_CTRL_TITLE);
	if (pRoot == NULL)
		return;

	//	Update the menu based on the selection

	CUIHelper Helper(m_HI);
	Helper.RefreshMenu(this, pRoot, CreateMenu(&CatalogEntry));
	}

void CModExchangeSession::CmdRefresh (DWORD dwFlags)

//	CmdRefresh
//
//	Refresh the list

	{
	//	If we're already waiting for a refresh, then ignore

	if (m_bWaitingForRefresh)
		{
		m_bRefreshAgain = true;
		return;
		}

	//	Get metrics

	const CVisualPalette &VI = m_HI.GetVisuals();
	RECT rcCenter;
	VI.GetWidescreenRect(&rcCenter);

	//	Done with current list

	StopPerformance(ID_LIST);

	//	Create a task to read the list of save files from disk

	dwFlags |= (m_bDebugMode ? CListCollectionTask::FLAG_DEBUG_MODE : 0);

	m_bWaitingForRefresh = true;
	m_HI.AddBackgroundTask(new CListCollectionTask(m_HI, m_Extensions, m_Multiverse, m_Service, ENTRY_WIDTH, dwFlags), 0, this, CMD_REFRESH_COMPLETE);

	//	Create a wait animation

	IAnimatron *pAni;
	VI.CreateWaitAnimation(NULL, ID_CTRL_WAIT, rcCenter, &pAni);
	StartPerformance(pAni, ID_CTRL_WAIT, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
	}

void CModExchangeSession::CmdReload (void)

//	CmdReload
//
//	Reload the collection from the server and possibly download new extensions.

	{
	//	If we're already waiting for a refresh, then ignore

	if (m_bWaitingForRefresh)
		{
		m_bRefreshAgain = true;
		return;
		}

	//	Get metrics

	const CVisualPalette &VI = m_HI.GetVisuals();
	RECT rcCenter;
	VI.GetWidescreenRect(&rcCenter);

	//	Done with current list

	StopPerformance(ID_LIST);

	//	Request reload

	m_bWaitingForRefresh = true;
	m_HI.HICommand(CMD_LOAD_COLLECTION);

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
	VI.GetWidescreenRect(&rcRect);

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

	//	Start the list

	StartPerformance(pList, ID_LIST, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Register

	CmdOnSelectionChanged();
	RegisterPerformanceEvent(pList, EVENT_ON_SELECTION_CHANGED, CMD_ON_SELECTION_CHANGED);

	//	If we were asked to refresh while waiting, refresh again.

	if (m_bRefreshAgain)
		{
		m_bRefreshAgain = false;
		CmdRefresh(CListCollectionTask::FLAG_NO_COLLECTION_REFRESH);
		}
	}

#ifdef STEAM_BUILD

TArray<CUIHelper::SMenuEntry> CModExchangeSession::CreateMenu (CMultiverseCatalogEntry *pEntry)
	{
	return TArray<CUIHelper::SMenuEntry>();
	}

#else

TArray<CUIHelper::SMenuEntry> CModExchangeSession::CreateMenu (CMultiverseCatalogEntry *pCatalogEntry)

//	CreateMenu
//
//	Creates the menu for the given entry.

	{
	TArray<CUIHelper::SMenuEntry> Menu;

	CUIHelper::SMenuEntry *pEntry = Menu.Insert();
	pEntry->sCommand = CMD_REFRESH;
	pEntry->sLabel = CONSTLIT("Refresh");

	pEntry = Menu.Insert();
	pEntry->sCommand = CMD_DISABLE_EXTENSION;
	pEntry->sLabel = CONSTLIT("Disable Extension");
	if (!CanBeEnabledDisabled(pCatalogEntry) 
			|| pCatalogEntry->GetStatus() == CMultiverseCatalogEntry::statusPlayerDisabled)
		pEntry->dwFlags |= CUIHelper::MENU_HIDDEN;

	pEntry = Menu.Insert();
	pEntry->sCommand = CMD_ENABLE_EXTENSION;
	pEntry->sLabel = CONSTLIT("Enable Extension");
	if (!CanBeEnabledDisabled(pCatalogEntry) 
			|| pCatalogEntry->GetStatus() != CMultiverseCatalogEntry::statusPlayerDisabled)
		pEntry->dwFlags |= CUIHelper::MENU_HIDDEN;

	//	Done

	return Menu;
	}
#endif

bool CModExchangeSession::GetCurrentSelection (CMultiverseCatalogEntry &Entry) const

//	GetCurrentSelection
//
//	Returns the currently selected entry (or FALSE, if none is selected).

	{
	//	Get the selected extension

	IAnimatron *pList = GetElement(ID_LIST);
	if (pList == NULL)
		return false;

	CString sUNID = pList->GetPropertyString(PROP_SELECTION_ID);
	DWORD dwUNID = strToInt(sUNID, 0);
	if (dwUNID == 0)
		return false;

	return m_Multiverse.FindEntry(dwUNID, &Entry);
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
	else if (strEquals(sCmd, CMD_ON_SELECTION_CHANGED))
		CmdOnSelectionChanged();
	else if (strEquals(sCmd, CMD_REFRESH))
		CmdReload();
	else if (strEquals(sCmd, CMD_REFRESH_COMPLETE))
		CmdRefreshComplete((CListCollectionTask *)pData);
	else if (strEquals(sCmd, CMD_SERVICE_EXTENSION_LOADED))
		{
		if (m_bWaitingForRefresh)
			{
			StopPerformance(ID_CTRL_WAIT);
			m_bWaitingForRefresh = false;
			}
		CmdRefresh(CListCollectionTask::FLAG_NO_COLLECTION_REFRESH);
		}
	else if (strEquals(sCmd, CMD_DISABLE_EXTENSION))
		CmdDisableExtension();
	else if (strEquals(sCmd, CMD_ENABLE_EXTENSION))
		CmdEnableExtension();

	return NOERROR;
	}

ALERROR CModExchangeSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	RECT rcCenter;
	VI.GetWidescreenRect(&rcCenter);

	//	Create a task to read the list of save files from disk

	DWORD dwFlags = 0;
	dwFlags |= (m_bDebugMode ? CListCollectionTask::FLAG_DEBUG_MODE : 0);

	m_bWaitingForRefresh = true;
	m_HI.AddBackgroundTask(new CListCollectionTask(m_HI, m_Extensions, m_Multiverse, m_Service, ENTRY_WIDTH, dwFlags), 0, this, CMD_REFRESH_COMPLETE);

	//	Create the title and menu

	CUIHelper Helper(m_HI);
	DWORD dwOptions = 0;

#ifdef STEAM_BUILD
	dwOptions = CUIHelper::OPTION_SESSION_OK_BUTTON | CUIHelper::OPTION_SESSION_NO_CANCEL_BUTTON;
#else
	dwOptions = CUIHelper::OPTION_SESSION_ADD_EXTENSION_BUTTON;
#endif

	IAnimatron *pTitle;
	Helper.CreateSessionTitle(this, m_Service, CONSTLIT("Mod Collection"), &CreateMenu(), dwOptions, &pTitle);
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
	VI.DrawSessionBackground(Screen, CG32bitImage(), CG32bitPixel(), CVisualPalette::OPTION_SESSION_DLG_BACKGROUND, &rcCenter);
	}

void CModExchangeSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash
//
//	Describe current state

	{
	*retsMessage = CONSTLIT("session: CModExchangeSession\r\n");
	}
