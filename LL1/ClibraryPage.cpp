//	CLibraryPage.cpp
//
//	CLibraryPage object

#include "PreComp.h"

#define BACKBONE_ID							200
#define PAGE_ID								201
#define EDITOR_ID							202
#define LIBRARIAN_ID						203

#define BACKBONE_WIDTH						80
#if 0
#define COMMANDLINE_HEIGHT					40
#define SHELF_HEIGHT						48
#else
#define COMMANDLINE_HEIGHT					0
#define SHELF_HEIGHT						0
#endif

static CCommandBar::MenuItemDesc g_StdMenu[] =
	{
		{	CCommandBar::Contents,			0,	(ControllerNotifyProc)&CLibraryPage::CmdGotoContents,	},
		{	CCommandBar::Home,				0,	(ControllerNotifyProc)&CLibraryPage::CmdGotoHome,	},
		{	CCommandBar::NewEntry,			0,	(ControllerNotifyProc)&CLibraryPage::CmdNewEntry,	},
		{	CCommandBar::EditEntry,			0,	(ControllerNotifyProc)&CLibraryPage::CmdEditEntry,	},
		{	CCommandBar::InvokeLibrarian,	0,	(ControllerNotifyProc)&CLibraryPage::CmdInvokeLibrarian,	},
		{	CCommandBar::Index,				0,	(ControllerNotifyProc)&CLibraryPage::CmdGotoIndex,	},
		{	CCommandBar::Quit,				0,	(ControllerNotifyProc)&CLibraryPage::CmdQuit,	},
	};

static CCommandBar::MenuItemDesc g_EditMenu[] =
	{
		{	CCommandBar::Accept,			0,	(ControllerNotifyProc)&CLibraryPage::CmdSaveEdits,	},
		{	CCommandBar::Cancel,			0,	(ControllerNotifyProc)&CLibraryPage::CmdCancelEdits,},
	};

#define STDMENU_SIZE			(sizeof(g_StdMenu) / sizeof(g_StdMenu[0]))
#define EDITMENU_SIZE			(sizeof(g_EditMenu) / sizeof(g_EditMenu[0]))

CLibraryPage::CLibraryPage (CMainController *pController) :
		m_pController(pController),
		m_pCore(NULL),
		m_pEntry(NULL),
		m_pFrame(NULL),
		m_fEditMode(FALSE),
		m_fNewEntry(FALSE),
		m_pCmdLine(NULL),
		m_pEditor(NULL)

//	CLibraryPage constructor

	{
	}

CLibraryPage::~CLibraryPage (void)

//	CLibraryPage destructor

	{
	CloseCore();
	}

ALERROR CLibraryPage::Boot (CUFrame *pFrame)

//	Init
//
//	Initializes the page. This must be called before anything else

	{
	ALERROR error;
	AutoSizeDesc AutoSize;
	CUFrame *pBackboneFrame;
	CUFrame *pPageFrame;

	//	Remember our frame

	m_pFrame = pFrame;

	//	Create all our controls under this frame. Start with the
	//	backbone

	AutoSize.SetLeftPanel(BACKBONE_WIDTH, 0, 0);
	if (error = pFrame->GetWindow()->CreateFrame(pFrame, BACKBONE_ID, 0, &AutoSize, &pBackboneFrame))
		return error;

	pBackboneFrame->SetBackgroundColor(RGB(45, 53, 45));

	//	Create the command bar

	m_pCmdBar = new CCommandBar(this);
	if (m_pCmdBar == NULL)
		return ERR_MEMORY;

	if (error = pBackboneFrame->SetContent(m_pCmdBar, TRUE))
		return error;

	//	Now create the frame for the page

	AutoSize.SetAutoSize(0, 0, BACKBONE_WIDTH, SHELF_HEIGHT, 0, SHELF_HEIGHT, AutoSizeDesc::DimVariable, AutoSizeDesc::DimVariable);
	if (error = pFrame->GetWindow()->CreateFrame(pFrame, PAGE_ID, 0, &AutoSize, &pPageFrame))
		return error;

	pPageFrame->SetBackgroundColor(RGB(0, 0, 0));

	//	Create the page control

	m_pViewer = new CPageViewer(this);
	if (m_pViewer == NULL)
		return ERR_MEMORY;

	m_pViewer->SetActionMsg((ControllerNotifyProc)&CLibraryPage::CmdGotoEntry);

	if (error = pPageFrame->SetContent(m_pViewer, TRUE))
		return error;

	//	Set the focus

	pPageFrame->SetFocus();

	//	Set the menu for the command bar

	if (error = m_pCmdBar->SetMenuDesc(g_StdMenu, STDMENU_SIZE))
		return error;

	return NOERROR;
	}

