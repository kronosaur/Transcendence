//	CSystemCreateEvents.cpp
//
//	CSystemCreateEvents class

#include "PreComp.h"

void CSystemCreateEvents::AddDeferredEvent (CSpaceObject *pObj, CExtension *pExtension, CXMLElement *pEventCode)

//	AddDeferredEvent
//
//	Adds a deferred event. The caller must guarantee the lifetime of pObj and pEventCode

	{
	SEventDesc *pEvent = m_Events.Insert();
	pEvent->pObj = pObj;
	pEvent->pExtension = pExtension;
	pEvent->pEventCode = pEventCode;
	}

ALERROR CSystemCreateEvents::FireDeferredEvent (const CString &sEvent, CString *retsError)

//	FireDeferredEvent
//
//	Fires the given event for all objects in the deferred list

	{
	int i;
	CCodeChainCtx Ctx;

	for (i = 0; i < m_Events.GetCount(); i++)
		{
		if (strEquals(sEvent, m_Events[i].pEventCode->GetTag()))
			{
			//	Link the code

			ICCItem *pCode = Ctx.Link(m_Events[i].pEventCode->GetContentText(0), 0, NULL);
			if (pCode->IsError())
				{
				if (retsError)
					*retsError = pCode->GetStringValue();

				Ctx.Discard(pCode);
				return ERR_FAIL;
				}

			//	Execute

			Ctx.SetExtension(m_Events[i].pExtension);
			Ctx.SaveAndDefineSourceVar(m_Events[i].pObj);

			ICCItem *pResult = Ctx.Run(pCode);
			Ctx.Discard(pCode);

			//	Check error

			if (pResult->IsError())
				{
				if (retsError)
					*retsError = pResult->GetStringValue();

				Ctx.Discard(pResult);
				return ERR_FAIL;
				}

			Ctx.Discard(pResult);
			}
		}

	return NOERROR;
	}
