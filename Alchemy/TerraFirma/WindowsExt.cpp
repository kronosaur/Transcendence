//	WindowsExt.cpp
//
//	Implements extensions to the CodeChain interpreter that
//	are specific to windowing system used by the LibraryLink

#include "Alchemy.h"
#include "TerraFirma.h"
#include "WindowsExt.h"

#define FN_CREATEAREA_BUTTON				1
#define FN_CREATEAREA_IMAGE					2

#define FN_SETOPTION_HIDE					1
#define FN_SETOPTION_SHOW					2

#define FN_SETAREAOPTION_DATA				1

ICCItem *fnWndCreate (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnWndCreateArea (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnWndSetAreaOption (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnWndSetAreaSize (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnWndSetOption (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnWndSetSize (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData);

static PRIMITIVEPROCDEF g_Extensions[] =
	{
		{	"wndAreaSetData",		fnWndSetAreaOption,	FN_SETAREAOPTION_DATA,	"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"wndAreaSetSize",		fnWndSetAreaSize,	0,						"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"wndCreate",			fnWndCreate,		0,						"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"wndCreateButtonArea",	fnWndCreateArea,	FN_CREATEAREA_BUTTON,	"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"wndCreateImageArea",	fnWndCreateArea,	FN_CREATEAREA_IMAGE,	"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"wndHide",				fnWndSetOption,		FN_SETOPTION_HIDE,		"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"wndSetSize",			fnWndSetSize,		0,						"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"wndShow",				fnWndSetOption,		FN_SETOPTION_SHOW,		"",		"",		PPFLAG_SIDEEFFECTS,	},
	};

#define EXTENSIONS_COUNT		(sizeof(g_Extensions) / sizeof(g_Extensions[0]))

ALERROR CTerraFirma::RegisterWindowsExtensions (void)

//	RegisterWindowsExtensions
//
//	Register extensions

	{
	ALERROR error;
	int i;

	for (i = 0; i < EXTENSIONS_COUNT; i++)
		if (error = m_CC.RegisterPrimitive(&g_Extensions[i]))
			return error;

	//	Initialize the window manager

	ASSERT(m_pWM == NULL);
	m_pWM = new CLWindowManager(this);
	if (m_pWM == NULL)
		return ERR_MEMORY;

	return NOERROR;
	}

ICCItem *fnWndCreate (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnWndCreate
//
//	Creates a new window as a child of the Library Link
//
//	(wndCreate options size-rect size-options controller) -> CLWindow
//
//		options: Options for creating the window. The options are a list
//			of the following format:
//
//			(backstore)
//
//			backstore: If True, use a bitmap backstore for fast repaints
//				If Nil, do not.
//
//		size-rect: A rect to use for size:
//
//			(left top right bottom)
//
//			same as for (wndSetSize)
//
//		size-options: A list of size options
//
//			(horz-mode vert-mode)
//
//			same as for (wndSetSize)
//
//		controller: An object that will respond to notification from
//			the window.

	{
	CTerraFirma *pTF = (CTerraFirma *)pCtx->pExternalCtx;
	CCodeChain *pCC = pTF->GetCC();
	CLWindowManager *pWM = pTF->GetWM();
	ICCItem *pArgs;
	ICCItem *pResult;
	ICCItem *pOptions;
	ICCItem *pItem;
	CLWindowOptions Options;
	CLSizeOptions SizeOptions;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, LITERAL("llll"));
	if (pArgs->IsError())
		return pArgs;

	//	Get the options

	pOptions = pArgs->GetElement(0);
	pItem = pOptions->GetElement(0);
	if (pItem)
		Options.fBackstore = !pItem->IsNil();

	//	Get the autosize options

	if (!SizeOptions.ReadFromItem(pArgs->GetElement(1), pArgs->GetElement(2)))
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(LITERAL("Invalid RECT"), NULL);
		}

	//	Create the window

	pResult = pWM->CreateLWindow(&Options, &SizeOptions, pArgs->GetElement(3));
	pArgs->Discard(pCC);

	return pResult;
	}

ICCItem *fnWndCreateArea (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnWndCreateArea
//
//	Creates a new area as a child of the window
//
//	(wndCreateButtonArea window areaID size-rect size-options controller)
//	(wndCreateImageArea window areaID size-rect size-options controller)
//
//		window: The window to create the area in
//
//		areaID: The areaID to use for the area. This must be unique
//			within the window.
//
//		size-rect: A rect to use for size:
//
//			(left top right bottom)
//
//			same as for (wndSetSize)
//
//		size-options: A list of size options
//
//			(horz-mode vert-mode)
//
//			same as for (wndSetSize)
//
//		controller: An object that will respond to notification from
//			the area. If this is Nil, the notifications will be sent
//			to the window's controller

	{
	CTerraFirma *pTF = (CTerraFirma *)pCtx->pExternalCtx;
	CCodeChain *pCC = pTF->GetCC();
	ICCItem *pResult;
	ICCItem *pItem;
	CLWindow *pWindow;
	int iAreaID;
	ICCItem *pController;
	ICCItem *pArgs;
	CLSizeOptions SizeOptions;
	CLArea *pArea;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, LITERAL("villl"));
	if (pArgs->IsError())
		return pArgs;

	//	The first arg is the window

	pWindow = dynamic_cast<CLWindow *>(pArgs->GetElement(0));
	if (pWindow == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(LITERAL("Window expected"), NULL);
		}

	//	Next is the ID

	pItem = pArgs->GetElement(1);
	iAreaID = pItem->GetIntegerValue();

	//	The next args are the size options

	if (!SizeOptions.ReadFromItem(pArgs->GetElement(2), pArgs->GetElement(3)))
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(LITERAL("Invalid RECT"), NULL);
		}

	//	Next is the controller

	pController = pArgs->GetElement(4);

	//	Do it

	switch (dwData)
		{
		case FN_CREATEAREA_BUTTON:
			break;

		case FN_CREATEAREA_IMAGE:
			pArea = new CLImageArea(pWindow, iAreaID, pController);
			break;

		default:
			ASSERT(FALSE);
		}

	//	Make sure we're ok

	if (pArea == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateMemoryError();
		}

	//	Do the actual create

	pResult = pArea->Create(&SizeOptions);

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnWndSetAreaOption (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnWndSetAreaOption
//
//	Sets various boolean options
//
//	(wndAreaSetData window areaID data)

	{
	CTerraFirma *pTF = (CTerraFirma *)pCtx->pExternalCtx;
	CCodeChain *pCC = pTF->GetCC();
	ICCItem *pResult;
	ICCItem *pArgs;
	CLWindow *pWindow;
	CLArea *pArea;

	//	Evaluate the arguments and validate them (note that we do not
	//	evaluate the last paramenter because it is an object. Objects
	//	that get passed to ObjMethod should not be evaluated.)

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, LITERAL("viu"));
	if (pArgs->IsError())
		return pArgs;

	//	The first arg is the window

	pWindow = dynamic_cast<CLWindow *>(pArgs->GetElement(0));
	if (pWindow == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(LITERAL("Window expected"), NULL);
		}

	//	Next is the area ID

	pArea = pWindow->FindArea(pArgs->GetElement(1)->GetIntegerValue());
	if (pArea == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(LITERAL("Invalid Area ID"), NULL);
		}

	//	Next arg varies

	switch (dwData)
		{
		case FN_SETAREAOPTION_DATA:
			pResult = pArea->SetData(pArgs->GetElement(2));
			break;

		default:
			ASSERT(FALSE);
		}

	//	Done

	pArgs->Discard(pCC);

	return pResult;
	}

ICCItem *fnWndSetAreaSize (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnWndSetAreaSize
//
//	Sets the size of the area
//
//	(wndSetAreaSize window areaID (left top right bottom) options)
//
//		options: A list of auto-size options
//
//			(horz-mode vert-mode)
//
//				horz-mode: One of the following values:
//
//					0:	Variable Width. left is the left margin;
//						right is the right margin.
//
//					1:	Fixed Left. left is the left margin;
//						right is the width.
//
//					2:	Fixed Right. right is the right margin;
//						left is the width.
//
//				vert-mode: One of the following values:
//
//					0:	Variable Height. top is the top margin;
//						bottom is the bottom margin.
//
//					1:	Fixed Top. top is the top margin;
//						bottom is the height.
//
//					2:	Fixed Bottom. bottom is the bottom margin;
//						top is the height.

	{
	CTerraFirma *pTF = (CTerraFirma *)pCtx->pExternalCtx;
	CCodeChain *pCC = pTF->GetCC();
	ICCItem *pResult;

	return pResult;
	}

ICCItem *fnWndSetOption (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnWndSetOption
//
//	Sets various boolean options
//
//	(wndHide window)
//	(wndShow window)

	{
	CTerraFirma *pTF = (CTerraFirma *)pCtx->pExternalCtx;
	CCodeChain *pCC = pTF->GetCC();
	ICCItem *pResult;
	ICCItem *pArgs;
	CLWindow *pWindow;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, LITERAL("v"));
	if (pArgs->IsError())
		return pArgs;

	//	The first arg is the window

	pWindow = dynamic_cast<CLWindow *>(pArgs->GetElement(0));
	if (pWindow == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(LITERAL("Window expected"), NULL);
		}

	//	Do appropriate action

	switch (dwData)
		{
		case FN_SETOPTION_SHOW:
			pWindow->ShowWindow();
			pResult = pCC->CreateTrue();
			break;

		case FN_SETOPTION_HIDE:
			pWindow->HideWindow();
			pResult = pCC->CreateTrue();
			break;

		default:
			ASSERT(FALSE);
		}

	//	Done

	pArgs->Discard(pCC);

	return pResult;
	}

ICCItem *fnWndSetSize (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnWndSetSize
//
//	Sets the size of the window
//
//	(wndSetSize window (left top right bottom) options)
//
//		options: A list of auto-size options
//
//			(horz-mode vert-mode)
//
//				horz-mode: One of the following values:
//
//					0:	Variable Width. left is the left margin;
//						right is the right margin.
//
//					1:	Fixed Left. left is the left margin;
//						right is the width.
//
//					2:	Fixed Right. right is the right margin;
//						left is the width.
//
//				vert-mode: One of the following values:
//
//					0:	Variable Height. top is the top margin;
//						bottom is the bottom margin.
//
//					1:	Fixed Top. top is the top margin;
//						bottom is the height.
//
//					2:	Fixed Bottom. bottom is the bottom margin;
//						top is the height.

	{
	CTerraFirma *pTF = (CTerraFirma *)pCtx->pExternalCtx;
	CCodeChain *pCC = pTF->GetCC();
	ICCItem *pArgs;
	CLWindow *pWindow;
	CLSizeOptions SizeOptions;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, LITERAL("vll"));
	if (pArgs->IsError())
		return pArgs;

	//	The first arg is the window

	pWindow = dynamic_cast<CLWindow *>(pArgs->GetElement(0));
	if (pWindow == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(LITERAL("Window expected"), NULL);
		}

	//	Get the autosize options

	if (!SizeOptions.ReadFromItem(pArgs->GetElement(1), pArgs->GetElement(2)))
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(LITERAL("Invalid RECT"), NULL);
		}

	//	Set the window size

	pWindow->SetSize(&SizeOptions);

	//	Done

	pArgs->Discard(pCC);

	return pCC->CreateTrue();
	}

