//	CPlayerSettings.cpp
//
//	CPlayerSettings class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

#define ARMOR_DISPLAY_TAG						CONSTLIT("ArmorDisplay")
#define ARMOR_SECTION_TAG						CONSTLIT("ArmorSection")
#define FUEL_LEVEL_IMAGE_TAG					CONSTLIT("FuelLevelImage")
#define FUEL_LEVEL_TEXT_TAG						CONSTLIT("FuelLevelText")
#define FUEL_LOW_LEVEL_IMAGE_TAG				CONSTLIT("FuelLowLevelImage")
#define IMAGE_TAG								CONSTLIT("Image")
#define POWER_LEVEL_IMAGE_TAG					CONSTLIT("PowerLevelImage")
#define POWER_LEVEL_TEXT_TAG					CONSTLIT("PowerLevelText")
#define REACTOR_DISPLAY_TAG						CONSTLIT("ReactorDisplay")
#define REACTOR_TEXT_TAG						CONSTLIT("ReactorText")
#define SHIELD_DISPLAY_TAG						CONSTLIT("ShieldDisplay")
#define SHIELD_EFFECT_TAG						CONSTLIT("ShieldLevelEffect")
#define SHIP_IMAGE_TAG							CONSTLIT("ShipImage")
#define WEAPON_DISPLAY_TAG						CONSTLIT("WeaponDisplay")

#define AUTOPILOT_ATTRIB						CONSTLIT("autopilot")
#define ARMOR_ID_ATTRIB							CONSTLIT("armorID")
#define DEBUG_ONLY_ATTRIB						CONSTLIT("debugOnly")
#define DESC_ATTRIB								CONSTLIT("desc")
#define DEST_X_ATTRIB							CONSTLIT("destX")
#define DEST_Y_ATTRIB							CONSTLIT("destY")
#define DOCK_SERVICES_SCREEN_ATTRIB				CONSTLIT("dockServicesScreen")
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
#define SEGMENT_ATTRIB							CONSTLIT("segment")
#define SHIELD_EFFECT_ATTRIB					CONSTLIT("shieldLevelEffect")
#define SHIP_SCREEN_ATTRIB						CONSTLIT("shipScreen")
#define STARTING_CREDITS_ATTRIB					CONSTLIT("startingCredits")
#define STARTING_MAP_ATTRIB						CONSTLIT("startingMap")
#define STARTING_POS_ATTRIB						CONSTLIT("startingPos")
#define STARTING_SYSTEM_ATTRIB					CONSTLIT("startingSystem")
#define WIDTH_ATTRIB							CONSTLIT("width")
#define X_ATTRIB								CONSTLIT("x")
#define Y_ATTRIB								CONSTLIT("y")

#define SEGMENT_AFT								CONSTLIT("aft")
#define SEGMENT_AFT_PORT						CONSTLIT("aft-port")
#define SEGMENT_AFT_STARBOARD					CONSTLIT("aft-starboard")
#define SEGMENT_FORE_PORT						CONSTLIT("fore-port")
#define SEGMENT_FORE_STARBOARD					CONSTLIT("fore-starboard")
#define SEGMENT_FORWARD							CONSTLIT("forward")
#define SEGMENT_PORT							CONSTLIT("port")
#define SEGMENT_STARBOARD						CONSTLIT("starboard")

#define ERR_SHIELD_DISPLAY_NEEDED				CONSTLIT("missing or invalid <ShieldDisplay> element")
#define ERR_ARMOR_DISPLAY_NEEDED				CONSTLIT("missing or invalid <ArmorDisplay> element")
#define ERR_REACTOR_DISPLAY_NEEDED				CONSTLIT("missing or invalid <ReactorDisplay> element")
#define ERR_INVALID_STARTING_CREDITS			CONSTLIT("invalid starting credits")
#define ERR_SHIP_IMAGE_NEEDED					CONSTLIT("invalid <ShipImage> element")
#define ERR_MUST_HAVE_SHIP_IMAGE				CONSTLIT("<ShipImage> in <ArmorDisplay> required if using shield level effect")
#define ERR_WEAPON_DISPLAY_NEEDED				CONSTLIT("missing or invalid <WeaponDisplay> element")

ALERROR InitRectFromElement (CXMLElement *pItem, RECT *retRect);

CPlayerSettings &CPlayerSettings::operator= (const CPlayerSettings &Source)

