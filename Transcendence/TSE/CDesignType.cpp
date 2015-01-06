//	CDesignType.cpp
//
//	CDesignType class

#include "PreComp.h"

#define ADVENTURE_DESC_TAG						CONSTLIT("AdventureDesc")
#define ATTRIBUTE_DESC_TAG						CONSTLIT("AttributeDesc")
#define DISPLAY_ATTRIBUTES_TAG					CONSTLIT("DisplayAttributes")
#define DOCK_SCREEN_TAG							CONSTLIT("DockScreen")
#define DOCK_SCREENS_TAG						CONSTLIT("DockScreens")
#define ECONOMY_TYPE_TAG						CONSTLIT("EconomyType")
#define EFFECT_TAG								CONSTLIT("Effect")
#define EFFECT_TYPE_TAG							CONSTLIT("EffectType")
#define ENCOUNTER_TABLE_TAG						CONSTLIT("EncounterTable")
#define EVENTS_TAG								CONSTLIT("Events")
#define GLOBAL_DATA_TAG							CONSTLIT("GlobalData")
#define IMAGE_TAG								CONSTLIT("Image")
#define INITIAL_DATA_TAG						CONSTLIT("InitialData")
#define ITEM_TABLE_TAG							CONSTLIT("ItemTable")
#define ITEM_TYPE_TAG							CONSTLIT("ItemType")
#define LANGUAGE_TAG							CONSTLIT("Language")
#define LOOKUP_TAG								CONSTLIT("Lookup")
#define MISSION_TYPE_TAG						CONSTLIT("MissionType")
#define OVERLAY_TYPE_TAG						CONSTLIT("OverlayType")
#define POWER_TAG								CONSTLIT("Power")
#define SHIP_CLASS_TAG							CONSTLIT("ShipClass")
#define SHIP_CLASS_OVERRIDE_TAG					CONSTLIT("ShipClassOverride")
#define SHIP_ENERGY_FIELD_TYPE_TAG				CONSTLIT("ShipEnergyFieldType")
#define SHIP_TABLE_TAG							CONSTLIT("ShipTable")
#define SOUNDTRACK_TAG							CONSTLIT("Soundtrack")
#define SOVEREIGN_TAG							CONSTLIT("Sovereign")
#define SPACE_ENVIRONMENT_TYPE_TAG				CONSTLIT("SpaceEnvironmentType")
#define STATIC_DATA_TAG							CONSTLIT("StaticData")
#define STATION_TYPE_TAG						CONSTLIT("StationType")
#define SYSTEM_MAP_TAG							CONSTLIT("SystemMap")
#define SYSTEM_FRAGMENT_TABLE_TAG				CONSTLIT("SystemPartTable")
#define SYSTEM_TYPE_TAG							CONSTLIT("SystemType")
#define TEMPLATE_TYPE_TAG						CONSTLIT("TemplateType")
#define TYPE_TAG								CONSTLIT("Type")

#define ATTRIBUTES_ATTRIB						CONSTLIT("attributes")
#define EFFECT_ATTRIB							CONSTLIT("effect")
#define INHERIT_ATTRIB							CONSTLIT("inherit")
#define MODIFIERS_ATTRIB						CONSTLIT("modifiers")
#define UNID_ATTRIB								CONSTLIT("UNID")

#define GET_CREATE_POS_EVENT					CONSTLIT("GetCreatePos")
#define GET_GLOBAL_ACHIEVEMENTS_EVENT			CONSTLIT("GetGlobalAchievements")
#define GET_GLOBAL_DOCK_SCREEN_EVENT			CONSTLIT("GetGlobalDockScreen")
#define GET_GLOBAL_PLAYER_PRICE_ADJ_EVENT		CONSTLIT("GetGlobalPlayerPriceAdj")
#define GET_GLOBAL_RESURRECT_POTENTIAL_EVENT	CONSTLIT("GetGlobalResurrectPotential")
#define ON_GLOBAL_MARK_IMAGES_EVENT				CONSTLIT("OnGlobalMarkImages")
#define ON_GLOBAL_OBJ_DESTROYED_EVENT			CONSTLIT("OnGlobalObjDestroyed")
#define ON_GLOBAL_DOCK_PANE_INIT_EVENT			CONSTLIT("OnGlobalPaneInit")
#define ON_GLOBAL_PLAYER_CHANGED_SHIPS_EVENT	CONSTLIT("OnGlobalPlayerChangedShips")
#define ON_GLOBAL_PLAYER_ENTERED_SYSTEM_EVENT	CONSTLIT("OnGlobalPlayerEnteredSystem")
#define ON_GLOBAL_PLAYER_LEFT_SYSTEM_EVENT		CONSTLIT("OnGlobalPlayerLeftSystem")
#define ON_GLOBAL_RESURRECT_EVENT				CONSTLIT("OnGlobalResurrect")
#define ON_GLOBAL_TOPOLOGY_CREATED_EVENT		CONSTLIT("OnGlobalTopologyCreated")
#define ON_GLOBAL_SYSTEM_CREATED_EVENT			CONSTLIT("OnGlobalSystemCreated")
#define ON_GLOBAL_SYSTEM_STARTED_EVENT			CONSTLIT("OnGlobalSystemStarted")
#define ON_GLOBAL_SYSTEM_STOPPED_EVENT			CONSTLIT("OnGlobalSystemStopped")
#define ON_GLOBAL_UNIVERSE_CREATED_EVENT		CONSTLIT("OnGlobalUniverseCreated")
#define ON_GLOBAL_UNIVERSE_LOAD_EVENT			CONSTLIT("OnGlobalUniverseLoad")
#define ON_GLOBAL_UNIVERSE_SAVE_EVENT			CONSTLIT("OnGlobalUniverseSave")
#define ON_GLOBAL_UPDATE_EVENT					CONSTLIT("OnGlobalUpdate")
#define ON_RANDOM_ENCOUNTER_EVENT				CONSTLIT("OnRandomEncounter")

#define SPECIAL_EVENT							CONSTLIT("event:")
#define SPECIAL_EXTENSION						CONSTLIT("extension:")
#define SPECIAL_UNID							CONSTLIT("unid:")

#define PROPERTY_API_VERSION					CONSTLIT("apiVersion")
#define PROPERTY_CLASS							CONSTLIT("class")
#define PROPERTY_EXTENSION						CONSTLIT("extension")

#define FIELD_EXTENSION_UNID					CONSTLIT("extensionUNID")
#define FIELD_NAME								CONSTLIT("name")
#define FIELD_UNID								CONSTLIT("unid")
#define FIELD_VERSION							CONSTLIT("version")

#define FIELD_ATTRIB_PREFIX						CONSTLIT("attrib-")

static char DESIGN_CHAR[designCount] =
	{
		'i',
		'b',
		's',
		'f',
		'y',
		't',
		'v',
		'd',
		'c',
		'p',

		'e',
		'h',
		'a',
		'g',
		'm',
		'u',
		'n',
		'q',
		'z',
		'w',

		'$',
		'_',
		'x',
	};

static char *DESIGN_CLASS_NAME[designCount] =
	{
		"ItemType",
		"ItemTable",
		"ShipClass",
		"OverlayType",
		"SystemType",
		"StationType",
		"Sovereign",
		"DockScreen",
		"EffectType",
		"Power",

		"SpaceEnvironment",
		"ShipTable",
		"AdventureDesc",
		"Globals",
		"Image",
		"Sound",
		"MissionType",
		"SystemTable",
		"SystemMap",
		"NameGenerator",

		"EconomyType",
		"TemplateType",
		"Type",
	};

static char *CACHED_EVENTS[CDesignType::evtCount] =
	{
		"CanInstallItem",
		"CanRemoveItem",
		"OnGlobalTypesInit",
		"OnObjDestroyed",
		"OnSystemObjAttacked",
		"OnSystemWeaponFire",
	};

CString ParseAchievementSection (ICCItem *pItem);
CString ParseAchievementSort (ICCItem *pItem);
CString ParseAchievementValue (ICCItem *pItem);

CDesignType::CDesignType (void) : 
		m_dwUNID(0), 
		m_pExtension(NULL),
		m_pXML(NULL),
		m_pLocalScreens(NULL), 
		m_dwInheritFrom(0), 
		m_pInheritFrom(NULL),
		m_bIsModification(false),
		m_bIsClone(false)
	{
	utlMemSet(m_EventsCache, sizeof(m_EventsCache), 0);
	}

CDesignType::~CDesignType (void)

//	CDesignType destructor

	{
	if (m_pLocalScreens)
		delete m_pLocalScreens;
	}

void CDesignType::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds types used by this type to the list

	{
	if (m_dwInheritFrom)
		retTypesUsed->SetAt(m_dwInheritFrom, true);

	OnAddTypesUsed(retTypesUsed);
	}

ALERROR CDesignType::BindDesign (SDesignLoadCtx &Ctx)

