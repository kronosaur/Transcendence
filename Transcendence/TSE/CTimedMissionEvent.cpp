//	CTimedMissionEvent.cpp
//
//	CTimeMissionEvent class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

CTimedMissionEvent::CTimedMissionEvent (int iTick,
										int iInterval,
										CMission *pMission,
										const CString &sEvent) :
		CTimedEvent(iTick),
		m_iInterval(iInterval),
		m_pMission(pMission),
		m_sEvent(sEvent)

//	CTimedMissionEvent constructor

	{
	}

CString CTimedMissionEvent::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Dump crash info

	{
	CString sResult;

	sResult.Append(CONSTLIT("TimedMissionEvent\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("m_pMission: 0x%x\r\n"), (m_pMission ? m_pMission->GetID() : 0)));
	sResult.Append(strPatternSubst(CONSTLIT("event: %s\r\n"), m_sEvent));
	return sResult;
	}

void CTimedMissionEvent::DoEvent (DWORD dwTick, CSystem *pSystem)

//	DoEvent
//
//	Run the event

	{
	if (m_pMission)
		m_pMission->FireCustomEvent(m_sEvent, NULL);

	if (m_iInterval)
		SetTick(dwTick + m_iInterval);
	else
		SetDestroyed();
	}

void CTimedMissionEvent::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read from stream

	{
	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_pMission = g_pUniverse->FindMission(dwLoad);

	m_sEvent.ReadFromStream(Ctx.pStream);
	Ctx.pStream->Read((char *)&m_iInterval, sizeof(DWORD));
	}

void CTimedMissionEvent::OnWriteClassToStream (IWriteStream *pStream)

//	OnWriteClassToStream
//
//	Write the event class

	{
	DWORD dwSave = cTimedMissionEvent;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CTimedMissionEvent::OnWriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	OnWriteToStream
//
//	Write to stream
//
//	DWORD		m_pType (UNID)
//	CString		m_sEvent
//	DWORD		m_iInterval

	{
	DWORD dwSave;

	dwSave = (m_pMission ? m_pMission->GetID() : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	m_sEvent.WriteToStream(pStream);
	pStream->Write((char *)&m_iInterval, sizeof(DWORD));
	}
