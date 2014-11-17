//	CInstalledDevice.cpp
//
//	CInstalledDevice class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define DEVICE_ID_ATTRIB						CONSTLIT("deviceID")

//	CInstalledDevice class

CInstalledDevice::CInstalledDevice (void) : 
		m_pItem(NULL),
		m_pOverlay(NULL),
		m_dwTargetID(0),
		m_pEnhancements(NULL),
		m_iDeviceSlot(-1),
		m_iPosAngle(0),
		m_iPosRadius(0),
		m_iPosZ(0),
		m_iMinFireArc(0),
		m_iMaxFireArc(0),

		m_iTimeUntilReady(0),
		m_iFireAngle(0),
		m_iTemperature(0),
		m_iActivateDelay(0),
		m_iSlotBonus(0),
		m_iSlotPosIndex(-1),

		m_fOmniDirectional(false),
		m_fSecondaryWeapon(false),
		m_fTriggered(false),
		m_fLastActivateSuccessful(false),
		m_f3DPosition(false),

		m_fLinkedFireAlways(false),
		m_fLinkedFireTarget(false),
		m_fLinkedFireEnemy(false)
	{
	}

CInstalledDevice::~CInstalledDevice (void)

//	CInstalledDevice destructor

	{
	if (m_pEnhancements)
		m_pEnhancements->Delete();
	}

CInstalledDevice &CInstalledDevice::operator= (const CInstalledDevice &Obj)

//	CInstalledDevice operator =

	{
	if (m_pEnhancements)
		m_pEnhancements->Delete();

	m_pItem = Obj.m_pItem;
	m_pClass = Obj.m_pClass;
	m_pOverlay = Obj.m_pOverlay;
	m_dwTargetID = Obj.m_dwTargetID;
	m_pEnhancements = (Obj.m_pEnhancements ? Obj.m_pEnhancements->AddRef() : NULL);

	m_dwData = Obj.m_dwData;

	m_iDeviceSlot = Obj.m_iDeviceSlot;

	m_iPosAngle = Obj.m_iPosAngle;
	m_iPosRadius = Obj.m_iPosRadius;
	m_iPosZ = Obj.m_iPosZ;
	m_iMinFireArc = Obj.m_iMinFireArc;
	m_iMaxFireArc = Obj.m_iMaxFireArc;

	m_iTimeUntilReady = Obj.m_iTimeUntilReady;
	m_iFireAngle = Obj.m_iFireAngle;

	m_iTemperature = Obj.m_iTemperature;
	m_iActivateDelay = Obj.m_iActivateDelay;
	m_iSlotBonus = Obj.m_iSlotBonus;
	m_iSlotPosIndex = Obj.m_iSlotPosIndex;
	
	m_fOmniDirectional = Obj.m_fOmniDirectional;
	m_fOverdrive = Obj.m_fOverdrive;
	m_fOptimized = Obj.m_fOptimized;
	m_fSecondaryWeapon = Obj.m_fSecondaryWeapon;
	m_fEnabled = Obj.m_fEnabled;
	m_fExternal = Obj.m_fExternal;
	m_fWaiting = Obj.m_fWaiting;
	m_fTriggered = Obj.m_fTriggered;

	m_fRegenerating = Obj.m_fRegenerating;
	m_fLastActivateSuccessful = Obj.m_fLastActivateSuccessful;
	m_f3DPosition = Obj.m_f3DPosition;
	m_fLinkedFireAlways = Obj.m_fLinkedFireAlways;
	m_fLinkedFireTarget = Obj.m_fLinkedFireTarget;
	m_fLinkedFireEnemy = Obj.m_fLinkedFireEnemy;

	return *this;
	}

int CInstalledDevice::CalcPowerUsed (CSpaceObject *pSource)

//	CalcPowerUsed
//
//	Calculates how much power this device used this turn

	{
	if (!IsEmpty()) 
		return m_pClass->CalcPowerUsed(this, pSource);
	else
		return 0;
	}

void CInstalledDevice::FinishInstall (CSpaceObject *pSource)