//	BindDesign
//
//	Bind design elements
	
	{
	ALERROR error;
	int i;

	//	Now that we've connected to our based classes, update the event cache
	//	with events from our ancestors.

	if (m_pInheritFrom)
		{
		for (i = 0; i < evtCount; i++)
			{
			if (m_EventsCache[i].pCode == NULL)
				{
				SEventHandlerDesc *pInherit = m_pInheritFrom->GetInheritedCachedEvent((ECachedHandlers)i);
				if (pInherit)
					m_EventsCache[i] = *pInherit;
				}
			}

		//	Update the language block with data from our ancestors

		m_pInheritFrom->MergeLanguageTo(m_Language);
		}

	//	Type-specific

	try
		{
		error = OnBindDesign(Ctx);
		}
	catch (...)
		{
		::kernelDebugLogMessage("Crash in OnBindDesign [UNID: %08x]", m_dwUNID);
		throw;
		}

	return error;
	}

ALERROR CDesignType::ComposeLoadError (SDesignLoadCtx &Ctx, const CString &sError)

//	ComposeLoadError
//
//	Sets Ctx.sError appropriately and returns ERR_FAIL

	{
	Ctx.sError = strPatternSubst("%s (%x): %s", GetTypeName(), GetUNID(), sError);
	return ERR_FAIL;
	}

void CDesignType::CreateClone (CDesignType **retpType)

//	CreateClone
//
//	Creates a clone of this type. Caller is responsible for freeing it.

	{
	CDesignType *pClone;

	switch (GetType())
		{
		case designItemType:
			pClone = new CItemType;
			break;

		case designItemTable:
			pClone = new CItemTable;
			break;

		case designShipClass:
			pClone = new CShipClass;
			break;

		case designEnergyFieldType:
			pClone = new COverlayType;
			break;

		case designSystemType:
			pClone = new CSystemType;
			break;

		case designStationType:
			pClone = new CStationType;
			break;

		case designSovereign:
			pClone = new CSovereign;
			break;

		case designDockScreen:
			pClone = new CDockScreenType;
			break;

		case designEffectType:
			{
			CEffectCreator *pEffectType = CEffectCreator::AsType(this);
			if (pEffectType == NULL)
				{
				ASSERT(false);
				return;
				}

			CEffectCreator *pEffectClone;
			if (CEffectCreator::CreateFromTag(pEffectType->GetTag(), &pEffectClone) != NOERROR)
				{
				ASSERT(false);
				return;
				}

			pClone = pEffectClone;
			break;
			}

		case designPower:
			pClone = new CPower;
			break;

		case designSpaceEnvironmentType:
			pClone = new CSpaceEnvironmentType;
			break;

		case designShipTable:
			pClone = new CShipTable;
			break;

		case designAdventureDesc:
			pClone = new CAdventureDesc;
			break;

		case designImage:
			pClone = new CObjectImage;
			break;

		case designSound:
			pClone = new CSoundType;
			break;

		case designMissionType:
			pClone = new CMissionType;
			break;

		case designSystemTable:
			pClone = new CSystemTable;
			break;

		case designSystemMap:
			pClone = new CSystemMap;
			break;

		case designEconomyType:
			pClone = new CEconomyType;
			break;

		case designTemplateType:
			pClone = new CTemplateType;
			break;

		case designGenericType:
			pClone = new CGenericType;
			break;

		default:
			ASSERT(false);
			return;
		}

	//	Initialize

	pClone->m_bIsClone = true;

	pClone->m_dwUNID = m_dwUNID;
	pClone->m_pExtension = m_pExtension;
	pClone->m_dwVersion = m_dwVersion;
	pClone->m_dwInheritFrom = m_dwInheritFrom;
	pClone->m_pInheritFrom = m_pInheritFrom;
	pClone->m_sAttributes = m_sAttributes;
	pClone->m_StaticData = m_StaticData;
	pClone->m_GlobalData = m_GlobalData;
	pClone->m_InitGlobalData = m_InitGlobalData;
	pClone->m_Language = m_Language;
	pClone->m_Events = m_Events;
	pClone->m_pLocalScreens = (m_pLocalScreens ? m_pLocalScreens->OrphanCopy() : NULL);
	pClone->m_DisplayAttribs = m_DisplayAttribs;

	//	Let our subclass initialize

	pClone->OnInitFromClone(this);

	//	Done

	*retpType = pClone;
	}

ALERROR CDesignType::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CDesignType **retpType)

//	CreateFromXML
//
//	Creates a design type from an XML element

	{
	try
		{
		ALERROR error;
		CDesignType *pType = NULL;
		bool bOverride = false;

		if (strEquals(pDesc->GetTag(), ITEM_TYPE_TAG))
			pType = new CItemType;
		else if (strEquals(pDesc->GetTag(), ITEM_TABLE_TAG))
			pType = new CItemTable;
		else if (strEquals(pDesc->GetTag(), SHIP_CLASS_TAG))
			pType = new CShipClass;
		else if (strEquals(pDesc->GetTag(), SHIP_ENERGY_FIELD_TYPE_TAG))
			pType = new COverlayType;
		else if (strEquals(pDesc->GetTag(), MISSION_TYPE_TAG))
			pType = new CMissionType;
		else if (strEquals(pDesc->GetTag(), OVERLAY_TYPE_TAG))
			pType = new COverlayType;
		else if (strEquals(pDesc->GetTag(), SYSTEM_TYPE_TAG))
			pType = new CSystemType;
		else if (strEquals(pDesc->GetTag(), STATION_TYPE_TAG))
			pType = new CStationType;
		else if (strEquals(pDesc->GetTag(), SOUNDTRACK_TAG))
			pType = new CSoundType;
		else if (strEquals(pDesc->GetTag(), SOVEREIGN_TAG))
			pType = new CSovereign;
		else if (strEquals(pDesc->GetTag(), DOCK_SCREEN_TAG))
			pType = new CDockScreenType;
		else if (strEquals(pDesc->GetTag(), POWER_TAG))
			pType = new CPower;
		else if (strEquals(pDesc->GetTag(), SPACE_ENVIRONMENT_TYPE_TAG))
			pType = new CSpaceEnvironmentType;
		else if (strEquals(pDesc->GetTag(), ENCOUNTER_TABLE_TAG))
			pType = new CShipTable;
		else if (strEquals(pDesc->GetTag(), SHIP_TABLE_TAG))
			pType = new CShipTable;
		else if (strEquals(pDesc->GetTag(), SYSTEM_FRAGMENT_TABLE_TAG))
			pType = new CSystemTable;
		else if (strEquals(pDesc->GetTag(), SYSTEM_MAP_TAG))
			pType = new CSystemMap;
		else if (strEquals(pDesc->GetTag(), IMAGE_TAG))
			pType = new CObjectImage;
		else if (strEquals(pDesc->GetTag(), ECONOMY_TYPE_TAG))
			pType = new CEconomyType;
		else if (strEquals(pDesc->GetTag(), TEMPLATE_TYPE_TAG))
			pType = new CTemplateType;
		else if (strEquals(pDesc->GetTag(), TYPE_TAG))
			pType = new CGenericType;
		else if (strEquals(pDesc->GetTag(), ADVENTURE_DESC_TAG))
			{
			//	Only valid if we are inside an Adventure

			if (Ctx.pExtension != NULL && Ctx.pExtension->GetType() != extAdventure)
				{
				Ctx.sError = CONSTLIT("<AdventureDesc> element is only valid for Adventures");
				return ERR_FAIL;
				}

			pType = new CAdventureDesc;
			}
		else if (strEquals(pDesc->GetTag(), EFFECT_TAG))
			{
			//	This is an old-style CEffectCreator for compatibility

			if (error = CEffectCreator::CreateFromXML(Ctx, pDesc, NULL_STR, (CEffectCreator **)&pType))
				return error;

			//	Load UNID

			if (error = ::LoadUNID(Ctx, pDesc->GetAttribute(UNID_ATTRIB), &pType->m_dwUNID))
				return error;

			if (!pDesc->FindAttribute(ATTRIBUTES_ATTRIB, &pType->m_sAttributes))
				pType->m_sAttributes = pDesc->GetAttribute(MODIFIERS_ATTRIB);

			if (retpType)
				*retpType = pType;

			//	We skip the normal initialization for backwards compatibility

			return NOERROR;
			}
		else if (strEquals(pDesc->GetTag(), EFFECT_TYPE_TAG))
			{
			//	This is a full effect type. The actual class depends on the content

			if (error = CEffectCreator::CreateTypeFromXML(Ctx, pDesc, (CEffectCreator **)&pType))
				return error;
			}
		else if (strEquals(pDesc->GetTag(), SHIP_CLASS_OVERRIDE_TAG))
			{
			pType = new CShipClass;
			bOverride = true;
			}
		else
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unknown design element: <%s>"), pDesc->GetTag());
			return ERR_FAIL;
			}

		//	Result

		if (retpType)
			*retpType = pType;

		//	Initialize

		return pType->InitFromXML(Ctx, pDesc, bOverride);
		}
	catch (...)
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Crash loading: %x"), pDesc->GetAttributeInteger(CONSTLIT("UNID")));
		return ERR_FAIL;
		}
	}

