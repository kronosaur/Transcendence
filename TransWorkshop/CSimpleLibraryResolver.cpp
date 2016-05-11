//	CLibraryResolver.cpp
//
//	CLibraryResolver class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define LIBRARY_TAG									CONSTLIT("Library")

#define RELEASE_ATTRIB								CONSTLIT("release")
#define UNID_ATTRIB									CONSTLIT("unid")

ALERROR CSimpleLibraryResolver::AddLibrary (DWORD dwUNID, CString *retsError)

//  AddLibrary
//
//  Adds a library to our list of entity tables.

    {
	//	Get the best extension with this UNID. If we don't find it, then
	//	continue (we will report an error later when we can't find
	//	the entity).

    CExternalEntityTable *pTable;
	if (!m_Extensions.FindLibraryEntities(dwUNID, &pTable))
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to find library: %08x"), dwUNID);
		return ERR_FAIL;
		}

	//	Add it to our list so that we can use it to resolve entities.

    AddTable(pTable);

	return NOERROR;
    }

ALERROR CSimpleLibraryResolver::OnOpenTag (CXMLElement *pElement, CString *retsError)

//  OnOpenTag
//
//  Handle library references

    {
	if (strEquals(pElement->GetTag(), LIBRARY_TAG))
		{
		DWORD dwUNID = pElement->GetAttributeInteger(UNID_ATTRIB);
		DWORD dwRelease = pElement->GetAttributeInteger(RELEASE_ATTRIB);

		return AddLibrary(dwUNID, retsError);
		}

	return NOERROR;
    }

CString CSimpleLibraryResolver::ResolveExternalEntity (const CString &sName, bool *retbFound)

//  ResolveExternalEntity
//
//  Resolve an entity.

    {
	int i;

	for (i = 0; i < m_Tables.GetCount(); i++)
		{
		bool bFound;
		CString sResult = m_Tables[i]->ResolveExternalEntity(sName, &bFound);
		if (bFound)
			{
			if (retbFound)
				*retbFound = true;
			return sResult;
			}
		}

	//	Not found

	if (retbFound)
		*retbFound = false;

	return NULL_STR;
    }
