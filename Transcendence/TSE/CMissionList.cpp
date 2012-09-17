//	CMissionList.cpp
//
//	CMissionList class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

void CMissionList::Delete (int iIndex)

//	Delete
//
//	Delete the given mission

	{
	if (m_bFree)
		delete m_List[iIndex];

	m_List.Delete(iIndex);
	}

void CMissionList::Delete (CMission *pMission)

//	Delete
//
//	Delete the given mission

	{
	int iIndex;
	if (m_List.Find(pMission, &iIndex))
		Delete(iIndex);
	}

void CMissionList::DeleteAll (void)

//	DeleteAll
//
//	Delete all missions

	{
	int i;

	if (m_bFree)
		{
		for (i = 0; i < m_List.GetCount(); i++)
			delete m_List[i];
		}

	m_List.DeleteAll();
	}

CMission *CMissionList::GetMissionByID (DWORD dwID) const

//	GetMissionByID
//
//	Returns a mission of the given ID (or NULL if not found)

	{
	int i;

	for (i = 0; i < m_List.GetCount(); i++)
		if (m_List[i]->GetID() == dwID)
			return m_List[i];

	return NULL;
	}

void CMissionList::Insert (CMission *pMission)

//	Insert
//
//	Insert a new mission

	{
	m_List.Insert(pMission);
	}

ALERROR CMissionList::ReadFromStream (SLoadCtx &Ctx, CString *retsError)

//	ReadFromStream
//
//	Reads from stream
//
//	DWORD		Count
//	CMission

	{
	int i;

	DeleteAll();

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
			*retsError = CSpaceObject::DebugLoadError(Ctx);
			return ERR_FAIL;
			}

		//	Add to global missions

		m_List[i] = pObj->AsMission();
		}

	return NOERROR;
	}

ALERROR CMissionList::WriteToStream (IWriteStream *pStream, CString *retsError)

//	WriteToStream
//
//	Writes to a stream
//
//	DWORD		Count
//	CMission

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
			CString sError = CONSTLIT("Unable to save mission:\r\n");
			ReportCrashObj(&sError, m_List[i]);
			kernelDebugLogMessage(sError);
			return ERR_FAIL;
			}
		}

	return NOERROR;
	}