//	CPlayerSettings operator =

	{
	CleanUp();

	m_sDesc = Source.m_sDesc;
	m_dwLargeImage = Source.m_dwLargeImage;

	//	Miscellaneous

	m_StartingCredits = Source.m_StartingCredits;		//	Starting credits
	m_sStartNode = Source.m_sStartNode;						//	Starting node (may be blank)
	m_sStartPos = Source.m_sStartPos;						//	Label of starting position (may be blank)
	m_pShipScreen = Source.m_pShipScreen;			//	Ship screen
	m_pDockServicesScreen = Source.m_pDockServicesScreen;

	//	Armor

	m_fHasArmorDesc = Source.m_fHasArmorDesc;
	m_ArmorDesc = Source.m_ArmorDesc;

	//	Shields

	m_fHasShieldDesc = Source.m_fHasShieldDesc;
	m_ShieldDesc = Source.m_ShieldDesc;

	//	Reactor

	m_fHasReactorDesc = Source.m_fHasReactorDesc;
	m_ReactorDesc = Source.m_ReactorDesc;

	//	Weapon

	m_fHasWeaponDesc = Source.m_fHasWeaponDesc;
	m_WeaponDesc = Source.m_WeaponDesc;

	//	Flags

	m_fInitialClass = Source.m_fInitialClass;					//	Use ship class at game start
	m_fDebug = Source.m_fDebug;
	m_fIncludeInAllAdventures = Source.m_fIncludeInAllAdventures;

	//	Don

	return *this;
	}

void CPlayerSettings::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds types used

	{
	retTypesUsed->SetAt(m_dwLargeImage, true);
	retTypesUsed->SetAt(strToInt(m_pShipScreen.GetUNID(), 0), true);
	retTypesUsed->SetAt(strToInt(m_pDockServicesScreen.GetUNID(), 0), true);

	//	LATER: Add armor images, etc.
	}

ALERROR CPlayerSettings::Bind (SDesignLoadCtx &Ctx, CShipClass *pClass)

