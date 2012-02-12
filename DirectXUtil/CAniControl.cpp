//	CAniControl.cpp
//
//	CAniControl class
//	Copyright (c) 2011 by Kronosaur Productions, LLC. All Rights Reserved.

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

#include "Reanimator.h"

CAniControl::~CAniControl (void)

//	CAniControl destructor

	{
	int i;

	for (i = 0; i < m_Styles.GetCount(); i++)
		if (m_Styles[i].pAni)
			delete m_Styles[i].pAni;
	}

void CAniControl::AddListener (const CString &sEvent, IAniCommand *pListener, const CString &sCmd, DWORD dwData)

//	AddListener
//
//	Adds a listener

	{
	SListener *pEntry = m_Listeners.Insert();
	pEntry->sEvent = sEvent;
	pEntry->pListener = pListener;
	pEntry->sCmd = sCmd;
	pEntry->dwData = dwData;
	}

IAnimatron *CAniControl::GetStyle (const CString &sComponent) const

//	GetStyle
//
//	Returns a style by name

	{
	//	Ask our derrived class to map a name to an index

	int iIndex = MapStyleName(sComponent);
	if (iIndex == -1)
		return NULL;

	return GetStyle(iIndex);
	}

void CAniControl::RaiseEvent (const CString &sEvent)

//	RaiseEvent
//
//	Fires the event to all appropriate listeners

	{
	int i;

	//	Make a copy of events to raise (we need to do this because after firing
	//	any one event we might destroy everything).

	CString sID = GetID();
	TArray<SListener> Raised;

	for (i = 0; i < m_Listeners.GetCount(); i++)
		if (m_Listeners[i].sEvent.IsBlank() || strEquals(sEvent, m_Listeners[i].sEvent))
			Raised.Insert(m_Listeners[i]);

	//	Fire

	for (i = 0; i < Raised.GetCount(); i++)
		Raised[i].pListener->AniCommand(sID, sEvent, Raised[i].sCmd, Raised[i].dwData);
	}

void CAniControl::RemoveListener (IAniCommand *pListener, const CString &sEvent)

//	RemoveListener
//
//	Removes the given listener

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

void CAniControl::SetStyle (int iIndex, IAnimatron *pImpl)

//	SetStyle
//
//	Sets a style by index

	{
	int iCount = m_Styles.GetCount();
	if (iIndex >= iCount)
		m_Styles.InsertEmpty(iIndex - iCount + 1);

	//	Free

	if (m_Styles[iIndex].pAni)
		delete m_Styles[iIndex].pAni;

	m_Styles[iIndex].pAni = pImpl;
	}

void CAniControl::SetStyle (const CString &sComponent, IAnimatron *pImpl)

//	SetStyle
//
//	Sets a style by name

	{
	//	Ask our derrived class to map a name to an index

	int iIndex = MapStyleName(sComponent);
	if (iIndex == -1)
		return;

	SetStyle(iIndex, pImpl);
	}
