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
#define CMD_SHOW_HIDE_LIBRARIES					CONSTLIT("cmdShowHideLibraries")

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

#define URL_MULTIVERSE_CATALOG					CONSTLIT("http://multiverse.kronosaur.com/catalog.hexm")

#define ERR_DESC								CONSTLIT("Unable to retrive collection from Multiverse: %s")
#define ERR_TITLE								CONSTLIT("Unable to list collection")

const int ENTRY_WIDTH =							720;

CModExchangeSession::CModExchangeSession (CHumanInterface &HI, CCloudService &Service, CMultiverseModel &Multiverse, CExtensionCollection &Extensions, bool bDebugMode) : IHISession(HI), 
		m_Service(Service),
		m_Multiverse(Multiverse),
		m_Extensions(Extensions),
		m_bDebugMode(bDebugMode)

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
	CmdRefresh();
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

void CModExchangeSession::CmdRefresh (void)

//	CmdRefresh
//
//	Refresh the list

	{
	//	Done with current list

	DWORD dwSelect = GetCurrentSelectionUNID();
	StopPerformance(ID_LIST);

	//	Create a task to read the list of save files from disk

	StartListCollectionTask(dwSelect);

	//	Create a wait animation

	StartWaitAnimation();
	}

void CModExchangeSession::CmdReload (void)

//	CmdReload
//
//	Reload the collection from the server and possibly download new extensions.

	{
	//	Make sure we're in the correct state

	if (m_iState != stateNone)
		return;

	//	Done with current list

	m_dwSelect = GetCurrentSelectionUNID();
	StopPerformance(ID_LIST);

	//	Request reload

	m_iState = stateWaitingForReload;
	m_HI.HICommand(CMD_LOAD_COLLECTION);

	//	Create a wait animation

	StartWaitAnimation();
	}

void CModExchangeSession::CmdRefreshComplete (CListCollectionTask *pTask)

//	CmdRefreshComplete
//
//	Done with refresh

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	RECT rcRect;
	VI.GetWidescreenRect(&rcRect);

	//	Get the collection

	m_Collection = pTask->GetCollection();

	//	Done with wait animation

	StopPerformance(ID_CTRL_WAIT);
	m_iState = stateNone;

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

	//	Start the list

	StartPerformance(pList, ID_LIST, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Register

	CmdOnSelectionChanged();
	RegisterPerformanceEvent(pList, EVENT_ON_SELECTION_CHANGED, CMD_ON_SELECTION_CHANGED);

	//	If we were asked to refresh while waiting, refresh again.

	if (m_bRefreshAgain)
		{
		m_bRefreshAgain = false;
		CmdRefresh();
		}
	}

#ifdef STEAM_BUILD

TArray<CUIHelper::SMenuEntry> CModExchangeSession::CreateMenu (CMultiverseCatalogEntry *pEntry)
	{
	TArray<CUIHelper::SMenuEntry> Menu;

	CUIHelper::SMenuEntry *pEntry = Menu.Insert();
	pEntry->sCommand = CMD_SHOW_HIDE_LIBRARIES;
	pEntry->sLabel = (m_bShowLibraries ? CONSTLIT("Hide Libraries") : CONSTLIT("Show Libraries"));

	return Menu;
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
	pEntry->sCommand = CMD_SHOW_HIDE_LIBRARIES;
	pEntry->sLabel = (m_bShowLibraries ? CONSTLIT("Hide Libraries") : CONSTLIT("Show Libraries"));

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
	//	If we're in the wrong state, then we fail.

	if (m_iState != stateNone)
		return false;

	//	Get the selected extension

	IAnimatron *pList = GetElement(ID_LIST);
	if (pList == NULL)
		return false;

	CString sUNID = pList->GetPropertyString(PROP_SELECTION_ID);
	DWORD dwUNID = strToInt(sUNID, 0);
	if (dwUNID == 0)
		return false;

	//	Find it.

	for (int i = 0; i < m_Collection.GetCount(); i++)
		if (m_Collection[i].GetUNID() == dwUNID)
			{
			Entry = m_Collection[i];
			return true;
			}

	//	Otherwise, not found

	return false;
	}

DWORD CModExchangeSession::GetCurrentSelectionUNID (void) const

//	GetCurrentSelectionUNID
//
//	Returns the UNID of the current selection (or 0 if no selection).

	{
	//	If we're in the wrong state, then we fail.

	if (m_iState != stateNone)
		return 0;

	//	Get the selected extension

	IAnimatron *pList = GetElement(ID_LIST);
	if (pList == NULL)
		return 0;

	CString sUNID = pList->GetPropertyString(PROP_SELECTION_ID);
	return strToInt(sUNID, 0);
	}

