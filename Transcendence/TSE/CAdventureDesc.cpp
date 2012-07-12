//	CAdventureDesc.cpp
//
//	CAdventureDesc class

#include "PreComp.h"

#define EVENTS_TAG								CONSTLIT("Events")

#define ADVENTURE_UNID_ATTRIB					CONSTLIT("adventureUNID")
#define BACKGROUND_ID_ATTRIB					CONSTLIT("backgroundID")
#define NAME_ATTRIB								CONSTLIT("name")
#define STARTING_POS_ATTRIB						CONSTLIT("startingPos")
#define STARTING_SHIP_CRITERIA_ATTRIB			CONSTLIT("startingShipCriteria")
#define STARTING_SYSTEM_ATTRIB					CONSTLIT("startingSystem")
#define WELCOME_MESSAGE_ATTRIB					CONSTLIT("welcomeMessage")

#define ON_GAME_START_EVENT						CONSTLIT("OnGameStart")
#define ON_GAME_END_EVENT						CONSTLIT("OnGameEnd")

#define FIELD_NAME								CONSTLIT("name")

#define ERR_STARTING_SHIP_CRITERIA				CONSTLIT("Unable to parse startingShipCriteria")

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

	//	Starting position

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

	return NOERROR;
	}
