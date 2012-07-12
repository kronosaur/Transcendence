//	CTerritoryList.cpp
//
//	CTerritoryList class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

CTerritoryList::~CTerritoryList (void)

//	CTerritoryList destructor

	{
	int i;

	for (i = 0; i < m_List.GetCount(); i++)
		delete m_List[i];
	}

bool CTerritoryList::HasAttribute (const CVector &vPos, const CString &sAttrib)

//	HasAttribute
//
//	Returns TRUE if the given point has the given attribute

	{
	int i, j;

	for (i = 0; i < m_List.GetCount(); i++)
		{
		//	Skipped marked items (because we are in the middle
		//	of evaluating them)

		if (m_List[i]->IsMarked())
			continue;

		//	Test

		if (m_List[i]->HasAttribute(vPos, sAttrib))
			{
			//	If we have the attribute, make sure that we match
			//	the criteria, if any.

			const CString &sCriteria = m_List[i]->GetCriteria();
			if (!sCriteria.IsBlank())
				{
				//	Mark this territory so we don't try to evaluate it

				m_List[i]->SetMarked(true);

				//	Parse the criteria

				CString sError;
				CAttributeCriteria Criteria;
				if (Criteria.Parse(sCriteria, 0, &sError) != NOERROR)
					kernelDebugLogMessage("Error parsing territory: %s", sError);

				//	Loop over all elements of the criteria

				bool bMatches = true;
				for (j = 0; j < Criteria.GetCount(); j++)
					{
					bool bRequired;
					const CString &sAttrib = Criteria.GetAttribAndRequired(j, &bRequired);

					//	Recusively see if we have the attribute

					bool bFound = HasAttribute(vPos, sAttrib);

					//	If we don't match, then we're done

					if (bFound != bRequired)
						{
						bMatches = false;
						break;
						}
					}

				//	Restore

				m_List[i]->SetMarked(false);

				//	If we matched, then we're done

				if (bMatches)
					return true;
				}

			//	If we have no criteria, then we match

			else
				return true;
			}
		}

	return false;
	}

void CTerritoryList::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads the structure from a stream
//
//	DWORD			Count
//	CTerritoryDef	Entries

	{
	int i;
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_List.InsertEmpty(dwLoad);

	for (i = 0; i < (int)dwLoad; i++)
		{
		CTerritoryDef *pDef = new CTerritoryDef;
		pDef->ReadFromStream(Ctx);

		m_List[i] = pDef;
		}
	}

void CTerritoryList::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Writes the structure to a stream
//
//	DWORD			Count
//	CTerritoryDef	Entries

	{
	int i;
	DWORD dwSave;

	dwSave = m_List.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < m_List.GetCount(); i++)
		m_List[i]->WriteToStream(pStream);
	}
