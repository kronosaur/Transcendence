//	CPlayerSettings.cpp
//
//	CPlayerSettings class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

#define ARMOR_DISPLAY_TAG						CONSTLIT("ArmorDisplay")
#define FUEL_LEVEL_IMAGE_TAG					CONSTLIT("FuelLevelImage")
#define FUEL_LEVEL_TEXT_TAG						CONSTLIT("FuelLevelText")
#define FUEL_LOW_LEVEL_IMAGE_TAG				CONSTLIT("FuelLowLevelImage")
#define IMAGE_TAG								CONSTLIT("Image")
#define POWER_LEVEL_IMAGE_TAG					CONSTLIT("PowerLevelImage")
#define POWER_LEVEL_TEXT_TAG					CONSTLIT("PowerLevelText")
#define REACTOR_DISPLAY_TAG						CONSTLIT("ReactorDisplay")
#define REACTOR_TEXT_TAG						CONSTLIT("ReactorText")
#define SHIELD_DISPLAY_TAG						CONSTLIT("ShieldDisplay")

#define AUTOPILOT_ATTRIB						CONSTLIT("autopilot")
#define ARMOR_ID_ATTRIB							CONSTLIT("armorID")
#define DEBUG_ONLY_ATTRIB						CONSTLIT("debugOnly")
#define DESC_ATTRIB								CONSTLIT("desc")
#define DEST_X_ATTRIB							CONSTLIT("destX")
#define DEST_Y_ATTRIB							CONSTLIT("destY")
#define HEIGHT_ATTRIB							CONSTLIT("height")
#define HP_X_ATTRIB								CONSTLIT("hpX")
#define HP_Y_ATTRIB								CONSTLIT("hpY")
#define INITIAL_CLASS_ATTRIB					CONSTLIT("initialClass")
#define LARGE_IMAGE_ATTRIB						CONSTLIT("largeImage")
#define NAME_ATTRIB								CONSTLIT("name")
#define NAME_BREAK_WIDTH						CONSTLIT("nameBreakWidth")
#define NAME_DEST_X_ATTRIB						CONSTLIT("nameDestX")
#define NAME_DEST_Y_ATTRIB						CONSTLIT("nameDestY")
#define NAME_Y_ATTRIB							CONSTLIT("nameY")
#define SHIP_SCREEN_ATTRIB						CONSTLIT("shipScreen")
#define STARTING_CREDITS_ATTRIB					CONSTLIT("startingCredits")
#define STARTING_POS_ATTRIB						CONSTLIT("startingPos")
#define STARTING_SYSTEM_ATTRIB					CONSTLIT("startingSystem")
#define WIDTH_ATTRIB							CONSTLIT("width")
#define X_ATTRIB								CONSTLIT("x")
#define Y_ATTRIB								CONSTLIT("y")

#define ERR_SHIELD_DISPLAY_NEEDED				CONSTLIT("missing or invalid <ShieldDisplay> element")
#define ERR_ARMOR_DISPLAY_NEEDED				CONSTLIT("missing or invalid <ArmorDisplay> element")
#define ERR_REACTOR_DISPLAY_NEEDED				CONSTLIT("missing or invalid <ReactorDisplay> element")
#define ERR_INVALID_STARTING_CREDITS			CONSTLIT("invalid starting credits")

ALERROR InitRectFromElement (CXMLElement *pItem, RECT *retRect);

CPlayerSettings &CPlayerSettings::operator= (const CPlayerSettings &Source)