//	FinishInstall
//
//	Finishes the parts of the install that fire events
//	(At object creation time these events fire after the entire object
//	is created).

	{
	m_pItem->FireOnInstall(pSource);
	m_pItem->FireOnEnabled(pSource);

	CShip *pShip = pSource->AsShip();
	if (pShip)
		pShip->GetController()->OnItemInstalled(*m_pItem);

	//	If necessary create an overlay for this device

	COverlayType *pOverlayType;
	pOverlayType = m_pClass->FireGetOverlayType(CItemCtx(pSource, this));

	//	Add it

	if (pOverlayType)
		{
		DWORD dwID;
		pSource->AddOverlay(pOverlayType, GetPosAngle(), GetPosRadius(), 0, -1, &dwID);

		CEnergyField *pOverlay = pSource->GetOverlay(dwID);
		if (pOverlay)
			{
			pOverlay->SetDevice(GetDeviceSlot());
			SetOverlay(pOverlay);
			}
		}
	}

int CInstalledDevice::GetActivateDelay (CSpaceObject *pSource)

//	GetActivateDelay
//
//	Returns the number of ticks to wait for activation
	
	{
	return m_iActivateDelay;
	}

CString CInstalledDevice::GetEnhancedDesc (CSpaceObject *pSource, const CItem *pItem)

//	GetEnhancedDesc
//
//	Returns description of the enhancement

	{
	int iDamageBonus;

	//	If the item is enhanced, then we show the enhancement
	//	(if a device happens to have the same type of enhancement, this
	//	function will add the two enhancements together)

	if (GetMods().IsNotEmpty())
		return GetMods().GetEnhancedDesc((pItem ? *pItem : pSource->GetItemForDevice(this)), pSource, this);

	//	Describe enhancements from the device only (e.g., confered by other devices)

	else if (GetActivateDelay(pSource) > m_pClass->GetActivateDelay(this, pSource))
		return CONSTLIT("-Slow");
	else if (GetActivateDelay(pSource) < m_pClass->GetActivateDelay(this, pSource))
		return CONSTLIT("+Fast");
	else if (iDamageBonus = (m_pEnhancements ? m_pEnhancements->GetBonus() : 0))
		return (iDamageBonus > 0 ? strPatternSubst(CONSTLIT("+%d%%"), iDamageBonus) : strPatternSubst(CONSTLIT("%d%%"), iDamageBonus));

	//	Other enhancements

	else if (IsEnhanced())
		return CONSTLIT("+Enhanced");
	else
		return NULL_STR;
	}

DWORD CInstalledDevice::GetLinkedFireOptions (void) const

//	GetLinkedFireOptions
//
//	Returns linked-fire options for the device slot.

	{
	if (m_fLinkedFireAlways)
		return CDeviceClass::lkfAlways;
	else if (m_fLinkedFireTarget)
		return CDeviceClass::lkfTargetInRange;
	else if (m_fLinkedFireEnemy)
		return CDeviceClass::lkfEnemyInRange;
	else if (m_fSecondaryWeapon)
		return CDeviceClass::lkfEnemyInRange;
	else
		return 0;
	}

CVector CInstalledDevice::GetPos (CSpaceObject *pSource)

//	GetPos
//
//	Returns the position of the device

	{
	if (m_f3DPosition)
		{
		int iScale = pSource->GetImage().GetImageViewportSize();

		CVector vOffset;
		C3DConversion::CalcCoord(iScale, pSource->GetRotation() + m_iPosAngle, m_iPosRadius, m_iPosZ, &vOffset);

		return pSource->GetPos() + vOffset;
		}
	else if (m_iPosRadius)
		{
		return pSource->GetPos() 
				+ PolarToVector((m_iPosAngle + pSource->GetRotation()) % 360,
					m_iPosRadius * g_KlicksPerPixel);
		}
	else
		return pSource->GetPos();
	}

CVector CInstalledDevice::GetPosOffset (CSpaceObject *pSource)

//	GetPosOffset
//
//	Returns the position of the device

	{
	if (m_f3DPosition)
		{
		int iScale = pSource->GetImage().GetImageViewportSize();

		CVector vOffset;
		C3DConversion::CalcCoord(iScale, 90 + m_iPosAngle, m_iPosRadius, m_iPosZ, &vOffset);

		return vOffset;
		}
	else if (m_iPosRadius)
		{
		return PolarToVector((m_iPosAngle + 90) % 360,
					m_iPosRadius * g_KlicksPerPixel);
		}
	else
		return CVector();
	}

void CInstalledDevice::InitFromDesc (const SDeviceDesc &Desc)

