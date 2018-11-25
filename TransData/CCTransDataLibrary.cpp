//	CCTransDataLibrary.cpp
//
//	Functions and placeholders defined by TransData
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

ICCItem *fnNil (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

static PRIMITIVEPROCDEF g_Library[] =
	{
		//	The following are placeholder functions that are normally 
		//	implemented in Transcendence (above TSE).

		{	"scrTranslate",					fnNil,		0,
			"PLACEHOLDER",
			"iv*",	0,	},
	};

static constexpr int FUNCTION_COUNT	=	((sizeof g_Library) / (sizeof g_Library[0]));

void GetCCTransDataLibrary (SPrimitiveDefTable *retpTable)

//	GetCCTransDataLibrary
//
//	Registers extensions

	{
	retpTable->pTable = g_Library;
	retpTable->iCount = FUNCTION_COUNT;
	}

ICCItem *fnNil (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnNil
//
//	Always returns Nil

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	return pCC->CreateNil();
	}