bool CDesignType::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Subclasses should call this method when they do not know the field.
//	[Normally we would do an OnFindDataField pattern, but we're too lazy
//	to change all the instances.]

	{
	int i;

	if (strEquals(sField, FIELD_EXTENSION_UNID))
		*retsValue = strPatternSubst("0x%08x", (m_pExtension ? m_pExtension->GetUNID() : 0));
	else if (strEquals(sField, FIELD_NAME))
		*retsValue = strPatternSubst("%s 0x%08x", GetTypeClassName(), m_dwUNID);
	else if (strEquals(sField, FIELD_UNID))
		*retsValue = strPatternSubst("0x%08x", m_dwUNID);
	else if (strEquals(sField, FIELD_VERSION))
		{
		//	Starting in version 12 we start numbering api versions independently 
		//	of release version.

		if (m_dwVersion >= 12)
			*retsValue = strFromInt(m_dwVersion);
		else
			*retsValue = strPatternSubst("%d", ExtensionVersionToInteger(m_dwVersion));
		}
	else if (strStartsWith(sField, FIELD_ATTRIB_PREFIX))
		{
		CString sParam = strSubString(sField, FIELD_ATTRIB_PREFIX.GetLength());

		//	Parse into a list of attributes

		TArray<CString> Attribs;
		char *pPos = sParam.GetASCIIZPointer();
		char *pStart = pPos;
		while (true)
			{
			if (*pPos == '\0' || *pPos == '-')
				{
				CString sAttrib = CString(pStart, (int)(pPos - pStart));
				if (!sAttrib.IsBlank())
					Attribs.Insert(sAttrib);

				if (*pPos == '\0')
					break;
				else
					pStart = pPos + 1;
				}

			pPos++;
			}

		//	See which attribute we have

		CString sValue;
		for (i = 0; i < Attribs.GetCount(); i++)
			if (HasAttribute(Attribs[i]))
				{
				if (sValue.IsBlank())
					sValue = Attribs[i];
				else
					sValue = strPatternSubst(CONSTLIT("%s, %s"), sValue, Attribs[i]);
				}

		//	If not found, return false

		if (sValue.IsBlank())
			return false;

		*retsValue = sValue;
		}
	else
		return false;

	return true;
	}

bool CDesignType::FindEventHandler (const CString &sEvent, SEventHandlerDesc *retEvent) const

//	FindEventHandler
//
//	Returns an event handler

	{
	//	Ask subclasses

	if (OnFindEventHandler(sEvent, retEvent))
		return true;

	//	If we have it, great

	ICCItem *pCode;
	if (m_Events.FindEvent(sEvent, &pCode))
		{
		if (retEvent)
			{
			retEvent->pExtension = m_pExtension;
			retEvent->pCode = pCode;
			}

		return true;
		}

	//	Otherwise, see if we inherit

	if (m_pInheritFrom)
		return m_pInheritFrom->FindEventHandler(sEvent, retEvent);

	//	Otherwise, nothing

	return false;
	}

bool CDesignType::FindStaticData (const CString &sAttrib, const CString **retpData) const

//	FindStaticData
//
//	Returns static data

	{
	if (m_StaticData.FindData(sAttrib, retpData))
		return true;

	if (m_pInheritFrom)
		return m_pInheritFrom->FindStaticData(sAttrib, retpData);

	return false;
	}

void CDesignType::FireCustomEvent (const CString &sEvent, ECodeChainEvents iEvent, ICCItem *pData, ICCItem **retpResult)

//	FireCustomEvent
//
//	Fires a custom event

	{
	CCodeChainCtx Ctx;

	SEventHandlerDesc Event;
	if (FindEventHandler(sEvent, &Event))
		{
		Ctx.SetEvent(iEvent);
		Ctx.SaveAndDefineDataVar(pData);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ReportEventError(sEvent, pResult);

		//	Either return the event result or discard it

		if (retpResult)
			*retpResult = pResult;
		else
			Ctx.Discard(pResult);
		}
	else
		{
		if (retpResult)
			*retpResult = Ctx.CreateNil();
		}
	}

bool CDesignType::FireGetCreatePos (CSpaceObject *pBase, CSpaceObject *pTarget, CSpaceObject **retpGate, CVector *retvPos)

//	FireGetCreatePos
//
//	Fire GetCreatePos event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(GET_CREATE_POS_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.DefineSpaceObject(CONSTLIT("aBaseObj"), pBase);
		Ctx.DefineSpaceObject(CONSTLIT("aTargetObj"), pTarget);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ReportEventError(GET_CREATE_POS_EVENT, pResult);

		bool bResult;
		if (pResult->IsInteger())
			{
			*retpGate = Ctx.AsSpaceObject(pResult);
			*retvPos = (*retpGate)->GetPos();
			bResult = true;
			}
		else if (pResult->IsList())
			{
			*retpGate = NULL;
			*retvPos = Ctx.AsVector(pResult);
			bResult = true;
			}
		else
			bResult = false;

		Ctx.Discard(pResult);
		return bResult;
		}

	return false;
	}

void CDesignType::FireGetGlobalAchievements (CGameStats &Stats)

