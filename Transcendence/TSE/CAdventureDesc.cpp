//	CAdventureDesc.cpp
//
//	CAdventureDesc class

#include "PreComp.h"

#define ARMOR_DAMAGE_ADJ_TAG					CONSTLIT("ArmorDamageAdj")
#define CONSTANTS_TAG							CONSTLIT("Constants")
#define EVENTS_TAG								CONSTLIT("Events")
#define SHIELD_DAMAGE_ADJ_TAG					CONSTLIT("ShieldDamageAdj")

#define ADVENTURE_UNID_ATTRIB					CONSTLIT("adventureUNID")
#define BACKGROUND_ID_ATTRIB					CONSTLIT("backgroundID")
#define DESC_ATTRIB								CONSTLIT("desc")
#define INCLUDE_10_STARTING_CLASSES_ATTRIB		CONSTLIT("include10StartingShips")
#define LEVEL_ATTRIB							CONSTLIT("level")
#define NAME_ATTRIB								CONSTLIT("name")
#define STARTING_MAP_ATTRIB						CONSTLIT("startingMap")
#define STARTING_POS_ATTRIB						CONSTLIT("startingPos")
#define STARTING_SHIP_CRITERIA_ATTRIB			CONSTLIT("startingShipCriteria")
#define STARTING_SYSTEM_ATTRIB					CONSTLIT("startingSystem")
#define WELCOME_MESSAGE_ATTRIB					CONSTLIT("welcomeMessage")

#define ON_GAME_START_EVENT						CONSTLIT("OnGameStart")
#define ON_GAME_END_EVENT						CONSTLIT("OnGameEnd")

#define FIELD_NAME								CONSTLIT("name")

#define LANGUAGE_DESCRIPTION					CONSTLIT("description")

#define ERR_STARTING_SHIP_CRITERIA				CONSTLIT("Unable to parse startingShipCriteria")

static int g_StdArmorDamageAdj[MAX_ITEM_LEVEL][damageCount] =
	{
		//	lsr knt par blt ion thr pos pls am  nan grv sng dac dst dlg dfr
		{	100,100,100,100,100,100,100,100,125,125,200,200,300,300,500,500 },
		{	 80, 80,100,100,100,100,100,100,125,125,200,200,300,300,500,500 },
		{	 60, 60,100,100,100,100,100,100,125,125,200,200,300,300,500,500 },
		{	 40, 40,100,100,100,100,100,100,100,100,125,125,200,200,300,300 },
		{	 25, 25, 80, 80,100,100,100,100,100,100,125,125,200,200,300,300 },

		{	 14, 14, 60, 60,100,100,100,100,100,100,125,125,200,200,300,300 },
		{	  8,  8, 40, 40,100,100,100,100,100,100,100,100,125,125,200,200 },
		{	  4,  4, 25, 25, 80, 80,100,100,100,100,100,100,125,125,200,200 },
		{	  2,  2, 14, 14, 60, 60,100,100,100,100,100,100,125,125,200,200 },
		{	  0,  0,  8,  8, 40, 40,100,100,100,100,100,100,100,100,125,125 },

		{	  0,  0,  4,  4, 25, 25, 80, 80,100,100,100,100,100,100,125,125 },
		{	  0,  0,  2,  2, 14, 14, 60, 60,100,100,100,100,100,100,125,125 },
		{	  0,  0,  0,  0,  8,  8, 40, 40,100,100,100,100,100,100,100,100 },
		{	  0,  0,  0,  0,  4,  4, 25, 25, 80, 80,100,100,100,100,100,100 },
		{	  0,  0,  0,  0,  2,  2, 14, 14, 60, 60,100,100,100,100,100,100 },

		{	  0,  0,  0,  0,  0,  0,  8,  8, 40, 40,100,100,100,100,100,100 },
		{	  0,  0,  0,  0,  0,  0,  4,  4, 25, 25, 80, 80,100,100,100,100 },
		{	  0,  0,  0,  0,  0,  0,  2,  2, 14, 14, 60, 60,100,100,100,100 },
		{	  0,  0,  0,  0,  0,  0,  0,  0,  8,  8, 40, 40,100,100,100,100 },
		{	  0,  0,  0,  0,  0,  0,  0,  0,  4,  4, 25, 25, 80, 80,100,100 },

		{	  0,  0,  0,  0,  0,  0,  0,  0,  2,  2, 14, 14, 60, 60,100,100 },
		{	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  8, 40, 40,100,100 },
		{	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  4, 25, 25, 80, 80 },
		{	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2, 14, 14, 60, 60 },
		{	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  8, 40, 40 },
	};

