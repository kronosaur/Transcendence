//	CodeChainExtensions.cpp
//
//	CodeChainExtensions

#include "PreComp.h"
#include "Transcendence.h"

#define FN_CNV_DRAW_RECT			1
#define FN_CNV_DRAW_IMAGE			2
#define FN_CNV_DRAW_TEXT			3
#define FN_CNV_DRAW_LINE			4

ICCItem *fnCanvas (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define FN_GAME_SET_CRAWL_TEXT		1
#define FN_GAME_SET_CRAWL_IMAGE		2
#define FN_GAME_SET_CRAWL_SOUNDTRACK	3
#define FN_GAME_SAVE				4
#define FN_GAME_END					5

ICCItem *fnGameSet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define	FN_SCR_GET_ITEM				1
#define FN_SCR_REMOVE_ITEM			2
#define FN_SCR_GET_ITEM_LIST_CURSOR	3
#define FN_SCR_REFRESH_ITEM			4

ICCItem *fnScrItem (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);

#define FN_PLY_CHARGE				1
#define FN_PLY_CREDITS				2
#define FN_PLY_DESTROYED			3
#define FN_PLY_MESSAGE				4
#define FN_PLY_INSURANCE_CLAIMS		5
#define FN_PLY_IS_INSURED			6
#define FN_PLY_INSURE				7
#define FN_PLY_INC_ORACUS			9
#define FN_PLY_COMPOSE_STRING		10
#define FN_PLY_REDIRECT_MESSAGE		11
#define FN_PLY_SHOW_HELP_REFUEL		12
#define FN_PLY_GENOME				13
#define FN_PLY_GET_STAT				14
#define FN_PLY_RECORD_BUY_ITEM		15
#define FN_PLY_RECORD_SELL_ITEM		16
#define FN_PLY_GET_ITEM_STAT		17
#define FN_PLY_CHANGE_SHIPS			18
#define FN_PLY_ENABLE_MESSAGE		19
#define FN_PLY_GET_KEY_EVENT_STAT	20
#define FN_PLY_USE_ITEM				21
#define FN_PLY_IS_MESSAGE_ENABLED	22

ICCItem *fnPlyGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);
ICCItem *fnPlyGetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnPlySet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);
ICCItem *fnPlySetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);

#define FN_SCR_SHOW_PANE			1
#define FN_SCR_DESC					2
#define FN_UNUSED1					3
#define FN_SCR_COUNTER				4
#define FN_SCR_EXIT_DOCK			5
#define FN_SCR_ENABLE_ACTION		6
#define FN_SCR_ACTION_LABEL			7
#define FN_SCR_LIST_FILTER			8
#define FN_SCR_IS_FIRST_ON_INIT		9
#define FN_SCR_INPUT_TEXT			10
#define FN_SCR_LIST_ENTRY			11
#define FN_SCR_SHOW_ACTION			12
#define FN_SCR_SET_DISPLAY_TEXT		13
#define FN_SCR_ADD_ACTION			14
#define FN_SCR_EXIT_SCREEN			15
#define FN_SCR_DATA					16
#define FN_SCR_REFRESH_SCREEN		17
#define FN_SCR_LIST_CURSOR			18
#define FN_SCR_CONTROL_VALUE		19
#define FN_SCR_TRANSLATE			20
#define FN_SCR_DESC_TRANSLATE		21
#define FN_SCR_REMOVE_ACTION		22
#define FN_SCR_ACTION_DESC			23
#define FN_SCR_IS_ACTION_ENABLED	24
#define FN_SCR_BACKGROUND_IMAGE		25

