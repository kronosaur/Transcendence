//	CItemEventDispatcher.cpp
//
//	CItemEventDispatcher object

#include "PreComp.h"

#define ON_AI_UPDATE_EVENT						CONSTLIT("OnAIUpdate")
#define ON_UPDATE_EVENT							CONSTLIT("OnUpdate")

CItemEventDispatcher::CItemEventDispatcher (void) : m_pFirstEntry(NULL)

//	CItemEventDispatcher constructor

	{
	}

CItemEventDispatcher::~CItemEventDispatcher (void)

//	CItemEventDispatcher destructor

	{
	RemoveAll();
	}

CItemEventDispatcher::SEntry *CItemEventDispatcher::AddEntry (void)

//	AddEntry
//
//	Adds a new entry to the beginning of the list

	{
	SEntry *pEntry = new SEntry;
	pEntry->pNext = m_pFirstEntry;
	m_pFirstEntry = pEntry;
	return pEntry;
	}

void CItemEventDispatcher::Init (CSpaceObject *pSource)

//	Init
//
//	Initializes the dispatcher from the item list

	{
	int i;

	RemoveAll();

	CItemListManipulator ItemList(pSource->GetItemList());
	while (ItemList.MoveCursorForward())
		{
		const CItem &Item = ItemList.GetItemAtCursor();

		//	Add entries for update events: OnAIUpdate and OnUpdate

		const CEventHandler &Handlers = Item.GetType()->GetEventHandlers();
		for (i = 0; i < Handlers.GetCount(); i++)
			{
			ECodeChainEvents iEvent;

			ICCItem *pCode;
			CString sEvent = Handlers.GetEvent(i, &pCode);
			if (strEquals(sEvent, ON_AI_UPDATE_EVENT))
				iEvent = eventOnAIUpdate;
			else if (strEquals(sEvent, ON_UPDATE_EVENT))
				iEvent = eventOnUpdate;
			else
				iEvent = eventNone;

			if (iEvent != eventNone)
				{
				SEntry *pEntry = AddEntry();
				pEntry->iType = dispatchFireEvent;
				pEntry->iEvent = iEvent;
				pEntry->pCode = pCode;
				pEntry->theItem = Item;
				pEntry->dwEnhancementID = OBJID_NULL;
				}
			}

		//	If this item has mods, see if we need to call any mods
		//	Add entries for OnEnhancementUpdate

		CItemType *pModType;
		if (Item.HasMods()
				&& (pModType = Item.GetMods().GetEnhancementType()))
			{
			//	If the mod has an expiration time, add a check here

			if (Item.GetMods().GetExpireTime() != -1)
				{
				SEntry *pEntry = AddEntry();
				pEntry->iType = dispatchCheckEnhancementLifetime;
				pEntry->iEvent = eventNone;
				pEntry->pCode = NULL;
				pEntry->theItem = Item;
				pEntry->dwEnhancementID = Item.GetMods().GetID();
				}
			}
		}
	}

void CItemEventDispatcher::FireEventFull (CSpaceObject *pSource, ECodeChainEvents iEvent)

//	FireEventFull
//
//	Fires the given event

	{
	CCodeChainCtx Ctx;
	bool bSavedVars = false;

	//	Fire event for all items that have it

	SEntry *pEntry = m_pFirstEntry;
	while (pEntry)
		{
		if (pEntry->iEvent == iEvent)
			{
			//	We don't bother saving the variables unless we've got an event

			if (!bSavedVars)
				{
				Ctx.SaveAndDefineSourceVar(pSource);
				Ctx.SaveItemVar();
				bSavedVars = true;
				}

			//	Set the item

			Ctx.DefineItem(pEntry->theItem);

			//	Run code

			ICCItem *pResult = Ctx.Run(pEntry->pCode);
			if (pResult->IsError())
				pSource->ReportEventError(strPatternSubst(CONSTLIT("Item %x Event"), pEntry->theItem.GetType()->GetUNID()), pResult);
			Ctx.Discard(pResult);
			}

		pEntry = pEntry->pNext;
		}
	}

void CItemEventDispatcher::FireUpdateEventsFull (CSpaceObject *pSource)

//	FireUpdateEventsFull
//
//	Fires all events at item update time

	{
	CCodeChainCtx Ctx;
	bool bSavedVars = false;

	//	Fire event for all items that have it

	SEntry *pEntry = m_pFirstEntry;
	while (pEntry)
		{
		//	Fire OnUpdate event

		if (pEntry->iEvent == eventOnUpdate)
			{
			//	We don't bother saving the variables unless we've got an event

			if (!bSavedVars)
				{
				Ctx.SaveAndDefineSourceVar(pSource);
				Ctx.SaveItemVar();
				bSavedVars = true;
				}

			//	Set the item

			Ctx.DefineItem(pEntry->theItem);

			//	Run code

			ICCItem *pResult = Ctx.Run(pEntry->pCode);
			if (pResult->IsError())
				pSource->ReportEventError(strPatternSubst(CONSTLIT("Item %x Event"), pEntry->theItem.GetType()->GetUNID()), pResult);
			Ctx.Discard(pResult);
			}

		//	Check for enhancement expiration

		else if (pEntry->iType == dispatchCheckEnhancementLifetime)
			{
			//	Remove any expired mods on the item
			//	(TRUE flag means, "removed expired only")

			pSource->RemoveItemEnhancement(pEntry->theItem, pEntry->dwEnhancementID, true);
			}

		pEntry = pEntry->pNext;
		}
	}

void CItemEventDispatcher::RemoveAll (void)

//	RemoveAll
//
//	Remove all entries

	{
	SEntry *pEntry = m_pFirstEntry;
	while (pEntry)
		{
		SEntry *pDelete = pEntry;
		pEntry = pEntry->pNext;
		delete pDelete;
		}

	m_pFirstEntry = NULL;
	}