static int g_StdShieldDamageAdj[MAX_ITEM_LEVEL][damageCount] =
	{
		//	lsr knt par blt  ion thr pos pls  am  nan grv sng  dac dst dlg dfr
		{	100,100,100,100, 100,100,160,160, 225,225,300,300, 375,375,500,500 },
		{	 95, 95,100,100, 100,100,140,140, 200,200,275,275, 350,350,450,450 },
		{	 90, 90,100,100, 100,100,120,120, 180,180,250,250, 325,325,400,400 },
		{	 85, 85,100,100, 100,100,100,100, 160,160,225,225, 300,300,375,375 },
		{	 80, 80, 95, 95, 100,100,100,100, 140,140,200,200, 275,275,350,350 },

		{	 75, 75, 90, 90, 100,100,100,100, 120,120,180,180, 250,250,325,325 },
		{	 70, 70, 85, 85, 100,100,100,100, 100,100,160,160, 225,225,300,300 },
		{	 65, 65, 80, 80,  95, 95,100,100, 100,100,140,140, 200,200,275,275 },
		{	 60, 60, 75, 75,  90, 90,100,100, 100,100,120,120, 180,180,250,250 },
		{	 55, 55, 70, 70,  85, 85,100,100, 100,100,100,100, 160,160,225,225 },

		{	 50, 50, 65, 65,  80, 80, 95, 95, 100,100,100,100, 140,140,200,200 },
		{	 40, 40, 60, 60,  75, 75, 90, 90, 100,100,100,100, 120,120,180,180 },
		{	 30, 30, 55, 55,  70, 70, 85, 85, 100,100,100,100, 100,100,160,160 },
		{	 20, 20, 50, 50,  65, 65, 80, 80,  95, 95,100,100, 100,100,140,140 },
		{	 10, 10, 40, 40,  60, 60, 75, 75,  90, 90,100,100, 100,100,120,120 },

		{	  0,  0, 30, 30,  55, 55, 70, 70,  85, 85,100,100, 100,100,100,100 },
		{	  0,  0, 20, 20,  50, 50, 65, 65,  80, 80, 95, 95, 100,100,100,100 },
		{	  0,  0, 10, 10,  40, 40, 60, 60,  75, 75, 90, 90, 100,100,100,100 },
		{	  0,  0,  0,  0,  30, 30, 55, 55,  70, 70, 85, 85, 100,100,100,100 },
		{	  0,  0,  0,  0,  20, 20, 50, 50,  65, 65, 80, 80,  95, 95,100,100 },

		{	  0,  0,  0,  0,  10, 10, 40, 40,  60, 60, 75, 75,  90, 90,100,100 },
		{	  0,  0,  0,  0,   0,  0, 30, 30,  55, 55, 70, 70,  85, 85,100,100 },
		{	  0,  0,  0,  0,   0,  0, 20, 20,  50, 50, 65, 65,  80, 80, 95, 95 },
		{	  0,  0,  0,  0,   0,  0, 10, 10,  40, 40, 60, 60,  75, 75, 90, 90 },
		{	  0,  0,  0,  0,   0,  0,  0,  0,  30, 30, 55, 55,  70, 70, 85, 85 },
	};

static bool g_bDamageAdjInit = false;
static CDamageAdjDesc g_ArmorDamageAdj[MAX_ITEM_LEVEL];
static CDamageAdjDesc g_ShieldDamageAdj[MAX_ITEM_LEVEL];

bool CAdventureDesc::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	if (strEquals(sField, FIELD_NAME))
		*retsValue = m_sName;
	else
		return false;

	return true;
	}

void CAdventureDesc::FireOnGameEnd (const CGameRecord &Game, const SBasicGameStats &BasicStats)

