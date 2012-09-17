//	CGlobalSpaceObject.cpp
//
//	CGlobalSpaceObject class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

void CGlobalSpaceObject::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads from a stream
//
//	DWORD		Obj ID

	{
	Ctx.pStream->Read((char *)&m_dwID, sizeof(DWORD));

	//	We wait until the whole system is read before resolving.
	}

void CGlobalSpaceObject::Resolve (void)

//	Resolve
//
//	Resolves the object pointer

	{
	if (m_dwID != OBJID_NULL)
		{
		CSystem *pSystem = g_pUniverse->GetCurrentSystem();
		if (pSystem)
			m_pObj = pSystem->FindObject(m_dwID);
		else
			m_pObj = NULL;
		}
	}

void CGlobalSpaceObject::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write to stream
//
//	DWORD		Obj ID

	{
	pStream->Write((char *)&m_dwID, sizeof(DWORD));
	}
