//	Events.cpp
//
//	Event classes

#include "PreComp.h"

void CTimedEvent::CreateFromStream (SLoadCtx &Ctx, CTimedEvent **retpEvent)

//	CreateFromStream
//
//	Creates a timed event from a stream
//
//	DWORD			class
//	DWORD			m_dwTick
//	DWORD			flags

	{
	//	Create the appropriate class

	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	CTimedEvent *pEvent;
	switch (dwLoad)
		{
		case cTimedEncounterEvent:
			pEvent = new CTimedEncounterEvent;
			break;

		case cTimedCustomEvent:
			pEvent = new CTimedCustomEvent;
			break;

		case cTimedRecurringEvent:
			pEvent = new CTimedRecurringEvent;
			break;

		case cTimedTypeEvent:
			pEvent = new CTimedTypeEvent;
			break;

		case cTimedMissionEvent:
			pEvent = new CTimedMissionEvent;
			break;
		}

	//	Load stuff

	Ctx.pStream->Read((char *)&pEvent->m_dwTick, sizeof(DWORD));

	//	Load flags

	if (Ctx.dwVersion >= 52)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		pEvent->m_bDestroyed = ((dwLoad & 0x00000001) ? true : false);
		}
	else
		pEvent->m_bDestroyed = false;
	

	//	Load subclass data

	pEvent->OnReadFromStream(Ctx);

	//	Done

	*retpEvent = pEvent;
	}