//	FireGetGlobalAchievements
//
//	Fires GetGlobalAchievements event

	{
	int i;

	SEventHandlerDesc Event;
	if (FindEventHandler(GET_GLOBAL_ACHIEVEMENTS_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		//	Run code

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ReportEventError(GET_GLOBAL_ACHIEVEMENTS_EVENT, pResult);
		else if (pResult->IsNil())
			;
		else if (pResult->IsList())
			{
			//	If we have a list of lists, then we have 
			//	a list of achievements

			if (pResult->GetCount() > 0 && pResult->GetElement(0)->IsList())
				{
				for (i = 0; i < pResult->GetCount(); i++)
					{
					ICCItem *pAchievement = pResult->GetElement(i);
					if (pAchievement->GetCount() > 0)
						{
						CString sName = pAchievement->GetElement(0)->GetStringValue();
						CString sValue = ParseAchievementValue(pAchievement->GetElement(1));
						CString sSection = ParseAchievementSection(pAchievement->GetElement(2));
						CString sSort = ParseAchievementSort(pAchievement->GetElement(3));

						if (!sName.IsBlank())
							Stats.Insert(sName, sValue, sSection, sSort);
						}
					}
				}

			//	Otherwise, we have a single achievement

			else if (pResult->GetCount() > 0)
				{
				CString sName = pResult->GetElement(0)->GetStringValue();
				CString sValue = ParseAchievementValue(pResult->GetElement(1));
				CString sSection = ParseAchievementSection(pResult->GetElement(2));
				CString sSort = ParseAchievementSort(pResult->GetElement(3));

				if (!sName.IsBlank())
					Stats.Insert(sName, sValue, sSection, sSort);
				}
			}

		Ctx.Discard(pResult);
		}
	}

bool CDesignType::FireGetGlobalDockScreen (const SEventHandlerDesc &Event, CSpaceObject *pObj, CString *retsScreen, ICCItem **retpData, int *retiPriority)

//	FireGetGlobalDockScreen
//
//	Asks a type to see if it wants to override an object's
//	dock screen.
//
//	NOTE: If we return TRUE and *retpData is non-NULL then the caller is responsible 
//	for discarding data.

	{
	//	Set up

	CCodeChainCtx Ctx;
	Ctx.SaveAndDefineSourceVar(pObj);

	//	Run

	ICCItem *pResult = Ctx.Run(Event);

	bool bResult;

	//	Error?

	if (pResult->IsError())
		{
		ReportEventError(GET_GLOBAL_DOCK_SCREEN_EVENT, pResult);
		bResult = false;
		}

	//	Parse the result

	else if (pResult->IsNil())
		bResult = false;

	else if (pResult->IsList() && pResult->GetCount() >= 2)
		{
		*retsScreen = pResult->GetElement(0)->GetStringValue();
		if (pResult->GetCount() >= 3)
			{
			*retpData = pResult->GetElement(1)->Reference();
			*retiPriority = pResult->GetElement(2)->GetIntegerValue();
			}
		else
			{
			*retpData = NULL;
			*retiPriority = pResult->GetElement(1)->GetIntegerValue();
			}
		bResult = true;
		}
	else if (pResult->GetCount() > 0)
		{
		*retsScreen = pResult->GetElement(0)->GetStringValue();
		*retiPriority = 0;
		*retpData = NULL;
		bResult = true;
		}
	else
		bResult = false;

	//	Done

	Ctx.Discard(pResult);
	return bResult;
	}

bool CDesignType::FireGetGlobalPlayerPriceAdj (const SEventHandlerDesc &Event, ETradeServiceTypes iService, CSpaceObject *pProvider, const CItem &Item, ICCItem *pData, int *retiPriceAdj)

//	FireGetGlobalPlayerPriceAdj
//
//	Fires event to allow types to modify prices charged to the player.

	{
	CCodeChainCtx Ctx;

	//	Set up

	Ctx.SetEvent(eventGetGlobalPlayerPriceAdj);
	Ctx.SetItemType(Item.GetType());
	Ctx.DefineString(CONSTLIT("aService"), CTradingDesc::ServiceToString(iService));
	Ctx.DefineSpaceObject(CONSTLIT("aProviderObj"), pProvider);
	Ctx.SaveAndDefineItemVar(Item);
	if (pData)
		Ctx.SaveAndDefineDataVar(pData);

	//	Run

	ICCItem *pResult = Ctx.Run(Event);

	int iPriceAdj = 100;
	if (pResult->IsError())
		ReportEventError(GET_GLOBAL_PLAYER_PRICE_ADJ_EVENT, pResult);
	else if (pResult->IsNil())
		;
	else
		iPriceAdj = pResult->GetIntegerValue();

	//	Done

	Ctx.Discard(pResult);

	if (retiPriceAdj)
		*retiPriceAdj = iPriceAdj;

	return (iPriceAdj != 100);
	}

int CDesignType::FireGetGlobalResurrectPotential (void)

//	FireGetGlobalResurrectPotential
//
//	Returns the potential that this design type will resurrect the player
//	(0 = no chance; 100 = greatest potential)

	{
	int iResult = 0;

	SEventHandlerDesc Event;
	if (FindEventHandler(GET_GLOBAL_RESURRECT_POTENTIAL_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		//	Run code

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ReportEventError(GET_GLOBAL_RESURRECT_POTENTIAL_EVENT, pResult);
		else
			iResult = pResult->GetIntegerValue();

		Ctx.Discard(pResult);
		}

	return iResult;
	}

void CDesignType::FireObjCustomEvent (const CString &sEvent, CSpaceObject *pObj, ICCItem **retpResult)

//	FireObjCustomEvent
//
//	Fires a named event and optionally returns result

	{
	CCodeChainCtx Ctx;

	SEventHandlerDesc Event;
	if (FindEventHandler(sEvent, &Event))
		{
		Ctx.SaveAndDefineSourceVar(pObj);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			pObj->ReportEventError(sEvent, pResult);

		//	Either return the event result or discard it

		if (retpResult)
			*retpResult = pResult;
		else
			Ctx.Discard(pResult);
		}
	else
		{
		if (retpResult)
			*retpResult = Ctx.CreateNil();
		}
	}

ALERROR CDesignType::FireOnGlobalDockPaneInit (const SEventHandlerDesc &Event, void *pScreen, DWORD dwScreenUNID, const CString &sScreen, const CString &sPane, CString *retsError)

//	FireOnGlobalDockPaneInit
//
//	Dock pane initialized

	{
	CCodeChainCtx Ctx;

	//	Set up

	Ctx.SetScreen(pScreen);
	Ctx.DefineInteger(CONSTLIT("aScreenUNID"), dwScreenUNID);
	Ctx.DefineString(CONSTLIT("aScreen"), sScreen);
	Ctx.DefineString(CONSTLIT("aPane"), sPane);

	//	Run

	ICCItem *pResult = Ctx.Run(Event);
	if (pResult->IsError())
		ReportEventError(ON_GLOBAL_DOCK_PANE_INIT_EVENT, pResult);

	//	Done

	Ctx.Discard(pResult);
	return NOERROR;
	}

void CDesignType::FireOnGlobalMarkImages (const SEventHandlerDesc &Event)

//	FireOnGlobalMarkImages
//
//	Fires OnGlobalMarkImages

	{
	CCodeChainCtx CCCtx;

	//	Run code

	ICCItem *pResult = CCCtx.Run(Event);
	if (pResult->IsError())
		ReportEventError(ON_GLOBAL_MARK_IMAGES_EVENT, pResult);

	CCCtx.Discard(pResult);
	}

void CDesignType::FireOnGlobalObjDestroyed (const SEventHandlerDesc &Event, SDestroyCtx &Ctx)

//	FireOnGlobalObjDestroyed
//
//	Fires OnGlobalObjDestroyed

	{
	CCodeChainCtx CCCtx;

	CCCtx.DefineSpaceObject(CONSTLIT("aObjDestroyed"), Ctx.pObj);
	CCCtx.DefineSpaceObject(CONSTLIT("aDestroyer"), Ctx.Attacker.GetObj());
	CCCtx.DefineSpaceObject(CONSTLIT("aOrderGiver"), (Ctx.Attacker.GetObj() ? Ctx.Attacker.GetObj()->GetOrderGiver(Ctx.iCause) : NULL));
	CCCtx.DefineSpaceObject(CONSTLIT("aWreckObj"), Ctx.pWreck);
	CCCtx.DefineString(CONSTLIT("aDestroyReason"), GetDestructionName(Ctx.iCause));

	//	Run code

	ICCItem *pResult = CCCtx.Run(Event);
	if (pResult->IsError())
		ReportEventError(ON_GLOBAL_OBJ_DESTROYED_EVENT, pResult);

	CCCtx.Discard(pResult);
	}

ALERROR CDesignType::FireOnGlobalPlayerChangedShips (CSpaceObject *pOldShip, CString *retsError)

//	FireOnGlobalPlayerChangedShips
//
//	Player changed ships

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_GLOBAL_PLAYER_CHANGED_SHIPS_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.DefineSpaceObject(CONSTLIT("aOldPlayerShip"), pOldShip);

		//	Run code

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ReportEventError(ON_GLOBAL_PLAYER_CHANGED_SHIPS_EVENT, pResult);

		Ctx.Discard(pResult);
		}

	return NOERROR;
	}

ALERROR CDesignType::FireOnGlobalPlayerEnteredSystem (CString *retsError)

//	FireOnGlobalPlayerEnteredSystem
//
//	Player entered the system

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_GLOBAL_PLAYER_ENTERED_SYSTEM_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		//	Run code

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ReportEventError(ON_GLOBAL_PLAYER_ENTERED_SYSTEM_EVENT, pResult);

		Ctx.Discard(pResult);
		}

	return NOERROR;
	}

ALERROR CDesignType::FireOnGlobalPlayerLeftSystem (CString *retsError)

//	FireOnGlobalPlayerLeftSystem
//
//	Player left the system

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_GLOBAL_PLAYER_LEFT_SYSTEM_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		//	Run code

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ReportEventError(ON_GLOBAL_PLAYER_LEFT_SYSTEM_EVENT, pResult);

		Ctx.Discard(pResult);
		}

	return NOERROR;
	}

ALERROR CDesignType::FireOnGlobalResurrect (CString *retsError)

//	FireOnGlobalResurrect
//
//	Resurrection code invoked

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_GLOBAL_RESURRECT_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		//	Run code

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ReportEventError(ON_GLOBAL_RESURRECT_EVENT, pResult);

		Ctx.Discard(pResult);
		}

	return NOERROR;
	}

ALERROR CDesignType::FireOnGlobalSystemCreated (SSystemCreateCtx &SysCreateCtx, CString *retsError)

//	FireOnGlobalSystemCreated
//
//	Fire event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_GLOBAL_SYSTEM_CREATED_EVENT, &Event))
		{
		CCodeChainCtx Ctx;
		Ctx.SetSystemCreateCtx(&SysCreateCtx);

		//	Run code

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ReportEventError(ON_GLOBAL_SYSTEM_CREATED_EVENT, pResult);

		Ctx.Discard(pResult);
		}

	return NOERROR;
	}

void CDesignType::FireOnGlobalSystemStarted (const SEventHandlerDesc &Event)

//	FireOnGlobalSystemStarted
//
//	System has started

	{
	CCodeChainCtx CCCtx;

	//	Run code

	ICCItem *pResult = CCCtx.Run(Event);
	if (pResult->IsError())
		ReportEventError(ON_GLOBAL_SYSTEM_STARTED_EVENT, pResult);

	CCCtx.Discard(pResult);
	}

void CDesignType::FireOnGlobalSystemStopped (const SEventHandlerDesc &Event)

//	FireOnGlobalSystemStopped
//
//	System has stopped

	{
	CCodeChainCtx CCCtx;

	//	Run code

	ICCItem *pResult = CCCtx.Run(Event);
	if (pResult->IsError())
		ReportEventError(ON_GLOBAL_SYSTEM_STOPPED_EVENT, pResult);

	CCCtx.Discard(pResult);
	}

ALERROR CDesignType::FireOnGlobalTopologyCreated (CString *retsError)

//	FireOnGlobalTopologyCreated
//
//	Fire event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_GLOBAL_TOPOLOGY_CREATED_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		//	Run code

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ReportEventError(ON_GLOBAL_TOPOLOGY_CREATED_EVENT, pResult);

		Ctx.Discard(pResult);
		}

	return NOERROR;
	}

ALERROR CDesignType::FireOnGlobalTypesInit (SDesignLoadCtx &Ctx)

