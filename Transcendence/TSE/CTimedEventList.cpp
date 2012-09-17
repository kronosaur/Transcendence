//	CTimedEventList.cpp
//
//	CTimedEventList class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

CTimedEventList::~CTimedEventList (void)

//	CTimedEvent destructor

	{
	DeleteAll();
	}

bool CTimedEventList::CancelEvent (CSpaceObject *pObj, bool bInDoEvent)

//	CancelEvent
//
//	Cancels the given event

	{
	int i;
	bool bFound = false;

	for (i = 0; i < GetCount(); i++)
		{
		CTimedEvent *pEvent = GetEvent(i);
		if (pEvent->GetEventHandlerObj() == pObj)
			{
			bFound = true;

			if (bInDoEvent)
				pEvent->SetDestroyed();
			else
				{
				RemoveEvent(i);
				i--;
				}
			}
		}

	return bFound;
	}

bool CTimedEventList::CancelEvent (CSpaceObject *pObj, const CString &sEvent, bool bInDoEvent)

//	CancelEvent
//
//	Cancels the given event

	{
	int i;
	bool bFound = false;

	for (i = 0; i < GetCount(); i++)
		{
		CTimedEvent *pEvent = GetEvent(i);
		if (pEvent->GetEventHandlerObj() == pObj 
				&& strEquals(pEvent->GetEventHandlerName(), sEvent))
			{
			bFound = true;

			if (bInDoEvent)
				pEvent->SetDestroyed();
			else
				{
				RemoveEvent(i);
				i--;
				}
			}
		}

	return bFound;
	}

void CTimedEventList::DeleteAll (void)

//	DeleteAll
//
//	Delete all events

	{
	int i;

	for (i = 0; i < m_List.GetCount(); i++)
		delete m_List[i];

	m_List.DeleteAll();
	}

void CTimedEventList::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads from a stream
//
//	DWORD		No of events
//	CTimedEvent	Event

	{
	int i;
	DWORD dwCount;

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		CTimedEvent *pEvent;
		CTimedEvent::CreateFromStream(Ctx, &pEvent);
		AddEvent(pEvent);
		}
	}

void CTimedEventList::Update (DWORD dwTick, CSystem *pSystem)

//	Update
//
//	Fire events

	{
	int i;

	for (i = 0; i < GetCount(); i++)
		{
		CTimedEvent *pEvent = GetEvent(i);
		SetProgramEvent(pEvent);

		if (!pEvent->IsDestroyed() && pEvent->GetTick() <= dwTick)
			pEvent->DoEvent(dwTick, pSystem);
		}

	SetProgramEvent(NULL);

	//	Delete events that were destroyed

	for (i = 0; i < GetCount(); i++)
		{
		CTimedEvent *pEvent = GetEvent(i);
		if (pEvent->IsDestroyed())
			{
			RemoveEvent(i);
			i--;
			}
		}
	}

void CTimedEventList::WriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	WriteToStream
//
//	Writes to a stream
//
//	DWORD		No of events
//	CTimedEvent	Event

	{
	int i;

	DWORD dwCount = GetCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));

	for (i = 0; i < (int)dwCount; i++)
		{
		CTimedEvent *pEvent = GetEvent(i);
		pEvent->WriteToStream(pSystem, pStream);
		}
	}

