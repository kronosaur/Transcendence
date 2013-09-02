//	TSUICodeChain.cpp
//
//	Registers CodeChain primitives from TSUI.
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

#define FN_GET_MUSIC_CATALOG			0
#define FN_PLAY_MUSIC					1
#define FN_STOP_MUSIC					2
#define FN_CAN_PLAY_MUSIC				3
#define FN_GET_MUSIC_STATE				4

ICCItem *fnUI (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

static PRIMITIVEPROCDEF g_Primitives[] =
	{
		//	UI functions
		//	------------

		{	"uiCanPlayMusic",				fnUI,			FN_CAN_PLAY_MUSIC,
			"(uiCanPlayMusic filename) -> True/Nil",
			"s",	0,	},

		{	"uiGetMusicCatalog",			fnUI,			FN_GET_MUSIC_CATALOG,
			"(uiGetMusicCatalog) -> list of files",
			"",	0,	},

		{	"uiGetMusicState",				fnUI,			FN_GET_MUSIC_STATE,
			"(uiGetMusicState) -> ('playing filename position length)",
			"",	0,	},

		{	"uiPlayMusic",					fnUI,			FN_PLAY_MUSIC,
			"(uiPlayMusic filename [pos]) -> True/Nil",
			"s*",	0,	},

		{	"uiStopMusic",					fnUI,			FN_STOP_MUSIC,
			"(uiStopMusic)",
			"",	0,	},
	};

#define PRIMITIVE_COUNT		(sizeof(g_Primitives) / sizeof(g_Primitives[0]))

ALERROR CHumanInterface::InitCodeChainPrimitives (CCodeChain &CC)

//	InitCodeChainPrimitives
//
//	Registers extensions

	{
	ALERROR error;
	int i;

	for (i = 0; i < PRIMITIVE_COUNT; i++)
		if (error = CC.RegisterPrimitive(&g_Primitives[i]))
			return error;

	return NOERROR;
	}

ICCItem *fnUI (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnUI
//
//	UI functions

	{
	int i;
	CCodeChain *pCC = pEvalCtx->pCC;

	switch (dwData)
		{
		case FN_CAN_PLAY_MUSIC:
			{
			//	Get the filespec

			CString sFilespec = pathAddComponent(g_pHI->GetOptions().m_sMusicFolder, pArgs->GetElement(0)->GetStringValue());
			
			//	Can play it?

			return pCC->CreateBool(g_pHI->GetSoundMgr().CanPlayMusic(sFilespec));
			}

		case FN_GET_MUSIC_CATALOG:
			{
			//	Get the list of music

			TArray<CString> Catalog;
			g_pHI->GetSoundMgr().GetMusicCatalog(g_pHI->GetOptions().m_sMusicFolder, &Catalog);

			//	Create a list to return

			ICCItem *pResult = pCC->CreateLinkedList();
			if (pResult->IsError())
				return pResult;

			CCLinkedList *pList = (CCLinkedList *)pResult;

			//	Add to list

			for (i = 0; i < Catalog.GetCount(); i++)
				pList->AppendStringValue(pCC, pathMakeRelative(Catalog[i], g_pHI->GetOptions().m_sMusicFolder, true));

			return pResult;
			}

		case FN_GET_MUSIC_STATE:
			{
			//	Get the state

			//	Are we playing?

			CSoundMgr::SMusicPlayState State;
			if (!g_pHI->GetSoundMgr().GetMusicPlayState(&State))
				return pCC->CreateNil();

			//	Create a list to return

			ICCItem *pResult = pCC->CreateLinkedList();
			if (pResult->IsError())
				return pResult;

			CCLinkedList *pList = (CCLinkedList *)pResult;

			//	Add to list

			pList->AppendStringValue(pCC, CONSTLIT("playing"));
			pList->AppendStringValue(pCC, pathMakeRelative(State.sFilename, g_pHI->GetOptions().m_sMusicFolder, true));
			pList->AppendIntegerValue(pCC, State.iPos);
			pList->AppendIntegerValue(pCC, State.iLength);

			return pResult;
			}

		case FN_PLAY_MUSIC:
			{
			CString sTrack = pArgs->GetElement(0)->GetStringValue();

			//	Get the filespec

			CString sFilespec = pathAddComponent(g_pHI->GetOptions().m_sMusicFolder, sTrack);
			int iPos = (pArgs->GetCount() > 1 ? pArgs->GetElement(1)->GetIntegerValue() : 0);
			
			//	Play it

			CString sError;
			if (!g_pHI->GetSoundMgr().PlayMusic(sFilespec, iPos, &sError))
				return pCC->CreateError(sError);

			//	Done

			return pCC->CreateTrue();
			}

		case FN_STOP_MUSIC:
			{
			g_pHI->GetSoundMgr().StopMusic();
			return pCC->CreateTrue();
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}