ALERROR CLibraryPage::CloseCore (void)

//	CloseCore
//
//	Close the core database

	{
	ALERROR error;

	if (m_pCore)
		{
		//	Clear the current entry

		if (m_pEntry)
			{
			m_pViewer->SetPage(NULL);
			delete m_pEntry;
			m_pEntry = NULL;
			}

		//	Close the core

		error = m_pCore->Close();

		delete m_pCore;
		m_pCore = NULL;
		}

	return NOERROR;
	}

int CLibraryPage::CmdCancelEdits (DWORD dwTag, DWORD dwDummy2)

//	CmdCancelEdits
//
//	Cancels edit mode

	{
	ALERROR error;
	CUFrame *pEditFrame;

	//	Clean up edit mode

	pEditFrame = m_pFrame->GetWindow()->FindFrame(EDITOR_ID);
	if (pEditFrame)
		{
		m_pFrame->GetWindow()->DestroyFrame(pEditFrame);
		m_pEditor = NULL;
		}

	//	Setup the menu

	if (error = m_pCmdBar->SetMenuDesc(g_StdMenu, STDMENU_SIZE))
		goto Fail;

	//	View something

	if (m_EditUNID == UndefinedUNID)
		{
		if (error = ViewPageByUNID(AllEntriesUNID))
			goto Fail;
		}
	else
		{
		if (error = ViewPageByUNID(m_EditUNID))
			goto Fail;
		}

	m_fEditMode = FALSE;

	return 0;

Fail:

	DisplayError(error);
	return -1;
	}

int CLibraryPage::CmdDoCommand (DWORD dwTag, DWORD dwDummy2)

//	CmdDoCommand
//
//	Do the command in the command line

	{
	CString sCmd;

	//	Get the command from the command line

	sCmd = m_pCmdLine->GetCommand();

	//	Execute it

	//	Clear it

	m_pCmdLine->ClearCommand();

	return 0;
	}

int CLibraryPage::CmdEditEntry (DWORD dwTag, DWORD dwDummy2)

//	CmdEditEntry
//
//	Edits the current entry

	{
	ALERROR error;
	CString sData;

	ASSERT(!m_fEditMode);

	//	We must have an entry

	if (m_pEntry == NULL)
		return -1;

	//	Do not allow this if we are looking at a readonly entry

	if (m_pEntry->IsReadOnly())
		return -1;

	//	Create the edit control

	if (error = CreateEditor())
		goto Fail;

	//	Remember our UNID

	m_EditUNID = m_pEntry->GetUNID();

	//	Set the editor with the entry data

	m_pEditor->SetPage(m_pEntry);

	//	Clear the old display

	ViewPageByUNID(UndefinedUNID);

	//	Setup the menu

	if (error = m_pCmdBar->SetMenuDesc(g_EditMenu, EDITMENU_SIZE))
		goto Fail;

	//	In edit mode

	m_fEditMode = TRUE;
	m_fNewEntry = FALSE;

	return 0;

Fail:

	DisplayError(error);
	return -1;
	}

int CLibraryPage::CmdGotoContents (DWORD dwTag, DWORD dwDummy2)

//	CmdGotoContents
//
//	Show the contents page

	{
	ALERROR error;

	if (error = ViewPageByUNID(CategoriesUNID))
		goto Fail;

	return 0;

Fail:

	DisplayError(error);
	return -1;
	}

int CLibraryPage::CmdGotoEntry (DWORD dwTag, DWORD dwDummy2)

//	CmdGotoEntry
//
//	Show the given entry

	{
	ALERROR error;
	CString sLink;

	//	Get the link info from the page

	sLink = m_pViewer->GetCurrentLink();

	//	Navigate

	if (error = ViewPage(sLink))
		goto Fail;

	return 0;

Fail:

	DisplayError(error);
	return -1;
	}

int CLibraryPage::CmdGotoIndex (DWORD dwTag, DWORD dwDummy2)