void CModExchangeSession::OnCollectionUpdated (void)

//	OnCollectionUpdated
//
//	The collection has changed.

	{
	switch (m_iState)
		{
		//	If we're waiting for the list to load when the collection gets 
		//	updated, then we need to refresh again.

		case stateWaitingForList:
			m_bRefreshAgain = true;
			break;

		//	If we're waiting for the collection to reload, then this is the 
		//	sign that we need to load the list.

		case stateWaitingForReload:
			StartListCollectionTask(m_dwSelect);

			//	NOTE: We've already got a wait animation going, so we don't need
			//	to create a new one.
			break;

		//	Otherwise, we just refresh

		default:
			CmdRefresh();
			break;
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
	else if (strEquals(sCmd, CMD_ON_SELECTION_CHANGED))
		CmdOnSelectionChanged();
	else if (strEquals(sCmd, CMD_REFRESH))
		CmdReload();
	else if (strEquals(sCmd, CMD_REFRESH_COMPLETE))
		CmdRefreshComplete((CListCollectionTask *)pData);
	else if (strEquals(sCmd, CMD_SERVICE_EXTENSION_LOADED))
		OnCollectionUpdated();
	else if (strEquals(sCmd, CMD_DISABLE_EXTENSION))
		CmdDisableExtension();
	else if (strEquals(sCmd, CMD_ENABLE_EXTENSION))
		CmdEnableExtension();
	else if (strEquals(sCmd, CMD_SHOW_HIDE_LIBRARIES))
		{
		m_bShowLibraries = !m_bShowLibraries;
		CmdRefresh();
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
	VI.GetWidescreenRect(&rcCenter);

	//	Load an image for the generic extension icon (we need to do this before
	//	we start the list collection).

	HBITMAP hDIB;
	if (JPEGLoadFromResource(NULL,
			MAKEINTRESOURCE(IDR_GENERIC_EXTENSION_SMALL),
			JPEG_LFR_DIB, 
			NULL,
			&hDIB) == NOERROR)
		{
		m_pGenericIcon = TSharedPtr<CG32bitImage>(new CG32bitImage);
		if (!m_pGenericIcon->CreateFromBitmap(hDIB))
			m_pGenericIcon.Delete();

		::DeleteObject(hDIB);
		}

	//	Create a task to read the list of save files from disk

	StartListCollectionTask();

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

	StartWaitAnimation();

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

void CModExchangeSession::StartListCollectionTask (DWORD dwSelect)

//	StartListCollectionTask
//
//	Generate a list of mods in our collection. We do this in the background and 
//	get called at CMD_REFRESH_COMPLETE when done. Callers are responsible for
//	adding a wait animation and making sure we don't start a background task 
//	while a different task is running.

	{
	//	Get metrics

	const CVisualPalette &VI = m_HI.GetVisuals();
	RECT rcCenter;
	VI.GetWidescreenRect(&rcCenter);

	//	List rect

	CListCollectionTask::SOptions Options;
	Options.rcRect.left = rcCenter.left + (RectWidth(rcCenter) - ENTRY_WIDTH) / 2;
	Options.rcRect.top = rcCenter.top;
	Options.rcRect.right = Options.rcRect.left + ENTRY_WIDTH;
	Options.rcRect.bottom = Options.rcRect.top + RectHeight(rcCenter);

	//	Options

	Options.dwSelectUNID = dwSelect;
	Options.pGenericIcon = m_pGenericIcon;
	Options.bShowLibraries = m_bShowLibraries;

	m_iState = stateWaitingForList;
	m_HI.AddBackgroundTask(new CListCollectionTask(m_HI, m_Extensions, m_Multiverse, m_Service, Options), 0, this, CMD_REFRESH_COMPLETE);
	}

void CModExchangeSession::StartWaitAnimation (void)

//	StartWaitAnimation
//
//	Starts the spinning circle animation.

	{
	//	Get metrics

	const CVisualPalette &VI = m_HI.GetVisuals();
	RECT rcCenter;
	VI.GetWidescreenRect(&rcCenter);

	//	Create a wait animation

	IAnimatron *pAni;
	VI.CreateWaitAnimation(NULL, ID_CTRL_WAIT, rcCenter, &pAni);
	StartPerformance(pAni, ID_CTRL_WAIT, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
	}