//	FireOnGlobalTypesInit
//
//	Give this type a chance to create dynamic types.

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(evtOnGlobalTypesInit, &Event))
		{
		CCodeChainCtx CCCtx;
		CCCtx.SetEvent(eventOnGlobalTypesInit);

		ICCItem *pResult = CCCtx.Run(Event);
		if (pResult->IsError())
			{
			Ctx.sError = strPatternSubst(CONSTLIT("OnGlobalTypesInit (%x): %s"), GetUNID(), pResult->GetStringValue());
			return ERR_FAIL;
			}

		CCCtx.Discard(pResult);
		}

	return NOERROR;
	}

ALERROR CDesignType::FireOnGlobalUniverseCreated (const SEventHandlerDesc &Event)

//	FireOnGlobalUniverseCreated
//
//	Fire event

	{
	CCodeChainCtx Ctx;

	//	Run code

	ICCItem *pResult = Ctx.Run(Event);
	if (pResult->IsError())
		ReportEventError(ON_GLOBAL_UNIVERSE_CREATED_EVENT, pResult);

	//	Done

	Ctx.Discard(pResult);
	return NOERROR;
	}

ALERROR CDesignType::FireOnGlobalUniverseLoad (const SEventHandlerDesc &Event)

//	FireOnGlobalUniverseLoad
//
//	Fire event

	{
	CCodeChainCtx Ctx;
	if (g_pUniverse->InResurrectMode())
		Ctx.DefineString(CONSTLIT("aReason"), CONSTLIT("resurrect"));
	else
		Ctx.DefineNil(CONSTLIT("aReason"));

	//	Run code

	ICCItem *pResult = Ctx.Run(Event);
	if (pResult->IsError())
		ReportEventError(ON_GLOBAL_UNIVERSE_LOAD_EVENT, pResult);

	//	Done

	Ctx.Discard(pResult);
	return NOERROR;
	}

ALERROR CDesignType::FireOnGlobalUniverseSave (const SEventHandlerDesc &Event)

//	FireOnGlobalUniverseSave
//
//	Fire event

	{
	CCodeChainCtx Ctx;

	//	Run code

	ICCItem *pResult = Ctx.Run(Event);
	if (pResult->IsError())
		ReportEventError(ON_GLOBAL_UNIVERSE_SAVE_EVENT, pResult);

	//	Done

	Ctx.Discard(pResult);
	return NOERROR;
	}

void CDesignType::FireOnGlobalUpdate (const SEventHandlerDesc &Event)

//	FireOnGlobalUpdate
//
//	Fire event

	{
	CCodeChainCtx Ctx;

	//	Run code

	ICCItem *pResult = Ctx.Run(Event);
	if (pResult->IsError())
		ReportEventError(ON_GLOBAL_UPDATE_EVENT, pResult);

	//	Done

	Ctx.Discard(pResult);
	}

void CDesignType::FireOnRandomEncounter (CSpaceObject *pObj)

//	FireOnRandomEncounter
//
//	Fire OnRandomEncounter event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(ON_RANDOM_ENCOUNTER_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(pObj);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ReportEventError(ON_RANDOM_ENCOUNTER_EVENT, pResult);

		Ctx.Discard(pResult);
		}
	}

ICCItem *CDesignType::GetEventHandler (const CString &sEvent) const

//	GetEventHandler
//
//	Returns an event handler (or NULL)

	{
	SEventHandlerDesc Event;
	if (FindEventHandler(sEvent, &Event))
		return Event.pCode;
	else
		return NULL;
	}

ICCItem *CDesignType::GetProperty (CCodeChainCtx &Ctx, const CString &sProperty)

//	GetProperty
//
//	Returns the value of the given property. We return an allocated CC item (which 
//	must be discarded by the caller). If the property is not found, we return
//	Nil.

	{
	CCodeChain &CC = g_pUniverse->GetCC();
	ICCItem *pResult;

	if (strEquals(sProperty, PROPERTY_API_VERSION))
		return CC.CreateInteger(GetAPIVersion());

	else if (strEquals(sProperty, PROPERTY_CLASS))
		return CC.CreateString(GetTypeClassName());

	else if (strEquals(sProperty, PROPERTY_EXTENSION))
		{
		if (m_pExtension)
			return CC.CreateInteger(m_pExtension->GetUNID());
		else
			return CC.CreateNil();
		}

	//	See if our subclass will handle it.

	else if (pResult = OnGetProperty(Ctx, sProperty))
		return pResult;

	//	Otherwise, we see if there is a data field

	else
		return CreateResultFromDataField(CC, GetDataField(sProperty));
	}

bool CDesignType::IsValidLoadXML (const CString &sTag)

//	IsValidLoadXML
//
//	Returns TRUE if the given XML element tag is a valid tag for all design
//	types.

	{
	return (strEquals(sTag, EVENTS_TAG)
			|| strEquals(sTag, DOCK_SCREENS_TAG)
			|| strEquals(sTag, GLOBAL_DATA_TAG)
			|| strEquals(sTag, LANGUAGE_TAG)
			|| strEquals(sTag, STATIC_DATA_TAG)
			|| strEquals(sTag, DISPLAY_ATTRIBUTES_TAG)
			|| strEquals(sTag, ATTRIBUTE_DESC_TAG));
	}

void CDesignType::AddUniqueHandlers (TSortMap<CString, SEventHandlerDesc> *retInheritedHandlers)

//	AddUniqueHandlers
//
//	Adds this type's handlers to the given array (as long as they are not 
//	already there).

	{
	int i;

	for (i = 0; i < m_Events.GetCount(); i++)
		{
		ICCItem *pCode;
		const CString &sEvent = m_Events.GetEvent(i, &pCode);

		if (!retInheritedHandlers->Find(sEvent))
			{
			SEventHandlerDesc *pDesc = retInheritedHandlers->Insert(sEvent);
			pDesc->pExtension = m_pExtension;
			pDesc->pCode = pCode;
			}
		}

	//	Add base classes

	if (m_pInheritFrom)
		m_pInheritFrom->AddUniqueHandlers(retInheritedHandlers);
	}

void CDesignType::GetEventHandlers (const CEventHandler **retpHandlers, TSortMap<CString, SEventHandlerDesc> *retInheritedHandlers)

//	GetEventHandlers
//
//	If this type has base classes, then it returns a list of events in the 
//	retInheritedHandlers array. Otherwise, retInheritedHandlers is undefined and
//	retpHandlers points to a CEventHandler object with all handlers
//	(which are all defined by the type).

	{
	//	If we don't inherit from anything, then we are the only ones who define
	//	events.

	if (m_pInheritFrom == NULL)
		{
		*retpHandlers = &m_Events;
		return;
		}

	//	Otherwise, we need to build the array of handlers.

	*retpHandlers = NULL;
	retInheritedHandlers->DeleteAll();
	AddUniqueHandlers(retInheritedHandlers);
	}

CXMLElement *CDesignType::GetScreen (const CString &sUNID)

//	GetScreen
//
//	Returns the given screen (either globally or from the local screens)

	{
	CDockScreenTypeRef Screen;
	Screen.LoadUNID(sUNID);
	Screen.Bind(GetLocalScreens());
	return Screen.GetDesc();
	}

const CString &CDesignType::GetStaticData (const CString &sAttrib) const

//	GetStaticData
//
//	Returns static data
	
	{
	const CString *pData;
	if (m_StaticData.FindData(sAttrib, &pData))
		return *pData;

	if (m_pInheritFrom)
		return m_pInheritFrom->GetStaticData(sAttrib);

	return NULL_STR;
	}

CString CDesignType::GetTypeChar (DesignTypes iType)

//	GetTypeChar
//
//	Returns the character associated with the given type

	{
	return CString(&DESIGN_CHAR[iType], 1);
	}

CString CDesignType::GetTypeClassName (void) const

//	GetTypeClassName
//
//	Returns the class name of the type

	{
	return CString(DESIGN_CLASS_NAME[GetType()]);
	}

bool CDesignType::HasAttribute (const CString &sAttrib) const

//	HasAttribute
//
//	Returns TRUE if we have the literal or special attribute.

	{
	if (HasLiteralAttribute(sAttrib))
		return true;

	return HasSpecialAttribute(sAttrib);
	}

bool CDesignType::HasSpecialAttribute (const CString &sAttrib) const

//	HasSpecialAttribute
//
//	Returns TRUE if we have the special attribute

	{
	if (strStartsWith(sAttrib, SPECIAL_EVENT))
		{
		CString sEvent = strSubString(sAttrib, SPECIAL_EVENT.GetLength());
		return FindEventHandler(sEvent);
		}
	else if (strStartsWith(sAttrib, SPECIAL_EXTENSION))
		{
		DWORD dwUNID = strToInt(strSubString(sAttrib, SPECIAL_EXTENSION.GetLength()), 0);
		return (m_pExtension && (m_pExtension->GetUNID() == dwUNID));
		}
	else if (strStartsWith(sAttrib, SPECIAL_UNID))
		{
		DWORD dwUNID = strToInt(strSubString(sAttrib, SPECIAL_UNID.GetLength()), 0);
		return (GetUNID() == dwUNID);
		}
	else
		return OnHasSpecialAttribute(sAttrib);
	}

