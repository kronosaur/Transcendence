//	CItemCtx.cpp
//
//	CItemCtx class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

void CItemCtx::ClearItemCache (void)

//	ClearItemCache
//
//	Clear m_pItem, forcing us to get it from the source again.

	{
	if (m_pSource
			&& (m_pDevice || m_pArmor))
		m_pItem = NULL;
	}

ICCItem *CItemCtx::CreateItemVariable(CCodeChain &CC)

//	CreateItemVariable
//
//	Creates a variable for the item (or Nil)

	{
	const CItem *pItem = GetItemPointer();
	if (pItem == NULL)
		return CC.CreateNil();

	return CreateListFromItem(CC, *pItem);
	}

CInstalledArmor *CItemCtx::GetArmor(void)

//	GetArmor
//
//	Returns the installed armor struct

	{
	//	If we've got it, done

	if (m_pArmor)
		return m_pArmor;

	//	Otherwise, try to get it from the item and source

	CShip *pShip;
	if (m_pSource 
			&& m_pItem 
			&& m_pItem->IsInstalled() 
			&& m_pItem->GetType()
			&& m_pItem->GetType()->GetCategory() == itemcatArmor
			&& (pShip = m_pSource->AsShip()))
		{
		//	Cache it in case someone asks later
		m_pArmor = pShip->GetArmorSection(m_pItem->GetInstalled());
		return m_pArmor;
		}

	//	Couldn't get it

	return NULL;
	}

CArmorClass *CItemCtx::GetArmorClass(void)

//	GetArmorClass
//
//	Returns the armor class

	{
	//	Get it from the installed device

	if (m_pArmor)
		return m_pArmor->GetClass();

	//	Otherwise, get it from the item

	if (m_pItem)
		{
		CItemType *pType = m_pItem->GetType();
		if (pType)
			return pType->GetArmorClass();
		}

	//	Couldn't get it

	return NULL;
	}

CInstalledDevice *CItemCtx::GetDevice(void)

//	GetDevice
//
//	Returns the installed device struct

	{
	//	If we've got it, done

	if (m_pDevice)
		return m_pDevice;

	//	Otherwise, try to get it from the item and source

	if (m_pSource 
			&& m_pItem 
			&& m_pItem->IsInstalled()
			&& m_pItem->GetType()
			&& m_pItem->GetType()->GetDeviceClass())
		{
		//	Cache it in case someone asks later
		m_pDevice = m_pSource->FindDevice(*m_pItem);
		return m_pDevice;
		}

	//	Couldn't get it

	return NULL;
	}

CDeviceClass *CItemCtx::GetDeviceClass(void)

//	GetDeviceClass
//
//	Returns the device class

	{
	//	Get it from the installed device

	if (m_pDevice)
		return m_pDevice->GetClass();

	//	Otherwise, get it from the item

	if (m_pItem)
		{
		CItemType *pType = m_pItem->GetType();
		if (pType)
			return pType->GetDeviceClass();
		}

	//	Couldn't get it

	return NULL;
	}

const CItem &CItemCtx::GetItem(void)

//	GetItem
//
//	Returns the item struct

	{
	//	If we've got an item, then return a reference to it
	//	Whoever set this is responsible to guaranteeing its lifespan

	if (m_pItem)
		return *m_pItem;

	//	Otherwise, we need to cons one up

	const CItem *pItem = GetItemPointer();
	if (pItem == NULL)
		return CItem::GetNullItem();

	//	We cache it for later

	m_Item = *pItem;
	m_pItem = &m_Item;

	//	Done

	return *m_pItem;
	}

const CItem *CItemCtx::GetItemPointer(void)

//	GetItemPointer
//
//	Returns a pointer to an item or NULL.
//	Note: We can't guarantee that the pointer will be valid across calls
//	to script (because an ItemTable might be moved around) so this
//	pointer should never be returned to callers.

	{
	//	If we've got it, done

	if (m_pItem)
		return m_pItem;

	//	If we have a source and device, find the item

	if (m_pDevice && m_pSource)
		return m_pDevice->GetItem();

	//	If we have a source and armor, find the item

	if (m_pArmor && m_pSource)
		{
		CItemListManipulator ItemList(m_pSource->GetItemList());
		m_pSource->SetCursorAtArmor(ItemList, m_pArmor);
		if (!ItemList.IsCursorValid())
			return NULL;

		return &ItemList.GetItemAtCursor();
		}

	//	Couldn't get it

	return NULL;
	}

const CItemEnhancement &CItemCtx::GetMods(void)

//	GetMods
//
//	Returns mods for the item

	{
	//	If we've got an item, get mods from there

	if (m_pItem)
		return m_pItem->GetMods();

	//	If we've got a device structure, then get mods from that

	if (m_pDevice)
		return m_pDevice->GetMods();

	//	If we've got an armor structure, then that also has mods

	if (m_pArmor)
		return m_pArmor->GetMods();

	//	Else, we have to get an item

	return GetItem().GetMods();
	}