//	FireOnGameEnd
//
//	Fire OnGameEnd event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_GAME_END_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		//	Initialize variables

		Ctx.DefineInteger(CONSTLIT("aScore"), Game.GetScore());
		Ctx.DefineInteger(CONSTLIT("aResurrectCount"), Game.GetResurrectCount());
		Ctx.DefineInteger(CONSTLIT("aSystemsVisited"), BasicStats.iSystemsVisited);
		Ctx.DefineInteger(CONSTLIT("aEnemiesDestroyed"), BasicStats.iEnemiesDestroyed);
		Ctx.DefineInteger(CONSTLIT("aBestEnemiesDestroyed"), BasicStats.iBestEnemyDestroyedCount);
		if (BasicStats.dwBestEnemyDestroyed)
			Ctx.DefineInteger(CONSTLIT("aBestEnemyClass"), BasicStats.dwBestEnemyDestroyed);
		else
			Ctx.DefineNil(CONSTLIT("aBestEnemyClass"));

		Ctx.DefineString(CONSTLIT("aEndGameReason"), Game.GetEndGameReason());
		Ctx.DefineString(CONSTLIT("aEpitaph"), Game.GetEndGameEpitaph());
		Ctx.DefineString(CONSTLIT("aEpitaphOriginal"), Game.GetEndGameEpitaph());
		Ctx.DefineString(CONSTLIT("aTime"), Game.GetPlayTimeString());

		//	Invoke

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			kernelDebugLogMessage("OnGameEnd error: %s", pResult->GetStringValue());
		Ctx.Discard(pResult);
		}
	}

void CAdventureDesc::FireOnGameStart (void)

//	FireOnGameStart
//
//	Fire OnGameStart event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_GAME_START_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		//	Run code

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			kernelDebugLogMessage("OnGameStart error: %s", pResult->GetStringValue());
		Ctx.Discard(pResult);
		}
	}

const CDamageAdjDesc *CAdventureDesc::GetDefaultArmorDamageAdj (int iLevel)

//	GetDefaultArmorDamageAdj
//
//	Returns the default table

	{
	InitDefaultDamageAdj();
	return &g_ArmorDamageAdj[iLevel - 1];
	}

const CDamageAdjDesc *CAdventureDesc::GetDefaultShieldDamageAdj (int iLevel)

//	GetDefaultShieldDamageAdj
//
//	Returns the default table

	{
	InitDefaultDamageAdj();
	return &g_ShieldDamageAdj[iLevel - 1];
	}

CString CAdventureDesc::GetDesc (void)

//	GetDesc
//
//	Returns a description for the adventure

	{
	CString sText;
	if (TranslateText(NULL, LANGUAGE_DESCRIPTION, NULL, &sText))
		return CTextBlock::LoadAsRichText(sText);
	else
		return CONSTLIT("{/rtf }");
	}

ALERROR CAdventureDesc::GetStartingShipClasses (TSortMap<CString, CShipClass *> *retClasses, CString *retsError)

//	GetStartingShipClasses
//
//	Returns a sorted list of ship classes for this adventure

	{
	int i;

	bool bShowDebugShips = g_pUniverse->InDebugMode();

	//	Make a list

	retClasses->DeleteAll();
	for (i = 0; i < g_pUniverse->GetShipClassCount(); i++)
		{
		CShipClass *pClass = g_pUniverse->GetShipClass(i);
		if (pClass->IsShownAtNewGame()
				&& IsValidStartingClass(pClass)
				&& (!pClass->IsDebugOnly() || bShowDebugShips))
			{
			CString sKey = strPatternSubst(CONSTLIT("%d %s !%x"), 
					(pClass->IsDebugOnly() ? 2 : 1),
					pClass->GetName(), 
					pClass->GetUNID());
			retClasses->Insert(sKey, pClass);
			}
		}

	return NOERROR;
	}

void CAdventureDesc::InitDefaultDamageAdj (void)

//	InitDefaultDamageAdj
//
//	Initialize default tables

	{
	int i;

	if (!g_bDamageAdjInit)
		{
		SDesignLoadCtx Ctx;

		for (i = 1; i <= MAX_ITEM_LEVEL; i++)
			{
			g_ArmorDamageAdj[i - 1].InitFromArray(g_StdArmorDamageAdj[i - 1]);
			g_ShieldDamageAdj[i - 1].InitFromArray(g_StdShieldDamageAdj[i - 1]);
			}

		g_bDamageAdjInit = true;
		}
	}

bool CAdventureDesc::IsValidStartingClass (CShipClass *pClass)

//	IsValidStartingClass
//
//	Return TRUE if this ship class is a valid starting class for this adventure.

	{
	if (m_fIncludeOldShipClasses && pClass->GetAPIVersion() < 12)
		{
		if (pClass->HasLiteralAttribute(CONSTLIT("notInPartI")))
			return false;
		else
			return true;
		}

	return (pClass->IsIncludedInAllAdventures() || pClass->MatchesCriteria(m_StartingShips));
	}

ALERROR CAdventureDesc::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design elements

	{
	return NOERROR;
	}