//	CmdGotoIndex
//
//	Show the index page

	{
	ALERROR error;

	if (error = ViewPageByUNID(AllEntriesUNID))
		goto Fail;

	return 0;

Fail:

	DisplayError(error);
	return -1;
	}

int CLibraryPage::CmdGotoHome (DWORD dwTag, DWORD dwDummy2)

//	CmdGotoHome
//
//	Show the home page

	{
	ALERROR error;

	if (error = ViewPage(LITERAL("core://home")))
		goto Fail;

	return 0;

Fail:

	DisplayError(error);
	return -1;
	}

int CLibraryPage::CmdInvokeLibrarian (DWORD dwTag, DWORD dwDummy2)

//	CmdInvokeLibrarian
//
//	Show the librarian

	{
	ALERROR error;

	if (m_pCmdLine)
		HideLibrarian();
	else
		{
		if (error = ShowLibrarian())
			goto Fail;
		}

	return 0;

Fail:

	DisplayError(error);
	return -1;
	}

int CLibraryPage::CmdNewEntry (DWORD dwTag, DWORD dwDummy2)

//	CmdNewEntry
//
//	Creates a new entry

	{
	ALERROR error;

	//	Clean up if we're already in edit mode

	if (m_fEditMode)
		{
		}

	//	Create the edit control

	if (error = CreateEditor())
		goto Fail;

	//	Clear the old display

	ViewPageByUNID(UndefinedUNID);

	//	Set the editor with some default info

	{
	CEntry BlankEntry(m_pCore);
	BlankEntry.SetTitle(LITERAL("Untitled"));
	m_pEditor->SetPage(&BlankEntry);
	}

	//	Setup the menu

	if (error = m_pCmdBar->SetMenuDesc(g_EditMenu, EDITMENU_SIZE))
		goto Fail;

	//	In edit mode

	m_EditUNID = UndefinedUNID;
	m_fEditMode = TRUE;
	m_fNewEntry = TRUE;

	return 0;

Fail:

	DisplayError(error);
	return -1;
	}

int CLibraryPage::CmdQuit (DWORD dwTag, DWORD dwDummy2)

//	CmdQuit
//
//	Quits the app

	{
#ifdef LATER
	//	Do this right
#else
	//	For now we use a hack to send a quit message to the app

	(dynamic_cast<CMainController *>(m_pController))->GetApp()->Quit();

	return 0;
#endif
	}

int CLibraryPage::CmdSaveEdits (DWORD dwTag, DWORD dwDummy2)

//	CmdSaveEdits
//
//	Saves edits

	{
	ALERROR error;
	CString sData;
	CString sError;
	CEntry *pEntry;

	//	Get the text and parse it

	if (error = m_pEditor->GetPage(m_pCore, &pEntry))
		return -1;

	//	Save our changes

	if (m_fNewEntry)
		{
		if (error = m_pCore->CreateNewEntry(pEntry))
			goto Fail;

		m_EditUNID = pEntry->GetUNID();
		}
	else
		{
		pEntry->SetUNID(m_EditUNID);

		if (error = m_pCore->SaveEntry(pEntry))
			goto Fail;
		}

	//	Clean up

	return CmdCancelEdits(dwTag, dwDummy2);

Fail:

	DisplayError(error);
	return -1;
	}

ALERROR CLibraryPage::CreateEditor (void)

//	CreateEditor
//
//	Creates the page editor

	{
	ALERROR error;
	AutoSizeDesc AutoSize;
	CUFrame *pPageFrame;

	ASSERT(m_pEditor == NULL);

	//	Create the edit control

	AutoSize.SetAutoSize(0,
			0,
			BACKBONE_WIDTH,
			SHELF_HEIGHT + 2,
			0,
			SHELF_HEIGHT + 2,
			AutoSizeDesc::DimVariable, AutoSizeDesc::DimVariable);
	if (error = m_pFrame->GetWindow()->CreateFrame(m_pFrame, EDITOR_ID, 0, &AutoSize, &pPageFrame))
		return error;

	pPageFrame->SetBackgroundColor(RGB(192, 192, 192));

	m_pEditor = new CPageEditor(this);
	if (m_pEditor == NULL)
		return ERR_MEMORY;

	m_pEditor->SetCommandProcessor(m_pCore);

	if (error = pPageFrame->SetContent(m_pEditor, TRUE))
		return error;

	return NOERROR;
	}

ALERROR CLibraryPage::DoneWithEdit (void)

