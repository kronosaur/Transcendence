//	Extensions.cpp
//
//	Implements extensions to the CodeChain interpreter that
//	are specific to the Library Link

#include "Alchemy.h"
#include "TerraFirma.h"

#define FN_INFO_GETMAP						1
#define FN_INFO_EDITORDATA					2

ICCItem *fnAddEntry (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnDeleteEntry (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnEditor (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnInfo (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnQuit (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnReadEntry (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnWriteEntry (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData);

static PRIMITIVEPROCDEF g_Extensions[] =
	{
		{	"libAddEntry",			fnAddEntry,		0,					"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"libDeleteEntry",		fnDeleteEntry,	0,					"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"libGetMap",			fnInfo,			FN_INFO_GETMAP,		"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"libReadEntry",			fnReadEntry,	0,					"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"libEditor",			fnEditor,		0,					"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"libEditorData",		fnInfo,			FN_INFO_EDITORDATA,	"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"libWriteEntry",		fnWriteEntry,	0,					"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"libQuit",				fnQuit,			0,					"",		"",		PPFLAG_SIDEEFFECTS,	},
	};

#define EXTENSIONS_COUNT		(sizeof(g_Extensions) / sizeof(g_Extensions[0]))

ALERROR CTerraFirma::RegisterExtensions (void)

//	RegisterExtensions
//
//	Register extensions

	{
	ALERROR error;
	int i;

	for (i = 0; i < EXTENSIONS_COUNT; i++)
		if (error = m_CC.RegisterPrimitive(&g_Extensions[i]))
			return error;

	return NOERROR;
	}

ICCItem *fnAddEntry (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnAddEntry
//
//	Add a new entry to the library
//
//	(libAddEntry entry) -> UNID

	{
	ALERROR error;
	CTerraFirma *pTF = (CTerraFirma *)pCtx->pExternalCtx;
	CCodeChain *pCC = pTF->GetCC();
	CLibraryFile *pLibrary = pTF->GetLibrary();
	ICCItem *pArgs;
	ICCItem *pEntry;
	ICCItem *pResult;
	int iUNID;

	//	The library better be open

	if (pLibrary == NULL)
		return pCC->CreateError(LITERAL("The library is not open."), NULL);

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, LITERAL("v"));
	if (pArgs->IsError())
		return pArgs;

	//	Get the entry

	pEntry = pArgs->Head(pCC);

	//	Add the entry to the library. We return either an error or
	//	the UNID at which the entry was made.

	error = pLibrary->CreateEntry(pEntry, &iUNID);
	if (error)
		pResult = pCC->CreateError(LITERAL("Unable to add new entry to the library."), NULL);
	else
		pResult = pCC->CreateInteger(iUNID);

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnDeleteEntry (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnDeleteEntry
//
//	Delete an entry
//
//	(libDeleteEntry UNID) -> True

	{
	ALERROR error;
	CTerraFirma *pTF = (CTerraFirma *)pCtx->pExternalCtx;
	CCodeChain *pCC = pTF->GetCC();
	CLibraryFile *pLibrary = pTF->GetLibrary();
	ICCItem *pArgs;
	ICCItem *pResult;
	int iUNID;

	//	The library better be open

	if (pLibrary == NULL)
		return pCC->CreateError(LITERAL("The library is not open."), NULL);

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, LITERAL("i"));
	if (pArgs->IsError())
		return pArgs;

	//	Get the UNID

	iUNID = pArgs->GetElement(0)->GetIntegerValue();

	//	Delete the entry

	error = pLibrary->DeleteEntry(iUNID);
	if (error)
		{
		CString sError = strPatternSubst(LITERAL("Unable to delete entry %d from the library."), iUNID);
		pResult = pCC->CreateError(sError, NULL);
		}
	else
		pResult = pCC->CreateTrue();

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnEditor (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnEditor
//
//	Shows the editor and loads it with data
//
//	(libEditor EditorID data)

	{
	CTerraFirma *pTF = (CTerraFirma *)pCtx->pExternalCtx;
	CCodeChain *pCC = pTF->GetCC();
	ICCItem *pResult;
	ICCItem *pArgs;
	DWORD dwEditorID;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, LITERAL("iv"));
	if (pArgs->IsError())
		return pArgs;

	//	Get the EditorID

	dwEditorID = (DWORD)pArgs->GetElement(0)->GetIntegerValue();

	//	Set the editor data

	pResult = pTF->EditData(dwEditorID, pArgs->GetElement(1));
	pArgs->Discard(pCC);
	if (pResult->IsError())
		return pCC->CreateError(LITERAL("Unable to create editor:"), pResult);

	//	Done

	return pResult;
	}

ICCItem *fnInfo (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnInfo
//
//	Returns global info

	{
	CTerraFirma *pTF = (CTerraFirma *)pCtx->pExternalCtx;
	CCodeChain *pCC = pTF->GetCC();
	ICCItem *pResult;

	switch (dwData)
		{
		case FN_INFO_GETMAP:
			{
			pResult = pTF->GetLibrary()->GetMap()->Reference();
			break;
			}

		case FN_INFO_EDITORDATA:
			{
			CNodeEditor *pEditor = pTF->GetNodeEditor();

			if (pEditor)
				pResult = pEditor->GetNodeData();
			else
				pResult = pCC->CreateError(LITERAL("Node Editor not available."), NULL);

			break;
			}

		default:
			ASSERT(FALSE);
			return NULL;
		}

	//	Done

	return pResult;
	}

ICCItem *fnQuit (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnQuit
//
//	Quits the application

	{
	CTerraFirma *pTF = (CTerraFirma *)pCtx->pExternalCtx;
	CCodeChain *pCC = pTF->GetCC();

	pTF->Quit();

	return pCC->CreateTrue();
	}

ICCItem *fnReadEntry (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnReadEntry
//
//	Reads an entry from the library
//
//	(libReadEntry UNID) -> entry

	{
	ALERROR error;
	CTerraFirma *pTF = (CTerraFirma *)pCtx->pExternalCtx;
	CCodeChain *pCC = pTF->GetCC();
	CLibraryFile *pLibrary = pTF->GetLibrary();
	ICCItem *pArgs;
	ICCItem *pEntry;
	ICCItem *pResult;
	int iUNID;

	//	The library better be open

	if (pLibrary == NULL)
		return pCC->CreateError(LITERAL("The library is not open."), NULL);

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, LITERAL("i"));
	if (pArgs->IsError())
		return pArgs;

	//	Get the UNID

	iUNID = pArgs->GetElement(0)->GetIntegerValue();

	//	Read the entry

	error = pLibrary->ReadEntry(iUNID, &pEntry);
	if (error)
		{
		CString sError = strPatternSubst(LITERAL("Unable to read entry %d from the library."), iUNID);
		pResult = pCC->CreateError(sError, NULL);
		}
	else
		pResult = pEntry;

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnWriteEntry (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnWriteEntry
//
//	Writes to an existing entry
//
//	(libWriteEntry UNID entry) -> UNID

	{
	ALERROR error;
	CTerraFirma *pTF = (CTerraFirma *)pCtx->pExternalCtx;
	CCodeChain *pCC = pTF->GetCC();
	CLibraryFile *pLibrary = pTF->GetLibrary();
	ICCItem *pArgs;
	ICCItem *pEntry;
	ICCItem *pResult;
	int iUNID;

	//	The library better be open

	if (pLibrary == NULL)
		return pCC->CreateError(LITERAL("The library is not open."), NULL);

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, LITERAL("iv"));
	if (pArgs->IsError())
		return pArgs;

	//	Get the UNID

	iUNID = pArgs->GetElement(0)->GetIntegerValue();

	//	Get the entry

	pEntry = pArgs->GetElement(1);

	//	Add the entry to the library. We return either an error or
	//	the UNID at which the entry was made.

	error = pLibrary->WriteEntry(iUNID, pEntry);
	if (error)
		{
		CString sError = strPatternSubst(LITERAL("Unable to write entry %d to the library."), iUNID);
		pResult = pCC->CreateError(sError, NULL);
		}
	else
		pResult = pCC->CreateInteger(iUNID);

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}