void CDesignType::InitCachedEvents (void)

//	InitCachedEvents
//
//	Enumerates events and adds the appropriate ones to the cache

	{
	int i, j;

	for (i = 0; i < m_Events.GetCount(); i++)
		{
		ICCItem *pCode;
		const CString &sEvent = m_Events.GetEvent(i, &pCode);

		for (j = 0; j < evtCount; j++)
			{
			if (strEquals(sEvent, CString(CACHED_EVENTS[j], -1, true)))
				{
				m_EventsCache[j].pExtension = m_pExtension;
				m_EventsCache[j].pCode = pCode;
				break;
				}
			}
		}
	}

void CDesignType::InitCachedEvents (int iCount, char **pszEvents, SEventHandlerDesc *retEvents)

//	InitCachedEvents
//
//	Initializes events cached by subclassess.

	{
	int i;

	for (i = 0; i < iCount; i++)
		{
		if (!FindEventHandler(CString(pszEvents[i], -1, true), &retEvents[i]))
			{
			retEvents[i].pExtension = NULL;
			retEvents[i].pCode = NULL;
			}
		}
	}

ALERROR CDesignType::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, bool bIsOverride)

//	InitFromXML
//
//	Creates a design type from an XML element

	{
	ALERROR error;
	int i;

	//	Remember the type we're loading

	Ctx.pType = this;

	//	Load UNID

	if (error = ::LoadUNID(Ctx, pDesc->GetAttribute(UNID_ATTRIB), &m_dwUNID))
		return error;

	if (m_dwUNID == 0)
		{
		Ctx.sError = strPatternSubst(CONSTLIT("<%s> must have a valid UNID."), pDesc->GetTag());
		Ctx.pType = NULL;
		return ERR_FAIL;
		}

	//	Extension information

	m_pExtension = Ctx.pExtension;
	m_dwVersion = Ctx.GetAPIVersion();

	//	Remember XML if necessary

	if (Ctx.bKeepXML)
		m_pXML = pDesc;

	//	Inheritance

	m_bIsModification = bIsOverride;

	if (error = ::LoadUNID(Ctx, pDesc->GetAttribute(INHERIT_ATTRIB), &m_dwInheritFrom))
		{
		Ctx.pType = NULL;
		return error;
		}

	m_pInheritFrom = NULL;

	//	Load attributes

	if (!pDesc->FindAttribute(ATTRIBUTES_ATTRIB, &m_sAttributes))
		m_sAttributes = pDesc->GetAttribute(MODIFIERS_ATTRIB);

	//	Initialize

	m_pLocalScreens = NULL;

	//	Load various elements

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pDesc->GetContentElement(i);

		if (strEquals(pItem->GetTag(), EVENTS_TAG))
			{
			if (error = m_Events.InitFromXML(Ctx, pItem))
				{
				Ctx.pType = NULL;
				return ComposeLoadError(Ctx, Ctx.sError);
				}

			InitCachedEvents();
			}
		else if (strEquals(pItem->GetTag(), STATIC_DATA_TAG))
			m_StaticData.SetFromXML(pItem);
		else if (strEquals(pItem->GetTag(), GLOBAL_DATA_TAG)
				|| ((GetType() == designSovereign || GetType() == designGenericType)
						&& strEquals(pItem->GetTag(), INITIAL_DATA_TAG)))
			{
			m_InitGlobalData.SetFromXML(pItem);
			m_GlobalData = m_InitGlobalData;
			}
		else if (strEquals(pItem->GetTag(), DOCK_SCREENS_TAG))
			m_pLocalScreens = pItem->OrphanCopy();
		else if (strEquals(pItem->GetTag(), LANGUAGE_TAG))
			{
			if (error = m_Language.InitFromXML(Ctx, pItem))
				return ComposeLoadError(Ctx, Ctx.sError);
			}
		else if (strEquals(pItem->GetTag(), DISPLAY_ATTRIBUTES_TAG)
				|| strEquals(pItem->GetTag(), ATTRIBUTE_DESC_TAG))
			{
			if (error = m_DisplayAttribs.InitFromXML(Ctx, pItem))
				{
				Ctx.pType = NULL;
				return ComposeLoadError(Ctx, Ctx.sError);
				}
			}

		//	Otherwise, it is some element that we don't understand.
		}

	//	Load specific data

	if (error = OnCreateFromXML(Ctx, pDesc))
		{
		Ctx.pType = NULL;
		return error;
		}

	//	Done

	Ctx.pType = NULL;
	return NOERROR;
	}

bool CDesignType::InSelfReference (CDesignType *pType)

//	InSelfReference
//
//	Returns TRUE if the inheritance chain loops above this type

	{
	CDesignType *pParent = g_pUniverse->FindDesignType(pType->m_dwInheritFrom);
	while (pParent)
		{
		if (pParent->m_dwUNID == pType->m_dwUNID)
			return true;

		if (InSelfReference(pParent))
			return true;

		pParent = g_pUniverse->FindDesignType(pParent->m_dwInheritFrom);
		}

	return false;
	}

bool CDesignType::MatchesCriteria (const CDesignTypeCriteria &Criteria)

//	MatchesCriteria
//
//	Returns TRUE if this type matches the given criteria

	{
	int i;

	//	If this type is not part of the criteria, then we're done

	if (!Criteria.MatchesDesignType(GetType()))
		return false;

	//	Skip virtual

	if (IsVirtual() && !Criteria.IncludesVirtual())
		return false;

	//	Check level

	if (Criteria.ChecksLevel())
		{
		int iMinLevel;
		int iMaxLevel;
		GetLevel(&iMinLevel, &iMaxLevel);

		if (iMinLevel == -1 || !Criteria.MatchesLevel(iMinLevel, iMaxLevel))
			return false;
		}

	//	Check required attributes

	for (i = 0; i < Criteria.GetRequiredAttribCount(); i++)
		if (!HasLiteralAttribute(Criteria.GetRequiredAttrib(i)))
			return false;

	for (i = 0; i < Criteria.GetRequiredSpecialAttribCount(); i++)
		if (!HasSpecialAttribute(Criteria.GetRequiredSpecialAttrib(i)))
			return false;

	//	Check excluded attributes

	for (i = 0; i < Criteria.GetExcludedAttribCount(); i++)
		if (HasLiteralAttribute(Criteria.GetExcludedAttrib(i)))
			return false;

	for (i = 0; i < Criteria.GetExcludedSpecialAttribCount(); i++)
		if (HasSpecialAttribute(Criteria.GetExcludedSpecialAttrib(i)))
			return false;

	//	If we get this far, then we match

	return true;
	}

void CDesignType::MergeLanguageTo (CLanguageDataBlock &Dest)

//	MergeLanguageTo
//
//	Merges our language block into the destination such that we add new messages
//	to the destination (i.e., we never overwrite existing messages in the
//	destination.

	{
	DEBUG_TRY

	Dest.MergeFrom(m_Language);

	//	If we inherit from another type, add that data too

	if (m_pInheritFrom)
		m_pInheritFrom->MergeLanguageTo(Dest);

	DEBUG_CATCH
	}

void CDesignType::MergeType (CDesignType *pSource)

//	MergeType
//
//	Merges the info from that given type
	
	{
	//	We take the extension of the source because it has taken responsibility
	//	for the type.
	//
	//	If we didn't do this then it would be possible for someone to override
	//	a registered type and use its permissions.

	m_pExtension = pSource->m_pExtension;
	m_dwVersion = Max(m_dwVersion, pSource->m_dwVersion);

	//	Merge our variables

	m_StaticData.MergeFrom(pSource->m_StaticData);
	m_InitGlobalData.MergeFrom(pSource->m_InitGlobalData);
	m_Language.MergeFrom(pSource->m_Language);
	m_Events.MergeFrom(pSource->m_Events);

	//	LATER: Merge local screens

	//	Let our subclass handle the rest.

	OnMergeType(pSource); 
	}

ALERROR CDesignType::PrepareBindDesign (SDesignLoadCtx &Ctx)

//	PrepareBindDesign
//
//	Do stuff that needs to happen before actual bind

	{
	//	Resolve inheritance

	if (m_dwInheritFrom)
		{
		m_pInheritFrom = g_pUniverse->FindDesignType(m_dwInheritFrom);
		if (m_pInheritFrom == NULL)
			return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unknown inherit design type: %x"), m_dwInheritFrom));

		if (m_pInheritFrom->GetType() != GetType())
			return ComposeLoadError(Ctx, CONSTLIT("Cannot inherit from a different type."));

		//	Make sure we are not in an inheritance loop

		if (InSelfReference(this))
			return ComposeLoadError(Ctx, CONSTLIT("Cannot inherit from self"));
		}

	//	Done

	return OnPrepareBindDesign(Ctx);
	}

