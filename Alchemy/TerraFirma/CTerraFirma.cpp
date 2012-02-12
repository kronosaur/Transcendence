//	CTerraFirma.cpp
//
//	Implements the CTerraFirma class, which manages the
//	main application window

#include "Alchemy.h"
#include "TerraFirma.h"

#define TOOLBAR_WIDTH								80
#define NAVBAR_WIDTH								80

CTerraFirma::CTerraFirma (HWND hWnd) :
		m_hWnd(hWnd),
		m_pLibrary(NULL),
		m_pEditor(NULL),
		m_pWM(NULL)

//	CTerraFirma constructor

	{
	}

CTerraFirma::~CTerraFirma (void)

//	CTerraFirma destructor

	{
	CloseLibrary();
	}

ALERROR CTerraFirma::Boot (CString sLibrary)

//	Boot
//
//	Boots the object. This method must be called
//	before any other. sLibrary is the path and filename of the library
//	to open. If this is blank, then it opens the default library
//	(Library.tfl in the same directory as the app)

	{
	ALERROR error;
	RECT rcRect;

	//	Boot the CodeChain interpreter

	if (error = m_CC.Boot())
		return error;

	//	Register CodeChain extensions

	if (error = RegisterExtensions())
		return error;

	//	Register windows extensions

	if (error = RegisterWindowsExtensions())
		return error;

	//	Calculate some metrics

	GetClientRect(m_hWnd, &rcRect);
	CalcMetrics(RectWidth(&rcRect), RectHeight(&rcRect));

	//	Check to see if we're opening the default library. We must make sure
	//	the m_CC is initialized before opening a library

	if (sLibrary.IsBlank())
		{
		//	Figure out the path of the default library

		sLibrary = pathGetExecutablePath(GetInstance());
		sLibrary = pathAddComponent(sLibrary, LITERAL("Library.tfl"));

		//	Attempt to open the default library

		error = OpenLibrary(sLibrary);

		//	If we could not find the file, then create a new one

		if (error)
			{
			if (error == ERR_NOTFOUND)
				{
				if (error = CreateLibrary(sLibrary))
					return error;
				}
			else
				return error;
			}
		}
	else
		{
		if (error = OpenLibrary(sLibrary))
			return error;
		}

	//	If we have not created an editor, load the emergency
	//	command shell

	if (m_pEditor == NULL)
		{
		if (error = OpenEmergencyShell())
			return error;
		}

	return NOERROR;
	}

void CTerraFirma::CalcMetrics (int cxWidth, int cyHeight)

//	CalcMetrics
//
//	Calculates metrics for the TerraFirma window

	{
	m_rcNodeEditor.left = TOOLBAR_WIDTH;
	m_rcNodeEditor.top = 0;
	m_rcNodeEditor.right = cxWidth - NAVBAR_WIDTH;
	m_rcNodeEditor.bottom = cyHeight;
	}

void CTerraFirma::CloseEmergencyShell (void)

//	CloseEmergencyShell
//
//	Make sure the emergency shell is closed

	{
	if (m_CommandLineWnd.IsOpen())
		{
		//	Save the command line buffer

		m_CommandLineWnd.SaveBuffer(LITERAL("CommandLineBuffer"));

		//	Done

		m_CommandLineWnd.Destroy();
		}
	}

ALERROR CTerraFirma::CloseLibrary (void)

//	CloseLibrary
//
//	Close the library file

	{
	ALERROR error = NOERROR;

	if (m_pLibrary)
		{
		//	On shutdown we need to save our state

		if (error = ExecuteShutdownCode())
			return error;

		//	Close the library

		error = m_pLibrary->Close();
		delete m_pLibrary;
		m_pLibrary = NULL;
		}

	return error;
	}

ALERROR CTerraFirma::CreateLibrary (CString sFilename)

//	CreateLibrary
//
//	Creates a new blank library and opens it. Note that m_CC must be
//	initialized before this call.

	{
	ALERROR error;

	ASSERT(m_pLibrary == NULL);

	m_pLibrary = new CLibraryFile(&m_CC, sFilename);
	if (m_pLibrary == NULL)
		return ERR_MEMORY;

	//	Create it

	if (error = m_pLibrary->Create())
		{
		delete m_pLibrary;
		m_pLibrary = NULL;
		return error;
		}

	return NOERROR;
	}

ALERROR CTerraFirma::DisplayError (ALERROR error)

//	DisplayError
//
//	Displays an error message

	{
	return ErrorSetDisplayed(error);
	}

ICCItem *CTerraFirma::EditData (DWORD dwEditorID, ICCItem *pData)

