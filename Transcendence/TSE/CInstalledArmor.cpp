//	CInstalledArmor.cpp
//
//	CInstalledArmor class

#include "PreComp.h"

void CInstalledArmor::FinishInstall (CSpaceObject *pSource)

//	FinishInstall
//
//	Finishes the parts of the install that fire events
//	(At object creation time these events fire after the entire object
//	is created).

	{
	m_pItem->FireOnInstall(pSource);

	CShip *pShip = pSource->AsShip();
	if (pShip)
		pShip->GetController()->OnItemInstalled(*m_pItem);
	}

void CInstalledArmor::Install (CSpaceObject *pObj, CItemListManipulator &ItemList, int iSect, bool bInCreate)

//	Install
//
//	Install the armor item

	{
	CItem *pItem = ItemList.GetItemPointerAtCursor();
	ASSERT(pItem);

	CItemCtx ItemCtx(pItem, pObj);

	CItemType *pType = pItem->GetType();
	ASSERT(pType);

	m_pArmorClass = pType->GetArmorClass();
	ASSERT(m_pArmorClass);

	m_iSect = iSect;
	m_fComplete = false;
	m_iHitPoints = m_pArmorClass->GetMaxHP(ItemCtx);
	if (pItem->IsDamaged())
		m_iHitPoints = m_iHitPoints / 2;

	//	Mark the item as installed

	ItemList.SetInstalledAtCursor(iSect);

	//	After we've installed, set the item pointer
	//	(we have to wait until after installation so that we
	//	have the proper pointer)

	m_pItem = ItemList.GetItemPointerAtCursor();

	//	Event (when creating a ship we wait until the
	//	whole ship is created before firing the event)

	if (!bInCreate)
		FinishInstall(pObj);
	}

void CInstalledArmor::ReadFromStream (CSpaceObject *pSource, int iSect, SLoadCtx &Ctx)

//	ReadFromStream
//
//	DWORD		class UNID
//	DWORD		m_iSect
//	DWORD		hit points
//	DWORD		mods
//	DWORD		flags

	{
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_pArmorClass = g_pUniverse->FindArmor(dwLoad);

	if (Ctx.dwVersion >= 54)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_iSect = dwLoad;
		}
	else
		m_iSect = iSect;

	//	The caller is responsible for initializing this for earlier versions

	Ctx.pStream->Read((char *)&m_iHitPoints, sizeof(DWORD));

	//	Previous versions saved mods

	if (Ctx.dwVersion < 59)
		{
		CItemEnhancement Dummy;
		Dummy.ReadFromStream(Ctx);
		}

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_fComplete =		((dwLoad & 0x00000001) ? true : false);

	//	Fix up the item pointer

	m_pItem = NULL;
	if (m_pArmorClass != NULL && m_iSect != -1)
		{
		CItemListManipulator ItemList(pSource->GetItemList());
		pSource->SetCursorAtArmor(ItemList, this);
		if (ItemList.IsCursorValid())
			m_pItem = ItemList.GetItemPointerAtCursor();
		}
	}

void CInstalledArmor::SetComplete (CSpaceObject *pSource, bool bComplete)

//	SetComplete
//
//	Adds the armor complete bonus

	{
	if (bComplete != m_fComplete)
		{
		m_fComplete = bComplete;

		if (m_fComplete)
			m_iHitPoints += m_pArmorClass->GetCompleteBonus();
		else
			m_iHitPoints = Min(m_iHitPoints, GetMaxHP(pSource));
		}
	}

void CInstalledArmor::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	DWORD		class UNID
//	DWORD		m_iSect
//	DWORD		hit points
//	DWORD		flags

	{
	DWORD dwSave = m_pArmorClass->GetUNID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = m_iSect;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_iHitPoints, sizeof(DWORD));

	dwSave = 0;
	dwSave |= (m_fComplete ?	0x00000001 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}