ICCItem *fnScrGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);
ICCItem *fnScrGetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnScrSet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);
ICCItem *fnScrSetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnScrShowScreen (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

ICCItem *fnPlyComposeString (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define ALIGN_CENTER				CONSTLIT("center")
#define ALIGN_RIGHT					CONSTLIT("right")
#define ALIGN_LEFT					CONSTLIT("left")

#define ACTION_SPECIAL_CANCEL		CONSTLIT("cancel")
#define ACTION_SPECIAL_DEFAULT		CONSTLIT("default")
#define ACTION_SPECIAL_NEXT_KEY		CONSTLIT("nextKey")
#define ACTION_SPECIAL_PREV_KEY		CONSTLIT("prevKey")

static PRIMITIVEPROCDEF g_Extensions[] =
	{
		//	Game function
		//	-------------

		{	"gamEnd",						fnGameSet,		FN_GAME_END,
			"(gamEnd endGameReason epitaph [scoreBonus]) -> True/Nil",

			"ss*",	PPFLAG_SIDEEFFECTS, },

		{	"gamSave",						fnGameSet,		FN_GAME_SAVE,
			"(gamSave [options]) -> True/Nil\n\n"
			
			"options:\n\n"
			
			"   'checkpoint (or Nil)\n"
			"   'missionCheckpoint\n",

			"*",	PPFLAG_SIDEEFFECTS, },

		{	"gamSetCrawlImage",				fnGameSet,		FN_GAME_SET_CRAWL_IMAGE,
			"(gamSetCrawlImage imageUNID) -> True/Nil",
			"i",	PPFLAG_SIDEEFFECTS, },

		{	"gamSetCrawlSoundtrack",		fnGameSet,		FN_GAME_SET_CRAWL_SOUNDTRACK,
			"(gamSetCrawlSoundtrack soundtrackUNID) -> True/Nil",
			"i",	PPFLAG_SIDEEFFECTS, },

		{	"gamSetCrawlText",				fnGameSet,		FN_GAME_SET_CRAWL_TEXT,
			"(gamSetCrawlText text) -> True/Nil",
			"s",	PPFLAG_SIDEEFFECTS, },

		//	Dock Screen functions
		//	---------------------

		{	"cnvDrawImage",					fnCanvas,			FN_CNV_DRAW_IMAGE,
			"(cnvDrawImage x y imageDesc [screen] [ID])",
			"iil*",	PPFLAG_SIDEEFFECTS,	},

		{	"cnvDrawLine",					fnCanvas,			FN_CNV_DRAW_LINE,
			"(cnvDrawLine xFrom yFrom xTo yTo width color [screen] [ID])",
			"iiiiiv*",	PPFLAG_SIDEEFFECTS,	},

		{	"cnvDrawRect",					fnCanvas,			FN_CNV_DRAW_RECT,
			"(cnvDrawRect x y width height color [screen] [ID])",
			"iiiiv*",	PPFLAG_SIDEEFFECTS,	},

		{	"cnvDrawText",					fnCanvas,			FN_CNV_DRAW_TEXT,
			"(cnvDrawText x y [width] text font color alignment [screen] [ID])",
			"ii*",	PPFLAG_SIDEEFFECTS,	},

		{	"scrAddAction",					fnScrSet,		FN_SCR_ADD_ACTION,
			"(scrAddAction screen actionID pos label [key] [special] code)",
			"isis*c",	PPFLAG_SIDEEFFECTS, },

		{	"scrEnableAction",				fnScrSet,		FN_SCR_ENABLE_ACTION,
			"(scrEnableAction screen actionID enabled)",
			"ivv",	PPFLAG_SIDEEFFECTS, },

		{	"scrExitScreen",				fnScrSet,		FN_SCR_EXIT_SCREEN,
			"(scrExitScreen screen ['forceUndock])",
			"i*",	PPFLAG_SIDEEFFECTS, },

		{	"scrGetCounter",				fnScrGetOld,		FN_SCR_COUNTER,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(scrGetCounter screen)

		{	"scrGetData",					fnScrGet,		FN_SCR_DATA,
			"(scrGetData screen attrib) -> data",
			"is",	0,	},

		{	"scrGetDesc",					fnScrGet,		FN_SCR_DESC,
			"(scrGetDesc screen)",
			"i",	0,	},

		{	"scrGetInputText",				fnScrGetOld,		FN_SCR_INPUT_TEXT,	"",		NULL,	PPFLAG_SIDEEFFECTS, },
		//	(scrGetInputText screen) => string

		{	"scrGetItem",					fnScrItem,		FN_SCR_GET_ITEM,	"",		NULL,	PPFLAG_SIDEEFFECTS, },
		//	(scrGetItem screen) => item

		{	"scrGetListCursor",				fnScrGet,		FN_SCR_LIST_CURSOR,
			"(scrGetListCursor screen) -> cursor",
			"i",	0,	},

		{	"scrGetListEntry",				fnScrGetOld,		FN_SCR_LIST_ENTRY,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(scrGetListEntry screen) -> entry

		{	"scrIsActionEnabled",			fnScrGet,		FN_SCR_IS_ACTION_ENABLED,	
			"(scrIsActionEnabled screen actionID) -> True/Nil",		
			"iv",	0, },

		{	"scrIsFirstOnInit",				fnScrGetOld,		FN_SCR_IS_FIRST_ON_INIT,	"",		NULL,	PPFLAG_SIDEEFFECTS, },
		//	(scrIsFirstOnInit screen) => True/Nil

		{	"scrRefreshScreen",				fnScrSet,		FN_SCR_REFRESH_SCREEN,
			"(scrRefreshScreen screen)",
			"i",	PPFLAG_SIDEEFFECTS, },

		{	"scrRemoveAction",				fnScrSet,		FN_SCR_REMOVE_ACTION,
			"(scrRemoveAction screen actionID)",
			"is",	PPFLAG_SIDEEFFECTS, },

		{	"scrRemoveItem",				fnScrItem,		FN_SCR_REMOVE_ITEM,	"",		NULL,	PPFLAG_SIDEEFFECTS, },
		//	(scrRemoveItem screen count) => item

		{	"scrSetActionDesc",				fnScrSet,		FN_SCR_ACTION_DESC,
			"(scrSetActionDesc screen actionID descID)",
			"ivv",		PPFLAG_SIDEEFFECTS, },

		{	"scrSetActionLabel",			fnScrSet,		FN_SCR_ACTION_LABEL,
			"(scrSetActionLabel screen actionID label [key] [special])",
			"ivs*",		PPFLAG_SIDEEFFECTS, },

		{	"scrSetBackgroundImage",		fnScrSet,		FN_SCR_BACKGROUND_IMAGE,
			"(scrSetBackgroundImage screen imageDesc)",
			"iv",		PPFLAG_SIDEEFFECTS, },

		{	"scrSetControlValue",			fnScrSet,		FN_SCR_CONTROL_VALUE,
			"(scrSetControlValue screen controlID value) -> True/Nil",
			"isv",		PPFLAG_SIDEEFFECTS, },

		{	"scrSetCounter",				fnScrSetOld,		FN_SCR_COUNTER,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(scrSetCounter screen counter)

		{	"scrSetData",					fnScrSet,		FN_SCR_DATA,
			"(scrSetData screen attrib data)",
			"isv",	PPFLAG_SIDEEFFECTS,	},

		{	"scrSetDesc",					fnScrSet,		FN_SCR_DESC,
			"(scrSetDesc screen text [text...])",
			"i*",	PPFLAG_SIDEEFFECTS,	},

		{	"scrSetDescTranslate",			fnScrSet,		FN_SCR_DESC_TRANSLATE,
			"(scrSetDescTranslate screen textID [data]) -> True/Nil",
			"is*",	PPFLAG_SIDEEFFECTS,	},

		{	"scrSetDisplayText",			fnScrSet,			FN_SCR_SET_DISPLAY_TEXT,
			"(scrSetDisplayText screen ID text [text...])",
			"iss*",	PPFLAG_SIDEEFFECTS,	},

		{	"scrSetInputText",				fnScrSetOld,		FN_SCR_INPUT_TEXT,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(scrSetInputText screen text)

		{	"scrSetListCursor",				fnScrSet,		FN_SCR_LIST_CURSOR,
			"(scrSetListCursor screen cursor)",
			"ii",	0,	},

		{	"scrSetListFilter",				fnScrSetOld,		FN_SCR_LIST_FILTER,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(scrSetListFilter screen filter)

		{	"scrShowAction",				fnScrSet,			FN_SCR_SHOW_ACTION,
			"(scrShowAction screen actionID shown)",
			"ivv",	PPFLAG_SIDEEFFECTS, },

		{	"scrShowPane",					fnScrSetOld,		FN_SCR_SHOW_PANE,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(scrShowPane screen pane)

		{	"scrShowScreen",				fnScrShowScreen,	0,
			"(scrShowScreen screenGlobal screen [pane] [data])",
			"vv*",	PPFLAG_SIDEEFFECTS,	},

		{	"scrTranslate",					fnScrGet,		FN_SCR_TRANSLATE,
			"(scrTranslate screen textID [data]) -> text or Nil",
			"is*",	0,	},

		//	Player functions
		//	----------------

		{	"plyChangeShip",				fnPlySet,		FN_PLY_CHANGE_SHIPS,
			"(plyChangeShip player newShip) -> True/Nil",
			"ii",	PPFLAG_SIDEEFFECTS,	},

		{	"plyCharge",					fnPlySet,		FN_PLY_CHARGE,
			"(plyCharge player [currency] charge) -> credits left",
			"iv*",	PPFLAG_SIDEEFFECTS,	},

		{	"plyComposeString",				fnPlyComposeString,		0,
			"(plyComposeString player string [arg1 arg2 ... argn]) -> string",
			"is*",	0,	},
		//	(plyComposeString player string)
		//		%name%				player name
		//		%he%				he or she
		//		%his%				his or her (matching case)
		//		%hers%				his or hers (matching case)
		//		%him%				him or her (matching case)
		//		%sir%				sir or ma'am (matching case)
		//		%man%				man or woman (matching case)
		//		%brother%			brother or sister (matching case)
		//		%son%				son or daughter (matching case)
		//		%%					%
		//		%1%					arg1
		//		%2%					...

		{	"plyCredit",					fnPlySet,	FN_PLY_CREDITS,
			"(plyCredit player [currency] credit) -> credits left",
			"iv*",	PPFLAG_SIDEEFFECTS,	},

		{	"plyDestroyed",					fnPlySetOld,		FN_PLY_DESTROYED, 
			"(plyDestroyed player epitaph)",
			NULL,	PPFLAG_SIDEEFFECTS, },

		{	"plyIsMessageEnabled",			fnPlyGet,			FN_PLY_IS_MESSAGE_ENABLED,
			"(plyIsMessageEnabled player messageID) -> True/Nil\n\n"

			"messageID:\n\n"
			
			"   'autopilotHint\n"
			"   'commsHint\n"
			"   'dockHint\n"
			"   'enableDeviceHint\n"
			"   'fireMissileHint\n"
			"   'galacticMapHint\n"
			"   'gateHint\n"
			"   'mapHint\n"
			"   'refuelHint\n"
			"   'switchMissileHint\n"
			"   'useItemHint\n",

			"is",	0, },

		{	"plyGetCredits",				fnPlyGet,		FN_PLY_CREDITS,
			"(plyGetCredits player [currency]) -> credits left",
			"i*",	0,	},

		{	"plyGetGenome",					fnPlyGet,			FN_PLY_GENOME,
			"(plyGetGenome player) -> 'humanMale | 'humanFemale",
			"i",	0,	},

		{	"plyGetItemStat",					fnPlyGet,			FN_PLY_GET_ITEM_STAT,
			"(plyGetItemStat player stat criteria|type) -> value\n\n"
			
			"stat:\n\n"
			
			"   'itemsBoughtCount\n"
			"   'itemsBoughtValue\n"
			"   'itemsDamagedHP\n"
			"   'itemsFiredCount\n"
			"   'itemsSoldCount\n"
			"   'itemsSoldValue\n",

			"isv",	0,	},

		{	"plyGetKeyEventStat",				fnPlyGet,			FN_PLY_GET_KEY_EVENT_STAT,
			"(plyGetKeyEventStat player stat nodeID typeCriteria) -> value\n\n"
			
			"stat:\n\n"
			
			"   'enemyObjsDestroyed\n"
			"   'friendlyObjsDestroyed\n"
			"   'missionCompleted\n"
			"   'missionFailure\n"
			"   'missionSuccess\n"
			"   'objsDestroyed\n",

			"isvs",	0,	},

		{	"plyEnableMessage",				fnPlySet,			FN_PLY_ENABLE_MESSAGE,
			"(plyEnableMessage player messageID True/Nil) -> True/Nil\n\n"
			
			"messageID:\n\n"
			
			"   'allHints\n"
			"   'allMessages\n"
			"   'enabledHints\n"
			"   (plus all messageIDs for plyIsMessageEnabled)\n",

			"isv",	PPFLAG_SIDEEFFECTS, },

		{	"plyGetRedirectMessage",		fnPlyGetOld,		FN_PLY_REDIRECT_MESSAGE,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(plyGetRedirectMessage player)

		{	"plyGetStat",					fnPlyGet,			FN_PLY_GET_STAT,
			"(plyGetStat player stat) -> value\n\n"
			
			"stat:\n\n"
			
			"   'bestEnemyShipDestroyed\n"
			"   'enemyShipsDestroyed\n"
			"   'enemyStationsDestroyed\n"
			"   'friendlyShipsDestroyed\n"
			"   'friendlyStationsDestroyed\n"
			"   'resurrectCount\n"
			"   'score\n"
			"   'systemData\n"
			"   'systemsVisited\n",

			"is",	0,	},

		{	"plyMessage",					fnPlySetOld,		FN_PLY_MESSAGE,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(plyMessage player message)

		{	"plyRecordBuyItem",				fnPlySet,			FN_PLY_RECORD_BUY_ITEM,
			"(plyRecordBuyItem player item [currency] totalPrice)",
			"iv*i",	0,	},

		{	"plyRecordSellItem",			fnPlySet,			FN_PLY_RECORD_SELL_ITEM,
			"(plyRecordSellItem player item [currency] totalPrice)",
			"iv*i",	0,	},

		{	"plyRedirectMessage",			fnPlySetOld,		FN_PLY_REDIRECT_MESSAGE,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(plyRedirectMessage player True/Nil)

		{	"plyUseItem",					fnPlySet,			FN_PLY_USE_ITEM,
			"(plyUseItem player item)",
			"iv",	PPFLAG_SIDEEFFECTS,	},

		//	Deprecated functions
		//	--------------------

		{	"scrExitDock",					fnScrSet,		FN_SCR_EXIT_DOCK,
			"DEPRECATED: Use scrExitScreen instead.",
			"i",	PPFLAG_SIDEEFFECTS, },

		{	"plyClearShowHelpRefuel",		fnPlyGetOld,		FN_PLY_SHOW_HELP_REFUEL,
			"DEPRECATED: Use (plyEnableMessage player 'refuelHint Nil) instead",		NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"plyGetInsuranceClaims",		fnPlyGetOld,		FN_PLY_INSURANCE_CLAIMS,
			"DEPRECATED: Use resurrection infrastructure instead.",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(plyGetInsuranceClaims player) -> No of insurance claims

		{	"plyInsure",					fnPlyGetOld,		FN_PLY_INSURE,
			"DEPRECATED: Use resurrection infrastructure instead.",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(plyInsure player)

		{	"plyIsInsured",					fnPlyGetOld,		FN_PLY_IS_INSURED,
			"DEPRECATED: Use resurrection infrastructure instead.",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(plyIsInsured player) -> True if insured

		{	"scrGetItemListCursor",			fnScrItem,		FN_SCR_GET_ITEM_LIST_CURSOR,
			"DEPRECATED: Use scrGetItem instead",		NULL,	PPFLAG_SIDEEFFECTS, },

		{	"scrRefreshItemListCursor",		fnScrItem,		FN_SCR_REFRESH_ITEM,	
			"DEPRECATED",		NULL,	PPFLAG_SIDEEFFECTS, },
		//	(scrRefreshItemListCursor screen item)

	};

#define EXTENSIONS_COUNT		(sizeof(g_Extensions) / sizeof(g_Extensions[0]))

inline CShip *GetShipArg (ICCItem *pArg) { return ((CSpaceObject *)(pArg->GetIntegerValue()))->AsShip(); }
inline CStation *GetStationArg (ICCItem *pArg) { return (CStation *)pArg->GetIntegerValue(); }
inline CDockScreen *GetDockScreenArg (ICCItem *pArg) { return (CDockScreen *)pArg->GetIntegerValue(); }
inline CArmorClass *GetArmorClassArg (ICCItem *pArg) { return (CArmorClass *)pArg->GetIntegerValue(); }
inline CPlayerShipController *GetPlayerArg (ICCItem *pArg) { return (CPlayerShipController *)pArg->GetIntegerValue(); }

CG32bitImage *GetCanvasArg (CEvalContext *pEvalCtx, ICCItem *pArgs, int iArg)

//	GetCanvasArg
//
//	Returns the canvas to use (NULL if not found)

	{
	//	If we have enough arguments, then use the given canvas

	if (pArgs->GetCount() > iArg + 1)
		{
		CDockScreen *pScreen = (CDockScreen *)pArgs->GetElement(iArg)->GetIntegerValue();
		if (pScreen == NULL)
			return NULL;

		return pScreen->GetDisplayCanvas(pArgs->GetElement(iArg+1)->GetStringValue());
		}

	//	Otherwise, use the context

	else
		{
		CCodeChainCtx *pCtx = (CCodeChainCtx *)pEvalCtx->pExternalCtx;
		return pCtx->GetCanvas();
		}
	}

CG32bitPixel GetColorArg (ICCItem *pArg)

//	GetColorArg
//
//	Returns color

	{
	if (pArg->IsInteger())
		return CG32bitPixel::FromDWORD((DWORD)pArg->GetIntegerValue());
	else if (pArg->GetCount() == 3)
		return CG32bitPixel(pArg->GetElement(0)->GetIntegerValue(),
				pArg->GetElement(1)->GetIntegerValue(),
				pArg->GetElement(2)->GetIntegerValue());
	else
		return CG32bitPixel::Null();
	}

void GetCodeChainExtensions (SPrimitiveDefTable *retpTable)

//	GetCodeChainExtensions
//
//	Registers extensions

	{
	retpTable->pTable = g_Extensions;
	retpTable->iCount = EXTENSIONS_COUNT;
	}

ICCItem *fnCanvas (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnCanvas
//
//	Canvas functions

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	switch (dwData)
		{
		case FN_CNV_DRAW_IMAGE:
			{
			int x = pArgs->GetElement(0)->GetIntegerValue();
			int y = pArgs->GetElement(1)->GetIntegerValue();
			CG32bitImage *pImage;
			RECT rcImage;
			GetImageDescFromList(*pCC, pArgs->GetElement(2), &pImage, &rcImage);
			if (pImage == NULL)
				return pCC->CreateNil();
			CG32bitImage *pCanvas = GetCanvasArg(pEvalCtx, pArgs, 3);
			if (pCanvas == NULL)
				return pCC->CreateNil();

			pCanvas->Blt(rcImage.left,
					rcImage.top,
					RectWidth(rcImage),
					RectHeight(rcImage),
					255,
					*pImage,
					x,
					y);

			return pCC->CreateTrue();
			}

		case FN_CNV_DRAW_LINE:
			{
			int xFrom = pArgs->GetElement(0)->GetIntegerValue();
			int yFrom = pArgs->GetElement(1)->GetIntegerValue();
			int xTo = pArgs->GetElement(2)->GetIntegerValue();
			int yTo = pArgs->GetElement(3)->GetIntegerValue();
			int iWidth = pArgs->GetElement(4)->GetIntegerValue();
			CG32bitPixel rgbColor = GetColorArg(pArgs->GetElement(5));
			CG32bitImage *pCanvas = GetCanvasArg(pEvalCtx, pArgs, 6);
			if (pCanvas == NULL)
				return pCC->CreateNil();

			if (iWidth <= 0)
				return pCC->CreateNil();

			CGDraw::LineHD(*pCanvas, xFrom, yFrom, xTo, yTo, iWidth, rgbColor);
			return pCC->CreateTrue();
			}

		case FN_CNV_DRAW_RECT:
			{
			int x = pArgs->GetElement(0)->GetIntegerValue();
			int y = pArgs->GetElement(1)->GetIntegerValue();
			int cxWidth = pArgs->GetElement(2)->GetIntegerValue();
			int cyHeight = pArgs->GetElement(3)->GetIntegerValue();
			CG32bitPixel rgbColor = GetColorArg(pArgs->GetElement(4));
			CG32bitImage *pCanvas = GetCanvasArg(pEvalCtx, pArgs, 5);
			if (pCanvas == NULL)
				return pCC->CreateNil();

			pCanvas->Fill(x, y, cxWidth, cyHeight, rgbColor);

			return pCC->CreateTrue();
			}

		case FN_CNV_DRAW_TEXT:
			{
			int iArg = 0;
			if (pArgs->GetCount() < 6)
				return pCC->CreateError(CONSTLIT("Insufficient arguments for cnvDrawText."));

			if (pArgs->GetElement(2)->IsInteger() && pArgs->GetCount() < 7)
				return pCC->CreateError(CONSTLIT("Insufficient arguments for cnvDrawText."));

			int x = pArgs->GetElement(iArg++)->GetIntegerValue();
			int y = pArgs->GetElement(iArg++)->GetIntegerValue();
			int cxWidth = (pArgs->GetElement(iArg)->IsInteger() ? pArgs->GetElement(iArg++)->GetIntegerValue() : -1);
			CString sText = g_pTrans->ComposePlayerNameString(pArgs->GetElement(iArg++)->GetStringValue());
			const CG16bitFont *pControlFont = &GetFontByName(g_pTrans->GetFonts(), pArgs->GetElement(iArg++)->GetStringValue());
			CG32bitPixel rgbColor = GetColorArg(pArgs->GetElement(iArg++));
			CString sAlign = pArgs->GetElement(iArg++)->GetStringValue();

			CG32bitImage *pCanvas = GetCanvasArg(pEvalCtx, pArgs, iArg);
			if (pCanvas == NULL)
				return pCC->CreateNil();

			//	Compute flags and rect

			DWORD dwFlags = 0;
			RECT rcRect;
			rcRect.top = y;
			rcRect.bottom = pCanvas->GetHeight();

			if (strEquals(sAlign, ALIGN_CENTER))
				{
				dwFlags |= CG16bitFont::AlignCenter;

				int cxHalfWidth;
				if (cxWidth == -1)
					cxHalfWidth = Min(x, (pCanvas->GetWidth() - x));
				else
					cxHalfWidth = cxWidth / 2;

				rcRect.left = x - cxHalfWidth;
				rcRect.right = x + cxHalfWidth;
				}
			else if (strEquals(sAlign, ALIGN_RIGHT))
				{
				dwFlags |= CG16bitFont::AlignRight;

				if (cxWidth == -1)
					cxWidth = x;

				rcRect.left = (x - cxWidth);
				rcRect.right = x;
				}
			else
				{
				if (cxWidth == -1)
					cxWidth = (pCanvas->GetWidth() - x);

				rcRect.left = x;
				rcRect.right = x + cxWidth;
				}

			//	Paint

			pControlFont->DrawText(*pCanvas, rcRect, rgbColor, sText, 0, dwFlags);

			//	Done

			return pCC->CreateTrue();
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnGameSet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnGameSet
//
//	Game functions

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	switch (dwData)
		{
		case FN_GAME_END:
			{
			CString sReason = pArgs->GetElement(0)->GetStringValue();
			CString sEpitaph = pArgs->GetElement(1)->GetStringValue();
			int iScoreBonus = (pArgs->GetCount() >= 3 ? pArgs->GetElement(2)->GetIntegerValue() : 0);

			if (g_pTrans->GetModel().EndGame(sReason, sEpitaph, iScoreBonus) != NOERROR)
				{
				::kernelDebugLogMessage("Unable to end game.");
				return pCC->CreateNil();
				}

			g_pTrans->PlayerEndGame();
			return pCC->CreateTrue();
			}

		case FN_GAME_SAVE:
			{
			CString sOption = (pArgs->GetCount() > 0 ? pArgs->GetElement(0)->GetStringValue() : NULL_STR);
			DWORD dwFlags;
			if (sOption.IsBlank() || strEquals(sOption, CONSTLIT("checkpoint")))
				dwFlags = CGameFile::FLAG_CHECKPOINT;
			else if (strEquals(sOption, CONSTLIT("missionCheckpoint")))
				dwFlags = CGameFile::FLAG_CHECKPOINT | CGameFile::FLAG_ACCEPT_MISSION;
			else
				return pCC->CreateError(CONSTLIT("Invalid option"), pArgs->GetElement(0));

			CString sError;
			if (g_pTrans->GetModel().SaveGame(dwFlags, &sError) != NOERROR)
				{
				::kernelDebugLogMessage("Unable to save game: %s", sError);
				return pCC->CreateNil();
				}

			return pCC->CreateTrue();
			}

		case FN_GAME_SET_CRAWL_IMAGE:
			g_pTrans->GetModel().SetCrawlImage((DWORD)pArgs->GetElement(0)->GetIntegerValue());
			return pCC->CreateTrue();

		case FN_GAME_SET_CRAWL_SOUNDTRACK:
			g_pTrans->GetModel().SetCrawlSoundtrack((DWORD)pArgs->GetElement(0)->GetIntegerValue());
			return pCC->CreateTrue();

		case FN_GAME_SET_CRAWL_TEXT:
			g_pTrans->GetModel().SetCrawlText(pArgs->GetElement(0)->GetStringValue());
			return pCC->CreateTrue();

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnScrItem (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnScrItem
//
//	Gets and sets items
//
//	(scrGetItem screen) => itemStruct
//	(scrRemoveItem screen count) => itemStruct

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	if (dwData == FN_SCR_REMOVE_ITEM)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("ii"));
	else if (dwData == FN_SCR_REFRESH_ITEM)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("il"));
	else
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("i"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a dock screen object

	CDockScreen *pScreen = GetDockScreenArg(pArgs->GetElement(0));
	if (pScreen == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(LITERAL("Entry pointer expected"), pArgs->GetElement(0));
		}

	//	Do the appropriate command

	CItem Result;
	switch (dwData)
		{
		case FN_SCR_GET_ITEM:
		case FN_SCR_GET_ITEM_LIST_CURSOR:
			if (pScreen->IsCurrentItemValid())
				{
				Result = pScreen->GetCurrentItem();
				pResult = CreateListFromItem(*pCC, Result);
				}
			else
				pResult = pCC->CreateNil();

			pArgs->Discard(pCC);
			break;

		case FN_SCR_REMOVE_ITEM:
			{
			if (pScreen->IsCurrentItemValid())
				{
				int iToDelete = pArgs->GetElement(1)->GetIntegerValue();

				Result = pScreen->GetCurrentItem();
				iToDelete = min(iToDelete, Result.GetCount());
				pScreen->DeleteCurrentItem(iToDelete);
				Result.SetCount(iToDelete);
				pResult = CreateListFromItem(*pCC, Result);
				}
			else
				pResult = pCC->CreateNil();

			pArgs->Discard(pCC);
			break;
			}

		case FN_SCR_REFRESH_ITEM:
			{
			CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
			pArgs->Discard(pCC);
			CItemListManipulator *pItemList = &pScreen->GetItemListManipulator();
			pItemList->Refresh(Item);
			pResult = pCC->CreateNil();
			break;
			}

		default:
			ASSERT(FALSE);
		}

	//	Create the result

	return pResult;
	}

ICCItem *fnPlyGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnPlyGet
//
//	Gets data about the player

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pResult;

	//	Convert the first argument into a player controller

	CPlayerShipController *pPlayer = GetPlayerArg(pArgs->GetElement(0));
	if (pPlayer == NULL)
		return pCC->CreateNil();

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_PLY_CREDITS:
			{
			CEconomyType *pEcon = GetEconomyTypeFromItem(*pCC, (pArgs->GetCount() > 1 ? pArgs->GetElement(1) : NULL));
			if (pEcon == NULL)
				return pCC->CreateError(CONSTLIT("Invalid economy type"), pArgs->GetElement(1));

			pResult = pCC->CreateInteger((int)pPlayer->GetCredits(pEcon->GetUNID()));
			break;
			}

		case FN_PLY_GENOME:
			pResult = pCC->CreateString(GetGenomeID(pPlayer->GetPlayerGenome()));
			break;

		case FN_PLY_GET_ITEM_STAT:
			{
			CString sResult = pPlayer->GetItemStat(pArgs->GetElement(1)->GetStringValue(), pArgs->GetElement(2));
			if (!sResult.IsBlank())
				pResult = pCC->Link(sResult, 0, NULL);
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_PLY_GET_KEY_EVENT_STAT:
			{
			CDesignTypeCriteria Crit;
			if (CDesignTypeCriteria::ParseCriteria(pArgs->GetElement(3)->GetStringValue(), &Crit) != NOERROR)
				return pCC->CreateError(CONSTLIT("Invalid type criteria:"), pArgs->GetElement(3));

			CString sNodeID = (!pArgs->GetElement(2)->IsNil() ? pArgs->GetElement(2)->GetStringValue() : NULL_STR);

			CString sResult = pPlayer->GetKeyEventStat(pArgs->GetElement(1)->GetStringValue(), sNodeID, Crit);
			if (!sResult.IsBlank())
				pResult = pCC->Link(sResult, 0, NULL);
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_PLY_GET_STAT:
			{
			CString sResult = pPlayer->GetStat(pArgs->GetElement(1)->GetStringValue());
			if (!sResult.IsBlank())
				pResult = pCC->Link(sResult, 0, NULL);
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_PLY_IS_MESSAGE_ENABLED:
			{
			UIMessageTypes iMsg = pPlayer->FindUIMessage(pArgs->GetElement(1)->GetStringValue());
			if (iMsg == uimsgUnknown)
				return pCC->CreateError(CONSTLIT("Unknown messageID"), pArgs->GetElement(1));

			pResult = pCC->CreateBool(pPlayer->IsUIMessageEnabled(iMsg));
			break;
			}

		default:
			ASSERT(FALSE);
			pResult = pCC->CreateNil();
		}

	return pResult;
	}

ICCItem *fnPlyGetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnPlyGetOld
//
//	Gets data about the player
//
//	(plyGetCredits player) -> Credits left

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("i"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a player controller

	CPlayerShipController *pPlayer = GetPlayerArg(pArgs->GetElement(0));
	if (pPlayer == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	//	No longer needed

	pArgs->Discard(pCC);

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_PLY_INSURE:
			pResult = pCC->CreateNil();
			break;

		case FN_PLY_INSURANCE_CLAIMS:
			pResult = pCC->CreateInteger(0);
			break;

		case FN_PLY_IS_INSURED:
			pResult = pCC->CreateBool(false);
			break;

		case FN_PLY_REDIRECT_MESSAGE:
			{
			const CString &sMsg = pPlayer->GetTrans()->GetRedirectMessage();
			if (!sMsg.IsBlank())
				pResult = pCC->CreateString(sMsg);
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_PLY_SHOW_HELP_REFUEL:
			pPlayer->SetUIMessageEnabled(uimsgRefuelHint, false);
			pResult = pCC->CreateTrue();
			break;

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnPlyComposeString (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnPlyComposeString
//
//	Composes a string

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	Convert the first argument into a player controller

	CPlayerShipController *pPlayer = GetPlayerArg(pArgs->GetElement(0));
	if (pPlayer == NULL)
		return pCC->CreateNil();

	//	Compose

	return pCC->CreateString(pPlayer->GetTrans()->ComposePlayerNameString(pArgs->GetElement(1)->GetStringValue(), pArgs));
	}

ICCItem *fnPlySet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnPlySet
//
//	Sets player data

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pResult;

	//	Convert the first argument into a player controller

	CPlayerShipController *pPlayer = GetPlayerArg(pArgs->GetElement(0));
	if (pPlayer == NULL)
		return pCC->CreateNil();

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_PLY_CHANGE_SHIPS:
			{
			CSpaceObject *pObj = CreateObjFromItem(*pCC, pArgs->GetElement(1));
			if (pObj == NULL || pObj->IsDestroyed())
				return pCC->CreateError(CONSTLIT("Invalid object"), pArgs->GetElement(1));

			CShip *pNewShip = pObj->AsShip();
			if (pNewShip == NULL)
				return pCC->CreateError(CONSTLIT("Object must be a ship"), pArgs->GetElement(1));

			//	The ship must be a player ship

			if (!pNewShip->GetClass()->IsPlayerShip())
				return pCC->CreateError(CONSTLIT("Ship must be a player ship class"), pArgs->GetElement(1));

			//	Can't pick player ship

			if (pNewShip == g_pUniverse->GetPlayerShip())
				return pCC->CreateError(CONSTLIT("Ship is already player's ship"), pArgs->GetElement(1));

			//	Change ships

			if (pPlayer->SwitchShips(pNewShip) != NOERROR)
				return pCC->CreateNil();

			//	Done

			pResult = pCC->CreateTrue();
			break;
			}

		case FN_PLY_CHARGE:
			{
			CEconomyType *pEcon = GetEconomyTypeFromItem(*pCC, (pArgs->GetCount() > 2 ? pArgs->GetElement(1) : NULL));
			if (pEcon == NULL)
				return pCC->CreateError(CONSTLIT("Invalid economy type"), pArgs->GetElement(1));

			int iCharge = (pArgs->GetCount() > 2 ? pArgs->GetElement(2)->GetIntegerValue() : pArgs->GetElement(1)->GetIntegerValue());

			CurrencyValue iNewValue = pPlayer->Charge(pEcon->GetUNID(), iCharge);
			pResult = pCC->CreateInteger((int)iNewValue);
			break;
			}

		case FN_PLY_CREDITS:
			{
			CEconomyType *pEcon = GetEconomyTypeFromItem(*pCC, (pArgs->GetCount() > 2 ? pArgs->GetElement(1) : NULL));
			if (pEcon == NULL)
				return pCC->CreateError(CONSTLIT("Invalid economy type"), pArgs->GetElement(1));

			int iValue = (pArgs->GetCount() > 2 ? pArgs->GetElement(2)->GetIntegerValue() : pArgs->GetElement(1)->GetIntegerValue());

			pPlayer->Payment(pEcon->GetUNID(), iValue);
			pResult = pCC->CreateInteger((int)pPlayer->GetCredits(pEcon->GetUNID()));
			break;
			}

		case FN_PLY_ENABLE_MESSAGE:
			{
			UIMessageTypes iMsg = pPlayer->FindUIMessage(pArgs->GetElement(1)->GetStringValue());
			if (iMsg == uimsgUnknown)
				return pCC->CreateError(CONSTLIT("Unknown message name"), pArgs->GetElement(1));

			pPlayer->SetUIMessageEnabled(iMsg, (pArgs->GetElement(2)->IsNil() ? false : true));
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_PLY_RECORD_BUY_ITEM:
		case FN_PLY_RECORD_SELL_ITEM:
			{
			int iArg = 1;

			//	The item that we sold or bought

			CItem Item = CreateItemFromList(*pCC, pArgs->GetElement(iArg));
			iArg++;

			//	The currency

			CEconomyType *pCurrency;
			if (pArgs->GetCount() > 3)
				{
				pCurrency = GetEconomyTypeFromItem(*pCC, pArgs->GetElement(iArg));
				if (pCurrency == NULL)
					return pCC->CreateError(CONSTLIT("Invalid economy type"), pArgs->GetElement(iArg));

				iArg++;
				}
			else
				{
				pCurrency = CEconomyType::AsType(g_pUniverse->FindDesignType(DEFAULT_ECONOMY_UNID));
				ASSERT(pCurrency);
				}

			//	The value

			CurrencyValue iPrice = pArgs->GetElement(iArg)->GetIntegerValue();

			//	Convert to credits, if necessary

			if (!pCurrency->IsCreditEquivalent())
				{
				CEconomyType *pCreditEcon = CEconomyType::AsType(g_pUniverse->FindDesignType(DEFAULT_ECONOMY_UNID));
				iPrice = pCreditEcon->Exchange(pCurrency, iPrice);
				}

			//	Record

			if (dwData == FN_PLY_RECORD_BUY_ITEM)
				pPlayer->OnItemBought(Item, iPrice);
			else
				pPlayer->OnItemSold(Item, iPrice);

			pResult = pCC->CreateTrue();
			break;
			}

		case FN_PLY_USE_ITEM:
			{
			CItem Item = CreateItemFromList(*pCC, pArgs->GetElement(1));
			if (Item.GetType() == NULL)
				return pCC->CreateError(CONSTLIT("Invalid item"), pArgs->GetElement(1));

			g_pTrans->GetModel().UseItem(Item);
			pResult = pCC->CreateTrue();
			break;
			}

		default:
			ASSERT(FALSE);
			pResult = pCC->CreateNil();
		}

	return pResult;
	}

ICCItem *fnPlySetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnPlySetOld
//
//	Sets player data
//
//	(plyCharge player charge)

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("i*"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a player controller

	CPlayerShipController *pPlayer = GetPlayerArg(pArgs->GetElement(0));
	if (pPlayer == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_PLY_COMPOSE_STRING:
			{
			pResult = pCC->CreateString(pPlayer->GetTrans()->ComposePlayerNameString(pArgs->GetElement(1)->GetStringValue()));
			pArgs->Discard(pCC);
			break;
			}

		case FN_PLY_DESTROYED:
			{
			CString sText = pArgs->GetElement(1)->GetStringValue();
			pArgs->Discard(pCC);
			CDamageSource Cause(NULL, killedByOther, NULL, sText, 0);
			pPlayer->GetShip()->Destroy(killedByOther, Cause);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_PLY_MESSAGE:
			{
			//	If we have more than one arg, then we concatenate

			CString sText;
			if (pArgs->GetCount() > 2)
				{
				for (int i = 1; i < pArgs->GetCount(); i++)
					if (!pArgs->GetElement(i)->IsNil())
						sText.Append(pArgs->GetElement(i)->GetStringValue());

				sText = pPlayer->GetTrans()->ComposePlayerNameString(sText);
				}

			//	Otherwise, we just set the text

			else
				sText = pPlayer->GetTrans()->ComposePlayerNameString(pArgs->GetElement(1)->GetStringValue());

			pPlayer->GetTrans()->DisplayMessage(sText);

			pArgs->Discard(pCC);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_PLY_REDIRECT_MESSAGE:
			{
			bool bRedirect = !pArgs->GetElement(1)->IsNil();
			pArgs->Discard(pCC);
			pPlayer->GetTrans()->RedirectDisplayMessage(bRedirect);
			pResult = pCC->CreateTrue();
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnScrGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnScrGet
//
//	Sets screen data

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	Convert the first argument into a dock screen object

	CDockScreen *pScreen = GetDockScreenArg(pArgs->GetElement(0));
	if (pScreen == NULL)
		return pCC->CreateError(CONSTLIT("Screen expected"), pArgs->GetElement(0));

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_SCR_LIST_CURSOR:
			return pCC->CreateInteger(pScreen->GetListCursor());

		case FN_SCR_DATA:
			{
			if (!g_pTrans->GetModel().InScreenSession())
				return pCC->CreateNil();

			return g_pTrans->GetModel().GetScreenData(pArgs->GetElement(1)->GetStringValue());
			}

		case FN_SCR_DESC:
			{
			const CString &sDesc = pScreen->GetDescription();
			if (sDesc.IsBlank())
				return pCC->CreateNil();

			return pCC->CreateString(sDesc);
			}

		case FN_SCR_IS_ACTION_ENABLED:
			{
			//	Only if valid

			if (!pScreen->IsValid())
				return pCC->CreateNil();

			//	Parameters

			int iAction;
			CDockScreenActions &Actions = pScreen->GetActions();
			if (!Actions.FindByID(pArgs->GetElement(1), &iAction))
				return pCC->CreateError(CONSTLIT("Invalid action ID"), pArgs->GetElement(1));

			return pCC->CreateBool(Actions.IsVisible(iAction) && Actions.IsEnabled(iAction));
			}

		case FN_SCR_TRANSLATE:
			{
			CString sText = pArgs->GetElement(1)->GetStringValue();

			ICCItem *pData = NULL;
			if (pArgs->GetCount() > 2)
				pData = pArgs->GetElement(2);

			ICCItem *pResult;
			if (!pScreen->Translate(sText, pData, &pResult))
				return pCC->CreateNil();

			return pResult;
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnScrGetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnScrGetOld
//
//	Sets screen data
//
//	(scrGetCounter screen)

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("i"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a dock screen object

	CDockScreen *pScreen = GetDockScreenArg(pArgs->GetElement(0));
	if (pScreen == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(LITERAL("Entry pointer expected"), pArgs->GetElement(0));
		}

	//	Done with args

	pArgs->Discard(pCC);

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_SCR_COUNTER:
			pResult = pCC->CreateInteger(pScreen->GetCounter());
			break;

		case FN_SCR_INPUT_TEXT:
			pResult = pCC->CreateString(pScreen->GetTextInput());
			break;

		case FN_SCR_IS_FIRST_ON_INIT:
			pResult = pCC->CreateBool(pScreen->IsFirstOnInit());
			break;

		case FN_SCR_LIST_ENTRY:
			pResult = pScreen->GetCurrentListEntry();
			break;

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnScrSet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnScrSet
//
//	Sets screen data

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	int i;

	//	Convert the first argument into a dock screen object

	CDockScreen *pScreen = GetDockScreenArg(pArgs->GetElement(0));
	if (pScreen == NULL)
		return pCC->CreateError(CONSTLIT("Screen expected"), pArgs->GetElement(0));

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_SCR_ACTION_DESC:
			{
			//	Only if valid

			if (!pScreen->IsValid())
				return pCC->CreateNil();

			//	Parameters

			int iAction;
			CDockScreenActions &Actions = pScreen->GetActions();
			if (!Actions.FindByID(pArgs->GetElement(1), &iAction))
				return pCC->CreateError(CONSTLIT("Invalid action ID"), pArgs->GetElement(1));

			CString sDesc;
			if (!pArgs->GetElement(2)->IsNil())
				sDesc = pArgs->GetElement(2)->GetStringValue();

			Actions.SetDesc(iAction, sDesc);

			return pCC->CreateTrue();
			}

		case FN_SCR_ACTION_LABEL:
			{
			//	Only if valid

			if (!pScreen->IsValid())
				return pCC->CreateNil();

			//	Parameters

			int iAction;
			CDockScreenActions &Actions = pScreen->GetActions();
			if (!Actions.FindByID(pArgs->GetElement(1), &iAction))
				return pCC->CreateError(CONSTLIT("Invalid action ID"), pArgs->GetElement(1));

			CString sLabel;
			if (!pArgs->GetElement(2)->IsNil())
				sLabel = pArgs->GetElement(2)->GetStringValue();

			CString sKey;
			if (pArgs->GetCount() > 3 && !pArgs->GetElement(3)->IsNil())
				sKey = pArgs->GetElement(3)->GetStringValue();
			else
				sKey = Actions.GetKey(iAction);

			Actions.SetLabel(iAction, sLabel, sKey);

			//	If we have a fifth arg then it is a list of special keys

			if (pArgs->GetCount() > 4)
				{
				ICCItem *pError;
				if (!Actions.SetSpecial(*pCC, iAction, pArgs->GetElement(4), &pError))
					return pError;
				}

			return pCC->CreateTrue();
			}

		case FN_SCR_ADD_ACTION:
			{
			CCodeChainCtx *pCtx = (CCodeChainCtx *)pEvalCtx->pExternalCtx;

			//	Only if valid

			if (!pScreen->IsValid())
				return pCC->CreateNil();

			//	Get the parameters

			CDockScreenActions &Actions = pScreen->GetActions();
			CString sID = pArgs->GetElement(1)->GetStringValue();
			int iPos = (pArgs->GetElement(2)->IsNil() ? -1 : pArgs->GetElement(2)->GetIntegerValue());
			CString sLabel = pArgs->GetElement(3)->GetStringValue();
			
			int iArg = 4;
			CString sKey;
			if (pArgs->GetCount() > iArg + 1)
				{
				if (!pArgs->GetElement(iArg)->IsNil())
					sKey = pArgs->GetElement(iArg)->GetStringValue();
				iArg++;
				}

			ICCItem *pSpecial = NULL;
			if (pArgs->GetCount() > iArg + 1)
				{
				pSpecial = pArgs->GetElement(iArg);
				iArg++;
				}

			ICCItem *pCode = pArgs->GetElement(iArg);

			//	Check for some error conditions

			if (Actions.FindByID(sID))
				return pCC->CreateError(CONSTLIT("Action ID already added"), pArgs->GetElement(1));

			if (iPos == Actions.GetCount())
				iPos = -1;

			if (iPos != -1 && (iPos < 0 || iPos > Actions.GetCount()))
				return pCC->CreateError(CONSTLIT("Invalid action position"), pArgs->GetElement(2));

			//	Add the action

			int iAction;
			if (Actions.AddAction(sID, iPos, sLabel, pCtx->GetExtension(), pCode, &iAction) != NOERROR)
				return pCC->CreateError(CONSTLIT("Unable to add action"), pArgs->GetElement(1));

			//	Set key and special

			if (!sKey.IsBlank())
				Actions.SetLabel(iAction, NULL_STR, sKey);

			if (pSpecial)
				Actions.SetSpecial(*pCC, iAction, pSpecial, NULL);

			return pCC->CreateTrue();
			}

		case FN_SCR_BACKGROUND_IMAGE:
			{
			IDockScreenDisplay::SBackgroundDesc Desc;
			if (!IDockScreenDisplay::ParseBackgrounDesc(pArgs->GetElement(1), &Desc))
				return pCC->CreateError(CONSTLIT("Invalid image description"), pArgs->GetElement(1));

			pScreen->SetBackground(Desc);
			return pCC->CreateTrue();
			}

		case FN_SCR_CONTROL_VALUE:
			{
			CString sID = pArgs->GetElement(1)->GetStringValue();
			ICCItem *pValue = pArgs->GetElement(2);
			return pCC->CreateBool(pScreen->SetControlValue(sID, pValue));
			}

		case FN_SCR_DATA:
			{
			if (!g_pTrans->GetModel().InScreenSession())
				return pCC->CreateNil();

			g_pTrans->GetModel().SetScreenData(pArgs->GetElement(1)->GetStringValue(), pArgs->GetElement(2));
			return pCC->CreateTrue();
			}

		case FN_SCR_DESC:
			{
			//	Only if valid

			if (!pScreen->IsValid())
				return pCC->CreateNil();

			//	If we have more than one arg, then we concatenate

			if (pArgs->GetCount() > 2)
				{
				CString sText;

				for (i = 1; i < pArgs->GetCount(); i++)
					if (!pArgs->GetElement(i)->IsNil())
						sText.Append(pArgs->GetElement(i)->GetStringValue());

				pScreen->SetDescription(g_pTrans->ComposePlayerNameString(sText));
				}

			//	Otherwise, we just set the text

			else
				pScreen->SetDescription(g_pTrans->ComposePlayerNameString(pArgs->GetElement(1)->GetStringValue()));

			//	Done

			return pCC->CreateTrue();
			}

		case FN_SCR_DESC_TRANSLATE:
			{
			//	Only if valid

			if (!pScreen->IsValid())
				return pCC->CreateNil();

			//	Translate

			CString sText = pArgs->GetElement(1)->GetStringValue();

			ICCItem *pData = NULL;
			if (pArgs->GetCount() > 2)
				pData = pArgs->GetElement(2);

			ICCItem *pResult;
			if (!pScreen->Translate(sText, pData, &pResult))
				{
				pScreen->SetDescription(strPatternSubst(CONSTLIT("Unknown Language ID: %s"), sText));
				return pCC->CreateNil();
				}

			//	Set the screen descriptor

			pScreen->SetDescription(g_pTrans->ComposePlayerNameString(pResult->GetStringValue()));
			pResult->Discard(pCC);

			return pCC->CreateTrue();
			}

		case FN_SCR_ENABLE_ACTION:
			{
			//	Only if valid

			if (!pScreen->IsValid())
				return pCC->CreateNil();

			//	Parameters

			int iAction;
			CDockScreenActions &Actions = pScreen->GetActions();
			if (!Actions.FindByID(pArgs->GetElement(1), &iAction))
				return pCC->CreateError(CONSTLIT("Invalid action ID"), pArgs->GetElement(1));

			bool bEnable = !pArgs->GetElement(2)->IsNil();

			Actions.SetEnabled(iAction, bEnable);
			return pCC->CreateTrue();
			}

		case FN_SCR_EXIT_DOCK:
		case FN_SCR_EXIT_SCREEN:
			{
			bool bForceUndock = (dwData == FN_SCR_EXIT_DOCK)
					|| (pArgs->GetCount() > 1 && !pArgs->GetElement(1)->IsNil());

			//	If we're not in a screen session then just fail silently. Destroying the
			//	player also exits the session, and sometimes people will destroy the player
			//	and exit the session for completeness.

			if (!g_pTrans->GetModel().InScreenSession())
				return pCC->CreateNil();

			g_pTrans->GetModel().ExitScreenSession(bForceUndock);
			return pCC->CreateTrue();
			}

		case FN_SCR_REFRESH_SCREEN:
			{
			if (!g_pTrans->GetModel().InScreenSession())
				return pCC->CreateNil();

			g_pTrans->GetModel().RefreshScreenSession();
			return pCC->CreateTrue();
			}

		case FN_SCR_REMOVE_ACTION:
			{
			//	Only if valid

			if (!pScreen->IsValid())
				return pCC->CreateNil();

			//	Parameters

			int iAction;
			CDockScreenActions &Actions = pScreen->GetActions();
			if (!Actions.FindByID(pArgs->GetElement(1), &iAction))
				return pCC->CreateError(CONSTLIT("Invalid action ID"), pArgs->GetElement(1));

			Actions.RemoveAction(iAction);
			return pCC->CreateTrue();
			}

		case FN_SCR_SET_DISPLAY_TEXT:
			{
			CString sID = pArgs->GetElement(1)->GetStringValue();

			//	If we have more than one arg, then we concatenate

			ALERROR error;
			if (pArgs->GetCount() > 3)
				{
				CString sText;

				for (i = 2; i < pArgs->GetCount(); i++)
					if (!pArgs->GetElement(i)->IsNil())
						sText.Append(pArgs->GetElement(i)->GetStringValue());

				error = pScreen->SetDisplayText(sID, sText);
				}

			//	Otherwise, we just set the text

			else
				error = pScreen->SetDisplayText(sID, pArgs->GetElement(2)->GetStringValue());

			if (error)
				return pCC->CreateError(CONSTLIT("Invalid display ID"), pArgs->GetElement(1));

			//	Done

			return pCC->CreateTrue();
			}

		case FN_SCR_LIST_CURSOR:
			pScreen->SetListCursor(pArgs->GetElement(1)->GetIntegerValue());
			return pCC->CreateTrue();

		case FN_SCR_SHOW_ACTION:
			{
			//	Only if valid

			if (!pScreen->IsValid())
				return pCC->CreateNil();

			//	Parameters

			int iAction;
			CDockScreenActions &Actions = pScreen->GetActions();
			if (!Actions.FindByID(pArgs->GetElement(1), &iAction))
				return pCC->CreateError(CONSTLIT("Invalid action ID"), pArgs->GetElement(1));

			bool bShow = !pArgs->GetElement(2)->IsNil();

			Actions.SetVisible(iAction, bShow);
			return pCC->CreateTrue();
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnScrSetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnScrSetOld
//
//	Sets screen data
//
//	(scrShowPane scr pane)

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	CCodeChainCtx *pCtx = (CCodeChainCtx *)pEvalCtx->pExternalCtx;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	if (dwData == FN_SCR_LIST_FILTER)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("iv"));
	else if (dwData == FN_SCR_COUNTER)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("ii"));
	else
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("is"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a dock screen object

	CDockScreen *pScreen = GetDockScreenArg(pArgs->GetElement(0));
	if (pScreen == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(LITERAL("Entry pointer expected"), pArgs->GetElement(0));
		}

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_SCR_SHOW_PANE:
			{
			//	Only if valid

			if (!pScreen->IsValid())
				return pCC->CreateNil();

			//	Do it

			if (g_pTrans->GetModel().ShowPane(pArgs->GetElement(1)->GetStringValue()) != NOERROR)
				{
				pResult = pCC->CreateError(CONSTLIT("Unable to show pane"), pArgs->GetElement(1));
				pArgs->Discard(pCC);
				return pResult;
				}
			pArgs->Discard(pCC);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_SCR_COUNTER:
			pScreen->SetCounter(pArgs->GetElement(1)->GetIntegerValue());
			pArgs->Discard(pCC);
			pResult = pCC->CreateTrue();
			break;

		case FN_SCR_INPUT_TEXT:
			pScreen->SetTextInput(pArgs->GetElement(1)->GetStringValue());
			pArgs->Discard(pCC);
			pResult = pCC->CreateTrue();
			break;

		case FN_SCR_LIST_FILTER:
			{
			if (pArgs->GetElement(1)->IsFunction())
				{
				CItemCriteria Filter;
				Filter.pFilter = pArgs->GetElement(1)->Reference();
				pArgs->Discard(pCC);

				pScreen->SetListFilter(Filter);
				}
			else
				{
				CString sFilter = pArgs->GetElement(1)->GetStringValue();
				pArgs->Discard(pCC);

				CItemCriteria Filter;
				CItem::ParseCriteria(sFilter, &Filter);

				pScreen->SetListFilter(Filter);
				}

			//	If we're not inside list initialization, advance to the next cursor 
			//	position (otherwise we end up with a cursor of -1)

			if (!pCtx->InEvent(eventInitDockScreenList))
				pScreen->SelectNextItem();

			pResult = pCC->CreateTrue();
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnScrShowScreen (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnScrShowScreen
//
//	Shows the given screen
//	(scrShowScreen gScreen screen [pane] [data])

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	CCodeChainCtx *pCtx = (CCodeChainCtx *)pEvalCtx->pExternalCtx;

	//	Get the arguments

	CDockScreen *pScreen = GetDockScreenArg(pArgs->GetElement(0));
	CString sScreen;
	CString sPane;
	ICCItem *pData = NULL;

	ICCItem *pScreenArg = pArgs->GetElement(1);
	if (pScreenArg->GetCount() > 1)
		{
		sScreen = pScreenArg->GetElement(0)->GetStringValue();
		if (pScreenArg->GetCount() == 2)
			{
			if (pScreenArg->GetElement(1)->IsIdentifier())
				sPane = pScreenArg->GetElement(1)->GetStringValue();
			else
				pData = pScreenArg->GetElement(1);
			}
		else
			{
			sPane = (pScreenArg->GetElement(1)->IsNil() ? NULL_STR : pScreenArg->GetElement(1)->GetStringValue());
			pData = pScreenArg->GetElement(2);
			}
		}
	else
		{
		sScreen = pScreenArg->GetStringValue();
		if (pArgs->GetCount() >= 4)
			{
			sPane = (pArgs->GetElement(2)->IsNil() ? NULL_STR : pArgs->GetElement(2)->GetStringValue());
			pData = pArgs->GetElement(3);
			}
		else if (pArgs->GetCount() >= 3)
			{
			if (pArgs->GetElement(2)->IsIdentifier())
				sPane = pArgs->GetElement(2)->GetStringValue();
			else
				pData = pArgs->GetElement(2);
			}
		}

	//	If we're not currently docked, then bring up a screen with the player ship as
	//	the location.

	if (!g_pTrans->GetModel().InScreenSession())
		{
		//	Sometimes the local screens root is set in the context. For example, if we're
		//	executing code from an item, the item type is set as the local screens root.

		CString sError;
		if (!g_pTrans->GetModel().ShowShipScreen(pCtx->GetScreensRoot(), NULL, sScreen, sPane, pData, &sError))
			return pCC->CreateError(sError);
		}

	//	Otherwise, call the current screen

	else
		{
		if (pScreen == NULL)
			return pCC->CreateError(CONSTLIT("Screen expected"), pArgs->GetElement(0));

		CString sError;
		if (g_pTrans->GetModel().ShowScreen(NULL, sScreen, sPane, pData, &sError) != NOERROR)
			return pCC->CreateError(sError, pArgs->GetElement(1));
		}

	//	Done

	return pCC->CreateTrue();
	}