//	Bind
//
//	Bind design

	{
	DEBUG_TRY

	ALERROR error;
	int i;

	//	Bind basic stuff

	if (error = m_pShipScreen.Bind(Ctx, pClass->GetLocalScreens()))
		return error;

	if (error = m_pDockServicesScreen.Bind(Ctx, pClass->GetLocalScreens()))
		return error;

	if (error = m_StartingCredits.Bind(Ctx))
		return error;

	//	Armor display

	if (m_fHasArmorDesc)
		{
		if (error = m_ArmorDesc.ShipImage.OnDesignLoadComplete(Ctx))
			return error;

		for (i = 0; i < m_ArmorDesc.Segments.GetCount(); i++)
			{
			SArmorSegmentImageDesc &ArmorDesc = m_ArmorDesc.Segments[i];

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
		if (!m_ShieldDesc.pShieldEffect.IsEmpty())
			{
			if (error = m_ShieldDesc.pShieldEffect.Bind(Ctx))
				return error;
			}
		else
			{
			if (error = m_ShieldDesc.Image.OnDesignLoadComplete(Ctx))
				return error;
			}
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

	//	Weapon

	if (m_fHasWeaponDesc)
		{
		if (error = m_WeaponDesc.Image.OnDesignLoadComplete(Ctx))
			return error;
		}
	else
		m_pWeaponDescInherited = pClass->GetWeaponDescInherited();

	//	Done

	return NOERROR;

	DEBUG_CATCH
	}

void CPlayerSettings::CleanUp (void)

//	CleanUp
//
//	Clean up our structures

	{
	}

ALERROR CPlayerSettings::ComposeLoadError (SDesignLoadCtx &Ctx, const CString &sError)

//	ComposeLoadError
//
//	Composes an error while loading from XML.

	{
	Ctx.sError = sError;
	return ERR_FAIL;
	}

CEffectCreator *CPlayerSettings::FindEffectCreator (const CString &sUNID)

//	FindEffectCreator
//
//	Finds an effect creator of the following form:
//
//	{unid}:p:s
//          ^

	{
	char *pPos = sUNID.GetASCIIZPointer();
	if (*pPos != ':')
		return NULL;
	
	pPos++;

	//	Figure out what

	switch (*pPos)
		{
		case 's':
			return m_ShieldDesc.pShieldEffect;

		default:
			return NULL;
		}
	}

int CPlayerSettings::GetArmorSegment (SDesignLoadCtx &Ctx, CShipClass *pClass, CXMLElement *pDesc)

//	GetArmorSegment
//
//	While loading the armor segment descriptors, returns the segment number for 
//	this descriptor.

	{
	//	For API versions 21 and higher, we figure out the segment version based
	//	on an explicit attribute.

	if (Ctx.GetAPIVersion() >= 21)
		{
		int iSegment = pDesc->GetAttributeIntegerBounded(SEGMENT_ATTRIB, 0, -1, -1);
		return (iSegment != -1 ? iSegment : m_ArmorDesc.Segments.GetCount());
		}

	//	Otherwise, it is based on the order of elements in the XML

	else
		return m_ArmorDesc.Segments.GetCount();
	}

ALERROR CPlayerSettings::InitFromXML (SDesignLoadCtx &Ctx, CShipClass *pClass, CXMLElement *pDesc)

//	InitFromXML
//
//	Initialize from an XML element

	{
	ALERROR error;
	int i;

	m_sDesc = pDesc->GetAttribute(DESC_ATTRIB);
	if (error = LoadUNID(Ctx, pDesc->GetAttribute(LARGE_IMAGE_ATTRIB), &m_dwLargeImage))
		return error;

	m_fDebug = pDesc->GetAttributeBool(DEBUG_ONLY_ATTRIB);
	CString sInitialClass = pDesc->GetAttribute(INITIAL_CLASS_ATTRIB);
	if (strEquals(sInitialClass, CONSTLIT("always")))
		{
		m_fInitialClass = true;
		m_fIncludeInAllAdventures = true;
		}
	else
		{
		m_fInitialClass = CXMLElement::IsBoolTrueValue(sInitialClass);
		m_fIncludeInAllAdventures = false;
		}

	m_fHasArmorDesc = false;
	m_fHasReactorDesc = false;
	m_fHasShieldDesc = false;
	m_fHasWeaponDesc = false;

	//	Load some miscellaneous data

	CString sAttrib;
	if (!pDesc->FindAttribute(STARTING_CREDITS_ATTRIB, &sAttrib))
		sAttrib = CONSTLIT("5d20+200");

	if (error = m_StartingCredits.InitFromXML(Ctx, sAttrib))
		return error;

	if (error = ::LoadUNID(Ctx, pDesc->GetAttribute(STARTING_MAP_ATTRIB), &m_dwStartMap))
		return error;

	m_sStartNode = pDesc->GetAttribute(STARTING_SYSTEM_ATTRIB);
	m_sStartPos = pDesc->GetAttribute(STARTING_POS_ATTRIB);
	if (m_sStartPos.IsBlank())
		m_sStartPos = CONSTLIT("Start");

	//	Load the ship screen

	CString sShipScreenUNID = pDesc->GetAttribute(SHIP_SCREEN_ATTRIB);
	if (sShipScreenUNID.IsBlank())
		sShipScreenUNID = strFromInt(DEFAULT_SHIP_SCREEN_UNID, false);
	m_pShipScreen.LoadUNID(Ctx, sShipScreenUNID);

	//	Load dock services screen

	CString sUNID;
	if (pDesc->FindAttribute(DOCK_SERVICES_SCREEN_ATTRIB, &sUNID))
		m_pDockServicesScreen.LoadUNID(Ctx, sUNID);
	else
		{
		if (pClass->GetAPIVersion() >= 23)
			sUNID = strFromInt(DEFAULT_DOCK_SERVICES_SCREEN, false);
		else
			sUNID = strFromInt(COMPATIBLE_DOCK_SERVICES_SCREEN, false);

		m_pDockServicesScreen.LoadUNID(Ctx, sUNID);
		}

	//	Load the armor display data

	CXMLElement *pArmorDisplay = pDesc->GetContentElementByTag(ARMOR_DISPLAY_TAG);
	if (pArmorDisplay && pArmorDisplay->GetContentElementCount() > 0)
		{
		//	Loop over all sub elements

		for (i = 0; i < pArmorDisplay->GetContentElementCount(); i++)
			{
			CXMLElement *pSub = pArmorDisplay->GetContentElement(i);

			if (strEquals(pSub->GetTag(), ARMOR_SECTION_TAG))
				{
				int iSegment = GetArmorSegment(Ctx, pClass, pSub);
				SArmorSegmentImageDesc &ArmorDesc = *m_ArmorDesc.Segments.SetAt(iSegment);

				if (error = ArmorDesc.Image.InitFromXML(Ctx, pSub))
					return ComposeLoadError(Ctx, ERR_ARMOR_DISPLAY_NEEDED);

				ArmorDesc.sName = pSub->GetAttribute(NAME_ATTRIB);
				ArmorDesc.xDest = pSub->GetAttributeInteger(DEST_X_ATTRIB);
				ArmorDesc.yDest = pSub->GetAttributeInteger(DEST_Y_ATTRIB);
				ArmorDesc.xHP = pSub->GetAttributeInteger(HP_X_ATTRIB);
				ArmorDesc.yHP = pSub->GetAttributeInteger(HP_Y_ATTRIB);
				ArmorDesc.yName = pSub->GetAttributeInteger(NAME_Y_ATTRIB);
				ArmorDesc.cxNameBreak = pSub->GetAttributeInteger(NAME_BREAK_WIDTH);
				ArmorDesc.xNameDestOffset = pSub->GetAttributeInteger(NAME_DEST_X_ATTRIB);
				ArmorDesc.yNameDestOffset = pSub->GetAttributeInteger(NAME_DEST_Y_ATTRIB);
				}
			else if (strEquals(pSub->GetTag(), SHIP_IMAGE_TAG))
				{
				if (error = m_ArmorDesc.ShipImage.InitFromXML(Ctx, pSub))
					return ComposeLoadError(Ctx, ERR_SHIP_IMAGE_NEEDED);
				}
			else
				return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unknown ArmorDisplay element: "), pSub->GetTag()));
			}

		m_fHasArmorDesc = true;
		}
	else
		m_fHasArmorDesc = false;

	//	Load shield display data

	CXMLElement *pShieldDisplay = pDesc->GetContentElementByTag(SHIELD_DISPLAY_TAG);
	if (pShieldDisplay)
		{
		//	Load the new shield effect

		if (error = m_ShieldDesc.pShieldEffect.LoadEffect(Ctx,
				strPatternSubst(CONSTLIT("%d:p:s"), pClass->GetUNID()),
				pShieldDisplay->GetContentElementByTag(SHIELD_EFFECT_TAG),
				pShieldDisplay->GetAttribute(SHIELD_EFFECT_ATTRIB)))
			return error;

		//	If we don't have the new effect, load the backwards compatibility
		//	image.

		if (m_ShieldDesc.pShieldEffect.IsEmpty())
			{
			if (error = m_ShieldDesc.Image.InitFromXML(Ctx, 
					pShieldDisplay->GetContentElementByTag(IMAGE_TAG)))
				return ComposeLoadError(Ctx, ERR_SHIELD_DISPLAY_NEEDED);
			}

		m_fHasShieldDesc = true;
		}

	//	If we have a shield effect then we must have an armor image

	if (!m_ShieldDesc.pShieldEffect.IsEmpty() && m_ArmorDesc.ShipImage.GetBitmapUNID() == 0)
		return ComposeLoadError(Ctx, ERR_MUST_HAVE_SHIP_IMAGE);

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

	//	Load weapon display data

	CXMLElement *pWeaponDisplay = pDesc->GetContentElementByTag(WEAPON_DISPLAY_TAG);
	if (pWeaponDisplay)
		{
		//	Load the image

		if (error = m_WeaponDesc.Image.InitFromXML(Ctx, 
				pWeaponDisplay->GetContentElementByTag(IMAGE_TAG)))
			return ComposeLoadError(Ctx, ERR_WEAPON_DISPLAY_NEEDED);

		m_fHasWeaponDesc = true;
		}

	//	Done

	return NOERROR;
	}

void CPlayerSettings::MergeFrom (const CPlayerSettings &Src)

//	MergeFrom
//
//	Merges from the source

	{
	if (Src.m_fHasArmorDesc)
		{
		m_ArmorDesc = Src.m_ArmorDesc;
		m_fHasArmorDesc = true;
		}

	if (Src.m_fHasReactorDesc)
		{
		m_ReactorDesc = Src.m_ReactorDesc;
		m_fHasReactorDesc = true;
		}

	if (Src.m_fHasShieldDesc)
		{
		m_ShieldDesc = Src.m_ShieldDesc;
		m_fHasShieldDesc = true;
		}

	if (Src.m_fHasWeaponDesc)
		{
		m_WeaponDesc = Src.m_WeaponDesc;
		m_fHasWeaponDesc = true;
		}
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