void CDesignType::ReadFromStream (SUniverseLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads the variant portions of the design type
	
	{
	//	Read global data

	if (Ctx.dwVersion >= 3)
		m_GlobalData.ReadFromStream(Ctx.pStream);

	//	Allow sub-classes to load

	OnReadFromStream(Ctx);
	}

void CDesignType::ReadGlobalData (SUniverseLoadCtx &Ctx)

//	ReadGlobalData
//
//	For compatibility with older save versions

	{
	if (Ctx.dwVersion < 3)
		m_GlobalData.ReadFromStream(Ctx.pStream);
	}

void CDesignType::Reinit (void)

//	Reinit
//
//	Reinitializes the variant portions of the design type
	
	{
	//	Reinit global data

	m_GlobalData = m_InitGlobalData;

	//	Allow sub-classes to reinit

	OnReinit();
	}

void CDesignType::ReportEventError (const CString &sEvent, ICCItem *pError)

//	ReportEventError
//
//	Reports an event error

	{
	CString sError = strPatternSubst(CONSTLIT("%s [%x]: %s"), sEvent, m_dwUNID, pError->GetStringValue());
	CSpaceObject *pPlayer = g_pUniverse->GetPlayer();
	if (pPlayer)
		pPlayer->SendMessage(NULL, sError);

	kernelDebugLogMessage(sError);
	}

bool CDesignType::Translate (CSpaceObject *pObj, const CString &sID, ICCItem *pData, ICCItem **retpResult)

//	Translate
//
//	Translate from a <Language> block to a CodeChain item.
//
//	NOTE: Caller is responsible for discarding the result (if we return TRUE).
	
	{
	if (m_Language.Translate(pObj, sID, pData, retpResult))
		return true;

	//	Backwards compatible translate

	return TranslateVersion2(pObj, sID, retpResult);
	}

bool CDesignType::TranslateText (CSpaceObject *pObj, const CString &sID, ICCItem *pData, CString *retsText)

//	Translate
//
//	Translate from a <Language> block to text.

	{
	if (m_Language.Translate(pObj, sID, pData, retsText))
		return true;

	//	Backwards compatible translate

	ICCItem *pItem;
	if (!TranslateVersion2(pObj, sID, &pItem))
		return false;

	if (retsText)
		*retsText = pItem->GetStringValue();

	pItem->Discard(&g_pUniverse->GetCC());
	return true;
	}
	
bool CDesignType::TranslateVersion2 (CSpaceObject *pObj, const CString &sID, ICCItem **retpResult)

//	TranslateVersion2
//
//	Translates using the old apiVersion="2" method, which relied on static data.

	{
	int i;

	if (GetVersion() > 2)
		return false;

	CString sData = GetStaticData(CONSTLIT("Language"));
	if (!sData.IsBlank())
		{
		CCodeChainCtx Ctx;

		ICCItem *pData = Ctx.Link(sData, 0, NULL);

		for (i = 0; i < pData->GetCount(); i++)
			{
			ICCItem *pEntry = pData->GetElement(i);
			if (pEntry->GetCount() == 2 && strEquals(sID, pEntry->GetElement(0)->GetStringValue()))
				{
				*retpResult = Ctx.Run(pEntry->GetElement(1));	//	LATER:Event
				Ctx.Discard(pData);
				return true;
				}
			}

		Ctx.Discard(pData);
		}

	return false;
	}

void CDesignType::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Writes the variant portions of the design type

	{
	//	Write out global data

	m_GlobalData.WriteToStream(pStream, NULL);

	//	Allow sub-classes to write

	OnWriteToStream(pStream);
	}

//	CItemTypeRef --------------------------------------------------------------

ALERROR CItemTypeRef::Bind (SDesignLoadCtx &Ctx, ItemCategories iCategory)
	{
	ALERROR error;
	if (error = CDesignTypeRef<CItemType>::Bind(Ctx))
		return error;

	if (m_pType && m_pType->GetCategory() != iCategory)
		{
		Ctx.sError = strPatternSubst(CONSTLIT("%s item expected: %x"), ::GetItemCategoryName(iCategory), m_dwUNID);
		return ERR_FAIL;
		}

	return NOERROR;
	}

//	CArmorClassRef -----------------------------------------------------------

ALERROR CArmorClassRef::Bind (SDesignLoadCtx &Ctx)
	{
	if (m_dwUNID)
		{
		CDesignType *pBaseType = g_pUniverse->FindDesignType(m_dwUNID);
		if (pBaseType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unknown armor design type: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		CItemType *pItemType = CItemType::AsType(pBaseType);
		if (pItemType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Armor item type expected: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		m_pType = pItemType->GetArmorClass();
		if (m_pType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Armor item type expected: %x"), m_dwUNID);
			return ERR_FAIL;
			}
		}

	return NOERROR;
	}

//	CDeviceClassRef -----------------------------------------------------------

ALERROR CDeviceClassRef::Bind (SDesignLoadCtx &Ctx)
	{
	if (m_dwUNID)
		{
		CDesignType *pBaseType = g_pUniverse->FindDesignType(m_dwUNID);
		if (pBaseType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unknown device design type: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		CItemType *pItemType = CItemType::AsType(pBaseType);
		if (pItemType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Device item type expected: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		m_pType = pItemType->GetDeviceClass();
		if (m_pType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Device item type expected: %x"), m_dwUNID);
			return ERR_FAIL;
			}
		}

	return NOERROR;
	}

void CDeviceClassRef::Set (CDeviceClass *pDevice)
	{
	if (pDevice)
		{
		m_pType = pDevice;
		m_dwUNID = pDevice->GetUNID();
		}
	else
		{
		m_pType = NULL;
		m_dwUNID = 0;
		}
	}

//	CWeaponFireDescRef -----------------------------------------------------------

ALERROR CWeaponFireDescRef::Bind (SDesignLoadCtx &Ctx)
	{
	if (m_dwUNID)
		{
		CDesignType *pBaseType = g_pUniverse->FindDesignType(m_dwUNID);
		if (pBaseType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unknown weapon fire desc design type: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		CItemType *pItemType = CItemType::AsType(pBaseType);
		if (pItemType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Weapon item type expected: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		CDeviceClass *pDevice = pItemType->GetDeviceClass();
		if (pDevice == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Weapon item type expected: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		CWeaponClass *pWeapon = pDevice->AsWeaponClass();
		if (pWeapon == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Weapon item type expected: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		m_pType = pWeapon->GetVariant(0);
		if (m_pType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Invalid weapon type: %x"), m_dwUNID);
			return ERR_FAIL;
			}
		}

	return NOERROR;
	}

//	CEffectCreatorRef ---------------------------------------------------------

CEffectCreatorRef::~CEffectCreatorRef (void)
	{
	if (m_bDelete && m_pType)
		delete m_pType;

	if (m_pSingleton)
		delete m_pSingleton;
	}

CEffectCreatorRef &CEffectCreatorRef::operator= (const CEffectCreatorRef &Source)

//	CEffectCreatorRef operator=

	{
	//	Free our current type, if necessary

	if (m_bDelete && m_pType)
		delete m_pType;

	m_dwUNID = Source.m_dwUNID;

	//	We make a reference

	m_pType = Source.m_pType;
	m_bDelete = false;

	return *this;
	}

ALERROR CEffectCreatorRef::Bind (SDesignLoadCtx &Ctx)
	{
	DEBUG_TRY

	//	Clean up, because we might want to recompute for next time.

	if (m_pSingleton)
		{
		try
			{
			delete m_pSingleton;
			}
		catch (...)
			{
			::kernelDebugLogMessage("Crash deleting singleton: %08x. UNID = %08x", (DWORD)m_pSingleton, m_dwUNID);
			throw;
			}

		m_pSingleton = NULL;
		}

	//	Bind

	if (m_dwUNID)
		return CDesignTypeRef<CEffectCreator>::Bind(Ctx);
	else if (m_pType)
		return m_pType->BindDesign(Ctx);

	return NOERROR;

	DEBUG_CATCH
	}

ALERROR CEffectCreatorRef::CreateBeamEffect (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)
	{
	ALERROR error;

	if (error = CEffectCreator::CreateBeamEffect(Ctx, pDesc, sUNID, &m_pType))
		return error;

	m_dwUNID = 0;
	m_bDelete = true;

	return NOERROR;
	}

ALERROR CEffectCreatorRef::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)
	{
	ALERROR error;

	if (error = CEffectCreator::CreateFromXML(Ctx, pDesc, sUNID, &m_pType))
		return error;

	m_dwUNID = 0;
	m_bDelete = true;

	return NOERROR;
	}

IEffectPainter *CEffectCreatorRef::CreatePainter (CCreatePainterCtx &Ctx)

//	CreatePainter
//
//	Use this call when we want to use a per-owner singleton.

	{
	if (m_pType == NULL)
		return NULL;

	//	If we have a singleton, then return that.

	if (m_pSingleton)
		return m_pSingleton;

	IEffectPainter *pPainter = m_pType->CreatePainter(Ctx);

	//	If we're an owner singleton then we only need to create this once.

	if (m_pType->GetInstance() == CEffectCreator::instOwner
			&& !pPainter->IsSingleton())
		{
		pPainter->SetSingleton(true);
		m_pSingleton = pPainter;

#ifdef DEBUG_SINGLETON_EFFECTS
		::kernelDebugLogMessage("Create singleton painter UNID = %08x: %08x", m_dwUNID, (DWORD)pPainter);
#endif
		}

	//	Done

	return pPainter;
	}

ALERROR CEffectCreatorRef::LoadEffect (SDesignLoadCtx &Ctx, const CString &sUNID, CXMLElement *pDesc, const CString &sAttrib)
	{
	ALERROR error;

	if (pDesc)
		{
		if (error = CreateFromXML(Ctx, pDesc, sUNID))
			return error;
		}
	else
		if (error = LoadUNID(Ctx, sAttrib))
			return error;

	return NOERROR;
	}

ALERROR CEffectCreatorRef::LoadSimpleEffect (SDesignLoadCtx &Ctx, const CString &sUNID, CXMLElement *pDesc)

//	LoadSimpleEffect
//
//	Loads an effect from XML.

	{
	ALERROR error;

	if (strEquals(pDesc->GetTag(), LOOKUP_TAG))
		{
		if (error = LoadUNID(Ctx, pDesc->GetAttribute(EFFECT_ATTRIB)))
			return error;
		}
	else
		{
		if (error = CEffectCreator::CreateSimpleFromXML(Ctx, pDesc, sUNID, &m_pType))
			return error;

		m_dwUNID = 0;
		m_bDelete = true;
		}

	return NOERROR;
	}

void CEffectCreatorRef::Set (CEffectCreator *pEffect)

//	Set
//
//	Sets the effect

	{
	if (m_bDelete && m_pType)
		delete m_pType;

	m_pType = pEffect;
	m_bDelete = false;
	if (m_pType)
		m_dwUNID = m_pType->GetUNID();
	else
		m_dwUNID = 0;
	}

//	CDesignTypeCriteria --------------------------------------------------------

bool CDesignTypeCriteria::MatchesLevel (int iMinLevel, int iMaxLevel) const

//	MatchesLevel
//
//	Returns true if we match the level

	{
	if (m_iGreaterThanLevel != INVALID_COMPARE 
			&& iMaxLevel <= m_iGreaterThanLevel)
		return false;

	if (m_iLessThanLevel != INVALID_COMPARE 
			&& iMinLevel >= m_iLessThanLevel)
		return false;

	return true;
	}

ALERROR CDesignTypeCriteria::ParseCriteria (const CString &sCriteria, CDesignTypeCriteria *retCriteria)

//	ParseCriteria
//
//	Parses the criteria and initializes retCriteria

	{
	//	Initialize

	retCriteria->m_dwTypeSet = 0;
	retCriteria->m_iGreaterThanLevel = INVALID_COMPARE;
	retCriteria->m_iLessThanLevel = INVALID_COMPARE;
	retCriteria->m_bIncludeVirtual = false;

	//	Parse

	char *pPos = sCriteria.GetPointer();
	while (*pPos != '\0')
		{
		switch (*pPos)
			{
			case '*':
				retCriteria->m_dwTypeSet = designSetAll;
				break;

			case charAdventureDesc:
				retCriteria->m_dwTypeSet |= (1 << designAdventureDesc);
				break;

			case charItemTable:
				retCriteria->m_dwTypeSet |= (1 << designItemTable);
				break;

			case charEffectType:
				retCriteria->m_dwTypeSet |= (1 << designEffectType);
				break;

			case charDockScreen:
				retCriteria->m_dwTypeSet |= (1 << designDockScreen);
				break;

			case charSpaceEnvironmentType:
				retCriteria->m_dwTypeSet |= (1 << designSpaceEnvironmentType);
				break;

			case charEconomyType:
				retCriteria->m_dwTypeSet |= (1 << designEconomyType);
				break;

			case charEnergyFieldType:
				retCriteria->m_dwTypeSet |= (1 << designEnergyFieldType);
				break;

			case charGenericType:
				retCriteria->m_dwTypeSet |= (1 << designGenericType);
				break;

			case charGlobals:
				retCriteria->m_dwTypeSet |= (1 << designGlobals);
				break;

			case charShipTable:
				retCriteria->m_dwTypeSet |= (1 << designShipTable);
				break;

			case charItemType:
				retCriteria->m_dwTypeSet |= (1 << designItemType);
				break;

			case charImage:
				retCriteria->m_dwTypeSet |= (1 << designImage);
				break;

			case charMissionType:
				retCriteria->m_dwTypeSet |= (1 << designMissionType);
				break;

			case charPower:
				retCriteria->m_dwTypeSet |= (1 << designPower);
				break;

			case charShipClass:
				retCriteria->m_dwTypeSet |= (1 << designShipClass);
				break;

			case charStationType:
				retCriteria->m_dwTypeSet |= (1 << designStationType);
				break;

			case charSound:
				retCriteria->m_dwTypeSet |= (1 << designSound);
				break;

			case charSovereign:
				retCriteria->m_dwTypeSet |= (1 << designSovereign);
				break;

			case charSystemTable:
				retCriteria->m_dwTypeSet |= (1 << designSystemTable);
				break;

			case charSystemType:
				retCriteria->m_dwTypeSet |= (1 << designSystemType);
				break;

			case charSystemMap:
				retCriteria->m_dwTypeSet |= (1 << designSystemMap);
				break;

			case charNameGenerator:
				retCriteria->m_dwTypeSet |= (1 << designNameGenerator);
				break;

			case charTemplateType:
				//	We don't support enumerating template types
				break;

			case 'L':
				{
				int iHigh;
				int iLow;

				if (ParseCriteriaParamLevelRange(&pPos, &iLow, &iHigh))
					{
					if (iHigh == -1)
						{
						retCriteria->m_iGreaterThanLevel = iLow - 1;
						retCriteria->m_iLessThanLevel = iLow + 1;
						}
					else
						{
						retCriteria->m_iGreaterThanLevel = iLow - 1;
						retCriteria->m_iLessThanLevel = iHigh + 1;
						}
					}

				break;
				}

			case 'V':
				retCriteria->m_bIncludeVirtual = true;
				break;

			case '+':
			case '-':
				{
				bool bRequired = (*pPos == '+');
				bool bBinaryParam;
				CString sParam = ParseCriteriaParam(&pPos, false, &bBinaryParam);

				if (bRequired)
					{
					if (bBinaryParam)
						retCriteria->m_sRequireSpecial.Insert(sParam);
					else
						retCriteria->m_sRequire.Insert(sParam);
					}
				else
					{
					if (bBinaryParam)
						retCriteria->m_sExcludeSpecial.Insert(sParam);
					else
						retCriteria->m_sExclude.Insert(sParam);
					}
				break;
				}

			case '=':
			case '>':
			case '<':
				{
				char chChar = *pPos;
				pPos++;

				//	<= or >=

				int iEqualAdj;
				if (*pPos == '=')
					{
					pPos++;
					iEqualAdj = 1;
					}
				else
					iEqualAdj = 0;

				//	Is this price?

				char comparison;
				if (*pPos == '$' || *pPos == '#')
					comparison = *pPos++;
				else
					comparison = '\0';

				//	Get the number

				char *pNewPos;
				int iValue = strParseInt(pPos, 0, &pNewPos);

				//	Back up one because we will increment at the bottom
				//	of the loop.

				if (pPos != pNewPos)
					pPos = pNewPos - 1;

				//	Level limits

				if (chChar == '=')
					{
					retCriteria->m_iGreaterThanLevel = iValue - 1;
					retCriteria->m_iLessThanLevel = iValue + 1;
					}
				else if (chChar == '>')
					retCriteria->m_iGreaterThanLevel = iValue - iEqualAdj;
				else if (chChar == '<')
					retCriteria->m_iLessThanLevel = iValue + iEqualAdj;

				break;
				}
			}

		pPos++;
		}

	return NOERROR;
	}

//	Utility -------------------------------------------------------------------

CString ParseAchievementSection (ICCItem *pItem)
	{
	if (pItem == NULL)
		return NULL_STR;
	else if (pItem->IsNil())
		return NULL_STR;
	else
		return pItem->GetStringValue();
	}

CString ParseAchievementSort (ICCItem *pItem)
	{
	if (pItem == NULL)
		return NULL_STR;
	else if (pItem->IsNil())
		return NULL_STR;
	else if (pItem->IsInteger())
		return strPatternSubst(CONSTLIT("%08x"), pItem->GetIntegerValue());
	else
		return pItem->GetStringValue();
	}

CString ParseAchievementValue (ICCItem *pItem)
	{
	if (pItem == NULL)
		return NULL_STR;
	else if (pItem->IsNil())
		return NULL_STR;
	else if (pItem->IsInteger())
		return strFormatInteger(pItem->GetIntegerValue(), -1, FORMAT_THOUSAND_SEPARATOR | FORMAT_UNSIGNED);
	else
		return pItem->GetStringValue();
	}