//	CItem.cpp
//
//	CItem class

#include "PreComp.h"

#ifdef DEBUG
//#define DEBUG_ITEM_NAME_MATCH
#endif

#define CAN_BE_INSTALLED_EVENT					CONSTLIT("CanBeInstalled")
#define CAN_BE_UNINSTALLED_EVENT				CONSTLIT("CanBeUninstalled")
#define ON_ADDED_AS_ENHANCEMENT_EVENT			CONSTLIT("OnAddedAsEnhancement")
#define ON_DISABLED_EVENT						CONSTLIT("OnDisable")
#define ON_ENABLED_EVENT						CONSTLIT("OnEnable")
#define ON_INSTALL_EVENT						CONSTLIT("OnInstall")
#define ON_OBJ_DESTROYED_EVENT					CONSTLIT("OnObjDestroyed")
#define ON_REACTOR_OVERLOAD_EVENT				CONSTLIT("OnReactorOverload")
#define ON_REMOVED_AS_ENHANCEMENT_EVENT			CONSTLIT("OnRemovedAsEnhancement")
#define ON_UNINSTALL_EVENT						CONSTLIT("OnUninstall")

#define PROPERTY_CATEGORY						CONSTLIT("category")
#define PROPERTY_CHARGES						CONSTLIT("charges")
#define PROPERTY_DAMAGED						CONSTLIT("damaged")
#define PROPERTY_DESCRIPTION					CONSTLIT("description")
#define PROPERTY_DISRUPTED						CONSTLIT("disrupted")
#define PROPERTY_INC_CHARGES					CONSTLIT("incCharges")
#define PROPERTY_INSTALLED						CONSTLIT("installed")

CItemEnhancement CItem::m_NullMod;
CItem CItem::m_NullItem;

CItem::CItem (void) : 
		m_pItemType(NULL), 
		m_dwCount(0),
		m_dwFlags(0),
		m_dwInstalled(0xff),
		m_pExtra(NULL)

//	CItem constructor

	{
	}

CItem::CItem (const CItem &Copy)

//	CItem constructor

	{
	m_pItemType = Copy.m_pItemType;
	m_dwCount = Copy.m_dwCount;
	m_dwFlags = Copy.m_dwFlags;
	m_dwInstalled = Copy.m_dwInstalled;

	if (Copy.m_pExtra)
		m_pExtra = new SExtra(*Copy.m_pExtra);
	else
		m_pExtra = NULL;
	}

CItem::CItem (CItemType *pItemType, int iCount)

//	CItem constructor

	{
	ASSERT(pItemType);

	m_pItemType = pItemType;
	m_dwCount = (DWORD)iCount;
	m_dwFlags = 0;
	m_dwInstalled = 0xff;
	m_pExtra = NULL;

	int iCharges = pItemType->GetCharges();
	if (iCharges)
		SetCharges(iCharges);
	}

CItem::~CItem (void)

//	CItem destructor

	{
	if (m_pExtra)
		delete m_pExtra;
	}

CItem &CItem::operator= (const CItem &Copy)

//	CItem equals operator

	{
	if (m_pExtra)
		delete m_pExtra;

	m_pItemType = Copy.m_pItemType;
	m_dwCount = Copy.m_dwCount;
	m_dwFlags = Copy.m_dwFlags;
	m_dwInstalled = Copy.m_dwInstalled;

	if (Copy.m_pExtra)
		m_pExtra = new SExtra(*Copy.m_pExtra);
	else
		m_pExtra = NULL;

	return *this;
	}

DWORD CItem::AddEnhancement (const CItemEnhancement &Enhancement)

//	AddEnhancement
//
//	Adds the enhancement and returns the ID

	{
	//	Deal with NOP

	if (Enhancement.IsEmpty() && GetMods().IsEmpty())
		return OBJID_NULL;

	//	Allocate extra and assign the enhancement

	Extra();
	m_pExtra->m_Mods = Enhancement;

	//	When removing a mod by setting to modcode 0, clear everything

	if (m_pExtra->m_Mods.IsEmpty())
		m_pExtra->m_Mods.SetID(OBJID_NULL);

	//	Otherwise, if we don't have an ID, set it now

	else if (m_pExtra->m_Mods.GetID() == OBJID_NULL)
		m_pExtra->m_Mods.SetID(g_pUniverse->CreateGlobalID());

	return m_pExtra->m_Mods.GetID();
	}

CItem CItem::CreateItemByName (const CString &sName, const CItemCriteria &Criteria, bool bActualName)