//	InitFromDesc
//
//	Initializes from a desc

	{
	m_fOmniDirectional = Desc.bOmnidirectional;
	m_iMinFireArc = Desc.iMinFireArc;
	m_iMaxFireArc = Desc.iMaxFireArc;

	m_iPosAngle = Desc.iPosAngle;
	m_iPosRadius = Desc.iPosRadius;
	m_f3DPosition = Desc.b3DPosition;
	m_iPosZ = Desc.iPosZ;
	m_fExternal = Desc.bExternal;

	SetLinkedFireOptions(Desc.dwLinkedFireOptions);

	m_fSecondaryWeapon = Desc.bSecondary;

	m_iSlotBonus = Desc.iSlotBonus;
	m_iSlotPosIndex = -1;
	}

ALERROR CInstalledDevice::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitFromXML
//
//	Initializes each device slot from an XML structure

	{
	ALERROR error;

	if (error = m_pClass.LoadUNID(Ctx, pDesc->GetAttribute(DEVICE_ID_ATTRIB)))
		return error;

	SDeviceDesc DeviceDesc;
	if (error = IDeviceGenerator::InitDeviceDescFromXML(Ctx, pDesc, &DeviceDesc))
		return error;

	InitFromDesc(DeviceDesc);

	return NOERROR;
	}

void CInstalledDevice::Install (CSpaceObject *pObj, CItemListManipulator &ItemList, int iDeviceSlot, bool bInCreate)

//	Install
//
//	Installs a new device of the given class

	{
	const CItem &Item = ItemList.GetItemAtCursor();

	m_pClass.Set(Item.GetType()->GetDeviceClass());
	m_iDeviceSlot = iDeviceSlot;
	m_iSlotPosIndex = -1;
	m_pOverlay = NULL;
	m_dwData = 0;
	m_iTemperature = 0;
	m_fWaiting = false;
	m_fEnabled = true;
	m_fTriggered = false;
	m_fRegenerating = false;
	m_fLastActivateSuccessful = false;

	//	Call the class

	m_pClass->OnInstall(this, pObj, ItemList);
	m_pClass->SelectFirstVariant(pObj, this);

	//	Mark the item as installed

	ItemList.SetInstalledAtCursor(iDeviceSlot);

	//	We remember the item after it is installed;
	//	otherwise, we get a pointer to the wrong item

	m_pItem = ItemList.GetItemPointerAtCursor();
	ASSERT(m_pItem);

	//	Default to basic fire delay. Callers must set the appropriate delay
	//	based on enhancements later.

	m_iActivateDelay = m_pClass->GetActivateDelay(this, pObj);

	//	If we're installing a device after creation then we
	//	zero-out the device position, etc. If necessary the
	//	caller or the device can set these fields later.
	//
	//	Note: This will overwrite whatever values were set
	//	at creation time.

	if (!bInCreate)
		{
		//	Desc is initialized to defaults even if FindDeviceSlotDesc fails.

		SDeviceDesc Desc;
		pObj->FindDeviceSlotDesc(Item, &Desc);

		//	Set the device slot properties

		m_iPosAngle = Desc.iPosAngle;
		m_iPosRadius = Desc.iPosRadius;
		m_iPosZ = Desc.iPosZ;
		m_f3DPosition = Desc.b3DPosition;

		m_fExternal = (Desc.bExternal || m_pClass->IsExternal());

		m_fOmniDirectional = Desc.bOmnidirectional;
		m_iMinFireArc = Desc.iMinFireArc;
		m_iMaxFireArc = Desc.iMaxFireArc;

		SetLinkedFireOptions(Desc.dwLinkedFireOptions);
		m_fSecondaryWeapon = Desc.bSecondary;

		m_iSlotBonus = Desc.iSlotBonus;
		}

	//	Event (when creating a ship we wait until the
	//	whole ship is created before firing the event)

	if (!bInCreate)
		FinishInstall(pObj);
	}

bool CInstalledDevice::IsLinkedFire (CItemCtx &Ctx, ItemCategories iTriggerCat) const

//	IsLinkedFire
//
//	Returns TRUE if we're linked to weapon trigger

	{
	DWORD dwOptions = GetClass()->GetLinkedFireOptions(Ctx);
	if (dwOptions == 0)
		return false;
	else if (iTriggerCat == itemcatNone)
		return true;
	else
		return (GetClass()->GetCategory() == iTriggerCat);
	}

bool CInstalledDevice::IsSelectable (CItemCtx &Ctx) const