//	DoneWithEdit
//
//	Save changes

	{
	return NOERROR;
	}

ALERROR CLibraryPage::EditPage (ENTRYUNID dwUNID)

//	EditPage
//
//	Edits the given page

	{
	return NOERROR;
	}

void CLibraryPage::HideLibrarian (void)

//	HideLibrarian
//
//	Hide the librarian

	{
	if (m_pCmdLine)
		{
		CUFrame *pCmdFrame;

		pCmdFrame = m_pFrame->GetWindow()->FindFrame(LIBRARIAN_ID);
		if (pCmdFrame)
			{
			m_pFrame->GetWindow()->DestroyFrame(pCmdFrame);
			m_pCmdLine = NULL;
			}
		}
	}

ALERROR CLibraryPage::NewPage (void)

//	NewPage
//
//	Creates a new entry

	{
	return NOERROR;
	}

ALERROR CLibraryPage::OpenCore (CString sFilename)

//	OpenCore
//
//	Open a new core database

	{
	ALERROR error;
	CCore *pCore;

	//	Create a new core

	pCore = new CCore;
	if (pCore == NULL)
		return ERR_MEMORY;

	//	Try to open the file

	if (error = pCore->Open(sFilename))
		{
		//	If we could not find the file, create a new one

		if (error == ERR_NOTFOUND)
			{
			if ((error = CCore::Create(sFilename)) == NOERROR)
				error = pCore->Open(sFilename);
			}

		//	Return error

		if (error)
			{
			delete pCore;
			return error;
			}
		}

	//	Close any previous core and remember the new one

	CloseCore();
	m_pCore = pCore;

	//	Navigate to the home page

	if (error = ViewPage(LITERAL("core://home")))
		return error;

	return NOERROR;
	}

ALERROR CLibraryPage::ShowLibrarian (void)

//	ShowLibrarian
//
//	Creates the command line control

	{
	ALERROR error;

	if (m_pCmdLine == NULL)
		{
		AutoSizeDesc AutoSize;
		CUFrame *pFrame;

		AutoSize.SetAutoSize(0,
				COMMANDLINE_HEIGHT,
				BACKBONE_WIDTH,
				0,
				BACKBONE_WIDTH * 4,
				0,
				AutoSizeDesc::DimVariable, AutoSizeDesc::MinVariable);
		if (error = m_pFrame->GetWindow()->CreateFrame(m_pFrame, LIBRARIAN_ID, 0, &AutoSize, &pFrame))
			return error;

		pFrame->SetBackgroundColor(RGB(0, 0, 0));

		//	Create the command line

		m_pCmdLine = new CCommandLine(this);
		if (m_pCmdLine == NULL)
			return ERR_MEMORY;

		m_pCmdLine->SetActionMsg((ControllerNotifyProc)&CLibraryPage::CmdDoCommand);

		if (error = pFrame->SetContent(m_pCmdLine, TRUE))
			return error;

		pFrame->UpdateRect(NULL);
		pFrame->SetFocus();
		}

	return error;
	}

ALERROR CLibraryPage::ViewPage (CString sURL)

//	ViewPage
//
//	Navigates to the given page

	{
	ALERROR error;
	CEntry *pEntry;

	if (error = m_pCore->LoadEntry(sURL, &pEntry))
		return error;

	//	Set the viewer to show this entry

	if (error = m_pViewer->SetPage(pEntry))
		{
		if (pEntry)
			delete pEntry;
		return error;
		}

	//	Delete our previous entry

	if (m_pEntry)
		delete m_pEntry;

	m_pEntry = pEntry;

	return NOERROR;
	}

ALERROR CLibraryPage::ViewPageByUNID (ENTRYUNID dwUNID)

//	ViewPageByUNID
//
//	Shows the given page

	{
	ALERROR error;
	CEntry *pEntry;

	if (dwUNID == UndefinedUNID)
		pEntry = NULL;
	else
		{
		//	Load the new entry from the core

		if (error = m_pCore->LoadEntryByUNID(dwUNID, &pEntry))
			return error;
		}

	//	Set the viewer to show this entry

	if (error = m_pViewer->SetPage(pEntry))
		{
		if (pEntry)
			delete pEntry;
		return error;
		}

	//	Delete our previous entry

	if (m_pEntry)
		delete m_pEntry;

	m_pEntry = pEntry;

	return NOERROR;
	}
