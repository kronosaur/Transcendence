//	CSystemEventHandler.cpp
//
//	CSystemEventHandler class

#include "PreComp.h"

CSystemEventHandler::CSystemEventHandler (void) : m_pHandler(NULL)

//	CSystemEventHandler constructor

	{
	}

void CSystemEventHandler::Create (CSpaceObject *pObj, Metric rMaxRange, CSystemEventHandler **retpHandler)

//	Create
//
//	Create a new object

	{
	CSystemEventHandler *pHandler = new CSystemEventHandler;

	pHandler->m_pHandler = pObj;
	pHandler->m_rMaxRange2 = rMaxRange * rMaxRange;

	*retpHandler = pHandler;
	}

bool CSystemEventHandler::InRange (const CVector &vPos) const

//	InRange
//
//	Returns TRUE if this event handler is in range of an event at the given pos

	{
	return (m_pHandler->GetPos() - vPos).Length2() < m_rMaxRange2; 
	}

void CSystemEventHandler::OnObjDestroyed (CSpaceObject *pObjDestroyed, bool *retbRemoveNode)

//	OnObjDestroyed
//
//	Notification that an object has been destroyed

	{
	*retbRemoveNode = (pObjDestroyed == m_pHandler);
	}

void CSystemEventHandler::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	CSpaceObject	m_pHandler
//	Metric			m_rMaxRange2

	{
	CSystem::ReadObjRefFromStream(Ctx, &m_pHandler);
	Ctx.pStream->Read((char *)&m_rMaxRange2, sizeof(Metric));
	}

void CSystemEventHandler::OnWriteToStream (CSystem *pSystem, IWriteStream *pStream) const

//	OnWriteToStream
//
//	CSpaceObject	m_pHandler
//	Metric			m_rMaxRange2

	{
	pSystem->WriteObjRefToStream(m_pHandler, pStream);
	pStream->Write((char *)&m_rMaxRange2, sizeof(Metric));
	}
