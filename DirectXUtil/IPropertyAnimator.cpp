//	PropertyAnimators.cpp
//
//	Property animator classes
//	Copyright (c) 2014 by Kronosaur Productions, LLC.

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

#include "Reanimator.h"

#define EVENT_ON_ANIMATION_DONE				CONSTLIT("onAnimationDone")

void IPropertyAnimator::AddListener (const CString &sEvent, IAniCommand *pListener, const CString &sCmd, DWORD dwData)

//	AddListener
//
//	Adds a listener to this animator

	{
	SAniEvent *pEntry = m_Listeners.Insert();
	pEntry->sEvent = sEvent;
	pEntry->pListener = pListener;
	pEntry->sCmd = sCmd;
	pEntry->dwData = dwData;
	}

void IPropertyAnimator::OnDoneAnimating (SAniUpdateCtx &Ctx)

//	OnDoneAnimating
//
//	Animation complete

	{
	int i;

	//	Loop over all listeners and add events to the context (they will fire
	//	when paint/update is done).

	for (i = 0; i < m_Listeners.GetCount(); i++)
		if (m_Listeners[i].sEvent.IsBlank() || strEquals(m_Listeners[i].sEvent, EVENT_ON_ANIMATION_DONE))
			Ctx.EventsToFire.Insert(m_Listeners[i]);
	}

void IPropertyAnimator::RaiseEvent (const CString &sID, const CString &sEvent)

//	RaiseEvent
//
//	Fires the event to all appropriate listeners

	{
	int i;

	//	Make a copy of events to raise (we need to do this because after firing
	//	any one event we might destroy everything).

	TArray<SAniEvent> Raised;

	for (i = 0; i < m_Listeners.GetCount(); i++)
		if (m_Listeners[i].sEvent.IsBlank() || strEquals(sEvent, m_Listeners[i].sEvent))
			Raised.Insert(m_Listeners[i]);

	//	Fire

	for (i = 0; i < Raised.GetCount(); i++)
		Raised[i].pListener->AniCommand(sID, sEvent, Raised[i].sCmd, Raised[i].dwData);
	}

void IPropertyAnimator::RemoveListener (IAniCommand *pListener, const CString &sEvent)

//	RemoveListener
//
//	Removes a listener

	{
	int i;

	for (i = 0; i < m_Listeners.GetCount(); i++)
		if (m_Listeners[i].pListener == pListener 
				&& (sEvent.IsBlank() || strEquals(m_Listeners[i].sEvent, sEvent)))
			{
			m_Listeners.Delete(i);
			i--;
			}
	}
