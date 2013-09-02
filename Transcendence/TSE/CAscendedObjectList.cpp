//	CAscendedObjectList.cpp
//
//	CAscendedObjectList class
//	Copyright (c) 2013 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

void CAscendedObjectList::CleanUp (void)

//	CleanUp
//
//	Clean up the structure. We own all objects.

	{
	int i;

	for (i = 0; i < m_List.GetCount(); i++)
		delete m_List[i];

	m_List.DeleteAll();
	}

void CAscendedObjectList::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read
//
//	DWORD		Count
//	CSpaceObject

	{
	int i;

	CleanUp();

	DWORD dwCount;
	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	m_List.InsertEmpty(dwCount);

	for (i = 0; i < (int)dwCount; i++)
		{
		CSpaceObject *pObj;
		try
			{
			CSpaceObject::CreateFromStream(Ctx, &pObj);
			}
		catch (...)
			{
			CString sError = CSpaceObject::DebugLoadError(Ctx);
			kernelDebugLogMessage(sError);
			return;
			}

		//	Add to list

		m_List[i] = pObj;
		}
	}

CSpaceObject *CAscendedObjectList::RemoveByID (DWORD dwID)

//	RemoveByID
//
//	Removes the given object by ID. Returns NULL if not found.

	{
	int i;

	for (i = 0; i < m_List.GetCount(); i++)
		if (m_List[i]->GetID() == dwID)
			{
			CSpaceObject *pObj = m_List[i];
			m_List.Delete(i);
			return pObj;
			}

	return NULL;
	}

void CAscendedObjectList::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write

	{
	int i;

	DWORD dwCount = m_List.GetCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));

	for (i = 0; i < (int)dwCount; i++)
		{
		try
			{
			m_List[i]->WriteToStream(pStream);
			}
		catch (...)
			{
			CString sError = CONSTLIT("Unable to save ascended object:\r\n");
			ReportCrashObj(&sError, m_List[i]);
			kernelDebugLogMessage(sError);
			return;
			}
		}
	}