//	CPlayerSettings operator =

	{
	int i;

	CleanUp();

	m_sDesc = Source.m_sDesc;
	m_dwLargeImage = Source.m_dwLargeImage;

	//	Miscellaneous

	m_StartingCredits = Source.m_StartingCredits;		//	Starting credits
	m_sStartNode = Source.m_sStartNode;						//	Starting node (may be blank)
	m_sStartPos = Source.m_sStartPos;						//	Label of starting position (may be blank)
	m_pShipScreen = Source.m_pShipScreen;			//	Ship screen

	//	Armor

	if (Source.m_pArmorDesc)
		{
		m_iArmorDescCount = Source.m_iArmorDescCount;
		m_pArmorDesc = new SArmorImageDesc [Source.m_iArmorDescCount];

		for (i = 0; i < Source.m_iArmorDescCount; i++)
			m_pArmorDesc[i] = Source.m_pArmorDesc[i];
		}
	else
		{
		m_fHasArmorDesc = false;
		m_iArmorDescCount = 0;
		m_pArmorDesc = NULL;
		}

	//	Shields

	m_fHasShieldDesc = Source.m_fHasShieldDesc;
	m_ShieldDesc = Source.m_ShieldDesc;

	//	Reactor

	m_fHasReactorDesc = Source.m_fHasReactorDesc;
	m_ReactorDesc = Source.m_ReactorDesc;

	//	Flags

	m_fInitialClass = Source.m_fInitialClass;					//	Use ship class at game start
	m_fDebug = Source.m_fDebug;
	m_fAutopilot = Source.m_fAutopilot;

	//	Don

	return *this;
	}

ALERROR CPlayerSettings::Bind (SDesignLoadCtx &Ctx, CShipClass *pClass)

//	Bind
//
//	Bind design

	{
	ALERROR error;
	int i;

	//	Bind basic stuff

	if (error = m_pShipScreen.Bind(Ctx, pClass->GetLocalScreens()))
		return error;

	if (error = m_StartingCredits.Bind(Ctx))
		return error;

	//	Armor display

	if (m_fHasArmorDesc)
		{
		for (i = 0; i < m_iArmorDescCount; i++)
			{
			SArmorImageDesc &ArmorDesc = m_pArmorDesc[i];

			if (error = ArmorDesc.Image.OnDesignLoadComplete(Ctx))
				return error;
			}
		}
	else if (m_pArmorDescInherited = pClass->GetArmorDescInherited())
		;
	else
		return ComposeLoadError(Ctx, ERR_ARMOR_DISPLAY_NEEDED);

	//	Shields

	if (m_fHasShieldDesc)
		{
		if (error = m_ShieldDesc.Image.OnDesignLoadComplete(Ctx))
			return error;
		}
	else if (m_pShieldDescInherited = pClass->GetShieldDescInherited())
		;
	else
		return ComposeLoadError(Ctx, ERR_SHIELD_DISPLAY_NEEDED);

	//	Reactor

	if (m_fHasReactorDesc)
		{
		if (error = m_ReactorDesc.ReactorImage.OnDesignLoadComplete(Ctx))
			return error;

		if (error = m_ReactorDesc.PowerLevelImage.OnDesignLoadComplete(Ctx))
			return error;

		if (error = m_ReactorDesc.FuelLevelImage.OnDesignLoadComplete(Ctx))
			return error;

		if (error = m_ReactorDesc.FuelLowLevelImage.OnDesignLoadComplete(Ctx))
			return error;
		}
	else if (m_pReactorDescInherited = pClass->GetReactorDescInherited())
		;
	else
		return ComposeLoadError(Ctx, ERR_REACTOR_DISPLAY_NEEDED);

	//	Done

	return NOERROR;
	}

void CPlayerSettings::CleanUp (void)

//	CleanUp
//
//	Clean up our structures

	{
	if (m_pArmorDesc)
		{
		delete [] m_pArmorDesc;
		m_pArmorDesc = NULL;
		}
	}

ALERROR CPlayerSettings::ComposeLoadError (SDesignLoadCtx &Ctx, const CString &sError)

//	ComposeLoadError
//
//	Composes an error while loading from XML.

	{
	Ctx.sError = sError;
	return ERR_FAIL;
	}

ALERROR CPlayerSettings::InitFromXML (SDesignLoadCtx &Ctx, CShipClass *pClass, CXMLElement *pDesc)