//	IsSelectable
//
//	Returns TRUE if device can be selected as a primary weapon or launcher.

	{
	return (!IsSecondaryWeapon()
			&& GetClass()->GetLinkedFireOptions(Ctx) == 0);
	}

ALERROR CInstalledDevice::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Done loading all design elements

	{
	ALERROR error;

	if (error = m_pClass.Bind(Ctx))
		return error;

	return NOERROR;
	}

void CInstalledDevice::ReadFromStream (CSpaceObject *pSource, SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read object from stream
//
//	DWORD		device: class UNID (0xffffffff if not installed)
//	DWORD		device: m_dwTargetID
//	DWORD		device: m_dwData
//	DWORD		device: low = m_iPosAngle; hi = m_iPosRadius
//	DWORD		device: low = m_iMinFireArc; hi = m_iMaxFireArc
//	DWORD		device: low = m_iTimeUntilReady; hi = m_iFireAngle
//	DWORD		device: low = m_iSlotPosIndex; hi = m_iTemperature
//	DWORD		device: low = m_iSlotBonus; hi = m_iDeviceSlot
//	DWORD		device: low = m_iActivateDelay; hi = m_iPosZ
//	DWORD		device: flags
//
//	CItemEnhancementStack

	{
	DWORD dwLoad;

	//	Class

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad == 0xffffffff)
		return;

	m_pClass.Set(g_pUniverse->FindDeviceClass(dwLoad));

	//	Other data

	if (Ctx.dwVersion >= 66)
		Ctx.pStream->Read((char *)&m_dwTargetID, sizeof(DWORD));

	Ctx.pStream->Read((char *)&m_dwData, sizeof(DWORD));

	//	In 1.08 we changed how we store alternating and repeating counters.

	if (Ctx.dwVersion < 75 && m_pClass && m_pClass->AsWeaponClass())
		m_dwData = (m_dwData & 0xFFFF0000);

	//	Flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iPosAngle = (int)LOWORD(dwLoad);
	m_iPosRadius = (int)HIWORD(dwLoad);

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iMinFireArc = (int)LOWORD(dwLoad);
	m_iMaxFireArc = (int)HIWORD(dwLoad);

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iTimeUntilReady = (int)LOWORD(dwLoad);
	m_iFireAngle = (int)HIWORD(dwLoad);

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iTemperature = (int)HIWORD(dwLoad);

	if (Ctx.dwVersion < 92)
		{
		int iBonus = (int)LOWORD(dwLoad);
		if (iBonus != 0)
			{
			m_pEnhancements = new CItemEnhancementStack;
			m_pEnhancements->InsertHPBonus(iBonus);
			}
		m_iSlotPosIndex = -1;
		}
	else if (Ctx.dwVersion < 106)
		m_iSlotPosIndex = -1;
	else
		m_iSlotPosIndex = (int)LOWORD(dwLoad);

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iSlotBonus = (int)LOWORD(dwLoad);

	if (Ctx.dwVersion >= 29)
		m_iDeviceSlot = (int)HIWORD(dwLoad);
	else
		m_iDeviceSlot = -1;

	if (Ctx.dwVersion >= 44)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_iActivateDelay = (int)LOWORD(dwLoad);
		m_iPosZ = (int)HIWORD(dwLoad);
		}
	else
		{
		m_iActivateDelay = 8;
		m_iPosZ = 0;
		}

	//	In newer versions we store an activation delay instead of an adjustment

	if (Ctx.dwVersion < 93)
		m_iActivateDelay = m_iActivateDelay * m_pClass->GetActivateDelay(this, pSource) / 100;

	//	We no longer store mods in the device structure

	if (Ctx.dwVersion < 58)
		{
		CItemEnhancement Dummy;
		Dummy.ReadFromStream(Ctx);
		}

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_fOmniDirectional =	((dwLoad & 0x00000001) ? true : false);
	m_f3DPosition =		   (((dwLoad & 0x00000002) ? true : false) && (Ctx.dwVersion >= 73));
	//	0x00000004 UNUSED as of version 58
	m_fOverdrive =			((dwLoad & 0x00000008) ? true : false);
	m_fOptimized =			((dwLoad & 0x00000010) ? true : false);
	m_fSecondaryWeapon =	((dwLoad & 0x00000020) ? true : false);
	//	0x00000040 UNUSED as of version 58
	m_fEnabled =			((dwLoad & 0x00000080) ? true : false);
	m_fWaiting =			((dwLoad & 0x00000100) ? true : false);
	m_fTriggered =			((dwLoad & 0x00000200) ? true : false);
	m_fRegenerating =		((dwLoad & 0x00000400) ? true : false);
	m_fLastActivateSuccessful = ((dwLoad & 0x00000800) ? true : false);

	m_fLinkedFireAlways =	((dwLoad & 0x00001000) ? true : false);
	m_fLinkedFireTarget =	((dwLoad & 0x00002000) ? true : false);
	m_fLinkedFireEnemy =	((dwLoad & 0x00004000) ? true : false);
	m_fExternal =			((dwLoad & 0x00008000) ? true : false);

	//	Previous versions did not save this flag

	if (Ctx.dwVersion < 107)
		m_fExternal = m_pClass->IsExternal();

	//	Fix up the item pointer (but only if it is installed)

	m_pItem = NULL;
	if (m_pClass != NULL && m_iDeviceSlot != -1)
		{
		CItemListManipulator ItemList(pSource->GetItemList());
		pSource->SetCursorAtDevice(ItemList, this);
		if (ItemList.IsCursorValid())
			m_pItem = ItemList.GetItemPointerAtCursor();

		//	In previous versions we automatically offset weapon positions.
		//	In later versions we explicitly set the position, so we have
		//	to do so here.

		if (Ctx.dwVersion < 62)
			{
			if (m_iPosRadius == 0
					&& (m_pClass->GetCategory() == itemcatWeapon || m_pClass->GetCategory() == itemcatLauncher))
				{
				m_iPosAngle = 0;
				m_iPosRadius = 20;
				m_iPosZ = 0;
				m_f3DPosition = false;
				}
			}
		}

	//	Enhancement stack

	if (Ctx.dwVersion >= 92)
		CItemEnhancementStack::ReadFromStream(Ctx, &m_pEnhancements);
	}