//	CreateItemByName
//
//	Generates an item structure based on natural language
//	name. If there is ambiguity, a random item is chosen.

	{
	int i;

	CIntArray List;

	//	Prepare the input name

	TArray<CString> Input;
	NLSplit(sName, &Input);

	//	If we've got nothing, then nothing matches

	if (Input.GetCount() == 0)
		return CItem();

	//	Loop over all items

	int iBestMatch = -1000;
	int iBestNonCriteriaMatch = -1000;
	for (i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
		{
		CItemType *pType = g_pUniverse->GetItemType(i);
		CItem Item(pType, 1);

		//	Compare names

		TArray<CString> ItemName;
		NLSplit(pType->GetNounPhrase(bActualName ? nounActual : 0), &ItemName);
		int iMatch = NLCompare(Input, ItemName);

		//	If this is not in the criteria, then continue

		if (!Item.MatchesCriteria(Criteria))
			{
			if (iMatch > iBestNonCriteriaMatch)
				iBestNonCriteriaMatch = iMatch;
			continue;
			}

		//	If we have a match, add to the list

		if (iMatch >= iBestMatch)
			{
			if (iMatch > iBestMatch)
				List.RemoveAll();

			List.AppendElement((int)pType, NULL);
			iBestMatch = iMatch;
			}
		}

#ifdef DEBUG_ITEM_NAME_MATCH
	{
	for (i = 0; i < List.GetCount(); i++)
		{
		CItemType *pItem = (CItemType *)List.GetElement(i);
		char szBuffer[1024];
		wsprintf(szBuffer, "%s\n", pItem->GetNounPhrase().GetASCIIZPointer());
		::OutputDebugString(szBuffer);
		}
	}
#endif

	//	If we don't have a good match and there is a better match not in the criteria, then
	//	we don't match anything

	if (iBestMatch == 0 && List.GetCount() > 1)
		List.RemoveAll();
	else if (iBestNonCriteriaMatch > iBestMatch && List.GetCount() > 1)
		List.RemoveAll();

	//	Pick a random item

	if (List.GetCount())
		{
		CItemType *pBest = (CItemType *)List.GetElement(mathRandom(0, List.GetCount()-1));
		return CItem(pBest, pBest->GetNumberAppearing().Roll());
		}
	else
		return CItem();
	}

void CItem::Extra (void)

//	Extra
//
//	Make sure that Extra data structure is allocated

	{
	if (m_pExtra == NULL)
		{
		m_pExtra = new SExtra;
		m_pExtra->m_dwCharges = 0;
		m_pExtra->m_dwVariant = 0;
		m_pExtra->m_dwDisruptedTime = 0;
		}
	}

bool CItem::FireCanBeInstalled (CSpaceObject *pSource, CString *retsError) const

//	FireCanBeInstalled
//
//	CanBeInstalled event

	{
	SEventHandlerDesc Event;
	if (m_pItemType->FindEventHandler(CAN_BE_INSTALLED_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.SaveAndDefineItemVar(*this);

		ICCItem *pResult = Ctx.Run(Event);

		bool bCanBeInstalled;
		if (pResult->IsError())
			{
			*retsError = pResult->GetStringValue();
			pSource->ReportEventError(strPatternSubst(CONSTLIT("Item %x CanBeInstalled"), m_pItemType->GetUNID()), pResult);
			bCanBeInstalled = false;
			}
		else if (!pResult->IsTrue())
			{
			*retsError = pResult->GetStringValue();
			bCanBeInstalled = false;
			}
		else
			bCanBeInstalled = true;

		Ctx.Discard(pResult);

		return bCanBeInstalled;
		}
	else
		return true;
	}

bool CItem::FireCanBeUninstalled (CSpaceObject *pSource, CString *retsError) const

//	FireCanBeUninstalled
//
//	CanBeInstalled event

	{
	SEventHandlerDesc Event;
	if (m_pItemType->FindEventHandler(CAN_BE_UNINSTALLED_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.SaveAndDefineItemVar(*this);

		ICCItem *pResult = Ctx.Run(Event);

		bool bCanBeUninstalled;
		if (pResult->IsError())
			{
			*retsError = pResult->GetStringValue();
			pSource->ReportEventError(strPatternSubst(CONSTLIT("Item %x CanBeUninstalled"), m_pItemType->GetUNID()), pResult);
			bCanBeUninstalled = false;
			}
		else if (!pResult->IsTrue())
			{
			*retsError = pResult->GetStringValue();
			bCanBeUninstalled = false;
			}
		else
			bCanBeUninstalled = true;

		Ctx.Discard(pResult);

		return bCanBeUninstalled;
		}
	else
		return true;
	}

void CItem::FireOnAddedAsEnhancement (CSpaceObject *pSource, const CItem &ItemEnhanced, EnhanceItemStatus iStatus) const

//	FireOnAddedAsEnhancement
//
//	OnAddedAsEnhancement event

	{
	SEventHandlerDesc Event;
	if (m_pItemType->FindEventHandler(ON_ADDED_AS_ENHANCEMENT_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.SaveAndDefineItemVar(ItemEnhanced);
		Ctx.DefineInteger(CONSTLIT("aResult"), (int)iStatus);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			pSource->ReportEventError(strPatternSubst(CONSTLIT("Item %x OnAddedAsEnhancement"), m_pItemType->GetUNID()), pResult);
		Ctx.Discard(pResult);
		}
	}

void CItem::FireOnDisabled (CSpaceObject *pSource) const

//	FireOnDisabled
//
//	OnDisabled event

	{
	SEventHandlerDesc Event;
	if (m_pItemType->FindEventHandler(ON_DISABLED_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.SaveAndDefineItemVar(*this);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			pSource->ReportEventError(strPatternSubst(CONSTLIT("Item %x OnDisable"), m_pItemType->GetUNID()), pResult);
		Ctx.Discard(pResult);
		}
	}

void CItem::FireOnEnabled (CSpaceObject *pSource) const

//	FireOnEnabled
//
//	OnEnabled event

	{
	SEventHandlerDesc Event;
	if (m_pItemType->FindEventHandlerItemType(CItemType::evtOnEnabled, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.SaveAndDefineItemVar(*this);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			pSource->ReportEventError(strPatternSubst(CONSTLIT("Item %x OnEnable"), m_pItemType->GetUNID()), pResult);
		Ctx.Discard(pResult);
		}
	}

void CItem::FireOnInstall (CSpaceObject *pSource) const

//	FireOnInstall
//
//	OnInstall event

	{
	SEventHandlerDesc Event;
	if (m_pItemType->FindEventHandlerItemType(CItemType::evtOnInstall, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.SaveAndDefineItemVar(*this);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			pSource->ReportEventError(strPatternSubst(CONSTLIT("Item %x OnInstall"), m_pItemType->GetUNID()), pResult);
		Ctx.Discard(pResult);
		}
	}

void CItem::FireOnObjDestroyed (CSpaceObject *pSource, const SDestroyCtx &Ctx) const

//	FireOnObjDestroyed
//
//	OnObjDestroyed event

	{
	SEventHandlerDesc Event;

	if (m_pItemType->FindEventHandler(CDesignType::evtOnObjDestroyed, &Event))
		{
		CCodeChainCtx CCCtx;

		CCCtx.SaveAndDefineSourceVar(pSource);
		CCCtx.SaveAndDefineItemVar(*this);
		CCCtx.DefineSpaceObject(CONSTLIT("aObjDestroyed"), Ctx.pObj);
		CCCtx.DefineSpaceObject(CONSTLIT("aDestroyer"), Ctx.Attacker.GetObj());
		CCCtx.DefineSpaceObject(CONSTLIT("aOrderGiver"), (Ctx.Attacker.GetObj() ? Ctx.Attacker.GetObj()->GetOrderGiver(Ctx.iCause) : NULL));
		CCCtx.DefineSpaceObject(CONSTLIT("aWreckObj"), Ctx.pWreck);
		CCCtx.DefineString(CONSTLIT("aDestroyReason"), GetDestructionName(Ctx.iCause));

		ICCItem *pResult = CCCtx.Run(Event);
		if (pResult->IsError())
			pSource->ReportEventError(strPatternSubst(CONSTLIT("Item %x OnObjDestroyed"), m_pItemType->GetUNID()), pResult);
		CCCtx.Discard(pResult);
		}
	}

bool CItem::FireOnReactorOverload (CSpaceObject *pSource) const

//	FireOnReactorOverload
//
//	OnReactorOverload event

	{
	SEventHandlerDesc Event;
	if (m_pItemType->FindEventHandler(ON_REACTOR_OVERLOAD_EVENT, &Event))
		{
		CCodeChainCtx Ctx;
		bool bHandled = false;

		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.SaveAndDefineItemVar(*this);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			pSource->ReportEventError(strPatternSubst(CONSTLIT("Item %x OnReactorOverload"), m_pItemType->GetUNID()), pResult);
		else
			bHandled = !pResult->IsNil();

		Ctx.Discard(pResult);
		return bHandled;
		}
	else
		return false;
	}

void CItem::FireOnRemovedAsEnhancement (CSpaceObject *pSource, const CItem &ItemEnhanced) const

//	FireOnRemovedAsEnhancement
//
//	OnRemovedAsEnhancement event

	{
	SEventHandlerDesc Event;
	if (m_pItemType->FindEventHandler(ON_REMOVED_AS_ENHANCEMENT_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.SaveAndDefineItemVar(ItemEnhanced);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			pSource->ReportEventError(strPatternSubst(CONSTLIT("Item %x OnRemovedAsEnhancement"), m_pItemType->GetUNID()), pResult);
		Ctx.Discard(pResult);
		}
	}

void CItem::FireOnUninstall (CSpaceObject *pSource) const

//	FireOnUninstall
//
//	OnUninstall event

	{
	SEventHandlerDesc Event;
	if (m_pItemType->FindEventHandler(ON_UNINSTALL_EVENT, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.SaveAndDefineItemVar(*this);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			pSource->ReportEventError(strPatternSubst(CONSTLIT("Item %x OnUninstall"), m_pItemType->GetUNID()), pResult);
		Ctx.Discard(pResult);
		}
	}

CString CItem::GetDesc (void) const

//	GetDesc
//
//	Gets the item description
	
	{
	//	If we have code, call it to generate the description

	SEventHandlerDesc Event;
	if (m_pItemType->FindEventHandlerItemType(CItemType::evtGetDescription, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SetEvent(eventGetDescription);
		Ctx.SetItemType(GetType());
		Ctx.SaveAndDefineSourceVar(NULL);
		Ctx.SaveAndDefineItemVar(*this);

		ICCItem *pResult = Ctx.Run(Event);
		CString sDesc = pResult->GetStringValue();
		Ctx.Discard(pResult);

		return sDesc;
		}

	//	Otherwise, get it from the item

	else
		return m_pItemType->GetDesc(); 
	}

bool CItem::GetDisplayAttributes (CItemCtx &Ctx, TArray<SDisplayAttribute> *retList) const

//	GetDisplayAttributes
//
//	Returns a list of display attributes.

	{
	//	Empty and check for edge cases

	retList->DeleteAll();
	if (m_pItemType == NULL)
		return false;

	//	Always add level

	retList->Insert(SDisplayAttribute(attribNeutral, strPatternSubst(CONSTLIT("level %d"), m_pItemType->GetApparentLevel())));

	//	Add additional custom attributes

	if (m_pItemType->IsKnown())
		g_pUniverse->GetAttributeDesc().AccumulateAttributes(*this, retList);

	//	Add some device-specific attributes

	CDeviceClass *pDeviceClass = m_pItemType->GetDeviceClass();
	if (pDeviceClass)
		{
		CInstalledDevice *pDevice = Ctx.GetDevice();

		//	External devices

		if (pDeviceClass->IsExternal() || (pDevice && pDevice->IsExternal()))
			retList->Insert(SDisplayAttribute(attribNeutral, CONSTLIT("external")));
		}

	//	Military and Illegal attributes

	if (m_pItemType->IsKnown()
			&& m_pItemType->HasLiteralAttribute(CONSTLIT("Military")))
		retList->Insert(SDisplayAttribute(attribPositive, CONSTLIT("military")));

	if (m_pItemType->IsKnown()
			&& m_pItemType->HasLiteralAttribute(CONSTLIT("Illegal")))
		retList->Insert(SDisplayAttribute(attribNegative, CONSTLIT("illegal")));

	//	Add various engine-based attributes

	if (IsDamaged())
		retList->Insert(SDisplayAttribute(attribNegative, CONSTLIT("damaged")));

	if (IsDisrupted())
		retList->Insert(SDisplayAttribute(attribNegative, CONSTLIT("ionized")));

	//	Add any enhancements as a display attribute

	CString sEnhanced = GetEnhancedDesc(Ctx.GetSource());
	if (!sEnhanced.IsBlank())
		{
		bool bDisadvantage = (*(sEnhanced.GetASCIIZPointer()) == '-');
		retList->Insert(SDisplayAttribute((bDisadvantage ? attribNegative : attribPositive), sEnhanced));
		}

	//	Done

	return (retList->GetCount() > 0);
	}

DWORD CItem::GetDisruptedDuration (void) const

//	GetDisruptedDuration
//
//	Returns how long the item will be disrupted for

	{
	if (m_pExtra == NULL || m_pExtra->m_dwDisruptedTime == 0)
		return 0;
	else if (m_pExtra->m_dwDisruptedTime == INFINITE_TICK)
		return INFINITE_TICK;
	else
		{
		DWORD dwNow = (DWORD)g_pUniverse->GetTicks();
		if (m_pExtra->m_dwDisruptedTime <= dwNow)
			return 0;
		else
			return (m_pExtra->m_dwDisruptedTime - dwNow);
		}
	}

CString CItem::GetEnhancedDesc (CSpaceObject *pInstalled) const

//	GetEnhancedDesc
//
//	Returns a string that describes the kind of enhancement
//	for this item.

	{
	CInstalledDevice *pDevice;

	//	If this is a device, then ask the device to describe the
	//	enhancements

	if (pInstalled && IsInstalled() && (pDevice = pInstalled->FindDevice(*this)))
		return pDevice->GetEnhancedDesc(pInstalled, this);

	//	Deal with complex enhancements
	//	(if a device happens to have the same type of enhancement, this
	//	function will add the two enhancements together)

	if (GetMods().IsNotEmpty())
		return GetMods().GetEnhancedDesc(*this, pInstalled);

	//	Otherwise, generic enhancement

	else if (IsEnhanced())
		return CONSTLIT("+Enhanced");

	//	Otherwise, not enhanced

	else
		return NULL_STR;
	}

int CItem::GetMassKg (void) const

//	GetMass
//
//	Returns the mass of a single unit of the item type.

	{
	return m_pItemType->GetMassKg(CItemCtx(*this));
	}

CString CItem::GetNounPhrase (DWORD dwFlags) const

//	GetNounPhrase
//
//	Returns a noun phrase that represents this item

	{
	CString sName;
	CString sModifier;
	DWORD dwNounFlags = 0;

	//	If we have code, call it to generate the name

	SEventHandlerDesc Event;
	if (m_pItemType->FindEventHandlerItemType(CItemType::evtGetName, &Event)
			&& !(dwFlags & nounNoEvent))
		{
		CCodeChainCtx Ctx;

		Ctx.SetEvent(eventGetName);
		Ctx.SetItemType(GetType());
		Ctx.SaveAndDefineSourceVar(NULL);
		Ctx.SaveAndDefineItemVar(*this);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			{
			sName = pResult->GetStringValue();
			dwNounFlags = 0;
			}
		else if (pResult->IsList() && pResult->GetCount() >= 2)
			{
			sName = pResult->GetElement(0)->GetStringValue();
			dwNounFlags = pResult->GetElement(1)->GetIntegerValue();
			}
		else
			{
			sName = pResult->GetStringValue();
			dwNounFlags = 0;
			}
		Ctx.Discard(pResult);
		}

	//	Otherwise, get it from the item type

	else
		{
		sName = m_pItemType->GetName(&dwNounFlags, ((dwFlags & nounActual) ? true : false));

		//	Modifiers

		if ((dwFlags & nounInstalledState)
				&& IsInstalled())
			sModifier = CONSTLIT("installed ");

		if (!(dwFlags & nounNoModifiers))
			{
			if (IsDamaged() || (GetMods().IsDisadvantage()))
				sModifier.Append(CONSTLIT("damaged "));
			else if (IsEnhanced() || GetMods().IsEnhancement())
				sModifier.Append(CONSTLIT("enhanced "));
			}
		}

	//	Compose phrase

	return ComposeNounPhrase(sName, (int)m_dwCount, sModifier, dwNounFlags, dwFlags);
	}

ICCItem *CItem::GetProperty (CCodeChainCtx *pCCCtx, CItemCtx &Ctx, const CString &sName) const

//	GetProperty
//
//	Returns an item property. Caller must free ICCItem.

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	if (strEquals(sName, PROPERTY_CATEGORY))
		return CC.CreateString(GetItemCategoryID(m_pItemType->GetCategory()));

	else if (strEquals(sName, PROPERTY_CHARGES))
		return CC.CreateInteger(GetCharges());

	else if (strEquals(sName, PROPERTY_DAMAGED))
		return CC.CreateBool(IsDamaged());

	else if (strEquals(sName, PROPERTY_DESCRIPTION))
		{
		if (pCCCtx && pCCCtx->InEvent(eventGetDescription))
			return CC.CreateString(m_pItemType->GetDesc());
		else
			return CC.CreateString(GetDesc());
		}

	else if (strEquals(sName, PROPERTY_DISRUPTED))
		return CC.CreateBool(IsDisrupted());

	else if (strEquals(sName, PROPERTY_INSTALLED))
		return CC.CreateBool(IsInstalled());

	else
		{
		CDeviceClass *pDevice;
		CArmorClass *pArmor;

		//	If this is a device, then pass it on

		if (pDevice = GetType()->GetDeviceClass())
			return pDevice->GetItemProperty(Ctx, sName);

		//	If this is armor, then pass it on

		else if (pArmor = GetType()->GetArmorClass())
			return pArmor->GetItemProperty(Ctx, sName);

		//	Otherwise, from the type

		else
			return CreateResultFromDataField(CC, GetType()->GetDataField(sName));
		}
	}

CString CItem::GetReference (CItemCtx &Ctx, int iVariant, DWORD dwFlags) const

//	GetReference
//
//	Returns a string that describes the basic stats for this
//	item

	{
	if (m_pItemType == NULL)
		return NULL_STR;

	//	Done

	return m_pItemType->GetReference(Ctx, iVariant, dwFlags);
	}

bool CItem::GetReferenceDamageAdj (CSpaceObject *pInstalled, DWORD dwFlags, int *retiHP, int *retArray) const

//	GetReferenceDamageAdj
//
//	Sets the damage type array with values for the item. Returns TRUE if any
//	values are filled

	{
	CArmorClass *pArmor;
	CDeviceClass *pDevice;

	//	No reference if unknown

	if (!m_pItemType->IsKnown() && !(dwFlags & CItemType::FLAG_ACTUAL_ITEM))
		return false;

	//	Return armor reference, if this is armor

	if (pArmor = m_pItemType->GetArmorClass())
		return pArmor->GetReferenceDamageAdj(this, pInstalled, retiHP, retArray);

	//	Return device reference, if this is a device

	else if (pDevice = m_pItemType->GetDeviceClass())
		return pDevice->GetReferenceDamageAdj(this, pInstalled, retiHP, retArray);

	//	Otherwise, nothing

	else
		return false;
	}

bool CItem::GetReferenceDamageType (CSpaceObject *pInstalled, int iVariant, DWORD dwFlags, DamageTypes *retiDamage, CString *retsReference) const

//	GetReferenceDamageType
//
//	Returns the damage type and text reference

	{
	CDeviceClass *pDevice;
	int iShotVariant;

	//	No reference if unknown

	if (!m_pItemType->IsKnown() && !(dwFlags & CItemType::FLAG_ACTUAL_ITEM))
		return false;

	//	Return device reference, if this is a device

	if (pDevice = m_pItemType->GetDeviceClass())
		{
		CItemCtx Ctx(this, pInstalled, (CInstalledDevice *)NULL);
		return pDevice->GetReferenceDamageType(Ctx, 
				iVariant, 
				retiDamage, 
				retsReference);
		}

	//	If a missile, then get the damage from the weapon

	else if (m_pItemType->IsMissile() && (pDevice = m_pItemType->GetAmmoLauncher(&iShotVariant)))
		{
		//	We only return damage type reference for missiles (not for ammo, since that is already
		//	accounted for in the weapon)

		if (pDevice->GetCategory() == itemcatLauncher)
			{
			CItemCtx Ctx(this, pInstalled, (CInstalledDevice *)NULL);
			return pDevice->GetReferenceDamageType(Ctx, 
					iShotVariant, 
					retiDamage, 
					retsReference);
			}
		else
			return NULL;
		}

	//	Otherwise, nothing

	else
		return false;
	}

int CItem::GetTradePrice (CSpaceObject *pObj, bool bActual) const

//	GetTradePrice
//
//	Returns the price that pObj would pay to buy the item
//	(or the price that pObj would accept to sell the item)
//
//	Normally this is identical to GetValue(), but this call allows
//	items to compute a different value based on other factors.

	{
	SEventHandlerDesc Event;
	if (m_pItemType->FindEventHandlerItemType(CItemType::evtGetTradePrice, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SetEvent(eventGetTradePrice);
		Ctx.SetItemType(GetType());
		Ctx.SaveAndDefineSourceVar(pObj);
		Ctx.SaveAndDefineItemVar(*this);
		Ctx.DefineString(CONSTLIT("aPriceType"), (bActual ? CONSTLIT("actual") : CONSTLIT("normal")));

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			pObj->ReportEventError(strPatternSubst(CONSTLIT("Item %x GetTradePrice"), m_pItemType->GetUNID()), pResult);
		int iPrice = pResult->GetIntegerValue();
		Ctx.Discard(pResult);

		return iPrice;
		}
	else
		return GetValue(bActual);
	}

int CItem::GetValue (bool bActual) const

//	GetValue
//
//	Returns the value of a single item in this group. Caller must multiply by
//	GetCount() to find the value of the whole item group. The value takes into
//	account damage, enhancements, and charges.

	{
	CItemCtx Ctx(*this);

	//	Compute value (this includes charges)

	int iValue = m_pItemType->GetValue(Ctx, bActual);

	//	Adjust value based on enhancements

	int iAdj;

	//	Deal with enhancements

	if (GetMods().IsNotEmpty())
		iAdj = GetMods().GetValueAdj(*this);

	//	Otherwise, generic enhancement

	else
		iAdj = (IsEnhanced() ? 75 : 0);

	//	Increase value

	iValue += iValue * iAdj / 100;

	//	If the item is damaged, value is only 20% of full value

	if (IsDamaged())
		iValue = iValue * 20 / 100;

	return iValue;
	}

bool CItem::HasSpecialAttribute (const CString &sAttrib) const

//	HasSpecialAttribute
//
//	Returns TRUE if we have the given special attribute

	{
	return m_pItemType->HasSpecialAttribute(sAttrib);
	}

bool CItem::IsEqual (const CItem &Item, bool bIgnoreInstalled) const

//	IsEqual
//
//	Returns TRUE if this item is the same as the given
//	item except for the count

	{
	return (m_pItemType == Item.m_pItemType
			&& m_dwFlags == Item.m_dwFlags
			&& (bIgnoreInstalled || m_dwInstalled == Item.m_dwInstalled)
			&& IsExtraEqual(Item.m_pExtra));
	}

bool CItem::IsExtraEmpty (const SExtra *pExtra)

//	IsExtraEmpty
//
//	Returns TRUE is the given pExtra struct is empty
//	(set to initial values)

	{
	return (pExtra->m_dwCharges == 0
			&& pExtra->m_dwVariant == 0
			&& pExtra->m_dwDisruptedTime == 0
			&& pExtra->m_Mods.IsEmpty()
			&& pExtra->m_Data.IsEmpty());
	}

bool CItem::IsExtraEqual (SExtra *pSrc) const

//	IsExtraEqual
//
//	Returns TRUE if this item's Extra struct is the same as the source

	{
	//	Both have extra struct

	if (m_pExtra && pSrc)
		{
		return (m_pExtra->m_dwCharges == pSrc->m_dwCharges
				&& m_pExtra->m_dwVariant == pSrc->m_dwVariant
				&& m_pExtra->m_dwDisruptedTime == pSrc->m_dwDisruptedTime
				&& m_pExtra->m_Mods.IsEqual(pSrc->m_Mods)
				&& m_pExtra->m_Data.IsEqual(pSrc->m_Data));
		}

	//	Neither has extra struct

	else if (m_pExtra == pSrc)
		return true;

	//	One has extra struct, but it's empty

	else if (m_pExtra == NULL)
		return IsExtraEmpty(pSrc);
	else if (pSrc == NULL)
		return IsExtraEmpty(m_pExtra);

	//	Extra structs don't match at all

	else
		return false;
	}

bool CItem::MatchesCriteria (const CItemCriteria &Criteria) const

//	MatchesCriteria
//
//	Return TRUE if this item matches the given criteria

	{
	int i;

	if (m_pItemType == NULL)
		return false;

	//	If we've got a filter, then use that

	if (Criteria.pFilter)
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		//	Create a list representing this item

		ICCItem *pItem = ::CreateListFromItem(CC, *this);

		//	Create an argument list consisting of the item

		ICCItem *pArgs = CC.CreateLinkedList();
		if (pArgs->IsError())
			{
			ASSERT(false);
			return false;
			}

		CCLinkedList *pList = (CCLinkedList *)pArgs;
		pList->Append(&CC, pItem, NULL);

		//	Apply the function to the arg list

		ICCItem *pResult = CC.Apply(Criteria.pFilter, pArgs, &g_pUniverse);
		bool bMatches = !pResult->IsNil();

		//	Done

		pResult->Discard(&CC);
		pArgs->Discard(&CC);
		pItem->Discard(&CC);

		return bMatches;
		}

	//	If we're looking for anything, then continue

	if (Criteria.dwItemCategories == 0xFFFFFFFF)
		NULL;

	//	If we're looking for fuel and this item is fuel, then
	//	we continue

	else if ((Criteria.dwItemCategories & itemcatFuel)
			&& m_pItemType->IsFuel())
		NULL;

	//	If we're looking for missiles and this item is a
	//	missile, then we continue.

	else if ((Criteria.dwItemCategories & itemcatMissile)
			&& m_pItemType->IsMissile())
		NULL;

	//	If we're looking for usable items and this item is
	//	isable, then we continue

	else if ((Criteria.dwItemCategories & itemcatUseful)
			&& m_pItemType->IsUsable())
		NULL;

	//	Otherwise, if this is not one of the required categories, bail out

	else if (!(Criteria.dwItemCategories & m_pItemType->GetCategory()))
		return false;

	//	Deal with exclusion

	if (Criteria.dwExcludeCategories == 0)
		NULL;
	else if ((Criteria.dwExcludeCategories & itemcatFuel) && m_pItemType->IsFuel())
		return false;
	else if ((Criteria.dwExcludeCategories & itemcatMissile) && m_pItemType->IsMissile())
		return false;
	else if ((Criteria.dwExcludeCategories & itemcatUseful) && m_pItemType->IsUsable())
		return false;
	else if (Criteria.dwExcludeCategories & m_pItemType->GetCategory())
		return false;

	//	Deal with must have

	if (Criteria.dwMustHaveCategories != 0)
		{
		if ((Criteria.dwMustHaveCategories & itemcatFuel) && !m_pItemType->IsFuel())
			return false;
		if ((Criteria.dwMustHaveCategories & itemcatMissile) && !m_pItemType->IsMissile())
			return false;
		if ((Criteria.dwMustHaveCategories & itemcatUseful) && !m_pItemType->IsUsable())
			return false;

		if ((Criteria.dwMustHaveCategories & itemcatDeviceMask) == itemcatDeviceMask)
			{
			if (!(m_pItemType->GetCategory() & itemcatDeviceMask))
				return false;
			}
		else if ((Criteria.dwMustHaveCategories & itemcatWeaponMask) == itemcatWeaponMask)
			{
			if (!(m_pItemType->GetCategory() & itemcatWeaponMask))
				return false;
			}
		else
			{
			DWORD dwCat = 1;
			for (i = 0; i < itemcatCount; i++)
				{
				if (dwCat != itemcatFuel && dwCat != itemcatMissile && dwCat != itemcatUseful
						&& (Criteria.dwMustHaveCategories & dwCat)
						&& m_pItemType->GetCategory() != dwCat)
					return false;
				
				dwCat = dwCat << 1;
				}
			}
		}

	//	If any of the flags that must be set is not set, then
	//	we do not match.

	if ((Criteria.wFlagsMustBeSet & m_dwFlags) != Criteria.wFlagsMustBeSet)
		return false;

	//	If any of the flags that must be cleared is set, then
	//	we do not match.

	if ((Criteria.wFlagsMustBeCleared & m_dwFlags) != 0)
		return false;

	//	Check installed flags

	if (Criteria.bInstalledOnly && !IsInstalled())
		return false;

	if (Criteria.bNotInstalledOnly && IsInstalled())
		return false;

	//	Check miscellaneous flags

	if (Criteria.bUsableItemsOnly && m_pItemType->GetUseScreen() == NULL)
		return false;

	if (Criteria.bExcludeVirtual && m_pItemType->IsVirtual())
		return false;

	if (Criteria.bLauncherMissileOnly && m_pItemType->IsAmmunition())
		return false;

	//	Check required modifiers

	for (i = 0; i < Criteria.ModifiersRequired.GetCount(); i++)
		if (!m_pItemType->HasLiteralAttribute(Criteria.ModifiersRequired[i]))
			return false;

	//	Check modifiers not allowed

	for (i = 0; i < Criteria.ModifiersNotAllowed.GetCount(); i++)
		if (m_pItemType->HasLiteralAttribute(Criteria.ModifiersNotAllowed[i]))
			return false;

	//	Check required special attributes

	for (i = 0; i < Criteria.SpecialAttribRequired.GetCount(); i++)
		if (!HasSpecialAttribute(Criteria.SpecialAttribRequired[i]))
			return false;

	//	Check special attributes not allowed

	for (i = 0; i < Criteria.SpecialAttribNotAllowed.GetCount(); i++)
		if (HasSpecialAttribute(Criteria.SpecialAttribNotAllowed[i]))
			return false;

	//	Check frequency range

	if (!Criteria.Frequency.IsBlank())
		{
		int iFreq = m_pItemType->GetFrequency();
		char *pPos = Criteria.Frequency.GetASCIIZPointer();
		bool bMatch = false;
		while (*pPos != '\0' && !bMatch)
			{
			switch (*pPos)
				{
				case 'c':
				case 'C':
					if (iFreq == ftCommon)
						bMatch = true;
					break;

				case 'u':
				case 'U':
					if (iFreq == ftUncommon)
						bMatch = true;
					break;

				case 'r':
				case 'R':
					if (iFreq == ftRare)
						bMatch = true;
					break;

				case 'v':
				case 'V':
					if (iFreq == ftVeryRare)
						bMatch = true;
					break;

				case '-':
				case 'n':
				case 'N':
					if (iFreq == ftNotRandom)
						bMatch = true;
					break;
				}

			pPos++;
			}

		if (!bMatch)
			return false;
		}

	//	Check for level modifiers

	if (Criteria.iEqualToLevel != -1 && m_pItemType->GetLevel() != Criteria.iEqualToLevel)
		return false;

	if (Criteria.iGreaterThanLevel != -1 && m_pItemType->GetLevel() <= Criteria.iGreaterThanLevel)
		return false;

	if (Criteria.iLessThanLevel != -1 && m_pItemType->GetLevel() >= Criteria.iLessThanLevel)
		return false;

	//	Check for price modifiers

	if (Criteria.iEqualToPrice != -1 && GetValue() != Criteria.iEqualToPrice)
		return false;

	if (Criteria.iGreaterThanPrice != -1 && GetValue() <= Criteria.iGreaterThanPrice)
		return false;

	if (Criteria.iLessThanPrice != -1 && GetValue() >= Criteria.iLessThanPrice)
		return false;

	//	Check for mass modifiers

	if (Criteria.iEqualToMass != -1 && GetMassKg() != Criteria.iEqualToMass)
		return false;

	if (Criteria.iGreaterThanMass != -1 && GetMassKg() <= Criteria.iGreaterThanMass)
		return false;

	if (Criteria.iLessThanMass != -1 && GetMassKg() >= Criteria.iLessThanMass)
		return false;

	//	Otherwise, we match completely

	return true;
	}

void CItem::InitCriteriaAll (CItemCriteria *retCriteria)

//	InitCriteriaAll
//
//	Initializes criteria set to all

	{
	retCriteria->dwItemCategories = 0xffffffff;
	retCriteria->dwExcludeCategories = 0;
	retCriteria->dwMustHaveCategories = 0;
	retCriteria->wFlagsMustBeSet = 0;
	retCriteria->wFlagsMustBeCleared = 0;

	retCriteria->bUsableItemsOnly = false;
	retCriteria->bExcludeVirtual = true;
	retCriteria->bInstalledOnly = false;
	retCriteria->bNotInstalledOnly = false;
	retCriteria->bLauncherMissileOnly = false;
	retCriteria->ModifiersRequired.DeleteAll();
	retCriteria->ModifiersNotAllowed.DeleteAll();
	retCriteria->SpecialAttribRequired.DeleteAll();
	retCriteria->SpecialAttribNotAllowed.DeleteAll();
	retCriteria->Frequency = NULL_STR;

	retCriteria->iEqualToLevel = -1;
	retCriteria->iGreaterThanLevel = -1;
	retCriteria->iLessThanLevel = -1;

	retCriteria->iEqualToPrice = -1;
	retCriteria->iGreaterThanPrice = -1;
	retCriteria->iLessThanPrice = -1;

	retCriteria->iEqualToMass = -1;
	retCriteria->iGreaterThanMass = -1;
	retCriteria->iLessThanMass = -1;

	retCriteria->pFilter = NULL;
	}

void WriteCategoryFlags (CMemoryWriteStream &Output, DWORD dwCategories)
	{
	if (dwCategories & itemcatArmor)
		Output.Write("a", 1);
	if (dwCategories & itemcatMiscDevice)
		Output.Write("b", 1);
	if (dwCategories & itemcatCargoHold)
		Output.Write("c", 1);
	if (dwCategories & itemcatFuel)
		Output.Write("f", 1);
	if (dwCategories & itemcatLauncher)
		Output.Write("l", 1);
	if (dwCategories & itemcatMissile)
		Output.Write("m", 1);
	if (dwCategories & itemcatWeapon)
		Output.Write("p", 1);
	if (dwCategories & itemcatReactor)
		Output.Write("r", 1);
	if (dwCategories & itemcatShields)
		Output.Write("s", 1);
	if (dwCategories & itemcatMisc)
		Output.Write("t", 1);
	if (dwCategories & itemcatUseful)
		Output.Write("u", 1);
	if (dwCategories & itemcatDrive)
		Output.Write("v", 1);
	}

CString CItem::GenerateCriteria (const CItemCriteria &Criteria)

//	GenerateCriteria
//
//	Returns a criteria string from a criteria structure

	{
	int i;
	CMemoryWriteStream Output(64 * 1024);
	CString sTerm;

	if (Output.Create() != NOERROR)
		return NULL_STR;
	
	if (Criteria.dwItemCategories == 0xFFFFFFFF)
		Output.Write("*", 1);
	else
		WriteCategoryFlags(Output, Criteria.dwItemCategories);

	if (Criteria.dwExcludeCategories != 0)
		{
		Output.Write("~", 1);
		WriteCategoryFlags(Output, Criteria.dwExcludeCategories);
		}

	if (Criteria.dwMustHaveCategories != 0)
		{
		Output.Write("&", 1);
		WriteCategoryFlags(Output, Criteria.dwMustHaveCategories);
		}

	if (Criteria.wFlagsMustBeSet & flagDamaged)
		Output.Write("D", 1);

	if (Criteria.wFlagsMustBeCleared & flagDamaged)
		Output.Write("N", 1);

	if (Criteria.bUsableItemsOnly)
		Output.Write("S", 1);

	if (Criteria.bInstalledOnly)
		Output.Write("I", 1);

	if (Criteria.bNotInstalledOnly)
		Output.Write("U", 1);

	if (Criteria.bLauncherMissileOnly)
		Output.Write("M", 1);

	if (!Criteria.bExcludeVirtual)
		Output.Write("V", 1);

	for (i = 0; i < Criteria.ModifiersRequired.GetCount(); i++)
		{
		sTerm = strPatternSubst(CONSTLIT(" +%s;"), Criteria.ModifiersRequired[i]);
		Output.Write(sTerm.GetPointer(), sTerm.GetLength());
		}

	for (i = 0; i < Criteria.ModifiersNotAllowed.GetCount(); i++)
		{
		sTerm = strPatternSubst(CONSTLIT(" -%s;"), Criteria.ModifiersNotAllowed[i]);
		Output.Write(sTerm.GetPointer(), sTerm.GetLength());
		}

	for (i = 0; i < Criteria.SpecialAttribRequired.GetCount(); i++)
		{
		sTerm = strPatternSubst(CONSTLIT(" +%s;"), Criteria.SpecialAttribRequired[i]);
		Output.Write(sTerm.GetPointer(), sTerm.GetLength());
		}

	for (i = 0; i < Criteria.SpecialAttribNotAllowed.GetCount(); i++)
		{
		sTerm = strPatternSubst(CONSTLIT(" +%s;"), Criteria.SpecialAttribNotAllowed[i]);
		Output.Write(sTerm.GetPointer(), sTerm.GetLength());
		}

	if (!Criteria.Frequency.IsBlank())
		{
		sTerm = strPatternSubst(CONSTLIT(" F:%s;"), Criteria.Frequency);
		Output.Write(sTerm.GetPointer(), sTerm.GetLength());
		}

	sTerm = NULL_STR;
	if (Criteria.iEqualToLevel != -1)
		sTerm = strPatternSubst(CONSTLIT(" =%d"), Criteria.iEqualToLevel);
	else if (Criteria.iGreaterThanLevel != -1 && Criteria.iLessThanLevel != -1)
		sTerm = strPatternSubst(CONSTLIT(" L:%d-%d;"), Criteria.iGreaterThanLevel + 1, Criteria.iLessThanLevel - 1);
	else
		{
		if (Criteria.iGreaterThanLevel != -1)
			sTerm = strPatternSubst(CONSTLIT(" >%d"), Criteria.iGreaterThanLevel);
		if (Criteria.iLessThanLevel != -1)
			sTerm = strPatternSubst(CONSTLIT(" <%d"), Criteria.iLessThanLevel);
		}

	if (!sTerm.IsBlank())
		Output.Write(sTerm.GetPointer(), sTerm.GetLength());

	sTerm = NULL_STR;
	if (Criteria.iEqualToPrice != -1)
		sTerm = strPatternSubst(CONSTLIT(" =$%d"), Criteria.iEqualToPrice);
	else
		{
		if (Criteria.iGreaterThanPrice != -1)
			sTerm = strPatternSubst(CONSTLIT(" >=$%d"), Criteria.iGreaterThanPrice + 1);
		if (Criteria.iLessThanPrice != -1)
			sTerm = strPatternSubst(CONSTLIT(" <=$%d"), Criteria.iLessThanPrice - 1);
		}

	if (!sTerm.IsBlank())
		Output.Write(sTerm.GetPointer(), sTerm.GetLength());

	sTerm = NULL_STR;
	if (Criteria.iEqualToMass != -1)
		sTerm = strPatternSubst(CONSTLIT(" =#%d"), Criteria.iEqualToMass);
	else
		{
		if (Criteria.iGreaterThanMass != -1)
			sTerm = strPatternSubst(CONSTLIT(" >=#%d"), Criteria.iGreaterThanMass + 1);
		if (Criteria.iLessThanMass != -1)
			sTerm = strPatternSubst(CONSTLIT(" <=#%d"), Criteria.iLessThanMass - 1);
		}

	if (!sTerm.IsBlank())
		Output.Write(sTerm.GetPointer(), sTerm.GetLength());

	//	Done

	return CString(Output.GetPointer(), Output.GetLength());
	}

void CItem::ParseCriteria (const CString &sCriteria, CItemCriteria *retCriteria)

//	ParseCriteria
//
//	Parses a criteria string and initializes the criteria structure

	{
	//	Initialize

	retCriteria->dwItemCategories = 0;
	retCriteria->dwExcludeCategories = 0;
	retCriteria->dwMustHaveCategories = 0;
	retCriteria->wFlagsMustBeSet = 0;
	retCriteria->wFlagsMustBeCleared = 0;

	retCriteria->bUsableItemsOnly = false;
	retCriteria->bExcludeVirtual = true;
	retCriteria->bInstalledOnly = false;
	retCriteria->bNotInstalledOnly = false;
	retCriteria->bLauncherMissileOnly = false;
	retCriteria->ModifiersRequired.DeleteAll();
	retCriteria->ModifiersNotAllowed.DeleteAll();
	retCriteria->SpecialAttribRequired.DeleteAll();
	retCriteria->SpecialAttribNotAllowed.DeleteAll();
	retCriteria->Frequency = NULL_STR;

	retCriteria->iEqualToLevel = -1;
	retCriteria->iGreaterThanLevel = -1;
	retCriteria->iLessThanLevel = -1;

	retCriteria->iEqualToPrice = -1;
	retCriteria->iGreaterThanPrice = -1;
	retCriteria->iLessThanPrice = -1;

	retCriteria->iEqualToMass = -1;
	retCriteria->iGreaterThanMass = -1;
	retCriteria->iLessThanMass = -1;

	retCriteria->pFilter = NULL;

	bool bExclude = false;
	bool bMustHave = false;

	//	Parse string

	char *pPos = sCriteria.GetASCIIZPointer();
	while (*pPos != '\0')
		{
		switch (*pPos)
			{
			case '*':
				retCriteria->dwItemCategories = 0xFFFFFFFF;
				break;

			case 'a':
				if (bExclude)
					retCriteria->dwExcludeCategories |= itemcatArmor;
				else if (bMustHave)
					retCriteria->dwMustHaveCategories |= itemcatArmor;
				else
					retCriteria->dwItemCategories |= itemcatArmor;
				break;

			case 'b':
				if (bExclude)
					retCriteria->dwExcludeCategories |= itemcatMiscDevice;
				else if (bMustHave)
					retCriteria->dwMustHaveCategories |= itemcatMiscDevice;
				else
					retCriteria->dwItemCategories |= itemcatMiscDevice;
				break;

			case 'c':
				if (bExclude)
					retCriteria->dwExcludeCategories |= itemcatCargoHold;
				else if (bMustHave)
					retCriteria->dwMustHaveCategories |= itemcatCargoHold;
				else
					retCriteria->dwItemCategories |= itemcatCargoHold;
				break;

			case 'd':
				if (bExclude)
					retCriteria->dwExcludeCategories |= itemcatMiscDevice
							| itemcatWeapon
							| itemcatLauncher
							| itemcatReactor
							| itemcatShields
							| itemcatCargoHold
							| itemcatDrive;
				else if (bMustHave)
					retCriteria->dwMustHaveCategories |= itemcatMiscDevice
							| itemcatWeapon
							| itemcatLauncher
							| itemcatReactor
							| itemcatShields
							| itemcatCargoHold
							| itemcatDrive;
				else
					retCriteria->dwItemCategories |= itemcatMiscDevice
							| itemcatWeapon
							| itemcatLauncher
							| itemcatReactor
							| itemcatShields
							| itemcatCargoHold
							| itemcatDrive;
				break;

			case 'f':
				if (bExclude)
					retCriteria->dwExcludeCategories |= itemcatFuel;
				else if (bMustHave)
					retCriteria->dwMustHaveCategories |= itemcatFuel;
				else
					retCriteria->dwItemCategories |= itemcatFuel;
				break;

			case 'l':
				if (bExclude)
					retCriteria->dwExcludeCategories |= itemcatLauncher;
				else if (bMustHave)
					retCriteria->dwMustHaveCategories |= itemcatLauncher;
				else
					retCriteria->dwItemCategories |= itemcatLauncher;
				break;

			case 'm':
				if (bExclude)
					retCriteria->dwExcludeCategories |= itemcatMissile;
				else if (bMustHave)
					retCriteria->dwMustHaveCategories |= itemcatMissile;
				else
					retCriteria->dwItemCategories |= itemcatMissile;
				break;

			case 'p':
				if (bExclude)
					retCriteria->dwExcludeCategories |= itemcatWeapon;
				else if (bMustHave)
					retCriteria->dwMustHaveCategories |= itemcatWeapon;
				else
					retCriteria->dwItemCategories |= itemcatWeapon;
				break;

			case 'r':
				if (bExclude)
					retCriteria->dwExcludeCategories |= itemcatReactor;
				else if (bMustHave)
					retCriteria->dwMustHaveCategories |= itemcatReactor;
				else
					retCriteria->dwItemCategories |= itemcatReactor;
				break;

			case 's':
				if (bExclude)
					retCriteria->dwExcludeCategories |= itemcatShields;
				else if (bMustHave)
					retCriteria->dwMustHaveCategories |= itemcatShields;
				else
					retCriteria->dwItemCategories |= itemcatShields;
				break;

			case 't':
				if (bExclude)
					retCriteria->dwExcludeCategories |= itemcatMisc;
				else if (bMustHave)
					retCriteria->dwMustHaveCategories |= itemcatMisc;
				else
					retCriteria->dwItemCategories |= itemcatMisc;
				break;

			case 'u':
				if (bExclude)
					retCriteria->dwExcludeCategories |= itemcatUseful;
				else if (bMustHave)
					retCriteria->dwMustHaveCategories |= itemcatUseful;
				else
					retCriteria->dwItemCategories |= itemcatUseful;
				break;

			case 'v':
				if (bExclude)
					retCriteria->dwExcludeCategories |= itemcatDrive;
				else if (bMustHave)
					retCriteria->dwMustHaveCategories |= itemcatDrive;
				else
					retCriteria->dwItemCategories |= itemcatDrive;
				break;

			case 'w':
				if (bExclude)
					retCriteria->dwExcludeCategories |= itemcatWeapon | itemcatLauncher;
				else if (bMustHave)
					retCriteria->dwMustHaveCategories |= itemcatWeapon | itemcatLauncher;
				else
					retCriteria->dwItemCategories |= itemcatWeapon | itemcatLauncher;
				break;

			case 'I':
				retCriteria->bInstalledOnly = true;
				break;

			case 'D':
				retCriteria->wFlagsMustBeSet |= flagDamaged;
				break;

			case 'F':
				retCriteria->Frequency = ParseCriteriaParam(&pPos);
				break;

			case 'L':
				{
				int iHigh;
				int iLow;

				if (ParseCriteriaParamLevelRange(&pPos, &iLow, &iHigh))
					{
					if (iHigh == -1)
						retCriteria->iEqualToLevel = iLow;
					else
						{
						retCriteria->iGreaterThanLevel = iLow - 1;
						retCriteria->iLessThanLevel = iHigh + 1;
						}
					}

				break;
				}

			case 'M':
				retCriteria->bLauncherMissileOnly = true;
				retCriteria->dwItemCategories |= itemcatMissile;
				break;

			case 'N':
				retCriteria->wFlagsMustBeCleared |= flagDamaged;
				break;

			case 'S':
				retCriteria->bUsableItemsOnly = true;
				break;

			case 'U':
				retCriteria->bNotInstalledOnly = true;
				break;

			case 'V':
				retCriteria->bExcludeVirtual = false;
				break;

			case '+':
			case '-':
				{
				bool bSpecialAttrib = false;
				char chChar = *pPos;
				pPos++;

				//	Deal with category exclusion

				//	Get the modifier

				char *pStart = pPos;
				while (*pPos != '\0' && *pPos != ';' && *pPos != ' ' && *pPos != '\t')
					{
					if (*pPos == ':')
						bSpecialAttrib = true;
					pPos++;
					}

				CString sModifier = CString(pStart, pPos - pStart);

				//	Required or Not Allowed

				if (chChar == '+')
					{
					if (bSpecialAttrib)
						retCriteria->SpecialAttribRequired.Insert(sModifier);
					else
						retCriteria->ModifiersRequired.Insert(sModifier);
					}
				else
					{
					if (bSpecialAttrib)
						retCriteria->SpecialAttribNotAllowed.Insert(sModifier);
					else
						retCriteria->ModifiersNotAllowed.Insert(sModifier);
					}

				//	No trailing semi

				if (*pPos == '\0')
					pPos--;

				break;
				}

			case '~':
				bExclude = true;
				break;

			case '^':
				bMustHave = true;
				break;

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

				//	Price limits

				if (comparison == '$')
					{
					if (chChar == '=')
						retCriteria->iEqualToPrice = iValue;
					else if (chChar == '>')
						retCriteria->iGreaterThanPrice = iValue - iEqualAdj;
					else if (chChar == '<')
						retCriteria->iLessThanPrice = iValue + iEqualAdj;
					}

				//	Mass limits

				else if (comparison == '#')
					{
					if (chChar == '=')
						retCriteria->iEqualToMass = iValue;
					else if (chChar == '>')
						retCriteria->iGreaterThanMass = iValue - iEqualAdj;
					else if (chChar == '<')
						retCriteria->iLessThanMass = iValue + iEqualAdj;
					}

				//	Level limits

				else
					{
					if (chChar == '=')
						retCriteria->iEqualToLevel = iValue;
					else if (chChar == '>')
						retCriteria->iGreaterThanLevel = iValue - iEqualAdj;
					else if (chChar == '<')
						retCriteria->iLessThanLevel = iValue + iEqualAdj;
					}

				break;
				}
			}

		pPos++;
		}
	}

void CItem::ReadFromCCItem (CCodeChain &CC, ICCItem *pBuffer)

//	ReadFromCCItem
//
//	Reads an item from ICCItem
//
//	The first version stored 3 ints
//	The second version stored 2 ints + 4 elements if m_pExtra was allocated
//	The third version stores 2 ints + 5 elements (1 int version + 4 elements for extra)

	{
	*this = CItem();

	if (!pBuffer->IsNil())
		{
		//	Load the item type

		DWORD dwUNID = (DWORD)pBuffer->GetElement(0)->GetIntegerValue();
		m_pItemType = g_pUniverse->FindItemType(dwUNID);
		if (m_pItemType == NULL)
			return;

		//	Next is the count, flags, and installed

		DWORD *pDest = (DWORD *)this;
		pDest[1] = (DWORD)pBuffer->GetElement(1)->GetIntegerValue();

		//	Previous version

		if (pBuffer->GetCount() == 3)
			{
			DWORD dwLoad = (DWORD)pBuffer->GetElement(2)->GetIntegerValue();
			if (dwLoad)
				{
				Extra();
				m_pExtra->m_dwCharges = LOBYTE(LOWORD(dwLoad));
				m_pExtra->m_dwVariant = 0;
				m_pExtra->m_dwDisruptedTime = 0;
				m_pExtra->m_Mods = CItemEnhancement(HIWORD(dwLoad));
				}
			}

		//	Otherwise

		else if (pBuffer->GetCount() > 3)
			{
			int iStart = 2;
			CString sData;

			Extra();

			//	Load the version, if we have it

			DWORD dwVersion = 45;
			if (pBuffer->GetCount() >= 7)
				{
				dwVersion = (DWORD)pBuffer->GetElement(iStart)->GetIntegerValue();
				iStart++;
				}

			//	Charges

			m_pExtra->m_dwCharges = (DWORD)pBuffer->GetElement(iStart)->GetIntegerValue();
			m_pExtra->m_dwVariant = (DWORD)pBuffer->GetElement(iStart+1)->GetIntegerValue();

			//	Mods

			if (pBuffer->GetElement(iStart+2)->IsIdentifier())
				{
				sData = pBuffer->GetElement(iStart+2)->GetStringValue();
				CMemoryReadStream Stream(sData.GetPointer(), sData.GetLength());
				Stream.Open();
				m_pExtra->m_Mods.ReadFromStream(dwVersion, &Stream);
				Stream.Close();
				}
			else
				m_pExtra->m_Mods = CItemEnhancement((DWORD)pBuffer->GetElement(iStart+2)->GetIntegerValue());

			//	Attribute data block

			sData = pBuffer->GetElement(iStart+3)->GetStringValue();
			CMemoryReadStream Stream(sData.GetPointer(), sData.GetLength());
			Stream.Open();
			m_pExtra->m_Data.ReadFromStream(&Stream);
			Stream.Close();

			//	Ionization

			if (pBuffer->GetCount() >= 8)
				m_pExtra->m_dwDisruptedTime = (DWORD)pBuffer->GetElement(iStart+4)->GetIntegerValue();
			}
		}
	}

void CItem::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads from a stream

	{
	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_pItemType = g_pUniverse->FindItemType(dwLoad);

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_dwCount = LOWORD(dwLoad);
	m_dwFlags = LOBYTE(HIWORD(dwLoad));
	m_dwInstalled = HIBYTE(HIWORD(dwLoad));

	//	Load SExtra struct

	if (Ctx.dwVersion >= 23)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		if (dwLoad)
			{
			m_pExtra = new SExtra;

			if (Ctx.dwVersion >= 71)
				{
				Ctx.pStream->Read((char *)&m_pExtra->m_dwCharges, sizeof(DWORD));
				Ctx.pStream->Read((char *)&m_pExtra->m_dwVariant, sizeof(DWORD));
				}
			else
				{
				Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
				m_pExtra->m_dwCharges = LOWORD(dwLoad);
				m_pExtra->m_dwVariant = 0;
				}

			if (Ctx.dwVersion >= 58)
				Ctx.pStream->Read((char *)&m_pExtra->m_dwDisruptedTime, sizeof(DWORD));
			else
				m_pExtra->m_dwDisruptedTime = 0;

			m_pExtra->m_Mods.ReadFromStream(Ctx);
			m_pExtra->m_Data.ReadFromStream(Ctx);
			}
		else
			m_pExtra = NULL;
		}
	else
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		DWORD dwCharges = LOBYTE(LOWORD(dwLoad));
		DWORD dwCondition = HIBYTE(LOWORD(dwLoad));
		DWORD dwMods = HIWORD(dwLoad);

		if (dwCharges || dwCondition || dwMods)
			{
			m_pExtra = new SExtra;
			m_pExtra->m_dwCharges = dwCharges;
			m_pExtra->m_dwVariant = 0;
			m_pExtra->m_dwDisruptedTime = 0;
			}
		else
			m_pExtra = NULL;
		}
	}

bool CItem::RemoveEnhancement (DWORD dwID)

//	RemoveEnhancement
//
//	Removes the given enhancement by ID. Returns TRUE if the enhancement was
//	removed.

	{
	if (dwID == OBJID_NULL || m_pExtra == NULL || m_pExtra->m_Mods.GetID() != dwID)
		return false;

	m_pExtra->m_Mods = CItemEnhancement();

	return true;
	}

void CItem::SetDisrupted (DWORD dwDuration)

//	SetDisrupted
//
//	Ionizes the item for the given number of ticks

	{
	Extra();

	if (m_pExtra->m_dwDisruptedTime == INFINITE_TICK)
		;
	else if (dwDuration == INFINITE_TICK)
		m_pExtra->m_dwDisruptedTime = INFINITE_TICK;
	else
		{
		//	Disruption time is cumulative

		DWORD dwNow = (DWORD)g_pUniverse->GetTicks();
		if (m_pExtra->m_dwDisruptedTime <= dwNow)
			m_pExtra->m_dwDisruptedTime = dwNow + dwDuration;
		else
			m_pExtra->m_dwDisruptedTime += dwDuration;
		}
	}

bool CItem::SetProperty (CItemCtx &Ctx, const CString &sName, ICCItem *pValue, CString *retsError)

//	SetProperty
//
//	Sets item property. If we cannot set the property we return an error. If
//	retsError is blank then we cannot set the property because the value is Nil.

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	if (strEquals(sName, PROPERTY_CHARGES))
		{
		if (pValue == NULL || pValue->IsNil())
			{
			*retsError = NULL_STR;
			return false;
			}
			
		SetCharges(pValue->GetIntegerValue());
		}
	else if (strEquals(sName, PROPERTY_DAMAGED))
		SetDamaged((pValue == NULL) || !pValue->IsNil());

	else if (strEquals(sName, PROPERTY_DISRUPTED))
		{
		if (pValue == NULL)
			SetDisrupted(INFINITE_TICK);
		else if (pValue->IsNil())
			ClearDisrupted();
		else if (pValue->IsInteger())
			SetDisrupted(pValue->GetIntegerValue());
		else
			SetDisrupted(INFINITE_TICK);
		}

	else if (strEquals(sName, PROPERTY_INC_CHARGES))
		{
		if (pValue == NULL)
			SetCharges(GetCharges() + 1);
		else if (pValue->IsNil())
			{
			*retsError = NULL_STR;
			return false;
			}
		else
			SetCharges(Max(0, GetCharges() + pValue->GetIntegerValue()));
		}

	else if (strEquals(sName, PROPERTY_INSTALLED))
		{
		if (pValue && pValue->IsNil())
			SetInstalled(-1);
		else
			{
			*retsError = CONSTLIT("Unable to set installation flag on item.");
			return false;
			}
		}

	else
		{
		//	If this is a device, then pass it on

		CDeviceClass *pDevice;
		if (pDevice = GetType()->GetDeviceClass())
			return pDevice->SetItemProperty(Ctx, sName, pValue, retsError);

		//	Otherwise, nothing

		else
			{
			*retsError = strPatternSubst(CONSTLIT("Unknown item property: %s."), sName);
			return false;
			}
		}

	return true;
	}

ICCItem *CItem::WriteToCCItem (CCodeChain &CC) const

//	WriteToCCItem
//
//	Converts item to a ICCItem

	{
	ICCItem *pResult = CC.CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	CCLinkedList *pList = (CCLinkedList *)pResult;
	ICCItem *pInt;

	//	Next integer is the item UNID

	pInt = CC.CreateInteger(GetType()->GetUNID());
	pList->Append(&CC, pInt, NULL);
	pInt->Discard(&CC);

	//	Next is the count, flags, and installed

	DWORD *pSource = (DWORD *)this;
	pInt = CC.CreateInteger(pSource[1]);
	pList->Append(&CC, pInt, NULL);
	pInt->Discard(&CC);

	//	Save extra

	if (m_pExtra)
		{
		//	Save the version (starting in v45)

		pInt = CC.CreateInteger(CSystem::GetSaveVersion());
		pList->Append(&CC, pInt, NULL);
		pInt->Discard(&CC);

		//	Charges

		pInt = CC.CreateInteger(m_pExtra->m_dwCharges);
		pList->Append(&CC, pInt, NULL);
		pInt->Discard(&CC);

		//	Condition

		pInt = CC.CreateInteger(m_pExtra->m_dwVariant);
		pList->Append(&CC, pInt, NULL);
		pInt->Discard(&CC);

		//	Mods

		CMemoryWriteStream Stream;
		Stream.Create();
		m_pExtra->m_Mods.WriteToStream(&Stream);
		Stream.Close();

		pInt = CC.CreateString(CString(Stream.GetPointer(), Stream.GetLength()));
		pList->Append(&CC, pInt, NULL);
		pInt->Discard(&CC);

		//	Attribute data block

		Stream.Create();
		m_pExtra->m_Data.WriteToStream(&Stream);
		Stream.Close();

		pInt = CC.CreateString(CString(Stream.GetPointer(), Stream.GetLength()));
		pList->Append(&CC, pInt, NULL);
		pInt->Discard(&CC);

		//	Disrupted time

		pInt = CC.CreateInteger(m_pExtra->m_dwDisruptedTime);
		pList->Append(&CC, pInt, NULL);
		pInt->Discard(&CC);
		}

	return pResult;
	}

void CItem::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Writes to a stream
//
//	DWORD		item type UNID
//	DWORD		low = m_dwCount; high = (low = m_dwFlags; high = m_dwInstalled)
//	DWORD		non-zero if SExtra follows
//
//	SExtra
//	DWORD		m_dwCharges
//	DWORD		m_dwCondition
//	DWORD		m_dwDisruptedTime
//	CItemEnhancement
//	CAttributeDataBlock

	{
	DWORD dwSave = m_pItemType->GetUNID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = MAKELONG(m_dwCount, MAKEWORD(m_dwFlags, m_dwInstalled));
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Save SExtra

	dwSave = (m_pExtra ? 0xffffffff : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	if (m_pExtra)
		{
		pStream->Write((char *)&m_pExtra->m_dwCharges, sizeof(DWORD));
		pStream->Write((char *)&m_pExtra->m_dwVariant, sizeof(DWORD));
		pStream->Write((char *)&m_pExtra->m_dwDisruptedTime, sizeof(DWORD));

		m_pExtra->m_Mods.WriteToStream(pStream);

		//	Note: Currently does not support saving object references

		m_pExtra->m_Data.WriteToStream(pStream);
		}
	}

//	CItemCriteria ------------------------------------------------------------

CItemCriteria::CItemCriteria (void) : pFilter(NULL)
	{ 
	}

CItemCriteria::CItemCriteria (const CItemCriteria &Copy)

//	Criteria copy constructor

	{
	dwItemCategories = Copy.dwItemCategories;
	dwExcludeCategories = Copy.dwExcludeCategories;
	dwMustHaveCategories = Copy.dwMustHaveCategories;
	wFlagsMustBeSet = Copy.wFlagsMustBeSet;
	wFlagsMustBeCleared = Copy.wFlagsMustBeCleared;

	bUsableItemsOnly = Copy.bUsableItemsOnly;
	bExcludeVirtual = Copy.bExcludeVirtual;
	bInstalledOnly = Copy.bInstalledOnly;
	bNotInstalledOnly = Copy.bNotInstalledOnly;
	bLauncherMissileOnly = Copy.bLauncherMissileOnly;

	ModifiersRequired = Copy.ModifiersRequired;
	ModifiersNotAllowed = Copy.ModifiersNotAllowed;
	SpecialAttribRequired = Copy.SpecialAttribRequired;
	SpecialAttribNotAllowed = Copy.SpecialAttribNotAllowed;
	Frequency = Copy.Frequency;

	iEqualToLevel = Copy.iEqualToLevel;
	iGreaterThanLevel = Copy.iGreaterThanLevel;
	iLessThanLevel = Copy.iLessThanLevel;

	iEqualToPrice = Copy.iEqualToPrice;
	iGreaterThanPrice = Copy.iGreaterThanPrice;
	iLessThanPrice = Copy.iLessThanPrice;

	iEqualToMass = Copy.iEqualToMass;
	iGreaterThanMass = Copy.iGreaterThanMass;
	iLessThanMass = Copy.iLessThanMass;

	pFilter = Copy.pFilter;
	if (pFilter)
		pFilter->Reference();
	}

CItemCriteria &CItemCriteria::operator= (const CItemCriteria &Copy)

//	Criteria equals operator

	{
	if (pFilter)
		pFilter->Discard(&g_pUniverse->GetCC());

	dwItemCategories = Copy.dwItemCategories;
	dwExcludeCategories = Copy.dwExcludeCategories;
	dwMustHaveCategories = Copy.dwMustHaveCategories;
	wFlagsMustBeSet = Copy.wFlagsMustBeSet;
	wFlagsMustBeCleared = Copy.wFlagsMustBeCleared;

	bUsableItemsOnly = Copy.bUsableItemsOnly;
	bExcludeVirtual = Copy.bExcludeVirtual;
	bInstalledOnly = Copy.bInstalledOnly;
	bNotInstalledOnly = Copy.bNotInstalledOnly;
	bLauncherMissileOnly = Copy.bLauncherMissileOnly;

	ModifiersRequired = Copy.ModifiersRequired;
	ModifiersNotAllowed = Copy.ModifiersNotAllowed;
	SpecialAttribRequired = Copy.SpecialAttribRequired;
	SpecialAttribNotAllowed = Copy.SpecialAttribNotAllowed;
	Frequency = Copy.Frequency;

	iEqualToLevel = Copy.iEqualToLevel;
	iGreaterThanLevel = Copy.iGreaterThanLevel;
	iLessThanLevel = Copy.iLessThanLevel;

	iEqualToPrice = Copy.iEqualToPrice;
	iGreaterThanPrice = Copy.iGreaterThanPrice;
	iLessThanPrice = Copy.iLessThanPrice;

	iEqualToMass = Copy.iEqualToMass;
	iGreaterThanMass = Copy.iGreaterThanMass;
	iLessThanMass = Copy.iLessThanMass;

	pFilter = Copy.pFilter;
	if (pFilter)
		pFilter->Reference();

	return *this;
	}

CItemCriteria::~CItemCriteria (void)

	{
	if (pFilter)
		pFilter->Discard(&g_pUniverse->GetCC());
	}

int CItemCriteria::GetMaxLevelMatched (void) const

//	GetMaxLevelMatches
//
//	Returns the maximum item level that this criteria matches. If there is no
//	explicit level match, then we laboriously check for every single
//	item type that matches and return the max level.

	{
	int i;

	if (iEqualToLevel != -1)
		return iEqualToLevel;

	if (iLessThanLevel != -1)
		return iLessThanLevel - 1;

	//	Look at every single item that might match

	int iMaxLevel = -1;
	for (i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
		{
		CItemType *pType = g_pUniverse->GetItemType(i);
		CItem Item(pType, 1);

		if (Item.MatchesCriteria(*this))
			iMaxLevel = Max(iMaxLevel, pType->GetLevel());
		}

	return iMaxLevel;
	}