//	EditData
//
//	Edit the given data in the appropriate editor. Note that we rely
//	on our caller to save any editor data.

	{
	ALERROR error;
	CNodeEditor *pNewEditor;
	ICCItem *pResult;

	//	If we've got an editor and it is the same type, then
	//	just set the data

	if (m_pEditor && m_pEditor->GetEditorID() == dwEditorID)
		return m_pEditor->SetNodeData(pData);

	//	Otherwise, we need to create a new editor

	switch (dwEditorID)
		{
		case NODEEDITOR_TYPE_COMMANDSHELL:
			pNewEditor = new CCommandShellEditor;
			break;

		case NODEEDITOR_TYPE_NODELIST:
			pNewEditor = new CNodeListEditor;
			break;

		default:
			return GetCC()->CreateError(LITERAL("Invalid editor ID."), NULL);
		}

	if (pNewEditor == NULL)
		return GetCC()->CreateMemoryError();

	//	Create the editor window

	if (error = pNewEditor->Create(this, m_hWnd, m_rcNodeEditor))
		{
		delete pNewEditor;
		return GetCC()->CreateSystemError(error);
		}

	//	Set the data

	pResult = pNewEditor->SetNodeData(pData);
	if (pResult->IsError())
		{
		pNewEditor->Destroy();
		delete pNewEditor;
		return pResult;
		}

	//	Delete the old editor

	if (m_pEditor)
		{
		m_pEditor->Destroy();
		delete m_pEditor;
		}

	//	Close the emergency shell (if open)

	CloseEmergencyShell();

	//	Done

	m_pEditor = pNewEditor;

	return GetCC()->CreateTrue();
	}

ICCItem *CTerraFirma::EditorNotification (int iNotification, ICCItem *pParam1, ICCItem *pParam2, ICCItem *pParam3)

//	EditorNotification
//
//	Calls a method for the current editor

	{
	ICCItem *pResult;
	ICCItem *pItem;

	//	Create $Editor

	pItem = GetCC()->CreateString(LITERAL("$Editor"));
	if (pItem->IsError())
		return pItem;

	//	Make the call

	pResult = ObjMethod(pItem, iNotification, pParam1, pParam2, pParam3);

	//	Done

	pItem->Discard(GetCC());
	return pResult;
	}

ALERROR CTerraFirma::ExecuteBootCode (void)

//	ExecuteBootCode
//
//	Executes the boot code of the currently open library

	{
	ALERROR error;
	ICCItem *pBootUNID;

	ASSERT(m_pLibrary);

	//	Look for the 'Boot' map entry

	pBootUNID = m_pLibrary->LoadMapEntry(LITERAL("boot"));

	//	If we found it, load the boot entry, otherwise, just continue

	if (!pBootUNID->IsError())
		{
		ICCItem *pBootCode;
		ICCItem *pResult;

		//	First we need to load the boot code

		error = m_pLibrary->ReadEntry(pBootUNID->GetIntegerValue(), &pBootCode);
		pBootUNID->Discard(&m_CC);
		if (error)
			return error;

		//	Now we evaluate it

		pResult = m_CC.TopLevel(pBootCode, this);
		pBootCode->Discard(&m_CC);

		//	If we got an error, abort

		if (pResult->IsError())
			{
			pResult->Discard(&m_CC);
			return ERR_FAIL;
			}

		//	Done

		pResult->Discard(&m_CC);
		}
	else
		{
		//	If we didn't find it, then ok, but if we got a different
		//	error then something is wrong

		if (pBootUNID->GetIntegerValue() != CCRESULT_NOTFOUND)
			{
			pBootUNID->Discard(&m_CC);
			return ERR_FAIL;
			}
		}

	return NOERROR;
	}

CString CTerraFirma::ExecuteCommand (CString sCommand)

//	ExecuteCommand
//
//	Evaluates a CodeChain expression and returns the result

	{
	CString sOutput;
	ICCItem *pInput;
	ICCItem *pOutput;

	//	Parse the input

	pInput = m_CC.Link(sCommand, 0, NULL);

	//	Evaluate

	pOutput = m_CC.TopLevel(pInput, this);
	pInput->Discard(&m_CC);

	//	Output result

	sOutput = m_CC.Unlink(pOutput);
	pOutput->Discard(&m_CC);

	return sOutput;
	}

ALERROR CTerraFirma::ExecuteShutdownCode (void)

//	ExecuteShutdownCode
//
//	Save our current state

	{
	ALERROR error;
	ICCItem *pShutdownUNID;

	ASSERT(m_pLibrary);

	//	Look for the 'Shutdown' map entry

	pShutdownUNID = m_pLibrary->LoadMapEntry(LITERAL("shutdown"));

	//	If we found it, load the shutdown entry, otherwise, just continue

	if (!pShutdownUNID->IsError())
		{
		ICCItem *pShutdownCode;
		ICCItem *pResult;

		//	First we need to load the Shutdown code

		error = m_pLibrary->ReadEntry(pShutdownUNID->GetIntegerValue(), &pShutdownCode);
		pShutdownUNID->Discard(&m_CC);
		if (error)
			return error;

		//	Now we evaluate it

		pResult = m_CC.TopLevel(pShutdownCode, this);
		pShutdownCode->Discard(&m_CC);

		//	If we got an error, abort

		if (pResult->IsError())
			{
			pResult->Discard(&m_CC);
			return ERR_FAIL;
			}

		//	Done

		pResult->Discard(&m_CC);
		}
	else
		{
		//	If we didn't find it, then ok, but if we got a different
		//	error then something is wrong

		if (pShutdownUNID->GetIntegerValue() != CCRESULT_NOTFOUND)
			{
			pShutdownUNID->Discard(&m_CC);
			return ERR_FAIL;
			}
		}

	return NOERROR;
	}