int CInstalledDevice::IncCharges (CSpaceObject *pSource, int iChange)

//	IncCharges
//
//	Increments charges

	{
	CShip *pShip = pSource->AsShip();
	if (pShip == NULL)
		return -1;

	CItemListManipulator ItemList(pSource->GetItemList());
	pShip->SetCursorAtDevice(ItemList, m_iDeviceSlot);
	pShip->RechargeItem(ItemList, iChange);

	return ItemList.GetItemAtCursor().GetCharges();
	}

void CInstalledDevice::SetEnhancements (CItemEnhancementStack *pStack)

//	SetEnhancements
//
//	Sets the enhancements stack. NOTE: We take ownership of the stack.

	{
	if (m_pEnhancements)
		m_pEnhancements->Delete();

	if (pStack && pStack->IsEmpty())
		{
		pStack->Delete();
		pStack = NULL;
		}

	m_pEnhancements = pStack;
	}

void CInstalledDevice::SetLinkedFireOptions (DWORD dwOptions)

//	SetLinkedFireOptions
//
//	Sets linked fire options

	{
	m_fLinkedFireAlways = false;
	m_fLinkedFireTarget = false;
	m_fLinkedFireEnemy = false;
	if (dwOptions & CDeviceClass::lkfAlways)
		m_fLinkedFireAlways = true;
	else if (dwOptions & CDeviceClass::lkfTargetInRange)
		m_fLinkedFireTarget = true;
	else if (dwOptions & CDeviceClass::lkfEnemyInRange)
		m_fLinkedFireEnemy = true;
	}

void CInstalledDevice::Uninstall (CSpaceObject *pObj, CItemListManipulator &ItemList)

//	Uninstall
//
//	Uninstalls the device

	{
	CItem theItem = ItemList.GetItemAtCursor();
	int iDevSlot = theItem.GetInstalled();
	if (iDevSlot == -1)
		{
		ASSERT(false);
		return;
		}

	//	Destroy the overlay, if necessary

	if (m_pOverlay)
		{
		pObj->RemoveOverlay(m_pOverlay->GetID());
		m_pOverlay = NULL;
		}

	//	Event (we fire the event before we uninstall because once we uninstall
	//	the item might get combined with others)

	ItemList.GetItemAtCursor().FireOnDisabled(pObj);
	ItemList.GetItemAtCursor().FireOnUninstall(pObj);
	CShip *pShip = pObj->AsShip();
	if (pShip)
		pShip->GetController()->OnItemUninstalled(theItem);

	//	We need to refresh the cursor because OnUninstall might
	//	have done something

	if (!pObj->SetCursorAtDevice(ItemList, iDevSlot))
		{
		m_pItem = NULL;
		m_pClass.Set(NULL);
		return;
		}

	//	Mark the item as uninstalled

	ItemList.SetInstalledAtCursor(-1);

	//	Let the class clean up also

	m_pClass->OnUninstall(this, pObj, ItemList);

	//	Done

	m_pItem = NULL;
	m_pClass.Set(NULL);
	m_fTriggered = false;
	m_fLastActivateSuccessful = false;
	}