//	InitFromXML
//
//	Initialize from an XML element

	{
	ALERROR error;
	int i;

	m_sDesc = pDesc->GetAttribute(DESC_ATTRIB);
	m_dwLargeImage = LoadUNID(Ctx, pDesc->GetAttribute(LARGE_IMAGE_ATTRIB));
	m_fDebug = pDesc->GetAttributeBool(DEBUG_ONLY_ATTRIB);
	m_fInitialClass = pDesc->GetAttributeBool(INITIAL_CLASS_ATTRIB);

	m_fHasArmorDesc = false;
	m_fHasReactorDesc = false;
	m_fHasShieldDesc = false;

	//	Some ship capabilities

	bool bValue;
	if (pDesc->FindAttributeBool(AUTOPILOT_ATTRIB, &bValue))
		m_fAutopilot = bValue;
	else
		m_fAutopilot = true;

	//	Load some miscellaneous data

	CString sAttrib;
	if (!pDesc->FindAttribute(STARTING_CREDITS_ATTRIB, &sAttrib))
		sAttrib = CONSTLIT("5d20+200");

	if (error = m_StartingCredits.InitFromXML(Ctx, sAttrib))
		return error;

	m_sStartNode = pDesc->GetAttribute(STARTING_SYSTEM_ATTRIB);
	m_sStartPos = pDesc->GetAttribute(STARTING_POS_ATTRIB);
	if (m_sStartPos.IsBlank())
		m_sStartPos = CONSTLIT("Start");

	//	Load the ship screen

	CString sShipScreenUNID = pDesc->GetAttribute(SHIP_SCREEN_ATTRIB);
	if (sShipScreenUNID.IsBlank())
		sShipScreenUNID = strFromInt(DEFAULT_SHIP_SCREEN_UNID, FALSE);
	m_pShipScreen.LoadUNID(Ctx, sShipScreenUNID);

	//	Load the armor display data

	CXMLElement *pArmorDisplay = pDesc->GetContentElementByTag(ARMOR_DISPLAY_TAG);
	if (pArmorDisplay && pArmorDisplay->GetContentElementCount() > 0)
		{
		m_iArmorDescCount = pArmorDisplay->GetContentElementCount();
		m_pArmorDesc = new SArmorImageDesc [m_iArmorDescCount];
		for (i = 0; i < m_iArmorDescCount; i++)
			{
			SArmorImageDesc &ArmorDesc = m_pArmorDesc[i];
			CXMLElement *pSegment = pArmorDisplay->GetContentElement(i);

			if (error = ArmorDesc.Image.InitFromXML(Ctx, pSegment))
				return ComposeLoadError(Ctx, ERR_ARMOR_DISPLAY_NEEDED);

			ArmorDesc.sName = pSegment->GetAttribute(NAME_ATTRIB);
			ArmorDesc.xDest = pSegment->GetAttributeInteger(DEST_X_ATTRIB);
			ArmorDesc.yDest = pSegment->GetAttributeInteger(DEST_Y_ATTRIB);
			ArmorDesc.xHP = pSegment->GetAttributeInteger(HP_X_ATTRIB);
			ArmorDesc.yHP = pSegment->GetAttributeInteger(HP_Y_ATTRIB);
			ArmorDesc.yName = pSegment->GetAttributeInteger(NAME_Y_ATTRIB);
			ArmorDesc.cxNameBreak = pSegment->GetAttributeInteger(NAME_BREAK_WIDTH);
			ArmorDesc.xNameDestOffset = pSegment->GetAttributeInteger(NAME_DEST_X_ATTRIB);
			ArmorDesc.yNameDestOffset = pSegment->GetAttributeInteger(NAME_DEST_Y_ATTRIB);
			}

		m_fHasArmorDesc = true;
		}
	else
		{
		m_fHasArmorDesc = false;
		m_iArmorDescCount = 0;
		m_pArmorDesc = NULL;
		}

	//	Load shield display data

	CXMLElement *pShieldDisplay = pDesc->GetContentElementByTag(SHIELD_DISPLAY_TAG);
	if (pShieldDisplay)
		{
		if (error = m_ShieldDesc.Image.InitFromXML(Ctx, 
				pShieldDisplay->GetContentElementByTag(IMAGE_TAG)))
			return ComposeLoadError(Ctx, ERR_SHIELD_DISPLAY_NEEDED);

		m_fHasShieldDesc = true;
		}

	//	Load reactor display data

	CXMLElement *pReactorDisplay = pDesc->GetContentElementByTag(REACTOR_DISPLAY_TAG);
	if (pReactorDisplay)
		{
		if (error = m_ReactorDesc.ReactorImage.InitFromXML(Ctx,
				pReactorDisplay->GetContentElementByTag(IMAGE_TAG)))
			return ComposeLoadError(Ctx, ERR_REACTOR_DISPLAY_NEEDED);

		CXMLElement *pImage = pReactorDisplay->GetContentElementByTag(POWER_LEVEL_IMAGE_TAG);
		if (pImage == NULL || (error = m_ReactorDesc.PowerLevelImage.InitFromXML(Ctx, pImage)))
			return ComposeLoadError(Ctx, ERR_REACTOR_DISPLAY_NEEDED);

		m_ReactorDesc.xPowerLevelImage = pImage->GetAttributeInteger(DEST_X_ATTRIB);
		m_ReactorDesc.yPowerLevelImage = pImage->GetAttributeInteger(DEST_Y_ATTRIB);

		pImage = pReactorDisplay->GetContentElementByTag(FUEL_LEVEL_IMAGE_TAG);
		if (pImage == NULL || (error = m_ReactorDesc.FuelLevelImage.InitFromXML(Ctx, pImage)))
			return ComposeLoadError(Ctx, ERR_REACTOR_DISPLAY_NEEDED);

		m_ReactorDesc.xFuelLevelImage = pImage->GetAttributeInteger(DEST_X_ATTRIB);
		m_ReactorDesc.yFuelLevelImage = pImage->GetAttributeInteger(DEST_Y_ATTRIB);

		pImage = pReactorDisplay->GetContentElementByTag(FUEL_LOW_LEVEL_IMAGE_TAG);
		if (pImage == NULL || (error = m_ReactorDesc.FuelLowLevelImage.InitFromXML(Ctx, pImage)))
			return ComposeLoadError(Ctx, ERR_REACTOR_DISPLAY_NEEDED);

		if (error = InitRectFromElement(pReactorDisplay->GetContentElementByTag(REACTOR_TEXT_TAG),
				&m_ReactorDesc.rcReactorText))
			return ComposeLoadError(Ctx, ERR_REACTOR_DISPLAY_NEEDED);

		if (error = InitRectFromElement(pReactorDisplay->GetContentElementByTag(POWER_LEVEL_TEXT_TAG),
				&m_ReactorDesc.rcPowerLevelText))
			return ComposeLoadError(Ctx, ERR_REACTOR_DISPLAY_NEEDED);

		if (error = InitRectFromElement(pReactorDisplay->GetContentElementByTag(FUEL_LEVEL_TEXT_TAG),
				&m_ReactorDesc.rcFuelLevelText))
			return ComposeLoadError(Ctx, ERR_REACTOR_DISPLAY_NEEDED);

		m_fHasReactorDesc = true;
		}

	//	Done

	return NOERROR;
	}

ALERROR InitRectFromElement (CXMLElement *pItem, RECT *retRect)

//	InitRectFromElement
//
//	Initializes the rect from x, y, width, height attributes

	{
	if (pItem == NULL)
		return ERR_FAIL;

	retRect->left = pItem->GetAttributeInteger(X_ATTRIB);
	retRect->top = pItem->GetAttributeInteger(Y_ATTRIB);
	retRect->right = retRect->left + pItem->GetAttributeInteger(WIDTH_ATTRIB);
	retRect->bottom = retRect->top + pItem->GetAttributeInteger(HEIGHT_ATTRIB);

	return NOERROR;
	}