ICCItem *CTerraFirma::GetNodeTable (void)

//	GetNodeTable
//
//	Returns the $NodeTable variable

	{
	return m_CC.LookupGlobal(LITERAL("$NodeTable"), this);
	}

LONG CTerraFirma::HandleEditorNotification (int iNotifyCode)

//	HandleEditorNotification
//
//	Pass on a notification message from a child control to the
//	appropriate editor object

	{
	if (m_pEditor)
		return m_pEditor->HandleControlNotification(iNotifyCode);

	return 0;
	}

ICCItem *CTerraFirma::ObjMethod (ICCItem *pObj, int iMethod, ICCItem *pParam1, ICCItem *pParam2, ICCItem *pParam3)

//	ObjMethod
//
//	Calls an object method

	{
	ICCItem *pResult;
	CCLinkedList *pExp;
	ICCItem *pItem;

	//	Create the list that will represent the expression

	pResult = GetCC()->CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	pExp = (CCLinkedList *)pResult;

	//	We create an expression of the form:
	//		(objInvoke pObj iMethod pParam1 pParam2 pParam3)

	pItem = GetCC()->CreateString(LITERAL("objInvoke"));
	if (pItem->IsError())
		{
		pExp->Discard(GetCC());
		return pItem;
		}

	pExp->Append(GetCC(), pItem, &pResult);
	pItem->Discard(GetCC());
	if (pResult->IsError())
		{
		pExp->Discard(GetCC());
		return pResult;
		}

	pResult->Discard(GetCC());

	//	Now add pObj

	pExp->Append(GetCC(), pObj, &pResult);
	if (pResult->IsError())
		{
		pExp->Discard(GetCC());
		return pResult;
		}

	pResult->Discard(GetCC());

	//	Now add iMethod

	pItem = GetCC()->CreateInteger(iMethod);
	if (pItem->IsError())
		{
		pExp->Discard(GetCC());
		return pItem;
		}

	pExp->Append(GetCC(), pItem, &pResult);
	pItem->Discard(GetCC());
	if (pResult->IsError())
		{
		pExp->Discard(GetCC());
		return pResult;
		}

	pResult->Discard(GetCC());

	//	If necessary, add pParam1

	if (pParam1)
		{
		pExp->Append(GetCC(), pParam1, &pResult);
		if (pResult->IsError())
			{
			pExp->Discard(GetCC());
			return pResult;
			}

		pResult->Discard(GetCC());
		}

	//	If necessary, add pParam2

	if (pParam2)
		{
		pExp->Append(GetCC(), pParam2, &pResult);
		if (pResult->IsError())
			{
			pExp->Discard(GetCC());
			return pResult;
			}

		pResult->Discard(GetCC());
		}

	//	If necessary, add pParam3

	if (pParam3)
		{
		pExp->Append(GetCC(), pParam3, &pResult);
		if (pResult->IsError())
			{
			pExp->Discard(GetCC());
			return pResult;
			}

		pResult->Discard(GetCC());
		}

	//	Evaluate the expression

	pResult = GetCC()->TopLevel(pExp, this);
	pExp->Discard(GetCC());

	return pResult;
	}

ALERROR CTerraFirma::OpenEmergencyShell (void)

//	OpenEmergencyShell
//
//	Opens up a command shell in case no NodeEditor is loaded
//	on boot.

	{
	ALERROR error;

	ASSERT(m_pEditor == NULL);
	ASSERT(!m_CommandLineWnd.IsOpen());

	//	Create the command line window

	if (error = m_CommandLineWnd.Create(this, m_hWnd, m_rcNodeEditor))
		return error;

	//	Set the focus

	m_CommandLineWnd.SetFocus();

	//	Load the command line buffer

	m_CommandLineWnd.LoadBuffer(LITERAL("CommandLineBuffer"));

	return NOERROR;
	}

ALERROR CTerraFirma::OpenLibrary (CString sFilename)

//	OpenLibrary
//
//	Open the library file. Note that m_CC must be initialized before
//	this call.

	{
	ALERROR error;

	ASSERT(m_pLibrary == NULL);

	m_pLibrary = new CLibraryFile(&m_CC, sFilename);
	if (m_pLibrary == NULL)
		return ERR_MEMORY;

	//	Open it it

	if (error = m_pLibrary->Open())
		{
		delete m_pLibrary;
		m_pLibrary = NULL;
		return error;
		}

	//	Execute the boot code

	if (error = ExecuteBootCode())
		{
		delete m_pLibrary;
		m_pLibrary = NULL;
		return error;
		}

	return NOERROR;
	}

ALERROR CTerraFirma::Shutdown (void)

//	Shutdown
//
//	Shuts down the object. This method must be called before
//	deleting the object, if Boot has been called.

	{
	ALERROR error;

	//	Close down the emergency shell (if open)

	CloseEmergencyShell();

	//	Close the current library

	if (error = CloseLibrary())
		return error;

	return NOERROR;
	}
