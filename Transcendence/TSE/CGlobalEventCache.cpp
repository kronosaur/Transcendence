//	CGlobalEventCache.cpp
//
//	CGlobalEventCache class

#include "PreComp.h"

bool CGlobalEventCache::Insert (CDesignType *pType, const CString &sEvent, const SEventHandlerDesc &Event)

//	Insert
//
//	Insert the given event handler to the cache

	{
	//	If this is not the event that we handle, then ignore

	if (!strEquals(sEvent, m_sEvent))
		return false;

	//	If the design type is an adventure desc that is not being used,
	//	then ignore it.

	if (pType->GetType() == designAdventureDesc)
		{
		CAdventureDesc *pDesc = CAdventureDesc::AsType(pType);
		if (!pDesc->IsCurrentAdventure())
			return false;
		}

	//	Add it

	SEntry *pNew = m_Cache.Insert();
	pNew->pType = pType;
	pNew->Event = Event;
	return true;
	}

