//	Primitives.cpp
//
//	CodeChain primitives
//	Copyright (c) 2010 by George Moromisato

#include "stdafx.h"

#define FN_QUIT						1

ICCItem *fnShell (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

static PRIMITIVEPROCDEF g_Extensions[] =
	{
		{	"quit",						fnShell,	FN_QUIT,
			"(quit) -> [shell exits]",
			"",		PPFLAG_SIDEEFFECTS,	},
	};

#define EXTENSIONS_COUNT		(sizeof(g_Extensions) / sizeof(g_Extensions[0]))

ALERROR RegisterShellPrimitives (CCodeChain &CC)

//	RegisterShellPrimitives
//
//	Registers shell primitives

	{
	ALERROR error;
	int i;

	//	Register primitives

	for (i = 0; i < EXTENSIONS_COUNT; i++)
		if (error = CC.RegisterPrimitive(&g_Extensions[i]))
			return error;

	return NOERROR;
	}

ICCItem *fnShell (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)
	{
	CCodeChain *pCC = pEvalCtx->pCC;
	SExecuteCtx *pCtx = (SExecuteCtx *)pEvalCtx->pExternalCtx;

	switch (dwData)
		{
		case FN_QUIT:
			pCtx->bQuit = true;
			return pCC->CreateTrue();

		default:
			ASSERT(false);
			return NULL;
		}
	}