void CTimedEvent::WriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	WriteToStream
//
//	Writes the event to a stream
//
//	DWORD		class
//	DWORD		m_dwTick
//	DWORD		flags
//	Subclass data

	{
	OnWriteClassToStream(pStream);
	pStream->Write((char *)&m_dwTick, sizeof(DWORD));

	//	Flags

	DWORD dwSave = 0;
	dwSave |= (m_bDestroyed ? 0x00000001 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Sub class

	OnWriteToStream(pSystem, pStream);
	}

//	CTimedEncounterEvent ------------------------------------------------------

CTimedEncounterEvent::CTimedEncounterEvent (int iTick,
											CSpaceObject *pTarget,
											DWORD dwEncounterTableUNID,
											CSpaceObject *pGate,
											Metric rDistance) :
		CTimedEvent(iTick),
		m_pTarget(pTarget),
		m_dwEncounterTableUNID(dwEncounterTableUNID),
		m_pGate(pGate),
		m_rDistance(rDistance)

//	CTimedEncounterEvent constructor

	{
	}

CString CTimedEncounterEvent::DebugCrashInfo (void)

//	DebugCrashInfo

	{
	CString sResult;

	sResult.Append(CONSTLIT("TimedEncounterEvent\r\n"));
	return sResult;
	}

void CTimedEncounterEvent::DoEvent (DWORD dwTick, CSystem *pSystem)

//	DoEvent
//
//	Do the actual event

	{
	CShipTable *pTable = g_pUniverse->FindEncounterTable(m_dwEncounterTableUNID);
	if (pTable == NULL)
		{
		SetDestroyed();
		return;
		}

	SShipCreateCtx Ctx;
	Ctx.pSystem = pSystem;
	Ctx.pTarget = m_pTarget;
	Ctx.dwFlags = SShipCreateCtx::ATTACK_NEAREST_ENEMY;

	//	Figure out where the encounter will come from

	if (m_rDistance > 0.0)
		{
		if (m_pTarget)
			Ctx.vPos = m_pTarget->GetPos() + ::PolarToVector(mathRandom(0, 359), m_rDistance);
		Ctx.PosSpread = DiceRange(3, 1, 2);
		}
	else if (m_pGate && m_pGate->IsActiveStargate())
		Ctx.pGate = m_pGate;
	else if (m_pGate)
		{
		Ctx.vPos = m_pGate->GetPos();
		Ctx.PosSpread = DiceRange(2, 1, 2);
		}
	else if (m_pTarget)
		Ctx.pGate = m_pTarget->GetNearestStargate(true);

	//	Create ships

	pTable->CreateShips(Ctx);

	//	Delete the event

	SetDestroyed();
	}

bool CTimedEncounterEvent::OnObjDestroyed (CSpaceObject *pObj)

//	OnObjDestroyed
//
//	Returns TRUE if the event should be destroyed

	{
	return (m_pTarget == pObj);
	}

void CTimedEncounterEvent::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read data

	{
	Ctx.pStream->Read((char *)&m_dwEncounterTableUNID, sizeof(DWORD));
	CSystem::ReadObjRefFromStream(Ctx, &m_pTarget);
	CSystem::ReadObjRefFromStream(Ctx, &m_pGate);
	Ctx.pStream->Read((char *)&m_rDistance, sizeof(Metric));
	}

void CTimedEncounterEvent::OnWriteClassToStream (IWriteStream *pStream)

//	OnWriteClassToStream
//
//	Writes the class

	{
	DWORD dwSave = cTimedEncounterEvent;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CTimedEncounterEvent::OnWriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes the obj data to stream
//
//	DWORD		m_dwEncounterTableUNID
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pGate (CSpaceObject ref)
//	Metric		m_rDistance

	{
	pStream->Write((char *)&m_dwEncounterTableUNID, sizeof(DWORD));
	pSystem->WriteObjRefToStream(m_pTarget, pStream);
	pSystem->WriteObjRefToStream(m_pGate, pStream);
	pStream->Write((char *)&m_rDistance, sizeof(Metric));
	}

//	CTimedCustomEvent class --------------------------------------------------

CTimedCustomEvent::CTimedCustomEvent (int iTick,
									  CSpaceObject *pObj,
									  const CString &sEvent) :
		CTimedEvent(iTick),
		m_pObj(pObj),
		m_sEvent(sEvent)

//	CTimedCustomEvent constructor

	{
	}

CString CTimedCustomEvent::DebugCrashInfo (void)

//	DebugCrashInfo

	{
	CString sResult;

	sResult.Append(CONSTLIT("TimedCustomEvent\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("m_pObj: %s\r\n"), CSpaceObject::DebugDescribe(m_pObj)));
	sResult.Append(strPatternSubst(CONSTLIT("event: %s\r\n"), m_sEvent));
	return sResult;
	}

void CTimedCustomEvent::DoEvent (DWORD dwTick, CSystem *pSystem)

//	DoEvent
//
//	Do the actual event

	{
	if (m_pObj)
		m_pObj->FireCustomEvent(m_sEvent, eventDoEvent);

	SetDestroyed();
	}

bool CTimedCustomEvent::OnObjChangedSystems (CSpaceObject *pObj)

//	OnObjChangedSystems
//
//	Returns TRUE if the event should be carried into the new system

	{
	return (m_pObj == pObj);
	}

bool CTimedCustomEvent::OnObjDestroyed (CSpaceObject *pObj)

//	OnObjDestroyed
//
//	Returns TRUE if the event should be destroyed

	{
	return (m_pObj == pObj);
	}

void CTimedCustomEvent::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read data

	{
	CSystem::ReadObjRefFromStream(Ctx, (CSpaceObject **)&m_pObj);
	m_sEvent.ReadFromStream(Ctx.pStream);
	}

void CTimedCustomEvent::OnWriteClassToStream (IWriteStream *pStream)

//	OnWriteClassToStream
//
//	Writes the class

	{
	DWORD dwSave = cTimedCustomEvent;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CTimedCustomEvent::OnWriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes the obj data to stream
//
//	DWORD		m_pObj (CSpaceObject ref)
//	CString		m_sEvent

	{
	pSystem->WriteObjRefToStream(m_pObj, pStream);
	m_sEvent.WriteToStream(pStream);
	}

//	CTimedRecurringEvent class --------------------------------------------------

CTimedRecurringEvent::CTimedRecurringEvent (int iInterval,
											CSpaceObject *pObj,
											const CString &sEvent) :
		CTimedEvent(pObj->GetSystem()->GetTick() + mathRandom(0, iInterval)),
		m_iInterval(iInterval),
		m_pObj(pObj),
		m_sEvent(sEvent)

//	CTimedRecurringEvent constructor

	{
	}

CString CTimedRecurringEvent::DebugCrashInfo (void)

//	DebugCrashInfo

	{
	CString sResult;

	sResult.Append(CONSTLIT("TimedRecurringEvent\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("m_pObj: %s\r\n"), CSpaceObject::DebugDescribe(m_pObj)));
	sResult.Append(strPatternSubst(CONSTLIT("event: %s\r\n"), m_sEvent));
	return sResult;
	}

void CTimedRecurringEvent::DoEvent (DWORD dwTick, CSystem *pSystem)

//	DoEvent
//
//	Do the actual event

	{
	if (m_pObj)
		m_pObj->FireCustomEvent(m_sEvent, eventDoEvent);

	SetTick(dwTick + m_iInterval);
	}

bool CTimedRecurringEvent::OnObjChangedSystems (CSpaceObject *pObj)

//	OnObjChangedSystems
//
//	Returns TRUE if the event should be carried into the new system

	{
	return (m_pObj == pObj);
	}

bool CTimedRecurringEvent::OnObjDestroyed (CSpaceObject *pObj)

//	OnObjDestroyed
//
//	Returns TRUE if the event should be destroyed

	{
	return (m_pObj == pObj);
	}

void CTimedRecurringEvent::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read data

	{
	CSystem::ReadObjRefFromStream(Ctx, (CSpaceObject **)&m_pObj);
	m_sEvent.ReadFromStream(Ctx.pStream);
	Ctx.pStream->Read((char *)&m_iInterval, sizeof(DWORD));
	}

void CTimedRecurringEvent::OnWriteClassToStream (IWriteStream *pStream)

//	OnWriteClassToStream
//
//	Writes the class

	{
	DWORD dwSave = cTimedRecurringEvent;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CTimedRecurringEvent::OnWriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes the obj data to stream
//
//	DWORD		m_pObj (CSpaceObject ref)
//	CString		m_sEvent
//	DWORD		m_iInterval

	{
	pSystem->WriteObjRefToStream(m_pObj, pStream);
	m_sEvent.WriteToStream(pStream);
	pStream->Write((char *)&m_iInterval, sizeof(DWORD));
	}

//	CTimedTypeEvent ------------------------------------------------------------

CTimedTypeEvent::CTimedTypeEvent (int iTick,
								  int iInterval,
								  CDesignType *pType,
								  const CString &sEvent) :
		CTimedEvent(iTick),
		m_iInterval(iInterval),
		m_pType(pType),
		m_sEvent(sEvent)

//	CTimedTypeEvent constructor

	{
	}

CString CTimedTypeEvent::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Dump crash info

	{
	CString sResult;

	sResult.Append(CONSTLIT("TimedTypeEvent\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("m_pType: 0x%x\r\n"), (m_pType ? m_pType->GetUNID() : 0)));
	sResult.Append(strPatternSubst(CONSTLIT("event: %s\r\n"), m_sEvent));
	return sResult;
	}

void CTimedTypeEvent::DoEvent (DWORD dwTick, CSystem *pSystem)

//	DoEvent
//
//	Run the event

	{
	if (m_pType)
		m_pType->FireCustomEvent(m_sEvent, eventDoEvent);

	if (m_iInterval)
		SetTick(dwTick + m_iInterval);
	else
		SetDestroyed();
	}

void CTimedTypeEvent::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read from stream

	{
	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_pType = g_pUniverse->FindDesignType(dwLoad);

	m_sEvent.ReadFromStream(Ctx.pStream);
	Ctx.pStream->Read((char *)&m_iInterval, sizeof(DWORD));
	}

void CTimedTypeEvent::OnWriteClassToStream (IWriteStream *pStream)

//	OnWriteClassToStream
//
//	Write the event class

	{
	DWORD dwSave = cTimedTypeEvent;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CTimedTypeEvent::OnWriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	OnWriteToStream
//
//	Write to stream
//
//	DWORD		m_pType (UNID)
//	CString		m_sEvent
//	DWORD		m_iInterval

	{
	DWORD dwSave;

	dwSave = (m_pType ? m_pType->GetUNID() : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	m_sEvent.WriteToStream(pStream);
	pStream->Write((char *)&m_iInterval, sizeof(DWORD));
	}
