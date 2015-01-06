//	dbgFunctions.cpp
//
//	Registers and implements debug functions
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

#if 0
#define FN_DEBUG_OUTPUT					1

ICCItem *fnDebug (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

static PRIMITIVEPROCDEF g_Extensions[] =
	{
		{	"dbgOutput",					fnDebug,			FN_DEBUG_OUTPUT,
			"(dbgOutput string [string]*)",
			"*",	0,	},
	};

const int EXTENSIONS_COUNT = (sizeof(g_Extensions) / sizeof(g_Extensions[0]));

ICCItem *fnDebug (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnDebug
//
//	(dbgOutput string [string]*)

	{
	int i;
	CCodeChain *pCC = pEvalCtx->pCC;

	switch (dwData)
		{
		case FN_DEBUG_OUTPUT:
			{
			CString sResult;

			//	Append each of the arguments together

			for (i = 0; i < pArgs->GetCount(); i++)
				{
				if (pArgs->GetElement(i)->IsList())
					sResult.Append(pCC->Unlink(pArgs->GetElement(i)));
				else
					sResult.Append(pArgs->GetElement(i)->Print(pCC, PRFLAG_NO_QUOTES));
				}

			//	Output to console

			printf("%s\n", sResult.GetASCIIZPointer());
			return pCC->CreateTrue();
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}
#endif

ALERROR dbgLibraryInit (CCodeChain &CC)
	{
#if 0
	ALERROR error;
	int i;

	for (i = 0; i < EXTENSIONS_COUNT; i++)
		if (error = CC.RegisterPrimitive(&g_Extensions[i]))
			return error;

#endif
	return NOERROR;
	}

