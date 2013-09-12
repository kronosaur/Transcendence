//	pageLibrary.cpp
//
//	Implements the page library for CodeChain
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include <windows.h>
#include "Kernel.h"
#include "KernelObjID.h"
#include "CodeChain.h"
#include "Pages.h"

#define FN_PAGE_MAP						1
#define FN_PAGE_GET						2

ICCItem *fnPageGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);
ICCItem *fnPageMap (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

static PRIMITIVEPROCDEF g_Extensions[] =
	{
		{	"pageGet",						fnPageGet,			FN_PAGE_GET,
			"(pageGet address|pageID enumMethod [startIndex] [count]) -> list",
			"vv*",	0,	},

		{	"pageMap",					fnPageMap,		FN_PAGE_MAP,
			"(pageMap address|pageID enumMethod ['excludeNil] var exp) -> list",
			"vv*qu",	0,	},
	};

const int EXTENSIONS_COUNT = (sizeof(g_Extensions) / sizeof(g_Extensions[0]));

ICCItem *fnPageMap (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnPageMap
//
//	(pageMap address|pageID enumMethod ['excludeNil] var exp) -> filtered list

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	Args

	ICCItem *pAddress = pArgs->GetElement(0);
	ICCItem *pMethod = pArgs->GetElement(1);

	bool bExcludeNil;
	int iOptionalArg = 2;
	if (pArgs->GetCount() > 4)
		bExcludeNil = strEquals(pArgs->GetElement(iOptionalArg++)->GetStringValue(), CONSTLIT("excludeNil"));
	else
		bExcludeNil = false;

	ICCItem *pVar = pArgs->GetElement(iOptionalArg++);
	ICCItem *pBody = pArgs->GetElement(iOptionalArg++);

	//	Open the page

	IPage *pPage;
	CString sError;
	if (g_PM.OpenPage(pAddress->GetStringValue(), &pPage, &sError) != NOERROR)
		return pCC->CreateError(sError, NULL);

	//	Prepare the method

	SPageEnumCtx EnumCtx;
	if (pPage->EnumReset(*pCC, pMethod, EnumCtx, &sError))
		{
		g_PM.ClosePage(pPage);
		return pCC->CreateError(sError, NULL);
		}

	//	Create a destination list

	ICCItem *pResult = pCC->CreateLinkedList();
	if (pResult->IsError())
		{
		g_PM.ClosePage(pPage);
		return pResult;
		}

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	Setup the locals. We start by creating a local symbol table

	ICCItem *pLocalSymbols = pCC->CreateSymbolTable();
	if (pLocalSymbols->IsError())
		{
		pResult->Discard(pCC);
		g_PM.ClosePage(pPage);
		return pLocalSymbols;
		}

	//	Associate the enumaration variable

	ICCItem *pError = pLocalSymbols->AddEntry(pCC, pVar, pCC->CreateNil());
	if (pError->IsError())
		{
		pLocalSymbols->Discard(pCC);
		pResult->Discard(pCC);
		g_PM.ClosePage(pPage);
		return pError;
		}

	pError->Discard(pCC);

	//	Setup the context

	if (pEvalCtx->pLocalSymbols)
		pLocalSymbols->SetParent(pEvalCtx->pLocalSymbols);
	else
		pLocalSymbols->SetParent(pEvalCtx->pLexicalSymbols);
	ICCItem *pOldSymbols = pEvalCtx->pLocalSymbols;
	pEvalCtx->pLocalSymbols = pLocalSymbols;

	//	Get the offset of the variable so we don't have to
	//	search for it all the time

	int iVarOffset = pLocalSymbols->FindOffset(pCC, pVar);

	//	Enumerate the page

	while (pPage->EnumHasMore(EnumCtx))
		{
		ICCItem *pItem = pPage->EnumGetNext(*pCC, EnumCtx);
		if (pItem->IsError())
			{
			pResult->Discard(pCC);
			pResult = pItem;
			break;
			}

		//	Set the element

		pLocalSymbols->AddByOffset(pCC, iVarOffset, pItem);

		//	Eval

		ICCItem *pMapped = pCC->Eval(pEvalCtx, pBody);
		if (pMapped->IsError())
			{
			pItem->Discard(pCC);
			pResult->Discard(pCC);
			pResult = pMapped;
			break;
			}

		//	If the evaluation is not Nil, then we include the
		//	item in the result

		if (!bExcludeNil || !pMapped->IsNil())
			pList->Append(pCC, pMapped, NULL);

		pItem->Discard(pCC);
		pMapped->Discard(pCC);
		}

	//	Clean up

	pEvalCtx->pLocalSymbols = pOldSymbols;
	pLocalSymbols->Discard(pCC);
	g_PM.ClosePage(pPage);

	//	Done

	if (pResult->GetCount() > 0)
		return pResult;
	else
		{
		pResult->Discard(pCC);
		return pCC->CreateNil();
		}
	}

ICCItem *fnPageGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnPageGet
//
//	(pageGet address|pageID enumMethod [startIndex] [count]) -> list

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	Args

	ICCItem *pAddress = pArgs->GetElement(0);
	ICCItem *pMethod = pArgs->GetElement(1);
	int iStart = (pArgs->GetCount() > 2 ? pArgs->GetElement(2)->GetIntegerValue() : 0);
	int iCount = (pArgs->GetCount() > 3 ? pArgs->GetElement(3)->GetIntegerValue() : -1);

	//	Open the page

	IPage *pPage;
	CString sError;
	if (g_PM.OpenPage(pAddress->GetStringValue(), &pPage, &sError) != NOERROR)
		return pCC->CreateError(sError, NULL);

	//	Prepare the method

	SPageEnumCtx EnumCtx;
	if (pPage->EnumReset(*pCC, pMethod, EnumCtx, &sError))
		{
		g_PM.ClosePage(pPage);
		return pCC->CreateError(sError, NULL);
		}

	//	Create a destination list

	ICCItem *pResult = pCC->CreateLinkedList();
	if (pResult->IsError())
		{
		g_PM.ClosePage(pPage);
		return pResult;
		}

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	Enumerate the page

	while ((iCount == -1 || iCount > 0) && pPage->EnumHasMore(EnumCtx))
		{
		ICCItem *pItem = pPage->EnumGetNext(*pCC, EnumCtx);
		if (pItem->IsError())
			{
			pResult->Discard(pCC);
			pResult = pItem;
			break;
			}

		//	Add to list

		if (iStart == 0)
			{
			pList->Append(pCC, pItem, NULL);
			iCount--;
			}
		else
			iStart--;

		pItem->Discard(pCC);
		}

	//	Clean up

	g_PM.ClosePage(pPage);

	//	Done

	if (pResult->GetCount() > 0)
		return pResult;
	else
		{
		pResult->Discard(pCC);
		return pCC->CreateNil();
		}
	}

ALERROR pageLibraryInit (CCodeChain &CC)
	{
	ALERROR error;
	int i;

	for (i = 0; i < EXTENSIONS_COUNT; i++)
		if (error = CC.RegisterPrimitive(&g_Extensions[i]))
			return error;

	return NOERROR;
	}