void CInstalledDevice::Update (CSpaceObject *pSource, 
							   int iTick,
							   bool *retbSourceDestroyed,
							   bool *retbConsumedItems,
							   bool *retbDisrupted)
	{
	DEBUG_TRY

	if (!IsEmpty()) 
		{
		m_pClass->Update(this, pSource, iTick, retbSourceDestroyed, retbConsumedItems);

		//	Counters

		if (m_iTimeUntilReady > 0 && IsEnabled())
			{
			m_iTimeUntilReady--;

			if (m_pClass->ShowActivationDelayCounter(pSource, this))
				pSource->OnComponentChanged(comDeviceCounter);
			}

		//	Disruption

		if (IsDisrupted())
			{
			if (retbDisrupted)
				*retbDisrupted = true;

			//	If disruption is about to end, then notify

			if (GetDisruptedDuration() == 1)
				pSource->OnDeviceStatus(this, CDeviceClass::statusDisruptionRepaired);
			}
		}

	DEBUG_CATCH
	}

void CInstalledDevice::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write object to stream
//
//	DWORD		device: class UNID (0xffffffff if not installed)
//	DWORD		device: m_dwTargetID
//	DWORD		device: m_dwData
//	DWORD		device: low = m_iPosAngle; hi = m_iPosRadius
//	DWORD		device: low = m_iMinFireArc; hi = m_iMaxFireArc
//	DWORD		device: low = m_iTimeUntilReady; hi = m_iFireAngle
//	DWORD		device: low = m_iSlotIndex; hi = m_iTemperature
//	DWORD		device: low = m_iSlotBonus; hi = m_iDeviceSlot
//	DWORD		device: low = m_iActivateDelay; hi = m_iPosZ
//	DWORD		device: flags
//
//	CItemEnhancementStack

	{
	DWORD dwSave;

	dwSave = (m_pClass ? m_pClass->GetUNID() : 0xffffffff);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	if (m_pClass == NULL)
		return;

	pStream->Write((char *)&m_dwTargetID, sizeof(DWORD));
	pStream->Write((char *)&m_dwData, sizeof(DWORD));

	dwSave = MAKELONG(m_iPosAngle, m_iPosRadius);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	
	dwSave = MAKELONG(m_iMinFireArc, m_iMaxFireArc);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	
	dwSave = MAKELONG(m_iTimeUntilReady, m_iFireAngle);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	
	dwSave = MAKELONG(m_iSlotPosIndex, m_iTemperature);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = MAKELONG(m_iSlotBonus, m_iDeviceSlot);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = MAKELONG(m_iActivateDelay, m_iPosZ);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = 0;
	dwSave |= (m_fOmniDirectional ?		0x00000001 : 0);
	dwSave |= (m_f3DPosition ?			0x00000002 : 0);
	//	0x00000004 UNUSED as of version 58
	dwSave |= (m_fOverdrive ?			0x00000008 : 0);
	dwSave |= (m_fOptimized ?			0x00000010 : 0);
	dwSave |= (m_fSecondaryWeapon ?		0x00000020 : 0);
	//	0x00000040 UNUSED as of version 58
	dwSave |= (m_fEnabled ?				0x00000080 : 0);
	dwSave |= (m_fWaiting ?				0x00000100 : 0);
	dwSave |= (m_fTriggered ?			0x00000200 : 0);
	dwSave |= (m_fRegenerating ?		0x00000400 : 0);
	dwSave |= (m_fLastActivateSuccessful ? 0x00000800 : 0);
	dwSave |= (m_fLinkedFireAlways ?	0x00001000 : 0);
	dwSave |= (m_fLinkedFireTarget ?	0x00002000 : 0);
	dwSave |= (m_fLinkedFireEnemy ?		0x00004000 : 0);
	dwSave |= (m_fExternal ?			0x00008000 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	CItemEnhancementStack::WriteToStream(m_pEnhancements, pStream);
	}