ALERROR CAdventureDesc::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Load from XML

	{
	ALERROR error;
	int i;

	//	If we are part of the default resource, then get the adventure UNID

	if (Ctx.pExtension == NULL)
		{
		m_dwExtensionUNID = pDesc->GetAttributeInteger(ADVENTURE_UNID_ATTRIB);
		m_fInDefaultResource = true;
		}

	//	Otherwise, we remember the extension that we were loaded from

	else
		{
		m_dwExtensionUNID = Ctx.pExtension->GetUNID();
		m_fInDefaultResource = false;
		}

	//	Load the name, etc

	m_sName = pDesc->GetAttribute(NAME_ATTRIB);
	if (error = ::LoadUNID(Ctx, pDesc->GetAttribute(BACKGROUND_ID_ATTRIB), &m_dwBackgroundUNID))
		return error;

	//	Starting ship criteria

	CString sCriteria;
	if (!pDesc->FindAttribute(STARTING_SHIP_CRITERIA_ATTRIB, &sCriteria))
		sCriteria = CONSTLIT("*");

	if (error = CDesignTypeCriteria::ParseCriteria(sCriteria, &m_StartingShips))
		return ComposeLoadError(Ctx, ERR_STARTING_SHIP_CRITERIA);

	m_fIncludeOldShipClasses = pDesc->GetAttributeBool(INCLUDE_10_STARTING_CLASSES_ATTRIB);

	//	Starting position

	if (error = ::LoadUNID(Ctx, pDesc->GetAttribute(STARTING_MAP_ATTRIB), &m_dwStartingMap))
		return error;

	m_sStartingNodeID = pDesc->GetAttribute(STARTING_SYSTEM_ATTRIB);
	m_sStartingPos = pDesc->GetAttribute(STARTING_POS_ATTRIB);

	//	Welcome message

	if (!pDesc->FindAttribute(WELCOME_MESSAGE_ATTRIB, &m_sWelcomeMessage))
		m_sWelcomeMessage = CONSTLIT("Welcome to Transcendence!");

	//	Init some flags

	m_fIsCurrentAdventure = false;

	//	If we don't have a name, then get it from the extension

	if (m_sName.IsBlank())
		{
		if (Ctx.pExtension)
			m_sName = Ctx.pExtension->GetName();
		}

	//	Otherwise, if the extension doesn't have a name, then we can set it

	else if (Ctx.pExtension && strFind(Ctx.pExtension->GetName(), CONSTLIT("Extension")) == 0)
		Ctx.pExtension->SetName(m_sName);

	//	Initialize armor and shield damage adjustment tables

	InitDefaultDamageAdj();
	for (i = 1; i <= MAX_ITEM_LEVEL; i++)
		{
		m_ArmorDamageAdj[i - 1] = g_ArmorDamageAdj[i - 1];
		m_ShieldDamageAdj[i - 1] = g_ShieldDamageAdj[i - 1];
		}

	//	Load constants

	CXMLElement *pConstants = pDesc->GetContentElementByTag(CONSTANTS_TAG);
	if (pConstants)
		{
		for (i = 0; i < pConstants->GetContentElementCount(); i++)
			{
			CXMLElement *pItem = pConstants->GetContentElement(i);

			if (strEquals(pItem->GetTag(), ARMOR_DAMAGE_ADJ_TAG))
				{
				int iLevel = pItem->GetAttributeInteger(LEVEL_ATTRIB);
				if (iLevel < 1 || iLevel > MAX_ITEM_LEVEL)
					{
					Ctx.sError = strPatternSubst(CONSTLIT("Invalid level: %d."), iLevel);
					return ERR_FAIL;
					}

				if (error = m_ArmorDamageAdj[iLevel - 1].InitFromXML(Ctx, pItem, true))
					return error;
				}
			else if (strEquals(pItem->GetTag(), SHIELD_DAMAGE_ADJ_TAG))
				{
				int iLevel = pItem->GetAttributeInteger(LEVEL_ATTRIB);
				if (iLevel < 1 || iLevel > MAX_ITEM_LEVEL)
					{
					Ctx.sError = strPatternSubst(CONSTLIT("Invalid level: %d."), iLevel);
					return ERR_FAIL;
					}

				if (error = m_ShieldDamageAdj[iLevel - 1].InitFromXML(Ctx, pItem, true))
					return error;
				}
			else
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Invalid constant definition element: %s."), pItem->GetTag());
				return ERR_FAIL;
				}
			}
		}

	return NOERROR;
	}
