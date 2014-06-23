//	CEventHandler.cpp
//
//	CEventHandler object

#include "PreComp.h"

CEventHandler::CEventHandler (void)

//	CEventHandler constructor

	{
	}

CEventHandler::~CEventHandler (void)

//	CEventHandler destructor

	{
	DeleteAll();
	}

CEventHandler &CEventHandler::operator= (const CEventHandler &Src)

//	CEventHandler equals operator

	{
	int i;

	//	Copy the data

	DeleteAll();
	m_Handlers = Src.m_Handlers;

	//	Add a reference to every item

	for (i = 0; i < m_Handlers.GetCount(); i++)
		{
		if (m_Handlers[i])
			m_Handlers[i] = m_Handlers[i]->Reference();
		}

	return *this;
	}

void CEventHandler::AddEvent (const CString &sEvent, ICCItem *pCode)

//	AddEvent
//
//	Adds an event

	{
	m_Handlers.Insert(sEvent, pCode);
	}

ALERROR CEventHandler::AddEvent (const CString &sEvent, const CString &sCode, CString *retsError)

//	AddEvent
//
//	Adds an event

	{
	ICCItem *pCode = g_pUniverse->GetCC().Link(sCode, 0, NULL);
	if (pCode->IsError())
		{
		if (retsError)
			*retsError = pCode->GetStringValue();
		return ERR_FAIL;
		}

	m_Handlers.Insert(sEvent, pCode);

	return NOERROR;
	}

void CEventHandler::DeleteAll (void)

//	DeleteAll
//
//	Delete all events

	{
	for (int i = 0; i < m_Handlers.GetCount(); i++)
		{
		ICCItem *pItem = m_Handlers[i];
		pItem->Discard(&g_pUniverse->GetCC());
		}

	m_Handlers.DeleteAll();
	}

bool CEventHandler::FindEvent (const CString &sEvent, ICCItem **retpCode) const

//	FindEvent
//
//	Finds the event handler by name

	{
	ICCItem **pCode = m_Handlers.GetAt(sEvent);
	if (pCode)
		{
		if (retpCode)
			*retpCode = *pCode;

		return true;
		}

	return false;
	}

const CString &CEventHandler::GetEvent (int iIndex, ICCItem **retpCode) const

//	GetEvent
//
//	Returns the event by index

	{
	if (retpCode)
		*retpCode = m_Handlers[iIndex];

	return m_Handlers.GetKey(iIndex);
	}

ALERROR CEventHandler::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitFromXML
//
//	Load all handlers

	{
	int i;

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pHandler = pDesc->GetContentElement(i);
		ICCItem *pCode = g_pUniverse->GetCC().Link(pHandler->GetContentText(0), 0, NULL);
		if (pCode->IsError())
			{
			Ctx.sError = strPatternSubst("<%s> event: %s", pHandler->GetTag(), pCode->GetStringValue());
			return ERR_FAIL;
			}

		//	If this is an old extension, then make sure the code is not using the
		//	gStation variable, because we no longer support it

		if (Ctx.GetAPIVersion() < 2)
			{
			if (g_pUniverse->GetCC().HasIdentifier(pCode, CONSTLIT("gStation")))
				{
				Ctx.sError = CONSTLIT("gStation variable has been deprecated--use gSource instead.");
				return ERR_FAIL;
				}
			}

		//	Done

		m_Handlers.Insert(pHandler->GetTag(), pCode);
		}

	return NOERROR;
	}

void CEventHandler::MergeFrom (const CEventHandler &Src)

//	MergeFrom
//
//	Merges from the source

	{
	int i;
	CCodeChain &CC(g_pUniverse->GetCC());

	for (i = 0; i < Src.GetCount(); i++)
		{
		ICCItem **ppCode = m_Handlers.GetAt(Src.m_Handlers.GetKey(i));
		if (ppCode)
			{
			(*ppCode)->Discard(&CC);
			(*ppCode) = Src.m_Handlers.GetValue(i)->Reference();
			}
		else
			m_Handlers.SetAt(Src.m_Handlers.GetKey(i), Src.m_Handlers.GetValue(i)->Reference());
		}
	}
